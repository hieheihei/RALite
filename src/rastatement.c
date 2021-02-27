#include "sqliteInt.h"

static SrcList* subqueryFromSrc(Parse *pParse,Select* subquery){
    Token nullAlias;
    return sqlite3SrcListAppendFromTerm(
        pParse,
        0,
        0,
        0,
        &nullAlias,
        subquery,
        0,
        0);
}

static SrcList* twoSubqueryFromSrc(Parse *pParse,Select* subquery1,Select* subquery2){
    Token nullAlias = {0,0};
    SrcList* psrc =  sqlite3SrcListAppendFromTerm(
        pParse,
        0,
        0,
        0,
        &nullAlias,
        subquery1,
        0,
        0);
    return sqlite3SrcListAppendFromTerm(
        pParse,
        psrc,
        0,
        0,
        &nullAlias,
        subquery2,
        0,
        0);
}

void raExecutSelectCommand(Parse *pParse,Select * pselect){
    SelectDest dest = {SRT_Output, 0, 0, 0, 0, 0, 0};
    sqlite3Select(pParse, pselect, &dest);
    sqlite3SelectDelete(pParse->db, pselect);
}

/**
 * R
 * select * from R
 * */
Select * raSelectRelationship(Parse *pParse,Token *pTable){
    ExprList* pselcollist = sqlite3ExprListAppend(pParse, 0, sqlite3Expr(pParse->db, TK_ASTERISK, 0));
    SrcList* pSrc = sqlite3SrcListAppend(pParse,0,pTable,0);
    return sqlite3SelectNew(pParse,pselcollist,pSrc,0,0,0,0,SF_Distinct,0);
}

/**
 * pi L(R)
 * select L from R
 * */
Select* raCalculateProjectionOp(Parse *pParse,ExprList* pselcollist,Select* prelationshipR){
    return sqlite3SelectNew(
        pParse,
        pselcollist,
        subqueryFromSrc(pParse,prelationshipR),
        0,0,0,0,SF_Distinct,0);
}

/**
 * sigma c(R)
 * select * from R where c
 * */
Select* raCalculateSelectOp(Parse *pParse,Select* prelationshipR,Expr *pWhere){
    ExprList* pselcollist = sqlite3ExprListAppend(pParse, 0, sqlite3Expr(pParse->db, TK_ASTERISK, 0));

     return sqlite3SelectNew(
         pParse,
          pselcollist,
          subqueryFromSrc(pParse,prelationshipR),
          pWhere,
          0,0,0,SF_Distinct,0);
}

/**
 * R setOp S
 * setOp : UNION | EXCEP | INTERSECT
 * 
 * */
Select* raCalculateSetOp(Parse *pParse,Select *pLhs,Select *pRhs,int setOp){
  if( pRhs && pRhs->pPrior ){
    SrcList *pFrom;
    Token x;
    x.n = 0;
    pFrom = sqlite3SrcListAppendFromTerm(pParse,0,0,0,&x,pRhs,0,0);
    pRhs = sqlite3SelectNew(pParse,0,pFrom,0,0,0,0,0,0);
  }
  if( pRhs ){
    pRhs->op = (u8)setOp;
    pRhs->pPrior = pLhs;
    if( ALWAYS(pLhs) ) pLhs->selFlags &= ~SF_MultiValue;
    pRhs->selFlags &= ~SF_MultiValue;
    if( setOp!=TK_ALL ) pParse->hasCompound = 1;
  }else{
    sqlite3SelectDelete(pParse->db, pLhs);
  }
  return pRhs;
}

/**
 * R joinOp S
 * select * from R joinOp S
 * 
 * */
Select* raCalculateJoinOp(Parse *pParse,Select *pLhs,Select *pRhs,int joinType){
    ExprList* pselcollist = sqlite3ExprListAppend(pParse, 0, sqlite3Expr(pParse->db, TK_ASTERISK, 0));
    SrcList* pSrc = twoSubqueryFromSrc(pParse,pLhs,pRhs);

    if( ALWAYS(pSrc && pSrc->nSrc>0) ) {
    pSrc->a[pSrc->nSrc-1].fg.jointype = (u8)joinType;
    }

    return sqlite3SelectNew(pParse,pselcollist,pSrc,0,0,0,0,SF_Distinct,0);
}

/**
 * R joinOp S
 * select * from R joinOp S
 * 
 * */
Select* raCalculateJoinOpWithCondition(Parse *pParse,Select *pLhs,Select *pRhs,int joinType,Expr* pwhereOpt){
    ExprList* pselcollist = sqlite3ExprListAppend(pParse, 0, sqlite3Expr(pParse->db, TK_ASTERISK, 0));
    SrcList* pSrc = twoSubqueryFromSrc(pParse,pLhs,pRhs);

    if( ALWAYS(pSrc && pSrc->nSrc>0) ) {
    pSrc->a[pSrc->nSrc-1].fg.jointype = (u8)joinType;
    }

    return sqlite3SelectNew(pParse,pselcollist,pSrc,pwhereOpt,0,0,0,SF_Distinct,0);
}


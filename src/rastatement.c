#include "sqliteInt.h"

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
 * pi L(A)
 * select L from R
 * */
Select* raCalculateProjectionOp(Parse *pParse,ExprList* pselcollist,Select* prelationshipR){
    Token nullAlias;
    SrcList* pSrc = sqlite3SrcListAppendFromTerm(
        pParse,
        0,
        0,
        0,
        &nullAlias,
        prelationshipR,
        0,
        0);
    return sqlite3SelectNew(pParse,pselcollist,pSrc,0,0,0,0,SF_Distinct,0);
}

/**
 * sigma c(A)
 * 
 * */
// Select* raCalculateSelectOp(Parse *pParse,){

// }

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



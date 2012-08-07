/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

/****************** I N C L U D E S **************************************/

// Programmuebergreifende Includes.
#include <rscdef.hxx>

/****************** C o d e **********************************************/
/****************** R s c I d ********************************************/
sal_Bool RscId::bNames = sal_True;

/*************************************************************************
|*
|*    static RscId::SetNames
|*    static RscId::SetNoNames
|*
*************************************************************************/
void RscId::SetNames( sal_Bool bSet )  { bNames = bSet;  }

/*************************************************************************
|*
|*    RscId::GetNumber
|*
*************************************************************************/
sal_Int32 RscId::GetNumber() const{
    sal_Int32 lVal;
    aExp.Evaluate( &lVal );
    return lVal;
}

/*************************************************************************
|*
|*    RscId::Create()
|*
*************************************************************************/
void RscId::Create( const RscExpType & rExpType )
{
    aExp = rExpType;
    if( aExp.IsDefinition() )
        aExp.aExp.pDef->IncRef();
    else if( aExp.IsExpression() ){
        sal_Int32 lValue;

        aExp.Evaluate( &lValue );
        aExp.SetLong( lValue );
    }
}

/*************************************************************************
|*
|*    RscId::Destroy()
|*
*************************************************************************/
void RscId::Destroy(){
    if( aExp.IsDefinition() )
        aExp.aExp.pDef->DecRef();
    aExp.cType = RSCEXP_NOTHING;
}

/*************************************************************************
|*
|*    RscId::RscId()
|*
*************************************************************************/
RscId::RscId( const RscId& rRscId ){
    aExp = rRscId.aExp;
    if( aExp.IsDefinition() )
        aExp.aExp.pDef->IncRef();
}

/*************************************************************************
|*
|*    RscId::RscId()
|*
*************************************************************************/
RscId::RscId( RscDefine * pDef ){
    RscExpType aExpType;

    aExpType.aExp.pDef = pDef;
    aExpType.cType = RSCEXP_DEF;
    Create( aExpType );
}

/*************************************************************************
|*
|*    RscId:: =
|*
*************************************************************************/
RscId& RscId::operator = ( const RscId& rRscId ){
    if( rRscId.aExp.IsDefinition() )
        rRscId.aExp.aExp.pDef->IncRef();
    Destroy();
    aExp = rRscId.aExp;
    return *this;
}

/*************************************************************************
|*
|*    RscId::operator ==
|*
*************************************************************************/
sal_Bool RscId::operator == ( const RscId& rRscId ) const
{
    return( GetNumber() == rRscId.GetNumber() );
}

/*************************************************************************
|*
|*    RscId::operator <
|*
*************************************************************************/
sal_Bool RscId::operator < ( const RscId& rRscId ) const
{
    return( GetNumber() < rRscId.GetNumber() );
}

/*************************************************************************
|*
|*    RscId::operator >
|*
*************************************************************************/
sal_Bool RscId::operator > ( const RscId& rRscId ) const
{
    return( GetNumber() > rRscId.GetNumber() );
}

/*************************************************************************
|*
|*    RscId::sal_Int32()
|*
*************************************************************************/
RscId::operator sal_Int32() const
{
    return( GetNumber() );
}

/*************************************************************************
|*
|*    RscId::GetNames()
|*
*************************************************************************/
rtl::OString RscId::GetName() const
{
    rtl::OStringBuffer aStr;

    if ( !aExp.IsNothing() )
    {
        if( bNames )
            aExp.AppendMacro(aStr);
        else
            aStr.append(GetNumber());
    }

    return aStr.makeStringAndClear();
}

/****************** R s c D e f i n e ************************************/
/*************************************************************************
|*
|*    RscDefine::RscDefine()
|*
*************************************************************************/
RscDefine::RscDefine( sal_uLong lKey, const rtl::OString& rDefName, sal_Int32 lDefId )
    : StringNode( rDefName )
{
    nRefCount = 0;
    lFileKey  = lKey;
    lId       = lDefId;
    pExp      = NULL;
}

RscDefine::RscDefine( sal_uLong lKey, const rtl::OString& rDefName,
                      RscExpression * pExpression  )
    : StringNode( rDefName )
{
    nRefCount = 0;
    lFileKey  = lKey;
    pExpression->Evaluate( &lId );
    pExp      = pExpression;
}

/*************************************************************************
|*
|*    RscDefine::~RscDefine()
|*
*************************************************************************/
RscDefine::~RscDefine(){
    if( pExp )
        delete pExp;
    if( nRefCount )
        RscExit( 14 );
}

/*************************************************************************
|*
|*    RscDefine::DecRef()
|*
*************************************************************************/
void RscDefine::DecRef(){
    nRefCount--;
    if( 0 == nRefCount ){
        delete this;
    }
}

/*************************************************************************
|*
|*    RscDefine::DefineToNumber()
|*
*************************************************************************/
void RscDefine::DefineToNumber()
{
    if( pExp )
        delete pExp;
    pExp = NULL;
    SetName(rtl::OString::valueOf(lId));
}

/*************************************************************************
|*
|*    RscDefine::Evaluate()
|*
*************************************************************************/
sal_Bool RscDefine::Evaluate(){
    sal_Bool    bRet = sal_True;

    if( pExp )
        bRet = !pExp->Evaluate( &lId );

    return bRet;
}

/*************************************************************************
|*
|*    RscDefine::Search()
|*
*************************************************************************/
RscDefine * RscDefine::Search( const char * pStr ){
    return (RscDefine *)StringNode::Search( pStr );
}

/*************************************************************************
|*
|*    RscDefine::GetMacro()
|*
*************************************************************************/
rtl::OString RscDefine::GetMacro()
{
    if( pExp )
        return pExp->GetMacro();
    return rtl::OString::valueOf(lId);
}

/****************** R s c D e f i n e L i s t ****************************/
/*************************************************************************
|*
|*    RscDefineList::New()
|*
*************************************************************************/
RscDefine * RscDefineList::New( sal_uLong lFileKey, const rtl::OString& rDefName,
                                sal_Int32 lDefId, size_t lPos )
{
    RscDefine * pDef;

    pDef = new RscDefine( lFileKey, rDefName, lDefId );
    pDef->IncRef();
    if ( lPos < maList.size() )
    {
        RscSubDefList::iterator it = maList.begin();
        ::std::advance( it, lPos );
        maList.insert( it, pDef );
    } else {
        maList.push_back( pDef );
    }
    return pDef;
}

RscDefine * RscDefineList::New( sal_uLong lFileKey, const rtl::OString& rDefName,
                                RscExpression * pExpression, size_t lPos )
{
    RscDefine * pDef;

    pDef = new RscDefine( lFileKey, rDefName, pExpression );
    pDef->IncRef();
    if ( lPos < maList.size() )
    {
        RscSubDefList::iterator it = maList.begin();
        ::std::advance( it, lPos );
        maList.insert( it, pDef );
    } else {
        maList.push_back( pDef );
    }
    return pDef;
}

sal_Bool RscDefineList::Remove() {
    if ( maList.empty() )
        return sal_False;

    maList[ 0 ]->DefineToNumber();
    maList[ 0 ]->DecRef();
    maList.erase( maList.begin() );
    return sal_True;
}

/*************************************************************************
|*
|*    RscDefineList::WriteAll()
|*
*************************************************************************/
void RscDefineList::WriteAll( FILE * fOutput )
{
    for ( size_t i = 0, n = maList.size(); i < n; ++i ) {
        RscDefine* pDefEle = maList[ i ];
        fprintf( fOutput, "#define %s %s\n",
                 pDefEle->GetName().getStr(),
                 pDefEle->GetMacro().getStr()
        );
    };
}

/****************** R s c E x p T y p e **********************************/
/*************************************************************************
|*
|*    RscExpType::Evaluate()
|*
*************************************************************************/
sal_Bool RscExpType::Evaluate( sal_Int32 * plValue ) const{
    if( IsDefinition() ){
        aExp.pDef->Evaluate();
        // Eventuellen Fehler ignorieren
        *plValue = aExp.pDef->GetNumber();
    }
    else if( IsExpression() )
        return( aExp.pExp->Evaluate( plValue ) );
    else if( IsNothing() )
        *plValue = 0;
    else
        *plValue = GetLong();

    return sal_True;
}

void RscExpType::AppendMacro(rtl::OStringBuffer& rStr) const
{
    if( IsDefinition() )
        rStr.append(aExp.pDef->GetName());
    else if( IsExpression() )
        rStr.append(aExp.pExp->GetMacro());
    else if( IsNumber() )
        rStr.append(GetLong());
}


/****************** R s c E x p r e s s i o n ****************************/
/*************************************************************************
|*
|*    RscExpression::RscExpression()
|*
*************************************************************************/
RscExpression::RscExpression( RscExpType aLE, char cOp, RscExpType aRE )
{
    aLeftExp   = aLE;
    cOperation = cOp;
    aRightExp  = aRE;
    if( aLeftExp.IsDefinition() )
        aLeftExp.aExp.pDef->IncRef();
    if( aRightExp.IsDefinition() )
        aRightExp.aExp.pDef->IncRef();
}

/*************************************************************************
|*
|*    RscExpression::~RscExpression()
|*
*************************************************************************/
RscExpression::~RscExpression(){
    if( aLeftExp.IsDefinition() )
        aLeftExp.aExp.pDef->DecRef();
    else if( aLeftExp.IsExpression() )
        delete aLeftExp.aExp.pExp;

    if( aRightExp.IsDefinition() )
        aRightExp.aExp.pDef->DecRef();
    else if( aRightExp.IsExpression() )
        delete aRightExp.aExp.pExp;
}

/*************************************************************************
|*
|*    RscExpression::Evaluate()
|*
*************************************************************************/
sal_Bool RscExpression::Evaluate( sal_Int32 * plValue ){
    sal_Int32 lLeft;
    sal_Int32 lRight;

    // linken und rechten Zweig auswerten
    if( aLeftExp.Evaluate( &lLeft ) && aRightExp.Evaluate( &lRight ) ){
        if( cOperation == '&' )
            *plValue = lLeft & lRight;
        else if( cOperation == '|' )
            *plValue = lLeft | lRight;
        else if( cOperation == '+' )
            *plValue = lLeft + lRight;
        else if( cOperation == '-' )
            *plValue = lLeft - lRight;
        else if( cOperation == '*' )
            *plValue = lLeft * lRight;
        else if( cOperation == 'r' )
            *plValue = lLeft >> lRight;
        else if( cOperation == 'l' )
            *plValue = lLeft << lRight;
        else{
            if( 0L == lRight )
                return sal_False;
            *plValue = lLeft / lRight;
        };
        return sal_True;
    }
    return sal_False;
}

/*************************************************************************
|*
|*    RscExpression::GetMacro()
|*
*************************************************************************/
rtl::OString RscExpression::GetMacro()
{
    rtl::OStringBuffer aLeft;

    // Ausgabeoptimierung
    if( aLeftExp.IsNothing() )
    {
        if ( '-' == cOperation )
        {
            aLeft.append('(');
            aLeft.append('-');
        }
        aRightExp.AppendMacro(aLeft);
        if( '-' == cOperation )
            aLeft.append(')');
    }
    else if( aRightExp.IsNothing() )
        aLeftExp.AppendMacro(aLeft);
    else{
        aLeft.append('(');
        // linken Zweig auswerten
        aLeftExp.AppendMacro(aLeft);

        aLeft.append(cOperation);

        aLeft.append('(');
        // rechten Zweig auswerten
        aRightExp.AppendMacro(aLeft);
        aLeft.append(')');

        aLeft.append(')');
    }

    return aLeft.makeStringAndClear();
}

/****************** R s c F i l e ****************************************/
/*************************************************************************
|*
|*    RscFile::RscFile()
|*
*************************************************************************/
RscFile :: RscFile(){
    bLoaded  = sal_False;
    bIncFile = sal_False;
    bDirty   = sal_False;
    bScanned = sal_False;
}

/*************************************************************************
|*
|*    RscFile::~RscFile()
|*
*************************************************************************/
RscFile :: ~RscFile() {
    for ( size_t i = 0, n = aDepLst.size(); i < n; ++i )
        delete aDepLst[ i ];
    aDepLst.clear();

    //von hinten nach vorne ist besser wegen der Abhaengigkeiten
    //Objekte zerstoeren sich, wenn Referenzzaehler NULL
    while( aDefLst.Remove() ) ;
}

/*************************************************************************
|*
|*    RscFile::Depend()
|*
|*    Beschreibung      Diese Methode gibt sal_True zurueck, wenn lDepend
|*                      existiert und hinter lFree steht, oder wenn
|*                      lDepend nicht existiert.
|*
*************************************************************************/
sal_Bool RscFile::Depend( sal_uLong lDepend, sal_uLong lFree ){
    RscDepend * pDep;

    for ( size_t i = aDepLst.size(); i > 0; )
    {
        pDep = aDepLst[ --i ];
        if( pDep->GetFileKey() == lDepend ) {
            for ( size_t j = i ? --i : 0; j > 0; )
            {
                pDep = aDepLst[ --j ];
                if( pDep->GetFileKey() == lFree )
                    return sal_True;
            }
            return sal_False;
        }
    }
    return sal_True;
}

/*************************************************************************
|*
|*    RscFile::InsertDependFile()
|*
*************************************************************************/
sal_Bool RscFile :: InsertDependFile( sal_uLong lIncFile, size_t lPos )
{
    for ( size_t i = 0, n = aDepLst.size(); i < n; ++i )
    {
        RscDepend* pDep = aDepLst[ i ];
        if( pDep->GetFileKey() == lIncFile )
            return sal_True;
    }

    // Current-Zeiger steht auf letztem Element
    if( lPos >= aDepLst.size() ) { //letztes Element muss immer letztes bleiben
        // Abhaengigkeit vor der letzten Position eintragen
        aDepLst.push_back( new RscDepend( lIncFile ) );
    }
    else {
        RscDependList::iterator it = aDepLst.begin();
        ::std::advance( it, lPos );
        aDepLst.insert( it, new RscDepend( lIncFile ) );
    }
    return sal_True;
}

/****************** R s c D e f T r e e **********************************/
/*************************************************************************
|*
|*    RscDefTree::~RscDefTree()
|*
*************************************************************************/
RscDefTree::~RscDefTree(){
    Remove();
}

/*************************************************************************
|*
|*    RscDefTree::Remove()
|*
*************************************************************************/
void RscDefTree::Remove(){
    RscDefine * pDef;
    while( pDefRoot ){
        pDef = pDefRoot;
        pDefRoot = (RscDefine *)pDefRoot->Remove( pDefRoot );
        pDef->DecRef();
    }
}

/*************************************************************************
|*
|*    RscDefTree::~Search()
|*
*************************************************************************/
RscDefine * RscDefTree::Search( const char * pName ){
    if( pDefRoot )
        return pDefRoot->Search( pName );
    return NULL;
}

/*************************************************************************
|*
|*    RscDefTree::Insert()
|*
*************************************************************************/
void RscDefTree::Insert( RscDefine * pDef ){
    if( pDefRoot )
        pDefRoot->Insert( pDef );
    else
        pDefRoot = pDef;
    pDef->IncRef();
}

/*************************************************************************
|*
|*    RscDefTree::Remove()
|*
*************************************************************************/
void RscDefTree::Remove( RscDefine * pDef ){
    if( pDefRoot ){
        //falls pDef == pDefRoot
        pDefRoot = (RscDefine *)pDefRoot->Remove( pDef );
    }
    pDef->DecRef();
}

/*************************************************************************
|*
|*    RscDefTree::Evaluate()
|*
*************************************************************************/
sal_Bool RscDefTree::Evaluate( RscDefine * pDef ){
    if( pDef ){
        if( !Evaluate( (RscDefine *)pDef->Left() ) )
            return sal_False;
        if( !Evaluate( (RscDefine *)pDef->Right() ) )
            return sal_False;
    };
    return sal_True;
}

/****************** R s c F i l e T a b **********************************/
/*************************************************************************
|*
|*    RscFileTab::RscFileTab()
|*
*************************************************************************/
RscFileTab::RscFileTab(){
}

/*************************************************************************
|*
|*    RscFileTab::~RscFileTab()
|*
*************************************************************************/
RscFileTab :: ~RscFileTab(){

    aDefTree.Remove();

    sal_uIntPtr aIndex = LastIndex();
    while( aIndex != UNIQUEINDEX_ENTRY_NOTFOUND ) {
        delete Remove( aIndex );
        aIndex = LastIndex();
    };
}

/*************************************************************************
|*
|*    RscFileTab::Find()
|*
*************************************************************************/
sal_uLong  RscFileTab :: Find( const rtl::OString& rName )
{
    sal_uIntPtr aIndex = FirstIndex();
    while( aIndex != UNIQUEINDEX_ENTRY_NOTFOUND && (Get(aIndex)->aFileName != rName) )
        aIndex = NextIndex(aIndex);

    if( aIndex != UNIQUEINDEX_ENTRY_NOTFOUND )
        return aIndex;
    else
        return NOFILE_INDEX;
}

/*************************************************************************
|*
|*    RscFileTab::FindDef()
|*
*************************************************************************/
RscDefine * RscFileTab::FindDef( const char * pName ){
    return aDefTree.Search( pName );
}

/*************************************************************************
|*
|*    RscFileTab::Depend()
|*
*************************************************************************/
sal_Bool RscFileTab::Depend( sal_uLong lDepend, sal_uLong lFree ){
    if( lDepend == lFree )
        return sal_True;

    sal_uIntPtr aIndex = FirstIndex();
    while( aIndex != UNIQUEINDEX_ENTRY_NOTFOUND ){
        RscFile * pFile = Get(aIndex);
        if( !pFile->IsIncFile() ){
            if( !pFile->Depend( lDepend, lFree ) )
                return sal_False;
        };
        aIndex = NextIndex(aIndex);
    };

    return sal_True;
}

/*************************************************************************
|*
|*    RscFileTab::TestDef()
|*
*************************************************************************/
sal_Bool RscFileTab::TestDef( sal_uLong lFileKey, size_t lPos,
                          const RscDefine * pDefDec )
{
    if( lFileKey == pDefDec->GetFileKey() ) {
        RscFile * pFile = GetFile( pDefDec->GetFileKey() );
        if( pFile && (lPos <= pFile->aDefLst.GetPos( (RscDefine *)pDefDec ))
          && (lPos != ULONG_MAX ) )
            return sal_False;
    }
    else if( !Depend( lFileKey, pDefDec->GetFileKey() ) )
        return sal_False;

    return TestDef( lFileKey, lPos, pDefDec->pExp );
}

/*************************************************************************
|*
|*    RscFileTab::TestDef()
|*
*************************************************************************/
sal_Bool RscFileTab::TestDef( sal_uLong lFileKey, size_t lPos,
                          const RscExpression * pExpDec )
{
    if( !pExpDec )
        return sal_True;

    if( pExpDec->aLeftExp.IsExpression() )
        if( !TestDef( lFileKey, lPos, pExpDec->aLeftExp.aExp.pExp ) )
            return sal_False;

    if( pExpDec->aLeftExp.IsDefinition() )
        if( !TestDef( lFileKey, lPos, pExpDec->aLeftExp.aExp.pDef ) )
            return sal_False;

    if( pExpDec->aRightExp.IsExpression() )
        if( !TestDef( lFileKey, lPos, pExpDec->aRightExp.aExp.pExp ) )
            return sal_False;

    if( pExpDec->aRightExp.IsDefinition() )
        if( !TestDef( lFileKey, lPos, pExpDec->aRightExp.aExp.pDef ) )
            return sal_False;

    return sal_True;
}

/*************************************************************************
|*
|*    RscFileTab::NewDef()
|*
*************************************************************************/
RscDefine * RscFileTab::NewDef( sal_uLong lFileKey, const rtl::OString& rDefName,
                                sal_Int32 lId, sal_uLong lPos )
{
    RscDefine * pDef = FindDef( rDefName );

    if( !pDef ){
        RscFile * pFile = GetFile( lFileKey );

        if( pFile ){
            pDef = pFile->aDefLst.New( lFileKey, rDefName, lId, lPos );
            aDefTree.Insert( pDef );
        }
    }
    else
        pDef = NULL;

    return( pDef );
}

/*************************************************************************
|*
|*    RscFileTab::NewDef()
|*
*************************************************************************/
RscDefine * RscFileTab::NewDef( sal_uLong lFileKey, const rtl::OString& rDefName,
                                RscExpression * pExp, sal_uLong lPos )
{
    RscDefine * pDef = FindDef( rDefName );

    if( !pDef ){
        //Macros in den Expressions sind definiert ?
        if( TestDef( lFileKey, lPos, pExp ) ){
            RscFile * pFile = GetFile( lFileKey );

            if( pFile ){
                pDef = pFile->aDefLst.New( lFileKey, rDefName, pExp, lPos );
                aDefTree.Insert( pDef );
            }
        }
    }
    else
        pDef = NULL;

    if( !pDef ){
        // pExp wird immer Eigentum und muss, wenn es nicht benoetigt wird
        // geloescht werden
        delete pExp;
    }
    return( pDef );
}

/*************************************************************************
|*
|*    RscFileTab::DeleteFileContext()
|*
*************************************************************************/
void RscFileTab :: DeleteFileContext( sal_uLong lFileKey ){
    RscFile     * pFName;

    pFName = GetFile( lFileKey );
    if( pFName ){
        RscDefine * pDef;

        for ( size_t i = 0, n = pFName->aDefLst.maList.size(); i < n; ++i ) {
            pDef = pFName->aDefLst.maList[ i ];
            aDefTree.Remove( pDef );
        };
        while( pFName->aDefLst.Remove() ) ;
    }
}

/*************************************************************************
|*
|*    RscFileTab::NewCodeFile()
|*
*************************************************************************/
sal_uLong  RscFileTab :: NewCodeFile( const rtl::OString& rName )
{
    sal_uLong       lKey;
    RscFile *   pFName;

    lKey = Find( rName );
    if( UNIQUEINDEX_ENTRY_NOTFOUND == lKey )
    {
        pFName = new RscFile();
        pFName->aFileName = rName;
        pFName->aPathName = rName;
        lKey = Insert( pFName );
        pFName->InsertDependFile( lKey, ULONG_MAX );
    }
    return lKey;
}

/*************************************************************************
|*
|*    RscFileTab::NewIncFile()
|*
*************************************************************************/
sal_uLong  RscFileTab :: NewIncFile(const rtl::OString& rName,
    const rtl::OString& rPath)
{
    sal_uLong         lKey;
    RscFile * pFName;

    lKey = Find( rName );
    if( UNIQUEINDEX_ENTRY_NOTFOUND == lKey )
    {
        pFName = new RscFile();
        pFName->aFileName = rName;
        pFName->aPathName = rPath;
        pFName->SetIncFlag();
        lKey = Insert( pFName );
        pFName->InsertDependFile( lKey, ULONG_MAX );
    }
    return lKey;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

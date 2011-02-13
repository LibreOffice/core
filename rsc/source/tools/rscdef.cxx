/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_rsc.hxx"
/****************** I N C L U D E S **************************************/

// Programmuebergreifende Includes.
#include <rscdef.hxx>

/****************** C o d e **********************************************/
/****************** R s c I d ********************************************/
BOOL RscId::bNames = TRUE;

/*************************************************************************
|*
|*    static RscId::SetNames
|*    static RscId::SetNoNames
|*
*************************************************************************/
void RscId::SetNames( BOOL bSet )  { bNames = bSet;  }
BOOL RscId::IsSetNames()           { return bNames;  }

/*************************************************************************
|*
|*    RscId::GetNumber
|*
*************************************************************************/
INT32 RscId::GetNumber() const{
    INT32 lVal;
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
        INT32 lValue;

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
BOOL RscId::operator == ( const RscId& rRscId ) const
{
    return( GetNumber() == rRscId.GetNumber() );
}

/*************************************************************************
|*
|*    RscId::operator <
|*
*************************************************************************/
BOOL RscId::operator < ( const RscId& rRscId ) const
{
    return( GetNumber() < rRscId.GetNumber() );
}

/*************************************************************************
|*
|*    RscId::operator >
|*
*************************************************************************/
BOOL RscId::operator > ( const RscId& rRscId ) const
{
    return( GetNumber() > rRscId.GetNumber() );
}

/*************************************************************************
|*
|*    RscId::INT32()
|*
*************************************************************************/
RscId::operator INT32() const
{
    return( GetNumber() );
}

/*************************************************************************
|*
|*    RscId::GetNames()
|*
*************************************************************************/
ByteString RscId::GetName() const
{
    ByteString aStr;

    if ( !aExp.IsNothing() )
    {
        if( bNames )
            aExp.GetMacro( aStr );
        else
            aStr = ByteString::CreateFromInt32( GetNumber() );
    }

    return aStr;
}

/*************************************************************************
|*
|*    RscId::GetMacro()
|*
*************************************************************************/
ByteString RscId::GetMacro() const
{
    ByteString aStr;

    if ( aExp.IsDefinition() )
        aStr = aExp.aExp.pDef->GetMacro();
    else
        aExp.GetMacro( aStr );

    return aStr;
}

/****************** R s c D e f i n e ************************************/
/*************************************************************************
|*
|*    RscDefine::RscDefine()
|*
*************************************************************************/
RscDefine::RscDefine( ULONG lKey, const ByteString & rDefName, INT32 lDefId )
    : StringNode( rDefName )
{
    nRefCount = 0;
    lFileKey  = lKey;
    lId       = lDefId;
    pExp      = NULL;
}

RscDefine::RscDefine( ULONG lKey, const ByteString & rDefName,
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
    SetName( ByteString::CreateFromInt32( lId ) );
}

/*************************************************************************
|*
|*    RscDefine::ChangeMacro()
|*
*************************************************************************/
void RscDefine::ChangeMacro( RscExpression * pExpression ){
    if( pExp )
        delete pExp;
    pExp = pExpression;
    pExp->Evaluate( &lId );
}

void RscDefine::ChangeMacro( INT32 lIdentifier ){
    if( pExp ){
        delete pExp;
        pExp = NULL;
    }
    lId = lIdentifier;
}

/*************************************************************************
|*
|*    RscDefine::Evaluate()
|*
*************************************************************************/
BOOL RscDefine::Evaluate(){
    BOOL    bRet = TRUE;

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
ByteString RscDefine::GetMacro()
{
    if( pExp )
        return pExp->GetMacro();
    return ByteString::CreateFromInt32( lId );
}

/****************** R s c D e f i n e L i s t ****************************/
/*************************************************************************
|*
|*    RscDefineList::New()
|*
*************************************************************************/
RscDefine * RscDefineList::New( ULONG lFileKey, const ByteString & rDefName,
                                INT32 lDefId, size_t lPos )
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

RscDefine * RscDefineList::New( ULONG lFileKey, const ByteString & rDefName,
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

/*************************************************************************
|*
|*    RscDefineList::Remove()
|*
*************************************************************************/
BOOL RscDefineList::Remove( RscDefine * pDef ) {
    for ( RscSubDefList::iterator it = maList.begin(); it < maList.end(); ++it ) {
        if ( *it == pDef ) {
            (*it)->DefineToNumber();
            (*it)->DecRef();
            maList.erase( it );
            return TRUE;
        }
    }
    return FALSE;
}

BOOL RscDefineList::Remove( size_t lIndex ) {
    if ( lIndex < maList.size() ) {
        RscSubDefList::iterator it = maList.begin();
        ::std::advance( it, lIndex );
        (*it)->DefineToNumber();
        (*it)->DecRef();
        maList.erase( it );
        return TRUE;
    }
    return FALSE;
}

BOOL RscDefineList::Remove() {
    if ( maList.empty() ) return FALSE;

    maList[ 0 ]->DefineToNumber();
    maList[ 0 ]->DecRef();
    maList.erase( maList.begin() );
    return TRUE;
}

/*************************************************************************
|*
|*    RscDefineList::Befor()
|*
*************************************************************************/
BOOL RscDefineList::Befor( const RscDefine * pFree,
                           const RscDefine * pDepend )
{
    size_t i = 0;
    size_t n = maList.size();
    while ( i < n ) {
        if ( maList[ i ] == pFree ) {
            for ( ++i ; i < n ; ++i ) {
                if ( maList[ i ] == pDepend ) {
                    return TRUE;
                }
            }
        }
        ++i;
    }
    return FALSE;
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
                 pDefEle->GetName().GetBuffer(),
                 pDefEle->GetMacro().GetBuffer()
        );
    };
}

/****************** R s c E x p T y p e **********************************/
/*************************************************************************
|*
|*    RscExpType::Evaluate()
|*
*************************************************************************/
BOOL RscExpType::Evaluate( INT32 * plValue ) const{
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

    return TRUE;
}

/*************************************************************************
|*
|*    RscExpType::GetMacro()
|*
*************************************************************************/
void RscExpType::GetMacro( ByteString & rStr ) const
{
    ByteString aStr;

    if( IsDefinition() )
    {
        rStr += aExp.pDef->GetName();
    }
    else if( IsExpression() )
        rStr += aExp.pExp->GetMacro();
    else if( IsNumber() )
        rStr += ByteString::CreateFromInt32( GetLong() );
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
BOOL RscExpression::Evaluate( INT32 * plValue ){
    INT32 lLeft;
    INT32 lRight;

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
                return FALSE;
            *plValue = lLeft / lRight;
        };
        return TRUE;
    }
    return FALSE;
}

/*************************************************************************
|*
|*    RscExpression::GetMacro()
|*
*************************************************************************/
ByteString RscExpression::GetMacro()
{
    ByteString aLeft;

    // Ausgabeoptimierung
    if( aLeftExp.IsNothing() )
    {
        if ( '-' == cOperation )
        {
            aLeft += '(';
            aLeft += '-';
        }
        aRightExp.GetMacro( aLeft );
        if( '-' == cOperation )
            aLeft += ')';
    }
    else if( aRightExp.IsNothing() )
        aLeftExp.GetMacro( aLeft );
    else{
        aLeft += '(';
        // linken Zweig auswerten
        aLeftExp.GetMacro( aLeft );

        aLeft += cOperation;

        aLeft += '(';
        // rechten Zweig auswerten
        aRightExp.GetMacro( aLeft );
        aLeft += ')';

        aLeft += ')';
    }

    return aLeft;
}

/****************** R s c F i l e ****************************************/
/*************************************************************************
|*
|*    RscFile::RscFile()
|*
*************************************************************************/
RscFile :: RscFile(){
    bLoaded  = FALSE;
    bIncFile = FALSE;
    bDirty   = FALSE;
    bScanned = FALSE;
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
|*    Beschreibung      Diese Methode gibt TRUE zurueck, wenn lDepend
|*                      existiert und hinter lFree steht, oder wenn
|*                      lDepend nicht existiert.
|*
*************************************************************************/
BOOL RscFile::Depend( ULONG lDepend, ULONG lFree ){
    RscDepend * pDep;

    for ( size_t i = aDepLst.size(); i > 0; )
    {
        pDep = aDepLst[ --i ];
        if( pDep->GetFileKey() == lDepend ) {
            for ( size_t j = i ? --i : 0; j > 0; )
            {
                pDep = aDepLst[ --j ];
                if( pDep->GetFileKey() == lFree )
                    return TRUE;
            }
            return FALSE;
        }
    }
    return TRUE;
}

/*************************************************************************
|*
|*    RscFile::InsertDependFile()
|*
*************************************************************************/
BOOL RscFile :: InsertDependFile( ULONG lIncFile, size_t lPos )
{
    for ( size_t i = 0, n = aDepLst.size(); i < n; ++i )
    {
        RscDepend* pDep = aDepLst[ i ];
        if( pDep->GetFileKey() == lIncFile )
            return TRUE;
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
    return TRUE;
}

/*************************************************************************
|*
|*    RscFile::RemoveDependFile()
|*
*************************************************************************/
void RscFile :: RemoveDependFile( ULONG lDepFile )
{
    for ( size_t i = aDepLst.size(); i > 0; )
    {
        RscDepend* pDep = aDepLst[ --i ];
        if( pDep->GetFileKey() == lDepFile ) {
            RscDependList::iterator it = aDepLst.begin();
            ::std::advance( it, i );
            delete *it;
            aDepLst.erase( it );
        }
    }
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
BOOL RscDefTree::Evaluate( RscDefine * pDef ){
    if( pDef ){
        if( !Evaluate( (RscDefine *)pDef->Left() ) )
            return FALSE;
        if( !Evaluate( (RscDefine *)pDef->Right() ) )
            return FALSE;
    };
    return TRUE;
}

BOOL RscDefTree::Evaluate(){
    return Evaluate( pDefRoot );
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
    RscFile * pFile;

    aDefTree.Remove();

    pFile = Last();
    while( pFile ){
        Remove( GetIndex( pFile ) );
        delete pFile;
        pFile = Prev();
    };
}

/*************************************************************************
|*
|*    RscFileTab::Find()
|*
*************************************************************************/
ULONG  RscFileTab :: Find( const ByteString & rName )
{
    RscFile * pFName;

    pFName = First();
    while( pFName && (pFName->aFileName != rName) )
        pFName = Next();

    if( pFName )
        return( GetIndex( pFName ) );
    else
        return( NOFILE_INDEX );
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
|*    RscFileTab::FindDef()
|*
*************************************************************************/
RscDefine * RscFileTab::FindDef( ULONG lFileKey, const ByteString & rName )
{
    RscDefine   * pDef = FindDef( rName );

    if( pDef )
        //befindet sich das DEFINE in einer Include-Datei in der
        //Datei lFileKey
        if( Depend( lFileKey, pDef->GetFileKey() ) )
            return pDef;
    return NULL;
}

/*************************************************************************
|*
|*    RscFileTab::Depend()
|*
*************************************************************************/
BOOL RscFileTab::Depend( ULONG lDepend, ULONG lFree ){
    if( lDepend == lFree )
        return TRUE;

    RscFile * pFile = First();
    while( pFile ){
        if( !pFile->IsIncFile() ){
            if( !pFile->Depend( lDepend, lFree ) )
                return FALSE;
        };
        pFile = Next();
    };

    return TRUE;
}

/*************************************************************************
|*
|*    RscFileTab::TestDef()
|*
*************************************************************************/
BOOL RscFileTab::TestDef( ULONG lFileKey, size_t lPos,
                          const RscDefine * pDefDec )
{
    if( lFileKey == pDefDec->GetFileKey() ) {
        RscFile * pFile = GetFile( pDefDec->GetFileKey() );
        if( pFile && (lPos <= pFile->aDefLst.GetPos( (RscDefine *)pDefDec ))
          && (lPos != LIST_APPEND) )
            return FALSE;
    }
    else if( !Depend( lFileKey, pDefDec->GetFileKey() ) )
        return FALSE;

    return TestDef( lFileKey, lPos, pDefDec->pExp );
}

/*************************************************************************
|*
|*    RscFileTab::TestDef()
|*
*************************************************************************/
BOOL RscFileTab::TestDef( ULONG lFileKey, size_t lPos,
                          const RscExpression * pExpDec )
{
    if( !pExpDec )
        return TRUE;

    if( pExpDec->aLeftExp.IsExpression() )
        if( !TestDef( lFileKey, lPos, pExpDec->aLeftExp.aExp.pExp ) )
            return FALSE;

    if( pExpDec->aLeftExp.IsDefinition() )
        if( !TestDef( lFileKey, lPos, pExpDec->aLeftExp.aExp.pDef ) )
            return FALSE;

    if( pExpDec->aRightExp.IsExpression() )
        if( !TestDef( lFileKey, lPos, pExpDec->aRightExp.aExp.pExp ) )
            return FALSE;

    if( pExpDec->aRightExp.IsDefinition() )
        if( !TestDef( lFileKey, lPos, pExpDec->aRightExp.aExp.pDef ) )
            return FALSE;

    return TRUE;
}

/*************************************************************************
|*
|*    RscFileTab::NewDef()
|*
*************************************************************************/
RscDefine * RscFileTab::NewDef( ULONG lFileKey, const ByteString & rDefName,
                                INT32 lId, ULONG lPos )
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
RscDefine * RscFileTab::NewDef( ULONG lFileKey, const ByteString & rDefName,
                                RscExpression * pExp, ULONG lPos )
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
|*    RscFileTab::IsDefUsed()
|*
*************************************************************************/
BOOL RscFileTab::IsDefUsed( const ByteString & rDefName )
{
    RscDefine * pDef = FindDef( rDefName );

    if( pDef )
        return( pDef->GetRefCount() != 2 );

    return FALSE;
}

/*************************************************************************
|*
|*    RscFileTab::DeleteDef()
|*
*************************************************************************/
void RscFileTab::DeleteDef( const ByteString & rDefName )
{
    RscDefine * pDef = FindDef( rDefName );
    RscFile   * pFile;

    if( pDef ){
        pFile = GetFile( pDef->GetFileKey() );
        if( pFile ){
            aDefTree.Remove( pDef );
            pFile->aDefLst.Remove( pDef );
        }
    };
}

/*************************************************************************
|*
|*    RscFileTab::ChangeDef()
|*
*************************************************************************/
BOOL RscFileTab::ChangeDef( const ByteString & rDefName, INT32 lId )
{
    RscDefine * pDef = FindDef( rDefName );

    if( pDef ){
        pDef->ChangeMacro( lId );
        //alle Macros neu bewerten
        return aDefTree.Evaluate();
    };
    return( FALSE );
}

/*************************************************************************
|*
|*    RscFileTab::ChangeDef()
|*
*************************************************************************/
BOOL RscFileTab::ChangeDef( const ByteString & rDefName,
                            RscExpression * pExp )
{
    RscDefine * pDef = FindDef( rDefName );
    RscFile   * pFile;

    if( pDef )
    {
        pFile = GetFile( pDef->GetFileKey() );
        ULONG lPos = 0;
        if( pFile )
            lPos = pFile->aDefLst.GetPos( pDef );
        //Macros in den Expressions sind definiert ?
        if( TestDef( pDef->GetFileKey(), lPos, pExp ) ){
            pDef->ChangeMacro( pExp );
            //alle Macros neu bewerten
            return aDefTree.Evaluate();
        }
    };

    // pExp wird immer Eigentum und muss, wenn es nicht benoetigt wird
    // geloescht werden
    delete pExp;

    return( FALSE );
}

/*************************************************************************
|*
|*    RscFileTab::ChangeDefName()
|*
*************************************************************************/
BOOL RscFileTab::ChangeDefName( const ByteString & rDefName,
                                const ByteString & rNewName )
{
    RscDefine * pDef = FindDef( rDefName );

    //Name gefunden ?
    if( pDef ){
        // und neuer Name noch nicht bekannt ?
        if( !FindDef( pDef->GetFileKey(), rNewName ) ){
            aDefTree.Remove( pDef );
            pDef->SetName( rNewName );
            aDefTree.Insert( pDef );
            return( TRUE );
        }
    };

    return( FALSE );
}

/*************************************************************************
|*
|*    RscFileTab::DeleteFileContext()
|*
*************************************************************************/
void RscFileTab :: DeleteFileContext( ULONG lFileKey ){
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
|*    RscFileTab::DeleteFile()
|*
*************************************************************************/
void RscFileTab :: DeleteFile( ULONG lFileKey ){
    RscFile     * pFName;

    //Defines freigeben
    DeleteFileContext( lFileKey );

    //Schleife ueber alle Abhaengigkeiten
    pFName = First();
    while( pFName ){
        pFName->RemoveDependFile( lFileKey );
        pFName = Next();
    };

    pFName = Remove( lFileKey );
    if( pFName )
        delete pFName;
}

/*************************************************************************
|*
|*    RscFileTab::NewCodeFile()
|*
*************************************************************************/
ULONG  RscFileTab :: NewCodeFile( const ByteString & rName )
{
    ULONG       lKey;
    RscFile *   pFName;

    lKey = Find( rName );
    if( UNIQUEINDEX_ENTRY_NOTFOUND == lKey )
    {
        pFName = new RscFile();
        pFName->aFileName = rName;
        pFName->aPathName = rName;
        lKey = Insert( pFName );
        pFName->InsertDependFile( lKey, LIST_APPEND );
    }
    return lKey;
}

/*************************************************************************
|*
|*    RscFileTab::NewIncFile()
|*
*************************************************************************/
ULONG  RscFileTab :: NewIncFile( const ByteString & rName,
                                 const ByteString & rPath )
{
    ULONG         lKey;
    RscFile * pFName;

    lKey = Find( rName );
    if( UNIQUEINDEX_ENTRY_NOTFOUND == lKey )
    {
        pFName = new RscFile();
        pFName->aFileName = rName;
        pFName->aPathName = rPath;
        pFName->SetIncFlag();
        lKey = Insert( pFName );
        pFName->InsertDependFile( lKey, LIST_APPEND );
    }
    return lKey;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

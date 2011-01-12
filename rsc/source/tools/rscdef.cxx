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
sal_Bool RscId::bNames = sal_True;

/*************************************************************************
|*
|*    static RscId::SetNames
|*    static RscId::SetNoNames
|*
|*    Beschreibung
|*    Ersterstellung    MM 26.06.91
|*    Letzte Aenderung  MM 26.06.91
|*
*************************************************************************/
void RscId::SetNames( sal_Bool bSet )  { bNames = bSet;  }
sal_Bool RscId::IsSetNames()           { return bNames;  }

/*************************************************************************
|*
|*    RscId::GetNumber
|*
|*    Beschreibung
|*    Ersterstellung    MM 17.05.91
|*    Letzte Aenderung  MM 17.05.91
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
|*    Beschreibung
|*    Ersterstellung    MM 01.11.91
|*    Letzte Aenderung  MM 01.11.91
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
|*    Beschreibung
|*    Ersterstellung    MM 01.11.91
|*    Letzte Aenderung  MM 01.11.91
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
|*    Beschreibung
|*    Ersterstellung    MM 01.11.91
|*    Letzte Aenderung  MM 01.11.91
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
|*    Beschreibung
|*    Ersterstellung    MM 01.11.91
|*    Letzte Aenderung  MM 25.11.91
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
|*    Beschreibung
|*    Ersterstellung    MM 01.11.91
|*    Letzte Aenderung  MM 01.11.91
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
|*    Beschreibung
|*    Ersterstellung    MM 16.05.91
|*    Letzte Aenderung  MM 16.05.91
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
|*    Beschreibung
|*    Ersterstellung    MM 16.05.91
|*    Letzte Aenderung  MM 16.05.91
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
|*    Beschreibung
|*    Ersterstellung    MM 16.05.91
|*    Letzte Aenderung  MM 16.05.91
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
|*    Beschreibung
|*    Ersterstellung    MM 16.05.91
|*    Letzte Aenderung  MM 16.05.91
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
|*    Beschreibung
|*    Ersterstellung    MM 16.05.91
|*    Letzte Aenderung  MM 25.11.91
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
|*    Beschreibung
|*    Ersterstellung    MM 01.11.91
|*    Letzte Aenderung  MM 25.11.91
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
|*    Beschreibung
|*    Ersterstellung    MM 01.11.91
|*    Letzte Aenderung  MM 01.11.91
|*
*************************************************************************/
RscDefine::RscDefine( sal_uLong lKey, const ByteString & rDefName, sal_Int32 lDefId )
    : StringNode( rDefName )
{
    nRefCount = 0;
    lFileKey  = lKey;
    lId       = lDefId;
    pExp      = NULL;
}

RscDefine::RscDefine( sal_uLong lKey, const ByteString & rDefName,
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
|*    Beschreibung
|*    Ersterstellung    MM 01.11.91
|*    Letzte Aenderung  MM 01.11.91
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
|*    Beschreibung
|*    Ersterstellung    MM 01.11.91
|*    Letzte Aenderung  MM 01.11.91
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
|*    Beschreibung
|*    Ersterstellung    MM 07.11.91
|*    Letzte Aenderung  MM 07.11.91
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
|*    Beschreibung
|*    Ersterstellung    MM 04.11.91
|*    Letzte Aenderung  MM 04.11.91
|*
*************************************************************************/
void RscDefine::ChangeMacro( RscExpression * pExpression ){
    if( pExp )
        delete pExp;
    pExp = pExpression;
    pExp->Evaluate( &lId );
}

void RscDefine::ChangeMacro( sal_Int32 lIdentifier ){
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
|*    Beschreibung
|*    Ersterstellung    MM 01.11.91
|*    Letzte Aenderung  MM 01.11.91
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
|*    Beschreibung
|*    Ersterstellung    MM 11.11.91
|*    Letzte Aenderung  MM 11.11.91
|*
*************************************************************************/
RscDefine * RscDefine::Search( const char * pStr ){
    return (RscDefine *)StringNode::Search( pStr );
}

/*************************************************************************
|*
|*    RscDefine::GetMacro()
|*
|*    Beschreibung
|*    Ersterstellung    MM 01.11.91
|*    Letzte Aenderung  MM 01.11.91
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
|*    Beschreibung
|*    Ersterstellung    MM 04.11.91
|*    Letzte Aenderung  MM 04.11.91
|*
*************************************************************************/
RscDefine * RscDefineList::New( sal_uLong lFileKey, const ByteString & rDefName,
                                sal_Int32 lDefId, sal_uLong lPos )
{
    RscDefine * pDef;

    pDef = new RscDefine( lFileKey, rDefName, lDefId );
    pDef->IncRef();
    Insert( pDef, lPos );
    return pDef;
}

RscDefine * RscDefineList::New( sal_uLong lFileKey, const ByteString & rDefName,
                                RscExpression * pExpression, sal_uLong lPos )
{
    RscDefine * pDef;

    pDef = new RscDefine( lFileKey, rDefName, pExpression );
    pDef->IncRef();
    Insert( pDef, lPos );

    return pDef;
}

/*************************************************************************
|*
|*    RscDefineList::Remove()
|*
|*    Beschreibung
|*    Ersterstellung    MM 04.11.91
|*    Letzte Aenderung  MM 04.11.91
|*
*************************************************************************/
sal_Bool RscDefineList::Remove( RscDefine * pDef ){
    pDef = RscSubDefList::Remove( pDef );
    if( pDef ){
        pDef->DefineToNumber();
        pDef->DecRef();
    }

    return( NULL != pDef );
}

sal_Bool RscDefineList::Remove( sal_uLong lIndex ){
    RscDefine * pDef = RscSubDefList::Remove( lIndex );
    if( pDef ){
        pDef->DefineToNumber();
        pDef->DecRef();
    }

    return( NULL != pDef );
}

sal_Bool RscDefineList::Remove(){
    RscDefine * pDef;

    pDef = RscSubDefList::Remove( (sal_uLong)0 );

    if( pDef ){
        pDef->DefineToNumber();
        pDef->DecRef();
    }
    return( NULL != pDef );
}

/*************************************************************************
|*
|*    RscDefineList::Befor()
|*
|*    Beschreibung
|*    Ersterstellung    MM 12.11.91
|*    Letzte Aenderung  MM 12.11.91
|*
*************************************************************************/
sal_Bool RscDefineList::Befor( const RscDefine * pFree,
                           const RscDefine * pDepend )
{
    RscDefine * pDef;

    pDef = First();
    while( pDef ){
        if( pDef == pFree ){
            pDef = Next();
            while( pDef ){
                if( pDef == pDepend )
                    return sal_True;
                pDef = Next();
            }
        }
        pDef = Next();
    };
    return sal_False;
}

/*************************************************************************
|*
|*    RscDefineList::WriteAll()
|*
|*    Beschreibung
|*    Ersterstellung    MM 28.10.91
|*    Letzte Aenderung  MM 28.10.91
|*
*************************************************************************/
void RscDefineList::WriteAll( FILE * fOutput )
{
    RscDefine * pDefEle = First();

    while( pDefEle )
    {
        fprintf( fOutput, "#define %s %s\n",
                 pDefEle->GetName().GetBuffer(),
                 pDefEle->GetMacro().GetBuffer() );
        pDefEle = Next();
    };
}

/****************** R s c E x p T y p e **********************************/
/*************************************************************************
|*
|*    RscExpType::Evaluate()
|*
|*    Beschreibung
|*    Ersterstellung    MM 01.11.91
|*    Letzte Aenderung  MM 01.11.91
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

/*************************************************************************
|*
|*    RscExpType::GetMacro()
|*
|*    Beschreibung
|*    Ersterstellung    MM 01.11.91
|*    Letzte Aenderung  MM 01.11.91
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
|*    Beschreibung
|*    Ersterstellung    MM 01.11.91
|*    Letzte Aenderung  MM 01.11.91
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
|*    Beschreibung
|*    Ersterstellung    MM 01.11.91
|*    Letzte Aenderung  MM 01.11.91
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
|*    Beschreibung
|*    Ersterstellung    MM 01.11.91
|*    Letzte Aenderung  MM 01.11.91
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
|*    Beschreibung
|*    Ersterstellung    MM 01.11.91
|*    Letzte Aenderung  MM 01.11.91
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
|*    Beschreibung
|*    Ersterstellung    MM 04.11.91
|*    Letzte Aenderung  MM 04.11.91
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
|*    Beschreibung
|*    Ersterstellung    MM 04.11.91
|*    Letzte Aenderung  MM 04.11.91
|*
*************************************************************************/
RscFile :: ~RscFile(){
    RscDepend * pDep = Remove( (sal_uLong)0 );

    while( pDep ){
        delete pDep;
        pDep = Remove( (sal_uLong)0 );
    }

    //von hinten nach vorne ist besser wegen der Abhaengigkeiten
    //Objekte zerstoeren sich, wenn Referenzzaehler NULL
    aDefLst.Last();
    while( aDefLst.Remove() ) ;
}

/*************************************************************************
|*
|*    RscFile::Depend()
|*
|*    Beschreibung      Diese Methode gibt sal_True zurueck, wenn lDepend
|*                      existiert und hinter lFree steht, oder wenn
|*                      lDepend nicht existiert.
|*    Ersterstellung    MM 12.11.91
|*    Letzte Aenderung  MM 12.11.91
|*
*************************************************************************/
sal_Bool RscFile::Depend( sal_uLong lDepend, sal_uLong lFree ){
    RscDepend * pDep;

    pDep = Last();
    while( pDep ){
        if( pDep->GetFileKey() == lDepend ){
            while( pDep ){
                if( pDep->GetFileKey() == lFree )
                    return sal_True;
                pDep = Prev();
            }
            return sal_False;
        }
        pDep = Prev();
    };

    return sal_True;
}

/*************************************************************************
|*
|*    RscFile::InsertDependFile()
|*
|*    Beschreibung
|*    Ersterstellung    MM 06.01.92
|*    Letzte Aenderung  MM 06.01.92
|*
*************************************************************************/
sal_Bool RscFile :: InsertDependFile( sal_uLong lIncFile, sal_uLong lPos )
{
    RscDepend * pDep;

    pDep = First();
    while( pDep ){
        if( pDep->GetFileKey() == lIncFile )
            return sal_True;
        pDep = Next();
    }

    // Current-Zeiger steht auf letztem Element
    if( lPos >= Count() ){ //letztes Element muss immer letztes bleiben
        // Abhaengigkeit vor der letzten Position eintragen
        Insert( new RscDepend( lIncFile ) );
    }
    else
        Insert( new RscDepend( lIncFile ), lPos );

    return sal_True;
}

/*************************************************************************
|*
|*    RscFile::RemoveDependFile()
|*
|*    Beschreibung
|*    Ersterstellung    MM 18.11.91
|*    Letzte Aenderung  MM 18.11.91
|*
*************************************************************************/
void RscFile :: RemoveDependFile( sal_uLong lDepFile )
{

    RscDepend * pDep = Last();

    while( pDep ){
        if( pDep->GetFileKey() == lDepFile ){
            Remove( pDep );
            delete pDep;
        }
        pDep = Prev();
    }
}

/****************** R s c D e f T r e e **********************************/
/*************************************************************************
|*
|*    RscDefTree::~RscDefTree()
|*
|*    Beschreibung
|*    Ersterstellung    MM 12.11.91
|*    Letzte Aenderung  MM 12.11.91
|*
*************************************************************************/
RscDefTree::~RscDefTree(){
    Remove();
}

/*************************************************************************
|*
|*    RscDefTree::Remove()
|*
|*    Beschreibung
|*    Ersterstellung    MM 12.11.91
|*    Letzte Aenderung  MM 12.11.91
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
|*    Beschreibung
|*    Ersterstellung    MM 12.11.91
|*    Letzte Aenderung  MM 12.11.91
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
|*    Beschreibung
|*    Ersterstellung    MM 12.11.91
|*    Letzte Aenderung  MM 12.11.91
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
|*    Beschreibung
|*    Ersterstellung    MM 12.11.91
|*    Letzte Aenderung  MM 12.11.91
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
|*    Beschreibung
|*    Ersterstellung    MM 12.11.91
|*    Letzte Aenderung  MM 12.11.91
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

sal_Bool RscDefTree::Evaluate(){
    return Evaluate( pDefRoot );
}

/****************** R s c F i l e T a b **********************************/
/*************************************************************************
|*
|*    RscFileTab::RscFileTab()
|*
|*    Beschreibung
|*    Ersterstellung    MM 07.11.91
|*    Letzte Aenderung  MM 07.11.91
|*
*************************************************************************/
RscFileTab::RscFileTab(){
}

/*************************************************************************
|*
|*    RscFileTab::~RscFileTab()
|*
|*    Beschreibung
|*    Ersterstellung    MM 04.11.91
|*    Letzte Aenderung  MM 04.11.91
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
|*    Beschreibung
|*    Ersterstellung    MM 16.05.91
|*    Letzte Aenderung  MM 16.05.91
|*
*************************************************************************/
sal_uLong  RscFileTab :: Find( const ByteString & rName )
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
|*    Beschreibung
|*    Ersterstellung    MM 30.10.91
|*    Letzte Aenderung  MM 01.11.91
|*
*************************************************************************/
RscDefine * RscFileTab::FindDef( const char * pName ){
    return aDefTree.Search( pName );
}

/*************************************************************************
|*
|*    RscFileTab::FindDef()
|*
|*    Beschreibung
|*    Ersterstellung    MM 30.10.91
|*    Letzte Aenderung  MM 01.11.91
|*
*************************************************************************/
RscDefine * RscFileTab::FindDef( sal_uLong lFileKey, const ByteString & rName )
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
|*    Beschreibung
|*    Ersterstellung    MM 08.11.91
|*    Letzte Aenderung  MM 08.11.91
|*
*************************************************************************/
sal_Bool RscFileTab::Depend( sal_uLong lDepend, sal_uLong lFree ){
    if( lDepend == lFree )
        return sal_True;

    RscFile * pFile = First();
    while( pFile ){
        if( !pFile->IsIncFile() ){
            if( !pFile->Depend( lDepend, lFree ) )
                return sal_False;
        };
        pFile = Next();
    };

    return sal_True;
}

/*************************************************************************
|*
|*    RscFileTab::TestDef()
|*
|*    Beschreibung
|*    Ersterstellung    MM 14.01.92
|*    Letzte Aenderung  MM 14.01.92
|*
*************************************************************************/
sal_Bool RscFileTab::TestDef( sal_uLong lFileKey, sal_uLong lPos,
                          const RscDefine * pDefDec )
{
    if( lFileKey == pDefDec->GetFileKey() ){
        RscFile * pFile = GetFile( pDefDec->GetFileKey() );
        if( pFile && (lPos <= pFile->aDefLst.GetPos( (RscDefine *)pDefDec ))
          && (lPos != LIST_APPEND) )
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
|*    Beschreibung
|*    Ersterstellung    MM 14.01.92
|*    Letzte Aenderung  MM 14.01.92
|*
*************************************************************************/
sal_Bool RscFileTab::TestDef( sal_uLong lFileKey, sal_uLong lPos,
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
|*    Beschreibung
|*    Ersterstellung    MM 04.11.91
|*    Letzte Aenderung  MM 04.11.91
|*
*************************************************************************/
RscDefine * RscFileTab::NewDef( sal_uLong lFileKey, const ByteString & rDefName,
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
|*    Beschreibung
|*    Ersterstellung    MM 04.11.91
|*    Letzte Aenderung  MM 04.11.91
|*
*************************************************************************/
RscDefine * RscFileTab::NewDef( sal_uLong lFileKey, const ByteString & rDefName,
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
|*    RscFileTab::IsDefUsed()
|*
|*    Beschreibung
|*    Ersterstellung    MM 22.11.91
|*    Letzte Aenderung  MM 22.11.91
|*
*************************************************************************/
sal_Bool RscFileTab::IsDefUsed( const ByteString & rDefName )
{
    RscDefine * pDef = FindDef( rDefName );

    if( pDef )
        return( pDef->GetRefCount() != 2 );

    return sal_False;
}

/*************************************************************************
|*
|*    RscFileTab::DeleteDef()
|*
|*    Beschreibung
|*    Ersterstellung    MM 11.11.91
|*    Letzte Aenderung  MM 11.11.91
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
|*    Beschreibung
|*    Ersterstellung    MM 04.11.91
|*    Letzte Aenderung  MM 11.11.91
|*
*************************************************************************/
sal_Bool RscFileTab::ChangeDef( const ByteString & rDefName, sal_Int32 lId )
{
    RscDefine * pDef = FindDef( rDefName );

    if( pDef ){
        pDef->ChangeMacro( lId );
        //alle Macros neu bewerten
        return aDefTree.Evaluate();
    };
    return( sal_False );
}

/*************************************************************************
|*
|*    RscFileTab::ChangeDef()
|*
|*    Beschreibung
|*    Ersterstellung    MM 04.11.91
|*    Letzte Aenderung  MM 11.11.91
|*
*************************************************************************/
sal_Bool RscFileTab::ChangeDef( const ByteString & rDefName,
                            RscExpression * pExp )
{
    RscDefine * pDef = FindDef( rDefName );
    RscFile   * pFile;
    sal_uLong       lPos = 0;

    if( pDef ){
        pFile = GetFile( pDef->GetFileKey() );
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

    return( sal_False );
}

/*************************************************************************
|*
|*    RscFileTab::ChangeDefName()
|*
|*    Beschreibung
|*    Ersterstellung    MM 04.11.91
|*    Letzte Aenderung  MM 04.11.91
|*
*************************************************************************/
sal_Bool RscFileTab::ChangeDefName( const ByteString & rDefName,
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
            return( sal_True );
        }
    };

    return( sal_False );
}

/*************************************************************************
|*
|*    RscFileTab::DeleteFileContext()
|*
|*    Beschreibung
|*    Ersterstellung    MM 09.12.91
|*    Letzte Aenderung  MM 09.12.91
|*
*************************************************************************/
void RscFileTab :: DeleteFileContext( sal_uLong lFileKey ){
    RscFile     * pFName;

    pFName = GetFile( lFileKey );
    if( pFName ){
        RscDefine * pDef;

        pDef = pFName->aDefLst.First();
        while( pDef ){
            aDefTree.Remove( pDef );
            pDef = pFName->aDefLst.Next();
        };
        while( pFName->aDefLst.Remove( (sal_uLong)0 ) ) ;
    }
}

/*************************************************************************
|*
|*    RscFileTab::DeleteFile()
|*
|*    Beschreibung
|*    Ersterstellung    MM 16.05.91
|*    Letzte Aenderung  MM 16.05.91
|*
*************************************************************************/
void RscFileTab :: DeleteFile( sal_uLong lFileKey ){
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
|*    Beschreibung
|*    Ersterstellung    MM 16.05.91
|*    Letzte Aenderung  MM 16.05.91
|*
*************************************************************************/
sal_uLong  RscFileTab :: NewCodeFile( const ByteString & rName )
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
        pFName->InsertDependFile( lKey, LIST_APPEND );
    }
    return lKey;
}

/*************************************************************************
|*
|*    RscFileTab::NewIncFile()
|*
|*    Beschreibung
|*    Ersterstellung    MM 16.05.91
|*    Letzte Aenderung  MM 16.05.91
|*
*************************************************************************/
sal_uLong  RscFileTab :: NewIncFile( const ByteString & rName,
                                 const ByteString & rPath )
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
        pFName->InsertDependFile( lKey, LIST_APPEND );
    }
    return lKey;
}

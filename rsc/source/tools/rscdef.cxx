/*************************************************************************
 *
 *  $RCSfile: rscdef.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:42:56 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
/************************************************************************

    Source Code Control System - Header

    $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/rsc/source/tools/rscdef.cxx,v 1.1.1.1 2000-09-18 16:42:56 hr Exp $

    Source Code Control System - Update

    $Log: not supported by cvs2svn $
    Revision 1.13  2000/09/17 12:51:12  willem.vandorp
    OpenOffice header added.

    Revision 1.12  2000/07/26 17:13:24  willem.vandorp
    Headers/footers replaced

    Revision 1.11  2000/07/11 17:17:36  th
    Unicode

    Revision 1.10  1997/08/27 18:17:52  MM
    neue Headerstruktur

**************************************************************************/
/****************** I N C L U D E S **************************************/

// Programmuebergreifende Includes.
#ifndef _RSCDEF_HXX
#include <rscdef.hxx>
#endif

/****************** C o d e **********************************************/
/****************** R s c I d ********************************************/
BOOL RscId::bNames = TRUE;

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
void RscId::SetNames( BOOL bSet )  { bNames = bSet;  }
BOOL RscId::IsSetNames()           { return bNames;  }

/*************************************************************************
|*
|*    RscId::GetNumber
|*
|*    Beschreibung
|*    Ersterstellung    MM 17.05.91
|*    Letzte Aenderung  MM 17.05.91
|*
*************************************************************************/
long RscId::GetNumber() const{
    long lVal;
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
        long lValue;

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
BOOL RscId::operator == ( const RscId& rRscId ) const
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
BOOL RscId::operator < ( const RscId& rRscId ) const
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
BOOL RscId::operator > ( const RscId& rRscId ) const
{
    return( GetNumber() > rRscId.GetNumber() );
}

/*************************************************************************
|*
|*    RscId::long()
|*
|*    Beschreibung
|*    Ersterstellung    MM 16.05.91
|*    Letzte Aenderung  MM 16.05.91
|*
*************************************************************************/
RscId::operator long() const
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
RscDefine::RscDefine( ULONG lKey, const ByteString & rDefName, long lDefId )
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

void RscDefine::ChangeMacro( long lIdentifier ){
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
RscDefine * RscDefineList::New( ULONG lFileKey, const ByteString & rDefName,
                                long lDefId, ULONG lPos )
{
    RscDefine * pDef;

    pDef = new RscDefine( lFileKey, rDefName, lDefId );
    pDef->IncRef();
    Insert( pDef, lPos );
    return pDef;
}

RscDefine * RscDefineList::New( ULONG lFileKey, const ByteString & rDefName,
                                RscExpression * pExpression, ULONG lPos )
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
BOOL RscDefineList::Remove( RscDefine * pDef ){
    pDef = RscSubDefList::Remove( pDef );
    if( pDef ){
        pDef->DefineToNumber();
        pDef->DecRef();
    }

    return( NULL != pDef );
}

BOOL RscDefineList::Remove( ULONG lIndex ){
    RscDefine * pDef = RscSubDefList::Remove( lIndex );
    if( pDef ){
        pDef->DefineToNumber();
        pDef->DecRef();
    }

    return( NULL != pDef );
}

BOOL RscDefineList::Remove(){
    RscDefine * pDef;

    pDef = RscSubDefList::Remove( (ULONG)0 );

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
BOOL RscDefineList::Befor( const RscDefine * pFree,
                           const RscDefine * pDepend )
{
    RscDefine * pDef;

    pDef = First();
    while( pDef ){
        if( pDef == pFree ){
            pDef = Next();
            while( pDef ){
                if( pDef == pDepend )
                    return TRUE;
                pDef = Next();
            }
        }
        pDef = Next();
    };
    return FALSE;
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
BOOL RscExpType::Evaluate( long * plValue ) const{
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
BOOL RscExpression::Evaluate( long * plValue ){
    long lLeft;
    long lRight;

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
    bLoaded  = FALSE;
    bIncFile = FALSE;
    bDirty   = FALSE;
    bScanned = FALSE;
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
    RscDepend * pDep = Remove( (ULONG)0 );

    while( pDep ){
        delete pDep;
        pDep = Remove( (ULONG)0 );
    }

    //von hinten nach vorne ist besser wegen der Abhaengigkeiten
    //Objekte zerstoeren sich, wenn Referenzzaehler NULL
    aDefLst.Last();
    while( aDefLst.Remove() );
}

/*************************************************************************
|*
|*    RscFile::Depend()
|*
|*    Beschreibung      Diese Methode gibt TRUE zurueck, wenn lDepend
|*                      existiert und hinter lFree steht, oder wenn
|*                      lDepend nicht existiert.
|*    Ersterstellung    MM 12.11.91
|*    Letzte Aenderung  MM 12.11.91
|*
*************************************************************************/
BOOL RscFile::Depend( ULONG lDepend, ULONG lFree ){
    RscDepend * pDep;

    pDep = Last();
    while( pDep ){
        if( pDep->GetFileKey() == lDepend ){
            while( pDep ){
                if( pDep->GetFileKey() == lFree )
                    return TRUE;
                pDep = Prev();
            }
            return FALSE;
        }
        pDep = Prev();
    };

    return TRUE;
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
BOOL RscFile :: InsertDependFile( ULONG lIncFile, ULONG lPos )
{
    RscDepend * pDep;

    pDep = First();
    while( pDep ){
        if( pDep->GetFileKey() == lIncFile )
            return TRUE;
        pDep = Next();
    }

    // Current-Zeiger steht auf letztem Element
    if( lPos >= Count() ){ //letztes Element muss immer letztes bleiben
        // Abhaengigkeit vor der letzten Position eintragen
        Insert( new RscDepend( lIncFile ) );
    }
    else
        Insert( new RscDepend( lIncFile ), lPos );

    return TRUE;
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
void RscFile :: RemoveDependFile( ULONG lDepFile )
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
|*    Beschreibung
|*    Ersterstellung    MM 08.11.91
|*    Letzte Aenderung  MM 08.11.91
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
|*    Beschreibung
|*    Ersterstellung    MM 14.01.92
|*    Letzte Aenderung  MM 14.01.92
|*
*************************************************************************/
BOOL RscFileTab::TestDef( ULONG lFileKey, ULONG lPos,
                          const RscDefine * pDefDec )
{
    if( lFileKey == pDefDec->GetFileKey() ){
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
|*    Beschreibung
|*    Ersterstellung    MM 14.01.92
|*    Letzte Aenderung  MM 14.01.92
|*
*************************************************************************/
BOOL RscFileTab::TestDef( ULONG lFileKey, ULONG lPos,
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
|*    Beschreibung
|*    Ersterstellung    MM 04.11.91
|*    Letzte Aenderung  MM 04.11.91
|*
*************************************************************************/
RscDefine * RscFileTab::NewDef( ULONG lFileKey, const ByteString & rDefName,
                                long lId, ULONG lPos )
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
|*    Beschreibung
|*    Ersterstellung    MM 22.11.91
|*    Letzte Aenderung  MM 22.11.91
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
BOOL RscFileTab::ChangeDef( const ByteString & rDefName, long lId )
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
|*    Beschreibung
|*    Ersterstellung    MM 04.11.91
|*    Letzte Aenderung  MM 11.11.91
|*
*************************************************************************/
BOOL RscFileTab::ChangeDef( const ByteString & rDefName,
                            RscExpression * pExp )
{
    RscDefine * pDef = FindDef( rDefName );
    RscFile   * pFile;
    ULONG       lPos;

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

    return( FALSE );
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
|*    Beschreibung
|*    Ersterstellung    MM 09.12.91
|*    Letzte Aenderung  MM 09.12.91
|*
*************************************************************************/
void RscFileTab :: DeleteFileContext( ULONG lFileKey ){
    RscFile     * pFName;

    pFName = GetFile( lFileKey );
    if( pFName ){
        RscDefine * pDef;

        pDef = pFName->aDefLst.First();
        while( pDef ){
            aDefTree.Remove( pDef );
            pDef = pFName->aDefLst.Next();
        };
        while( pFName->aDefLst.Remove( (ULONG)0 ) );
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
|*    Beschreibung
|*    Ersterstellung    MM 16.05.91
|*    Letzte Aenderung  MM 16.05.91
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
|*    Beschreibung
|*    Ersterstellung    MM 16.05.91
|*    Letzte Aenderung  MM 16.05.91
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

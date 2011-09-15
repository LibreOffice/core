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
#include "precompiled_automation.hxx"

#include "comm_bas.hxx"
#include <tools/errcode.hxx>
#include <basic/sbxobj.hxx>
#include <basic/sbx.hxx>
#include <basic/sbxvar.hxx>
#include <vcl/svapp.hxx>
#include <automation/communi.hxx>
#include <basic/ttstrhlp.hxx>

// The CommunicationManager consists of the following elements:
// 1) Properties:
//    none
// 2) Methods:
//    CommunicationLink StartCommunication( Host, Port )
//    StopAllCommunication
//    sal_Bool IsCommunicationRunning
//    String GetMyName
//    sal_Bool IsLinkValid( CommunicationLink )
//    SetCommunicationEventHandler( String )

// The CommunicationLink consists of the following elements:
// 1) Properties:
//    none
// 2) Methods:
//    StopCommunication
//    String GetMyName
//    String GetHostName
//    Send( String )
//    String GetString


// This implementation is an example for a table-controlled
// version that can contain a lot of elements. The elements are
// taken from the table to the object when needed.

// The nArgs-field of a table entry is encrypted as follows:

#define _ARGSMASK   0x00FF  // up to 255 arguments
#define _RWMASK     0x0F00  // mask for R/W-Bits
#define _TYPEMASK   0xF000  // mask for the entry type

#define _READ       0x0100  // can be read
#define _BWRITE     0x0200  // can be used as Lvalue
#define _LVALUE     _BWRITE  // can be used as Lvalue
#define _READWRITE  0x0300  // both
#define _OPT        0x0400  // TRUE: optional parameter
#define _METHOD     0x1000  // mask bit for a method
#define _PROPERTY   0x2000  // mask bit for a property
#define _COLL       0x4000  // mask bit for a collection
                            // combination of bits above:
#define _FUNCTION   0x1100  // mask for a function
#define _LFUNCTION  0x1300  // mask for a function, that works as Lvalue too
#define _ROPROP     0x2100  // mask Read Only-Property
#define _WOPROP     0x2200  // mask Write Only-Property
#define _RWPROP     0x2300  // mask Read/Write-Property
#define _COLLPROP   0x4100  // mask Read-Collection-Element

#define COLLNAME    "Elements"  // the collection's name, here wired hard



CommunicationWrapper::Methods CommunicationWrapper::aManagerMethods[] = {

{ "StartCommunication", SbxEMPTY, &CommunicationWrapper::MStartCommunication, 2 | _FUNCTION },

    { "Host", SbxSTRING, NULL, 0 },
    { "Port", SbxLONG, NULL, 0 },

{ "StopAllCommunication", SbxEMPTY, &CommunicationWrapper::MStopAllCommunication, 0 | _FUNCTION },

{ "IsCommunicationRunning", SbxBOOL, &CommunicationWrapper::MIsCommunicationRunning, 0 | _FUNCTION },
// as FQDN
{ "GetMyName", SbxSTRING, &CommunicationWrapper::MGetMyName, 0 | _FUNCTION },

{ "IsLinkValid", SbxBOOL, &CommunicationWrapper::MIsLinkValid, 1 | _FUNCTION },

    { "Link", SbxOBJECT, NULL, 0 },

{ "SetCommunicationEventHandler", SbxEMPTY, &CommunicationWrapper::MSetCommunicationEventHandler, 1 | _FUNCTION },

    { "FuncName", SbxSTRING, NULL, 0 },

{ NULL, SbxNULL, NULL, -1 }}; // end of the table






CommunicationWrapper::Methods CommunicationWrapper::aLinkMethods[] = {

{ "StopCommunication", SbxEMPTY, &CommunicationWrapper::LStopCommunication, 0 | _FUNCTION },

{ "GetMyName", SbxSTRING, &CommunicationWrapper::LGetMyName, 0 | _FUNCTION },

{ "GetHostName", SbxSTRING, &CommunicationWrapper::LGetHostName, 0 | _FUNCTION },

{ "Send", SbxEMPTY, &CommunicationWrapper::LSend, 1 | _FUNCTION },

    { "SendString", SbxSTRING, NULL, 0 },

{ "GetString", SbxSTRING, &CommunicationWrapper::LGetString, 0 | _FUNCTION },

{ NULL, SbxNULL, NULL, -1 }};  // end of the table





// constructor for the manager
CommunicationWrapper::CommunicationWrapper( const String& rClass ) : SbxObject( rClass )
, m_pLink( NULL )
, m_bIsManager( sal_True )
, m_bCatchOpen( sal_False )
, m_pNewLink( NULL )
{
    m_pMethods = &aManagerMethods[0];
    m_pManager = new CommunicationManagerClientViaSocket;
    m_pManager->SetConnectionOpenedHdl( LINK( this, CommunicationWrapper, Open ) );
    m_pManager->SetConnectionClosedHdl( LINK( this, CommunicationWrapper, Close ) );
    m_pManager->SetDataReceivedHdl( LINK( this, CommunicationWrapper, Data ) );
}

// constructor for the link
CommunicationWrapper::CommunicationWrapper( CommunicationLink *pThisLink ) : SbxObject( CUniString("Link") )
, m_pLink( pThisLink )
, m_bIsManager( sal_False )
, m_bCatchOpen( sal_False )
, m_pNewLink( NULL )
{
    m_pMethods = &aLinkMethods[0];
    m_pManager = (CommunicationManagerClientViaSocket*)pThisLink->GetCommunicationManager();
}

// deconstructor
CommunicationWrapper::~CommunicationWrapper()
{
    if ( m_bIsManager )
        delete m_pManager;
}


// Search for an element:
// Here it goes through the method table until an appropriate one
// has been found.
// If the method/property has not been found, get back only NULL
// without error code because that way a whole chain of objects
// can be asked for the method/property.

SbxVariable* CommunicationWrapper::Find( const String& rName, SbxClassType t )
{
    // Does the element exist already?
    SbxVariable* pRes = SbxObject::Find( rName, t );
    if( !pRes && t != SbxCLASS_OBJECT )
    {
        // look for it if not
        Methods* p = m_pMethods;
        short nIndex = 0;
        sal_Bool bFound = sal_False;
        while( p->nArgs != -1 )
        {
            if( rName.CompareIgnoreCaseToAscii( p->pName ) == COMPARE_EQUAL )
            {
                bFound = sal_True; break;
            }
            nIndex += ( p->nArgs & _ARGSMASK ) + 1;
            p = m_pMethods + nIndex;
        }
        if( bFound )
        {
            // isolate args-fields:
            short nAccess = ( p->nArgs & _RWMASK ) >> 8;
            short nType   = ( p->nArgs & _TYPEMASK );
            String aName( p->pName, RTL_TEXTENCODING_ASCII_US );
            SbxClassType eCT = SbxCLASS_OBJECT;
            if( nType & _PROPERTY )
                eCT = SbxCLASS_PROPERTY;
            else if( nType & _METHOD )
                eCT = SbxCLASS_METHOD;
            pRes = Make( aName, eCT, p->eType );
            // We set the array-index + 1, because there are still
            // other standard properties existing, which have to be activated.
            pRes->SetUserData( nIndex + 1 );
            pRes->SetFlags( nAccess );
        }
    }
    return pRes;
}

// activation of an element or asking for an infoblock

void CommunicationWrapper::SFX_NOTIFY( SfxBroadcaster& rBC, const TypeId& rBCT,
                             const SfxHint& rHint, const TypeId& rHT )
{
    const SbxHint* pHint = PTR_CAST(SbxHint,&rHint);
    if( pHint )
    {
        SbxVariable* pVar = pHint->GetVar();
        SbxArray* pPar = pVar->GetParameters();
        sal_uInt16 nIndex = (sal_uInt16) pVar->GetUserData();
        // no index: hand on!
        if( nIndex )
        {
            sal_uLong t = pHint->GetId();
            if( t == SBX_HINT_INFOWANTED )
                pVar->SetInfo( GetInfo( (short) pVar->GetUserData() ) );
            else
            {
                sal_Bool bWrite = sal_False;
                if( t == SBX_HINT_DATACHANGED )
                    bWrite = sal_True;
                if( t == SBX_HINT_DATAWANTED || bWrite )
                {
                    // parameter test for methods:
                    sal_uInt16 nPar = m_pMethods[ --nIndex ].nArgs & 0x00FF;
                    // element 0 is the return value
                    if( ( !pPar && nPar )
                     || ( pPar && pPar->Count() != nPar+1 ) )
                        SetError( SbxERR_WRONG_ARGS );

                    else
                    {
                        (this->*(m_pMethods[ nIndex ].pFunc))( pVar, pPar, bWrite );
                    }
                }
            }
        }
        SbxObject::SFX_NOTIFY( rBC, rBCT, rHint, rHT );
    }
}

// construction of the info-structure for single elements

SbxInfo* CommunicationWrapper::GetInfo( short nIdx )
{
    Methods* p = &m_pMethods[ nIdx ];
    // if there's a help-file some time:
    // SbxInfo* pInfo = new SbxInfo( helpfilename, p->nHelpId );
    SbxInfo* pRetInfo = new SbxInfo;
    short nPar = p->nArgs & _ARGSMASK;
    for( short i = 0; i < nPar; i++ )
    {
        p++;
        String aName( p->pName, RTL_TEXTENCODING_ASCII_US );
        sal_uInt16 nIFlags = ( p->nArgs >> 8 ) & 0x03;
        if( p->nArgs & _OPT )
            nIFlags |= SBX_OPTIONAL;
        pRetInfo->AddParam( aName, p->eType, nIFlags );
    }
    return pRetInfo;
}


////////////////////////////////////////////////////////////////////////////

// help methods for the manager

IMPL_LINK( CommunicationWrapper, Open, CommunicationLink*, pLink )
{
    if ( m_bCatchOpen )
        m_pNewLink = pLink;
    else
        Events( CUniString("Open"), pLink );
    return 1;
}

IMPL_LINK( CommunicationWrapper, Close, CommunicationLink*, pLink )
{
    Events( CUniString("Close"), pLink );
    return 1;
}

IMPL_LINK( CommunicationWrapper, Data, CommunicationLink*, pLink )
{
    Events( CUniString("Data"), pLink );
    return 1;
}

void CommunicationWrapper::Events( String aType, CommunicationLink* pLink )
{
    if ( m_aEventHandlerName.Len() )
    {
        SbxArrayRef pPar = new SbxArray( SbxVARIANT );
        pPar->Put( new SbxVariable( SbxSTRING ), 1 );
        pPar->Get( 1 )->PutString( aType );

        pPar->Put( new SbxVariable( SbxOBJECT ), 2 );
        pPar->Get( 2 )->PutObject( new CommunicationWrapper( pLink ) );

        Call( m_aEventHandlerName, pPar );
    }
    else
        delete pLink->GetServiceData();     // give away the stream to prevent blocking
}


////////////////////////////////////////////////////////////////////////////

// Properties and methods put down the return value for Get (bPut = sal_False) at
// element 0 of the Argv; for Put (bPut = sal_True) the value from element 0 is saved.

// the methods:

// manager
void CommunicationWrapper::MStartCommunication( SbxVariable* pVar, SbxArray* pPar, sal_Bool /*bWrite*/ )
{ //    CommunicationLink StartCommunication( Host, Port )
    m_bCatchOpen = sal_True;
    if ( m_pManager->StartCommunication( ByteString( pPar->Get( 1 )->GetString(), RTL_TEXTENCODING_UTF8 ), pPar->Get( 2 )->GetULong() ) )
    {
        while ( !m_pNewLink )
            GetpApp()->Reschedule();
        m_bCatchOpen = sal_False;
        CommunicationWrapper *pNewLinkWrapper = new CommunicationWrapper( m_pNewLink );
        m_pNewLink = NULL;
        pVar->PutObject( pNewLinkWrapper );
    }

}

void CommunicationWrapper::MStopAllCommunication( SbxVariable* /*pVar*/, SbxArray* /*pPar*/, sal_Bool /*bWrite*/ )
{ //    StopAllCommunication
    m_pManager->StopCommunication();
}

void CommunicationWrapper::MIsCommunicationRunning( SbxVariable* pVar, SbxArray* /*pPar*/, sal_Bool /*bWrite*/ )
{ //    sal_Bool IsCommunicationRunning
    pVar->PutBool( m_pManager->IsCommunicationRunning() );
}

void CommunicationWrapper::MGetMyName( SbxVariable* pVar, SbxArray* /*pPar*/, sal_Bool /*bWrite*/ )
{ //    String GetMyName
    pVar->PutString( UniString( m_pManager->GetMyName( CM_FQDN ), RTL_TEXTENCODING_UTF8 ) );
}

void CommunicationWrapper::MIsLinkValid( SbxVariable* pVar, SbxArray* pPar, sal_Bool /*bWrite*/ )
{ //    sal_Bool IsLinkValid( CommunicationLink )
    CommunicationWrapper *pWrapper = (CommunicationWrapper*)(pPar->Get( 1 )->GetObject());
    pVar->PutBool( m_pManager->IsLinkValid( pWrapper->GetCommunicationLink() ) );
}

void CommunicationWrapper::MSetCommunicationEventHandler( SbxVariable* /*pVar*/, SbxArray* pPar, sal_Bool /*bWrite*/ )
{ //    SetCommunicationEventHandler( String )
    m_aEventHandlerName = pPar->Get( 1 )->GetString();
}





//      Link
void CommunicationWrapper::LStopCommunication( SbxVariable* /*pVar*/, SbxArray* /*pPar*/, sal_Bool /*bWrite*/ )
{ //    StopCommunication
    m_pLink->StopCommunication();
}

void CommunicationWrapper::LGetMyName( SbxVariable* pVar, SbxArray* /*pPar*/, sal_Bool /*bWrite*/ )
{ //    String GetMyName
    pVar->PutString( UniString( m_pLink->GetMyName( CM_FQDN ), RTL_TEXTENCODING_UTF8 ) );
}

void CommunicationWrapper::LGetHostName( SbxVariable* pVar, SbxArray* /*pPar*/, sal_Bool /*bWrite*/ )
{ //    String GetHostName
    pVar->PutString( UniString( m_pLink->GetCommunicationPartner( CM_FQDN ), RTL_TEXTENCODING_UTF8 ) );
}

void CommunicationWrapper::LSend( SbxVariable* /*pVar*/, SbxArray* pPar, sal_Bool /*bWrite*/ )
{ //    Send(String )
    SvStream *pSendStream = m_pLink->GetBestCommunicationStream();
    String aSendString = pPar->Get( 1 )->GetString();
    pSendStream->WriteByteString( aSendString, RTL_TEXTENCODING_UTF8 );
    m_pLink->TransferDataStream( pSendStream );
    delete pSendStream;
}

void CommunicationWrapper::LGetString( SbxVariable* pVar, SbxArray* /*pPar*/, sal_Bool /*bWrite*/ )
{ //    String GetString
    SvStream *pReceiveStream = m_pLink->GetServiceData();
    if ( pReceiveStream )
    {
        sal_uLong nLength = pReceiveStream->Seek( STREAM_SEEK_TO_END );
        pReceiveStream->Seek( STREAM_SEEK_TO_BEGIN );
        char *pBuffer = new char[nLength];
        pReceiveStream->Read( pBuffer, nLength );
        String aReceive(
            pBuffer, sal::static_int_cast< xub_StrLen >( nLength ),
            RTL_TEXTENCODING_UTF8 );
        delete [] pBuffer;
        pVar->PutString( aReceive );
        delete pReceiveStream;
    }
    else
        pVar->PutString( UniString() );
}



SbxObject* CommunicationFactory::CreateObject( const String& rClass )
{
    if( rClass.CompareIgnoreCaseToAscii( "CommunicationManager" ) == COMPARE_EQUAL )
        return new CommunicationWrapper( rClass );
    return NULL;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

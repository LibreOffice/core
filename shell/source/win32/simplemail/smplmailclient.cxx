/*************************************************************************
 *
 *  $RCSfile: smplmailclient.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: rt $ $Date: 2003-10-06 15:44:41 $
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

//------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#ifndef _SMPLMAILCLIENT_HXX_
#include "smplmailclient.hxx"
#endif

#ifndef _SMPLMAILMSG_HXX_
#include "smplmailmsg.hxx"
#endif

#ifndef _COM_SUN_STAR_SYSTEM_SIMPLEMAILCLIENTFLAGS_HPP_
#include <com/sun/star/system/SimpleMailClientFlags.hpp>
#endif

#ifndef _RTL_TENCINFO_H
#include <rtl/tencinfo.h>
#endif

#ifndef _OSL_THREAD_H_
#include <osl/thread.h>
#endif

#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif

#include <process.h>

//------------------------------------------------------------------------
// namespace directives
//------------------------------------------------------------------------

using com::sun::star::uno::Reference;
using com::sun::star::uno::Exception;
using com::sun::star::uno::RuntimeException;
using com::sun::star::uno::Sequence;
using com::sun::star::lang::IllegalArgumentException;
using com::sun::star::system::XSimpleMailClient;
using com::sun::star::system::XSimpleMailMessage;
using rtl::OUString;
using rtl::OString;
using osl::Mutex;

using namespace com::sun::star::system::SimpleMailClientFlags;
using namespace cppu;

//------------------------------------------------
//
//------------------------------------------------

namespace // private
{
    const sal_uInt32 LEN_SMTP_PREFIX = 5; // "SMTP:"

    inline
    OString getSmtpPrefixedEmailAddress( const OUString& aAddress )
    {
        return  OString( "SMTP:" ) +
                OString(
                    aAddress.getStr( ),
                    aAddress.getLength( ),
                    osl_getThreadTextEncoding( ) );
    }

    struct MAPISendMailParam
    {
        CSimpleMapi* pSimpleMapi;
        LHANDLE lhSession;
        ULONG ulUIParam;
        lpMapiMessage lpMessage;
        FLAGS flFlags;
    };

} // end private namespace

//------------------------------------------------
//
//------------------------------------------------

CSmplMailClient::CSmplMailClient( LHANDLE hMapiSession ) :
    m_hMapiSession( hMapiSession ),
    m_pSimpleMapi( CSimpleMapi::create( ) )
{
    OSL_ASSERT( m_hMapiSession );
}

//------------------------------------------------
//
//------------------------------------------------

CSmplMailClient::~CSmplMailClient( )
{
    ULONG ulRet = m_pSimpleMapi->MAPILogoff( m_hMapiSession, 0, 0, 0 );
    OSL_ASSERT( SUCCESS_SUCCESS == ulRet );
}

//------------------------------------------------
//
//------------------------------------------------

Reference< XSimpleMailMessage > SAL_CALL CSmplMailClient::createSimpleMailMessage(  )
        throw (::com::sun::star::uno::RuntimeException)
{
    return Reference< XSimpleMailMessage >( new CSmplMailMsg( ) );
}

//------------------------------------------------
//
//------------------------------------------------

void SAL_CALL CSmplMailClient::sendSimpleMailMessage( const Reference< XSimpleMailMessage >& xSimpleMailMessage, sal_Int32 aFlag )
    throw (IllegalArgumentException, Exception, RuntimeException)
{
    try
    {
        validateParameter( xSimpleMailMessage, aFlag );

        MapiMessage mapiMsg;
        FLAGS       flFlags;

        initMapiMessage( xSimpleMailMessage, mapiMsg );
        initMapiSendMailFlags( aFlag, flFlags );

        // fix for #95743 we must create a separate thread because
        // again simple mapi works only from within a STA thread :-(
        ULONG ulRet = threadExecuteMAPISendMail(
            m_pSimpleMapi,
            m_hMapiSession, // we use an existing session #93077#
            0,              // no parent window
            &mapiMsg,       // a configured mapi message struct
            flFlags );      // reserved

        if ( SUCCESS_SUCCESS != ulRet )
        {
            throw Exception(
                getMapiErrorMsg( ulRet ),
                static_cast< XSimpleMailClient* >( this ) );
        }
    }
    catch( RuntimeException& )
    {
        OSL_ASSERT( sal_False );
    }
}

//------------------------------------------------
//
//------------------------------------------------

ULONG SAL_CALL CSmplMailClient::threadExecuteMAPISendMail( CSimpleMapi* pSimpleMapi, LHANDLE lhSession, ULONG ulUIParam, lpMapiMessage lpMessage, FLAGS flFlags )
{
    ULONG    ulRet = MAPI_E_FAILURE;
    unsigned ThreadId;

    MAPISendMailParam param;

    param.pSimpleMapi = pSimpleMapi;
    param.lhSession   = lhSession;
    param.ulUIParam   = ulUIParam;
    param.lpMessage   = lpMessage;
    param.flFlags     = flFlags;

    HANDLE hThread = reinterpret_cast< HANDLE >(
        _beginthreadex( 0, 0, CSmplMailClient::threadProc, &param, 0, &ThreadId ) );

    OSL_ENSURE( hThread, "could not create STA thread" );

    if ( hThread )
    {
        // stop the calling thread until the sta thread has ended
        WaitForSingleObject( hThread, INFINITE );
        GetExitCodeThread( hThread, &ulRet );
        CloseHandle( hThread );
    }

    return ulRet;
}

//------------------------------------------------
//
//------------------------------------------------

unsigned __stdcall CSmplMailClient::threadProc( void* pParam )
{
    MAPISendMailParam* param = reinterpret_cast< MAPISendMailParam* >( pParam );

    CSimpleMapi* pSimpleMapi = param->pSimpleMapi;

    OSL_ENSURE( pSimpleMapi, "invalid thread parameter" );

    return pSimpleMapi->MAPISendMail(
        param->lhSession,  // we use an existing session #93077#
        param->ulUIParam,  // no parent window
        param->lpMessage,  // a configured mapi message struct
        param->flFlags,    // some flags
        0 );              // reserved
}

//------------------------------------------------
//
//------------------------------------------------

void CSmplMailClient::validateParameter(
    const Reference< XSimpleMailMessage >& xSimpleMailMessage, sal_Int32 aFlag )
{
    // check the flags, the allowed range is 0 - (2^n - 1)
    if ( aFlag < 0 || aFlag > 3 )
        throw IllegalArgumentException(
            OUString::createFromAscii( "Invalid flag value" ),
            static_cast< XSimpleMailClient* >( this ),
            2 );

    // check if a recipient is specified of the flags NO_USER_INTERFACE is specified
    if ( (aFlag & NO_USER_INTERFACE) && !xSimpleMailMessage->getRecipient( ).getLength( ) )
        throw IllegalArgumentException(
            OUString::createFromAscii( "No recipient specified" ),
            static_cast< XSimpleMailClient* >( this ),
            1 );
}

//------------------------------------------------
//
//------------------------------------------------

void CSmplMailClient::initMapiMessage(
    const Reference< XSimpleMailMessage >& xSimpleMailMessage, MapiMessage& aMapiMessage )
{
    ZeroMemory( &aMapiMessage, sizeof( aMapiMessage ) );

    if ( xSimpleMailMessage.is( ) )
    {
        // unfortunately the simple mapi functions have
        // only an ANSI prototype, so we have to convert
        // all strings to ascii assuming an us-ascii
        // encoding

        // we hand the buffer of this OStrings directly
        // to the MapiMessage members but have to
        // cast away the constness of the returned buffer
        // pointer, we assume the function MAPISendMail
        // doesn't alter the strings

        m_Subject = OString(
            xSimpleMailMessage->getSubject( ).getStr( ),
            xSimpleMailMessage->getSubject( ).getLength( ),
            osl_getThreadTextEncoding( ) );

        aMapiMessage.lpszSubject = const_cast< LPSTR >( m_Subject.getStr( ) );

        // set the originator information

        if ( xSimpleMailMessage->getOriginator( ).getLength( ) )
        {
            ZeroMemory( &m_MsgOriginator, sizeof( m_MsgOriginator ) );

            m_SmtpAddressOriginator = getSmtpPrefixedEmailAddress(
                xSimpleMailMessage->getOriginator( ) );

            m_MsgOriginator.ulRecipClass = MAPI_ORIG;
            m_MsgOriginator.lpszName     = "";
            m_MsgOriginator.lpszAddress  =
                const_cast< LPSTR >( m_SmtpAddressOriginator.getStr( ) );

            aMapiMessage.lpOriginator = &m_MsgOriginator;
        }

        // set the recipient information

        sal_uInt32 nRecips = calcNumRecipients( xSimpleMailMessage );
        if ( nRecips > 0 )
        {
            m_RecipientList.realloc( nRecips );
            m_RecipientList.clean( );
            m_RecipsSmtpAddressList.clear( );

            size_t nPos = 0;

            // init the main recipient
            OUString aRecipient = xSimpleMailMessage->getRecipient( );
            if ( aRecipient.getLength( ) )
            {
                m_RecipsSmtpAddressList.push_back(
                    getSmtpPrefixedEmailAddress( aRecipient ) );

                m_RecipientList[nPos].ulRecipClass = MAPI_TO;
                m_RecipientList[nPos].lpszName     =
                    const_cast< LPSTR >( m_RecipsSmtpAddressList.back( ).getStr( ) + LEN_SMTP_PREFIX );
                m_RecipientList[nPos].lpszAddress  =
                    const_cast< LPSTR >( m_RecipsSmtpAddressList.back( ).getStr( ) );

                nPos++;
            }

            // add all cc recipients to the list
            initRecipientList(
                xSimpleMailMessage->getCcRecipient( ),
                MAPI_CC,
                nPos );

            // add all bcc recipients to the list
            initRecipientList(
                xSimpleMailMessage->getBccRecipient( ),
                MAPI_BCC,
                nPos );

            aMapiMessage.lpRecips    = &m_RecipientList;
            aMapiMessage.nRecipCount = m_RecipientList.size( );
        }

        initAttachementList( xSimpleMailMessage );
        aMapiMessage.lpFiles    = &m_AttachementList;
        aMapiMessage.nFileCount = m_AttachementList.size( );
    }
}

//------------------------------------------------
//
//------------------------------------------------

sal_uInt32 CSmplMailClient::calcNumRecipients(
    const Reference< XSimpleMailMessage >& xSimpleMailMessage )
{
    sal_uInt32 nRecips = xSimpleMailMessage->getCcRecipient( ).getLength( );
    nRecips += xSimpleMailMessage->getBccRecipient( ).getLength( );

    if ( xSimpleMailMessage->getRecipient( ).getLength( ) )
        nRecips += 1;

    return nRecips;
}

//------------------------------------------------
//
//------------------------------------------------

void CSmplMailClient::initRecipientList(
    const Sequence< rtl::OUString >& aRecipList,
    ULONG ulRecipClass,
    size_t& nPos )
{
    OSL_PRECOND( nPos < m_RecipientList.size( ), "Wrong index" );

    for( sal_Int32 i = 0; i < aRecipList.getLength( ); i++ )
    {
        m_RecipsSmtpAddressList.push_back(
            getSmtpPrefixedEmailAddress( aRecipList[i] ) );

        m_RecipientList[nPos].ulRecipClass = ulRecipClass;
        m_RecipientList[nPos].lpszName =
            const_cast< LPSTR >( m_RecipsSmtpAddressList.back( ).getStr( ) + LEN_SMTP_PREFIX );
        m_RecipientList[nPos].lpszAddress  =
            const_cast< LPSTR >( m_RecipsSmtpAddressList.back( ).getStr( ) );

        nPos++;
    }
}

//------------------------------------------------
//
//------------------------------------------------

void CSmplMailClient::initAttachementList(
    const Reference< XSimpleMailMessage >& xSimpleMailMessage )
{
    Sequence< OUString > aAttachementList =
        xSimpleMailMessage->getAttachement( );

    sal_uInt32 nAttachements = aAttachementList.getLength( );

    // avoid old entries
    m_AttchmtsSysPathList.clear( );
    m_AttachementList.realloc( nAttachements );
    m_AttachementList.clean( );

    OUString aSysPath;
    for ( sal_uInt32 i = 0; i < nAttachements; i++ )
    {
        osl::FileBase::RC rc =
            osl::FileBase::getSystemPathFromFileURL(
                aAttachementList[i], aSysPath );

        if ( osl::FileBase::RC::E_None != rc )
            throw IllegalArgumentException(
                OUString::createFromAscii( " " ),
                static_cast< XSimpleMailClient* >( this ),
                1 );

        m_AttchmtsSysPathList.push_back(
            OString(
                aSysPath.getStr( ),
                aSysPath.getLength( ),
                osl_getThreadTextEncoding( ) ) );

        m_AttachementList[i].lpszPathName = const_cast< LPSTR >(
            m_AttchmtsSysPathList.back( ).getStr( ) );
        m_AttachementList[i].nPosition = -1;
    }
}

//------------------------------------------------
//
//------------------------------------------------

void CSmplMailClient::initMapiSendMailFlags( sal_Int32 aFlags, FLAGS& aMapiFlags )
{
    // #93077#
    OSL_ENSURE( !( aFlags & NO_LOGON_DIALOG ), "Flag NO_LOGON_DIALOG has currently no effect" );

    aMapiFlags = 0; // we should not use MAPI_UNICODE else
                    // Netscape interprets all string as UNICODE!
                    // #93077# MAPI_NEW_SESSION

    if ( !( aFlags & NO_USER_INTERFACE ) )
        aMapiFlags |= MAPI_DIALOG;

    if ( !( aFlags & NO_LOGON_DIALOG ) )
        aMapiFlags |= MAPI_LOGON_UI;
}

//------------------------------------------------
//
//------------------------------------------------

rtl::OUString CSmplMailClient::getMapiErrorMsg( ULONG ulMapiError )
{
    LPVOID lpMsgBuff;
    DWORD dwRet = FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_HMODULE,
        GetModuleHandleA( "mapi32.dll" ),
        ulMapiError,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        reinterpret_cast< LPSTR >( &lpMsgBuff ),
        0,
        NULL );

    OUString errMsg;

    if ( dwRet )
    {
        errMsg = OUString::createFromAscii(
            static_cast< LPSTR >( lpMsgBuff ) );
        LocalFree( lpMsgBuff );
    }

    return errMsg;
}

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
#include "precompiled_sfx2.hxx"

#if defined(WIN) || defined(WNT)
#include <tools/svwin.h>
#endif

#include "impldde.hxx"

#include <vcl/svapp.hxx>
#include <vcl/fixed.hxx>
#include <vcl/edit.hxx>
#include <vcl/button.hxx>
#include <vcl/msgbox.hxx>
#include <sot/exchange.hxx>
#include <rtl/ustring.hxx>

#include "dde.hrc"
#include <sfx2/lnkbase.hxx>
#include <sfx2/linkmgr.hxx>
#include "sfxresid.hxx"

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>

#include <svl/svdde.hxx>
#include <sot/formats.hxx>

#define DDELINK_COLD        0
#define DDELINK_HOT         1

#define DDELINK_ERROR_APP   1
#define DDELINK_ERROR_DATA  2
#define DDELINK_ERROR_LINK  3

using namespace ::com::sun::star::uno;

namespace sfx2
{

class SvDDELinkEditDialog : public ModalDialog
{
    FixedText aFtDdeApp;
    Edit aEdDdeApp;
    FixedText aFtDdeTopic;
    Edit aEdDdeTopic;
    FixedText aFtDdeItem;
    Edit aEdDdeItem;
    FixedLine aGroupDdeChg;
    OKButton aOKButton1;
    CancelButton aCancelButton1;

    DECL_STATIC_LINK( SvDDELinkEditDialog, EditHdl_Impl, Edit* );
public:
    SvDDELinkEditDialog( Window* pParent, SvBaseLink* );
    String GetCmd() const;
};

SvDDELinkEditDialog::SvDDELinkEditDialog( Window* pParent, SvBaseLink* pLink )
    : ModalDialog( pParent, SfxResId( MD_DDE_LINKEDIT ) ),
    aFtDdeApp( this, SfxResId( FT_DDE_APP ) ),
    aEdDdeApp( this, SfxResId( ED_DDE_APP ) ),
    aFtDdeTopic( this, SfxResId( FT_DDE_TOPIC ) ),
    aEdDdeTopic( this, SfxResId( ED_DDE_TOPIC ) ),
    aFtDdeItem( this, SfxResId( FT_DDE_ITEM ) ),
    aEdDdeItem( this, SfxResId( ED_DDE_ITEM ) ),
    aGroupDdeChg( this, SfxResId( GROUP_DDE_CHG ) ),
    aOKButton1( this, SfxResId( 1 ) ),
    aCancelButton1( this, SfxResId( 1 ) )
{
    FreeResource();

    String sServer, sTopic, sItem;
    pLink->GetLinkManager()->GetDisplayNames( pLink, &sServer, &sTopic, &sItem );

    aEdDdeApp.SetText( sServer );
    aEdDdeTopic.SetText( sTopic );
    aEdDdeItem.SetText( sItem );

    aEdDdeApp.SetModifyHdl( STATIC_LINK( this, SvDDELinkEditDialog, EditHdl_Impl));
    aEdDdeTopic.SetModifyHdl( STATIC_LINK( this, SvDDELinkEditDialog, EditHdl_Impl));
    aEdDdeItem.SetModifyHdl( STATIC_LINK( this, SvDDELinkEditDialog, EditHdl_Impl));

    aOKButton1.Enable( sServer.Len() && sTopic.Len() && sItem.Len() );
}

String SvDDELinkEditDialog::GetCmd() const
{
    String sCmd( aEdDdeApp.GetText() ), sRet;
    ::sfx2::MakeLnkName( sRet, &sCmd, aEdDdeTopic.GetText(), aEdDdeItem.GetText() );
    return sRet;
}

IMPL_STATIC_LINK( SvDDELinkEditDialog, EditHdl_Impl, Edit *, pEdit )
{
    (void)pEdit; // unused variable
    pThis->aOKButton1.Enable( pThis->aEdDdeApp.GetText().Len() &&
                              pThis->aEdDdeTopic.GetText().Len() &&
                              pThis->aEdDdeItem.GetText().Len() );
    return 0;
}

SvDDEObject::SvDDEObject()
    : pConnection( 0 ), pLink( 0 ), pRequest( 0 ), pGetData( 0 ), nError( 0 )
{
    SetUpdateTimeout( 100 );
    bWaitForData = FALSE;
}

SvDDEObject::~SvDDEObject()
{
    delete pLink;
    delete pRequest;
    delete pConnection;
}

BOOL SvDDEObject::GetData( ::com::sun::star::uno::Any & rData /*out param*/,
                            const String & rMimeType,
                            BOOL bSynchron )
{
    if( !pConnection )
        return FALSE;

    if( pConnection->GetError() )  // then we try once more
    {
        String sServer( pConnection->GetServiceName() );
        String sTopic( pConnection->GetTopicName() );

        delete pConnection;
        pConnection = new DdeConnection( sServer, sTopic );
        if( pConnection->GetError() )
            nError = DDELINK_ERROR_APP;
    }

    if( bWaitForData ) // we are in an rekursive loop, get out again
        return FALSE;

    // Lock against Reentrance
    bWaitForData = TRUE;

    // if you want to print, we'll wait until the data is available
    if( bSynchron )
    {
        DdeRequest aReq( *pConnection, sItem, 5000 );
        aReq.SetDataHdl( LINK( this, SvDDEObject, ImplGetDDEData ) );
        aReq.SetFormat( SotExchange::GetFormatIdFromMimeType( rMimeType ));

        pGetData = &rData;

        do {
            aReq.Execute();
        } while( aReq.GetError() && ImplHasOtherFormat( aReq ) );

        if( pConnection->GetError() )
            nError = DDELINK_ERROR_DATA;

        bWaitForData = FALSE;
    }
    else
    {
        // otherwise it will be executed asynchronously
        {
            if( pRequest )
                delete pRequest;

            pRequest = new DdeRequest( *pConnection, sItem );
            pRequest->SetDataHdl( LINK( this, SvDDEObject, ImplGetDDEData ) );
            pRequest->SetDoneHdl( LINK( this, SvDDEObject, ImplDoneDDEData ) );
            pRequest->SetFormat( SotExchange::GetFormatIdFromMimeType(
                                    rMimeType ) );
            pRequest->Execute();
        }

        ::rtl::OUString aEmptyStr;
        rData <<= aEmptyStr;
    }
    return 0 == pConnection->GetError();
}


BOOL SvDDEObject::Connect( SvBaseLink * pSvLink )
{
#if defined(WIN) || defined(WNT)
    static BOOL bInWinExec = FALSE;
#endif
    USHORT nLinkType = pSvLink->GetUpdateMode();
    if( pConnection )           // Connection is already made
    {
        // well, then just add it as dependent
        AddDataAdvise( pSvLink,
                SotExchange::GetFormatMimeType( pSvLink->GetContentType()),
                LINKUPDATE_ONCALL == nLinkType
                        ? ADVISEMODE_ONLYONCE
                        : 0 );
        AddConnectAdvise( pSvLink );

        return TRUE;
    }

    if( !pSvLink->GetLinkManager() )
        return FALSE;

    String sServer, sTopic;
    pSvLink->GetLinkManager()->GetDisplayNames( pSvLink, &sServer, &sTopic, &sItem );

    if( !sServer.Len() || !sTopic.Len() || !sItem.Len() )
        return FALSE;

    pConnection = new DdeConnection( sServer, sTopic );
    if( pConnection->GetError() )
    {
        // kann man denn das System-Topic ansprechen ?
        // dann ist der Server oben, kennt nur nicht das Topic!
       // Is it possible to address the system-Topic?
       // then the server is up, it just does not know the topic!
        if( sTopic.EqualsIgnoreCaseAscii( "SYSTEM" ) )
        {
            BOOL bSysTopic;
            {
                DdeConnection aTmp( sServer, String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "SYSTEM" ) ) );
                bSysTopic = !aTmp.GetError();
            }

            if( bSysTopic )
            {
                nError = DDELINK_ERROR_DATA;
                return FALSE;
            }
            // otherwise in  Win/WinNT, start the Application directly
        }

#if defined(WIN) || defined(WNT)

        // Server not up, try once more to start it.
        if( !bInWinExec )
        {
            ByteString aCmdLine( sServer, RTL_TEXTENCODING_ASCII_US );
            aCmdLine.Append( ".exe " );
            aCmdLine.Append( ByteString( sTopic, RTL_TEXTENCODING_ASCII_US ) );

            if( WinExec( aCmdLine.GetBuffer(), SW_SHOWMINIMIZED ) < 32 )
                nError = DDELINK_ERROR_APP;
            else
            {
                USHORT i;
                for( i=0; i<5; i++ )
                {
                    bInWinExec = TRUE;
                    Application::Reschedule();
                    bInWinExec = FALSE;

                    delete pConnection;
                    pConnection = new DdeConnection( sServer, sTopic );
                    if( !pConnection->GetError() )
                        break;
                }

                if( i == 5 )
                {
                    nError = DDELINK_ERROR_APP;
                }
            }
        }
        else
#endif  // WIN / WNT
        {
            nError = DDELINK_ERROR_APP;
        }
    }

    if( LINKUPDATE_ALWAYS == nLinkType && !pLink && !pConnection->GetError() )
    {
        // Setting up Hot Link, Data will be available at some point later on
        pLink = new DdeHotLink( *pConnection, sItem );
        pLink->SetDataHdl( LINK( this, SvDDEObject, ImplGetDDEData ) );
        pLink->SetDoneHdl( LINK( this, SvDDEObject, ImplDoneDDEData ) );
        pLink->SetFormat( pSvLink->GetContentType() );
        pLink->Execute();
    }

    if( pConnection->GetError() )
        return FALSE;

    AddDataAdvise( pSvLink,
                SotExchange::GetFormatMimeType( pSvLink->GetContentType()),
                LINKUPDATE_ONCALL == nLinkType
                        ? ADVISEMODE_ONLYONCE
                        : 0 );
    AddConnectAdvise( pSvLink );
    SetUpdateTimeout( 0 );
    return TRUE;
}

void SvDDEObject::Edit( Window* pParent, sfx2::SvBaseLink* pBaseLink, const Link& rEndEditHdl )
{
    SvDDELinkEditDialog aDlg( pParent, pBaseLink );
    if ( RET_OK == aDlg.Execute() && rEndEditHdl.IsSet() )
    {
        String sCommand = aDlg.GetCmd();
        rEndEditHdl.Call( &sCommand );
    }
}

BOOL SvDDEObject::ImplHasOtherFormat( DdeTransaction& rReq )
{
    USHORT nFmt = 0;
    switch( rReq.GetFormat() )
    {
    case FORMAT_RTF:
        nFmt = FORMAT_STRING;
        break;

    case SOT_FORMATSTR_ID_HTML_SIMPLE:
    case SOT_FORMATSTR_ID_HTML:
        nFmt = FORMAT_RTF;
        break;

    case FORMAT_GDIMETAFILE:
        nFmt = FORMAT_BITMAP;
        break;

    case SOT_FORMATSTR_ID_SVXB:
        nFmt = FORMAT_GDIMETAFILE;
        break;

    // something else?
    }
    if( nFmt )
        rReq.SetFormat( nFmt );         // try it once more
    return 0 != nFmt;
}

BOOL SvDDEObject::IsPending() const
/*  [Description]

    The method determines whether the data-object can be read from a DDE.

    Returned is the following:
        ERRCODE_NONE                    if it has been completely read
        ERRCODE_SO_PENDING              if it has not been completely read
        ERRCODE_SO_FALSE                otherwise
*/
{
    return bWaitForData;
}

BOOL SvDDEObject::IsDataComplete() const
{
    return bWaitForData;
}

IMPL_LINK( SvDDEObject, ImplGetDDEData, DdeData*, pData )
{
    ULONG nFmt = pData->GetFormat();
    switch( nFmt )
    {
    case FORMAT_GDIMETAFILE:
        break;

    case FORMAT_BITMAP:
        break;

    default:
        {
            const sal_Char* p = (sal_Char*)( pData->operator const void*() );
            long nLen = FORMAT_STRING == nFmt ? (p ? strlen( p ) : 0) : (long)*pData;

            Sequence< sal_Int8 > aSeq( (const sal_Int8*)p, nLen );
            if( pGetData )
            {
                *pGetData <<= aSeq;  // Copy Data
                pGetData = 0;        // reset the pointer here
            }
            else
            {
                Any aVal;
                aVal <<= aSeq;
                DataChanged( SotExchange::GetFormatMimeType(
                                                pData->GetFormat() ), aVal );
                bWaitForData = FALSE;
            }
        }
    }

    return 0;
}

IMPL_LINK( SvDDEObject, ImplDoneDDEData, void*, pData )
{
    BOOL bValid = (BOOL)(ULONG)pData;
    if( !bValid && ( pRequest || pLink ))
    {
        DdeTransaction* pReq = 0;
        if( !pLink || ( pLink && pLink->IsBusy() ))
            pReq = pRequest;  // only the one that is ready
        else if( pRequest && pRequest->IsBusy() )
            pReq = pLink;  // only the one that is ready

        if( pReq )
        {
            if( ImplHasOtherFormat( *pReq ) )
            {
                pReq->Execute();
            }
            else if( pReq == pRequest )
            {
                bWaitForData = FALSE;
            }
        }
    }
    else
        // End waiting
        bWaitForData = FALSE;

    return 0;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

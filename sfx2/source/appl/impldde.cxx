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


#if defined(WNT)
#include <prewin.h>
#include <postwin.h>
#endif

#include "impldde.hxx"

#include <comphelper/string.hxx>
#include <vcl/svapp.hxx>
#include <vcl/fixed.hxx>
#include <vcl/edit.hxx>
#include <vcl/button.hxx>
#include <vcl/msgbox.hxx>
#include <sot/exchange.hxx>
#include <rtl/strbuf.hxx>
#include <rtl/ustring.hxx>

#include <sfx2/lnkbase.hxx>
#include <sfx2/linkmgr.hxx>
#include <sfx2/sfxresid.hxx>

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>

#include <svl/svdde.hxx>
#include <sot/formats.hxx>

#include <unotools/securityoptions.hxx>

#define DDELINK_ERROR_APP   1
#define DDELINK_ERROR_DATA  2

using namespace ::com::sun::star::uno;

namespace sfx2
{

class SvDDELinkEditDialog : public ModalDialog
{
    Edit            *m_pEdDdeApp;
    Edit            *m_pEdDdeTopic;
    Edit            *m_pEdDdeItem;
    OKButton        *m_pOKButton;

    DECL_STATIC_LINK( SvDDELinkEditDialog, EditHdl_Impl, Edit* );
public:
    SvDDELinkEditDialog( vcl::Window* pParent, SvBaseLink* );
    OUString GetCmd() const;
};

SvDDELinkEditDialog::SvDDELinkEditDialog( vcl::Window* pParent, SvBaseLink* pLink )
    : ModalDialog( pParent, "LinkEditDialog", "sfx/ui/linkeditdialog.ui" )
{
    get(m_pOKButton, "ok");
    get(m_pEdDdeApp, "app");
    get(m_pEdDdeTopic, "file");
    get(m_pEdDdeItem, "category");

    OUString sServer, sTopic, sItem;
    pLink->GetLinkManager()->GetDisplayNames( pLink, &sServer, &sTopic, &sItem );

    m_pEdDdeApp->SetText( sServer );
    m_pEdDdeTopic->SetText( sTopic );
    m_pEdDdeItem->SetText( sItem );

    m_pEdDdeApp->SetModifyHdl( STATIC_LINK( this, SvDDELinkEditDialog, EditHdl_Impl));
    m_pEdDdeTopic->SetModifyHdl( STATIC_LINK( this, SvDDELinkEditDialog, EditHdl_Impl));
    m_pEdDdeItem->SetModifyHdl( STATIC_LINK( this, SvDDELinkEditDialog, EditHdl_Impl));

    m_pOKButton->Enable( !sServer.isEmpty() && !sTopic.isEmpty() && !sItem.isEmpty() );
}

OUString SvDDELinkEditDialog::GetCmd() const
{
    OUString sCmd( m_pEdDdeApp->GetText() ), sRet;
    ::sfx2::MakeLnkName( sRet, &sCmd, m_pEdDdeTopic->GetText(), m_pEdDdeItem->GetText() );
    return sRet;
}

IMPL_STATIC_LINK( SvDDELinkEditDialog, EditHdl_Impl, Edit *, pEdit )
{
    (void)pEdit; // unused variable
    pThis->m_pOKButton->Enable( !pThis->m_pEdDdeApp->GetText().isEmpty() &&
                              !pThis->m_pEdDdeTopic->GetText().isEmpty() &&
                              !pThis->m_pEdDdeItem->GetText().isEmpty() );
    return 0;
}

SvDDEObject::SvDDEObject()
    : pConnection( 0 ), pLink( 0 ), pRequest( 0 ), pGetData( 0 ), nError( 0 )
{
    SetUpdateTimeout( 100 );
    bWaitForData = sal_False;
}

SvDDEObject::~SvDDEObject()
{
    delete pLink;
    delete pRequest;
    delete pConnection;
}

bool SvDDEObject::GetData( ::com::sun::star::uno::Any & rData /*out param*/,
                            const OUString & rMimeType,
                            bool bSynchron )
{
    if( !pConnection )
        return false;

    if( pConnection->GetError() )  // then we try once more
    {
        OUString sServer( pConnection->GetServiceName() );
        OUString sTopic( pConnection->GetTopicName() );

        delete pConnection;
        pConnection = new DdeConnection( sServer, sTopic );
        if( pConnection->GetError() )
            nError = DDELINK_ERROR_APP;
    }

    if( bWaitForData ) // we are in an rekursive loop, get out again
        return false;

    // Lock against Reentrance
    bWaitForData = sal_True;

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

        bWaitForData = sal_False;
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

        OUString aEmptyStr;
        rData <<= aEmptyStr;
    }
    return 0 == pConnection->GetError();
}


bool SvDDEObject::Connect( SvBaseLink * pSvLink )
{
    sal_uInt16 nLinkType = pSvLink->GetUpdateMode();
    if( pConnection )           // Connection is already made
    {
        // well, then just add it as dependent
        AddDataAdvise( pSvLink,
                SotExchange::GetFormatMimeType( pSvLink->GetContentType()),
                LINKUPDATE_ONCALL == nLinkType
                        ? ADVISEMODE_ONLYONCE
                        : 0 );
        AddConnectAdvise( pSvLink );

        return true;
    }

    if( !pSvLink->GetLinkManager() )
        return false;

    OUString sServer, sTopic;
    pSvLink->GetLinkManager()->GetDisplayNames( pSvLink, &sServer, &sTopic, &sItem );

    if( sServer.isEmpty() || sTopic.isEmpty() || sItem.isEmpty() )
        return false;

    pConnection = new DdeConnection( sServer, sTopic );
    if( pConnection->GetError() )
    {
        // check if the DDE server knows the "SYSTEM" topic
        bool bSysTopic = false;
        if (!sTopic.equalsIgnoreAsciiCase("SYSTEM"))
        {
            DdeConnection aTmp(sServer, OUString("SYSTEM"));
            bSysTopic = !aTmp.GetError();
        }

        if( bSysTopic )
        {
            // if the system topic works then the server is up but just doesn't know the original topic
            nError = DDELINK_ERROR_DATA;
            return false;
        }

        nError = DDELINK_ERROR_APP;
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
        return false;

    AddDataAdvise( pSvLink,
                SotExchange::GetFormatMimeType( pSvLink->GetContentType()),
                LINKUPDATE_ONCALL == nLinkType
                        ? ADVISEMODE_ONLYONCE
                        : 0 );
    AddConnectAdvise( pSvLink );
    SetUpdateTimeout( 0 );
    return true;
}

void SvDDEObject::Edit( vcl::Window* pParent, sfx2::SvBaseLink* pBaseLink, const Link& rEndEditHdl )
{
    SvDDELinkEditDialog aDlg( pParent, pBaseLink );
    if ( RET_OK == aDlg.Execute() && rEndEditHdl.IsSet() )
    {
        OUString sCommand = aDlg.GetCmd();
        rEndEditHdl.Call( &sCommand );
    }
}

bool SvDDEObject::ImplHasOtherFormat( DdeTransaction& rReq )
{
    sal_uInt16 nFmt = 0;
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

bool SvDDEObject::IsPending() const
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

bool SvDDEObject::IsDataComplete() const
{
    return bWaitForData;
}

IMPL_LINK( SvDDEObject, ImplGetDDEData, DdeData*, pData )
{
    sal_uIntPtr nFmt = pData->GetFormat();
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

            Sequence< sal_Int8 > aSeq( reinterpret_cast<const sal_Int8*>(p), nLen );
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
                bWaitForData = sal_False;
            }
        }
    }

    return 0;
}

IMPL_LINK( SvDDEObject, ImplDoneDDEData, void*, pData )
{
    bool bValid = (bool)reinterpret_cast<sal_uIntPtr>(pData);
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
                bWaitForData = sal_False;
            }
        }
    }
    else
        // End waiting
        bWaitForData = sal_False;

    return 0;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

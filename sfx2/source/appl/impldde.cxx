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
    VclPtr<Edit>            m_pEdDdeApp;
    VclPtr<Edit>            m_pEdDdeTopic;
    VclPtr<Edit>            m_pEdDdeItem;
    VclPtr<OKButton>        m_pOKButton;

    DECL_LINK_TYPED( EditHdl_Impl, Edit&, void );
public:
    SvDDELinkEditDialog( vcl::Window* pParent, SvBaseLink* );
    virtual ~SvDDELinkEditDialog();
    virtual void dispose() override;
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
    sfx2::LinkManager::GetDisplayNames( pLink, &sServer, &sTopic, &sItem );

    m_pEdDdeApp->SetText( sServer );
    m_pEdDdeTopic->SetText( sTopic );
    m_pEdDdeItem->SetText( sItem );

    m_pEdDdeApp->SetModifyHdl( LINK( this, SvDDELinkEditDialog, EditHdl_Impl));
    m_pEdDdeTopic->SetModifyHdl( LINK( this, SvDDELinkEditDialog, EditHdl_Impl));
    m_pEdDdeItem->SetModifyHdl( LINK( this, SvDDELinkEditDialog, EditHdl_Impl));

    m_pOKButton->Enable( !sServer.isEmpty() && !sTopic.isEmpty() && !sItem.isEmpty() );
}

SvDDELinkEditDialog::~SvDDELinkEditDialog()
{
    disposeOnce();
}

void SvDDELinkEditDialog::dispose()
{
    m_pEdDdeApp.clear();
    m_pEdDdeTopic.clear();
    m_pEdDdeItem.clear();
    m_pOKButton.clear();
    ModalDialog::dispose();
}

OUString SvDDELinkEditDialog::GetCmd() const
{
    OUString sCmd( m_pEdDdeApp->GetText() ), sRet;
    ::sfx2::MakeLnkName( sRet, &sCmd, m_pEdDdeTopic->GetText(), m_pEdDdeItem->GetText() );
    return sRet;
}

IMPL_LINK_NOARG_TYPED( SvDDELinkEditDialog, EditHdl_Impl, Edit&, void)
{
    m_pOKButton->Enable( !m_pEdDdeApp->GetText().isEmpty() &&
                         !m_pEdDdeTopic->GetText().isEmpty() &&
                         !m_pEdDdeItem->GetText().isEmpty() );
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

bool SvDDEObject::GetData( css::uno::Any & rData /*out param*/,
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
            delete pRequest;

            pRequest = new DdeRequest( *pConnection, sItem );
            pRequest->SetDataHdl( LINK( this, SvDDEObject, ImplGetDDEData ) );
            pRequest->SetDoneHdl( LINK( this, SvDDEObject, ImplDoneDDEData ) );
            pRequest->SetFormat( SotExchange::GetFormatIdFromMimeType(
                                    rMimeType ) );
            pRequest->Execute();
        }

        rData <<= OUString();
    }
    return 0 == pConnection->GetError();
}


bool SvDDEObject::Connect( SvBaseLink * pSvLink )
{
    SfxLinkUpdateMode nLinkType = pSvLink->GetUpdateMode();
    if( pConnection )           // Connection is already made
    {
        // well, then just add it as dependent
        AddDataAdvise( pSvLink,
                SotExchange::GetFormatMimeType( pSvLink->GetContentType()),
                SfxLinkUpdateMode::ONCALL == nLinkType
                        ? ADVISEMODE_ONLYONCE
                        : 0 );
        AddConnectAdvise( pSvLink );

        return true;
    }

    if( !pSvLink->GetLinkManager() )
        return false;

    OUString sServer, sTopic;
    sfx2::LinkManager::GetDisplayNames( pSvLink, &sServer, &sTopic, &sItem );

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

    if( SfxLinkUpdateMode::ALWAYS == nLinkType && !pLink && !pConnection->GetError() )
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
                SfxLinkUpdateMode::ONCALL == nLinkType
                        ? ADVISEMODE_ONLYONCE
                        : 0 );
    AddConnectAdvise( pSvLink );
    SetUpdateTimeout( 0 );
    return true;
}

void SvDDEObject::Edit( vcl::Window* pParent, sfx2::SvBaseLink* pBaseLink, const Link<const OUString&, void>& rEndEditHdl )
{
    ScopedVclPtrInstance< SvDDELinkEditDialog > aDlg(pParent, pBaseLink);
    if ( RET_OK == aDlg->Execute() && rEndEditHdl.IsSet() )
    {
        OUString sCommand = aDlg->GetCmd();
        rEndEditHdl.Call( sCommand );
    }
}

bool SvDDEObject::ImplHasOtherFormat( DdeTransaction& rReq )
{
    SotClipboardFormatId nFmt = SotClipboardFormatId::NONE;
    switch( rReq.GetFormat() )
    {
    case SotClipboardFormatId::RTF:
        nFmt = SotClipboardFormatId::STRING;
        break;

    case SotClipboardFormatId::HTML_SIMPLE:
    case SotClipboardFormatId::HTML:
        nFmt = SotClipboardFormatId::RTF;
        break;

    case SotClipboardFormatId::GDIMETAFILE:
        nFmt = SotClipboardFormatId::BITMAP;
        break;

    case SotClipboardFormatId::SVXB:
        nFmt = SotClipboardFormatId::GDIMETAFILE;
        break;

    // something else?
    default: break;
    }
    if( nFmt != SotClipboardFormatId::NONE )
        rReq.SetFormat( nFmt );         // try it once more
    return SotClipboardFormatId::NONE != nFmt;
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

IMPL_LINK_TYPED( SvDDEObject, ImplGetDDEData, const DdeData*, pData, void )
{
    SotClipboardFormatId nFmt = pData->GetFormat();
    switch( nFmt )
    {
    case SotClipboardFormatId::GDIMETAFILE:
        break;

    case SotClipboardFormatId::BITMAP:
        break;

    default:
        {
            const sal_Char* p = static_cast<sal_Char const *>(pData->operator const void*());
            long nLen = SotClipboardFormatId::STRING == nFmt ? (p ? strlen( p ) : 0) : (long)*pData;

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
}

IMPL_LINK_TYPED( SvDDEObject, ImplDoneDDEData, bool, bValid, void )
{
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
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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


#if defined(_WIN32)
#include <prewin.h>
#include <postwin.h>
#endif

#include "impldde.hxx"

#include <vcl/weld.hxx>
#include <sot/exchange.hxx>
#include <rtl/ustring.hxx>

#include <sfx2/lnkbase.hxx>
#include <sfx2/linkmgr.hxx>

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>

#include <svl/svdde.hxx>
#include <sot/formats.hxx>

using namespace ::com::sun::star::uno;

namespace sfx2
{

namespace {

class SvDDELinkEditDialog : public weld::GenericDialogController
{
    std::unique_ptr<weld::Entry> m_xEdDdeApp;
    std::unique_ptr<weld::Entry> m_xEdDdeTopic;
    std::unique_ptr<weld::Entry> m_xEdDdeItem;
    std::unique_ptr<weld::Button> m_xOKButton;

    DECL_LINK(EditHdl_Impl, weld::Entry&, void);
public:
    SvDDELinkEditDialog(weld::Window* pParent, SvBaseLink const*);
    OUString GetCmd() const;
};

}

SvDDELinkEditDialog::SvDDELinkEditDialog(weld::Window* pParent, SvBaseLink const * pLink)
    : GenericDialogController(pParent, u"sfx/ui/linkeditdialog.ui"_ustr, u"LinkEditDialog"_ustr)
    , m_xEdDdeApp(m_xBuilder->weld_entry(u"app"_ustr))
    , m_xEdDdeTopic(m_xBuilder->weld_entry(u"file"_ustr))
    , m_xEdDdeItem(m_xBuilder->weld_entry(u"category"_ustr))
    , m_xOKButton(m_xBuilder->weld_button(u"ok"_ustr))
{
    OUString sServer, sTopic, sItem;
    sfx2::LinkManager::GetDisplayNames( pLink, &sServer, &sTopic, &sItem );

    m_xEdDdeApp->set_text( sServer );
    m_xEdDdeTopic->set_text( sTopic );
    m_xEdDdeItem->set_text( sItem );

    m_xEdDdeApp->connect_changed( LINK( this, SvDDELinkEditDialog, EditHdl_Impl));
    m_xEdDdeTopic->connect_changed( LINK( this, SvDDELinkEditDialog, EditHdl_Impl));
    m_xEdDdeItem->connect_changed( LINK( this, SvDDELinkEditDialog, EditHdl_Impl));

    m_xOKButton->set_sensitive(!sServer.isEmpty() && !sTopic.isEmpty() && !sItem.isEmpty());
}

OUString SvDDELinkEditDialog::GetCmd() const
{
    OUString sCmd( m_xEdDdeApp->get_text() ), sRet;
    ::sfx2::MakeLnkName( sRet, &sCmd, m_xEdDdeTopic->get_text(), m_xEdDdeItem->get_text() );
    return sRet;
}

IMPL_LINK_NOARG( SvDDELinkEditDialog, EditHdl_Impl, weld::Entry&, void)
{
    m_xOKButton->set_sensitive(!m_xEdDdeApp->get_text().isEmpty() &&
                               !m_xEdDdeTopic->get_text().isEmpty() &&
                               !m_xEdDdeItem->get_text().isEmpty() );
}

SvDDEObject::SvDDEObject()
    :  pGetData( nullptr )
{
    SetUpdateTimeout( 100 );
    bWaitForData = false;
}

SvDDEObject::~SvDDEObject()
{
    pLink.reset();
    pRequest.reset();
    pConnection.reset();
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

        pConnection.reset( new DdeConnection( sServer, sTopic ) );
    }

    if( bWaitForData ) // we are in a recursive loop, get out again
        return false;

    // Lock against Reentrance
    bWaitForData = true;

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

        bWaitForData = false;
    }
    else
    {
        // otherwise it will be executed asynchronously
        {
            pRequest.reset( new DdeRequest( *pConnection, sItem ) );
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

    pConnection.reset( new DdeConnection( sServer, sTopic ) );
    if( pConnection->GetError() )
    {
        // check if the DDE server knows the "SYSTEM" topic
        bool bSysTopic = false;
        if (!sTopic.equalsIgnoreAsciiCase("SYSTEM"))
        {
            DdeConnection aTmp(sServer, u"SYSTEM"_ustr);
            bSysTopic = !aTmp.GetError();
        }

        if( bSysTopic )
        {
            // if the system topic works then the server is up but just doesn't know the original topic
            return false;
        }
    }

    if( SfxLinkUpdateMode::ALWAYS == nLinkType && !pLink && !pConnection->GetError() )
    {
        // Setting up Hot Link, Data will be available at some point later on
        pLink.reset( new DdeHotLink( *pConnection, sItem ) );
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

void SvDDEObject::Edit(weld::Window* pParent, sfx2::SvBaseLink* pBaseLink, const Link<const OUString&, void>& rEndEditHdl)
{
    SvDDELinkEditDialog aDlg(pParent, pBaseLink);
    if (RET_OK == aDlg.run() && rEndEditHdl.IsSet())
    {
        OUString sCommand = aDlg.GetCmd();
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
/*
    The method determines whether the data-object can be read from a DDE.
*/
{
    return bWaitForData;
}

bool SvDDEObject::IsDataComplete() const
{
    return bWaitForData;
}

IMPL_LINK( SvDDEObject, ImplGetDDEData, const DdeData*, pData, void )
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
            const char* p = static_cast<char const *>(pData->getData());
            tools::Long nLen = SotClipboardFormatId::STRING == nFmt ? (p ? strlen( p ) : 0) : pData->getSize();

            Sequence< sal_Int8 > aSeq( reinterpret_cast<const sal_Int8*>(p), nLen );
            if( pGetData )
            {
                *pGetData <<= aSeq;  // Copy Data
                pGetData = nullptr;        // reset the pointer here
            }
            else
            {
                Any aVal;
                aVal <<= aSeq;
                DataChanged( SotExchange::GetFormatMimeType(
                                                pData->GetFormat() ), aVal );
                bWaitForData = false;
            }
        }
    }
}

IMPL_LINK( SvDDEObject, ImplDoneDDEData, bool, bValid, void )
{
    if( !bValid && ( pRequest || pLink ))
    {
        DdeTransaction* pReq = nullptr;
        if( !pLink || ( pLink && pLink->IsBusy() ))
            pReq = pRequest.get();  // only the one that is ready
        else if( pRequest && pRequest->IsBusy() )
            pReq = pLink.get();  // only the one that is ready

        if( pReq )
        {
            if( ImplHasOtherFormat( *pReq ) )
            {
                pReq->Execute();
            }
            else if( pReq == pRequest.get() )
            {
                bWaitForData = false;
            }
        }
    }
    else
        // End waiting
        bWaitForData = false;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

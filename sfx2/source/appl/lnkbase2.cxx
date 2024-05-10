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


#include <memory>
#include <sfx2/lnkbase.hxx>
#include <sot/exchange.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <sfx2/linkmgr.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <sfx2/strings.hrc>
#include <sfx2/sfxresid.hxx>
#include <sfx2/filedlghelper.hxx>
#include <tools/debug.hxx>
#include <svl/svdde.hxx>
#include <osl/diagnose.h>
#include <officecfg/Office/Common.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace sfx2
{

namespace {
class ImplDdeItem;
}

// only for internal management
struct ImplBaseLinkData
{
    struct tClientType
    {
        // applies for all links
        SotClipboardFormatId nCntntType; // Update Format
        // Not Ole-Links
        bool                 bIntrnlLnk;  // It is an internal link
        SfxLinkUpdateMode    nUpdateMode; // UpdateMode
    };

    struct tDDEType
    {
        ImplDdeItem* pItem;
    };

    union {
        tClientType ClientType;
        tDDEType DDEType;
    };
    ImplBaseLinkData()
    {
        ClientType.nCntntType = SotClipboardFormatId::NONE;
        ClientType.bIntrnlLnk = false;
        ClientType.nUpdateMode = SfxLinkUpdateMode::NONE;
        DDEType.pItem = nullptr;
    }
};

namespace {

class ImplDdeItem : public DdeGetPutItem
{
    SvBaseLink* pLink;
    DdeData aData;
    Sequence< sal_Int8 > aSeq;  // Datacontainer for DdeData !!!
    bool bIsValidData : 1;
    bool bIsInDTOR : 1;
public:
#if defined(_WIN32)
    ImplDdeItem( SvBaseLink& rLink, const OUString& rStr )
        : DdeGetPutItem( rStr ), pLink( &rLink ), bIsValidData( false ),
        bIsInDTOR( false )
    {}
#endif
    virtual ~ImplDdeItem() override;

    virtual DdeData* Get( SotClipboardFormatId ) override;
    virtual bool     Put( const DdeData* ) override;
    virtual void     AdviseLoop( bool ) override;

    void Notify()
    {
        bIsValidData = false;
        DdeGetPutItem::NotifyClient();
    }

    bool IsInDTOR() const { return bIsInDTOR; }
};

}

SvBaseLink::SvBaseLink()
    : m_pLinkMgr( nullptr )
    , m_pParentWin( nullptr )
    , m_bIsConnect( false )
    , m_bIsReadOnly(false)
{
    mnObjType = SvBaseLinkObjectType::ClientSo;
    pImplData.reset( new ImplBaseLinkData );
    bVisible = bSynchron = true;
    bWasLastEditOK = false;
}


SvBaseLink::SvBaseLink( SfxLinkUpdateMode nUpdateMode, SotClipboardFormatId nContentType )
    : m_pLinkMgr( nullptr )
    , m_pParentWin( nullptr )
    , m_bIsConnect( false )
    , m_bIsReadOnly(false)
{
    mnObjType = SvBaseLinkObjectType::ClientSo;
    pImplData.reset( new ImplBaseLinkData );
    bVisible = bSynchron = true;
    bWasLastEditOK = false;

    // It is going to be an OLE-Link,
    pImplData->ClientType.nUpdateMode = nUpdateMode;
    pImplData->ClientType.nCntntType = nContentType;
    pImplData->ClientType.bIntrnlLnk = false;
}

#if defined(_WIN32)

static DdeTopic* FindTopic( const OUString & rLinkName, sal_uInt16* pItemStt )
{
    if( rLinkName.isEmpty() )
        return nullptr;

    OUString sNm( rLinkName );
    sal_Int32 nTokenPos = 0;
    OUString sService( sNm.getToken( 0, cTokenSeparator, nTokenPos ) );

    DdeServices& rSvc = DdeService::GetServices();
    for (auto const& elem : rSvc)
    {
        if(elem->GetName() == sService)
        {
            // then we search for the Topic
            OUString sTopic( sNm.getToken( 0, cTokenSeparator, nTokenPos ) );
            if( pItemStt )
                *pItemStt = nTokenPos;

            std::vector<DdeTopic*>& rTopics = elem->GetTopics();

            for (auto const& topic : rTopics)
                if( topic->GetName() == sTopic )
                    return topic;
            break;
        }
    }
    return nullptr;
}

SvBaseLink::SvBaseLink( const OUString& rLinkName, SvBaseLinkObjectType nObjectType, SvLinkSource* pObj )
    : m_pLinkMgr( nullptr )
    , m_pParentWin( nullptr )
    , m_bIsConnect( false )
    , m_bIsReadOnly(false)
{
    bVisible = bSynchron = true;
    bWasLastEditOK = false;
    aLinkName = rLinkName;
    pImplData.reset( new ImplBaseLinkData );
    mnObjType = nObjectType;

    if( !pObj )
    {
        DBG_ASSERT( pObj, "Where is my left-most object" );
        return;
    }

    if( SvBaseLinkObjectType::DdeExternal == mnObjType )
    {
        sal_uInt16 nItemStt = 0;
        DdeTopic* pTopic = FindTopic( aLinkName, &nItemStt );
        if( pTopic )
        {
            // then we have it all together
            // MM_TODO how do I get the name
            OUString aStr = aLinkName; // xLinkName->GetDisplayName();
            aStr = aStr.copy( nItemStt );
            pImplData->DDEType.pItem = new ImplDdeItem( *this, aStr );
            pTopic->InsertItem( pImplData->DDEType.pItem );

            // store the Advice
            xObj = pObj;
        }
    }
    else if( pObj->Connect( this ) )
        xObj = pObj;
}

#endif

SvBaseLink::~SvBaseLink()
{
    Disconnect();

    if( mnObjType == SvBaseLinkObjectType::DdeExternal )
    {
        if( !pImplData->DDEType.pItem->IsInDTOR() )
            delete pImplData->DDEType.pItem;
    }

    pImplData.reset();
}

IMPL_LINK( SvBaseLink, EndEditHdl, const OUString&, _rNewName, void )
{
    OUString sNewName = _rNewName;
    if ( !ExecuteEdit( sNewName ) )
        sNewName.clear();
    bWasLastEditOK = !sNewName.isEmpty();
    m_aEndEditLink.Call( *this );
}


void SvBaseLink::SetObjType( SvBaseLinkObjectType mnObjTypeP )
{
    DBG_ASSERT( mnObjType != SvBaseLinkObjectType::ClientDde, "type already set" );
    DBG_ASSERT( !xObj.is(), "object exist" );

    mnObjType = mnObjTypeP;
}


void SvBaseLink::SetName( const OUString & rNm )
{
    aLinkName = rNm;
}


void SvBaseLink::SetObj( SvLinkSource * pObj )
{
    DBG_ASSERT( (isClientType(mnObjType) &&
                 pImplData->ClientType.bIntrnlLnk) ||
                mnObjType == SvBaseLinkObjectType::ClientGraphic,
                "no intern link" );
    xObj = pObj;
}


void SvBaseLink::SetLinkSourceName( const OUString & rLnkNm )
{
    if( aLinkName == rLnkNm )
        return;

    AddNextRef(); // should be superfluous
    // remove old connection
    Disconnect();

    aLinkName = rLnkNm;

    // New Connection
    GetRealObject_();
    ReleaseRef(); // should be superfluous
}


void SvBaseLink::SetUpdateMode( SfxLinkUpdateMode nMode )
{
    if( isClientType(mnObjType) &&
        pImplData->ClientType.nUpdateMode != nMode )
    {
        AddNextRef();
        Disconnect();

        pImplData->ClientType.nUpdateMode = nMode;
        GetRealObject_();
        ReleaseRef();
    }
}

// #i88291#
void SvBaseLink::clearStreamToLoadFrom()
{
    m_xInputStreamToLoadFrom.clear();
    if( xObj.is() )
    {
        xObj->clearStreamToLoadFrom();
    }
}

bool SvBaseLink::Update()
{
    if(officecfg::Office::Common::Security::Scripting::DisableActiveContent::get())
        return false;

    if( isClientType(mnObjType) )
    {
        AddNextRef();
        Disconnect();

        GetRealObject_();
        ReleaseRef();
        if( xObj.is() )
        {
            xObj->setStreamToLoadFrom(m_xInputStreamToLoadFrom,m_bIsReadOnly);
            OUString sMimeType( SotExchange::GetFormatMimeType(
                            pImplData->ClientType.nCntntType ));
            Any aData;

            if( xObj->GetData( aData, sMimeType ) )
            {
                UpdateResult eRes = DataChanged(sMimeType, aData);
                bool bSuccess = eRes == SUCCESS;
                //for manual Updates there is no need to hold the ServerObject
                if( SvBaseLinkObjectType::ClientDde == mnObjType &&
                    SfxLinkUpdateMode::ONCALL == GetUpdateMode() && xObj.is() )
                    xObj->RemoveAllDataAdvise( this );
                return bSuccess;
            }
            if( xObj.is() )
            {
                // should be asynchronous?
                if( xObj->IsPending() )
                    return true;

                // we do not need the object anymore
                AddNextRef();
                Disconnect();
                ReleaseRef();
            }
        }
    }
    return false;
}


SfxLinkUpdateMode SvBaseLink::GetUpdateMode() const
{
    return isClientType(mnObjType)
            ? pImplData->ClientType.nUpdateMode
            : SfxLinkUpdateMode::ONCALL;
}


void SvBaseLink::GetRealObject_( bool bConnect)
{
    if( !m_pLinkMgr )
        return;

    DBG_ASSERT( !xObj.is(), "object already exist" );

    if( SvBaseLinkObjectType::ClientDde == mnObjType )
    {
        OUString sServer;
        if( sfx2::LinkManager::GetDisplayNames( this, &sServer ) &&
            sServer == Application::GetAppName() )  // internal Link !!!
        {
            // so that the Internal link can be created!
            mnObjType = SvBaseLinkObjectType::Internal;
            xObj = sfx2::LinkManager::CreateObj( this );

            pImplData->ClientType.bIntrnlLnk = true;
            mnObjType = SvBaseLinkObjectType::ClientDde;  // so we know what it once was!
        }
        else
        {
            pImplData->ClientType.bIntrnlLnk = false;
            xObj = sfx2::LinkManager::CreateObj( this );
        }
    }
    else if( isClientType(mnObjType) )
        xObj = sfx2::LinkManager::CreateObj( this );

    if( bConnect && ( !xObj.is() || !xObj->Connect( this ) ) )
        Disconnect();
}

SotClipboardFormatId SvBaseLink::GetContentType() const
{
    if( isClientType(mnObjType) )
        return pImplData->ClientType.nCntntType;

    return SotClipboardFormatId::NONE;  // all Formats ?
}


void SvBaseLink::SetContentType( SotClipboardFormatId nType )
{
    if( isClientType(mnObjType) )
    {
        pImplData->ClientType.nCntntType = nType;
    }
}

LinkManager* SvBaseLink::GetLinkManager()
{
    return m_pLinkMgr;
}

const LinkManager* SvBaseLink::GetLinkManager() const
{
    return m_pLinkMgr;
}

void SvBaseLink::SetLinkManager( LinkManager* _pMgr )
{
    m_pLinkMgr = _pMgr;
}

void SvBaseLink::Disconnect()
{
    if( xObj.is() )
    {
        xObj->RemoveAllDataAdvise( this );
        xObj->RemoveConnectAdvise( this );
        xObj.clear();
    }
}

SvBaseLink::UpdateResult SvBaseLink::DataChanged( const OUString &, const css::uno::Any & )
{
    if ( mnObjType == SvBaseLinkObjectType::DdeExternal )
    {
        if( pImplData->DDEType.pItem )
            pImplData->DDEType.pItem->Notify();
    }
    return SUCCESS;
}

void SvBaseLink::Edit(weld::Window* pParent, const Link<SvBaseLink&,void>& rEndEditHdl )
{
    m_pParentWin = pParent;
    m_aEndEditLink = rEndEditHdl;
    m_bIsConnect = xObj.is();
    if( !m_bIsConnect )
        GetRealObject_( xObj.is() );

    bool bAsync = false;
    Link<const OUString&, void> aLink = LINK( this, SvBaseLink, EndEditHdl );

    if( isClientType(mnObjType) && pImplData->ClientType.bIntrnlLnk )
    {
        if( m_pLinkMgr )
        {
            SvLinkSourceRef ref = sfx2::LinkManager::CreateObj( this );
            if( ref.is() )
            {
                ref->Edit( pParent, this, aLink );
                bAsync = true;
            }
        }
    }
    else
    {
        xObj->Edit( pParent, this, aLink );
        bAsync = true;
    }

    if ( !bAsync )
    {
        ExecuteEdit( OUString() );
        bWasLastEditOK = false;
        m_aEndEditLink.Call( *this );
    }
}

bool SvBaseLink::ExecuteEdit( const OUString& _rNewName )
{
    if( !_rNewName.isEmpty() )
    {
        SetLinkSourceName( _rNewName );
        if( !Update() )
        {
            OUString sApp, sTopic, sItem, sError;
            sfx2::LinkManager::GetDisplayNames( this, &sApp, &sTopic, &sItem );
            if( mnObjType == SvBaseLinkObjectType::ClientDde )
            {
                sError = SfxResId(STR_DDE_ERROR);

                sal_Int32 nFndPos = sError.indexOf( "%1" );
                if( -1 != nFndPos )
                {
                    sError = sError.replaceAt( nFndPos, 2, sApp );
                    nFndPos = nFndPos + sApp.getLength();

                    if( -1 != ( nFndPos = sError.indexOf( "%2", nFndPos )))
                    {
                        sError = sError.replaceAt( nFndPos, 2, sTopic );
                        nFndPos = nFndPos + sTopic.getLength();

                        if( -1 != ( nFndPos = sError.indexOf( "%3", nFndPos )))
                            sError = sError.replaceAt( nFndPos, 2, sItem );
                    }
                }
            }
            else
                return false;

            std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(m_pParentWin,
                                                                     VclMessageType::Warning, VclButtonsType::Ok, sError));
            xBox->run();
        }
    }
    else if( !m_bIsConnect )
        Disconnect();
    m_bIsConnect = false;
    return true;
}

void SvBaseLink::Closed()
{
    if( xObj.is() )
        xObj->RemoveAllDataAdvise( this );
}

FileDialogHelper & SvBaseLink::GetInsertFileDialog(const OUString& rFactory)
{
    m_pFileDlg.reset( new FileDialogHelper(
            ui::dialogs::TemplateDescription::FILEOPEN_SIMPLE,
            FileDialogFlags::Insert, rFactory, SfxFilterFlags::NONE, SfxFilterFlags::NONE, m_pParentWin) );
    return *m_pFileDlg;
}

ImplDdeItem::~ImplDdeItem()
{
    bIsInDTOR = true;
    // So that no-one gets the idea to delete the pointer when Disconnecting!
    tools::SvRef<SvBaseLink> aRef( pLink );
    aRef->Disconnect();
}

DdeData* ImplDdeItem::Get( SotClipboardFormatId nFormat )
{
    if( pLink->GetObj() )
    {
        // is it still valid?
        if( bIsValidData && nFormat == aData.GetFormat() )
            return &aData;

        Any aValue;
        OUString sMimeType( SotExchange::GetFormatMimeType( nFormat ));
        if( pLink->GetObj()->GetData( aValue, sMimeType ) )
        {
            if( aValue >>= aSeq )
            {
                aData = DdeData( aSeq.getConstArray(), aSeq.getLength(), nFormat );

                bIsValidData = true;
                return &aData;
            }
        }
    }
    aSeq.realloc( 0 );
    bIsValidData = false;
    return nullptr;
}


bool ImplDdeItem::Put( const DdeData*  )
{
    OSL_FAIL( "ImplDdeItem::Put not implemented" );
    return false;
}


void ImplDdeItem::AdviseLoop( bool bOpen )
{
    // Connection is closed, so also unsubscribe link
    if( !pLink->GetObj() )
        return;

    if( bOpen )
    {
        // A connection is re-established
        if( SvBaseLinkObjectType::DdeExternal == pLink->GetObjType() )
        {
            pLink->GetObj()->AddDataAdvise( pLink, u"text/plain;charset=utf-16"_ustr,  ADVISEMODE_NODATA );
            pLink->GetObj()->AddConnectAdvise( pLink );
        }
    }
    else
    {
        // So that no-one gets the idea to delete the pointer
        // when Disconnecting!
        tools::SvRef<SvBaseLink> aRef( pLink );
        aRef->Disconnect();
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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

#include <sfx2/lnkbase.hxx>
#include <sot/exchange.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <vcl/msgbox.hxx>
#include <sfx2/linkmgr.hxx>
#include <vcl/svapp.hxx>
#include "app.hrc"
#include "sfxresid.hxx"
#include <sfx2/filedlghelper.hxx>
#include <tools/debug.hxx>
#include <svl/svdde.hxx>

using namespace ::com::sun::star::uno;

namespace sfx2
{

TYPEINIT0( SvBaseLink )

static DdeTopic* FindTopic( const String &, USHORT* = 0 );

class  ImplDdeItem;

struct BaseLink_Impl
{
    Link                m_aEndEditLink;
    LinkManager*      m_pLinkMgr;
    Window*             m_pParentWin;
    FileDialogHelper*   m_pFileDlg;
    bool                m_bIsConnect;

    BaseLink_Impl() :
          m_pLinkMgr( NULL )
        , m_pParentWin( NULL )
        , m_pFileDlg( NULL )
        , m_bIsConnect( false )
        {}

    ~BaseLink_Impl()
        { delete m_pFileDlg; }
};

// only for internal management
struct ImplBaseLinkData
{
    struct tClientType
    {
        // applies for all links
        ULONG   nCntntType;  // Update Format
        // Not Ole-Links
        BOOL    bIntrnlLnk;  // It is an internal link
        USHORT  nUpdateMode; // UpdateMode
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
        ClientType.nCntntType = 0;
        ClientType.bIntrnlLnk = FALSE;
        ClientType.nUpdateMode = 0;
        DDEType.pItem = NULL;
    }
};


class ImplDdeItem : public DdeGetPutItem
{
    SvBaseLink* pLink;
    DdeData aData;
    Sequence< sal_Int8 > aSeq;  // Datacontainer for DdeData !!!
    BOOL bIsValidData : 1;
    BOOL bIsInDTOR : 1;
public:
    ImplDdeItem( SvBaseLink& rLink, const String& rStr )
        : DdeGetPutItem( rStr ), pLink( &rLink ), bIsValidData( FALSE ),
        bIsInDTOR( FALSE )
    {}
    virtual ~ImplDdeItem();

    virtual DdeData* Get( ULONG );
    virtual BOOL Put( const DdeData* );
    virtual void AdviseLoop( BOOL );

    void Notify()
    {
        bIsValidData = FALSE;
        DdeGetPutItem::NotifyClient();
    }

    BOOL IsInDTOR() const { return bIsInDTOR; }
};

//--------------------------------------------------------------------------

SvBaseLink::SvBaseLink()
{
    pImpl = new BaseLink_Impl();
    nObjType = OBJECT_CLIENT_SO;
    pImplData = new ImplBaseLinkData;
    bVisible = bSynchron = bUseCache = TRUE;
    bWasLastEditOK = FALSE;
}

//--------------------------------------------------------------------------

SvBaseLink::SvBaseLink( USHORT nUpdateMode, ULONG nContentType )
{
    pImpl = new BaseLink_Impl();
    nObjType = OBJECT_CLIENT_SO;
    pImplData = new ImplBaseLinkData;
    bVisible = bSynchron = bUseCache = TRUE;
    bWasLastEditOK = FALSE;

    // It it going to be a Ole-Link,
    pImplData->ClientType.nUpdateMode = nUpdateMode;
    pImplData->ClientType.nCntntType = nContentType;
    pImplData->ClientType.bIntrnlLnk = FALSE;
}

//--------------------------------------------------------------------------

SvBaseLink::SvBaseLink( const String& rLinkName, USHORT nObjectType, SvLinkSource* pObj )
{
    bVisible = bSynchron = bUseCache = TRUE;
    bWasLastEditOK = FALSE;
    aLinkName = rLinkName;
    pImplData = new ImplBaseLinkData;
    nObjType = nObjectType;

    if( !pObj )
    {
        DBG_ASSERT( pObj, "Where is my left-most object" );
        return;
    }

    if( OBJECT_DDE_EXTERN == nObjType )
    {
        USHORT nItemStt = 0;
        DdeTopic* pTopic = FindTopic( aLinkName, &nItemStt );
        if( pTopic )
        {
            // then we have it all together
            // MM_TODO how do I get the name
            String aStr = aLinkName; // xLinkName->GetDisplayName();
            aStr = aStr.Copy( nItemStt );
            pImplData->DDEType.pItem = new ImplDdeItem( *this, aStr );
            pTopic->InsertItem( pImplData->DDEType.pItem );

            // store the Advice
            xObj = pObj;
        }
    }
    else if( pObj->Connect( this ) )
        xObj = pObj;
}

//--------------------------------------------------------------------------

SvBaseLink::~SvBaseLink()
{
    Disconnect();

    switch( nObjType )
    {
    case OBJECT_DDE_EXTERN:
        if( !pImplData->DDEType.pItem->IsInDTOR() )
            delete pImplData->DDEType.pItem;
        break;
    }

    delete pImplData;
}

IMPL_LINK( SvBaseLink, EndEditHdl, String*, _pNewName )
{
    String sNewName;
    if ( _pNewName )
        sNewName = *_pNewName;
    if ( !ExecuteEdit( sNewName ) )
        sNewName.Erase();
    bWasLastEditOK = ( sNewName.Len() > 0 );
    if ( pImpl->m_aEndEditLink.IsSet() )
        pImpl->m_aEndEditLink.Call( this );
    return 0;
}

//--------------------------------------------------------------------------

void SvBaseLink::SetObjType( USHORT nObjTypeP )
{
    DBG_ASSERT( nObjType != OBJECT_CLIENT_DDE, "type already set" );
    DBG_ASSERT( !xObj.Is(), "object exist" );

    nObjType = nObjTypeP;
}

//--------------------------------------------------------------------------

void SvBaseLink::SetName( const String & rNm )
{
    aLinkName = rNm;
}

//--------------------------------------------------------------------------

String SvBaseLink::GetName() const
{
    return aLinkName;
}

//--------------------------------------------------------------------------

void SvBaseLink::SetObj( SvLinkSource * pObj )
{
    DBG_ASSERT( (nObjType & OBJECT_CLIENT_SO &&
                pImplData->ClientType.bIntrnlLnk) ||
                nObjType == OBJECT_CLIENT_GRF,
                "no intern link" );
    xObj = pObj;
}

//--------------------------------------------------------------------------

void SvBaseLink::SetLinkSourceName( const String & rLnkNm )
{
    if( aLinkName == rLnkNm )
        return;

    AddNextRef(); // should be superfluous
    // remove old connection
    Disconnect();

    aLinkName = rLnkNm;

    // New Connection
    _GetRealObject();
    ReleaseRef(); // should be superfluous
}

//--------------------------------------------------------------------------

String  SvBaseLink::GetLinkSourceName() const
{
    return aLinkName;
}

//--------------------------------------------------------------------------

void SvBaseLink::SetUpdateMode( USHORT nMode )
{
    if( ( OBJECT_CLIENT_SO & nObjType ) &&
        pImplData->ClientType.nUpdateMode != nMode )
    {
        AddNextRef();
        Disconnect();

        pImplData->ClientType.nUpdateMode = nMode;
        _GetRealObject();
        ReleaseRef();
    }
}

// #i88291#
void SvBaseLink::clearStreamToLoadFrom()
{
    m_xInputStreamToLoadFrom.clear();
    if( xObj.Is() )
    {
        xObj->clearStreamToLoadFrom();
    }
}

BOOL SvBaseLink::Update()
{
    if( OBJECT_CLIENT_SO & nObjType )
    {
        AddNextRef();
        Disconnect();

        _GetRealObject();
        ReleaseRef();
        if( xObj.Is() )
        {
            xObj->setStreamToLoadFrom(m_xInputStreamToLoadFrom,m_bIsReadOnly);
            String sMimeType( SotExchange::GetFormatMimeType(
                            pImplData->ClientType.nCntntType ));
            Any aData;

            if( xObj->GetData( aData, sMimeType ) )
            {
                DataChanged( sMimeType, aData );
                //for manual Updates there is no need to hold the ServerObject
                if( OBJECT_CLIENT_DDE == nObjType &&
                    LINKUPDATE_ONCALL == GetUpdateMode() && xObj.Is() )
                    xObj->RemoveAllDataAdvise( this );
                return TRUE;
            }
            if( xObj.Is() )
            {
                // should be asynschron?
                if( xObj->IsPending() )
                    return TRUE;

                // we do not need the object anymore
                AddNextRef();
                Disconnect();
                ReleaseRef();
            }
        }
    }
    return FALSE;
}


USHORT SvBaseLink::GetUpdateMode() const
{
    return ( OBJECT_CLIENT_SO & nObjType )
            ? pImplData->ClientType.nUpdateMode
            : sal::static_int_cast< USHORT >( LINKUPDATE_ONCALL );
}


void SvBaseLink::_GetRealObject( BOOL bConnect)
{
    if( !pImpl->m_pLinkMgr )
        return;

    DBG_ASSERT( !xObj.Is(), "object already exist" );

    if( OBJECT_CLIENT_DDE == nObjType )
    {
        String sServer;
        if( pImpl->m_pLinkMgr->GetDisplayNames( this, &sServer ) &&
            sServer == GetpApp()->GetAppName() )  // internal Link !!!
        {
            // so that the Internal link can be created!
            nObjType = OBJECT_INTERN;
            xObj = pImpl->m_pLinkMgr->CreateObj( this );

            pImplData->ClientType.bIntrnlLnk = TRUE;
            nObjType = OBJECT_CLIENT_DDE;  // so we know what it once was!
        }
        else
        {
            pImplData->ClientType.bIntrnlLnk = FALSE;
            xObj = pImpl->m_pLinkMgr->CreateObj( this );
        }
    }
    else if( (OBJECT_CLIENT_SO & nObjType) )
        xObj = pImpl->m_pLinkMgr->CreateObj( this );

    if( bConnect && ( !xObj.Is() || !xObj->Connect( this ) ) )
        Disconnect();
}

ULONG SvBaseLink::GetContentType() const
{
    if( OBJECT_CLIENT_SO & nObjType )
        return pImplData->ClientType.nCntntType;

    return 0;  // all Formats ?
}


BOOL SvBaseLink::SetContentType( ULONG nType )
{
    if( OBJECT_CLIENT_SO & nObjType )
    {
        pImplData->ClientType.nCntntType = nType;
        return TRUE;
    }
    return FALSE;
}

LinkManager* SvBaseLink::GetLinkManager()
{
    return pImpl->m_pLinkMgr;
}

const LinkManager* SvBaseLink::GetLinkManager() const
{
    return pImpl->m_pLinkMgr;
}

void SvBaseLink::SetLinkManager( LinkManager* _pMgr )
{
    pImpl->m_pLinkMgr = _pMgr;
}

void SvBaseLink::Disconnect()
{
    if( xObj.Is() )
    {
        xObj->RemoveAllDataAdvise( this );
        xObj->RemoveConnectAdvise( this );
        xObj.Clear();
    }
}

void SvBaseLink::DataChanged( const String &, const ::com::sun::star::uno::Any & )
{
    switch( nObjType )
    {
    case OBJECT_DDE_EXTERN:
        if( pImplData->DDEType.pItem )
            pImplData->DDEType.pItem->Notify();
        break;
    }
}

void SvBaseLink::Edit( Window* pParent, const Link& rEndEditHdl )
{
    pImpl->m_pParentWin = pParent;
    pImpl->m_aEndEditLink = rEndEditHdl;
    pImpl->m_bIsConnect = ( xObj.Is() != sal_False );
    if( !pImpl->m_bIsConnect )
        _GetRealObject( xObj.Is() );

    bool bAsync = false;
    Link aLink = LINK( this, SvBaseLink, EndEditHdl );

    if( OBJECT_CLIENT_SO & nObjType && pImplData->ClientType.bIntrnlLnk )
    {
        if( pImpl->m_pLinkMgr )
        {
            SvLinkSourceRef ref = pImpl->m_pLinkMgr->CreateObj( this );
            if( ref.Is() )
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
        ExecuteEdit( String() );
        bWasLastEditOK = FALSE;
        if ( pImpl->m_aEndEditLink.IsSet() )
            pImpl->m_aEndEditLink.Call( this );
    }
}

bool SvBaseLink::ExecuteEdit( const String& _rNewName )
{
    if( _rNewName.Len() != 0 )
    {
        SetLinkSourceName( _rNewName );
        if( !Update() )
        {
            String sApp, sTopic, sItem, sError;
            pImpl->m_pLinkMgr->GetDisplayNames( this, &sApp, &sTopic, &sItem );
            if( nObjType == OBJECT_CLIENT_DDE )
            {
                sError = SfxResId( STR_DDE_ERROR );

                USHORT nFndPos = sError.Search( '%' );
                if( STRING_NOTFOUND != nFndPos )
                {
                    sError.Erase( nFndPos, 1 ).Insert( sApp, nFndPos );
                    nFndPos = nFndPos + sApp.Len();
                }
                if( STRING_NOTFOUND != ( nFndPos = sError.Search( '%', nFndPos )))
                {
                    sError.Erase( nFndPos, 1 ).Insert( sTopic, nFndPos );
                    nFndPos = nFndPos + sTopic.Len();
                }
                if( STRING_NOTFOUND != ( nFndPos = sError.Search( '%', nFndPos )))
                    sError.Erase( nFndPos, 1 ).Insert( sItem, nFndPos );
            }
            else
                return false;

            ErrorBox( pImpl->m_pParentWin, WB_OK, sError ).Execute();
        }
    }
    else if( !pImpl->m_bIsConnect )
        Disconnect();
    pImpl->m_bIsConnect = false;
    return true;
}

void SvBaseLink::Closed()
{
    if( xObj.Is() )
        xObj->RemoveAllDataAdvise( this );
}

FileDialogHelper* SvBaseLink::GetFileDialog( sal_uInt32 nFlags, const String& rFactory ) const
{
    if ( pImpl->m_pFileDlg )
        delete pImpl->m_pFileDlg;
    pImpl->m_pFileDlg = new FileDialogHelper( nFlags, rFactory );
    return pImpl->m_pFileDlg;
}

ImplDdeItem::~ImplDdeItem()
{
    bIsInDTOR = TRUE;
    // So that no-one gets the idea to delete the pointer when Disconnecting!
    SvBaseLinkRef aRef( pLink );
    aRef->Disconnect();
}

DdeData* ImplDdeItem::Get( ULONG nFormat )
{
    if( pLink->GetObj() )
    {
        // is it still valid?
        if( bIsValidData && nFormat == aData.GetFormat() )
            return &aData;

        Any aValue;
        String sMimeType( SotExchange::GetFormatMimeType( nFormat ));
        if( pLink->GetObj()->GetData( aValue, sMimeType ) )
        {
            if( aValue >>= aSeq )
            {
                aData = DdeData( (const char *)aSeq.getConstArray(), aSeq.getLength(), nFormat );

                bIsValidData = TRUE;
                return &aData;
            }
        }
    }
    aSeq.realloc( 0 );
    bIsValidData = FALSE;
    return 0;
}


BOOL ImplDdeItem::Put( const DdeData*  )
{
    OSL_FAIL( "ImplDdeItem::Put not implemented" );
    return FALSE;
}


void ImplDdeItem::AdviseLoop( BOOL bOpen )
{
    // Connection is closed, so also unsubscribe link
    if( pLink->GetObj() )
    {
        if( bOpen )
        {
            // A connection is re-established
            if( OBJECT_DDE_EXTERN == pLink->GetObjType() )
            {
                pLink->GetObj()->AddDataAdvise( pLink, String::CreateFromAscii( "text/plain;charset=utf-16" ),  ADVISEMODE_NODATA );
                pLink->GetObj()->AddConnectAdvise( pLink );
            }
        }
        else
        {
            // So that no-one gets the idea to delete the pointer
            // when Disconnecting!
            SvBaseLinkRef aRef( pLink );
            aRef->Disconnect();
        }
    }
}


static DdeTopic* FindTopic( const String & rLinkName, USHORT* pItemStt )
{
    if( 0 == rLinkName.Len() )
        return 0;

    String sNm( rLinkName );
    USHORT nTokenPos = 0;
    String sService( sNm.GetToken( 0, cTokenSeperator, nTokenPos ) );

    DdeServices& rSvc = DdeService::GetServices();
    for( DdeService* pService = rSvc.First(); pService;
                                                pService = rSvc.Next() )
        if( pService->GetName() == sService )
        {
            // then we search for the Topic
            String sTopic( sNm.GetToken( 0, cTokenSeperator, nTokenPos ) );
            if( pItemStt )
                *pItemStt = nTokenPos;

            DdeTopics& rTopics = pService->GetTopics();

            for( int i = 0; i < 2; ++i )
            {
                for( DdeTopic* pTopic = rTopics.First(); pTopic;
                                                pTopic = rTopics.Next() )
                    if( pTopic->GetName() == sTopic )
                        return pTopic;

                // Topic not found?
                // then we try once to create it
                if( i || !pService->MakeTopic( sTopic ) )
                    break;  // did not work, exiting
            }
            break;
        }
    return 0;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

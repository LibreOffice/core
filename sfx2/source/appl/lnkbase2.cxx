/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: lnkbase2.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 17:42:39 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/


#include "lnkbase.hxx"
#include <sot/exchange.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <vcl/msgbox.hxx>

#include "linkmgr.hxx"
//#include "svuidlg.hrc"
//#include "iface.hxx"
#include <vcl/svapp.hxx>
//#include <soerr.hxx>

#include "app.hrc"
#include "sfxresid.hxx"

#include <tools/debug.hxx>
#pragma hdrstop

#include <svtools/svdde.hxx>

using namespace ::com::sun::star::uno;

namespace sfx2
{

TYPEINIT0( SvBaseLink )

static DdeTopic* FindTopic( const String &, USHORT* = 0 );

class  ImplDdeItem;

// nur fuer die interne Verwaltung
struct ImplBaseLinkData
{
    union {
        struct
        {
            // gilt fuer alle Links
            ULONG               nCntntType; // Update Format
            // nicht Ole-Links
            BOOL            bIntrnlLnk; // ist es ein interner Link
            USHORT          nUpdateMode;// UpdateMode
        } ClientType;

        struct
        {
            ImplDdeItem* pItem;
        } DDEType;
    };
    ImplBaseLinkData()
    {
        ClientType.nCntntType = NULL;
        ClientType.bIntrnlLnk = FALSE;
        ClientType.nUpdateMode = 0;
        DDEType.pItem = NULL;
    }
};


class ImplDdeItem : public DdeGetPutItem
{
    SvBaseLink* pLink;
    DdeData aData;
    Sequence< sal_Int8 > aSeq;          // Datacontainer for DdeData !!!
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


/************************************************************************
|*    SvBaseLink::SvBaseLink()
|*
|*    Beschreibung
*************************************************************************/

SvBaseLink::SvBaseLink()
{
    nObjType = OBJECT_CLIENT_SO;
    pLinkMgr = 0;
    pImplData = new ImplBaseLinkData;
    bVisible = bSynchron = bUseCache = TRUE;
}

/************************************************************************
|*    SvBaseLink::SvBaseLink()
|*
|*    Beschreibung
*************************************************************************/

SvBaseLink::SvBaseLink( USHORT nUpdateMode, ULONG nContentType )
{
    nObjType = OBJECT_CLIENT_SO;
    pLinkMgr = 0;
    pImplData = new ImplBaseLinkData;
    bVisible = bSynchron = bUseCache = TRUE;

    // falls es ein Ole-Link wird,
    pImplData->ClientType.nUpdateMode = nUpdateMode;
    pImplData->ClientType.nCntntType = nContentType;
    pImplData->ClientType.bIntrnlLnk = FALSE;
}

/************************************************************************
|*    SvBaseLink::SvBaseLink()
|*
|*    Beschreibung
*************************************************************************/

SvBaseLink::SvBaseLink( const String& rLinkName, USHORT nObjectType, SvLinkSource* pObj )
{
    bVisible = bSynchron = bUseCache = TRUE;
    aLinkName = rLinkName;
    pImplData = new ImplBaseLinkData;
    nObjType = nObjectType;

    if( !pObj )
    {
        DBG_ASSERT( pObj, "Wo ist mein zu linkendes Object" );
        return;
    }

    if( OBJECT_DDE_EXTERN == nObjType )
    {
        USHORT nItemStt = 0;
        DdeTopic* pTopic = FindTopic( aLinkName, &nItemStt );
        if( pTopic )
        {
            // dann haben wir alles zusammen
            // MM hat gefummelt ???
            // MM_TODO wie kriege ich den Namen
            String aStr = aLinkName; // xLinkName->GetDisplayName();
            aStr = aStr.Copy( nItemStt );
            pImplData->DDEType.pItem = new ImplDdeItem( *this, aStr );
            pTopic->InsertItem( pImplData->DDEType.pItem );

            // dann koennen wir uns auch das Advise merken
            xObj = pObj;
        }
    }
    else if( pObj->Connect( this ) )
        xObj = pObj;
}

/************************************************************************
|*    SvBaseLink::~SvBaseLink()
|*
|*    Beschreibung
*************************************************************************/

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

/************************************************************************
|*    SvBaseLink::SetObjType()
|*
|*    Beschreibung
*************************************************************************/

void SvBaseLink::SetObjType( USHORT nObjTypeP )
{
    DBG_ASSERT( nObjType != OBJECT_CLIENT_DDE, "type already set" )
    DBG_ASSERT( !xObj.Is(), "object exist" )

    nObjType = nObjTypeP;
}

/************************************************************************
|*    SvBaseLink::SetName()
|*
|*    Beschreibung
*************************************************************************/

void SvBaseLink::SetName( const String & rNm )
{
    aLinkName = rNm;
}

/************************************************************************
|*    SvBaseLink::GetName()
|*
|*    Beschreibung
*************************************************************************/

String SvBaseLink::GetName() const
{
    return aLinkName;
}

/************************************************************************
|*    SvBaseLink::SetObj()
|*
|*    Beschreibung
*************************************************************************/

void SvBaseLink::SetObj( SvLinkSource * pObj )
{
    DBG_ASSERT( (nObjType & OBJECT_CLIENT_SO &&
                pImplData->ClientType.bIntrnlLnk) ||
                nObjType == OBJECT_CLIENT_GRF,
                "no intern link" )
    xObj = pObj;
}

/************************************************************************
|*    SvBaseLink::SetLinkSourceName()
|*
|*    Beschreibung
*************************************************************************/

void SvBaseLink::SetLinkSourceName( const String & rLnkNm )
{
    if( aLinkName == rLnkNm )
        return;

    AddNextRef(); // sollte ueberfluessig sein
    // Alte Verbindung weg
    Disconnect();

    aLinkName = rLnkNm;

    // Neu verbinden
    _GetRealObject();
    ReleaseRef(); // sollte ueberfluessig sein
}

/************************************************************************
|*    SvBaseLink::GetLinkSourceName()
|*
|*    Beschreibung
*************************************************************************/

String  SvBaseLink::GetLinkSourceName() const
{
    return aLinkName;
}


/************************************************************************
|*    SvBaseLink::SetUpdateMode()
|*
|*    Beschreibung
*************************************************************************/

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
            // m_xInputStreamToLoadFrom = 0;
            String sMimeType( SotExchange::GetFormatMimeType(
                            pImplData->ClientType.nCntntType ));
            Any aData;

            if( xObj->GetData( aData, sMimeType ) )
            {
                DataChanged( sMimeType, aData );
                //JP 13.07.00: Bug 76817 - for manual Updates there is no
                //              need to hold the ServerObject
                if( OBJECT_CLIENT_DDE == nObjType &&
                    LINKUPDATE_ONCALL == GetUpdateMode() && xObj.Is() )
                    xObj->RemoveAllDataAdvise( this );
                return TRUE;
            }
            if( xObj.Is() )
            {
                // sollten wir asynschron sein?
                if( xObj->IsPending() )
                    return TRUE;

                // dann brauchen wir das Object auch nicht mehr
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
            : LINKUPDATE_ONCALL;
}


void SvBaseLink::_GetRealObject( BOOL bConnect)
{
    if( !pLinkMgr )
        return;

    DBG_ASSERT( !xObj.Is(), "object already exist" )

    if( OBJECT_CLIENT_DDE == nObjType )
    {
        String sServer;
        if( pLinkMgr->GetDisplayNames( this, &sServer ) &&
            sServer == GetpApp()->GetAppName() )        // interner Link !!!
        {
            // damit der Internal - Link erzeugt werden kann !!!
            nObjType = OBJECT_INTERN;
            xObj = pLinkMgr->CreateObj( this );

            pImplData->ClientType.bIntrnlLnk = TRUE;
            nObjType = OBJECT_CLIENT_DDE;       // damit wir wissen was es mal war !!
        }
        else
        {
            pImplData->ClientType.bIntrnlLnk = FALSE;
            xObj = pLinkMgr->CreateObj( this );
        }
    }
    else if( (OBJECT_CLIENT_SO & nObjType) )
        xObj = pLinkMgr->CreateObj( this );

    if( bConnect && ( !xObj.Is() || !xObj->Connect( this ) ) )
        Disconnect();
}

ULONG SvBaseLink::GetContentType() const
{
    if( OBJECT_CLIENT_SO & nObjType )
        return pImplData->ClientType.nCntntType;

    return 0;       // alle Formate ?
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


BOOL SvBaseLink::Edit( Window* pParent )
{
    BOOL bConnect = xObj.Is();
    if( !bConnect )
        _GetRealObject( xObj.Is() );

    String aNewNm;

    if( OBJECT_CLIENT_SO & nObjType &&
        pImplData->ClientType.bIntrnlLnk )
    {
        if( pLinkMgr )
        {
            SvLinkSourceRef ref = pLinkMgr->CreateObj( this );
            if( ref.Is() )
                aNewNm = ref->Edit( pParent, this );
        }
    }
    else
        aNewNm = xObj->Edit( pParent, this );

    if( aNewNm.Len() != 0 )
    {
        SetLinkSourceName( aNewNm );
        if( !Update() )
        {
            String sApp, sTopic, sItem, sError;
            pLinkMgr->GetDisplayNames( this, &sApp, &sTopic, &sItem );
            if( nObjType == OBJECT_CLIENT_DDE )
            {
                sError = SfxResId( STR_DDE_ERROR );

                USHORT nFndPos = sError.Search( '%' );
                if( STRING_NOTFOUND != nFndPos )
                {
                    sError.Erase( nFndPos, 1 ).Insert( sApp, nFndPos );
                    nFndPos += sApp.Len();
                }
                if( STRING_NOTFOUND != ( nFndPos = sError.Search( '%', nFndPos )))
                {
                    sError.Erase( nFndPos, 1 ).Insert( sTopic, nFndPos );
                    nFndPos += sTopic.Len();
                }
                if( STRING_NOTFOUND != ( nFndPos = sError.Search( '%', nFndPos )))
                    sError.Erase( nFndPos, 1 ).Insert( sItem, nFndPos );
            }
            else
                return FALSE;

            ErrorBox( pParent, WB_OK, sError ).Execute();
        }
    }
    else if( !bConnect )
        Disconnect();
    return aNewNm.Len() != 0;
}


void SvBaseLink::Closed()
{
    if( xObj.Is() )
        // beim Advise Abmelden
        xObj->RemoveAllDataAdvise( this );
}


ImplDdeItem::~ImplDdeItem()
{
    bIsInDTOR = TRUE;
    // damit im Disconnect nicht jemand auf die Idee kommt, den Pointer zu
    // loeschen!!
    SvBaseLinkRef aRef( pLink );
    aRef->Disconnect();
}

DdeData* ImplDdeItem::Get( ULONG nFormat )
{
    if( pLink->GetObj() )
    {
        // ist das noch gueltig?
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
    DBG_ERROR( "ImplDdeItem::Put not implemented" );
    return FALSE;
}


void ImplDdeItem::AdviseLoop( BOOL bOpen )
{
    // Verbindung wird geschlossen, also Link abmelden
    if( pLink->GetObj() )
    {
        if( bOpen )
        {
            // es wird wieder eine Verbindung hergestellt
            if( OBJECT_DDE_EXTERN == pLink->GetObjType() )
            {
                pLink->GetObj()->AddDataAdvise( pLink, String::CreateFromAscii( "text/plain;charset=utf-16" ),  ADVISEMODE_NODATA );
                pLink->GetObj()->AddConnectAdvise( pLink );
            }
        }
        else
        {
            // damit im Disconnect nicht jemand auf die Idee kommt,
            // den Pointer zu loeschen!!
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
            // dann suchen wir uns das Topic
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

                // Topic nicht gefunden ?
                // dann versuchen wir ihn mal anzulegen
                if( i || !pService->MakeTopic( sTopic ) )
                    break;  // hat nicht geklappt, also raus
            }
            break;
        }
    return 0;
}

}

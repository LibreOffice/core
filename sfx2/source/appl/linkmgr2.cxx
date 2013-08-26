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

#include <comphelper/string.hxx>
#include <sfx2/linkmgr.hxx>
#include <com/sun/star/document/UpdateDocMode.hpp>
#include <sfx2/objsh.hxx>
#include <svl/urihelper.hxx>
#include <sot/formats.hxx>
#include <tools/urlobj.hxx>
#include <sot/exchange.hxx>
#include <tools/debug.hxx>
#include <vcl/msgbox.hxx>
#include <sfx2/lnkbase.hxx>
#include <sfx2/app.hxx>
#include <vcl/graph.hxx>
#include <svl/stritem.hxx>
#include <svl/eitem.hxx>
#include <svl/intitem.hxx>
#include <unotools/localfilehelper.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <sfx2/request.hxx>
#include <vcl/dibtools.hxx>

#include "fileobj.hxx"
#include "impldde.hxx"
#include "app.hrc"
#include "sfx2/sfxresid.hxx"

#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/util/XCloseable.hpp>

using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::lang::XComponent;
using ::com::sun::star::util::XCloseable;

namespace sfx2
{

class SvxInternalLink : public sfx2::SvLinkSource
{
public:
    SvxInternalLink() {}

    virtual sal_Bool Connect( sfx2::SvBaseLink* );
};


LinkManager::LinkManager(SfxObjectShell* p)
    : pPersist( p )
{
}


LinkManager::~LinkManager()
{
    for( size_t n = 0; n < aLinkTbl.size(); ++n)
    {
        SvBaseLinkRef* pTmp = aLinkTbl[ n ];
        if( pTmp->Is() )
        {
            (*pTmp)->Disconnect();
            (*pTmp)->SetLinkManager( NULL );
        }
        delete pTmp;
    }
}

void LinkManager::InsertCachedComp(const Reference<XComponent>& xComp)
{
    maCachedComps.push_back(xComp);
}

void LinkManager::CloseCachedComps()
{
    CompVector::iterator itr = maCachedComps.begin(), itrEnd = maCachedComps.end();
    for (; itr != itrEnd; ++itr)
    {
        Reference<XCloseable> xCloseable(*itr, UNO_QUERY);
        if (!xCloseable.is())
            continue;

        xCloseable->close(true);
    }
    maCachedComps.clear();
}

//--------------------------------------------------------------------------

void LinkManager::Remove( SvBaseLink *pLink )
{
    // No duplicate links inserted
    int bFound = sal_False;
    for( size_t n = 0; n < aLinkTbl.size(); )
    {
        SvBaseLinkRef* pTmp = aLinkTbl[ n ];
        if( pLink == *pTmp )
        {
            (*pTmp)->Disconnect();
            (*pTmp)->SetLinkManager( NULL );
            (*pTmp).Clear();
            bFound = sal_True;
        }

        // Remove empty ones if they exist
        if( !pTmp->Is() )
        {
            delete pTmp;
            aLinkTbl.erase( aLinkTbl.begin() + n );
            if( bFound )
                return ;
        }
        else
            ++n;
    }
}


void LinkManager::Remove( size_t nPos, size_t nCnt )
{
    if( nCnt && nPos < aLinkTbl.size() )
    {
        if (sal::static_int_cast<size_t>(nPos + nCnt) > aLinkTbl.size())
            nCnt = aLinkTbl.size() - nPos;

        for( size_t n = nPos; n < nPos + nCnt; ++n)
        {
            SvBaseLinkRef* pTmp = aLinkTbl[ n ];
            if( pTmp->Is() )
            {
                (*pTmp)->Disconnect();
                (*pTmp)->SetLinkManager( NULL );
            }
            delete pTmp;
        }
        aLinkTbl.erase( aLinkTbl.begin() + nPos, aLinkTbl.begin() + nPos + nCnt );
    }
}


sal_Bool LinkManager::Insert( SvBaseLink* pLink )
{
    for( size_t n = 0; n < aLinkTbl.size(); ++n )
    {
        SvBaseLinkRef* pTmp = aLinkTbl[ n ];
        if( !pTmp->Is() )
        {
            delete pTmp;
            aLinkTbl.erase( aLinkTbl.begin() + n-- );
        }
        else if( pLink == *pTmp )
            return sal_False; // No duplicate links inserted
    }

    SvBaseLinkRef* pTmp = new SvBaseLinkRef( pLink );
    pLink->SetLinkManager( this );
    aLinkTbl.push_back( pTmp );
    return sal_True;
}


sal_Bool LinkManager::InsertLink( SvBaseLink * pLink,
                                sal_uInt16 nObjType,
                                sal_uInt16 nUpdateMode,
                                const OUString* pName )
{
    // This First
    pLink->SetObjType( nObjType );
    if( pName )
        pLink->SetName( *pName );
    pLink->SetUpdateMode( nUpdateMode );
    return Insert( pLink );
}


sal_Bool LinkManager::InsertDDELink( SvBaseLink * pLink,
                                    const OUString& rServer,
                                    const OUString& rTopic,
                                    const OUString& rItem )
{
    if( !( OBJECT_CLIENT_SO & pLink->GetObjType() ) )
        return sal_False;

    OUString sCmd;
    ::sfx2::MakeLnkName( sCmd, &rServer, rTopic, rItem );

    pLink->SetObjType( OBJECT_CLIENT_DDE );
    pLink->SetName( sCmd );
    return Insert( pLink );
}


sal_Bool LinkManager::InsertDDELink( SvBaseLink * pLink )
{
    DBG_ASSERT( OBJECT_CLIENT_SO & pLink->GetObjType(), "no OBJECT_CLIENT_SO" );
    if( !( OBJECT_CLIENT_SO & pLink->GetObjType() ) )
        return sal_False;

    if( pLink->GetObjType() == OBJECT_CLIENT_SO )
        pLink->SetObjType( OBJECT_CLIENT_DDE );

    return Insert( pLink );
}


// Obtain the string for the dialog
bool LinkManager::GetDisplayNames( const SvBaseLink * pLink,
                                        OUString* pType,
                                        OUString* pFile,
                                        OUString* pLinkStr,
                                        OUString* pFilter ) const
{
    bool bRet = false;
    const String sLNm( pLink->GetLinkSourceName() );
    if( sLNm.Len() )
    {
        switch( pLink->GetObjType() )
        {
            case OBJECT_CLIENT_FILE:
            case OBJECT_CLIENT_GRF:
            case OBJECT_CLIENT_OLE:
                {
                    sal_Int32 nPos = 0;
                    String sFile( sLNm.GetToken( 0, ::sfx2::cTokenSeparator, nPos ) );
                    String sRange( sLNm.GetToken( 0, ::sfx2::cTokenSeparator, nPos ) );

                    if( pFile )
                        *pFile = sFile;
                    if( pLinkStr )
                        *pLinkStr = sRange;
                    if( pFilter )
                        *pFilter = sLNm.Copy( nPos );

                    if( pType )
                    {
                        sal_uInt16 nObjType = pLink->GetObjType();
                        *pType = SfxResId(
                                    ( OBJECT_CLIENT_FILE == nObjType || OBJECT_CLIENT_OLE == nObjType )
                                            ? RID_SVXSTR_FILELINK
                                            : RID_SVXSTR_GRAFIKLINK).toString();
                    }
                    bRet = true;
                }
                break;
            case OBJECT_CLIENT_DDE:
                {
                    sal_Int32 nTmp = 0;
                    String sCmd( sLNm );
                    String sServer( sCmd.GetToken( 0, cTokenSeparator, nTmp ) );
                    String sTopic( sCmd.GetToken( 0, cTokenSeparator, nTmp ) );

                    if( pType )
                        *pType = sServer;
                    if( pFile )
                        *pFile = sTopic;
                    if( pLinkStr )
                        *pLinkStr = sCmd.Copy( nTmp );
                    bRet = true;
                }
                break;
            default:
                break;
        }
    }

    return bRet;
}

void LinkManager::UpdateAllLinks(
    bool bAskUpdate,
    bool /*bCallErrHdl*/,
    bool bUpdateGrfLinks,
    Window* pParentWin )
{
    // First make a copy of the array in order to update links
    // links in ... no contact between them!
    std::vector<SvBaseLink*> aTmpArr;
    for( size_t n = 0; n < aLinkTbl.size(); ++n )
    {
        SvBaseLink* pLink = *aLinkTbl[ n ];
        if( !pLink )
        {
            Remove( n-- );
            continue;
        }
        aTmpArr.push_back( pLink );
    }

    for( size_t n = 0; n < aTmpArr.size(); ++n )
    {
        SvBaseLink* pLink = aTmpArr[ n ];

        // search first in the array after the entry
        bool bFound = false;
        for( size_t i = 0; i < aLinkTbl.size(); ++i )
            if( pLink == *aLinkTbl[ i ] )
            {
                bFound = true;
                break;
            }

        if( !bFound )
            continue;  // was not available!

        // Graphic-Links not to update yet
        if( !pLink->IsVisible() ||
            ( !bUpdateGrfLinks && OBJECT_CLIENT_GRF == pLink->GetObjType() ))
            continue;

        if( bAskUpdate )
        {
            int nRet = QueryBox( pParentWin, WB_YES_NO | WB_DEF_YES, SfxResId( STR_QUERY_UPDATE_LINKS ).toString() ).Execute();
            if( RET_YES != nRet )
                return ;         // nothing should be updated
            bAskUpdate = false;  // once is enough
        }

        pLink->Update();
    }
    CloseCachedComps();
}

//--------------------------------------------------------------------------

SvLinkSourceRef LinkManager::CreateObj( SvBaseLink * pLink )
{
    switch( pLink->GetObjType() )
    {
        case OBJECT_CLIENT_FILE:
        case OBJECT_CLIENT_GRF:
        case OBJECT_CLIENT_OLE:
            return new SvFileObject;
        case OBJECT_INTERN:
            return new SvxInternalLink;
        case OBJECT_CLIENT_DDE:
            return new SvDDEObject;
        default:
            return SvLinkSourceRef();
       }
}

sal_Bool LinkManager::InsertServer( SvLinkSource* pObj )
{
    // no duplicate inserts
    if( !pObj )
        return sal_False;

    return aServerTbl.insert( pObj ).second;
}


void LinkManager::RemoveServer( SvLinkSource* pObj )
{
    aServerTbl.erase( pObj );
}


void MakeLnkName( OUString& rName, const OUString* pType, const OUString& rFile,
                    const OUString& rLink, const OUString* pFilter )
{
    if( pType )
    {
        rName = comphelper::string::strip(*pType, ' ');
        rName += OUString(cTokenSeparator);
    }
    else if( !rName.isEmpty() )
        rName = "";

    rName += rFile;

    rName = comphelper::string::strip(rName, ' ');
    rName += OUString(cTokenSeparator);
    rName = comphelper::string::strip(rName, ' ');
    rName += rLink;
    if( pFilter )
    {
        rName += OUString(cTokenSeparator);
        rName += *pFilter;
        rName = comphelper::string::strip(rName, ' ');
    }
}

void LinkManager::ReconnectDdeLink(SfxObjectShell& rServer)
{
    SfxMedium* pMed = rServer.GetMedium();
    if (!pMed)
        return;

    const ::sfx2::SvBaseLinks& rLinks = GetLinks();
    size_t n = rLinks.size();

    for (size_t i = 0; i < n; ++i)
    {
        ::sfx2::SvBaseLink* p = *rLinks[i];
        OUString aType, aFile, aLink, aFilter;
        if (!GetDisplayNames(p, &aType, &aFile, &aLink, &aFilter))
            continue;

        if (aType != "soffice")
            // DDE connections between OOo apps are always named 'soffice'.
            continue;

        OUString aTmp;
        OUString aURL = aFile;
        if (utl::LocalFileHelper::ConvertPhysicalNameToURL(aFile, aTmp))
            aURL = aTmp;

        if (!aURL.equalsIgnoreAsciiCase(pMed->GetName()))
            // This DDE link is not associated with this server shell...  Skip it.
            continue;

        if (aLink.isEmpty())
            continue;

        LinkServerShell(aLink, rServer, *p);
    }
}

void LinkManager::LinkServerShell(const OUString& rPath, SfxObjectShell& rServer, ::sfx2::SvBaseLink& rLink) const
{
    ::sfx2::SvLinkSource* pSrvSrc = rServer.DdeCreateLinkSource(rPath);
    if (pSrvSrc)
    {
        ::com::sun::star::datatransfer::DataFlavor aFl;
        SotExchange::GetFormatDataFlavor(rLink.GetContentType(), aFl);
        rLink.SetObj(pSrvSrc);
        pSrvSrc->AddDataAdvise(
            &rLink, aFl.MimeType,
            sfx2::LINKUPDATE_ONCALL == rLink.GetUpdateMode() ? ADVISEMODE_ONLYONCE : 0);
    }
}

bool LinkManager::InsertFileLink(
    sfx2::SvBaseLink& rLink, sal_uInt16 nFileType, const OUString& rFileNm,
    const OUString* pFilterNm, const OUString* pRange)
{
    if (!(OBJECT_CLIENT_SO & rLink.GetObjType()))
        return false;

    OUStringBuffer aBuf;
    aBuf.append(rFileNm);
    aBuf.append(sfx2::cTokenSeparator);

    if (pRange)
        aBuf.append(*pRange);

    if (pFilterNm)
    {
        aBuf.append(sfx2::cTokenSeparator);
        aBuf.append(*pFilterNm);
    }

    OUString aCmd = aBuf.makeStringAndClear();
    return InsertLink(&rLink, nFileType, sfx2::LINKUPDATE_ONCALL, &aCmd);
}

// A transfer is aborted, so cancel all download media
// (for now this is only of interest for the file links!)
void LinkManager::CancelTransfers()
{
    SvFileObject* pFileObj;
    sfx2::SvBaseLink* pLnk;

    const sfx2::SvBaseLinks& rLnks = GetLinks();
    for( size_t n = rLnks.size(); n; )
        if( 0 != ( pLnk = &(*rLnks[ --n ])) &&
            OBJECT_CLIENT_FILE == (OBJECT_CLIENT_FILE & pLnk->GetObjType()) &&
            0 != ( pFileObj = (SvFileObject*)pLnk->GetObj() ) )
            pFileObj->CancelTransfers();
}
    // For the purpose of sending Status information from the file object to
    // the base link, there exist a dedicated ClipBoardId. The SvData-object
    // gets the appropriate information as a string
    // For now this is required for file object in conjunction with JavaScript
    // - needs information about Load/Abort/Error
sal_uIntPtr LinkManager::RegisterStatusInfoId()
{
    static sal_uIntPtr nFormat = 0;

    if( !nFormat )
    {
        nFormat = SotExchange::RegisterFormatName(
                    OUString("StatusInfo from SvxInternalLink"));
    }
    return nFormat;
}

// ----------------------------------------------------------------------

sal_Bool LinkManager::GetGraphicFromAny( const OUString& rMimeType,
                                const ::com::sun::star::uno::Any & rValue,
                                Graphic& rGrf )
{
    sal_Bool bRet = sal_False;
    ::com::sun::star::uno::Sequence< sal_Int8 > aSeq;
    if( rValue.hasValue() && ( rValue >>= aSeq ) )
    {
        SvMemoryStream aMemStm( (void*)aSeq.getConstArray(), aSeq.getLength(),
                                STREAM_READ );
        aMemStm.Seek( 0 );

        switch( SotExchange::GetFormatIdFromMimeType( rMimeType ) )
        {
        case SOT_FORMATSTR_ID_SVXB:
            {
                aMemStm >> rGrf;
                bRet = sal_True;
            }
            break;
        case FORMAT_GDIMETAFILE:
            {
                GDIMetaFile aMtf;
                aMtf.Read( aMemStm );
                rGrf = aMtf;
                bRet = sal_True;
            }
            break;
        case FORMAT_BITMAP:
            {
                Bitmap aBmp;
                ReadDIB(aBmp, aMemStm, true);
                rGrf = aBmp;
                bRet = sal_True;
            }
            break;
        }
    }
    return bRet;
}


// ----------------------------------------------------------------------
String lcl_DDE_RelToAbs( const OUString& rTopic, const OUString& rBaseURL )
{
    OUString sRet;
    INetURLObject aURL( rTopic );
    if( INET_PROT_NOT_VALID == aURL.GetProtocol() )
        utl::LocalFileHelper::ConvertSystemPathToURL( rTopic, rBaseURL, sRet );
    if( sRet.isEmpty() )
        sRet = URIHelper::SmartRel2Abs( INetURLObject(rBaseURL), rTopic, URIHelper::GetMaybeFileHdl(), true );
    return String(sRet);
}

sal_Bool SvxInternalLink::Connect( sfx2::SvBaseLink* pLink )
{
    SfxObjectShell* pFndShell = 0;
    sal_uInt16 nUpdateMode = com::sun::star::document::UpdateDocMode::NO_UPDATE;
    OUString sTopic, sItem, sReferer;
    LinkManager* pLinkMgr = pLink->GetLinkManager();
    if (pLinkMgr && pLinkMgr->GetDisplayNames(pLink, 0, &sTopic, &sItem) && !sTopic.isEmpty())
    {
        // first only loop over the DocumentShells the shells and find those
        // with the name:
        CharClass aCC( LanguageTag( LANGUAGE_SYSTEM) );

        TypeId aType( TYPE(SfxObjectShell) );

        sal_Bool bFirst = sal_True;
        SfxObjectShell* pShell = pLinkMgr->GetPersist();
        if( pShell && pShell->GetMedium() )
        {
            sReferer = pShell->GetMedium()->GetBaseURL();
            SFX_ITEMSET_ARG( pShell->GetMedium()->GetItemSet(), pItem, SfxUInt16Item, SID_UPDATEDOCMODE, sal_False );
            if ( pItem )
                nUpdateMode = pItem->GetValue();
        }

        String sNmURL(aCC.lowercase(lcl_DDE_RelToAbs(sTopic, sReferer)));

        if ( !pShell )
        {
            bFirst = sal_False;
            pShell = SfxObjectShell::GetFirst( &aType, sal_False );
        }

        String sTmp;
        while( pShell )
        {
            if( !sTmp.Len() )
            {
                sTmp = pShell->GetTitle( SFX_TITLE_FULLNAME );
                sTmp = lcl_DDE_RelToAbs(sTmp, sReferer );
            }


            sTmp = aCC.lowercase( sTmp );
            if( sTmp == sNmURL )  // we want these
            {
                pFndShell = pShell;
                break;
            }

            if( bFirst )
            {
                bFirst = sal_False;
                pShell = SfxObjectShell::GetFirst( &aType, sal_False );
            }
            else
                pShell = SfxObjectShell::GetNext( *pShell, &aType, sal_False );

            sTmp.Erase();
        }
    }

    // empty topics are not allowed - which document is it
    if( sTopic.isEmpty() )
        return sal_False;

    if (pFndShell)
    {
        sfx2::SvLinkSource* pNewSrc = pFndShell->DdeCreateLinkSource( sItem );
        if( pNewSrc )
        {
            ::com::sun::star::datatransfer::DataFlavor aFl;
            SotExchange::GetFormatDataFlavor( pLink->GetContentType(), aFl );

            pLink->SetObj( pNewSrc );
            pNewSrc->AddDataAdvise( pLink, aFl.MimeType,
                                sfx2::LINKUPDATE_ONCALL == pLink->GetUpdateMode()
                                    ? ADVISEMODE_ONLYONCE
                                    : 0 );
            return true;
        }
    }
    else
    {
        // then try to download the file:
        INetURLObject aURL( sTopic );
        INetProtocol eOld = aURL.GetProtocol();
        aURL.SetURL( sTopic = lcl_DDE_RelToAbs( sTopic, sReferer ) );
        if( INET_PROT_NOT_VALID != eOld ||
            INET_PROT_HTTP != aURL.GetProtocol() )
        {
            SfxStringItem aName( SID_FILE_NAME, sTopic );
            SfxBoolItem aMinimized(SID_MINIMIZED, sal_True);
            SfxBoolItem aHidden(SID_HIDDEN, sal_True);
            SfxStringItem aTarget( SID_TARGETNAME, OUString("_blank") );
            SfxStringItem aReferer( SID_REFERER, sReferer );
            SfxUInt16Item aUpdate( SID_UPDATEDOCMODE, nUpdateMode );
            SfxBoolItem aReadOnly(SID_DOC_READONLY, false);

            // Disable automatic re-connection to avoid this link instance
            // being destroyed at re-connection.
            SfxBoolItem aDdeConnect(SID_DDE_RECONNECT_ONLOAD, false);

            // #i14200# (DDE-link crashes wordprocessor)
            SfxAllItemSet aArgs( SFX_APP()->GetPool() );
            aArgs.Put(aReferer);
            aArgs.Put(aTarget);
            aArgs.Put(aHidden);
            aArgs.Put(aMinimized);
            aArgs.Put(aName);
            aArgs.Put(aUpdate);
            aArgs.Put(aReadOnly);
            aArgs.Put(aDdeConnect);
            Reference<XComponent> xComp = SfxObjectShell::CreateAndLoadComponent(aArgs);
            pFndShell = SfxObjectShell::GetShellFromComponent(xComp);
            if (xComp.is() && pFndShell)
            {
                pLinkMgr->InsertCachedComp(xComp);
                pLinkMgr->LinkServerShell(sItem, *pFndShell, *pLink);
                return true;
            }
        }
    }

    return false;
}


}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

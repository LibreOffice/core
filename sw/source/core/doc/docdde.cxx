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

#include <stdlib.h>

#include <vcl/svapp.hxx>
#include <tools/urlobj.hxx>

#include <sfx2/linkmgr.hxx>
#include <unotools/charclass.hxx>
#include <fmtcntnt.hxx>
#include <doc.hxx>
#include <swserv.hxx>
#include <IMark.hxx>
#include <bookmrk.hxx>
#include <section.hxx>
#include <swtable.hxx>
#include <node.hxx>
#include <ndtxt.hxx>
#include <pam.hxx>
#include <docary.hxx>
#include <MarkManager.hxx>
#include <boost/foreach.hpp>

using namespace ::com::sun::star;

namespace
{

    static ::sw::mark::DdeBookmark* lcl_FindDdeBookmark(
        const IDocumentMarkAccess& rMarkAccess,
        const OUString& rName,
        const bool bCaseSensitive )
    {
        //Iterating over all bookmarks, checking DdeBookmarks
        const OUString sNameLc = bCaseSensitive ? rName : GetAppCharClass().lowercase(rName);
        for(IDocumentMarkAccess::const_iterator_t ppMark = rMarkAccess.getCommonMarksBegin();
            ppMark != rMarkAccess.getCommonMarksEnd();
            ++ppMark)
        {
            if ( IDocumentMarkAccess::GetType( *(ppMark->get()) ) == IDocumentMarkAccess::DDE_BOOKMARK)
            {
                ::sw::mark::DdeBookmark* const pBkmk = dynamic_cast< ::sw::mark::DdeBookmark*>(ppMark->get());
                if (!pBkmk)
                    return NULL;
                if (
                    (bCaseSensitive && (pBkmk->GetName() == sNameLc)) ||
                    (!bCaseSensitive && GetAppCharClass().lowercase(pBkmk->GetName()) == sNameLc)
                   )
                {
                    return pBkmk;
                }
            }
        }
        return NULL;
    }
}

struct _FindItem
{
    const OUString m_Item;
    SwTableNode* pTblNd;
    SwSectionNode* pSectNd;

    _FindItem(const OUString& rS)
        : m_Item(rS), pTblNd(0), pSectNd(0)
    {}
};

static bool lcl_FindSection( const SwSectionFmt* pSectFmt, _FindItem * const pItem, bool bCaseSensitive )
{
    SwSection* pSect = pSectFmt->GetSection();
    if( pSect )
    {
        OUString sNm( (bCaseSensitive)
                ? pSect->GetSectionName()
                : GetAppCharClass().lowercase( pSect->GetSectionName() ));
        OUString sCompare( (bCaseSensitive)
                ? pItem->m_Item
                : GetAppCharClass().lowercase( pItem->m_Item ) );
        if( sNm == sCompare )
        {
            // found, so get the data
            const SwNodeIndex* pIdx;
            if( 0 != (pIdx = pSectFmt->GetCntnt().GetCntntIdx() ) &&
                &pSectFmt->GetDoc()->GetNodes() == &pIdx->GetNodes() )
            {
                // a table in the normal NodesArr
                pItem->pSectNd = pIdx->GetNode().GetSectionNode();
                return false;
            }
            // If the name is already correct, but not the rest then we don't have them.
            // The names are always unique.
        }
    }
    return true;
}

static bool lcl_FindTable( const SwFrmFmt* pTableFmt, _FindItem * const pItem )
{
    OUString sNm( GetAppCharClass().lowercase( pTableFmt->GetName() ));
    if ( sNm == pItem->m_Item )
    {
        SwTable* pTmpTbl;
        SwTableBox* pFBox;
        if( 0 != ( pTmpTbl = SwTable::FindTable( pTableFmt ) ) &&
            0 != ( pFBox = pTmpTbl->GetTabSortBoxes()[0] ) &&
            pFBox->GetSttNd() &&
            &pTableFmt->GetDoc()->GetNodes() == &pFBox->GetSttNd()->GetNodes() )
        {
            // a table in the normal NodesArr
            pItem->pTblNd = (SwTableNode*)
                                        pFBox->GetSttNd()->FindTableNode();
            return false;
        }
        // If the name is already correct, but not the rest then we don't have them.
        // The names are always unique.
    }
    return true;
}

bool SwDoc::GetData( const OUString& rItem, const OUString& rMimeType,
                     uno::Any & rValue ) const
{
    // search for bookmarks and sections case senstive at first. If nothing is found then try again case insensitive
    bool bCaseSensitive = true;
    while( true )
    {
        ::sw::mark::DdeBookmark* const pBkmk = lcl_FindDdeBookmark(*mpMarkManager, rItem, bCaseSensitive);
        if(pBkmk)
            return SwServerObject(*pBkmk).GetData(rValue, rMimeType);

        // Do we already have the Item?
        OUString sItem( bCaseSensitive ? rItem : GetAppCharClass().lowercase(rItem));
        _FindItem aPara( sItem );
        BOOST_FOREACH( const SwSectionFmt* pFmt, *mpSectionFmtTbl )
        {
            if (!(lcl_FindSection(pFmt, &aPara, bCaseSensitive)))
                break;
        }
        if( aPara.pSectNd )
        {
            // found, so get the data
            return SwServerObject( *aPara.pSectNd ).GetData( rValue, rMimeType );
        }
        if( !bCaseSensitive )
            break;
        bCaseSensitive = false;
    }

    _FindItem aPara( GetAppCharClass().lowercase( rItem ));
    BOOST_FOREACH( const SwFrmFmt* pFmt, *mpTblFrmFmtTbl )
    {
        if (!(lcl_FindTable(pFmt, &aPara)))
            break;
    }
    if( aPara.pTblNd )
    {
        return SwServerObject( *aPara.pTblNd ).GetData( rValue, rMimeType );
    }

    return false;
}

bool SwDoc::SetData( const OUString& rItem, const OUString& rMimeType,
                     const uno::Any & rValue )
{
    // search for bookmarks and sections case senstive at first. If nothing is found then try again case insensitive
    bool bCaseSensitive = true;
    while( true )
    {
        ::sw::mark::DdeBookmark* const pBkmk = lcl_FindDdeBookmark(*mpMarkManager, rItem, bCaseSensitive);
        if(pBkmk)
            return SwServerObject(*pBkmk).SetData(rMimeType, rValue);

        // Do we already have the Item?
        OUString sItem( bCaseSensitive ? rItem : GetAppCharClass().lowercase(rItem));
        _FindItem aPara( sItem );
        BOOST_FOREACH( const SwSectionFmt* pFmt, *mpSectionFmtTbl )
        {
            if (!(lcl_FindSection(pFmt, &aPara, bCaseSensitive)))
                break;
        }
        if( aPara.pSectNd )
        {
            // found, so get the data
            return SwServerObject( *aPara.pSectNd ).SetData( rMimeType, rValue );
        }
        if( !bCaseSensitive )
            break;
        bCaseSensitive = false;
    }

    OUString sItem(GetAppCharClass().lowercase(rItem));
    _FindItem aPara( sItem );
    BOOST_FOREACH( const SwFrmFmt* pFmt, *mpTblFrmFmtTbl )
    {
        if (!(lcl_FindTable(pFmt, &aPara)))
            break;
    }
    if( aPara.pTblNd )
    {
        return SwServerObject( *aPara.pTblNd ).SetData( rMimeType, rValue );
    }

    return false;
}

::sfx2::SvLinkSource* SwDoc::CreateLinkSource(const OUString& rItem)
{
    SwServerObject* pObj = NULL;

    // search for bookmarks and sections case senstive at first. If nothing is found then try again case insensitive
    bool bCaseSensitive = true;
    while( true )
    {
        // bookmarks
        ::sw::mark::DdeBookmark* const pBkmk = lcl_FindDdeBookmark(*mpMarkManager, rItem, bCaseSensitive);
        if(pBkmk && pBkmk->IsExpanded()
            && (0 == (pObj = pBkmk->GetRefObject())))
        {
            // mark found, but no link yet -> create hotlink
            pObj = new SwServerObject(*pBkmk);
            pBkmk->SetRefObject(pObj);
            GetLinkManager().InsertServer(pObj);
        }
        if(pObj)
            return pObj;

        _FindItem aPara(bCaseSensitive ? rItem : GetAppCharClass().lowercase(rItem));
        // sections
        BOOST_FOREACH( const SwSectionFmt* pFmt, *mpSectionFmtTbl )
        {
            if (!(lcl_FindSection(pFmt, &aPara, bCaseSensitive)))
                break;
        }

        if(aPara.pSectNd
            && (0 == (pObj = aPara.pSectNd->GetSection().GetObject())))
        {
            // section found, but no link yet -> create hotlink
            pObj = new SwServerObject( *aPara.pSectNd );
            aPara.pSectNd->GetSection().SetRefObject( pObj );
            GetLinkManager().InsertServer(pObj);
        }
        if(pObj)
            return pObj;
        if( !bCaseSensitive )
            break;
        bCaseSensitive = false;
    }

    _FindItem aPara( GetAppCharClass().lowercase(rItem) );
    // tables
    BOOST_FOREACH( const SwFrmFmt* pFmt, *mpTblFrmFmtTbl )
    {
        if (!(lcl_FindTable(pFmt, &aPara)))
            break;
    }
    if(aPara.pTblNd
        && (0 == (pObj = aPara.pTblNd->GetTable().GetObject())))
    {
        // table found, but no link yet -> create hotlink
        pObj = new SwServerObject(*aPara.pTblNd);
        aPara.pTblNd->GetTable().SetRefObject(pObj);
        GetLinkManager().InsertServer(pObj);
    }
    return pObj;
}

bool SwDoc::SelectServerObj( const OUString& rStr, SwPaM*& rpPam,
                            SwNodeRange*& rpRange ) const
{
    // Do we actually have the Item?
    rpPam = 0;
    rpRange = 0;

    OUString sItem( INetURLObject::decode( rStr, INET_HEX_ESCAPE,
                                         INetURLObject::DECODE_WITH_CHARSET,
                                        RTL_TEXTENCODING_UTF8 ));

    sal_Int32 nPos = sItem.indexOf( cMarkSeparator );

    const CharClass& rCC = GetAppCharClass();

    // Extension for sections: not only link bookmarks/sections
    // but also frames (text!), tables, outlines:
    if( -1 != nPos )
    {
        bool bContinue = false;
        OUString sName( sItem.copy( 0, nPos ) );
        OUString sCmp( sItem.copy( nPos + 1 ));
        sItem = rCC.lowercase( sItem );

        _FindItem aPara( sName );

        if( sCmp == "table" )
        {
            sName = rCC.lowercase( sName );
            BOOST_FOREACH( const SwFrmFmt* pFmt, *mpTblFrmFmtTbl )
            {
                if (!(lcl_FindTable(pFmt, &aPara)))
                    break;
            }
            if( aPara.pTblNd )
            {
                rpRange = new SwNodeRange( *aPara.pTblNd, 0,
                                *aPara.pTblNd->EndOfSectionNode(), 1 );
                return true;
            }
        }
        else if( sCmp == "frame" )
        {
            SwNodeIndex* pIdx;
            SwNode* pNd;
            const SwFlyFrmFmt* pFlyFmt = FindFlyByName( sName );
            if( pFlyFmt &&
                0 != ( pIdx = (SwNodeIndex*)pFlyFmt->GetCntnt().GetCntntIdx() ) &&
                !( pNd = &pIdx->GetNode())->IsNoTxtNode() )
            {
                rpRange = new SwNodeRange( *pNd, 1, *pNd->EndOfSectionNode() );
                return true;
            }
        }
        else if( sCmp == "region" )
        {
            sItem = sName;              // Is being dealt with further down!
            bContinue = true;
        }
        else if( sCmp == "outline" )
        {
            SwPosition aPos( SwNodeIndex( (SwNodes&)GetNodes() ));
            if( GotoOutline( aPos, sName ))
            {
                SwNode* pNd = &aPos.nNode.GetNode();
                const int nLvl = pNd->GetTxtNode()->GetAttrOutlineLevel()-1;

                const SwOutlineNodes& rOutlNds = GetNodes().GetOutLineNds();
                sal_uInt16 nTmpPos;
                rOutlNds.Seek_Entry( pNd, &nTmpPos );
                rpRange = new SwNodeRange( aPos.nNode, 0, aPos.nNode );

                // look for the section's end, now
                for( ++nTmpPos;
                        nTmpPos < rOutlNds.size() &&
                        nLvl < rOutlNds[ nTmpPos ]->GetTxtNode()->
                                GetAttrOutlineLevel()-1;
                    ++nTmpPos )
                    ;       // there is no block

                if( nTmpPos < rOutlNds.size() )
                    rpRange->aEnd = *rOutlNds[ nTmpPos ];
                else
                    rpRange->aEnd = GetNodes().GetEndOfContent();
                return true;
            }
        }

        if( !bContinue )
            return false;
    }

    // search for bookmarks and sections case senstive at first. If nothing is found then try again case insensitive
    bool bCaseSensitive = true;
    while( true )
    {
        ::sw::mark::DdeBookmark* const pBkmk = lcl_FindDdeBookmark(*mpMarkManager, sItem, bCaseSensitive);
        if(pBkmk)
        {
            if(pBkmk->IsExpanded())
                rpPam = new SwPaM(
                    pBkmk->GetMarkPos(),
                    pBkmk->GetOtherMarkPos());
            return static_cast<bool>(rpPam);
        }

        _FindItem aPara( bCaseSensitive ? sItem : rCC.lowercase( sItem ) );

        if( !mpSectionFmtTbl->empty() )
        {
            BOOST_FOREACH( const SwSectionFmt* pFmt, *mpSectionFmtTbl )
            {
                if (!(lcl_FindSection(pFmt, &aPara, bCaseSensitive)))
                    break;
            }
            if( aPara.pSectNd )
            {
                rpRange = new SwNodeRange( *aPara.pSectNd, 1,
                                        *aPara.pSectNd->EndOfSectionNode() );
                return true;

            }
        }
        if( !bCaseSensitive )
            break;
        bCaseSensitive = false;
    }
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

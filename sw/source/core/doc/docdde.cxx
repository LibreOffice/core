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

#include <stdlib.h>

#include <vcl/svapp.hxx>
#include <tools/urlobj.hxx>

#include <sfx2/linkmgr.hxx>         // LinkManager
#include <unotools/charclass.hxx>
#include <fmtcntnt.hxx>
#include <doc.hxx>
#include <swserv.hxx>           // for server functionality
#include <IMark.hxx>
#include <bookmrk.hxx>
#include <section.hxx>          // for SwSectionFmt
#include <swtable.hxx>          // for SwTable
#include <node.hxx>
#include <ndtxt.hxx>
#include <pam.hxx>
#include <docary.hxx>
#include <MarkManager.hxx>
#include <boost/foreach.hpp>

using namespace ::com::sun::star;

namespace
{

    static ::sw::mark::DdeBookmark* lcl_FindDdeBookmark(const IDocumentMarkAccess& rMarkAccess, const rtl::OUString& rName, bool bCaseSensitive)
    {
        //Iterating over all bookmarks, checking DdeBookmarks
        const ::rtl::OUString sNameLc = bCaseSensitive ? rName : GetAppCharClass().lowercase(rName);
        for(IDocumentMarkAccess::const_iterator_t ppMark = rMarkAccess.getMarksBegin();
            ppMark != rMarkAccess.getMarksEnd();
            ++ppMark)
        {
            if (::sw::mark::DdeBookmark* const pBkmk = dynamic_cast< ::sw::mark::DdeBookmark*>(ppMark->get()))
            {
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
    const String m_Item;
    SwTableNode* pTblNd;
    SwSectionNode* pSectNd;

    _FindItem(const String& rS)
        : m_Item(rS), pTblNd(0), pSectNd(0)
    {}
};

static bool lcl_FindSection( const SwSectionFmt* pSectFmt, _FindItem * const pItem, bool bCaseSensitive )
{
    SwSection* pSect = pSectFmt->GetSection();
    if( pSect )
    {
        String sNm( (bCaseSensitive)
                ? pSect->GetSectionName()
                : String(GetAppCharClass().lowercase( pSect->GetSectionName() )));
        String sCompare( (bCaseSensitive)
                ? pItem->m_Item
                : String(GetAppCharClass().lowercase( pItem->m_Item ) ));
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
    String sNm( GetAppCharClass().lowercase( pTableFmt->GetName() ));
    if (sNm.Equals( pItem->m_Item ))
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



bool SwDoc::GetData( const rtl::OUString& rItem, const String& rMimeType,
                     uno::Any & rValue ) const
{
    // search for bookmarks and sections case senstive at first. If nothing is found then try again case insensitive
    bool bCaseSensitive = true;
    while( true )
    {
        ::sw::mark::DdeBookmark* const pBkmk = lcl_FindDdeBookmark(*pMarkManager, rItem, bCaseSensitive);
        if(pBkmk)
            return SwServerObject(*pBkmk).GetData(rValue, rMimeType);

        // Do we already have the Item?
        String sItem( bCaseSensitive ? rItem : GetAppCharClass().lowercase(rItem));
        _FindItem aPara( sItem );
        BOOST_FOREACH( const SwSectionFmt* pFmt, *pSectionFmtTbl )
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
    BOOST_FOREACH( const SwFrmFmt* pFmt, *pTblFrmFmtTbl )
    {
        if (!(lcl_FindTable(pFmt, &aPara)))
            break;
    }
    if( aPara.pTblNd )
    {
        return SwServerObject( *aPara.pTblNd ).GetData( rValue, rMimeType );
    }

    return sal_False;
}



bool SwDoc::SetData( const rtl::OUString& rItem, const String& rMimeType,
                     const uno::Any & rValue )
{
    // search for bookmarks and sections case senstive at first. If nothing is found then try again case insensitive
    bool bCaseSensitive = true;
    while( true )
    {
        ::sw::mark::DdeBookmark* const pBkmk = lcl_FindDdeBookmark(*pMarkManager, rItem, bCaseSensitive);
        if(pBkmk)
            return SwServerObject(*pBkmk).SetData(rMimeType, rValue);

        // Do we already have the Item?
        String sItem( bCaseSensitive ? rItem : GetAppCharClass().lowercase(rItem));
        _FindItem aPara( sItem );
        BOOST_FOREACH( const SwSectionFmt* pFmt, *pSectionFmtTbl )
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

    String sItem(GetAppCharClass().lowercase(rItem));
    _FindItem aPara( sItem );
    BOOST_FOREACH( const SwFrmFmt* pFmt, *pTblFrmFmtTbl )
    {
        if (!(lcl_FindTable(pFmt, &aPara)))
            break;
    }
    if( aPara.pTblNd )
    {
        return SwServerObject( *aPara.pTblNd ).SetData( rMimeType, rValue );
    }

    return sal_False;
}



::sfx2::SvLinkSource* SwDoc::CreateLinkSource(const rtl::OUString& rItem)
{
    SwServerObject* pObj = NULL;

    // search for bookmarks and sections case senstive at first. If nothing is found then try again case insensitive
    bool bCaseSensitive = true;
    while( true )
    {
        // bookmarks
        ::sw::mark::DdeBookmark* const pBkmk = lcl_FindDdeBookmark(*pMarkManager, rItem, bCaseSensitive);
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
        BOOST_FOREACH( const SwSectionFmt* pFmt, *pSectionFmtTbl )
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
    BOOST_FOREACH( const SwFrmFmt* pFmt, *pTblFrmFmtTbl )
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

sal_Bool SwDoc::SelectServerObj( const String& rStr, SwPaM*& rpPam,
                            SwNodeRange*& rpRange ) const
{
    // Do we actually have the Item?
    rpPam = 0;
    rpRange = 0;

    String sItem( INetURLObject::decode( rStr, INET_HEX_ESCAPE,
                                         INetURLObject::DECODE_WITH_CHARSET,
                                        RTL_TEXTENCODING_UTF8 ));

    xub_StrLen nPos = sItem.Search( cMarkSeperator );

    const CharClass& rCC = GetAppCharClass();

    // Extension for sections: not only link bookmarks/sections
    // but also frames (text!), tables, outlines:
    if( STRING_NOTFOUND != nPos )
    {
        sal_Bool bWeiter = sal_False;
        String sName( sItem.Copy( 0, nPos ) );
        String sCmp( sItem.Copy( nPos + 1 ));
        sItem = rCC.lowercase( sItem );

        _FindItem aPara( sName );

        if( sCmp.EqualsAscii( pMarkToTable ) )
        {
            sName = rCC.lowercase( sName );
            BOOST_FOREACH( const SwFrmFmt* pFmt, *pTblFrmFmtTbl )
            {
                if (!(lcl_FindTable(pFmt, &aPara)))
                    break;
            }
            if( aPara.pTblNd )
            {
                rpRange = new SwNodeRange( *aPara.pTblNd, 0,
                                *aPara.pTblNd->EndOfSectionNode(), 1 );
                return sal_True;
            }
        }
        else if( sCmp.EqualsAscii( pMarkToFrame ) )
        {
            SwNodeIndex* pIdx;
            SwNode* pNd;
            const SwFlyFrmFmt* pFlyFmt = FindFlyByName( sName );
            if( pFlyFmt &&
                0 != ( pIdx = (SwNodeIndex*)pFlyFmt->GetCntnt().GetCntntIdx() ) &&
                !( pNd = &pIdx->GetNode())->IsNoTxtNode() )
            {
                rpRange = new SwNodeRange( *pNd, 1, *pNd->EndOfSectionNode() );
                return sal_True;
            }
        }
        else if( sCmp.EqualsAscii( pMarkToRegion ) )
        {
            sItem = sName;              // Is being dealt with further down!
            bWeiter = sal_True;
        }
        else if( sCmp.EqualsAscii( pMarkToOutline ) )
        {
            SwPosition aPos( SwNodeIndex( (SwNodes&)GetNodes() ));
            if( GotoOutline( aPos, sName ))
            {
                SwNode* pNd = &aPos.nNode.GetNode();
                //sal_uInt8 nLvl = pNd->GetTxtNode()->GetTxtColl()->GetOutlineLevel();//#outline level,zhaojianwei
                const int nLvl = pNd->GetTxtNode()->GetAttrOutlineLevel()-1;//<-end,zhaojianwei

                const SwOutlineNodes& rOutlNds = GetNodes().GetOutLineNds();
                sal_uInt16 nTmpPos;
                rOutlNds.Seek_Entry( pNd, &nTmpPos );
                rpRange = new SwNodeRange( aPos.nNode, 0, aPos.nNode );

                // look for the section's end, now
                for( ++nTmpPos;
                        nTmpPos < rOutlNds.size() &&
                        nLvl < rOutlNds[ nTmpPos ]->GetTxtNode()->
                                //GetTxtColl()->GetOutlineLevel();//#outline level,zhaojianwei
                                GetAttrOutlineLevel()-1;//<-end,zhaojianwei
                    ++nTmpPos )
                    ;       // there is no block

                if( nTmpPos < rOutlNds.size() )
                    rpRange->aEnd = *rOutlNds[ nTmpPos ];
                else
                    rpRange->aEnd = GetNodes().GetEndOfContent();
                return sal_True;
            }
        }

        if( !bWeiter )
            return sal_False;
    }

    // search for bookmarks and sections case senstive at first. If nothing is found then try again case insensitive
    bool bCaseSensitive = true;
    while( true )
    {
        ::sw::mark::DdeBookmark* const pBkmk = lcl_FindDdeBookmark(*pMarkManager, sItem, bCaseSensitive);
        if(pBkmk)
        {
            if(pBkmk->IsExpanded())
                rpPam = new SwPaM(
                    pBkmk->GetMarkPos(),
                    pBkmk->GetOtherMarkPos());
            return static_cast<bool>(rpPam);
        }

        //
        _FindItem aPara( bCaseSensitive ? sItem : String(rCC.lowercase( sItem )) );

        if( !pSectionFmtTbl->empty() )
        {
            BOOST_FOREACH( const SwSectionFmt* pFmt, *pSectionFmtTbl )
            {
                if (!(lcl_FindSection(pFmt, &aPara, bCaseSensitive)))
                    break;
            }
            if( aPara.pSectNd )
            {
                rpRange = new SwNodeRange( *aPara.pSectNd, 1,
                                        *aPara.pSectNd->EndOfSectionNode() );
                return sal_True;

            }
        }
        if( !bCaseSensitive )
            break;
        bCaseSensitive = false;
    }
    return sal_False;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

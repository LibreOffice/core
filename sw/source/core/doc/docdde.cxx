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
#include "precompiled_sw.hxx"



#include <stdlib.h>

#ifndef _APP_HXX
#include <vcl/svapp.hxx>
#endif
#include <tools/urlobj.hxx>

#define _SVSTDARR_STRINGS
#include <svl/svstdarr.hxx>
#include <sfx2/linkmgr.hxx>         // LinkManager
#include <unotools/charclass.hxx>
#include <fmtcntnt.hxx>
#include <doc.hxx>
#include <swserv.hxx>           // fuer Server-Funktionalitaet
#include <IMark.hxx>
#include <bookmrk.hxx>
#include <section.hxx>          // fuer SwSectionFmt
#include <swtable.hxx>          // fuer SwTable
#include <node.hxx>
#include <ndtxt.hxx>
#include <pam.hxx>
#include <docary.hxx>
#include <MarkManager.hxx>

using namespace ::com::sun::star;

namespace
{

    static ::sw::mark::DdeBookmark* lcl_FindDdeBookmark(const IDocumentMarkAccess& rMarkAccess, const String& rName, bool bCaseSensitive)
    {
        //Iterating over all bookmarks, checking DdeBookmarks
        const ::rtl::OUString sNameLc = bCaseSensitive ? rName : GetAppCharClass().lower(rName);
        for(IDocumentMarkAccess::const_iterator_t ppMark = rMarkAccess.getMarksBegin();
            ppMark != rMarkAccess.getMarksEnd();
            ppMark++)
        {
            if (::sw::mark::DdeBookmark* const pBkmk = dynamic_cast< ::sw::mark::DdeBookmark*>(ppMark->get()))
            {
                if (
                    (bCaseSensitive && (pBkmk->GetName() == sNameLc)) ||
                    (!bCaseSensitive && GetAppCharClass().lower(pBkmk->GetName()) == String(sNameLc))
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

sal_Bool lcl_FindSection( const SwSectionFmtPtr& rpSectFmt, void* pArgs, bool bCaseSensitive )
{
    _FindItem * const pItem( static_cast<_FindItem*>(pArgs) );
    SwSection* pSect = rpSectFmt->GetSection();
    if( pSect )
    {
        String sNm( (bCaseSensitive)
                ? pSect->GetSectionName()
                : GetAppCharClass().lower( pSect->GetSectionName() ));
        String sCompare( (bCaseSensitive)
                ? pItem->m_Item
                : GetAppCharClass().lower( pItem->m_Item ) );
        if( sNm == sCompare )
        {
            // gefunden, als erfrage die Daten
            const SwNodeIndex* pIdx;
            if( 0 != (pIdx = rpSectFmt->GetCntnt().GetCntntIdx() ) &&
                &rpSectFmt->GetDoc()->GetNodes() == &pIdx->GetNodes() )
            {
                // eine Tabelle im normalen NodesArr
                pItem->pSectNd = pIdx->GetNode().GetSectionNode();
                return sal_False;
            }
//nein!!            // sollte der Namen schon passen, der Rest aber nicht, dann haben wir
            // sie nicht. Die Namen sind immer eindeutig.
        }
    }
    return sal_True;        // dann weiter
}
sal_Bool lcl_FindSectionCaseSensitive( const SwSectionFmtPtr& rpSectFmt, void* pArgs )
{
    return lcl_FindSection( rpSectFmt, pArgs, true );
}
sal_Bool lcl_FindSectionCaseInsensitive( const SwSectionFmtPtr& rpSectFmt, void* pArgs )
{
    return lcl_FindSection( rpSectFmt, pArgs, false );
}



sal_Bool lcl_FindTable( const SwFrmFmtPtr& rpTableFmt, void* pArgs )
{
    _FindItem * const pItem( static_cast<_FindItem*>(pArgs) );
    String sNm( GetAppCharClass().lower( rpTableFmt->GetName() ));
    if (sNm.Equals( pItem->m_Item ))
    {
        SwTable* pTmpTbl;
        SwTableBox* pFBox;
        if( 0 != ( pTmpTbl = SwTable::FindTable( rpTableFmt ) ) &&
            0 != ( pFBox = pTmpTbl->GetTabSortBoxes()[0] ) &&
            pFBox->GetSttNd() &&
            &rpTableFmt->GetDoc()->GetNodes() == &pFBox->GetSttNd()->GetNodes() )
        {
            // eine Tabelle im normalen NodesArr
            pItem->pTblNd = (SwTableNode*)
                                        pFBox->GetSttNd()->FindTableNode();
            return sal_False;
        }
//nein!     // sollte der Namen schon passen, der Rest aber nicht, dann haben wir
        // sie nicht. Die Namen sind immer eindeutig.
    }
    return sal_True;        // dann weiter
}



bool SwDoc::GetData( const String& rItem, const String& rMimeType,
                     uno::Any & rValue ) const
{
    //search for bookmarks and sections case senstive at first. If nothing is found then try again case insensitive
    bool bCaseSensitive = true;
    while( true )
    {
        ::sw::mark::DdeBookmark* const pBkmk = lcl_FindDdeBookmark(*pMarkManager, rItem, bCaseSensitive);
        if(pBkmk)
            return SwServerObject(*pBkmk).GetData(rValue, rMimeType);

        // haben wir ueberhaupt das Item vorraetig?
        String sItem( bCaseSensitive ? rItem : GetAppCharClass().lower(rItem));
        _FindItem aPara( sItem );
        ((SwSectionFmts&)*pSectionFmtTbl).ForEach( 0, pSectionFmtTbl->Count(),
                                                    bCaseSensitive ? lcl_FindSectionCaseSensitive : lcl_FindSectionCaseInsensitive, &aPara );
        if( aPara.pSectNd )
        {
            // gefunden, als erfrage die Daten
            return SwServerObject( *aPara.pSectNd ).GetData( rValue, rMimeType );
        }
        if( !bCaseSensitive )
            break;
        bCaseSensitive = false;
    }

    _FindItem aPara( GetAppCharClass().lower( rItem ));
    ((SwFrmFmts*)pTblFrmFmtTbl)->ForEach( 0, pTblFrmFmtTbl->Count(),
                                            lcl_FindTable, &aPara );
    if( aPara.pTblNd )
    {
        return SwServerObject( *aPara.pTblNd ).GetData( rValue, rMimeType );
    }

    return sal_False;
}



bool SwDoc::SetData( const String& rItem, const String& rMimeType,
                     const uno::Any & rValue )
{
    //search for bookmarks and sections case senstive at first. If nothing is found then try again case insensitive
    bool bCaseSensitive = true;
    while( true )
    {
        ::sw::mark::DdeBookmark* const pBkmk = lcl_FindDdeBookmark(*pMarkManager, rItem, bCaseSensitive);
        if(pBkmk)
            return SwServerObject(*pBkmk).SetData(rMimeType, rValue);

        // haben wir ueberhaupt das Item vorraetig?
        String sItem( bCaseSensitive ? rItem : GetAppCharClass().lower(rItem));
        _FindItem aPara( sItem );
        pSectionFmtTbl->ForEach( 0, pSectionFmtTbl->Count(), bCaseSensitive ? lcl_FindSectionCaseSensitive : lcl_FindSectionCaseInsensitive, &aPara );
        if( aPara.pSectNd )
        {
            // gefunden, als erfrage die Daten
            return SwServerObject( *aPara.pSectNd ).SetData( rMimeType, rValue );
        }
        if( !bCaseSensitive )
            break;
        bCaseSensitive = false;
    }

    String sItem(GetAppCharClass().lower(rItem));
    _FindItem aPara( sItem );
    pTblFrmFmtTbl->ForEach( 0, pTblFrmFmtTbl->Count(), lcl_FindTable, &aPara );
    if( aPara.pTblNd )
    {
        return SwServerObject( *aPara.pTblNd ).SetData( rMimeType, rValue );
    }

    return sal_False;
}



::sfx2::SvLinkSource* SwDoc::CreateLinkSource(const String& rItem)
{
    SwServerObject* pObj = NULL;

    //search for bookmarks and sections case senstive at first. If nothing is found then try again case insensitive
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

        _FindItem aPara(bCaseSensitive ? rItem : GetAppCharClass().lower(rItem));
        // sections
        ((SwSectionFmts&)*pSectionFmtTbl).ForEach(0, pSectionFmtTbl->Count(), bCaseSensitive ? lcl_FindSectionCaseSensitive : lcl_FindSectionCaseInsensitive, &aPara);
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

    _FindItem aPara( GetAppCharClass().lower(rItem) );
    // tables
    ((SwFrmFmts*)pTblFrmFmtTbl)->ForEach(0, pTblFrmFmtTbl->Count(), lcl_FindTable, &aPara);
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
    // haben wir ueberhaupt das Item vorraetig?
    rpPam = 0;
    rpRange = 0;

    String sItem( INetURLObject::decode( rStr, INET_HEX_ESCAPE,
                                         INetURLObject::DECODE_WITH_CHARSET,
                                        RTL_TEXTENCODING_UTF8 ));

    xub_StrLen nPos = sItem.Search( cMarkSeperator );

    const CharClass& rCC = GetAppCharClass();

    // Erweiterung fuer die Bereiche, nicht nur Bookmarks/Bereiche linken,
    // sondern auch Rahmen(Text!), Tabellen, Gliederungen:
    if( STRING_NOTFOUND != nPos )
    {
        sal_Bool bWeiter = sal_False;
        String sName( sItem.Copy( 0, nPos ) );
        String sCmp( sItem.Copy( nPos + 1 ));
        rCC.toLower( sItem );

        _FindItem aPara( sName );

        if( sCmp.EqualsAscii( pMarkToTable ) )
        {
            rCC.toLower( sName );
            ((SwFrmFmts*)pTblFrmFmtTbl)->ForEach( 0, pTblFrmFmtTbl->Count(),
                                                    lcl_FindTable, &aPara );
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
            sItem = sName;              // wird unten behandelt !
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

                // dann suche jetzt noch das Ende vom Bereich
                for( ++nTmpPos;
                        nTmpPos < rOutlNds.Count() &&
                        nLvl < rOutlNds[ nTmpPos ]->GetTxtNode()->
                                //GetTxtColl()->GetOutlineLevel();//#outline level,zhaojianwei
                                GetAttrOutlineLevel()-1;//<-end,zhaojianwei
                    ++nTmpPos )
                    ;       // es gibt keinen Block

                if( nTmpPos < rOutlNds.Count() )
                    rpRange->aEnd = *rOutlNds[ nTmpPos ];
                else
                    rpRange->aEnd = GetNodes().GetEndOfContent();
                return sal_True;
            }
        }

        if( !bWeiter )
            return sal_False;
    }

    //search for bookmarks and sections case senstive at first. If nothing is found then try again case insensitive
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
        _FindItem aPara( bCaseSensitive ? sItem : rCC.lower( sItem ) );

        if( pSectionFmtTbl->Count() )
        {
            ((SwSectionFmts&)*pSectionFmtTbl).ForEach( 0, pSectionFmtTbl->Count(),
                                                    bCaseSensitive ? lcl_FindSectionCaseSensitive : lcl_FindSectionCaseInsensitive, &aPara );
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


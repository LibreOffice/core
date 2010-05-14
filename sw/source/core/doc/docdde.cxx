/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: docdde.cxx,v $
 * $Revision: 1.14 $
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
#include <svtools/svstdarr.hxx>
#include <svx/linkmgr.hxx>          // LinkManager
#include <unotools/charclass.hxx>
#include <fmtcntnt.hxx>
#include <doc.hxx>
#include <swserv.hxx>           // fuer Server-Funktionalitaet
#include <bookmrk.hxx>          // fuer die Bookmarks
#include <section.hxx>          // fuer SwSectionFmt
#include <swtable.hxx>          // fuer SwTable
#include <node.hxx>
#include <ndtxt.hxx>
#include <pam.hxx>
#include <docary.hxx>

using namespace ::com::sun::star;

struct _FindItem
{
    const String& rItem;
    SwBookmark* pBkmk;
    SwTableNode* pTblNd;
    SwSectionNode* pSectNd;

    _FindItem( const String& rS )
        : rItem( rS ), pBkmk( 0 ), pTblNd( 0 ), pSectNd( 0 )
    {}

    void ClearObj()
    {
        if( pBkmk )
            pBkmk->SetRefObject( 0 );
        else if( pSectNd )
            pSectNd->GetSection().SetRefObject( 0 );
        else if( pTblNd )
            pTblNd->GetTable().SetRefObject( 0 );
    }
};


BOOL lcl_FindBookmark( const SwBookmarkPtr& rpBkmk, void* pArgs )
{
    BOOL bRet = TRUE;
    String sNm( GetAppCharClass().lower( rpBkmk->GetName() ));
    if( sNm.Equals( ((_FindItem*)pArgs)->rItem ) )
    {
        ((_FindItem*)pArgs)->pBkmk = rpBkmk;
        bRet = FALSE;
    }

    return bRet;
}



BOOL lcl_FindSection( const SwSectionFmtPtr& rpSectFmt, void* pArgs )
{
    SwSection* pSect = rpSectFmt->GetSection();
    if( pSect )
    {
        String sNm( GetAppCharClass().lower( pSect->GetName() ));
        if( sNm.Equals( ((_FindItem*)pArgs)->rItem ))
        {
            // gefunden, als erfrage die Daten
            const SwNodeIndex* pIdx;
            if( 0 != (pIdx = rpSectFmt->GetCntnt().GetCntntIdx() ) &&
                &rpSectFmt->GetDoc()->GetNodes() == &pIdx->GetNodes() )
            {
                // eine Tabelle im normalen NodesArr
                ((_FindItem*)pArgs)->pSectNd = pIdx->GetNode().GetSectionNode();
                return FALSE;
            }
//nein!!            // sollte der Namen schon passen, der Rest aber nicht, dann haben wir
            // sie nicht. Die Namen sind immer eindeutig.
        }
    }
    return TRUE;        // dann weiter
}



BOOL lcl_FindTable( const SwFrmFmtPtr& rpTableFmt, void* pArgs )
{
    String sNm( GetAppCharClass().lower( rpTableFmt->GetName() ));
    if( sNm.Equals( ((_FindItem*)pArgs)->rItem ))
    {
        SwTable* pTmpTbl;
        SwTableBox* pFBox;
        if( 0 != ( pTmpTbl = SwTable::FindTable( rpTableFmt ) ) &&
            0 != ( pFBox = pTmpTbl->GetTabSortBoxes()[0] ) &&
            pFBox->GetSttNd() &&
            &rpTableFmt->GetDoc()->GetNodes() == &pFBox->GetSttNd()->GetNodes() )
        {
            // eine Tabelle im normalen NodesArr
            ((_FindItem*)pArgs)->pTblNd = (SwTableNode*)
                                        pFBox->GetSttNd()->FindTableNode();
            return FALSE;
        }
//nein!     // sollte der Namen schon passen, der Rest aber nicht, dann haben wir
        // sie nicht. Die Namen sind immer eindeutig.
    }
    return TRUE;        // dann weiter
}



bool SwDoc::GetData( const String& rItem, const String& rMimeType,
                     uno::Any & rValue ) const
{
    // haben wir ueberhaupt das Item vorraetig?
    String sItem( GetAppCharClass().lower( rItem ));
    _FindItem aPara( sItem );
    ((SwBookmarks&)*pBookmarkTbl).ForEach( 0, pBookmarkTbl->Count(),
                                            lcl_FindBookmark, &aPara );
    if( aPara.pBkmk )
    {
        // gefunden, als erfrage die Daten
        return SwServerObject( *aPara.pBkmk ).GetData( rValue, rMimeType );
    }

    ((SwSectionFmts&)*pSectionFmtTbl).ForEach( 0, pSectionFmtTbl->Count(),
                                                lcl_FindSection, &aPara );
    if( aPara.pSectNd )
    {
        // gefunden, als erfrage die Daten
        return SwServerObject( *aPara.pSectNd ).GetData( rValue, rMimeType );
    }

    ((SwFrmFmts*)pTblFrmFmtTbl)->ForEach( 0, pTblFrmFmtTbl->Count(),
                                            lcl_FindTable, &aPara );
    if( aPara.pTblNd )
    {
        return SwServerObject( *aPara.pTblNd ).GetData( rValue, rMimeType );
    }

    return FALSE;
}



bool SwDoc::SetData( const String& rItem, const String& rMimeType,
                     const uno::Any & rValue )
{
    // haben wir ueberhaupt das Item vorraetig?
    String sItem( GetAppCharClass().lower( rItem ));
    _FindItem aPara( sItem );
    pBookmarkTbl->ForEach( 0, pBookmarkTbl->Count(), lcl_FindBookmark, &aPara );
    if( aPara.pBkmk )
    {
        // gefunden, als erfrage die Daten
        return SwServerObject( *aPara.pBkmk ).SetData( rMimeType, rValue );
    }

    pSectionFmtTbl->ForEach( 0, pSectionFmtTbl->Count(), lcl_FindSection, &aPara );
    if( aPara.pSectNd )
    {
        // gefunden, als erfrage die Daten
        return SwServerObject( *aPara.pSectNd ).SetData( rMimeType, rValue );
    }

    pTblFrmFmtTbl->ForEach( 0, pTblFrmFmtTbl->Count(), lcl_FindTable, &aPara );
    if( aPara.pTblNd )
    {
        return SwServerObject( *aPara.pTblNd ).SetData( rMimeType, rValue );
    }

    return FALSE;
}



::sfx2::SvLinkSource* SwDoc::CreateLinkSource( const String& rItem )
{
    // haben wir ueberhaupt das Item vorraetig?
    String sItem( GetAppCharClass().lower( rItem ));
    _FindItem aPara( sItem );

    SwServerObject* pObj;

    do {    // middle check Loop
        ((SwBookmarks&)*pBookmarkTbl).ForEach( 0, pBookmarkTbl->Count(),
                                                lcl_FindBookmark, &aPara );
        if( aPara.pBkmk && aPara.pBkmk->GetOtherBookmarkPos() )
        {
            // gefunden, also Hotlink einrichten
            // sollten wir schon einer sein?
            if( 0 == (pObj = aPara.pBkmk->GetObject()) )
            {
                pObj = new SwServerObject( *aPara.pBkmk );
                aPara.pBkmk->SetRefObject( pObj );
            }
            else if( pObj->HasDataLinks() )
                return pObj;
            break;
        }

        ((SwSectionFmts&)*pSectionFmtTbl).ForEach( 0, pSectionFmtTbl->Count(),
                                                    lcl_FindSection, &aPara );
        if( aPara.pSectNd )
        {
            // gefunden, also Hotlink einrichten
            // sollten wir schon einer sein?
            if( 0 == (pObj = aPara.pSectNd->GetSection().GetObject()) )
            {
                pObj = new SwServerObject( *aPara.pSectNd );
                aPara.pSectNd->GetSection().SetRefObject( pObj );
            }
            else if( pObj->HasDataLinks() )
                return pObj;
            break;
        }

        ((SwFrmFmts*)pTblFrmFmtTbl)->ForEach( 0, pTblFrmFmtTbl->Count(),
                                                lcl_FindTable, &aPara );
        if( aPara.pTblNd )
        {
            // gefunden, also Hotlink einrichten
            // sollten wir schon einer sein?
            if( 0 == (pObj = aPara.pTblNd->GetTable().GetObject()) )
            {
                pObj = new SwServerObject( *aPara.pTblNd );
                aPara.pTblNd->GetTable().SetRefObject( pObj );
            }
            else if( pObj->HasDataLinks() )
                return pObj;
            break;
        }
        // bis hierhin, also nicht vorhanden
        return 0;
    } while( FALSE );

    // neu angelegt also ab in die Verwaltung
    GetLinkManager().InsertServer( pObj );
    return pObj;
}



BOOL SwDoc::SelectServerObj( const String& rStr, SwPaM*& rpPam,
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
        BOOL bWeiter = FALSE;
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
                return TRUE;
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
                return TRUE;
            }
        }
        else if( sCmp.EqualsAscii( pMarkToRegion ) )
        {
            sItem = sName;              // wird unten behandelt !
            bWeiter = TRUE;
        }
        else if( sCmp.EqualsAscii( pMarkToOutline ) )
        {
            SwPosition aPos( SwNodeIndex( (SwNodes&)GetNodes() ));
            if( GotoOutline( aPos, sName ))
            {
                SwNode* pNd = &aPos.nNode.GetNode();
                //BYTE nLvl = pNd->GetTxtNode()->GetTxtColl()->GetOutlineLevel();//#outline level,zhaojianwei
                const int nLvl = pNd->GetTxtNode()->GetAttrOutlineLevel()-1;//<-end,zhaojianwei

                const SwOutlineNodes& rOutlNds = GetNodes().GetOutLineNds();
                USHORT nTmpPos;
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
                return TRUE;
            }
        }

        if( !bWeiter )
            return FALSE;
    }

    // alte "Mechanik"
    rCC.toLower( sItem );
    _FindItem aPara( sItem );
    if( pBookmarkTbl->Count() )
    {
        ((SwBookmarks&)*pBookmarkTbl).ForEach( 0, pBookmarkTbl->Count(),
                                            lcl_FindBookmark, &aPara );
        if( aPara.pBkmk )
        {
            // gefunden, also erzeuge einen Bereich
            if( aPara.pBkmk->GetOtherBookmarkPos() )
                // ein aufgespannter Bereich
                rpPam = new SwPaM( aPara.pBkmk->GetBookmarkPos(),
                                    *aPara.pBkmk->GetOtherBookmarkPos() );
            return 0 != rpPam;
        }
    }

    if( pSectionFmtTbl->Count() )
    {
        ((SwSectionFmts&)*pSectionFmtTbl).ForEach( 0, pSectionFmtTbl->Count(),
                                                lcl_FindSection, &aPara );
        if( aPara.pSectNd )
        {
            rpRange = new SwNodeRange( *aPara.pSectNd, 1,
                                    *aPara.pSectNd->EndOfSectionNode() );
            return TRUE;

        }
    }
    return FALSE;
}

/*************************************************************************
 *
 *  $RCSfile: htmlatr.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:55 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/


#ifdef PRECOMPILED
#include "filt_pch.hxx"
#endif

#pragma hdrstop

#define ITEMID_BOXINFO      SID_ATTR_BORDER_INNER
#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#ifndef _WRKWIN_HXX //autogen
#include <vcl/wrkwin.hxx>
#endif
#ifndef _URLOBJ_HXX //autogen
#include <tools/urlobj.hxx>
#endif

#ifndef _HTMLOUT_HXX //autogen
#include <svtools/htmlout.hxx>
#endif
#ifndef _HTMLKYWD_HXX
#include <svtools/htmlkywd.hxx>
#endif
#ifndef _HTMLTOKN_H
#include <svtools/htmltokn.h>
#endif
#ifndef _SFX_WHITER_HXX //autogen
#include <svtools/whiter.hxx>
#endif
#ifndef _SFXMACITEM_HXX //autogen
#include <svtools/macitem.hxx>
#endif

#ifndef _SVX_HTMLMODE_HXX
#include <svx/htmlmode.hxx>
#endif
#ifndef _SVX_ESCPITEM_HXX //autogen
#include <svx/escpitem.hxx>
#endif
#ifndef _SVX_BRKITEM_HXX //autogen
#include <svx/brkitem.hxx>
#endif
#ifndef _SVX_BOXITEM_HXX //autogen
#include <svx/boxitem.hxx>
#endif
#ifndef _SVX_ULSPITEM_HXX //autogen
#include <svx/ulspitem.hxx>
#endif
#ifndef _SVX_UDLNITEM_HXX //autogen
#include <svx/udlnitem.hxx>
#endif
#ifndef _SVX_CRSDITEM_HXX //autogen
#include <svx/crsditem.hxx>
#endif
#ifndef _SVX_BLNKITEM_HXX //autogen
#include <svx/blnkitem.hxx>
#endif
#ifndef _SVX_CMAPITEM_HXX //autogen
#include <svx/cmapitem.hxx>
#endif
#ifndef _SVX_COLRITEM_HXX //autogen
#include <svx/colritem.hxx>
#endif
#ifndef _SVX_FONTITEM_HXX //autogen
#include <svx/fontitem.hxx>
#endif
#ifndef _SVX_FHGTITEM_HXX //autogen
#include <svx/fhgtitem.hxx>
#endif
#ifndef _SVX_POSTITEM_HXX //autogen
#include <svx/postitem.hxx>
#endif
#ifndef _SVX_KERNITEM_HXX //autogen
#include <svx/kernitem.hxx>
#endif
#ifndef _SVX_WGHTITEM_HXX //autogen
#include <svx/wghtitem.hxx>
#endif
#ifndef _SVX_LSPCITEM_HXX //autogen
#include <svx/lspcitem.hxx>
#endif
#ifndef _SVX_ADJITEM_HXX //autogen
#include <svx/adjitem.hxx>
#endif
#ifndef _SVX_LRSPITEM_HXX //autogen
#include <svx/lrspitem.hxx>
#endif
#ifndef _SVX_BRSHITEM_HXX //autogen
#include <svx/brshitem.hxx>
#endif


#ifndef _FCHRFMT_HXX //autogen
#include <fchrfmt.hxx>
#endif
#ifndef _FMTFSIZE_HXX //autogen
#include <fmtfsize.hxx>
#endif
#ifndef _FMTCLDS_HXX //autogen
#include <fmtclds.hxx>
#endif
#ifndef _FMTPDSC_HXX //autogen
#include <fmtpdsc.hxx>
#endif
#ifndef _FMTFLCNT_HXX //autogen
#include <fmtflcnt.hxx>
#endif
#ifndef _FMTINFMT_HXX //autogen
#include <fmtinfmt.hxx>
#endif
#ifndef _FMTFTN_HXX //autogen
#include <fmtftn.hxx>
#endif
#ifndef _TXATBASE_HXX //autogen
#include <txatbase.hxx>
#endif
#ifndef _FRMATR_HXX
#include <frmatr.hxx>
#endif
#ifndef _CHARFMT_HXX //autogen
#include <charfmt.hxx>
#endif
#ifndef _FMTHBSH_HXX //autogen
#include <fmthbsh.hxx>
#endif

#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _PAM_HXX
#include <pam.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _SWERROR_H
#include <swerror.h>
#endif
#ifndef _PARATR_HXX
#include <paratr.hxx>
#endif

#ifndef _FMTCOL_HXX
#include <fmtcol.hxx>
#endif
#ifndef _POOLFMT_HXX
#include <poolfmt.hxx>
#endif
#ifndef _PAGEDESC_HXX
#include <pagedesc.hxx>
#endif
#ifndef _SWTABLE_HXX
#include <swtable.hxx>
#endif

#ifndef _WRTTXATR_HXX
#include <wrttxatr.hxx>
#endif
#ifndef _HTMLNUM_HXX
#include <htmlnum.hxx>
#endif
#ifndef _WRTHTML_HXX
#include <wrthtml.hxx>
#endif
#ifndef _HTMLFLY_HXX
#include <htmlfly.hxx>
#endif


/*
 * um nicht immer wieder nach einem Update festzustellen, das irgendwelche
 * Hint-Ids dazugekommen sind, wird hier definiert, die Groesse der Tabelle
 * definiert und mit der akt. verglichen. Bei unterschieden wird der
 * Compiler schon meckern.
 *
 * diese Section und die dazugeherigen Tabellen muessen in folgenden Files
 * gepflegt werden: rtf\rtfatr.cxx, sw6\sw6atr.cxx, w4w\w4watr.cxx
 */
#if !defined(UNX) && !defined(MSC) && !defined(PPC) && !defined(CSET) && !defined(__MWERKS__) && !defined(WTC)

#define ATTRFNTAB_SIZE 121
#if ATTRFNTAB_SIZE != POOLATTR_END - POOLATTR_BEGIN
#error Attribut-Tabelle ist ungueltigt. Wurden neue Hint-IDs zugefuegt ??
#endif

#ifdef FORMAT_TABELLE
// da sie nicht benutzt wird!
#define FORMATTAB_SIZE 7
#if FORMATTAB_SIZE != RES_FMT_END - RES_FMT_BEGIN
#error Format-Tabelle ist ungueltigt. Wurden neue Hint-IDs zugefuegt ??
#endif
#endif

#define NODETAB_SIZE 3
#if NODETAB_SIZE != RES_NODE_END - RES_NODE_BEGIN
#error Node-Tabelle ist ungueltigt. Wurden neue Hint-IDs zugefuegt ??
#endif

#endif

#define HTML_BULLETCHAR_DISC    34
#define HTML_BULLETCHAR_CIRCLE  38
#define HTML_BULLETCHAR_SQUARE  36

#define COLFUZZY 20

//-----------------------------------------------------------------------

HTMLOutEvent __FAR_DATA aAnchorEventTable[] =
{
    { sHTML_O_SDonclick,        sHTML_O_onclick,        SFX_EVENT_MOUSECLICK_OBJECT },
    { sHTML_O_SDonmouseover,    sHTML_O_onmouseover,    SFX_EVENT_MOUSEOVER_OBJECT  },
    { sHTML_O_SDonmouseout,     sHTML_O_onmouseout,     SFX_EVENT_MOUSEOUT_OBJECT   },
    { 0,                        0,                      0                           }
};

static Writer& OutHTML_SvxAdjust( Writer& rWrt, const SfxPoolItem& rHt );

static Writer& OutHTML_HoriSpacer( Writer& rWrt, INT16 nSize )
{
    ASSERT( nSize>0, "horizontaler SPACER mit negativem Wert?" )
    if( nSize <= 0 )
        return rWrt;

    if( Application::GetDefaultDevice() )
    {
        nSize = (INT16)Application::GetDefaultDevice()
            ->LogicToPixel( Size(nSize,0), MapMode(MAP_TWIP) ).Width();
    }

    ByteString sOut( '<' );
    (((((((((sOut += sHTML_spacer)
        += ' ') += sHTML_O_type) += '=') += sHTML_SPTYPE_horizontal)
        += ' ') += sHTML_O_size) += '=')
                        +=ByteString::CreateFromInt32(nSize)) += '>';

    rWrt.Strm() << sOut.GetBuffer();

    return rWrt;
}

USHORT SwHTMLWriter::GetDefListLvl( const String& rNm, USHORT nPoolId )
{
    if( nPoolId == RES_POOLCOLL_HTML_DD )
    {
        return 1 | HTML_DLCOLL_DD;
    }
    else if( nPoolId == RES_POOLCOLL_HTML_DT )
    {
        return 1 | HTML_DLCOLL_DT;
    }

    String sDTDD( String::CreateFromAscii(sHTML_dt) );
    sDTDD += ' ';
    if( COMPARE_EQUAL == sDTDD.CompareTo( rNm, sDTDD.Len() ) )
        // DefinitionList - term
        return (USHORT)rNm.Copy( sDTDD.Len() ).ToInt32() | HTML_DLCOLL_DT;

    sDTDD.AssignAscii( sHTML_dd );
    sDTDD += ' ';
    if( COMPARE_EQUAL == sDTDD.CompareTo( rNm, sDTDD.Len() ) )
        // DefinitionList - definition
        return (USHORT)rNm.Copy( sDTDD.Len() ).ToInt32() | HTML_DLCOLL_DD;

    return 0;
}

void SwHTMLWriter::OutAndSetDefList( USHORT nNewLvl )
{
    // eventuell muss erst mal eine Liste aufgemacht werden
    if( nDefListLvl < nNewLvl )
    {
        // entsprechend dem Level-Unterschied schreiben!
        for( USHORT i=nDefListLvl; i<nNewLvl; i++ )
        {
            if( bLFPossible )
                OutNewLine();
            HTMLOutFuncs::Out_AsciiTag( Strm(), sHTML_deflist, TRUE );
            IncIndentLevel();
            bLFPossible = TRUE;
        }
    }
    else if( nDefListLvl > nNewLvl )
    {
        for( USHORT i=nNewLvl ; i < nDefListLvl; i++ )
        {
            DecIndentLevel();
            if( bLFPossible )
                OutNewLine();
            HTMLOutFuncs::Out_AsciiTag( Strm(), sHTML_deflist, FALSE );
            bLFPossible = TRUE;
        }
    }

    nDefListLvl = nNewLvl;
}


void SwHTMLWriter::ChangeParaToken( USHORT nNew )
{
    if( nNew != nLastParaToken && HTML_PREFORMTXT_ON == nLastParaToken )
    {
        HTMLOutFuncs::Out_AsciiTag( Strm(), sHTML_preformtxt, FALSE );
        bLFPossible = TRUE;
    }
    nLastParaToken = nNew;
}

// fuer die Formate muesste eine einzige Ausgabe-Funktion genuegen !
/*
 * Formate wie folgt ausgeben:
 * - fuer Formate, fuer die es entsprechende HTML-Tags gibt wird das
 *   Tag ausgegeben
 * - fuer alle anderen wird ein Absatz-Tag <P> ausgegeben und bUserFmt
 *   gesetzt
 * - Wenn eine Absatz-Ausrichtung am uebergebenen Item-Set des Nodes
 *   oder im Item-Set des Format gesetzt ist, wird ein ALIGN=xxx ausgegeben,
 *   sofern HTML es zulaesst
 * - in jedem Fall wird harte Attributierung als STYLE-Option geschrieben.
 *   Wenn bUserFmt nicht gesetzt ist, wird nur der uebergebene Item-Set
 *   betrachtet. Sonst werden auch Attribute des Formats ausgegeben.
 */

struct SwHTMLTxtCollOutputInfo
{
    ByteString aToken;              // auszugendens End-Token
    SfxItemSet *pItemSet;       // harte Attributierung

    BOOL bInNumBulList;         // in einer Aufzaehlungs-Liste;
    BOOL bParaPossible;         // ein </P> darf zusaetzlich ausgegeben werden
    BOOL bOutPara;              // ein </P> soll ausgegeben werden
    BOOL bOutDiv;               // write a </DIV>

    SwHTMLTxtCollOutputInfo() :
        pItemSet( 0 ),
        bInNumBulList( FALSE ),
        bParaPossible( FALSE ),
        bOutPara( FALSE ),
        bOutDiv( FALSE )
    {}

    ~SwHTMLTxtCollOutputInfo();

    BOOL HasParaToken() const { return aToken.Len()==1 && aToken.GetChar(0)=='P'; }
    BOOL ShouldOutputToken() const { return bOutPara || !HasParaToken(); }
};

SwHTMLTxtCollOutputInfo::~SwHTMLTxtCollOutputInfo()
{
    delete pItemSet;
}

struct SwHTMLFmtInfo
{
    const SwFmt *pFmt;      // das Format selbst
    const SwFmt *pRefFmt;   // das Vergleichs-Format

    ByteString aToken;          // das auszugebende Token
    String aClass;          // die auszugebende Klasse

    SfxItemSet *pItemSet;   // der auszugebende Attribut-Set

    USHORT nLeftMargin;     // ein par default-Werte fuer
    USHORT nRightMargin;    // Absatz-Vorlagen
    short nFirstLineIndent;

    USHORT nTopMargin;
    USHORT nBottomMargin;

    // Konstruktor fuer einen Dummy zum Suchen
    SwHTMLFmtInfo( const SwFmt *pF ) :
        pFmt( pF ), pItemSet( 0 )
    {}


    // Konstruktor zum Erstellen der Format-Info
    SwHTMLFmtInfo( const SwFmt *pFmt, SwDoc *pDoc, SwDoc *pTemlate,
                   BOOL bOutStyles, BOOL bHardDrop=FALSE );
    ~SwHTMLFmtInfo();

    friend BOOL operator==( const SwHTMLFmtInfo& rInfo1,
                            const SwHTMLFmtInfo& rInfo2 )
    {
        return (long)rInfo1.pFmt == (long)rInfo2.pFmt;
    }

    friend BOOL operator<( const SwHTMLFmtInfo& rInfo1,
                            const SwHTMLFmtInfo& rInfo2 )
    {
        return (long)rInfo1.pFmt < (long)rInfo2.pFmt;
    }

};

SV_IMPL_OP_PTRARR_SORT( SwHTMLFmtInfos, SwHTMLFmtInfo* )

SwHTMLFmtInfo::SwHTMLFmtInfo( const SwFmt *pF, SwDoc *pDoc, SwDoc *pTemplate,
                              BOOL bOutStyles, BOOL bHardDrop ) :
    pFmt( pF ), pItemSet( 0 )
{
    USHORT nRefPoolId = 0;
    // Den Selektor des Formats holen
    USHORT nDeep = SwHTMLWriter::GetCSS1Selector( pFmt, aToken, aClass,
                                                  nRefPoolId );
    ASSERT( nDeep ? aToken.Len()>0 : aToken.Len()==0,
            "Hier stimmt doch was mit dem Token nicht!" );
    ASSERT( nDeep ? nRefPoolId : !nRefPoolId,
            "Hier stimmt doch was mit der Vergleichs-Vorlage nicht!" );

    BOOL bTxtColl = pFmt->Which() == RES_TXTFMTCOLL ||
                    pFmt->Which() == RES_CONDTXTFMTCOLL;

    const SwFmt *pRefFmt = 0;   // Vergleichs-Format
    if( nDeep != 0 )
    {
        // Es ist eine HTML-Tag-Vorlage oder die Vorlage ist von einer
        // solchen abgeleitet
        if( !bOutStyles )
        {
            // wenn keine Styles exportiert werden, muss evtl. zusaetlich
            // harte Attributierung geschrieben werden
            switch( nDeep )
            {
            case CSS1_FMT_ISTAG:
            case CSS1_FMT_CMPREF:
                // fuer HTML-Tag-Vorlagen die Unterscheide zum Original
                // (sofern verfuegbar)
                if( pTemplate )
                    pRefFmt = SwHTMLWriter::GetTemplateFmt( nRefPoolId,
                                                            pTemplate );
                break;

            default:
                // sonst die zur HTML-Tag-Vorlage des Originals oder des
                // aktuellen Doks, wenn die nicht verfuegbar ist
                if( pTemplate )
                    pRefFmt = SwHTMLWriter::GetTemplateFmt( nRefPoolId,
                                                            pTemplate );
                else
                    pRefFmt = SwHTMLWriter::GetParentFmt( *pFmt, nDeep );
                break;
            }
        }
    }
    else if( bTxtColl )
    {
        // Nicht von einer HTML-Tag-Vorlage abgeleitete Absatz-Vorlagen
        // muessen als harte Attributierung relativ zur Textkoerper-Volage
        // exportiert werden. Fuer Nicht-Styles-Export sollte die der
        // HTML-Vorlage als Referenz dienen
        if( !bOutStyles && pTemplate )
            pRefFmt = pTemplate->GetTxtCollFromPool( RES_POOLCOLL_TEXT );
        else
            pRefFmt = pDoc->GetTxtCollFromPool( RES_POOLCOLL_TEXT );
    }

    if( pRefFmt || nDeep==0 )
    {
        // wenn Unterschiede zu einer anderen Vorlage geschrieben werden
        // sollen ist harte Attributierung noetig. Fuer Vorlagen, die
        // nicht von HTML-Tag-Vorlagen abgeleitet sind, gilt das immer
        pItemSet = new SfxItemSet( *pFmt->GetAttrSet().GetPool(),
                                   pFmt->GetAttrSet().GetRanges() );

        pItemSet->Set( pFmt->GetAttrSet(), TRUE );
        if( pRefFmt )
            SwHTMLWriter::SubtractItemSet( *pItemSet, pRefFmt->GetAttrSet(), TRUE );

        // einen leeren Item-Set gleich loeschen, das spart speater
        // Arbeit
        if( !pItemSet->Count() )
        {
            delete pItemSet;
            pItemSet = 0;
        }
    }

    if( bTxtColl )
    {
        // Ggf. noch ein DropCap-Attribut uebernehmen
        if( bOutStyles && bHardDrop && nDeep != 0 )
        {
            const SfxPoolItem *pItem;
            if( SFX_ITEM_SET==pFmt->GetAttrSet().GetItemState(
                                    RES_PARATR_DROP, TRUE, &pItem ) )
            {
                BOOL bPut = TRUE;
                if( pTemplate )
                {
                    pRefFmt = SwHTMLWriter::GetTemplateFmt( nRefPoolId, pTemplate );
                    const SfxPoolItem *pRefItem;
                    BOOL bRefItemSet =
                        SFX_ITEM_SET==pRefFmt->GetAttrSet().GetItemState(
                                        RES_PARATR_DROP, TRUE, &pRefItem );
                    bPut = !bRefItemSet || *pItem!=*pRefItem;
                }
                if( bPut )
                {
                    ASSERT( !pItemSet, "Wo kommt der ItemSet her?" );
                    pItemSet = new SfxItemSet( *pFmt->GetAttrSet().GetPool(),
                                               pFmt->GetAttrSet().GetRanges() );
                    pItemSet->Put( *pItem );
                }
            }
        }


        // Die diversen default-Abstaende aus der Vorlage oder der
        // Vergleischs-Vorlage merken
        const SvxLRSpaceItem &rLRSpace =
            (pRefFmt ? pRefFmt : pFmt)->GetLRSpace();
        nLeftMargin = rLRSpace.GetTxtLeft();
        nRightMargin = rLRSpace.GetRight();
        nFirstLineIndent = rLRSpace.GetTxtFirstLineOfst();

        const SvxULSpaceItem &rULSpace =
            (pRefFmt ? pRefFmt : pFmt)->GetULSpace();
        nTopMargin = rULSpace.GetUpper();
        nBottomMargin = rULSpace.GetLower();
    }
}

SwHTMLFmtInfo::~SwHTMLFmtInfo()
{
    delete pItemSet;
}

void OutHTML_SwFmt( Writer& rWrt, const SwFmt& rFmt,
                    const SfxItemSet *pNodeItemSet,
                    SwHTMLTxtCollOutputInfo& rInfo )
{
    ASSERT( RES_CONDTXTFMTCOLL==rFmt.Which() || RES_TXTFMTCOLL==rFmt.Which(),
            "keine Absatz-Vorlage" );

    SwHTMLWriter & rHWrt = (SwHTMLWriter&)rWrt;

    // Erstmal ein par Flags ...
    USHORT nNewDefListLvl = 0;
    USHORT nNumStart = USHRT_MAX;
    BOOL bForceDL = FALSE;
    BOOL bDT = FALSE;
    rInfo.bInNumBulList = FALSE;    // Wir sind in einer Liste?
    BOOL bNumbered = FALSE;         // Der aktuelle Absatz ist numeriert
    BOOL bPara = FALSE;             // das aktuelle Token ist <P>
    rInfo.bParaPossible = FALSE;    // ein <P> darf zusaetzlich ausgegeben werden
    BOOL bNoEndTag = FALSE;         // kein End-Tag ausgeben

    rHWrt.bNoAlign = FALSE;         // kein ALIGN=... moeglich
    BOOL bNoStyle = FALSE;          // kein STYLE=... moeglich
    BYTE nBulletGrfLvl = 255;       // Die auszugebende Bullet-Grafik

    // Sind wir in einer Aufzaehlungs- oder Numerierungliste?
    const SwTxtNode* pTxtNd = rWrt.pCurPam->GetNode()->GetTxtNode();
    ULONG nPos = pTxtNd->GetIndex();

    SwHTMLNumRuleInfo aNumInfo;
    if( rHWrt.GetNextNumInfo() )
    {
        aNumInfo = *rHWrt.GetNextNumInfo();
        rHWrt.ClearNextNumInfo();
    }
    else
    {
        aNumInfo.Set( *pTxtNd );
    }

    if( aNumInfo.GetNumRule() )
    {
        rInfo.bInNumBulList = TRUE;
        nNewDefListLvl = 0;

        // ist der aktuelle Absatz numeriert?
        bNumbered = aNumInfo.IsNumbered();
        BYTE nLvl = aNumInfo.GetLevel();

        const SwNodeNum& rAktNum = *pTxtNd->GetNum();
        ASSERT( GetRealLevel( rAktNum.GetLevel() ) == nLvl,
                "Gemerkter Num-Level ist falsch" );
        ASSERT( bNumbered == ((rAktNum.GetLevel() & NO_NUMLEVEL) == 0),
                "Gemerkter Numerierungs-Zustand ist falsch" );

        if( bNumbered )
        {
            nBulletGrfLvl = nLvl; // nur veruebergehend!!!
            nNumStart = rAktNum.GetSetValue();
            DBG_ASSERT( rHWrt.nLastParaToken == 0,
                "<PRE> wurde nicht vor <LI> beendet." );
        }

#ifndef NUM_RELSPACE
        // die Absatz-Abstaende/Einzuege muessen beruecksichtigen,
        // dass wir in einer Liste sind
        rHWrt.nLeftMargin = (nLvl+1) *  HTML_NUMBUL_MARGINLEFT;

        rHWrt.nFirstLineIndent = bNumbered ? HTML_NUMBUL_INDENT : 0;
#endif
    }

    // Jetzt holen wir das Token und ggf. die Klasse
    SwHTMLFmtInfo aFmtInfo( &rFmt );
    USHORT nArrayPos;
    const SwHTMLFmtInfo *pFmtInfo;
    if( rHWrt.aTxtCollInfos.Seek_Entry( &aFmtInfo, &nArrayPos ) )
    {
        pFmtInfo = rHWrt.aTxtCollInfos[nArrayPos];
    }
    else
    {
        pFmtInfo = new SwHTMLFmtInfo( &rFmt, rWrt.pDoc, rHWrt.pTemplate,
                                      rHWrt.bCfgOutStyles,
                                      !rHWrt.IsHTMLMode(HTMLMODE_DROPCAPS) );
        rHWrt.aTxtCollInfos.C40_PTR_INSERT( SwHTMLFmtInfo, pFmtInfo );
    }

    // Jetzt wird festgelegt, was aufgrund des Tokens so moeglich ist
    USHORT nToken = 0;          // Token fuer Tag-Wechsel
    BOOL bOutNewLine = FALSE;   // nur ein LF ausgeben?
    if( pFmtInfo->aToken.Len() )
    {
        // Es ist eine HTML-Tag-Vorlage oder die Vorlage ist von einer
        // solchen abgeleitet
        rInfo.aToken = pFmtInfo->aToken;

        // der erste Buchstabe reicht meistens
        switch( rInfo.aToken.GetChar( 0 ) )
        {
        case 'A':   ASSERT( rInfo.aToken.Equals(sHTML_address),
                            "Doch kein ADDRESS?" );
                    rInfo.bParaPossible = TRUE;
                    rHWrt.bNoAlign = TRUE;
                    break;

        case 'B':   ASSERT( rInfo.aToken.Equals(sHTML_blockquote),
                            "Doch kein BLOCKQUOTE?" );
                    rInfo.bParaPossible = TRUE;
                    rHWrt.bNoAlign = TRUE;
                    break;

        case 'P':   if( rInfo.aToken.Len() == 1 )
                    {
                        bPara = TRUE;
                    }
                    else
                    {
                        ASSERT( rInfo.aToken.Equals(sHTML_preformtxt),
                                "Doch kein PRE?" );
                        if( HTML_PREFORMTXT_ON == rHWrt.nLastParaToken )
                        {
                            bOutNewLine = TRUE;
                        }
                        else
                        {
                            nToken = HTML_PREFORMTXT_ON;
                            rHWrt.bNoAlign = TRUE;
                            bNoEndTag = TRUE;
                        }
                    }
                    break;

        case 'D':   ASSERT( rInfo.aToken.Equals(sHTML_dt) ||
                            rInfo.aToken.Equals(sHTML_dd),
                            "Doch kein DD/DT?" );
                    bDT = rInfo.aToken.Equals(sHTML_dt);
                    rInfo.bParaPossible = !bDT;
                    rHWrt.bNoAlign = TRUE;
                    bForceDL = TRUE;
                    break;
        }
    }
    else
    {
        // alle Vorlagen, die nicht einem HTML-Tag entsprechen oder von
        // diesem abgeleitet sind, werden als <P> exportiert

        rInfo.aToken = sHTML_parabreak;
        bPara = TRUE;
    }

    // Falls noetig, die harte Attributierung der Vorlage uebernehmen
    if( pFmtInfo->pItemSet )
    {
        ASSERT( !rInfo.pItemSet, "Wo kommt der Item-Set her?" );
        rInfo.pItemSet = new SfxItemSet( *pFmtInfo->pItemSet );
    }

    // und noch die harte Attributierung des Absatzes dazunehmen
    if( pNodeItemSet )
    {
        if( rInfo.pItemSet )
            rInfo.pItemSet->Put( *pNodeItemSet );
        else
            rInfo.pItemSet = new SfxItemSet( *pNodeItemSet );
    }

    // den unteren Absatz-Abstand brauchen wir noch
    const SvxULSpaceItem& rULSpace =
        pNodeItemSet ? ((const SvxULSpaceItem &)pNodeItemSet->Get(RES_UL_SPACE))
                     : rFmt.GetULSpace();

    if( (rHWrt.bOutHeader &&
         rWrt.pCurPam->GetPoint()->nNode.GetIndex() ==
            rWrt.pCurPam->GetMark()->nNode.GetIndex()) ||
         rHWrt.bOutFooter )
    {
        if( rHWrt.bCfgOutStyles )
        {
            SvxULSpaceItem aULSpaceItem( rULSpace );
            if( rHWrt.bOutHeader )
                aULSpaceItem.SetLower( rHWrt.nHeaderFooterSpace );
            else
                aULSpaceItem.SetUpper( rHWrt.nHeaderFooterSpace );

            if( !rInfo.pItemSet )
                rInfo.pItemSet = new SfxItemSet(
                                    *rFmt.GetAttrSet().GetPool(),
                                    RES_UL_SPACE, RES_UL_SPACE );
            rInfo.pItemSet->Put( aULSpaceItem );
        }
        rHWrt.bOutHeader = FALSE;
        rHWrt.bOutFooter = FALSE;
    }

    if( bOutNewLine )
    {
        // nur einen Zeilen-Umbruch (ohne Einrueckung) am Absatz-Anfang
        // ausgeben
        rInfo.aToken.Erase();   // kein End-Tag ausgeben
        rWrt.Strm() << SwHTMLWriter::sNewLine;

        return;
    }


    // soll ein ALIGN=... geschrieben werden?
    const SfxPoolItem* pAdjItem = 0;
    const SfxPoolItem* pItem;

    if( rInfo.pItemSet &&
        SFX_ITEM_SET == rInfo.pItemSet->GetItemState( RES_PARATR_ADJUST,
                                                      FALSE, &pItem ) )
    {
        pAdjItem = pItem;
    }

    // Unteren Absatz-Abstand beachten ? (nie im letzen Absatz von
    // Tabellen)
    BOOL bUseParSpace = !rHWrt.bOutTable ||
                        (rWrt.pCurPam->GetPoint()->nNode.GetIndex() !=
                         rWrt.pCurPam->GetMark()->nNode.GetIndex());
    // Wenn Styles exportiert werden, wird aus eingerueckten Absaetzen
    // eine Definitions-Liste
    const SvxLRSpaceItem& rLRSpace =
        pNodeItemSet ? ((const SvxLRSpaceItem &)pNodeItemSet->Get(RES_LR_SPACE))
                     : rFmt.GetLRSpace();
    if( (!rHWrt.bCfgOutStyles || bForceDL) && !rInfo.bInNumBulList )
    {
        USHORT nLeftMargin;
        if( bForceDL )
            nLeftMargin = rLRSpace.GetTxtLeft();
        else
            nLeftMargin = rLRSpace.GetTxtLeft() > pFmtInfo->nLeftMargin
                ? rLRSpace.GetTxtLeft() - pFmtInfo->nLeftMargin
                : 0;

        nNewDefListLvl = (nLeftMargin + (rHWrt.nDefListMargin/2)) /
                         rHWrt.nDefListMargin;

        BOOL bIsNextTxtNode =
            rWrt.pDoc->GetNodes()[rWrt.pCurPam->GetPoint()->nNode.GetIndex()+1]
                     ->IsTxtNode();

        if( bForceDL && bDT )
        {
            // Statt eines DD muessen wir hier auch ein DT der Ebene
            // darueber nehmen
            nNewDefListLvl++;
        }
        else if( !nNewDefListLvl && !rHWrt.bCfgOutStyles && bPara &&
                 rULSpace.GetLower()==0 &&
                 ((bUseParSpace && bIsNextTxtNode) || rHWrt.nDefListLvl==1) &&
                 (!pAdjItem || SVX_ADJUST_LEFT==
                    ((const SvxAdjustItem *)pAdjItem)->GetAdjust()) )
        {
            // Absaetze ohne unteren Abstand als DT exportieren
            nNewDefListLvl = 1;
            bDT = TRUE;
            rInfo.bParaPossible = FALSE;
            rHWrt.bNoAlign = TRUE;
        }
    }

    if( nNewDefListLvl != rHWrt.nDefListLvl )
        rHWrt.OutAndSetDefList( nNewDefListLvl );

    // ggf. eine Aufzaehlung- oder Numerierungsliste beginnen
    if( rInfo.bInNumBulList )
    {
        ASSERT( !rHWrt.nDefListLvl, "DL in OL geht nicht!" );
        OutHTML_NumBulListStart( rHWrt, aNumInfo );

        if( bNumbered )
        {
            if( rHWrt.aBulletGrfs[nBulletGrfLvl].Len()  )
                bNumbered = FALSE;
            else
                nBulletGrfLvl = 255;
        }

#ifndef NUM_RELSPACE
        // Wenn der Ziel-Browser mit horizontalen Absatz-Abstaenden in
        // Listen nicht zurechtkommt, loeschen wir das Item.
        if( !rHWrt.IsHTMLMode( HTMLMODE_LRSPACE_IN_NUMBUL ) &&
            rInfo.pItemSet )
        {
            rInfo.pItemSet->ClearItem( RES_LR_SPACE );
        }
#endif
    }

    // Die Defaults aus der Vorlage merken, denn sie muessen nicht
    // exportiert werden
    rHWrt.nDfltLeftMargin = pFmtInfo->nLeftMargin;
    rHWrt.nDfltRightMargin = pFmtInfo->nRightMargin;
    rHWrt.nDfltFirstLineIndent = pFmtInfo->nFirstLineIndent;

#ifdef NUM_RELSPACE
    if( rInfo.bInNumBulList )
    {
        if( !rHWrt.IsHTMLMode( HTMLMODE_LSPACE_IN_NUMBUL ) )
            rHWrt.nDfltLeftMargin = rLRSpace.GetTxtLeft();

        // In Numerierungs-Listen keinen Ertzeilen-Einzug ausgeben.
        rHWrt.nFirstLineIndent = rLRSpace.GetTxtFirstLineOfst();
    }
#endif

    if( rInfo.bInNumBulList && bNumbered && bPara && !rHWrt.bCfgOutStyles )
    {
        // ein einzelnes LI hat keinen Abstand
        rHWrt.nDfltTopMargin = 0;
        rHWrt.nDfltBottomMargin = 0;
    }
    else if( rHWrt.nDefListLvl && bPara )
    {
        // ein einzelnes DD hat auch keinen Abstand
        rHWrt.nDfltTopMargin = 0;
        rHWrt.nDfltBottomMargin = 0;
    }
    else
    {
        rHWrt.nDfltTopMargin = pFmtInfo->nTopMargin;
        // #60393#: Wenn im letzten Absatz einer Tabelle der
        // untere Absatz-Abstand veraendert wird, vertut sich
        // Netscape total. Deshalb exportieren wir hier erstmal
        // nichts, indem wir den Abstand aus dem Absatz als Default
        // setzen.
        if( rHWrt.bCfgNetscape4 && !bUseParSpace )
            rHWrt.nDfltBottomMargin = rULSpace.GetLower();
        else
            rHWrt.nDfltBottomMargin = pFmtInfo->nBottomMargin;
    }

    if( rHWrt.nDefListLvl )
    {
        rHWrt.nLeftMargin =
            (rHWrt.nDefListLvl-1) * rHWrt.nDefListMargin;
    }

    if( rHWrt.bLFPossible )
        rHWrt.OutNewLine(); // Absatz-Tag in neue Zeile
    rInfo.bOutPara = FALSE;

    // das ist jetzt unser neues Token
    rHWrt.ChangeParaToken( nToken );

    BOOL bHasParSpace = bUseParSpace && rULSpace.GetLower() > 0;

    // ggf ein List-Item aufmachen
    if( rInfo.bInNumBulList && bNumbered )
    {
        ByteString sOut( '<' );
        sOut += sHTML_li;
        if( USHRT_MAX != nNumStart )
            (((sOut += ' ') += sHTML_O_value) += '=')
                += ByteString::CreateFromInt32(nNumStart);
        sOut += '>';
        rWrt.Strm() << sOut.GetBuffer();
    }

    if( rHWrt.nDefListLvl > 0 && !bForceDL )
    {
        HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), bDT ? sHTML_dt : sHTML_dd );
    }

    if( pAdjItem &&
        rHWrt.IsHTMLMode( HTMLMODE_NO_CONTROL_CENTERING ) &&
        rHWrt.HasControls() )
    {
        // #64687#: The align=... attribute does behave strange in netscape
        // if there are controls in a paragraph, because the control and
        // all text behind the control does not recognize this attribute.
        ByteString sOut( '<' );
        sOut += sHTML_division;
        rWrt.Strm() << sOut.GetBuffer();

        rHWrt.bTxtAttr = FALSE;
        rHWrt.bOutOpts = TRUE;
        OutHTML_SvxAdjust( rWrt, *pAdjItem );
        rWrt.Strm() << '>';
        pAdjItem = 0;
        rHWrt.bNoAlign = FALSE;
        rInfo.bOutDiv = TRUE;
        rHWrt.IncIndentLevel();
        rHWrt.bLFPossible = TRUE;
            rHWrt.OutNewLine();
    }

    // fuer BLOCKQUOTE, ADDRESS und DD wird noch ein Absatz-Token
    // ausgegeben, wenn,
    // - keine Styles geschrieben werden, und
    // - ein untere Abstand oder eine Absatz-Ausrichtung existiert
    ByteString aToken = rInfo.aToken;
    if( !rHWrt.bCfgOutStyles && rInfo.bParaPossible && !bPara &&
        (bHasParSpace || pAdjItem) )
    {
        HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), rInfo.aToken.GetBuffer() );
        aToken = sHTML_parabreak;
        bPara = TRUE;
        rHWrt.bNoAlign = FALSE;
        bNoStyle = FALSE;
    }

    // Ein <P> wird nur geschrieben, wenn
    // - wir in keiner OL/UL/DL sind, oder
    // - der Absatz einer OL/UL nicht numeriert ist, oder
    // - keine Styles exportiert werden und
    //      - ein unterer Abstand oder
    //      - eine Absatz-Ausrichtung existiert, ode
    // - Styles exportiert werden und,
    //      - die Textkoerper-Vorlage geaendert wurde, oder
    //      - ein Benutzer-Format exportiert wird, oder
    //      - Absatz-Attribute existieren
    if( !bPara ||
        (!rInfo.bInNumBulList && !rHWrt.nDefListLvl) ||
        (rInfo.bInNumBulList && !bNumbered) ||
        (!rHWrt.bCfgOutStyles && (bHasParSpace || pAdjItem)) ||
        (rHWrt.bCfgOutStyles /*&& rHWrt.bPoolCollTextModified*/) )
    {
        // jetzt werden Optionen ausgegeben
        rHWrt.bTxtAttr = FALSE;
        rHWrt.bOutOpts = TRUE;

        ByteString sOut( '<' );
        sOut += aToken;

        if( rHWrt.bCfgOutStyles && pFmtInfo->aClass.Len() )
        {
            ((sOut += ' ') += sHTML_O_class) += "=\"";
            rWrt.Strm() << sOut.GetBuffer();
            HTMLOutFuncs::Out_String( rWrt.Strm(), pFmtInfo->aClass,
                                      rHWrt.eDestEnc );
            sOut = '\"';
        }
        rWrt.Strm() << sOut.GetBuffer();

        // ggf. Ausrichtung ausgeben.
        if( !rHWrt.bNoAlign && pAdjItem )
            OutHTML_SvxAdjust( rWrt, *pAdjItem );

        // und nun ggf. noch die STYLE-Option
        if( rHWrt.bCfgOutStyles && rInfo.pItemSet && !bNoStyle)
        {
            OutCSS1_ParaTagStyleOpt( rWrt, *rInfo.pItemSet );
        }

        rWrt.Strm() << '>';

        // Soll ein </P> geschrieben wenrden
        rInfo.bOutPara =
            bPara &&
            ( rHWrt.bCfgOutStyles ||
                (!rHWrt.bCfgOutStyles && bHasParSpace) );

        // wenn kein End-Tag geschrieben werden soll, es loeschen
        if( bNoEndTag )
            rInfo.aToken.Erase();
    }

    // ??? Warum nicht ueber den Hint-Mechanismus ???
    if( rHWrt.IsHTMLMode(HTMLMODE_FIRSTLINE) )
    {
        const SvxLRSpaceItem& rLRSpace =
            pNodeItemSet ? ((const SvxLRSpaceItem &)pNodeItemSet->Get(RES_LR_SPACE))
                         : rFmt.GetLRSpace();
        if( rLRSpace.GetTxtFirstLineOfst() > 0 )
        {
            OutHTML_HoriSpacer( rWrt, rLRSpace.GetTxtFirstLineOfst() );
        }
    }

    if( nBulletGrfLvl != 255 )
    {
        ASSERT( aNumInfo.GetNumRule(), "Wo ist die Numerierung geblieben???" );
        ASSERT( nBulletGrfLvl < MAXLEVEL, "So viele Ebenen gibt's nicht" );
        const SwNumFmt& rNumFmt = aNumInfo.GetNumRule()->Get(nBulletGrfLvl);

        OutHTML_BulletImage( rWrt, sHTML_image, 0,
                             rHWrt.aBulletGrfs[nBulletGrfLvl],
                             rNumFmt.GetGrfSize(), rNumFmt.GetGrfOrient() );
    }

    rHWrt.GetNumInfo() = aNumInfo;

    // die Defaults zuruecksetzen
    rHWrt.nDfltLeftMargin = 0;
    rHWrt.nDfltRightMargin = 0;
    rHWrt.nDfltFirstLineIndent = 0;
    rHWrt.nDfltTopMargin = 0;
    rHWrt.nDfltBottomMargin = 0;
    rHWrt.nLeftMargin = 0;
    rHWrt.nFirstLineIndent = 0;
}

void OutHTML_SwFmtOff( Writer& rWrt, const SwHTMLTxtCollOutputInfo& rInfo )
{
    SwHTMLWriter & rHWrt = (SwHTMLWriter&)rWrt;

    // wenn es kein Token gibt haben wir auch nichts auszugeben
    if( !rInfo.aToken.Len() )
    {
        rHWrt.FillNextNumInfo();
        const SwHTMLNumRuleInfo& rNextInfo = *rHWrt.GetNextNumInfo();
        // Auch in PRE muss eine Bullet-Liste beendet werden
        if( rInfo.bInNumBulList )
        {

            const SwHTMLNumRuleInfo& rInfo = rHWrt.GetNumInfo();
            if( rNextInfo.GetNumRule() != rInfo.GetNumRule() ||
                rNextInfo.GetDepth() != rInfo.GetDepth() ||
                rNextInfo.IsNumbered() || rNextInfo.IsRestart() )
                rHWrt.ChangeParaToken( 0 );
            OutHTML_NumBulListEnd( rHWrt, rNextInfo );
        }
        else if( rNextInfo.GetNumRule() != 0 )
            rHWrt.ChangeParaToken( 0 );

        return;
    }

    if( rInfo.ShouldOutputToken() )
    {
        if( rHWrt.bLFPossible )
            rHWrt.OutNewLine( TRUE );

        // fuer BLOCKQUOTE, ADDRESS und DD wird ggf noch ein
        // Absatz-Token ausgegeben, wenn
        // - keine Styles geschrieben werden, und
        // - ein untere Abstand existiert
        if( rInfo.bParaPossible && rInfo.bOutPara )
            HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), sHTML_parabreak, FALSE );

        HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), rInfo.aToken.GetBuffer(),
                                    FALSE );
        rHWrt.bLFPossible = !rInfo.aToken.Equals( sHTML_dt ) &&
                            !rInfo.aToken.Equals( sHTML_dd ) &&
                            !rInfo.aToken.Equals( sHTML_li );
    }
    if( rInfo.bOutDiv )
    {
        rHWrt.DecIndentLevel();
        if( rHWrt.bLFPossible )
            rHWrt.OutNewLine();
        HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), sHTML_division, FALSE );
        rHWrt.bLFPossible = TRUE;
    }

    // ggf. eine Aufzaehlung- oder Numerierungsliste beenden
    if( rInfo.bInNumBulList )
    {
        rHWrt.FillNextNumInfo();
        OutHTML_NumBulListEnd( rHWrt, *rHWrt.GetNextNumInfo() );
    }
}


class HTMLSttEndPos
{
    xub_StrLen nStart;
    xub_StrLen nEnd;
    SfxPoolItem* pItem;

public:

    HTMLSttEndPos( const SfxPoolItem& rItem, xub_StrLen nStt, xub_StrLen nE );
    ~HTMLSttEndPos();

    const SfxPoolItem *GetItem() const { return pItem; }

    void SetStart( xub_StrLen nStt ) { nStart = nStt; }
    xub_StrLen GetStart() const { return nStart; }

    xub_StrLen GetEnd() const { return nEnd; }
    void SetEnd( xub_StrLen nE ) { nEnd = nE; }
};

HTMLSttEndPos::HTMLSttEndPos( const SfxPoolItem& rItem, xub_StrLen nStt,
                                                        xub_StrLen nE ) :
    nStart( nStt ),
    nEnd( nE ),
    pItem( rItem.Clone() )
{}

HTMLSttEndPos::~HTMLSttEndPos()
{
    delete pItem;
}

typedef HTMLSttEndPos *HTMLSttEndPosPtr;
SV_DECL_PTRARR( _HTMLEndLst, HTMLSttEndPosPtr, 5, 5 )

enum HTMLOnOffState { HTML_NOT_SUPPORTED,   // nicht unterst. Attribut
                      HTML_REAL_VALUE,      // Attribut mit Wert
                      HTML_ON_VALUE,        // Attribut entspricht On-Tag
                      HTML_OFF_VALUE,       // Attribut entspricht Off-Tag
                      HTML_CHRFMT_VALUE,    // Attribut fuer Zeichenvorlage
                      HTML_COLOR_VALUE,     // Attribut fuer Vordergrundfarbe
                      HTML_STYLE_VALUE,     // Attribut muss als Style exp.
                      HTML_DROPCAP_VALUE }; // DropCap-Attributs


class HTMLEndPosLst
{
    _HTMLEndLst aStartLst;  // nach Anfangs-Psoitionen sortierte Liste
    _HTMLEndLst aEndLst;    // nach End-Psotionen sortierte Liste

    SwDoc *pDoc;            // das aktuelle Dokument
    SwDoc* pTemplate;       // die HTML-Vorlage (oder 0)
    const Color* pDfltColor;// die Default-Vordergrund-Farbe

    ULONG nHTMLMode;
    BOOL bOutStyles : 1;    // werden Styles exportiert


    // die Position eines Items in der Start-/Ende-Liste suchen
    USHORT _FindStartPos( const HTMLSttEndPos *pPos ) const;
    USHORT _FindEndPos( const HTMLSttEndPos *pPos ) const;

    // Eine SttEndPos in die Start- und Ende-Listen eintragen bzw. aus
    // ihnen loeschen, wobei die Ende-Position bekannt ist
    void _InsertItem( HTMLSttEndPos *pPos, USHORT nEndPos );
    void _RemoveItem( USHORT nEndPos );

    // die "Art" es Attributs ermitteln
    HTMLOnOffState GetHTMLItemState( const SfxPoolItem& rItem );

    // Existiert ein bestimmtes On-Tag-Item
    BOOL ExistsOnTagItem( USHORT nWhich, xub_StrLen nPos );

    // Existiert ein Item zum ausschalten eines Attributs, das genauso
    // exportiert wird wie das uebergebene Item im gleichen Bereich?
    BOOL ExistsOffTagItem( USHORT nWhich, xub_StrLen nStartPos,
                                          xub_StrLen nEndPos );


    // das Ende eines gesplitteten Items anpassen
    void FixSplittedItem( HTMLSttEndPos *pPos, USHORT nStartPos,
                          xub_StrLen nNewEnd );

    // Ein Attribut in die Listen eintragen und ggf. aufteilen
    void InsertItem( const SfxPoolItem& rItem, xub_StrLen nStart,
                                               xub_StrLen nEnd );

    // Ein bereits vorhandenes Attribut aufteilen
    void SplitItem( const SfxPoolItem& rItem, xub_StrLen nStart,
                                              xub_StrLen nEnd );


public:

    HTMLEndPosLst( SwDoc *pDoc, SwDoc* pTemplate, const Color* pDfltColor,
                   BOOL bOutStyles, ULONG nHTMLMode );
    ~HTMLEndPosLst();

    // Ein Attribut einfuegen
    void Insert( const SfxPoolItem& rItem, xub_StrLen nStart,  xub_StrLen nEnd,
                 SwHTMLFmtInfos& rFmtInfos, BOOL bParaAttrs=FALSE );
    void Insert( const SfxItemSet& rItemSet, xub_StrLen nStart, xub_StrLen nEnd,
                 SwHTMLFmtInfos& rFmtInfos, BOOL bDeep,
                 BOOL bParaAttrs=FALSE );
    void Insert( const SwDrawFrmFmt& rFmt, xub_StrLen nPos,
                 SwHTMLFmtInfos& rFmtInfos );

    void OutStartAttrs( SwHTMLWriter& rHWrt, xub_StrLen nPos );
    void OutEndAttrs( SwHTMLWriter& rHWrt, xub_StrLen nPos );

    USHORT Count() const { return aEndLst.Count(); }

    BOOL IsHTMLMode( ULONG nMode ) const { return (nHTMLMode & nMode) != 0; }
};


USHORT HTMLEndPosLst::_FindStartPos( const HTMLSttEndPos *pPos ) const
{
    for( USHORT i=0; i<aStartLst.Count() && aStartLst[i]!=pPos;  i++ )
        ;

    ASSERT( i != aStartLst.Count(), "Item nicht in Start-Liste gefunden!" );

    return i==aStartLst.Count() ? USHRT_MAX : i;
}

USHORT HTMLEndPosLst::_FindEndPos( const HTMLSttEndPos *pPos ) const
{
    for( USHORT i=0; i<aEndLst.Count() && aEndLst[i]!=pPos;  i++ )
        ;

    ASSERT( i != aEndLst.Count(), "Item nicht in Ende-Liste gefunden" );

    return i==aEndLst.Count() ? USHRT_MAX : i;
}


void HTMLEndPosLst::_InsertItem( HTMLSttEndPos *pPos, USHORT nEndPos )
{
    // In der Start-Liste das Attribut hinter allen vorher und an
    // der gleichen Position gestarteten Attributen einfuegen
    xub_StrLen nStart = pPos->GetStart();
    for( USHORT i=0; i < aStartLst.Count() &&
                     aStartLst[i]->GetStart() <= nStart; i++ )
        ;
    aStartLst.Insert( pPos, i );

    // die Position in der Ende-Liste wurde uebergeben
    aEndLst.Insert( pPos, nEndPos );
}

void HTMLEndPosLst::_RemoveItem( USHORT nEndPos )
{
    HTMLSttEndPos *pPos = aEndLst[nEndPos];

    // jetzt Suchen wir es in der Start-Liste
    USHORT nStartPos = _FindStartPos( pPos );
    if( nStartPos != USHRT_MAX )
        aStartLst.Remove( nStartPos, 1 );

    aEndLst.Remove( nEndPos, 1 );

    delete pPos;
}

HTMLOnOffState HTMLEndPosLst::GetHTMLItemState( const SfxPoolItem& rItem )
{
    HTMLOnOffState eState = HTML_NOT_SUPPORTED;
    switch( rItem.Which() )
    {
    case RES_CHRATR_POSTURE:
        switch( ((const SvxPostureItem&)rItem).GetPosture() )
        {
        case ITALIC_NORMAL:
            eState = HTML_ON_VALUE;
            break;
        case ITALIC_NONE:
            eState = HTML_OFF_VALUE;
            break;
        default:
            if( IsHTMLMode(HTMLMODE_SOME_STYLES) )
                eState = HTML_STYLE_VALUE;
            break;
        }
        break;

    case RES_CHRATR_CROSSEDOUT:
        switch( ((const SvxCrossedOutItem&)rItem).GetStrikeout() )
        {
        case STRIKEOUT_SINGLE:
        case STRIKEOUT_DOUBLE:
            eState = HTML_ON_VALUE;
            break;
        case STRIKEOUT_NONE:
            eState = HTML_OFF_VALUE;
            break;
        }
        break;

    case RES_CHRATR_ESCAPEMENT:
        switch( (const SvxEscapement)
                        ((const SvxEscapementItem&)rItem).GetEnumValue() )
        {
        case SVX_ESCAPEMENT_SUPERSCRIPT:
        case SVX_ESCAPEMENT_SUBSCRIPT:
            eState = HTML_ON_VALUE;
            break;
        case SVX_ESCAPEMENT_OFF:
            eState = HTML_OFF_VALUE;
            break;
        }
        break;

    case RES_CHRATR_UNDERLINE:
        switch( ((const SvxUnderlineItem&)rItem).GetUnderline() )
        {
        case UNDERLINE_SINGLE:
            eState = HTML_ON_VALUE;
            break;
        case UNDERLINE_NONE:
            eState = HTML_OFF_VALUE;
            break;
        default:
            if( IsHTMLMode(HTMLMODE_SOME_STYLES) )
                eState = HTML_STYLE_VALUE;
            break;
        }
        break;

    case RES_CHRATR_WEIGHT:
        switch( ((const SvxWeightItem&)rItem).GetWeight() )
        {
        case WEIGHT_BOLD:
            eState = HTML_ON_VALUE;
            break;
        case WEIGHT_NORMAL:
            eState = HTML_OFF_VALUE;
            break;
        default:
            if( IsHTMLMode(HTMLMODE_SOME_STYLES) )
                eState = HTML_STYLE_VALUE;
            break;
        }
        break;

    case RES_CHRATR_BLINK:
        if( IsHTMLMode(HTMLMODE_BLINK) )
            eState = ((const SvxBlinkItem&)rItem).GetValue() ? HTML_ON_VALUE
                                                             : HTML_OFF_VALUE;
        break;

    case RES_CHRATR_COLOR:
        eState = HTML_COLOR_VALUE;
        break;

    case RES_CHRATR_FONT:
    case RES_CHRATR_FONTSIZE:
    case RES_TXTATR_INETFMT:
        eState = HTML_REAL_VALUE;
        break;

    case RES_TXTATR_CHARFMT:
        eState = HTML_CHRFMT_VALUE;
        break;

    case RES_CHRATR_CASEMAP:
        if( IsHTMLMode(HTMLMODE_SMALL_CAPS) )
            eState = HTML_STYLE_VALUE;

    case RES_CHRATR_KERNING:
        if( IsHTMLMode(HTMLMODE_FULL_STYLES) )
            eState = HTML_STYLE_VALUE;
        break;

    case RES_CHRATR_BACKGROUND:
        if( IsHTMLMode(HTMLMODE_SOME_STYLES) )
            eState = HTML_STYLE_VALUE;
        break;

    case RES_PARATR_DROP:
        eState = HTML_DROPCAP_VALUE;
        break;

//  default:
//      eState = HTML_NOT_SUPPORTED;
//      break;
    }

    return eState;
}

BOOL HTMLEndPosLst::ExistsOnTagItem( USHORT nWhich, xub_StrLen nPos )
{
    for( USHORT i=0; i<aStartLst.Count(); i++ )
    {
        HTMLSttEndPos *pTest = aStartLst[i];

        if( pTest->GetStart() > nPos )
        {
            // dieses uns alle folgenden Attribute beginnen spaeter
            break;
        }
        else if( pTest->GetEnd() > nPos )
        {
            // das Attribut beginnt vor oder an der aktuellen Position
            // und endet hinter ihr
            const SfxPoolItem *pItem = pTest->GetItem();
            if( pItem->Which() == nWhich &&
                HTML_ON_VALUE == GetHTMLItemState(*pItem) )
            {
                // ein On-Tag-Attibut wurde gefunden
                return TRUE;
            }
        }
    }

    return FALSE;
}

BOOL HTMLEndPosLst::ExistsOffTagItem( USHORT nWhich, xub_StrLen nStartPos,
                                      xub_StrLen nEndPos )
{
    if( nWhich != RES_CHRATR_CROSSEDOUT &&
        nWhich != RES_CHRATR_UNDERLINE &&
        nWhich != RES_CHRATR_BLINK )
    {
        return FALSE;
    }

    for( USHORT i=0; i<aStartLst.Count(); i++ )
    {
        HTMLSttEndPos *pTest = aStartLst[i];

        if( pTest->GetStart() > nStartPos )
        {
            // dieses uns alle folgenden Attribute beginnen spaeter
            break;
        }
        else if( pTest->GetStart()==nStartPos &&
                 pTest->GetEnd()==nEndPos )
        {
            // das Attribut beginnt vor oder an der aktuellen Position
            // und endet hinter ihr
            const SfxPoolItem *pItem = pTest->GetItem();
            USHORT nTstWhich = pItem->Which() ;
            if( (nTstWhich == RES_CHRATR_CROSSEDOUT ||
                 nTstWhich == RES_CHRATR_UNDERLINE ||
                 nTstWhich == RES_CHRATR_BLINK) &&
                HTML_OFF_VALUE == GetHTMLItemState(*pItem) )
            {
                // Ein Off-Tag-Attibut wurde gefunden, das genauso
                // exportiert wird, wie das aktuelle Item
                return TRUE;
            }
        }
    }

    return FALSE;
}

void HTMLEndPosLst::FixSplittedItem( HTMLSttEndPos *pPos, xub_StrLen nNewEnd,
                                     USHORT nStartPos )
{
    // die End-Position entsprechend fixen
    pPos->SetEnd( nNewEnd );

    // das Item aus der End-Liste entfernen
    USHORT nEndPos = _FindEndPos( pPos );
    if( nEndPos != USHRT_MAX )
        aEndLst.Remove( nEndPos, 1 );

    // es wird von nun an als letztes an der entsprechenden Position
    // beendet
    for( nEndPos=0; nEndPos < aEndLst.Count() &&
                    aEndLst[nEndPos]->GetEnd() <= nNewEnd; nEndPos++ )
        ;
    aEndLst.Insert( pPos, nEndPos );

    // jetzt noch die spaeter gestarteten Attribute anpassen
    for( USHORT i=nStartPos+1; i<aStartLst.Count(); i++ )
    {
        HTMLSttEndPos *pTest = aStartLst[i];
        xub_StrLen nTestEnd = pTest->GetEnd();
        if( pTest->GetStart() >= nNewEnd )
        {
            // das Test-Attribut und alle folgenden beginnen, nachdem das
            // gesplittete Attribut endet
            break;
        }
        else if( nTestEnd > nNewEnd )
        {
            // das Test-Attribut beginnt, bevor das gesplittete Attribut
            // endet und endet danach, muss also auch gesplittet werden

            // das neue Ende setzen
            pTest->SetEnd( nNewEnd );

            // das Attribut aus der End-Liste entfernen
            USHORT nEPos = _FindEndPos( pTest );
            if( nEPos != USHRT_MAX )
                aEndLst.Remove( nEPos, 1 );

            // es endet jetzt als erstes Attribut an der entsprechenden
            // Position. Diese Position in der Ende-Liste kennen wir schon.
            aEndLst.Insert(pTest, nEndPos );

            // den "Rest" des Attributs neu einfuegen
            InsertItem( *pTest->GetItem(), nNewEnd, nTestEnd );
        }
    }
}


void HTMLEndPosLst::InsertItem( const SfxPoolItem& rItem, xub_StrLen nStart,
                                                          xub_StrLen nEnd )
{
    for( USHORT i=0; i<aEndLst.Count(); i++ )
    {
        HTMLSttEndPos *pTest = aEndLst[i];
        xub_StrLen nTestEnd = pTest->GetEnd();
        if( nTestEnd <= nStart )
        {
            // das Test-Attribut endet, bevor das neue beginnt
            continue;
        }
        else if( nTestEnd < nEnd )
        {
            // das Test-Attribut endet, bevor das neue endet. Das
            // neue Attribut muss deshalb aufgesplittet werden
            _InsertItem( new HTMLSttEndPos( rItem, nStart, nTestEnd ), i );
            nStart = nTestEnd;
        }
        else
        {
            // das Test-Attribut (und alle folgenden) endet, bevor das neue
            // endet
            break;
        }
    }

    // ein Attribut muss noch eingefuegt werden
    _InsertItem( new HTMLSttEndPos( rItem, nStart, nEnd ), i );
}



void HTMLEndPosLst::SplitItem( const SfxPoolItem& rItem, xub_StrLen nStart,
                                                           xub_StrLen nEnd )
{
    USHORT nWhich = rItem.Which();

    // erstmal muessen wir die alten Items anhand der Startliste suchen
    // und die neuen Item-Bereiche festlegen

    for( USHORT i=0; i<aStartLst.Count(); i++ )
    {
        HTMLSttEndPos *pTest = aStartLst[i];
        xub_StrLen nTestStart = pTest->GetStart();
        xub_StrLen nTestEnd = pTest->GetEnd();

        if( nTestStart >= nEnd )
        {
            // dieses und alle nachfolgenden Attribute beginnen spaeter
            break;
        }
        else if( nTestEnd > nStart )
        {
            // das Test Attribut endet im zu loeschenenden Bereich
            const SfxPoolItem *pItem = pTest->GetItem();

            // nur entsprechende On-Tag Attribute muessen beruecksichtigt
            // werden
            if( pItem->Which() == nWhich &&
                HTML_ON_VALUE == GetHTMLItemState( *pItem ) )
            {
                BOOL bDelete = TRUE;

                if( nTestStart < nStart )
                {
                    // der Start des neuen Attribut entspricht
                    // dem neuen Ende des Attribts
                    FixSplittedItem( pTest, nStart, i );
                    bDelete = FALSE;
                }
                else
                {
                    // das Test-Item beginnt erst hinter dem neuen
                    // Ende des Attribts und kann deshalb komplett
                    // geloescht werden
                    aStartLst.Remove( i, 1 );
                    i--;

                    USHORT nEndPos = _FindEndPos( pTest );
                    if( nEndPos != USHRT_MAX )
                        aEndLst.Remove( nEndPos, 1 );
                }

                // ggf den zweiten Teil des gesplitteten Attribts einfuegen
                if( nTestEnd > nEnd )
                {
                    InsertItem( *pTest->GetItem(), nEnd, nTestEnd );
                }

                if( bDelete )
                    delete pTest;
            }
        }
    }
}

HTMLEndPosLst::HTMLEndPosLst( SwDoc *pD, SwDoc* pTempl,
                              const Color* pDfltCol, BOOL bStyles,
                              ULONG nMode ):
    pDoc( pD ), pTemplate( pTempl ), pDfltColor( pDfltCol ),
    bOutStyles( bStyles ), nHTMLMode( nMode )
{}

HTMLEndPosLst::~HTMLEndPosLst()
{
    ASSERT( !aStartLst.Count(), "Start-Liste im Destruktor nicht leer" );
    ASSERT( !aEndLst.Count(), "End-Liste im Destruktor nicht leer" );
}



void HTMLEndPosLst::Insert( const SfxPoolItem& rItem,
                            xub_StrLen nStart, xub_StrLen nEnd,
                            SwHTMLFmtInfos& rFmtInfos, BOOL bParaAttrs )
{
    // kein Bereich ?? dann nicht aufnehmen, wird nie wirksam !!
    if( nStart != nEnd )
    {
        BOOL bSet = FALSE, bSplit = FALSE;
        switch( GetHTMLItemState(rItem) )
        {
        case HTML_ON_VALUE:
            // das Attribut wird ausgegeben, wenn es nicht sowieso
            // schon an ist
            if( !ExistsOnTagItem( rItem.Which(), nStart ) )
                bSet = TRUE;
            break;

        case HTML_OFF_VALUE:
            // wenn das entsprechne Attribut an ist, wird es gesplittet,
            // Zusaetlich wird es aber als Style ausgegeben, wenn es nicht
            // am ganzen Absatz gesetzt ist, weil es dann ja schon mit dem
            // ABsatz-Tag ausgegeben wurde.
            if( ExistsOnTagItem( rItem.Which(), nStart ) )
                bSplit = TRUE;
            bSet = bOutStyles && !bParaAttrs &&
                   !ExistsOffTagItem( rItem.Which(), nStart, nEnd );
            break;

        case HTML_REAL_VALUE:
            // das Attribut kann immer ausgegeben werden
            bSet = TRUE;
            break;

        case HTML_STYLE_VALUE:
            // Das Attribut kann nur als CSS1 ausgegeben werden. Wenn
            // es am Absatz gesetzt ist, wurde es schon mit dem
            // Absatz-Tag ausgegeben. Einzige Ausnahme ist das
            // Zeichen-Hintergrund-Attribut. Es muss immer wie ein
            // Hint behandelt werden.
            bSet = bOutStyles &&
                   (!bParaAttrs || rItem.Which()==RES_CHRATR_BACKGROUND);
            break;

        case HTML_CHRFMT_VALUE:
            {
                ASSERT( RES_TXTATR_CHARFMT == rItem.Which(),
                        "Doch keine Zeichen-Vorlage" );
                const SwFmtCharFmt& rChrFmt = (const SwFmtCharFmt&)rItem;
                const SwCharFmt* pFmt = rChrFmt.GetCharFmt();

                const SwHTMLFmtInfo *pFmtInfo;
                SwHTMLFmtInfo aFmtInfo( pFmt );
                USHORT nPos;
                if( rFmtInfos.Seek_Entry( &aFmtInfo, &nPos ) )
                {
                    pFmtInfo = rFmtInfos[nPos];
                }
                else
                {
                    pFmtInfo = new SwHTMLFmtInfo( pFmt, pDoc, pTemplate,
                                                  bOutStyles );
                    rFmtInfos.C40_PTR_INSERT( SwHTMLFmtInfo, pFmtInfo );
                }
                if( pFmtInfo->aToken.Len() )
                {
                    // das Zeichenvorlagen-Tag muss vor den harten
                    // Attributen ausgegeben werden
                    InsertItem( rItem, nStart, nEnd );
                }
                if( pFmtInfo->pItemSet )
                {
                    Insert( *pFmtInfo->pItemSet, nStart, nEnd,
                            rFmtInfos, TRUE, bParaAttrs );
                }
            }
            break;

        case HTML_COLOR_VALUE:
            // Eine Vordergrund-Farbe als Absatz-Attribut wird nur
            // exportiert, wenn sie nicht der Default-Farbe entspricht.
            ASSERT( RES_CHRATR_COLOR == rItem.Which(),
                    "Doch keine Vordergrund-Farbe" );
            bSet = !bParaAttrs || !pDfltColor ||
                   !pDfltColor->IsRGBEqual(
                        ((const SvxColorItem&)rItem).GetValue() );
            break;

        case HTML_DROPCAP_VALUE:
            {
                ASSERT( RES_PARATR_DROP == rItem.Which(),
                        "Doch kein Drop-Cap" );
                const SwFmtDrop& rDrop = (const SwFmtDrop&)rItem;
                nEnd = nStart + rDrop.GetChars();
                if( !bOutStyles )
                {
                    // Zumindest die Attribute der Zeichenvorlage uebernehmen
                    const SwCharFmt *pCharFmt = rDrop.GetCharFmt();
                    if( pCharFmt )
                    {
                        Insert( pCharFmt->GetAttrSet(), nStart, nEnd,
                                rFmtInfos, TRUE, bParaAttrs );
                    }
                }
                else
                {
                    bSet = TRUE;
                }
            }
            break;
        }

        if( bSet )
            InsertItem( rItem, nStart, nEnd );
        if( bSplit )
            SplitItem( rItem, nStart, nEnd );
    }
}

void HTMLEndPosLst::Insert( const SfxItemSet& rItemSet,
                            xub_StrLen nStart, xub_StrLen nEnd,
                            SwHTMLFmtInfos& rFmtInfos,
                            BOOL bDeep, BOOL bParaAttrs )
{
    SfxWhichIter aIter( rItemSet );

    USHORT nWhich = aIter.FirstWhich();
    while( nWhich )
    {
        const SfxPoolItem *pItem;
        if( SFX_ITEM_SET == rItemSet.GetItemState( nWhich, bDeep, &pItem ) )
        {
            Insert( *pItem, nStart, nEnd, rFmtInfos, bParaAttrs );
        }

        nWhich = aIter.NextWhich();
    }
}

void HTMLEndPosLst::Insert( const SwDrawFrmFmt& rFmt, xub_StrLen nPos,
                            SwHTMLFmtInfos& rFmtInfos )
{
    // der Type-Cast ist nur noetig, um nicht seinetwegen
    // svdrwobt.hxx zu includem
    const SdrObject* pTextObj =
        (const SdrObject *)SwHTMLWriter::GetMarqueeTextObj( rFmt );

    if( pTextObj )
    {
        // die Edit-Engine-Attribute des Objekts als SW-Attribute holen
        // und als Hints einsortieren. Wegen der Menge der Hints werden
        // Styles hierbei nicht beruecksichtigt!
        const SfxItemSet& rFmtItemSet = rFmt.GetAttrSet();
        SfxItemSet aItemSet( *rFmtItemSet.GetPool(), RES_CHRATR_BEGIN,
                                                     RES_CHRATR_END );
        SwHTMLWriter::GetEEAttrsFromDrwObj( aItemSet, pTextObj, TRUE );
        BOOL bOutStylesOld = bOutStyles;
        bOutStyles = FALSE;
        Insert( aItemSet, nPos, nPos+1, rFmtInfos, FALSE, FALSE );
        bOutStyles = bOutStylesOld;
    }
}

void HTMLEndPosLst::OutStartAttrs( SwHTMLWriter& rHWrt, xub_StrLen nPos )
{
    rHWrt.bTagOn = TRUE;

    // die Attribute in der Start-Liste sind aufsteigend sortiert
    for( USHORT i=0; i< aStartLst.Count(); i++ )
    {
        HTMLSttEndPos *pPos = aStartLst[i];
        xub_StrLen nStart = pPos->GetStart();
        if( nStart > nPos )
        {
            // dieses und alle folgenden Attribute werden erst noch geoeffnet
            break;
        }
        else if( nStart == nPos )
        {
            // das Attribut ausgeben
            Out( aHTMLAttrFnTab, *pPos->GetItem(), rHWrt );
        }
    }
}

void HTMLEndPosLst::OutEndAttrs( SwHTMLWriter& rHWrt, xub_StrLen nPos )
{
    rHWrt.bTagOn = FALSE;

    // die Attribute in der End-Liste sind aufsteigend sortiert
    USHORT i=0;
    while( i < aEndLst.Count() )
    {
        HTMLSttEndPos *pPos = aEndLst[i];
        xub_StrLen nEnd = pPos->GetEnd();

        if( STRING_MAXLEN==nPos || nEnd == nPos )
        {
            Out( aHTMLAttrFnTab, *pPos->GetItem(), rHWrt );
            _RemoveItem( i );
        }
        else if( nEnd > nPos )
        {
            // dieses und alle folgenden Attribute werden erst spaeter beendet
            break;
        }
        else
        {
            // Das Attribut wird vor der aktuellen Position beendet. Das
            // darf nicht sein, aber wie koennen trotzdem damit umgehen
            ASSERT( nEnd >= nPos,
                    "Das Attribut sollte schon laengst beendet sein" );
            i++;
        }
    }
}


/* Ausgabe der Nodes */
Writer& OutHTML_SwTxtNode( Writer& rWrt, const SwCntntNode& rNode )
{
    SwTxtNode * pNd = &((SwTxtNode&)rNode);
    SwHTMLWriter & rHTMLWrt = (SwHTMLWriter&)rWrt;

    const String& rStr = pNd->GetTxt();
    xub_StrLen nEnde = rStr.Len();

    // Besonderheit: leere Node und HR-Vorlage (horizontaler Strich)
    //              nur ein <HR> ausgeben
    USHORT nPoolId = pNd->GetAnyFmtColl().GetPoolFmtId();

    if( !nEnde && (RES_POOLCOLL_HTML_HR==nPoolId ||
                   pNd->GetAnyFmtColl().GetName().EqualsAscii( sHTML_horzrule) ) )
    {
        // dann die absatz-gebundenen Grafiken/OLE-Objekte im Absatz
        // MIB 8.7.97: Ein <PRE> spannen wir um die Linie auf. Dann stimmen
        // zwar die Abstaende nicht, aber sonst bekommen wir einen leeren
        // Absatz hinter dem <HR> und das ist noch unschoener.
        rHTMLWrt.ChangeParaToken( 0 );

        // Alle an dem Node verankerten Rahmen ausgeben
        rHTMLWrt.OutFlyFrm( rNode.GetIndex(), 0, HTML_POS_ANY );

        if( rHTMLWrt.bLFPossible )
            rHTMLWrt.OutNewLine(); // Absatz-Tag in eine neue Zeile

        rHTMLWrt.bLFPossible = TRUE;

        ByteString sOut( '<' );
        sOut += sHTML_horzrule;

        const SfxItemSet* pItemSet = pNd->GetpSwAttrSet();
        if( !pItemSet )
        {
            rWrt.Strm() << sOut.GetBuffer() << '>';
            return rHTMLWrt;
        }
        const SfxPoolItem* pItem;
        if( SFX_ITEM_SET == pItemSet->GetItemState( RES_LR_SPACE, FALSE, &pItem ))
        {
            USHORT nLeft = ((SvxLRSpaceItem*)pItem)->GetLeft();
            USHORT nRight = ((SvxLRSpaceItem*)pItem)->GetRight();
            if( nLeft || nRight )
            {
                const SwFrmFmt& rPgFmt =
                    rHTMLWrt.pDoc->GetPageDescFromPool( RES_POOLPAGE_HTML )->GetMaster();

                const SwFmtFrmSize& rSz   = rPgFmt.GetFrmSize();
                const SvxLRSpaceItem& rLR = rPgFmt.GetLRSpace();
                const SwFmtCol& rCol = rPgFmt.GetCol();

                long nPageWidth = rSz.GetWidth() - rLR.GetLeft() - rLR.GetRight();

                if( 1 < rCol.GetNumCols() )
                    nPageWidth /= rCol.GetNumCols();

                const SwTableNode* pTblNd = pNd->FindTableNode();
                if( pTblNd )
                {
                    const SwTableBox* pBox = pTblNd->GetTable().GetTblBox(
                                    pNd->StartOfSectionIndex() );
                    if( pBox )
                        nPageWidth = pBox->GetFrmFmt()->GetFrmSize().GetWidth();
                }

                ((sOut += ' ') += sHTML_O_width) += '=';
                rWrt.Strm() << sOut.GetBuffer();
                rWrt.OutULong( rHTMLWrt.ToPixel(nPageWidth-nLeft-nRight) );

                ((sOut = ' ') += sHTML_O_align) += '=';
                if( !nLeft )
                    sOut += sHTML_AL_left;
                else if( !nRight )
                    sOut += sHTML_AL_right;
                else
                    sOut += sHTML_AL_center;
            }
        }
        rWrt.Strm() << sOut.GetBuffer();
        if( SFX_ITEM_SET == pItemSet->GetItemState( RES_BOX, FALSE, &pItem ))
        {
            const SvxBoxItem* pBoxItem = (const SvxBoxItem*)pItem;
            const SvxBorderLine* pBorderLine = pBoxItem->GetBottom();
            if( pBorderLine )
            {
                USHORT nWidth = pBorderLine->GetOutWidth() +
                                pBorderLine->GetInWidth() +
                                pBorderLine->GetDistance();
                ((sOut = ' ') += sHTML_O_size) += '=';
                rWrt.Strm() << sOut.GetBuffer();
                rWrt.OutULong( rHTMLWrt.ToPixel(nWidth) );

                const Color& rBorderColor = pBorderLine->GetColor();
                if( !rBorderColor.IsRGBEqual( Color(COL_GRAY) ) )
                {
                    ((sOut = ' ') += sHTML_O_color) += '=';
                    rWrt.Strm() << sOut.GetBuffer();
                    HTMLOutFuncs::Out_Color( rWrt.Strm(), rBorderColor,
                                             rHTMLWrt.eDestEnc );
                }

                if( !pBorderLine->GetInWidth() )
                {
                    (sOut = ' ') += sHTML_O_noshade;
                    rWrt.Strm() << sOut.GetBuffer();
                }
            }
        }
        rWrt.Strm() << '>';
        return rHTMLWrt;
    }

    // Die leeren Nodes mit 2pt Font-Hoehe und der Stand-Vorlage, die
    // vor Tabellen und Bereichen eingefuegt werden, nicht exportieren,
    // Bookmarks oder absatzgebundene Grafiken aber schon.
    // MIB 21.7.97: Ausserdem auch keine leeren Tabellen-Zellen exportieren.
    if( !nEnde && (nPoolId == RES_POOLCOLL_STANDARD ||
                   nPoolId == RES_POOLCOLL_TABLE ||
                   nPoolId == RES_POOLCOLL_TABLE_HDLN) )
    {
        // Der aktuelle Node ist leer und enthaelt Standard-Vorlage ...
        const SfxPoolItem* pItem;
        const SfxItemSet *pItemSet = pNd->GetpSwAttrSet();
        if( pItemSet && pItemSet->Count() &&
            SFX_ITEM_SET == pItemSet->GetItemState( RES_CHRATR_FONTSIZE, FALSE, &pItem ) &&
            40 == ((const SvxFontHeightItem *)pItem)->GetHeight() )
        {
            // ... ausserdem ist die 2pt Schrift eingestellt ...
            ULONG nNdPos = rWrt.pCurPam->GetPoint()->nNode.GetIndex();
            const SwNode *pNextNd = rWrt.pDoc->GetNodes()[nNdPos+1];
            const SwNode *pPrevNd = rWrt.pDoc->GetNodes()[nNdPos-1];
            BOOL bStdColl = nPoolId == RES_POOLCOLL_STANDARD;
            if( ( bStdColl && (pNextNd->IsTableNode() ||
                               pNextNd->IsSectionNode()) ) ||
                ( !bStdColl && pNextNd->IsEndNode() &&
                               pPrevNd->IsStartNode() &&
                               SwTableBoxStartNode==
                                pPrevNd->GetStartNode()->GetStartNodeType() ) )
            {
                // ... und er steht vor einer Tabelle ohne einem Bereich
                rHTMLWrt.OutBookmarks();
                rHTMLWrt.bLFPossible = !rHTMLWrt.nLastParaToken;

                // Alle an dem Node verankerten Rahmen ausgeben
                rHTMLWrt.OutFlyFrm( rNode.GetIndex(), 0, HTML_POS_ANY );
                rHTMLWrt.bLFPossible = FALSE;

                return rWrt;
            }
        }
    }

    // PagePreaks uns PagDescs abfangen
    BOOL bPageBreakBehind = FALSE;
    if( rHTMLWrt.bCfgFormFeed &&
        !(rHTMLWrt.bOutTable || rHTMLWrt.GetFlyFrmFmt()) &&
        rHTMLWrt.pStartNdIdx->GetIndex() !=
        rHTMLWrt.pCurPam->GetPoint()->nNode.GetIndex() )
    {
        BOOL bPageBreakBefore = FALSE;
        const SfxPoolItem* pItem;
        const SfxItemSet* pItemSet = pNd->GetpSwAttrSet();

        if( pItemSet )
        {
            if( SFX_ITEM_SET ==
                pItemSet->GetItemState( RES_PAGEDESC, TRUE, &pItem ) &&
                ((SwFmtPageDesc *)pItem)->GetPageDesc() )
                bPageBreakBefore = TRUE;
            else if( SFX_ITEM_SET ==
                     pItemSet->GetItemState( RES_BREAK, TRUE, &pItem ) )
            {
                switch( ((SvxFmtBreakItem *)pItem)->GetBreak() )
                {
                case SVX_BREAK_PAGE_BEFORE:
                    bPageBreakBefore = TRUE;
                    break;
                case SVX_BREAK_PAGE_AFTER:
                    bPageBreakBehind = TRUE;
                    break;
                case SVX_BREAK_PAGE_BOTH:
                    bPageBreakBefore = TRUE;
                    bPageBreakBehind = TRUE;
                    break;
                }
            }
        }

        if( bPageBreakBefore )
            rWrt.Strm() << '\f';
    }

    // eventuell eine Form oeffnen
    rHTMLWrt.OutForm();

    // An dem Node "verankerte" Seitenegebunde Rahmen ausgeben
    BOOL bFlysLeft = rHTMLWrt.OutFlyFrm( rNode.GetIndex(),
                                         0, HTML_POS_PREFIX );
    // An dem Node verankerte Rahmen ausgeben, die vor dem
    // Absatz-Tag geschrieben werden sollen.
    if( bFlysLeft )
        bFlysLeft = rHTMLWrt.OutFlyFrm( rNode.GetIndex(),
                                        0, HTML_POS_BEFORE );

    if( rHTMLWrt.pCurPam->GetPoint()->nNode == rHTMLWrt.pCurPam->GetMark()->nNode )
        nEnde = rHTMLWrt.pCurPam->GetMark()->nContent.GetIndex();

    // gibt es harte Attribute, die als Optionen geschrieben werden muessen?
    rHTMLWrt.bTagOn = TRUE;

    // jetzt das Tag des Absatzes ausgeben
    const SwFmt& rFmt = pNd->GetAnyFmtColl();
    SwHTMLTxtCollOutputInfo aFmtInfo;
    BOOL bOldLFPossible = rHTMLWrt.bLFPossible;
    OutHTML_SwFmt( rWrt, rFmt, pNd->GetpSwAttrSet(), aFmtInfo );

    // Wenn vor dem Absatz-Tag keine neue Zeile aufgemacht wurde, dann
    // tun wir das jetzt
    rHTMLWrt.bLFPossible = !rHTMLWrt.nLastParaToken;
    if( !bOldLFPossible && rHTMLWrt.bLFPossible )
        rHTMLWrt.OutNewLine();


    // dann die Bookmarks (inkl. End-Tag)
    rHTMLWrt.bOutOpts = FALSE;
    rHTMLWrt.OutBookmarks();

    // jetzt ist noch mal eine gute Gelegenheit fuer ein LF, sofern es noch
    // erlaubt ist
    if( rHTMLWrt.bLFPossible &&
        rHTMLWrt.GetLineLen() >= rHTMLWrt.nWhishLineLen )
    {
        rHTMLWrt.OutNewLine();
    }
    rHTMLWrt.bLFPossible = FALSE;

    // Text, der aus einer Outline-Numerierung kommt ermitteln
    xub_StrLen nOffset = 0;
    String aOutlineTxt;
    if( pNd->GetOutlineNum() && !pNd->GetNum() )
    {
        aOutlineTxt = pNd->GetNumString();
        nOffset += aOutlineTxt.Len();
    }
    if( rHTMLWrt.pFmtFtn )
        nOffset += rHTMLWrt.GetFootEndNoteSymLen( *rHTMLWrt.pFmtFtn );

    // gibt es harte Attribute, die als Tags geschrieben werden muessen?
    HTMLEndPosLst aEndPosLst( rWrt.pDoc, rHTMLWrt.pTemplate,
                              rHTMLWrt.pDfltColor, rHTMLWrt.bCfgOutStyles,
                              rHTMLWrt.GetHTMLMode() );
    if( aFmtInfo.pItemSet )
    {
        aEndPosLst.Insert( *aFmtInfo.pItemSet, 0, nEnde + nOffset,
                           rHTMLWrt.aChrFmtInfos, FALSE, TRUE );
    }


    if( aOutlineTxt.Len() || rHTMLWrt.pFmtFtn )
    {
        // Absatz-Attribute ausgeben, damit der Text die Attribute des
        // Absatzes bekommt.
        aEndPosLst.OutStartAttrs( rHTMLWrt, 0 );

        // Theoretisch muesste man hier die Zeichen-Vorlage der Numerierung
        // beachten. Da man die ueber die UI nicht setzen kann, ignorieren
        // wir sie erstmal.

        if( aOutlineTxt.Len() )
            HTMLOutFuncs::Out_String( rWrt.Strm(), aOutlineTxt,
                                         rHTMLWrt.eDestEnc );

        if( rHTMLWrt.pFmtFtn )
        {
            rHTMLWrt.OutFootEndNoteSym( *rHTMLWrt.pFmtFtn );
            rHTMLWrt.pFmtFtn = 0;
        }
    }

    // erstmal den Start berichtigen. D.h. wird nur ein Teil vom Satz
    // ausgegeben, so muessen auch da die Attribute stimmen!!
    rHTMLWrt.bTxtAttr = TRUE;

    USHORT nAttrPos = 0;
    xub_StrLen nStrPos = rHTMLWrt.pCurPam->GetPoint()->nContent.GetIndex();
    const SwTxtAttr * pHt = 0;
    USHORT nCntAttr = pNd->HasHints() ? pNd->GetSwpHints().Count() : 0;
    if( nCntAttr && nStrPos > *( pHt = pNd->GetSwpHints()[ 0 ] )->GetStart() )
    {
        // Ok, es gibt vorher Attribute, die ausgegeben werden muessen
        do {
            aEndPosLst.OutEndAttrs( rHTMLWrt, nStrPos + nOffset );

            nAttrPos++;
            if( RES_TXTATR_FIELD == pHt->Which() )      // Felder nicht
                continue;                               // ausgeben

            if( pHt->GetEnd() )
            {
                xub_StrLen nHtEnd = *pHt->GetEnd(),
                       nHtStt = *pHt->GetStart();
                if( !rHTMLWrt.bWriteAll && nHtEnd <= nStrPos )
                    continue;

                // leere Hints am Anfang nicht beachten, oder ??
                if( nHtEnd == nHtStt )
                    continue;

                // Attribut in die Liste aufnehemen
                if( rHTMLWrt.bWriteAll )
                    aEndPosLst.Insert( pHt->GetAttr(), nHtStt + nOffset,
                                       nHtEnd + nOffset,
                                       rHTMLWrt.aChrFmtInfos );
                else
                {
                    xub_StrLen nTmpStt = nHtStt < nStrPos ? nStrPos : nHtStt;
                    xub_StrLen nTmpEnd = nHtEnd < nEnde ? nHtEnd : nEnde;
                    aEndPosLst.Insert( pHt->GetAttr(), nTmpStt + nOffset,
                                       nTmpEnd + nOffset,
                                       rHTMLWrt.aChrFmtInfos );
                }
                continue;
                // aber nicht ausgeben, das erfolgt spaeter !!
            }

        } while( nAttrPos < nCntAttr && nStrPos >
            *( pHt = pNd->GetSwpHints()[ nAttrPos ] )->GetStart() );

        // dann gebe mal alle gesammelten Attribute von der String-Pos aus
        aEndPosLst.OutEndAttrs( rHTMLWrt, nStrPos + nOffset );
        aEndPosLst.OutStartAttrs( rHTMLWrt, nStrPos + nOffset );
    }

    BOOL bWriteBreak = (HTML_PREFORMTXT_ON != rHTMLWrt.nLastParaToken);
    if( bWriteBreak && pNd->GetNum() &&
        NO_NUMBERING != pNd->GetNum()->GetLevel() )
        bWriteBreak = FALSE;

    xub_StrLen nPreSplitPos = 0;
    for( ; nStrPos < nEnde; nStrPos++ )
    {
        aEndPosLst.OutEndAttrs( rHTMLWrt, nStrPos + nOffset );

        // Die an der aktuellen Position verankerten Rahmen ausgeben
        if( bFlysLeft )
            bFlysLeft = rHTMLWrt.OutFlyFrm( rNode.GetIndex(),
                                            nStrPos, HTML_POS_INSIDE );

        BOOL bOutChar = TRUE;
        const SwTxtAttr * pTxtHt = 0;
        if( nAttrPos < nCntAttr && *pHt->GetStart() == nStrPos
            && nStrPos != nEnde )
        {
            do {
                if( pHt->GetEnd() )
                {
                    if( RES_CHRATR_KERNING == pHt->Which() &&
                        rHTMLWrt.IsHTMLMode(HTMLMODE_FIRSTLINE) &&
                        *pHt->GetEnd() - nStrPos == 1 &&
                        ' ' == rStr.GetChar(nStrPos) &&
                        ((const SvxKerningItem&)pHt->GetAttr()).GetValue() > 0 )
                    {
                        // Wenn erlaubt, wird das Ding als Spacer exportiert

                        bOutChar = FALSE;   // Space nicht ausgeben
                        bWriteBreak = FALSE;    // der Absatz ist aber auch nicht leer

                        OutHTML_HoriSpacer( rWrt,
                            ((const SvxKerningItem&)pHt->GetAttr()).GetValue() );

                        // Der Hint braucht nun doch nicht weiter
                        // beruecksichtigt werden.
                    }
                    else if( *pHt->GetEnd() != nStrPos )
                    {
                        // Hints mit Ende einsortieren, wenn sie keinen
                        // leeren Bereich aufspannen (Hints, die keinen
                        // Bereich aufspannen werden ignoriert
                        aEndPosLst.Insert( pHt->GetAttr(), nStrPos + nOffset,
                                           *pHt->GetEnd() + nOffset,
                                           rHTMLWrt.aChrFmtInfos );
                    }
                    else
                    {
#if 0
                        // leere Hinst gleich ausgeben
                        rHTMLWrt.bTagOn = TRUE;
                        Out( aHTMLAttrFnTab, pHt->GetAttr(), rHTMLWrt );
                        rHTMLWrt.bTagOn = FALSE;
                        Out( aHTMLAttrFnTab, pHt->GetAttr(), rHTMLWrt );
#endif
                    }
                }
                else
                {
                    // Hints ohne-Ende werden als letztes ausgebeben
                    ASSERT( !pTxtHt,
                            "Wieso gibt es da schon ein Attribut ohne Ende?" );
                    if( rHTMLWrt.nTxtAttrsToIgnore>0 )
                    {
                        rHTMLWrt.nTxtAttrsToIgnore--;
                    }
                    else
                    {
                        pTxtHt = pHt;
                        bWriteBreak = FALSE;    // der Absatz ist aber auch nicht leer
                    }
                    bOutChar = FALSE;       // keine 255 ausgeben
                }
            } while( ++nAttrPos < nCntAttr && nStrPos ==
                *( pHt = pNd->GetSwpHints()[ nAttrPos ] )->GetStart() );
        }

        // Manche Draw-Formate koennen auch noch Attribute mitbringen
        if( pTxtHt && RES_TXTATR_FLYCNT == pTxtHt->Which() )
        {
            const SwFrmFmt* pFrmFmt =
                ((const SwFmtFlyCnt &)pTxtHt->GetAttr()).GetFrmFmt();

            if( RES_DRAWFRMFMT == pFrmFmt->Which() )
                aEndPosLst.Insert( *((const SwDrawFrmFmt *)pFrmFmt),
                                    nStrPos + nOffset,
                                    rHTMLWrt.aChrFmtInfos );
        }

        aEndPosLst.OutEndAttrs( rHTMLWrt, nStrPos + nOffset );
        aEndPosLst.OutStartAttrs( rHTMLWrt, nStrPos + nOffset );

        if( pTxtHt )
        {
            rHTMLWrt.bLFPossible = !rHTMLWrt.nLastParaToken && nStrPos > 0 &&
                                   rStr.GetChar(nStrPos-1) == ' ';
            Out( aHTMLAttrFnTab, pTxtHt->GetAttr(), rHTMLWrt );
            rHTMLWrt.bLFPossible = FALSE;
        }

        if( bOutChar )
        {
            sal_Unicode c = rStr.GetChar( nStrPos );
            // versuche nach ungefaehr 255 Zeichen eine neue Zeile zu
            // beginnen, aber nicht in PRE und nur bei Spaces
            // MIB 15.4.99: Auf Wunsch von ST wird in Mails auch in einem
            // PRE ganz bewusst umgebrocheni (#64159#).
            if( ' '==c && (!rHTMLWrt.nLastParaToken || rHTMLWrt.HasCId()) )
            {
                xub_StrLen nLineLen;
                if( rHTMLWrt.nLastParaToken )
                    nLineLen = nStrPos - nPreSplitPos;
                else
                    nLineLen = rHTMLWrt.GetLineLen();

                xub_StrLen nWordLen = rStr.Search( ' ', nStrPos+1 );
                if( nWordLen == STRING_NOTFOUND )
                    nWordLen = nEnde;
                nWordLen -= nStrPos;

                if( nLineLen >= rHTMLWrt.nWhishLineLen ||
                    (nLineLen+nWordLen) >= rHTMLWrt.nWhishLineLen )
                {
                    rHTMLWrt.OutNewLine();
                    bOutChar = FALSE;
                    if( rHTMLWrt.nLastParaToken )
                        nPreSplitPos = nStrPos+1;
                }
            }

            if( bOutChar )
            {
                if( 0x0a == c )
                    HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), sHTML_linebreak );
                else
                    HTMLOutFuncs::Out_Char( rWrt.Strm(), c, rHTMLWrt.eDestEnc );

                // Wenn das letzte Zeichen eines Absatzed ein harter
                // Zeilen-Umbruch ist brauchen wir noch ein <BR> mehr, weil
                // Netscape & Co in diesem Fall fuer den naechsten Absatz
                // nicht in die naechste Zeile gehen.
                bWriteBreak = (0x0a == c) &&
                              (HTML_PREFORMTXT_ON != rHTMLWrt.nLastParaToken);
            }
        }
    }


    BOOL bEndAttrsWritten = aEndPosLst.Count() > 0;
    aEndPosLst.OutEndAttrs( rHTMLWrt, STRING_MAXLEN );

    // Die an der letzten Position verankerten Rahmen ausgeben
    if( bFlysLeft )
        bFlysLeft = rHTMLWrt.OutFlyFrm( rNode.GetIndex(),
                                       nEnde, HTML_POS_INSIDE );
    ASSERT( !bFlysLeft, "Es wurden nicht alle Rahmen gespeichert!" );

    rHTMLWrt.bTxtAttr = FALSE;

    if( bWriteBreak )
    {
        BOOL bEndOfCell = rHTMLWrt.bOutTable &&
                         rWrt.pCurPam->GetPoint()->nNode.GetIndex() ==
                         rWrt.pCurPam->GetMark()->nNode.GetIndex();

        if( bEndOfCell && !nEnde &&
            rHTMLWrt.IsHTMLMode(HTMLMODE_NBSP_IN_TABLES) )
        {
            // Wenn der letzte Absatz einer Tabellezelle leer ist und
            // wir fuer den MS-IE exportieren, schreiben wir statt eines
            // <BR> ein &nbsp;
            rWrt.Strm() << '&' << sHTML_S_nbsp << ';';
        }
        else
        {
            HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), sHTML_linebreak );
            const SvxULSpaceItem& rULSpace =
                (const SvxULSpaceItem &)pNd->GetSwAttrSet().Get(RES_UL_SPACE);
            if( rULSpace.GetLower() > 0 && !bEndOfCell &&
                !rHTMLWrt.IsHTMLMode(HTMLMODE_NO_BR_AT_PAREND) )
                HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), sHTML_linebreak );
            rHTMLWrt.bLFPossible = TRUE;
        }
    }

    if( rHTMLWrt.bClearLeft || rHTMLWrt.bClearRight )
    {
        const sal_Char *pStr;
        if( rHTMLWrt.bClearLeft )
        {
            if( rHTMLWrt.bClearRight )
                pStr = sHTML_AL_all;
            else
                pStr = sHTML_AL_left;
        }
        else
            pStr = sHTML_AL_right;

        ByteString sOut( sHTML_linebreak );
        (((sOut += ' ') += sHTML_O_clear) += '=') += pStr;

        HTMLOutFuncs::Out_AsciiTag( rHTMLWrt.Strm(), sOut.GetBuffer() );
        rHTMLWrt.bClearLeft = FALSE;
        rHTMLWrt.bClearRight = FALSE;

        rHTMLWrt.bLFPossible = TRUE;
    }

    // wenn ein LF nicht schon erlaubt ist wird es erlaubt, wenn der
    // Absatz mit einem ' ' endet
    if( !rHTMLWrt.bLFPossible && !rHTMLWrt.nLastParaToken &&
        nEnde > 0 && ' ' == rStr.GetChar(nEnde-1) )
        rHTMLWrt.bLFPossible = TRUE;

    rHTMLWrt.bTagOn = FALSE;
    OutHTML_SwFmtOff( rWrt, aFmtInfo );

    // eventuell eine Form schliessen
    rHTMLWrt.OutForm( FALSE );

    if( bPageBreakBehind )
        rWrt.Strm() << '\f';

    return rHTMLWrt;
}


ULONG SwHTMLWriter::ToPixel( ULONG nVal ) const
{
    if( Application::GetDefaultDevice() && nVal )
    {
        nVal = Application::GetDefaultDevice()->LogicToPixel(
                    Size( nVal, nVal ), MapMode( MAP_TWIP ) ).Width();
        if( !nVal )     // wo ein Twip ist sollte auch ein Pixel sein
            nVal = 1;
    }
    return nVal;
}


static Writer& OutHTML_CSS1Attr( Writer& rWrt, const SfxPoolItem& rHt )
{
    // wenn gerade Hints geschrieben werden versuchen wir den Hint als
    // CSS1-Attribut zu schreiben

    if( ((SwHTMLWriter&)rWrt).bCfgOutStyles && ((SwHTMLWriter&)rWrt).bTxtAttr )
        OutCSS1_HintSpanTag( rWrt, rHt );

    return rWrt;
}


/* File CHRATR.HXX: */

static Writer& OutHTML_SvxColor( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwHTMLWriter& rHTMLWrt = (SwHTMLWriter&)rWrt;
    if( rHTMLWrt.bOutOpts )
        return rWrt;

    const Color& rColor = ((const SvxColorItem&)rHt).GetValue();

    // die Default-Farbe nur Schreiben, wenn sie als Hint vorkommt
    //if( rHTMLWrt.bTagOn && !rHTMLWrt.bTxtAttr && rHTMLWrt.pDfltColor
    //  && rColor == *rHTMLWrt.pDfltColor )
    //  return rWrt;

    if( !rHTMLWrt.bTxtAttr && rHTMLWrt.bCfgOutStyles && rHTMLWrt.bCfgPreferStyles )
    {
        // Font-Farbe nicht als Tag schreiben, wenn Styles normalen Tags
        // vorgezogen werden
        return rWrt;
    }

    if( rHTMLWrt.bTagOn )
    {
        ByteString sOut( '<' );
        (((sOut += sHTML_font) += ' ') += sHTML_O_color) += '=';
        rWrt.Strm() << sOut.GetBuffer();
        HTMLOutFuncs::Out_Color( rWrt.Strm(), rColor ) << '>';
    }
    else
        HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), sHTML_font, FALSE );

    return rWrt;
}


static Writer& OutHTML_SwPosture( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwHTMLWriter& rHTMLWrt = (SwHTMLWriter&)rWrt;
    if( rHTMLWrt.bOutOpts )
        return rWrt;

    const FontItalic nPosture = ((const SvxPostureItem&)rHt).GetPosture();
    if( ITALIC_NORMAL == nPosture )
    {
        HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), sHTML_italic, rHTMLWrt.bTagOn );
    }
    else if( rHTMLWrt.bCfgOutStyles && rHTMLWrt.bTxtAttr )
    {
        // vielleicht als CSS1-Attribut ?
        OutCSS1_HintSpanTag( rWrt, rHt );
    }

    return rWrt;
}

static Writer& OutHTML_SvxFont( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwHTMLWriter& rHTMLWrt = (SwHTMLWriter&)rWrt;
    if( rHTMLWrt.bOutOpts )
        return rWrt;

    if( rHTMLWrt.bTagOn )
    {
        String aNames;
        SwHTMLWriter::PrepareFontList( ((const SvxFontItem&)rHt), aNames, 0,
                           rHTMLWrt.IsHTMLMode(HTMLMODE_FONT_GENERIC) );
        ByteString sOut( '<' );
        (((sOut += sHTML_font) += ' ') += sHTML_O_face) += "=\"";
        rWrt.Strm() << sOut.GetBuffer();
        HTMLOutFuncs::Out_String( rWrt.Strm(), aNames, rHTMLWrt.eDestEnc )
            << "\">";
    }
    else
        HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), sHTML_font , FALSE );

    return rWrt;
}

static Writer& OutHTML_SvxFontHeight( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwHTMLWriter& rHTMLWrt = (SwHTMLWriter&)rWrt;
    if( rHTMLWrt.bOutOpts )
        return rWrt;

    if( rHTMLWrt.bTagOn )
    {
        ByteString sOut( '<' );
        sOut += sHTML_font;

        UINT32 nHeight = ((const SvxFontHeightItem&)rHt).GetHeight();
        USHORT nSize = rHTMLWrt.GetHTMLFontSize( nHeight );
        (((sOut += ' ') += sHTML_O_size) += '=')
            += ByteString::CreateFromInt32( nSize );
        rWrt.Strm() << sOut.GetBuffer();

        if( rHTMLWrt.bCfgOutStyles && rHTMLWrt.bTxtAttr &&
            rHTMLWrt.aFontHeights[nSize-1] != nHeight )
        {
            // wenn die Groesse keiner HTML-Groesse entspricht,
            // wird sie noch zusatzlich als Style-Option exportiert
            OutCSS1_HintStyleOpt( rWrt, rHt );
        }
        rWrt.Strm() << '>';
    }
    else
    {
        HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), sHTML_font, FALSE );
    }

    return rWrt;
}

static Writer& OutHTML_SwWeight( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwHTMLWriter& rHTMLWrt = (SwHTMLWriter&)rWrt;
    if( rHTMLWrt.bOutOpts )
        return rWrt;

    const FontWeight nBold = ((const SvxWeightItem&)rHt).GetWeight();
    if( WEIGHT_BOLD == nBold )
    {
        HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), sHTML_bold, rHTMLWrt.bTagOn );
    }
    else if( rHTMLWrt.bCfgOutStyles && rHTMLWrt.bTxtAttr )
    {
        // vielleicht als CSS1-Attribut ?
        OutCSS1_HintSpanTag( rWrt, rHt );
    }

    return rWrt;
}


static Writer& OutHTML_SwCrossedOut( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwHTMLWriter& rHTMLWrt = (SwHTMLWriter&)rWrt;
    if( rHTMLWrt.bOutOpts )
        return rWrt;

    // Wegen Netscape schrieben wir hier STRIKE und nicht S raus!
    const FontStrikeout nStrike = ((const SvxCrossedOutItem&)rHt).GetStrikeout();
    if( STRIKEOUT_NONE != nStrike )
    {
        HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), sHTML_strike, rHTMLWrt.bTagOn );
    }
    else if( rHTMLWrt.bCfgOutStyles && rHTMLWrt.bTxtAttr )
    {
        // vielleicht als CSS1-Attribut ?
        OutCSS1_HintSpanTag( rWrt, rHt );
    }

    return rWrt;
}


static Writer& OutHTML_SvxEscapement( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwHTMLWriter& rHTMLWrt = (SwHTMLWriter&)rWrt;
    if( rHTMLWrt.bOutOpts )
        return rWrt;

    const SvxEscapement eEscape =
        (const SvxEscapement)((const SvxEscapementItem&)rHt).GetEnumValue();
    const sal_Char *pStr = 0;
    switch( eEscape )
    {
    case SVX_ESCAPEMENT_SUPERSCRIPT: pStr = sHTML_superscript; break;
    case SVX_ESCAPEMENT_SUBSCRIPT: pStr = sHTML_subscript; break;
    }

    if( pStr )
    {
        HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), pStr, rHTMLWrt.bTagOn );
    }
    else if( rHTMLWrt.bCfgOutStyles && rHTMLWrt.bTxtAttr )
    {
        // vielleicht als CSS1-Attribut ?
        OutCSS1_HintSpanTag( rWrt, rHt );
    }

    return rWrt;
}



static Writer& OutHTML_SwUnderline( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwHTMLWriter& rHTMLWrt = (SwHTMLWriter&)rWrt;
    if( rHTMLWrt.bOutOpts )
        return rWrt;

    const FontUnderline eUnder = ((const SvxUnderlineItem&)rHt).GetUnderline();
    if( UNDERLINE_NONE != eUnder )
    {
        HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), sHTML_underline, rHTMLWrt.bTagOn );
    }
    else if( rHTMLWrt.bCfgOutStyles && rHTMLWrt.bTxtAttr )
    {
        // vielleicht als CSS1-Attribut ?
        OutCSS1_HintSpanTag( rWrt, rHt );
    }

    return rWrt;
}


static Writer& OutHTML_SwFlyCnt( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwHTMLWriter & rHTMLWrt = (SwHTMLWriter&)rWrt;
    SwFmtFlyCnt& rFlyCnt = (SwFmtFlyCnt&)rHt;

    const SwFrmFmt& rFmt = *rFlyCnt.GetFrmFmt();
    const SdrObject *pSdrObj = 0;

    SwHTMLFrmType eType =
        (SwHTMLFrmType)rHTMLWrt.GuessFrmType( rFmt, pSdrObj );
    BYTE nMode = aHTMLOutFrmAsCharTable[eType][rHTMLWrt.nExportMode];
    rHTMLWrt.OutFrmFmt( nMode, rFmt, pSdrObj );
    return rWrt;
}

static Writer& OutHTML_SwHardBlank( Writer& rWrt, const SfxPoolItem& rHt )
{
    HTMLOutFuncs::Out_Char( rWrt.Strm(), ((SwFmtHardBlank&)rHt).GetChar(),
                            ((SwHTMLWriter&)rWrt).eDestEnc );
    return rWrt;
}

// Das ist jetzt unser Blink-Item. Blinkend wird eingeschaltet, indem man
// das Item auf TRUE setzt!
static Writer& OutHTML_SwBlink( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwHTMLWriter& rHTMLWrt = (SwHTMLWriter&)rWrt;
    if( rHTMLWrt.bOutOpts || !rHTMLWrt.IsHTMLMode(HTMLMODE_BLINK) )
        return rWrt;

    if( ((const SvxBlinkItem&)rHt).GetValue() )
    {
        HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), sHTML_blink, rHTMLWrt.bTagOn );
    }
    else if( rHTMLWrt.bCfgOutStyles && rHTMLWrt.bTxtAttr )
    {
        // vielleicht als CSS1-Attribut ?
        OutCSS1_HintSpanTag( rWrt, rHt );
    }

    return rWrt;
}

Writer& OutHTML_INetFmt( Writer& rWrt, const SwFmtINetFmt& rINetFmt, BOOL bOn )
{
    SwHTMLWriter& rHTMLWrt = (SwHTMLWriter&)rWrt;

    String aURL( rINetFmt.GetValue() );
    const SvxMacroTableDtor *pMacTable = rINetFmt.GetMacroTbl();
    BOOL bEvents = pMacTable != 0 && pMacTable->Count() > 0;

    // Gibt es ueberhaupt etwas auszugeben?
    if( !aURL.Len() && !bEvents && !rINetFmt.GetName().Len() )
        return rWrt;

    // Tag aus? Dann nur ein </A> ausgeben.
    if( !bOn )
    {
        HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(), sHTML_anchor, FALSE );
        return rWrt;
    }

    ByteString sOut( '<' );
    sOut += sHTML_anchor;
    rWrt.Strm() << sOut.GetBuffer();

#define REL_HACK
#ifdef REL_HACK
    String sRel;
#endif

    if( aURL.Len() || bEvents )
    {
#ifdef REL_HACK
        String sTmp( aURL );
        sTmp.ToUpperAscii();
        xub_StrLen nPos = sTmp.SearchAscii( "\" REL=" );
        if( nPos!=STRING_NOTFOUND )
        {
            sRel = aURL.Copy( nPos+1 );
            aURL.Erase( nPos );
        }
#endif
        aURL.EraseLeadingChars().EraseTrailingChars();

        ((sOut = ' ') += sHTML_O_href) += "=\"";
        rWrt.Strm() << sOut.GetBuffer();
        aURL = INetURLObject::AbsToRel( aURL, INetURLObject::WAS_ENCODED,
                                        INetURLObject::DECODE_WITH_CHARSET );
        HTMLOutFuncs::Out_String( rWrt.Strm(), aURL, rHTMLWrt.eDestEnc );
        sOut = '\"';
    }
    else
        sOut.Erase();

    if( rINetFmt.GetName().Len() )
    {
        ((sOut += ' ') += sHTML_O_name) += "=\"";
        rWrt.Strm() << sOut.GetBuffer();
        HTMLOutFuncs::Out_String( rWrt.Strm(), rINetFmt.GetName(),
                                  rHTMLWrt.eDestEnc );
        sOut = '\"';
    }

    const String& rTarget = rINetFmt.GetTargetFrame();
    if( rTarget.Len() )
    {
        ((sOut += ' ') += sHTML_O_target) += "=\"";
        rWrt.Strm() << sOut.GetBuffer();
        HTMLOutFuncs::Out_String( rWrt.Strm(), rTarget, rHTMLWrt.eDestEnc );
        sOut = '\"';
    }

#ifdef REL_HACK
    if( sRel.Len() )
        sOut += ByteString( sRel, RTL_TEXTENCODING_ASCII_US );
#endif
    if( sOut.Len() )
        rWrt.Strm() << sOut.GetBuffer();

    if( bEvents )
        HTMLOutFuncs::Out_Events( rWrt.Strm(), *pMacTable, aAnchorEventTable,
                                  rHTMLWrt.bCfgStarBasic, rHTMLWrt.eDestEnc );
    rWrt.Strm() << ">";

    return rWrt;
}

static Writer& OutHTML_SwFmtINetFmt( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwHTMLWriter& rHTMLWrt = (SwHTMLWriter&)rWrt;

    if( rHTMLWrt.bOutOpts )
        return rWrt;

    const SwFmtINetFmt& rINetFmt = (const SwFmtINetFmt&)rHt;

    if( rHTMLWrt.bTagOn )
    {
        // ggf. ein noch offenes Attribut voruebergehend beenden
        if( rHTMLWrt.aINetFmts.Count() )
        {
            SwFmtINetFmt *pINetFmt =
                rHTMLWrt.aINetFmts[ rHTMLWrt.aINetFmts.Count()-1 ];
            OutHTML_INetFmt( rWrt, *pINetFmt, FALSE );
        }

        // jetzt das neue aufmachen
        OutHTML_INetFmt( rWrt, rINetFmt, TRUE );

        // und merken
        const SwFmtINetFmt *pINetFmt = new SwFmtINetFmt( rINetFmt );
        rHTMLWrt.aINetFmts.C40_INSERT( SwFmtINetFmt, pINetFmt,
                                       rHTMLWrt.aINetFmts.Count() );
    }
    else
    {
        // das
        OutHTML_INetFmt( rWrt, rINetFmt, FALSE );

        ASSERT( rHTMLWrt.aINetFmts.Count(), "da fehlt doch ein URL-Attribut" );
        if( rHTMLWrt.aINetFmts.Count() )
        {
            // das eigene Attribut vom Stack holen
            SwFmtINetFmt *pINetFmt =
                rHTMLWrt.aINetFmts[ rHTMLWrt.aINetFmts.Count()-1 ];

            rHTMLWrt.aINetFmts.Remove( rHTMLWrt.aINetFmts.Count()-1, 1 );
            delete pINetFmt;
        }

        if( rHTMLWrt.aINetFmts.Count() )
        {
            // es ist noch ein Attribut auf dem Stack, das wieder geoeffnet
            // werden muss
            SwFmtINetFmt *pINetFmt =
                rHTMLWrt.aINetFmts[ rHTMLWrt.aINetFmts.Count()-1 ];
            OutHTML_INetFmt( rWrt, *pINetFmt, TRUE );
        }
    }

    return rWrt;
}

static Writer& OutHTML_SwTxtCharFmt( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwHTMLWriter& rHTMLWrt = (SwHTMLWriter&)rWrt;
    if( rHTMLWrt.bOutOpts )
        return rWrt;

    const SwFmtCharFmt& rChrFmt = (const SwFmtCharFmt&)rHt;
    const SwCharFmt* pFmt = rChrFmt.GetCharFmt();

    if( !pFmt )
    {
        return rWrt;
    }

    SwHTMLFmtInfo aFmtInfo( pFmt );
    USHORT nPos;
    if( !rHTMLWrt.aChrFmtInfos.Seek_Entry( &aFmtInfo, &nPos ) )
        return rWrt;

    const SwHTMLFmtInfo *pFmtInfo = rHTMLWrt.aChrFmtInfos[nPos];
    ASSERT( pFmtInfo, "Wieso gint es keine Infos ueber die Zeichenvorlage?" );

    if( rHTMLWrt.bTagOn )
    {
        ByteString sOut( '<' );
        if( pFmtInfo->aToken.Len() > 0 )
            sOut += pFmtInfo->aToken;
        else
            sOut += sHTML_span;
        if( rHTMLWrt.bCfgOutStyles && pFmtInfo->aClass.Len() )
        {
            ((sOut += ' ') += sHTML_O_class) += "=\"";
            rWrt.Strm() << sOut.GetBuffer();
            HTMLOutFuncs::Out_String( rWrt.Strm(), pFmtInfo->aClass,
                                          rHTMLWrt.eDestEnc );
            sOut = '\"';
        }
        sOut += '>';
        rWrt.Strm() << sOut.GetBuffer();
    }
    else
    {
        HTMLOutFuncs::Out_AsciiTag( rWrt.Strm(),
                pFmtInfo->aToken.Len() ? pFmtInfo->aToken.GetBuffer()
                                       : sHTML_span,
                FALSE );
    }

    return rWrt;
}

static Writer& OutHTML_SvxAdjust( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwHTMLWriter & rHTMLWrt = (SwHTMLWriter&)rWrt;
    if( !rHTMLWrt.bOutOpts || !rHTMLWrt.bTagOn )
        return  rWrt;

    SvxAdjustItem& rAdjust = (SvxAdjustItem&)rHt;
    const sal_Char* pStr = 0;
    switch( rAdjust.GetAdjust() )
    {
    case SVX_ADJUST_CENTER: pStr = sHTML_AL_center; break;
    case SVX_ADJUST_LEFT: pStr = sHTML_AL_left; break;
    case SVX_ADJUST_RIGHT: pStr = sHTML_AL_right; break;
    case SVX_ADJUST_BLOCK: pStr = sHTML_AL_justify; break;
    }
    if( pStr )
    {
        ByteString sOut( ' ' );
        ((sOut += sHTML_O_align) += '=') += pStr;
        rWrt.Strm() << sOut.GetBuffer();
    }

    return rWrt;
}

/*
 * lege hier die Tabellen fuer die HTML-Funktions-Pointer auf
 * die Ausgabe-Funktionen an.
 * Es sind lokale Strukturen, die nur innerhalb der HTML-DLL
 * bekannt sein muessen.
 */


SwAttrFnTab aHTMLAttrFnTab = {
/* RES_CHRATR_CASEMAP   */          OutHTML_CSS1Attr,
/* RES_CHRATR_CHARSETCOLOR  */      0,
/* RES_CHRATR_COLOR */              OutHTML_SvxColor,
/* RES_CHRATR_CONTOUR   */          0,
/* RES_CHRATR_CROSSEDOUT    */      OutHTML_SwCrossedOut,
/* RES_CHRATR_ESCAPEMENT    */      OutHTML_SvxEscapement,
/* RES_CHRATR_FONT  */              OutHTML_SvxFont,
/* RES_CHRATR_FONTSIZE  */          OutHTML_SvxFontHeight,
/* RES_CHRATR_KERNING   */          OutHTML_CSS1Attr,
/* RES_CHRATR_LANGUAGE  */          0,
/* RES_CHRATR_POSTURE   */          OutHTML_SwPosture,
/* RES_CHRATR_PROPORTIONALFONTSIZE*/0,
/* RES_CHRATR_SHADOWED  */          0,
/* RES_CHRATR_UNDERLINE */          OutHTML_SwUnderline,
/* RES_CHRATR_WEIGHT    */          OutHTML_SwWeight,
/* RES_CHRATR_WORDLINEMODE  */      0,
/* RES_CHRATR_AUTOKERN  */          0,
/* RES_CHRATR_BLINK */              OutHTML_SwBlink,
/* RES_CHRATR_NOHYPHEN  */          0, // Neu: nicht trennen
/* RES_CHRATR_NOLINEBREAK */        0, // Neu: nicht umbrechen
/* RES_CHRATR_BACKGROUND */         OutHTML_CSS1Attr, // Neu: Zeichenhintergrund
/* RES_CHRATR_CJK_FONT */           0,
/* RES_CHRATR_CJK_FONTSIZE */       0,
/* RES_CHRATR_CJK_LANGUAGE */       0,
/* RES_CHRATR_CJK_POSTURE */        0,
/* RES_CHRATR_CJK_WEIGHT */         0,
/* RES_CHRATR_CTL_FONT */           0,
/* RES_CHRATR_CTL_FONTSIZE */       0,
/* RES_CHRATR_CTL_LANGUAGE */       0,
/* RES_CHRATR_CTL_POSTURE */        0,
/* RES_CHRATR_CTL_WEIGHT */         0,
/* RES_CHRATR_WRITING_DIRECTION */  0,
/* RES_CHRATR_DUMMY2 */             0,
/* RES_CHRATR_DUMMY3 */             0,
/* RES_CHRATR_DUMMY4 */             0,
/* RES_CHRATR_DUMMY5 */             0,
/* RES_CHRATR_DUMMY1 */             0, // Dummy:

/* RES_TXTATR_INETFMT   */          OutHTML_SwFmtINetFmt,
/* RES_TXTATR_DUMMY4    */          0,
/* RES_TXTATR_REFMARK*/             0,
/* RES_TXTATR_TOXMARK */            0,
/* RES_TXTATR_CHARFMT   */          OutHTML_SwTxtCharFmt,
/* RES_TXTATR_TWO_LINES */          0,
/* RES_TXTATR_CJK_RUBY */           0,
/* RES_TXTATR_UNKNOWN_CONTAINER */  0,
/* RES_TXTATR_DUMMY5 */             0,
/* RES_TXTATR_DUMMY6 */             0,

/* RES_TXTATR_FIELD */              OutHTML_SwFmtFld,
/* RES_TXTATR_FLYCNT */             OutHTML_SwFlyCnt,
/* RES_TXTATR_FTN */                OutHTML_SwFmtFtn,
/* RES_TXTATR_SOFTHYPH */           0,
/* RES_TXTATR_HARDBLANK*/           OutHTML_SwHardBlank,
/* RES_TXTATR_DUMMY1 */             0, // Dummy:
/* RES_TXTATR_DUMMY2 */             0, // Dummy:

/* RES_PARATR_LINESPACING   */      0,
/* RES_PARATR_ADJUST    */          OutHTML_SvxAdjust,
/* RES_PARATR_SPLIT */              0,
/* RES_PARATR_WIDOWS    */          0,
/* RES_PARATR_ORPHANS   */          0,
/* RES_PARATR_TABSTOP   */          0,
/* RES_PARATR_HYPHENZONE*/          0,
/* RES_PARATR_DROP */               OutHTML_CSS1Attr,
/* RES_PARATR_REGISTER */           0, // neu:  Registerhaltigkeit
/* RES_PARATR_DUMMY1 */             0, // Dummy:
/* RES_PARATR_DUMMY2 */             0, // Dummy:

/* RES_FILL_ORDER   */              0,
/* RES_FRM_SIZE */                  0,
/* RES_PAPER_BIN    */              0,
/* RES_LR_SPACE */                  0,
/* RES_UL_SPACE */                  0,
/* RES_PAGEDESC */                  0,
/* RES_BREAK */                     0,
/* RES_CNTNT */                     0,
/* RES_HEADER */                    0,
/* RES_FOOTER */                    0,
/* RES_PRINT */                     0,
/* RES_OPAQUE */                    0,
/* RES_PROTECT */                   0,
/* RES_SURROUND */                  0,
/* RES_VERT_ORIENT */               0,
/* RES_HORI_ORIENT */               0,
/* RES_ANCHOR */                    0,
/* RES_BACKGROUND */                0,
/* RES_BOX  */                      0,
/* RES_SHADOW */                    0,
/* RES_FRMMACRO */                  0,
/* RES_COL */                       0,
/* RES_KEEP */                      0,
/* RES_URL */                       0,
/* RES_EDIT_IN_READONLY */          0,
/* RES_LAYOUT_SPLIT */              0,
/* RES_FRMATR_DUMMY1 */             0, // Dummy:
/* RES_FRMATR_DUMMY2 */             0, // Dummy:
/* RES_FRMATR_DUMMY3 */             0, // Dummy:
/* RES_FRMATR_DUMMY4 */             0, // Dummy:
/* RES_FRMATR_DUMMY5 */             0, // Dummy:
/* RES_FRMATR_DUMMY6 */             0, // Dummy:
/* RES_FRMATR_DUMMY7 */             0, // Dummy:
/* RES_FRMATR_DUMMY8 */             0, // Dummy:
/* RES_FRMATR_DUMMY9 */             0, // Dummy:

/* RES_GRFATR_MIRRORGRF */          0,
/* RES_GRFATR_CROPGRF   */          0,
/* RES_GRFATR_ROTATION */           0,
/* RES_GRFATR_LUMINANCE */          0,
/* RES_GRFATR_CONTRAST */           0,
/* RES_GRFATR_CHANNELR */           0,
/* RES_GRFATR_CHANNELG */           0,
/* RES_GRFATR_CHANNELB */           0,
/* RES_GRFATR_GAMMA */              0,
/* RES_GRFATR_INVERT */             0,
/* RES_GRFATR_TRANSPARENCY */       0,
/* RES_GRFATR_DRWAMODE */           0,
/* RES_GRFATR_DUMMY1 */             0,
/* RES_GRFATR_DUMMY2 */             0,
/* RES_GRFATR_DUMMY3 */             0,
/* RES_GRFATR_DUMMY4 */             0,
/* RES_GRFATR_DUMMY5 */             0,

/* RES_BOXATR_FORMAT */             0,
/* RES_BOXATR_FORMULA */            0,
/* RES_BOXATR_VALUE */              0
};




/*************************************************************************

      Source Code Control System - Header

      $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/sw/source/filter/html/htmlatr.cxx,v 1.1.1.1 2000-09-18 17:14:55 hr Exp $

      Source Code Control System - Update

      $Log: not supported by cvs2svn $
      Revision 1.246  2000/09/18 16:04:43  willem.vandorp
      OpenOffice header added.

      Revision 1.245  2000/08/18 13:03:11  jp
      don't export escaped URLs

      Revision 1.244  2000/08/04 10:55:10  jp
      Soft-/HardHyphens & HardBlanks changed from attribute to unicode character

      Revision 1.243  2000/07/31 19:24:25  jp
      new attributes for CJK/CTL and graphic

      Revision 1.242  2000/06/26 09:51:53  jp
      must change: GetAppWindow->GetDefaultDevice

      Revision 1.241  2000/04/28 14:29:10  mib
      unicode

      Revision 1.240  2000/04/10 12:20:55  mib
      unicode

      Revision 1.239  2000/02/11 14:36:52  hr
      #70473# changes for unicode ( patched by automated patchtool )

      Revision 1.238  1999/12/08 13:50:39  jp
      Task #70258#: textattr for XMLAttrContainer

      Revision 1.237  1999/09/21 09:48:43  mib
      multiple text encodings

      Revision 1.236  1999/06/22 06:30:08  MIB
      #64687#: if a center paragraph contains controls add a div tag


      Rev 1.235   22 Jun 1999 08:30:08   MIB
   #64687#: if a center paragraph contains controls add a div tag

      Rev 1.234   10 Jun 1999 10:33:52   JP
   have to change: no AppWin from SfxApp

      Rev 1.233   09 Jun 1999 19:35:54   JP
   have to change: no cast from GetpApp to SfxApp/OffApp, SfxShell only subclass of SfxApp

      Rev 1.232   15 Apr 1999 13:25:30   MIB
   #64159#: Beim Versenden von Mails auch in PRE umbrechen

      Rev 1.231   23 Mar 1999 15:27:32   MIB
   #63049#: Relative Einzuege in Numerierungen

      Rev 1.230   17 Mar 1999 16:45:20   MIB
   #63049#: Numerierungen mit relativen Abstaenden

      Rev 1.229   18 Jan 1999 10:14:38   MIB
   #60806#: Auch bei <FONT FACE=...> generische Fontnamen exportieren

      Rev 1.228   06 Jan 1999 13:54:42   MIB
   #60393#: Fuer Nscp4 in Tabellen keine line-spacing und margin-bottom exportieren

      Rev 1.227   06 Jan 1999 10:33:38   MIB
   #60311#: In Listen fuer nicht-numerierte Absaetze keine <PRE> ausgeben

      Rev 1.226   02 Nov 1998 17:25:58   MIB
   #58480#: Fuss-/Endnoten

*************************************************************************/


/*************************************************************************
 *
 *  $RCSfile: rtfatr.cxx,v $
 *
 *  $Revision: 1.39 $
 *
 *  last change: $Author: hr $ $Date: 2003-11-05 14:13:47 $
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

/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil -*- */

/*
 * Dieses File enthaelt alle Ausgabe-Funktionen des RTF-Writers;
 * fuer alle Nodes, Attribute, Formate und Chars.
 */


#pragma hdrstop

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif
#define ITEMID_BOXINFO      SID_ATTR_BORDER_INNER

#ifndef _COM_SUN_STAR_I18N_SCRIPTTYPE_HDL_
#include <com/sun/star/i18n/ScriptType.hdl>
#endif

#ifndef _OUTDEV_HXX //autogen
#include <vcl/outdev.hxx>
#endif
#ifndef SVTOOLS_URIHELPER_HXX
#include <svtools/urihelper.hxx>
#endif
#ifndef _SFXSTRITEM_HXX //autogen
#include <svtools/stritem.hxx>
#endif
#ifndef _RTFKEYWD_HXX //autogen
#include <svtools/rtfkeywd.hxx>
#endif
#ifndef _SFX_WHITER_HXX
#include <svtools/whiter.hxx>
#endif
#ifndef _RTFOUT_HXX
#include <svtools/rtfout.hxx>
#endif
#ifndef _SFXITEMITER_HXX //autogen
#include <svtools/itemiter.hxx>
#endif
#ifndef _URLOBJ_HXX //autogen
#include <tools/urlobj.hxx>
#endif
#ifndef _SVX_FONTITEM_HXX
#include <svx/fontitem.hxx>
#endif
#ifndef _SVX_HYZNITEM_HXX //autogen
#include <svx/hyznitem.hxx>
#endif
#ifndef _SVX_TSTPITEM_HXX //autogen
#include <svx/tstpitem.hxx>
#endif
#ifndef _SVX_LSPCITEM_HXX //autogen
#include <svx/lspcitem.hxx>
#endif
#ifndef _SVX_LANGITEM_HXX //autogen
#include <svx/langitem.hxx>
#endif
#ifndef _SVX_KEEPITEM_HXX //autogen
#include <svx/keepitem.hxx>
#endif
#ifndef _SVX_UDLNITEM_HXX //autogen
#include <svx/udlnitem.hxx>
#endif
#ifndef _SVX_SHADITEM_HXX //autogen
#include <svx/shaditem.hxx>
#endif
#ifndef _SVX_CMAPITEM_HXX //autogen
#include <svx/cmapitem.hxx>
#endif
#ifndef _SVX_BRSHITEM_HXX //autogen
#include <svx/brshitem.hxx>
#endif
#ifndef _SVX_PROTITEM_HXX //autogen
#include <svx/protitem.hxx>
#endif
#ifndef _SVX_OPAQITEM_HXX //autogen
#include <svx/opaqitem.hxx>
#endif
#ifndef _SVX_ULSPITEM_HXX //autogen
#include <svx/ulspitem.hxx>
#endif
#ifndef _SVX_PRNTITEM_HXX //autogen
#include <svx/prntitem.hxx>
#endif
#ifndef _SVX_COLRITEM_HXX //autogen
#include <svx/colritem.hxx>
#endif
#ifndef _SVX_ESCPITEM_HXX //autogen
#include <svx/escpitem.hxx>
#endif
#ifndef _SVX_FHGTITEM_HXX //autogen
#include <svx/fhgtitem.hxx>
#endif
#ifndef _SVX_SPLTITEM_HXX //autogen
#include <svx/spltitem.hxx>
#endif
#ifndef _SVX_ADJITEM_HXX //autogen
#include <svx/adjitem.hxx>
#endif
#ifndef _SVX_LRSPITEM_HXX //autogen
#include <svx/lrspitem.hxx>
#endif
#ifndef _SVX_BOXITEM_HXX //autogen
#include <svx/boxitem.hxx>
#endif
#ifndef _SVX_CRSDITEM_HXX //autogen
#include <svx/crsditem.hxx>
#endif
#ifndef _SVX_CNTRITEM_HXX //autogen
#include <svx/cntritem.hxx>
#endif
#ifndef _SVX_POSTITEM_HXX //autogen
#include <svx/postitem.hxx>
#endif
#ifndef _SVX_SHDDITEM_HXX //autogen
#include <svx/shdditem.hxx>
#endif
#ifndef _SVX_WGHTITEM_HXX //autogen
#include <svx/wghtitem.hxx>
#endif
#ifndef _SVX_BRKITEM_HXX //autogen
#include <svx/brkitem.hxx>
#endif
#ifndef _SVX_BRSHITEM_HXX //autogen
#include <svx/brshitem.hxx>
#endif
#ifndef _SVX_WRLMITEM_HXX
#include <svx/wrlmitem.hxx>
#endif
#ifndef _SVX_EMPHITEM_HXX
#include <svx/emphitem.hxx>
#endif
#ifndef _SVX_TWOLINESITEM_HXX
#include <svx/twolinesitem.hxx>
#endif
#ifndef _SVX_CHARSCALEITEM_HXX
#include <svx/charscaleitem.hxx>
#endif
#ifndef _SVX_CHARROTATEITEM_HXX
#include <svx/charrotateitem.hxx>
#endif
#ifndef _SVX_CHARRELIEFITEM_HXX
#include <svx/charreliefitem.hxx>
#endif
#ifndef _XOUTBMP_HXX //autogen
#include <svx/xoutbmp.hxx>
#endif
#ifndef _SVX_PARAVERTALIGNITEM_HXX
#include <svx/paravertalignitem.hxx>
#endif
#ifndef _SVX_HNGPNCTITEM_HXX
#include <svx/hngpnctitem.hxx>
#endif
#ifndef _SVX_SRIPTSPACEITEM_HXX
#include <svx/scriptspaceitem.hxx>
#endif
#ifndef _SVX_FORBIDDENRULEITEM_HXX
#include <svx/forbiddenruleitem.hxx>
#endif
#ifndef _SVX_FRMDIRITEM_HXX
#include <svx/frmdiritem.hxx>
#endif
#ifndef _UNOTOOLS_CHARCLASS_HXX
#include <unotools/charclass.hxx>
#endif

#ifndef _REFFLD_HXX //autogen wg. SwGetRefField
#include <reffld.hxx>
#endif
#ifndef _FRMATR_HXX
#include <frmatr.hxx>
#endif
#ifndef _CHARATR_HXX
#include <charatr.hxx>
#endif
#ifndef _FMTFSIZE_HXX //autogen
#include <fmtfsize.hxx>
#endif
#ifndef _FMTPDSC_HXX //autogen
#include <fmtpdsc.hxx>
#endif
#ifndef _FMTORNT_HXX //autogen
#include <fmtornt.hxx>
#endif
#ifndef _FMTFLD_HXX //autogen
#include <fmtfld.hxx>
#endif
#ifndef _FMTFLCNT_HXX //autogen
#include <fmtflcnt.hxx>
#endif
#ifndef _FMTFTN_HXX //autogen
#include <fmtftn.hxx>
#endif
#ifndef _FMTHBSH_HXX //autogen
#include <fmthbsh.hxx>
#endif
#ifndef _FCHRFMT_HXX //autogen
#include <fchrfmt.hxx>
#endif
#ifndef _FMTCNTNT_HXX //autogen
#include <fmtcntnt.hxx>
#endif
#ifndef _FMTHDFT_HXX //autogen
#include <fmthdft.hxx>
#endif
#ifndef _FMTCLDS_HXX //autogen
#include <fmtclds.hxx>
#endif
#ifndef _TXTFTN_HXX //autogen
#include <txtftn.hxx>
#endif
#ifndef _FMTSRND_HXX //autogen
#include <fmtsrnd.hxx>
#endif
#ifndef _FMTANCHR_HXX //autogen
#include <fmtanchr.hxx>
#endif
#ifndef _CHARFMT_HXX //autogen
#include <charfmt.hxx>
#endif
#ifndef _FMTINFMT_HXX //autogen
#include <fmtinfmt.hxx>
#endif
#ifndef _TXTINET_HXX //autogen
#include <txtinet.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _PAM_HXX
#include <pam.hxx>
#endif
#ifndef _PARATR_HXX
#include <paratr.hxx>
#endif
#ifndef _FLDBAS_HXX
#include <fldbas.hxx>           // fuer SwField ...
#endif
#ifndef _WRTRTF_HXX
#include <wrtrtf.hxx>
#endif
#ifndef _RTF_HXX
#include <rtf.hxx>          // fuer SwPictureType
#endif
#ifndef _NDGRF_HXX
#include <ndgrf.hxx>
#endif
#ifndef _GRFATR_HXX
#include <grfatr.hxx>
#endif
#ifndef _FMTCOL_HXX
#include <fmtcol.hxx>
#endif
#ifndef _DOCUFLD_HXX
#include <docufld.hxx>
#endif
#ifndef _FLDDAT_HXX
#include <flddat.hxx>
#endif
#ifndef _PAGEDESC_HXX
#include <pagedesc.hxx>     // fuer SwPageDesc ...
#endif
#ifndef _SWTABLE_HXX
#include <swtable.hxx>      // fuer SwPageDesc ...
#endif
#ifndef _LINETYPE_HXX
#include <linetype.hxx>     // fuer SwPageDesc ...
#endif
#ifndef _DOCSH_HXX
#include <docsh.hxx>
#endif
#ifndef _SWRECT_HXX //autogen
#include <swrect.hxx>
#endif
#ifndef _SECTION_HXX //autogen
#include <section.hxx>
#endif
#ifndef _WRTSWTBL_HXX
#include <wrtswtbl.hxx>
#endif
#ifndef _HTMLTBL_HXX
#include <htmltbl.hxx>
#endif
#ifndef _FMTCLBL_HXX
#include <fmtclbl.hxx>
#endif
#ifndef _BREAKIT_HXX
#include <breakit.hxx>
#endif
#ifndef _FMTRUBY_HXX
#include <fmtruby.hxx>
#endif
#ifndef _TXTATR_HXX
#include <txtatr.hxx>
#endif
#ifndef _FLTINI_HXX
#include <fltini.hxx>
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

#define ATTRFNTAB_SIZE 130
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

#ifdef WNT
// ueber xoutbmp.hxx wird das winuser.h angezogen. Dort gibt es ein
// define GetProp das mit unserem SvxEscapement kollidiert!
#undef GetProp
#endif

//-----------------------------------------------------------------------

static Writer& OutRTF_SwFmtCol( Writer& rWrt, const SfxPoolItem& rHt );

//-----------------------------------------------------------------------

SvStream& OutComment( Writer& rWrt, const sal_Char* pStr )
{
    return (rWrt.Strm() << '{' << sRTF_IGNORE << pStr);
}

SvStream& OutComment( Writer& rWrt, const char* pStr, BOOL bSetFlag )
{
    // setze Flag, damit bei der Ausgabe von Attributen zu erkennen,
    // ob ueberhaupt etwas ausgegeben wurde.
    ((SwRTFWriter&)rWrt).bOutFmtAttr = bSetFlag;
    return (rWrt.Strm() << '{' << sRTF_IGNORE << pStr);
}

Writer& OutRTF_AsByteString( Writer& rWrt, const String& rStr )
{
    ByteString sOutStr( rStr, DEF_ENCODING );
    rWrt.Strm() << sOutStr.GetBuffer();
    return rWrt;
}

void OutRTF_SfxItemSet( SwRTFWriter& rWrt, const SfxItemSet& rSet,
                        BOOL bDeep )
{
    bool bFrameDirOut=false;
    bool bAdjustOut=false;
    // erst die eigenen Attribute ausgeben
    SvPtrarr aAsian( 0, 5 ), aCmplx( 0, 5 ), aLatin( 0, 5 );

    const SfxItemPool& rPool = *rSet.GetPool();
    SfxWhichIter aIter( rSet );
    const SfxPoolItem* pItem;
    FnAttrOut pOut;
    register USHORT nWhich = aIter.FirstWhich();
    while( nWhich )
    {
        if( SFX_ITEM_SET == rSet.GetItemState( nWhich, bDeep, &pItem ))
        {
            pOut = aRTFAttrFnTab[ nWhich - RES_CHRATR_BEGIN];
            if( pOut &&
                ( *pItem != rPool.GetDefaultItem( nWhich )
                    || ( rSet.GetParent() &&
                        *pItem != rSet.GetParent()->Get( nWhich ) )
              ) )
                ;
            else
                pOut = 0;
        }
        else if( !bDeep )
            pOut = 0;
        else if( 0 != ( pItem = rPool.GetPoolDefaultItem( nWhich )) )
            pOut = aRTFAttrFnTab[ nWhich - RES_CHRATR_BEGIN];
        else
            pOut = 0;

        if (!pOut && bDeep)
        {
            switch( nWhich )
            {
                case RES_CHRATR_FONTSIZE:
                case RES_CHRATR_CJK_FONTSIZE:
                case RES_CHRATR_CTL_FONTSIZE:
                case RES_CHRATR_LANGUAGE:
                case RES_CHRATR_CJK_LANGUAGE:
                case RES_CHRATR_CTL_LANGUAGE:
                    pItem = &rPool.GetDefaultItem( nWhich );
                    pOut = aRTFAttrFnTab[ nWhich - RES_CHRATR_BEGIN];
                    break;
                default:
                    break;
            }
        }

        if( pOut )
        {
            void* pVoidItem = (void*)pItem;
            switch( nWhich )
            {
            case RES_CHRATR_FONT:
            case RES_CHRATR_FONTSIZE:
            case RES_CHRATR_LANGUAGE:
            case RES_CHRATR_POSTURE:
            case RES_CHRATR_WEIGHT:
                aLatin.Insert( pVoidItem, aLatin.Count() );
                pOut = 0;
                break;

            case RES_CHRATR_CJK_FONT:
            case RES_CHRATR_CJK_FONTSIZE:
            case RES_CHRATR_CJK_LANGUAGE:
            case RES_CHRATR_CJK_POSTURE:
            case RES_CHRATR_CJK_WEIGHT:
                aAsian.Insert( pVoidItem, aAsian.Count() );
                pOut = 0;
                break;

            case RES_CHRATR_CTL_FONT:
            case RES_CHRATR_CTL_FONTSIZE:
            case RES_CHRATR_CTL_LANGUAGE:
            case RES_CHRATR_CTL_POSTURE:
            case RES_CHRATR_CTL_WEIGHT:
                aCmplx.Insert( pVoidItem, aCmplx.Count() );
                pOut = 0;
                break;

            case RES_FRAMEDIR:
                bFrameDirOut=true;
                break;
            case RES_PARATR_ADJUST:
                bAdjustOut=true;
                break;
            }
        }

        if (pOut)
            (*pOut)( rWrt, *pItem );
        nWhich = aIter.NextWhich();
    }

    //If rtlpar set and no following alignment. And alignment is not rtl then
    //output alignment
    if (bFrameDirOut && !bAdjustOut && !rWrt.pFlyFmt && !rWrt.bOutPageDesc)
    {
        if (pOut = aRTFAttrFnTab[RES_PARATR_ADJUST - RES_CHRATR_BEGIN])
            (*pOut)(rWrt, rSet.Get(RES_PARATR_ADJUST));
    }
    if (rWrt.pFlyFmt && !rWrt.bOutPageDesc && !bFrameDirOut)
    {
        if (pOut = aRTFAttrFnTab[RES_FRAMEDIR - RES_CHRATR_BEGIN])
            (*pOut)(rWrt, rSet.Get(RES_FRAMEDIR));
    }

    if( aAsian.Count() || aCmplx.Count() ||aLatin.Count() )
    {
        SvPtrarr* aArr[ 3 ];
        switch (rWrt.GetCurrScriptType())
        {
        case ::com::sun::star::i18n::ScriptType::LATIN:
            aArr[ 0 ] = &aCmplx;
            aArr[ 1 ] = &aAsian;
            aArr[ 2 ] = &aLatin;
            break;

        case ::com::sun::star::i18n::ScriptType::ASIAN:
            aArr[ 0 ] = &aCmplx;
            aArr[ 1 ] = &aLatin;
            aArr[ 2 ] = &aAsian;
            break;

        case ::com::sun::star::i18n::ScriptType::COMPLEX:
            aArr[ 0 ] = &aLatin;
            aArr[ 1 ] = &aAsian;
            aArr[ 2 ] = &aCmplx;
            break;

        default:
            return ;
        }

        BOOL bOutLTOR = TRUE;
        for( int nArrCnt = 0; nArrCnt < 3; ++nArrCnt )
        {
            SvPtrarr* pCurArr = aArr[ nArrCnt ];
            if( pCurArr->Count() )
            {
                rWrt.SetAssociatedFlag( 2 != nArrCnt );
                if( pCurArr == &aLatin )
                {
                    if( bOutLTOR )
                    {   rWrt.Strm() << sRTF_LTRCH; bOutLTOR = FALSE; }
                    rWrt.Strm() << sRTF_LOCH;
                }
                else if( pCurArr == &aAsian )
                {
                    if( bOutLTOR )
                    {   rWrt.Strm() << sRTF_LTRCH; bOutLTOR = FALSE; }
                    rWrt.Strm() << sRTF_DBCH;
                }
                else
                    rWrt.Strm() << sRTF_RTLCH;

                for( USHORT n = 0; n < pCurArr->Count(); ++n )
                {
                    pItem = (const SfxPoolItem*)(*pCurArr)[ n ];
                    pOut = aRTFAttrFnTab[ pItem->Which() - RES_CHRATR_BEGIN];
                    (*pOut)( rWrt, *pItem );
                }
            }
        }
    }
}

// fuer die Formate muesste eine einzige Ausgabe-Funktion genuegen !
/*
 * Formate wie folgt ausgeben:
 *      - gebe alle Attribute vom Format aus
 */

bool SwFmtToSet(SwRTFWriter& rWrt, const SwFmt& rFmt, SfxItemSet &rSet)
{
    bool bOutItemSet = true;
    rSet.SetParent(rFmt.GetAttrSet().GetParent());

    switch( rFmt.Which() )
    {
    case RES_CONDTXTFMTCOLL:
    case RES_TXTFMTCOLL:
        {
            USHORT nId = rWrt.GetId( (const SwTxtFmtColl&)rFmt );
            if (0 == nId )
                return false;       // Default-TextStyle nicht ausgeben !!

            rWrt.Strm() << sRTF_S;
            rWrt.OutULong( nId );
            rWrt.bOutFmtAttr = TRUE;
            BYTE nLvl = ((const SwTxtFmtColl&)rFmt).GetOutlineLevel();
            if( MAXLEVEL > nLvl )
            {
                USHORT nNumId = rWrt.GetNumRuleId(
                                        *rWrt.pDoc->GetOutlineNumRule() );
                if( USHRT_MAX != nNumId )
                {
                    BYTE nWWLvl = 8 >= nLvl ? nLvl : 8;
                    rWrt.Strm() << sRTF_LS;
                    rWrt.OutULong( nNumId );
                    rWrt.Strm() << sRTF_ILVL; rWrt.OutULong( nWWLvl );
                    rWrt.Strm() << sRTF_OUTLINELEVEL; rWrt.OutULong( nWWLvl );
                    if( nWWLvl != nLvl )            // RTF-kennt nur 9 Ebenen
                    {
                        OutComment( rWrt, sRTF_SOUTLVL );
                        rWrt.OutULong( nLvl ) << '}';
                    }
                }

                const SwNumFmt* pNFmt = &rWrt.pDoc->GetOutlineNumRule()->Get( nLvl );
                if( pNFmt->GetAbsLSpace() )
                {
                    SfxItemSet aSet( *rFmt.GetAttrSet().GetPool(),
                                    rFmt.GetAttrSet().GetRanges() );
                    aSet.SetParent( &rFmt.GetAttrSet() );
                    SvxLRSpaceItem aLR( (SvxLRSpaceItem&)aSet.Get( RES_LR_SPACE ) );

                    aLR.SetTxtLeft( aLR.GetTxtLeft() + pNFmt->GetAbsLSpace() );
                    aLR.SetTxtFirstLineOfst( pNFmt->GetFirstLineOffset() );

                    aSet.Put(aLR);
                    rSet.Put(aSet);
                    bOutItemSet = false;
                }
            }
        }
        break;
    case RES_CHRFMT:
        {
            USHORT nId = rWrt.GetId( (const SwCharFmt&)rFmt );
            if (0 == nId)
                return false;       // Default-CharStyle nicht ausgeben !!

            rWrt.Strm() << sRTF_IGNORE << sRTF_CS;
            rWrt.OutULong( nId );
            rWrt.bOutFmtAttr = TRUE;
        }
        break;

//  case RES_GRFFMTCOLL:
// ?????
    }

    if (bOutItemSet)
        rSet.Put(rFmt.GetAttrSet());

    return true;
}

Writer& OutRTF_SwFmt(Writer& rWrt, const SwFmt& rFmt)
{
    SwRTFWriter & rRTFWrt = (SwRTFWriter&)rWrt;

    SfxItemSet aSet(*rFmt.GetAttrSet().GetPool(),
        rFmt.GetAttrSet().GetRanges() );
    if (SwFmtToSet(rRTFWrt, rFmt, aSet))
        OutRTF_SfxItemSet(rRTFWrt, aSet, TRUE);

    return rWrt;
}

void OutRTF_SwFlyFrmFmt( SwRTFWriter& rRTFWrt )
{
    // ist der aktuelle Absatz in einem freifliegenden Rahmen ? Dann
    // muessen noch die Attribute dafuer ausgegeben werden.
    ASSERT( rRTFWrt.pFlyFmt, "wo ist das Fly-Format?" );

    if( rRTFWrt.bOutFmtAttr )
    {
        rRTFWrt.Strm() << ' ';
        rRTFWrt.bOutFmtAttr = FALSE;
    }
    // gebe erstmal alle RTF-Spezifischen Attribute aus
    rRTFWrt.bRTFFlySyntax = TRUE;
    OutRTF_SwFmt( rRTFWrt, *rRTFWrt.pFlyFmt );

    // dann gebe alle eigenen Attribute aus
    {
        // dazu erzeuge einen temp strstream, um festzustellen ob es
        // ueberhaupt eigene Attribute gibt !
        SvMemoryStream aTmpStrm;
        SvStream* pSaveStrm = &rRTFWrt.Strm();
        rRTFWrt.SetStrm( aTmpStrm );

        rRTFWrt.bRTFFlySyntax = FALSE;
        OutRTF_SwFmt( rRTFWrt, *rRTFWrt.pFlyFmt );

        rRTFWrt.SetStrm( *pSaveStrm );  // Stream-Pointer wieder zurueck

        if( aTmpStrm.GetSize() )            // gibt es SWG spezifische Attribute ??
        {
            aTmpStrm.Seek( 0L );
            rRTFWrt.Strm() << '{' << sRTF_IGNORE << aTmpStrm << '}';
        }
    }
//  rRTFWrt.pFlyFmt = 0;        // wieder zuruecksetzen !!
}

/* Ausgabe der Nodes */

/*
 *  Ausgabe der Texte-Attribute:
 *      Die Text-Attribute sind in einem VarArray nach ihren Start-Positionen
 *      sortiert. Fuer den RTF-Writer ist aber auch das Ende von Bedeutung.
 *      Darum die Idee, sobald im SwpHints-Array ein Attribut mit der Start-
 *      Position gefunden wurde, in einem Sortierten-Array die Endposition
 *      zu speichern. Sobald der Writer die Position erreicht hat, wird die
 *      schliessende Klammer ausgegeben und die Position aus dem Sort.Array
 *      geloescht.
 *  15.3.93: Es reicht leider nicht aus nur Start und End zuspeichern, denn
 *      im SwpHints Array stehen sie nicht nach Ende sortiert, so dass
 *      Attribute die falsche schliessende Klammer bekommen. (z.B
 *      Bold von 0..3, dann folgt Underline von 0..4. Dann bekommt Underline
 *      die schliessende Klammer von Bold !!)
 *      Also muessen erst alle Attribute einer Position gesammelt, nach
 *      Ende sortiert und dann erst ausgegeben werden.
 */

SV_DECL_PTRARR( SfxPoolItems, SfxPoolItem*, 4, 4 )
class RTFEndPosLst;

class SttEndPos
{
    // falls mehrere Attribute den gleichen Bereich umspannen, sammeln
    SfxPoolItems aArr;
    xub_StrLen nStart, nEnd;

    SttEndPos( const SttEndPos & rSEPos );

public:
    SttEndPos( const SfxPoolItem& rAttr, xub_StrLen nStt, xub_StrLen nEnd );
    ~SttEndPos();

    xub_StrLen  GetStart() const            { return nStart; }
    xub_StrLen  GetEnd() const              { return nEnd; }

    const SfxPoolItems& GetAttrs() const    { return aArr; }
    void AddAttr( const SfxPoolItem& rAttr );
    BOOL HasScriptChange() const;
};


SV_DECL_PTRARR_DEL( _EndPosLst, SttEndPos*, 5, 5 )
SV_IMPL_PTRARR( _EndPosLst, SttEndPos* )

class RTFEndPosLst : private _EndPosLst
{
    const SwTxtNode& rNode;
    SwRTFWriter& rWrt;
    RTFEndPosLst* pOldPosLst;
    xub_StrLen nCurPos;
    USHORT nCurScript;
public:
    _EndPosLst::Count;
    _EndPosLst::operator[];
    _EndPosLst::DeleteAndDestroy;

    RTFEndPosLst( SwRTFWriter& rWrt, const SwTxtNode& rNd, xub_StrLen nStart );
    ~RTFEndPosLst();

    USHORT GetCurScript() const         { return nCurScript; }
    BOOL MatchScriptToId( USHORT nId ) const;
    int Insert( const SfxPoolItem& rAttr, xub_StrLen nStt, xub_StrLen nEnd );

    void OutAttrs( xub_StrLen nStrPos );
    void EndAttrs( xub_StrLen nStrPos );
    void OutScriptChange( xub_StrLen nStrPos );

    const SfxPoolItem* HasItem( USHORT nWhich ) const;
    const SfxPoolItem& GetItem( USHORT nWhich ) const;
    void OutFontAttrs( USHORT nScript );

    SfxItemPool& GetPool() {return *rNode.GetSwAttrSet().GetPool(); }
};


SttEndPos::SttEndPos( const SfxPoolItem& rAttr,
                        xub_StrLen nStt, xub_StrLen nEd )
    : nStart( nStt ), nEnd( nEd )
{
    AddAttr( rAttr );
}

SttEndPos::~SttEndPos()
{
    for( USHORT n = 0, nEnd = aArr.Count(); n < nEnd; ++n )
        if( RES_FLTRATTR_BEGIN <= aArr[ n ]->Which() )
            delete aArr[ n ];
}

BOOL SttEndPos::HasScriptChange() const
{
    for( USHORT n = 0, nEnd = aArr.Count(); n < nEnd; ++n )
        if( RES_FLTR_SCRIPTTYPE == aArr[ n ]->Which() )
            return TRUE;
    return FALSE;
}

void SttEndPos::AddAttr( const SfxPoolItem& rAttr )
{
#if 1
    const SfxPoolItem* pI = &rAttr;
    aArr.Insert(pI, aArr.Count());
#else
    const SfxPoolItem* pI = &rAttr;
    USHORT n = aArr.Count();
    switch( rAttr.Which() )
    {
    case RES_FLTR_SCRIPTTYPE:
        {
            while( n )
                switch( aArr[ --n ]->Which() )
                {
                case RES_CHRATR_FONT:
                case RES_CHRATR_FONTSIZE:
                case RES_CHRATR_LANGUAGE:
                case RES_CHRATR_POSTURE:
                case RES_CHRATR_WEIGHT:
                case RES_CHRATR_CJK_FONT:
                case RES_CHRATR_CJK_FONTSIZE:
                case RES_CHRATR_CJK_LANGUAGE:
                case RES_CHRATR_CJK_POSTURE:
                case RES_CHRATR_CJK_WEIGHT:
                case RES_CHRATR_CTL_FONT:
                case RES_CHRATR_CTL_FONTSIZE:
                case RES_CHRATR_CTL_LANGUAGE:
                case RES_CHRATR_CTL_POSTURE:
                case RES_CHRATR_CTL_WEIGHT:
                    aArr.Remove( n, 1 );
                    break;
                }
        }
        break;
    case RES_CHRATR_FONT:
    case RES_CHRATR_FONTSIZE:
    case RES_CHRATR_LANGUAGE:
    case RES_CHRATR_POSTURE:
    case RES_CHRATR_WEIGHT:
    case RES_CHRATR_CJK_FONT:
    case RES_CHRATR_CJK_FONTSIZE:
    case RES_CHRATR_CJK_LANGUAGE:
    case RES_CHRATR_CJK_POSTURE:
    case RES_CHRATR_CJK_WEIGHT:
    case RES_CHRATR_CTL_FONT:
    case RES_CHRATR_CTL_FONTSIZE:
    case RES_CHRATR_CTL_LANGUAGE:
    case RES_CHRATR_CTL_POSTURE:
    case RES_CHRATR_CTL_WEIGHT:
        while( n )
            if( RES_FLTR_SCRIPTTYPE == aArr[ --n ]->Which() )
            {
                pI = 0;
                break;
            }
        break;
    }
    if( pI )
        aArr.Insert( pI, aArr.Count() );
#endif
}

RTFEndPosLst::RTFEndPosLst( SwRTFWriter& rWriter, const SwTxtNode& rNd,
                            xub_StrLen nStart )
    : rNode( rNd ), rWrt( rWriter ), nCurPos( -1 )
{
    pOldPosLst = rWrt.pCurEndPosLst;
    rWrt.pCurEndPosLst = this;

    // JP 15.11.00: first default to latin - this must be change, if it
    //              clear how we handle empty paragraphs
    nCurScript = ::com::sun::star::i18n::ScriptType::LATIN;

    // search the script changes and add this positions as a Item into
    // the attribute list
    if( pBreakIt->xBreak.is() )
    {
        const String& rTxt = rNode.GetTxt();
        xub_StrLen nChg = nStart, nSttPos = nChg, nLen = rTxt.Len();

        while( nChg < nLen )
        {
            USHORT nScript = pBreakIt->xBreak->getScriptType( rTxt, nChg );
            nChg = (xub_StrLen)pBreakIt->xBreak->endOfScript( rTxt, nChg, nScript );
               switch( nScript )
            {
            case ::com::sun::star::i18n::ScriptType::LATIN:
            case ::com::sun::star::i18n::ScriptType::ASIAN:
            case ::com::sun::star::i18n::ScriptType::COMPLEX:
                if( nSttPos != nChg )
                {
                    SfxPoolItem* pAttr = new
                        SfxInt16Item( RES_FLTR_SCRIPTTYPE, nScript );
                    Insert( *pAttr, nSttPos, nChg );
                    nSttPos = nChg;
                }
                break;
            }
        }
    }
}

RTFEndPosLst::~RTFEndPosLst()
{
    rWrt.pCurEndPosLst = pOldPosLst;
}

int RTFEndPosLst::Insert( const SfxPoolItem& rAttr, xub_StrLen nStt,
                            xub_StrLen nEnd )
{
    if (rAttr.Which() == RES_TXTATR_INETFMT)
        return false;

    if( nStt == nEnd )
        return FALSE;

    USHORT nPos;
    for( nPos = 0; nPos < Count(); ++nPos )
    {
        SttEndPos* pTmp = GetObject( nPos );
        if( pTmp->GetStart() == nStt && pTmp->GetEnd() == nEnd )
        {
            pTmp->AddAttr( rAttr );
            return FALSE;       // schon vorhanden
        }
        if( nEnd < pTmp->GetEnd() )
        {
            SttEndPos* pNew = new SttEndPos( rAttr, nStt, nEnd );
            _EndPosLst::C40_INSERT( SttEndPos, pNew, nPos );
            return TRUE;
        }
    }

    SttEndPos* pNew = new SttEndPos( rAttr, nStt, nEnd );
    _EndPosLst::C40_INSERT( SttEndPos, pNew, nPos );
    return TRUE;
}

void RTFEndPosLst::OutScriptChange( xub_StrLen nStrPos )
{
    SttEndPos* pStt;
    for( USHORT n = 0, nEnd = Count(); n < nEnd; ++n )
        if( nStrPos == (pStt = GetObject( n ))->GetStart())
        {
            if( pStt->HasScriptChange() )
                OutAttrs( nStrPos );
            break;
        }
}

void RTFEndPosLst::OutAttrs( xub_StrLen nStrPos )
{
    SttEndPos* pStt;
    nCurPos = nStrPos;
    for( USHORT n = Count(); n ;  )
        if( nStrPos == (pStt = (*this)[ --n ])->GetStart() )
        {
            rWrt.Strm() << '{';
            for( USHORT i = 0; i < pStt->GetAttrs().Count(); ++i )
            {
                const SfxPoolItem* pItem = pStt->GetAttrs()[i];
                if( RES_FLTR_SCRIPTTYPE == pItem->Which() )
                    OutFontAttrs( ((SfxUInt16Item*)pItem)->GetValue() );
                else
                    Out( aRTFAttrFnTab, *pItem, rWrt );
            }
        }

    nCurPos = -1;
}

void RTFEndPosLst::OutFontAttrs( USHORT nScript )
{
    // script change, write the correct attributes:
    // start first with the Fontname

    rWrt.bOutFmtAttr = TRUE;
    nCurScript = nScript;
    rWrt.SetCurrScriptType( nScript );
    rWrt.SetAssociatedFlag( FALSE );

    // the font MUST be at the first position !!!
    static const USHORT aLatinIds[] =
    {
        RES_CHRATR_FONT,
        RES_CHRATR_FONTSIZE, RES_CHRATR_LANGUAGE,
        RES_CHRATR_POSTURE,  RES_CHRATR_WEIGHT,
        0
    };
    static const USHORT aAsianIds[] =
    {
        RES_CHRATR_CJK_FONT,
        RES_CHRATR_CJK_FONTSIZE, RES_CHRATR_CJK_LANGUAGE,
        RES_CHRATR_CJK_POSTURE,  RES_CHRATR_CJK_WEIGHT,
        0
    };
    static const USHORT aCmplxIds[] =
    {
        RES_CHRATR_CTL_FONT,
        RES_CHRATR_CTL_FONTSIZE, RES_CHRATR_CTL_LANGUAGE,
        RES_CHRATR_CTL_POSTURE,  RES_CHRATR_CTL_WEIGHT,
        0
    };

    // size/weight/posture optional
    const USHORT* pIdArr = 0;
    ByteString sOut;
    switch (nScript)
    {
        default:    //fall through
            ASSERT(pIdArr, "unknown script, strange");
        case ::com::sun::star::i18n::ScriptType::LATIN:
            sOut = sRTF_LOCH;
            pIdArr = aLatinIds;
            break;
        case ::com::sun::star::i18n::ScriptType::ASIAN:
            sOut = sRTF_DBCH;
            pIdArr = aAsianIds;
            break;
        case ::com::sun::star::i18n::ScriptType::COMPLEX:
            sOut = sRTF_RTLCH;
            pIdArr = aCmplxIds;
            break;
    }

    if (sOut.Len())
    {
        rWrt.Strm() << sOut.GetBuffer();

        for (const USHORT* pId = pIdArr; *pId; ++pId)
        {
            if (FnAttrOut pOut = aRTFAttrFnTab[ *pId - RES_CHRATR_BEGIN])
            {
                const SfxPoolItem* pItem = HasItem(*pId);
                if (!pItem)
                    pItem = &GetPool().GetDefaultItem(*pId);
                (*pOut)(rWrt, *pItem);
            }
        }
    }
}

void RTFEndPosLst::EndAttrs( xub_StrLen nStrPos )
{
    xub_StrLen nClipStart=STRING_MAXLEN;
    bool bClosed=false;
    SttEndPos* pSEPos;
    while( 0 != Count() && 0 != (pSEPos = GetObject( 0 )) &&
        ( STRING_MAXLEN == nStrPos || nStrPos == pSEPos->GetEnd() ))
    {
        const SfxPoolItems& rAttrs = pSEPos->GetAttrs();
        for( USHORT nAttr = rAttrs.Count(); nAttr; )
            switch( rAttrs[ --nAttr ]->Which() )
            {
#if 0
            case RES_TXTATR_INETFMT:
                // Hyperlinks werden als Felder geschrieben, aber der
                // "FieldResult" steht als Text im TextNode. Also muss
                // bei diesen Attributen am Ende 2 Klammern stehen!
                rWrt.Strm() << "}}";
                break;
#endif

            case RES_TXTATR_CJK_RUBY:
                rWrt.Strm() << ")}{" << sRTF_FLDRSLT << " }}";
                break;
            }

        rWrt.Strm() << '}';     // end of all attributes from this position
        if (pSEPos->GetStart() < nClipStart)
            nClipStart = pSEPos->GetStart();
        bClosed=true;
        DeleteAndDestroy( 0, 1 );
    }

    if (bClosed)
    {
        //If there are open ranges whose start is before this point, and whose
        //start is after the cliping start then they have been closed whether
        //we wanted this or not. So accept that fact and then restart then
        //again
        USHORT nSize = Count();
        while (nSize > 0)
        {
            pSEPos = GetObject(--nSize);
            if ( pSEPos->GetStart() < nStrPos &&
                    pSEPos->GetStart() >= nClipStart)
            {
                rWrt.Strm() << '}';
            }
        }

        nSize = Count();
        USHORT n = 0;
        while (n < nSize)
        {
            SttEndPos* pStt = (*this)[n++];
            if (pStt->GetStart() < nStrPos && pStt->GetStart() >= nClipStart)
            {
                rWrt.Strm() << '{';
                for( USHORT i = 0; i < pStt->GetAttrs().Count(); ++i )
                {
                    const SfxPoolItem* pItem = pStt->GetAttrs()[i];
                    if( RES_FLTR_SCRIPTTYPE == pItem->Which() )
                        OutFontAttrs( ((SfxUInt16Item*)pItem)->GetValue() );
                    else
                        Out( aRTFAttrFnTab, *pItem, rWrt );
                }
            }
        }
    }
}

BOOL RTFEndPosLst::MatchScriptToId( USHORT nWhich ) const
{
    BOOL bRet = FALSE;
    switch( nWhich )
    {
    case RES_CHRATR_FONT:
    case RES_CHRATR_FONTSIZE:
    case RES_CHRATR_LANGUAGE:
    case RES_CHRATR_POSTURE:
    case RES_CHRATR_WEIGHT:
        bRet = nCurScript == ::com::sun::star::i18n::ScriptType::LATIN;
        break;
    case RES_CHRATR_CJK_FONT:
    case RES_CHRATR_CJK_FONTSIZE:
    case RES_CHRATR_CJK_LANGUAGE:
    case RES_CHRATR_CJK_POSTURE:
    case RES_CHRATR_CJK_WEIGHT:
        bRet = nCurScript == ::com::sun::star::i18n::ScriptType::ASIAN;
        break;
    case RES_CHRATR_CTL_FONT:
    case RES_CHRATR_CTL_FONTSIZE:
    case RES_CHRATR_CTL_LANGUAGE:
    case RES_CHRATR_CTL_POSTURE:
    case RES_CHRATR_CTL_WEIGHT:
        bRet = nCurScript == ::com::sun::star::i18n::ScriptType::COMPLEX;
        break;
    }
    return bRet;
}

const SfxPoolItem& RTFEndPosLst::GetItem( USHORT nWhich ) const
{
    const SfxPoolItem* pItem = HasItem( nWhich );
    if( !pItem )
        pItem = &rNode.GetSwAttrSet().GetPool()->GetDefaultItem( nWhich );
    return *pItem;
}

const SfxPoolItem* RTFEndPosLst::HasItem( USHORT nWhich ) const
{
    const SfxPoolItem* pItem;
    if( RES_TXTATR_END > nWhich )
    {
        // it's a character/text attribute so look into the item array
        for( USHORT n = Count(); n; )
        {
            SttEndPos* pTmp = GetObject( --n );
            for( USHORT i = pTmp->GetAttrs().Count(); i; )
            {
                pItem = pTmp->GetAttrs()[ --i ];
                if( pItem->Which() == nWhich )
                    return pItem;

                // look into the charfmt?
                if( RES_TXTATR_CHARFMT == pItem->Which() &&
                    ((SwFmtCharFmt*) pItem)->GetCharFmt() &&
                    SFX_ITEM_SET == ((SwFmtCharFmt*) pItem)->GetCharFmt()->
                        GetItemState( nWhich, TRUE, &pItem ))
                    return pItem;
            }
        }
    }

    if( SFX_ITEM_SET != rNode.GetSwAttrSet().GetItemState(
                                nWhich, TRUE, &pItem ))
        pItem = 0;
    return pItem;
}

const SfxPoolItem& SwRTFWriter::GetItem( USHORT nWhich ) const
{
    if( pCurEndPosLst )
        return pCurEndPosLst->GetItem( nWhich );
    if( pAttrSet )
        return pAttrSet->Get( nWhich );

    return pDoc->GetAttrPool().GetDefaultItem( nWhich );
}

static void OutSvxFrmDir(SwRTFWriter& rRTFWrt, const SfxPoolItem& rHt )
{
    // write it only for pasgedesc's - not for frames
    SvxFrameDirectionItem aItem((const SvxFrameDirectionItem&)rHt);
    USHORT nVal = 0;
    const sal_Char* pStr = 0;
    bool bRTL = false;

    if (rRTFWrt.pFlyFmt)
        aItem.SetValue(rRTFWrt.TrueFrameDirection(*rRTFWrt.pFlyFmt));

    switch (aItem.GetValue())
    {
        case FRMDIR_ENVIRONMENT:
            ASSERT(0, "Not expected to see FRMDIR_ENVIRONMENT here");
            break;
        case FRMDIR_VERT_TOP_RIGHT:
            nVal = 1;
            pStr = sRTF_FRMTXTBRLV;
            break;
        case FRMDIR_HORI_RIGHT_TOP:
            bRTL = true;
//          nVal = 3;
//          A val of three isn't working as expected in word :-( so leave it
//          as normal ltr 0 textflow with rtl sect property, neither does
//          the frame textflow
//          pStr = sRTF_FRMTXTBRL;
            break;
        case FRMDIR_VERT_TOP_LEFT:
            nVal = 4;
            pStr = sRTF_FRMTXLRTBV;
            break;
    }

    if( rRTFWrt.pFlyFmt && rRTFWrt.bRTFFlySyntax && pStr )
    {
        rRTFWrt.Strm() << pStr;
        rRTFWrt.bOutFmtAttr = TRUE;
    }
    else if( rRTFWrt.bOutPageDesc)
    {
        if (nVal)
        {
            rRTFWrt.Strm() << sRTF_STEXTFLOW;
            rRTFWrt.OutULong( nVal );
        }
        if (bRTL)
            rRTFWrt.Strm() << sRTF_RTLSECT;
        rRTFWrt.bOutFmtAttr = TRUE;
    }
    else if (!rRTFWrt.pFlyFmt && !rRTFWrt.bOutPageDesc)
    {
        rRTFWrt.Strm() << (bRTL ? sRTF_RTLPAR : sRTF_LTRPAR);
        rRTFWrt.bOutFmtAttr = TRUE;
    }
}

void OutRTF_SwRTL(SwRTFWriter& rWrt, const SwTxtNode *pNd)
{
    if (!pNd)
        return;
    SvxFrameDirection eDir = FRMDIR_ENVIRONMENT;
    if (const SvxFrameDirectionItem* pItem = (const SvxFrameDirectionItem*)
        pNd->GetSwAttrSet().GetItem(RES_FRAMEDIR))
    {
        eDir = static_cast<SvxFrameDirection>(pItem->GetValue());
    }
    if (eDir == FRMDIR_ENVIRONMENT)
    {
        SwPosition aPos(*pNd);
        eDir =
            static_cast<SvxFrameDirection>(rWrt.pDoc->GetTextDirection(aPos));
    }
    OutSvxFrmDir(rWrt, SvxFrameDirectionItem(eDir));
}

static Writer& OutRTF_SwTxtINetFmt( Writer& rWrt, const SfxPoolItem& rHt )
{
    const SwFmtINetFmt& rURL = (const SwFmtINetFmt&)rHt;
    SwRTFWriter& rRTFWrt = (SwRTFWriter&)rWrt;
    if( rURL.GetValue().Len() )
    {
        rWrt.Strm() << '{' << sRTF_FIELD << '{' << sRTF_IGNORE
                    << sRTF_FLDINST << " HYPERLINK ";

        String sURL( rURL.GetValue() );
        if( INET_MARK_TOKEN != sURL.GetChar(0) )
        {
            INetURLObject aTmp( INetURLObject::AbsToRel( sURL ) );
            sURL = aTmp.GetURLNoMark( INetURLObject::DECODE_UNAMBIGUOUS);
/*          if( INET_PROT_FILE == aTmp.GetProtocol() )
            {
                // WW97 wollen keine FILE-URL haben, sondern einen normalen
                // Dateinamen. Aber ab WW2000 kennen sie FileURLs.
                sURL = aTmp.GetFull();
            }
*/          rWrt.Strm() << '\"';
            RTFOutFuncs::Out_String( rWrt.Strm(), sURL, DEF_ENCODING,
                                    rRTFWrt.bWriteHelpFmt ) << "\" ";
            sURL = aTmp.GetMark();
        }

        if( sURL.Len() )
        {
            rWrt.Strm() << "\\\\l \"";
            sURL.Erase( 0, 1 );
            RTFOutFuncs::Out_String( rWrt.Strm(), sURL, DEF_ENCODING,
                                    rRTFWrt.bWriteHelpFmt ) << "\" ";
        }

        if( rURL.GetTargetFrame().Len() )
        {
            rWrt.Strm() << "\\\\t \"";
            RTFOutFuncs::Out_String( rWrt.Strm(), rURL.GetTargetFrame(),
                        DEF_ENCODING, rRTFWrt.bWriteHelpFmt ) << "\" ";
        }

        rWrt.Strm() << "}{" << sRTF_FLDRSLT << ' ';
        rRTFWrt.bOutFmtAttr = FALSE;

        // und dann noch die Attributierung ausgeben
        const SwCharFmt* pFmt;
        const SwTxtINetFmt* pTxtAtr = rURL.GetTxtINetFmt();
        if( pTxtAtr && 0 != ( pFmt = pTxtAtr->GetCharFmt() ))
            OutRTF_SwFmt( rWrt, *pFmt );
    }
    return rWrt;
}

void HandleHyperlinks(Writer& rWrt, const SwpHints* pTxtAttrs, xub_StrLen nPos)
{
    USHORT nCount = pTxtAttrs ? pTxtAttrs->Count() : 0;
    for(USHORT i = 0; i < nCount; ++i )
    {
        const SwTxtAttr* pHt = (*pTxtAttrs)[i];
        const SfxPoolItem &rItem = pHt->GetAttr();
        if (rItem.Which() == RES_TXTATR_INETFMT)
        {
            const xub_StrLen* pEnd;
            if (nPos == *pHt->GetStart())
                OutRTF_SwTxtINetFmt(rWrt, rItem);
            if (0 != ( pEnd = pHt->GetEnd() ) && nPos == *pEnd)
            {
                // Hyperlinks werden als Felder geschrieben, aber der
                // "FieldResult" // steht als Text im TextNode. Also muss bei
                // diesen Attributen am // Ende 2 Klammern stehen!
                rWrt.Strm() << "}}";
            }
        }
    }
}

static Writer& OutRTF_SwTxtNode( Writer& rWrt, SwCntntNode& rNode )
{
    SwTxtNode * pNd = &((SwTxtNode&)rNode);
    SwRTFWriter & rRTFWrt = (SwRTFWriter&)rWrt;
    xub_StrLen nStrPos = rRTFWrt.pCurPam->GetPoint()->nContent.GetIndex();
    RTFEndPosLst aEndPosLst( rRTFWrt, *pNd, nStrPos );
    USHORT nAttrPos = 0;

    const String& rStr = pNd->GetTxt();
    xub_StrLen nEnde = rStr.Len();
    if( rRTFWrt.pCurPam->GetPoint()->nNode == rRTFWrt.pCurPam->GetMark()->nNode )
        nEnde = Min( nEnde, rRTFWrt.pCurPam->GetMark()->nContent.GetIndex() );

    int bNewFmts = rRTFWrt.GetAttrSet() != &pNd->GetSwAttrSet();
    if( bNewFmts )
    {
        // harte Attributierung am Node und am Vorgaenger ?
        const SfxItemSet* pNdSet = pNd->GetpSwAttrSet();
        if( pNdSet && rRTFWrt.GetAttrSet() && rRTFWrt.bAutoAttrSet &&
            pNdSet->GetParent() == rRTFWrt.GetAttrSet()->GetParent() &&
            pNdSet->Count() == rRTFWrt.GetAttrSet()->Count() )
        {
            // die beiden Parents sind gleich, dann teste doch mal die
            // Attribute im Set

            int bEqual = TRUE;
            if( pNdSet->Count() )
            {
                SfxItemIter aIter( *rRTFWrt.GetAttrSet() );
                const SfxPoolItem *pItem, *pCurr = aIter.GetCurItem();
                while( TRUE )
                {
                    if( SFX_ITEM_SET != pNdSet->GetItemState( pCurr->Which(),
                        FALSE, &pItem ) || *pItem != *pCurr )
                    {
                        bEqual = FALSE;
                        break;
                    }

                    if( aIter.IsAtEnd() )
                        break;
                    pCurr = aIter.NextItem();
                }
            }
            if( bEqual )
                bNewFmts = FALSE;
        }
        rRTFWrt.SetAttrSet( &pNd->GetSwAttrSet() );
        rRTFWrt.bAutoAttrSet = 0 != pNdSet;
    }

    // Flag zuruecksetzen, damit nach der Ausgabe der Collection
    // getestet werden kann, ob noch ein Blank auszugeben ist
    rRTFWrt.bOutFmtAttr = FALSE;

    // in der Ausgabe eines Flys? Dann vorm ausgeben der AbsatzAttribute
    // den Format-Pointer auf 0 setzen!
    SwFlyFrmFmt* pSaveFmt = rRTFWrt.pFlyFmt;

    SfxItemSet aMergedSet(rRTFWrt.pDoc->GetAttrPool(), POOLATTR_BEGIN,
        POOLATTR_END-1);
    bool bDeep = false;

    if( rRTFWrt.bWriteAll )
    {
        rRTFWrt.Strm() << sRTF_PARD << sRTF_PLAIN << ' ';       // alle Attribute zuruecksetzen
        if( rRTFWrt.bOutTable )
            rRTFWrt.Strm() << sRTF_INTBL;

        // ist der aktuelle Absatz in einem freifliegenden Rahmen ? Dann
        // muessen noch die Attribute dafuer ausgegeben werden.
        if( pSaveFmt )
        {
            OutRTF_SwFlyFrmFmt( rRTFWrt );
            rRTFWrt.pFlyFmt = 0;
        }

        rRTFWrt.OutListNum( *pNd );
        OutRTF_SwRTL(rRTFWrt, pNd);
        SwFmtToSet(rRTFWrt, pNd->GetAnyFmtColl(), aMergedSet);
        bDeep = true;
    }
    else if( !rRTFWrt.bWriteAll && rRTFWrt.bFirstLine )
    {
        OutRTF_SwRTL(rRTFWrt, pNd);
        SwFmtToSet(rRTFWrt, pNd->GetAnyFmtColl(), aMergedSet);
        bDeep = true;
    }

    // gibt es harte Attributierung ?
    if( bNewFmts && ( pNd->GetpSwAttrSet() || pNd->GetNum() ))
    {
        rRTFWrt.pFlyFmt = 0;

        const SfxItemSet& rNdSet = pNd->GetSwAttrSet();

        const SwNumRule* pRule;
        const SwNodeNum* pNum;
        if( (( 0 != ( pNum = pNd->GetNum() ) &&
                0 != ( pRule = pNd->GetNumRule() )) ||
                ( 0 != ( pNum = pNd->GetOutlineNum() ) &&
                0 != ( pRule = rWrt.pDoc->GetOutlineNumRule() ) ) ) &&
                pNum->GetLevel() < NO_NUM )
        {
            BYTE nLvl = GetRealLevel( pNum->GetLevel() );
            const SwNumFmt* pFmt = pRule->GetNumFmt( nLvl );
            if( !pFmt )
                pFmt = &pRule->Get( nLvl );

            SfxItemSet aSet( rNdSet );
            SvxLRSpaceItem aLR( (SvxLRSpaceItem&)rNdSet.Get( RES_LR_SPACE ) );

            aLR.SetTxtLeft( aLR.GetTxtLeft() + pFmt->GetAbsLSpace() );
            if( MAXLEVEL > pNum->GetLevel() )
                aLR.SetTxtFirstLineOfst( pFmt->GetFirstLineOffset() );
            else
                aSet.ClearItem( RES_PARATR_NUMRULE );
            aSet.Put( aLR );
            aMergedSet.Put(aSet);
        }
        else
            aMergedSet.Put(rNdSet);
    }

    OutRTF_SfxItemSet(rRTFWrt, aMergedSet, bDeep);

    rRTFWrt.pFlyFmt = pSaveFmt;

    // erstmal den Start berichtigen. D.h. wird nur ein Teil vom Satz
    // ausgegeben, so muessen auch da die Attribute stimmen!!
    const SwTxtAttr * pHt = 0;
    USHORT nCntAttr = pNd->HasHints() ? pNd->GetSwpHints().Count() : 0;
    if( nCntAttr && nStrPos > *( pHt = pNd->GetSwpHints()[ 0 ] )->GetStart() )
    {
        rRTFWrt.bTxtAttr = TRUE;
        // Ok, es gibt vorher Attribute, die ausgegeben werden muessen
        do {
            nAttrPos++;
            if( RES_TXTATR_FIELD == pHt->Which() )      // Felder nicht
                continue;                               // ausgeben

            if( pHt->GetEnd() )
            {
                xub_StrLen nHtEnd = *pHt->GetEnd(), nHtStt = *pHt->GetStart();
                if( !rRTFWrt.bWriteAll && nHtEnd <= nStrPos )
                    continue;

                // leere Hints am Anfang nicht beachten, oder ??
                if( nHtEnd == nHtStt )
                    continue;

                // Attribut in die Liste aufnehemen
                if( !rRTFWrt.bWriteAll )
                {
                    if( nHtStt < nStrPos ) nHtStt = nStrPos;
                    if( nHtEnd >= nEnde ) nHtEnd = nEnde;
                }
                aEndPosLst.Insert( pHt->GetAttr(), nHtStt, nHtEnd );
                continue;
                // aber nicht ausgeben, das erfolgt spaeter !!
            }
            Out( aRTFAttrFnTab, pHt->GetAttr(), rRTFWrt );

        } while( nAttrPos < nCntAttr && nStrPos >
            *( pHt = pNd->GetSwpHints()[ nAttrPos ] )->GetStart() );

        // dann gebe mal alle gesammelten Attribute von der String-Pos aus
        aEndPosLst.OutAttrs( nStrPos );

        rRTFWrt.bTxtAttr = FALSE;
    }

    if( rRTFWrt.bOutFmtAttr &&
        ( nAttrPos < nCntAttr ? *pHt->GetStart() != nStrPos : TRUE ))
        rRTFWrt.Strm() << ' ';

    // das Flag gibt an, ob das SwTxtFld am Ende vom Absatz steht. Denn
    // dann ist vor dem Absatzumbruch die schliessende Klammer auszugeben
    xub_StrLen nChrCnt = 0;
    for( ; nStrPos <= nEnde; nStrPos++ )
    {
        rRTFWrt.bOutFmtAttr = FALSE;
        if( nStrPos != nEnde && aEndPosLst.Count() )
            aEndPosLst.EndAttrs( nStrPos );

        // versuche nach ungefaehr 255 Zeichen eine neue Zeile zu beginnen
        if( nChrCnt != ( nStrPos & 0xff00 ))
        {
            rWrt.Strm() << SwRTFWriter::sNewLine;
            nChrCnt = nStrPos & 0xff00;
        }

        rRTFWrt.bTxtAttr = TRUE;

        HandleHyperlinks(rWrt, pNd->GetpSwpHints(), nStrPos);

        if( nAttrPos < nCntAttr && *pHt->GetStart() == nStrPos
            && nStrPos != nEnde )
        {
            do {
                BOOL bEmpty = FALSE;
                if( pHt->GetEnd() )
                {
                    if( FALSE == (bEmpty = *pHt->GetEnd() == nStrPos) )
                    {
                        aEndPosLst.Insert( pHt->GetAttr(), nStrPos,
                                            *pHt->GetEnd() );
                        continue;
                    }
                    rRTFWrt.Strm() << '{';
                }
                Out( aRTFAttrFnTab, pHt->GetAttr(), rRTFWrt );
                if( bEmpty )
                {
                    rRTFWrt.Strm() << '}';
                    rRTFWrt.bOutFmtAttr = FALSE;
                }
            } while( ++nAttrPos < nCntAttr && nStrPos ==
                *( pHt = pNd->GetSwpHints()[ nAttrPos ] )->GetStart() );

            // dann gebe mal alle gesammelten Attribute von der String-Pos aus
            aEndPosLst.OutAttrs( nStrPos );

        }
        else
            aEndPosLst.OutScriptChange( nStrPos );

        if( rRTFWrt.bOutFmtAttr )
            rRTFWrt.Strm() << ' ';
        rRTFWrt.bTxtAttr = FALSE;

        rRTFWrt.OutBookmarks( nStrPos );

        if (nStrPos != nEnde)
        {
            RTFOutFuncs::Out_String(rWrt.Strm(), String(rStr.GetChar(nStrPos)),
                DEF_ENCODING, rRTFWrt.bWriteHelpFmt);
        }
    }

    // noch eine schliesende Klammer da ??
    if( aEndPosLst.Count() )
        aEndPosLst.EndAttrs( USHRT_MAX );

    // wenn bis zum Ende vom Node, dann auch das AbsatzEnde ausgeben
    if( rRTFWrt.pCurPam->GetMark()->nNode.GetIndex() ==
        rRTFWrt.pCurPam->GetPoint()->nNode.GetIndex() )
    {
        if( pNd->Len() != rRTFWrt.pCurPam->GetMark()->nContent.GetIndex() )
            return rWrt;

        if( rRTFWrt.bOutTable )
        {
            rRTFWrt.Strm() << SwRTFWriter::sNewLine;
            return rWrt;
        }
    }

    rRTFWrt.Strm() << SwRTFWriter::sNewLine << sRTF_PAR << ' ';
    return rRTFWrt;
}



static Writer& OutRTF_SwGrfNode( Writer& rWrt, SwCntntNode & rNode )
{
    SwRTFWriter & rRTFWrt = (SwRTFWriter&)rWrt;
    SwGrfNode * pNd = &((SwGrfNode&)rNode);

    // ist der aktuelle Absatz in einem freifliegenden Rahmen ? Dann
    // muessen noch die Attribute dafuer ausgegeben werden.
    if( rRTFWrt.pFlyFmt )
        OutRTF_SwFlyFrmFmt( rRTFWrt );
    rRTFWrt.Strm() << "{{";

    // damit die eigenen Grafik-Attribute nach der PICT / import Anweisung
    // stehen, muessen die am Anfang ausgegeben werden.
    rRTFWrt.bOutFmtAttr = FALSE;
    OutRTF_SwFmt( rRTFWrt, *pNd->GetFmtColl() );

    if( rRTFWrt.bOutFmtAttr )       // wurde ueberhaupt ein Attrribut
        rRTFWrt.Strm() << "}{";     // ausgegeben ??

    String aGrfNm;
    const SwMirrorGrf& rMirror = pNd->GetSwAttrSet().GetMirrorGrf();
    if( !pNd->IsLinkedFile() || RES_DONT_MIRROR_GRF != rMirror.GetValue() )
    {
        USHORT nErr = 1;
        // Grafik als File-Referenz speichern (als JPEG-Grafik speichern)
        // but only if we save into a file and have a URL
        if( rWrt.GetOrigFileName() )
        {
            aGrfNm = *rWrt.GetOrigFileName();
            pNd->SwapIn( TRUE );
            ULONG nFlags = XOUTBMP_USE_NATIVE_IF_POSSIBLE;
            switch( rMirror.GetValue() )
            {
            case RES_MIRROR_GRF_VERT:   nFlags = XOUTBMP_MIRROR_HORZ; break;
            case RES_MIRROR_GRF_HOR:    nFlags = XOUTBMP_MIRROR_VERT; break;
            case RES_MIRROR_GRF_BOTH:
                nFlags = XOUTBMP_MIRROR_VERT | XOUTBMP_MIRROR_HORZ;
                break;
            }

            Size aMM100Size;
            Size* pMM100Size = 0;
            if( rRTFWrt.pFlyFmt )
            {
                const SwFmtFrmSize& rSize = rRTFWrt.pFlyFmt->GetFrmSize();
                aMM100Size = OutputDevice::LogicToLogic( rSize.GetSize(),
                                MapMode( MAP_TWIP ), MapMode( MAP_100TH_MM ));
                pMM100Size = &aMM100Size;
            }

            nErr = XOutBitmap::WriteGraphic( pNd->GetGrf(), aGrfNm,
                    String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "JPG" )),
                                                    nFlags, pMM100Size );
        }
        if( nErr )              // fehlerhaft, da ist nichts auszugeben
        {
            rRTFWrt.Strm() << "}}";
            return rWrt;
        }
    }
    else
        pNd->GetFileFilterNms( &aGrfNm, 0 );

    // MIB->JP: Warum erst AbsToRel und dann das URL-Objekt? So
    // kommt bei relativierbaren URLs als Protokoll "unknown" raus.
    // Ist das Absicht?
    aGrfNm = INetURLObject::AbsToRel( aGrfNm, INetURLObject::WAS_ENCODED,
                                    INetURLObject::DECODE_UNAMBIGUOUS);
    INetURLObject aUrl( aGrfNm );
    if( aUrl.GetProtocol() == INET_PROT_FILE )
        aGrfNm = aUrl.PathToFileName();

    // Bitmap als File-Referenz speichern
    rRTFWrt.Strm() << sRTF_FIELD << sRTF_FLDPRIV;
    OutComment( rRTFWrt, sRTF_FLDINST ) << "{\\\\import ";
    RTFOutFuncs::Out_String( rWrt.Strm(), aGrfNm, DEF_ENCODING,
                                rRTFWrt.bWriteHelpFmt );
    rRTFWrt.Strm() << "}}{" << sRTF_FLDRSLT << " }}";

    rRTFWrt.Strm() << '}' << SwRTFWriter::sNewLine;
    return rRTFWrt;
}

static Writer& OutRTF_SwOLENode( Writer& rWrt, SwCntntNode & rNode )
{
    SwRTFWriter & rRTFWrt = (SwRTFWriter&)rWrt;

    // ist der aktuelle Absatz in einem freifliegenden Rahmen ? Dann
    // muessen noch die Attribute dafuer ausgegeben werden.
    if( rRTFWrt.pFlyFmt )
        OutRTF_SwFlyFrmFmt( rRTFWrt );

    rWrt.Strm() << SwRTFWriter::sNewLine << sRTF_PAR;
    return rWrt;
}

static void OutTBLBorderLine(SwRTFWriter& rWrt, const SvxBorderLine* pLine,
    const sal_Char* pStr)
{
    ByteString sLineStr;
    if( pLine->GetInWidth() )
    {
        // doppelte Linie
        sLineStr = sRTF_BRDRDB;
        switch( pLine->GetInWidth() )
        {
        case DEF_LINE_WIDTH_0:
            ( sLineStr += sRTF_BRDRW ) += "15";
            break;
        case DEF_LINE_WIDTH_1:
            ( sLineStr += sRTF_BRDRW ) += "30";
            break;
        case DEF_LINE_WIDTH_2:
        case DEF_LINE_WIDTH_3:
            ( sLineStr += sRTF_BRDRW ) += "45";
            break;
        }
    }
    else
    {
        // einfache Linie
        if( DEF_LINE_WIDTH_1 >= pLine->GetOutWidth() )
            (( sLineStr = sRTF_BRDRS ) += sRTF_BRDRW ) +=
                        ByteString::CreateFromInt32( pLine->GetOutWidth() );
        else
            (( sLineStr = sRTF_BRDRTH ) += sRTF_BRDRW ) +=
                        ByteString::CreateFromInt32( pLine->GetOutWidth() / 2 );
    }

    rWrt.Strm() << pStr << sLineStr.GetBuffer() << sRTF_BRDRCF;
    rWrt.OutULong( rWrt.GetId( pLine->GetColor() ) );
}

static void OutBorderLine(SwRTFWriter& rWrt, const SvxBorderLine* pLine,
    const sal_Char* pStr, USHORT nDist)
{
    OutTBLBorderLine(rWrt, pLine, pStr);
    rWrt.Strm() << sRTF_BRSP;
    rWrt.OutULong( nDist );
}

static void OutSwTblBorder(SwRTFWriter& rWrt, const SvxBoxItem& rBox,
    const SvxBoxItem *pDefault)
{
    static const USHORT aBorders[] =
    {
        BOX_LINE_TOP, BOX_LINE_LEFT, BOX_LINE_BOTTOM, BOX_LINE_RIGHT
    };
    static const char* aBorderNames[] =
    {
        sRTF_CLBRDRT, sRTF_CLBRDRL, sRTF_CLBRDRB, sRTF_CLBRDRR
    };
    //Yes left and top are swapped with eachother for cell padding! Because
    //that's what the thunderingly annoying rtf export/import word xp does.
    static const char* aCellPadNames[] =
    {
        sRTF_CLPADL, sRTF_CLPADT, sRTF_CLPADB, sRTF_CLPADR
    };
    static const char* aCellPadUnits[] =
    {
        sRTF_CLPADFL, sRTF_CLPADFT, sRTF_CLPADFB, sRTF_CLPADFR
    };
    const USHORT* pBrd = aBorders;
    for (int i = 0; i < 4; ++i)
    {
        if (const SvxBorderLine* pLn = rBox.GetLine(aBorders[i]))
            OutTBLBorderLine(rWrt, pLn, aBorderNames[i]);
        if (!pDefault || pDefault->GetDistance(aBorders[i]) !=
            rBox.GetDistance(aBorders[i]))
        {
            rWrt.Strm() << aCellPadUnits[i];
            rWrt.OutULong(3);
            rWrt.Strm() << aCellPadNames[i];
            rWrt.OutULong(rBox.GetDistance(aBorders[i]));
        }
    }
}

static void OutSwTblBackground( SwRTFWriter& rWrt, const SvxBrushItem& rBack )
{
    if( !rBack.GetColor().GetTransparency() )
    {
        rWrt.Strm() << sRTF_CLCBPAT;
        rWrt.OutULong( rWrt.GetId( rBack.GetColor() ) );
    }
}


Writer& OutRTF_SwTblNode( Writer& rWrt, SwTableNode & rNode )
{
    SwRTFWriter & rRTFWrt = (SwRTFWriter&)rWrt;
    const SwTable& rTbl = rNode.GetTable();
    const SwTableLines& rLns = rTbl.GetTabLines();
    SwTwips nPageSize = 0, nTblOffset = 0;

/*
//!!!!!!!!!!!!! for clipboard create any view if the table is complex !!!
    if( rTbl.IsTblComplex() )
    {
        // then we have to create any layout
        SFX_APP()->CreateViewFrame( *xDocSh, 0, TRUE );
    }
*/

    const SwFrmFmt *pFmt = rTbl.GetFrmFmt();
    ASSERT(pFmt, "Impossible");
    {
        Point aPt;
        SwRect aRect( pFmt->FindLayoutRect( FALSE, &aPt ));
        if( aRect.IsEmpty() )
        {
            // dann besorge mal die Seitenbreite ohne Raender !!
            const SwFrmFmt* pFmt = rRTFWrt.pFlyFmt ? rRTFWrt.pFlyFmt :
                rWrt.pDoc->GetPageDesc(0).GetPageFmtOfNode( rNode, FALSE );

            aRect = pFmt->FindLayoutRect( TRUE );
            if( 0 == ( nPageSize = aRect.Width() ))
            {
                const SvxLRSpaceItem& rLR = pFmt->GetLRSpace();
                nPageSize = pFmt->GetFrmSize().GetWidth() -
                                rLR.GetLeft() - rLR.GetRight();
            }
        }
        else
            nPageSize = aRect.Width();
    }

    SwTwips nTblSz = pFmt->GetFrmSize().GetWidth();

    ByteString aTblAdjust( sRTF_TRQL );
    switch( pFmt->GetHoriOrient().GetHoriOrient() )
    {
    case HORI_CENTER:   aTblAdjust = sRTF_TRQC; break;
    case HORI_RIGHT:    aTblAdjust = sRTF_TRQR; break;
    case HORI_NONE:
    case HORI_LEFT_AND_WIDTH:
            {
                const SvxLRSpaceItem& rLRSp = pFmt->GetLRSpace();
                nTblOffset = rLRSp.GetLeft();
                nPageSize -= (nTblOffset + rLRSp.GetRight());
                aTblAdjust += sRTF_TRLEFT;
                aTblAdjust += ByteString::CreateFromInt32( nTblOffset );
            }
            break;
//  case case FLY_HORI_FULL:
    }

    if (rRTFWrt.TrueFrameDirection(*pFmt) == FRMDIR_HORI_RIGHT_TOP)
        aTblAdjust += sRTF_RTLROW;

    // ist die Tabelle wesentlich (PageSize + 10%) groesser als die Seite,
    // dann sind die Box-Breiten relative Angaben.
    BOOL bRelBoxSize = TRUE /*ALWAYS relativ (nPageSize + ( nPageSize / 10 )) < nTblSz*/;

    SwWriteTable* pTableWrt;
    const SwHTMLTableLayout *pLayout = rTbl.GetHTMLTableLayout();
    if( pLayout && pLayout->IsExportable() )
        pTableWrt = new SwWriteTable( pLayout );
    else
        pTableWrt = new SwWriteTable( rTbl.GetTabLines(), (USHORT)nPageSize,
                                          (USHORT)nTblSz, FALSE );

    // rCols are the array of all cols of the table
    const SwWriteTableCols& rCols = pTableWrt->GetCols();
    USHORT nColCnt = rCols.Count();
    SwWriteTableCellPtr* pBoxArr = new SwWriteTableCellPtr[ nColCnt ];
    USHORT* pRowSpans = new USHORT[ nColCnt ];
    memset( pBoxArr, 0, sizeof( pBoxArr[0] ) * nColCnt );
    memset( pRowSpans, 0, sizeof( pRowSpans[0] ) * nColCnt );
    long nLastHeight = 0;
    const SwWriteTableRows& rRows = pTableWrt->GetRows();
    for( USHORT nLine = 0; nLine < rRows.Count(); ++nLine )
    {
        USHORT nBox;

        const SwWriteTableRow *pRow = rRows[ nLine ];
        const SwWriteTableCells& rCells = pRow->GetCells();

        BOOL bFixRowHeight = FALSE;
        for( nColCnt = 0, nBox = 0; nBox < rCells.Count(); ++nColCnt )
        {
            if( !pRowSpans[ nColCnt ] )
            {
                // set new BoxPtr
                SwWriteTableCell* pCell = rCells[ nBox++ ];
                pBoxArr[ nColCnt ] = pCell;
                pRowSpans[ nColCnt ] = pCell->GetRowSpan();
                for( USHORT nCellSpan = pCell->GetColSpan(), nCS = 1;
                        nCS < nCellSpan; ++nCS, ++nColCnt )
                {
                    pBoxArr[ nColCnt+1 ] = pBoxArr[ nColCnt ];
                    pRowSpans[ nColCnt+1 ] = pRowSpans[ nColCnt ];
                }
            }
            if( 1 != pRowSpans[ nColCnt ] )
                bFixRowHeight = TRUE;
        }

        for( ; nColCnt < rCols.Count() && pRowSpans[ nColCnt ]; ++nColCnt )
            bFixRowHeight = TRUE;

        // Start Tabellendefinition
        rWrt.Strm() << sRTF_TROWD << aTblAdjust.GetBuffer();

        if( !nLine && rTbl.IsHeadlineRepeat() )
            rWrt.Strm() << sRTF_TRHDR;

        // Zeilenhoehe ausgeben
        long nHeight = 0;
        if( bFixRowHeight && rWrt.pDoc->GetRootFrm() )
        {
            nHeight = -pRow->GetPos();      //neg. => abs. height!
            if( nLine )
                nHeight += rRows[ nLine - 1 ]->GetPos();

            // merged line -> dont split it
            rWrt.Strm() << sRTF_TRKEEP;
        }
        else
        {
            const SwTableLine* pLine = pBoxArr[ 0 ]->GetBox()->GetUpper();
            const SwFmtFrmSize& rLSz = pLine->GetFrmFmt()->GetFrmSize();
            if( ATT_VAR_SIZE != rLSz.GetSizeType() && rLSz.GetHeight() )
                nHeight = ATT_MIN_SIZE == rLSz.GetSizeType()
                                                ? rLSz.GetHeight()
                                                : -rLSz.GetHeight();
        }
        if( nHeight )
        {
            rWrt.Strm() << sRTF_TRRH;
            rWrt.OutLong( nHeight );
        }

        const SvxBoxItem *pDefaultBox = 0;
        if (nColCnt)
        {
            pDefaultBox = &(pBoxArr[0]->GetBox()->GetFrmFmt()->GetBox());

            static const USHORT aBorders[] =
            {
                BOX_LINE_TOP, BOX_LINE_LEFT, BOX_LINE_BOTTOM, BOX_LINE_RIGHT
            };
            static const char* aRowPadNames[] =
            {
                sRTF_TRPADDT, sRTF_TRPADDL, sRTF_TRPADDB, sRTF_TRPADDR
            };
            static const char* aRowPadUnits[] =
            {
                sRTF_TRPADDFT, sRTF_TRPADDFL, sRTF_TRPADDFB, sRTF_TRPADDFR
            };
            const USHORT* pBrd = aBorders;
            for (int i = 0; i < 4; ++i)
            {
                rWrt.Strm() << aRowPadUnits[i];
                rWrt.OutULong(3);
                rWrt.Strm() << aRowPadNames[i];
                rWrt.OutULong(pDefaultBox->GetDistance(aBorders[i]));
            }
        }

        // Breite der Boxen ausgeben
        SwTwips nSz = 0, nCalc;
        for( nBox = 0; nBox < nColCnt; ++nBox )
        {
            SwWriteTableCell* pCell = pBoxArr[ nBox ];
            if( nBox && pBoxArr[ nBox-1 ] == pBoxArr[ nBox ] )
                continue;

            const SwFrmFmt& rFmt = *pCell->GetBox()->GetFrmFmt();
            if( 1 < pCell->GetRowSpan() )
                rWrt.Strm() << ( pCell->GetRowSpan() == pRowSpans[ nBox ]
                                ? sRTF_CLVMGF
                                : sRTF_CLVMRG );

            const SfxPoolItem* pItem;
            if (SFX_ITEM_SET == rFmt.GetAttrSet().GetItemState(RES_BOX, TRUE,
                &pItem))
            {
                OutSwTblBorder(rRTFWrt, (SvxBoxItem&)*pItem, pDefaultBox);
            }

// RTF kennt Schattierung in unserem Sinne nicht!
//              if( SFX_ITEM_SET == pBoxFmt->GetAttrSet().GetItemState(
//                                          RES_SHADOW, TRUE, &pItem ) )
//                  OutSwTblShadow( rRTFWrt, *pItem );

            if( SFX_ITEM_SET == rFmt.GetAttrSet().GetItemState(
                                            RES_BACKGROUND, TRUE, &pItem )
                || 0 != ( pItem = pCell->GetBackground() )
                || 0 != ( pItem = pRow->GetBackground() ) )
                OutSwTblBackground( rRTFWrt, (SvxBrushItem&)*pItem );

            if( SFX_ITEM_SET == rFmt.GetAttrSet().GetItemState(
                                            RES_VERT_ORIENT, TRUE, &pItem ) )
                switch( ((SwFmtVertOrient*)pItem)->GetVertOrient() )
                {
                case VERT_CENTER:   rWrt.Strm() << sRTF_CLVERTALC; break;
                case VERT_BOTTOM:   rWrt.Strm() << sRTF_CLVERTALB; break;
                default:            rWrt.Strm() << sRTF_CLVERTALT; break;
                }

            const SwFmtFrmSize& rLSz = rFmt.GetFrmSize();
            nSz += rLSz.GetWidth();
            rWrt.Strm() << sRTF_CELLX;

            nCalc = nSz;
            if( bRelBoxSize )
            {
                nCalc *= nPageSize;
                nCalc /= nTblSz;
            }
            rWrt.OutLong( nTblOffset + nCalc );
        }

        // Inhalt der Boxen ausgeben
        rWrt.Strm() << SwRTFWriter::sNewLine << sRTF_PARD << sRTF_INTBL;
        for( nBox = 0; nBox < nColCnt; ++nBox )
        {
            if( nBox && pBoxArr[ nBox-1 ] == pBoxArr[ nBox ] )
                continue;

            if( pBoxArr[ nBox ]->GetRowSpan() == pRowSpans[ nBox ] )
            {
                // new Box
                const SwStartNode* pSttNd = pBoxArr[ nBox ]->GetBox()->GetSttNd();
                RTFSaveData aSaveData( rRTFWrt,
                        pSttNd->GetIndex()+1, pSttNd->EndOfSectionIndex() );
                rRTFWrt.bOutTable = TRUE;
                rRTFWrt.Out_SwDoc( rRTFWrt.pCurPam );
            }
            rWrt.Strm() << sRTF_CELL;
        }

        // das wars mit der Line
        rWrt.Strm() << sRTF_ROW << sRTF_PARD << ' ';

        for( nBox = 0; nBox < nColCnt; ++nBox )
            --pRowSpans[ nBox ];
    }

    delete pTableWrt;
    delete pBoxArr;
    delete pRowSpans;

    // Pam hinter die Tabelle verschieben
    rRTFWrt.pCurPam->GetPoint()->nNode = *rNode.EndOfSectionNode();
    rRTFWrt.SetAttrSet( 0 );

    return rWrt;
}

Writer& OutRTF_SwSectionNode( Writer& rWrt, SwSectionNode& rNode )
{
    SwRTFWriter & rRTFWrt = (SwRTFWriter&)rWrt;
    const SwSection& rSect = rNode.GetSection();

    // folgt dahinter noch ein SectionNode? Dann wird erst die innere
    // Section aktiv. Hier wird die Verschachtelung aufgebrochen, weil
    // RTF das nicht kennt
    BOOL bPgDscWrite = FALSE;
    {
        SwNodeIndex aIdx( rNode, 1 );
        const SwNode& rNd = aIdx.GetNode();
        if( rNd.IsSectionNode() /*&& CONTENT_SECTION ==
            aIdx.GetNode().GetSectionNode()->GetSection().GetType()*/ )
            return rWrt;

        // falls als erstes Position ein Content- oder Tabellen-Node steht,
        // dann kann dieser einen PageDesc gesetzt haben und der muss vor
        // der Bereichsbeschreibung geschrieben werden!
        // Umgekehrt muss im OutBreaks dann
        if( rNd.IsCntntNode() )
        {
            bPgDscWrite = rRTFWrt.OutBreaks( ((SwCntntNode&)rNd).GetSwAttrSet() );
            rRTFWrt.bIgnoreNextPgBreak = TRUE;
        }
        else if( rNd.IsTableNode() )
        {
            bPgDscWrite = rRTFWrt.OutBreaks( ((SwTableNode&)rNd).GetTable().
                                            GetFrmFmt()->GetAttrSet() );
            rRTFWrt.bIgnoreNextPgBreak = TRUE;
        }
    }


//  if( CONTENT_SECTION == rSect.GetType() )
    {
        // als fortlaufenden Abschnittwechsel heraus schreiben
        if( !bPgDscWrite )
            rWrt.Strm() << sRTF_SECT << sRTF_SBKNONE;
        //JP 19.03.99 - es muss fuer den Import auf jedenfall das Cols
        //              Token geschrieben werden. Sonst kann nicht erkannt
        //              werden, wann ein PageDesc & eine Section gueltig ist
        rWrt.Strm() << sRTF_COLS << '1';
        rRTFWrt.bOutFmtAttr = TRUE;
        const SfxPoolItem* pItem;
        const SwFrmFmt *pFmt = rSect.GetFmt();
        ASSERT(pFmt, "Impossible");
        const SfxItemSet& rSet = pFmt->GetAttrSet();
        if( SFX_ITEM_SET == rSet.GetItemState( RES_COL, FALSE, &pItem ))
            OutRTF_SwFmtCol( rWrt, *pItem );
        else
            rWrt.Strm() << sRTF_COLS << '1' << sRTF_COLSX << '709';

        if( SFX_ITEM_SET == rSet.GetItemState( RES_COLUMNBALANCE,
            FALSE, &pItem ) && ((SwFmtNoBalancedColumns*)pItem)->GetValue() )
            OutComment( rWrt, sRTF_BALANCEDCOLUMN ) << '}';

        if (FRMDIR_HORI_RIGHT_TOP == rRTFWrt.TrueFrameDirection(*pFmt))
            rWrt.Strm() << sRTF_RTLSECT;
        else
            rWrt.Strm() << sRTF_LTRSECT;

        rWrt.Strm() << SwRTFWriter::sNewLine;
    }

    return rWrt;
}


/* File CHRATR.HXX: */

static Writer& OutRTF_SwFont( Writer& rWrt, const SfxPoolItem& rHt )
{
    /* trage den Font in die Font-Liste vom Writer ein und gebe hier nur
     *  die entsprechende Nummer aus. Der Font wird spaeter ueber diese
     * Tabelle im RTF-Dokument referenziert.
     */
    SwRTFWriter& rRTFWrt = (SwRTFWriter&)rWrt;

    if( !rRTFWrt.bTxtAttr ||
        ( rRTFWrt.GetEndPosLst() &&
        rRTFWrt.GetEndPosLst()->MatchScriptToId( rHt.Which() ) ))
    {
        rRTFWrt.bOutFmtAttr = true;
        const SvxFontItem&rFont = (const SvxFontItem&)rHt;
        bool bAssoc = rRTFWrt.IsAssociatedFlag();
        /*
         #109522#
         Word is a bit of a silly bugger of a program when its comes to symbol
         font useage. If a symbol font is actually being used, i.e.  exported
         here with bTxtAttr true then both AF and F must be set to the same
         value
        */
        if (rRTFWrt.bTxtAttr && (rFont.GetCharSet() == RTL_TEXTENCODING_SYMBOL))
        {
            const sal_Char* pCmd = !bAssoc ? sRTF_AF : sRTF_F;
            rWrt.Strm() << pCmd;
        }
        const sal_Char* pCmd = bAssoc ? sRTF_AF : sRTF_F;
        rWrt.Strm() << pCmd;
        rWrt.OutULong(rRTFWrt.GetId(rFont));
    }
    return rWrt;
}

static Writer& OutRTF_SwPosture( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwRTFWriter& rRTFWrt = (SwRTFWriter&)rWrt;

    if( !rRTFWrt.bTxtAttr ||
        ( rRTFWrt.GetEndPosLst() &&
        rRTFWrt.GetEndPosLst()->MatchScriptToId( rHt.Which() ) ))
    {
        const FontItalic nPosture = ((const SvxPostureItem&)rHt).GetPosture();
        int bTxtOut = rRTFWrt.bTxtAttr && ITALIC_NONE == nPosture;
        if( ITALIC_NORMAL == nPosture || bTxtOut )
        {
            rRTFWrt.bOutFmtAttr = TRUE;
            const sal_Char* pCmd = rRTFWrt.IsAssociatedFlag() ? sRTF_AI : sRTF_I;
            rWrt.Strm() << pCmd;
        }
        if( bTxtOut )
            rWrt.Strm() << '0';     // wieder abschalten
    }
    return rWrt;
}


static Writer& OutRTF_SwWeight( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwRTFWriter& rRTFWrt = (SwRTFWriter&)rWrt;
    if( !rRTFWrt.bTxtAttr ||
        ( rRTFWrt.GetEndPosLst() &&
        rRTFWrt.GetEndPosLst()->MatchScriptToId( rHt.Which() ) ))
    {
        const FontWeight nBold = ((const SvxWeightItem&)rHt).GetWeight();
        int bTxtOut = rRTFWrt.bTxtAttr && WEIGHT_NORMAL == nBold;
        if( WEIGHT_BOLD == nBold || bTxtOut )
        {
            rRTFWrt.bOutFmtAttr = TRUE;
            const sal_Char* pCmd = rRTFWrt.IsAssociatedFlag() ? sRTF_AB : sRTF_B;
            rWrt.Strm() << pCmd;
        }
        if( bTxtOut )
            rWrt.Strm() <<  '0';
    }
    return rWrt;
}

static Writer& OutRTF_SwEmphasisMark( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwRTFWriter& rRTFWrt = (SwRTFWriter&)rWrt;
    const sal_Char* pStr;
    switch( ((const SvxEmphasisMarkItem&)rHt).GetEmphasisMark())
    {
    case EMPHASISMARK_NONE:         pStr = sRTF_ACCNONE;    break;
    case EMPHASISMARK_SIDE_DOTS:    pStr = sRTF_ACCCOMMA;   break;
    default:                        pStr = sRTF_ACCDOT;     break;
    }

    rRTFWrt.bOutFmtAttr = TRUE;
    rWrt.Strm() << pStr;
    return rWrt;
}

static Writer& OutRTF_SwTwoInOne( Writer& rWrt, const SfxPoolItem& rHt )
{
    if( ((SvxTwoLinesItem&)rHt).GetValue() )
    {
        SwRTFWriter& rRTFWrt = (SwRTFWriter&)rWrt;

        sal_Unicode cStart = ((SvxTwoLinesItem&)rHt).GetStartBracket();
        sal_Unicode cEnd = ((SvxTwoLinesItem&)rHt).GetStartBracket();

        USHORT nType;
        if( !cStart && !cEnd )
            nType = 0;
        else if( '{' == cStart || '}' == cEnd )
            nType = 4;
        else if( '<' == cStart || '>' == cEnd )
            nType = 3;
        else if( '[' == cStart || ']' == cEnd )
            nType = 2;
        else                            // all other kind of brackets
            nType = 1;

        rWrt.Strm() << sRTF_TWOINONE;
        rWrt.OutULong( nType );
        rRTFWrt.bOutFmtAttr = TRUE;
    }
    return rWrt;
}

static Writer& OutRTF_SwCharRotate( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwRTFWriter& rRTFWrt = (SwRTFWriter&)rWrt;
    rRTFWrt.bOutFmtAttr = TRUE;
    rWrt.Strm() << sRTF_HORZVERT;
    rWrt.OutLong( ((SvxCharRotateItem&)rHt).IsFitToLine() ? 1 : 0 );
    return rWrt;
}
static Writer& OutRTF_SwCharScaleW( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwRTFWriter& rRTFWrt = (SwRTFWriter&)rWrt;
    rRTFWrt.bOutFmtAttr = TRUE;
    rWrt.Strm() << sRTF_CHARSCALEX;
    rWrt.OutLong( ((SvxCharScaleWidthItem&)rHt).GetValue() );
    return rWrt;
}

static Writer& OutRTF_SwCharRelief( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwRTFWriter& rRTFWrt = (SwRTFWriter&)rWrt;
    const SvxCharReliefItem& rAttr = (SvxCharReliefItem&)rHt;
    const sal_Char* pStr;
    switch( ((SvxCharReliefItem&)rHt).GetValue() )
    {
    case RELIEF_EMBOSSED:   pStr = sRTF_EMBO;   break;
    case RELIEF_ENGRAVED:   pStr = sRTF_IMPR;   break;
    default:                pStr = 0;           break;
    }

    if( pStr )
    {
        rRTFWrt.bOutFmtAttr = TRUE;
        rWrt.Strm() << pStr;
    }
    return rWrt;
}


static Writer& OutRTF_SwChrBckgrnd( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwRTFWriter& rRTFWrt = (SwRTFWriter&)rWrt;
    const SvxBrushItem& rBack = (const SvxBrushItem&)rHt;
    if( !rBack.GetColor().GetTransparency() )
    {
        ByteString sOut( sRTF_CHCBPAT );
        sOut += ByteString::CreateFromInt32(
                            rRTFWrt.GetId( rBack.GetColor() ));

        rRTFWrt.bOutFmtAttr = TRUE;
        rWrt.Strm() << sOut.GetBuffer();
    }
    return rWrt;
}

static Writer& OutRTF_SwShadowed( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwRTFWriter& rRTFWrt = (SwRTFWriter&)rWrt;
    const BOOL bShadow = ((const SvxShadowedItem&)rHt).GetValue();
    int bTxtOut = rRTFWrt.bTxtAttr && !bShadow;
    if( bShadow || bTxtOut )
    {
        rRTFWrt.bOutFmtAttr = TRUE;
        rWrt.Strm() << sRTF_SHAD;
    }
    if( bTxtOut )
        rWrt.Strm() << '0';
    return rWrt;
}



static Writer& OutRTF_SwContour( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwRTFWriter& rRTFWrt = (SwRTFWriter&)rWrt;
    const BOOL bContour = ((const SvxContourItem&)rHt).GetValue();
    int bTxtOut = rRTFWrt.bTxtAttr && !bContour;
    if( bContour || bTxtOut )
    {
        rRTFWrt.bOutFmtAttr = TRUE;
        rWrt.Strm() << sRTF_OUTL;
    }
    if( bTxtOut )
        rWrt.Strm() << '0';
    return rWrt;
}



#ifdef USED
static Writer& OutRTF_SwKerning( Writer& rWrt, const SfxPoolItem& )
{
    return rWrt;
}
#endif


static Writer& OutRTF_SwCrossedOut( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwRTFWriter& rRTFWrt = (SwRTFWriter&)rWrt;
    const FontStrikeout nStrike = ((const SvxCrossedOutItem&)rHt).GetStrikeout();
    int bTxtOut = rRTFWrt.bTxtAttr && STRIKEOUT_NONE == nStrike;

    if( (STRIKEOUT_NONE != nStrike && STRIKEOUT_DONTKNOW != nStrike) || bTxtOut )
    {
        rRTFWrt.bOutFmtAttr = TRUE;
        if( STRIKEOUT_DOUBLE == nStrike )
        {
            rWrt.Strm() << sRTF_STRIKED;
            if( !bTxtOut )
                rWrt.Strm() << '1';
        }
        else
            rWrt.Strm() << sRTF_STRIKE;
    }
    if( bTxtOut )
        rWrt.Strm() << '0';
    return rWrt;
}



static Writer& OutRTF_SwCaseMap( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwRTFWriter& rRTFWrt = (SwRTFWriter&)rWrt;
    switch( ((const SvxCaseMapItem&)rHt).GetValue() )
    {
    case SVX_CASEMAP_KAPITAELCHEN:
        rWrt.Strm() << sRTF_SCAPS;
        break;

    case SVX_CASEMAP_VERSALIEN:
        rWrt.Strm() << sRTF_CAPS;
        break;

    case SVX_CASEMAP_NOT_MAPPED:
        rWrt.Strm() << sRTF_CAPS << '0' << sRTF_SCAPS << '0';       // beide aus !!
        break;

    default:
        return rWrt;
    }

    rRTFWrt.bOutFmtAttr = TRUE;
    return rWrt;
}


static Writer& OutRTF_SwUnderline( Writer& rWrt, const SfxPoolItem& rHt )
{
    const char* pStr = 0;
    switch( ((const SvxUnderlineItem&)rHt).GetUnderline() )
    {
    case UNDERLINE_SINGLE:          pStr = sRTF_UL;         break;
    case UNDERLINE_DOUBLE:          pStr = sRTF_ULDB;       break;
    case UNDERLINE_NONE:            pStr = sRTF_ULNONE;     break;
    case UNDERLINE_DOTTED:          pStr = sRTF_ULD;        break;
    case UNDERLINE_DASH:            pStr = sRTF_ULDASH;     break;
    case UNDERLINE_DASHDOT:         pStr = sRTF_ULDASHD;    break;
    case UNDERLINE_DASHDOTDOT:      pStr = sRTF_ULDASHDD;   break;
    case UNDERLINE_BOLD:            pStr = sRTF_ULTH;       break;
    case UNDERLINE_WAVE:            pStr = sRTF_ULWAVE;     break;
    case UNDERLINE_BOLDDOTTED:      pStr = sRTF_ULTHD;      break;
    case UNDERLINE_BOLDDASH:        pStr = sRTF_ULTHDASH;   break;
    case UNDERLINE_LONGDASH:        pStr = sRTF_ULLDASH;    break;
    case UNDERLINE_BOLDLONGDASH:    pStr = sRTF_ULTHLDASH;  break;
    case UNDERLINE_BOLDDASHDOT:     pStr = sRTF_ULTHDASHD;  break;
    case UNDERLINE_BOLDDASHDOTDOT:  pStr = sRTF_ULTHDASHDD; break;
    case UNDERLINE_BOLDWAVE:        pStr = sRTF_ULHWAVE;    break;
    case UNDERLINE_DOUBLEWAVE:      pStr = sRTF_ULULDBWAVE; break;
    }

    if( pStr )
    {
        SwRTFWriter& rRTFWrt = (SwRTFWriter&)rWrt;

        if( UNDERLINE_SINGLE == ((const SvxUnderlineItem&)rHt).GetUnderline()
            && ((SvxWordLineModeItem&)rRTFWrt.GetItem(
                RES_CHRATR_WORDLINEMODE )).GetValue() )
            pStr = sRTF_ULW;

        rRTFWrt.Strm() << pStr;
        rRTFWrt.bOutFmtAttr = TRUE;

        rWrt.Strm() << sRTF_ULC;
        rWrt.OutULong( rRTFWrt.GetId(((const SvxUnderlineItem&)rHt).GetColor()) );

    }

    return rWrt;
}



static Writer& OutRTF_SwLanguage( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwRTFWriter& rRTFWrt = (SwRTFWriter&)rWrt;
    if( !rRTFWrt.bTxtAttr ||
        ( rRTFWrt.GetEndPosLst() &&
        rRTFWrt.GetEndPosLst()->MatchScriptToId( rHt.Which() ) ))
    {

        rRTFWrt.bOutFmtAttr = TRUE;
        const sal_Char* p = RES_CHRATR_CJK_LANGUAGE == rHt.Which()
                                    ? sRTF_LANGFE : sRTF_LANG;
        rWrt.Strm() << p;
        rWrt.OutULong( ((const SvxLanguageItem&)rHt).GetLanguage() );
    }
    return rWrt;
}



static Writer& OutRTF_SwEscapement( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwRTFWriter& rRTFWrt = (SwRTFWriter&)rWrt;
    const SvxEscapementItem& rEsc = (const SvxEscapementItem&)rHt;
    const char * pUpDn;

    SwTwips nH = ((SvxFontHeightItem&)rRTFWrt.GetItem(
                                RES_CHRATR_FONTSIZE )).GetHeight();

    if( 0 < rEsc.GetEsc() )
        pUpDn = sRTF_UP;
    else if( 0 > rEsc.GetEsc() )
    {
        pUpDn = sRTF_DN;
        nH = -nH;               // den negativen Wert herrausrechnen
    }
    else
        return rWrt;

    // prozentuale Veraenderung speichern !
    short nEsc = rEsc.GetEsc();
    short nProp = rEsc.GetProp() * 100;
    if( DFLT_ESC_AUTO_SUPER == nEsc )
    {
        nEsc = 100 - rEsc.GetProp();
        ++nProp;
    }
    else if( DFLT_ESC_AUTO_SUB == nEsc )
    {
        nEsc = - 100 + rEsc.GetProp();
        ++nProp;
    }

    OutComment( rWrt, sRTF_UPDNPROP, TRUE );
    rWrt.OutULong( nProp ) << '}' << pUpDn;

    /*
     * berechne aus der akt. FontSize und dem ProzentWert die Verschiebung,
     * wobei im RTF File 1/2 Points stehen muessen, waehrend intern
     * mit Twips gerechnet wird.
     * Formel :         (FontSize * 1/20 ) pts         x * 2
     *                  -----------------------  = ------------
     *                    100%                      Escapement
     */

    rWrt.OutULong( ( (long( nEsc ) * nH) + 500L ) / 1000L );
                                                // 500L zum Aufrunden !!
    return rWrt;
}



static Writer& OutRTF_SwSize( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwRTFWriter& rRTFWrt = (SwRTFWriter&)rWrt;
    if( !rRTFWrt.bTxtAttr ||
        ( rRTFWrt.GetEndPosLst() &&
        rRTFWrt.GetEndPosLst()->MatchScriptToId( rHt.Which() ) ))
    {
        rRTFWrt.bOutFmtAttr = TRUE;

        const sal_Char* pCmd = rRTFWrt.IsAssociatedFlag() ? sRTF_AFS : sRTF_FS;
        rWrt.Strm() << pCmd;
        rWrt.OutULong( ((const SvxFontHeightItem&)rHt).GetHeight() / 10 );
    }
    return rWrt;
}



static Writer& OutRTF_SwColor( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwRTFWriter& rRTFWrt = (SwRTFWriter&)rWrt;
    /* trage die Color in die Color-Liste vom Writer ein und gebe hier nur
     * die entsprechende Nummer aus. Die Color wird spaeter ueber diese
     * Tabelle im RTF-Dokument referenziert.
     */
    rRTFWrt.bOutFmtAttr = TRUE;
    rWrt.Strm() << sRTF_CF;
    rWrt.OutULong( rRTFWrt.GetId( ((const SvxColorItem&)rHt).GetValue() ));
    return rWrt;
}


extern void sw3io_ConvertToOldField( const SwField* pFld, USHORT& rWhich,
                              ULONG& rFmt, ULONG nFFVersion );

static Writer& OutRTF_SwField( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwFmtFld & rFld = (SwFmtFld&)rHt;
    SwRTFWriter& rRTFWrt = (SwRTFWriter&)rWrt;

    const SwField* pFld = rFld.GetFld();

    ByteString aFldStt( '{' );
    ((((aFldStt += sRTF_FIELD) += '{' ) += sRTF_IGNORE ) += sRTF_FLDINST) += ' ';
    switch( pFld->GetTyp()->Which() )
    {
    case RES_COMBINED_CHARS:
        {
            /*
            We need a font size to fill in the defaults, if these are overridden
            (as they generally are) by character properties then those properties
            win.

            The fontsize that is used in MS for determing the defaults is always
            the CJK fontsize even if the text is not in that language, in OOo the
            largest fontsize used in the field is the one we should take, but
            whatever we do, word will actually render using the fontsize set for
            CJK text. Nevertheless we attempt to guess whether the script is in
            asian or western text based up on the first character and use the
            font size of that script as our default.
            */
            const String& rFldPar1 = pFld->GetPar1();
            USHORT nScript;
            if( pBreakIt->xBreak.is() )
                nScript = pBreakIt->xBreak->getScriptType( rFldPar1, 0);
            else
                nScript = ::com::sun::star::i18n::ScriptType::ASIAN;

            long nHeight = ((SvxFontHeightItem&)rRTFWrt.GetItem(
                GetWhichOfScript(RES_CHRATR_FONTSIZE, nScript ))).GetHeight();
            nHeight = (nHeight + 10) / 20; //Font Size in points;

            /*
            Divide the combined char string into its up and down part. Get the
            font size and fill in the defaults as up == half the font size and
            down == a fifth the font size
            */
            xub_StrLen nAbove = (rFldPar1.Len()+1)/2;
            rWrt.Strm() << aFldStt.GetBuffer() << "EQ \\\\o (\\\\s\\\\up ";
            rWrt.OutLong( nHeight/2 ) << '(';
            RTFOutFuncs::Out_String( rWrt.Strm(), rFldPar1.Copy(0,nAbove),
                                    DEF_ENCODING, rRTFWrt.bWriteHelpFmt );
            rWrt.Strm() << "), \\\\s\\\\do ";
            rWrt.OutLong( nHeight/5 ) << '(';
            RTFOutFuncs::Out_String( rWrt.Strm(), rFldPar1.Copy( nAbove ),
                                    DEF_ENCODING, rRTFWrt.bWriteHelpFmt )
                    << "))";
        }
        break;

    case RES_DBFLD:
            aFldStt += "MERGEFIELD ";
            // kein break !!
    case RES_USERFLD:
            rWrt.Strm() << aFldStt.GetBuffer();
            RTFOutFuncs::Out_String( rWrt.Strm(), pFld->GetTyp()->GetName(),
                                    DEF_ENCODING, rRTFWrt.bWriteHelpFmt );
            break;
    case RES_GETREFFLD:
        {
            BYTE nFldTyp = 0;
            rWrt.Strm() << aFldStt.GetBuffer() << " REF ";
            const SwGetRefField& rRFld = *(SwGetRefField*)pFld;
            switch( pFld->GetSubType() )
            {
                case REF_SETREFATTR:
                case REF_BOOKMARK:
                    RTFOutFuncs::Out_String( rWrt.Strm(), rRFld.GetSetRefName(),
                                    DEF_ENCODING, rRTFWrt.bWriteHelpFmt );
                    nFldTyp = 3;
                    break;
            }

            if( nFldTyp )
            {
                switch( pFld->GetFormat() )
                {
                    case REF_PAGE_PGDESC:
                    case REF_PAGE:
                        rWrt.Strm() << "SEITEN";
                        nFldTyp = 37;
                        break;
                    case REF_UPDOWN:
                        rWrt.Strm() << " \\p";
                        nFldTyp = 3;
                        break;
                    case REF_CHAPTER:
                        rWrt.Strm() << " \\n";
                        break;
                    case REF_ONLYNUMBER:
                    case REF_ONLYCAPTION:
                    case REF_ONLYSEQNO:
                        break;
                    // default:
                    // case REF_CONTENT:
                }
                rWrt.Strm() << " \\\\h ";       // insert hyperlink
            }
        }
        break;
//  case RES_CHAPTERFLD:
//          rWrt.Strm() << ' ';
//          break;
    case RES_PAGENUMBERFLD:
            rWrt.Strm() << aFldStt.GetBuffer() << "\\\\page";
            {
                char __READONLY_DATA
                    sNType0[] = "ALPHABETIC",       /* CHARS_UPPER_LETTER*/
                    sNType1[] = "alphabetic",       /* CHARS_LOWER_LETTER*/
                    sNType2[] = "ROMAN",            /* ROMAN_UPPER       */
                    sNType3[] = "roman",            /* ROMAN_LOWER       */
                    sNType4[] = "ARABIC";           /* ARABIC            */

                const char* pFmtStr = 0;
                switch( pFld->GetFormat() )
                {
                case SVX_NUM_CHARS_UPPER_LETTER:
                case SVX_NUM_CHARS_UPPER_LETTER_N:  pFmtStr = sNType0; break;
                case SVX_NUM_CHARS_LOWER_LETTER:
                case SVX_NUM_CHARS_LOWER_LETTER_N:  pFmtStr = sNType1; break;
                case SVX_NUM_ROMAN_UPPER:           pFmtStr = sNType2; break;
                case SVX_NUM_ROMAN_LOWER:           pFmtStr = sNType3; break;
                case SVX_NUM_ARABIC:                pFmtStr = sNType4; break;
                }

                if( pFmtStr )
                    rWrt.Strm() << "\\\\* " << pFmtStr;
            }
            break;
    case RES_FILENAMEFLD:
            rWrt.Strm() << aFldStt.GetBuffer() << "\\\\filename ";
            break;
    case RES_DBNAMEFLD:
    {
            rWrt.Strm() << aFldStt.GetBuffer() << "\\\\data ";
            SwDBData aData = rWrt.pDoc->GetDBData();
            String sOut(aData.sDataSource);
            sOut += DB_DELIM;
            sOut += (String)aData.sCommand;
            RTFOutFuncs::Out_String( rWrt.Strm(), sOut,
                                    DEF_ENCODING, rRTFWrt.bWriteHelpFmt );
    }
    break;
    case RES_AUTHORFLD:
            rWrt.Strm() << aFldStt.GetBuffer() << "\\\\author ";
            break;

    case RES_HIDDENTXTFLD:
            if( TYP_CONDTXTFLD == ((SwHiddenTxtField*)pFld)->GetSubType() )
                RTFOutFuncs::Out_String( rWrt.Strm(), pFld->Expand(),
                                        DEF_ENCODING, rRTFWrt.bWriteHelpFmt );
            else
            {
                rWrt.Strm() << '{' << sRTF_V << ' ';
                OutRTF_AsByteString( rWrt, pFld->GetPar2() ).Strm()
                            << '}' << SwRTFWriter::sNewLine;
            }
            return rWrt;        // nicht bis zum Ende, kein RTF-Feld !!

    case RES_DATETIMEFLD:
            if (!(pFld->GetSubType() & FIXEDFLD))
            {
                USHORT nWhich = RES_DATETIMEFLD;
                ULONG nFmt = pFld->GetFormat();
                sw3io_ConvertToOldField(pFld, nWhich, nFmt, SOFFICE_FILEFORMAT_40 );
                if (pFld->GetSubType() & DATEFLD)
                {
                    rWrt.Strm() << aFldStt.GetBuffer() << "\\\\date \\\\@\"";
                    switch( nFmt )
                    {
                    case DF_SSYS:       rWrt.Strm() << "tt.MM.yy";              break;
                    case DF_LSYS:       rWrt.Strm() << "tttt, t. MMMM yyyy";    break;
                    case DF_SHORT:      rWrt.Strm() << "tt.MM.yy";              break;
                    case DF_SCENT:      rWrt.Strm() << "tt.MM.yyyy";            break;
                    case DF_LMON:       rWrt.Strm() << "t. MMM yyyy";           break;
                    case DF_LMONTH:     rWrt.Strm() << "t. MMMM yyyy";          break;
                    case DF_LDAYMON:    rWrt.Strm() << "ttt, t. MMMM yyyy";     break;
                    case DF_LDAYMONTH:  rWrt.Strm() << "tttt, t. MMMM yyyy";    break;
                    }
                }
                else
                {
                    rWrt.Strm() << aFldStt.GetBuffer() << "\\\\date \\\\@\"";
                    switch( nFmt )
                    {
                    case TF_SSMM_24:    rWrt.Strm() << "HH:mm"; break;
                    case TF_SSMM_12:    rWrt.Strm() << "hh:mm"; break;  // ???
                    case TF_SYSTEM:     rWrt.Strm() << "HH:mm"; break;
                    }
                }
                rWrt.Strm() << '\"';
                break;
            }
            // Kein break: in default-Zweig laufen!

    default:
        {
            aFldStt.Erase();

            // JP 20.07.95: warum nicht das Expandierte rausschreiben ?
            sal_Char cCh;
            rWrt.Strm().SeekRel(-1);
            rWrt.Strm() >> cCh;
            if( ' ' != cCh )            // vorweg immer einen Trenner
                rWrt.Strm() << ' ';
            RTFOutFuncs::Out_String( rWrt.Strm(), pFld->Expand(),
                                        DEF_ENCODING, rRTFWrt.bWriteHelpFmt );
        }
        break;
    }

    if( aFldStt.Len() )
    {
        rWrt.Strm() << "}{" << sRTF_FLDRSLT << ' ';
        RTFOutFuncs::Out_String( rWrt.Strm(), pFld->Expand(),
                                        DEF_ENCODING, rRTFWrt.bWriteHelpFmt );
        rWrt.Strm() << "}}";
        rRTFWrt.bOutFmtAttr = FALSE;
    }

    return rWrt;
}

static Writer& OutRTF_SwFlyCntnt( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwFrmFmt* pFmt = ((SwFmtFlyCnt&)rHt).GetFrmFmt();
    if( RES_DRAWFRMFMT != pFmt->Which() )
    {
        OutComment( rWrt, sRTF_FLYINPARA );
        ((SwRTFWriter&)rWrt).OutRTFFlyFrms( *((SwFlyFrmFmt*)pFmt) );
        rWrt.Strm() << '}';
        ((SwRTFWriter&)rWrt).bOutFmtAttr = FALSE;
    }
// Zeichengebundene Zeichenobjekte werden ignoriert.
    return rWrt;
}



#ifdef USED
static Writer& OutRTF_SwTOXMark( Writer& rWrt, const SfxPoolItem& )
{
    return rWrt;
}
#endif

#ifdef USED
static Writer& OutRTF_SwNoHyphen( Writer& rWrt, const SfxPoolItem& )
{
    return rWrt;
}
#endif


static Writer& OutRTF_SwFtn( Writer& rWrt, const SfxPoolItem& rHt )
{
    const SwFmtFtn& rFtn = (const SwFmtFtn&)rHt;
    SwRTFWriter& rRTFWrt = (SwRTFWriter&)rWrt;

    do {    // middle-check-loop
        if( !rFtn.GetTxtFtn() || !rFtn.GetTxtFtn()->GetStartNode() )
            break;          // es gibt keine Kopf-/Fusszeile/Fussnote

        // Hole vom Node und vom letzten Node die Position in der Section
        ULONG   nStart = rFtn.GetTxtFtn()->GetStartNode()->GetIndex()+1,
                nEnd = rWrt.pDoc->GetNodes()[ nStart-1 ]->EndOfSectionIndex();

        // kein Bereich also kein gueltiger Node
        if( nStart >= nEnd )
            break;

        rWrt.Strm() << '{' << sRTF_SUPER << ' ';
        if( !rFtn.GetNumStr().Len() )
        {
            rWrt.Strm() << sRTF_CHFTN;
            OutComment( rWrt, sRTF_FOOTNOTE );
            if( rFtn.IsEndNote() )
                rWrt.Strm() << sRTF_FTNALT;
            rWrt.Strm() << ' ' << sRTF_CHFTN;
        }
        else
        {
            OutRTF_AsByteString( rWrt, rFtn.GetNumStr() );
            OutComment( rWrt, sRTF_FOOTNOTE );
            if( rFtn.IsEndNote() )
                rWrt.Strm() << sRTF_FTNALT;
            rWrt.Strm() << ' ';
            OutRTF_AsByteString( rWrt, rFtn.GetNumStr() );
        }
        RTFSaveData aSaveData( rRTFWrt, nStart, nEnd );
        // damit kein \par ausgegeben wird !!
        rRTFWrt.pCurPam->GetMark()->nContent++;
        rRTFWrt.Out_SwDoc( rRTFWrt.pCurPam );

        rWrt.Strm() << "}}" << SwRTFWriter::sNewLine;
        rRTFWrt.bOutFmtAttr = FALSE;

    } while( FALSE );
    return rWrt;
}

static Writer& OutRTF_SwHardBlank( Writer& rWrt, const SfxPoolItem& rHt)
{
    RTFOutFuncs::Out_String(rWrt.Strm(),
        String(((SwFmtHardBlank&)rHt).GetChar()), DEF_ENCODING,
        ((SwRTFWriter&)rWrt).bWriteHelpFmt);
    return rWrt;
}

#ifdef USED
static Writer& OutRTF_SwRefMark( Writer& rWrt, const SfxPoolItem& )
{
    return rWrt;
}
#endif


#ifdef USED
static Writer& OutRTF_SwNoLinebreakHere( Writer& rWrt, const SfxPoolItem& )
{
    return rWrt;
}
#endif

static Writer& OutRTF_SwTxtCharFmt( Writer& rWrt, const SfxPoolItem& rHt )
{
    const SwFmtCharFmt& rChrFmt = (const SwFmtCharFmt&)rHt;
    const SwCharFmt* pFmt = rChrFmt.GetCharFmt();

    if( pFmt )
        OutRTF_SwFmt( rWrt, *pFmt );
    return rWrt;
}

static Writer& OutRTF_SwTxtRuby( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwRTFWriter& rRTFWrt = (SwRTFWriter&)rWrt;
    const SwFmtRuby& rRuby = (const SwFmtRuby&)rHt;
    const SwTxtRuby* pRubyTxt = rRuby.GetTxtRuby();
    const SwTxtNode* pNd;

    if( !pRubyTxt || 0 == (pNd = pRubyTxt->GetpTxtNode() ))
        return rWrt;

    sal_Char cDirective = 0, cJC = '0';
    switch( rRuby.GetAdjustment() )
    {
    case 0:     cJC = '3';  cDirective = 'l';   break;
    case 2:     cJC = '4';  cDirective = 'r';   break;
    case 3:     cJC = '1';  cDirective = 'd';   break;
    case 4:     cJC = '2';  cDirective = 'd';   break;
    case 1:     break;          //defaults to 0
    default:
        ASSERT( FALSE, "Unhandled Ruby justication code" );
        break;
    }

    /*
        MS needs to know the name and size of the font used in the ruby item,
        but we coud have written it in a mixture of asian and western
        scripts, and each of these can be a different font and size than the
        other, so we make a guess based upon the first character of the text,
        defaulting to asian.
        */
    USHORT nScript;
    if( pBreakIt->xBreak.is() )
        nScript = pBreakIt->xBreak->getScriptType( rRuby.GetText(), 0);
    else
        nScript = ::com::sun::star::i18n::ScriptType::ASIAN;

    const SwCharFmt* pFmt = pRubyTxt->GetCharFmt();
    const SvxFontItem *pFont;
    long nHeight;

    if( pFmt )
    {
        const SwAttrSet& rSet = pFmt->GetAttrSet();
        pFont = &(const SvxFontItem&)rSet.Get( GetWhichOfScript(
                                                RES_CHRATR_FONT, nScript ));

        nHeight = ((SvxFontHeightItem&)rSet.Get(
                GetWhichOfScript( RES_CHRATR_FONTSIZE, nScript ))).GetHeight();
    }
    else
    {
        /*Get document defaults if no formatting on ruby text*/
        const SfxItemPool *pPool = pNd->GetSwAttrSet().GetPool();
        pFont = &(const SvxFontItem&)pPool->GetDefaultItem(
                            GetWhichOfScript( RES_CHRATR_FONT, nScript ));

        nHeight = ((SvxFontHeightItem&)pPool->GetDefaultItem(
                GetWhichOfScript( RES_CHRATR_FONTSIZE, nScript ))).GetHeight();
    }
    ( nHeight += 5 ) /= 10;

    // don't change " EQ " to any other without changing the code in RTFFLD.CXX
    rWrt.Strm() << '{' << sRTF_FIELD << '{' << sRTF_IGNORE << sRTF_FLDINST
                << " EQ \\\\* jc" << cJC
                << " \\\\* \"Font:";
    RTFOutFuncs::Out_String( rWrt.Strm(), pFont->GetFamilyName(),
                            DEF_ENCODING, rRTFWrt.bWriteHelpFmt );
    rWrt.Strm() << "\" \\\\* hps";
    rWrt.OutLong( nHeight );
    rWrt.Strm() << " \\\\o";
    if( cDirective )
        rWrt.Strm() << "\\\\a" << cDirective;
    rWrt.Strm() << "(\\\\s\\\\up ";

    if( pBreakIt->xBreak.is() )
        nScript = pBreakIt->xBreak->getScriptType( pNd->GetTxt(),
                                                   *pRubyTxt->GetStart() );
    else
        nScript = ::com::sun::star::i18n::ScriptType::ASIAN;

    const SwAttrSet& rSet = pNd->GetSwAttrSet();
    nHeight = ((SvxFontHeightItem&)rSet.Get(
                GetWhichOfScript( RES_CHRATR_FONTSIZE, nScript ))).GetHeight();
    (nHeight += 10) /= 20-1;
    rWrt.OutLong( nHeight ) << '(';

    if( pFmt )
    {
        rWrt.Strm() << '{';
        OutRTF_SwFmt( rWrt, *pFmt );
        if( rRTFWrt.bOutFmtAttr )
            rWrt.Strm() << ' ';
    }
    RTFOutFuncs::Out_String( rWrt.Strm(), rRuby.GetText(),
                            DEF_ENCODING, rRTFWrt.bWriteHelpFmt );
    if( pFmt )
        rWrt.Strm() << '}';

    rWrt.Strm() << "),";
    rRTFWrt.bOutFmtAttr = FALSE;

    return rWrt;
}


/* File FRMATR.HXX */

static Writer& OutRTF_SwFrmSize( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwRTFWriter& rRTFWrt = (SwRTFWriter&)rWrt;
    const SwFmtFrmSize& rSz = (const SwFmtFrmSize&)rHt;
    if( rRTFWrt.pFlyFmt )       // wird das FlyFrmFmt ausgegeben ?
    {
        if( !rRTFWrt.bRTFFlySyntax )
            return rWrt;

        if( rSz.GetWidth() )
        {
            rWrt.Strm() << sRTF_ABSW;
            rWrt.OutLong( rSz.GetWidth() );
            rRTFWrt.bOutFmtAttr = TRUE;
        }

        if( rSz.GetHeight() )
        {
            long nH = rSz.GetHeight();
            if( ATT_FIX_SIZE == rSz.GetSizeType() )
                nH = -nH;
            rWrt.Strm() << sRTF_ABSH;
            rWrt.OutLong( nH );
            rRTFWrt.bOutFmtAttr = TRUE;
        }
    }
    else if( rRTFWrt.bOutPageDesc )
    {
        rWrt.Strm() << sRTF_PGWSXN;
        rWrt.OutLong( rSz.GetWidth() );
        rWrt.Strm() << sRTF_PGHSXN;
        rWrt.OutLong( rSz.GetHeight() );
        rRTFWrt.bOutFmtAttr = TRUE;
    }
    return rWrt;
}



#ifdef USED
static Writer& OutRTF_SwFillOrder( Writer& rWrt, const SfxPoolItem& )
{
    return rWrt;
}
#endif


#ifdef USED
static Writer& OutRTF_SwFmtPaperBin( Writer& rWrt, const SfxPoolItem& )
{
    return rWrt;
}
#endif



static Writer& OutRTF_SwFmtLRSpace( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwRTFWriter& rRTFWrt = (SwRTFWriter&)rWrt;
    const SvxLRSpaceItem & rLR = (const SvxLRSpaceItem&) rHt;
    if( !rRTFWrt.pFlyFmt )
    {
        if( rRTFWrt.bOutPageDesc )
        {
            if( rLR.GetLeft() )
            {
                rWrt.Strm() << sRTF_MARGLSXN;
                rWrt.OutLong( rLR.GetLeft() );
                rRTFWrt.bOutFmtAttr = TRUE;
            }
            if( rLR.GetRight() )
            {
                rWrt.Strm() << sRTF_MARGRSXN;
                rWrt.OutLong( rLR.GetRight() );
                rRTFWrt.bOutFmtAttr = TRUE;
            }
        }
        else
        {
            rRTFWrt.bOutFmtAttr = TRUE;
            rWrt.Strm() << sRTF_LI;
            rWrt.OutLong( rLR.GetTxtLeft() ) << sRTF_RI;
            rWrt.OutLong( rLR.GetRight() );
            rWrt.Strm() << sRTF_LIN;
            rWrt.OutLong( rLR.GetTxtLeft() ) << sRTF_RIN;
            rWrt.OutLong( rLR.GetRight() );
            rWrt.Strm() << sRTF_FI;
            rWrt.OutLong( rLR.GetTxtFirstLineOfst() );
        }
    }
    else if( rLR.GetLeft() == rLR.GetRight() && rRTFWrt.bRTFFlySyntax )
    {
        rRTFWrt.bOutFmtAttr = TRUE;
        rWrt.Strm() << sRTF_DFRMTXTX;
        rWrt.OutLong( rLR.GetLeft() );
    }
    return rWrt;
}

static Writer& OutRTF_SwFmtULSpace( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwRTFWriter& rRTFWrt = (SwRTFWriter&)rWrt;
    const SvxULSpaceItem & rUL = (const SvxULSpaceItem&) rHt;
    if( rRTFWrt.pFlyFmt )
    {
        if( rUL.GetUpper() == rUL.GetLower() && rRTFWrt.bRTFFlySyntax )
        {
            rRTFWrt.bOutFmtAttr = TRUE;
            rWrt.Strm() << sRTF_DFRMTXTY;
            rWrt.OutLong( rUL.GetLower() );
        }
    }
    else
    {
        const char* p;
        USHORT nValue = rUL.GetUpper();
        if( rRTFWrt.bOutPageDesc )
        {
            p = sRTF_MARGTSXN;
            if( !rRTFWrt.bOutPageDescTbl )
            {
                SwRect aRect;
                const SwFmtHeader* pHdr;
                if( SFX_ITEM_SET == rRTFWrt.pAktPageDesc->GetMaster().
                        GetItemState( RES_HEADER, FALSE,
                        (const SfxPoolItem**)&pHdr ) && pHdr->IsActive() )
                {
                    aRect = pHdr->GetHeaderFmt()->FindLayoutRect( FALSE );
                    if( aRect.Height() )
                        nValue += aRect.Height();
                    else
                    {
                        const SwFmtFrmSize& rSz = pHdr->GetHeaderFmt()->GetFrmSize();
                        if( ATT_VAR_SIZE != rSz.GetSizeType() )
                            nValue += rSz.GetHeight();
                        else
                            nValue += 274;      // defaulten fuer 12pt Schrift
                        nValue += pHdr->GetHeaderFmt()->GetULSpace().GetLower();
                    }
                }
            }
        }
        else
            p = sRTF_SB;

        if( rRTFWrt.bOutPageDesc || nValue )
        {
            rRTFWrt.bOutFmtAttr = TRUE;
            rWrt.Strm() << p;
            rWrt.OutLong( nValue );
        }


        nValue = rUL.GetLower();
        if( rRTFWrt.bOutPageDesc )
        {
            p = sRTF_MARGBSXN;
            if( !rRTFWrt.bOutPageDescTbl )
            {
                SwRect aRect;
                const SwFmtFooter* pFtr;
                if( SFX_ITEM_SET == rRTFWrt.pAktPageDesc->GetMaster().
                        GetItemState( RES_FOOTER, FALSE,
                        (const SfxPoolItem**)&pFtr ) && pFtr->IsActive() )
                {
                    aRect = pFtr->GetFooterFmt()->FindLayoutRect( FALSE );
                    if( aRect.Height() )
                        nValue += aRect.Height();
                    else
                    {
                        const SwFmtFrmSize& rSz = pFtr->GetFooterFmt()->GetFrmSize();
                        if( ATT_VAR_SIZE != rSz.GetSizeType() )
                            nValue += rSz.GetHeight();
                        else
                            nValue += 274;      // defaulten fuer 12pt Schrift
                        nValue += pFtr->GetFooterFmt()->GetULSpace().GetUpper();
                    }
                }
            }
        }
        else
            p = sRTF_SA;

        if( rRTFWrt.bOutPageDesc || nValue )
        {
            rRTFWrt.bOutFmtAttr = TRUE;
            rWrt.Strm() << p;
            rWrt.OutLong( nValue );
        }
    }
    return rWrt;
}

// Header-Footer werden auch vom RTF-Writer direkt gerufen, also kein static!


Writer& OutRTF_SwFmtHeader( Writer& rWrt, const SfxPoolItem& rHt )
{
    const SwFmtHeader& rHd = (const SwFmtHeader&)rHt;
    if( !rHd.IsActive() )       // nicht aktiv, dann nichts weiter ausgeben
        return rWrt;

    // hole einen Node zu dem Request
    SwStartNode *pSttNode = 0;
    const SwFmtCntnt& rCntnt = rHd.GetHeaderFmt()->GetCntnt();
    if( rCntnt.GetCntntIdx() )
        pSttNode = rCntnt.GetCntntIdx()->GetNode().GetStartNode();

    do {    // middle-check-loop
        if( !pSttNode )
            break;          // es gibt keine Kopf-/Fusszeile/Fussnote

        SwRTFWriter& rRTFWrt = (SwRTFWriter&)rWrt;

        // Hole vom Node und vom letzten Node die Position in der Section
        ULONG nStart = pSttNode->GetIndex() + 1,
                nEnd = pSttNode->EndOfSectionIndex();

        // kein Bereich also kein gueltiger Node
        if( nStart >= nEnd )
            break;
        ASSERT( rRTFWrt.pAktPageDesc, "Header-Attribut ohne PageDesc" );

        const sal_Char * pHdNm = sRTF_HEADER;
        rWrt.Strm() << pHdNm << 'y';
        if( rRTFWrt.bOutPageDescTbl )
        {
            // hole die Ober-/Unterkanten vom Header
            const SvxULSpaceItem& rUL = rHd.GetHeaderFmt()->GetULSpace();
            const SvxLRSpaceItem& rLR = rHd.GetHeaderFmt()->GetLRSpace();
            const SwFmtFrmSize& rSz = rHd.GetHeaderFmt()->GetFrmSize();

            rWrt.OutLong( rUL.GetUpper() );
            OutComment( rWrt, pHdNm ) << "yb";
            rWrt.OutLong( rUL.GetLower() ) << pHdNm << "xl";
            rWrt.OutLong( rLR.GetLeft() ) << pHdNm << "xr";
            rWrt.OutLong( rLR.GetRight() )  << pHdNm << "yh";
            rWrt.OutLong( ATT_FIX_SIZE  == rSz.GetSizeType()
                            ? -rSz.GetHeight()
                            : rSz.GetHeight() ) << '}';
        }
        else
            rWrt.OutLong( rRTFWrt.pAktPageDesc->GetMaster().
                            GetULSpace().GetUpper() );

        // wird nicht die PageDesc-Tabelle ausgegeben und gibt es einen
        // Nachfolger, dann handelt es sich um die "1.Seite" nach RTF.
        sal_Char cTyp = 0;
        if( rRTFWrt.pAktPageDesc->GetFollow() &&
            rRTFWrt.pAktPageDesc->GetFollow() != rRTFWrt.pAktPageDesc )
        {
            rWrt.Strm() << sRTF_TITLEPG;        //i13107
            cTyp = 'f';     // dann FirstPage-Header
        }
        else if( !rRTFWrt.pAktPageDesc->IsHeaderShared() )
        {
            rWrt.Strm() << sRTF_FACINGP;        //i13107
            cTyp = rRTFWrt.bOutLeftHeadFoot ? 'l' : 'r';
        }

        rWrt.Strm() << '{'<< pHdNm;
        if( cTyp ) rWrt.Strm() << cTyp;
        rWrt.Strm() << ' ';

        {
            RTFSaveData aSaveData( rRTFWrt, nStart, nEnd );
            rRTFWrt.Out_SwDoc( rRTFWrt.pCurPam );
        }

        rWrt.Strm() << '}' << SwRTFWriter::sNewLine;

    } while( FALSE );
    return rWrt;
}
// Header-Footer werden auch vom RTF-Writer direkt gerufen, also kein static!


Writer& OutRTF_SwFmtFooter( Writer& rWrt, const SfxPoolItem& rHt )
{
    const SwFmtFooter& rFt = (const SwFmtFooter&)rHt;
    if( !rFt.IsActive() )       // nicht aktiv, dann nichts weiter ausgeben
        return rWrt;

    SwStartNode *pSttNode = 0;
    const SwFmtCntnt& rCntnt = rFt.GetFooterFmt()->GetCntnt();
    if( rCntnt.GetCntntIdx() )
        pSttNode = rCntnt.GetCntntIdx()->GetNode().GetStartNode();

    do {    // middle-check-loop
        if( !pSttNode )
            break;          // es gibt keine Kopf-/Fusszeile/Fussnote

        SwRTFWriter& rRTFWrt = (SwRTFWriter&)rWrt;

            // Hole vom Node und vom letzten Node die Position in der Section
        ULONG nStart = pSttNode->GetIndex()+1,
              nEnd = pSttNode->EndOfSectionIndex();

        // kein Bereich also kein gueltiger Node
        if( nStart >= nEnd )
            break;
        ASSERT( rRTFWrt.pAktPageDesc, "Footer-Attribut ohne PageDesc" );

        const sal_Char * pFtNm = sRTF_FOOTER;
        rWrt.Strm() << pFtNm << 'y';
        if( rRTFWrt.bOutPageDescTbl )
        {
            // hole die Ober-/Unterkanten vom Footer
            const SvxULSpaceItem& rUL = rFt.GetFooterFmt()->GetULSpace();
            const SvxLRSpaceItem& rLR = rFt.GetFooterFmt()->GetLRSpace();
            const SwFmtFrmSize& rSz = rFt.GetFooterFmt()->GetFrmSize();

            rWrt.OutLong( rUL.GetLower() );
            OutComment( rWrt, pFtNm ) << "yt";
            rWrt.OutLong( rUL.GetUpper() ) << pFtNm << "xl";
            rWrt.OutLong( rLR.GetLeft() ) << pFtNm << "xr";
            rWrt.OutLong( rLR.GetRight() )  << pFtNm << "yh";
            rWrt.OutLong( ATT_FIX_SIZE  == rSz.GetSizeType()
                            ? -rSz.GetHeight()
                            : rSz.GetHeight() )  << '}';
        }
        else
            rWrt.OutLong( rRTFWrt.pAktPageDesc->GetMaster().
                            GetULSpace().GetLower() );

        // wird nicht die PageDesc-Tabelle ausgegeben und gibt es einen
        // Nachfolger, dann handelt es sich um die "1.Seite" nach RTF.
        sal_Char cTyp = 0;
        if( !rRTFWrt.bOutPageDesc && rRTFWrt.pAktPageDesc->GetFollow() &&
            rRTFWrt.pAktPageDesc->GetFollow() != rRTFWrt.pAktPageDesc )
        {
            rWrt.Strm() << sRTF_TITLEPG;        //i13107
            cTyp = 'f';     // dann FirstPage-Header
        }
        else if( !rRTFWrt.pAktPageDesc->IsFooterShared() )
        {
            rWrt.Strm() << sRTF_FACINGP;        //i13107
            cTyp = rRTFWrt.bOutLeftHeadFoot ? 'l' : 'r';
        }

        rWrt.Strm() << '{'<< pFtNm;
        if( cTyp ) rWrt.Strm() << cTyp;
        rWrt.Strm() << ' ';

        {
            RTFSaveData aSaveData( rRTFWrt, nStart, nEnd );
            rRTFWrt.Out_SwDoc( rRTFWrt.pCurPam );
        }

        rWrt.Strm() << '}' << SwRTFWriter::sNewLine;

    } while( FALSE );
    return rWrt;
}

static Writer& OutRTF_SwFmtPrint( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwRTFWriter& rRTFWrt = (SwRTFWriter&)rWrt;
    if( !rRTFWrt.bRTFFlySyntax && !((const SvxPrintItem&)rHt).GetValue() )
    {
        rRTFWrt.bOutFmtAttr = TRUE;
        rWrt.Strm() << sRTF_FLYPRINT;
    }
    return rWrt;
}


static Writer& OutRTF_SwFmtOpaque( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwRTFWriter& rRTFWrt = (SwRTFWriter&)rWrt;
    if( !rRTFWrt.bRTFFlySyntax && !((const SvxOpaqueItem&)rHt).GetValue() )
    {
        rRTFWrt.bOutFmtAttr = TRUE;
        rWrt.Strm() << sRTF_FLYOPAQUE;
    }
    return rWrt;
}


static Writer& OutRTF_SwFmtProtect( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwRTFWriter& rRTFWrt = (SwRTFWriter&)rWrt;
    if( !rRTFWrt.bRTFFlySyntax )
    {
        const SvxProtectItem & rFlyProtect = (const SvxProtectItem&) rHt;
        RTFProtect aP( rFlyProtect.IsCntntProtected(),
                        rFlyProtect.IsSizeProtected(),
                        rFlyProtect.IsPosProtected() );
        rRTFWrt.bOutFmtAttr = TRUE;
        rWrt.Strm() << sRTF_FLYPRTCTD;
        rWrt.OutULong( aP.GetValue() );
    }
    return rWrt;
}


static Writer& OutRTF_SwFmtSurround( Writer& rWrt, const SfxPoolItem& rHt )
{
    const SwFmtSurround& rFlySurround = (const SwFmtSurround&) rHt;
    SwRTFWriter& rRTFWrt = (SwRTFWriter&)rWrt;
    if( rRTFWrt.bRTFFlySyntax )
    {
        if( SURROUND_NONE == rFlySurround.GetSurround() )
        {
            rRTFWrt.bOutFmtAttr = TRUE;
            rWrt.Strm() << sRTF_NOWRAP;
        }
    }
    else
    {
        SwSurround eSurround = rFlySurround.GetSurround();
        BOOL bGold = SURROUND_IDEAL == eSurround;
        if( bGold )
            eSurround = SURROUND_PARALLEL;
        RTFSurround aMC( bGold, eSurround );
        rWrt.Strm() << sRTF_FLYMAINCNT;
        rWrt.OutULong( aMC.GetValue() );
        rRTFWrt.bOutFmtAttr = TRUE;
    }
    return rWrt;
}

static Writer& OutRTF_SwFmtVertOrient ( Writer& rWrt, const SfxPoolItem& rHt )
{
    const SwFmtVertOrient& rFlyVert = (const SwFmtVertOrient&) rHt;
    RTFVertOrient aVO( rFlyVert.GetVertOrient(), rFlyVert.GetRelationOrient() );
    SwRTFWriter& rRTFWrt = (SwRTFWriter&)rWrt;

    if( rRTFWrt.bRTFFlySyntax && rRTFWrt.pFlyFmt )
    {
        rRTFWrt.bOutFmtAttr = TRUE;
        const char* pOrient;
        RndStdIds eAnchor = rRTFWrt.pFlyFmt->GetAnchor().GetAnchorId();
        SwRelationOrient eOrient = rFlyVert.GetRelationOrient();
        if( FLY_PAGE == eAnchor )
        {
            if( REL_PG_FRAME == eOrient || FRAME == eOrient )
                pOrient = sRTF_PVPG;
            else
                pOrient = sRTF_PVMRG;
        }
        else
            pOrient = sRTF_PVPARA;
        rWrt.Strm() << pOrient;

        switch( rFlyVert.GetVertOrient() )
        {
        case VERT_TOP:
        case VERT_LINE_TOP:     rWrt.Strm() << sRTF_POSYT;  break;
        case VERT_BOTTOM:
        case VERT_LINE_BOTTOM:  rWrt.Strm() << sRTF_POSYB;  break;
        case VERT_CENTER:
        case VERT_LINE_CENTER:  rWrt.Strm() << sRTF_POSYC;  break;
        case VERT_NONE:
            {
                rWrt.Strm() << sRTF_POSY;
                rWrt.OutULong( rFlyVert.GetPos() );
            }
            break;
        }
    }
    else if( !rRTFWrt.bRTFFlySyntax )
    {
        rRTFWrt.bOutFmtAttr = TRUE;
        rWrt.Strm() << sRTF_FLYVERT;
        rWrt.OutULong( aVO.GetValue() );
    }

    return rWrt;
}

static Writer& OutRTF_SwFmtHoriOrient( Writer& rWrt, const SfxPoolItem& rHt )
{
    const SwFmtHoriOrient& rFlyHori = (const SwFmtHoriOrient&) rHt;
    RTFHoriOrient aHO( rFlyHori.GetHoriOrient(),
                          rFlyHori.GetRelationOrient() );

    SwRTFWriter& rRTFWrt = (SwRTFWriter&)rWrt;
    if( rRTFWrt.bRTFFlySyntax && rRTFWrt.pFlyFmt )
    {
        rRTFWrt.bOutFmtAttr = TRUE;
        const char* pS;
        RndStdIds eAnchor = rRTFWrt.pFlyFmt->GetAnchor().GetAnchorId();
        SwRelationOrient eOrient = rFlyHori.GetRelationOrient();
        if( FLY_PAGE == eAnchor )
        {
            if( REL_PG_FRAME == eOrient || FRAME == eOrient )
                pS = sRTF_PHPG;
            else
                pS = sRTF_PHMRG;
        }
        else
            pS = sRTF_PHCOL;
        rWrt.Strm() << pS;

        pS = 0;
        switch( rFlyHori.GetHoriOrient() )
        {
        case HORI_RIGHT:        pS = rFlyHori.IsPosToggle()
                                    ? sRTF_POSXO
                                    : sRTF_POSXR;
                                break;
        case HORI_LEFT:         pS = rFlyHori.IsPosToggle()
                                    ? sRTF_POSXI
                                    : sRTF_POSXL;
                                break;
        case HORI_CENTER:       pS = sRTF_POSXC;    break;
        case HORI_NONE:
                {
                    rWrt.Strm() << sRTF_POSX;
                    rWrt.OutULong( rFlyHori.GetPos() );
                }
                break;
        }
        if( pS )
            rWrt.Strm() << pS;
    }
    else
    {
        rRTFWrt.bOutFmtAttr = TRUE;
        rWrt.Strm()  << sRTF_FLYHORZ;
        rWrt.OutULong( aHO.GetValue() );
    }
    return rWrt;
}

static Writer& OutRTF_SwFmtAnchor( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwRTFWriter& rRTFWrt = (SwRTFWriter&)rWrt;
    if( !rRTFWrt.bRTFFlySyntax )
    {
        const SwFmtAnchor& rAnchor = (const SwFmtAnchor&) rHt;
        USHORT nId = rAnchor.GetAnchorId();
        rWrt.Strm() << sRTF_FLYANCHOR;
        rWrt.OutULong( nId );
        rRTFWrt.bOutFmtAttr = TRUE;
        switch( nId )
        {
        case FLY_PAGE:
                rWrt.Strm() << sRTF_FLYPAGE;
                rWrt.OutULong( rAnchor.GetPageNum() );
            break;
        case FLY_AT_CNTNT:
        case FLY_IN_CNTNT:
            rWrt.Strm() << sRTF_FLYCNTNT;
            break;
        }
    }
    return rWrt;
}



static Writer& OutRTF_SwFmtBackground( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwRTFWriter& rRTFWrt = (SwRTFWriter&)rWrt;
    // wird das FlyFrmFmt ausgegeben, dann Background nur ausgeben, wenn
    // RTF-Syntax gesetzt ist !
    if( !rRTFWrt.pFlyFmt || !rRTFWrt.bRTFFlySyntax )
    {
        const SvxBrushItem& rBack = (const SvxBrushItem&)rHt;
        if( !rBack.GetColor().GetTransparency() )
        {
            ByteString sOut( sRTF_CBPAT );
            sOut += ByteString::CreateFromInt32(
                                rRTFWrt.GetId( rBack.GetColor() ));

            if( rRTFWrt.pFlyFmt || rRTFWrt.bOutPageDesc )
            {
                rWrt.Strm() << '{' << sOut.GetBuffer() << '}';
            }
            else
            {
                rRTFWrt.bOutFmtAttr = TRUE;
                rWrt.Strm() << sOut.GetBuffer();
            }
        }
    }
    return rWrt;
}


static Writer& OutRTF_SwFmtShadow( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwRTFWriter& rRTFWrt = (SwRTFWriter&)rWrt;
    // wird das FlyFrmFmt ausgegeben, dann Schatten nur ausgeben, wenn
    // nicht RTF-Syntax gesetzt ist !
    if( rRTFWrt.pFlyFmt && rRTFWrt.bRTFFlySyntax )
        return rWrt;

    const SvxShadowItem& rShadow = (const SvxShadowItem&)rHt;
                             // FALSE wegen schliessender Klammer !!
    OutComment( rWrt, sRTF_SHADOW, FALSE );
    rWrt.OutULong( rShadow.GetLocation() ) << sRTF_SHDWDIST;
    rWrt.OutULong( rShadow.GetWidth() ) << sRTF_SHDWSTYLE;
    const Color& rColor = rShadow.GetColor();
    rWrt.OutULong( rColor.GetTransparency() ? SW_SV_BRUSH_NULL : SW_SV_BRUSH_SOLID );
    rWrt.OutULong( rRTFWrt.GetId( rColor ) ) << sRTF_SHDWFCOL;
    rWrt.OutULong( 0 ) << '}';
    return rWrt;
}


static void OutBorderLine( SwRTFWriter& rWrt, const SvxBorderLine* pLine,
                            const char* pStr )
{
    rWrt.Strm() << pStr << sRTF_BRDLNCOL;
    rWrt.OutULong( rWrt.GetId( pLine->GetColor() ) ) << sRTF_BRDLNIN;
    rWrt.OutULong( pLine->GetInWidth() ) << sRTF_BRDLNOUT;
    rWrt.OutULong( pLine->GetOutWidth() ) << sRTF_BRDLNDIST;
    rWrt.OutULong( pLine->GetDistance() );
}


static Writer& OutRTF_SwFmtBox( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwRTFWriter& rRTFWrt = (SwRTFWriter&)rWrt;

    const SvxBoxItem& rBox = (const SvxBoxItem&)rHt;

    static USHORT __READONLY_DATA aBorders[] = {
            BOX_LINE_TOP, BOX_LINE_LEFT, BOX_LINE_BOTTOM, BOX_LINE_RIGHT };
    static const sal_Char* __READONLY_DATA aBorderNames[] = {
            sRTF_BRDRT, sRTF_BRDRL, sRTF_BRDRB, sRTF_BRDRR };

    USHORT nDist = rBox.GetDistance();

    // wird das FlyFrmFmt ausgegeben, dann Border nur ausgeben, wenn
    // nicht RTF-Syntax gesetzt ist!
    if( rRTFWrt.pFlyFmt && rRTFWrt.bRTFFlySyntax )
    {
/*
RTF kennt keine Rahmen Umrandung!
        // die normale RTF-Definition
        if( rBox.GetTop() && rBox.GetBottom() &&
            rBox.GetLeft() && rBox.GetRight() &&
            *rBox.GetTop() == *rBox.GetBottom() &&
            *rBox.GetTop() == *rBox.GetLeft() &&
            *rBox.GetTop() == *rBox.GetRight() )
            OutBorderLine( rRTFWrt, rBox.GetTop(), sRTF_BOX, nDist );
        else
        {
            OUT_BRDLINE( rBox.GetTop(),     sRTF_BRDRT, nDist );
            OUT_BRDLINE( rBox.GetBottom(),  sRTF_BRDRB, nDist );
            OUT_BRDLINE( rBox.GetLeft(),    sRTF_BRDRL, nDist );
            OUT_BRDLINE( rBox.GetRight(),   sRTF_BRDRR, nDist );
        }
*/
        return rWrt;
    }
    else if( !rRTFWrt.pFlyFmt )
    {
        // erst die normale RTF-Definition, dann unsere eigene
        if( rBox.GetTop() && rBox.GetBottom() &&
            rBox.GetLeft() && rBox.GetRight() &&
            *rBox.GetTop() == *rBox.GetBottom() &&
            *rBox.GetTop() == *rBox.GetLeft() &&
            *rBox.GetTop() == *rBox.GetRight() &&
            nDist == rBox.GetDistance( BOX_LINE_TOP ) &&
            nDist == rBox.GetDistance( BOX_LINE_LEFT ) &&
            nDist == rBox.GetDistance( BOX_LINE_BOTTOM ) &&
            nDist == rBox.GetDistance( BOX_LINE_RIGHT ))
            OutBorderLine( rRTFWrt, rBox.GetTop(), sRTF_BOX, nDist );
        else
        {
            const USHORT* pBrd = aBorders;
            const sal_Char** pBrdNms = (const sal_Char**)aBorderNames;
            for(int i = 0; i < 4; ++i, ++pBrd, ++pBrdNms)
            {
                if (const SvxBorderLine* pLn = rBox.GetLine(*pBrd))
                {
                    OutBorderLine(rRTFWrt, pLn, *pBrdNms,
                        rBox.GetDistance(*pBrd));
                }
            }
        }
    }

    const USHORT* pBrd = aBorders;
    const sal_Char** pBrdNms = (const sal_Char**)aBorderNames;
    for( int i = 0; i < 4; ++i, ++pBrd, ++pBrdNms )
    {
        const SvxBorderLine* pLn = rBox.GetLine( *pBrd );
        if( pLn )
        {
            rWrt.Strm() << '{' << sRTF_IGNORE;
            OutBorderLine( rRTFWrt, pLn, *pBrdNms );
            rWrt.Strm() << '}' << sRTF_BRSP;
            rWrt.OutULong( rBox.GetDistance( *pBrd ));
        }
    }

    rRTFWrt.bOutFmtAttr = FALSE;
    return rWrt;
}


#ifdef USED
static Writer& OutRTF_SwFmtFrmMacro( Writer& rWrt, const SfxPoolItem& )
{
    return rWrt;
}
#endif

static Writer& OutRTF_SwFmtCol( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwRTFWriter& rRTFWrt = ((SwRTFWriter&)rWrt);
    if( rRTFWrt.pFlyFmt && rRTFWrt.bRTFFlySyntax )
        return rWrt;

    const SwFmtCol& rCol = (const SwFmtCol&)rHt;
    const SwColumns& rColumns = rCol.GetColumns();

    USHORT nCols = rColumns.Count();
    if( 1 < nCols )
    {
        // dann besorge mal die Seitenbreite ohne Raender !!
        const SwFrmFmt* pFmt;

        if( rRTFWrt.pFlyFmt )
            pFmt = rRTFWrt.pFlyFmt;
        else if( rRTFWrt.pAktPageDesc )
            pFmt = &rRTFWrt.pAktPageDesc->GetMaster();
        else
            pFmt = &rWrt.pDoc->GetPageDesc(0).GetMaster();

        const SvxLRSpaceItem& rLR = pFmt->GetLRSpace();

        USHORT nPageSize = pFmt->GetFrmSize().GetWidth() -
                            rLR.GetLeft() - rLR.GetRight();

        rWrt.Strm() << sRTF_COLS;
        rWrt.OutLong( nCols );

        if( rCol.IsOrtho() )
        {
            rWrt.Strm() << sRTF_COLSX;
            rWrt.OutLong( rCol.GetGutterWidth( TRUE ) );
        }
        else
            for( USHORT n = 0; n < nCols; )
            {
                rWrt.Strm() << sRTF_COLNO;
                rWrt.OutLong( n+1 );

                rWrt.Strm() << sRTF_COLW;
                rWrt.OutLong( rCol.CalcPrtColWidth( n, nPageSize ) );
                if( ++n != nCols )
                {
                    rWrt.Strm() << sRTF_COLSR;
                    rWrt.OutLong( rColumns[ n-1 ]->GetRight() +
                                  rColumns[ n ]->GetLeft() );
                }
            }
        ((SwRTFWriter&)rWrt).bOutFmtAttr = TRUE;
    }
    return rWrt;
}

static Writer& OutRTF_SvxFmtKeep( Writer& rWrt, const SfxPoolItem& rHt )
{
    const SvxFmtKeepItem& rItem = (const SvxFmtKeepItem&)rHt;
    if( rItem.GetValue() )
    {
        rWrt.Strm() << sRTF_KEEPN;
        ((SwRTFWriter&)rWrt).bOutFmtAttr = TRUE;
    }
    return rWrt;
}

static Writer& OutRTF_SvxFrmDir( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwRTFWriter& rRTFWrt = ((SwRTFWriter&)rWrt);
    if (rRTFWrt.pFlyFmt || rRTFWrt.bOutPageDesc)
        OutSvxFrmDir(rRTFWrt, rHt);
    return rWrt;
}

/* File GRFATR.HXX */

static Writer& OutRTF_SwMirrorGrf( Writer& rWrt, const SfxPoolItem& rHt )
{
    const SwMirrorGrf & rMirror = (const SwMirrorGrf&)rHt;
    if( RES_DONT_MIRROR_GRF == rMirror.GetValue() )
        return rWrt;

    ((SwRTFWriter&)rWrt).bOutFmtAttr = TRUE;
    rWrt.Strm() << sRTF_IGNORE << sRTF_GRFMIRROR;
    rWrt.OutULong( rMirror.GetValue() );
    return rWrt;
}

static Writer& OutRTF_SwCropGrf( Writer& rWrt, const SfxPoolItem& rHt )
{
    const SwCropGrf & rCrop = (const SwCropGrf&)rHt;
    ByteString aStr;
    if( rCrop.GetLeft() )
        ( aStr += sRTF_PICCROPL ) +=
                        ByteString::CreateFromInt32( (short)rCrop.GetLeft() );
    if( rCrop.GetRight() )
        ( aStr += sRTF_PICCROPR ) +=
                        ByteString::CreateFromInt32( (short)rCrop.GetRight() );
    if( rCrop.GetTop() )
        ( aStr += sRTF_PICCROPT ) +=
                        ByteString::CreateFromInt32( (short)rCrop.GetTop() );
    if( rCrop.GetBottom() )
        ( aStr += sRTF_PICCROPB ) +=
                        ByteString::CreateFromInt32( (short)rCrop.GetBottom() );
    if( aStr.Len() )
    {
        ((SwRTFWriter&)rWrt).bOutFmtAttr = TRUE;
        rWrt.Strm() << aStr.GetBuffer();
    }
    return rWrt;
}



/* File PARATR.HXX  */

static Writer& OutRTF_SwLineSpacing( Writer& rWrt, const SfxPoolItem& rHt )
{
    const SvxLineSpacingItem &rLs = (const SvxLineSpacingItem&)rHt;

    switch( rLs.GetLineSpaceRule() )
    {
    case SVX_LINE_SPACE_AUTO:
    case SVX_LINE_SPACE_FIX:
    case SVX_LINE_SPACE_MIN:
        {
            ((SwRTFWriter&)rWrt).bOutFmtAttr = TRUE;
            rWrt.Strm() << sRTF_SL;
            sal_Char cMult = '0';
            switch( rLs.GetInterLineSpaceRule() )
            {
            case SVX_INTER_LINE_SPACE_FIX:      // unser Durchschuss
                // gibt es aber nicht in WW - also wie kommt man an
                // die MaxLineHeight heran?
                rWrt.OutLong( (short)rLs.GetInterLineSpace() );
                break;

            case SVX_INTER_LINE_SPACE_PROP:
                rWrt.OutLong( (240L * rLs.GetPropLineSpace()) / 100L );
                cMult = '1';
                break;

            default:
                if( SVX_LINE_SPACE_FIX == rLs.GetLineSpaceRule() )
                    rWrt.Strm() << '-';

                rWrt.OutLong( rLs.GetLineHeight() );
                break;
            }
            rWrt.Strm() << sRTF_SLMULT << cMult;
        }
        break;
    }
    return rWrt;
}

static Writer& OutRTF_SwAdjust( Writer& rWrt, const SfxPoolItem& rHt )
{
    const SvxAdjustItem & rAdjust = ((const SvxAdjustItem&)rHt);
    ByteString aAttr( "\\q" );
    switch( rAdjust.GetAdjust() )
    {
    case SVX_ADJUST_LEFT:       aAttr += 'l';   break;
    case SVX_ADJUST_RIGHT:      aAttr += 'r';   break;
    case SVX_ADJUST_BLOCKLINE:
    case SVX_ADJUST_BLOCK:      aAttr += 'j';   break;
    case SVX_ADJUST_CENTER:     aAttr += 'c';   break;

    default:
        return rWrt;        // kein gueltiges Attriut, also returnen
    }
    ((SwRTFWriter&)rWrt).bOutFmtAttr = TRUE;
    rWrt.Strm() << aAttr.GetBuffer();
    return rWrt;
}

static Writer& OutRTF_SvxFmtSplit( Writer& rWrt, const SfxPoolItem& rHt )
{
    const SvxFmtSplitItem& rItem = (const SvxFmtSplitItem&)rHt;
    if( !rItem.GetValue() )
    {
        rWrt.Strm() << sRTF_KEEP;
        ((SwRTFWriter&)rWrt).bOutFmtAttr = TRUE;
    }
    return rWrt;
}


#ifdef USED
static Writer& OutRTF_SwWidows( Writer& rWrt, const SfxPoolItem& )
{
    return rWrt;
}
#endif

#ifdef USED
static Writer& OutRTF_SwOrphans( Writer& rWrt, const SfxPoolItem& )
{
    return rWrt;
}
#endif



static Writer& OutRTF_SwTabStop( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwRTFWriter& rRTFWrt = (SwRTFWriter&)rWrt;
    const SvxTabStopItem & rTStops = (const SvxTabStopItem&)rHt;
    long nOffset = ((SvxLRSpaceItem&)rRTFWrt.GetItem( RES_LR_SPACE )).GetTxtLeft();
    for( USHORT n = 0; n < rTStops.Count(); n++ )
    {
        const SvxTabStop & rTS = rTStops[ n ];
        if( SVX_TAB_ADJUST_DEFAULT != rTS.GetAdjustment() )
        {
            BOOL bOutDecimal = TRUE;
            const char* pFill = 0;
            switch( rTS.GetFill() )
            {
            case cDfltFillChar:
                break;

            case '.':   pFill = sRTF_TLDOT; break;
            case '_':   pFill = sRTF_TLUL;  break;
            case '-':   pFill = sRTF_TLTH;  break;
            case '=':   pFill = sRTF_TLEQ;  break;
            default:
                if( !rRTFWrt.bWriteHelpFmt )
                {
                    OutComment( rWrt, sRTF_TLSWG, FALSE );
                    rWrt.OutULong( (((USHORT)rTS.GetFill()) << 8 ) +
                                    rTS.GetDecimal() )
                        << '}';
                    bOutDecimal = FALSE;
                }
            }
            if( pFill )
                rWrt.Strm() << pFill;

            if( !rRTFWrt.bWriteHelpFmt && bOutDecimal &&
                rTS.GetDecimal() != ((SvxTabStopItem&)*GetDfltAttr(
                    RES_PARATR_TABSTOP ))[ 0 ].GetDecimal() )
            {
                OutComment( rWrt, sRTF_TLSWG, FALSE );
                rWrt.OutULong( (((USHORT)rTS.GetFill()) << 8 ) +
                                rTS.GetDecimal() ) << '}';
            }

            const sal_Char* pAdjStr = 0;
            switch( rTS.GetAdjustment() )
            {
            case SVX_TAB_ADJUST_RIGHT:      pAdjStr = sRTF_TQR;     break;
            case SVX_TAB_ADJUST_DECIMAL:    pAdjStr = sRTF_TQDEC;   break;
            case SVX_TAB_ADJUST_CENTER:     pAdjStr = sRTF_TQC;     break;
            }
            if( pAdjStr )
                rWrt.Strm() << pAdjStr;
            rWrt.Strm() << sRTF_TX;
            rWrt.OutLong( rTS.GetTabPos() + nOffset );
        }
    }
    rRTFWrt.bOutFmtAttr = TRUE;
    return rWrt;
}

static Writer& OutRTF_SwHypenZone( Writer& rWrt, const SfxPoolItem& rHt )
{
    if( !((SwRTFWriter&)rWrt).bWriteHelpFmt )
    {
        const SvxHyphenZoneItem& rAttr = (const SvxHyphenZoneItem&)rHt;
        USHORT nFlags = rAttr.IsHyphen() ? 1 : 0;
        if( rAttr.IsPageEnd() ) nFlags += 2;

        OutComment( rWrt, sRTF_HYPHEN, FALSE );
        rWrt.OutULong( nFlags ) << sRTF_HYPHLEAD;
        rWrt.OutULong( rAttr.GetMinLead() ) << sRTF_HYPHTRAIL;
        rWrt.OutULong( rAttr.GetMinTrail() ) << sRTF_HYPHMAX;
        rWrt.OutULong( rAttr.GetMaxHyphens() ) << '}';
    }
    return rWrt;
}

static Writer& OutRTF_SwNumRule( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwRTFWriter& rRTFWrt = (SwRTFWriter&)rWrt;
    if( !rRTFWrt.bOutListNumTxt )
    {
        const SwNumRuleItem& rAttr = (const SwNumRuleItem&)rHt;
        USHORT nId;
        if( rAttr.GetValue().Len() &&
            USHRT_MAX != (nId = rRTFWrt.GetId( rAttr ) ))
        {
            rWrt.Strm() << sRTF_LS;
            rWrt.OutULong( nId );
            rRTFWrt.bOutFmtAttr = TRUE;
        }
    }
    return rWrt;
}

static Writer& OutRTF_SwScriptSpace( Writer& rWrt, const SfxPoolItem& rHt )
{
    if( ((const SvxScriptSpaceItem&)rHt).GetValue() )
    {
        rWrt.Strm() << sRTF_ASPALPHA;
        ((SwRTFWriter&)rWrt).bOutFmtAttr = TRUE;
    }
    return rWrt;
}
static Writer& OutRTF_SwHangPunctuation( Writer& rWrt, const SfxPoolItem& rHt )
{
    if( !((const SvxHangingPunctuationItem&)rHt).GetValue() )
    {
        rWrt.Strm() << sRTF_NOOVERFLOW;
        ((SwRTFWriter&)rWrt).bOutFmtAttr = TRUE;
    }
    return rWrt;
}
static Writer& OutRTF_SwForbiddenRule( Writer& rWrt, const SfxPoolItem& rHt )
{
    if( !((const SvxForbiddenRuleItem&)rHt).GetValue() )
    {
        rWrt.Strm() << sRTF_NOCWRAP;
        ((SwRTFWriter&)rWrt).bOutFmtAttr = TRUE;
    }
    return rWrt;
}

static Writer& OutRTF_SwFontAlign( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwRTFWriter& rRTFWrt = (SwRTFWriter&)rWrt;
    const SvxParaVertAlignItem & rAttr = (const SvxParaVertAlignItem &)rHt;
    const char* pStr;
    switch ( rAttr.GetValue() )
    {
    case SvxParaVertAlignItem::TOP:         pStr = sRTF_FAHANG;     break;
    case SvxParaVertAlignItem::BOTTOM:      pStr = sRTF_FAVAR;      break;
    case SvxParaVertAlignItem::CENTER:      pStr = sRTF_FACENTER;   break;
    case SvxParaVertAlignItem::BASELINE:    pStr = sRTF_FAROMAN;    break;

//  case SvxParaVertAlignItem::AUTOMATIC:
    default:                                pStr = sRTF_FAAUTO;     break;
    }
    rWrt.Strm() << pStr;
    rRTFWrt.bOutFmtAttr = TRUE;
    return rWrt;
}

/*
 * lege hier die Tabellen fuer die RTF-Funktions-Pointer auf
 * die Ausgabe-Funktionen an.
 * Es sind lokale Strukturen, die nur innerhalb der RTF-DLL
 * bekannt sein muessen.
 */

SwAttrFnTab aRTFAttrFnTab = {
/* RES_CHRATR_CASEMAP   */          OutRTF_SwCaseMap,
/* RES_CHRATR_CHARSETCOLOR  */      0,
/* RES_CHRATR_COLOR */              OutRTF_SwColor,
/* RES_CHRATR_CONTOUR   */          OutRTF_SwContour,
/* RES_CHRATR_CROSSEDOUT    */      OutRTF_SwCrossedOut,
/* RES_CHRATR_ESCAPEMENT    */      OutRTF_SwEscapement,
/* RES_CHRATR_FONT  */              OutRTF_SwFont,
/* RES_CHRATR_FONTSIZE  */          OutRTF_SwSize,
/* RES_CHRATR_KERNING   */          0, // NOT USED!! OutRTF_SwKerning,
/* RES_CHRATR_LANGUAGE  */          OutRTF_SwLanguage,
/* RES_CHRATR_POSTURE   */          OutRTF_SwPosture,
/* RES_CHRATR_PROPORTIONALFONTSIZE*/0,
/* RES_CHRATR_SHADOWED  */          OutRTF_SwShadowed,
/* RES_CHRATR_UNDERLINE */          OutRTF_SwUnderline,
/* RES_CHRATR_WEIGHT    */          OutRTF_SwWeight,
/* RES_CHRATR_WORDLINEMODE  */      0, // Neu: Wortweises Unter-/Durchstreichen
/* RES_CHRATR_AUTOKERN  */          0, // Neu: Automatisches Pairkerning
/* RES_CHRATR_BLINK */              0, // Neu: Blinkender Text
/* RES_CHRATR_NOHYPHEN  */          0, // Neu: nicht trennen
/* RES_CHRATR_NOLINEBREAK */        0, // Neu: nicht umbrechen
/* RES_CHRATR_BACKGROUND */         OutRTF_SwChrBckgrnd, // Neu: Zeichenhintergrund
/* RES_CHRATR_CJK_FONT */           OutRTF_SwFont,
/* RES_CHRATR_CJK_FONTSIZE */       OutRTF_SwSize,
/* RES_CHRATR_CJK_LANGUAGE */       OutRTF_SwLanguage,
/* RES_CHRATR_CJK_POSTURE */        OutRTF_SwPosture,
/* RES_CHRATR_CJK_WEIGHT */         OutRTF_SwWeight,
/* RES_CHRATR_CTL_FONT */           OutRTF_SwFont,
/* RES_CHRATR_CTL_FONTSIZE */       OutRTF_SwSize,
/* RES_CHRATR_CTL_LANGUAGE */       OutRTF_SwLanguage,
/* RES_CHRATR_CTL_POSTURE */        OutRTF_SwPosture,
/* RES_CHRATR_CTL_WEIGHT */         OutRTF_SwWeight,
/* RES_CHRATR_ROTATE */             OutRTF_SwCharRotate,
/* RES_CHRATR_EMPHASIS_MARK */      OutRTF_SwEmphasisMark,
/* RES_CHRATR_TWO_LINES */          OutRTF_SwTwoInOne,
/* RES_CHRATR_SCALEW */             OutRTF_SwCharScaleW,
/* RES_CHRATR_RELIEF */             OutRTF_SwCharRelief,
/* RES_CHRATR_DUMMY1 */             0, // Dummy:

/* RES_TXTATR_INETFMT   */          OutRTF_SwTxtINetFmt, // Dummy
/* RES_TXTATR_NOHYPHEN  */          0, // Dummy
/* RES_TXTATR_REFMARK*/             0, // NOT USED!! OutRTF_SwRefMark,
/* RES_TXTATR_TOXMARK */            0, // NOT USED!! OutRTF_SwTOXMark,
/* RES_TXTATR_CHARFMT   */          OutRTF_SwTxtCharFmt,
/* RES_TXTATR_TWO_LINES */          0,
/* RES_TXTATR_CJK_RUBY */           OutRTF_SwTxtRuby,
/* RES_TXTATR_UNKNOWN_CONTAINER */  0,
/* RES_TXTATR_DUMMY5 */             0,
/* RES_TXTATR_DUMMY6 */             0,

/* RES_TXTATR_FIELD */              OutRTF_SwField,
/* RES_TXTATR_FLYCNT */             OutRTF_SwFlyCntnt,
/* RES_TXTATR_FTN */                OutRTF_SwFtn,
/* RES_TXTATR_SOFTHYPH */           0,  // old attr. - coded now by character
/* RES_TXTATR_HARDBLANK*/           OutRTF_SwHardBlank,
/* RES_TXTATR_DUMMY1 */             0, // Dummy:
/* RES_TXTATR_DUMMY2 */             0, // Dummy:

/* RES_PARATR_LINESPACING   */      OutRTF_SwLineSpacing,
/* RES_PARATR_ADJUST    */          OutRTF_SwAdjust,
/* RES_PARATR_SPLIT */              OutRTF_SvxFmtSplit,
/* RES_PARATR_WIDOWS    */          0, // NOT USED!! OutRTF_SwWidows,
/* RES_PARATR_ORPHANS   */          0, // NOT USED!! OutRTF_SwOrphans,
/* RES_PARATR_TABSTOP   */          OutRTF_SwTabStop,
/* RES_PARATR_HYPHENZONE*/          OutRTF_SwHypenZone,
/* RES_PARATR_DROP */               0,
/* RES_PARATR_REGISTER */           0, // neu:  Registerhaltigkeit
/* RES_PARATR_NUMRULE */            OutRTF_SwNumRule,
/* RES_PARATR_SCRIPTSPACE */        OutRTF_SwScriptSpace,
/* RES_PARATR_HANGINGPUNCTUATION */ OutRTF_SwHangPunctuation,
/* RES_PARATR_FORBIDDEN_RULE*/      OutRTF_SwForbiddenRule,
/* RES_PARATR_VERTALIGN */          OutRTF_SwFontAlign,
/* RES_PARATR_DUMMY3 */             0, // Dummy:
/* RES_PARATR_DUMMY4 */             0, // Dummy:
/* RES_PARATR_DUMMY5 */             0, // Dummy:
/* RES_PARATR_DUMMY6 */             0, // Dummy:
/* RES_PARATR_DUMMY7 */             0, // Dummy:
/* RES_PARATR_DUMMY8 */             0, // Dummy:

/* RES_FILL_ORDER   */              0, // NOT USED!! OutRTF_SwFillOrder,
/* RES_FRM_SIZE */                  OutRTF_SwFrmSize,
/* RES_PAPER_BIN    */              0, // NOT USED!! OutRTF_SwFmtPaperBin,
/* RES_LR_SPACE */                  OutRTF_SwFmtLRSpace,
/* RES_UL_SPACE */                  OutRTF_SwFmtULSpace,
/* RES_PAGEDESC */                  0,
/* RES_BREAK */                     0,
/* RES_CNTNT */                     0,
/* RES_HEADER */                    OutRTF_SwFmtHeader,
/* RES_FOOTER */                    OutRTF_SwFmtFooter,
/* RES_PRINT */                     OutRTF_SwFmtPrint,
/* RES_OPAQUE */                    OutRTF_SwFmtOpaque,
/* RES_PROTECT */                   OutRTF_SwFmtProtect,
/* RES_SURROUND */                  OutRTF_SwFmtSurround,
/* RES_VERT_ORIENT */               OutRTF_SwFmtVertOrient,
/* RES_HORI_ORIENT */               OutRTF_SwFmtHoriOrient,
/* RES_ANCHOR */                    OutRTF_SwFmtAnchor,
/* RES_BACKGROUND */                OutRTF_SwFmtBackground,
/* RES_BOX  */                      OutRTF_SwFmtBox,
/* RES_SHADOW */                    OutRTF_SwFmtShadow,
/* RES_FRMMACRO */                  0, // NOT USED!! OutRTF_SwFmtFrmMacro,
/* RES_COL */                       OutRTF_SwFmtCol,
/* RES_KEEP */                      OutRTF_SvxFmtKeep,
/* RES_URL */                       0, // URL
/* RES_EDIT_IN_READONLY */          0,
/* RES_LAYOUT_SPLIT */              0,
/* RES_FRMATR_DUMMY1 */             0, // Dummy:
/* RES_FRMATR_DUMMY2 */             0, // Dummy:
/* RES_FRMATR_DUMMY3 */             0, // Dummy:
/* RES_FRMATR_DUMMY4 */             0, // Dummy:
/* RES_FRMATR_DUMMY5 */             0, // Dummy:
/* RES_FRMATR_DUMMY6 */             0, // Dummy:
/* RES_FRAMEDIR*/                   OutRTF_SvxFrmDir,
/* RES_FRMATR_DUMMY8 */             0, // Dummy:
/* RES_FRMATR_DUMMY9 */             0, // Dummy:

/* RES_GRFATR_MIRRORGRF */          OutRTF_SwMirrorGrf,
/* RES_GRFATR_CROPGRF   */          OutRTF_SwCropGrf,
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
/* RES_BOXATR_VALUE */              0,

/* RES_UNKNOWNATR_CONTAINER */      0
};

SwNodeFnTab aRTFNodeFnTab = {
/* RES_TXTNODE  */                   OutRTF_SwTxtNode,
/* RES_GRFNODE  */                   OutRTF_SwGrfNode,
/* RES_OLENODE  */                   OutRTF_SwOLENode
};

/* vi:set tabstop=4 shiftwidth=4 expandtab: */

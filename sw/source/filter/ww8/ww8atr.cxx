/*************************************************************************
 *
 *  $RCSfile: ww8atr.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: jp $ $Date: 2000-11-20 09:38:45 $
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

/*
 * Dieses File enthaelt alle Ausgabe-Funktionen des WW8-Writers;
 * fuer alle Nodes, Attribute, Formate und Chars.
 */

#ifdef PRECOMPILED
#include "filt_pch.hxx"
#endif

#pragma hdrstop

#define ITEMID_BOXINFO      SID_ATTR_BORDER_INNER

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _APP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#ifndef _SV_SALBTYPE_HXX
#include <vcl/salbtype.hxx>
#endif
#ifndef _ZFORMAT_HXX
#include <svtools/zformat.hxx>
#endif
#ifndef _SFXITEMITER_HXX //autogen
#include <svtools/itemiter.hxx>
#endif
#ifndef _SVX_FONTITEM_HXX
#include <svx/fontitem.hxx>
#endif
#ifndef _SVX_TSTPITEM_HXX //autogen
#include <svx/tstpitem.hxx>
#endif
#ifndef _SVX_ADJITEM_HXX //autogen
#include <svx/adjitem.hxx>
#endif
#ifndef _SVX_SPLTITEM_HXX //autogen
#include <svx/spltitem.hxx>
#endif
#ifndef _SVX_WIDWITEM_HXX //autogen
#include <svx/widwitem.hxx>
#endif
#ifndef _SVX_LSPCITEM_HXX //autogen
#include <svx/lspcitem.hxx>
#endif
#ifndef _SVX_KEEPITEM_HXX //autogen
#include <svx/keepitem.hxx>
#endif
#ifndef _SVX_SHADITEM_HXX //autogen
#include <svx/shaditem.hxx>
#endif
#ifndef _SVX_BRSHITEM_HXX //autogen
#include <svx/brshitem.hxx>
#endif
#ifndef _SVX_POSTITEM_HXX //autogen
#include <svx/postitem.hxx>
#endif
#ifndef _SVX_WGHTITEM_HXX //autogen
#include <svx/wghtitem.hxx>
#endif
#ifndef _SVX_KERNITEM_HXX //autogen
#include <svx/kernitem.hxx>
#endif
#ifndef _SVX_CRSDITEM_HXX //autogen
#include <svx/crsditem.hxx>
#endif
#ifndef _SVX_CMAPITEM_HXX //autogen
#include <svx/cmapitem.hxx>
#endif
#ifndef _SVX_WRLMITEM_HXX //autogen
#include <svx/wrlmitem.hxx>
#endif
#ifndef _SVX_UDLNITEM_HXX //autogen
#include <svx/udlnitem.hxx>
#endif
#ifndef _SVX_LANGITEM_HXX //autogen
#include <svx/langitem.hxx>
#endif
#ifndef _SVX_ESCPITEM_HXX //autogen
#include <svx/escpitem.hxx>
#endif
#ifndef _SVX_FHGTITEM_HXX //autogen
#include <svx/fhgtitem.hxx>
#endif
#ifndef _SVX_COLRITEM_HXX //autogen
#include <svx/colritem.hxx>
#endif
#ifndef _SVX_HYZNITEM_HXX //autogen
#include <svx/hyznitem.hxx>
#endif
#ifndef _SVX_BRKITEM_HXX //autogen
#include <svx/brkitem.hxx>
#endif
#ifndef _SVX_LRSPITEM_HXX //autogen
#include <svx/lrspitem.hxx>
#endif
#ifndef _SVX_ULSPITEM_HXX //autogen
#include <svx/ulspitem.hxx>
#endif
#ifndef _SVX_BOXITEM_HXX //autogen
#include <svx/boxitem.hxx>
#endif
#ifndef _SVX_ITEM_HXX //autogen
#include <svx/cntritem.hxx>
#endif
#ifndef _SVX_SHDDITEM_HXX //autogen
#include <svx/shdditem.hxx>
#endif
#ifndef _SVX_AKRNITEM_HXX //autogen wg. SvxAutoKernItem
#include <svx/akrnitem.hxx>
#endif
#ifndef _SVX_PBINITEM_HXX //autogen wg. SvxPaperBinItem
#include <svx/pbinitem.hxx>
#endif

#ifndef _FMTFLD_HXX //autogen
#include <fmtfld.hxx>
#endif
#ifndef _FCHRFMT_HXX //autogen
#include <fchrfmt.hxx>
#endif
#ifndef _FMTFSIZE_HXX //autogen
#include <fmtfsize.hxx>
#endif
#ifndef _FMTPDSC_HXX //autogen
#include <fmtpdsc.hxx>
#endif
#ifndef _FMTHDFT_HXX //autogen
#include <fmthdft.hxx>
#endif
#ifndef _FMTORNT_HXX //autogen
#include <fmtornt.hxx>
#endif
#ifndef _FMTANCHR_HXX //autogen
#include <fmtanchr.hxx>
#endif
#ifndef _FMTCLDS_HXX //autogen
#include <fmtclds.hxx>
#endif
#ifndef _FMTSRND_HXX //autogen
#include <fmtsrnd.hxx>
#endif
#ifndef _FMTFTN_HXX //autogen
#include <fmtftn.hxx>
#endif
#ifndef _FMTFLCNT_HXX //autogen
#include <fmtflcnt.hxx>
#endif
#ifndef _FRMATR_HXX
#include <frmatr.hxx>
#endif
#ifndef _FMTINFMT_HXX //autogen wg. SwFmtINetFmt
#include <fmtinfmt.hxx>
#endif
#ifndef _TXTFLD_HXX //autogen wg. SwTxtFld
#include <txtfld.hxx>
#endif
#ifndef _TXTFTN_HXX //autogen wg. SwTxtFtn
#include <txtftn.hxx>
#endif
#ifndef _POOLFMT_HXX
#include <poolfmt.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>          // Doc fuer Fussnoten
#endif
#ifndef _PARATR_HXX
#include <paratr.hxx>
#endif
#ifndef _FLDBAS_HXX
#include <fldbas.hxx>       // fuer SwField ...
#endif
#ifndef _DOCUFLD_HXX
#include <docufld.hxx>       // fuer SwField ...
#endif
#ifndef _FORMAT_HXX
#include <format.hxx>
#endif
#ifndef _PAGEDESC_HXX
#include <pagedesc.hxx>     // fuer SwPageDesc...
#endif
#ifndef _FLDDAT_HXX
#include <flddat.hxx>       // fuer Datum-Felder
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>        // fuer Numrules
#endif
#ifndef _NDINDEX_HXX
#include <ndindex.hxx>
#endif
#ifndef _FMTHBSH_HXX //autogen
#include <fmthbsh.hxx>
#endif
#ifndef _SWRECT_HXX //autogen
#include <swrect.hxx>
#endif
#ifndef _REFFLD_HXX //autogen wg. SwGetRefField
#include <reffld.hxx>
#endif
#ifndef _FTNINFO_HXX
#include <ftninfo.hxx>
#endif
#ifndef _CHARFMT_HXX
#include <charfmt.hxx>
#endif
#ifndef _SECTION_HXX
#include <section.hxx>
#endif
#ifndef SW_LINEINFO_HXX
#include <lineinfo.hxx>
#endif
#ifndef SW_FMTLINE_HXX
#include <fmtline.hxx>
#endif
#ifndef _TOX_HXX
#include <tox.hxx>
#endif
#ifndef _FMTFTNTX_HXX
#include <fmtftntx.hxx>
#endif

#ifndef _UNOTOOLS_LOCALEDATAWRAPPER_HXX
#include <unotools/localedatawrapper.hxx>
#endif

#ifdef DEBUG
# ifndef _FMTCNTNT_HXX
#  include <fmtcntnt.hxx>
# endif
#endif

#ifndef _WRTWW8_HXX
#include <wrtww8.hxx>
#endif


#define APP_ASC(s)          AppendAscii( RTL_CONSTASCII_STRINGPARAM( s ))
#define ASIGN_ASC(s)        AssignAscii( RTL_CONSTASCII_STRINGPARAM( s ))
#define CREA_ASC(s)         String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( s ))

/*
 * um nicht immer wieder nach einem Update festzustellen, das irgendwelche
 * Hint-Ids dazugekommen sind, wird hier definiert, die Groesse der Tabelle
 * definiert und mit der akt. verglichen. Bei unterschieden wird der
 * Compiler schon meckern.
 *
 * diese Section und die dazugeherigen Tabellen muessen in folgenden Files
 * gepflegt werden: rtf\rtfatr.cxx, sw6\sw6atr.cxx, w4w\w4watr.cxx
*/

#if !defined(MSC) && !defined(UNX) && !defined(PPC) && !defined(CSET) && !defined(__MWERKS__) && !defined(WTC)

#define ATTRFNTAB_SIZE 130
#if ATTRFNTAB_SIZE != POOLATTR_END - POOLATTR_BEGIN
#error "Attribut-Tabelle ist ungueltigt. Wurden neue Hint-ID's zugefuegt ??"
#endif

#if 0
#define FORMATTAB_SIZE 6
#if FORMATTAB_SIZE != RES_FMT_END - RES_FMT_BEGIN
#error "Format-Tabelle ist ungueltigt. Wurden neue Hint-ID's zugefuegt ??"
#endif
#endif

#define NODETAB_SIZE 3
#if NODETAB_SIZE != RES_NODE_END - RES_NODE_BEGIN
#error "Node-Tabelle ist ungueltigt. Wurden neue Hint-ID's zugefuegt ??"
#endif

#endif



extern void sw3io_ConvertToOldField( const SwField* pFld, USHORT& rWhich,
                              ULONG& rFmt, ULONG nFFVersion );


//#define WW8_OUT( rWrt, a ) ((SwWW8Writer&)rWrt).pO->Insert( (BYTE*)&a,\
//                   sizeof( a ), ((SwWW8Writer&)rWrt).pO->Count() )

#define WW8_OUT( rWrt, a ) \
    ((SwWW8Writer&)rWrt).OutSprmBytes( (BYTE*)&a, sizeof( a ) )



//------------------------------------------------------------
//  Forward-Declarationen
//------------------------------------------------------------
static Writer& OutWW8_SwFmtBreak( Writer& rWrt, const SfxPoolItem& rHt );
static Writer& OutWW8_SwNumRuleItem( Writer& rWrt, const SfxPoolItem& rHt );

//------------------------------------------------------------
//  Hilfsroutinen fuer Styles
//------------------------------------------------------------

/*
 * Format wie folgt ausgeben:
 *      - gebe die Attribute aus; ohne Parents!
 */

void SwWW8Writer::Out_SfxItemSet( const SfxItemSet& rSet,
                                  BOOL bPapFmt, BOOL bChpFmt )
{
    if( rSet.Count() )
    {
        const SfxPoolItem* pItem;
        FnAttrOut pOut;

        pISet = &rSet;                  // fuer Doppel-Attribute

        // first the NumRule and than the LRSpace, so will W97 understand it
        // and move away the paragraph from the left side
        if( bPapFmt && SFX_ITEM_SET == rSet.GetItemState( RES_PARATR_NUMRULE,
                                                            FALSE, &pItem ))
        {
            pOut = aWW8AttrFnTab[ RES_PARATR_NUMRULE - RES_CHRATR_BEGIN ];
            (*pOut)( *this, *pItem );
        }

        SfxItemIter aIter( rSet );
        pItem = aIter.GetCurItem();

        do {
            register USHORT nWhich = pItem->Which();
            pOut = aWW8AttrFnTab[nWhich - RES_CHRATR_BEGIN];
            if( 0 != pOut && (!bPapFmt || RES_PARATR_NUMRULE != nWhich ))
            {
                BOOL bChp = nWhich >= RES_CHRATR_BEGIN
                            && nWhich < RES_TXTATR_END;
                BOOL bPap = nWhich >= RES_PARATR_BEGIN
                            && nWhich < RES_FRMATR_END;
                if( ( bChpFmt && bChp  ) || ( bPapFmt && bPap ) )
                    (*pOut)( *this, *pItem );
            }
        } while( !aIter.IsAtEnd() && 0 != ( pItem = aIter.NextItem() ) );

        pISet = 0;                      // fuer Doppel-Attribute
    }
}

// Da WW nur Break-After ( Pagebreak und Sectionbreaks ) kennt, im SW aber
// Bagebreaks "vor" und "nach" und Pagedescs nur "vor" existieren, werden
// die Breaks 2* durchgeklimpert, naemlich vor und hinter jeder Zeile.
// Je nach BreakTyp werden sie vor oder nach der Zeile gesetzt.
// Es duerfen nur Funktionen gerufen werden, die nicht in den
// Ausgabebereich pO schreiben, da dieser nur einmal fuer CHP und PAP existiert
// und damit im falschen landen wuerden.
void SwWW8Writer::Out_SfxBreakItems( const SfxItemSet& rSet, const SwNode& rNd )
{
    if( rSet.Count() )
    {
        bBreakBefore = TRUE;

        const SfxPoolItem* pItem;
        if( !bStyDef && !bOutKF && !bOutPageDescs && SFX_ITEM_SET ==
            rSet.GetItemState( RES_PAGEDESC, FALSE, &pItem ) &&
            ((SwFmtPageDesc*)pItem)->GetRegisteredIn() )
        {
            // Die PageDescs werden beim Auftreten von PageDesc-Attributen nur
            // in WW8Writer::pSepx mit der entsprechenden Position eingetragen.
            // Das Aufbauen und die Ausgabe der am PageDesc haengenden Attribute
            // und Kopf/Fusszeilen passiert nach dem Haupttext und seinen Attributen.
            const SwFmtPageDesc& rPgDesc = *(SwFmtPageDesc*)pItem;
            pAktPageDesc = rPgDesc.GetPageDesc();
            if( pAktPageDesc )
            {
                ULONG nFcPos = ReplaceCr( 0x0c ); // Page/Section-Break

                // tatsaechlich wird hier NOCH NICHTS ausgegeben, sondern
                // nur die Merk-Arrays aCps, aSects entsprechend ergaenzt
                if( nFcPos )
                    pSepx->AppendSep( Fc2Cp( nFcPos ), rPgDesc, rNd,
                                      ((SwFmtLineNumber&)rSet.Get(
                                            RES_LINENUMBER )).GetStartValue());
            }
        }
        else if( SFX_ITEM_SET == rSet.GetItemState( RES_BREAK, FALSE, &pItem ) )
            OutWW8_SwFmtBreak( *this, *pItem );

        bBreakBefore = FALSE;
    }
}

void SwWW8Writer::CorrTabStopInSet( SfxItemSet& rSet, USHORT nAbsLeft )
{
    const SfxPoolItem* pItem;
    if( SFX_ITEM_SET == rSet.GetItemState( RES_PARATR_TABSTOP, TRUE, &pItem ))
    {
        // dann muss das fuer die Ausgabe korrigiert werden
        SvxTabStopItem aTStop( *(SvxTabStopItem*)pItem );
        for( USHORT nCnt = 0; nCnt < aTStop.Count(); ++nCnt )
        {
            SvxTabStop& rTab = (SvxTabStop&)aTStop[ nCnt ];
            if( SVX_TAB_ADJUST_DEFAULT != rTab.GetAdjustment() &&
                rTab.GetTabPos() >= nAbsLeft )
                rTab.GetTabPos() -= nAbsLeft;
            else
            {
                aTStop.Remove( nCnt );
                --nCnt;
            }
        }
        rSet.Put( aTStop );
    }
}

BYTE SwWW8Writer::GetNumId( USHORT eNumType )
{
    BYTE nRet = 0;
    switch( eNumType )
    {
    case SVX_NUM_CHARS_UPPER_LETTER:
    case SVX_NUM_CHARS_UPPER_LETTER_N:  nRet = 3;       break;
    case SVX_NUM_CHARS_LOWER_LETTER:
    case SVX_NUM_CHARS_LOWER_LETTER_N:  nRet = 4;       break;
    case SVX_NUM_ROMAN_UPPER:           nRet = 1;       break;
    case SVX_NUM_ROMAN_LOWER:           nRet = 2;       break;

    case SVX_NUM_BITMAP:
    case SVX_NUM_CHAR_SPECIAL:          nRet = 23;      break;

    // nix, macht WW undokumentiert auch so
    case SVX_NUM_NUMBER_NONE:           nRet = 0xff;    break;
    }
    return nRet;
}

void SwWW8Writer::Out_SwFmt( const SwFmt& rFmt, BOOL bPapFmt, BOOL bChpFmt,
                                BOOL bFlyFmt )
{
    BOOL bCallOutSet = TRUE;
    const SwModify* pOldMod = pOutFmtNode;
    pOutFmtNode = &rFmt;

    switch( rFmt.Which() )
    {
    case RES_CONDTXTFMTCOLL:
    case RES_TXTFMTCOLL:
        if( bPapFmt )
        {
            BYTE nLvl = ((const SwTxtFmtColl&)rFmt).GetOutlineLevel();
            if( MAXLEVEL > nLvl )
            {
                // if Write StyleDefinition then write the OutlineRule
                const SwNumFmt& rNFmt = pDoc->GetOutlineNumRule()->Get(nLvl);
                if( bStyDef )
                {
                    if( nLvl >= nWW8MaxListLevel )
                        nLvl = nWW8MaxListLevel-1;

                    if( bWrtWW8 )
                    {
                        // write sprmPIlvl and sprmPIlfo
                        SwWW8Writer::InsUInt16( *pO, 0x260a );
                        pO->Insert( nLvl, pO->Count() );
                        SwWW8Writer::InsUInt16( *pO, 0x460b );
                        SwWW8Writer::InsUInt16( *pO, 1 + GetId(
                                            *pDoc->GetOutlineNumRule() ) );
                    }
                    else
                    {
                        Out_SwNumLvl( nLvl );
                        if( rNFmt.GetAbsLSpace() )
                        {
                            SwNumFmt aNumFmt( rNFmt );
                            const SvxLRSpaceItem& rLR = (SvxLRSpaceItem&)
                                            rFmt.GetAttr( RES_LR_SPACE );
                            aNumFmt.SetAbsLSpace( aNumFmt.GetAbsLSpace() +
                                                    rLR.GetLeft() );
                            Out_NumRuleAnld( *pDoc->GetOutlineNumRule(),
                                            aNumFmt, nLvl );
                        }
                        else
                            Out_NumRuleAnld( *pDoc->GetOutlineNumRule(),
                                            rNFmt, nLvl );
                    }
                }

                if( rNFmt.GetAbsLSpace() )
                {
                    SfxItemSet aSet( rFmt.GetAttrSet() );
                    SvxLRSpaceItem aLR( (SvxLRSpaceItem&)aSet.Get( RES_LR_SPACE ) );

                    aLR.SetTxtLeft( aLR.GetTxtLeft() + rNFmt.GetAbsLSpace() );
                    aLR.SetTxtFirstLineOfst( rNFmt.GetFirstLineOffset() );

                    aSet.Put( aLR );
                    SwWW8Writer::CorrTabStopInSet( aSet, rNFmt.GetAbsLSpace() );
                    Out_SfxItemSet( aSet, bPapFmt, bChpFmt );
                    bCallOutSet = FALSE;
                }
            }
        }
        break;

    case RES_CHRFMT:    break;
    case RES_FLYFRMFMT:
        if( bFlyFmt )
        {
            SfxItemSet aSet( pDoc->GetAttrPool(), RES_FRMATR_BEGIN,
                                                RES_FRMATR_END-1 );
            aSet.Set( pFlyFmt->GetAttrSet() );

            // Fly als Zeichen werden bei uns zu Absatz-gebundenen
            // jetzt den Abstand vom Absatz-Rand setzen
            if( pFlyOffset )
            {
                aSet.Put( SwFmtVertOrient( pFlyOffset->Y() ));
                aSet.Put( SwFmtHoriOrient( pFlyOffset->X() ));
            }

            if( SFX_ITEM_SET != aSet.GetItemState( RES_SURROUND ))
                aSet.Put( SwFmtSurround( SURROUND_NONE ) );

            bOutFlyFrmAttrs  = TRUE;
            Out_SfxItemSet( aSet, TRUE, FALSE );
            bOutFlyFrmAttrs = FALSE;

            bCallOutSet = FALSE;
        }
        break;
    default:            ASSERT( !this, "Was wird hier ausgegeben ??? " );
                        break;
    }

    if( bCallOutSet )
        Out_SfxItemSet( rFmt.GetAttrSet(), bPapFmt, bChpFmt );
    pOutFmtNode = pOldMod;
}

BOOL SwWW8Writer::HasRefToObject( USHORT nTyp, const String* pNm,
                                    USHORT nSeqNo )
{
    BOOL bFnd = FALSE;
    const SwTxtNode* pNd;
    SwClientIter aIter( *pDoc->GetSysFldType( RES_GETREFFLD ));
    for( SwFmtFld* pFld = (SwFmtFld*)aIter.First( TYPE( SwFmtFld ));
            pFld && !bFnd; pFld = (SwFmtFld*)aIter.Next() )
        if( pFld->GetTxtFld() && nTyp == pFld->GetFld()->GetSubType() &&
            0 != ( pNd = pFld->GetTxtFld()->GetpTxtNode() ) &&
            pNd->GetNodes().IsDocNodes() )
        {
            const SwGetRefField& rRFld = *(SwGetRefField*)pFld->GetFld();
            switch( nTyp )
            {
            case REF_BOOKMARK:
            case REF_SETREFATTR:    bFnd = *pNm == rRFld.GetSetRefName(); break;
            case REF_FOOTNOTE:
            case REF_ENDNOTE:       bFnd = nSeqNo == rRFld.GetSeqNo(); break;

            case REF_SEQUENCEFLD:   break;      // ???
            case REF_OUTLINE:       break;      // ???
            }
        }

    return bFnd;
}

String SwWW8Writer::GetBookmarkName( USHORT nTyp, const String* pNm,
                                    USHORT nSeqNo )
{
    String sRet;
    switch( nTyp )
    {
    case REF_SETREFATTR:    (sRet.APP_ASC( "Ref_" )) += *pNm; break;
    case REF_SEQUENCEFLD:   break;      // ???
    case REF_BOOKMARK:      sRet = *pNm;    break;
    case REF_OUTLINE:       break;      // ???
    case REF_FOOTNOTE:      (sRet.APP_ASC( "_RefF" ))
                                += String::CreateFromInt32( nSeqNo );
                            break;
    case REF_ENDNOTE:       (sRet.APP_ASC( "_RefE" ))
                                += String::CreateFromInt32( nSeqNo );
                            break;
    }
    return sRet;
}

//-----------------------------------------------------------------------
/*  */
/* File CHRATR.HXX: */

static Writer& OutWW8_SwBoldUSW( Writer& rWrt, BYTE nId, BOOL bVal )
{
    SwWW8Writer& rWrtWW8 = (SwWW8Writer&)rWrt;
    if( rWrtWW8.bWrtWW8 )
        rWrtWW8.InsUInt16( 7 == nId ? 0x2a53 : 0x0835 + nId );
    else if( 7 == nId )
        return rWrt;            // das Attribut gibt es im WW6 nicht
    else
        rWrtWW8.pO->Insert( 85 + nId, rWrtWW8.pO->Count() );
    rWrtWW8.pO->Insert( bVal ? 1 : 0, rWrtWW8.pO->Count() );
    return rWrt;
}

static Writer& OutWW8_SwFont( Writer& rWrt, const SfxPoolItem& rHt )
{
    const SvxFontItem& rAttr = (const SvxFontItem&)rHt;
    SwWW8Writer& rWrtWW8 = (SwWW8Writer&)rWrt;
    USHORT nFontID= rWrtWW8.GetId( rAttr );

    if( rWrtWW8.bWrtWW8 )
    {
        // what about sprmCIdctHint (286F) ??
        rWrtWW8.InsUInt16( 0x4a4f );
        rWrtWW8.InsUInt16( nFontID );
        rWrtWW8.InsUInt16( 0x4a51 );
    }
    else
        rWrtWW8.pO->Insert( 93, rWrtWW8.pO->Count() );
    rWrtWW8.InsUInt16( nFontID );
    return rWrt;
}

static Writer& OutWW8_SwPosture( Writer& rWrt, const SfxPoolItem& rHt )
{
    return OutWW8_SwBoldUSW( rWrt, 1,
                ITALIC_NONE != ((const SvxPostureItem&)rHt).GetPosture() );
}

static Writer& OutWW8_SwWeight( Writer& rWrt, const SfxPoolItem& rHt )
{
    return OutWW8_SwBoldUSW( rWrt, 0,
                WEIGHT_BOLD == ((const SvxWeightItem&)rHt).GetWeight() );
}

// Shadowed und Contour gibts in WW-UI nicht. JP: ??
static Writer& OutWW8_SwContour( Writer& rWrt, const SfxPoolItem& rHt )
{
    return OutWW8_SwBoldUSW( rWrt, 3,
                            ((const SvxContourItem&)rHt).GetValue() );
}

static Writer& OutWW8_SwShadow( Writer& rWrt, const SfxPoolItem& rHt )
{
    return OutWW8_SwBoldUSW( rWrt, 4,
                            ((const SvxShadowedItem&)rHt).GetValue() );
}

static Writer& OutWW8_SwKerning( Writer& rWrt, const SfxPoolItem& rHt )
{
    const SvxKerningItem& rAttr = (const SvxKerningItem&)rHt;
    SwWW8Writer& rWrtWW8 = (SwWW8Writer&)rWrt;
    if( rWrtWW8.bWrtWW8 )
        rWrtWW8.InsUInt16( 0x8840 );
    else
        rWrtWW8.pO->Insert( 96, rWrtWW8.pO->Count() );
    rWrtWW8.InsUInt16( rAttr.GetValue() );
    return rWrt;
}

static Writer& OutWW8_SvxAutoKern( Writer& rWrt, const SfxPoolItem& rHt )
{
    const SvxAutoKernItem& rAttr = (const SvxAutoKernItem&)rHt;
    SwWW8Writer& rWrtWW8 = (SwWW8Writer&)rWrt;
    if( rWrtWW8.bWrtWW8 )
        rWrtWW8.InsUInt16( 0x484B );
    else
        rWrtWW8.pO->Insert( 107, rWrtWW8.pO->Count() );
    rWrtWW8.pO->Insert( rAttr.GetValue() ? 1 : 0, rWrtWW8.pO->Count() );
    return rWrt;
}


static Writer& OutWW8_SwCrossedOut( Writer& rWrt, const SfxPoolItem& rHt )
{
    FontStrikeout eSt = ((const SvxCrossedOutItem&)rHt).GetStrikeout();
    if( STRIKEOUT_DOUBLE == eSt )
        return OutWW8_SwBoldUSW( rWrt, 7, TRUE );
    if( STRIKEOUT_NONE != eSt )
        return OutWW8_SwBoldUSW( rWrt, 2, TRUE );

    // dann auch beide ausschalten!
    OutWW8_SwBoldUSW( rWrt, 7, FALSE );
    return OutWW8_SwBoldUSW( rWrt, 2, FALSE );
}

static Writer& OutWW8_SwCaseMap( Writer& rWrt, const SfxPoolItem& rHt )
{
    USHORT eSt = ((const SvxCaseMapItem&)rHt).GetValue();
    if( SVX_CASEMAP_KAPITAELCHEN == eSt )
        return OutWW8_SwBoldUSW( rWrt, 5, TRUE );
    if( SVX_CASEMAP_VERSALIEN == eSt )
        return OutWW8_SwBoldUSW( rWrt, 6, TRUE );

    // dann auch beide ausschalten!
    OutWW8_SwBoldUSW( rWrt, 5, FALSE );
    return OutWW8_SwBoldUSW( rWrt, 6, FALSE );
}

static Writer& OutWW8_SwUnderline( Writer& rWrt, const SfxPoolItem& rHt )
{
    const SvxUnderlineItem& rAttr = (const SvxUnderlineItem&)rHt;
    SwWW8Writer& rWrtWW8 = (SwWW8Writer&)rWrt;
    if( rWrtWW8.bWrtWW8 )
        rWrtWW8.InsUInt16( 0x2A3E );
    else
        rWrtWW8.pO->Insert( 94, rWrtWW8.pO->Count() );

    const SfxPoolItem* pItem = ((SwWW8Writer&)rWrt).HasItem(
                                            RES_CHRATR_WORDLINEMODE );
    BOOL bWord = pItem ? ((const SvxWordLineModeItem*)pItem)->GetValue() : FALSE;

        // WW95 - parameters:   0 = none,   1 = single, 2 = by Word,
        //                      3 = double, 4 = dotted, 5 = hidden
        // WW97 - additional parameters:
                            //  6 = thick,   7 = dash,       8 = dot(not used)
                            //  9 = dotdash 10 = dotdotdash, 11 = wave
    BYTE b = 0;
    switch( rAttr.GetUnderline() )
    {
    case UNDERLINE_SINGLE:          b = ( bWord ) ? 2 : 1;          break;
    case UNDERLINE_BOLD:            b = rWrtWW8.bWrtWW8 ?  6 : 1;   break;
    case UNDERLINE_DOUBLE:          b = 3;                          break;
    case UNDERLINE_DOTTED:          b = 4;                          break;
    case UNDERLINE_DASH:            b = rWrtWW8.bWrtWW8 ?  7 : 4;   break;
    case UNDERLINE_DASHDOT:         b = rWrtWW8.bWrtWW8 ?  9 : 4;   break;
    case UNDERLINE_DASHDOTDOT:      b = rWrtWW8.bWrtWW8 ? 10 : 4;   break;
    case UNDERLINE_WAVE:            b = rWrtWW8.bWrtWW8 ? 11 : 3;   break;
    // -------------  new in WW2000  -------------------------------------
    case UNDERLINE_BOLDDOTTED:      b = rWrtWW8.bWrtWW8 ? 20 : 4;   break;
    case UNDERLINE_BOLDDASH:        b = rWrtWW8.bWrtWW8 ? 23 : 4;   break;
    case UNDERLINE_LONGDASH:        b = rWrtWW8.bWrtWW8 ? 39 : 4;   break;
    case UNDERLINE_BOLDLONGDASH:    b = rWrtWW8.bWrtWW8 ? 55 : 4;   break;
    case UNDERLINE_BOLDDASHDOT:     b = rWrtWW8.bWrtWW8 ? 25 : 4;   break;
    case UNDERLINE_BOLDDASHDOTDOT:  b = rWrtWW8.bWrtWW8 ? 26 : 4;   break;
    case UNDERLINE_BOLDWAVE:        b = rWrtWW8.bWrtWW8 ? 27 : 3;   break;
    case UNDERLINE_DOUBLEWAVE:      b = rWrtWW8.bWrtWW8 ? 43 : 3;   break;
    }

    rWrtWW8.pO->Insert( b, rWrtWW8.pO->Count() );
    return rWrt;
}

static Writer& OutWW8_SwLanguage( Writer& rWrt, const SfxPoolItem& rHt )
{
    const SvxLanguageItem& rAttr = (const SvxLanguageItem&)rHt;
    SwWW8Writer& rWrtWW8 = (SwWW8Writer&)rWrt;
    if( rWrtWW8.bWrtWW8 )
        rWrtWW8.InsUInt16( 0x486D );    // use sprmCRgLid0 rather than sprmCLid
    else
        rWrtWW8.pO->Insert( 97, rWrtWW8.pO->Count() );
    rWrtWW8.InsUInt16( rAttr.GetLanguage() );
    return rWrt;
}

static Writer& OutWW8_SwEscapement( Writer& rWrt, const SfxPoolItem& rHt )
{
    const SvxEscapementItem& rAttr = (const SvxEscapementItem&)rHt;
    SwWW8Writer& rWrtWW8 = (SwWW8Writer&)rWrt;

    BYTE b = 0xFF;
    short nEsc = rAttr.GetEsc(), nProp = rAttr.GetProp();
    if( !nEsc )
    {
        b = 0;
        nEsc = 0;
        nProp = 100;
    }
    else if( DFLT_ESC_PROP == nProp )
    {
        if( DFLT_ESC_SUB == nEsc || DFLT_ESC_AUTO_SUB == nEsc )
            b = 2;
        else if( DFLT_ESC_SUPER == nEsc || DFLT_ESC_AUTO_SUPER == nEsc )
            b = 1;
    }

    if( 0xFF != b )
    {
        if( rWrtWW8.bWrtWW8 )
            rWrtWW8.InsUInt16( 0x2A48 );
        else
            rWrtWW8.pO->Insert( 104, rWrtWW8.pO->Count() );
        rWrtWW8.pO->Insert( b, rWrtWW8.pO->Count() );
    }

    if( 0 == b || 0xFF == b )
    {
        long nHeight = ((SvxFontHeightItem&)rWrtWW8.GetItem(
                                    RES_CHRATR_FONTSIZE )).GetHeight();
        if( rWrtWW8.bWrtWW8 )
            rWrtWW8.InsUInt16( 0x4845 );
        else
            rWrtWW8.pO->Insert( 101, rWrtWW8.pO->Count() );
        rWrtWW8.InsUInt16( (short)(( nHeight * nEsc + 500 ) / 1000 ));

        if( 100 != nProp || !b )
        {
            if( rWrtWW8.bWrtWW8 )
                rWrtWW8.InsUInt16( 0x4A43 );
            else
                rWrtWW8.pO->Insert( 99, rWrtWW8.pO->Count() );
            rWrtWW8.InsUInt16( ( nHeight * nProp + 500 ) / 1000 );
        }
    }
    return rWrt;
}

static Writer& OutWW8_SwSize( Writer& rWrt, const SfxPoolItem& rHt )
{
    const SvxFontHeightItem& rAttr = (const SvxFontHeightItem&)rHt;
    SwWW8Writer& rWrtWW8 = (SwWW8Writer&)rWrt;
    if( rWrtWW8.bWrtWW8 )
        rWrtWW8.InsUInt16( 0x4A43 );
    else
        rWrtWW8.pO->Insert( 99, rWrtWW8.pO->Count() );
    rWrtWW8.InsUInt16( (UINT16)(( rAttr.GetHeight() + 5 ) / 10 ) );
    return rWrt;
}


// TransCol uebersetzt SW-Farben in WW. Heraus kommt die bei WW fuer
// Text- und Hintergrundfarbe benutzte Codierung.
// Gibt es keine direkte Entsprechung, dann wird versucht, eine moeglichst
// aehnliche WW-Farbe zu finden.
// return: 5-Bit-Wert ( 0..16 )
BYTE SwWW8Writer::TransCol( const Color& rCol )
{
    BYTE nCol = 0;      // ->Auto
    switch( rCol.GetColor() )
    {
    case COL_BLACK:         nCol = 1;   break;
    case COL_BLUE:          nCol = 9;   break;
    case COL_GREEN:         nCol = 11;  break;
    case COL_CYAN:          nCol = 10;  break;
    case COL_RED:           nCol = 13;  break;
    case COL_MAGENTA:       nCol = 12;  break;
    case COL_BROWN:         nCol = 14;  break;
    case COL_GRAY:          nCol = 15;  break;
    case COL_LIGHTGRAY:     nCol = 16;  break;
    case COL_LIGHTBLUE:     nCol = 2;   break;
    case COL_LIGHTGREEN:    nCol = 4;   break;
    case COL_LIGHTCYAN:     nCol = 3;   break;
    case COL_LIGHTRED:      nCol = 6;   break;
    case COL_LIGHTMAGENTA:  nCol = 5;   break;
    case COL_YELLOW:        nCol = 7;   break;
    case COL_WHITE:         nCol = 8;   break;

    default:
        if( !pBmpPal )
        {
            pBmpPal = new BitmapPalette( 16 );
            static const ColorData aColArr[ 16 ] = {
                COL_BLACK,      COL_LIGHTBLUE,  COL_LIGHTCYAN,  COL_LIGHTGREEN,
                COL_LIGHTMAGENTA,COL_LIGHTRED,  COL_YELLOW,     COL_WHITE,
                COL_BLUE,       COL_CYAN,       COL_GREEN,      COL_MAGENTA,
                COL_RED,        COL_BROWN,      COL_GRAY,       COL_LIGHTGRAY
            };

            for( int i = 0; i < 16; ++i )
                pBmpPal->operator[]( i ) = Color( aColArr[ i ] );
        }
        nCol = pBmpPal->GetBestIndex( rCol ) + 1;
        break;
    }
    return nCol;
}

// TransBrush uebersetzt SW-Brushes in WW. Heraus kommt WW8_SHD.
// Nicht-Standardfarben des SW werden noch nicht in die
// Misch-Werte ( 0 .. 95% ) vom WW uebersetzt.
// Return: Echte Brush ( nicht transparent )
// auch bei Transparent wird z.B. fuer Tabellen eine transparente Brush
// geliefert
BOOL SwWW8Writer::TransBrush( const Color& rCol, WW8_SHD& rShd )
{
    if( rCol.GetTransparency() )
        rShd = WW8_SHD();               // alles Nullen : transparent
    else
    {
        rShd.SetFore( 8);
        rShd.SetBack( TransCol( rCol ) );
        rShd.SetStyle( bWrtWW8, 0 );
    }
    return !rCol.GetTransparency();
}

static Writer& OutWW8_SwColor( Writer& rWrt, const SfxPoolItem& rHt )
{
    const SvxColorItem& rAttr = (const SvxColorItem&)rHt;
    SwWW8Writer& rWrtWW8 = (SwWW8Writer&)rWrt;
    if( rWrtWW8.bWrtWW8 )
        rWrtWW8.InsUInt16( 0x2A42 );
    else
        rWrtWW8.pO->Insert( 98, rWrtWW8.pO->Count() );

    rWrtWW8.pO->Insert( rWrtWW8.TransCol( ((const SvxColorItem&)rHt).
                                        GetValue() ), rWrtWW8.pO->Count() );
    return rWrt;
}

static Writer& OutWW8_SwFmtCharBackground( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwWW8Writer& rWW8Wrt = (SwWW8Writer&)rWrt;

    if( rWW8Wrt.bWrtWW8 )   // nur WW8 kann ZeichenHintergrund
    {
        const SvxBrushItem& rBack = (const SvxBrushItem&)rHt;
        WW8_SHD aSHD;

        if( rWW8Wrt.TransBrush( rBack.GetColor(), aSHD ) )
        {
            // sprmCShd
            rWW8Wrt.InsUInt16( 0x4866 );
            rWW8Wrt.InsUInt16( aSHD.GetValue() );
        }
    }
    return rWrt;
}

static Writer& OutSwFmtINetFmt( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwWW8Writer& rWrtWW8 = (SwWW8Writer&)rWrt;
    const SwFmtINetFmt& rINet = (SwFmtINetFmt&)rHt;

    if( rINet.GetValue().Len() )
    {
        USHORT nId;
        const String& rStr = rINet.GetINetFmt();
        if( rStr.Len() )
            nId = rINet.GetINetFmtId();
        else
            nId = RES_POOLCHR_INET_NORMAL;

        const SwCharFmt* pFmt = IsPoolUserFmt( nId )
                        ? rWrt.pDoc->FindCharFmtByName( rStr )
                        : rWrt.pDoc->GetCharFmtFromPool( nId );

        if( rWrtWW8.bWrtWW8 )
            rWrtWW8.InsUInt16( 0x4A30 );
        else
            rWrtWW8.pO->Insert( 80, rWrtWW8.pO->Count() );

        rWrtWW8.InsUInt16( rWrtWW8.GetId( *pFmt ) );
    }
    return rWrt;
}


static void InsertSpecialChar( SwWW8Writer& rWrt, BYTE c )
{
    WW8Bytes aItems;
    if( rWrt.pChpIter )
        rWrt.pChpIter->GetItems( aItems );

    if( 0x13 == c )
        rWrt.pChpPlc->AppendFkpEntry( rWrt.Strm().Tell() );
    else
        rWrt.pChpPlc->AppendFkpEntry( rWrt.Strm().Tell(), aItems.Count(),
                                    aItems.GetData() );

    // at end the attributes writes by the Textnode
    if( 0x15 == c )
        aItems.Remove( 0, aItems.Count() );

    rWrt.WriteChar( c );

    // fSpec-Attribut TRUE
    if( rWrt.bWrtWW8 )
        SwWW8Writer::InsUInt16( aItems, 0x855 );
    else
        aItems.Insert( 117, aItems.Count() );
    aItems.Insert( 1, aItems.Count() );

    rWrt.pChpPlc->AppendFkpEntry( rWrt.Strm().Tell(), aItems.Count(),
                                    aItems.GetData() );
}


void SwWW8Writer::OutField( const SwField* pFld, BYTE nFldType,
                            const String& rFldCmd, BYTE nMode )
{
    static BYTE                 aFld13[2] = { 0x13, 0x00 };  // will change
    static BYTE __READONLY_DATA aFld14[2] = { 0x14, 0xff };
    static BYTE __READONLY_DATA aFld15[2] = { 0x15, 0x80 };

    BOOL bUnicode = IsUnicode();
    WW8_WrPlcFld* pFldP;
    switch ( nTxtTyp )
    {
    case TXT_MAINTEXT:  pFldP = pFldMain;       break;
    case TXT_HDFT:      pFldP = pFldHdFt;       break;
    case TXT_FTN:       pFldP = pFldFtn;        break;
    case TXT_EDN:       pFldP = pFldEdn;        break;
    case TXT_TXTBOX:    pFldP = pFldTxtBxs;     break;
    case TXT_HFTXTBOX:  pFldP = pFldHFTxtBxs;   break;

    default:
        ASSERT( !this, "was ist das fuer ein SubDoc-Type?" );
        break;
    }

    if( WRITEFIELD_START & nMode )
    {
        aFld13[1] = nFldType;                           // Typ nachtragen
        pFldP->Append( Fc2Cp( Strm().Tell() ), aFld13 );
        InsertSpecialChar( *this, 0x13 );
    }
    if( WRITEFIELD_CMD_START & nMode )
    {
        if( bUnicode )
            SwWW8Writer::WriteString16( Strm(), rFldCmd, FALSE );
        else
            SwWW8Writer::WriteString8( Strm(), rFldCmd, FALSE,
                                                RTL_TEXTENCODING_MS_1252 );
    }
    if( WRITEFIELD_CMD_END & nMode )
    {
//      Strm() << " \\* FORMATVERBINDEN ";
        pFldP->Append( Fc2Cp( Strm().Tell() ), aFld14 );
        InsertSpecialChar( *this, 0x14 );
    }
    if( WRITEFIELD_END & nMode )
    {
        String sOut;
        if( pFld )
            sOut = pFld->Expand();
        else
            sOut = rFldCmd;
        if( sOut.Len() )
        {
            if( bUnicode )
                SwWW8Writer::WriteString16( Strm(), sOut, FALSE );
            else
                SwWW8Writer::WriteString8( Strm(), sOut, FALSE,
                                                RTL_TEXTENCODING_MS_1252 );
        }
    }
    if( WRITEFIELD_CLOSE & nMode )
    {
        pFldP->Append( Fc2Cp( Strm().Tell() ), aFld15 );
        InsertSpecialChar( *this, 0x15 );
    }
}

void SwWW8Writer::StartCommentOutput(const String& rName)
{
    String aStr( CREA_ASC( " ANGEBEN [" )); ( aStr += rName ).APP_ASC( "] " );
    OutField( 0, 35, aStr, WRITEFIELD_START | WRITEFIELD_CMD_START );
}

void SwWW8Writer::EndCommentOutput(const String& rName)
{
    String aStr( CREA_ASC( " [" )); ( aStr += rName ).APP_ASC(  "] " );
    OutField( 0, 35, aStr, WRITEFIELD_CMD_END | WRITEFIELD_END |
                            WRITEFIELD_CLOSE );
}


USHORT SwWW8Writer::GetId( const SwTOXType& rTOXType )
{
    void* p = (void*)&rTOXType;
    USHORT nRet = aTOXArr.GetPos( p );
    if( USHRT_MAX == nRet )
        aTOXArr.Insert( p, nRet = aTOXArr.Count() );
    return nRet;
}

// return values:  1 - no PageNum,
//                 2 - TabStop before PageNum,
//                 3 - Text before PageNum - rTxt hold the text
//                 4 - no Text and no TabStop before PageNum
int lcl_CheckForm( const SwForm& rForm, BYTE nLvl, String& rText )
{
    int nRet = 4;
    rText.Erase();

    SwFormTokenEnumerator aIter( rForm.CreateTokenEnumerator( nLvl ));
    BOOL bPgNumFnd = FALSE;
    FormTokenType eTType;
    while( TOKEN_END != ( eTType = aIter.GetNextTokenType() ) && !bPgNumFnd )
    {
        switch( eTType )
        {
        case TOKEN_PAGE_NUMS:
            bPgNumFnd = TRUE;
            break;

        case TOKEN_TAB_STOP:
            nRet = 2;
            break;
        case TOKEN_TEXT:
            nRet = 3;
            rText = aIter.GetCurToken().sText.Copy( 0, 5 );
            break;

        case TOKEN_LINK_START:
        case TOKEN_LINK_END:
            break;

        default:
            nRet = 4;
            break;
        }
    }

    if( !bPgNumFnd )
        nRet = 1;

    return nRet;
}


void SwWW8Writer::StartTOX( const SwSection& rSect )
{
    const SwTOXBase* pTOX = rSect.GetTOXBase();
    if( pTOX )
    {
        static sal_Char __READONLY_DATA sContent[] = " VERZEICHNIS \\w \\x ";
        static sal_Char __READONLY_DATA sIndex[] = " INDEX ";
        static sal_Char __READONLY_DATA sEntryEnd[] = "\" ";


        BYTE nCode;
        String aStr;
        switch( pTOX->GetType() )
        {
        case TOX_INDEX:
            {
                nCode = 8;
                aStr.AssignAscii( sIndex );

                if( pTOX->GetTOXForm().IsCommaSeparated() )
                    aStr.APP_ASC( "\\r " );

                if( TOI_ALPHA_DELIMITTER & pTOX->GetOptions() )
                    aStr.APP_ASC( "\\h \"A\" " );

                {
                    String aFillTxt;
                    for( BYTE n = 1; n <= 3; ++n )
                    {
                        String aTxt;
                        int nRet = ::lcl_CheckForm( pTOX->GetTOXForm(),
                                                    n, aTxt );

                        if( 3 == nRet )
                            aFillTxt = aTxt;
                        else if( 4 == nRet )
                            aFillTxt = '\t';
                        else
                            aFillTxt.Erase();
                    }
                    (aStr.APP_ASC( "\\e \"" )) += aFillTxt;
                    aStr.AppendAscii( sEntryEnd );
                }
            }
            break;

//      case TOX_AUTHORITIES:   nCode = 73; aStr = ???; break;

        case TOX_ILLUSTRATIONS:
        case TOX_OBJECTS:
        case TOX_TABLES:
            if( !pTOX->IsFromObjectNames() )
            {
                nCode = 13;
                aStr.AssignAscii( sContent );

                (( aStr.APP_ASC( "\\c \"" )) += pTOX->GetSequenceName()
                                ).AppendAscii( sEntryEnd );
                String aTxt;
                int nRet = ::lcl_CheckForm( pTOX->GetTOXForm(), 1, aTxt );
                if( 1 == nRet )
                    aStr.APP_ASC( "\\n " );
                else if( 3 == nRet || 4 == nRet )
                    (( aStr.APP_ASC( "\\p \"" )) += aTxt ).AppendAscii(sEntryEnd);
            }
            break;

//      case TOX_USER:
//      case TOX_CONTENT:
        default:
            {
                nCode = 13;
                aStr.AssignAscii( sContent );

                String sTOption;
                USHORT n, nTOXLvl = pTOX->GetLevel();
                if( !nTOXLvl )
                    ++nTOXLvl;

                if( TOX_MARK & pTOX->GetCreateType() )
                {
                    aStr.APP_ASC( "\\f " );

                    if( TOX_USER == pTOX->GetType() )
                        (( aStr += '\"' ) +=
                                (sal_Char)( 'A' + GetId( *pTOX->GetTOXType() ))
                                    ).AppendAscii( sEntryEnd );
                }

                if( TOX_OUTLINELEVEL & pTOX->GetCreateType() )
                {
                    // are the chapter template the normal headline 1 to 9 ?
                    // search all outlined collections
                    BYTE nLvl, nMinLvl = MAXLEVEL;
                    const SwTxtFmtColls& rColls = *pDoc->GetTxtFmtColls();
                    const SwTxtFmtColl* pColl;
                    for( n = rColls.Count(); n; )
                        if( MAXLEVEL > (nLvl = ( pColl =
                                    rColls[ --n ] )->GetOutlineLevel() ) &&
                              nMinLvl > nLvl &&
                            ( RES_POOLCOLL_HEADLINE1 > pColl->GetPoolFmtId() ||
                              RES_POOLCOLL_HEADLINE9 < pColl->GetPoolFmtId() ))
                        {
                            nMinLvl = nLvl;
                        }

                    nLvl = nMinLvl < nTOXLvl ? nMinLvl : (BYTE)nTOXLvl;
                    if( nLvl )
                    {
                        USHORT nTmpLvl = nLvl + 1;
                        if( nTmpLvl > nWW8MaxListLevel )
                            nTmpLvl = nWW8MaxListLevel;

                        ((aStr.APP_ASC( "\\o \"1-" )) +=
                            String::CreateFromInt32( nTmpLvl ))
                                    .AppendAscii(sEntryEnd);
                    }

                    if( nLvl != nMinLvl )
                    {
                        // collect this templates into the \t otion
                        for( n = rColls.Count(); n; )
                            if( MAXLEVEL > (nLvl = ( pColl =
                                    rColls[ --n ] )->GetOutlineLevel() ) &&
                                  nMinLvl <= nLvl )
                            {
                                if( sTOption.Len() )
                                    sTOption += ';';
                                (( sTOption += pColl->GetName() ) += ';' )
                                        += String::CreateFromInt32( nLvl + 1 );
                            }
                    }
                }

                if( TOX_TEMPLATE & pTOX->GetCreateType() )
                    for( n = 0; n < nTOXLvl; ++n )
                    {
                        const String& rStyles = pTOX->GetStyleNames( n );
                        if( rStyles.Len() )
                        {
                            xub_StrLen nPos = 0;
                            String sLvl( ';' );
                            sLvl += String::CreateFromInt32( n + 1 );
                            do {
                                String sStyle( rStyles.GetToken( 0,
                                            TOX_STYLE_DELIMITER, nPos ));
                                if( sStyle.Len() )
                                {
                                    if( sTOption.Len() )
                                        sTOption += ';';
                                    ( sTOption += sStyle ) += sLvl;
                                }
                            } while( STRING_NOTFOUND != nPos );
                        }
                    }

                {
                    String aFillTxt;
                    BYTE nNoPgStt = MAXLEVEL, nNoPgEnd = MAXLEVEL;
                    BOOL bFirstFillTxt = TRUE, bOnlyText = TRUE;
                    for( n = 0; n < nTOXLvl; ++n )
                    {
                        String aTxt;
                        int nRet = ::lcl_CheckForm( pTOX->GetTOXForm(),
                                                    n+1, aTxt );
                        if( 1 == nRet )
                        {
                            bOnlyText = FALSE;
                            if( MAXLEVEL == nNoPgStt )
                                nNoPgStt = n+1;
                        }
                        else
                        {
                            if( MAXLEVEL != nNoPgStt &&
                                MAXLEVEL == nNoPgEnd )
                                nNoPgEnd = BYTE(n);

                            bOnlyText = bOnlyText && 3 == nRet;
                            if( 3 == nRet || 4 == nRet )
                            {
                                if( bFirstFillTxt )
                                    aFillTxt = aTxt;
                                else if( aFillTxt != aTxt )
                                    aFillTxt.Erase();
                                bFirstFillTxt = FALSE;
                            }
                        }
                    }
                    if( MAXLEVEL != nNoPgStt )
                    {
                        if( nWW8MaxListLevel < nNoPgEnd )
                            nNoPgEnd = nWW8MaxListLevel;
                        aStr.APP_ASC( "\\n " );
                        (aStr += String::CreateFromInt32( nNoPgStt )) += '-';
                        (aStr += String::CreateFromInt32( nNoPgEnd  )) += ' ';
                    }
                    if( bOnlyText )
                        ((aStr.APP_ASC( "\\p \"" )) += aFillTxt )
                                .AppendAscii(sEntryEnd);
                }

                if( sTOption.Len() )
                    (( aStr.APP_ASC( "\\t \"" )) += sTOption )
                        .AppendAscii(sEntryEnd);
            }
            break;
        }

        if( aStr.Len() )
        {
            bInWriteTOX = TRUE;
            OutField( 0, nCode, aStr, WRITEFIELD_START |
                                WRITEFIELD_CMD_START | WRITEFIELD_CMD_END );
        }
    }
    bStartTOX = FALSE;
}

void SwWW8Writer::EndTOX( const SwSection& rSect )
{
    const SwTOXBase* pTOX = rSect.GetTOXBase();
    if( pTOX )
    {
        BYTE nCode = TOX_INDEX == pTOX->GetType() ? 8 : 13;
        OutField( 0, nCode, aEmptyStr, WRITEFIELD_CLOSE );
    }
    bInWriteTOX = FALSE;
}



// GetDatePara, GetTimePara, GetNumberPara modifizieren die String-Ref
// Es werden die deutschen Format-Spezifier benutzt, da im FIB auch dt. als
// Creator angegeben ist.
BOOL SwWW8Writer::GetNumberFmt( const SwField& rFld, String& rStr )
{
    BOOL bHasFmt = FALSE;
    SvNumberFormatter* pNFmtr = pDoc->GetNumberFormatter();
    UINT32 nFmtIdx = rFld.GetFormat();
    const SvNumberformat* pNumFmt = pNFmtr->GetEntry( nFmtIdx );
    if( pNumFmt )
    {
        USHORT nLng = rFld.GetLanguage();
        LocaleDataWrapper aLocDat( pNFmtr->GetServiceManager(),
            SvNumberFormatter::ConvertLanguageToLocale( nLng ) );
        if( !pKeyMap )
        {
            pKeyMap = new NfKeywordTable;
            NfKeywordTable& rKeyMap = *(NfKeywordTable*)pKeyMap;
//          aKeyMap[ NF_KEY_NONE = 0,
//          aKeyMap[ NF_KEY_E,
//          aKeyMap[ NF_KEY_AMPM,
//          aKeyMap[ NF_KEY_AP,
            rKeyMap[ NF_KEY_MI      ].ASIGN_ASC( "m" );
            rKeyMap[ NF_KEY_MMI     ].ASIGN_ASC( "mm" );
            rKeyMap[ NF_KEY_M       ].ASIGN_ASC( "M" );
            rKeyMap[ NF_KEY_MM      ].ASIGN_ASC( "MM" );
            rKeyMap[ NF_KEY_MMM     ].ASIGN_ASC( "MMM" );
            rKeyMap[ NF_KEY_MMMM    ].ASIGN_ASC( "MMMM" );
            rKeyMap[ NF_KEY_H       ].ASIGN_ASC( "H" );
            rKeyMap[ NF_KEY_HH      ].ASIGN_ASC( "HH" );
            rKeyMap[ NF_KEY_S       ].ASIGN_ASC( "s" );
            rKeyMap[ NF_KEY_SS      ].ASIGN_ASC( "ss" );
//          aKeyMap[ NF_KEY_Q,
//          aKeyMap[ NF_KEY_QQ,
            rKeyMap[ NF_KEY_T       ].ASIGN_ASC( "t" );
            rKeyMap[ NF_KEY_TT      ].ASIGN_ASC( "tt" );
            rKeyMap[ NF_KEY_TTT     ].ASIGN_ASC( "ttt" );
            rKeyMap[ NF_KEY_TTTT    ].ASIGN_ASC( "tttt" );
            rKeyMap[ NF_KEY_JJ      ].ASIGN_ASC( "jj" );
            rKeyMap[ NF_KEY_JJJJ    ].ASIGN_ASC( "jjjj" );
            rKeyMap[ NF_KEY_NN      ].ASIGN_ASC( "ttt" );
            rKeyMap[ NF_KEY_NNNN    ].ASIGN_ASC( "tttt" );
//          aKeyMap[ NF_KEY_CCC,
//          aKeyMap[ NF_KEY_GENERAL,
//          aKeyMap[ NF_KEY_NNN,
//          aKeyMap[ NF_KEY_WW,
//          aKeyMap[ NF_KEY_QUARTER,
//          aKeyMap[ NF_KEY_TRUE,
//          aKeyMap[ NF_KEY_FALSE,
//          aKeyMap[ NF_KEY_BOOLEAN,
        }

        String sFmt( pNumFmt->GetMappedFormatstring(
                        *(NfKeywordTable*)pKeyMap, aLocDat, TRUE ));
        if( sFmt.Len() )
        {
            (( rStr.APP_ASC( "\\@\"" )) += sFmt ).APP_ASC( "\" " );
            bHasFmt = TRUE;
        }
    }
    return bHasFmt;
}

void WW8_GetNumberPara( String& rStr, const SwField& rFld )
{
    switch( rFld.GetFormat() )
    {
    case SVX_NUM_CHARS_UPPER_LETTER:
    case SVX_NUM_CHARS_UPPER_LETTER_N: rStr.APP_ASC( "\\*ALPHABETISCH "); break;
    case SVX_NUM_CHARS_LOWER_LETTER:
    case SVX_NUM_CHARS_LOWER_LETTER_N: rStr.APP_ASC("\\*alphabetisch "); break;
    case SVX_NUM_ROMAN_UPPER:        rStr.APP_ASC("\\*R\xd6MISCH "); break;
    case SVX_NUM_ROMAN_LOWER:        rStr.APP_ASC("\\*r\xf6misch "); break;
    case SVX_NUM_ARABIC:             rStr.APP_ASC("\\*Arabisch "); break;
    }
}

void SwWW8Writer::WritePostItBegin( const SwPostItField& rPostIt,
                                    WW8Bytes* pO )
{
    BYTE aArr[ 3 ];
    BYTE* pArr = aArr;

    // sprmCFSpec TRUE
    if( bWrtWW8 )
        Set_UInt16( pArr, 0x855 );
    else
        Set_UInt8( pArr, 117 );
    Set_UInt8( pArr, 1 );

    pChpPlc->AppendFkpEntry( Strm().Tell() );
    WriteChar( 0x05 );              // Annotation reference

    if( pO )
        pO->Insert( aArr, pArr - aArr, pO->Count() );
    else
        pChpPlc->AppendFkpEntry( Strm().Tell(), pArr - aArr, aArr );
}

static Writer& OutWW8_SwField( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwWW8Writer& rWW8Wrt = (SwWW8Writer&)rWrt;
    const SwFmtFld& rFld = (SwFmtFld&)rHt;
    const SwField* pFld = rFld.GetFld();
    String aStr;        // fuer optionale Parameter
    BOOL bWriteExpand = FALSE;
    USHORT nSubType = pFld->GetSubType();
    BYTE nFldTyp = 0;

    switch( pFld->GetTyp()->Which() )
    {
//  case RES_CHAPTERFLD:
//      break;

    case RES_SETEXPFLD:
        if( GSE_SEQ == nSubType )
        {
            (( aStr.ASIGN_ASC( " SEQ \"" )) += pFld->GetTyp()->GetName() )
                    .APP_ASC( "\" " );
            ::WW8_GetNumberPara( aStr, *pFld );
            rWW8Wrt.OutField( pFld, 12, aStr );
        }
        else
            bWriteExpand = TRUE;
        break;

    case RES_PAGENUMBERFLD:
        aStr.ASIGN_ASC( " SEITE " );
        ::WW8_GetNumberPara( aStr, *pFld );
        rWW8Wrt.OutField( pFld, 33, aStr );
        break;

    case RES_FILENAMEFLD:
        rWW8Wrt.OutField( pFld, 29, CREA_ASC( "DATEINAME" ));
        break;

    case RES_DBNAMEFLD:
        aStr.ASIGN_ASC( "DATENBANK " );         // ok ??
        aStr += rWrt.pDoc->GetDBName();
        rWW8Wrt.OutField( pFld, 78, aStr );
        break;

    case RES_AUTHORFLD:
        rWW8Wrt.OutField( pFld, (AF_SHORTCUT & nSubType ? 61 : 60),
                            CREA_ASC( "AUTOR" ));
        break;

    case RES_TEMPLNAMEFLD:
        rWW8Wrt.OutField( pFld, 30, CREA_ASC("DOKVORLAGE" ));
        break;

    case RES_DOCINFOFLD:    // Last printed, last edited,...
        if( DI_SUB_FIXED & nSubType )
            bWriteExpand = TRUE;
        else
        {
            static sal_Char __READONLY_DATA sFld15[] = "TITLE";
            static sal_Char __READONLY_DATA sFld16[] = "THEMA";
            static sal_Char __READONLY_DATA sFld17[] = "AUTOR ";
            static sal_Char __READONLY_DATA sFld18[] = "STICHW\xd6RTER";
            static sal_Char __READONLY_DATA sFld19[] = "KOMMENTAR";
            static sal_Char __READONLY_DATA sFld20[] = "GESPEICHERTVON";
            static sal_Char __READONLY_DATA sFld21[] = "ERSTELLDAT ";
            static sal_Char __READONLY_DATA sFld22[] = "SPEICHERDAT ";
            static sal_Char __READONLY_DATA sFld23[] = "DRUCKDAT ";
            static sal_Char __READONLY_DATA sFld24[] = "\xdc" "BERARBEITUNGSNUMMER";

            static const sal_Char* __READONLY_DATA aFldArr[] = {
                    sFld15, sFld16, sFld17, sFld18, sFld19,
                    sFld20, sFld21, sFld22, sFld23, sFld24
            };

            switch( 0xff & nSubType )
            {
            case DI_TITEL:      nFldTyp = 15;   break;
            case DI_THEMA:      nFldTyp = 16;   break;
            case DI_KEYS:       nFldTyp = 18;   break;
            case DI_COMMENT:    nFldTyp = 19;   break;
            case DI_DOCNO:      nFldTyp = 24;   break;

            case DI_CREATE:
                if( DI_SUB_AUTHOR == (nSubType & ~DI_SUB_AUTHOR ))
                    nFldTyp = 17;
                else if( rWW8Wrt.GetNumberFmt( *pFld, aStr ))
                    nFldTyp = 21;
                break;

            case DI_CHANGE:
                if( DI_SUB_AUTHOR == (nSubType & ~DI_SUB_AUTHOR ))
                    nFldTyp = 20;
                break;

            case DI_PRINT:
                if( DI_SUB_AUTHOR != (nSubType & ~DI_SUB_AUTHOR ) &&
                    rWW8Wrt.GetNumberFmt( *pFld, aStr ))
                    nFldTyp = 23;
                break;
            case DI_EDIT:
                if( DI_SUB_AUTHOR != (nSubType & ~DI_SUB_AUTHOR ) &&
                    rWW8Wrt.GetNumberFmt( *pFld, aStr ))
                    nFldTyp = 22;
                break;
            }

            if( nFldTyp )
            {
                aStr.InsertAscii( aFldArr[ nFldTyp - 15 ], 0 );
                rWW8Wrt.OutField( pFld, nFldTyp, aStr );
            }
            else
                bWriteExpand = TRUE;
        }
        break;

    case RES_DATETIMEFLD:
        if( FIXEDFLD & nSubType || !rWW8Wrt.GetNumberFmt( *pFld, aStr ) )
            bWriteExpand = TRUE;
        else
        {
            const sal_Char* pAddStr;
            USHORT nTyp;
            if( DATEFLD & nSubType )
            {
                pAddStr = " AKTUALDAT ";
                nTyp = 31;
            }
            else
            {
                pAddStr = " ZEIT ";
                nTyp = 32;
            }
            aStr.InsertAscii( pAddStr, 0 );
            rWW8Wrt.OutField( pFld, nTyp, aStr );
        }
        break;

    case RES_DOCSTATFLD:
        {
            switch( nSubType )
            {
            case DS_PAGE:   aStr.ASIGN_ASC(" ANZSEITEN ");      nFldTyp = 26; break;
            case DS_WORD:   aStr.ASIGN_ASC(" ANZW\xd6RTER ");   nFldTyp = 27; break;
            case DS_CHAR:   aStr.ASIGN_ASC(" ANZZEICHEN ");     nFldTyp = 28; break;
            }

            if( nFldTyp )
            {
                ::WW8_GetNumberPara( aStr, *pFld );
                rWW8Wrt.OutField( pFld, nFldTyp, aStr );
            }
            else
                bWriteExpand = TRUE;
        }
        break;
    case RES_EXTUSERFLD:
        {
            switch( 0xFF & nSubType  )
            {
            case EU_FIRSTNAME:
            case EU_NAME:
                nFldTyp = 60;
                aStr.ASIGN_ASC("BENUTZERNAME");
                break;

            case EU_SHORTCUT:
                nFldTyp = 61;
                aStr.ASIGN_ASC("BENUTZERINITIALEN");
                break;

            case EU_STREET:
            case EU_COUNTRY:
            case EU_ZIP:
            case EU_CITY:
                nFldTyp = 62;
                aStr.ASIGN_ASC("BENUTZERADR");
                break;
            }
            if( nFldTyp )
                rWW8Wrt.OutField( pFld, nFldTyp, aStr );
            else
                bWriteExpand = TRUE;
        }
        break;

    case RES_POSTITFLD:
        {
            const SwPostItField& rPFld = *(SwPostItField*)pFld;
            rWW8Wrt.pAtn->Append( rWW8Wrt.Fc2Cp( rWrt.Strm().Tell() ), rPFld );
            rWW8Wrt.WritePostItBegin( rPFld, rWW8Wrt.pO );
        }
        break;

    case RES_INPUTFLD:
        ((aStr.ASIGN_ASC("EINGEBEN \"" )) += pFld->GetPar2() )+= '\"';
        rWW8Wrt.OutField( pFld, 39, aStr );
        break;

    case RES_GETREFFLD:
        {
            aStr.ASIGN_ASC(" REF ");
            const SwGetRefField& rRFld = *(SwGetRefField*)pFld;
            switch( nSubType )
            {
            case REF_SETREFATTR:
            case REF_BOOKMARK:
                aStr += rWW8Wrt.GetBookmarkName( nSubType,
                                                &rRFld.GetSetRefName(), 0 );
                nFldTyp = 3;
                break;

            case REF_FOOTNOTE:
            case REF_ENDNOTE:
                aStr += rWW8Wrt.GetBookmarkName( nSubType, 0,
                                                rRFld.GetSeqNo() );
                nFldTyp = REF_ENDNOTE == nSubType ? 72 : 5;
                switch( pFld->GetFormat() )
                {
                case REF_PAGE_PGDESC:
                case REF_PAGE:
                case REF_UPDOWN:
                    break;
                default:
                    aStr.InsertAscii( "FUSSENDNOTE", 1 );
                    break;
                }
                break;

//          case REF_SEQUENCEFLD:   // ???
//          case REF_OUTLINE:       // 10
            // page reference - 37
            // DDE reference  - 45
            // DDE automatic reference - 46
            }

            if( nFldTyp )
            {
                switch( pFld->GetFormat() )
                {
                case REF_PAGE_PGDESC:
                case REF_PAGE:
                    aStr.InsertAscii( "SEITEN", 1 );
                    nFldTyp = 37;
                    break;

                case REF_UPDOWN:
                    aStr.APP_ASC( " \\p" );
                    nFldTyp = 3;
                    break;

                case REF_CHAPTER:
                case REF_ONLYNUMBER:
                case REF_ONLYCAPTION:
                case REF_ONLYSEQNO:
                    break;
                // default:
                // case REF_CONTENT:
                }

                aStr.APP_ASC( " \\h " );        // insert hyperlink
                rWW8Wrt.OutField( pFld, nFldTyp, aStr );

            }
            else
                bWriteExpand = TRUE;
        }
        break;

    default:
        bWriteExpand = TRUE;
        break;
    }

    if( bWriteExpand )
    {
        if( rWW8Wrt.IsUnicode() )
            SwWW8Writer::WriteString16( rWrt.Strm(), pFld->Expand(), FALSE );
        else
            SwWW8Writer::WriteString8(  rWrt.Strm(), pFld->Expand(), FALSE,
                                                    RTL_TEXTENCODING_MS_1252 );
    }

    return rWrt;
}

static Writer& OutWW8_SwFlyCntnt( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwWW8Writer& rWrtWW8 = (SwWW8Writer&)rWrt;
    if( rWrtWW8.pOutFmtNode && rWrtWW8.pOutFmtNode->ISA( SwCntntNode ))
    {
        SwTxtNode* pTxtNd = (SwTxtNode*)rWrtWW8.pOutFmtNode;

        Point aLayPos;
        aLayPos = pTxtNd->FindLayoutRect( FALSE, &aLayPos ).Pos();

        rWrtWW8.OutWW8FlyFrm( *((SwFmtFlyCnt&)rHt).GetFrmFmt(), aLayPos );
    }
    return rWrt;
}

// TOXMarks fehlen noch

// Detaillierte Einstellungen zur Trennung erlaubt WW nur dokumentenweise.
// Man koennte folgende Mimik einbauen: Die Werte des Style "Standard" werden,
// falls vorhanden, in die Document Properties ( DOP ) gesetzt.
// ---
// ACK.  Dieser Vorschlag passt exakt zu unserer Implementierung des Import,
// daher setze ich das gleich mal um. (KHZ, 07/15/2000)
static Writer& OutWW8_SvxHyphenZone( Writer& rWrt, const SfxPoolItem& rHt )
{
// sprmPFNoAutoHyph
    const SvxHyphenZoneItem& rAttr = (const SvxHyphenZoneItem&)rHt;
    SwWW8Writer& rWrtWW8 = (SwWW8Writer&)rWrt;
    if( rWrtWW8.bWrtWW8 )
        rWrtWW8.InsUInt16( 0x242A );
    else
        rWrtWW8.pO->Insert( 44, rWrtWW8.pO->Count() );

    rWrtWW8.pO->Insert( rAttr.IsHyphen() ? 0 : 1, rWrtWW8.pO->Count() );
    return rWrt;
}

// NoHyphen: ich habe keine Entsprechung in der SW-UI und WW-UI gefunden

static Writer& OutWW8_SwHardBlank( Writer& rWrt, const SfxPoolItem& rHt )
{
    ((SwWW8Writer&)rWrt).WriteChar( ((SwFmtHardBlank&)rHt).GetChar() );
    return rWrt;
}

// RefMark, NoLineBreakHere  fehlen noch

void SwWW8Writer::WriteFtnBegin( const SwFmtFtn& rFtn, WW8Bytes* pOutArr )
{
    WW8Bytes aAttrArr;
    BOOL bAutoNum = !rFtn.GetNumStr().Len();    // Auto-Nummer
    if( bAutoNum )
    {
        if( bWrtWW8 )
        {
            static BYTE __READONLY_DATA aSpec[] = {
                0x03, 0x6a, 0, 0, 0, 0,         // sprmCObjLocation (wahrscheinlich unnoetig)
                0x55, 0x08, 1                   // sprmCFSpec
            };

            aAttrArr.Insert( aSpec, sizeof( aSpec ), aAttrArr.Count() );
        }
        else
        {
            static BYTE __READONLY_DATA aSpec[] = {
                117, 1,                         // sprmCFSpec
                68, 4, 0, 0, 0, 0               // sprmCObjLocation (wahrscheinlich unnoetig)
            };
            aAttrArr.Insert( aSpec, sizeof( aSpec ), aAttrArr.Count() );
        }
    }

        // sprmCIstd
    const SwEndNoteInfo* pInfo;
    if( rFtn.IsEndNote() )
        pInfo = &pDoc->GetEndNoteInfo();
    else
        pInfo = &pDoc->GetFtnInfo();
    const SwCharFmt* pCFmt = pOutArr
                                ? pInfo->GetAnchorCharFmt( *pDoc )
                                : pInfo->GetCharFmt( *pDoc );
    if( bWrtWW8 )
        SwWW8Writer::InsUInt16( aAttrArr, 0x4a30 );
    else
        aAttrArr.Insert( 80, aAttrArr.Count() );
    SwWW8Writer::InsUInt16( aAttrArr, GetId( *pCFmt ) );

                                                // fSpec-Attribut TRUE
                            // Fuer Auto-Nummer muss ein Spezial-Zeichen
                            // in den Text und darum ein fSpec-Attribut
    pChpPlc->AppendFkpEntry( Strm().Tell() );
    if( bAutoNum )
        WriteChar( 0x02 );              // Auto-Nummer-Zeichen
    else
        // User-Nummerierung
        OutSwString( rFtn.GetNumStr(), 0, rFtn.GetNumStr().Len(),
                        IsUnicode(), RTL_TEXTENCODING_MS_1252 );

    if( pOutArr )
    {
        // insert at start of array, so the "hard" attribute overrule the
        // attributes of the character template
        pOutArr->Insert( &aAttrArr, 0 );
    }
    else
    {
        WW8Bytes aOutArr;

        // insert at start of array, so the "hard" attribute overrule the
        // attributes of the character template
        aOutArr.Insert( &aAttrArr, 0 );

        // write for the ftn number in the content, the font of the anchor
        const SwTxtFtn* pTxtFtn = rFtn.GetTxtFtn();
        if( pTxtFtn )
        {
            WW8Bytes* pOld = pO;
            pO = &aOutArr;
            SfxItemSet aSet( pDoc->GetAttrPool(), RES_CHRATR_FONT,
                                                    RES_CHRATR_FONT );

            pCFmt = pInfo->GetCharFmt( *pDoc );
            aSet.Set( pCFmt->GetAttrSet() );

            pTxtFtn->GetTxtNode().GetAttr( aSet, *pTxtFtn->GetStart(),
                                            (*pTxtFtn->GetStart()) + 1 );
            ::OutWW8_SwFont( *this, aSet.Get( RES_CHRATR_FONT ));
            pO = pOld;
        }
        pChpPlc->AppendFkpEntry( Strm().Tell(), aOutArr.Count(),
                                                aOutArr.GetData() );
    }
}

static BOOL lcl_IsAtTxtEnd( const SwFmtFtn& rFtn )
{
    BOOL bRet = TRUE;
    if( rFtn.GetTxtFtn() )
    {
        USHORT nWh = rFtn.IsEndNote() ? RES_END_AT_TXTEND
                                      : RES_FTN_AT_TXTEND;
        const SwSectionNode* pSectNd = rFtn.GetTxtFtn()->GetTxtNode().
                                                        FindSectionNode();
        while( pSectNd && FTNEND_ATPGORDOCEND ==
                ((const SwFmtFtnAtTxtEnd&)pSectNd->GetSection().GetFmt()->
                GetAttr( nWh, TRUE )).GetValue() )
            pSectNd = pSectNd->FindStartNode()->FindSectionNode();

        if( !pSectNd )
            bRet = FALSE;   // the is ftn/end collected at Page- or Doc-End
    }
    return bRet;
}


static Writer& OutWW8_SwFtn( Writer& rWrt, const SfxPoolItem& rHt )
{
    const SwFmtFtn& rFtn = (const SwFmtFtn&)rHt;
    SwWW8Writer& rWW8Wrt = (SwWW8Writer&)rWrt;

    USHORT nTyp;
    WW8_WrPlcFtnEdn* pFtnEnd;
    if( rFtn.IsEndNote() )
    {
        pFtnEnd = rWW8Wrt.pEdn;
        nTyp = REF_ENDNOTE;
        if( rWW8Wrt.bEndAtTxtEnd )
            rWW8Wrt.bEndAtTxtEnd = lcl_IsAtTxtEnd( rFtn );
    }
    else
    {
        pFtnEnd = rWW8Wrt.pFtn;
        nTyp = REF_FOOTNOTE;
        if( rWW8Wrt.bFtnAtTxtEnd )
            rWW8Wrt.bFtnAtTxtEnd = lcl_IsAtTxtEnd( rFtn );
    }

    // if any reference to this footnote/endnote then insert an internal
    // Bookmark.
    String sBkmkNm;
    if( rWW8Wrt.HasRefToObject( nTyp, 0, rFtn.GetTxtFtn()->GetSeqRefNo() ))
    {
        sBkmkNm = rWW8Wrt.GetBookmarkName( nTyp, 0,
                                    rFtn.GetTxtFtn()->GetSeqRefNo() );
        rWW8Wrt.AppendBookmark( sBkmkNm );
    }


    pFtnEnd->Append( rWW8Wrt.Fc2Cp( rWrt.Strm().Tell() ), rFtn );
    rWW8Wrt.WriteFtnBegin( rFtn, rWW8Wrt.pO );

    if( sBkmkNm.Len() )
        rWW8Wrt.AppendBookmark( sBkmkNm );

    return rWrt;
}



static Writer& OutWW8_SwTxtCharFmt( Writer& rWrt, const SfxPoolItem& rHt )
{
    const SwFmtCharFmt& rAttr = (const SwFmtCharFmt&)rHt;
    if( rAttr.GetCharFmt() )
    {
        SwWW8Writer& rWrtWW8 = (SwWW8Writer&)rWrt;
        if( rWrtWW8.bWrtWW8 )
            rWrtWW8.InsUInt16( 0x4A30 );
        else
            rWrtWW8.pO->Insert( 80, rWrtWW8.pO->Count() );

        rWrtWW8.InsUInt16( rWrtWW8.GetId( *rAttr.GetCharFmt() ) );
    }
    return rWrt;
}


static Writer& OutWW8_SwNumRuleItem( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwWW8Writer& rWW8Wrt = (SwWW8Writer&)rWrt;
    const SwNumRuleItem& rNumRule = (const SwNumRuleItem&)rHt;

    const SwTxtNode* pTxtNd = 0;
    USHORT nNumId;
    BYTE nLvl = 0;
    if( rNumRule.GetValue().Len() )
    {
        const SwNumRule* pRule = rWrt.pDoc->FindNumRulePtr(
                                        rNumRule.GetValue() );
        if( pRule && USHRT_MAX != ( nNumId = rWW8Wrt.GetId( *pRule )) )
        {
            ++nNumId;
            if( rWW8Wrt.pOutFmtNode )
            {
                if( rWW8Wrt.pOutFmtNode->ISA( SwCntntNode ))
                {
                    pTxtNd = (SwTxtNode*)rWW8Wrt.pOutFmtNode;
                    const SwNodeNum* pNum = pTxtNd->GetNum();

                    if( pNum && NO_NUM > pNum->GetLevel() )
                        nLvl = GetRealLevel( pNum->GetLevel() );
                }
                else if( rWW8Wrt.pOutFmtNode->ISA( SwTxtFmtColl ))
                {
                    const SwTxtFmtColl* pC = (SwTxtFmtColl*)rWW8Wrt.pOutFmtNode;
                    if( pC && MAXLEVEL > pC->GetOutlineLevel() )
                        nLvl = pC->GetOutlineLevel();
                }
            }
        }
        else
            nNumId = USHRT_MAX;
    }
    else
        nNumId = 0;

    if( USHRT_MAX != nNumId )
    {
        if( nLvl >= nWW8MaxListLevel )
            nLvl = nWW8MaxListLevel-1;
        if( rWW8Wrt.bWrtWW8 )
        {
            // write sprmPIlvl and sprmPIlfo
            SwWW8Writer::InsUInt16( *rWW8Wrt.pO, 0x260a );
            rWW8Wrt.pO->Insert( nLvl, rWW8Wrt.pO->Count() );
            SwWW8Writer::InsUInt16( *rWW8Wrt.pO, 0x460b );
            SwWW8Writer::InsUInt16( *rWW8Wrt.pO, nNumId );
        }
        else if( pTxtNd && rWW8Wrt.Out_SwNum( pTxtNd ) )                // NumRules
            rWW8Wrt.pSepx->SetNum( pTxtNd );
    }
    return rWrt;
}

/* File FRMATR.HXX */

static Writer& OutWW8_SwFrmSize( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwWW8Writer& rWW8Wrt = (SwWW8Writer&)rWrt;
    const SwFmtFrmSize& rSz = (const SwFmtFrmSize&)rHt;

    if( rWW8Wrt.bOutFlyFrmAttrs )                   // Flys
    {
        if( rWW8Wrt.bOutGrf )
            return rWrt;                // Fly um Grafik -> Auto-Groesse

//???? was ist bei Prozentangaben ???
        if( rSz.GetWidth() )
        {
            //"sprmPDxaWidth"
            if( rWW8Wrt.bWrtWW8 )
                rWW8Wrt.InsUInt16( 0x841A );
            else
                rWW8Wrt.pO->Insert( 28, rWW8Wrt.pO->Count() );
            rWW8Wrt.InsUInt16( (USHORT)rSz.GetWidth() );
        }

        if( rSz.GetHeight() )
        {
            // sprmPWHeightAbs
            if( rWW8Wrt.bWrtWW8 )
                rWW8Wrt.InsUInt16( 0x442B );
            else
                rWW8Wrt.pO->Insert( 45, rWW8Wrt.pO->Count() );

            USHORT nH = 0;
            switch( rSz.GetSizeType() )
            {
            case ATT_VAR_SIZE: break;
            case ATT_FIX_SIZE: nH = (USHORT)rSz.GetHeight() & 0x7fff; break;
            default:           nH = (USHORT)rSz.GetHeight() | 0x8000; break;
            }
            rWW8Wrt.InsUInt16( nH );
        }
    }
    else if( rWW8Wrt.bOutPageDescs )            // PageDesc : Breite + Hoehe
    {
        if( rWW8Wrt.pAktPageDesc->GetLandscape() )
        {
            /*sprmSBOrientation*/
            if( rWW8Wrt.bWrtWW8 )
                rWW8Wrt.InsUInt16( 0x301d );
            else
                rWW8Wrt.pO->Insert( 162, rWW8Wrt.pO->Count() );
            rWW8Wrt.pO->Insert( 2, rWW8Wrt.pO->Count() );
        }

        /*sprmSXaPage*/
        if( rWW8Wrt.bWrtWW8 )
            rWW8Wrt.InsUInt16( 0xB01F );
        else
            rWW8Wrt.pO->Insert( 164, rWW8Wrt.pO->Count() );
        rWW8Wrt.InsUInt16( (USHORT)rSz.GetWidth() );

        /*sprmSYaPage*/
        if( rWW8Wrt.bWrtWW8 )
            rWW8Wrt.InsUInt16( 0xB020 );
        else
            rWW8Wrt.pO->Insert( 165, rWW8Wrt.pO->Count() );
        rWW8Wrt.InsUInt16( (USHORT)rSz.GetHeight() );
    }
    return rWrt;
}

// FillOrder fehlt noch

// ReplaceCr() wird fuer Pagebreaks und Pagedescs gebraucht. Es wird ein
// bereits geschriebenes CR durch ein Break-Zeichen ersetzt. Replace muss
// direkt nach Schreiben des CR gerufen werden.
// Rueckgabe: FilePos des ersetzten CRs + 1 oder 0 fuer nicht ersetzt

ULONG SwWW8Writer::ReplaceCr( BYTE nChar )
{
    // Bug #49917#
    ASSERT( nChar, "gegen 0 ersetzt bringt WW97/95 zum Absturz" );

    ULONG nRetPos = 0, nPos = Strm().Tell();
    Strm().SeekRel( IsUnicode() ? -2 : -1 );
    BYTE nC1;
    Strm().Read( &nC1, 1 );
    if( nC1 == 0x0d )               // CR ?
    {
        Strm().SeekRel( -1 );       // CR hinter Zeile
        Strm().Write( &nChar, 1 );  // durch nChar ersetzen
        nRetPos = nPos;
    }
    Strm().Seek( nPos );

    if( 0x7 == nC1 )            // at end of TableRow ?
    {
        // then write as normal char
        WriteChar( nChar );
        nRetPos = Strm().Tell();
    }
#ifdef PRODUCT
    else
    {
        ASSERT( nRetPos || nPos == (ULONG)pFib->fcMin,
                "WW8_ReplaceCr an falscher FilePos gerufen" );
    }
#endif
    return nRetPos;
}

// Breaks schreiben nichts in das Ausgabe-Feld rWrt.pO,
// sondern nur in den Text-Stream ( Bedingung dafuer, dass sie von Out_Break...
// gerufen werden duerfen )
static Writer& OutWW8_SwFmtBreak( Writer& rWrt, const SfxPoolItem& rHt )
{
/*UMSTELLEN*/
    SwWW8Writer& rWW8Wrt = (SwWW8Writer&)rWrt;
    const SvxFmtBreakItem &rBreak = (const SvxFmtBreakItem&)rHt;

    if( rWW8Wrt.bStyDef )
    {
        switch( rBreak.GetBreak() )
        {
//JP 21.06.99: column breaks does never change to pagebreaks
//      case SVX_BREAK_COLUMN_BEFORE:
//      case SVX_BREAK_COLUMN_BOTH:

        case SVX_BREAK_PAGE_BEFORE:
        case SVX_BREAK_PAGE_BOTH:
            // sprmPPageBreakBefore/sprmPFPageBreakBefore
            if( rWW8Wrt.bWrtWW8 )
                rWW8Wrt.InsUInt16( 0x2407 );
            else
                rWW8Wrt.pO->Insert( 9, rWW8Wrt.pO->Count() );
            rWW8Wrt.pO->Insert( rBreak.GetValue() ? 1 : 0, rWW8Wrt.pO->Count() );
            break;
        }
    }
    else if( !rWW8Wrt.pFlyFmt )
    {
        BYTE nC = 0;
        BOOL bBefore = FALSE;

        switch( rBreak.GetBreak() )
        {
        case SVX_BREAK_NONE:                                // Ausgeschaltet
            if( !rWW8Wrt.bBreakBefore )
            {
                // sprmPPageBreakBefore/sprmPFPageBreakBefore
                if( rWW8Wrt.bWrtWW8 )
                    rWW8Wrt.InsUInt16( 0x2407 );
                else
                    rWW8Wrt.pO->Insert( 9, rWW8Wrt.pO->Count() );
                rWW8Wrt.pO->Insert( (BYTE)0, rWW8Wrt.pO->Count() );
            }
            return rWrt;

        case SVX_BREAK_COLUMN_BEFORE:                       // ColumnBreak
            bBefore = TRUE;
                // no break;
        case SVX_BREAK_COLUMN_AFTER:
        case SVX_BREAK_COLUMN_BOTH:
            nC = 0xe;
            break;

        case SVX_BREAK_PAGE_BEFORE:                         // PageBreak
            bBefore = TRUE;
            // no break;
        case SVX_BREAK_PAGE_AFTER:
        case SVX_BREAK_PAGE_BOTH:
            nC = 0xc;
            break;
        }

        if( (bBefore == rWW8Wrt.bBreakBefore ) && nC )  // #49917#
            rWW8Wrt.ReplaceCr( nC );
    }
    return rWrt;
}

static Writer& OutWW8_SvxPaperBin( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwWW8Writer& rWrtWW8 = (SwWW8Writer&)rWrt;
    const SvxPaperBinItem& rItem = (const SvxPaperBinItem&)rHt;

    if( rWrtWW8.bOutPageDescs )
    {
        USHORT nVal;
        switch( rItem.GetValue() )
        {
        case 0: nVal = 15;  break;      // Automatically select
        case 1: nVal = 1;   break;      // Upper paper tray
        case 2: nVal = 4;   break;      // Manual paper feed
        default: nVal = 0;  break;
        }

        if( nVal )
        {
            // sprmSDmBinFirst  0x5007  word
            // sprmSDmBinOther  0x5008  word
            BYTE nOff = rWrtWW8.bOutFirstPage ? 0 : 1;
            if( rWrtWW8.bWrtWW8 )
                rWrtWW8.InsUInt16( 0x5007 + nOff );
            else
                rWrtWW8.pO->Insert( 140 + nOff, rWrtWW8.pO->Count() );
            rWrtWW8.InsUInt16( nVal );
        }
    }
    return rWrt;
}

static Writer& OutWW8_SwFmtLRSpace( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwWW8Writer& rWW8Wrt = (SwWW8Writer&)rWrt;
    const SvxLRSpaceItem & rLR = (const SvxLRSpaceItem&) rHt;


    // Flys fehlen noch ( siehe RTF )

    if( rWW8Wrt.bOutFlyFrmAttrs )                   // Flys
    {
        // sprmPDxaFromText10
        if( rWW8Wrt.bWrtWW8 )
            rWW8Wrt.InsUInt16( 0x4622 );
        else
            rWW8Wrt.pO->Insert( 49, rWW8Wrt.pO->Count() );
        // Mittelwert nehmen, da WW nur 1 Wert kennt
        rWW8Wrt.InsUInt16( (USHORT) ( ( rLR.GetLeft() + rLR.GetRight() ) / 2 ) );
    }
    else if( rWW8Wrt.bOutPageDescs )                // PageDescs
    {
        USHORT nLDist, nRDist;
        const SfxPoolItem* pItem = ((SwWW8Writer&)rWrt).HasItem( RES_BOX );
        if( pItem )
        {
            nRDist = ((SvxBoxItem*)pItem)->CalcLineSpace( BOX_LINE_LEFT );
            nLDist = ((SvxBoxItem*)pItem)->CalcLineSpace( BOX_LINE_RIGHT );
        }
        else
            nLDist = nRDist = 0;
        nLDist += rLR.GetLeft();
        nRDist += rLR.GetRight();

        // sprmSDxaLeft
        if( rWW8Wrt.bWrtWW8 )
            rWW8Wrt.InsUInt16( 0xB021 );
        else
            rWW8Wrt.pO->Insert( 166, rWW8Wrt.pO->Count() );
        rWW8Wrt.InsUInt16( nLDist );
        // sprmSDxaRight
        if( rWW8Wrt.bWrtWW8 )
            rWW8Wrt.InsUInt16( 0xB022 );
        else
            rWW8Wrt.pO->Insert( 167, rWW8Wrt.pO->Count() );
        rWW8Wrt.InsUInt16( nRDist );
    }
    else
    {                                          // normale Absaetze
        // sprmPDxaLeft
        if( rWW8Wrt.bWrtWW8 )
            rWW8Wrt.InsUInt16( 0x840F );
        else
            rWW8Wrt.pO->Insert( 17, rWW8Wrt.pO->Count() );
        rWW8Wrt.InsUInt16( rLR.GetTxtLeft() );
        // sprmPDxaRight
        if( rWW8Wrt.bWrtWW8 )
            rWW8Wrt.InsUInt16( 0x840E );
        else
            rWW8Wrt.pO->Insert( 16, rWW8Wrt.pO->Count() );
        rWW8Wrt.InsUInt16( rLR.GetRight() );
        // sprmPDxaLeft1
        if( rWW8Wrt.bWrtWW8 )
            rWW8Wrt.InsUInt16( 0x8411 );
        else
            rWW8Wrt.pO->Insert( 19, rWW8Wrt.pO->Count() );
        rWW8Wrt.InsUInt16( rLR.GetTxtFirstLineOfst() );
    }
    return rWrt;
}

static USHORT lcl_CalcHdFtDist( const SwFrmFmt& rFmt, BOOL bLower )
{
    long nDist = 0;
    SwRect aRect( rFmt.FindLayoutRect( FALSE ));
    if( aRect.Height() )
        nDist += aRect.Height();
    else
    {
        const SwFmtFrmSize& rSz = rFmt.GetFrmSize();
        if( ATT_VAR_SIZE != rSz.GetSizeType() )
            nDist += rSz.GetHeight();
        else
        {
            nDist += 274;       // defaulten fuer 12pt Schrift
            const SvxULSpaceItem& rUL = rFmt.GetULSpace();
            nDist += ( bLower ? rUL.GetLower() : rUL.GetUpper() );
        }
    }
    return (USHORT)nDist;
}

static Writer& OutWW8_SwFmtULSpace( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwWW8Writer& rWW8Wrt = (SwWW8Writer&)rWrt;
    const SvxULSpaceItem & rUL = (const SvxULSpaceItem&) rHt;

    // Flys fehlen noch ( siehe RTF )

    if( rWW8Wrt.bOutFlyFrmAttrs )                   // Flys
    {
        // sprmPDyaFromText
        if( rWW8Wrt.bWrtWW8 )
            rWW8Wrt.InsUInt16( 0x842E );
        else
            rWW8Wrt.pO->Insert( 48, rWW8Wrt.pO->Count() );
        // Mittelwert nehmen, da WW nur 1 Wert kennt
        rWW8Wrt.InsUInt16( (USHORT) ( ( rUL.GetUpper() + rUL.GetLower() ) / 2 ) );
    }
    else if( rWW8Wrt.bOutPageDescs )            // Page-UL
    {
        USHORT nUDist, nLDist;
        const SfxPoolItem* pItem = rWW8Wrt.HasItem( RES_BOX );
        if( pItem )
        {
            nUDist = ((SvxBoxItem*)pItem)->CalcLineSpace( BOX_LINE_TOP );
            nLDist = ((SvxBoxItem*)pItem)->CalcLineSpace( BOX_LINE_BOTTOM );
        }
        else
            nUDist = nLDist = 0;
        nUDist += rUL.GetUpper();
        nLDist += rUL.GetLower();

        // Kopf-/Fusszeilen beachten:
        if( 0 != ( pItem = rWW8Wrt.HasItem( RES_HEADER )) &&
            ((SwFmtHeader*)pItem)->IsActive() )
        {
            // dann aufjedenfall schon mal den Abstand der Kopfzeile zum
            // SeitenAnfang ausgeben
            // sprmSDyaHdrTop
            if( rWW8Wrt.bWrtWW8 )
                rWW8Wrt.InsUInt16( 0xB017 );
            else
                rWW8Wrt.pO->Insert( 156, rWW8Wrt.pO->Count() );
            rWW8Wrt.InsUInt16( nUDist );
            nUDist += ::lcl_CalcHdFtDist( *((SwFmtHeader*)pItem)->
                                            GetHeaderFmt(), TRUE );
        }
        // sprmSDyaTop
        if( rWW8Wrt.bWrtWW8 )
            rWW8Wrt.InsUInt16( 0x9023 );
        else
            rWW8Wrt.pO->Insert( 168, rWW8Wrt.pO->Count() );
        rWW8Wrt.InsUInt16( nUDist );


        if( 0 != ( pItem = rWW8Wrt.HasItem( RES_FOOTER )) &&
            ((SwFmtFooter*)pItem)->IsActive() )
        {
            // dann aufjedenfall schon mal den Abstand der Fusszeile zum
            // SeitenEnde ausgeben
            // sprmSDyaHdrBottom
            if( rWW8Wrt.bWrtWW8 )
                rWW8Wrt.InsUInt16( 0xB018 );
            else
                rWW8Wrt.pO->Insert( 157, rWW8Wrt.pO->Count() );
            rWW8Wrt.InsUInt16( nLDist );
            nLDist += ::lcl_CalcHdFtDist( *((SwFmtFooter*)pItem)->
                                            GetFooterFmt(), FALSE );
        }
        // sprmSDyaBottom
        if( rWW8Wrt.bWrtWW8 )
            rWW8Wrt.InsUInt16( 0x9024 );
        else
            rWW8Wrt.pO->Insert( 169, rWW8Wrt.pO->Count() );
        rWW8Wrt.InsUInt16( nLDist );
    }
    else
    {
        // sprmPDyaBefore
        if( rWW8Wrt.bWrtWW8 )
            rWW8Wrt.InsUInt16( 0xA413 );
        else
            rWW8Wrt.pO->Insert( 21, rWW8Wrt.pO->Count() );
        rWW8Wrt.InsUInt16( rUL.GetUpper() );
        // sprmPDyaAfter
        if( rWW8Wrt.bWrtWW8 )
            rWW8Wrt.InsUInt16( 0xA414 );
        else
            rWW8Wrt.pO->Insert( 22, rWW8Wrt.pO->Count() );
        rWW8Wrt.InsUInt16( rUL.GetLower() );
    }
    return rWrt;
}

// Print, Opaque, Protect fehlen noch

static Writer& OutWW8_SwFmtSurround( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwWW8Writer& rWW8Wrt = (SwWW8Writer&)rWrt;
    if( rWW8Wrt.bOutFlyFrmAttrs )
    {
        if( rWW8Wrt.bWrtWW8 )
            rWW8Wrt.InsUInt16( 0x2423 );
        else
            rWW8Wrt.pO->Insert( 37, rWW8Wrt.pO->Count() );

        rWW8Wrt.pO->Insert( (SURROUND_NONE !=
                        ((const SwFmtSurround&) rHt).GetSurround() )
                    ? 2 : 1, rWW8Wrt.pO->Count() );
    }
    return rWrt;
}

Writer& OutWW8_SwFmtVertOrient( Writer& rWrt, const SfxPoolItem& rHt )
{
    const SwFmtVertOrient& rFlyVert = (const SwFmtVertOrient&) rHt;
    SwWW8Writer& rWW8Wrt = (SwWW8Writer&)rWrt;

//!!!! Ankertyp und entsprechende Umrechnung fehlt noch

    if( rWW8Wrt.bOutFlyFrmAttrs )
    {
        const SwFmtVertOrient& rFlyVert = (const SwFmtVertOrient&) rHt;

        short nPos;
        switch( rFlyVert.GetVertOrient() )
        {
        case VERT_NONE:         nPos = (short)rFlyVert.GetPos(); break;
        case VERT_CENTER:
        case VERT_LINE_CENTER:  nPos = -8; break;
        case VERT_BOTTOM:
        case VERT_LINE_BOTTOM:  nPos = -12; break;
        case VERT_TOP:
        case VERT_LINE_TOP:
        default:                nPos = -4; break;
        }

        // sprmPDyaAbs
        if( rWW8Wrt.bWrtWW8 )
            rWW8Wrt.InsUInt16( 0x8419 );
        else
            rWW8Wrt.pO->Insert( 27, rWW8Wrt.pO->Count() );
        rWW8Wrt.InsUInt16( nPos );
    }
    return rWrt;
}


Writer& OutWW8_SwFmtHoriOrient( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwWW8Writer& rWW8Wrt = (SwWW8Writer&)rWrt;
    if( !rWW8Wrt.pFlyFmt )
    {
        ASSERT( rWW8Wrt.pFlyFmt, "HoriOrient ohne pFlyFmt !!" );
        return rWrt;
    }

//!!!! Ankertyp und entsprechende Umrechnung fehlt noch
    if( rWW8Wrt.bOutFlyFrmAttrs )
    {
        const SwFmtHoriOrient& rFlyHori = (const SwFmtHoriOrient&) rHt;

        short nPos;
        switch( rFlyHori.GetHoriOrient() )
        {
        case HORI_NONE:    {
                                nPos = (short)rFlyHori.GetPos();
                                if( !nPos )
                                    nPos = 1;   // WW: 0 ist reserviert
#if 0
// ist nicht noetig, da Bindung an Absatz als hor. Bindung an die
// Spalte exportiert wird, da so das Verhalten bei Mehrspaltigkeit
// am AEhnlichsten ist.
                                RndStdIds eAn = rWW8Wrt.pFlyFmt->GetAttrSet().
                                                GetAnchor().GetAnchorId();
                                if( eAn == FLY_AT_CNTNT || eAn == FLY_IN_CNTNT )
                                    nPos += GetPageL( rWW8Wrt );
#endif
                           }
                           break;

        case HORI_LEFT:    nPos = rFlyHori.IsPosToggle() ? -12 : 0;
                           break;
        case HORI_RIGHT:   nPos = rFlyHori.IsPosToggle() ? -16 : -8;
                           break;
        case HORI_CENTER:
        case HORI_FULL:                         // FULL nur fuer Tabellen
        default:           nPos = -4; break;
        }

        // sprmPDxaAbs
        if( rWW8Wrt.bWrtWW8 )
            rWW8Wrt.InsUInt16( 0x8418 );
        else
            rWW8Wrt.pO->Insert( 26, rWW8Wrt.pO->Count() );
        rWW8Wrt.InsUInt16( nPos );
    }
    return rWrt;
}

static Writer& OutWW8_SwFmtAnchor( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwWW8Writer& rWW8Wrt = (SwWW8Writer&)rWrt;
    ASSERT( rWW8Wrt.pFlyFmt, "Anker ohne pFlyFmt !!" );

    if( rWW8Wrt.bOutFlyFrmAttrs )
    {
        const SwFmtAnchor& rAnchor = (const SwFmtAnchor&) rHt;
        BYTE nP = 0;
        switch( rAnchor.GetAnchorId() )
        {
        case FLY_PAGE:      nP |= ( 1 << 4 )        // Vert: Page
                                  | ( 2 << 6 );     // Horz: Page
                            break;
        // Im Fall eine Flys als Zeichen: Absatz-gebunden setzen!!!
        case FLY_AT_FLY:
        case FLY_AUTO_CNTNT:
        case FLY_AT_CNTNT:
        case FLY_IN_CNTNT:  nP |= ( 2 << 4 )        // Vert: Text
                                  | ( 0 << 6 );     // Horz: Column
                            break;
        }

        // sprmPPc
        if( rWW8Wrt.bWrtWW8 )
            rWW8Wrt.InsUInt16( 0x261B );
        else
            rWW8Wrt.pO->Insert( 29, rWW8Wrt.pO->Count() );
        rWW8Wrt.pO->Insert( nP, rWW8Wrt.pO->Count() );
    }
    return rWrt;
}

static Writer& OutWW8_SwFmtBackground( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwWW8Writer& rWW8Wrt = (SwWW8Writer&)rWrt;

    if( !rWW8Wrt.bOutPageDescs )    // WW kann keinen Hintergrund
    {                               // in Section
        const SvxBrushItem& rBack = (const SvxBrushItem&)rHt;
        WW8_SHD aSHD;

        if( rWW8Wrt.TransBrush( rBack.GetColor(), aSHD ) )
        {
            // sprmPShd
            if( rWW8Wrt.bWrtWW8 )
                rWW8Wrt.InsUInt16( 0x442D );
            else
                rWW8Wrt.pO->Insert( 47, rWW8Wrt.pO->Count() );
            rWW8Wrt.InsUInt16( aSHD.GetValue() );
        }
    }
    return rWrt;
}


// TranslateBorderLine() liefert eine UINT32 zurueck, der noch in einen
// SVBT32 uebersetzt werden muss.
// bei Export als WW6 sind nur die unteren 16 Bit von Interesse
// bShadow darf bei Tabellenzellen *nicht* gesetzt sein !
UINT32 SwWW8Writer::TranslateBorderLine( const SvxBorderLine& rLine,
                                        USHORT nDist, BOOL bShadow )
{
    UINT32 nBrd = 0;
    UINT16 nWidth = rLine.GetInWidth() + rLine.GetOutWidth(), brcType = 0;
    UINT32 nColCode = 0;
    if( nWidth )                                // Linie ?
    {
        // BRC.brcType
        BOOL bDouble = 0 != rLine.GetInWidth() && 0 != rLine.GetOutWidth();
        BOOL bThick = !bDouble && !bWrtWW8 && nWidth > 75;
        if( bDouble )
            brcType = 3;
        else if( bThick )
            brcType = 2;
        else
            brcType = 1;

        // BRC.dxpLineWidth
        if( bThick )
            nWidth /= 2;

        if( bWrtWW8 )
        {
            // Angabe in 8tel Punkten, also durch 2.5, da 1 Punkt = 20 Twips
            nWidth = (( nWidth * 8 ) + 10 ) / 20;
            if( 0xff < nWidth )
                nWidth = 0xff;
        }
        else
        {
            // Angabe in 0.75 pt
            nWidth = ( nWidth + 7 ) / 15;
            if( nWidth > 5 )
                nWidth = 5;
        }

        if( 0 == nWidth )                       // ganz duenne Linie
            nWidth = 1;                         //       nicht weglassen

        // BRC.ico
        nColCode = TransCol( rLine.GetColor() );
    }

    // BRC.dxpSpace
    long nLDist = nDist;
    nLDist /= 20;               // Masseinheit : pt
    if( nLDist > 0x1f )
        nLDist = 0x1f;

    if( bWrtWW8 )
    {
        nBrd = nWidth + ( brcType << 8 );
        nBrd |= nColCode << 16;
        nBrd |= nLDist << 24;

        // fShadow, keine weiteren Einstellungen im WW moeglich
        if( bShadow )
            nBrd |= (0x2000L << 16);
    }
    else
    {
        nBrd = nWidth + ( brcType << 3 );
        nBrd |= (nColCode & 0x1f) << 6;
        nBrd |= nLDist << 11;
        // fShadow, keine weiteren Einstellungen im WW moeglich
        if( bShadow )
            nBrd |= 0x20;
    }

    return nBrd;
}

// MakeBorderLine() bekommt einen WW8Bytes* uebergeben, um die Funktion
// auch fuer die Tabellen-Umrandungen zu benutzen.
// Wenn nSprmNo == 0, dann wird der Opcode nicht ausgegeben.
// bShadow darf bei Tabellenzellen *nicht* gesetzt sein !
void SwWW8Writer::Out_BorderLine( WW8Bytes& rO, const SvxBorderLine* pLine,
                                  USHORT nDist, USHORT nOffset,
                                  BOOL bShadow )
{
    ASSERT( (0 <= nOffset && nOffset <= 3) || USHRT_MAX == nOffset ||
            ((0x702b - 0x6424) <= nOffset && nOffset <= (0x702e - 0x6424)),
                "SprmOffset ausserhalb des Bereichs" );


    UINT32 nBrdCd = pLine ? TranslateBorderLine( *pLine, nDist, bShadow )
                          : 0;
    if( bWrtWW8 )
    {
// WW97-SprmIds
// 0x6424, sprmPBrcTop      pap.brcTop;BRC;long; !!!!
// 0x6425, sprmPBrcLeft
// 0x6426, sprmPBrcBottom
// 0x6427, sprmPBrcRight
        if( USHRT_MAX != nOffset )                      // mit OpCode-Ausgabe ?
            SwWW8Writer::InsUInt16( rO, 0x6424 + nOffset );

        SwWW8Writer::InsUInt32( rO, nBrdCd );
    }
    else
    {
// WW95-SprmIds
//     38, sprmPBrcTop      - pap.brcTop BRC short !!!
//     39, sprmPBrcLeft
//     40, sprmPBrcBottom
//     41, sprmPBrcRight
        if( USHRT_MAX != nOffset )                      // mit OpCode-Ausgabe ?
            rO.Insert( (BYTE)( 38 + nOffset ), rO.Count() );
        SwWW8Writer::InsUInt16( rO, (UINT16)nBrdCd );
    }
}

// OutWW8_SwFmtBox1() ist fuer alle Boxen ausser in Tabellen.
// es wird pO des WW8Writers genommen
void SwWW8Writer::Out_SwFmtBox( const SvxBoxItem& rBox, BOOL bShadow )
{
    USHORT nOffset = 0;
    if( bOutPageDescs )
    {
        if( !bWrtWW8 )
            return ;            // WW95 kennt keine Seitenumrandung

// WW97-SprmIds
// 0x702b, sprmSBrcTop      pap.brcTop;BRC;long; !!!!
// 0x702c, sprmSBrcLeft
// 0x702d, sprmSBrcBottom
// 0x702e, sprmSBrcRight
        nOffset = (0x702b - 0x6424);
    }

    static USHORT __READONLY_DATA aBorders[] = {
            BOX_LINE_TOP, BOX_LINE_LEFT, BOX_LINE_BOTTOM, BOX_LINE_RIGHT };
    const USHORT* pBrd = aBorders;
    for( int i = 0; i < 4; ++i, ++pBrd )
    {
        const SvxBorderLine* pLn = rBox.GetLine( *pBrd );
        Out_BorderLine( *pO, pLn, rBox.GetDistance( *pBrd ), nOffset+i,
                            bShadow );
    }
}

// OutWW8_SwFmtBox2() ist fuer TC-Strukturen in Tabellen. Der Sprm-Opcode
// wird nicht geschrieben, da es in der TC-Structur ohne Opcode gepackt ist.
// dxpSpace wird immer 0, da WW das in Tabellen so verlangt
// ( Tabellenumrandungen fransen sonst aus )
// Ein WW8Bytes-Ptr wird als Ausgabe-Parameter uebergeben

void SwWW8Writer::Out_SwFmtTableBox( WW8Bytes& rO, const SvxBoxItem& rBox )
{
    // moeglich und vielleicht besser waere 0xffff
    static USHORT __READONLY_DATA aBorders[] = {
            BOX_LINE_TOP, BOX_LINE_LEFT, BOX_LINE_BOTTOM, BOX_LINE_RIGHT };
    const USHORT* pBrd = aBorders;
    for( int i = 0; i < 4; ++i, ++pBrd )
    {
        const SvxBorderLine* pLn = rBox.GetLine( *pBrd );
        Out_BorderLine( rO, pLn, 0, USHRT_MAX, FALSE );
    }
}

static Writer& OutWW8_SwFmtBox( Writer& rWrt, const SfxPoolItem& rHt )
{
                                // Fly um Grafik-> keine Umrandung hier, da
                                // der GrafikHeader bereits die Umrandung hat
    SwWW8Writer& rWW8Wrt = ((SwWW8Writer&)rWrt);
    if( !rWW8Wrt.bOutGrf )
    {
        BOOL bShadow = FALSE;
        const SfxPoolItem* pItem = ((SwWW8Writer&)rWrt).HasItem( RES_SHADOW );
        if( pItem )
        {
            const SvxShadowItem* p = (const SvxShadowItem*)pItem;
            bShadow = ( p->GetLocation() != SVX_SHADOW_NONE )
                      && ( p->GetWidth() != 0 );
        }

        rWW8Wrt.Out_SwFmtBox( (SvxBoxItem&)rHt, bShadow );
    }
    return rWrt;
}


static Writer& OutWW8_SwFmtCol( Writer& rWrt, const SfxPoolItem& rHt )
{
    const SwFmtCol& rCol = (const SwFmtCol&)rHt;
    const SwColumns& rColumns = rCol.GetColumns();
    SwWW8Writer& rWW8Wrt = ((SwWW8Writer&)rWrt);

    USHORT nCols = rColumns.Count();
    if( 1 < nCols
        && !rWW8Wrt.bOutFlyFrmAttrs )       // mehrspaltige Rahmen kann WW nicht
    {
        // dann besorge mal die Seitenbreite ohne Raender !!
        SwTwips nPageSize;
        const SwFrmFmt* pFmt = rWW8Wrt.pAktPageDesc
                                ? &rWW8Wrt.pAktPageDesc->GetMaster()
                                : &rWW8Wrt.pDoc->GetPageDesc(0).GetMaster();

        SwRect aRect( pFmt->FindLayoutRect( TRUE ));
        if( 0 == ( nPageSize = aRect.Width() ))
        {
            const SvxLRSpaceItem& rLR = pFmt->GetLRSpace();
            nPageSize = pFmt->GetFrmSize().GetWidth() -
                            rLR.GetLeft() - rLR.GetRight();
        }

        // CColumns
        if( rWW8Wrt.bWrtWW8 )
            rWW8Wrt.InsUInt16( 0x500b );
        else
            rWW8Wrt.pO->Insert( 144, rWW8Wrt.pO->Count() );
        rWW8Wrt.InsUInt16( nCols - 1 );

        // DxaColumns
        if( rWW8Wrt.bWrtWW8 )
            rWW8Wrt.InsUInt16( 0x900c );
        else
            rWW8Wrt.pO->Insert( 145, rWW8Wrt.pO->Count() );
        rWW8Wrt.InsUInt16( rCol.GetGutterWidth( TRUE ) );

        // LBetween
        if( rWW8Wrt.bWrtWW8 )
            rWW8Wrt.InsUInt16( 0x3019 );
        else
            rWW8Wrt.pO->Insert( 158, rWW8Wrt.pO->Count() );
        rWW8Wrt.pO->Insert( COLADJ_NONE == rCol.GetLineAdj() ? 0 : 1,
                            rWW8Wrt.pO->Count() );

        // Nachsehen, ob alle Spalten gleich sind
        BOOL bEven = TRUE;
        USHORT n;
        USHORT nColWidth = rCol.CalcPrtColWidth( 0, (USHORT)nPageSize );
        for( n = 1; n < nCols; n++ )
        {
            short nDiff = nColWidth - rCol.CalcPrtColWidth( n, (USHORT)nPageSize );
            if( nDiff > 10 || nDiff < -10 )      // Toleranz: 10 tw
            {
                bEven = FALSE;
                break;
            }
        }
        if( bEven )
        {
            USHORT nSpace = rColumns[0]->GetRight() + rColumns[1]->GetLeft();
            for( n = 2; n < nCols; n++ )
            {
                short nDiff = nSpace - ( rColumns[n - 1]->GetRight()
                                         + rColumns[n]->GetLeft() );
                if( nDiff > 10 || nDiff < -10 )
                { // Toleranz: 10 tw
                    bEven = FALSE;
                    break;
                }
            }
        }

        // FEvenlySpaced
        if( rWW8Wrt.bWrtWW8 )
            rWW8Wrt.InsUInt16( 0x3005 );
        else
            rWW8Wrt.pO->Insert( 138, rWW8Wrt.pO->Count() );
        rWW8Wrt.pO->Insert( bEven ? 1 : 0, rWW8Wrt.pO->Count() );

        if( !bEven )
        {
            for( USHORT n = 0; n < nCols; ++n )
            {
                //sprmSDxaColWidth
                if( rWW8Wrt.bWrtWW8 )
                    rWW8Wrt.InsUInt16( 0xF203 );
                else
                    rWW8Wrt.pO->Insert( 136, rWW8Wrt.pO->Count() );
                rWW8Wrt.pO->Insert( n, rWW8Wrt.pO->Count() );
                rWW8Wrt.InsUInt16( rCol.CalcPrtColWidth( n, (USHORT)nPageSize ) );

                if( n+1 != nCols )
                {
                    //sprmSDxaColSpacing
                    if( rWW8Wrt.bWrtWW8 )
                        rWW8Wrt.InsUInt16( 0xF204 );
                    else
                        rWW8Wrt.pO->Insert( 137, rWW8Wrt.pO->Count() );
                    rWW8Wrt.pO->Insert( n, rWW8Wrt.pO->Count() );
                    rWW8Wrt.InsUInt16( rColumns[ n ]->GetRight() +
                                       rColumns[ n + 1 ]->GetLeft() );
                }
            }
        }
    }
    return rWrt;
}

// "Absaetze zusammenhalten"
static Writer& OutWW8_SvxFmtKeep( Writer& rWrt, const SfxPoolItem& rHt )
{
// sprmFKeepFollow
    const SvxFmtKeepItem& rAttr = (const SvxFmtKeepItem&)rHt;
    SwWW8Writer& rWrtWW8 = (SwWW8Writer&)rWrt;
    if( rWrtWW8.bWrtWW8 )
        rWrtWW8.InsUInt16( 0x2406 );
    else
        rWrtWW8.pO->Insert( 8, rWrtWW8.pO->Count() );

    rWrtWW8.pO->Insert( rAttr.GetValue() ? 1 : 0, rWrtWW8.pO->Count() );
    return rWrt;
}


// exclude a paragraph from Line Numbering
static Writer& OutWW8_SwFmtLineNumber( Writer& rWrt, const SfxPoolItem& rHt )
{
// sprmPFNoLineNumb
    const SwFmtLineNumber& rAttr = (const SwFmtLineNumber&)rHt;

    SwWW8Writer& rWrtWW8 = (SwWW8Writer&)rWrt;
    if( rWrtWW8.bWrtWW8 )
        rWrtWW8.InsUInt16( 0x240C );
    else
        rWrtWW8.pO->Insert( 14, rWrtWW8.pO->Count() );

    rWrtWW8.pO->Insert( rAttr.IsCount() ? 0 : 1, rWrtWW8.pO->Count() );

    return rWrt;
}


/* File PARATR.HXX  */

static Writer& OutWW8_SvxLineSpacing( Writer& rWrt, const SfxPoolItem& rHt )
{
// sprmPDyaLine
    const SvxLineSpacingItem& rAttr = (const SvxLineSpacingItem&)rHt;
    SwWW8Writer& rWrtWW8 = (SwWW8Writer&)rWrt;
    if( rWrtWW8.bWrtWW8 )
        rWrtWW8.InsUInt16( 0x6412 );
    else
        rWrtWW8.pO->Insert( 20, rWrtWW8.pO->Count() );

    short nSpace = 240, nMulti = 0;

    switch( rAttr.GetLineSpaceRule() )
    {
    case SVX_LINE_SPACE_AUTO:
    case SVX_LINE_SPACE_FIX:
    case SVX_LINE_SPACE_MIN:
        {
            switch( rAttr.GetInterLineSpaceRule() )
            {
            case SVX_INTER_LINE_SPACE_FIX:      // unser Durchschuss
                // gibt es aber nicht in WW - also wie kommt man an
                // die MaxLineHeight heran?
                nSpace = (short)rAttr.GetInterLineSpace();
                break;

            case SVX_INTER_LINE_SPACE_PROP:
                nSpace = (short)(( 240L * rAttr.GetPropLineSpace() ) / 100L );
                nMulti = 1;
                break;

            default:                    // z.B. Minimum oder FIX?
                if( SVX_LINE_SPACE_FIX == rAttr.GetLineSpaceRule() )
                    nSpace = -(short)rAttr.GetLineHeight();
                else
                    nSpace = (short)rAttr.GetLineHeight();
                break;
            }
        }
        break;
    }

    rWrtWW8.InsUInt16( nSpace );
    rWrtWW8.InsUInt16( nMulti );
    return rWrt;
}


static Writer& OutWW8_SvxAdjust( Writer& rWrt, const SfxPoolItem& rHt )
{
// sprmPJc
    const SvxAdjustItem& rAttr = (const SvxAdjustItem&)rHt;
    BYTE nAdj = 255;
    switch( rAttr.GetAdjust() )
    {
    case SVX_ADJUST_LEFT:       nAdj = 0;   break;
    case SVX_ADJUST_RIGHT:      nAdj = 2;   break;
    case SVX_ADJUST_BLOCKLINE:
    case SVX_ADJUST_BLOCK:      nAdj = 3;   break;
    case SVX_ADJUST_CENTER:     nAdj = 1;   break;
    default:                    return rWrt;    // kein gueltiges Attribut
    }

    if( 255 != nAdj )               // gueltiges Attribut?
    {
        SwWW8Writer& rWrtWW8 = (SwWW8Writer&)rWrt;
        if( rWrtWW8.bWrtWW8 )
            rWrtWW8.InsUInt16( 0x2403 );
        else
            rWrtWW8.pO->Insert( 5, rWrtWW8.pO->Count() );
        rWrtWW8.pO->Insert( nAdj, rWrtWW8.pO->Count() );
    }
    return rWrt;
}

// "Absaetze trennen"
static Writer& OutWW8_SvxFmtSplit( Writer& rWrt, const SfxPoolItem& rHt )
{
// sprmPFKeep
    const SvxFmtSplitItem& rAttr = (const SvxFmtSplitItem&)rHt;
    SwWW8Writer& rWrtWW8 = (SwWW8Writer&)rWrt;
    if( rWrtWW8.bWrtWW8 )
        rWrtWW8.InsUInt16( 0x2405 );
    else
        rWrtWW8.pO->Insert( 7, rWrtWW8.pO->Count() );
    rWrtWW8.pO->Insert( rAttr.GetValue() ? 0 : 1, rWrtWW8.pO->Count() );
    return rWrt;
}

//  Es wird nur das Item "SvxWidowItem" und nicht die Orphans uebersetzt,
//  da es fuer beides im WW nur ein Attribut "Absatzkontrolle" gibt und
//  im SW wahrscheinlich vom Anwender immer Beide oder keiner gesetzt werden.
static Writer& OutWW8_SvxWidows( Writer& rWrt, const SfxPoolItem& rHt )
{
// sprmPFWidowControl
    const SvxWidowsItem& rAttr = (const SvxWidowsItem&)rHt;
    SwWW8Writer& rWrtWW8 = (SwWW8Writer&)rWrt;
    if( rWrtWW8.bWrtWW8 )
        rWrtWW8.InsUInt16( 0x2431 );
    else
        rWrtWW8.pO->Insert( 51, rWrtWW8.pO->Count() );
    rWrtWW8.pO->Insert( rAttr.GetValue() ? 1 : 0, rWrtWW8.pO->Count() );
    return rWrt;
}


class SwWW8WrTabu
{
    BYTE* pDel;                 // DelArray
    BYTE* pAddPos;              // AddPos-Array
    BYTE* pAddTyp;              // AddTyp-Array
    USHORT nAdd;                // so viele Tabs kommen hinzu
    USHORT nDel;                // so viele Tabs fallen weg
    USHORT nLeftParaMgn;        // fuer anderes Wertesystem SW/WW
public:
    SwWW8WrTabu( USHORT nDelMax, USHORT nAddMax, USHORT nLeftPara );
    ~SwWW8WrTabu();

    void Add( const SvxTabStop & rTS );
    void Del( const SvxTabStop & rTS );
    void PutAll( SwWW8Writer& rWW8Wrt );
};

SwWW8WrTabu::SwWW8WrTabu( USHORT nDelMax, USHORT nAddMax, USHORT nLeftPara )
    : nAdd( 0 ), nDel( 0 ), nLeftParaMgn( nLeftPara )
{
    pDel = ( nDelMax ) ? new BYTE[ nDelMax * 2 ] : 0;
    pAddPos = new BYTE[ nAddMax * 2 ];
    pAddTyp = new BYTE[ nAddMax ];
}

SwWW8WrTabu::~SwWW8WrTabu()
{
    delete[] pAddTyp;
    delete[] pAddPos;
    delete[] pDel;
}

// Add( const SvxTabStop & rTS ) fuegt einen Tab in die WW-Struktur ein
void SwWW8WrTabu::Add( const SvxTabStop & rTS )
{
    // Tab-Position eintragen
    ShortToSVBT16( (USHORT)rTS.GetTabPos() + nLeftParaMgn,
                   pAddPos + ( nAdd * 2 ) );

    // Tab-Typ eintragen
    BYTE nPara = 0;
    switch( rTS.GetAdjustment() )
    {
    case SVX_TAB_ADJUST_RIGHT:      nPara = 2;  break;
    case SVX_TAB_ADJUST_CENTER:     nPara = 1;  break;
    case SVX_TAB_ADJUST_DECIMAL:
            nPara = ',' == rTS.GetDecimal() ? 3 : 1;
            break;
    }

    switch( rTS.GetFill() )
    {
    case '.':   nPara |= 1 << 3; break;     // dotted leader
    case '_':   nPara |= 3 << 3; break;     // Single line leader
    case '-':   nPara |= 2 << 3; break;     // hyphenated leader
    case '=':   nPara |= 4 << 3; break;     // heavy line leader
    }

    ByteToSVBT8( nPara, pAddTyp + nAdd );
    ++nAdd;
}

// Del( const SvxTabStop & rTS ) fuegt einen zu loeschenden Tab
// in die WW-Struktur ein
void SwWW8WrTabu::Del( const SvxTabStop & rTS )
{
                                        // Tab-Position eintragen
    ShortToSVBT16( (USHORT)rTS.GetTabPos() + nLeftParaMgn,
                   pDel + ( nDel * 2 ) );
    ++nDel;
}

//  PutAll( SwWW8Writer& rWW8Wrt ) schreibt das Attribut nach rWrt.pO
void SwWW8WrTabu::PutAll( SwWW8Writer& rWrt )
{
    if( nAdd > 255 )
    {
        ASSERT( !this, "+ mehr als 255 Tabs dazu ?????" );
        nAdd = 255;
    }
    if( nDel > 255 )
    {
        ASSERT( !this, "+ mehr als 255 Tabs weg ?????" );
        nDel = 255;
    }

    USHORT nSiz = 2 * nDel + 3 * nAdd + 2;
    if( nSiz > 255 )
        nSiz = 255;

    if( rWrt.bWrtWW8 )
        rWrt.InsUInt16( 0xC60D );
    else
        rWrt.pO->Insert( 15, rWrt.pO->Count() );
    rWrt.pO->Insert( (BYTE)nSiz, rWrt.pO->Count() );    // cch eintragen

    // DelArr schreiben
    rWrt.pO->Insert( (BYTE)nDel, rWrt.pO->Count() );    // nDel eintragen
    rWrt.OutSprmBytes( pDel, nDel * 2 );
    // InsArr schreiben
    rWrt.pO->Insert( (BYTE)nAdd, rWrt.pO->Count() );    // nAdd schreiben
    rWrt.OutSprmBytes( pAddPos, 2 * nAdd );         // AddPosArray
    rWrt.OutSprmBytes( pAddTyp, nAdd );             // AddTypArray
}


static void OutWW8_SwTabStopAdd( Writer& rWrt, const SvxTabStopItem& rTStops,
                                 USHORT nLParaMgn  )
{
    SwWW8WrTabu aTab( 0, rTStops.Count(), nLParaMgn );

    for( USHORT n = 0; n < rTStops.Count(); n++ )
    {
        const SvxTabStop& rTS = rTStops[ n ];
        if( SVX_TAB_ADJUST_DEFAULT != rTS.GetAdjustment() ) // Def-Tabs ignorieren
            aTab.Add( rTS );
    }
    aTab.PutAll( (SwWW8Writer&)rWrt );
}

static void OutWW8_SwTabStopDelAdd( Writer& rWrt, const SvxTabStopItem& rTOld,
                                const SvxTabStopItem& rTNew, USHORT nLParaMgn )
{
    SwWW8WrTabu aTab( rTOld.Count(), rTNew.Count(), nLParaMgn );

    USHORT nO = 0;      // rTOld Index
    USHORT nN = 0;      // rTNew Index

    do {
        const SvxTabStop* pTO;
        long nOP;
        if( nO < rTOld.Count() )                    // alt noch nicht am Ende ?
        {
            pTO = &rTOld[ nO ];
            nOP = pTO->GetTabPos();
            if( SVX_TAB_ADJUST_DEFAULT == pTO->GetAdjustment() )
            {
                nO++;                                // Default-Tab ignorieren
                continue;
            }
        }
        else
        {
            pTO = 0;
            nOP = LONG_MAX;
        }

        const SvxTabStop* pTN;
        long nNP;
        if( nN < rTNew.Count() )                    // neu noch nicht am Ende
        {
            pTN = &rTNew[ nN ];
            nNP = pTN->GetTabPos();
            if( SVX_TAB_ADJUST_DEFAULT == pTN->GetAdjustment() )
            {
                nN++;                               // Default-Tab ignorieren
                continue;
            }
        }
        else
        {
            pTN = 0;
            nNP = LONG_MAX;
        }

        if( nOP == LONG_MAX && nNP == LONG_MAX )
            break;                                  // alles fertig

        if( nOP < nNP )                             // naechster Tab ist alt
        {
            aTab.Del( *pTO );                       // muss geloescht werden
            nO++;
        }
        else if( nNP < nOP )                        // naechster Tab ist neu
        {
            aTab.Add( *pTN );                       // muss eigefuegt werden
            nN++;
        }
        else if ( pTO->IsEqual( *pTN ) )            // Tabs sind gleich
        {
            nO++;                                   // nichts zu tun
            nN++;
        }
        else                                        // Tabs selbe Pos, diff Typ
        {
            aTab.Del( *pTO );                       // alten loeschen
            aTab.Add( *pTN );                       // neuen einfuegen
            nO++;
            nN++;
        }
    } while( 1 );

    aTab.PutAll( (SwWW8Writer&)rWrt );
}

static Writer& OutWW8_SwTabStop( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwWW8Writer& rWW8Wrt = (SwWW8Writer&)rWrt;
    const SvxTabStopItem & rTStops = (const SvxTabStopItem&)rHt;
    const SfxPoolItem* pLR = rWW8Wrt.HasItem( RES_LR_SPACE );
    USHORT nOffset = pLR ? ((const SvxLRSpaceItem*)pLR)->GetTxtLeft() : 0;

    const SfxPoolItem* pItem = 0;

    if( rWW8Wrt.bStyDef             // StyleDef -> "einfach" eintragen
        || !rWW8Wrt.pStyAttr        // keine Style-Attrs -> dito
        || ( SFX_ITEM_SET != rWW8Wrt.pStyAttr->GetItemState(
            RES_PARATR_TABSTOP, TRUE, &pItem  ) ) ) // kein Tab-Attr in Styles -> dito
    {
        OutWW8_SwTabStopAdd( rWW8Wrt, rTStops, nOffset );
    }
    else if ( pItem )       // Harte Attribute: Differenzen zum Style eintragen
    {
        OutWW8_SwTabStopDelAdd( rWW8Wrt, *(const SvxTabStopItem*)pItem,
                                rTStops, nOffset );
    }
    return rWrt;
}

//-----------------------------------------------------------------------

/*
 * lege hier die Tabellen fuer die WW-Funktions-Pointer auf
 * die Ausgabe-Funktionen an.
 * Es sind lokale Strukturen, die nur innerhalb
 * bekannt sein muessen.
 */

SwAttrFnTab aWW8AttrFnTab = {
/* RES_CHRATR_CASEMAP   */          OutWW8_SwCaseMap,
/* RES_CHRATR_CHARSETCOLOR */       0,
/* RES_CHRATR_COLOR */              OutWW8_SwColor,
/* RES_CHRATR_CONTOUR   */          OutWW8_SwContour,
/* RES_CHRATR_CROSSEDOUT    */      OutWW8_SwCrossedOut,
/* RES_CHRATR_ESCAPEMENT    */      OutWW8_SwEscapement,
/* RES_CHRATR_FONT  */              OutWW8_SwFont,
/* RES_CHRATR_FONTSIZE  */          OutWW8_SwSize,
/* RES_CHRATR_KERNING   */          OutWW8_SwKerning,
/* RES_CHRATR_LANGUAGE  */          OutWW8_SwLanguage,
/* RES_CHRATR_POSTURE   */          OutWW8_SwPosture,
/* RES_CHRATR_PROPORTIONALFONTSIZE*/0,
/* RES_CHRATR_SHADOWED  */          OutWW8_SwShadow,
/* RES_CHRATR_UNDERLINE */          OutWW8_SwUnderline,
/* RES_CHRATR_WEIGHT    */          OutWW8_SwWeight,
/* RES_CHRATR_WORDLINEMODE   */     0,   // Wird bei Underline mitbehandelt
/* RES_CHRATR_AUTOKERN   */         OutWW8_SvxAutoKern,
/* RES_CHRATR_BLINK */              0, // neu: blinkender Text
/* RES_CHRATR_NOHYPHEN  */          0, // Neu: nicht trennen
/* RES_CHRATR_NOLINEBREAK */        0, // Neu: nicht umbrechen
/* RES_CHRATR_BACKGROUND */         OutWW8_SwFmtCharBackground,
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

/* RES_TXTATR_INETFMT */            OutSwFmtINetFmt,
/* RES_TXTATR_DUMMY */              0,
/* RES_TXTATR_REFMARK */            0,      // handel by SwAttrIter
/* RES_TXTATR_TOXMARK   */          0,      // handel by SwAttrIter
/* RES_TXTATR_CHARFMT   */          OutWW8_SwTxtCharFmt,
/* RES_TXTATR_TWO_LINES */          0,
/* RES_TXTATR_CJK_RUBY */           0,
/* RES_TXTATR_UNKNOWN_CONTAINER */  0,
/* RES_TXTATR_DUMMY5 */             0,
/* RES_TXTATR_DUMMY6 */             0,

/* RES_TXTATR_FIELD */              OutWW8_SwField,
/* RES_TXTATR_FLYCNT    */          OutWW8_SwFlyCntnt,
/* RES_TXTATR_FTN       */          OutWW8_SwFtn,
/* RES_TXTATR_SOFTHYPH */           0,  // old attr. - coded now by character
/* RES_TXTATR_HARDBLANK */          OutWW8_SwHardBlank,
/* RES_TXTATR_DUMMY1 */             0, // Dummy:
/* RES_TXTATR_DUMMY2 */             0, // Dummy:

/* RES_PARATR_LINESPACING   */      OutWW8_SvxLineSpacing,
/* RES_PARATR_ADJUST    */          OutWW8_SvxAdjust,
/* RES_PARATR_SPLIT */              OutWW8_SvxFmtSplit,
/* RES_PARATR_ORPHANS   */          0, // OutW4W_SwOrphans, // kann WW nicht unabhaengig von Widows
/* RES_PARATR_WIDOWS    */          OutWW8_SvxWidows,
/* RES_PARATR_TABSTOP   */          OutWW8_SwTabStop,
/* RES_PARATR_HYPHENZONE*/          OutWW8_SvxHyphenZone,
/* RES_PARATR_DROP */               0,
/* RES_PARATR_REGISTER */           0, // neu:  Registerhaltigkeit
/* RES_PARATR_NUMRULE */            OutWW8_SwNumRuleItem,
/* RES_PARATR_SCRIPTSPACE */        0, // Dummy:
/* RES_PARATR_HANGINGPUNCTUATION */ 0, // Dummy:
/* RES_PARATR_DUMMY1 */             0, // Dummy:
/* RES_PARATR_DUMMY2 */             0, // Dummy:
/* RES_PARATR_DUMMY3 */             0, // Dummy:
/* RES_PARATR_DUMMY4 */             0, // Dummy:
/* RES_PARATR_DUMMY5 */             0, // Dummy:
/* RES_PARATR_DUMMY6 */             0, // Dummy:
/* RES_PARATR_DUMMY7 */             0, // Dummy:
/* RES_PARATR_DUMMY8 */             0, // Dummy:

/* RES_FILL_ORDER   */              0, // OutW4W_SwFillOrder,
/* RES_FRM_SIZE */                  OutWW8_SwFrmSize,
/* RES_PAPER_BIN   */               OutWW8_SvxPaperBin,
/* RES_LR_SPACE */                  OutWW8_SwFmtLRSpace,
/* RES_UL_SPACE */                  OutWW8_SwFmtULSpace,
/* RES_PAGEDESC */                  0,  // wird bei OutBreaks beachtet
/* RES_BREAK */                     OutWW8_SwFmtBreak,
/* RES_CNTNT */                     0, /* 0, // OutW4W_??? */
/* RES_HEADER */                    0,  // wird bei der PageDesc ausgabe beachtet
/* RES_FOOTER */                    0,  // wird bei der PageDesc ausgabe beachtet
/* RES_PRINT */                     0, // OutW4W_SwFmtPrint,
/* RES_OPAQUE */                    0, // OutW4W_SwFmtOpaque, // kann WW nicht
/* RES_PROTECT */                   0, // OutW4W_SwFmtProtect,
/* RES_SURROUND */                  OutWW8_SwFmtSurround,
/* RES_VERT_ORIENT */               OutWW8_SwFmtVertOrient,
/* RES_HORI_ORIENT */               OutWW8_SwFmtHoriOrient,
/* RES_ANCHOR */                    OutWW8_SwFmtAnchor,
/* RES_BACKGROUND */                OutWW8_SwFmtBackground,
/* RES_BOX  */                      OutWW8_SwFmtBox,
/* RES_SHADOW */                    0, // Wird bei SwFmtBox mitbehandelt
/* RES_FRMMACRO */                  0, /* 0, // OutW4W_??? */
/* RES_COL */                       OutWW8_SwFmtCol,
/* RES_KEEP */                      OutWW8_SvxFmtKeep,
/* RES_URL */                       0, // URL
/* RES_EDIT_IN_READONLY */          0,
/* RES_LAYOUT_SPLIT */              0,
/* RES_CHAIN */                     0,
/* RES_FRMATR_DUMMY2 */             0, // Dummy:
/* RES_LINENUMBER */                OutWW8_SwFmtLineNumber, // Line Numbering
/* RES_FRMATR_DUMMY4 */             0, // Dummy:
/* RES_FRMATR_DUMMY5 */             0, // Dummy:
/* RES_FRMATR_DUMMY6 */             0, // Dummy:
/* RES_FRMATR_DUMMY7 */             0, // Dummy:
/* RES_FRMATR_DUMMY8 */             0, // Dummy:
/* RES_FRMATR_DUMMY9 */             0, // Dummy:


/* RES_GRFATR_MIRRORGRF */          0, // OutW4W_SwMirrorGrf,
/* RES_GRFATR_CROPGRF   */          0, // OutW4W_SwCropGrf
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


/*************************************************************************

      $Log: not supported by cvs2svn $
      Revision 1.2  2000/11/04 21:49:59  er
      GetMappedFormatstring with LocaleDataWrapper instead of International

      Revision 1.1.1.1  2000/09/18 17:14:58  hr
      initial import

      Revision 1.51  2000/09/18 16:04:58  willem.vandorp
      OpenOffice header added.

      Revision 1.50  2000/08/21 10:14:14  khz
      Export Line Numbering (restart on new section)

      Revision 1.49  2000/08/04 10:56:44  jp
      Soft-/HardHyphens & HardBlanks changed from attribute to unicode character

      Revision 1.48  2000/07/31 19:24:54  jp
      new attributes for CJK/CTL and graphic

      Revision 1.47  2000/07/25 14:46:38  khz
      #76811# save AutoHyphenation flag in Document Properties

      Revision 1.46  2000/07/06 11:05:03  khz
      Export *result* of field nonexisting in WW as either Unicode or 8bit text

      Revision 1.45  2000/06/19 09:30:59  khz
      Export the new WW2000 _underlining_ features

      Revision 1.44  2000/06/09 16:30:26  jp
      Write correct edit-/create-/print-/change - datetime fields

      Revision 1.43  2000/05/12 16:14:18  jp
      Changes for Unicode

      Revision 1.42  2000/04/18 12:58:03  khz
      Task #75183# to set char language use sprmCRgLid0 rather than sprmCLid

      Revision 1.41  2000/03/03 15:20:01  os
      StarView remainders removed

      Revision 1.40  2000/02/21 13:05:36  jp
      #70473# changes for unicode

      Revision 1.39  2000/02/11 14:40:03  hr
      #70473# changes for unicode ( patched by automated patchtool )

      Revision 1.38  1999/12/20 17:41:37  jp
      Bug #70749#: Date/Time-Numberformats in german format from the formatter

      Revision 1.37  1999/12/15 17:06:22  jp
      Bug #70913#: write Ftn-/EndNoteInfo - at end of section or page/doc

      Revision 1.36  1999/12/08 13:53:01  jp
      Task #70258#: textattr for XMLAttrContainer

      Revision 1.35  1999/11/29 09:09:49  jp
      InsertSpecialChar: ask for ChpIter

      Revision 1.34  1999/11/25 21:22:33  jp
      Bug #70131#: OutFields: append the charattributes of the node to the sprms

      Revision 1.33  1999/11/24 13:14:25  jp
      remove GetTimePara/GetDatePara, ReplaceCr: insert char at row end

      Revision 1.32  1999/11/22 18:26:04  jp
      export of TOX and TOX entries

      Revision 1.31  1999/11/12 16:07:12  jp
      write foot-/endnote attributes with correct charfmts

      Revision 1.30  1999/11/12 13:23:25  jp
      Escapement: write AUTO-Escapment correct

      Revision 1.29  1999/11/10 16:09:54  jp
      Bug #67922#: write header/footer for w95

      Revision 1.28  1999/10/29 17:45:57  jp
      export of 'complex'(vert. merged) tables

      Revision 1.26  1999/10/14 21:06:13  jp
      Bug #68617#,#68705# and other little things

*************************************************************************/





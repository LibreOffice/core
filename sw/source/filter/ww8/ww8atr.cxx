/*************************************************************************
 *
 *  $RCSfile: ww8atr.cxx,v $
 *
 *  $Revision: 1.82 $
 *
 *  last change: $Author: kz $ $Date: 2004-08-02 14:20:52 $
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
 * Dieses File enthaelt alle Ausgabe-Funktionen des WW8-Writers;
 * fuer alle Nodes, Attribute, Formate und Chars.
 */


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
#ifndef _SVX_PARAVERTALIGNITEM_HXX
#include <svx/paravertalignitem.hxx>
#endif
#ifndef _SVX_PGRDITEM_HXX
#include <svx/pgrditem.hxx>
#endif
#ifndef _SVX_FRMDIRITEM_HXX
#include <svx/frmdiritem.hxx>
#endif
#ifndef _SVX_BLNKITEM_HXX
#include <svx/blnkitem.hxx>
#endif
#ifndef _SVX_CHARHIDDENITEM_HXX
#include <svx/charhiddenitem.hxx>
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
#ifndef _PAM_HXX
#include <pam.hxx>
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
#ifndef _EXPFLD_HXX
#include <expfld.hxx>
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
#ifndef _BREAKIT_HXX
#include <breakit.hxx>
#endif
#ifndef _COM_SUN_STAR_I18N_SCRIPTTYPE_HDL_
#include <com/sun/star/i18n/ScriptType.hdl>
#endif
#ifndef _UNOTOOLS_LOCALEDATAWRAPPER_HXX
#include <unotools/localedatawrapper.hxx>
#endif
#ifndef SW_TGRDITEM_HXX
#include <tgrditem.hxx>
#endif
#ifndef _FLDDROPDOWN_HXX
#include <flddropdown.hxx>
#endif
#ifndef _CHPFLD_HXX
#include <chpfld.hxx>
#endif
#ifndef _FMTHDFT_HXX
#include <fmthdft.hxx>
#endif

#if OSL_DEBUG_LEVEL > 1
# ifndef _FMTCNTNT_HXX
#  include <fmtcntnt.hxx>
# endif
#endif

#ifndef SW_WRITERHELPER
#include "writerhelper.hxx"
#endif
#ifndef SW_WRITERWORDGLUE
#include "writerwordglue.hxx"
#endif

#ifndef _WRTWW8_HXX
#include "wrtww8.hxx"
#endif
#ifndef _WW8PAR_HXX
#include "ww8par.hxx"
#endif
#ifndef WW_FIELDS_HXX
#include "fields.hxx"
#endif

#ifndef _SV_OUTDEV_HXX
#include <vcl/outdev.hxx>
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

#if !defined(MSC) && !defined(UNX) && !defined(PPC) && !defined(CSET) && !defined(__MWERKS__) && !defined(WTC)

#define ATTRFNTAB_SIZE 130
#if ATTRFNTAB_SIZE != POOLATTR_END - POOLATTR_BEGIN
#   error "Attribut-Tabelle ist ungueltigt. Wurden neue Hint-ID's zugefuegt ??"
#endif

#define NODETAB_SIZE 3
#if NODETAB_SIZE != RES_NODE_END - RES_NODE_BEGIN
#   error "Node-Tabelle ist ungueltigt. Wurden neue Hint-ID's zugefuegt ??"
#endif

#endif

using namespace sw::util;
using namespace sw::types;
//------------------------------------------------------------
//  Forward-Declarationen
//------------------------------------------------------------
static Writer& OutWW8_SwFmtBreak( Writer& rWrt, const SfxPoolItem& rHt );
static Writer& OutWW8_SwNumRuleItem( Writer& rWrt, const SfxPoolItem& rHt );

/*
Sadly word does not have two different sizes for asian font size and western
font size, it has to different fonts, but not sizes, so we have to use our
guess as to the script used and disable the export of one type. The same
occurs for font weight and posture (bold and italic)

In addition WW7- has only one character language identifier while WW8+ has two
*/
bool SwWW8Writer::CollapseScriptsforWordOk(USHORT nScript, USHORT nWhich)
{
    bool bRet = true;
    if (nScript == com::sun::star::i18n::ScriptType::ASIAN)
    {
        //for asian in ww8, there is only one fontsize
        //and one fontstyle (posture/weight) for ww6
        //there is the additional problem that there
        //is only one font setting for all three scripts
        switch (nWhich)
        {
            case RES_CHRATR_FONTSIZE:
            case RES_CHRATR_POSTURE:
            case RES_CHRATR_WEIGHT:
                bRet = false;
                break;
            case RES_CHRATR_LANGUAGE:
            case RES_CHRATR_CTL_FONT:
            case RES_CHRATR_CTL_FONTSIZE:
            case RES_CHRATR_CTL_LANGUAGE:
            case RES_CHRATR_CTL_POSTURE:
            case RES_CHRATR_CTL_WEIGHT:
                if (bWrtWW8 == 0)
                    bRet = false;
            default:
                break;
        }
    }
    else if (nScript == com::sun::star::i18n::ScriptType::COMPLEX)
    {
        //Complex is ok in ww8, but for ww6 there is only
        //one font, one fontsize, one fontsize (weight/posture)
        //and only one language
        if (bWrtWW8 == 0)
        {
            switch (nWhich)
            {
                case RES_CHRATR_CJK_FONT:
                case RES_CHRATR_CJK_FONTSIZE:
                case RES_CHRATR_CJK_POSTURE:
                case RES_CHRATR_CJK_WEIGHT:
                case RES_CHRATR_CJK_LANGUAGE:
                case RES_CHRATR_FONT:
                case RES_CHRATR_FONTSIZE:
                case RES_CHRATR_POSTURE:
                case RES_CHRATR_WEIGHT:
                case RES_CHRATR_LANGUAGE:
                    bRet = false;
                    break;
                default:
                    break;
            }
        }
    }
    else
    {
        //for western in ww8, there is only one fontsize
        //and one fontstyle (posture/weight) for ww6
        //there is the additional problem that there
        //is only one font setting for all three scripts
        switch (nWhich)
        {
            case RES_CHRATR_CJK_FONTSIZE:
            case RES_CHRATR_CJK_POSTURE:
            case RES_CHRATR_CJK_WEIGHT:
                bRet = false;
                break;
            case RES_CHRATR_CJK_LANGUAGE:
            case RES_CHRATR_CTL_FONT:
            case RES_CHRATR_CTL_FONTSIZE:
            case RES_CHRATR_CTL_LANGUAGE:
            case RES_CHRATR_CTL_POSTURE:
            case RES_CHRATR_CTL_WEIGHT:
                if (bWrtWW8 == 0)
                    bRet = false;
            default:
                break;
        }
    }
    return bRet;
}

//------------------------------------------------------------
//  Hilfsroutinen fuer Styles
//------------------------------------------------------------

void SwWW8Writer::ExportPoolItemsToCHP(sw::PoolItems &rItems, USHORT nScript)
{
    sw::cPoolItemIter aEnd = rItems.end();
    for (sw::cPoolItemIter aI = rItems.begin(); aI != aEnd; ++aI)
    {
        const SfxPoolItem *pItem = aI->second;
        USHORT nWhich = pItem->Which();
        if (FnAttrOut pOut = aWW8AttrFnTab[nWhich - RES_CHRATR_BEGIN])
        {
            if (nWhich < RES_CHRATR_BEGIN || nWhich >= RES_TXTATR_END)
                continue;
            if (SwWW8Writer::CollapseScriptsforWordOk(nScript, nWhich))
                (*pOut)(*this, *pItem);
        }
    }
}

/*
 * Format wie folgt ausgeben:
 *      - gebe die Attribute aus; ohne Parents!
 */

void SwWW8Writer::Out_SfxItemSet(const SfxItemSet& rSet, bool bPapFmt,
    bool bChpFmt, USHORT nScript)
{
    if (rSet.Count())
    {
        const SfxPoolItem* pItem;
        FnAttrOut pOut;

        pISet = &rSet;                  // fuer Doppel-Attribute

        //If frame dir is set, but not adjust, then force adjust as well
        if (bPapFmt && SFX_ITEM_SET == rSet.GetItemState(RES_FRAMEDIR, false))
        {
            //No explicit adjust set ?
            if (SFX_ITEM_SET != rSet.GetItemState(RES_PARATR_ADJUST, false))
            {
                if ((pItem = rSet.GetItem(RES_PARATR_ADJUST)))
                {
                    // then set the adjust used by the parent format
                    pOut = aWW8AttrFnTab[RES_PARATR_ADJUST - RES_CHRATR_BEGIN];
                    (*pOut)( *this, *pItem );
                }
            }
        }

        if (bPapFmt && SFX_ITEM_SET == rSet.GetItemState(RES_PARATR_NUMRULE,
            false, &pItem))
        {
            pOut = aWW8AttrFnTab[ RES_PARATR_NUMRULE - RES_CHRATR_BEGIN ];
            (*pOut)( *this, *pItem );
            // switch off the numerbering?
            if( !((SwNumRuleItem*)pItem)->GetValue().Len() && SFX_ITEM_SET
                != rSet.GetItemState( RES_LR_SPACE, false) && SFX_ITEM_SET
                == rSet.GetItemState( RES_LR_SPACE, true, &pItem ) )
            {
                // the set the LR-Space of the parentformat!
                pOut = aWW8AttrFnTab[ RES_LR_SPACE - RES_CHRATR_BEGIN ];
                (*pOut)( *this, *pItem );
            }
        }

        sw::PoolItems aItems;
        GetPoolItems(rSet, aItems);
        if (bChpFmt)
            ExportPoolItemsToCHP(aItems, nScript);

        sw::cPoolItemIter aEnd = aItems.end();
        for (sw::cPoolItemIter aI = aItems.begin(); aI != aEnd; ++aI)
        {
            pItem = aI->second;
            USHORT nWhich = pItem->Which();
            pOut = aWW8AttrFnTab[nWhich - RES_CHRATR_BEGIN];
            if( 0 != pOut && (!bPapFmt || RES_PARATR_NUMRULE != nWhich ))
            {
                bool bPap = nWhich >= RES_PARATR_BEGIN
                            && nWhich < RES_FRMATR_END;
                if (bPapFmt && bPap)
                    (*pOut)(*this, *pItem);
            }
        }
        pISet = 0;                      // fuer Doppel-Attribute
    }
}

void SwWW8Writer::GatherChapterFields()
{
    //If the header/footer contains a chapter field
    SwClientIter aIter(*pDoc->GetSysFldType(RES_CHAPTERFLD));
    const SwClient *pField = aIter.First(TYPE(SwFmtFld));
    while (pField)
    {
        const SwFmtFld* pFld = (const SwFmtFld*)(pField);
        if (const SwTxtFld *pTxtFld = pFld->GetTxtFld())
        {
            const SwTxtNode &rTxtNode = pTxtFld->GetTxtNode();
            maChapterFieldLocs.push_back(rTxtNode.GetIndex());
        }
       pField = aIter.Next();
    }
}

bool SwWW8Writer::CntntContainsChapterField(const SwFmtCntnt &rCntnt) const
{
    bool bRet = false;
    if (const SwNodeIndex* pSttIdx = rCntnt.GetCntntIdx())
    {
        SwNodeIndex aIdx(*pSttIdx, 1);
        SwNodeIndex aEnd(*pSttIdx->GetNode().EndOfSectionNode());
        ULONG nStart = aIdx.GetIndex();
        ULONG nEnd = aEnd.GetIndex();
        //If the header/footer contains a chapter field
        mycCFIter aIEnd = maChapterFieldLocs.end();
        for (mycCFIter aI = maChapterFieldLocs.begin(); aI != aIEnd; ++aI)
        {
            if ((nStart <= *aI) && (*aI <= nEnd))
            {
                bRet = true;
                break;
            }
        }
    }
    return bRet;
}

bool SwWW8Writer::FmtHdFtContainsChapterField(const SwFrmFmt &rFmt) const
{
    if (maChapterFieldLocs.empty())
        return false;
    bool bRet = false;

    const SwFrmFmt *pFmt = 0;
    if (pFmt = rFmt.GetHeader().GetHeaderFmt())
        bRet = CntntContainsChapterField(pFmt->GetCntnt());
    if (!bRet && (pFmt = rFmt.GetFooter().GetFooterFmt()))
        bRet = CntntContainsChapterField(pFmt->GetCntnt());

    return bRet;
}

bool SwWW8Writer::SetAktPageDescFromNode(const SwNode &rNd)
{
    bool bNewPageDesc = false;
    const SwPageDesc* pCurrent = SwPageDesc::GetPageDescOfNode(rNd);
    ASSERT(pCurrent && pAktPageDesc, "Not possible surely");
    if (pAktPageDesc && pCurrent)
    {
        if (pCurrent != pAktPageDesc)
        {
            if (pAktPageDesc->GetFollow() != pCurrent)
                bNewPageDesc = true;
            else
            {
                const SwFrmFmt& rTitleFmt = pAktPageDesc->GetMaster();
                const SwFrmFmt& rFollowFmt = pCurrent->GetMaster();

                bNewPageDesc = !IsPlausableSingleWordSection(rTitleFmt,
                    rFollowFmt);
            }
            pAktPageDesc = pCurrent;
        }
        else
        {
            const SwFrmFmt &rFmt = pCurrent->GetMaster();
            bNewPageDesc = FmtHdFtContainsChapterField(rFmt);
        }
    }
    return bNewPageDesc;
}

// Da WW nur Break-After ( Pagebreak und Sectionbreaks ) kennt, im SW aber
// Bagebreaks "vor" und "nach" und Pagedescs nur "vor" existieren, werden
// die Breaks 2* durchgeklimpert, naemlich vor und hinter jeder Zeile.
// Je nach BreakTyp werden sie vor oder nach der Zeile gesetzt.
// Es duerfen nur Funktionen gerufen werden, die nicht in den
// Ausgabebereich pO schreiben, da dieser nur einmal fuer CHP und PAP existiert
// und damit im falschen landen wuerden.
void SwWW8Writer::Out_SfxBreakItems(const SfxItemSet *pSet, const SwNode& rNd)
{
    bool bAllowOutputPageDesc = false;
    if (!bStyDef && !bOutKF && !bInWriteEscher && !bOutPageDescs)
        bAllowOutputPageDesc = true;

    if (!bAllowOutputPageDesc)
        return;

    bBreakBefore = true;

    bool bNewPageDesc = false;
    const SfxPoolItem* pItem=0;
    const SwFmtPageDesc *pPgDesc=0;

    //Output a sectionbreak if theres a new pagedesciptor.  otherwise output a
    //pagebreak if there is a pagebreak here, unless the new page (follow
    //style) is different to the current one, in which case plump for a
    //section.
    bool bBreakSet = false;
    if (pSet && pSet->Count())
    {
        if (SFX_ITEM_SET == pSet->GetItemState(RES_PAGEDESC, false, &pItem)
            && ((SwFmtPageDesc*)pItem)->GetRegisteredIn())
        {
            bBreakSet = true;
            bNewPageDesc = true;
            pPgDesc = (const SwFmtPageDesc*)pItem;
            pAktPageDesc = pPgDesc->GetPageDesc();
        }
        else if (SFX_ITEM_SET == pSet->GetItemState(RES_BREAK, false, &pItem))
        {
            bBreakSet = true;
            ASSERT(pAktPageDesc, "should not be possible");
            /*
             If because of this pagebreak the page desc following the page
             break is the follow style of the current page desc then output a
             section break using that style instead.  At least in those cases
             we end up with the same style in word and writer, nothing can be
             done when it happens when we get a new pagedesc because we
             overflow from the first page style.
            */
            if (pAktPageDesc)
                bNewPageDesc = SetAktPageDescFromNode(rNd);
            if (!bNewPageDesc)
                OutWW8_SwFmtBreak( *this, *pItem );
        }
    }

    /*
    #i9301#
    No explicit page break, lets see if the style had one and we've moved to a
    new page style because of it, if we have to then we take the opportunity to
    set the equivalent word section here. We *could* do it for every paragraph
    that moves onto a new page because of layout, but that would be insane.
    */
    bool bHackInBreak = false;
    if (!bBreakSet)
    {
        if (const SwCntntNode *pNd = rNd.GetCntntNode())
        {
            const SvxFmtBreakItem &rBreak =
                ItemGet<SvxFmtBreakItem>(*pNd, RES_BREAK);
            if (rBreak.GetBreak() == SVX_BREAK_PAGE_BEFORE)
                bHackInBreak = true;
        }
    }

    if (bHackInBreak)
    {
        ASSERT(pAktPageDesc, "should not be possible");
        if (pAktPageDesc)
            bNewPageDesc = SetAktPageDescFromNode(rNd);
    }

    if (bNewPageDesc && pAktPageDesc)
    {
        // Die PageDescs werden beim Auftreten von PageDesc-Attributen nur in
        // WW8Writer::pSepx mit der entsprechenden Position eingetragen.  Das
        // Aufbauen und die Ausgabe der am PageDesc haengenden Attribute und
        // Kopf/Fusszeilen passiert nach dem Haupttext und seinen Attributen.

        ULONG nFcPos = ReplaceCr(0x0c); // Page/Section-Break

        const SwSectionFmt *pFmt = 0;
        const SwSectionNode* pSect = rNd.FindSectionNode();
        if (pSect && CONTENT_SECTION == pSect->GetSection().GetType())
            pFmt = pSect->GetSection().GetFmt();

        // tatsaechlich wird hier NOCH NICHTS ausgegeben, sondern
        // nur die Merk-Arrays aCps, aSects entsprechend ergaenzt
        if (nFcPos)
        {
            const SwFmtLineNumber *pNItem = 0;
            if (pSet)
                pNItem = &(ItemGet<SwFmtLineNumber>(*pSet,RES_LINENUMBER));
            else if (const SwCntntNode *pNd = rNd.GetCntntNode())
                pNItem = &(ItemGet<SwFmtLineNumber>(*pNd,RES_LINENUMBER));
            ULONG nLnNm = pNItem ? pNItem->GetStartValue() : 0;

            if (pPgDesc)
                pSepx->AppendSep(Fc2Cp(nFcPos), *pPgDesc, rNd, pFmt, nLnNm);
            else
                pSepx->AppendSep(Fc2Cp(nFcPos), pAktPageDesc, rNd, pFmt, nLnNm);
        }
    }
    bBreakBefore = false;
}

void SwWW8Writer::CorrTabStopInSet(SfxItemSet& rSet, USHORT nAbsLeft)
{
    const SvxTabStopItem *pItem =
        sw::util::HasItem<SvxTabStopItem>(rSet, RES_PARATR_TABSTOP);

    if (pItem)
    {
        // dann muss das fuer die Ausgabe korrigiert werden
        SvxTabStopItem aTStop(*pItem);
        for(USHORT nCnt = 0; nCnt < aTStop.Count(); ++nCnt)
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

void SwWW8Writer::ExportOutlineNumbering(BYTE nLvl, const SwNumFmt &rNFmt,
    const SwFmt &rFmt)
{
    if (nLvl >= WW8ListManager::nMaxLevel)
        nLvl = WW8ListManager::nMaxLevel-1;

    if( bWrtWW8 )
    {
        // write sprmPOutLvl sprmPIlvl and sprmPIlfo
        SwWW8Writer::InsUInt16( *pO, 0x2640 );
        pO->Insert( nLvl, pO->Count() );
        SwWW8Writer::InsUInt16( *pO, 0x260a );
        pO->Insert( nLvl, pO->Count() );
        SwWW8Writer::InsUInt16( *pO, 0x460b );
        SwWW8Writer::InsUInt16( *pO, 1 + GetId(
                            *pDoc->GetOutlineNumRule() ) );
    }
    else
    {
        Out_SwNumLvl( nLvl );
        if (rNFmt.GetAbsLSpace())
        {
            SwNumFmt aNumFmt(rNFmt);
            const SvxLRSpaceItem& rLR =
                ItemGet<SvxLRSpaceItem>(rFmt, RES_LR_SPACE);
            aNumFmt.SetAbsLSpace(writer_cast<short>(
                    aNumFmt.GetAbsLSpace() + rLR.GetLeft()));
            Out_NumRuleAnld( *pDoc->GetOutlineNumRule(),
                            aNumFmt, nLvl );
        }
        else
            Out_NumRuleAnld( *pDoc->GetOutlineNumRule(),
                            rNFmt, nLvl );
    }
}

void SwWW8Writer::DisallowInheritingOutlineNumbering(const SwFmt &rFmt)
{
    //If there is no numbering on this fmt, but its parent was outline
    //numbered, then in writer this is no inheritied, but in word it would
    //be, so we must export "no numbering" and "body level" to make word
    //behave like writer (see #i25755)
    if (SFX_ITEM_SET != rFmt.GetItemState(RES_PARATR_NUMRULE, false))
    {
        if (const SwFmt *pParent = rFmt.DerivedFrom())
        {
            BYTE nLvl = ((const SwTxtFmtColl*)pParent)->GetOutlineLevel();
            if (MAXLEVEL > nLvl)
            {
                if (bWrtWW8)
                {
                    SwWW8Writer::InsUInt16(*pO, 0x2640);
                    pO->Insert(BYTE(9), pO->Count());
                    SwWW8Writer::InsUInt16(*pO, 0x460b);
                    SwWW8Writer::InsUInt16(*pO, 0);
                }
                /*whats the winword 6 way to do this ?*/
            }
        }
    }
}

void SwWW8Writer::Out_SwFmt(const SwFmt& rFmt, bool bPapFmt, bool bChpFmt,
    bool bFlyFmt)
{
    bool bCallOutSet = true;
    const SwModify* pOldMod = pOutFmtNode;
    pOutFmtNode = &rFmt;

    switch( rFmt.Which() )
    {
    case RES_CONDTXTFMTCOLL:
    case RES_TXTFMTCOLL:
        if( bPapFmt )
        {
            BYTE nLvl = ((const SwTxtFmtColl&)rFmt).GetOutlineLevel();
            if (MAXLEVEL > nLvl)
            {
                //if outline numbered
                // if Write StyleDefinition then write the OutlineRule
                const SwNumFmt& rNFmt = pDoc->GetOutlineNumRule()->Get(nLvl);
                if (bStyDef)
                    ExportOutlineNumbering(nLvl, rNFmt, rFmt);

                if (rNFmt.GetAbsLSpace())
                {
                    SfxItemSet aSet( rFmt.GetAttrSet() );
                    SvxLRSpaceItem aLR(
                        ItemGet<SvxLRSpaceItem>(aSet, RES_LR_SPACE));

                    aLR.SetTxtLeft( aLR.GetTxtLeft() + rNFmt.GetAbsLSpace() );
                    aLR.SetTxtFirstLineOfst( GetWordFirstLineOffset(rNFmt));

                    aSet.Put( aLR );
                    SwWW8Writer::CorrTabStopInSet( aSet, rNFmt.GetAbsLSpace() );
                    Out_SfxItemSet( aSet, bPapFmt, bChpFmt,
                        com::sun::star::i18n::ScriptType::LATIN);
                    bCallOutSet = false;
                }
            }
            else
            {
                //otherwise we might have to remove outline numbering from
                //what gets exported if the parent style was outline numbered
                if (bStyDef)
                    DisallowInheritingOutlineNumbering(rFmt);
            }
        }
        break;

    case RES_CHRFMT:
        break;
    case RES_FLYFRMFMT:
        if (bFlyFmt)
        {
            ASSERT(mpParentFrame, "No parent frame, all broken");

            if (mpParentFrame)
            {
                const SwFrmFmt &rFmt = mpParentFrame->GetFrmFmt();

                SfxItemSet aSet(pDoc->GetAttrPool(), RES_FRMATR_BEGIN,
                    RES_FRMATR_END-1);
                aSet.Set(rFmt.GetAttrSet());

                // Fly als Zeichen werden bei uns zu Absatz-gebundenen
                // jetzt den Abstand vom Absatz-Rand setzen
                if (pFlyOffset)
                {
                    aSet.Put(SwFmtHoriOrient(pFlyOffset->X()));
                    aSet.Put(SwFmtVertOrient(pFlyOffset->Y()));
                    SwFmtAnchor aAnchor(rFmt.GetAnchor());
                    aAnchor.SetType(eNewAnchorType);
                    aSet.Put(aAnchor);
                }

                if (SFX_ITEM_SET != aSet.GetItemState(RES_SURROUND))
                    aSet.Put(SwFmtSurround(SURROUND_NONE));

                bOutFlyFrmAttrs = true;
                //script doesn't matter if not exporting chp
                Out_SfxItemSet(aSet, true, false,
                    com::sun::star::i18n::ScriptType::LATIN);
                bOutFlyFrmAttrs = false;

                bCallOutSet = false;
            }
        }
        break;
    default:
        ASSERT( !this, "Was wird hier ausgegeben ??? " );
        break;
    }

    if( bCallOutSet )
        Out_SfxItemSet( rFmt.GetAttrSet(), bPapFmt, bChpFmt,
            com::sun::star::i18n::ScriptType::LATIN);
    pOutFmtNode = pOldMod;
}

bool SwWW8Writer::HasRefToObject(USHORT nTyp, const String* pNm, USHORT nSeqNo)
{
    bool bFnd = false;
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
            case REF_SETREFATTR:
                bFnd = (*pNm == rRFld.GetSetRefName()) ? true : false;
                break;
            case REF_FOOTNOTE:
            case REF_ENDNOTE:
                bFnd = (nSeqNo == rRFld.GetSeqNo()) ? true : false;
                break;
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
        case REF_SETREFATTR:
            sRet.APPEND_CONST_ASC( "Ref_" );
            sRet += *pNm;
            break;
        case REF_SEQUENCEFLD:
            break;      // ???
        case REF_BOOKMARK:
            sRet = *pNm;
            break;
        case REF_OUTLINE:
            break;      // ???
        case REF_FOOTNOTE:
            sRet.APPEND_CONST_ASC( "_RefF" );
            sRet += String::CreateFromInt32( nSeqNo );
            break;
        case REF_ENDNOTE:
            sRet.APPEND_CONST_ASC( "_RefE" );
            sRet += String::CreateFromInt32( nSeqNo );
            break;
    }
    return sRet;
}

//-----------------------------------------------------------------------
/*  */
/* File CHRATR.HXX: */

static Writer& OutWW8_SwBoldUSW(Writer& rWrt, BYTE nId, bool bVal)
{
    SwWW8Writer& rWrtWW8 = (SwWW8Writer&)rWrt;
    if( rWrtWW8.bWrtWW8 )
        rWrtWW8.InsUInt16(8 == nId ? 0x2a53 : 0x0835 + nId);
    else if (8 == nId )
        return rWrt;            // das Attribut gibt es im WW6 nicht
    else
        rWrtWW8.pO->Insert( 85 + nId, rWrtWW8.pO->Count() );
    rWrtWW8.pO->Insert( bVal ? 1 : 0, rWrtWW8.pO->Count() );
    return rWrt;
}

static Writer& OutWW8_SwBoldBiDiUSW(Writer& rWrt, BYTE nId, bool bVal)
{
    SwWW8Writer& rWrtWW8 = (SwWW8Writer&)rWrt;
    ASSERT(nId <= 1, "out of range");
    if (!rWrtWW8.bWrtWW8 || nId > 1)
        return rWrt;

    rWrtWW8.InsUInt16(0x085C + nId);
    rWrtWW8.pO->Insert(bVal ? 1 : 0, rWrtWW8.pO->Count());
    return rWrt;
}

static Writer& OutWW8_SwFont( Writer& rWrt, const SfxPoolItem& rHt )
{
    const SvxFontItem& rAttr = (const SvxFontItem&)rHt;
    SwWW8Writer& rWrtWW8 = (SwWW8Writer&)rWrt;
    USHORT nFontID= rWrtWW8.GetId(rAttr);

    if( rWrtWW8.bWrtWW8 )
    {
        rWrtWW8.InsUInt16( 0x4a4f );
        rWrtWW8.InsUInt16( nFontID );
        rWrtWW8.InsUInt16( 0x4a51 );
    }
    else
        rWrtWW8.pO->Insert( 93, rWrtWW8.pO->Count() );
    rWrtWW8.InsUInt16( nFontID );
    return rWrt;
}

static Writer& OutWW8_SwCTLFont(Writer& rWrt, const SfxPoolItem& rHt)
{
    SwWW8Writer& rWrtWW8 = (SwWW8Writer&)rWrt;
    if (rWrtWW8.bWrtWW8)
    {
        rWrtWW8.InsUInt16(0x4A5E);
        rWrtWW8.InsUInt16(rWrtWW8.GetId((const SvxFontItem&)rHt));
    }
    return rWrt;
}

static Writer& OutWW8_SwCJKFont( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwWW8Writer& rWrtWW8 = (SwWW8Writer&)rWrt;
    if( rWrtWW8.bWrtWW8 )
    {
#if 0
        if (nIdPreferred = RES_CHRATR_CJK_FONT)
        {
            rWrtWW8.InsUInt16( 0x286f );
            rWrtWW8.pO->Insert( 1, rWrtWW8.pO->Count() );
        }
#endif
        rWrtWW8.InsUInt16( 0x4a50 );
        rWrtWW8.InsUInt16(rWrtWW8.GetId((const SvxFontItem&)rHt));
    }
    return rWrt;
}

static Writer& OutWW8_SwBiDiWeight( Writer& rWrt, const SfxPoolItem& rHt )
{
    //Can only export in 8+, in 7- export as normal varient and expect that
    //upperlevel code has blocked exporting clobbering attributes
    SwWW8Writer& rWrtWW8 = (SwWW8Writer&)rWrt;
    if (rWrtWW8.bWrtWW8)
    {
        OutWW8_SwBoldBiDiUSW(rWrt, 0,
            WEIGHT_BOLD == ((const SvxWeightItem&)rHt).GetWeight());
    }
    else
    {
        OutWW8_SwBoldUSW(rWrt, 0,
            WEIGHT_BOLD == ((const SvxWeightItem&)rHt).GetWeight());
    }
    return rWrt;
}

static Writer& OutWW8_SwBiDiPosture( Writer& rWrt, const SfxPoolItem& rHt )
{
    //Can only export in 8+, in 7- export as normal varient and expect that
    //upperlevel code has blocked exporting clobbering attributes
    SwWW8Writer& rWrtWW8 = (SwWW8Writer&)rWrt;
    if (rWrtWW8.bWrtWW8)
    {
        OutWW8_SwBoldBiDiUSW( rWrt, 1,
            ITALIC_NONE != ((const SvxPostureItem&)rHt).GetPosture() );
    }
    else
    {
        OutWW8_SwBoldUSW( rWrt, 1,
            ITALIC_NONE != ((const SvxPostureItem&)rHt).GetPosture() );
    }
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
    return OutWW8_SwBoldUSW(rWrt, 3,
        ((const SvxContourItem&)rHt).GetValue() ? true : false);
}

static Writer& OutWW8_SwShadow( Writer& rWrt, const SfxPoolItem& rHt )
{
    return OutWW8_SwBoldUSW(rWrt, 4,
        ((const SvxShadowedItem&)rHt).GetValue() ? true : false);
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
    rWrtWW8.InsUInt16( rAttr.GetValue() ? 1 : 0 );
    return rWrt;
}

static Writer& OutWW8_SwAnimatedText( Writer& rWrt, const SfxPoolItem& rHt )
{
    const SvxBlinkItem& rAttr = (const SvxBlinkItem&)rHt;
    SwWW8Writer& rWrtWW8 = (SwWW8Writer&)rWrt;
    if( rWrtWW8.bWrtWW8 )
    {
        rWrtWW8.InsUInt16( 0x2859 );
        // At the moment the only animated text effect we support is blinking
        rWrtWW8.InsUInt16( rAttr.GetValue() ? 2 : 0 );
    }
    return rWrt;
}


static Writer& OutWW8_SwCrossedOut( Writer& rWrt, const SfxPoolItem& rHt )
{
    FontStrikeout eSt = ((const SvxCrossedOutItem&)rHt).GetStrikeout();
    if( STRIKEOUT_DOUBLE == eSt )
        return OutWW8_SwBoldUSW(rWrt, 8, true);
    if( STRIKEOUT_NONE != eSt )
        return OutWW8_SwBoldUSW(rWrt, 2, true);

    // dann auch beide ausschalten!
    OutWW8_SwBoldUSW(rWrt, 8, false);
    return OutWW8_SwBoldUSW(rWrt, 2, false);
}

static Writer& OutWW8_SwCaseMap( Writer& rWrt, const SfxPoolItem& rHt )
{
    USHORT eSt = ((const SvxCaseMapItem&)rHt).GetValue();
    switch (eSt)
    {
        case SVX_CASEMAP_KAPITAELCHEN:
            return OutWW8_SwBoldUSW(rWrt, 5, true);
        case SVX_CASEMAP_VERSALIEN:
            return OutWW8_SwBoldUSW(rWrt, 6, true);
        case SVX_CASEMAP_TITEL:
            //NO such feature in word
            break;
        default:
            // dann auch beide ausschalten!
            OutWW8_SwBoldUSW(rWrt, 5, false);
            return OutWW8_SwBoldUSW(rWrt, 6, false);
    }
    return rWrt;
}

static Writer& OutWW8_SvxCharHidden(Writer& rWrt, const SfxPoolItem& rHt )
{
    OutWW8_SwBoldUSW(rWrt, 7, (item_cast<SvxCharHiddenItem>(rHt)).GetValue());
    return rWrt;
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
    bool bWord = false;
    if (pItem)
        bWord = ((const SvxWordLineModeItem*)pItem)->GetValue() ? true : false;

        // WW95 - parameters:   0 = none,   1 = single, 2 = by Word,
        //                      3 = double, 4 = dotted, 5 = hidden
        // WW97 - additional parameters:
                            //  6 = thick,   7 = dash,       8 = dot(not used)
                            //  9 = dotdash 10 = dotdotdash, 11 = wave
    BYTE b = 0;
    switch (rAttr.GetUnderline())
    {
        case UNDERLINE_SINGLE:
            b = ( bWord ) ? 2 : 1;
            break;
        case UNDERLINE_BOLD:
            b = rWrtWW8.bWrtWW8 ?  6 : 1;
            break;
        case UNDERLINE_DOUBLE:
            b = 3;
            break;
        case UNDERLINE_DOTTED:
            b = 4;
            break;
        case UNDERLINE_DASH:
            b = rWrtWW8.bWrtWW8 ?  7 : 4;
            break;
        case UNDERLINE_DASHDOT:
            b = rWrtWW8.bWrtWW8 ?  9 : 4;
            break;
        case UNDERLINE_DASHDOTDOT:
            b = rWrtWW8.bWrtWW8 ? 10 : 4;
            break;
        case UNDERLINE_WAVE:
            b = rWrtWW8.bWrtWW8 ? 11 : 3;
            break;
        // ------------  new in WW2000  -------------------------------------
        case UNDERLINE_BOLDDOTTED:
            b = rWrtWW8.bWrtWW8 ? 20 : 4;
            break;
        case UNDERLINE_BOLDDASH:
            b = rWrtWW8.bWrtWW8 ? 23 : 4;
            break;
        case UNDERLINE_LONGDASH:
            b = rWrtWW8.bWrtWW8 ? 39 : 4;
            break;
        case UNDERLINE_BOLDLONGDASH:
            b = rWrtWW8.bWrtWW8 ? 55 : 4;
            break;
        case UNDERLINE_BOLDDASHDOT:
            b = rWrtWW8.bWrtWW8 ? 25 : 4;
            break;
        case UNDERLINE_BOLDDASHDOTDOT:
            b = rWrtWW8.bWrtWW8 ? 26 : 4;
            break;
        case UNDERLINE_BOLDWAVE:
            b = rWrtWW8.bWrtWW8 ? 27 : 3;
            break;
        case UNDERLINE_DOUBLEWAVE:
            b = rWrtWW8.bWrtWW8 ? 43 : 3;
            break;
        case UNDERLINE_NONE:
            b = 0;
            break;
        default:
            ASSERT(rAttr.GetUnderline() == UNDERLINE_NONE, "Unhandled underline type");
            break;
    }

    rWrtWW8.pO->Insert(b, rWrtWW8.pO->Count());
    return rWrt;
}

static Writer& OutWW8_SwLanguage( Writer& rWrt, const SfxPoolItem& rHt )
{
    USHORT nId = 0;
    SwWW8Writer& rWrtWW8 = (SwWW8Writer&)rWrt;
    if (rWrtWW8.bWrtWW8)
    {
        switch (rHt.Which())
        {
            case RES_CHRATR_LANGUAGE:
                nId = 0x486D;
                break;
            case RES_CHRATR_CJK_LANGUAGE:
                nId = 0x486E;
                break;
            case RES_CHRATR_CTL_LANGUAGE:
                nId = 0x485F;
                break;
        }
    }
    else
        nId = 97;

    if (nId)
    {
        if (rWrtWW8.bWrtWW8) // use sprmCRgLid0 rather than sprmCLid
            rWrtWW8.InsUInt16(nId);
        else
            rWrtWW8.pO->Insert((BYTE)nId, rWrtWW8.pO->Count());
        rWrtWW8.InsUInt16(((const SvxLanguageItem&)rHt).GetLanguage());

        //unknown as to exactly why, but this seems to shadow the other
        //paramater in word 2000 and without it spellchecking doesn't work
        if (nId == 0x486D)
        {
            rWrtWW8.InsUInt16(0x4873);
            rWrtWW8.InsUInt16(((const SvxLanguageItem&)rHt).GetLanguage());
        }
        else if (nId == 0x485F)
        {
            rWrtWW8.InsUInt16(0x4874);
            rWrtWW8.InsUInt16(((const SvxLanguageItem&)rHt).GetLanguage());
        }

    }
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
            rWrtWW8.InsUInt16(
                msword_cast<sal_uInt16>((nHeight * nProp + 500 ) / 1000));

        }
    }
    return rWrt;
}

static Writer& OutWW8_SwSize( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwWW8Writer& rWrtWW8 = (SwWW8Writer&)rWrt;
    USHORT nId = 0;
    if (rWrtWW8.bWrtWW8)
    {
        switch (rHt.Which())
        {
            case RES_CHRATR_FONTSIZE:
            case RES_CHRATR_CJK_FONTSIZE:
                nId = 0x4A43;
                break;
            case RES_CHRATR_CTL_FONTSIZE:
                nId = 0x4A61;
                break;
        }
    }
    else
        nId = 99;

    if( nId )
    {
        if( rWrtWW8.bWrtWW8 )
            rWrtWW8.InsUInt16( nId );
        else
            rWrtWW8.pO->Insert( (BYTE)nId, rWrtWW8.pO->Count() );

        const SvxFontHeightItem& rAttr = (const SvxFontHeightItem&)rHt;
        rWrtWW8.InsUInt16( (UINT16)(( rAttr.GetHeight() + 5 ) / 10 ) );
    }
    return rWrt;
}

static Writer& OutWW8_ScaleWidth( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwWW8Writer& rWrtWW8 = (SwWW8Writer&)rWrt;
    if( rWrtWW8.bWrtWW8 )
    {
        rWrtWW8.InsUInt16( 0x4852 );
        rWrtWW8.InsUInt16( ((SvxCharScaleWidthItem&)rHt).GetValue() );
    }
    return rWrt;
}

static Writer& OutWW8_Relief( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwWW8Writer& rWrtWW8 = (SwWW8Writer&)rWrt;
    if( rWrtWW8.bWrtWW8 )
    {
        const SvxCharReliefItem& rAttr = (const SvxCharReliefItem&)rHt;
        USHORT nId;
        switch ( rAttr.GetValue() )
        {
        case RELIEF_EMBOSSED:   nId = 0x858;    break;
        case RELIEF_ENGRAVED:   nId = 0x854;    break;
        default:                nId = 0;        break;
        }

        if( nId )
        {
            rWrtWW8.InsUInt16( nId );
            rWrtWW8.pO->Insert( (BYTE)0x81, rWrtWW8.pO->Count() );
        }
        else
        {
            // switch both flags off
            rWrtWW8.InsUInt16( 0x858 );
            rWrtWW8.pO->Insert( (BYTE)0x0, rWrtWW8.pO->Count() );
            rWrtWW8.InsUInt16( 0x854 );
            rWrtWW8.pO->Insert( (BYTE)0x0, rWrtWW8.pO->Count() );
        }
    }
    return rWrt;
}


static Writer& OutWW8_CharRotate( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwWW8Writer& rWrtWW8 = (SwWW8Writer&)rWrt;
    if( rWrtWW8.bWrtWW8 )
    {
        const SvxCharRotateItem& rAttr = (const SvxCharRotateItem&)rHt;

        rWrtWW8.InsUInt16( 0xCA78 );
        rWrtWW8.pO->Insert( (BYTE)0x06, rWrtWW8.pO->Count() ); //len 6
        rWrtWW8.pO->Insert( (BYTE)0x01, rWrtWW8.pO->Count() );

        rWrtWW8.InsUInt16( rAttr.IsFitToLine() ? 1 : 0 );
        static const BYTE aZeroArr[ 3 ] = { 0, 0, 0 };
        rWrtWW8.pO->Insert( aZeroArr, 3, rWrtWW8.pO->Count() );
    }
    return rWrt;
}


static Writer& OutWW8_EmphasisMark( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwWW8Writer& rWrtWW8 = (SwWW8Writer&)rWrt;
    if( rWrtWW8.bWrtWW8 )
    {
        BYTE nVal;
        switch ( ((const SvxEmphasisMarkItem&)rHt).GetValue() )
        {
        case EMPHASISMARK_NONE:             nVal = 0;   break;
        case EMPHASISMARK_SIDE_DOTS:        nVal = 2;   break;
        case EMPHASISMARK_CIRCLE_ABOVE:     nVal = 3;   break;
        case EMPHASISMARK_DOTS_BELOW:       nVal = 4;   break;
//      case 1:
        default:                            nVal = 1;   break;
        }

        rWrtWW8.InsUInt16( 0x2A34 );
        rWrtWW8.pO->Insert( nVal, rWrtWW8.pO->Count() );
    }
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
    case COL_AUTO:          nCol = 0;   break;

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
bool SwWW8Writer::TransBrush(const Color& rCol, WW8_SHD& rShd)
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

sal_uInt32 SuitableBGColor(sal_uInt32 nIn)
{
    if (nIn == COL_AUTO)
        return 0xFF000000;
    return wwUtility::RGBToBGR(nIn);
}

static Writer& OutWW8_SwColor( Writer& rWrt, const SfxPoolItem& rHt )
{
    const SvxColorItem& rAttr = (const SvxColorItem&)rHt;
    SwWW8Writer& rWrtWW8 = (SwWW8Writer&)rWrt;
    if (rWrtWW8.bWrtWW8)
        rWrtWW8.InsUInt16(0x2A42);
    else
        rWrtWW8.pO->Insert(98, rWrtWW8.pO->Count());

    BYTE nColour = rWrtWW8.TransCol(rAttr.GetValue());
    rWrtWW8.pO->Insert(nColour, rWrtWW8.pO->Count());

    if (rWrtWW8.bWrtWW8 && nColour)
    {
        rWrtWW8.InsUInt16(0x6870);
        rWrtWW8.InsUInt32(wwUtility::RGBToBGR(rAttr.GetValue().GetColor()));
    }
    return rWrt;
}

static Writer& OutWW8_SwFmtCharBackground( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwWW8Writer& rWW8Wrt = (SwWW8Writer&)rWrt;

    if( rWW8Wrt.bWrtWW8 )   // nur WW8 kann ZeichenHintergrund
    {
        const SvxBrushItem& rBack = (const SvxBrushItem&)rHt;
        WW8_SHD aSHD;

        rWW8Wrt.TransBrush(rBack.GetColor(), aSHD);
        // sprmCShd
        rWW8Wrt.InsUInt16( 0x4866 );
        rWW8Wrt.InsUInt16( aSHD.GetValue() );

        //Quite a few unknowns, some might be transparency or something
        //of that nature...
        rWW8Wrt.InsUInt16(0xCA71);
        rWW8Wrt.pO->Insert(10, rWW8Wrt.pO->Count());
        rWW8Wrt.InsUInt32(0xFF000000);
        rWW8Wrt.InsUInt32(SuitableBGColor(rBack.GetColor().GetColor()));
        rWW8Wrt.InsUInt16(0x0000);
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
    rWrt.GetCurrentItems(aItems);

    if (c == 0x13)
        rWrt.pChpPlc->AppendFkpEntry(rWrt.Strm().Tell());
    else
        rWrt.pChpPlc->AppendFkpEntry(rWrt.Strm().Tell(), aItems.Count(),
            aItems.GetData());

    rWrt.WriteChar(c);

    // fSpec-Attribut true
    if( rWrt.bWrtWW8 )
        SwWW8Writer::InsUInt16( aItems, 0x855 );
    else
        aItems.Insert( 117, aItems.Count() );
    aItems.Insert( 1, aItems.Count() );

    rWrt.pChpPlc->AppendFkpEntry(rWrt.Strm().Tell(), aItems.Count(),
        aItems.GetData());
}

String lcl_GetExpandedField(const SwField &rFld)
{
    String sRet(rFld.Expand());

    //replace LF 0x0A with VT 0x0B
    sRet.SearchAndReplaceAll(0x0A, 0x0B);

    return sRet;
}

void SwWW8Writer::OutField(const SwField* pFld, ww::eField eFldType,
    const String& rFldCmd, BYTE nMode)
{
    bool bUnicode = IsUnicode();
    WW8_WrPlcFld* pFldP;
    switch (nTxtTyp)
    {
        case TXT_MAINTEXT:
            pFldP = pFldMain;
            break;
        case TXT_HDFT:
            pFldP = pFldHdFt;
            break;
        case TXT_FTN:
            pFldP = pFldFtn;
            break;
        case TXT_EDN:
            pFldP = pFldEdn;
            break;
        case TXT_TXTBOX:
            pFldP = pFldTxtBxs;
            break;
        case TXT_HFTXTBOX:
            pFldP = pFldHFTxtBxs;
            break;
        default:
            ASSERT( !this, "was ist das fuer ein SubDoc-Type?" );
            return;
    }

    if (WRITEFIELD_START & nMode)
    {
        BYTE aFld13[2] = { 0x13, 0x00 };  // will change
        //#i3958#, Needed to make this field work correctly in Word 2000
        if (eFldType == ww::eSHAPE)
            aFld13[0] |= 0x80;
        aFld13[1] = eFldType;                           // Typ nachtragen
        pFldP->Append( Fc2Cp( Strm().Tell() ), aFld13 );
        InsertSpecialChar( *this, 0x13 );
    }
    if (WRITEFIELD_CMD_START & nMode)
    {
        if (bUnicode)
            SwWW8Writer::WriteString16(Strm(), rFldCmd, false);
        else
        {
            SwWW8Writer::WriteString8(Strm(), rFldCmd, false,
                RTL_TEXTENCODING_MS_1252);
        }
    }
    if (WRITEFIELD_CMD_END & nMode)
    {
        static const BYTE aFld14[2] = { 0x14, 0xff };
        pFldP->Append( Fc2Cp( Strm().Tell() ), aFld14 );
        InsertSpecialChar( *this, 0x14 );
    }
    if (WRITEFIELD_END & nMode)
    {
        String sOut;
        if( pFld )
            sOut = lcl_GetExpandedField(*pFld);
        else
            sOut = rFldCmd;
        if( sOut.Len() )
        {
            if( bUnicode )
                SwWW8Writer::WriteString16(Strm(), sOut, false);
            else
            {
                SwWW8Writer::WriteString8(Strm(), sOut, false,
                    RTL_TEXTENCODING_MS_1252);
            }
        }
    }
    if (WRITEFIELD_CLOSE & nMode)
    {
        static const BYTE aFld15[2] = { 0x15, 0x80 };
        pFldP->Append( Fc2Cp( Strm().Tell() ), aFld15 );
        InsertSpecialChar( *this, 0x15 );
    }
}

void SwWW8Writer::StartCommentOutput(const String& rName)
{
    String sStr(FieldString(ww::eQUOTE));
    sStr.APPEND_CONST_ASC("[");
    sStr += rName;
    sStr.APPEND_CONST_ASC("] ");
    OutField(0, ww::eQUOTE, sStr, WRITEFIELD_START | WRITEFIELD_CMD_START);
}

void SwWW8Writer::EndCommentOutput(const String& rName)
{
    String sStr(CREATE_CONST_ASC(" ["));
    sStr += rName;
    sStr.APPEND_CONST_ASC("] ");
    OutField(0, ww::eQUOTE, sStr, WRITEFIELD_CMD_END | WRITEFIELD_END |
        WRITEFIELD_CLOSE);
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

    // #i21237#
    SwFormTokens aPattern = rForm.GetPattern(nLvl);
    SwFormTokens::iterator aIt = aPattern.begin();
    bool bPgNumFnd = false;
    FormTokenType eTType;
    // #i21237#
    while( TOKEN_END != ( eTType = (++aIt)->eTokenType ) && !bPgNumFnd )
    {
        switch( eTType )
        {
        case TOKEN_PAGE_NUMS:
            bPgNumFnd = true;
            break;

        case TOKEN_TAB_STOP:
            nRet = 2;
            break;
        case TOKEN_TEXT:
            nRet = 3;
            rText = aIt->sText.Copy( 0, 5 ); // #i21237#
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

bool lcl_IsHyperlinked(const SwForm& rForm, USHORT nTOXLvl)
{
    for (USHORT nI = 1; nI < nTOXLvl; ++nI)
    {
        // #i21237#
        SwFormTokens aPattern = rForm.GetPattern(nI);
        SwFormTokens::iterator aIt = aPattern.begin();

        FormTokenType eTType;
        // #i21237#
        while (TOKEN_END != (eTType = (++aIt)->eTokenType))
        {
            switch (eTType)
            {
                case TOKEN_LINK_START:
                case TOKEN_LINK_END:
                    return true;
                    break;
            }
        }
    }
    return false;
}

void SwWW8Writer::StartTOX( const SwSection& rSect )
{
    if (const SwTOXBase* pTOX = rSect.GetTOXBase())
    {
        static const sal_Char sEntryEnd[] = "\" ";

        ww::eField eCode = ww::eTOC;
        String sStr;
        switch (pTOX->GetType())
        {
        case TOX_INDEX:
            eCode = ww::eINDEX;
            sStr = FieldString(eCode);

            if (pTOX->GetTOXForm().IsCommaSeparated())
                sStr.APPEND_CONST_ASC("\\r ");

            if (TOI_ALPHA_DELIMITTER & pTOX->GetOptions())
                sStr.APPEND_CONST_ASC("\\h \"A\" ");

            {
                String aFillTxt;
                for (BYTE n = 1; n <= 3; ++n)
                {
                    String aTxt;
                    int nRet = ::lcl_CheckForm(pTOX->GetTOXForm(), n, aTxt);

                    if( 3 == nRet )
                        aFillTxt = aTxt;
                    else if ((4 == nRet) || (2 == nRet)) //#109414#
                        aFillTxt = '\t';
                    else
                        aFillTxt.Erase();
                }
                sStr.APPEND_CONST_ASC("\\e \"");
                sStr += aFillTxt;
                sStr.AppendAscii(sEntryEnd);
            }
            break;

//      case TOX_AUTHORITIES:   eCode = eTOA; sStr = ???; break;

        case TOX_ILLUSTRATIONS:
        case TOX_OBJECTS:
        case TOX_TABLES:
            if (!pTOX->IsFromObjectNames())
            {
                sStr = FieldString(eCode);

                sStr.APPEND_CONST_ASC("\\c \"");
                sStr += pTOX->GetSequenceName();
                sStr.AppendAscii(sEntryEnd);

                String aTxt;
                int nRet = ::lcl_CheckForm( pTOX->GetTOXForm(), 1, aTxt );
                if (1 == nRet)
                    sStr.APPEND_CONST_ASC("\\n ");
                else if( 3 == nRet || 4 == nRet )
                {
                    sStr.APPEND_CONST_ASC("\\p \"");
                    sStr += aTxt;
                    sStr.AppendAscii(sEntryEnd);
                }
            }
            break;

//      case TOX_USER:
//      case TOX_CONTENT:
        default:
            {
                sStr = FieldString(eCode);

                String sTOption;
                USHORT n, nTOXLvl = pTOX->GetLevel();
                if( !nTOXLvl )
                    ++nTOXLvl;

                if( TOX_MARK & pTOX->GetCreateType() )
                {
                    sStr.APPEND_CONST_ASC( "\\f " );

                    if( TOX_USER == pTOX->GetType() )
                    {
                         sStr += '\"';
                         sStr += (sal_Char)('A' + GetId( *pTOX->GetTOXType()));
                         sStr.AppendAscii( sEntryEnd );
                    }
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
                        if (nTmpLvl > WW8ListManager::nMaxLevel)
                            nTmpLvl = WW8ListManager::nMaxLevel;

                        sStr.APPEND_CONST_ASC( "\\o \"1-" );
                        sStr += String::CreateFromInt32( nTmpLvl );
                        sStr.AppendAscii(sEntryEnd);

                    }

                    if( nLvl != nMinLvl )
                    {
                        // collect this templates into the \t otion
                        for( n = rColls.Count(); n;)
                        {
                            pColl = rColls[--n];
                            nLvl =  pColl->GetOutlineLevel();
                            if (MAXLEVEL > nLvl && nMinLvl <= nLvl)
                            {
                                if( sTOption.Len() )
                                    sTOption += ';';
                                (( sTOption += pColl->GetName() ) += ';' )
                                        += String::CreateFromInt32( nLvl + 1 );
                            }
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
                    bool bFirstFillTxt = true, bOnlyText = true;
                    for( n = 0; n < nTOXLvl; ++n )
                    {
                        String aTxt;
                        int nRet = ::lcl_CheckForm( pTOX->GetTOXForm(),
                                                    n+1, aTxt );
                        if( 1 == nRet )
                        {
                            bOnlyText = false;
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
                                bFirstFillTxt = false;
                            }
                        }
                    }
                    if( MAXLEVEL != nNoPgStt )
                    {
                        if (WW8ListManager::nMaxLevel < nNoPgEnd)
                            nNoPgEnd = WW8ListManager::nMaxLevel;
                        sStr.APPEND_CONST_ASC( "\\n " );
                        sStr += String::CreateFromInt32( nNoPgStt );
                        sStr += '-';
                        sStr += String::CreateFromInt32( nNoPgEnd  );
                        sStr += ' ';
                    }
                    if( bOnlyText )
                    {
                        sStr.APPEND_CONST_ASC( "\\p \"" );
                        sStr += aFillTxt;
                        sStr.AppendAscii(sEntryEnd);
                    }
                }

                if( sTOption.Len() )
                {
                    sStr.APPEND_CONST_ASC( "\\t \"" );
                    sStr += sTOption;
                    sStr.AppendAscii(sEntryEnd);
                }

                if (lcl_IsHyperlinked(pTOX->GetTOXForm(), nTOXLvl))
                    sStr.APPEND_CONST_ASC("\\h");
            }
            break;
        }

        if( sStr.Len() )
        {
            bInWriteTOX = true;
            OutField(0, eCode, sStr, WRITEFIELD_START | WRITEFIELD_CMD_START |
                WRITEFIELD_CMD_END);
        }
    }
    bStartTOX = false;
}

void SwWW8Writer::EndTOX( const SwSection& rSect )
{
    const SwTOXBase* pTOX = rSect.GetTOXBase();
    if( pTOX )
    {
        ww::eField eCode = TOX_INDEX == pTOX->GetType() ? ww::eINDEX : ww::eTOC;
        OutField(0, eCode, aEmptyStr, WRITEFIELD_CLOSE);
    }
    bInWriteTOX = false;
}

void SwapQuotesInField(String &rFmt)
{
    //Swap unescaped " and ' with ' and "
    xub_StrLen nLen = rFmt.Len();
    for (xub_StrLen nI = 0; nI < nLen; ++nI)
    {
        if ((rFmt.GetChar(nI) == '\"') && (!nI || rFmt.GetChar(nI-1) != '\\'))
            rFmt.SetChar(nI, '\'');
        else if ((rFmt.GetChar(nI) == '\'') && (!nI || rFmt.GetChar(nI-1) != '\\'))
            rFmt.SetChar(nI, '\"');
    }
}

// GetDatePara, GetTimePara, GetNumberPara modifizieren die String-Ref
// Es werden die deutschen Format-Spezifier benutzt, da im FIB auch dt. als
// Creator angegeben ist.
bool SwWW8Writer::GetNumberFmt(const SwField& rFld, String& rStr)
{
    bool bHasFmt = false;
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
            rKeyMap[ NF_KEY_AMPM    ].ASSIGN_CONST_ASC( "AM/PM" );
            rKeyMap[ NF_KEY_AP      ].ASSIGN_CONST_ASC( "A/P" );
            rKeyMap[ NF_KEY_MI      ].ASSIGN_CONST_ASC( "m" );
            rKeyMap[ NF_KEY_MMI     ].ASSIGN_CONST_ASC( "mm" );
            rKeyMap[ NF_KEY_M       ].ASSIGN_CONST_ASC( "M" );
            rKeyMap[ NF_KEY_MM      ].ASSIGN_CONST_ASC( "MM" );
            rKeyMap[ NF_KEY_MMM     ].ASSIGN_CONST_ASC( "MMM" );
            rKeyMap[ NF_KEY_MMMM    ].ASSIGN_CONST_ASC( "MMMM" );
            rKeyMap[ NF_KEY_MMMMM   ].ASSIGN_CONST_ASC( "MMMMM" );
            rKeyMap[ NF_KEY_H       ].ASSIGN_CONST_ASC( "H" );
            rKeyMap[ NF_KEY_HH      ].ASSIGN_CONST_ASC( "HH" );
            rKeyMap[ NF_KEY_S       ].ASSIGN_CONST_ASC( "s" );
            rKeyMap[ NF_KEY_SS      ].ASSIGN_CONST_ASC( "ss" );
//          aKeyMap[ NF_KEY_Q,
//          aKeyMap[ NF_KEY_QQ,
            rKeyMap[ NF_KEY_D       ].ASSIGN_CONST_ASC( "t" );
            rKeyMap[ NF_KEY_DD      ].ASSIGN_CONST_ASC( "tt" );
            rKeyMap[ NF_KEY_DDD     ].ASSIGN_CONST_ASC( "ttt" );
            rKeyMap[ NF_KEY_DDDD    ].ASSIGN_CONST_ASC( "tttt" );
            rKeyMap[ NF_KEY_YY      ].ASSIGN_CONST_ASC( "jj" );
            rKeyMap[ NF_KEY_YYYY    ].ASSIGN_CONST_ASC( "jjjj" );
            rKeyMap[ NF_KEY_NN      ].ASSIGN_CONST_ASC( "ttt" );
            rKeyMap[ NF_KEY_NNNN    ].ASSIGN_CONST_ASC( "tttt" );
//          aKeyMap[ NF_KEY_CCC,
//          aKeyMap[ NF_KEY_GENERAL,
//          aKeyMap[ NF_KEY_NNN,
//          aKeyMap[ NF_KEY_WW,
//          aKeyMap[ NF_KEY_QUARTER,
//          aKeyMap[ NF_KEY_TRUE,
//          aKeyMap[ NF_KEY_FALSE,
//          aKeyMap[ NF_KEY_BOOLEAN,
            rKeyMap[ NF_KEY_AAA     ].ASSIGN_CONST_ASC( "aaa" );
            rKeyMap[ NF_KEY_AAAA    ].ASSIGN_CONST_ASC( "aaaa" );
            rKeyMap[ NF_KEY_EC      ].ASSIGN_CONST_ASC( "e" );
            rKeyMap[ NF_KEY_EEC     ].ASSIGN_CONST_ASC( "ee" );
            rKeyMap[ NF_KEY_G       ].ASSIGN_CONST_ASC( "g" );
            rKeyMap[ NF_KEY_GG      ].ASSIGN_CONST_ASC( "gg" );
            rKeyMap[ NF_KEY_GGG     ].ASSIGN_CONST_ASC( "ggg" );
        }

        String sFmt(pNumFmt->GetMappedFormatstring(*(NfKeywordTable*)pKeyMap,
            aLocDat));
        if (sFmt.Len())
        {
            SwapQuotesInField(sFmt);

            rStr.APPEND_CONST_ASC( "\\@\"" );
            rStr += sFmt;
            rStr.APPEND_CONST_ASC( "\" " );
            bHasFmt = true;
        }
    }
    return bHasFmt;
}

void WW8_GetNumberPara( String& rStr, const SwField& rFld )
{
    switch(rFld.GetFormat())
    {
        case SVX_NUM_CHARS_UPPER_LETTER:
        case SVX_NUM_CHARS_UPPER_LETTER_N:
            rStr.APPEND_CONST_ASC( "\\*ALPHABETIC ");
            break;
        case SVX_NUM_CHARS_LOWER_LETTER:
        case SVX_NUM_CHARS_LOWER_LETTER_N:
            rStr.APPEND_CONST_ASC("\\*alphabetic ");
            break;
        case SVX_NUM_ROMAN_UPPER:
            rStr.APPEND_CONST_ASC("\\*ROMAN ");
            break;
        case SVX_NUM_ROMAN_LOWER:
            rStr.APPEND_CONST_ASC("\\*roman ");
            break;
        default:
            ASSERT(rFld.GetFormat() == SVX_NUM_ARABIC,
                "Unknown numbering type exported as default\n");
        case SVX_NUM_ARABIC:
            rStr.APPEND_CONST_ASC("\\*Arabic ");
            break;
        case SVX_NUM_PAGEDESC:
            //Nothing, use word's default
            break;
    }
}

void SwWW8Writer::WritePostItBegin( WW8Bytes* pOut )
{
    BYTE aArr[ 3 ];
    BYTE* pArr = aArr;

    // sprmCFSpec true
    if( bWrtWW8 )
        Set_UInt16( pArr, 0x855 );
    else
        Set_UInt8( pArr, 117 );
    Set_UInt8( pArr, 1 );

    pChpPlc->AppendFkpEntry( Strm().Tell() );
    WriteChar( 0x05 );              // Annotation reference

    if( pOut )
        pOut->Insert( aArr, pArr - aArr, pOut->Count() );
    else
        pChpPlc->AppendFkpEntry( Strm().Tell(), pArr - aArr, aArr );
}

String FieldString(ww::eField eIndex)
{
    String sRet(CREATE_CONST_ASC("  "));
    if (const char *pField = ww::GetEnglishFieldName(eIndex))
        sRet.InsertAscii(pField, 1);
    return sRet;
}

void OutWW8_RefField(SwWW8Writer& rWW8Wrt, const SwField &rFld,
    const String &rRef)
{
    String sStr(FieldString(ww::eREF));
    sStr.APPEND_CONST_ASC("\"");
    sStr += rRef;
    sStr.APPEND_CONST_ASC( "\" " );
    rWW8Wrt.OutField(&rFld, ww::eREF, sStr, WRITEFIELD_START |
        WRITEFIELD_CMD_START | WRITEFIELD_CMD_END);
    String sVar = lcl_GetExpandedField(rFld);
    if (sVar.Len())
    {
        if (rWW8Wrt.IsUnicode())
            SwWW8Writer::WriteString16(rWW8Wrt.Strm(), sVar, false);
        else
        {
            SwWW8Writer::WriteString8(rWW8Wrt.Strm(), sVar, false,
                RTL_TEXTENCODING_MS_1252);
        }
    }
    rWW8Wrt.OutField(&rFld, ww::eREF, sStr, WRITEFIELD_CLOSE);
}

void WriteExpand(SwWW8Writer& rWW8Wrt, const SwField &rFld)
{
    String sExpand(lcl_GetExpandedField(rFld));
    if (rWW8Wrt.IsUnicode())
        SwWW8Writer::WriteString16(rWW8Wrt.Strm(), sExpand, false);
    else
    {
        SwWW8Writer::WriteString8(rWW8Wrt.Strm(), sExpand, false,
            RTL_TEXTENCODING_MS_1252);
    }
}

static Writer& OutWW8_SwField( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwWW8Writer& rWW8Wrt = (SwWW8Writer&)rWrt;
    const SwFmtFld& rFld = (SwFmtFld&)rHt;
    const SwField* pFld = rFld.GetFld();
    String sStr;        // fuer optionale Parameter
    bool bWriteExpand = false;
    USHORT nSubType = pFld->GetSubType();

    switch (pFld->GetTyp()->Which())
    {
    case RES_GETEXPFLD:
        if (nSubType == GSE_STRING)
        {
            const SwGetExpField *pGet=(const SwGetExpField*)(pFld);
            OutWW8_RefField(rWW8Wrt, *pGet, pGet->GetFormula());
        }
        else
            bWriteExpand = true;
        break;
    case RES_SETEXPFLD:
        if (GSE_SEQ == nSubType)
        {
            sStr = FieldString(ww::eSEQ);
            sStr.APPEND_CONST_ASC("\"");
            sStr += pFld->GetTyp()->GetName();
            sStr.APPEND_CONST_ASC( "\" " );

            ::WW8_GetNumberPara( sStr, *pFld );
            rWW8Wrt.OutField(pFld, ww::eSEQ, sStr);
        }
        else if (nSubType & GSE_STRING)
        {
            bool bShowAsWell = false;
            ww::eField eFieldNo;
            const SwSetExpField *pSet=(const SwSetExpField*)(pFld);
            const String &rVar = pSet->GetPar2();
            if (pSet->GetInputFlag())
            {
                sStr = FieldString(ww::eASK);
                sStr.APPEND_CONST_ASC("\"");
                sStr += pSet->GetPar1();
                sStr.APPEND_CONST_ASC( "\" " );
                sStr += pSet->GetPromptText();
                sStr.APPEND_CONST_ASC( " \\d " );
                sStr += rVar;
                eFieldNo = ww::eASK;
            }
            else
            {
                sStr = FieldString(ww::eSET);
                sStr += pSet->GetPar1();
                sStr.APPEND_CONST_ASC(" \"");
                sStr += rVar;
                sStr.APPEND_CONST_ASC("\" ");
                eFieldNo = ww::eSET;
                bShowAsWell = (nSubType & SUB_INVISIBLE) ? false : true;
            }

            ULONG nFrom = rWW8Wrt.Fc2Cp(rWrt.Strm().Tell());

            rWW8Wrt.OutField(pFld, eFieldNo, sStr, WRITEFIELD_START |
                WRITEFIELD_CMD_START | WRITEFIELD_CMD_END);

            /*
            Is there a bookmark at the start position of this field, if so
            move it to the 0x14 of the result of the field.  This is what word
            does. MoveFieldBookmarks moves any bookmarks at this position to
            the beginning of the field result, and marks the bookmark as a
            fieldbookmark which is to be ended before the field end mark
            instead of after it like a normal bookmark.
            */
            rWW8Wrt.MoveFieldBookmarks(nFrom,rWW8Wrt.Fc2Cp(rWrt.Strm().Tell()));

            if (rVar.Len())
            {
                if (rWW8Wrt.IsUnicode())
                    SwWW8Writer::WriteString16(rWrt.Strm(), rVar, false);
                else
                {
                    SwWW8Writer::WriteString8(rWrt.Strm(), rVar, false,
                        RTL_TEXTENCODING_MS_1252);
                }
            }
            rWW8Wrt.OutField(pFld, eFieldNo, sStr, WRITEFIELD_CLOSE);

            if (bShowAsWell)
                OutWW8_RefField(rWW8Wrt, *pSet, pSet->GetPar1());
        }
        else
            bWriteExpand = true;
        break;
    case RES_PAGENUMBERFLD:
        sStr = FieldString(ww::ePAGE);
        ::WW8_GetNumberPara(sStr, *pFld);
        rWW8Wrt.OutField(pFld, ww::ePAGE, sStr);
        break;
    case RES_FILENAMEFLD:
        sStr = FieldString(ww::eFILENAME);
        if (pFld->GetFormat() == FF_PATHNAME)
            sStr.APPEND_CONST_ASC("\\p ");
        rWW8Wrt.OutField(pFld, ww::eFILENAME, sStr);
        break;
    case RES_DBNAMEFLD:
        {
            sStr = FieldString(ww::eDATABASE);
            SwDBData aData = rWrt.pDoc->GetDBData();
            sStr += String(aData.sDataSource);
            sStr += DB_DELIM;
            sStr += String(aData.sCommand);
            rWW8Wrt.OutField(pFld, ww::eDATABASE, sStr);
        }
        break;
    case RES_AUTHORFLD:
        {
            ww::eField eFld =
                (AF_SHORTCUT & nSubType ? ww::eUSERINITIALS : ww::eUSERNAME);
            rWW8Wrt.OutField(pFld, eFld, FieldString(eFld));
        }
        break;
    case RES_TEMPLNAMEFLD:
        rWW8Wrt.OutField(pFld, ww::eTEMPLATE, FieldString(ww::eTEMPLATE));
        break;
    case RES_DOCINFOFLD:    // Last printed, last edited,...
        if( DI_SUB_FIXED & nSubType )
            bWriteExpand = true;
        else
        {
            ww::eField eFld(ww::eNONE);
            switch (0xff & nSubType)
            {
                case DI_TITEL:
                    eFld = ww::eTITLE;
                    break;
                case DI_THEMA:
                    eFld = ww::eSUBJECT;
                    break;
                case DI_KEYS:
                    eFld = ww::eKEYWORDS;
                    break;
                case DI_COMMENT:
                    eFld = ww::eCOMMENTS;
                    break;
                case DI_DOCNO:
                    eFld = ww::eREVNUM;
                    break;
                case DI_CREATE:
                    if (DI_SUB_AUTHOR == (nSubType & ~DI_SUB_AUTHOR))
                        eFld = ww::eAUTHOR;
                    else if (rWW8Wrt.GetNumberFmt(*pFld, sStr))
                        eFld = ww::eCREATEDATE;
                    break;

                case DI_CHANGE:
                    if (DI_SUB_AUTHOR == (nSubType & ~DI_SUB_AUTHOR))
                        eFld = ww::eLASTSAVEDBY;
                    else if (rWW8Wrt.GetNumberFmt(*pFld, sStr))
                        eFld = ww::eSAVEDATE;
                    break;

                case DI_PRINT:
                    if (DI_SUB_AUTHOR != (nSubType & ~DI_SUB_AUTHOR) &&
                        rWW8Wrt.GetNumberFmt(*pFld, sStr))
                        eFld = ww::ePRINTDATE;
                    break;
                case DI_EDIT:
                    if( DI_SUB_AUTHOR != (nSubType & ~DI_SUB_AUTHOR ) &&
                        rWW8Wrt.GetNumberFmt( *pFld, sStr ))
                        eFld = ww::eSAVEDATE;
                    break;
            }

            if (eFld != ww::eNONE)
            {
                sStr.Insert(FieldString(eFld), 0);
                rWW8Wrt.OutField(pFld, eFld, sStr);
            }
            else
                bWriteExpand = true;
        }
        break;
    case RES_DATETIMEFLD:
        if (FIXEDFLD & nSubType || !rWW8Wrt.GetNumberFmt(*pFld, sStr))
            bWriteExpand = true;
        else
        {
            ww::eField eFld = (DATEFLD & nSubType) ? ww::eDATE : ww::eTIME;
            rWW8Wrt.OutField(pFld, eFld, FieldString(eFld));
        }
        break;
    case RES_DOCSTATFLD:
        {
            ww::eField eFld = ww::eNONE;

            switch (nSubType)
            {
                case DS_PAGE:
                    eFld = ww::eNUMPAGE;
                    break;
                case DS_WORD:
                    eFld = ww::eNUMWORDS;
                    break;
                case DS_CHAR:
                    eFld = ww::eNUMCHARS;
                    break;
            }

            if (eFld != ww::eNONE)
            {
                sStr = FieldString(eFld);
                ::WW8_GetNumberPara(sStr, *pFld);
                rWW8Wrt.OutField(pFld, eFld, sStr);
            }
            else
                bWriteExpand = true;
        }
        break;
    case RES_EXTUSERFLD:
        {
            ww::eField eFld = ww::eNONE;
            switch (0xFF & nSubType)
            {
                case EU_FIRSTNAME:
                case EU_NAME:
                    eFld = ww::eUSERNAME;
                    break;
                case EU_SHORTCUT:
                    eFld = ww::eUSERINITIALS;
                    break;
                case EU_STREET:
                case EU_COUNTRY:
                case EU_ZIP:
                case EU_CITY:
                    eFld = ww::eUSERADDRESS;
                    break;
            }

            if (eFld != ww::eNONE)
                rWW8Wrt.OutField(pFld, eFld, sStr);
            else
                bWriteExpand = true;
        }
        break;
    case RES_POSTITFLD:
        //Sadly only possible for word in main document text
        if (rWW8Wrt.nTxtTyp == TXT_MAINTEXT)
        {
            const SwPostItField& rPFld = *(SwPostItField*)pFld;
            rWW8Wrt.pAtn->Append( rWW8Wrt.Fc2Cp( rWrt.Strm().Tell() ), rPFld );
            rWW8Wrt.WritePostItBegin( rWW8Wrt.pO );
        }
        break;
    case RES_INPUTFLD:
        sStr = FieldString(ww::eFILLIN);
        sStr.ASSIGN_CONST_ASC("\"");
        sStr += pFld->GetPar2();
        sStr += '\"';
        rWW8Wrt.OutField(pFld, ww::eFILLIN, sStr);
        break;
    case RES_GETREFFLD:
        {
            ww::eField eFld = ww::eNONE;
            const SwGetRefField& rRFld = *(SwGetRefField*)pFld;
            switch (nSubType)
            {
                case REF_SETREFATTR:
                case REF_BOOKMARK:
                    switch (pFld->GetFormat())
                    {
                        case REF_PAGE_PGDESC:
                        case REF_PAGE:
                            eFld = ww::ePAGEREF;
                            break;
                        default:
                            eFld = ww::eREF;
                            break;
                    }
                    sStr = FieldString(eFld);
                    sStr += rWW8Wrt.GetBookmarkName(nSubType,
                        &rRFld.GetSetRefName(), 0);
                    break;
                case REF_FOOTNOTE:
                case REF_ENDNOTE:
                    switch (pFld->GetFormat())
                    {
                        case REF_PAGE_PGDESC:
                        case REF_PAGE:
                            eFld = ww::ePAGEREF;
                            break;
                        case REF_UPDOWN:
                            eFld = ww::eREF;
                            break;
                        default:
                            eFld =
                                REF_ENDNOTE == nSubType ? ww::eNOTEREF : ww::eFOOTREF;
                            break;
                    }
                    sStr = FieldString(eFld);
                    sStr += rWW8Wrt.GetBookmarkName(nSubType, 0,
                        rRFld.GetSeqNo());
                    break;
            }

            if (eFld != ww::eNONE)
            {
                switch (pFld->GetFormat())
                {
                    case REF_UPDOWN:
                        sStr.APPEND_CONST_ASC(" \\p");
                        break;
                    case REF_CHAPTER:
                        sStr.APPEND_CONST_ASC(" \\n");
                        break;
                    default:
                        break;
                }
                sStr.APPEND_CONST_ASC(" \\h ");       // insert hyperlink
                rWW8Wrt.OutField(pFld, eFld, sStr);
            }
            else
                bWriteExpand = true;
        }
        break;
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
        USHORT nScript;
        if( pBreakIt->xBreak.is() )
            nScript = pBreakIt->xBreak->getScriptType( pFld->GetPar1(), 0);
        else
            nScript = com::sun::star::i18n::ScriptType::ASIAN;

        long nHeight = ((SvxFontHeightItem&)(((SwWW8Writer&)rWrt).GetItem(
            GetWhichOfScript(RES_CHRATR_FONTSIZE,nScript)))).GetHeight();;

        nHeight = (nHeight + 10) / 20; //Font Size in points;

        /*
        Divide the combined char string into its up and down part. Get the
        font size and fill in the defaults as up == half the font size and
        down == a fifth the font size
        */
        xub_StrLen nAbove = (pFld->GetPar1().Len()+1)/2;
        sStr = FieldString(ww::eEQ);
        sStr.APPEND_CONST_ASC("\\o (\\s\\up ");
        sStr += String::CreateFromInt32(nHeight/2);

        sStr.Append('(');
        sStr += String(pFld->GetPar1(),0,nAbove);
        sStr.APPEND_CONST_ASC("), \\s\\do ");
        sStr += String::CreateFromInt32(nHeight/5);

        sStr.Append('(');
        sStr += String(pFld->GetPar1(),nAbove,pFld->GetPar1().Len()-nAbove);
        sStr.APPEND_CONST_ASC("))");
        rWW8Wrt.OutField(pFld, ww::eEQ, sStr);
        }
        break;
    case RES_DROPDOWN:
        if (rWW8Wrt.bWrtWW8)
        {
            const SwDropDownField& rFld = *(SwDropDownField*)pFld;
            com::sun::star::uno::Sequence<rtl::OUString> aItems =
                rFld.GetItemSequence();
            rWW8Wrt.DoComboBox(rFld.GetName(), rFld.GetSelectedItem(), aItems);
        }
        else
            bWriteExpand = true;
        break;
    case RES_CHAPTERFLD:
        bWriteExpand = true;
        if (rWW8Wrt.bOutKF && rFld.GetTxtFld())
        {
            const SwTxtNode *pTxtNd = rWW8Wrt.GetHdFtPageRoot();
            if (!pTxtNd)
            {
                if (const SwNode *pNd = rWW8Wrt.pCurPam->GetNode())
                    pTxtNd = pNd->GetTxtNode();
            }

            if (pTxtNd)
            {
                SwChapterField aCopy(*(const SwChapterField*)pFld);
                aCopy.ChangeExpansion(*pTxtNd, false);
                WriteExpand(rWW8Wrt, aCopy);
                bWriteExpand = false;
            }
        }
        break;
        case RES_HIDDENTXTFLD:
        {
            String sExpand(pFld->GetPar2());
            if (sExpand.Len())
            {
                //replace LF 0x0A with VT 0x0B
                sExpand.SearchAndReplaceAll(0x0A, 0x0B);
                rWW8Wrt.pChpPlc->AppendFkpEntry(rWW8Wrt.Strm().Tell());
                if (rWW8Wrt.IsUnicode())
                {
                    SwWW8Writer::WriteString16(rWW8Wrt.Strm(), sExpand, false);
                    static BYTE aArr[] =
                    {
                        0x3C, 0x08, 0x1
                    };
                    rWW8Wrt.pChpPlc->AppendFkpEntry(rWW8Wrt.Strm().Tell(), sizeof(aArr), aArr);
                }
                else
                {
                    SwWW8Writer::WriteString8(rWW8Wrt.Strm(), sExpand, false,
                        RTL_TEXTENCODING_MS_1252);
                    static BYTE aArr[] =
                    {
                        92, 0x1
                    };
                    rWW8Wrt.pChpPlc->AppendFkpEntry(rWW8Wrt.Strm().Tell(), sizeof(aArr), aArr);
                }
            }
        }
        break;
    default:
        bWriteExpand = true;
        break;
    }

    if (bWriteExpand)
        WriteExpand(rWW8Wrt, *pFld);

    return rWrt;
}

static Writer& OutWW8_SwFlyCntnt( Writer& rWrt, const SfxPoolItem& rHt )
{
        SwWW8Writer& rWrtWW8 = (SwWW8Writer&)rWrt;
    if (rWrtWW8.pOutFmtNode && rWrtWW8.pOutFmtNode->ISA(SwCntntNode))
    {
        SwTxtNode* pTxtNd = (SwTxtNode*)rWrtWW8.pOutFmtNode;

        Point aLayPos;
        aLayPos = pTxtNd->FindLayoutRect(false, &aLayPos).Pos();

        SwPosition aPos(*pTxtNd);
        sw::Frame aFrm(*((const SwFmtFlyCnt&)rHt).GetFrmFmt(), aPos);

        rWrtWW8.OutWW8FlyFrm(aFrm, aLayPos);
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

static Writer& OutWW8_SfxBoolItem( Writer& rWrt, const SfxPoolItem& rHt )
{
    USHORT nId = 0;
    SwWW8Writer& rWrtWW8 = (SwWW8Writer&)rWrt;
    if( rWrtWW8.bWrtWW8 )
        switch ( rHt.Which() )
        {
        case RES_PARATR_SCRIPTSPACE:            nId = 0x2437;   break;
        case RES_PARATR_HANGINGPUNCTUATION:     nId = 0x2435;   break;
        case RES_PARATR_FORBIDDEN_RULES:        nId = 0x2433;   break;
        }

    if( nId )
    {
        if( rWrtWW8.bWrtWW8 )
            rWrtWW8.InsUInt16( nId );
        else
            rWrtWW8.pO->Insert( (BYTE)nId, rWrtWW8.pO->Count() );

        rWrtWW8.pO->Insert( ((SfxBoolItem&)rHt).GetValue() ? 1 : 0,
                            rWrtWW8.pO->Count() );
    }
    return rWrt;
}

static Writer& OutWW8_SvxParaGridItem(Writer& rWrt, const SfxPoolItem& rHt)
{
//  sprmPFUsePgsuSettings

    SwWW8Writer& rWrtWW8 = (SwWW8Writer&)rWrt;
    //97+ only
    if (!rWrtWW8.bWrtWW8)
        return rWrt;

    rWrtWW8.InsUInt16(0x2447);
    const SvxParaGridItem& rAttr = (const SvxParaGridItem&)rHt;
    rWrtWW8.pO->Insert( rAttr.GetValue(), rWrtWW8.pO->Count() );
    return rWrt;
}

static Writer& OutWW8_SvxParaVertAlignItem(Writer& rWrt, const SfxPoolItem& rHt)
{
// sprmPWAlignFont

    SwWW8Writer& rWrtWW8 = (SwWW8Writer&)rWrt;
    //97+ only
    if( !rWrtWW8.bWrtWW8 )
        return rWrt;

    rWrtWW8.InsUInt16( 0x4439 );
    const SvxParaVertAlignItem & rAttr = (const SvxParaVertAlignItem&)rHt;

    INT16 nVal = rAttr.GetValue();
    switch (nVal)
    {
        case SvxParaVertAlignItem::BASELINE:
            nVal = 2;
            break;
        case SvxParaVertAlignItem::TOP:
            nVal = 0;
            break;
        case SvxParaVertAlignItem::CENTER:
            nVal = 1;
            break;
        case SvxParaVertAlignItem::BOTTOM:
            nVal = 3;
            break;
        case SvxParaVertAlignItem::AUTOMATIC:
            nVal = 4;
            break;
        default:
            nVal = 4;
            ASSERT(!(&rWrt), "Unknown vert alignment");
            break;
    }
    rWrtWW8.InsUInt16( nVal );
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
    bool bAutoNum = !rFtn.GetNumStr().Len();    // Auto-Nummer
    if( bAutoNum )
    {
        if( bWrtWW8 )
        {
            static const BYTE aSpec[] =
            {
                0x03, 0x6a, 0, 0, 0, 0, // sprmCObjLocation
                0x55, 0x08, 1           // sprmCFSpec
            };

            aAttrArr.Insert(aSpec, sizeof(aSpec), aAttrArr.Count());
        }
        else
        {
            static BYTE const aSpec[] =
            {
                117, 1,                         // sprmCFSpec
                68, 4, 0, 0, 0, 0               // sprmCObjLocation
            };

            aAttrArr.Insert(aSpec, sizeof(aSpec), aAttrArr.Count());
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

                                                // fSpec-Attribut true
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

static bool lcl_IsAtTxtEnd(const SwFmtFtn& rFtn)
{
    bool bRet = true;
    if( rFtn.GetTxtFtn() )
    {
        USHORT nWh = rFtn.IsEndNote() ? RES_END_AT_TXTEND
                                      : RES_FTN_AT_TXTEND;
        const SwSectionNode* pSectNd = rFtn.GetTxtFtn()->GetTxtNode().
                                                        FindSectionNode();
        while( pSectNd && FTNEND_ATPGORDOCEND ==
                ((const SwFmtFtnAtTxtEnd&)pSectNd->GetSection().GetFmt()->
                GetAttr( nWh, true)).GetValue() )
            pSectNd = pSectNd->FindStartNode()->FindSectionNode();

        if (!pSectNd)
            bRet = false;   // the is ftn/end collected at Page- or Doc-End
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

/*
 See ww8par6.cxx Read_DoubleLine for some more info
 */
static Writer& OutWW8_SvxTwoLinesItem( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwWW8Writer& rWrtWW8 = (SwWW8Writer&)rWrt;
    //97+ only
    if( !rWrtWW8.bWrtWW8 )
        return rWrt;

    const SvxTwoLinesItem& rAttr = (const SvxTwoLinesItem&)rHt;
    rWrtWW8.InsUInt16( 0xCA78 );
    rWrtWW8.pO->Insert( (BYTE)0x06, rWrtWW8.pO->Count() ); //len 6
    rWrtWW8.pO->Insert( (BYTE)0x02, rWrtWW8.pO->Count() );

    sal_Unicode cStart = rAttr.GetStartBracket();
    sal_Unicode cEnd = rAttr.GetStartBracket();

    /*
    As per usual we have problems. We can have seperate left and right brackets
    in OOo, it doesn't appear that you can in word. Also in word there appear
    to only be a limited number of possibilities, we can use pretty much
    anything.

    So if we have none, we export none, if either bracket is set to a known
    word type we export both as that type (with the bracket winning out in
    the case of a conflict simply being the order of test here.

    Upshot being a documented created in word will be reexported with no
    ill effects.
    */

    USHORT nType;
    if (!cStart && !cEnd)
        nType = 0;
    else if ((cStart == '{') || (cEnd == '}'))
        nType = 4;
    else if ((cStart == '<') || (cEnd == '>'))
        nType = 3;
    else if ((cStart == '[') || (cEnd == ']'))
        nType = 2;
    else
        nType = 1;
    rWrtWW8.InsUInt16( nType );
    static const BYTE aZeroArr[ 3 ] = { 0, 0, 0 };
    rWrtWW8.pO->Insert( aZeroArr, 3, rWrtWW8.pO->Count() );
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

                    if( pNum && pNum->IsShowNum() )
                        nLvl = GetRealLevel( pNum->GetLevel() );

                    if (pNum && (USHRT_MAX != pNum->GetSetValue() || pNum->IsStart()))
                    {
                        USHORT nStartWith = (USHRT_MAX != pNum->GetSetValue()) ? pNum->GetSetValue() : 1;
                        nNumId = rWW8Wrt.DupNumRuleWithLvlStart(pRule,nLvl,nStartWith);
                        if (USHRT_MAX != nNumId)
                            ++nNumId;
                    }
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

    if (USHRT_MAX != nNumId)
    {
        if (nLvl >= WW8ListManager::nMaxLevel)
            nLvl = WW8ListManager::nMaxLevel-1;
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
        if( rSz.GetWidth() && rSz.GetWidthSizeType() == ATT_FIX_SIZE)
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
            switch( rSz.GetHeightSizeType() )
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
        rWW8Wrt.InsUInt16(
            msword_cast<sal_uInt16>(SnapPageDimension(rSz.GetWidth())));

        /*sprmSYaPage*/
        if( rWW8Wrt.bWrtWW8 )
            rWW8Wrt.InsUInt16( 0xB020 );
        else
            rWW8Wrt.pO->Insert( 165, rWW8Wrt.pO->Count() );
        rWW8Wrt.InsUInt16(
            msword_cast<sal_uInt16>(SnapPageDimension(rSz.GetHeight())));
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

    bool bReplaced = false;
    SvStream& rStrm = Strm();
    ULONG nRetPos = 0, nPos = rStrm.Tell();
    BYTE nBCode=0;
    UINT16 nUCode=0;
    //If there is at least two characters already output
    if (nPos - (IsUnicode() ? 2 : 1) >= ULONG(pFib->fcMin))
    {
        rStrm.SeekRel(IsUnicode() ? -2 : -1);
        if (IsUnicode())
            rStrm >> nUCode;
        else
        {
            rStrm >> nBCode;
            nUCode = nBCode;
        }
        //If the last char was a cr
        if (nUCode == 0x0d)             // CR ?
        {
            if ((nChar == 0x0c) &&
                (nPos - (IsUnicode() ? 4 : 2) >= ULONG(pFib->fcMin)))
            {
                rStrm.SeekRel( IsUnicode() ? -4 : -2 );
                if (IsUnicode())
                    rStrm >> nUCode;
                else
                {
                    rStrm >> nUCode;
                    nUCode = nBCode;
                }
            }
            else
            {
                rStrm.SeekRel( IsUnicode() ? -2 : -1 );
                nUCode = 0x0;
            }
            //And the para is not of len 0, then replace this cr with the mark
            if (nUCode == 0x0d)
                bReplaced = false;
            else
            {
                bReplaced = true;
                WriteChar(nChar);
                nRetPos = nPos;
            }
        }
        else if ((nUCode == 0x0c) && (nChar == 0x0e))
        {
            //#108854# a column break after a section has
            //no effect in writer
            bReplaced = true;
        }
        rStrm.Seek( nPos );
    }
    else
        bReplaced = true;

    if (!bReplaced)
    {
        // then write as normal char
        WriteChar(nChar);
        pPiece->SetParaBreak();
        pPapPlc->AppendFkpEntry(rStrm.Tell());
        nRetPos = rStrm.Tell();
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

void SwWW8Writer::WriteCellEnd()
{
    //Technically in a word document this is a different value for a
    //cell without a graphic. But it doesn't seem to make a difference
    ULONG nOffset = ReplaceCr( (BYTE)0x07 );
    ASSERT(nOffset, "Eek!, no para end mark to replace with row end mark");
    if (nOffset)
        pMagicTable->Append(Fc2Cp(nOffset),0x122);
}

void SwWW8Writer::WriteRowEnd()
{
    WriteChar( (BYTE)0x07 );
    //Technically in a word document this is a different value for a row ends
    //that are not row ends directly after a cell with a graphic. But it
    //doesn't seem to make a difference
    pMagicTable->Append(Fc2Cp(Strm().Tell()),0x1B6);
}

static Writer& OutWW8_SwFmtPageDesc(Writer& rWrt, const SfxPoolItem& rHt)
{
    SwWW8Writer& rWW8Wrt = (SwWW8Writer&)rWrt;
    if (rWW8Wrt.bStyDef && rWW8Wrt.pOutFmtNode && rWW8Wrt.pOutFmtNode->ISA(SwTxtFmtColl))
    {
        const SwFmtPageDesc &rPgDesc = (const SwFmtPageDesc&)rHt;
        const SwTxtFmtColl* pC = (SwTxtFmtColl*)rWW8Wrt.pOutFmtNode;
        if ((SFX_ITEM_SET != pC->GetItemState(RES_BREAK, false)) && rPgDesc.GetRegisteredIn())
            OutWW8_SwFmtBreak(rWrt, SvxFmtBreakItem(SVX_BREAK_PAGE_BEFORE));
    }
    return rWrt;
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
        case SVX_BREAK_NONE:
        case SVX_BREAK_PAGE_BEFORE:
        case SVX_BREAK_PAGE_BOTH:
            // sprmPPageBreakBefore/sprmPFPageBreakBefore
            if (rWW8Wrt.bWrtWW8)
                rWW8Wrt.InsUInt16(0x2407);
            else
                rWW8Wrt.pO->Insert(9, rWW8Wrt.pO->Count());
            rWW8Wrt.pO->Insert(rBreak.GetValue() ? 1 : 0,
                rWW8Wrt.pO->Count());
            break;
        default:
            break;
        }
    }
    else if (!rWW8Wrt.mpParentFrame)
    {
        BYTE nC = 0;
        bool bBefore = false;

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
            bBefore = true;
                // no break;
        case SVX_BREAK_COLUMN_AFTER:
        case SVX_BREAK_COLUMN_BOTH:
            ASSERT (rWW8Wrt.pSepx, "how come this is 0");
            if (rWW8Wrt.pSepx &&
                rWW8Wrt.pSepx->CurrentNoColumns(*rWW8Wrt.pDoc) > 1)
            {
                nC = 0xe;
            }
            break;

        case SVX_BREAK_PAGE_BEFORE:                         // PageBreak
            bBefore = true;
            // no break;
        case SVX_BREAK_PAGE_AFTER:
        case SVX_BREAK_PAGE_BOTH:
            nC = 0xc;
            break;
        default:
            break;
        }

        if( (bBefore == rWW8Wrt.bBreakBefore ) && nC )  // #49917#
            rWW8Wrt.ReplaceCr( nC );
    }
    return rWrt;
}

static Writer& OutWW8_SwTextGrid( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwWW8Writer& rWrtWW8 = (SwWW8Writer&)rWrt;
    if (rWrtWW8.bOutPageDescs && rWrtWW8.bWrtWW8)
    {
        const SwTextGridItem& rItem = (const SwTextGridItem&)rHt;
        UINT16 nGridType=0;
        switch (rItem.GetGridType())
        {
            default:
                ASSERT(!(&rWrt), "Unknown grid type");
            case GRID_NONE:
                nGridType = 0;
                break;
            case GRID_LINES_ONLY:
                nGridType = 2;
                break;
            case GRID_LINES_CHARS:
                nGridType = 1;
                break;
        }
        rWrtWW8.InsUInt16(0x5032);
        rWrtWW8.InsUInt16(nGridType);

        UINT16 nHeight = rItem.GetBaseHeight() + rItem.GetRubyHeight();
        rWrtWW8.InsUInt16(0x9031);
        rWrtWW8.InsUInt16(nHeight);
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
        nLDist += (USHORT)rLR.GetLeft();
        nRDist += (USHORT)rLR.GetRight();

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
        {
            rWW8Wrt.InsUInt16( 0x840F );
            rWW8Wrt.InsUInt16( (USHORT)rLR.GetTxtLeft() );
            rWW8Wrt.InsUInt16( 0x845E );        //asian version ?
            rWW8Wrt.InsUInt16( (USHORT)rLR.GetTxtLeft() );

        }
        else
        {
            rWW8Wrt.pO->Insert( 17, rWW8Wrt.pO->Count() );
            rWW8Wrt.InsUInt16( (USHORT)rLR.GetTxtLeft() );
        }
        // sprmPDxaRight
        if( rWW8Wrt.bWrtWW8 )
        {
            rWW8Wrt.InsUInt16( 0x840E );
            rWW8Wrt.InsUInt16( (USHORT)rLR.GetRight() );
            rWW8Wrt.InsUInt16( 0x845D );        //asian version ?
            rWW8Wrt.InsUInt16( (USHORT)rLR.GetRight() );
        }
        else
        {
            rWW8Wrt.pO->Insert( 16, rWW8Wrt.pO->Count() );
            rWW8Wrt.InsUInt16( (USHORT)rLR.GetRight() );
        }
        // sprmPDxaLeft1
        if( rWW8Wrt.bWrtWW8 )
        {
            rWW8Wrt.InsUInt16( 0x8411 );
            rWW8Wrt.InsUInt16( rLR.GetTxtFirstLineOfst() );
            rWW8Wrt.InsUInt16( 0x8460 );        //asian version ?
            rWW8Wrt.InsUInt16( rLR.GetTxtFirstLineOfst() );
        }
        else
        {
            rWW8Wrt.pO->Insert( 19, rWW8Wrt.pO->Count() );
            rWW8Wrt.InsUInt16( rLR.GetTxtFirstLineOfst() );
        }
    }
    return rWrt;
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
        ASSERT(rWW8Wrt.GetCurItemSet(), "Impossible");
        if (!rWW8Wrt.GetCurItemSet())
            return rWrt;

        HdFtDistanceGlue aDistances(*rWW8Wrt.GetCurItemSet());

        if (aDistances.HasHeader())
        {
            //sprmSDyaHdrTop
            if (rWW8Wrt.bWrtWW8)
                rWW8Wrt.InsUInt16(0xB017);
            else
                rWW8Wrt.pO->Insert(156, rWW8Wrt.pO->Count());
            rWW8Wrt.InsUInt16(aDistances.dyaHdrTop);
        }

        // sprmSDyaTop
        if (rWW8Wrt.bWrtWW8)
            rWW8Wrt.InsUInt16(0x9023);
        else
            rWW8Wrt.pO->Insert(168, rWW8Wrt.pO->Count());
        rWW8Wrt.InsUInt16(aDistances.dyaTop);

        if (aDistances.HasFooter())
        {
            //sprmSDyaHdrBottom
            if (rWW8Wrt.bWrtWW8)
                rWW8Wrt.InsUInt16(0xB018);
            else
                rWW8Wrt.pO->Insert(157, rWW8Wrt.pO->Count());
            rWW8Wrt.InsUInt16(aDistances.dyaHdrBottom);
        }

        //sprmSDyaBottom
        if (rWW8Wrt.bWrtWW8)
            rWW8Wrt.InsUInt16(0x9024);
        else
            rWW8Wrt.pO->Insert(169, rWW8Wrt.pO->Count());
        rWW8Wrt.InsUInt16(aDistances.dyaBottom);
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
    SwWW8Writer& rWW8Wrt = (SwWW8Writer&)rWrt;

//!!!! Ankertyp und entsprechende Umrechnung fehlt noch

    if( rWW8Wrt.bOutFlyFrmAttrs )
    {
        const SwFmtVertOrient& rFlyVert = (const SwFmtVertOrient&) rHt;

        short nPos;
        switch( rFlyVert.GetVertOrient() )
        {
        case VERT_NONE:
            nPos = (short)rFlyVert.GetPos();
            break;
        case VERT_CENTER:
        case VERT_LINE_CENTER:
            nPos = -8;
            break;
        case VERT_BOTTOM:
        case VERT_LINE_BOTTOM:
            nPos = -12;
            break;
        case VERT_TOP:
        case VERT_LINE_TOP:
        default:
            nPos = -4;
            break;
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
    if (!rWW8Wrt.mpParentFrame)
    {
        ASSERT(rWW8Wrt.mpParentFrame, "HoriOrient without mpParentFrame !!");
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
    ASSERT(rWW8Wrt.mpParentFrame, "Anchor without mpParentFrame !!");

    if( rWW8Wrt.bOutFlyFrmAttrs )
    {
        const SwFmtAnchor& rAnchor = (const SwFmtAnchor&) rHt;
        BYTE nP = 0;
        switch( rAnchor.GetAnchorId() )
        {
            case FLY_PAGE:
                // Vert: Page | Horz: Page
                nP |= (1 << 4) | (2 << 6);
                break;
            // Im Fall eine Flys als Zeichen: Absatz-gebunden setzen!!!
            case FLY_AT_FLY:
            case FLY_AUTO_CNTNT:
            case FLY_AT_CNTNT:
            case FLY_IN_CNTNT:
                // Vert: Page | Horz: Page
                nP |= (2 << 4) | (0 << 6);
                break;
            default:
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

        rWW8Wrt.TransBrush(rBack.GetColor(), aSHD);
        // sprmPShd
        if (rWW8Wrt.bWrtWW8)
            rWW8Wrt.InsUInt16(0x442D);
        else
            rWW8Wrt.pO->Insert(47, rWW8Wrt.pO->Count());
        rWW8Wrt.InsUInt16( aSHD.GetValue() );

        //Quite a few unknowns, some might be transparency or something
        //of that nature...
        if (rWW8Wrt.bWrtWW8)
        {
            rWW8Wrt.InsUInt16(0xC64D);
            rWW8Wrt.pO->Insert(10, rWW8Wrt.pO->Count());
            rWW8Wrt.InsUInt32(0xFF000000);
            rWW8Wrt.InsUInt32(SuitableBGColor(
                rBack.GetColor().GetColor()));
            rWW8Wrt.InsUInt16(0x0000);
        }
    }
    return rWrt;
}

WW8_BRC SwWW8Writer::TranslateBorderLine(const SvxBorderLine& rLine,
    USHORT nDist, bool bShadow)
{
    // M.M. This function writes out border lines to the word format similar to
    // what SwRTFWriter::OutRTFBorder does in the RTF filter Eventually it
    // would be nice if all this functionality was in the one place
    WW8_BRC aBrc;
    UINT16 nWidth = rLine.GetInWidth() + rLine.GetOutWidth();
    BYTE brcType = 0, nColCode = 0;

    if( nWidth )                                // Linie ?
    {
        // BRC.brcType
        bool bDouble = 0 != rLine.GetInWidth() && 0 != rLine.GetOutWidth();
        bool bThick = !bDouble && !bWrtWW8 && nWidth > 75;
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
    USHORT nLDist = nDist;
    nLDist /= 20;               // Masseinheit : pt
    if( nLDist > 0x1f )
        nLDist = 0x1f;

    if( bWrtWW8 )
    {
        aBrc.aBits1[0] = BYTE(nWidth);
        aBrc.aBits1[1] = brcType;
        aBrc.aBits2[0] = nColCode;
        aBrc.aBits2[1] = BYTE(nLDist);

        // fShadow, keine weiteren Einstellungen im WW moeglich
        if( bShadow )
            aBrc.aBits2[1] |= 0x20;
    }
    else
    {
        USHORT aBits = nWidth + ( brcType << 3 );
        aBits |= (nColCode & 0x1f) << 6;
        aBits |= nLDist << 11;
        // fShadow, keine weiteren Einstellungen im WW moeglich
        if( bShadow )
            aBits |= 0x20;
        ShortToSVBT16( aBits, aBrc.aBits1);
    }

    return aBrc;
}

// MakeBorderLine() bekommt einen WW8Bytes* uebergeben, um die Funktion
// auch fuer die Tabellen-Umrandungen zu benutzen.
// Wenn nSprmNo == 0, dann wird der Opcode nicht ausgegeben.
// bShadow darf bei Tabellenzellen *nicht* gesetzt sein !
void SwWW8Writer::Out_BorderLine(WW8Bytes& rO, const SvxBorderLine* pLine,
    USHORT nDist, USHORT nOffset, bool bShadow)
{
    ASSERT( (nOffset <= 3) || USHRT_MAX == nOffset ||
            ((0x702b - 0x6424) <= nOffset && nOffset <= (0x702e - 0x6424)),
                "SprmOffset ausserhalb des Bereichs" );

    WW8_BRC aBrc;

    if (pLine)
        aBrc = TranslateBorderLine( *pLine, nDist, bShadow );

    if( bWrtWW8 )
    {
// WW97-SprmIds
// 0x6424, sprmPBrcTop      pap.brcTop;BRC;long; !!!!
// 0x6425, sprmPBrcLeft
// 0x6426, sprmPBrcBottom
// 0x6427, sprmPBrcRight
        if( USHRT_MAX != nOffset )                      // mit OpCode-Ausgabe ?
            SwWW8Writer::InsUInt16( rO, 0x6424 + nOffset );

        rO.Insert( aBrc.aBits1, 2, rO.Count() );
        rO.Insert( aBrc.aBits2, 2, rO.Count() );
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
        rO.Insert( aBrc.aBits1, 2, rO.Count() );
    }
}

// OutWW8_SwFmtBox1() ist fuer alle Boxen ausser in Tabellen.
// es wird pO des WW8Writers genommen
void SwWW8Writer::Out_SwFmtBox(const SvxBoxItem& rBox, bool bShadow)
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

    static const USHORT aBorders[] =
    {
        BOX_LINE_TOP, BOX_LINE_LEFT, BOX_LINE_BOTTOM, BOX_LINE_RIGHT
    };
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
    static const USHORT aBorders[] =
    {
        BOX_LINE_TOP, BOX_LINE_LEFT, BOX_LINE_BOTTOM, BOX_LINE_RIGHT
    };
    const USHORT* pBrd = aBorders;
    for( int i = 0; i < 4; ++i, ++pBrd )
    {
        const SvxBorderLine* pLn = rBox.GetLine( *pBrd );
        Out_BorderLine(rO, pLn, 0, USHRT_MAX, false);
    }
}

static Writer& OutWW8_SwFmtBox( Writer& rWrt, const SfxPoolItem& rHt )
{
                                // Fly um Grafik-> keine Umrandung hier, da
                                // der GrafikHeader bereits die Umrandung hat
    SwWW8Writer& rWW8Wrt = ((SwWW8Writer&)rWrt);
    if( !rWW8Wrt.bOutGrf )
    {
        bool bShadow = false;
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

SwTwips SwWW8Writer::CurrentPageWidth(SwTwips &rLeft, SwTwips &rRight) const
{
    const SwFrmFmt* pFmt = pAktPageDesc ? &pAktPageDesc->GetMaster()
        : &const_cast<const SwDoc *>(pDoc)->GetPageDesc(0).GetMaster();

    const SvxLRSpaceItem& rLR = pFmt->GetLRSpace();
    SwTwips nPageSize = pFmt->GetFrmSize().GetWidth();
    rLeft = rLR.GetLeft();
    rRight = rLR.GetRight();
    return nPageSize;
}

static Writer& OutWW8_SwFmtCol( Writer& rWrt, const SfxPoolItem& rHt )
{
    const SwFmtCol& rCol = (const SwFmtCol&)rHt;
    const SwColumns& rColumns = rCol.GetColumns();
    SwWW8Writer& rWW8Wrt = ((SwWW8Writer&)rWrt);

    USHORT nCols = rColumns.Count();
    if (1 < nCols && !rWW8Wrt.bOutFlyFrmAttrs)
    {
        // dann besorge mal die Seitenbreite ohne Raender !!

        SwTwips nLeft, nRight, nPageSize;
        nPageSize = rWW8Wrt.CurrentPageWidth(nLeft, nRight);
        nPageSize -= (nLeft + nRight);

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
        rWW8Wrt.InsUInt16(rCol.GetGutterWidth(true));

        // LBetween
        if( rWW8Wrt.bWrtWW8 )
            rWW8Wrt.InsUInt16( 0x3019 );
        else
            rWW8Wrt.pO->Insert( 158, rWW8Wrt.pO->Count() );
        rWW8Wrt.pO->Insert( COLADJ_NONE == rCol.GetLineAdj() ? 0 : 1,
                            rWW8Wrt.pO->Count() );

        // Nachsehen, ob alle Spalten gleich sind
        bool bEven = true;
        USHORT n;
        USHORT nColWidth = rCol.CalcPrtColWidth( 0, (USHORT)nPageSize );
        for (n = 1; n < nCols; n++)
        {
            short nDiff = nColWidth -
                rCol.CalcPrtColWidth( n, (USHORT)nPageSize );

            if( nDiff > 10 || nDiff < -10 )      // Toleranz: 10 tw
            {
                bEven = false;
                break;
            }
        }
        if (bEven)
        {
            USHORT nSpace = rColumns[0]->GetRight() + rColumns[1]->GetLeft();
            for( n = 2; n < nCols; n++ )
            {
                short nDiff = nSpace - ( rColumns[n - 1]->GetRight()
                                         + rColumns[n]->GetLeft() );
                if (nDiff > 10 || nDiff < -10)
                {
                    // Toleranz: 10 tw
                    bEven = false;
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
            for (n = 0; n < nCols; ++n)
            {
                //sprmSDxaColWidth
                if( rWW8Wrt.bWrtWW8 )
                    rWW8Wrt.InsUInt16( 0xF203 );
                else
                    rWW8Wrt.pO->Insert( 136, rWW8Wrt.pO->Count() );
                rWW8Wrt.pO->Insert( (BYTE)n, rWW8Wrt.pO->Count() );
                rWW8Wrt.InsUInt16(rCol.CalcPrtColWidth(n, (USHORT)nPageSize));

                if( n+1 != nCols )
                {
                    //sprmSDxaColSpacing
                    if( rWW8Wrt.bWrtWW8 )
                        rWW8Wrt.InsUInt16( 0xF204 );
                    else
                        rWW8Wrt.pO->Insert( 137, rWW8Wrt.pO->Count() );
                    rWW8Wrt.pO->Insert( (BYTE)n, rWW8Wrt.pO->Count() );
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

    switch (rAttr.GetLineSpaceRule())
    {
        default:
            break;
        case SVX_LINE_SPACE_AUTO:
        case SVX_LINE_SPACE_FIX:
        case SVX_LINE_SPACE_MIN:
        {
            switch (rAttr.GetInterLineSpaceRule())
            {
                case SVX_INTER_LINE_SPACE_FIX:      // unser Durchschuss
                {
                    // gibt es aber nicht in WW - also wie kommt man an
                    // die MaxLineHeight heran?
                    nSpace = (short)rAttr.GetInterLineSpace();
                    sal_uInt16 nScript =
                        com::sun::star::i18n::ScriptType::LATIN;
                    const SwAttrSet *pSet = 0;
                    if (rWrtWW8.pOutFmtNode && rWrtWW8.pOutFmtNode->ISA(SwFmt))
                    {
                        const SwFmt *pFmt = (const SwFmt*)(rWrtWW8.pOutFmtNode);
                        pSet = &pFmt->GetAttrSet();
                    }
                    else if (rWrtWW8.pOutFmtNode &&
                        rWrtWW8.pOutFmtNode->ISA(SwTxtNode))
                    {
                        const SwTxtNode* pNd =
                            (const SwTxtNode*)rWrtWW8.pOutFmtNode;
                        pSet = &pNd->GetSwAttrSet();
                        if (pBreakIt->xBreak.is())
                        {
                            nScript = pBreakIt->xBreak->
                                getScriptType(pNd->GetTxt(), 0);
                        }
                    }
                    ASSERT(pSet, "No attrset for lineheight :-(");
                    if (pSet)
                    {
                        nSpace += (short)(AttrSetToLineHeight(*rWrtWW8.pDoc,
                            *pSet, *Application::GetDefaultDevice(), nScript));
                    }
                }
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

    rWrtWW8.InsUInt16(nSpace);
    rWrtWW8.InsUInt16(nMulti);
    return rWrt;
}

static Writer& OutWW8_SvxAdjust(Writer& rWrt, const SfxPoolItem& rHt)
{
// sprmPJc
    const SvxAdjustItem& rAttr = (const SvxAdjustItem&)rHt;
    BYTE nAdj = 255;
    BYTE nAdjBiDi = 255;
    switch(rAttr.GetAdjust())
    {
        case SVX_ADJUST_LEFT:
            nAdj = 0;
            nAdjBiDi = 2;
            break;
        case SVX_ADJUST_RIGHT:
            nAdj = 2;
            nAdjBiDi = 0;
            break;
        case SVX_ADJUST_BLOCKLINE:
        case SVX_ADJUST_BLOCK:
            nAdj = nAdjBiDi = 3;
            break;
        case SVX_ADJUST_CENTER:
            nAdj = nAdjBiDi = 1;
            break;
        default:
            return rWrt;    // not a supported Attribut
    }

    if (255 != nAdj)        // supported Attribut?
    {
        SwWW8Writer& rWrtWW8 = (SwWW8Writer&)rWrt;
        if (rWrtWW8.bWrtWW8)
        {
            rWrtWW8.InsUInt16(0x2403);
            rWrtWW8.pO->Insert(nAdj, rWrtWW8.pO->Count());

            /*
            Sadly for left to right paragraphs both these values are the same,
            for right to left paragraphs the bidi one is the reverse of the
            normal one.
            */
            rWrtWW8.InsUInt16(0x2461); //bidi version ?
            bool bBiDiSwap=false;
            if (rWrtWW8.pOutFmtNode)
            {
                short nDirection = FRMDIR_HORI_LEFT_TOP;
                if (rWrtWW8.pOutFmtNode->ISA(SwTxtNode))
                {
                    SwPosition aPos(*(const SwCntntNode*)rWrtWW8.pOutFmtNode);
                    nDirection = rWrtWW8.pDoc->GetTextDirection(aPos);
                }
                else if (rWrtWW8.pOutFmtNode->ISA(SwTxtFmtColl))
                {
                    const SwTxtFmtColl* pC =
                        (const SwTxtFmtColl*)rWrtWW8.pOutFmtNode;
                    const SvxFrameDirectionItem &rItem =
                        ItemGet<SvxFrameDirectionItem>(*pC, RES_FRAMEDIR);
                    nDirection = rItem.GetValue();
                }
                if (nDirection == FRMDIR_HORI_RIGHT_TOP)
                    bBiDiSwap=true;
            }

            if (bBiDiSwap)
                rWrtWW8.pO->Insert(nAdjBiDi, rWrtWW8.pO->Count());
            else
                rWrtWW8.pO->Insert(nAdj, rWrtWW8.pO->Count());
        }
        else
        {
            rWrtWW8.pO->Insert(5, rWrtWW8.pO->Count());
            rWrtWW8.pO->Insert(nAdj, rWrtWW8.pO->Count());
        }
    }
    return rWrt;
}

static Writer& OutWW8_SvxFrameDirection( Writer& rWrt, const SfxPoolItem& rHt )
{
    SwWW8Writer& rWrtWW8 = (SwWW8Writer&)rWrt;
    if (!rWrtWW8.bWrtWW8)   //8+ only
        return rWrt;

    const SvxFrameDirectionItem& rItem = (const SvxFrameDirectionItem&)rHt;
    UINT16 nTextFlow=0;
    bool bBiDi = false;
    short nDir = rItem.GetValue();

    if (nDir == FRMDIR_ENVIRONMENT)
    {
        if (rWrtWW8.bOutPageDescs)
            nDir = rWrtWW8.GetCurrentPageDirection();
        else if (rWrtWW8.pOutFmtNode)
        {
            if (rWrtWW8.bOutFlyFrmAttrs)  //frame
            {
                nDir = rWrtWW8.TrueFrameDirection(
                    *(const SwFrmFmt*)rWrtWW8.pOutFmtNode);
            }
            else if (rWrtWW8.pOutFmtNode->ISA(SwCntntNode))   //pagagraph
            {
                const SwCntntNode* pNd =
                    (const SwCntntNode*)rWrtWW8.pOutFmtNode;
                SwPosition aPos(*pNd);
                nDir = rWrt.pDoc->GetTextDirection(aPos);
            }
            else if (rWrtWW8.pOutFmtNode->ISA(SwTxtFmtColl))
                nDir = FRMDIR_HORI_LEFT_TOP;    //what else can we do :-(
        }

        if (nDir == FRMDIR_ENVIRONMENT)
            nDir = FRMDIR_HORI_LEFT_TOP;    //Set something
    }

    switch (nDir)
    {
        default:
            //Can't get an unknown type here
            ASSERT(!(&rWrt), "Unknown frame direction");
        case FRMDIR_HORI_LEFT_TOP:
            nTextFlow = 0;
            break;
        case FRMDIR_HORI_RIGHT_TOP:
            nTextFlow = 0;
            bBiDi = true;
            break;
        case FRMDIR_VERT_TOP_LEFT:  //word doesn't have this
        case FRMDIR_VERT_TOP_RIGHT:
            nTextFlow = 1;
            break;
    }

    if (rWrtWW8.bOutPageDescs)
    {
        rWrtWW8.InsUInt16(0x5033);
        rWrtWW8.InsUInt16(nTextFlow);
        rWrtWW8.InsUInt16(0x3228);
        rWrtWW8.pO->Insert(bBiDi, rWrtWW8.pO->Count() );
    }
    else if (!rWrtWW8.bOutFlyFrmAttrs)  //paragraph/style
    {
        rWrtWW8.InsUInt16(0x2441);
        rWrtWW8.pO->Insert(bBiDi, rWrtWW8.pO->Count() );
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
    sal_uInt16 nAdd;            // so viele Tabs kommen hinzu
    sal_uInt16 nDel;            // so viele Tabs fallen weg
public:
    SwWW8WrTabu(sal_uInt16 nDelMax, sal_uInt16 nAddMax);
    ~SwWW8WrTabu();

    void Add(const SvxTabStop &rTS, long nAdjustment);
    void Del(const SvxTabStop &rTS, long nAdjustment);
    void PutAll(SwWW8Writer& rWW8Wrt);
};

SwWW8WrTabu::SwWW8WrTabu(sal_uInt16 nDelMax, sal_uInt16 nAddMax)
    : nAdd(0), nDel(0)
{
    pDel = nDelMax ? new BYTE[nDelMax * 2] : 0;
    pAddPos = new BYTE[nAddMax * 2];
    pAddTyp = new BYTE[nAddMax];
}

SwWW8WrTabu::~SwWW8WrTabu()
{
    delete[] pAddTyp;
    delete[] pAddPos;
    delete[] pDel;
}

// Add( const SvxTabStop & rTS ) fuegt einen Tab in die WW-Struktur ein
void SwWW8WrTabu::Add(const SvxTabStop & rTS, long nAdjustment)
{
    // Tab-Position eintragen
    ShortToSVBT16(msword_cast<sal_Int16>(rTS.GetTabPos() + nAdjustment),
        pAddPos + (nAdd * 2));

    // Tab-Typ eintragen
    BYTE nPara = 0;
    switch (rTS.GetAdjustment())
    {
        case SVX_TAB_ADJUST_RIGHT:
            nPara = 2;
            break;
        case SVX_TAB_ADJUST_CENTER:
            nPara = 1;
            break;
        case SVX_TAB_ADJUST_DECIMAL:
            /*
            Theres nothing we can do btw the the decimal seperator has been
            customized, but if you think different remember that different
            locales have different seperators, i.e. german is a , while english
            is a .
            */
            nPara = 3;
            break;
        default:
            break;
    }

    switch( rTS.GetFill() )
    {
        case '.':   // dotted leader
            nPara |= 1 << 3;
            break;
        case '_':   // Single line leader
            nPara |= 3 << 3;
            break;
        case '-':   // hyphenated leader
            nPara |= 2 << 3;
            break;
        case '=':   // heavy line leader
            nPara |= 4 << 3;
            break;
    }

    ByteToSVBT8(nPara, pAddTyp + nAdd);
    ++nAdd;
}

// Del( const SvxTabStop & rTS ) fuegt einen zu loeschenden Tab
// in die WW-Struktur ein
void SwWW8WrTabu::Del(const SvxTabStop &rTS, long nAdjustment)
{
    // Tab-Position eintragen
    ShortToSVBT16(msword_cast<sal_Int16>(rTS.GetTabPos() + nAdjustment),
        pDel + (nDel * 2));
    ++nDel;
}

//  PutAll( SwWW8Writer& rWW8Wrt ) schreibt das Attribut nach rWrt.pO
void SwWW8WrTabu::PutAll(SwWW8Writer& rWrt)
{
    ASSERT(nAdd <= 255, "more than 255 added tabstops ?");
    ASSERT(nDel <= 255, "more than 244 removed tabstops ?");
    if (nAdd > 255)
        nAdd = 255;
    if (nDel > 255)
        nDel = 255;

    sal_uInt16 nSiz = 2 * nDel + 3 * nAdd + 2;
    if (nSiz > 255)
        nSiz = 255;

    if (rWrt.bWrtWW8)
        rWrt.InsUInt16(0xC60D);
    else
        rWrt.pO->Insert(15, rWrt.pO->Count());
    // cch eintragen
    rWrt.pO->Insert(msword_cast<sal_uInt8>(nSiz), rWrt.pO->Count());
    // DelArr schreiben
    rWrt.pO->Insert(msword_cast<sal_uInt8>(nDel), rWrt.pO->Count());
    rWrt.OutSprmBytes(pDel, nDel * 2);
    // InsArr schreiben
    rWrt.pO->Insert(msword_cast<sal_uInt8>(nAdd), rWrt.pO->Count());
    rWrt.OutSprmBytes(pAddPos, 2 * nAdd);         // AddPosArray
    rWrt.OutSprmBytes(pAddTyp, nAdd);             // AddTypArray
}


static void OutWW8_SwTabStopAdd(Writer& rWrt, const SvxTabStopItem& rTStops,
    long nLParaMgn)
{
    SwWW8WrTabu aTab( 0, rTStops.Count());

    for( USHORT n = 0; n < rTStops.Count(); n++ )
    {
        const SvxTabStop& rTS = rTStops[n];
        // Def-Tabs ignorieren
        if (SVX_TAB_ADJUST_DEFAULT != rTS.GetAdjustment())
            aTab.Add(rTS, nLParaMgn);
    }
    aTab.PutAll( (SwWW8Writer&)rWrt );
}

bool lcl_IsEqual(long nOneLeft, const SvxTabStop &rOne,
    long nTwoLeft, const SvxTabStop &rTwo)
{
    return(
            nOneLeft == nTwoLeft &&
            rOne.GetAdjustment() == rTwo.GetAdjustment() &&
            rOne.GetDecimal() == rTwo.GetDecimal() &&
            rOne.GetFill() == rTwo.GetFill()
          );
}

static void OutWW8_SwTabStopDelAdd(Writer& rWrt, const SvxTabStopItem& rTStyle,
    long nLStypeMgn, const SvxTabStopItem& rTNew, long nLParaMgn)
{
    SwWW8WrTabu aTab(rTStyle.Count(), rTNew.Count());

    USHORT nO = 0;      // rTStyle Index
    USHORT nN = 0;      // rTNew Index

    do {
        const SvxTabStop* pTO;
        long nOP;
        if( nO < rTStyle.Count() )                  // alt noch nicht am Ende ?
        {
            pTO = &rTStyle[ nO ];
            nOP = pTO->GetTabPos() + nLStypeMgn;
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
            nNP = pTN->GetTabPos() + nLParaMgn;
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
            aTab.Del(*pTO, nLStypeMgn);             // muss geloescht werden
            nO++;
        }
        else if( nNP < nOP )                        // naechster Tab ist neu
        {
            aTab.Add(*pTN, nLParaMgn);              // muss eigefuegt werden
            nN++;
        }
        else if (lcl_IsEqual(nOP, *pTO, nNP, *pTN)) // Tabs sind gleich:
        {
            nO++;                                   // nichts zu tun
            nN++;
        }
        else                                        // Tabs selbe Pos, diff Typ
        {
            aTab.Del(*pTO, nLStypeMgn);             // alten loeschen
            aTab.Add(*pTN, nLParaMgn);              // neuen einfuegen
            nO++;
            nN++;
        }
    } while( 1 );

    aTab.PutAll( (SwWW8Writer&)rWrt );
}

static Writer& OutWW8_SwTabStop(Writer& rWrt, const SfxPoolItem& rHt)
{
    SwWW8Writer& rWW8Wrt = (SwWW8Writer&)rWrt;
    const SvxTabStopItem & rTStops = (const SvxTabStopItem&)rHt;
    const SfxPoolItem* pLR = rWW8Wrt.HasItem( RES_LR_SPACE );
    long nCurrentLeft = pLR ? ((const SvxLRSpaceItem*)pLR)->GetTxtLeft() : 0;

    // StyleDef -> "einfach" eintragen || keine Style-Attrs -> dito
    const SvxTabStopItem* pStyleTabs = 0;
    if (!rWW8Wrt.bStyDef && rWW8Wrt.pStyAttr)
    {
        pStyleTabs =
            HasItem<SvxTabStopItem>(*rWW8Wrt.pStyAttr, RES_PARATR_TABSTOP);
    }

    if (!pStyleTabs)
        OutWW8_SwTabStopAdd(rWW8Wrt, rTStops, nCurrentLeft);
    else
    {
        const SvxLRSpaceItem &rStyleLR =
            ItemGet<SvxLRSpaceItem>(*rWW8Wrt.pStyAttr, RES_LR_SPACE);
        long nStyleLeft = rStyleLR.GetTxtLeft();

        OutWW8_SwTabStopDelAdd(rWW8Wrt, *pStyleTabs, nStyleLeft, rTStops,
            nCurrentLeft);
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
/* RES_CHRATR_BLINK */              OutWW8_SwAnimatedText, // neu: blinkender Text
/* RES_CHRATR_NOHYPHEN  */          0, // Neu: nicht trennen
/* RES_CHRATR_NOLINEBREAK */        0, // Neu: nicht umbrechen
/* RES_CHRATR_BACKGROUND */         OutWW8_SwFmtCharBackground,
/* RES_CHRATR_CJK_FONT */           OutWW8_SwCJKFont,
/* RES_CHRATR_CJK_FONTSIZE */       OutWW8_SwSize,
/* RES_CHRATR_CJK_LANGUAGE */       OutWW8_SwLanguage,
/* RES_CHRATR_CJK_POSTURE */        OutWW8_SwPosture,
/* RES_CHRATR_CJK_WEIGHT */         OutWW8_SwWeight,
/* RES_CHRATR_CTL_FONT */           OutWW8_SwCTLFont,
/* RES_CHRATR_CTL_FONTSIZE */       OutWW8_SwSize,
/* RES_CHRATR_CTL_LANGUAGE */       OutWW8_SwLanguage,
/* RES_CHRATR_CTL_POSTURE */        OutWW8_SwBiDiPosture,
/* RES_CHRATR_CTL_WEIGHT */         OutWW8_SwBiDiWeight,
/* RES_CHRATR_WRITING_DIRECTION */  OutWW8_CharRotate,
/* RES_CHRATR_EMPHASIS_MARK*/       OutWW8_EmphasisMark,
/* RES_TXTATR_TWO_LINES */          OutWW8_SvxTwoLinesItem,
/* RES_CHRATR_DUMMY4 */             OutWW8_ScaleWidth,
/* RES_CHRATR_RELIEF*/              OutWW8_Relief,
/* RES_CHRATR_HIDDEN */             OutWW8_SvxCharHidden,

/* RES_TXTATR_INETFMT */            OutSwFmtINetFmt,
/* RES_TXTATR_DUMMY4 */             0,
/* RES_TXTATR_REFMARK */            0,      // handled by SwAttrIter
/* RES_TXTATR_TOXMARK   */          0,      // handled by SwAttrIter
/* RES_TXTATR_CHARFMT   */          OutWW8_SwTxtCharFmt,
/* RES_TXTATR_DUMMY5*/              0,
/* RES_TXTATR_CJK_RUBY */           0,      // handled by SwAttrIter
/* RES_TXTATR_UNKNOWN_CONTAINER */  0,
/* RES_TXTATR_DUMMY6 */             0,
/* RES_TXTATR_DUMMY7 */             0,

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
/* RES_PARATR_SCRIPTSPACE */        OutWW8_SfxBoolItem,
/* RES_PARATR_HANGINGPUNCTUATION */ OutWW8_SfxBoolItem,
/* RES_PARATR_FORBIDDEN_RULES */    OutWW8_SfxBoolItem,
/* RES_PARATR_VERTALIGN */          OutWW8_SvxParaVertAlignItem,
/* RES_PARATR_SNAPTOGRID*/          OutWW8_SvxParaGridItem,
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
/* RES_PAGEDESC */                  OutWW8_SwFmtPageDesc,
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
/* RES_TEXTGRID*/                   OutWW8_SwTextGrid,
/* RES_LINENUMBER */                OutWW8_SwFmtLineNumber, // Line Numbering
/* RES_FTN_AT_TXTEND*/              0, // Dummy:
/* RES_END_AT_TXTEND*/              0, // Dummy:
/* RES_COLUMNBALANCE*/              0, // Dummy:
/* RES_FRAMEDIR*/                   OutWW8_SvxFrameDirection,
/* RES_FRMATR_DUMMY8 */             0, // Dummy:
/* RES_FRMATR_DUMMY9 */             0, // Dummy:
/* RES_FOLLOW_TEXT_FLOW */          0,
/* RES_WRAP_INFLUENCE_ON_OBJPOS */  0,
/* RES_FRMATR_DUMMY2 */             0, // Dummy:
/* RES_FRMATR_DUMMY3 */             0, // Dummy:
/* RES_FRMATR_DUMMY4 */             0, // Dummy:
/* RES_FRMATR_DUMMY5 */             0, // Dummy:

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

/* vi:set tabstop=4 shiftwidth=4 expandtab: */

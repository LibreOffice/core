/*************************************************************************
 *
 *  $RCSfile: wrtw8nds.cxx,v $
 *
 *  $Revision: 1.56 $
 *
 *  last change: $Author: hr $ $Date: 2003-11-05 14:15:58 $
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

#ifdef PCH
#include "filt_pch.hxx"
#endif

#pragma hdrstop

#ifndef __SGI_STL_VECTOR
#include <vector>
#endif
#ifndef __SGI_STL_LIST
#include <list>
#endif
#ifndef __SGI_STL_UTILITY
#include <utility>
#endif
#ifndef __SGI_STL_ALGORITHM
#include <algorithm>
#endif
#ifndef __SGI_STL_FUNCTIONAL
#include <functional>
#endif

#include <unicode/ubidi.h>

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _URLOBJ_HXX //autogen wg. INetURLObject
#include <tools/urlobj.hxx>
#endif
#ifndef _SVX_BOXITEM_HXX //autogen
#include <svx/boxitem.hxx>
#endif
#ifndef _SVX_CMAPITEM_HXX
#include <svx/cmapitem.hxx>
#endif
#ifndef _SVX_LANGITEM_HXX
#include <svx/langitem.hxx>
#endif
#ifndef _SVX_SVXFONT_HXX
#include <svx/svxfont.hxx>
#endif
#ifndef _SVX_LRSPITEM_HXX //autogen
#include <svx/lrspitem.hxx>
#endif
#ifndef _SVX_BRSHITEM_HXX //autogen
#include <svx/brshitem.hxx>
#endif
#ifndef _SVX_FONTITEM_HXX //autogen wg. SvxFontItem
#include <svx/fontitem.hxx>
#endif
#ifndef _SVX_FHGTITEM_HXX
#include <svx/fhgtitem.hxx>
#endif
#ifndef _SVX_ULSPITEM_HXX //autogen wg. SvxULSpaceItem
#include <svx/ulspitem.hxx>
#endif
#ifndef _SVX_BRKITEM_HXX //autogen
#include <svx/brkitem.hxx>
#endif
#ifndef _SVX_FRMDIRITEM_HXX
#include <svx/frmdiritem.hxx>
#endif

#ifndef _TOOLS_TENCCVT_HXX
#include <tools/tenccvt.hxx>
#endif
#ifndef _FMTPDSC_HXX //autogen
#include <fmtpdsc.hxx>
#endif
#ifndef _FMTFSIZE_HXX //autogen
#include <fmtfsize.hxx>
#endif
#ifndef _FMTORNT_HXX //autogen
#include <fmtornt.hxx>
#endif
#ifndef _FMTFLCNT_HXX //autogen
#include <fmtflcnt.hxx>
#endif
#ifndef _FMTANCHR_HXX //autogen
#include <fmtanchr.hxx>
#endif
#ifndef _FMTCNTNT_HXX //autogen
#include <fmtcntnt.hxx>
#endif
#ifndef _FRMATR_HXX //autogen
#include <frmatr.hxx>
#endif
#ifndef _PARATR_HXX //autogen
#include <paratr.hxx>
#endif
#ifndef _TXATBASE_HXX //autogen
#include <txatbase.hxx>
#endif
#ifndef _FMTINFMT_HXX //autogen wg. SwFmtINetFmt
#include <fmtinfmt.hxx>
#endif
#ifndef _FMTRFMRK_HXX //autogen wg. SwFmtRefMark
#include <fmtrfmrk.hxx>
#endif
#ifndef _FCHRFMT_HXX //autogen wg. SwFmtCharFmt
#include <fchrfmt.hxx>
#endif
#ifndef _CHARFMT_HXX //autogen wg. SwCharFmt
#include <charfmt.hxx>
#endif
#ifndef _TOX_HXX //autogen wg. SwTOXMark
#include <tox.hxx>
#endif
#ifndef _NDTXT_HXX //autogen
#include <ndtxt.hxx>
#endif
#ifndef _PAM_HXX //autogen
#include <pam.hxx>
#endif
#ifndef _DOC_HXX //autogen
#include <doc.hxx>
#endif
#ifndef _DOCARY_HXX //autogen
#include <docary.hxx>
#endif
#ifndef _SWTABLE_HXX //autogen
#include <swtable.hxx>
#endif
#ifndef _SECTION_HXX //autogen
#include <section.hxx>
#endif
#ifndef _PAGEDESC_HXX //autogen
#include <pagedesc.hxx>
#endif
#ifndef _SWRECT_HXX //autogen
#include <swrect.hxx>
#endif
#ifndef _FLYPOS_HXX
#include <flypos.hxx>           // SwPosFlyFrms
#endif
#ifndef _REFFLD_HXX //autogen wg. SwGetRefField
#include <reffld.hxx>
#endif
#ifndef _REDLINE_HXX
#include <redline.hxx>
#endif
#ifndef _WRTSWTBL_HXX
#include <wrtswtbl.hxx>
#endif
#ifndef _HTMLTBL_HXX
#include <htmltbl.hxx>
#endif
#ifndef _TXTTXMRK_HXX
#include <txttxmrk.hxx>
#endif
#ifndef SW_FMTLINE_HXX
#include <fmtline.hxx>
#endif
#ifndef _FMTRUBY_HXX
#include <fmtruby.hxx>
#endif
#ifndef _BREAKIT_HXX
#include <breakit.hxx>
#endif
#ifndef _TXTATR_HXX
#include <txtatr.hxx>
#endif
#ifndef _COM_SUN_STAR_I18N_SCRIPTTYPE_HDL_
#include <com/sun/star/i18n/ScriptType.hdl>
#endif
#ifndef _COM_SUN_STAR_I18N_WORDTYPE_HDL_
#include <com/sun/star/i18n/WordType.hdl>
#endif

#ifndef INCLUDED_I18NUTIL_UNICODE_HXX
#include <i18nutil/unicode.hxx>
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

using namespace com::sun::star::i18n;
using namespace sw::util;
using namespace sw::types;

/*  */

WW8_AttrIter::WW8_AttrIter(SwWW8Writer& rWr)
    : pOld(rWr.pChpIter), rWrt(rWr)
{
    rWrt.pChpIter = this;
}

WW8_AttrIter::~WW8_AttrIter()
{
    rWrt.pChpIter = pOld;
}

/*  */

// Die Klasse WW8_SwAttrIter ist eine Hilfe zum Aufbauen der Fkp.chpx.
// Dabei werden nur Zeichen-Attribute beachtet; Absatz-Attribute brauchen
// diese Behandlung nicht.
// Die Absatz- und Textattribute des Writers kommen rein, und es wird
// mit Where() die naechste Position geliefert, an der sich die Attribute
// aendern. IsTxtAtr() sagt, ob sich an der mit Where() gelieferten Position
// ein Attribut ohne Ende und mit \xff im Text befindet.
// Mit OutAttr() werden die Attribute an der angegebenen SwPos
// ausgegeben.

class swFlyFrm
{
public:
    const SwFrmFmt* mpFlyFrm;
    const SwCntntNode *mpNode;
    SwPosition maPos;
    swFlyFrm(const SwFrmFmt* pFlyFrm, const SwCntntNode *pNode,
        const SwPosition &rPos)
    :   mpFlyFrm(pFlyFrm), mpNode(pNode), maPos(rPos) {}
};

class WW8_SwAttrIter : public WW8_AttrIter
{
private:
    const SwTxtNode& rNd;

    typedef std::pair<xub_StrLen, sal_Int16> CharSetEntry;
    std::vector<CharSetEntry> maCharSets;
    typedef std::vector<CharSetEntry>::iterator mychsiter;
    mychsiter maCharSetIter;
    rtl_TextEncoding meChrSet;

    typedef std::pair<xub_StrLen, sal_uInt16> ScriptEntry;
    std::vector<ScriptEntry> maScripts;
    typedef std::vector<ScriptEntry>::iterator mysiter;
    mysiter maScriptIter;
    sal_uInt16 mnScript;


    const SwRedline* pCurRedline;
    xub_StrLen nAktSwPos;
    USHORT nCurRedlinePos;

    typedef std::pair<UTextOffset, bool> DirEntry;
    std::vector<DirEntry> maDirChanges;
    typedef std::vector<DirEntry>::const_iterator myciter;
    myciter maBiDiIter;
    bool mbCharIsRTL;
    bool mbParaIsRTL;

    SwFmtDrop* pSwFmtDrop;

    std::vector<swFlyFrm> maFlyFrms;     // #i2916#
    typedef std::vector<swFlyFrm>::iterator myflyiter;
    myflyiter maFlyIter;

    xub_StrLen SearchNext( xub_StrLen nStartPos );
    void FieldVanish( const String& rTxt );

    void OutSwFmtINetFmt(const SwFmtINetFmt& rAttr, bool bStart);
    void OutSwFmtRefMark(const SwFmtRefMark& rAttr, bool bStart);
    void OutSwTOXMark(const SwTOXMark& rAttr, bool bStart);
    void OutSwFmtRuby(const SwFmtRuby& rRuby, bool bStart);
    sal_Int16 getScriptClass(sal_Unicode cChar) const;

    //No copying
    WW8_SwAttrIter(const WW8_SwAttrIter&);
    WW8_SwAttrIter& operator=(const WW8_SwAttrIter&);
public:
    WW8_SwAttrIter( SwWW8Writer& rWr, const SwTxtNode& rNd );

    bool IsTxtAttr( xub_StrLen nSwPos );
    bool IsRedlineAtEnd( xub_StrLen nPos ) const;
    bool IsDropCap( int nSwPos );

    void NextPos() { nAktSwPos = SearchNext( nAktSwPos + 1 ); }

    void OutAttr( xub_StrLen nSwPos );
    virtual const SfxPoolItem* HasTextItem( USHORT nWhich ) const;
    virtual const SfxPoolItem& GetItem( USHORT nWhich ) const;
    int OutAttrWithRange(xub_StrLen nPos);
    void OutRedlines(xub_StrLen nPos);
    void OutFlys(xub_StrLen nSwPos);

    xub_StrLen WhereNext() const    { return nAktSwPos; }
    bool IsCharRTL() const { return mbCharIsRTL; }
    bool IsParaRTL() const { return mbParaIsRTL; }
    rtl_TextEncoding GetCharSet() const { return meChrSet; }
    String GetSnippet(const String &rStr, xub_StrLen nAktPos,
        xub_StrLen nLen) const;
    SwFmtDrop* GetSwFmtDrop() const                 { return pSwFmtDrop; }
};

void SwWW8Writer::push_charpropstart(xub_StrLen nPos)
{
    maCurrentCharPropStarts.push(nPos);
}

void SwWW8Writer::pop_charpropstart()
{
    ASSERT(!maCurrentCharPropStarts.empty(), "cannot be empty!");
    if (!maCurrentCharPropStarts.empty())
        maCurrentCharPropStarts.pop();
}

xub_StrLen SwWW8Writer::top_charpropstart() const
{
    ASSERT(!maCurrentCharPropStarts.empty(), "cannot be empty!");
    return maCurrentCharPropStarts.empty() ? 0 : maCurrentCharPropStarts.top();
}

bool SwWW8Writer::empty_charpropstart() const
{
    return maCurrentCharPropStarts.empty();
}

class sortswflys :
    public std::binary_function<const swFlyFrm&, const swFlyFrm&, bool>
{
public:
    bool operator()(const swFlyFrm &rOne, const swFlyFrm &rTwo) const
    {
        return rOne.maPos.nContent.GetIndex() < rTwo.maPos.nContent.GetIndex();
    }
};

sal_Int16 WW8_SwAttrIter::getScriptClass(sal_Unicode cChar) const
{
    static ScriptTypeList aScripts[] =
    {
        { UnicodeScript_kBasicLatin, RTL_TEXTENCODING_MS_1252},
        { UnicodeScript_kLatin1Supplement, RTL_TEXTENCODING_MS_1252},
        { UnicodeScript_kLatinExtendedA, RTL_TEXTENCODING_MS_1250},
        { UnicodeScript_kLatinExtendedB, RTL_TEXTENCODING_MS_1257},
        { UnicodeScript_kGreek, RTL_TEXTENCODING_MS_1253},
        { UnicodeScript_kCyrillic, RTL_TEXTENCODING_MS_1251},
        { UnicodeScript_kHebrew, RTL_TEXTENCODING_MS_1255},
        { UnicodeScript_kArabic, RTL_TEXTENCODING_MS_1256},
        { UnicodeScript_kThai, RTL_TEXTENCODING_MS_1258},
        { UnicodeScript_kScriptCount, RTL_TEXTENCODING_MS_1252}
    };

    return unicode::getUnicodeScriptType(cChar, aScripts,
        RTL_TEXTENCODING_MS_1252);
}

WW8_SwAttrIter::WW8_SwAttrIter(SwWW8Writer& rWr, const SwTxtNode& rTxtNd)
    : WW8_AttrIter(rWr), rNd(rTxtNd), pCurRedline(0), nAktSwPos(0),
    nCurRedlinePos(USHRT_MAX), mbCharIsRTL(false)
{
    SwPosition aPos(rTxtNd);
    if (FRMDIR_HORI_RIGHT_TOP == rWr.pDoc->GetTextDirection(aPos))
        mbParaIsRTL = true;
    else
        mbParaIsRTL = false;

    pSwFmtDrop = new SwFmtDrop(rTxtNd.GetSwAttrSet().GetDrop());
    const String &rTxt = rTxtNd.GetTxt();

    if (rTxt.Len() && pBreakIt->xBreak.is())
        mnScript = pBreakIt->xBreak->getScriptType(rTxt, 0);
    else
        mnScript = ScriptType::LATIN;

    // Attributwechsel an Pos 0 wird ignoriert, da davon ausgegangen
    // wird, dass am Absatzanfang sowieso die Attribute neu ausgegeben
    // werden.
    meChrSet = ItemGet<SvxFontItem>(rNd,
        GetWhichOfScript(RES_CHRATR_FONT, mnScript)).GetCharSet();
    meChrSet = GetExtendedTextEncoding(meChrSet);

    if (rTxt.Len())
    {
        UBiDiDirection eDefaultDir = mbParaIsRTL ? UBIDI_RTL : UBIDI_LTR;
        UErrorCode nError = U_ZERO_ERROR;
        UBiDi* pBidi = ubidi_openSized(rTxt.Len(), 0, &nError);
        ubidi_setPara(pBidi, rTxt.GetBuffer(), rTxt.Len(), eDefaultDir, NULL,
            &nError);

        sal_Int32 nCount = ubidi_countRuns(pBidi, &nError);
        maDirChanges.reserve(nCount);

        UTextOffset nStart = 0;
        UTextOffset nEnd;
        UBiDiLevel nCurrDir;

        for (sal_Int32 nIdx = 0; nIdx < nCount; ++nIdx)
        {
            ubidi_getLogicalRun(pBidi, nStart, &nEnd, &nCurrDir);
            /*
            UBiDiLevel is the type of the level values in this BiDi
            implementation.

            It holds an embedding level and indicates the visual direction by
            its bit 0 (even/odd value).

            The value for UBIDI_DEFAULT_LTR is even and the one for
            UBIDI_DEFAULT_RTL is odd
            */
            maDirChanges.push_back(DirEntry(nEnd, nCurrDir & 0x1));
            nStart = nEnd;
        }
        ubidi_close(pBidi);

        if (!maDirChanges.empty())
            mbCharIsRTL = maDirChanges.begin()->second;

        //Split unicode text into plausable 8bit ranges for export to older
        //non unicode aware format
        if (!rWrt.bWrtWW8)
        {
            xub_StrLen nLen = rTxt.Len();
            xub_StrLen nPos = 0;
            while (nPos != nLen)
            {
                sal_Int16 ScriptType = getScriptClass(rTxt.GetChar(nPos++));
                while (
                        (nPos != nLen) &&
                        (ScriptType == getScriptClass(rTxt.GetChar(nPos)))
                      )
                {
                    ++nPos;
                }

                if (maCharSets.empty())
                    meChrSet = ScriptType;

                maCharSets.push_back(CharSetEntry(nPos, ScriptType));
            }
        }

        if (pBreakIt->xBreak.is())
        {
            xub_StrLen nLen = rTxt.Len();
            xub_StrLen nPos = 0;
            sal_uInt16 nScript = mnScript;
            while (nPos < nLen)
            {
                sal_uInt32 nEnd =
                    pBreakIt->xBreak->endOfScript(rTxt, nPos, nScript);
                if (nEnd == -1)
                    break;
                nPos = writer_cast<xub_StrLen>(nEnd);
                maScripts.push_back(ScriptEntry(nPos, nScript));
                nScript = pBreakIt->xBreak->getScriptType(rTxt, nPos);
            }
        }

    }
    maBiDiIter = maDirChanges.begin();
    maCharSetIter = maCharSets.begin();
    maScriptIter = maScripts.begin();

    /*
     #i2916#
     Get list of any graphics which may be anchored from this paragraph.
    */
    ULONG nCurPos = rNd.GetIndex();
    for (USHORT n = rWr.maFlyPos.Count(); n > 0;)
    {
        SwNodeIndex aIdx = rWr.maFlyPos[--n]->GetNdIndex();
        ULONG nFlyNdPos = aIdx.GetIndex();

        if (nFlyNdPos == nCurPos)
        {
            const SwFrmFmt* pEntry = &rWr.maFlyPos[n]->GetFmt();
            const SwPosition* pAnchor = pEntry->GetAnchor().GetCntntAnchor();
            if (pAnchor)
            {
                maFlyFrms.push_back(swFlyFrm(pEntry,
                    aIdx.GetNode().GetCntntNode(), *pAnchor));
            }
            else
            {
                SwPosition aPos(aIdx);
                if (SwTxtNode* pTxtNd = aIdx.GetNode().GetTxtNode())
                    aPos.nContent.Assign(pTxtNd, 0);
                maFlyFrms.push_back(swFlyFrm(pEntry,
                    aIdx.GetNode().GetCntntNode(), aPos));
            }
        }
    }

    std::sort(maFlyFrms.begin(), maFlyFrms.end(), sortswflys());

    maFlyIter = maFlyFrms.begin();

    if (rWrt.pDoc->GetRedlineTbl().Count())
    {
        SwPosition aPos( rNd, SwIndex( (SwTxtNode*)&rNd ) );
        pCurRedline = rWrt.pDoc->GetRedline( aPos, &nCurRedlinePos );
    }
    nAktSwPos = SearchNext( 1 );
}

xub_StrLen WW8_SwAttrIter::SearchNext( xub_StrLen nStartPos )
{
    xub_StrLen nPos;
    xub_StrLen nMinPos = STRING_MAXLEN;
    xub_StrLen i=0;

    // first the redline, then the attributes
    if( pCurRedline )
    {
        const SwPosition* pEnd = pCurRedline->End();
        if (pEnd->nNode == rNd && ((i = pEnd->nContent.GetIndex()) >= nStartPos))
            nMinPos = i;
    }

    if( nCurRedlinePos < rWrt.pDoc->GetRedlineTbl().Count() )
    {
        // nCurRedlinePos point to the next redline
        nPos = nCurRedlinePos;
        if( pCurRedline )
            ++nPos;

        for( ; nPos < rWrt.pDoc->GetRedlineTbl().Count(); ++nPos )
        {
            const SwRedline* pRedl = rWrt.pDoc->GetRedlineTbl()[ nPos ];

            const SwPosition* pStt = pRedl->Start();
            const SwPosition* pEnd = pStt == pRedl->GetPoint()
                                        ? pRedl->GetMark()
                                        : pRedl->GetPoint();

            if( pStt->nNode == rNd )
            {
                if( ( i = pStt->nContent.GetIndex() ) >= nStartPos &&
                    i < nMinPos )
                    nMinPos = i;
            }
            else
                break;

            if( pEnd->nNode == rNd &&
                ( i = pEnd->nContent.GetIndex() ) < nMinPos &&
                i >= nStartPos )
                    nMinPos = i;
        }
    }

    if(pSwFmtDrop->GetWholeWord() && nStartPos <= rNd.GetDropLen(0))
        nMinPos = rNd.GetDropLen(0);
    else if(nStartPos <= pSwFmtDrop->GetChars())
        nMinPos = pSwFmtDrop->GetChars();

    if(const SwpHints* pTxtAttrs = rNd.GetpSwpHints())
    {

// kann noch optimiert werden, wenn ausgenutzt wird, dass die TxtAttrs
// nach der Anfangsposition geordnet sind. Dann muessten
// allerdings noch 2 Indices gemerkt werden
        for( i = 0; i < pTxtAttrs->Count(); i++ )
        {
            const SwTxtAttr* pHt = (*pTxtAttrs)[i];
            nPos = *pHt->GetStart();    // gibt erstes Attr-Zeichen
            if( nPos >= nStartPos && nPos <= nMinPos )
                nMinPos = nPos;

            if( pHt->GetEnd() )         // Attr mit Ende
            {
                nPos = *pHt->GetEnd();      // gibt letztes Attr-Zeichen + 1
                if( nPos >= nStartPos && nPos <= nMinPos )
                    nMinPos = nPos;
            }
            else
            {
                // Attr ohne Ende Laenge 1 wegen CH_TXTATR im Text
                nPos = *pHt->GetStart() + 1;
                if( nPos >= nStartPos && nPos <= nMinPos )
                    nMinPos = nPos;
            }
        }
    }

    if (maBiDiIter != maDirChanges.end())
    {
        if (maBiDiIter->first < nMinPos)
            nMinPos = writer_cast<xub_StrLen>(maBiDiIter->first);
        mbCharIsRTL = maBiDiIter->second;
    }

    if (maCharSetIter != maCharSets.end())
    {
        if (maCharSetIter->first < nMinPos)
            nMinPos = maCharSetIter->first;
        meChrSet = maCharSetIter->second;
    }

    if (maScriptIter != maScripts.end())
    {
        if (maScriptIter->first < nMinPos)
            nMinPos = maScriptIter->first;
        mnScript = maScriptIter->second;
    }

    /*
     #i2916#
     Check to see if there are any graphics anchored to characters in this
     paragraph's text.  Set nMinPos to 1 past the placement for anchored to
     character because anchors in Word appear after the character they are
     anchored to.
    */
    if (maFlyIter != maFlyFrms.end())
    {
        const SwPosition &rAnchor = maFlyIter->maPos;

        nPos = rAnchor.nContent.GetIndex();
        if (nPos >= nStartPos && nPos <= nMinPos)
            nMinPos = nPos;

        if (maFlyIter->mpFlyFrm->GetAnchor().GetAnchorId() == FLY_AUTO_CNTNT)
        {
            ++nPos;
            if (nPos >= nStartPos && nPos <= nMinPos)
                nMinPos = nPos;
        }
    }

    //nMinPos found and not going to change at this point

    if (maBiDiIter != maDirChanges.end())
    {
        if (maBiDiIter->first == nMinPos)
            ++maBiDiIter;
    }

    if (maCharSetIter != maCharSets.end())
    {
        if (maCharSetIter->first == nMinPos)
            ++maCharSetIter;
    }

    if (maScriptIter != maScripts.end())
    {
        if (maScriptIter->first == nMinPos)
            ++maScriptIter;
    }

    return nMinPos;
}

void WW8_SwAttrIter::OutAttr( xub_StrLen nSwPos )
{
    sal_uInt16 nFontId = GetWhichOfScript(RES_CHRATR_FONT, mnScript);
    const SvxFontItem &rParentFont = ItemGet<SvxFontItem>(
        (const SwTxtFmtColl&)rNd.GetAnyFmtColl(), nFontId);
    const SvxFontItem *pFont = &rParentFont;

    if (rNd.GetpSwAttrSet())
    {
        SfxItemSet aSet(rNd.GetSwAttrSet());
        const SvxFontItem &rFontInUse = ItemGet<SvxFontItem>(aSet, nFontId);
        pFont = &rFontInUse;
        aSet.ClearItem(nFontId);
        rWrt.Out_SfxItemSet(aSet, false, true, mnScript);
    }

    if (rWrt.bWrtWW8 && IsCharRTL())
    {
        rWrt.InsUInt16(0x85a);
        rWrt.pO->Insert((BYTE)1, rWrt.pO->Count());
    }

    if (const SwpHints* pTxtAttrs = rNd.GetpSwpHints())
    {
        const SwModify* pOldMod = rWrt.pOutFmtNode;
        rWrt.pOutFmtNode = &rNd;

        rWrt.push_charpropstart(nSwPos);
        for (xub_StrLen i = 0; i < pTxtAttrs->Count(); ++i)
        {
            const SwTxtAttr* pHt = (*pTxtAttrs)[i];
            const xub_StrLen* pEnd = pHt->GetEnd();

            if( pEnd ? ( nSwPos >= *pHt->GetStart() && nSwPos < *pEnd )
                        : nSwPos == *pHt->GetStart() )
            {
                if (rWrt.CollapseScriptsforWordOk(mnScript,
                    pHt->GetAttr().Which()))
                {
                    if (pHt->GetAttr().Which() == nFontId)
                        pFont = &(item_cast<SvxFontItem>(pHt->GetAttr()));
                    else
                        Out(aWW8AttrFnTab, pHt->GetAttr(), rWrt);
                }
            }
            else if (nSwPos < *pHt->GetStart())
                break;
        }
        rWrt.pop_charpropstart();   // HasTextItem nur in dem obigen Bereich erlaubt
        rWrt.pOutFmtNode = pOldMod;
    }

    ASSERT(pFont, "must be *some* font associated with this txtnode");
    if (pFont)
    {
        SvxFontItem aFont(*pFont);

        /*
         If we are a nonunicode aware format then we set the charset we want to
         use for export of this range. If necessary this will generate a pseudo
         font to use for this range.
        */
        if (!rWrt.bWrtWW8)
            aFont.GetCharSet() = meChrSet;

        if (rParentFont != aFont)
            Out(aWW8AttrFnTab, aFont, rWrt);
    }

    OutRedlines(nSwPos);
}

void WW8_SwAttrIter::OutFlys(xub_StrLen nSwPos)
{
    /*
     #i2916#
     May have an anchored graphic to be placed, loop through sorted array
     and output all at this position
    */
    while (maFlyIter != maFlyFrms.end())
    {
        const SwPosition &rAnchor = maFlyIter->maPos;
        xub_StrLen nPos = rAnchor.nContent.GetIndex();

        if (nPos != nSwPos)
            break;
        else
        {
            SwWW8Writer& rWrtWW8 = (SwWW8Writer&)rWrt;
            rWrtWW8.OutFlyFrm(*maFlyIter->mpNode, *maFlyIter->mpFlyFrm);
            ++maFlyIter;
        }
    }
}

bool WW8_SwAttrIter::IsTxtAttr( xub_StrLen nSwPos )
{
    // search for attrs without end position
    if (const SwpHints* pTxtAttrs = rNd.GetpSwpHints())
    {
        for (USHORT i = 0; i < pTxtAttrs->Count(); ++i)
        {
            const SwTxtAttr* pHt = (*pTxtAttrs)[i];
            if( !pHt->GetEnd() && *pHt->GetStart() == nSwPos )
                return true;
        }
    }

    return false;
}

bool WW8_SwAttrIter::IsDropCap( int nSwPos )
{
    // see if the current position falls on a DropCap
    if(pSwFmtDrop)
    {
        int nDropChars = pSwFmtDrop->GetChars();
        bool bWholeWord = pSwFmtDrop->GetWholeWord();
        if (bWholeWord)
        {
            short nWordLen = rNd.GetDropLen(0);
            if(nSwPos == nWordLen && nSwPos != 0)
                return true;
        }
        else
        {
            if (nSwPos == nDropChars && nSwPos != 0)
                return true;
        }
    }

    return false;
}

// HasItem ist fuer die Zusammenfassung des Doppel-Attributes Underline
// und WordLineMode als TextItems. OutAttr() ruft die Ausgabefunktion,
// die dann ueber HasItem() nach anderen Items an der
// Attribut-Anfangposition fragen kann.
// Es koennen nur Attribute mit Ende abgefragt werden.
// Es wird mit bDeep gesucht
const SfxPoolItem* WW8_SwAttrIter::HasTextItem( USHORT nWhich ) const
{
    const SfxPoolItem* pRet = 0;
    const SwpHints* pTxtAttrs = rNd.GetpSwpHints();
    xub_StrLen nTmpSwPos = rWrt.top_charpropstart();
    if (pTxtAttrs)
    {
        for (USHORT i = 0; i < pTxtAttrs->Count(); ++i)
        {
            const SwTxtAttr* pHt = (*pTxtAttrs)[i];
            const SfxPoolItem* pItem = &pHt->GetAttr();
            const xub_StrLen* pAtrEnd = 0;
            if( 0 != ( pAtrEnd = pHt->GetEnd() ) &&     // nur Attr mit Ende
                nWhich == pItem->Which() &&             //
                nTmpSwPos >= *pHt->GetStart() && nTmpSwPos < *pAtrEnd )
            {
                pRet = pItem;       // gefunden
                break;
            }
            else if (nTmpSwPos < *pHt->GetStart())
                break;              // dann kommt da nichts mehr
        }
    }
    return pRet;
}

void SwWW8Writer::GetCurrentItems(WW8Bytes& rItems) const
{
    USHORT nEnd = pO ? pO->Count() : 0;
    for (USHORT nI = 0; nI < nEnd; ++nI)
        rItems.Insert((*pO)[nI], rItems.Count());
}

const SfxPoolItem& WW8_SwAttrIter::GetItem(USHORT nWhich) const
{
    const SfxPoolItem* pRet = HasTextItem(nWhich);
    return pRet ? *pRet : rNd.SwCntntNode::GetAttr(nWhich);
}

void WW8_SwAttrIter::OutSwFmtRuby(const SwFmtRuby& rRuby, bool bStart)
{
    if (bStart)
    {
        String aStr(FieldString(ww::eEQ));
        aStr.APPEND_CONST_ASC("\\* jc");
        sal_Int32 nJC=0;
        sal_Char cDirective=0;
        switch(rRuby.GetAdjustment())
        {
            case 0:
                nJC = 3;
                cDirective = 'l';
                break;
            case 1:
                //defaults to 0
                break;
            case 2:
                nJC = 4;
                cDirective = 'r';
                break;
            case 3:
                nJC = 1;
                cDirective = 'd';
                break;
            case 4:
                nJC = 2;
                cDirective = 'd';
                break;
            default:
                ASSERT(!this,"Unhandled Ruby justication code");
                break;
        }
        aStr += String::CreateFromInt32(nJC);

        /*
         MS needs to know the name and size of the font used in the ruby item,
         but we coud have written it in a mixture of asian and western
         scripts, and each of these can be a different font and size than the
         other, so we make a guess based upon the first character of the text,
         defaulting to asian.
         */
        USHORT nRubyScript;
        if( pBreakIt->xBreak.is() )
            nRubyScript = pBreakIt->xBreak->getScriptType( rRuby.GetText(), 0);
        else
            nRubyScript = ScriptType::ASIAN;

        const SwTxtRuby* pRubyTxt = rRuby.GetTxtRuby();
        const SwCharFmt* pFmt = pRubyTxt ? pRubyTxt->GetCharFmt() : 0;
        String sFamilyName;
        long nHeight;
        if (pFmt)
        {
            const SvxFontItem &rFont = ItemGet<SvxFontItem>(*pFmt,
                GetWhichOfScript(RES_CHRATR_FONT,nRubyScript));
            sFamilyName = rFont.GetFamilyName();

            const SvxFontHeightItem &rHeight = ItemGet<SvxFontHeightItem>(*pFmt,
                GetWhichOfScript(RES_CHRATR_FONTSIZE,nRubyScript));
            nHeight = rHeight.GetHeight();
        }
        else
        {
            /*Get defaults if no formatting on ruby text*/

            const SfxItemPool *pPool = rNd.GetSwAttrSet().GetPool();
            const SfxItemPool &rPool =
                pPool ? *pPool : rWrt.pDoc->GetAttrPool();

            const SvxFontItem &rFont  = DefaultItemGet<SvxFontItem>(rPool,
                GetWhichOfScript(RES_CHRATR_FONT,nRubyScript));
            sFamilyName = rFont.GetFamilyName();

            const SvxFontHeightItem &rHeight = DefaultItemGet<SvxFontHeightItem>
                (rPool, GetWhichOfScript(RES_CHRATR_FONTSIZE,nRubyScript));
            nHeight = rHeight.GetHeight();
        }
        nHeight = (nHeight + 5)/10;

        aStr.APPEND_CONST_ASC(" \\* \"Font:");
        aStr.Append(sFamilyName);
        aStr.APPEND_CONST_ASC("\" \\* hps");
        aStr += String::CreateFromInt32(nHeight);
        aStr.APPEND_CONST_ASC(" \\o");
        if (cDirective)
        {
            aStr.APPEND_CONST_ASC("\\a");
            aStr.Append(cDirective);
        }
        aStr.APPEND_CONST_ASC("(\\s\\up ");


        if( pBreakIt->xBreak.is() )
            nRubyScript = pBreakIt->xBreak->getScriptType( rNd.GetTxt(),
                *(pRubyTxt->GetStart()));
        else
            nRubyScript = ScriptType::ASIAN;

        const SwAttrSet& rSet = rNd.GetSwAttrSet();
        const SvxFontHeightItem &rHeightItem  =
            (const SvxFontHeightItem&)rSet.Get(
            GetWhichOfScript(RES_CHRATR_FONTSIZE,nRubyScript));
        nHeight = (rHeightItem.GetHeight() + 10)/20-1;
        aStr += String::CreateFromInt32(nHeight);
        aStr += '(';
        aStr += rRuby.GetText();
        aStr.APPEND_CONST_ASC(");");
        rWrt.OutField(0, ww::eEQ, aStr,
            WRITEFIELD_START | WRITEFIELD_CMD_START);
    }
    else
    {
        rWrt.WriteChar(')');
        rWrt.OutField(0, ww::eEQ, aEmptyStr, WRITEFIELD_END | WRITEFIELD_CLOSE);
    }
}

void WW8_SwAttrIter::OutSwFmtINetFmt(const SwFmtINetFmt& rINet, bool bStart)
{
    if( bStart )
        StartURL(rINet.GetValue(), rINet.GetTargetFrame());
    else
        EndURL();
}

/*#i15387# Better ideas welcome*/
String &TruncateBookmark(String &rRet)
{
    if (rRet.Len() > 40)
        rRet.Erase(40);
    ASSERT(rRet.Len() <= 40, "Word cannot have bookmarks longer than 40 chars");
    return rRet;
}

void WW8_AttrIter::StartURL(const String &rUrl, const String &rTarget)
{
    using namespace ww;
    bool bBookMarkOnly = false;
    INetURLObject aURL(rUrl);
    String sURL;
    String sMark;

    if (rUrl.Len() > 1 && rUrl.GetChar(0) == INET_MARK_TOKEN)
    {
        sMark = rUrl.Copy(1);
        TruncateBookmark(sMark);
    }
    else
    {
        sURL = aURL.GetURLNoMark(INetURLObject::DECODE_UNAMBIGUOUS);
        sMark = aURL.GetMark(INetURLObject::DECODE_UNAMBIGUOUS);

        sURL = INetURLObject::AbsToRel(sURL, INetURLObject::WAS_ENCODED,
            INetURLObject::DECODE_UNAMBIGUOUS);
    }

    if (sMark.Len() && !sURL.Len())
        bBookMarkOnly = true;

    if (bBookMarkOnly)
        sURL = FieldString(eHYPERLINK);
    else
    {
        String sFld(FieldString(eHYPERLINK));
        sFld.APPEND_CONST_ASC("\"");
        sURL.Insert(sFld, 0);
        sURL += '\"';
    }

    if (sMark.Len())
        (( sURL.APPEND_CONST_ASC(" \\l \"") ) += sMark) += '\"';

    if (rTarget.Len())
        (sURL.APPEND_CONST_ASC(" \\n ")) += rTarget;

    rWrt.OutField(0, eHYPERLINK, sURL, WRITEFIELD_START | WRITEFIELD_CMD_START);

    // write the refence to the "picture" structure
    ULONG nDataStt = rWrt.pDataStrm->Tell();
    rWrt.pChpPlc->AppendFkpEntry( rWrt.Strm().Tell() );

//  WinWord 2000 doesn't write this - so its a temp solution by W97 ?
    rWrt.WriteChar( 0x01 );

    static BYTE aArr1[] = {
        0x03, 0x6a, 0,0,0,0,    // sprmCPicLocation

        0x06, 0x08, 0x01,       // sprmCFData
        0x55, 0x08, 0x01,       // sprmCFSpec
        0x02, 0x08, 0x01        // sprmCFFldVanish
    };
    BYTE* pDataAdr = aArr1 + 2;
    Set_UInt32( pDataAdr, nDataStt );

    rWrt.pChpPlc->AppendFkpEntry( rWrt.Strm().Tell(), sizeof( aArr1 ), aArr1 );

    rWrt.OutField(0, eHYPERLINK, sURL, WRITEFIELD_CMD_END);

    // now write the picture structur
    sURL = aURL.GetURLNoMark();

    //all links end up in the data stream as absolute references.
    bool bAbsolute = !bBookMarkOnly;

    static BYTE __READONLY_DATA aURLData1[] = {
        0,0,0,0,        // len of struct
        0x44,0,         // the start of "next" data
        0,0,0,0,0,0,0,0,0,0,                // PIC-Structure!
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,    //  |
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,    //  |
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,    //  |
        0,0,0,0,                            // /
    };
    static BYTE __READONLY_DATA MAGIC_A[] = {
        // start of "next" data
        0xD0,0xC9,0xEA,0x79,0xF9,0xBA,0xCE,0x11,
        0x8C,0x82,0x00,0xAA,0x00,0x4B,0xA9,0x0B
    };

    rWrt.pDataStrm->Write( aURLData1, sizeof(aURLData1) );
    BYTE nAnchor=0x00;
    if( sMark.Len() )
        nAnchor=0x08;
    rWrt.pDataStrm->Write( &nAnchor, 1 );
    rWrt.pDataStrm->Write( MAGIC_A, sizeof(MAGIC_A) );
    SwWW8Writer::WriteLong( *rWrt.pDataStrm, 0x00000002);
    UINT32 nFlag = bBookMarkOnly ? 0 : 0x01;
    if (bAbsolute) nFlag |= 0x02;
    if( sMark.Len() ) nFlag |= 0x08;
    SwWW8Writer::WriteLong( *rWrt.pDataStrm, nFlag );

    INetProtocol eProto = aURL.GetProtocol();
    if (eProto == INET_PROT_FILE)
    {
// version 1 (for a document)

        static BYTE __READONLY_DATA MAGIC_C[] = {
            0x03, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46,
            0x00, 0x00
        };

        static BYTE __READONLY_DATA MAGIC_D[] = {
            0xFF, 0xFF, 0xAD, 0xDE, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        };

        sURL = aURL.PathToFileName();

        rWrt.pDataStrm->Write( MAGIC_C, sizeof(MAGIC_C) );
        SwWW8Writer::WriteLong( *rWrt.pDataStrm, sURL.Len()+1 );
        SwWW8Writer::WriteString8( *rWrt.pDataStrm, sURL, true,
                                    RTL_TEXTENCODING_MS_1252 );
        rWrt.pDataStrm->Write( MAGIC_D, sizeof( MAGIC_D) );

        SwWW8Writer::WriteLong( *rWrt.pDataStrm, 2*sURL.Len()+6);
        SwWW8Writer::WriteLong( *rWrt.pDataStrm, 2*sURL.Len());
        SwWW8Writer::WriteShort( *rWrt.pDataStrm, 3 );
        SwWW8Writer::WriteString16(*rWrt.pDataStrm, sURL, false);
    }
    else if (eProto != INET_PROT_NOT_VALID)
    {
        // version 2 (simple url)
        // an write some data to the data stream, but dont ask
        // what the data mean, except for the URL.
        // The First piece is the WW8_PIC structure.
        //
        static BYTE __READONLY_DATA MAGIC_B[] = {
            0xE0,0xC9,0xEA,0x79,0xF9,0xBA,0xCE,0x11,
            0x8C,0x82,0x00,0xAA,0x00,0x4B,0xA9,0x0B
        };

        rWrt.pDataStrm->Write( MAGIC_B, sizeof(MAGIC_B) );
        SwWW8Writer::WriteLong( *rWrt.pDataStrm, 2 * (sURL.Len()+1) );
        SwWW8Writer::WriteString16( *rWrt.pDataStrm, sURL, true);
    }

    if (sMark.Len())
    {
        SwWW8Writer::WriteLong( *rWrt.pDataStrm, sMark.Len()+1 );
        SwWW8Writer::WriteString16( *rWrt.pDataStrm, sMark, true);
    }
    SwWW8Writer::WriteLong( *rWrt.pDataStrm, nDataStt,
        rWrt.pDataStrm->Tell() - nDataStt );
}

void WW8_AttrIter::EndURL()
{
    rWrt.OutField(0, ww::eHYPERLINK, aEmptyStr, WRITEFIELD_CLOSE);
}

String BookmarkToWord(const String &rBookmark)
{
    String sRet(INetURLObject::encode(rBookmark,
        INetURLObject::PART_REL_SEGMENT_EXTRA, '%',
        INetURLObject::ENCODE_ALL, RTL_TEXTENCODING_ASCII_US));
    return TruncateBookmark(sRet);
}

String BookmarkToWriter(const String &rBookmark)
{
    return INetURLObject::decode(rBookmark, '%',
        INetURLObject::DECODE_UNAMBIGUOUS, RTL_TEXTENCODING_ASCII_US);
}

void WW8_SwAttrIter::OutSwFmtRefMark(const SwFmtRefMark& rAttr, bool)
{
    if( rWrt.HasRefToObject( REF_SETREFATTR, &rAttr.GetRefName(), 0 ))
        rWrt.AppendBookmark( rWrt.GetBookmarkName( REF_SETREFATTR,
                                            &rAttr.GetRefName(), 0 ));
}

void WW8_SwAttrIter::FieldVanish( const String& rTxt )
{
    WW8Bytes aItems;
    rWrt.GetCurrentItems(aItems);

    // sprmCFFldVanish
    if( rWrt.bWrtWW8 )
        SwWW8Writer::InsUInt16( aItems, 0x802 );
    else
        aItems.Insert( 67, aItems.Count() );
    aItems.Insert( 1, aItems.Count() );

    USHORT nStt_sprmCFSpec = aItems.Count();

    // sprmCFSpec --  fSpec-Attribut true
    if( rWrt.bWrtWW8 )
        SwWW8Writer::InsUInt16( aItems, 0x855 );
    else
        aItems.Insert( 117, aItems.Count() );
    aItems.Insert( 1, aItems.Count() );

    rWrt.WriteChar( '\x13' );
    rWrt.pChpPlc->AppendFkpEntry( rWrt.Strm().Tell(), aItems.Count(),
                                    aItems.GetData() );
    rWrt.OutSwString( rTxt, 0, rTxt.Len(), rWrt.IsUnicode(),
                        RTL_TEXTENCODING_MS_1252 );
    rWrt.pChpPlc->AppendFkpEntry( rWrt.Strm().Tell(), nStt_sprmCFSpec,
                                    aItems.GetData() );
    rWrt.WriteChar( '\x15' );
    rWrt.pChpPlc->AppendFkpEntry( rWrt.Strm().Tell(), aItems.Count(),
                                    aItems.GetData() );
}

void WW8_SwAttrIter::OutSwTOXMark(const SwTOXMark& rAttr, bool bStart)
{
    // its a field; so get the Text form the Node and build the field
    ASSERT( bStart, "calls only with the startposition!" );
    String sTxt;

    const SwTxtTOXMark& rTxtTOXMark = *rAttr.GetTxtTOXMark();
    const xub_StrLen* pTxtEnd = rTxtTOXMark.GetEnd();
    if( pTxtEnd )       // has range?
        sTxt = rNd.GetExpandTxt( *rTxtTOXMark.GetStart(),
                                 *pTxtEnd  - *rTxtTOXMark.GetStart() );
    else
        sTxt = rAttr.GetAlternativeText();

    switch( rAttr.GetTOXType()->GetType() )
    {
    case TOX_INDEX:
        if( rAttr.GetPrimaryKey().Len() )
        {
            sTxt.Insert( ':', 0 );
            sTxt.Insert( rAttr.GetPrimaryKey(), 0 );
        }
        sTxt.InsertAscii( " XE \"", 0 );
        sTxt.InsertAscii( "\" " );
        break;

    case TOX_USER:
        ( sTxt.APPEND_CONST_ASC( "\" \\f \"" ))
                += (sal_Char)( 'A' + rWrt.GetId( *rAttr.GetTOXType() ));
        // kein break;
    case TOX_CONTENT:
        {
            sTxt.InsertAscii( " INHALT \"", 0 );
            USHORT nLvl = rAttr.GetLevel();
            if (nLvl > WW8ListManager::nMaxLevel)
                nLvl = WW8ListManager::nMaxLevel;

            ((sTxt.APPEND_CONST_ASC( "\" \\l " ))
                += String::CreateFromInt32( nLvl )) += ' ';
        }
        break;
    }

    if( sTxt.Len() )
        FieldVanish( sTxt );
}

int WW8_SwAttrIter::OutAttrWithRange(xub_StrLen nPos)
{
    int nRet = 0;
    if (const SwpHints* pTxtAttrs = rNd.GetpSwpHints())
    {
        rWrt.push_charpropstart(nPos);
        const xub_StrLen* pEnd;
        for (USHORT i = 0; i < pTxtAttrs->Count(); ++i )
        {
            const SwTxtAttr* pHt = (*pTxtAttrs)[i];
            const SfxPoolItem* pItem = &pHt->GetAttr();
            switch( pItem->Which() )
            {
            case RES_TXTATR_INETFMT:
                if( rWrt.bWrtWW8 )  // nur WW8 kann Hyperlinks
                {
                    if( nPos == *pHt->GetStart() )
                    {
                        OutSwFmtINetFmt((SwFmtINetFmt&)*pItem, true);
                        ++nRet;
                    }
                    if( 0 != ( pEnd = pHt->GetEnd() ) && nPos == *pEnd )
                    {
                        OutSwFmtINetFmt((SwFmtINetFmt&)*pItem, false);
                        --nRet;
                    }
                }
                break;
            case RES_TXTATR_REFMARK:
                if( nPos == *pHt->GetStart() )
                {
                    OutSwFmtRefMark((SwFmtRefMark&)*pItem, true);
                    ++nRet;
                }
                if( 0 != ( pEnd = pHt->GetEnd() ) && nPos == *pEnd )
                {
                    OutSwFmtRefMark((SwFmtRefMark&)*pItem, false);
                    --nRet;
                }
                break;
            case RES_TXTATR_TOXMARK:
                if (nPos == *pHt->GetStart())
                    OutSwTOXMark((SwTOXMark&)*pItem, true);
                break;
            case RES_TXTATR_CJK_RUBY:
                if( nPos == *pHt->GetStart() )
                {
                    OutSwFmtRuby((SwFmtRuby&)*pItem, true);
                    ++nRet;
                }
                if( 0 != ( pEnd = pHt->GetEnd() ) && nPos == *pEnd )
                {
                    OutSwFmtRuby((SwFmtRuby&)*pItem, false);
                    --nRet;
                }
                break;
            }
        }
        rWrt.pop_charpropstart();//HasTextItem nur in dem obigen Bereich erlaubt
    }
    return nRet;
}

bool WW8_SwAttrIter::IsRedlineAtEnd( xub_StrLen nEnd ) const
{
    bool bRet = false;
    // search next Redline
    for( USHORT nPos = nCurRedlinePos;
        nPos < rWrt.pDoc->GetRedlineTbl().Count(); ++nPos )
    {
        const SwPosition* pEnd = rWrt.pDoc->GetRedlineTbl()[ nPos ]->End();
        if( pEnd->nNode == rNd )
        {
            if( pEnd->nContent.GetIndex() == nEnd )
            {
                bRet = true;
                break;
            }
        }
        else
            break;
    }
    return bRet;
}

void WW8_SwAttrIter::OutRedlines( xub_StrLen nPos )
{
    if( pCurRedline )
    {
        const SwPosition* pEnd = pCurRedline->End();
        if( pEnd->nNode == rNd &&
            pEnd->nContent.GetIndex() <= nPos )
        {
            pCurRedline = 0;
            ++nCurRedlinePos;
        }
        else
            // write data of current redline
            rWrt.OutRedline( pCurRedline->GetRedlineData() );
    }

    if( !pCurRedline )
    {
        // search next Redline
        for( ; nCurRedlinePos < rWrt.pDoc->GetRedlineTbl().Count();
                ++nCurRedlinePos )
        {
            const SwRedline* pRedl = rWrt.pDoc->GetRedlineTbl()[ nCurRedlinePos ];

            const SwPosition* pStt = pRedl->Start();
            const SwPosition* pEnd = pStt == pRedl->GetPoint()
                                        ? pRedl->GetMark()
                                        : pRedl->GetPoint();

            if( pStt->nNode == rNd )
            {
                if( pStt->nContent.GetIndex() >= nPos )
                {
                    if( pStt->nContent.GetIndex() == nPos )
                    {
                        // write data of this redline
                        pCurRedline = pRedl;
                        rWrt.OutRedline( pCurRedline->GetRedlineData() );
                    }
                    break;
                }
            }
            else
                break;

            if( pEnd->nNode == rNd &&
                pEnd->nContent.GetIndex() < nPos )
            {
                pCurRedline = pRedl;
                break;
            }
        }
    }
}

/*  */

short SwWW8Writer::GetCurrentPageDirection() const
{
    const SwFrmFmt &rFmt = pAktPageDesc
                    ? pAktPageDesc->GetMaster()
                    : pDoc->GetPageDesc(0).GetMaster();
    return rFmt.GetFrmDir().GetValue();
}

short SwWW8Writer::TrueFrameDirection(const SwFrmFmt &rFlyFmt) const
{
    const SwFrmFmt *pFlyFmt = &rFlyFmt;
    const SvxFrameDirectionItem* pItem = 0;
    while (pFlyFmt)
    {
        pItem = &pFlyFmt->GetFrmDir();
        if (FRMDIR_ENVIRONMENT == pItem->GetValue())
        {
            pItem = 0;
            const SwFmtAnchor* pAnchor = &pFlyFmt->GetAnchor();
            if (FLY_PAGE != pAnchor->GetAnchorId() &&
                pAnchor->GetCntntAnchor())
            {
                pFlyFmt =
                    pAnchor->GetCntntAnchor()->nNode.GetNode().GetFlyFmt();
            }
            else
                pFlyFmt = 0;
        }
        else
            pFlyFmt = 0;
    }

    short nRet;
    if (pItem)
        nRet = pItem->GetValue();
    else
        nRet = GetCurrentPageDirection();

    ASSERT(nRet != FRMDIR_ENVIRONMENT, "leaving with environment direction");
    return nRet;
}

const SvxBrushItem* SwWW8Writer::GetCurrentPageBgBrush() const
{
    const SwFrmFmt  &rFmt = pAktPageDesc
                    ? pAktPageDesc->GetMaster()
                    : pDoc->GetPageDesc(0).GetMaster();

    const SfxPoolItem* pItem = 0;
    //If not set, or "no fill", get real bg
    SfxItemState eState = rFmt.GetItemState(RES_BACKGROUND, true, &pItem);

    const SvxBrushItem* pRet = (const SvxBrushItem*)pItem;
    if (SFX_ITEM_SET != eState || (!pRet->GetGraphic() &&
        pRet->GetColor() == COL_TRANSPARENT))
    {
        pRet = &(DefaultItemGet<SvxBrushItem>(*pDoc,RES_BACKGROUND));
    }
    return pRet;
}

SvxBrushItem SwWW8Writer::TrueFrameBgBrush(const SwFrmFmt &rFlyFmt) const
{
    const SwFrmFmt *pFlyFmt = &rFlyFmt;
    const SvxBrushItem* pRet = 0;

    while (pFlyFmt)
    {
        //If not set, or "no fill", get real bg
        const SfxPoolItem* pItem = 0;
        SfxItemState eState =
            pFlyFmt->GetItemState(RES_BACKGROUND, true, &pItem);
        pRet = (const SvxBrushItem*)pItem;
        if (SFX_ITEM_SET != eState || (!pRet->GetGraphic() &&
            pRet->GetColor() == COL_TRANSPARENT))
        {
            pRet = 0;
            const SwFmtAnchor* pAnchor = &pFlyFmt->GetAnchor();
            if (FLY_PAGE != pAnchor->GetAnchorId() &&
                pAnchor->GetCntntAnchor())
            {
                pFlyFmt =
                    pAnchor->GetCntntAnchor()->nNode.GetNode().GetFlyFmt();
            }
            else
                pFlyFmt = 0;
        }
        else
            pFlyFmt = 0;
    }

    if (!pRet)
        pRet = GetCurrentPageBgBrush();

    SvxBrushItem aRet = Color(COL_WHITE);
    if (pRet && (pRet->GetGraphic() ||( pRet->GetColor() != COL_TRANSPARENT)))
        aRet = *pRet;

    return aRet;
}


/*
Convert characters that need to be converted, the basic replacements and the
ridicously complicated title case attribute mapping to hardcoded upper case
because word doesn't have the feature
*/
String WW8_SwAttrIter::GetSnippet(const String &rStr, xub_StrLen nAktPos,
    xub_StrLen nLen) const
{
    String aSnippet(rStr, nAktPos, nLen);
    if (!nLen)
        return aSnippet;

    // 0x0a     ( Hard Line Break ) -> 0x0b
    // 0xad     ( soft hyphen )     -> 0x1f
    // 0x2011   ( hard hyphen )     -> 0x1e
    aSnippet.SearchAndReplaceAll(0x0A, 0x0B);
    aSnippet.SearchAndReplaceAll(CHAR_HARDHYPHEN, 0x1e);
    aSnippet.SearchAndReplaceAll(CHAR_SOFTHYPHEN, 0x1f);

    rWrt.push_charpropstart(nAktPos);
    const SfxPoolItem &rItem = GetItem(RES_CHRATR_CASEMAP);

    if (SVX_CASEMAP_TITEL == ((const SvxCaseMapItem&)rItem).GetValue())
    {
        sal_uInt16 nScriptType = ScriptType::LATIN;
        if (pBreakIt->xBreak.is())
            nScriptType = pBreakIt->xBreak->getScriptType(aSnippet, 0);

        LanguageType nLanguage;
        switch (nScriptType)
        {
            case ScriptType::ASIAN:
                nLanguage = ((const SvxLanguageItem&)GetItem(RES_CHRATR_CJK_LANGUAGE)).GetLanguage();
                break;
            case ScriptType::COMPLEX:
                nLanguage = ((const SvxLanguageItem&)GetItem(RES_CHRATR_CTL_LANGUAGE)).GetLanguage();
                break;
            case ScriptType::LATIN:
            default:
                nLanguage = ((const SvxLanguageItem&)GetItem(RES_CHRATR_LANGUAGE)).GetLanguage();
                break;
        }

        SvxFont aFontHelper;
        aFontHelper.SetCaseMap(SVX_CASEMAP_TITEL);
        aFontHelper.SetLanguage(nLanguage);
        aSnippet = aFontHelper.CalcCaseMap(aSnippet);

        //If we weren't at the begin of a word undo the case change.
        //not done before doing the casemap because the sequence might start
        //with whitespace
        if (pBreakIt->xBreak.is() && !pBreakIt->xBreak->isBeginWord(
            rStr, nAktPos, pBreakIt->GetLocale(nLanguage),
            com::sun::star::i18n::WordType::ANYWORD_IGNOREWHITESPACES ) )
        {
            aSnippet.SetChar(0, rStr.GetChar(nAktPos));
        }
    }
    rWrt.pop_charpropstart();

    return aSnippet;
}

Writer& OutWW8_SwTxtNode( Writer& rWrt, SwCntntNode& rNode )
{
    SwWW8Writer& rWW8Wrt = (SwWW8Writer&)rWrt;
    SwTxtNode* pNd = &((SwTxtNode&)rNode);

    bool bFlyInTable = rWW8Wrt.pFlyFmt && rWW8Wrt.bIsInTable;

    // akt. Style
    if( !bFlyInTable )
    {
        rWW8Wrt.nStyleBeforeFly
            = rWW8Wrt.GetId( (SwTxtFmtColl&)pNd->GetAnyFmtColl() );
    }

    SVBT16 nSty;
    ShortToSVBT16( rWW8Wrt.nStyleBeforeFly, nSty );

    WW8Bytes* pO = rWW8Wrt.pO;
    WW8_SwAttrIter aAttrIter( rWW8Wrt, *pNd );
    rtl_TextEncoding eChrSet = aAttrIter.GetCharSet();

    if( rWW8Wrt.bStartTOX )
    {
        // ignore TOX header section
        const SwSectionNode* pSectNd = rNode.FindSectionNode();
        if( TOX_CONTENT_SECTION == pSectNd->GetSection().GetType() )
        {
            rWW8Wrt.StartTOX( pSectNd->GetSection() );
            rWW8Wrt.push_charpropstart(0);
        }
    }

    const SwSection* pTOXSect = 0;
    if( rWW8Wrt.bInWriteTOX )
    {
        // check for end of TOX
        SwNodeIndex aIdx( *pNd, 1 );
        if( !aIdx.GetNode().IsTxtNode() )
        {
            const SwSectionNode* pTOXSectNd = rNode.FindSectionNode();
            pTOXSect = &pTOXSectNd->GetSection();

            const SwNode* pNxt = rNode.GetNodes().GoNext( &aIdx );
            if( pNxt && pNxt->FindSectionNode() == pTOXSectNd )
                pTOXSect = 0;
        }
    }

    ASSERT( !pO->Count(), " pO ist am Zeilenanfang nicht leer" );

    String aStr( pNd->GetTxt() );

    xub_StrLen nAktPos = 0;
    xub_StrLen nEnd = aStr.Len();
    bool bUnicode = rWW8Wrt.bWrtWW8, bRedlineAtEnd = false;
    int nOpenAttrWithRange = 0;

    do {
        xub_StrLen nNextAttr = aAttrIter.WhereNext();

        if( nNextAttr > nEnd )
            nNextAttr = nEnd;

        aAttrIter.OutFlys(nAktPos);
        //Append bookmarks in this range after flys, exclusive of final
        //position of this range
        rWW8Wrt.AppendBookmarks( *pNd, nAktPos, nNextAttr - nAktPos );
        bool bTxtAtr = aAttrIter.IsTxtAttr( nAktPos );
        nOpenAttrWithRange += aAttrIter.OutAttrWithRange(nAktPos);

        xub_StrLen nLen = nNextAttr - nAktPos;
        if (!bTxtAtr && nLen)
        {
            String aSnippet(aAttrIter.GetSnippet(aStr, nAktPos, nLen));
            rWW8Wrt.OutSwString(aSnippet, 0, nLen, bUnicode, eChrSet );
        }

        if (aAttrIter.IsDropCap(nNextAttr))
        {
            const SwFmtDrop *pSwFmtDrop = aAttrIter.GetSwFmtDrop();
            short nDropLines = pSwFmtDrop->GetLines();
            short nDistance = pSwFmtDrop->GetDistance();

            pO->Insert( (BYTE*)&nSty, 2, pO->Count() );     // Style #

            rWW8Wrt.InsUInt16( 0x261b );            // Alignment (sprmPPc)
            rWW8Wrt.pO->Insert( 0x20, rWW8Wrt.pO->Count() );

            rWW8Wrt.InsUInt16( 0x2423 );            // Wrapping (sprmPWr)
            rWW8Wrt.pO->Insert( 0x02, rWW8Wrt.pO->Count() );

            rWW8Wrt.InsUInt16( 0x442c );            // Dropcap (sprmPDcs)
            int nDCS = (nDropLines << 3) | 0x01;
            rWW8Wrt.InsUInt16( nDCS );

            rWW8Wrt.InsUInt16( 0x842F );            // Distance from text (sprmPDxaFromText)
            rWW8Wrt.InsUInt16( nDistance );

            int rFontHeight, rDropHeight, rDropDescent;

            if(pNd->GetDropSize(rFontHeight, rDropHeight, rDropDescent))
            {
                rWW8Wrt.InsUInt16( 0x6412 );            // Line spacing
                rWW8Wrt.InsUInt16( -rDropHeight );
                rWW8Wrt.InsUInt16( 0 );
            }

            rWW8Wrt.WriteCR();

            rWW8Wrt.pPapPlc->AppendFkpEntry( rWrt.Strm().Tell(), pO->Count(),
                pO->GetData() );
            pO->Remove( 0, pO->Count() );

            rWW8Wrt.InsUInt16( 0x4845 );            // Lower the chars
            rWW8Wrt.InsUInt16(-((nDropLines - 1)*rDropDescent) / 10 );

            rWW8Wrt.InsUInt16( 0x4a43 );            // Font Size
            rWW8Wrt.InsUInt16( rFontHeight / 10 );

            rWW8Wrt.pChpPlc->AppendFkpEntry( rWrt.Strm().Tell(),
                                            pO->Count(), pO->GetData() );
            pO->Remove( 0, pO->Count() );

        }

        // Am Zeilenende werden die Attribute bis ueber das CR aufgezogen.
        // Ausnahme: Fussnoten am Zeilenende
        if (nNextAttr == nEnd)
        {
            ASSERT(nOpenAttrWithRange >= 0,
                "odd to see this happening, expected >= 0");
            if (!bTxtAtr && nOpenAttrWithRange <= 0)
            {
                if (aAttrIter.IsRedlineAtEnd(nEnd))
                    bRedlineAtEnd = true;
                else
                {
                    //insert final graphic anchors if any before CR
                    aAttrIter.OutFlys(nEnd);
                    //insert final bookmarks if any before CR and after flys
                    rWW8Wrt.AppendBookmarks( *pNd, nEnd, 1 );
                    if (pTOXSect)
                    {
                        rWW8Wrt.pop_charpropstart();
                        rWW8Wrt.EndTOX(*pTOXSect);
                    }
                    rWW8Wrt.WriteCR();              // CR danach
                }
            }
        }
                                        // Ausgabe der Zeichenattribute
        aAttrIter.OutAttr( nAktPos );   // nAktPos - 1 ??
        rWW8Wrt.pChpPlc->AppendFkpEntry( rWrt.Strm().Tell(),
                                            pO->Count(), pO->GetData() );
        pO->Remove( 0, pO->Count() );                   // leeren

                    // Ausnahme: Fussnoten am Zeilenende
        if (nNextAttr == nEnd)
        {
            ASSERT(nOpenAttrWithRange >= 0,
                "odd to see this happening, expected >= 0");
            bool bAttrWithRange = (nOpenAttrWithRange > 0);
            if (nAktPos != nEnd)
            {
                nOpenAttrWithRange += aAttrIter.OutAttrWithRange(nEnd);
                ASSERT(nOpenAttrWithRange == 0,
                    "odd to see this happening, expected 0");
            }

            if(pO->Count())
            {
                rWW8Wrt.pChpPlc->AppendFkpEntry( rWrt.Strm().Tell(),
                                            pO->Count(), pO->GetData() );
                pO->Remove(0, pO->Count());                   // leeren
            }

            if( bTxtAtr || bAttrWithRange || bRedlineAtEnd )
            {
                //insert final graphic anchors if any before CR
                aAttrIter.OutFlys(nEnd);
                //insert final bookmarks if any before CR and after flys
                rWW8Wrt.AppendBookmarks( *pNd, nEnd, 1 );

                if (pTOXSect)
                {
                    rWW8Wrt.pop_charpropstart();
                    rWW8Wrt.EndTOX( *pTOXSect );
                }

                rWW8Wrt.WriteCR();              // CR danach

                if( bRedlineAtEnd )
                {
                    aAttrIter.OutRedlines( nEnd );
                    if( pO->Count() )
                    {
                        rWW8Wrt.pChpPlc->AppendFkpEntry( rWrt.Strm().Tell(),
                            pO->Count(), pO->GetData() );
                        pO->Remove( 0, pO->Count() );   // delete
                    }
                }
            }
        }
        nAktPos = nNextAttr;
        aAttrIter.NextPos();
        eChrSet = aAttrIter.GetCharSet();
    }
    while( nAktPos < nEnd );

    ASSERT( !pO->Count(), " pO ist am ZeilenEnde nicht leer" );

    pO->Insert( (BYTE*)&nSty, 2, pO->Count() );     // Style #

    if( rWW8Wrt.pFlyFmt && !rWW8Wrt.bIsInTable )    // Fly-Attrs
        rWW8Wrt.Out_SwFmt(*rWW8Wrt.pFlyFmt, false, false, true);

    if( rWW8Wrt.bOutTable )
    {                                               // Tab-Attr
        // sprmPFInTable
        if( rWW8Wrt.bWrtWW8 )
            SwWW8Writer::InsUInt16( *pO, 0x2416 );
        else
            pO->Insert( 24, pO->Count() );
        pO->Insert( 1, pO->Count() );
    }

    if( !bFlyInTable )
    {
        SfxItemSet* pTmpSet = 0;
        const BYTE nPrvNxtNd =
            ( WWFL_ULSPACE_LIKE_SWG & rWW8Wrt.GetIniFlags())
                    ? pNd->HasPrevNextLayNode()
                    : (ND_HAS_PREV_LAYNODE|ND_HAS_NEXT_LAYNODE);

        if( (ND_HAS_PREV_LAYNODE|ND_HAS_NEXT_LAYNODE ) != nPrvNxtNd )
        {
            const SfxPoolItem* pItem;
            if( SFX_ITEM_SET == pNd->GetSwAttrSet().GetItemState(
                    RES_UL_SPACE, true, &pItem ) &&
                ( ( !( ND_HAS_PREV_LAYNODE & nPrvNxtNd ) &&
                   ((SvxULSpaceItem*)pItem)->GetUpper()) ||
                  ( !( ND_HAS_NEXT_LAYNODE & nPrvNxtNd ) &&
                   ((SvxULSpaceItem*)pItem)->GetLower()) ))
            {
                pTmpSet = new SfxItemSet( pNd->GetSwAttrSet() );
                SvxULSpaceItem aUL( *(SvxULSpaceItem*)pItem );
                if( !(ND_HAS_PREV_LAYNODE & nPrvNxtNd ))
                    aUL.SetUpper( 0 );
                if( !(ND_HAS_NEXT_LAYNODE & nPrvNxtNd ))
                    aUL.SetLower( 0 );
                pTmpSet->Put( aUL );
            }
        }

        if( !pTmpSet )
            pTmpSet = pNd->GetpSwAttrSet();

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

            if( pTmpSet == pNd->GetpSwAttrSet() )
                pTmpSet = new SfxItemSet( pNd->GetSwAttrSet() );

            SvxLRSpaceItem aLR(ItemGet<SvxLRSpaceItem>(*pTmpSet, RES_LR_SPACE));
            aLR.SetTxtLeft( aLR.GetTxtLeft() + pFmt->GetAbsLSpace() );

            if( MAXLEVEL > pNum->GetLevel() )
            {
                aLR.SetTxtFirstLineOfst(GetWordFirstLineOffset(*pFmt));
                if (pNum == pNd->GetNum() && SFX_ITEM_SET !=
                   pTmpSet->GetItemState(RES_PARATR_NUMRULE, false) )
                {
                    //If the numbering is not outline, and theres no numrule
                    //name in the itemset, put one in there

                    // NumRule from a template - then put it into the itemset
                    pTmpSet->Put( SwNumRuleItem( pRule->GetName() ));
                }
                else if (pNum != pNd->GetNum())
                {
                    //#111936#
                    //If the numbering in use is outline numbering, then
                    //any numbering in the itemset is not being used, so
                    //remove it
                    pTmpSet->ClearItem(RES_PARATR_NUMRULE);
                }
            }
            else
                pTmpSet->ClearItem(RES_PARATR_NUMRULE);

            pTmpSet->Put( aLR );
            SwWW8Writer::CorrTabStopInSet( *pTmpSet, pFmt->GetAbsLSpace() );
        }

        /*
        If a given para is using the FRMDIR_ENVIRONMENT direction we
        cannot export that, its its ltr then that's ok as thats word's
        default. Otherwise we must add a RTL attribute to our export list
        */
        const SvxFrameDirectionItem* pItem = (const SvxFrameDirectionItem*)
            pNd->GetSwAttrSet().GetItem(RES_FRAMEDIR);
        if (
            (!pItem || pItem->GetValue() == FRMDIR_ENVIRONMENT) &&
            aAttrIter.IsParaRTL()
           )
        {
            if (pTmpSet == pNd->GetpSwAttrSet())
                pTmpSet = new SfxItemSet(pNd->GetSwAttrSet());

            pTmpSet->Put(SvxFrameDirectionItem(FRMDIR_HORI_RIGHT_TOP));
        }

        if( pTmpSet )
        {                                               // Para-Attrs
            rWW8Wrt.pStyAttr = &pNd->GetAnyFmtColl().GetAttrSet();

            const SwModify* pOldMod = rWW8Wrt.pOutFmtNode;
            rWW8Wrt.pOutFmtNode = pNd;

            // Pap-Attrs, so script is not necessary
            rWW8Wrt.Out_SfxItemSet( *pTmpSet, true, false,
                com::sun::star::i18n::ScriptType::LATIN);

            rWW8Wrt.pStyAttr = 0;
            rWW8Wrt.pOutFmtNode = pOldMod;

            if( pTmpSet != pNd->GetpSwAttrSet() )
                delete pTmpSet;
        }
    }

    rWW8Wrt.pPapPlc->AppendFkpEntry( rWrt.Strm().Tell(), pO->Count(),
        pO->GetData() );
    pO->Remove( 0, pO->Count() );                       // leeren
    return rWrt;
}


/*  */

USHORT SwWW8Writer::StartTableFromFrmFmt(WW8Bytes &rAt, const SwFrmFmt *pFmt,
    SwTwips &rTblOffset)

{
    //Tell the undocumented table hack that everything between here and
    //the last table position is nontable text
    if (WW8_CP nPos = Fc2Cp(Strm().Tell()))
        pMagicTable->Append(nPos,0);

    // sprmPDxaFromText10
    if( bWrtWW8 )
    {
        static BYTE __READONLY_DATA  aTabLineAttr[] = {
                0, 0,               // Sty # 0
                0x16, 0x24, 1,      // sprmPFInTable
                0x17, 0x24, 1 };    // sprmPFTtp
        rAt.Insert( aTabLineAttr, sizeof( aTabLineAttr ), rAt.Count() );
    }
    else
    {
        static BYTE __READONLY_DATA  aTabLineAttr[] = {
                0, 0,               // Sty # 0
                24, 1,              // sprmPFInTable
                25, 1 };            // sprmPFTtp
        rAt.Insert( aTabLineAttr, sizeof( aTabLineAttr ), rAt.Count() );
    }

    ASSERT(pFmt, "No pFmt!");
    if (pFmt)
    {
        const SwFmtHoriOrient &rHori = pFmt->GetHoriOrient();
        const SwFmtVertOrient &rVert = pFmt->GetVertOrient();
        if (
            (PRTAREA == rHori.GetRelationOrient() ||
             FRAME == rHori.GetRelationOrient())
            &&
            (PRTAREA == rVert.GetRelationOrient() ||
             FRAME == rVert.GetRelationOrient())
           )
        {
            SwHoriOrient eHOri = rHori.GetHoriOrient();
            switch (eHOri)
            {
                case HORI_CENTER:
                case HORI_RIGHT:
                    if( bWrtWW8 )
                        InsUInt16( rAt, 0x5400 );
                    else
                        rAt.Insert( 182, rAt.Count() );
                    InsUInt16( rAt, (HORI_RIGHT == eHOri ? 2 : 1 ));
                    break;
                default:
#if 1
#if 1
                    rTblOffset = rHori.GetPos();
                    const SvxLRSpaceItem& rLRSp = pFmt->GetLRSpace();
                    rTblOffset += rLRSp.GetLeft();
#else
                    Point aOffset = pFmt->FindLayoutRect(true).Pos();
                    rTblOffset = aOffset.X();
#endif

#else
                    {
                        const SvxLRSpaceItem& rLRSp = pFmt->GetLRSpace();
                        rTblOffset = rLRSp.GetLeft();
                        rPageSize -= rTblOffset + rLRSp.GetRight();
                    }
#endif
                    break;
            }
        }
    }
    return rAt.Count();
}


//---------------------------------------------------------------------------
//       Tabellen
//---------------------------------------------------------------------------

Writer& OutWW8_SwTblNode( Writer& rWrt, SwTableNode & rNode )
{
    SwWW8Writer & rWW8Wrt = (SwWW8Writer&)rWrt;
    SwTable& rTbl = rNode.GetTable();
    const SwFrmFmt *pFmt = rTbl.GetFrmFmt();
    ASSERT(pFmt,"Impossible");
    if (!pFmt)
        return rWrt;
    rWW8Wrt.Out_SfxBreakItems(pFmt->GetAttrSet(), rNode);

    /*
    ALWAYS relative when HORI_NONE (nPageSize + ( nPageSize / 10 )) < nTblSz,
    in that case the cell width's and table width's are not real. The table
    width is maxed and cells relative, so we need the frame (generally page)
    width that the table is in to work out the true widths.
    */
    SwFmtHoriOrient aHori(pFmt->GetHoriOrient());
    bool bRelBoxSize = (pFmt->GetHoriOrient().GetHoriOrient() == HORI_FULL);
    unsigned long nTblSz = static_cast<unsigned long>(pFmt->GetFrmSize().GetWidth());
    if (nTblSz == USHRT_MAX && !bRelBoxSize)
    {
        ASSERT(bRelBoxSize, "huge table width but not relative");
        bRelBoxSize = true;
    }

    unsigned long nPageSize = nTblSz;
    {
        Point aPt;
        SwRect aRect(pFmt->FindLayoutRect(false, &aPt));
        if (aRect.IsEmpty())
        {
            // dann besorge mal die Seitenbreite ohne Raender !!
            const SwFrmFmt* pParentFmt = rWW8Wrt.pFlyFmt ? rWW8Wrt.pFlyFmt :
                rWrt.pDoc->GetPageDesc(0).GetPageFmtOfNode(rNode, false);
            aRect = pParentFmt->FindLayoutRect(true);
            if (!(nPageSize = aRect.Width()))
            {
                const SvxLRSpaceItem& rLR = pParentFmt->GetLRSpace();
                nPageSize = pParentFmt->GetFrmSize().GetWidth() - rLR.GetLeft() -
                    rLR.GetRight();
            }
        }
        else
            nPageSize = aRect.Width();

        const SvxLRSpaceItem &rLR = pFmt->GetLRSpace();

        nPageSize -= (rLR.GetLeft() + rLR.GetRight());
    }

    WW8Bytes aAt( 128, 128 );   // Attribute fuer's Tabellen-Zeilenende
    SwTwips nTblOffset = 0;
    USHORT nStdAtLen = rWW8Wrt.StartTableFromFrmFmt(aAt, pFmt, nTblOffset);
    static const BYTE aNullBytes[] = { 0, 0, 0, 0 };

    SwWriteTable* pTableWrt;
    const SwHTMLTableLayout *pLayout = rTbl.GetHTMLTableLayout();
    if( pLayout && pLayout->IsExportable() )
        pTableWrt = new SwWriteTable( pLayout );
    else
    {
        pTableWrt = new SwWriteTable( rTbl.GetTabLines(), nPageSize,
            (USHORT)nTblSz, false);
    }

    const SwFrmFmt *pDefaultFmt = 0;
    // WW6 / 8 can not have more then 31 / 64 cells
    const BYTE nMaxCols = rWW8Wrt.bWrtWW8 ? 64 : 31;
    // rCols are the array of all cols of the table
    const SwWriteTableCols& rCols = pTableWrt->GetCols();
    USHORT nColCnt = rCols.Count();
    SwWriteTableCellPtr* pBoxArr = new SwWriteTableCellPtr[ nColCnt ];
    USHORT* pRowSpans = new USHORT[ nColCnt ];
    memset( pBoxArr, 0, sizeof( pBoxArr[0] ) * nColCnt );
    memset( pRowSpans, 0, sizeof( pRowSpans[0] ) * nColCnt );
    const SwWriteTableRows& rRows = pTableWrt->GetRows();
    for( USHORT nLine = 0; nLine < rRows.Count(); ++nLine )
    {
        USHORT nBox, nRealBox;

        const SwWriteTableRow *pRow = rRows[ nLine ];
        const SwWriteTableCells& rCells = pRow->GetCells();

        bool bFixRowHeight = false;
        USHORT nRealColCnt = 0;
        USHORT nTotal = rCells.Count();
        ASSERT(nTotal <= rCols.Count(),
            "oh oh!,row has more cells than table is wide!");
        if (nTotal > rCols.Count())
            nTotal = rCols.Count();
        for( nColCnt = 0, nBox = 0; nBox < nTotal; ++nColCnt )
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
                ++nRealColCnt;
            }
            else if( !nColCnt || pBoxArr[ nColCnt-1 ] != pBoxArr[ nColCnt ] )
                ++nRealColCnt;

            if( 1 != pRowSpans[ nColCnt ] )
                bFixRowHeight = true;
        }

        for( ; nColCnt < rCols.Count() && pRowSpans[ nColCnt ]; ++nColCnt )
        {
            if( !nColCnt || pBoxArr[ nColCnt-1 ] != pBoxArr[ nColCnt ] )
                ++nRealColCnt;
            bFixRowHeight = true;
        }

        //Winword column export limited to 31/64 cells
        sal_uInt8 nWWColMax = nRealColCnt > nMaxCols ?
            nMaxCols : msword_cast<sal_uInt8>(nRealColCnt);

        // 1.Zeile eine Headline?  sprmTTableHeader
        if( !nLine && rTbl.IsHeadlineRepeat() )
        {
            if( rWW8Wrt.bWrtWW8 )
                SwWW8Writer::InsUInt16( aAt, 0x3404 );
            else
                aAt.Insert( 186, aAt.Count() );
            aAt.Insert( 1, aAt.Count() );
        }

        // Zeilenhoehe ausgeben   sprmTDyaRowHeight
        long nHeight = 0;
        if( bFixRowHeight )
        {
            nHeight = -pRow->GetPos();      //neg. => abs. height!
            if( nLine )
                nHeight += rRows[ nLine - 1 ]->GetPos();
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
            if( rWW8Wrt.bWrtWW8 )
                SwWW8Writer::InsUInt16( aAt, 0x9407 );
            else
                aAt.Insert( 189, aAt.Count() );
            SwWW8Writer::InsUInt16( aAt, (USHORT)nHeight );
        }

        /*
        #i4569#
        Writer always behaves as if it has 0x3403 set as true sprmTFCantSplit
        - our tables may be never split in the line
        */
        if( rWW8Wrt.bWrtWW8 )
            SwWW8Writer::InsUInt16( aAt, 0x3403 );
        else
            aAt.Insert( 185, aAt.Count() );
        aAt.Insert( (BYTE)1, aAt.Count() );


        if (rWW8Wrt.bWrtWW8)
        {
            if (rWW8Wrt.TrueFrameDirection(*pFmt) == FRMDIR_HORI_RIGHT_TOP)
            {
                SwWW8Writer::InsUInt16(aAt, 0x560B);
                SwWW8Writer::InsUInt16(aAt, 1);
            }
        }

        // Inhalt der Boxen ausgeben
        for( nBox = 0, nRealBox = 0; nBox < nColCnt; ++nBox )
        {
            if( nBox && pBoxArr[ nBox-1 ] == pBoxArr[ nBox ] )
                continue;

            if( pBoxArr[ nBox ]->GetRowSpan() == pRowSpans[ nBox ] )
            {
                // new Box
                const SwStartNode* pSttNd = pBoxArr[nBox]->GetBox()->GetSttNd();
                WW8SaveData aSaveData( rWW8Wrt, pSttNd->GetIndex()+1,
                                        pSttNd->EndOfSectionIndex() );
                rWW8Wrt.bOutTable = true;
                rWW8Wrt.bIsInTable= true;
                rWW8Wrt.WriteText();
            }
            else
            {
                rWW8Wrt.bOutTable = true;
                rWW8Wrt.WriteStringAsPara( aEmptyStr );
                rWW8Wrt.bOutTable = false;
            }

            if( nWWColMax < nRealColCnt )
            {
                if( nRealBox+1 < nWWColMax || nRealBox+1 == nRealColCnt )
                    rWW8Wrt.WriteCellEnd(); // SpaltenEnde
            }
            else if( nRealBox < nWWColMax )
                    rWW8Wrt.WriteCellEnd(); // SpaltenEnde
            ++nRealBox;
        }

        // das wars mit der Line
        rWW8Wrt.WriteRowEnd();  // TabellenZeilen-Ende

        if( rWW8Wrt.bWrtWW8 )
        {
            // SprmTDefTable
            // 0+1 - OpCode, 2+3 - Laenge = Cells * (sizeof TC + uint16)
            // 4 - Cells (max 32!)
            SwWW8Writer::InsUInt16( aAt, 0xD608 );
            SwWW8Writer::InsUInt16( aAt, 2 + ( nWWColMax + 1 ) * 2 +
                                                ( nWWColMax * 20 ));
            aAt.Insert( nWWColMax, aAt.Count() );               //
        }
        else
        {
            // SprmTDefTable
            // 0 - OpCode, 1,2 - Laenge = Cells * (sizeof TC + uint16)
            // 3 - Cells (max 32!)
            aAt.Insert( 190, aAt.Count() );
            SwWW8Writer::InsUInt16( aAt, nWWColMax * 12 + 4 );  // Sprm-Laenge
            aAt.Insert( nWWColMax, aAt.Count() );               //
        }

        SwTwips nSz = 0, nCalc;
        SwWW8Writer::InsUInt16( aAt, (USHORT)nTblOffset );

        for( nBox = 0, nRealBox = 0; nRealBox < nWWColMax; ++nBox )
        {
            if( nBox && pBoxArr[ nBox-1 ] == pBoxArr[ nBox ] )
                continue;

            const SwFrmFmt* pBoxFmt = pBoxArr[ nBox ]->GetBox()->GetFrmFmt();
            const SwFmtFrmSize& rLSz = pBoxFmt->GetFrmSize();
            nSz += rLSz.GetWidth();
            nCalc = nSz;
            if( bRelBoxSize )
            {
                nCalc *= nPageSize;
                nCalc /= nTblSz;
            }
            SwWW8Writer::InsUInt16( aAt, (USHORT)(nTblOffset + nCalc ));
            ++nRealBox;
        }

        for( nBox = 0, nRealBox = 0; nRealBox < nWWColMax; nBox++ )
        {
            if( nBox && pBoxArr[ nBox-1 ] == pBoxArr[ nBox ] )
                continue;
            // rgf, erstmal alles 0 - WW8: 4, sonst 2 Byte
            // write vertikal alignment
            const SwFrmFmt& rFmt = *pBoxArr[ nBox ]->GetBox()->GetFrmFmt();
            if( rWW8Wrt.bWrtWW8 )
            {
                USHORT nFlags = pBoxArr[ nBox ]->GetRowSpan();
                if( 1 < nFlags )
                {
                    if( nFlags == pRowSpans[ nBox ] )
                        nFlags = 0x60;      // start a new vert. merge
                    else
                        nFlags = 0x20;      // continue a vert. merge
                }
                else
                    nFlags = 0;             // no vert. merge

                switch( rFmt.GetVertOrient().GetVertOrient() )
                {
                    case VERT_CENTER:
                        nFlags |= 0x080;
                        break;
                    case VERT_BOTTOM:
                        nFlags |= 0x100;
                        break;
                }
                SwWW8Writer::InsUInt16( aAt, nFlags );
            }
            ++nRealBox;
            aAt.Insert( aNullBytes, 2, aAt.Count() );   // dummy
            rWW8Wrt.Out_SwFmtTableBox( aAt, rFmt.GetBox() ); // 8/16 Byte
        }

        //Cell widths and heights
        if (rWW8Wrt.bWrtWW8)
        {
            if (!pDefaultFmt && nWWColMax)
                pDefaultFmt = pBoxArr[ 0 ]->GetBox()->GetFrmFmt();

            static USHORT __READONLY_DATA aBorders[] =
            {
                BOX_LINE_TOP, BOX_LINE_LEFT,
                BOX_LINE_BOTTOM, BOX_LINE_RIGHT
            };
            const USHORT* pBrd = aBorders;

            //Export non default border spacing
            for( nBox = 0, nRealBox = 0; nRealBox < nWWColMax; nBox++ )
            {
                if( nBox && pBoxArr[ nBox-1 ] == pBoxArr[ nBox ] )
                    continue;
                const SwFrmFmt& rFmt = *pBoxArr[ nBox ]->GetBox()->GetFrmFmt();

                pBrd = aBorders;
                for( int i = 0; i < 4; ++i, ++pBrd)
                {
                    USHORT nDist = rFmt.GetBox().GetDistance(*pBrd);
                    if (pDefaultFmt->GetBox().GetDistance(*pBrd) != nDist)
                    {
                        SwWW8Writer::InsUInt16(aAt, 0xD632);
                        aAt.Insert( BYTE(6), aAt.Count() );
                        aAt.Insert( BYTE(nRealBox), aAt.Count() );
                        aAt.Insert( BYTE(3), aAt.Count() );
                        aAt.Insert( BYTE(1 << i), aAt.Count() );
                        aAt.Insert( BYTE(3), aAt.Count() );
                        SwWW8Writer::InsUInt16(aAt, nDist);
                    }
                }
                ++nRealBox;
            }

            //Set Default, just taken from the first cell of the first
            //row
            pBrd = aBorders;
            for( int i = 0; i < 4; ++i, ++pBrd)
            {
                SwWW8Writer::InsUInt16(aAt, 0xD634);
                aAt.Insert( BYTE(6), aAt.Count() );
                aAt.Insert( BYTE(0), aAt.Count() );
                aAt.Insert( BYTE(1), aAt.Count() );
                aAt.Insert( BYTE(1 << i), aAt.Count() );
                aAt.Insert( BYTE(3), aAt.Count() );

                SwWW8Writer::InsUInt16(aAt,
                    pDefaultFmt->GetBox().GetDistance(*pBrd));
            }
        }

        // Background
        USHORT nBackg;
        if( pRow->GetBackground() )
            // over all boxes!
            nBackg = nWWColMax;
        else
        {
            nBackg = 0;
            for( nBox = 0, nRealBox = 0; nRealBox < nWWColMax; ++nBox )
            {
                if( nBox && pBoxArr[ nBox-1 ] == pBoxArr[ nBox ] )
                    continue;

                SwWriteTableCell* pCell = pBoxArr[ nBox ];
                if( pCell->GetBackground() ||
                    SFX_ITEM_ON == pCell->GetBox()->GetFrmFmt()->
                    GetItemState( RES_BACKGROUND, false) )
                    nBackg = nRealBox + 1;
                ++nRealBox;
            }
        }

        if( nBackg )
        {
            // TableShade, 0(+1) - OpCode, 1(2) - Count * sizeof( SHD ),
            // 2..(3..) - SHD
            if( rWW8Wrt.bWrtWW8 )
                SwWW8Writer::InsUInt16( aAt, 0xD609 );
            else
                aAt.Insert( (BYTE)191, aAt.Count() );
            aAt.Insert( (BYTE)(nBackg * 2), aAt.Count() );  // Len

            Color *pColors = new Color[nBackg];

            const SfxPoolItem* pI;
            for( nBox = 0, nRealBox = 0; nRealBox < nBackg; ++nBox )
            {
                if( nBox && pBoxArr[ nBox-1 ] == pBoxArr[ nBox ] )
                    continue;

                SwWriteTableCell* pCell = pBoxArr[ nBox ];
                if( SFX_ITEM_ON == pCell->GetBox()->GetFrmFmt()->
                    GetAttrSet().GetItemState( RES_BACKGROUND, false, &pI )
                    || 0 != ( pI = pCell->GetBackground() )
                    || 0 != ( pI = pRow->GetBackground() ) )
                {
                    pColors[nRealBox] = ((const SvxBrushItem*)pI)->GetColor();
                }
                else
                    pColors[nRealBox] = COL_AUTO;
                ++nRealBox;
            }

            for(USHORT nI = 0; nI < nBackg; ++nI)
            {
                WW8_SHD aShd;
                rWW8Wrt.TransBrush(pColors[nI], aShd);
                SwWW8Writer::InsUInt16(aAt, aShd.GetValue());
            }

            if( rWW8Wrt.bWrtWW8 )
            {
                SwWW8Writer::InsUInt16( aAt, 0xD612 );
                aAt.Insert( (BYTE)(nBackg * 10), aAt.Count() ); // Len
                for(USHORT nI = 0; nI < nBackg; ++nI)
                {
                    SwWW8Writer::InsUInt32(aAt, 0xFF000000);
                    SwWW8Writer::InsUInt32(aAt,
                        wwUtility::RGBToBGR(pColors[nI].GetColor()));
                    SwWW8Writer::InsUInt16(aAt, 0x0000);
                }
            }

            delete[] pColors;
        }

        rWW8Wrt.pPapPlc->AppendFkpEntry( rWrt.Strm().Tell(),
                                   aAt.Count(), aAt.GetData() );
        if( aAt.Count() > nStdAtLen )
            aAt.Remove( nStdAtLen, aAt.Count() - nStdAtLen );

        for( nBox = 0; nBox < nColCnt; ++nBox )
            --pRowSpans[ nBox ];
    }

    delete pTableWrt;
    delete[] pBoxArr;
    delete[] pRowSpans;

    // Pam hinter die Tabelle verschieben
    rWW8Wrt.pCurPam->GetPoint()->nNode = *rNode.EndOfSectionNode();

    return rWrt;
}

bool SwWW8Writer::NoPageBreakSection(const SfxItemSet* pSet)
{
    bool bRet = false;
    const SfxPoolItem* pI;
    if( pSet)
    {
        bool bNoPageBreak = false;
        if ( SFX_ITEM_ON != pSet->GetItemState(RES_PAGEDESC, true, &pI)
            || 0 == ((SwFmtPageDesc*)pI)->GetPageDesc() )
        {
            bNoPageBreak = true;
        }

        if (bNoPageBreak)
        {
            if (SFX_ITEM_ON != pSet->GetItemState(RES_BREAK, true, &pI))
                bNoPageBreak = true;
            else
            {
                SvxBreak eBreak = ((const SvxFmtBreakItem*)pI)->GetBreak();
                switch (eBreak)
                {
                    case SVX_BREAK_PAGE_BEFORE:
                    case SVX_BREAK_PAGE_AFTER:
                        bNoPageBreak = false;
                        break;
                    default:
                        break;
                }
            }
        }
        bRet = bNoPageBreak;
    }
    return bRet;
}

/*  */

Writer& OutWW8_SwSectionNode( Writer& rWrt, SwSectionNode& rSectionNode )
{
    SwWW8Writer& rWW8Wrt = (SwWW8Writer&)rWrt;
    const SwSection& rSection = rSectionNode.GetSection();

    SwNodeIndex aIdx( rSectionNode, 1 );
    const SwNode& rNd = aIdx.GetNode();
    if (!rNd.IsSectionNode() && !rWW8Wrt.bIsInTable) //No sections in table
    {
        // Bug 74245 - if the first Node inside the section has an own
        //              PageDesc or PageBreak attribut, then dont write
        //              here the section break
        ULONG nRstLnNum = 0;
        const SfxItemSet* pSet;
        if( rNd.IsTableNode() )
            pSet = &rNd.GetTableNode()->GetTable().GetFrmFmt()->GetAttrSet();
        else if( rNd.IsCntntNode() )
        {
            pSet = &rNd.GetCntntNode()->GetSwAttrSet();
            nRstLnNum = ((SwFmtLineNumber&)pSet->Get(
                            RES_LINENUMBER )).GetStartValue();
        }
        else
            pSet = 0;

        if( pSet && SwWW8Writer::NoPageBreakSection(pSet))
            pSet = 0;

        if( !pSet )
        {
            // new Section with no own PageDesc/-Break
            //  -> write follow section break;
            const SwSectionFmt& rFmt = *rSection.GetFmt();
            rWW8Wrt.ReplaceCr( (char)0xc ); // Indikator fuer Page/Section-Break

            //Get the page in use at the top of this section
            SwNodeIndex aIdx(rSectionNode, 1);
            const SwPageDesc *pCurrent =
                SwPageDesc::GetPageDescOfNode(aIdx.GetNode());
            if (!pCurrent)
                pCurrent = rWW8Wrt.pAktPageDesc;

            rWW8Wrt.pSepx->AppendSep(rWW8Wrt.Fc2Cp(rWrt.Strm().Tell()),
                pCurrent, &rFmt, nRstLnNum);
        }
    }
    if( TOX_CONTENT_SECTION == rSection.GetType() )
        rWW8Wrt.bStartTOX = true;
    return rWrt;
}

/*  */

//---------------------------------------------------------------------------
//       Flys
//---------------------------------------------------------------------------

void SwWW8Writer::OutWW8FlyFrmsInCntnt( const SwTxtNode& rNd )
{
    if (const SwpHints* pTxtAttrs = rNd.GetpSwpHints())
    {
        for( USHORT n=0; n < pTxtAttrs->Count(); ++n )
        {
            const SwTxtAttr* pAttr = (*pTxtAttrs)[ n ];
            if( RES_TXTATR_FLYCNT == pAttr->Which() )
            {
                // zeichengebundenes Attribut
                const SwFmtFlyCnt& rFlyCntnt = pAttr->GetFlyCnt();
                const SwFlyFrmFmt& rFlyFrmFmt = *(SwFlyFrmFmt*)rFlyCntnt.GetFrmFmt();
                const SwNodeIndex* pNodeIndex = rFlyFrmFmt.GetCntnt().GetCntntIdx();

                if( pNodeIndex )
                {
                    ULONG nStt = pNodeIndex->GetIndex()+1,
                          nEnd = pNodeIndex->GetNode().EndOfSectionIndex();

                    if( (nStt < nEnd) && !pDoc->GetNodes()[ nStt ]->IsNoTxtNode() )
                    {
                        Point aOffset;
                        // Rechtecke des Flys und des Absatzes besorgen
                        SwRect aParentRect(rNd.FindLayoutRect(false, &aOffset)),
                               aFlyRect(rFlyFrmFmt.FindLayoutRect(false, &aOffset ) );

                        aOffset = aFlyRect.Pos() - aParentRect.Pos();

                        // PaM umsetzen: auf Inhalt des Fly-Frameformats
                        WW8SaveData aSaveData( *this, nStt, nEnd );

                        // wird in Out_SwFmt() ausgewertet
                        pFlyOffset = &aOffset;
                        pFlyFmt = (SwFlyFrmFmt*)&rFlyFrmFmt;
                        eNewAnchorType = pFlyFmt->GetAnchor().GetAnchorId();
                        // Ok, rausschreiben:
                        WriteText();
                    }
                }
            }
        }
    }
}


void SwWW8Writer::OutWW8FlyFrm(const SwFrmFmt& rFrmFmt, const Point& rNdTopLeft)
{
    const SwFmtAnchor& rAnch = rFrmFmt.GetAnchor();

    /*
    ##897##
    Note that something anchored as a character must be
    exported using the older WW6 mechanism
    */
    if (!bWrtWW8 || (FLY_IN_CNTNT == rAnch.GetAnchorId()))
    {
        if (RES_DRAWFRMFMT == rFrmFmt.Which())
        {
            bool bComboBoxHack = false;
            if (bWrtWW8 && (FLY_IN_CNTNT == rAnch.GetAnchorId())) //#110185#
                bComboBoxHack = MiserableFormFieldExportHack(rFrmFmt);
            ASSERT(bComboBoxHack , "OutWW8FlyFrm: DrawInCnt-Baustelle " );
            return ;
        }

        bool bDone = false;

        // Hole vom Node und vom letzten Node die Position in der Section
        /*
        const SwFmtCntnt& rFlyCntnt = rFrmFmt.GetCntnt();

        ULONG nStt = rFlyCntnt.GetCntntIdx()->GetIndex()+1;
        ULONG nEnd = pDoc->GetNodes()[ nStt - 1 ]->EndOfSectionIndex();
        */
        const SwNodeIndex* pNodeIndex = rFrmFmt.GetCntnt().GetCntntIdx();

        ULONG nStt = pNodeIndex ? pNodeIndex->GetIndex()+1                  : 0;
        ULONG nEnd = pNodeIndex ? pNodeIndex->GetNode().EndOfSectionIndex() : 0;

        if( nStt >= nEnd )      // kein Bereich, also kein gueltiger Node
            return;

        if( !bIsInTable && (FLY_IN_CNTNT == rAnch.GetAnchorId()) )
        {
            // ein zeichen(!)gebundener Rahmen liegt vor
            SwTxtNode* pParTxtNode = rAnch.GetCntntAnchor()->nNode.GetNode().GetTxtNode();

            if( pParTxtNode && !pDoc->GetNodes()[ nStt ]->IsNoTxtNode() )
                bDone = true;
        }
        if( !bDone )
        {
            // ein NICHT zeichengebundener Rahmen liegt vor

            const SwFmtFrmSize& rS = rFrmFmt.GetFrmSize();
            nFlyWidth  = rS.GetWidth();  // Fuer Anpassung Graphic-Groesse
            nFlyHeight = rS.GetHeight();

            {
                WW8SaveData aSaveData( *this, nStt, nEnd );

                Point aOffset;
                if( pFlyFmt )
                {
                    /*
                    #90804#
                    Munge flys in fly into absolutely positioned elements for
                    word 6
                    */
                    aOffset = rFrmFmt.FindLayoutRect().Pos();
                    aOffset.X()-=DOCUMENTBORDER;
                    aOffset.Y()-=DOCUMENTBORDER;
                    pFlyOffset = &aOffset;
                    eNewAnchorType = FLY_PAGE;
                }

                pFlyFmt = (SwFlyFrmFmt*)&rFrmFmt;
                if( pFlyFmt )
                {
                    if( bIsInTable && (FLY_PAGE != rAnch.GetAnchorId()) &&
                        !pDoc->GetNodes()[ nStt ]->IsNoTxtNode() )
                    {
                        // Beachten: Flag  bOutTable  wieder setzen,
                        //           denn wir geben ja ganz normalen Content der
                        //           Tabelenzelle aus und keinen Rahmen
                        //           (Flag wurde oben in  aSaveData()  geloescht)
                        bOutTable = true;
                        const String& rName = pFlyFmt->GetName();
                        StartCommentOutput( rName );
                        WriteText();
                        EndCommentOutput( rName );
                    }
                    else
                        WriteText();
                }
                else
                    ASSERT( !this, "+Fly-Ausgabe ohne FlyFmt" );
            }
            // ASSERT( !pFlyFmt, " pFlyFmt ist hinter einem Rahmen nicht 0" );
        }
    }
    else
    {
        // write as escher
        WW8_CP nCP = Fc2Cp( Strm().Tell() );
        AppendFlyInFlys( nCP, rFrmFmt, rNdTopLeft );
    }
}

void SwWW8Writer::OutFlyFrm(const SwCntntNode& rNode, const SwFrmFmt& rFmt)
{
    if (bInWriteEscher)
        return;

    Point aNdPos, aPgPos;
    Point* pLayPos;
    bool bValidNdPos = false, bValidPgPos = false;

    if (FLY_PAGE == rFmt.GetAnchor().GetAnchorId())
    {
        // get the Layout Node-Position.
        if (!bValidPgPos)
        {
            aPgPos = rNode.FindPageFrmRect(false, &aPgPos).Pos();
            bValidPgPos = true;
        }
        pLayPos = &aPgPos;
    }
    else
    {
        // get the Layout Node-Position.
        if (!bValidNdPos)
        {
            aNdPos = rNode.FindLayoutRect(false, &aNdPos).Pos();
            bValidNdPos = true;
        }
        pLayPos = &aNdPos;
    }

    OutWW8FlyFrm(rFmt, *pLayPos);
}

// write data of any redline
void SwWW8Writer::OutRedline( const SwRedlineData& rRedline )
{
    if( rRedline.Next() )
        OutRedline( *rRedline.Next() );

    static USHORT __READONLY_DATA aSprmIds[ 2 * 2 * 3 ] =
    {
        // Ids for insert
            0x0801, 0x4804, 0x6805,         // for WW8
            0x0042, 0x0045, 0x0046,         // for WW6
        // Ids for delete
            0x0800, 0x4863, 0x6864,         // for WW8
            0x0041, 0x0045, 0x0046          // for WW6
    };

    const USHORT* pSprmIds = 0;
    switch( rRedline.GetType() )
    {
    case REDLINE_INSERT:
        pSprmIds = aSprmIds;
        break;

    case REDLINE_DELETE:
        pSprmIds = aSprmIds + (2 * 3);
        break;

    case REDLINE_FORMAT:
        if( bWrtWW8 )
        {
            InsUInt16( 0xca57 );
            pO->Insert( 7, pO->Count() );       // len
            pO->Insert( 1, pO->Count() );
            InsUInt16( AddRedlineAuthor( rRedline.GetAuthor() ) );
            InsUInt32( SwWW8Writer::GetDTTM( rRedline.GetTimeStamp() ));
        }
        break;
    }

    if( pSprmIds )
    {
        if( !bWrtWW8 )
            pSprmIds += 3;

        if( bWrtWW8 )
            InsUInt16( pSprmIds[0] );
        else
            pO->Insert(msword_cast<sal_uInt8>(pSprmIds[0]), pO->Count());
        pO->Insert( 1, pO->Count() );

        if( bWrtWW8 )
            InsUInt16( pSprmIds[1] );
        else
            pO->Insert(msword_cast<sal_uInt8>(pSprmIds[1]), pO->Count());
        InsUInt16( AddRedlineAuthor( rRedline.GetAuthor() ) );

        if( bWrtWW8 )
            InsUInt16( pSprmIds[2] );
        else
            pO->Insert(msword_cast<sal_uInt8>(pSprmIds[2]), pO->Count());
        InsUInt32( SwWW8Writer::GetDTTM( rRedline.GetTimeStamp() ));
    }
}

/*  */

SwNodeFnTab aWW8NodeFnTab = {
/* RES_TXTNODE  */                   OutWW8_SwTxtNode,
/* RES_GRFNODE  */                   OutWW8_SwGrfNode,
/* RES_OLENODE  */                   OutWW8_SwOleNode,
};

/* vi:set tabstop=4 shiftwidth=4 expandtab: */

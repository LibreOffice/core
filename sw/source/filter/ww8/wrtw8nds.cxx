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


#include <vector>
#include <list>
#include <utility>
#include <algorithm>
#include <functional>
#include <iostream>

#include <hintids.hxx>
#include <comphelper/string.hxx>
#include <tools/urlobj.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/cmapitem.hxx>
#include <editeng/langitem.hxx>
#include <editeng/svxfont.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/keepitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/formatbreakitem.hxx>
#include <editeng/frmdiritem.hxx>
#include <editeng/tstpitem.hxx>
#include "svl/urihelper.hxx"
#include <svl/whiter.hxx>
#include <fmtpdsc.hxx>
#include <fmtfsize.hxx>
#include <fmtornt.hxx>
#include <fmtlsplt.hxx>
#include <fmtflcnt.hxx>
#include <fmtanchr.hxx>
#include <fmtcntnt.hxx>
#include <frmatr.hxx>
#include <paratr.hxx>
#include <txatbase.hxx>
#include <fmtinfmt.hxx>
#include <fmtrfmrk.hxx>
#include <fchrfmt.hxx>
#include <fmtautofmt.hxx>
#include <charfmt.hxx>
#include <tox.hxx>
#include <ndtxt.hxx>
#include <pam.hxx>
#include <doc.hxx>
#include <docary.hxx>
#include <swtable.hxx>
#include <swtblfmt.hxx>
#include <section.hxx>
#include <pagedesc.hxx>
#include <swrect.hxx>
#include <reffld.hxx>
#include <redline.hxx>
#include <wrtswtbl.hxx>
#include <htmltbl.hxx>
#include <txttxmrk.hxx>
#include <fmtline.hxx>
#include <fmtruby.hxx>
#include <breakit.hxx>
#include <txtatr.hxx>
#include <fmtsrnd.hxx>
#include <fmtrowsplt.hxx>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <com/sun/star/i18n/WordType.hpp>

#include <doctok/sprmids.hxx>

#include "writerhelper.hxx"
#include "writerwordglue.hxx"
#include <numrule.hxx>
#include "wrtww8.hxx"
#include "ww8par.hxx"
#include <IMark.hxx>
#include "ww8attributeoutput.hxx"

#include <ndgrf.hxx>
#include <ndole.hxx>

#include <cstdio>

using namespace ::com::sun::star;
using namespace ::com::sun::star::i18n;
using namespace sw::util;
using namespace sw::types;
using namespace sw::mark;
using namespace nsFieldFlags;

static OUString lcl_getFieldCode( const IFieldmark* pFieldmark )
{
    OSL_ENSURE(pFieldmark!=NULL, "where is my fieldmark???");

    if ( !pFieldmark)
        return OUString();
    if ( pFieldmark->GetFieldname( ) == ODF_FORMTEXT )
        return OUString(" FORMTEXT ");
    if ( pFieldmark->GetFieldname( ) == ODF_FORMDROPDOWN )
        return OUString(" FORMDROPDOWN ");
    if ( pFieldmark->GetFieldname( ) == ODF_FORMCHECKBOX )
        return OUString(" FORMCHECKBOX ");
    if ( pFieldmark->GetFieldname( ) == ODF_TOC )
        return OUString(" TOC ");
    if ( pFieldmark->GetFieldname( ) == ODF_HYPERLINK )
        return OUString(" HYPERLINK ");
    if ( pFieldmark->GetFieldname( ) == ODF_PAGEREF )
        return OUString(" PAGEREF ");
    return pFieldmark->GetFieldname();
}

static ww::eField lcl_getFieldId( const IFieldmark* pFieldmark ) {
    OSL_ENSURE(pFieldmark!=NULL, "where is my fieldmark???");
    if ( !pFieldmark )
        return ww::eUNKNOWN;
    if ( pFieldmark->GetFieldname( ) == ODF_FORMTEXT )
        return ww::eFORMTEXT;
    if ( pFieldmark->GetFieldname( ) == ODF_FORMDROPDOWN )
        return ww::eFORMDROPDOWN;
    if ( pFieldmark->GetFieldname( ) == ODF_FORMCHECKBOX )
        return ww::eFORMCHECKBOX;
    if ( pFieldmark->GetFieldname( ) == ODF_TOC )
        return ww::eTOC;
    if ( pFieldmark->GetFieldname( ) == ODF_HYPERLINK )
        return ww::eHYPERLINK;
    if ( pFieldmark->GetFieldname( ) == ODF_PAGEREF )
        return ww::ePAGEREF;
    return ww::eUNKNOWN;
}

MSWordAttrIter::MSWordAttrIter( MSWordExportBase& rExport )
    : pOld( rExport.pChpIter ), m_rExport( rExport )
{
    m_rExport.pChpIter = this;
}

MSWordAttrIter::~MSWordAttrIter()
{
    m_rExport.pChpIter = pOld;
}

class sortswflys :
    public std::binary_function<const sw::Frame&, const sw::Frame&, bool>
{
public:
    bool operator()(const sw::Frame &rOne, const sw::Frame &rTwo) const
    {
        return rOne.GetPosition() < rTwo.GetPosition();
    }
};

void SwWW8AttrIter::IterToCurrent()
{
    OSL_ENSURE(maCharRuns.begin() != maCharRuns.end(), "Impossible");
    mnScript = maCharRunIter->mnScript;
    meChrSet = maCharRunIter->meCharSet;
    mbCharIsRTL = maCharRunIter->mbRTL;
}

SwWW8AttrIter::SwWW8AttrIter(MSWordExportBase& rWr, const SwTxtNode& rTxtNd) :
    MSWordAttrIter(rWr),
    rNd(rTxtNd),
    maCharRuns(GetPseudoCharRuns(rTxtNd, 0, !rWr.SupportsUnicode())),
    pCurRedline(0),
    nAktSwPos(0),
    nCurRedlinePos(USHRT_MAX),
    mrSwFmtDrop(rTxtNd.GetSwAttrSet().GetDrop())
{

    SwPosition aPos(rTxtNd);
    if (FRMDIR_HORI_RIGHT_TOP == rWr.pDoc->GetTextDirection(aPos))
        mbParaIsRTL = true;
    else
        mbParaIsRTL = false;

    maCharRunIter = maCharRuns.begin();
    IterToCurrent();

    /*
     #i2916#
     Get list of any graphics which may be anchored from this paragraph.
    */
    maFlyFrms = GetFramesInNode(rWr.maFrames, rNd);
    std::sort(maFlyFrms.begin(), maFlyFrms.end(), sortswflys());

    /*
     #i18480#
     If we are inside a frame then anything anchored inside this frame can
     only be supported by word anchored inline ("as character"), so force
     this in the supportable case.
    */
    if (rWr.SupportsUnicode() && rWr.bInWriteEscher)
    {
        std::for_each(maFlyFrms.begin(), maFlyFrms.end(),
            std::mem_fun_ref(&sw::Frame::ForceTreatAsInline));
    }

    maFlyIter = maFlyFrms.begin();

    if ( !m_rExport.pDoc->GetRedlineTbl().empty() )
    {
        SwPosition aPosition( rNd, SwIndex( (SwTxtNode*)&rNd ) );
        pCurRedline = m_rExport.pDoc->GetRedline( aPosition, &nCurRedlinePos );
    }

    nAktSwPos = SearchNext(1);
}

xub_StrLen lcl_getMinPos( xub_StrLen pos1, xub_StrLen pos2 )
{
    xub_StrLen min = STRING_NOTFOUND;
    if ( pos1 == STRING_NOTFOUND && pos2 != STRING_NOTFOUND )
        min = pos2;
    else if ( pos2 == STRING_NOTFOUND && pos1 != STRING_NOTFOUND )
        min = pos1;
    else if ( pos1 != STRING_NOTFOUND && pos2 != STRING_NOTFOUND )
    {
        if ( pos1 < pos2 )
            min = pos1;
        else
            min = pos2;
    }

    return min;
}

xub_StrLen SwWW8AttrIter::SearchNext( xub_StrLen nStartPos )
{
    xub_StrLen nPos;
    xub_StrLen nMinPos = STRING_MAXLEN;
    xub_StrLen i=0;

    const String aTxt = rNd.GetTxt();
    xub_StrLen fieldEndPos = aTxt.Search(CH_TXT_ATR_FIELDEND, nStartPos);
    xub_StrLen fieldStartPos = aTxt.Search(CH_TXT_ATR_FIELDSTART, nStartPos);
    xub_StrLen formElementPos = aTxt.Search(CH_TXT_ATR_FORMELEMENT, nStartPos);

    xub_StrLen pos = lcl_getMinPos( fieldEndPos, fieldStartPos );
    pos = lcl_getMinPos( pos, formElementPos );

    if (pos!=STRING_NOTFOUND)
        nMinPos=pos;

    // first the redline, then the attributes
    if( pCurRedline )
    {
        const SwPosition* pEnd = pCurRedline->End();
        if (pEnd->nNode == rNd && ((i = pEnd->nContent.GetIndex()) >= nStartPos) && i < nMinPos )
                nMinPos = i;
    }

    if ( nCurRedlinePos < m_rExport.pDoc->GetRedlineTbl().size() )
    {
        // nCurRedlinePos point to the next redline
        nPos = nCurRedlinePos;
        if( pCurRedline )
            ++nPos;

        for ( ; nPos < m_rExport.pDoc->GetRedlineTbl().size(); ++nPos )
        {
            const SwRedline* pRedl = m_rExport.pDoc->GetRedlineTbl()[ nPos ];

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


    if (mrSwFmtDrop.GetWholeWord() && nStartPos <= rNd.GetDropLen(0))
        nMinPos = rNd.GetDropLen(0);
    else if(nStartPos <= mrSwFmtDrop.GetChars())
        nMinPos = mrSwFmtDrop.GetChars();

    if(const SwpHints* pTxtAttrs = rNd.GetpSwpHints())
    {

// can be optimized if we consider that the TxtAttrs are sorted by start position.
// but then we'd have to save 2 indices
        for( i = 0; i < pTxtAttrs->Count(); i++ )
        {
            const SwTxtAttr* pHt = (*pTxtAttrs)[i];
            nPos = *pHt->GetStart();    // first Attr characters
            if( nPos >= nStartPos && nPos <= nMinPos )
                nMinPos = nPos;

            if( pHt->GetEnd() )         // Attr with end
            {
                nPos = *pHt->GetEnd();      // last Attr character + 1
                if( nPos >= nStartPos && nPos <= nMinPos )
                    nMinPos = nPos;
            }
            if (pHt->HasDummyChar())
            {
                // pos + 1 because of CH_TXTATR in Text
                nPos = *pHt->GetStart() + 1;
                if( nPos >= nStartPos && nPos <= nMinPos )
                    nMinPos = nPos;
            }
        }
    }

    if (maCharRunIter != maCharRuns.end())
    {
        if (maCharRunIter->mnEndPos < nMinPos)
            nMinPos = maCharRunIter->mnEndPos;
        IterToCurrent();
    }

    /*
     #i2916#
     Check to see if there are any graphics anchored to characters in this
     paragraph's text. Set nMinPos to 1 past the placement for anchored to
     character because anchors in Word appear after the character they are
     anchored to.
    */
    if (maFlyIter != maFlyFrms.end())
    {
        const SwPosition &rAnchor = maFlyIter->GetPosition();

        nPos = rAnchor.nContent.GetIndex();
        if (nPos >= nStartPos && nPos <= nMinPos)
            nMinPos = nPos;

        if (maFlyIter->GetFrmFmt().GetAnchor().GetAnchorId() == FLY_AT_CHAR)
        {
            ++nPos;
            if (nPos >= nStartPos && nPos <= nMinPos)
                nMinPos = nPos;
        }
    }

    //nMinPos found and not going to change at this point

    if (maCharRunIter != maCharRuns.end())
    {
        if (maCharRunIter->mnEndPos == nMinPos)
            ++maCharRunIter;
    }

    return nMinPos;
}

static bool lcl_isFontsizeItem( const SfxPoolItem& rItem )
{
    return ( rItem.Which( ) == RES_CHRATR_FONTSIZE ||
            rItem.Which( ) == RES_CHRATR_CJK_FONTSIZE ||
            rItem.Which( ) == RES_CHRATR_CTL_FONTSIZE );
}

void SwWW8AttrIter::OutAttr( xub_StrLen nSwPos, bool bRuby )
{
    m_rExport.AttrOutput().RTLAndCJKState( IsCharRTL(), GetScript() );

    /*
     Depending on whether text is in CTL/CJK or Western, get the id of that
     script, the idea is that the font that is actually in use to render this
     range of text ends up in pFont
    */
    sal_uInt16 nFontId = GetWhichOfScript( RES_CHRATR_FONT, GetScript() );

    const SvxFontItem &rParentFont = ItemGet<SvxFontItem>(
        (const SwTxtFmtColl&)rNd.GetAnyFmtColl(), nFontId);
    const SvxFontItem *pFont = &rParentFont;

    SfxItemSet aExportSet(*rNd.GetSwAttrSet().GetPool(),
        RES_CHRATR_BEGIN, RES_TXTATR_END - 1);

    //The hard formatting properties that affect the entire paragraph
    if (rNd.HasSwAttrSet())
    {
        sal_Bool bDeep = sal_False;
        // only copy hard attributes - bDeep = false
        aExportSet.Set(rNd.GetSwAttrSet(), bDeep);
        // get the current font item. Use rNd.GetSwAttrSet instead of aExportSet:
        const SvxFontItem &rNdFont = ItemGet<SvxFontItem>(rNd.GetSwAttrSet(), nFontId);
        pFont = &rNdFont;
        aExportSet.ClearItem(nFontId);
    }

    //The additional hard formatting properties that affect this range in the
    //paragraph
    sw::PoolItems aRangeItems;
    if (const SwpHints* pTxtAttrs = rNd.GetpSwpHints())
    {
        for (xub_StrLen i = 0; i < pTxtAttrs->Count(); ++i)
        {
            const SwTxtAttr* pHt = (*pTxtAttrs)[i];
            const xub_StrLen* pEnd = pHt->GetEnd();

            if (pEnd ? ( nSwPos >= *pHt->GetStart() && nSwPos < *pEnd)
                        : nSwPos == *pHt->GetStart() )
            {
                sal_uInt16 nWhich = pHt->GetAttr().Which();
                if (nWhich == RES_TXTATR_AUTOFMT)
                {
                    const SwFmtAutoFmt& rAutoFmt = static_cast<const SwFmtAutoFmt&>(pHt->GetAttr());
                    const boost::shared_ptr<SfxItemSet> pSet = rAutoFmt.GetStyleHandle();
                    SfxWhichIter aIter( *pSet );
                    const SfxPoolItem* pItem;
                    sal_uInt16 nWhichId = aIter.FirstWhich();
                    while( nWhichId )
                    {
                        if( SFX_ITEM_SET == pSet->GetItemState( nWhichId, sal_False, &pItem ))
                        {
                            if (nWhichId == nFontId)
                                pFont = &(item_cast<SvxFontItem>(*pItem));
                            else
                                aRangeItems[nWhichId] = pItem;
                        }
                        nWhichId = aIter.NextWhich();
                    }
                }
                else
                    aRangeItems[nWhich] = (&(pHt->GetAttr()));
            }
            else if (nSwPos < *pHt->GetStart())
                break;
        }
    }

    /*
     For #i24291# we need to explictly remove any properties from the
     aExportSet which a SwCharFmt would override, we can't rely on word doing
     this for us like writer does
    */
    const SwFmtCharFmt *pCharFmtItem =
        HasItem< SwFmtCharFmt >( aRangeItems, RES_TXTATR_CHARFMT );
    if ( pCharFmtItem )
        ClearOverridesFromSet( *pCharFmtItem, aExportSet );

    sw::PoolItems aExportItems;
    GetPoolItems( aExportSet, aExportItems, false );

    sw::cPoolItemIter aEnd = aRangeItems.end();
    for ( sw::cPoolItemIter aI = aRangeItems.begin(); aI != aEnd; ++aI )
    {
        if ( !bRuby || !lcl_isFontsizeItem( *aI->second ) )
            aExportItems[aI->first] = aI->second;
    }

    if ( !aExportItems.empty() )
    {
        const SwModify* pOldMod = m_rExport.pOutFmtNode;
        m_rExport.pOutFmtNode = &rNd;
        m_rExport.m_aCurrentCharPropStarts.push( nSwPos );

        m_rExport.ExportPoolItemsToCHP( aExportItems, GetScript() );

        // HasTextItem only allowed in the above range
        m_rExport.m_aCurrentCharPropStarts.pop();
        m_rExport.pOutFmtNode = pOldMod;
    }

    OSL_ENSURE( pFont, "must be *some* font associated with this txtnode" );
    if ( pFont )
    {
        SvxFontItem aFont( *pFont );

        /*
         If we are a nonunicode aware format then we set the charset we want to
         use for export of this range. If necessary this will generate a pseudo
         font to use for this range.

         So now we are guaranteed to have a font with the correct charset set
         for WW6/95 which will match the script we have exported this range in,
         this makes older nonunicode aware versions of word display the correct
         characters.
        */
        if ( !m_rExport.SupportsUnicode() )
            aFont.SetCharSet( GetCharSet() );

        if ( rParentFont != aFont )
            m_rExport.AttrOutput().OutputItem( aFont );
    }
}

void SwWW8AttrIter::OutFlys(xub_StrLen nSwPos)
{
    /*
     #i2916#
     May have an anchored graphic to be placed, loop through sorted array
     and output all at this position
    */
    while ( maFlyIter != maFlyFrms.end() )
    {
        const SwPosition &rAnchor = maFlyIter->GetPosition();
        xub_StrLen nPos = rAnchor.nContent.GetIndex();

        if ( nPos != nSwPos )
            break;
        else
        {
            m_rExport.AttrOutput().OutputFlyFrame( *maFlyIter );
            ++maFlyIter;
        }
    }
}

bool SwWW8AttrIter::IsTxtAttr( xub_StrLen nSwPos )
{
    // search for attrs with CH_TXTATR
    if (const SwpHints* pTxtAttrs = rNd.GetpSwpHints())
    {
        for (sal_uInt16 i = 0; i < pTxtAttrs->Count(); ++i)
        {
            const SwTxtAttr* pHt = (*pTxtAttrs)[i];
            if ( pHt->HasDummyChar() && (*pHt->GetStart() == nSwPos) )
                return true;
        }
    }

    return false;
}

bool SwWW8AttrIter::IsDropCap( int nSwPos )
{
    // see if the current position falls on a DropCap
    int nDropChars = mrSwFmtDrop.GetChars();
    bool bWholeWord = mrSwFmtDrop.GetWholeWord();
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
    return false;
}

bool SwWW8AttrIter::RequiresImplicitBookmark()
{
    SwImplBookmarksIter bkmkIterEnd = m_rExport.maImplicitBookmarks.end();
    for ( SwImplBookmarksIter aIter = m_rExport.maImplicitBookmarks.begin(); aIter != bkmkIterEnd; ++aIter )
    {
        sal_uLong sample  = aIter->second;

        if ( sample == rNd.GetIndex() )
            return true;
    }
    return false;
}

//HasItem is for the summary of the double attributes: Underline and WordlineMode as TextItems.
// OutAttr () calls the output function, which can call HasItem() for other items at the attribute's start position.
// Only attributes with end can be queried.
// It searches with bDeep
const SfxPoolItem* SwWW8AttrIter::HasTextItem( sal_uInt16 nWhich ) const
{
    const SfxPoolItem* pRet = 0;
    const SwpHints* pTxtAttrs = rNd.GetpSwpHints();

    if (pTxtAttrs && !m_rExport.m_aCurrentCharPropStarts.empty())
    {
        xub_StrLen nTmpSwPos = m_rExport.m_aCurrentCharPropStarts.top();
        for (sal_uInt16 i = 0; i < pTxtAttrs->Count(); ++i)
        {
            const SwTxtAttr* pHt = (*pTxtAttrs)[i];
            const SfxPoolItem* pItem = &pHt->GetAttr();
            const xub_StrLen* pAtrEnd = 0;
            if( 0 != ( pAtrEnd = pHt->GetEnd() ) &&     // only Attr with an end
                nWhich == pItem->Which() &&
                nTmpSwPos >= *pHt->GetStart() && nTmpSwPos < *pAtrEnd )
            {
                pRet = pItem;       // found it
                break;
            }
            else if (nTmpSwPos < *pHt->GetStart())
                break;              // nothing more to come
        }
    }
    return pRet;
}

void WW8Export::GetCurrentItems(ww::bytes &rItems) const
{
    rItems.insert(rItems.end(), pO->begin(), pO->end());
}

const SfxPoolItem& SwWW8AttrIter::GetItem(sal_uInt16 nWhich) const
{
    const SfxPoolItem* pRet = HasTextItem(nWhich);
    return pRet ? *pRet : rNd.SwCntntNode::GetAttr(nWhich);
}

void WW8AttributeOutput::StartRuby( const SwTxtNode& rNode, xub_StrLen /*nPos*/, const SwFmtRuby& rRuby )
{
    String aStr( FieldString( ww::eEQ ) );
    aStr.AppendAscii( "\\* jc" );
    sal_Int32 nJC = 0;
    sal_Char cDirective = 0;
    switch ( rRuby.GetAdjustment() )
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
            OSL_ENSURE( !this,"Unhandled Ruby justication code" );
            break;
    }
    aStr += OUString::number( nJC );

    /*
     MS needs to know the name and size of the font used in the ruby item,
     but we coud have written it in a mixture of asian and western
     scripts, and each of these can be a different font and size than the
     other, so we make a guess based upon the first character of the text,
     defaulting to asian.
     */
    sal_uInt16 nRubyScript;
    if( g_pBreakIt->GetBreakIter().is() )
        nRubyScript = g_pBreakIt->GetBreakIter()->getScriptType( rRuby.GetText(), 0);
    else
        nRubyScript = i18n::ScriptType::ASIAN;

    const SwTxtRuby* pRubyTxt = rRuby.GetTxtRuby();
    const SwCharFmt* pFmt = pRubyTxt ? pRubyTxt->GetCharFmt() : 0;
    String sFamilyName;
    long nHeight;
    if ( pFmt )
    {
        const SvxFontItem &rFont = ItemGet< SvxFontItem >( *pFmt,
                GetWhichOfScript(RES_CHRATR_FONT,nRubyScript) );
        sFamilyName = rFont.GetFamilyName();

        const SvxFontHeightItem &rHeight = ItemGet< SvxFontHeightItem >( *pFmt,
                GetWhichOfScript( RES_CHRATR_FONTSIZE, nRubyScript ) );
        nHeight = rHeight.GetHeight();
    }
    else
    {
        /*Get defaults if no formatting on ruby text*/

        const SfxItemPool *pPool = rNode.GetSwAttrSet().GetPool();
        pPool = pPool ? pPool : &m_rWW8Export.pDoc->GetAttrPool();

        const SvxFontItem &rFont  = DefaultItemGet< SvxFontItem >( *pPool,
                GetWhichOfScript( RES_CHRATR_FONT,nRubyScript ) );
        sFamilyName = rFont.GetFamilyName();

        const SvxFontHeightItem &rHeight = DefaultItemGet< SvxFontHeightItem >
            ( *pPool, GetWhichOfScript( RES_CHRATR_FONTSIZE, nRubyScript ) );
        nHeight = rHeight.GetHeight();
    }
    nHeight = (nHeight + 5)/10;

    aStr.AppendAscii( " \\* \"Font:" );
    aStr.Append( sFamilyName );
    aStr.AppendAscii( "\" \\* hps" );
    aStr += OUString::number( nHeight );
    aStr.AppendAscii( " \\o" );
    if ( cDirective )
    {
        aStr.AppendAscii( "\\a" );
        aStr.Append( cDirective );
    }
    aStr.AppendAscii( "(\\s\\up " );


    if ( g_pBreakIt->GetBreakIter().is() )
        nRubyScript = g_pBreakIt->GetBreakIter()->getScriptType( rNode.GetTxt(),
                *( pRubyTxt->GetStart() ) );
    else
        nRubyScript = i18n::ScriptType::ASIAN;

    const SwAttrSet& rSet = rNode.GetSwAttrSet();
    const SvxFontHeightItem &rHeightItem  =
        ( const SvxFontHeightItem& )rSet.Get(
                                             GetWhichOfScript( RES_CHRATR_FONTSIZE, nRubyScript ) );
    nHeight = (rHeightItem.GetHeight() + 10)/20-1;
    aStr += OUString::number(nHeight);
    aStr += '(';
    aStr += rRuby.GetText();
    aStr.AppendAscii( ")" );

    // The parameter separator depends on the FIB.lid
    if ( m_rWW8Export.pFib->getNumDecimalSep() == '.' )
        aStr.AppendAscii( "," );
    else
        aStr.AppendAscii( ";" );

    m_rWW8Export.OutputField( 0, ww::eEQ, aStr,
            WRITEFIELD_START | WRITEFIELD_CMD_START );
}

void WW8AttributeOutput::EndRuby()
{
    m_rWW8Export.WriteChar( ')' );
    m_rWW8Export.OutputField( 0, ww::eEQ, aEmptyStr, WRITEFIELD_END | WRITEFIELD_CLOSE );
}

/*#i15387# Better ideas welcome*/
String &TruncateBookmark( String &rRet )
{
    if ( rRet.Len() > 40 )
        rRet.Erase( 40 );
    OSL_ENSURE( rRet.Len() <= 40, "Word cannot have bookmarks longer than 40 chars" );
    return rRet;
}

bool AttributeOutputBase::AnalyzeURL( const String& rUrl, const String& /*rTarget*/, String* pLinkURL, String* pMark )
{
    bool bBookMarkOnly = false;

    INetURLObject aURL( rUrl );
    String sMark;
    String sURL;

    if ( rUrl.Len() > 1 && rUrl.GetChar(0) == INET_MARK_TOKEN )
    {
        sMark = BookmarkToWriter( rUrl.Copy(1) );

        xub_StrLen nPos = sMark.SearchBackward( cMarkSeparator );

        String sRefType(comphelper::string::remove(sMark.Copy(nPos+1), ' '));

        // #i21465# Only interested in outline references
        if ( sRefType.EqualsAscii( pMarkToOutline ) )
        {
            String sLink = sMark.Copy(0, nPos);
            SwImplBookmarksIter bkmkIterEnd = GetExport().maImplicitBookmarks.end();
            for ( SwImplBookmarksIter aIter = GetExport().maImplicitBookmarks.begin(); aIter != bkmkIterEnd; ++aIter )
            {
                String bkmkName  = aIter->first;

                if ( bkmkName == sLink )
                {
                    sMark = String(  "_toc"  );
                    sMark += OUString::number( aIter->second );
                }
            }
        }
    }
    else
    {
        sURL = aURL.GetURLNoMark( INetURLObject::DECODE_UNAMBIGUOUS );
        sMark = aURL.GetMark( INetURLObject::DECODE_UNAMBIGUOUS );

    }

    if ( sMark.Len() && !sURL.Len() )
        bBookMarkOnly = true;



    *pMark = sMark;
    *pLinkURL = sURL;
    return bBookMarkOnly;
}

bool WW8AttributeOutput::AnalyzeURL( const String& rUrl, const String& rTarget, String* pLinkURL, String* pMark )
{
    bool bBookMarkOnly = AttributeOutputBase::AnalyzeURL( rUrl, rTarget, pLinkURL, pMark );

    String sURL = *pLinkURL;
    String sMark = *pMark;

    if ( sURL.Len() )
        sURL = URIHelper::simpleNormalizedMakeRelative( m_rWW8Export.GetWriter().GetBaseURL(), sURL );

    if ( bBookMarkOnly )
        sURL = FieldString( ww::eHYPERLINK );
    else
    {
        String sFld( FieldString( ww::eHYPERLINK ) );
        sFld.AppendAscii( "\"" );
        sURL.Insert( sFld, 0 );
        sURL += '\"';
    }

    if ( sMark.Len() )
        ( ( sURL.AppendAscii( " \\l \"" ) ) += sMark ) += '\"';

    if ( rTarget.Len() )
        ( sURL.AppendAscii( " \\n " ) ) += rTarget;

    *pLinkURL = sURL;
    *pMark = sMark;

    return bBookMarkOnly;
}

bool WW8AttributeOutput::StartURL( const String &rUrl, const String &rTarget )
{
    // hyperlinks only in WW8
    if ( !m_rWW8Export.bWrtWW8 )
        return false;

    INetURLObject aURL( rUrl );
    String sURL;
    String sMark;

    bool bBookMarkOnly = AnalyzeURL( rUrl, rTarget, &sURL, &sMark );


    m_rWW8Export.OutputField( 0, ww::eHYPERLINK, sURL, WRITEFIELD_START | WRITEFIELD_CMD_START );

    // write the refence to the "picture" structure
    sal_uLong nDataStt = m_rWW8Export.pDataStrm->Tell();
    m_rWW8Export.pChpPlc->AppendFkpEntry( m_rWW8Export.Strm().Tell() );

    // WinWord 2000 doesn't write this - so its a temp solution by W97 ?
    m_rWW8Export.WriteChar( 0x01 );

    static sal_uInt8 aArr1[] = {
        0x03, 0x6a, 0,0,0,0,    // sprmCPicLocation

        0x06, 0x08, 0x01,       // sprmCFData
        0x55, 0x08, 0x01,       // sprmCFSpec
        0x02, 0x08, 0x01        // sprmCFFldVanish
    };
    sal_uInt8* pDataAdr = aArr1 + 2;
    Set_UInt32( pDataAdr, nDataStt );

    m_rWW8Export.pChpPlc->AppendFkpEntry( m_rWW8Export.Strm().Tell(), sizeof( aArr1 ), aArr1 );

    m_rWW8Export.OutputField( 0, ww::eHYPERLINK, sURL, WRITEFIELD_CMD_END );

    // now write the picture structur
    sURL = aURL.GetURLNoMark();

    // Compare the URL written by AnalyzeURL with the original one to see if
    // the output URL is absolute or relative.
    String sRelativeURL;
    if ( rUrl.Len() )
        sRelativeURL = URIHelper::simpleNormalizedMakeRelative( m_rWW8Export.GetWriter().GetBaseURL(), rUrl );
    bool bAbsolute = sRelativeURL.Equals( rUrl );

    static sal_uInt8 aURLData1[] = {
        0,0,0,0,        // len of struct
        0x44,0,         // the start of "next" data
        0,0,0,0,0,0,0,0,0,0,                // PIC-Structure!
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,    //  |
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,    //  |
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,    //  |
        0,0,0,0,                            // /
    };
    static sal_uInt8 MAGIC_A[] = {
        // start of "next" data
        0xD0,0xC9,0xEA,0x79,0xF9,0xBA,0xCE,0x11,
        0x8C,0x82,0x00,0xAA,0x00,0x4B,0xA9,0x0B
    };

    m_rWW8Export.pDataStrm->Write( aURLData1, sizeof( aURLData1 ) );
    /* Write HFD Structure */
    sal_uInt8 nAnchor = 0x00;
    if ( sMark.Len() )
        nAnchor = 0x08;
    m_rWW8Export.pDataStrm->Write( &nAnchor, 1 ); // HFDBits
    m_rWW8Export.pDataStrm->Write( MAGIC_A, sizeof(MAGIC_A) ); //clsid

    /* Write Hyperlink Object see [MS-OSHARED] spec*/
    SwWW8Writer::WriteLong( *m_rWW8Export.pDataStrm, 0x00000002);
    sal_uInt32 nFlag = bBookMarkOnly ? 0 : 0x01;
    if ( bAbsolute )
        nFlag |= 0x02;
    if ( sMark.Len() )
        nFlag |= 0x08;
    SwWW8Writer::WriteLong( *m_rWW8Export.pDataStrm, nFlag );

    INetProtocol eProto = aURL.GetProtocol();
    if ( eProto == INET_PROT_FILE || eProto == INET_PROT_SMB )
    {
        // version 1 (for a document)

        static sal_uInt8 MAGIC_C[] = {
            0x03, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46,
            0x00, 0x00
        };

        static sal_uInt8 MAGIC_D[] = {
            0xFF, 0xFF, 0xAD, 0xDE, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        };

        // save the links to files as relative
        sURL = URIHelper::simpleNormalizedMakeRelative( m_rWW8Export.GetWriter().GetBaseURL(), sURL );
        if ( eProto == INET_PROT_FILE && sURL.EqualsAscii( "/", 0, 1 ) )
            sURL = aURL.PathToFileName();

        // special case for the absolute windows names
        // (convert '/c:/foo/bar.doc' into 'c:\foo\bar.doc')
        sal_Unicode aDrive = ( sURL.Len() > 1 )? sURL.GetChar( 1 ): 0;
        if ( sURL.EqualsAscii( "/", 0, 1 ) &&
             ( ( aDrive >= 'A' && aDrive <= 'Z' ) || ( aDrive >= 'a' && aDrive <= 'z' ) ) &&
             sURL.EqualsAscii( ":", 2, 1 ) )
        {
            sURL.Erase( 0, 1 );
            sURL.SearchAndReplaceAll( '/', '\\' );
        }

        // n#261623 convert smb notation to '\\'
        const char pSmb[] = "smb://";
        if ( eProto == INET_PROT_SMB &&
             sURL.EqualsAscii( pSmb, 0, sizeof( pSmb ) - 1 ) )
        {
            sURL.Erase( 0, sizeof( pSmb ) - 3 );
            sURL.SearchAndReplaceAll( '/', '\\' );
        }

        m_rWW8Export.pDataStrm->Write( MAGIC_C, sizeof(MAGIC_C) );
        SwWW8Writer::WriteLong( *m_rWW8Export.pDataStrm, sURL.Len()+1 );
        SwWW8Writer::WriteString8( *m_rWW8Export.pDataStrm, sURL, true,
                                    RTL_TEXTENCODING_MS_1252 );
        m_rWW8Export.pDataStrm->Write( MAGIC_D, sizeof( MAGIC_D ) );

        SwWW8Writer::WriteLong( *m_rWW8Export.pDataStrm, 2*sURL.Len() + 6 );
        SwWW8Writer::WriteLong( *m_rWW8Export.pDataStrm, 2*sURL.Len() );
        SwWW8Writer::WriteShort( *m_rWW8Export.pDataStrm, 3 );
        SwWW8Writer::WriteString16( *m_rWW8Export.pDataStrm, sURL, false );
    }
    else if ( eProto != INET_PROT_NOT_VALID )
    {
        // version 2 (simple url)
        // an write some data to the data stream, but dont ask
        // what the data mean, except for the URL.
        // The First piece is the WW8_PIC structure.
        static sal_uInt8 MAGIC_B[] = {
            0xE0,0xC9,0xEA,0x79,0xF9,0xBA,0xCE,0x11,
            0x8C,0x82,0x00,0xAA,0x00,0x4B,0xA9,0x0B
        };

        m_rWW8Export.pDataStrm->Write( MAGIC_B, sizeof(MAGIC_B) );
        SwWW8Writer::WriteLong( *m_rWW8Export.pDataStrm, 2 * ( sURL.Len() + 1 ) );
        SwWW8Writer::WriteString16( *m_rWW8Export.pDataStrm, sURL, true );
    }

    if ( sMark.Len() )
    {
        SwWW8Writer::WriteLong( *m_rWW8Export.pDataStrm, sMark.Len()+1 );
        SwWW8Writer::WriteString16( *m_rWW8Export.pDataStrm, sMark, true );
    }
    SwWW8Writer::WriteLong( *m_rWW8Export.pDataStrm, nDataStt,
        m_rWW8Export.pDataStrm->Tell() - nDataStt );

    return true;
}

bool WW8AttributeOutput::EndURL()
{
    // hyperlinks only in WW8
    if ( !m_rWW8Export.bWrtWW8 )
        return false;

    m_rWW8Export.OutputField( 0, ww::eHYPERLINK, aEmptyStr, WRITEFIELD_CLOSE );

    return true;
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

void SwWW8AttrIter::OutSwFmtRefMark(const SwFmtRefMark& rAttr, bool)
{
    if ( m_rExport.HasRefToObject( REF_SETREFATTR, &rAttr.GetRefName(), 0 ) )
        m_rExport.AppendBookmark( m_rExport.GetBookmarkName( REF_SETREFATTR,
                                            &rAttr.GetRefName(), 0 ));
}

void WW8AttributeOutput::FieldVanish( const String& rTxt, ww::eField /*eType*/ )
{
    ww::bytes aItems;
    m_rWW8Export.GetCurrentItems( aItems );

    // sprmCFFldVanish
    if ( m_rWW8Export.bWrtWW8 )
        SwWW8Writer::InsUInt16( aItems, NS_sprm::LN_CFFldVanish );
    else
        aItems.push_back( 67 );
    aItems.push_back( 1 );

    sal_uInt16 nStt_sprmCFSpec = aItems.size();

    // sprmCFSpec --  fSpec-Attribut true
    if ( m_rWW8Export.bWrtWW8 )
        SwWW8Writer::InsUInt16( aItems, 0x855 );
    else
        aItems.push_back( 117 );
    aItems.push_back( 1 );

    m_rWW8Export.WriteChar( '\x13' );
    m_rWW8Export.pChpPlc->AppendFkpEntry( m_rWW8Export.Strm().Tell(), aItems.size(),
                                    aItems.data() );
    m_rWW8Export.OutSwString( rTxt, 0, rTxt.Len(), m_rWW8Export.IsUnicode(),
                        RTL_TEXTENCODING_MS_1252 );
    m_rWW8Export.pChpPlc->AppendFkpEntry( m_rWW8Export.Strm().Tell(), nStt_sprmCFSpec,
                                    aItems.data() );
    m_rWW8Export.WriteChar( '\x15' );
    m_rWW8Export.pChpPlc->AppendFkpEntry( m_rWW8Export.Strm().Tell(), aItems.size(),
                                    aItems.data() );
}

void AttributeOutputBase::TOXMark( const SwTxtNode& rNode, const SwTOXMark& rAttr )
{
    // its a field; so get the Text form the Node and build the field
    String sTxt;
    ww::eField eType = ww::eNONE;

    const SwTxtTOXMark& rTxtTOXMark = *rAttr.GetTxtTOXMark();
    const xub_StrLen* pTxtEnd = rTxtTOXMark.GetEnd();
    if ( pTxtEnd ) // has range?
    {
        sTxt = rNode.GetExpandTxt( *rTxtTOXMark.GetStart(),
                                   *pTxtEnd - *rTxtTOXMark.GetStart() );
    }
    else
        sTxt = rAttr.GetAlternativeText();

    switch ( rAttr.GetTOXType()->GetType() )
    {
        case TOX_INDEX:
            eType = ww::eXE;
            if ( !rAttr.GetPrimaryKey().isEmpty() )
            {
                if ( !rAttr.GetSecondaryKey().isEmpty() )
                {
                    sTxt.Insert( ':', 0 );
                    sTxt.Insert( rAttr.GetSecondaryKey(), 0 );
                }

                sTxt.Insert( ':', 0 );
                sTxt.Insert( rAttr.GetPrimaryKey(), 0 );
            }
            sTxt.InsertAscii( " XE \"", 0 );
            sTxt.InsertAscii( "\" " );
            break;

        case TOX_USER:
            ( sTxt.AppendAscii( "\" \\f \"" ) )
                += (sal_Char)( 'A' + GetExport( ).GetId( *rAttr.GetTOXType() ) );
            // fall through - no break;
        case TOX_CONTENT:
            {
                eType = ww::eTC;
                sTxt.InsertAscii( " TC \"", 0 );
                sal_uInt16 nLvl = rAttr.GetLevel();
                if (nLvl > WW8ListManager::nMaxLevel)
                    nLvl = WW8ListManager::nMaxLevel;

                ((sTxt.AppendAscii( "\" \\l " ))
                 += OUString::number( nLvl )) += ' ';
            }
            break;
        default:
            OSL_ENSURE( !this, "Unhandled option for toc export" );
            break;
    }

    if ( sTxt.Len() )
        FieldVanish( sTxt, eType );
}

int SwWW8AttrIter::OutAttrWithRange(xub_StrLen nPos)
{
    int nRet = 0;
    if ( const SwpHints* pTxtAttrs = rNd.GetpSwpHints() )
    {
        m_rExport.m_aCurrentCharPropStarts.push( nPos );
        const xub_StrLen* pEnd;
        for ( sal_uInt16 i = 0; i < pTxtAttrs->Count(); ++i )
        {
            const SwTxtAttr* pHt = (*pTxtAttrs)[i];
            const SfxPoolItem* pItem = &pHt->GetAttr();
            switch ( pItem->Which() )
            {
                case RES_TXTATR_INETFMT:
                    if ( nPos == *pHt->GetStart() )
                    {
                        const SwFmtINetFmt *rINet = static_cast< const SwFmtINetFmt* >( pItem );
                        if ( m_rExport.AttrOutput().StartURL( rINet->GetValue(), rINet->GetTargetFrame() ) )
                            ++nRet;
                    }
                    if ( 0 != ( pEnd = pHt->GetEnd() ) && nPos == *pEnd )
                    {
                        if ( m_rExport.AttrOutput().EndURL() )
                            --nRet;
                    }
                    break;
                case RES_TXTATR_REFMARK:
                    if ( nPos == *pHt->GetStart() )
                    {
                        OutSwFmtRefMark( *static_cast< const SwFmtRefMark* >( pItem ), true );
                        ++nRet;
                    }
                    if ( 0 != ( pEnd = pHt->GetEnd() ) && nPos == *pEnd )
                    {
                        OutSwFmtRefMark( *static_cast< const SwFmtRefMark* >( pItem ), false );
                        --nRet;
                    }
                    break;
                case RES_TXTATR_TOXMARK:
                    if ( nPos == *pHt->GetStart() )
                        m_rExport.AttrOutput().TOXMark( rNd, *static_cast< const SwTOXMark* >( pItem ) );
                    break;
                case RES_TXTATR_CJK_RUBY:
                    if ( nPos == *pHt->GetStart() )
                    {
                        m_rExport.AttrOutput().StartRuby( rNd, nPos, *static_cast< const SwFmtRuby* >( pItem ) );
                        ++nRet;
                    }
                    if ( 0 != ( pEnd = pHt->GetEnd() ) && nPos == *pEnd )
                    {
                        m_rExport.AttrOutput().EndRuby();
                        --nRet;
                    }
                    break;
            }
        }
        m_rExport.m_aCurrentCharPropStarts.pop(); // HasTextItem only allowed in the above range
    }
    return nRet;
}

bool SwWW8AttrIter::IsRedlineAtEnd( xub_StrLen nEnd ) const
{
    bool bRet = false;
    // search next Redline
    for( sal_uInt16 nPos = nCurRedlinePos;
        nPos < m_rExport.pDoc->GetRedlineTbl().size(); ++nPos )
    {
        const SwPosition* pEnd = m_rExport.pDoc->GetRedlineTbl()[ nPos ]->End();
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

const SwRedlineData* SwWW8AttrIter::GetRedline( xub_StrLen nPos )
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
        {
            // write data of current redline
            return &( pCurRedline->GetRedlineData() );
        }
    }

    if( !pCurRedline )
    {
        // search next Redline
        for( ; nCurRedlinePos < m_rExport.pDoc->GetRedlineTbl().size();
                ++nCurRedlinePos )
        {
            const SwRedline* pRedl = m_rExport.pDoc->GetRedlineTbl()[ nCurRedlinePos ];

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
                        return &( pCurRedline->GetRedlineData() );
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
    return NULL;
}


short MSWordExportBase::GetCurrentPageDirection() const
{
    const SwFrmFmt &rFmt = pAktPageDesc
                    ? pAktPageDesc->GetMaster()
                    : pDoc->GetPageDesc( 0 ).GetMaster();
    return rFmt.GetFrmDir().GetValue();
}

short MSWordExportBase::GetDefaultFrameDirection( ) const
{
    short nDir = FRMDIR_ENVIRONMENT;

    if ( bOutPageDescs )
        nDir = GetCurrentPageDirection(  );
    else if ( pOutFmtNode )
    {
        if ( bOutFlyFrmAttrs ) //frame
        {
            nDir = TrueFrameDirection( *( const SwFrmFmt * ) pOutFmtNode );
        }
        else if ( pOutFmtNode->ISA( SwCntntNode ) )    //pagagraph
        {
            const SwCntntNode *pNd = ( const SwCntntNode * ) pOutFmtNode;
            SwPosition aPos( *pNd );
            nDir = pDoc->GetTextDirection( aPos );
        }
        else if ( pOutFmtNode->ISA( SwTxtFmtColl ) )
            nDir = FRMDIR_HORI_LEFT_TOP;    //what else can we do :-(
    }

    if ( nDir == FRMDIR_ENVIRONMENT )
        nDir = FRMDIR_HORI_LEFT_TOP;        //Set something

    return nDir;
}

short MSWordExportBase::TrueFrameDirection( const SwFrmFmt &rFlyFmt ) const
{
    const SwFrmFmt *pFlyFmt = &rFlyFmt;
    const SvxFrameDirectionItem* pItem = 0;
    while ( pFlyFmt )
    {
        pItem = &pFlyFmt->GetFrmDir();
        if ( FRMDIR_ENVIRONMENT == pItem->GetValue() )
        {
            pItem = 0;
            const SwFmtAnchor* pAnchor = &pFlyFmt->GetAnchor();
            if ((FLY_AT_PAGE != pAnchor->GetAnchorId()) &&
                pAnchor->GetCntntAnchor() )
            {
                pFlyFmt = pAnchor->GetCntntAnchor()->nNode.GetNode().GetFlyFmt();
            }
            else
                pFlyFmt = 0;
        }
        else
            pFlyFmt = 0;
    }

    short nRet;
    if ( pItem )
        nRet = pItem->GetValue();
    else
        nRet = GetCurrentPageDirection();

    OSL_ENSURE( nRet != FRMDIR_ENVIRONMENT, "leaving with environment direction" );
    return nRet;
}

const SvxBrushItem* WW8Export::GetCurrentPageBgBrush() const
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

SvxBrushItem WW8Export::TrueFrameBgBrush(const SwFrmFmt &rFlyFmt) const
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
            if ((FLY_AT_PAGE != pAnchor->GetAnchorId()) &&
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

    const Color aTmpColor( COL_WHITE );
    SvxBrushItem aRet( aTmpColor, RES_BACKGROUND );
    if (pRet && (pRet->GetGraphic() ||( pRet->GetColor() != COL_TRANSPARENT)))
        aRet = *pRet;

    return aRet;
}


/*
Convert characters that need to be converted, the basic replacements and the
ridicously complicated title case attribute mapping to hardcoded upper case
because word doesn't have the feature
*/
String SwWW8AttrIter::GetSnippet(const String &rStr, xub_StrLen nAktPos,
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

    m_rExport.m_aCurrentCharPropStarts.push( nAktPos );
    const SfxPoolItem &rItem = GetItem(RES_CHRATR_CASEMAP);

    if (SVX_CASEMAP_TITEL == ((const SvxCaseMapItem&)rItem).GetValue())
    {
        sal_uInt16 nScriptType = i18n::ScriptType::LATIN;
        if (g_pBreakIt->GetBreakIter().is())
            nScriptType = g_pBreakIt->GetBreakIter()->getScriptType(aSnippet, 0);

        LanguageType nLanguage;
        switch (nScriptType)
        {
        case i18n::ScriptType::ASIAN:
                nLanguage = ((const SvxLanguageItem&)GetItem(RES_CHRATR_CJK_LANGUAGE)).GetLanguage();
                break;
        case i18n::ScriptType::COMPLEX:
                nLanguage = ((const SvxLanguageItem&)GetItem(RES_CHRATR_CTL_LANGUAGE)).GetLanguage();
                break;
        case i18n::ScriptType::LATIN:
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
        if (g_pBreakIt->GetBreakIter().is() && !g_pBreakIt->GetBreakIter()->isBeginWord(
            rStr, nAktPos, g_pBreakIt->GetLocale(nLanguage),
            i18n::WordType::ANYWORD_IGNOREWHITESPACES ) )
        {
            aSnippet.SetChar(0, rStr.GetChar(nAktPos));
        }
    }
    m_rExport.m_aCurrentCharPropStarts.pop();

    return aSnippet;
}

/** Delivers the right paragraph style

    Because of the different style handling for delete operations,
    the track changes have to be analysed. A deletion, starting in paragraph A
    with style A, ending in paragraph B with style B, needs a hack.
*/
static SwTxtFmtColl& lcl_getFormatCollection( MSWordExportBase& rExport, const SwTxtNode* pTxtNode )
{
    sal_uInt16 nPos = 0;
    sal_uInt16 nMax = rExport.pDoc->GetRedlineTbl().size();
    while( nPos < nMax )
    {
        const SwRedline* pRedl = rExport.pDoc->GetRedlineTbl()[ nPos++ ];
        const SwPosition* pStt = pRedl->Start();
        const SwPosition* pEnd = pStt == pRedl->GetPoint()
                                    ? pRedl->GetMark()
                                    : pRedl->GetPoint();
        // Looking for deletions, which ends in current pTxtNode
        if( nsRedlineType_t::REDLINE_DELETE == pRedl->GetRedlineData().GetType() &&
            pEnd->nNode == *pTxtNode && pStt->nNode != *pTxtNode &&
            pStt->nNode.GetNode().IsTxtNode() )
        {
            pTxtNode = pStt->nNode.GetNode().GetTxtNode();
            nMax = nPos;
            nPos = 0;
        }
    }
    return static_cast<SwTxtFmtColl&>( pTxtNode->GetAnyFmtColl() );
}

void WW8AttributeOutput::FormatDrop( const SwTxtNode& rNode, const SwFmtDrop &rSwFmtDrop, sal_uInt16 nStyle,
        ww8::WW8TableNodeInfo::Pointer_t pTextNodeInfo, ww8::WW8TableNodeInfoInner::Pointer_t pTextNodeInfoInner )
{
    short nDropLines = rSwFmtDrop.GetLines();
    short nDistance = rSwFmtDrop.GetDistance();
    int rFontHeight, rDropHeight, rDropDescent;

    SVBT16 nSty;
    ShortToSVBT16( nStyle, nSty );
    m_rWW8Export.pO->insert( m_rWW8Export.pO->end(), (sal_uInt8*)&nSty, (sal_uInt8*)&nSty+2 );     // Style #

    if ( m_rWW8Export.bWrtWW8 )
    {
        m_rWW8Export.InsUInt16( NS_sprm::LN_PPc );            // Alignment (sprmPPc)
        m_rWW8Export.pO->push_back( 0x20 );

        m_rWW8Export.InsUInt16( NS_sprm::LN_PWr );            // Wrapping (sprmPWr)
        m_rWW8Export.pO->push_back( 0x02 );

        m_rWW8Export.InsUInt16( NS_sprm::LN_PDcs );            // Dropcap (sprmPDcs)
        int nDCS = ( nDropLines << 3 ) | 0x01;
        m_rWW8Export.InsUInt16( static_cast< sal_uInt16 >( nDCS ) );

        m_rWW8Export.InsUInt16( NS_sprm::LN_PDxaFromText );            // Distance from text (sprmPDxaFromText)
        m_rWW8Export.InsUInt16( nDistance );

        if ( rNode.GetDropSize( rFontHeight, rDropHeight, rDropDescent ) )
        {
            m_rWW8Export.InsUInt16( NS_sprm::LN_PDyaLine );            // Line spacing
            m_rWW8Export.InsUInt16( static_cast< sal_uInt16 >( -rDropHeight ) );
            m_rWW8Export.InsUInt16( 0 );
        }
    }
    else
    {
        m_rWW8Export.pO->push_back( 29 );    // Alignment (sprmPPc)
        m_rWW8Export.pO->push_back( 0x20 );

        m_rWW8Export.pO->push_back( 37 );    // Wrapping (sprmPWr)
        m_rWW8Export.pO->push_back( 0x02 );

        m_rWW8Export.pO->push_back( 46 );    // Dropcap (sprmPDcs)
        int nDCS = ( nDropLines << 3 ) | 0x01;
        m_rWW8Export.InsUInt16( static_cast< sal_uInt16 >( nDCS ) );

        m_rWW8Export.pO->push_back( 49 );      // Distance from text (sprmPDxaFromText)
        m_rWW8Export.InsUInt16( nDistance );

        if (rNode.GetDropSize(rFontHeight, rDropHeight, rDropDescent))
        {
            m_rWW8Export.pO->push_back( 20 );  // Line spacing
            m_rWW8Export.InsUInt16( static_cast< sal_uInt16 >( -rDropHeight ) );
            m_rWW8Export.InsUInt16( 0 );
        }
    }

    m_rWW8Export.WriteCR( pTextNodeInfoInner );

    if ( pTextNodeInfo.get() != NULL )
    {
#ifdef DBG_UTIL
        SAL_INFO( "sw.ww8", pTextNodeInfo->toString());
#endif
        TableInfoCell( pTextNodeInfoInner );
    }

    m_rWW8Export.pPapPlc->AppendFkpEntry( m_rWW8Export.Strm().Tell(), m_rWW8Export.pO->size(), m_rWW8Export.pO->data() );
    m_rWW8Export.pO->clear();

    if ( rNode.GetDropSize( rFontHeight, rDropHeight, rDropDescent ) )
    {
        if ( m_rWW8Export.bWrtWW8 )
        {
            const SwCharFmt *pSwCharFmt = rSwFmtDrop.GetCharFmt();
            if ( pSwCharFmt )
            {
                m_rWW8Export.InsUInt16( NS_sprm::LN_CIstd );
                m_rWW8Export.InsUInt16( m_rWW8Export.GetId( *pSwCharFmt ) );
            }

            m_rWW8Export.InsUInt16( NS_sprm::LN_CHpsPos );            // Lower the chars
            m_rWW8Export.InsUInt16( static_cast< sal_uInt16 >( -((nDropLines - 1)*rDropDescent) / 10 ) );

            m_rWW8Export.InsUInt16( NS_sprm::LN_CHps );            // Font Size
            m_rWW8Export.InsUInt16( static_cast< sal_uInt16 >( rFontHeight / 10 ) );
        }
        else
        {
            const SwCharFmt *pSwCharFmt = rSwFmtDrop.GetCharFmt();
            if ( pSwCharFmt )
            {
                m_rWW8Export.InsUInt16( 80 );
                m_rWW8Export.InsUInt16( m_rWW8Export.GetId( *pSwCharFmt ) );
            }

            m_rWW8Export.pO->push_back( 101 );      // Lower the chars
            m_rWW8Export.InsUInt16( static_cast< sal_uInt16 >( -((nDropLines - 1)*rDropDescent) / 10 ) );

            m_rWW8Export.pO->push_back( 99 );      // Font Size
            m_rWW8Export.InsUInt16( static_cast< sal_uInt16 >( rFontHeight / 10 ) );
        }
    }

    m_rWW8Export.pChpPlc->AppendFkpEntry( m_rWW8Export.Strm().Tell(), m_rWW8Export.pO->size(), m_rWW8Export.pO->data() );
    m_rWW8Export.pO->clear();
}

xub_StrLen MSWordExportBase::GetNextPos( SwWW8AttrIter* aAttrIter, const SwTxtNode& rNode, xub_StrLen nAktPos  )
{
    // Get the bookmarks for the normal run
    xub_StrLen nNextPos = aAttrIter->WhereNext();
    xub_StrLen nNextBookmark = nNextPos;

    if( nNextBookmark > nAktPos ) //no need to search for bookmarks otherwise (checked in UpdatePosition())
    {
        GetSortedBookmarks( rNode, nAktPos, nNextBookmark - nAktPos );
        NearestBookmark( nNextBookmark, nAktPos, false );
    }
    return std::min( nNextPos, nNextBookmark );
}

void MSWordExportBase::UpdatePosition( SwWW8AttrIter* aAttrIter, xub_StrLen nAktPos, xub_StrLen /*nEnd*/ )
{
    xub_StrLen nNextPos;

    // go to next attribute if no bookmark is found or if the bookmark is behind the next attribute position
    // It may happened that the WhereNext() wasn't used in the previous increment because there was a
    // bookmark before it. Use that position before trying to find another one.
    bool bNextBookmark = NearestBookmark( nNextPos, nAktPos, true );
    if( nAktPos == aAttrIter->WhereNext() && ( !bNextBookmark || nNextPos > aAttrIter->WhereNext() ) )
        aAttrIter->NextPos();
}

bool MSWordExportBase::GetBookmarks( const SwTxtNode& rNd, xub_StrLen nStt,
                    xub_StrLen nEnd, IMarkVector& rArr )
{
    IDocumentMarkAccess* const pMarkAccess = pDoc->getIDocumentMarkAccess();
    sal_uLong nNd = rNd.GetIndex( );

    const sal_Int32 nMarks = pMarkAccess->getMarksCount();
    for ( sal_Int32 i = 0; i < nMarks; i++ )
    {
        IMark* pMark = ( pMarkAccess->getMarksBegin() + i )->get();

        // Only keep the bookmarks starting or ending in this node
        if ( pMark->GetMarkStart().nNode == nNd ||
             pMark->GetMarkEnd().nNode == nNd )
        {
            xub_StrLen nBStart = pMark->GetMarkStart().nContent.GetIndex();
            xub_StrLen nBEnd = pMark->GetMarkEnd().nContent.GetIndex();

            // Keep only the bookmars starting or ending in the snippet
            bool bIsStartOk = ( pMark->GetMarkStart().nNode == nNd ) && ( nBStart >= nStt ) && ( nBStart <= nEnd );
            bool bIsEndOk = ( pMark->GetMarkEnd().nNode == nNd ) && ( nBEnd >= nStt ) && ( nBEnd <= nEnd );

            IFieldmark* pFieldmark = dynamic_cast<IFieldmark*>(pMark);
            if (pFieldmark && pFieldmark->GetFieldname() == ODF_COMMENTRANGE)
                continue;

            if ( bIsStartOk || bIsEndOk )
                rArr.push_back( pMark );
        }
    }
    return ( rArr.size() > 0 );
}

class CompareMarksEnd : public std::binary_function < const IMark *, const IMark *, bool >
{
public:
    inline bool operator() ( const IMark * pOneB, const IMark * pTwoB ) const
    {
        xub_StrLen nOEnd = pOneB->GetMarkEnd().nContent.GetIndex();
        xub_StrLen nTEnd = pTwoB->GetMarkEnd().nContent.GetIndex();

        return nOEnd < nTEnd;
    }
};

bool MSWordExportBase::NearestBookmark( xub_StrLen& rNearest, const xub_StrLen nAktPos, bool bNextPositionOnly )
{
    bool bHasBookmark = false;

    if ( !m_rSortedMarksStart.empty() )
    {
        IMark* pMarkStart = m_rSortedMarksStart.front();
        xub_StrLen nNext = pMarkStart->GetMarkStart().nContent.GetIndex();
        if( !bNextPositionOnly || (nNext > nAktPos ))
        {
            rNearest = nNext;
            bHasBookmark = true;
        }
    }

    if ( !m_rSortedMarksEnd.empty() )
    {
        IMark* pMarkEnd = m_rSortedMarksEnd[0];
        xub_StrLen nNext = pMarkEnd->GetMarkEnd().nContent.GetIndex();
        if( !bNextPositionOnly || nNext > nAktPos )
        {
            if ( !bHasBookmark )
                rNearest = nNext;
            else
                rNearest = std::min( rNearest, nNext );
            bHasBookmark = true;
        }
    }

    return bHasBookmark;
}

void MSWordExportBase::GetSortedBookmarks( const SwTxtNode& rNode, xub_StrLen nAktPos, xub_StrLen nLen )
{
    IMarkVector aMarksStart;
    if ( GetBookmarks( rNode, nAktPos, nAktPos + nLen, aMarksStart ) )
    {
        IMarkVector aSortedEnd;
        IMarkVector aSortedStart;
        for ( IMarkVector::const_iterator it = aMarksStart.begin(), end = aMarksStart.end();
              it != end; ++it )
        {
            IMark* pMark = (*it);

            // Remove the positions egals to the current pos
            xub_StrLen nStart = pMark->GetMarkStart().nContent.GetIndex();
            xub_StrLen nEnd = pMark->GetMarkEnd().nContent.GetIndex();

            if ( nStart > nAktPos && ( pMark->GetMarkStart().nNode == rNode.GetIndex()) )
                aSortedStart.push_back( pMark );

            if ( nEnd > nAktPos && nEnd <= ( nAktPos + nLen ) && (pMark->GetMarkEnd().nNode == rNode.GetIndex()) )
                aSortedEnd.push_back( pMark );
        }

        // Sort the bookmarks by end position
        std::sort( aSortedEnd.begin(), aSortedEnd.end(), CompareMarksEnd() );

        m_rSortedMarksStart.swap( aSortedStart );
        m_rSortedMarksEnd.swap( aSortedEnd );
    }
    else
    {
        m_rSortedMarksStart.clear( );
        m_rSortedMarksEnd.clear( );
    }
}

void MSWordExportBase::OutputTextNode( const SwTxtNode& rNode )
{
    SAL_INFO( "sw.ww8", "<OutWW8_SwTxtNode>" );

    ww8::WW8TableNodeInfo::Pointer_t pTextNodeInfo( mpTableInfo->getTableNodeInfo( &rNode ) );

    //For i120928,identify the last node
    bool bLastCR = false;
    bool bExported = false;
    {
        SwNodeIndex aNextIdx(rNode,1);
        SwNodeIndex aLastIdx(rNode.GetNodes().GetEndOfContent());
        if (aNextIdx == aLastIdx)
            bLastCR = true;
    }

    AttrOutput().StartParagraph( pTextNodeInfo );

    bool bFlyInTable = mpParentFrame && IsInTable();

    if ( !bFlyInTable )
        nStyleBeforeFly = GetId( lcl_getFormatCollection( *this, &rNode ) );

    // nStyleBeforeFly may change when we recurse into another node, so we
    // have to remember it in nStyle
    sal_uInt16 nStyle = nStyleBeforeFly;

    SwWW8AttrIter aAttrIter( *this, rNode );
    rtl_TextEncoding eChrSet = aAttrIter.GetCharSet();

    if ( bStartTOX )
    {
        // ignore TOX header section
        const SwSectionNode* pSectNd = rNode.FindSectionNode();
        if ( pSectNd && TOX_CONTENT_SECTION == pSectNd->GetSection().GetType() )
        {
            AttrOutput().StartTOX( pSectNd->GetSection() );
            m_aCurrentCharPropStarts.push( 0 );
        }
    }

    const SwSection* pTOXSect = 0;
    if( bInWriteTOX )
    {
        // check for end of TOX
        SwNodeIndex aIdx( rNode, 1 );
        if( !aIdx.GetNode().IsTxtNode() )
        {
            const SwSectionNode* pTOXSectNd = rNode.FindSectionNode();
            if ( pTOXSectNd )
            {
                pTOXSect = &pTOXSectNd->GetSection();

                const SwNode* pNxt = rNode.GetNodes().GoNext( &aIdx );
                if( pNxt && pNxt->FindSectionNode() == pTOXSectNd )
                    pTOXSect = 0;
            }
        }
    }

    if ( aAttrIter.RequiresImplicitBookmark() )
    {
        String sBkmkName = String(  "_toc"  );
        sBkmkName += OUString::number( rNode.GetIndex() );
        AppendWordBookmark( sBkmkName );
    }

    OUString aStr( rNode.GetTxt() );

    sal_Int32 nAktPos = 0;
    sal_Int32 const nEnd = aStr.getLength();
    bool bRedlineAtEnd = false;
    int nOpenAttrWithRange = 0;

    ww8::WW8TableNodeInfoInner::Pointer_t pTextNodeInfoInner;
    if ( pTextNodeInfo.get() != NULL )
        pTextNodeInfoInner = pTextNodeInfo->getFirstInner();

    do {
        const SwRedlineData* pRedlineData = aAttrIter.GetRedline( nAktPos );

        xub_StrLen nNextAttr = GetNextPos( &aAttrIter, rNode, nAktPos );
        // Is this the only run in this paragraph and it's empty?
        bool bSingleEmptyRun = nAktPos == 0 && nNextAttr == 0;
        AttrOutput().StartRun( pRedlineData, bSingleEmptyRun );
        if( nTxtTyp == TXT_FTN || nTxtTyp == TXT_EDN )
            AttrOutput().FootnoteEndnoteRefTag();

        if( nNextAttr > nEnd )
            nNextAttr = nEnd;

        aAttrIter.OutFlys( nAktPos );
        // Append bookmarks in this range after flys, exclusive of final
        // position of this range
        AppendBookmarks( rNode, nAktPos, nNextAttr - nAktPos );
        bool bTxtAtr = aAttrIter.IsTxtAttr( nAktPos );
        nOpenAttrWithRange += aAttrIter.OutAttrWithRange(nAktPos);

        xub_StrLen nLen = nNextAttr - nAktPos;
        if ( !bTxtAtr && nLen )
        {
            sal_Unicode ch = aStr[nAktPos];
            int ofs = ( ch == CH_TXT_ATR_FIELDSTART || ch == CH_TXT_ATR_FIELDEND || ch == CH_TXT_ATR_FORMELEMENT? 1: 0 );

            IDocumentMarkAccess* const pMarkAccess = pDoc->getIDocumentMarkAccess();
            if ( ch == CH_TXT_ATR_FIELDSTART )
            {
                SwPosition aPosition( rNode, SwIndex( const_cast< SwTxtNode* >( &rNode ), nAktPos ) );
                ::sw::mark::IFieldmark const * const pFieldmark = pMarkAccess->getFieldmarkFor( aPosition );
                OSL_ENSURE( pFieldmark, "Looks like this doc is broken...; where is the Fieldmark for the FIELDSTART??" );

                if ( pFieldmark && pFieldmark->GetFieldname() == ODF_FORMTEXT )
                    AppendBookmark( pFieldmark->GetName(), false );
                ww::eField eFieldId = lcl_getFieldId( pFieldmark );
                OUString sCode = lcl_getFieldCode( pFieldmark );
                if ( pFieldmark && pFieldmark->GetFieldname() == ODF_UNHANDLED )
                {
                    IFieldmark::parameter_map_t::const_iterator it = pFieldmark->GetParameters()->find( ODF_ID_PARAM );
                    if ( it != pFieldmark->GetParameters()->end() )
                    {
                        OUString sFieldId;
                        it->second >>= sFieldId;
                        eFieldId = (ww::eField)sFieldId.toInt32();
                    }

                    it = pFieldmark->GetParameters()->find( ODF_CODE_PARAM );
                    if ( it != pFieldmark->GetParameters()->end() )
                    {
                        it->second >>= sCode;
                    }
                }

                bool bCommentRange = pFieldmark && pFieldmark->GetFieldname() == ODF_COMMENTRANGE;
                if (bCommentRange)
                    AttrOutput().WritePostitFieldStart();
                else
                    OutputField( NULL, eFieldId, sCode, WRITEFIELD_START | WRITEFIELD_CMD_START );

                if ( pFieldmark && pFieldmark->GetFieldname( ) == ODF_FORMTEXT )
                    WriteFormData( *pFieldmark );
                else if ( pFieldmark && pFieldmark->GetFieldname( ) == ODF_HYPERLINK )
                    WriteHyperlinkData( *pFieldmark );
                if (!bCommentRange)
                    OutputField( NULL, lcl_getFieldId( pFieldmark ), String(), WRITEFIELD_CMD_END );

                if ( pFieldmark && pFieldmark->GetFieldname() == ODF_UNHANDLED )
                {
                    // Check for the presence of a linked OLE object
                    IFieldmark::parameter_map_t::const_iterator it = pFieldmark->GetParameters()->find( ODF_OLE_PARAM );
                    if ( it != pFieldmark->GetParameters()->end() )
                    {
                        OUString sOleId;
                        uno::Any aValue = it->second;
                        aValue >>= sOleId;
                        if ( !sOleId.isEmpty() )
                            OutputLinkedOLE( sOleId );
                    }
                }
            }
            else if ( ch == CH_TXT_ATR_FIELDEND )
            {
                SwPosition aPosition( rNode, SwIndex( const_cast< SwTxtNode* >( &rNode ), nAktPos ) );
                ::sw::mark::IFieldmark const * const pFieldmark = pMarkAccess->getFieldmarkFor( aPosition );

                OSL_ENSURE( pFieldmark, "Looks like this doc is broken...; where is the Fieldmark for the FIELDEND??" );

                ww::eField eFieldId = lcl_getFieldId( pFieldmark );
                if ( pFieldmark && pFieldmark->GetFieldname() == ODF_UNHANDLED )
                {
                    IFieldmark::parameter_map_t::const_iterator it = pFieldmark->GetParameters()->find( ODF_ID_PARAM );
                    if ( it != pFieldmark->GetParameters()->end() )
                    {
                        OUString sFieldId;
                        it->second >>= sFieldId;
                        eFieldId = (ww::eField)sFieldId.toInt32();
                    }
                }

                if (pFieldmark && pFieldmark->GetFieldname() == ODF_COMMENTRANGE)
                    AttrOutput().WritePostitFieldEnd();
                else
                    OutputField( NULL, eFieldId, String(), WRITEFIELD_CLOSE );

                if ( pFieldmark && pFieldmark->GetFieldname() == ODF_FORMTEXT )
                    AppendBookmark( pFieldmark->GetName(), false );
            }
            else if ( ch == CH_TXT_ATR_FORMELEMENT )
            {
                SwPosition aPosition( rNode, SwIndex( const_cast< SwTxtNode* >( &rNode ), nAktPos ) );
                ::sw::mark::IFieldmark const * const pFieldmark = pMarkAccess->getFieldmarkFor( aPosition );
                OSL_ENSURE( pFieldmark, "Looks like this doc is broken...; where is the Fieldmark for the FIELDSTART??" );

                bool isDropdownOrCheckbox = pFieldmark && (pFieldmark->GetFieldname( ) == ODF_FORMDROPDOWN ||
                    pFieldmark->GetFieldname( ) == ODF_FORMCHECKBOX );

                if ( isDropdownOrCheckbox )
                    AppendBookmark( pFieldmark->GetName(), 0 );
                OutputField( NULL, lcl_getFieldId( pFieldmark ),
                        lcl_getFieldCode( pFieldmark ),
                        WRITEFIELD_START | WRITEFIELD_CMD_START );
                if ( isDropdownOrCheckbox )
                    WriteFormData( *pFieldmark );
                OutputField( NULL, lcl_getFieldId( pFieldmark ), String(), WRITEFIELD_CLOSE );
                if ( isDropdownOrCheckbox )
                    AppendBookmark( pFieldmark->GetName(), false );
            }
            nLen -= static_cast< sal_Int32 >( ofs );

            String aSnippet( aAttrIter.GetSnippet( aStr, nAktPos + static_cast< sal_Int32 >( ofs ), nLen ) );
            if ( ( nTxtTyp == TXT_EDN || nTxtTyp == TXT_FTN ) && nAktPos == 0 && nLen > 0 )
            {
                // Insert tab for aesthetic puposes #i24762#
                if ( aSnippet.GetChar( 0 ) != 0x09 )
                    aSnippet.Insert( 0x09, 0 );
            }
            AttrOutput().RunText( aSnippet, eChrSet );
        }

        if ( aAttrIter.IsDropCap( nNextAttr ) )
            AttrOutput().FormatDrop( rNode, aAttrIter.GetSwFmtDrop(), nStyle, pTextNodeInfo, pTextNodeInfoInner );

        if (0 != nEnd)
        {
            // Output the character attributes
            // #i51277# do this before writing flys at end of paragraph
            AttrOutput().StartRunProperties();
            aAttrIter.OutAttr( nAktPos );
            AttrOutput().EndRunProperties( pRedlineData );
        }

        // At the end of line, output the attributes until the CR.
        // Exception: footnotes at the end of line
        if ( nNextAttr == nEnd )
        {
            OSL_ENSURE( nOpenAttrWithRange >= 0, "odd to see this happening, expected >= 0" );
            if ( !bTxtAtr && nOpenAttrWithRange <= 0 )
            {
                if ( aAttrIter.IsRedlineAtEnd( nEnd ) )
                    bRedlineAtEnd = true;
                else
                {
                    // insert final graphic anchors if any before CR
                    aAttrIter.OutFlys( nEnd );
                    // insert final bookmarks if any before CR and after flys
                    AppendBookmarks( rNode, nEnd, 1 );
                    if ( pTOXSect )
                    {
                        m_aCurrentCharPropStarts.pop();
                        AttrOutput().EndTOX( *pTOXSect );
                    }
                    //For i120928,the position of the bullet's graphic is at end of doc
                    if (bLastCR && (!bExported))
                    {
                        ExportGrfBullet(rNode);
                        bExported = true;
                    }

                    WriteCR( pTextNodeInfoInner );
                }
            }
        }

        if (0 == nEnd)
        {
            // Output the character attributes
            // do it after WriteCR for an empty paragraph (otherwise
            // WW8_WrFkp::Append throws SPRMs away...)
            AttrOutput().StartRunProperties();
            aAttrIter.OutAttr( nAktPos );
            AttrOutput().EndRunProperties( pRedlineData );
        }

        // Exception: footnotes at the end of line
        if ( nNextAttr == nEnd )
        {
            OSL_ENSURE(nOpenAttrWithRange >= 0,
                "odd to see this happening, expected >= 0");
            bool bAttrWithRange = (nOpenAttrWithRange > 0);
            if ( nAktPos != nEnd )
            {
                nOpenAttrWithRange += aAttrIter.OutAttrWithRange(nEnd);
                OSL_ENSURE(nOpenAttrWithRange == 0,
                    "odd to see this happening, expected 0");
            }

            AttrOutput().OutputFKP();

            if ( bTxtAtr || bAttrWithRange || bRedlineAtEnd )
            {
                // insert final graphic anchors if any before CR
                aAttrIter.OutFlys( nEnd );
                // insert final bookmarks if any before CR and after flys
                AppendBookmarks( rNode, nEnd, 1 );
                // #i120928 - position of the bullet's graphic is at end of doc
                if (bLastCR && (!bExported))
                {
                    ExportGrfBullet(rNode);
                    bExported = true;
                }

                if ( pTOXSect )
                {
                    m_aCurrentCharPropStarts.pop();
                    AttrOutput().EndTOX( *pTOXSect );
                }

                WriteCR( pTextNodeInfoInner );

                if ( bRedlineAtEnd )
                {
                    AttrOutput().Redline( aAttrIter.GetRedline( nEnd ) );
                    AttrOutput().OutputFKP();
                }
            }
        }

        AttrOutput().WritePostitFieldReference();

        AttrOutput().EndRun();

        nAktPos = nNextAttr;
        UpdatePosition( &aAttrIter, nAktPos, nEnd );
        eChrSet = aAttrIter.GetCharSet();
    }
    while ( nAktPos < nEnd );

    AttrOutput().StartParagraphProperties( rNode );

    AttrOutput().ParagraphStyle( nStyle );

    if ( mpParentFrame && IsInTable() )    // Fly-Attrs
        OutputFormat( mpParentFrame->GetFrmFmt(), false, false, true );

    if ( pTextNodeInfo.get() != NULL )
    {
#ifdef DBG_UTIL
        SAL_INFO( "sw.ww8", pTextNodeInfo->toString());
#endif

        AttrOutput().TableInfoCell( pTextNodeInfoInner );
        if (pTextNodeInfoInner->isFirstInTable())
        {
            const SwTable * pTable = pTextNodeInfoInner->getTable();

            const SwTableFmt * pTabFmt = pTable->GetTableFmt();
            if (pTabFmt != NULL)
            {
                if (pTabFmt->GetBreak().GetBreak() == SVX_BREAK_PAGE_BEFORE)
                    AttrOutput().PageBreakBefore(true);
            }
        }
    }

    if ( !bFlyInTable )
    {
        SfxItemSet* pTmpSet = 0;
        const sal_uInt8 nPrvNxtNd = rNode.HasPrevNextLayNode();

        if( (ND_HAS_PREV_LAYNODE|ND_HAS_NEXT_LAYNODE ) != nPrvNxtNd )
        {
            const SfxPoolItem* pItem;
            if( SFX_ITEM_SET == rNode.GetSwAttrSet().GetItemState(
                    RES_UL_SPACE, true, &pItem ) &&
                ( ( !( ND_HAS_PREV_LAYNODE & nPrvNxtNd ) &&
                   ((SvxULSpaceItem*)pItem)->GetUpper()) ||
                  ( !( ND_HAS_NEXT_LAYNODE & nPrvNxtNd ) &&
                   ((SvxULSpaceItem*)pItem)->GetLower()) ))
            {
                pTmpSet = new SfxItemSet( rNode.GetSwAttrSet() );
                SvxULSpaceItem aUL( *(SvxULSpaceItem*)pItem );
                // #i25901#- consider compatibility option
                if (!pDoc->get(IDocumentSettingAccess::PARA_SPACE_MAX_AT_PAGES))
                {
                    if( !(ND_HAS_PREV_LAYNODE & nPrvNxtNd ))
                        aUL.SetUpper( 0 );
                }
                // #i25901# - consider compatibility option
                if (!pDoc->get(IDocumentSettingAccess::ADD_PARA_SPACING_TO_TABLE_CELLS))
                {
                    if( !(ND_HAS_NEXT_LAYNODE & nPrvNxtNd ))
                        aUL.SetLower( 0 );
                }
                pTmpSet->Put( aUL );
            }
        }

        bool bParaRTL = false;
        const SvxFrameDirectionItem* pItem = (const SvxFrameDirectionItem*)
            rNode.GetSwAttrSet().GetItem(RES_FRAMEDIR);
        if ( aAttrIter.IsParaRTL())
            bParaRTL = true;

        if( rNode.IsNumbered())
        {
            const SwNumRule* pRule = rNode.GetNumRule();
            sal_uInt8 nLvl = static_cast< sal_uInt8 >( rNode.GetActualListLevel() );
            const SwNumFmt* pFmt = pRule->GetNumFmt( nLvl );
            if( !pFmt )
                pFmt = &pRule->Get( nLvl );

            if( !pTmpSet )
                pTmpSet = new SfxItemSet( rNode.GetSwAttrSet() );

            SvxLRSpaceItem aLR(ItemGet<SvxLRSpaceItem>(*pTmpSet, RES_LR_SPACE));
            // #i86652#
            if ( pFmt->GetPositionAndSpaceMode() ==
                                    SvxNumberFormat::LABEL_WIDTH_AND_POSITION )
            {
                aLR.SetTxtLeft( aLR.GetTxtLeft() + pFmt->GetAbsLSpace() );
            }

            if( rNode.IsNumbered() && rNode.IsCountedInList() )
            {
                // #i86652#
                if ( pFmt->GetPositionAndSpaceMode() ==
                                        SvxNumberFormat::LABEL_WIDTH_AND_POSITION )
                {
                    if (bParaRTL)
                        aLR.SetTxtFirstLineOfstValue(pFmt->GetAbsLSpace() - pFmt->GetFirstLineOffset());
                    else
                        aLR.SetTxtFirstLineOfst(GetWordFirstLineOffset(*pFmt));
                }

                // correct fix for issue i94187
                if (SFX_ITEM_SET !=
                    pTmpSet->GetItemState(RES_PARATR_NUMRULE, false) )
                {
                    // List style set via paragraph style - then put it into the itemset.
                    // This is needed to get list level and list id exported for
                    // the paragraph.
                    pTmpSet->Put( SwNumRuleItem( pRule->GetName() ));

                    // Put indent values into the itemset in case that the list
                    // style is applied via paragraph style and the list level
                    // indent values are not applicable.
                    if ( pFmt->GetPositionAndSpaceMode() ==
                                            SvxNumberFormat::LABEL_ALIGNMENT &&
                         !rNode.AreListLevelIndentsApplicable() )
                    {
                        pTmpSet->Put( aLR );
                    }
                }
            }
            else
                pTmpSet->ClearItem(RES_PARATR_NUMRULE);

            // #i86652#
            if ( pFmt->GetPositionAndSpaceMode() ==
                                    SvxNumberFormat::LABEL_WIDTH_AND_POSITION )
            {
                pTmpSet->Put(aLR);

                //#i21847#
                SvxTabStopItem aItem(
                    ItemGet<SvxTabStopItem>(*pTmpSet, RES_PARATR_TABSTOP));
                SvxTabStop aTabStop(pFmt->GetAbsLSpace());
                aItem.Insert(aTabStop);
                pTmpSet->Put(aItem);

                MSWordExportBase::CorrectTabStopInSet(*pTmpSet, pFmt->GetAbsLSpace());
            }
        }

        /*
        If a given para is using the FRMDIR_ENVIRONMENT direction we
        cannot export that, its its ltr then that's ok as thats word's
        default. Otherwise we must add a RTL attribute to our export list
        */
        pItem = (const SvxFrameDirectionItem*)
            rNode.GetSwAttrSet().GetItem(RES_FRAMEDIR);
        if (
            (!pItem || pItem->GetValue() == FRMDIR_ENVIRONMENT) &&
            aAttrIter.IsParaRTL()
           )
        {
            if ( !pTmpSet )
                pTmpSet = new SfxItemSet(rNode.GetSwAttrSet());

            pTmpSet->Put(SvxFrameDirectionItem(FRMDIR_HORI_RIGHT_TOP, RES_FRAMEDIR));
        }
        // move code for handling of numbered,
        // but not counted paragraphs to this place. Otherwise, the paragraph
        // isn't exported as numbered, but not counted, if no other attribute
        // is found in <pTmpSet>
        // #i44815# adjust numbering/indents for numbered paragraphs
        //          without number (NO_NUMLEVEL)
        // #i47013# need to check rNode.GetNumRule()!=NULL as well.
        if ( ! rNode.IsCountedInList() && rNode.GetNumRule()!=NULL )
        {
            // WW8 does not know numbered paragraphs without number
            // (NO_NUMLEVEL). In WW8AttributeOutput::ParaNumRule(), we will export
            // the RES_PARATR_NUMRULE as list-id 0, which in WW8 means
            // no numbering. Here, we will adjust the indents to match
            // visually.

            if ( !pTmpSet )
                pTmpSet = new SfxItemSet(rNode.GetSwAttrSet());

            // create new LRSpace item, based on the current (if present)
            const SfxPoolItem* pPoolItem = NULL;
            pTmpSet->GetItemState(RES_LR_SPACE, sal_True, &pPoolItem);
            SvxLRSpaceItem aLRSpace(
                ( pPoolItem == NULL )
                    ? SvxLRSpaceItem(0, 0, 0, 0, RES_LR_SPACE)
                    : *static_cast<const SvxLRSpaceItem*>( pPoolItem ) );

            // new left margin = old left + label space
            const SwNumRule* pRule = rNode.GetNumRule();
            const SwNumFmt& rNumFmt = pRule->Get( static_cast< sal_uInt16 >(rNode.GetActualListLevel()) );
            // #i86652#
            if ( rNumFmt.GetPositionAndSpaceMode() ==
                                    SvxNumberFormat::LABEL_WIDTH_AND_POSITION )
            {
                aLRSpace.SetTxtLeft( aLRSpace.GetLeft() + rNumFmt.GetAbsLSpace() );

                // new first line indent = 0
                // (first line indent is ignored for NO_NUMLEVEL)
                if (!bParaRTL)
                    aLRSpace.SetTxtFirstLineOfst( 0 );

                // put back the new item
                pTmpSet->Put( aLRSpace );
            }

            // assure that numbering rule is in <pTmpSet>
            if (SFX_ITEM_SET != pTmpSet->GetItemState(RES_PARATR_NUMRULE, false) )
            {
                pTmpSet->Put( SwNumRuleItem( pRule->GetName() ));
            }
        }

        // #i75457#
        // Export page break after attribute from paragraph style.
        // If page break attribute at the text node exist, an existing page
        // break after at the paragraph style hasn't got to be considered.
        if ( !rNode.GetpSwAttrSet() ||
             SFX_ITEM_SET != rNode.GetpSwAttrSet()->GetItemState(RES_BREAK, false) )
        {
            const SvxFmtBreakItem* pBreakAtParaStyle =
                &(ItemGet<SvxFmtBreakItem>(rNode.GetSwAttrSet(), RES_BREAK));
            if ( pBreakAtParaStyle &&
                 pBreakAtParaStyle->GetBreak() == SVX_BREAK_PAGE_AFTER )
            {
                if ( !pTmpSet )
                {
                    pTmpSet = new SfxItemSet(rNode.GetSwAttrSet());
                }
                pTmpSet->Put( *pBreakAtParaStyle );
            }
            else if( pTmpSet )
            {   // Even a pagedesc item is set, the break item can be set 'NONE',
                // this has to be overruled.
                const SwFmtPageDesc& rPageDescAtParaStyle =
                    ItemGet<SwFmtPageDesc>( rNode, RES_PAGEDESC );
                if( rPageDescAtParaStyle.KnowsPageDesc() )
                    pTmpSet->ClearItem( RES_BREAK );
            }
        }

        // #i76520# Emulate non-splitting tables
        if ( bOutTable )
        {
            const SwTableNode* pTableNode = rNode.FindTableNode();

            if ( pTableNode )
            {
                const SwTable& rTable = pTableNode->GetTable();
                const SvxFmtKeepItem& rKeep = rTable.GetFrmFmt()->GetKeep();
                const bool bKeep = rKeep.GetValue();
                const bool bDontSplit = !bKeep ?
                                        !rTable.GetFrmFmt()->GetLayoutSplit().GetValue() :
                                        false;

                if ( bKeep || bDontSplit )
                {
                    // bKeep: set keep at first paragraphs in all lines
                    // bDontSplit : set keep at first paragraphs in all lines except from last line
                    // but only for non-complex tables
                    const SwTableBox* pBox = rNode.GetTblBox();
                    const SwTableLine* pLine = pBox ? pBox->GetUpper() : 0;

                    if ( pLine && !pLine->GetUpper() )
                    {
                        // check if box is first in that line:
                        if ( 0 == pLine->GetTabBoxes().GetPos( pBox ) && pBox->GetSttNd() )
                        {
                            // check if paragraph is first in that line:
                            if ( 1 == ( rNode.GetIndex() - pBox->GetSttNd()->GetIndex() ) )
                            {
                                bool bSetAtPara = false;
                                if ( bKeep )
                                    bSetAtPara = true;
                                else if ( bDontSplit )
                                {
                                    // check if pLine isn't last line in table
                                    if ( rTable.GetTabLines().size() - rTable.GetTabLines().GetPos( pLine ) != 1 )
                                        bSetAtPara = true;
                                }

                                if ( bSetAtPara )
                                {
                                    if ( !pTmpSet )
                                        pTmpSet = new SfxItemSet(rNode.GetSwAttrSet());

                                    const SvxFmtKeepItem aKeepItem( sal_True, RES_KEEP );
                                    pTmpSet->Put( aKeepItem );
                                }
                            }
                        }
                    }
                }
            }
        }

        const SfxItemSet* pNewSet = pTmpSet ? pTmpSet : rNode.GetpSwAttrSet();
        if( pNewSet )
        {                                               // Para-Attrs
            pStyAttr = &rNode.GetAnyFmtColl().GetAttrSet();

            const SwModify* pOldMod = pOutFmtNode;
            pOutFmtNode = &rNode;

            // Pap-Attrs, so script is not necessary
            OutputItemSet( *pNewSet, true, false, i18n::ScriptType::LATIN, false);

            pStyAttr = 0;
            pOutFmtNode = pOldMod;

            if( pNewSet != rNode.GetpSwAttrSet() )
                delete pNewSet;
        }
    }

    AttrOutput().EndParagraphProperties();

    AttrOutput().EndParagraph( pTextNodeInfoInner );

    SAL_INFO( "sw.ww8", "</OutWW8_SwTxtNode>" );
}

void WW8AttributeOutput::TableNodeInfo( ww8::WW8TableNodeInfo::Pointer_t pNodeInfo )
{
    SVBT16 nSty;
    ShortToSVBT16( GetExport().nStyleBeforeFly, nSty );

    ww8::WW8TableNodeInfo::Inners_t::const_iterator aIt( pNodeInfo->getInners().begin() );
    ww8::WW8TableNodeInfo::Inners_t::const_iterator aItEnd( pNodeInfo->getInners().end() );

    while (aIt != aItEnd)
    {
        ww8::WW8TableNodeInfoInner::Pointer_t pInner = aIt->second;
        if ( pInner->isEndOfCell() )
        {
            TableRowEnd( pInner->getDepth() );

            m_rWW8Export.pO->insert( m_rWW8Export.pO->end(), (sal_uInt8*)&nSty, (sal_uInt8*)&nSty+2);     // Style #
            TableInfoRow( pInner );
            m_rWW8Export.pPapPlc->AppendFkpEntry( m_rWW8Export.Strm().Tell(), m_rWW8Export.pO->size(), m_rWW8Export.pO->data());
            m_rWW8Export.pO->clear();
        }

        if ( pInner->isEndOfLine() )
        {
        }

        ++aIt;
    }
}

// Tables

void WW8AttributeOutput::EmptyParagraph()
{
    m_rWW8Export.WriteStringAsPara( aEmptyStr );
}

bool MSWordExportBase::NoPageBreakSection( const SfxItemSet* pSet )
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


void MSWordExportBase::OutputSectionNode( const SwSectionNode& rSectionNode )
{
    const SwSection& rSection = rSectionNode.GetSection();

    SwNodeIndex aIdx( rSectionNode, 1 );
    const SwNode& rNd = aIdx.GetNode();
    if ( !rNd.IsSectionNode() && !IsInTable() ) //No sections in table
    {
        // if the first Node inside the section has an own
        // PageDesc or PageBreak attribut, then dont write
        // here the section break
        sal_uLong nRstLnNum = 0;
        const SfxItemSet* pSet;
        if ( rNd.IsTableNode() )
            pSet = &rNd.GetTableNode()->GetTable().GetFrmFmt()->GetAttrSet();
        else if ( rNd.IsCntntNode() )
        {
            pSet = &rNd.GetCntntNode()->GetSwAttrSet();
            nRstLnNum = ((SwFmtLineNumber&)pSet->Get(
                            RES_LINENUMBER )).GetStartValue();
        }
        else
            pSet = 0;

        if ( pSet && NoPageBreakSection( pSet ) )
            pSet = 0;

        if ( !pSet )
        {
            // new Section with no own PageDesc/-Break
            //  -> write follow section break;
            const SwSectionFmt& rFmt = *rSection.GetFmt();
            ReplaceCr( msword::PageBreak ); // Indikator fuer Page/Section-Break

            // Get the page in use at the top of this section
            SwNodeIndex aIdxTmp(rSectionNode, 1);
            const SwPageDesc *pCurrent =
                SwPageDesc::GetPageDescOfNode(aIdxTmp.GetNode());
            if (!pCurrent)
                pCurrent = pAktPageDesc;

            AppendSection( pCurrent, &rFmt, nRstLnNum );
        }
    }
    if ( TOX_CONTENT_SECTION == rSection.GetType() )
        bStartTOX = true;
}


void WW8Export::AppendSection( const SwPageDesc *pPageDesc, const SwSectionFmt* pFmt, sal_uLong nLnNum )
{
    pSepx->AppendSep(Fc2Cp(Strm().Tell()), pPageDesc, pFmt, nLnNum);
}

// Flys

void WW8Export::OutWW6FlyFrmsInCntnt( const SwTxtNode& rNd )
{
    OSL_ENSURE(!bWrtWW8, "I shouldn't be needed for Word >=8");
    if ( bWrtWW8 )
        return;

    if (const SwpHints* pTxtAttrs = rNd.GetpSwpHints())
    {
        for( sal_uInt16 n=0; n < pTxtAttrs->Count(); ++n )
        {
            const SwTxtAttr* pAttr = (*pTxtAttrs)[ n ];
            if( RES_TXTATR_FLYCNT == pAttr->Which() )
            {
                // attribute bound to a character
                const SwFmtFlyCnt& rFlyCntnt = pAttr->GetFlyCnt();
                const SwFlyFrmFmt& rFlyFrmFmt = *(SwFlyFrmFmt*)rFlyCntnt.GetFrmFmt();
                const SwNodeIndex* pNodeIndex = rFlyFrmFmt.GetCntnt().GetCntntIdx();

                if( pNodeIndex )
                {
                    sal_uLong nStt = pNodeIndex->GetIndex()+1,
                          nEnd = pNodeIndex->GetNode().EndOfSectionIndex();

                    if( (nStt < nEnd) && !pDoc->GetNodes()[ nStt ]->IsNoTxtNode() )
                    {
                        Point aOffset;
                        // get rectangle (bounding box?) of Fly and paragraph
                        SwRect aParentRect(rNd.FindLayoutRect(false, &aOffset)),
                               aFlyRect(rFlyFrmFmt.FindLayoutRect(false, &aOffset ) );

                        aOffset = aFlyRect.Pos() - aParentRect.Pos();

                        // let PaM point to content of Fly-frame format
                        SaveData( nStt, nEnd );

                        // is analysed in OutputFormat()
                        pFlyOffset = &aOffset;
                        eNewAnchorType = rFlyFrmFmt.GetAnchor().GetAnchorId();
                        sw::Frame aFrm(rFlyFrmFmt, SwPosition(rNd));
                        mpParentFrame = &aFrm;
                        // Ok, write it out:
                        WriteText();

                        RestoreData();
                    }
                }
            }
        }
    }
}

void WW8AttributeOutput::OutputFlyFrame_Impl( const sw::Frame& rFmt, const Point& rNdTopLeft )
{
    const SwFrmFmt &rFrmFmt = rFmt.GetFrmFmt();
    const SwFmtAnchor& rAnch = rFrmFmt.GetAnchor();

    bool bUseEscher = m_rWW8Export.bWrtWW8;

    if ( m_rWW8Export.bWrtWW8 && rFmt.IsInline() )
    {
        sw::Frame::WriterSource eType = rFmt.GetWriterType();
        if ((eType == sw::Frame::eGraphic) || (eType == sw::Frame::eOle))
            bUseEscher = false;
        else
            bUseEscher = true;

        /*
         A special case for converting some inline form controls to form fields
         when in winword 8+ mode
        */
        if ((bUseEscher == true) && (eType == sw::Frame::eFormControl))
        {
            if ( m_rWW8Export.MiserableFormFieldExportHack( rFrmFmt ) )
                return ;
        }
    }

    if (bUseEscher)
    {
        OSL_ENSURE( m_rWW8Export.bWrtWW8, "this has gone horribly wrong" );
        // write as escher
        m_rWW8Export.AppendFlyInFlys(rFmt, rNdTopLeft);
    }
    else
    {
        bool bDone = false;

        // Hole vom Node und vom letzten Node die Position in der Section
        const SwNodeIndex* pNodeIndex = rFrmFmt.GetCntnt().GetCntntIdx();

        sal_uLong nStt = pNodeIndex ? pNodeIndex->GetIndex()+1                  : 0;
        sal_uLong nEnd = pNodeIndex ? pNodeIndex->GetNode().EndOfSectionIndex() : 0;

        if( nStt >= nEnd )      // no range, hence no valid node
            return;

        if ( !m_rWW8Export.IsInTable() && rFmt.IsInline() )
        {
            //Test to see if this textbox contains only a single graphic/ole
            SwTxtNode* pParTxtNode = rAnch.GetCntntAnchor()->nNode.GetNode().GetTxtNode();
            if ( pParTxtNode && !m_rWW8Export.pDoc->GetNodes()[ nStt ]->IsNoTxtNode() )
                bDone = true;
        }
        if( !bDone )
        {

            m_rWW8Export.SaveData( nStt, nEnd );

            Point aOffset;
            if ( m_rWW8Export.mpParentFrame )
            {
                /* Munge flys in fly into absolutely positioned elements for word 6 */
                const SwTxtNode* pParTxtNode = rAnch.GetCntntAnchor()->nNode.GetNode().GetTxtNode();
                const SwRect aPageRect = pParTxtNode->FindPageFrmRect( sal_False, 0, sal_False );

                aOffset = rFrmFmt.FindLayoutRect().Pos();
                aOffset -= aPageRect.Pos();

                m_rWW8Export.pFlyOffset = &aOffset;
                m_rWW8Export.eNewAnchorType = FLY_AT_PAGE;
            }

            m_rWW8Export.mpParentFrame = &rFmt;
            if (
                m_rWW8Export.IsInTable() &&
                 (FLY_AT_PAGE != rAnch.GetAnchorId()) &&
                 !m_rWW8Export.pDoc->GetNodes()[ nStt ]->IsNoTxtNode()
               )
            {
                // note: set Flag  bOutTable again,
                // because we deliver the normal content of the table cell, and no border
                // ( Flag was deleted above in aSaveData() )
                m_rWW8Export.bOutTable = true;
                const OUString aName = rFrmFmt.GetName();
                m_rWW8Export.StartCommentOutput(aName);
                m_rWW8Export.WriteText();
                m_rWW8Export.EndCommentOutput(aName);
            }
            else
                m_rWW8Export.WriteText();

            m_rWW8Export.RestoreData();
        }
    }
}

void AttributeOutputBase::OutputFlyFrame( const sw::Frame& rFmt )
{
    if ( !rFmt.GetCntntNode() )
        return;

    const SwCntntNode &rNode = *rFmt.GetCntntNode();
    Point aNdPos, aPgPos;
    Point* pLayPos;
    bool bValidNdPos = false, bValidPgPos = false;

    if (FLY_AT_PAGE == rFmt.GetFrmFmt().GetAnchor().GetAnchorId())
    {
        // get the Layout Node-Position.
        if ( !bValidPgPos )
        {
            aPgPos = rNode.FindPageFrmRect(false, &aPgPos).Pos();
            bValidPgPos = true;
        }
        pLayPos = &aPgPos;
    }
    else
    {
        // get the Layout Node-Position.
        if ( !bValidNdPos )
        {
            aNdPos = rNode.FindLayoutRect(false, &aNdPos).Pos();
            bValidNdPos = true;
        }
        pLayPos = &aNdPos;
    }

    OutputFlyFrame_Impl( rFmt, *pLayPos );
}

// write data of any redline
void WW8AttributeOutput::Redline( const SwRedlineData* pRedline )
{
    if ( !pRedline )
        return;

    if ( pRedline->Next() )
        Redline( pRedline->Next() );

    static const sal_uInt16 aSprmIds[ 2 * 2 * 3 ] =
    {
        // Ids for insert
            NS_sprm::LN_CFRMark, NS_sprm::LN_CIbstRMark, NS_sprm::LN_CDttmRMark,         // for WW8
            0x0042, 0x0045, 0x0046,         // for WW6
        // Ids for delete
            NS_sprm::LN_CFRMarkDel, NS_sprm::LN_CIbstRMarkDel, NS_sprm::LN_CDttmRMarkDel,         // for WW8
            0x0041, 0x0045, 0x0046          // for WW6
    };

    const sal_uInt16* pSprmIds = 0;
    switch( pRedline->GetType() )
    {
    case nsRedlineType_t::REDLINE_INSERT:
        pSprmIds = aSprmIds;
        break;

    case nsRedlineType_t::REDLINE_DELETE:
        pSprmIds = aSprmIds + (2 * 3);
        break;

    case nsRedlineType_t::REDLINE_FORMAT:
        if( m_rWW8Export.bWrtWW8 )
        {
            m_rWW8Export.InsUInt16( NS_sprm::LN_CPropRMark );
            m_rWW8Export.pO->push_back( 7 );       // len
            m_rWW8Export.pO->push_back( 1 );
            m_rWW8Export.InsUInt16( m_rWW8Export.AddRedlineAuthor( pRedline->GetAuthor() ) );
            m_rWW8Export.InsUInt32( sw::ms::DateTime2DTTM( pRedline->GetTimeStamp() ));
        }
        break;
    default:
        OSL_ENSURE(!this, "Unhandled redline type for export");
        break;
    }

    if ( pSprmIds )
    {
        if ( !m_rWW8Export.bWrtWW8 )
            pSprmIds += 3;

        if ( m_rWW8Export.bWrtWW8 )
            m_rWW8Export.InsUInt16( pSprmIds[0] );
        else
            m_rWW8Export.pO->push_back( msword_cast<sal_uInt8>(pSprmIds[0]) );
        m_rWW8Export.pO->push_back( 1 );

        if ( m_rWW8Export.bWrtWW8 )
            m_rWW8Export.InsUInt16( pSprmIds[1] );
        else
            m_rWW8Export.pO->push_back( msword_cast<sal_uInt8>(pSprmIds[1]) );
        m_rWW8Export.InsUInt16( m_rWW8Export.AddRedlineAuthor( pRedline->GetAuthor() ) );

        if ( m_rWW8Export.bWrtWW8 )
            m_rWW8Export.InsUInt16( pSprmIds[2] );
        else
            m_rWW8Export.pO->push_back( msword_cast<sal_uInt8>(pSprmIds[2]) );
        m_rWW8Export.InsUInt32( sw::ms::DateTime2DTTM( pRedline->GetTimeStamp() ));
    }
}


void MSWordExportBase::OutputContentNode( const SwCntntNode& rNode )
{
    switch ( rNode.GetNodeType() )
    {
        case ND_TEXTNODE:
        {
            const SwTxtNode& rTextNode = *rNode.GetTxtNode();
            if( !mbOutOutlineOnly || rTextNode.IsOutline() )
                OutputTextNode( rTextNode );
        }
        break;
        case ND_GRFNODE:
            OutputGrfNode( *rNode.GetGrfNode() );
            break;
        case ND_OLENODE:
            OutputOLENode( *rNode.GetOLENode() );
            break;
        default:
            OSL_TRACE("Unhandled node, type == %d", rNode.GetNodeType() );
            break;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

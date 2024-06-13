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
#include <DocumentStylePoolManager.hxx>
#include <SwStyleNameMapper.hxx>
#include <doc.hxx>
#include <DocumentSettingManager.hxx>
#include <IDocumentState.hxx>
#include <IDocumentUndoRedo.hxx>
#include <fmtanchr.hxx>
#include <fmtfsize.hxx>
#include <paratr.hxx>
#include <poolfmt.hxx>
#include <fmtornt.hxx>
#include <charfmt.hxx>
#include <fmtsrnd.hxx>
#include <docary.hxx>
#include <pagedesc.hxx>
#include <frmfmt.hxx>
#include <fmtline.hxx>
#include <numrule.hxx>
#include <hints.hxx>
#include <editeng/paperinf.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/tstpitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/lspcitem.hxx>
#include <editeng/adjustitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/keepitem.hxx>
#include <editeng/opaqitem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/cmapitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/protitem.hxx>
#include <editeng/escapementitem.hxx>
#include <editeng/langitem.hxx>
#include <editeng/charrotateitem.hxx>
#include <editeng/emphasismarkitem.hxx>
#include <editeng/scriptspaceitem.hxx>
#include <o3tl/unit_conversion.hxx>
#include <svx/strings.hrc>
#include <svx/dialmgr.hxx>
#include <sal/log.hxx>
#include <osl/diagnose.h>
#include <strings.hrc>
#include <frmatr.hxx>
#include <frameformats.hxx>
#include <com/sun/star/text/VertOrientation.hpp>
#include <com/sun/star/text/RelOrientation.hpp>
#include <com/sun/star/text/HoriOrientation.hpp>
#include <unotools/syslocale.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <comphelper/lok.hxx>

using namespace ::editeng;
using namespace ::com::sun::star;

bool IsConditionalByPoolId(sal_uInt16 nId)
{
    // TODO: why is this style conditional?
    // If it is changed to no longer be conditional, then a style "Text Body"
    // will be imported without its conditions from ODF.
    return RES_POOLCOLL_TEXT == nId;
}

namespace
{
    const sal_uInt16 PT_3   =  3 * 20;      //  3 pt
    const sal_uInt16 PT_6   =  6 * 20;      //  6 pt
    const sal_uInt16 PT_7   =  7 * 20;      //  7 pt
    const sal_uInt16 PT_9   =  9 * 20;      //  9 pt
    const sal_uInt16 PT_10  = 10 * 20;      // 10 pt
    const sal_uInt16 PT_12  = 12 * 20;      // 12 pt
    const sal_uInt16 PT_13  = 13 * 20;      // 13 pt
    const sal_uInt16 PT_14  = 14 * 20;      // 14 pt
    const sal_uInt16 PT_16  = 16 * 20;      // 16 pt
    const sal_uInt16 PT_18  = 18 * 20;      // 18 pt
    const sal_uInt16 PT_24  = 24 * 20;      // 24 pt
    const sal_uInt16 PT_28  = 28 * 20;      // 28 pt

    const sal_uInt16 HTML_PARSPACE = o3tl::convert(5, o3tl::Length::mm, o3tl::Length::twip);

    const sal_uInt16 aHeadlineSizes[ 2 * MAXLEVEL ] = {
        // we do everything percentual now:
        PT_18, PT_16, PT_14, PT_13, PT_12,
        PT_12, PT_10, PT_10, PT_9, PT_9,  // normal

        PT_24, PT_18, PT_14, PT_12, PT_10,
        PT_7, PT_7, PT_7, PT_7, PT_7            // HTML mode
    };

    tools::Long lcl_GetRightMargin( SwDoc& rDoc )
    {
        // Make sure that the printer settings are taken over to the standard
        // page style
        const SwFrameFormat& rPgDscFormat = rDoc.GetPageDesc( 0 ).GetMaster();
        const SvxLRSpaceItem& rLR = rPgDscFormat.GetLRSpace();
        const tools::Long nLeft = rLR.GetLeft();
        const tools::Long nRight = rLR.GetRight();
        const tools::Long nWidth = rPgDscFormat.GetFrameSize().GetWidth();
        return nWidth - nLeft - nRight;
    }

    void lcl_SetDfltFont( DefaultFontType nFntType, SfxItemSet& rSet )
    {
        static struct {
            sal_uInt16 nResLngId;
            sal_uInt16 nResFntId;
        } aArr[ 3 ] = {
            { RES_CHRATR_LANGUAGE, RES_CHRATR_FONT },
            { RES_CHRATR_CJK_LANGUAGE, RES_CHRATR_CJK_FONT },
            { RES_CHRATR_CTL_LANGUAGE, RES_CHRATR_CTL_FONT }
        };
        for(const auto & n : aArr)
        {
            LanguageType nLng = static_cast<const SvxLanguageItem&>(rSet.GetPool()->GetUserOrPoolDefaultItem(
                                n.nResLngId )).GetLanguage();
            vcl::Font aFnt( OutputDevice::GetDefaultFont( nFntType,
                                    nLng, GetDefaultFontFlags::OnlyOne ) );

            rSet.Put( SvxFontItem( aFnt.GetFamilyType(), aFnt.GetFamilyName(),
                                OUString(), aFnt.GetPitch(),
                                aFnt.GetCharSet(), n.nResFntId ));
        }
    }

    void lcl_SetDfltFont( DefaultFontType nLatinFntType, DefaultFontType nCJKFntType,
                            DefaultFontType nCTLFntType, SfxItemSet& rSet )
    {
        static struct {
            sal_uInt16 nResLngId;
            sal_uInt16 nResFntId;
            DefaultFontType nFntType;
        } aArr[ 3 ] = {
            { RES_CHRATR_LANGUAGE, RES_CHRATR_FONT, static_cast<DefaultFontType>(0) },
            { RES_CHRATR_CJK_LANGUAGE, RES_CHRATR_CJK_FONT, static_cast<DefaultFontType>(0) },
            { RES_CHRATR_CTL_LANGUAGE, RES_CHRATR_CTL_FONT, static_cast<DefaultFontType>(0) }
        };
        aArr[0].nFntType = nLatinFntType;
        aArr[1].nFntType = nCJKFntType;
        aArr[2].nFntType = nCTLFntType;

        for(const auto & n : aArr)
        {
            LanguageType nLng = static_cast<const SvxLanguageItem&>(rSet.GetPool()->GetUserOrPoolDefaultItem(
                                n.nResLngId )).GetLanguage();
            vcl::Font aFnt( OutputDevice::GetDefaultFont( n.nFntType,
                                    nLng, GetDefaultFontFlags::OnlyOne ) );

            rSet.Put( SvxFontItem( aFnt.GetFamilyType(), aFnt.GetFamilyName(),
                                OUString(), aFnt.GetPitch(),
                                aFnt.GetCharSet(), n.nResFntId ));
        }
    }

    void lcl_SetHeadline( SwDoc& rDoc, SwTextFormatColl* pColl,
                            SfxItemSet& rSet,
                            sal_uInt16 nOutLvlBits, sal_uInt8 nLevel, bool bItalic )
    {
        SetAllScriptItem( rSet, SvxWeightItem( WEIGHT_BOLD, RES_CHRATR_WEIGHT ) );
        SvxFontHeightItem aHItem(240, 100, RES_CHRATR_FONTSIZE);
        const bool bHTMLMode = rDoc.GetDocumentSettingManager().get(DocumentSettingId::HTML_MODE);
        if( bHTMLMode )
            aHItem.SetHeight( aHeadlineSizes[ MAXLEVEL + nLevel ] );
        else
            aHItem.SetHeight( aHeadlineSizes[ nLevel ] );
        SetAllScriptItem( rSet, aHItem );

        if( bItalic && !bHTMLMode )
            SetAllScriptItem( rSet, SvxPostureItem( ITALIC_NORMAL, RES_CHRATR_POSTURE ) );

        if( bHTMLMode )
        {
            lcl_SetDfltFont( DefaultFontType::LATIN_TEXT, DefaultFontType::CJK_TEXT,
                                DefaultFontType::CTL_TEXT, rSet );
        }

        if( !pColl )
            return;

        if( !( nOutLvlBits & ( 1 << nLevel )) )
        {
            pColl->AssignToListLevelOfOutlineStyle(nLevel);
            if( !bHTMLMode )
            {
                SwNumRule * pOutlineRule = rDoc.GetOutlineNumRule();
                const SwNumFormat& rNFormat = pOutlineRule->Get( nLevel );

                if ( rNFormat.GetPositionAndSpaceMode() ==
                                    SvxNumberFormat::LABEL_WIDTH_AND_POSITION &&
                        ( rNFormat.GetAbsLSpace() || rNFormat.GetFirstLineOffset() ) )
                {
                    SvxFirstLineIndentItem firstLine(pColl->GetFormatAttr(RES_MARGIN_FIRSTLINE));
                    SvxTextLeftMarginItem leftMargin(pColl->GetFormatAttr(RES_MARGIN_TEXTLEFT));
                    firstLine.SetTextFirstLineOffsetValue(rNFormat.GetFirstLineOffset());
                        //TODO: overflow
                    leftMargin.SetTextLeft(rNFormat.GetAbsLSpace());
                    pColl->SetFormatAttr(firstLine);
                    pColl->SetFormatAttr(leftMargin);
                }

                // All paragraph styles, which are assigned to a level of the
                // outline style has to have the outline style set as its list style.
                SwNumRuleItem aItem(pOutlineRule->GetName());
                pColl->SetFormatAttr(aItem);
            }
        }
        pColl->SetNextTextFormatColl( *rDoc.getIDocumentStylePoolAccess().GetTextCollFromPool( RES_POOLCOLL_TEXT ));
    }

    void lcl_SetRegister( SwDoc& rDoc, SfxItemSet& rSet, sal_uInt16 nFact,
                            bool bHeader, bool bTab )
    {
        sal_uInt16 nLeft = o3tl::convert(5 * nFact, o3tl::Length::mm, o3tl::Length::twip);
        SvxFirstLineIndentItem const firstLine(0, RES_MARGIN_FIRSTLINE);
        SvxTextLeftMarginItem const leftMargin(nLeft, RES_MARGIN_TEXTLEFT);
        rSet.Put(firstLine);
        rSet.Put(leftMargin);
        if( bHeader )
        {
            SetAllScriptItem( rSet, SvxWeightItem( WEIGHT_BOLD, RES_CHRATR_WEIGHT ) );
            SetAllScriptItem( rSet, SvxFontHeightItem( PT_16, 100, RES_CHRATR_FONTSIZE ) );
        }
        if( bTab )
        {
            tools::Long nRightMargin = lcl_GetRightMargin( rDoc );
            SvxTabStopItem aTStops( 0, 0, SvxTabAdjust::Default, RES_PARATR_TABSTOP );
            aTStops.Insert( SvxTabStop( nRightMargin - nLeft,
                                        SvxTabAdjust::Right,
                                        cDfltDecimalChar, '.' ));
            rSet.Put( aTStops );
        }
    }

    void lcl_SetNumBul( SwDoc& rDoc, SwTextFormatColl* pColl,
                            SfxItemSet& rSet,
                            sal_uInt16 nNxt, SwTwips nEZ, SwTwips nLeft,
                            SwTwips nUpper, SwTwips nLower )
    {
        SvxFirstLineIndentItem const firstLine(sal_uInt16(nEZ), RES_MARGIN_FIRSTLINE);
        SvxTextLeftMarginItem const leftMargin(sal_uInt16(nLeft), RES_MARGIN_TEXTLEFT);
        rSet.Put(firstLine);
        rSet.Put(leftMargin);
        SvxULSpaceItem aUL( RES_UL_SPACE );
        aUL.SetUpper( sal_uInt16(nUpper) );
        aUL.SetLower( sal_uInt16(nLower) );
        rSet.Put( aUL );

        if( pColl )
            pColl->SetNextTextFormatColl( *rDoc.getIDocumentStylePoolAccess().GetTextCollFromPool( nNxt ));
    }

    void lcl_PutStdPageSizeIntoItemSet( SwDoc& rDoc, SfxItemSet& rSet )
    {
        SwPageDesc* pStdPgDsc = rDoc.getIDocumentStylePoolAccess().GetPageDescFromPool( RES_POOLPAGE_STANDARD );
        SwFormatFrameSize aFrameSz( pStdPgDsc->GetMaster().GetFrameSize() );
        if( pStdPgDsc->GetLandscape() )
        {
            SwTwips nTmp = aFrameSz.GetHeight();
            aFrameSz.SetHeight( aFrameSz.GetWidth() );
            aFrameSz.SetWidth( nTmp );
        }
        rSet.Put( aFrameSz );
    }
}

const TranslateId STR_POOLCOLL_TEXT_ARY[] =
{
    // Category Text
    STR_POOLCOLL_STANDARD,
    STR_POOLCOLL_TEXT,
    STR_POOLCOLL_TEXT_IDENT,
    STR_POOLCOLL_TEXT_NEGIDENT,
    STR_POOLCOLL_TEXT_MOVE,
    STR_POOLCOLL_GREETING,
    STR_POOLCOLL_SIGNATURE,
    STR_POOLCOLL_CONFRONTATION,
    STR_POOLCOLL_MARGINAL,
    // Subcategory Headlines
    STR_POOLCOLL_HEADLINE_BASE,
    STR_POOLCOLL_HEADLINE1,
    STR_POOLCOLL_HEADLINE2,
    STR_POOLCOLL_HEADLINE3,
    STR_POOLCOLL_HEADLINE4,
    STR_POOLCOLL_HEADLINE5,
    STR_POOLCOLL_HEADLINE6,
    STR_POOLCOLL_HEADLINE7,
    STR_POOLCOLL_HEADLINE8,
    STR_POOLCOLL_HEADLINE9,
    STR_POOLCOLL_HEADLINE10
};

const TranslateId STR_POOLCOLL_LISTS_ARY[]
{
    // Category Lists
    STR_POOLCOLL_NUMBER_BULLET_BASE,
    // Subcategory Numbering
    STR_POOLCOLL_NUM_LEVEL1S,
    STR_POOLCOLL_NUM_LEVEL1,
    STR_POOLCOLL_NUM_LEVEL1E,
    STR_POOLCOLL_NUM_NONUM1,
    STR_POOLCOLL_NUM_LEVEL2S,
    STR_POOLCOLL_NUM_LEVEL2,
    STR_POOLCOLL_NUM_LEVEL2E,
    STR_POOLCOLL_NUM_NONUM2,
    STR_POOLCOLL_NUM_LEVEL3S,
    STR_POOLCOLL_NUM_LEVEL3,
    STR_POOLCOLL_NUM_LEVEL3E,
    STR_POOLCOLL_NUM_NONUM3,
    STR_POOLCOLL_NUM_LEVEL4S,
    STR_POOLCOLL_NUM_LEVEL4,
    STR_POOLCOLL_NUM_LEVEL4E,
    STR_POOLCOLL_NUM_NONUM4,
    STR_POOLCOLL_NUM_LEVEL5S,
    STR_POOLCOLL_NUM_LEVEL5,
    STR_POOLCOLL_NUM_LEVEL5E,
    STR_POOLCOLL_NUM_NONUM5,

    // Subcategory Enumeration
    STR_POOLCOLL_BULLET_LEVEL1S,
    STR_POOLCOLL_BULLET_LEVEL1,
    STR_POOLCOLL_BULLET_LEVEL1E,
    STR_POOLCOLL_BULLET_NONUM1,
    STR_POOLCOLL_BULLET_LEVEL2S,
    STR_POOLCOLL_BULLET_LEVEL2,
    STR_POOLCOLL_BULLET_LEVEL2E,
    STR_POOLCOLL_BULLET_NONUM2,
    STR_POOLCOLL_BULLET_LEVEL3S,
    STR_POOLCOLL_BULLET_LEVEL3,
    STR_POOLCOLL_BULLET_LEVEL3E,
    STR_POOLCOLL_BULLET_NONUM3,
    STR_POOLCOLL_BULLET_LEVEL4S,
    STR_POOLCOLL_BULLET_LEVEL4,
    STR_POOLCOLL_BULLET_LEVEL4E,
    STR_POOLCOLL_BULLET_NONUM4,
    STR_POOLCOLL_BULLET_LEVEL5S,
    STR_POOLCOLL_BULLET_LEVEL5,
    STR_POOLCOLL_BULLET_LEVEL5E,
    STR_POOLCOLL_BULLET_NONUM5
};

// Special Areas
const TranslateId STR_POOLCOLL_EXTRA_ARY[]
{
    // Subcategory Header
    STR_POOLCOLL_HEADERFOOTER,
    STR_POOLCOLL_HEADER,
    STR_POOLCOLL_HEADERL,
    STR_POOLCOLL_HEADERR,
    // Subcategory Footer
    STR_POOLCOLL_FOOTER,
    STR_POOLCOLL_FOOTERL,
    STR_POOLCOLL_FOOTERR,
    // Subcategory Table
    STR_POOLCOLL_TABLE,
    STR_POOLCOLL_TABLE_HDLN,
    // Subcategory Labels
    STR_POOLCOLL_LABEL,
    STR_POOLCOLL_LABEL_ABB,
    STR_POOLCOLL_LABEL_TABLE,
    STR_POOLCOLL_LABEL_FRAME,
    STR_POOLCOLL_LABEL_FIGURE,
    // Miscellaneous
    STR_POOLCOLL_FRAME,
    STR_POOLCOLL_FOOTNOTE,
    STR_POOLCOLL_ENVELOPE_ADDRESS,
    STR_POOLCOLL_SEND_ADDRESS,
    STR_POOLCOLL_ENDNOTE,
    STR_POOLCOLL_LABEL_DRAWING,
    STR_POOLCOLL_COMMENT
};

const TranslateId STR_POOLCOLL_REGISTER_ARY[] =
{
    // Category Directories
    STR_POOLCOLL_REGISTER_BASE,
    // Subcategory Index-Directories
    STR_POOLCOLL_TOX_IDXH,
    STR_POOLCOLL_TOX_IDX1,
    STR_POOLCOLL_TOX_IDX2,
    STR_POOLCOLL_TOX_IDX3,
    STR_POOLCOLL_TOX_IDXBREAK,
    // Subcategory Tables of Contents
    STR_POOLCOLL_TOX_CNTNTH,
    STR_POOLCOLL_TOX_CNTNT1,
    STR_POOLCOLL_TOX_CNTNT2,
    STR_POOLCOLL_TOX_CNTNT3,
    STR_POOLCOLL_TOX_CNTNT4,
    STR_POOLCOLL_TOX_CNTNT5,
    // Subcategory User-Directories:
    STR_POOLCOLL_TOX_USERH,
    STR_POOLCOLL_TOX_USER1,
    STR_POOLCOLL_TOX_USER2,
    STR_POOLCOLL_TOX_USER3,
    STR_POOLCOLL_TOX_USER4,
    STR_POOLCOLL_TOX_USER5,
    // Subcategory Table of Contents more Levels 5 - 10
    STR_POOLCOLL_TOX_CNTNT6,
    STR_POOLCOLL_TOX_CNTNT7,
    STR_POOLCOLL_TOX_CNTNT8,
    STR_POOLCOLL_TOX_CNTNT9,
    STR_POOLCOLL_TOX_CNTNT10,
    // Illustrations Index
    STR_POOLCOLL_TOX_ILLUSH,
    STR_POOLCOLL_TOX_ILLUS1,
    //  Object Index
    STR_POOLCOLL_TOX_OBJECTH,
    STR_POOLCOLL_TOX_OBJECT1,
    //  Tables Index
    STR_POOLCOLL_TOX_TABLESH,
    STR_POOLCOLL_TOX_TABLES1,
    //  Index of Authorities
    STR_POOLCOLL_TOX_AUTHORITIESH,
    STR_POOLCOLL_TOX_AUTHORITIES1,
    // Subcategory User-Directories more Levels 5 - 10
    STR_POOLCOLL_TOX_USER6,
    STR_POOLCOLL_TOX_USER7,
    STR_POOLCOLL_TOX_USER8,
    STR_POOLCOLL_TOX_USER9,
    STR_POOLCOLL_TOX_USER10
};

const TranslateId STR_POOLCOLL_DOC_ARY[] =
{
    // Category Chapter/Document
    STR_POOLCOLL_DOC_TITLE,
    STR_POOLCOLL_DOC_SUBTITLE,
    STR_POOLCOLL_DOC_APPENDIX
};

const TranslateId STR_POOLCOLL_HTML_ARY[] =
{
    // Category HTML-Templates
    STR_POOLCOLL_HTML_BLOCKQUOTE,
    STR_POOLCOLL_HTML_PRE,
    STR_POOLCOLL_HTML_HR,
    STR_POOLCOLL_HTML_DD,
    STR_POOLCOLL_HTML_DT
};

const TranslateId STR_POOLCHR_ARY[] =
{
    STR_POOLCHR_FOOTNOTE,
    STR_POOLCHR_PAGENO,
    STR_POOLCHR_LABEL,
    STR_POOLCHR_DROPCAPS,
    STR_POOLCHR_NUM_LEVEL,
    STR_POOLCHR_BULLET_LEVEL,
    STR_POOLCHR_INET_NORMAL,
    STR_POOLCHR_INET_VISIT,
    STR_POOLCHR_JUMPEDIT,
    STR_POOLCHR_TOXJUMP,
    STR_POOLCHR_ENDNOTE,
    STR_POOLCHR_LINENUM,
    STR_POOLCHR_IDX_MAIN_ENTRY,
    STR_POOLCHR_FOOTNOTE_ANCHOR,
    STR_POOLCHR_ENDNOTE_ANCHOR,
    STR_POOLCHR_RUBYTEXT,
    STR_POOLCHR_VERT_NUM
};

const TranslateId STR_POOLCHR_HTML_ARY[] =
{
    STR_POOLCHR_HTML_EMPHASIS,
    STR_POOLCHR_HTML_CITATION,
    STR_POOLCHR_HTML_STRONG,
    STR_POOLCHR_HTML_CODE,
    STR_POOLCHR_HTML_SAMPLE,
    STR_POOLCHR_HTML_KEYBOARD,
    STR_POOLCHR_HTML_VARIABLE,
    STR_POOLCHR_HTML_DEFINSTANCE,
    STR_POOLCHR_HTML_TELETYPE
};

const TranslateId STR_POOLFRM_ARY[] =
{
    STR_POOLFRM_FRAME,
    STR_POOLFRM_GRAPHIC,
    STR_POOLFRM_OLE,
    STR_POOLFRM_FORMEL,
    STR_POOLFRM_MARGINAL,
    STR_POOLFRM_WATERSIGN,
    STR_POOLFRM_LABEL
};

const TranslateId STR_POOLPAGE_ARY[] =
{
    // Page styles
    STR_POOLPAGE_STANDARD,
    STR_POOLPAGE_FIRST,
    STR_POOLPAGE_LEFT,
    STR_POOLPAGE_RIGHT,
    STR_POOLPAGE_ENVELOPE,
    STR_POOLPAGE_REGISTER,
    STR_POOLPAGE_HTML,
    STR_POOLPAGE_FOOTNOTE,
    STR_POOLPAGE_ENDNOTE,
    STR_POOLPAGE_LANDSCAPE
};

const TranslateId STR_POOLNUMRULE_NUM_ARY[] =
{
    // Numbering styles
    STR_POOLNUMRULE_NOLIST,
    STR_POOLNUMRULE_NUM1,
    STR_POOLNUMRULE_NUM2,
    STR_POOLNUMRULE_NUM3,
    STR_POOLNUMRULE_NUM4,
    STR_POOLNUMRULE_NUM5,
    STR_POOLNUMRULE_BUL1,
    STR_POOLNUMRULE_BUL2,
    STR_POOLNUMRULE_BUL3,
    STR_POOLNUMRULE_BUL4,
    STR_POOLNUMRULE_BUL5
};

// XXX MUST match the entries of TableStyleProgNameTable in
// sw/source/core/doc/SwStyleNameMapper.cxx and MUST match the order of
// RES_POOL_TABLESTYLE_TYPE in sw/inc/poolfmt.hxx
const TranslateId STR_TABSTYLE_ARY[] =
{
    // XXX MUST be in order, Writer first, then Svx old, then Svx new
    // 1 Writer resource string
    STR_TABSTYLE_DEFAULT,
    // 16 old styles Svx resource strings
    RID_SVXSTR_TBLAFMT_3D,
    RID_SVXSTR_TBLAFMT_BLACK1,
    RID_SVXSTR_TBLAFMT_BLACK2,
    RID_SVXSTR_TBLAFMT_BLUE,
    RID_SVXSTR_TBLAFMT_BROWN,
    RID_SVXSTR_TBLAFMT_CURRENCY,
    RID_SVXSTR_TBLAFMT_CURRENCY_3D,
    RID_SVXSTR_TBLAFMT_CURRENCY_GRAY,
    RID_SVXSTR_TBLAFMT_CURRENCY_LAVENDER,
    RID_SVXSTR_TBLAFMT_CURRENCY_TURQUOISE,
    RID_SVXSTR_TBLAFMT_GRAY,
    RID_SVXSTR_TBLAFMT_GREEN,
    RID_SVXSTR_TBLAFMT_LAVENDER,
    RID_SVXSTR_TBLAFMT_RED,
    RID_SVXSTR_TBLAFMT_TURQUOISE,
    RID_SVXSTR_TBLAFMT_YELLOW,
    // 10 new styles since LibreOffice 6.0 Svx resource strings
    RID_SVXSTR_TBLAFMT_LO6_ACADEMIC,
    RID_SVXSTR_TBLAFMT_LO6_BOX_LIST_BLUE,
    RID_SVXSTR_TBLAFMT_LO6_BOX_LIST_GREEN,
    RID_SVXSTR_TBLAFMT_LO6_BOX_LIST_RED,
    RID_SVXSTR_TBLAFMT_LO6_BOX_LIST_YELLOW,
    RID_SVXSTR_TBLAFMT_LO6_ELEGANT,
    RID_SVXSTR_TBLAFMT_LO6_FINANCIAL,
    RID_SVXSTR_TBLAFMT_LO6_SIMPLE_GRID_COLUMNS,
    RID_SVXSTR_TBLAFMT_LO6_SIMPLE_GRID_ROWS,
    RID_SVXSTR_TBLAFMT_LO6_SIMPLE_LIST_SHADED
};

namespace sw
{

DocumentStylePoolManager::DocumentStylePoolManager( SwDoc& i_rSwdoc ) : m_rDoc( i_rSwdoc )
{
}

SwTextFormatColl* DocumentStylePoolManager::GetTextCollFromPool( sal_uInt16 nId, bool bRegardLanguage )
{
    OSL_ENSURE(
        (RES_POOLCOLL_TEXT_BEGIN <= nId && nId < RES_POOLCOLL_TEXT_END) ||
        (RES_POOLCOLL_LISTS_BEGIN <= nId && nId < RES_POOLCOLL_LISTS_END) ||
        (RES_POOLCOLL_EXTRA_BEGIN <= nId && nId < RES_POOLCOLL_EXTRA_END) ||
        (RES_POOLCOLL_REGISTER_BEGIN <= nId && nId < RES_POOLCOLL_REGISTER_END) ||
        (RES_POOLCOLL_DOC_BEGIN <= nId && nId < RES_POOLCOLL_DOC_END) ||
        (RES_POOLCOLL_HTML_BEGIN <= nId && nId < RES_POOLCOLL_HTML_END),
            "Wrong  AutoFormat Id" );

    SwTextFormatColl* pNewColl;
    sal_uInt16 nOutLvlBits = 0;

    for (size_t n = 0, nSize = m_rDoc.GetTextFormatColls()->size(); n < nSize; ++n)
    {
        pNewColl = (*m_rDoc.GetTextFormatColls())[ n ];
        if( nId == pNewColl->GetPoolFormatId() )
        {
            // in online we can have multiple languages, use translated name
            if (comphelper::LibreOfficeKit::isActive())
            {
                OUString aName;
                SwStyleNameMapper::GetUIName(nId, aName);
                if (!aName.isEmpty())
                    pNewColl->SetFormatName(aName);
            }

            return pNewColl;
        }

        if( pNewColl->IsAssignedToListLevelOfOutlineStyle())
            nOutLvlBits |= ( 1 << pNewColl->GetAssignedOutlineStyleLevel() );
    }

    // Didn't find it until here -> create anew
    TranslateId pResId;
    if (RES_POOLCOLL_TEXT_BEGIN <= nId && nId < RES_POOLCOLL_TEXT_END)
    {
        static_assert(SAL_N_ELEMENTS(STR_POOLCOLL_TEXT_ARY) == RES_POOLCOLL_TEXT_END - RES_POOLCOLL_TEXT_BEGIN, "### unexpected size!");
        pResId = STR_POOLCOLL_TEXT_ARY[nId - RES_POOLCOLL_TEXT_BEGIN];
    }
    else if (RES_POOLCOLL_LISTS_BEGIN <= nId && nId < RES_POOLCOLL_LISTS_END)
    {
        static_assert(SAL_N_ELEMENTS(STR_POOLCOLL_LISTS_ARY) == RES_POOLCOLL_LISTS_END - RES_POOLCOLL_LISTS_BEGIN, "### unexpected size!");
        pResId = STR_POOLCOLL_LISTS_ARY[nId - RES_POOLCOLL_LISTS_BEGIN];
    }
    else if (RES_POOLCOLL_EXTRA_BEGIN <= nId && nId < RES_POOLCOLL_EXTRA_END)
    {
        static_assert(SAL_N_ELEMENTS(STR_POOLCOLL_EXTRA_ARY) == RES_POOLCOLL_EXTRA_END - RES_POOLCOLL_EXTRA_BEGIN, "### unexpected size!");
        pResId = STR_POOLCOLL_EXTRA_ARY[nId - RES_POOLCOLL_EXTRA_BEGIN];
    }
    else if (RES_POOLCOLL_REGISTER_BEGIN <= nId && nId < RES_POOLCOLL_REGISTER_END)
    {
        static_assert(SAL_N_ELEMENTS(STR_POOLCOLL_REGISTER_ARY) == RES_POOLCOLL_REGISTER_END - RES_POOLCOLL_REGISTER_BEGIN, "### unexpected size!");
        pResId = STR_POOLCOLL_REGISTER_ARY[nId - RES_POOLCOLL_REGISTER_BEGIN];
    }
    else if (RES_POOLCOLL_DOC_BEGIN <= nId && nId < RES_POOLCOLL_DOC_END)
    {
        static_assert(SAL_N_ELEMENTS(STR_POOLCOLL_DOC_ARY) == RES_POOLCOLL_DOC_END - RES_POOLCOLL_DOC_BEGIN, "### unexpected size!");
        pResId = STR_POOLCOLL_DOC_ARY[nId - RES_POOLCOLL_DOC_BEGIN];
    }
    else if (RES_POOLCOLL_HTML_BEGIN <= nId && nId < RES_POOLCOLL_HTML_END)
    {
        static_assert(SAL_N_ELEMENTS(STR_POOLCOLL_HTML_ARY) == RES_POOLCOLL_HTML_END - RES_POOLCOLL_HTML_BEGIN, "### unexpected size!");
        pResId = STR_POOLCOLL_HTML_ARY[nId - RES_POOLCOLL_HTML_BEGIN];
    }

    OSL_ENSURE(pResId, "Invalid Pool ID");
    if (!pResId)
        return GetTextCollFromPool(RES_POOLCOLL_STANDARD);

    OUString aNm(SwResId(pResId));

    // A Set for all to-be-set Attributes
    SwAttrSet aSet( m_rDoc.GetAttrPool(), aTextFormatCollSetRange );
    sal_uInt16 nParent = GetPoolParent( nId );

    {

        if(::IsConditionalByPoolId( nId ))
            pNewColl = new SwConditionTextFormatColl( m_rDoc.GetAttrPool(), aNm, !nParent
                                                ? m_rDoc.GetDfltTextFormatColl()
                                                : GetTextCollFromPool( nParent ));
        else
            pNewColl = new SwTextFormatColl( m_rDoc.GetAttrPool(), aNm, !nParent
                                            ? m_rDoc.GetDfltTextFormatColl()
                                            : GetTextCollFromPool( nParent ));
        pNewColl->SetPoolFormatId( nId );
        m_rDoc.GetTextFormatColls()->push_back( pNewColl );
    }

    bool bNoDefault = m_rDoc.GetDocumentSettingManager().get( DocumentSettingId::STYLES_NODEFAULT );
    if ( !bNoDefault )
    {
        switch( nId )
        {
        // General content forms
        case RES_POOLCOLL_STANDARD:
            /* koreans do not like SvxScriptItem(TRUE) */
            if (bRegardLanguage)
            {
                LanguageType nAppLanguage = GetAppLanguage();
                if (GetDefaultFrameDirection(nAppLanguage) ==
                    SvxFrameDirection::Horizontal_RL_TB)
                {
                    SvxAdjustItem aAdjust(SvxAdjust::Right, RES_PARATR_ADJUST );
                    aSet.Put(aAdjust);
                }
                if (nAppLanguage == LANGUAGE_KOREAN)
                {
                    SvxScriptSpaceItem aScriptSpace(false, RES_PARATR_SCRIPTSPACE);
                    aSet.Put(aScriptSpace);
                }
            }
            break;

        case RES_POOLCOLL_TEXT:                 // Text body
            {
                SvxLineSpacingItem aLSpc( LINE_SPACE_DEFAULT_HEIGHT, RES_PARATR_LINESPACING );
                SvxULSpaceItem aUL( 0, PT_7, RES_UL_SPACE );
                aLSpc.SetPropLineSpace( 115 );
                if( m_rDoc.GetDocumentSettingManager().get(DocumentSettingId::HTML_MODE) ) aUL.SetLower( HTML_PARSPACE );
                aSet.Put( aUL );
                aSet.Put( aLSpc );
            }
            break;
        case RES_POOLCOLL_TEXT_IDENT:           // Text body indentation
            {
                auto const first(o3tl::convert(5, o3tl::Length::mm, o3tl::Length::twip));
                SvxFirstLineIndentItem const firstLine(first, RES_MARGIN_FIRSTLINE);
                SvxTextLeftMarginItem const leftMargin(0, RES_MARGIN_TEXTLEFT);
                aSet.Put(firstLine);
                aSet.Put(leftMargin);
            }
            break;
        case RES_POOLCOLL_TEXT_NEGIDENT:        // Text body neg. indentation
            {
                auto const first(-o3tl::convert(5, o3tl::Length::mm, o3tl::Length::twip));
                auto const left(o3tl::convert(1, o3tl::Length::cm, o3tl::Length::twip));
                SvxFirstLineIndentItem const firstLine(first, RES_MARGIN_FIRSTLINE);
                SvxTextLeftMarginItem const leftMargin(left, RES_MARGIN_TEXTLEFT);
                aSet.Put(firstLine);
                aSet.Put(leftMargin);

                SvxTabStopItem aTStops(RES_PARATR_TABSTOP);
                aTStops.Insert( SvxTabStop( 0 ));
                aSet.Put( aTStops );
            }
            break;
        case RES_POOLCOLL_TEXT_MOVE:            // Text body move
            {
                auto const left(o3tl::convert(5, o3tl::Length::mm, o3tl::Length::twip));
                SvxFirstLineIndentItem const firstLine(0, RES_MARGIN_FIRSTLINE);
                SvxTextLeftMarginItem const leftMargin(left, RES_MARGIN_TEXTLEFT);
                aSet.Put(firstLine);
                aSet.Put(leftMargin);
            }
            break;

        case RES_POOLCOLL_CONFRONTATION:    // Text body confrontation
            {
                auto const first(-o3tl::convert(45, o3tl::Length::mm, o3tl::Length::twip));
                auto const left(o3tl::convert(5, o3tl::Length::cm, o3tl::Length::twip));
                SvxFirstLineIndentItem const firstLine(first, RES_MARGIN_FIRSTLINE);
                SvxTextLeftMarginItem const leftMargin(left, RES_MARGIN_TEXTLEFT);
                aSet.Put(firstLine);
                aSet.Put(leftMargin);

                SvxTabStopItem aTStops( RES_PARATR_TABSTOP );
                aTStops.Insert( SvxTabStop( 0 ));
                aSet.Put( aTStops );
            }
            break;
        case RES_POOLCOLL_MARGINAL:         // Text body marginal
            {
                auto const left(o3tl::convert(4, o3tl::Length::cm, o3tl::Length::twip));
                SvxFirstLineIndentItem const firstLine(0, RES_MARGIN_FIRSTLINE);
                SvxTextLeftMarginItem const leftMargin(left, RES_MARGIN_TEXTLEFT);
                aSet.Put(firstLine);
                aSet.Put(leftMargin);
            }
            break;

        case RES_POOLCOLL_HEADLINE_BASE:            // Base headline
            {
                static const sal_uInt16 aFontWhich[] =
                {   RES_CHRATR_FONT,
                    RES_CHRATR_CJK_FONT,
                    RES_CHRATR_CTL_FONT
                };
                static const sal_uInt16 aLangTypes[] =
                {
                    RES_CHRATR_LANGUAGE,
                    RES_CHRATR_CJK_LANGUAGE,
                    RES_CHRATR_CTL_LANGUAGE
                };
                static const LanguageType aLangs[] =
                {
                    LANGUAGE_ENGLISH_US,
                    LANGUAGE_ENGLISH_US,
                    LANGUAGE_ARABIC_SAUDI_ARABIA
                };
                static const DefaultFontType nFontTypes[] =
                {
                    DefaultFontType::LATIN_HEADING,
                    DefaultFontType::CJK_HEADING,
                    DefaultFontType::CTL_HEADING
                };

                for( int i = 0; i < 3; ++i )
                {
                    LanguageType nLng = static_cast<const SvxLanguageItem&>(m_rDoc.GetDefault( aLangTypes[i] )).GetLanguage();
                    if( LANGUAGE_DONTKNOW == nLng )
                        nLng = aLangs[i];

                    vcl::Font aFnt( OutputDevice::GetDefaultFont( nFontTypes[i],
                                            nLng, GetDefaultFontFlags::OnlyOne ) );

                    aSet.Put( SvxFontItem( aFnt.GetFamilyType(), aFnt.GetFamilyName(),
                                            OUString(), aFnt.GetPitch(),
                                            aFnt.GetCharSet(), aFontWhich[i] ));
                }

                SvxFontHeightItem aFntSize( PT_14, 100, RES_CHRATR_FONTSIZE );
                SvxULSpaceItem aUL( PT_12, PT_6, RES_UL_SPACE );
                if( m_rDoc.GetDocumentSettingManager().get(DocumentSettingId::HTML_MODE) )
                    aUL.SetLower( HTML_PARSPACE );
                aSet.Put( SvxFormatKeepItem( true, RES_KEEP ));

                pNewColl->SetNextTextFormatColl( *GetTextCollFromPool( RES_POOLCOLL_TEXT ));

                aSet.Put( aUL );
                SetAllScriptItem( aSet, aFntSize );
            }
            break;

        case RES_POOLCOLL_NUMBER_BULLET_BASE:          // Base Numbering
            break;

        case RES_POOLCOLL_GREETING:             // Greeting
        case RES_POOLCOLL_REGISTER_BASE:        // Base indexes
        case RES_POOLCOLL_SIGNATURE:            // Signatures
        case RES_POOLCOLL_TABLE:                // Table content
            {
                SwFormatLineNumber aLN;
                aLN.SetCountLines( false );
                aSet.Put( aLN );
                if (nId == RES_POOLCOLL_TABLE)
                {
                    aSet.Put( SvxWidowsItem( 0, RES_PARATR_WIDOWS ) );
                    aSet.Put( SvxOrphansItem( 0, RES_PARATR_ORPHANS ) );
                }
            }
            break;

        case RES_POOLCOLL_HEADLINE1:        // Heading 1
            {
                SvxULSpaceItem aUL( PT_12, PT_6, RES_UL_SPACE );
                aSet.Put( aUL );
                lcl_SetHeadline( m_rDoc, pNewColl, aSet, nOutLvlBits, 0, false );
            }
            break;
        case RES_POOLCOLL_HEADLINE2:        // Heading 2
            {
                SvxULSpaceItem aUL( PT_10, PT_6, RES_UL_SPACE );
                aSet.Put( aUL );
                lcl_SetHeadline( m_rDoc, pNewColl, aSet, nOutLvlBits, 1, false );
            }
            break;
        case RES_POOLCOLL_HEADLINE3:        // Heading 3
            {
                SvxULSpaceItem aUL( PT_7, PT_6, RES_UL_SPACE );
                aSet.Put( aUL );
                lcl_SetHeadline( m_rDoc, pNewColl, aSet, nOutLvlBits, 2, false );
            }
            break;
        case RES_POOLCOLL_HEADLINE4:        // Heading 4
            {
                SvxULSpaceItem aUL( PT_6, PT_6, RES_UL_SPACE );
                aSet.Put( aUL );
                lcl_SetHeadline( m_rDoc, pNewColl, aSet, nOutLvlBits, 3, true );
            }
            break;
        case RES_POOLCOLL_HEADLINE5:        // Heading 5
            {
                SvxULSpaceItem aUL( PT_6, PT_3, RES_UL_SPACE );
                aSet.Put( aUL );
                lcl_SetHeadline( m_rDoc, pNewColl, aSet, nOutLvlBits, 4, false );
            }
            break;
        case RES_POOLCOLL_HEADLINE6:        // Heading 6
            {
                SvxULSpaceItem aUL( PT_3, PT_3, RES_UL_SPACE );
                aSet.Put( aUL );
                lcl_SetHeadline( m_rDoc, pNewColl, aSet, nOutLvlBits, 5, true );
            }
            break;
        case RES_POOLCOLL_HEADLINE7:        // Heading 7
            {
                SvxULSpaceItem aUL( PT_3, PT_3, RES_UL_SPACE );
                aSet.Put( aUL );
                lcl_SetHeadline( m_rDoc, pNewColl, aSet, nOutLvlBits, 6, false );
            }
            break;
        case RES_POOLCOLL_HEADLINE8:        // Heading 8
            {
                SvxULSpaceItem aUL( PT_3, PT_3, RES_UL_SPACE );
                aSet.Put( aUL );
                lcl_SetHeadline( m_rDoc, pNewColl, aSet, nOutLvlBits, 7, true );
            }
            break;
        case RES_POOLCOLL_HEADLINE9:        // Heading 9
            {
                SvxULSpaceItem aUL( PT_3, PT_3, RES_UL_SPACE );
                aSet.Put( aUL );
                lcl_SetHeadline( m_rDoc, pNewColl, aSet, nOutLvlBits, 8, false );
            }
            break;
        case RES_POOLCOLL_HEADLINE10:       // Heading 10
            {
                SvxULSpaceItem aUL( PT_3, PT_3, RES_UL_SPACE );
                aSet.Put( aUL );
                lcl_SetHeadline( m_rDoc, pNewColl, aSet, nOutLvlBits, 9, false );
            }
            break;

        // Special sections:
        // Header
        case RES_POOLCOLL_HEADERFOOTER:
        case RES_POOLCOLL_HEADER:
        case RES_POOLCOLL_HEADERL:
        case RES_POOLCOLL_HEADERR:
        // Footer
        case RES_POOLCOLL_FOOTER:
        case RES_POOLCOLL_FOOTERL:
        case RES_POOLCOLL_FOOTERR:
            {
                SwFormatLineNumber aLN;
                aLN.SetCountLines( false );
                aSet.Put( aLN );

                tools::Long nRightMargin = lcl_GetRightMargin( m_rDoc );

                SvxTabStopItem aTStops( 0, 0, SvxTabAdjust::Default, RES_PARATR_TABSTOP );
                aTStops.Insert( SvxTabStop( nRightMargin / 2, SvxTabAdjust::Center ) );
                aTStops.Insert( SvxTabStop( nRightMargin, SvxTabAdjust::Right ) );

                aSet.Put( aTStops );

                if ( (nId==RES_POOLCOLL_HEADERR) || (nId==RES_POOLCOLL_FOOTERR) ) {
                    SvxAdjustItem aAdjust(SvxAdjust::Right, RES_PARATR_ADJUST );
                    aSet.Put(aAdjust);
                }
            }
            break;

        case RES_POOLCOLL_TABLE_HDLN:
            {
                SetAllScriptItem( aSet, SvxWeightItem( WEIGHT_BOLD, RES_CHRATR_WEIGHT ) );
                aSet.Put( SvxAdjustItem( SvxAdjust::Center, RES_PARATR_ADJUST ) );
                SwFormatLineNumber aLN;
                aLN.SetCountLines( false );
                aSet.Put( aLN );
            }
            break;

        case RES_POOLCOLL_FOOTNOTE:             // paragraph style Footnote
        case RES_POOLCOLL_ENDNOTE:              // paragraph style Endnote
            {
                auto const first(-o3tl::convert(6, o3tl::Length::mm, o3tl::Length::twip));
                auto const left(o3tl::convert(6, o3tl::Length::mm, o3tl::Length::twip));
                SvxFirstLineIndentItem const firstLine(first, RES_MARGIN_FIRSTLINE);
                SvxTextLeftMarginItem const leftMargin(left, RES_MARGIN_TEXTLEFT);
                aSet.Put(firstLine);
                aSet.Put(leftMargin);

                SetAllScriptItem( aSet, SvxFontHeightItem( PT_10, 100, RES_CHRATR_FONTSIZE ) );
                SwFormatLineNumber aLN;
                aLN.SetCountLines( false );
                aSet.Put( aLN );
            }
            break;

        case RES_POOLCOLL_LABEL:                // basic caption
            {
                SvxULSpaceItem aUL( RES_UL_SPACE );
                aUL.SetUpper( PT_6 );
                aUL.SetLower( PT_6 );
                aSet.Put( aUL );
                SetAllScriptItem( aSet, SvxPostureItem( ITALIC_NORMAL, RES_CHRATR_POSTURE ) );
                SetAllScriptItem( aSet, SvxFontHeightItem( PT_10, 100, RES_CHRATR_FONTSIZE ) );
                SwFormatLineNumber aLN;
                aLN.SetCountLines( false );
                aSet.Put( aLN );
            }
            break;

        case RES_POOLCOLL_FRAME:                // Frame content
        case RES_POOLCOLL_LABEL_ABB:            // caption image
        case RES_POOLCOLL_LABEL_TABLE:          // caption table
        case RES_POOLCOLL_LABEL_FRAME:          // caption frame
        case RES_POOLCOLL_LABEL_DRAWING:        // caption drawing
        case RES_POOLCOLL_LABEL_FIGURE:
            break;

        case RES_POOLCOLL_ENVELOPE_ADDRESS:          // envelope address
            {
                SvxULSpaceItem aUL( RES_UL_SPACE );
                aUL.SetLower( PT_3 );
                aSet.Put( aUL );
                SwFormatLineNumber aLN;
                aLN.SetCountLines( false );
                aSet.Put( aLN );
            }
            break;

        case RES_POOLCOLL_SEND_ADDRESS:           // Sender address
            {
                if( m_rDoc.GetDocumentSettingManager().get(DocumentSettingId::HTML_MODE) )
                    SetAllScriptItem( aSet, SvxPostureItem(ITALIC_NORMAL, RES_CHRATR_POSTURE) );
                else
                {
                    SvxULSpaceItem aUL( RES_UL_SPACE ); aUL.SetLower( PT_3 );
                    aSet.Put( aUL );
                }
                SwFormatLineNumber aLN;
                aLN.SetCountLines( false );
                aSet.Put( aLN );
            }
            break;

        case RES_POOLCOLL_COMMENT:       // Comment
            {
                SetAllScriptItem(aSet, SvxFontHeightItem(PT_10, 100, RES_CHRATR_FONTSIZE));

                auto const aIndent(o3tl::convert(0.1, o3tl::Length::cm, o3tl::Length::twip));
                SvxTextLeftMarginItem const leftMargin(aIndent, RES_MARGIN_TEXTLEFT);
                SvxRightMarginItem const rightMargin(aIndent, RES_MARGIN_RIGHT);
                aSet.Put(leftMargin);
                aSet.Put(rightMargin);

                auto const aSpacing(o3tl::convert(0.1, o3tl::Length::cm, o3tl::Length::twip));
                SvxULSpaceItem topSpacing( aSpacing, 0, RES_UL_SPACE );
                aSet.Put(topSpacing);
            }
            break;

        // User defined indexes:
        case RES_POOLCOLL_TOX_USERH:            // Header
            lcl_SetRegister( m_rDoc, aSet, 0, true, false );
            {
                SwFormatLineNumber aLN;
                aLN.SetCountLines( false );
                aSet.Put( aLN );
            }
            break;
        case RES_POOLCOLL_TOX_USER1:            // 1st level
            lcl_SetRegister( m_rDoc, aSet, 0, false, true );
            break;
        case RES_POOLCOLL_TOX_USER2:            // 2nd level
            lcl_SetRegister( m_rDoc, aSet, 1, false, true );
            break;
        case RES_POOLCOLL_TOX_USER3:            // 3rd level
            lcl_SetRegister( m_rDoc, aSet, 2, false, true );
            break;
        case RES_POOLCOLL_TOX_USER4:            // 4th level
            lcl_SetRegister( m_rDoc, aSet, 3, false, true );
            break;
        case RES_POOLCOLL_TOX_USER5:            // 5th level
            lcl_SetRegister( m_rDoc, aSet, 4, false, true );
            break;
        case RES_POOLCOLL_TOX_USER6:            // 6th level
            lcl_SetRegister( m_rDoc, aSet, 5, false, true );
            break;
        case RES_POOLCOLL_TOX_USER7:            // 7th level
            lcl_SetRegister( m_rDoc, aSet, 6, false, true );
            break;
        case RES_POOLCOLL_TOX_USER8:            // 8th level
            lcl_SetRegister( m_rDoc, aSet, 7, false, true );
            break;
        case RES_POOLCOLL_TOX_USER9:            // 9th level
            lcl_SetRegister( m_rDoc, aSet, 8, false, true );
            break;
        case RES_POOLCOLL_TOX_USER10:           // 10th level
            lcl_SetRegister( m_rDoc, aSet, 9, false, true );
            break;

        // Index
        case RES_POOLCOLL_TOX_IDXH:         // Header
            lcl_SetRegister( m_rDoc, aSet, 0, true, false );
            {
                SwFormatLineNumber aLN;
                aLN.SetCountLines( false );
                aSet.Put( aLN );
            }
            break;
        case RES_POOLCOLL_TOX_IDX1:         // 1st level
            lcl_SetRegister( m_rDoc, aSet, 0, false, false );
            break;
        case RES_POOLCOLL_TOX_IDX2:         // 2nd level
            lcl_SetRegister( m_rDoc, aSet, 1, false, false );
            break;
        case RES_POOLCOLL_TOX_IDX3:         // 3rd level
            lcl_SetRegister( m_rDoc, aSet, 2, false, false );
            break;
        case RES_POOLCOLL_TOX_IDXBREAK:     // Separator
            lcl_SetRegister( m_rDoc, aSet, 0, false, false );
            break;

        // Table of Content
        case RES_POOLCOLL_TOX_CNTNTH:       // Header
            lcl_SetRegister( m_rDoc, aSet, 0, true, false );
            {
                SwFormatLineNumber aLN;
                aLN.SetCountLines( false );
                aSet.Put( aLN );
            }
            break;
        case RES_POOLCOLL_TOX_CNTNT1:       // 1st level
            lcl_SetRegister( m_rDoc, aSet, 0, false, true );
            break;
        case RES_POOLCOLL_TOX_CNTNT2:       // 2nd level
            lcl_SetRegister( m_rDoc, aSet, 1, false, true );
            break;
        case RES_POOLCOLL_TOX_CNTNT3:       // 3rd level
            lcl_SetRegister( m_rDoc, aSet, 2, false, true );
            break;
        case RES_POOLCOLL_TOX_CNTNT4:       // 4th level
            lcl_SetRegister( m_rDoc, aSet, 3, false, true );
            break;
        case RES_POOLCOLL_TOX_CNTNT5:       // 5th level
            lcl_SetRegister( m_rDoc, aSet, 4, false, true );
            break;
        case RES_POOLCOLL_TOX_CNTNT6:       // 6th level
            lcl_SetRegister( m_rDoc, aSet, 5, false, true );
            break;
        case RES_POOLCOLL_TOX_CNTNT7:       // 7th level
            lcl_SetRegister( m_rDoc, aSet, 6, false, true );
            break;
        case RES_POOLCOLL_TOX_CNTNT8:       // 8th level
            lcl_SetRegister( m_rDoc, aSet, 7, false, true );
            break;
        case RES_POOLCOLL_TOX_CNTNT9:       // 9th level
            lcl_SetRegister( m_rDoc, aSet, 8, false, true );
            break;
        case RES_POOLCOLL_TOX_CNTNT10:      // 10th level
            lcl_SetRegister( m_rDoc, aSet, 9, false, true );
            break;

        case RES_POOLCOLL_TOX_ILLUSH:
        case RES_POOLCOLL_TOX_OBJECTH:
        case RES_POOLCOLL_TOX_TABLESH:
        case RES_POOLCOLL_TOX_AUTHORITIESH:
            lcl_SetRegister( m_rDoc, aSet, 0, true, false );
            {
                SwFormatLineNumber aLN;
                aLN.SetCountLines( false );
                aSet.Put( aLN );
            }
            break;
        case RES_POOLCOLL_TOX_ILLUS1:
        case RES_POOLCOLL_TOX_OBJECT1:
        case RES_POOLCOLL_TOX_TABLES1:
        case RES_POOLCOLL_TOX_AUTHORITIES1:
            lcl_SetRegister( m_rDoc, aSet, 0, false, true );
        break;

        case RES_POOLCOLL_NUM_LEVEL1S:
            lcl_SetNumBul( m_rDoc, pNewColl, aSet, RES_POOLCOLL_NUM_LEVEL1,
                            lNumberFirstLineOffset, SwNumRule::GetNumIndent( 0 ),
                            PT_12, PT_6 );
            break;
        case RES_POOLCOLL_NUM_LEVEL1:
            lcl_SetNumBul( m_rDoc, pNewColl, aSet, RES_POOLCOLL_NUM_LEVEL1,
                            lNumberFirstLineOffset, SwNumRule::GetNumIndent( 0 ),
                            0, PT_6 );
            break;
        case RES_POOLCOLL_NUM_LEVEL1E:
            lcl_SetNumBul( m_rDoc, pNewColl, aSet, RES_POOLCOLL_NUM_LEVEL1,
                            lNumberFirstLineOffset, SwNumRule::GetNumIndent( 0 ),
                            0, PT_12 );
            break;
        case RES_POOLCOLL_NUM_NONUM1:
            lcl_SetNumBul( m_rDoc, pNewColl, aSet, RES_POOLCOLL_NUM_NONUM1,
                            0, SwNumRule::GetNumIndent( 0 ), 0, PT_6 );
            break;
        case RES_POOLCOLL_NUM_LEVEL2S:
            lcl_SetNumBul( m_rDoc, pNewColl, aSet, RES_POOLCOLL_NUM_LEVEL2,
                            lNumberFirstLineOffset, SwNumRule::GetNumIndent( 1 ),
                            PT_12, PT_6 );
            break;
        case RES_POOLCOLL_NUM_LEVEL2:
            lcl_SetNumBul( m_rDoc, pNewColl, aSet, RES_POOLCOLL_NUM_LEVEL2,
                            lNumberFirstLineOffset, SwNumRule::GetNumIndent( 1 ),
                            0, PT_6 );
            break;
        case RES_POOLCOLL_NUM_LEVEL2E:
            lcl_SetNumBul( m_rDoc, pNewColl, aSet, RES_POOLCOLL_NUM_LEVEL2,
                            lNumberFirstLineOffset, SwNumRule::GetNumIndent( 1 ),
                            0, PT_12 );
            break;
        case RES_POOLCOLL_NUM_NONUM2:
            lcl_SetNumBul( m_rDoc, pNewColl, aSet, RES_POOLCOLL_NUM_NONUM2,
                            0, SwNumRule::GetNumIndent( 1 ), 0, PT_6 );
            break;
        case RES_POOLCOLL_NUM_LEVEL3S:
            lcl_SetNumBul( m_rDoc, pNewColl, aSet, RES_POOLCOLL_NUM_LEVEL3,
                            lNumberFirstLineOffset, SwNumRule::GetNumIndent( 2 ),
                            PT_12, PT_6 );
            break;
        case RES_POOLCOLL_NUM_LEVEL3:
            lcl_SetNumBul( m_rDoc, pNewColl, aSet, RES_POOLCOLL_NUM_LEVEL3,
                            lNumberFirstLineOffset, SwNumRule::GetNumIndent( 2 ),
                            0, PT_6 );
            break;
        case RES_POOLCOLL_NUM_LEVEL3E:
            lcl_SetNumBul( m_rDoc, pNewColl, aSet, RES_POOLCOLL_NUM_LEVEL3,
                            lNumberFirstLineOffset, SwNumRule::GetNumIndent( 2 ),
                            0, PT_12 );
            break;
        case RES_POOLCOLL_NUM_NONUM3:
            lcl_SetNumBul( m_rDoc, pNewColl, aSet, RES_POOLCOLL_NUM_NONUM3,
                            0, SwNumRule::GetNumIndent( 2 ), 0, PT_6 );
            break;
        case RES_POOLCOLL_NUM_LEVEL4S:
            lcl_SetNumBul( m_rDoc, pNewColl, aSet, RES_POOLCOLL_NUM_LEVEL4,
                            lNumberFirstLineOffset, SwNumRule::GetNumIndent( 3 ),
                            PT_12, PT_6 );
            break;
        case RES_POOLCOLL_NUM_LEVEL4:
            lcl_SetNumBul( m_rDoc, pNewColl, aSet, RES_POOLCOLL_NUM_LEVEL4,
                            lNumberFirstLineOffset, SwNumRule::GetNumIndent( 3 ),
                            0, PT_6 );
            break;
        case RES_POOLCOLL_NUM_LEVEL4E:
            lcl_SetNumBul( m_rDoc, pNewColl, aSet, RES_POOLCOLL_NUM_LEVEL4,
                            lNumberFirstLineOffset, SwNumRule::GetNumIndent( 3 ),
                            0, PT_12 );
            break;
        case RES_POOLCOLL_NUM_NONUM4:
            lcl_SetNumBul( m_rDoc, pNewColl, aSet, RES_POOLCOLL_NUM_NONUM4,
                            0, SwNumRule::GetNumIndent( 3 ), 0, PT_6 );
            break;
        case RES_POOLCOLL_NUM_LEVEL5S:
            lcl_SetNumBul( m_rDoc, pNewColl, aSet, RES_POOLCOLL_NUM_LEVEL5,
                            lNumberFirstLineOffset, SwNumRule::GetNumIndent( 4 ),
                            PT_12, PT_6 );
            break;
        case RES_POOLCOLL_NUM_LEVEL5:
            lcl_SetNumBul( m_rDoc, pNewColl, aSet, RES_POOLCOLL_NUM_LEVEL5,
                            lNumberFirstLineOffset, SwNumRule::GetNumIndent( 4 ),
                            0, PT_6 );
            break;
        case RES_POOLCOLL_NUM_LEVEL5E:
            lcl_SetNumBul( m_rDoc, pNewColl, aSet, RES_POOLCOLL_NUM_LEVEL5,
                            lNumberFirstLineOffset, SwNumRule::GetNumIndent( 4 ),
                            0, PT_12 );
            break;
        case RES_POOLCOLL_NUM_NONUM5:
            lcl_SetNumBul( m_rDoc, pNewColl, aSet, RES_POOLCOLL_NUM_NONUM5,
                            0, SwNumRule::GetNumIndent( 4 ), 0, PT_6 );
            break;

        case RES_POOLCOLL_BULLET_LEVEL1S:
            lcl_SetNumBul( m_rDoc, pNewColl, aSet, RES_POOLCOLL_BULLET_LEVEL1,
                            lBulletFirstLineOffset, SwNumRule::GetBullIndent( 0 ),
                            PT_12, PT_6 );
            break;
        case RES_POOLCOLL_BULLET_LEVEL1:
            lcl_SetNumBul( m_rDoc, pNewColl, aSet, RES_POOLCOLL_BULLET_LEVEL1,
                            lBulletFirstLineOffset, SwNumRule::GetBullIndent( 0 ),
                            0, PT_6 );
            break;
        case RES_POOLCOLL_BULLET_LEVEL1E:
            lcl_SetNumBul( m_rDoc, pNewColl, aSet, RES_POOLCOLL_BULLET_LEVEL1,
                            lBulletFirstLineOffset, SwNumRule::GetBullIndent( 0 ),
                            0, PT_12 );
            break;
        case RES_POOLCOLL_BULLET_NONUM1:
            lcl_SetNumBul( m_rDoc, pNewColl, aSet, RES_POOLCOLL_BULLET_NONUM1,
                            0, SwNumRule::GetBullIndent( 0 ), 0, PT_6 );
            break;
        case RES_POOLCOLL_BULLET_LEVEL2S:
            lcl_SetNumBul( m_rDoc, pNewColl, aSet, RES_POOLCOLL_BULLET_LEVEL2,
                            lBulletFirstLineOffset, SwNumRule::GetBullIndent( 1 ),
                            PT_12, PT_6 );
            break;
        case RES_POOLCOLL_BULLET_LEVEL2:
            lcl_SetNumBul( m_rDoc, pNewColl, aSet, RES_POOLCOLL_BULLET_LEVEL2,
                            lBulletFirstLineOffset, SwNumRule::GetBullIndent( 1 ),
                            0, PT_6 );
            break;
        case RES_POOLCOLL_BULLET_LEVEL2E:
            lcl_SetNumBul( m_rDoc, pNewColl, aSet, RES_POOLCOLL_BULLET_LEVEL2,
                            lBulletFirstLineOffset, SwNumRule::GetBullIndent( 1 ),
                            0, PT_12 );
            break;
        case RES_POOLCOLL_BULLET_NONUM2:
            lcl_SetNumBul( m_rDoc, pNewColl, aSet, RES_POOLCOLL_BULLET_NONUM2,
                            0, SwNumRule::GetBullIndent( 1 ), 0, PT_6 );
            break;
        case RES_POOLCOLL_BULLET_LEVEL3S:
            lcl_SetNumBul( m_rDoc, pNewColl, aSet, RES_POOLCOLL_BULLET_LEVEL3,
                            lBulletFirstLineOffset, SwNumRule::GetBullIndent( 2 ),
                            PT_12, PT_6 );
            break;
        case RES_POOLCOLL_BULLET_LEVEL3:
            lcl_SetNumBul( m_rDoc, pNewColl, aSet, RES_POOLCOLL_BULLET_LEVEL3,
                            lBulletFirstLineOffset, SwNumRule::GetBullIndent( 2 ),
                            0, PT_6 );
            break;
        case RES_POOLCOLL_BULLET_LEVEL3E:
            lcl_SetNumBul( m_rDoc, pNewColl, aSet, RES_POOLCOLL_BULLET_LEVEL3,
                            lBulletFirstLineOffset, SwNumRule::GetBullIndent( 2 ),
                            0, PT_12 );
            break;
        case RES_POOLCOLL_BULLET_NONUM3:
            lcl_SetNumBul( m_rDoc, pNewColl, aSet, RES_POOLCOLL_BULLET_NONUM3,
                            0, SwNumRule::GetBullIndent( 2 ), 0, PT_6 );
            break;
        case RES_POOLCOLL_BULLET_LEVEL4S:
            lcl_SetNumBul( m_rDoc, pNewColl, aSet, RES_POOLCOLL_BULLET_LEVEL4,
                            lBulletFirstLineOffset, SwNumRule::GetBullIndent( 3 ),
                            PT_12, PT_6 );
            break;
        case RES_POOLCOLL_BULLET_LEVEL4:
            lcl_SetNumBul( m_rDoc, pNewColl, aSet, RES_POOLCOLL_BULLET_LEVEL4,
                            lBulletFirstLineOffset, SwNumRule::GetBullIndent( 3 ),
                            0, PT_6 );
            break;
        case RES_POOLCOLL_BULLET_LEVEL4E:
            lcl_SetNumBul( m_rDoc, pNewColl, aSet, RES_POOLCOLL_BULLET_LEVEL4,
                            lBulletFirstLineOffset, SwNumRule::GetBullIndent( 3 ),
                            0, PT_12 );
            break;
        case RES_POOLCOLL_BULLET_NONUM4:
            lcl_SetNumBul( m_rDoc, pNewColl, aSet, RES_POOLCOLL_BULLET_NONUM4,
                            0, SwNumRule::GetBullIndent( 3 ), 0, PT_6 );
            break;
        case RES_POOLCOLL_BULLET_LEVEL5S:
            lcl_SetNumBul( m_rDoc, pNewColl, aSet, RES_POOLCOLL_BULLET_LEVEL5,
                            lBulletFirstLineOffset, SwNumRule::GetBullIndent( 4 ),
                            PT_12, PT_6 );
            break;
        case RES_POOLCOLL_BULLET_LEVEL5:
            lcl_SetNumBul( m_rDoc, pNewColl, aSet, RES_POOLCOLL_BULLET_LEVEL5,
                            lBulletFirstLineOffset, SwNumRule::GetBullIndent( 4 ),
                            0, PT_6 );
            break;
        case RES_POOLCOLL_BULLET_LEVEL5E:
            lcl_SetNumBul( m_rDoc, pNewColl, aSet, RES_POOLCOLL_BULLET_LEVEL5,
                            lBulletFirstLineOffset, SwNumRule::GetBullIndent( 4 ),
                            0, PT_12 );
            break;
        case RES_POOLCOLL_BULLET_NONUM5:
            lcl_SetNumBul( m_rDoc, pNewColl, aSet, RES_POOLCOLL_BULLET_NONUM5,
                            0, SwNumRule::GetBullIndent( 4 ), 0, PT_6 );
            break;

        case RES_POOLCOLL_DOC_TITLE:            // Document Title
            {
                SetAllScriptItem( aSet, SvxWeightItem( WEIGHT_BOLD, RES_CHRATR_WEIGHT ) );
                SetAllScriptItem( aSet, SvxFontHeightItem( PT_28, 100, RES_CHRATR_FONTSIZE ) );

                aSet.Put( SvxAdjustItem( SvxAdjust::Center, RES_PARATR_ADJUST ) );

                pNewColl->SetNextTextFormatColl( *GetTextCollFromPool( RES_POOLCOLL_TEXT ));
            }
            break;

        case RES_POOLCOLL_DOC_SUBTITLE:         // Document subtitle
            {
                SvxULSpaceItem aUL( PT_3, PT_6, RES_UL_SPACE );
                aSet.Put( aUL );
                SetAllScriptItem( aSet, SvxFontHeightItem( PT_18, 100, RES_CHRATR_FONTSIZE ));

                aSet.Put( SvxAdjustItem( SvxAdjust::Center, RES_PARATR_ADJUST ));

                pNewColl->SetNextTextFormatColl( *GetTextCollFromPool( RES_POOLCOLL_TEXT ));
            }
            break;

        case RES_POOLCOLL_DOC_APPENDIX:         // Document Appendix tdf#114090
            {
                SetAllScriptItem( aSet, SvxWeightItem( WEIGHT_BOLD, RES_CHRATR_WEIGHT ) );
                SetAllScriptItem( aSet, SvxFontHeightItem( PT_16, 100, RES_CHRATR_FONTSIZE ) );

                aSet.Put( SvxAdjustItem( SvxAdjust::Center, RES_PARATR_ADJUST ) );

                pNewColl->SetNextTextFormatColl( *GetTextCollFromPool( RES_POOLCOLL_TEXT ));
            }
            break;

        case RES_POOLCOLL_HTML_BLOCKQUOTE:
            {
                auto const left(o3tl::convert(1, o3tl::Length::cm, o3tl::Length::twip));
                auto const right(o3tl::convert(1, o3tl::Length::cm, o3tl::Length::twip));
                SvxFirstLineIndentItem const firstLine(0, RES_MARGIN_FIRSTLINE);
                SvxTextLeftMarginItem const leftMargin(left, RES_MARGIN_TEXTLEFT);
                SvxRightMarginItem const rightMargin(right, RES_MARGIN_RIGHT);
                aSet.Put(firstLine);
                aSet.Put(leftMargin);
                aSet.Put(rightMargin);

                std::unique_ptr<SvxULSpaceItem> aUL(pNewColl->GetULSpace().Clone());
                aUL->SetLower( HTML_PARSPACE );
                aSet.Put(std::move(aUL));
            }
            break;

        case RES_POOLCOLL_HTML_PRE:
            {
                ::lcl_SetDfltFont( DefaultFontType::FIXED, aSet );

                // WORKAROUND: Set PRE to 10pt
                SetAllScriptItem( aSet, SvxFontHeightItem(PT_10, 100, RES_CHRATR_FONTSIZE) );

                // The lower paragraph distance is set explicitly (makes
                // assigning hard attributes easier)
                std::unique_ptr<SvxULSpaceItem> aULSpaceItem(pNewColl->GetULSpace().Clone());
                aULSpaceItem->SetLower( 0 );
                aSet.Put(std::move(aULSpaceItem));
            }
            break;

        case RES_POOLCOLL_HTML_HR:
            {
                SvxBoxItem aBox( RES_BOX );
                Color aColor( COL_GRAY );
                SvxBorderLine aNew(&aColor, 3, SvxBorderLineStyle::DOUBLE);
                aBox.SetLine( &aNew, SvxBoxItemLine::BOTTOM );

                aSet.Put( aBox );
                aSet.Put( SwParaConnectBorderItem( false ) );
                SetAllScriptItem( aSet, SvxFontHeightItem(120, 100, RES_CHRATR_FONTSIZE) );

                std::unique_ptr<SvxULSpaceItem> aUL;
                {
                    pNewColl->SetNextTextFormatColl( *GetTextCollFromPool( RES_POOLCOLL_TEXT ));
                    aUL.reset(pNewColl->GetULSpace().Clone());
                }
                aUL->SetLower( HTML_PARSPACE );
                aSet.Put(std::move(aUL));
                SwFormatLineNumber aLN;
                aLN.SetCountLines( false );
                aSet.Put( aLN );
            }
            break;

        case RES_POOLCOLL_HTML_DD:
            {
                // We indent by 1 cm. The IDs are always 2 away from each other!
                auto const left(o3tl::convert(1, o3tl::Length::cm, o3tl::Length::twip));
                SvxTextLeftMarginItem const leftMargin(left, RES_MARGIN_TEXTLEFT);
                aSet.Put(leftMargin);
            }
            break;
        case RES_POOLCOLL_HTML_DT:
            {
                {
                    pNewColl->SetNextTextFormatColl( *GetTextCollFromPool( RES_POOLCOLL_HTML_DD ));
                }
                // We indent by 0 cm. The IDs are always 2 away from each other!
                SvxTextLeftMarginItem const leftMargin(0, RES_MARGIN_TEXTLEFT);
                aSet.Put(leftMargin);
            }
            break;
        }
    }

    if( aSet.Count() )
        pNewColl->SetFormatAttr( aSet );
    return pNewColl;
}

/// Return the AutomaticFormat with the supplied Id. If it doesn't
/// exist, create it.
SwFormat* DocumentStylePoolManager::GetFormatFromPool( sal_uInt16 nId )
{
    SwFormat *pNewFormat = nullptr;
    SwFormat *pDeriveFormat = nullptr;

    SwFormatsBase* pArray[ 2 ];
    sal_uInt16 nArrCnt = 1;
    TranslateId pRCId;
    WhichRangesContainer const* pWhichRange;

    switch( nId & (COLL_GET_RANGE_BITS + POOLGRP_NOCOLLID) )
    {
    case POOLGRP_CHARFMT:
        {
            pArray[0] = m_rDoc.GetCharFormats();
            pDeriveFormat = m_rDoc.GetDfltCharFormat();
            pWhichRange = &aCharFormatSetRange;

            if (nId >= RES_POOLCHR_HTML_BEGIN && nId < RES_POOLCHR_HTML_END)
                pRCId = STR_POOLCHR_HTML_ARY[nId - RES_POOLCHR_HTML_BEGIN];
            else if (nId >= RES_POOLCHR_NORMAL_BEGIN && nId < RES_POOLCHR_NORMAL_END)
                pRCId = STR_POOLCHR_ARY[nId - RES_POOLCHR_BEGIN];
            else
            {
                // Fault: unknown Format, but a CharFormat -> return the first one
                OSL_ENSURE( false, "invalid Id" );
                pRCId = STR_POOLCHR_ARY[0];
            }
        }
        break;
    case POOLGRP_FRAMEFMT:
        {
            pArray[0] = m_rDoc.GetFrameFormats();
            pArray[1] = m_rDoc.GetSpzFrameFormats();
            pDeriveFormat = m_rDoc.GetDfltFrameFormat();
            nArrCnt = 2;
            pWhichRange = &aFrameFormatSetRange;

            // Fault: unknown Format, but a FrameFormat
            //             -> return the first one
            if( RES_POOLFRM_BEGIN > nId || nId >= RES_POOLFRM_END )
            {
                OSL_ENSURE( false, "invalid Id" );
                nId = RES_POOLFRM_BEGIN;
            }

            pRCId = STR_POOLFRM_ARY[nId - RES_POOLFRM_BEGIN];
        }
        break;

    default:
        // Fault, unknown Format
        OSL_ENSURE( nId, "invalid Id" );
        return nullptr;
    }
    OSL_ENSURE(pRCId, "invalid Id");

    while( nArrCnt-- )
        for( size_t n = 0; n < (*pArray[nArrCnt]).GetFormatCount(); ++n )
        {
            pNewFormat = (*pArray[ nArrCnt ] ).GetFormat( n );
            if( nId == pNewFormat->GetPoolFormatId() )
            {
                return pNewFormat;
            }
        }

    OUString aNm(SwResId(pRCId));
    SwAttrSet aSet(m_rDoc.GetAttrPool(), *pWhichRange);

    {
        bool bIsModified = m_rDoc.getIDocumentState().IsModified();

        {
            ::sw::UndoGuard const undoGuard(m_rDoc.GetIDocumentUndoRedo());
            switch (nId & (COLL_GET_RANGE_BITS + POOLGRP_NOCOLLID) )
            {
                case POOLGRP_CHARFMT:
                    pNewFormat = m_rDoc.MakeCharFormat_(aNm, pDeriveFormat, false, true);
                    break;
                case POOLGRP_FRAMEFMT:
                    pNewFormat = m_rDoc.MakeFrameFormat_(aNm, pDeriveFormat, false, true);
                    break;
                default:
                    break;
            }
        }

        if( !bIsModified )
            m_rDoc.getIDocumentState().ResetModified();
        pNewFormat->SetPoolFormatId( nId );
        pNewFormat->SetAuto(false);      // no AutoFormat
    }

    switch( nId )
    {
    case RES_POOLCHR_FOOTNOTE:              // Footnote
    case RES_POOLCHR_PAGENO:                // Page/Field
    case RES_POOLCHR_LABEL:                 // Label
    case RES_POOLCHR_DROPCAPS:              // Dropcaps
    case RES_POOLCHR_NUM_LEVEL:             // Numbering level
    case RES_POOLCHR_TOXJUMP:               // Table of contents jump
    case RES_POOLCHR_ENDNOTE:               // Endnote
    case RES_POOLCHR_LINENUM:               // Line numbering
        break;

    case RES_POOLCHR_ENDNOTE_ANCHOR:        // Endnote anchor
    case RES_POOLCHR_FOOTNOTE_ANCHOR:       // Footnote anchor
        {
            aSet.Put( SvxEscapementItem( DFLT_ESC_AUTO_SUPER, DFLT_ESC_PROP, RES_CHRATR_ESCAPEMENT ) );
        }
        break;

    case RES_POOLCHR_BULLET_LEVEL:             // Bullet character
        {
            const vcl::Font& rBulletFont = numfunc::GetDefBulletFont();
            SetAllScriptItem( aSet, SvxFontItem( rBulletFont.GetFamilyType(),
                        rBulletFont.GetFamilyName(), rBulletFont.GetStyleName(),
                        rBulletFont.GetPitch(), rBulletFont.GetCharSet(), RES_CHRATR_FONT ));
        }
        break;

    case RES_POOLCHR_INET_NORMAL:
        {
            aSet.Put( SvxColorItem( COL_BLUE, RES_CHRATR_COLOR ) );
            aSet.Put( SvxUnderlineItem( LINESTYLE_SINGLE, RES_CHRATR_UNDERLINE ) );
        }
        break;
    case RES_POOLCHR_INET_VISIT:
        {
            aSet.Put( SvxColorItem( COL_RED, RES_CHRATR_COLOR ) );
            aSet.Put( SvxUnderlineItem( LINESTYLE_SINGLE, RES_CHRATR_UNDERLINE ) );
        }
        break;
    case RES_POOLCHR_JUMPEDIT:
        {
            aSet.Put( SvxColorItem( COL_CYAN, RES_CHRATR_COLOR ) );
            aSet.Put( SvxUnderlineItem( LINESTYLE_DOTTED, RES_CHRATR_UNDERLINE ) );
            aSet.Put( SvxCaseMapItem( SvxCaseMap::SmallCaps, RES_CHRATR_CASEMAP ) );
        }
        break;

    case RES_POOLCHR_RUBYTEXT:
        {
            tools::Long nH = GetDfltAttr( RES_CHRATR_CJK_FONTSIZE )->GetHeight() / 2;
            SetAllScriptItem( aSet, SvxFontHeightItem( nH, 100, RES_CHRATR_FONTSIZE));
            aSet.Put(SvxUnderlineItem( LINESTYLE_NONE, RES_CHRATR_UNDERLINE ));
            aSet.Put(SvxEmphasisMarkItem( FontEmphasisMark::NONE, RES_CHRATR_EMPHASIS_MARK) );
        }
        break;

    case RES_POOLCHR_HTML_EMPHASIS:
    case RES_POOLCHR_HTML_CITATION:
    case RES_POOLCHR_HTML_VARIABLE:
        {
            SetAllScriptItem( aSet, SvxPostureItem( ITALIC_NORMAL, RES_CHRATR_POSTURE) );
        }
        break;

    case RES_POOLCHR_IDX_MAIN_ENTRY:
    case RES_POOLCHR_HTML_STRONG:
        {
            SetAllScriptItem( aSet, SvxWeightItem( WEIGHT_BOLD, RES_CHRATR_WEIGHT ));
        }
        break;

    case RES_POOLCHR_HTML_CODE:
    case RES_POOLCHR_HTML_SAMPLE:
    case RES_POOLCHR_HTML_KEYBOARD:
    case RES_POOLCHR_HTML_TELETYPE:
        {
            ::lcl_SetDfltFont( DefaultFontType::FIXED, aSet );
        }
        break;
    case RES_POOLCHR_VERT_NUM:
            aSet.Put( SvxCharRotateItem( 900_deg10, false, RES_CHRATR_ROTATE ) );
    break;

    case RES_POOLFRM_FRAME:
        {
            if ( m_rDoc.GetDocumentSettingManager().get(DocumentSettingId::HTML_MODE) )
            {
                aSet.Put( SwFormatAnchor( RndStdIds::FLY_AS_CHAR ));
                aSet.Put( SwFormatVertOrient( 0, text::VertOrientation::LINE_CENTER, text::RelOrientation::PRINT_AREA ) );
                aSet.Put( SwFormatSurround( css::text::WrapTextMode_NONE ) );
            }
            else
            {
                aSet.Put( SwFormatAnchor( RndStdIds::FLY_AT_PARA ));
                aSet.Put( SwFormatSurround( css::text::WrapTextMode_PARALLEL ) );
                aSet.Put( SwFormatHoriOrient( 0, text::HoriOrientation::CENTER, text::RelOrientation::PRINT_AREA ) );
                aSet.Put( SwFormatVertOrient( 0, text::VertOrientation::TOP, text::RelOrientation::PRINT_AREA ) );
                Color aCol( COL_BLACK );
                SvxBorderLine aLine( &aCol, SvxBorderLineWidth::Hairline );
                SvxBoxItem aBox( RES_BOX );
                aBox.SetLine( &aLine, SvxBoxItemLine::TOP );
                aBox.SetLine( &aLine, SvxBoxItemLine::BOTTOM );
                aBox.SetLine( &aLine, SvxBoxItemLine::LEFT );
                aBox.SetLine( &aLine, SvxBoxItemLine::RIGHT );
                aBox.SetAllDistances( 85 );
                aSet.Put( aBox );
                aSet.Put( SvxLRSpaceItem( 114, 114, 0, RES_LR_SPACE ) );
                aSet.Put( SvxULSpaceItem( 114, 114, RES_UL_SPACE ) );
            }

            // for styles of FlyFrames do not set the FillStyle to make it a derived attribute
            aSet.ClearItem(XATTR_FILLSTYLE);
        }
        break;
    case RES_POOLFRM_GRAPHIC:
    case RES_POOLFRM_OLE:
        {
            aSet.Put( SwFormatAnchor( RndStdIds::FLY_AT_PARA ));
            aSet.Put( SwFormatHoriOrient( 0, text::HoriOrientation::CENTER, text::RelOrientation::FRAME ));
            aSet.Put( SwFormatVertOrient( 0, text::VertOrientation::TOP, text::RelOrientation::FRAME ));
            aSet.Put( SwFormatSurround( css::text::WrapTextMode_DYNAMIC ));
        }
        break;
    case RES_POOLFRM_FORMEL:
        {
            aSet.Put( SwFormatAnchor( RndStdIds::FLY_AS_CHAR ) );
            aSet.Put( SwFormatVertOrient( 0, text::VertOrientation::CHAR_CENTER, text::RelOrientation::FRAME ) );
            aSet.Put( SvxLRSpaceItem( 0, 0, 0, RES_LR_SPACE ) );
        }
        break;
    case RES_POOLFRM_MARGINAL:
        {
            aSet.Put( SwFormatAnchor( RndStdIds::FLY_AT_PARA ));
            aSet.Put( SwFormatHoriOrient( 0, text::HoriOrientation::LEFT, text::RelOrientation::FRAME ));
            aSet.Put( SwFormatVertOrient( 0, text::VertOrientation::TOP, text::RelOrientation::FRAME ));
            aSet.Put( SwFormatSurround( css::text::WrapTextMode_PARALLEL ));
            // Set the default width to 3.5 cm, use the minimum value for the height
            aSet.Put(SwFormatFrameSize(SwFrameSize::Minimum,
                                       o3tl::toTwips(35, o3tl::Length::mm),
                                       o3tl::toTwips(5, o3tl::Length::mm)));
        }
        break;
    case RES_POOLFRM_WATERSIGN:
        {
            aSet.Put( SwFormatAnchor( RndStdIds::FLY_AT_PAGE ));
            aSet.Put( SwFormatHoriOrient( 0, text::HoriOrientation::CENTER, text::RelOrientation::FRAME ));
            aSet.Put( SwFormatVertOrient( 0, text::VertOrientation::CENTER, text::RelOrientation::FRAME ));
            aSet.Put( SvxOpaqueItem( RES_OPAQUE, false ));
            aSet.Put( SwFormatSurround( css::text::WrapTextMode_THROUGH ));
        }
        break;
    case RES_POOLFRM_LABEL:
        {
            aSet.Put( SwFormatAnchor( RndStdIds::FLY_AS_CHAR ) );
            aSet.Put( SwFormatVertOrient( 0, text::VertOrientation::TOP, text::RelOrientation::FRAME ) );
            aSet.Put( SvxLRSpaceItem( 114, 114, 0, RES_LR_SPACE ) );

            SvxProtectItem aProtect( RES_PROTECT );
            aProtect.SetSizeProtect( true );
            aProtect.SetPosProtect( true );
            aSet.Put( aProtect );

            pNewFormat->SetAutoUpdateOnDirectFormat();
        }
        break;
    }
    if( aSet.Count() )
    {
        pNewFormat->SetFormatAttr( aSet );
    }
    return pNewFormat;
}

SwFrameFormat* DocumentStylePoolManager::GetFrameFormatFromPool( sal_uInt16 nId )
{
    return static_cast<SwFrameFormat*>(GetFormatFromPool( nId ));
}

SwCharFormat* DocumentStylePoolManager::GetCharFormatFromPool( sal_uInt16 nId )
{
    return static_cast<SwCharFormat*>(GetFormatFromPool( nId ));
}

SwPageDesc* DocumentStylePoolManager::GetPageDescFromPool( sal_uInt16 nId, bool bRegardLanguage )
{
    OSL_ENSURE( RES_POOLPAGE_BEGIN <= nId && nId < RES_POOLPAGE_END,
            "Wrong AutoFormat Id" );

    for( size_t n = 0; n < m_rDoc.GetPageDescCnt(); ++n )
    {
        if ( nId == m_rDoc.GetPageDesc(n).GetPoolFormatId() )
        {
            return &m_rDoc.GetPageDesc(n);
        }
    }

    if( RES_POOLPAGE_BEGIN > nId ||  nId >= RES_POOLPAGE_END )
    {
        // unknown page pool ID
        OSL_ENSURE( false, "<SwDoc::GetPageDescFromPool(..)> - unknown page pool ID" );
        nId = RES_POOLPAGE_BEGIN;
    }

    SwPageDesc* pNewPgDsc = nullptr;
    {
        static_assert(SAL_N_ELEMENTS(STR_POOLPAGE_ARY) == RES_POOLPAGE_END - RES_POOLPAGE_BEGIN, "### unexpected size!");
        const OUString aNm(SwResId(STR_POOLPAGE_ARY[nId - RES_POOLPAGE_BEGIN]));
        const bool bIsModified = m_rDoc.getIDocumentState().IsModified();

        {
            ::sw::UndoGuard const undoGuard(m_rDoc.GetIDocumentUndoRedo());
            pNewPgDsc = m_rDoc.MakePageDesc(aNm, nullptr, bRegardLanguage);
        }

        pNewPgDsc->SetPoolFormatId( nId );
        if ( !bIsModified )
        {
            m_rDoc.getIDocumentState().ResetModified();
        }
    }

    SvxLRSpaceItem aLR( RES_LR_SPACE );
    {
        aLR.SetLeft(o3tl::convert(2, o3tl::Length::cm, o3tl::Length::twip));
        aLR.SetRight( aLR.GetLeft() );
    }
    SvxULSpaceItem aUL( RES_UL_SPACE );
    {
        aUL.SetUpper( o3tl::narrowing<sal_uInt16>(aLR.GetLeft()) );
        aUL.SetLower( o3tl::narrowing<sal_uInt16>(aLR.GetLeft()) );
    }

    SwAttrSet aSet( m_rDoc.GetAttrPool(), aPgFrameFormatSetRange );
    bool bSetLeft = true;

    switch( nId )
    {
    case RES_POOLPAGE_STANDARD:     // "Default"
        {
            aSet.Put( aLR );
            aSet.Put( aUL );
            pNewPgDsc->SetUseOn( UseOnPage::All | UseOnPage::FirstShare );
        }
        break;

    case RES_POOLPAGE_FIRST:        // "First Page"
    case RES_POOLPAGE_REGISTER:     // "Index"
        {
            lcl_PutStdPageSizeIntoItemSet( m_rDoc, aSet );
            aSet.Put( aLR );
            aSet.Put( aUL );
            pNewPgDsc->SetUseOn( UseOnPage::All );
            if( RES_POOLPAGE_FIRST == nId )
                pNewPgDsc->SetFollow( GetPageDescFromPool( RES_POOLPAGE_STANDARD ));
        }
        break;

    case RES_POOLPAGE_LEFT:         // "Left Page"
        {
            lcl_PutStdPageSizeIntoItemSet( m_rDoc, aSet );
            aSet.Put( aLR );
            aSet.Put( aUL );
            bSetLeft = false;
            pNewPgDsc->SetUseOn( UseOnPage::Left );
            // this relies on GetPageDescFromPool() not going into infinite recursion
            // (by this point RES_POOLPAGE_LEFT will not reach this place again)
            pNewPgDsc->SetFollow( GetPageDescFromPool( RES_POOLPAGE_RIGHT ));
        }
        break;
    case RES_POOLPAGE_RIGHT:        // "Right Page"
        {
            lcl_PutStdPageSizeIntoItemSet( m_rDoc, aSet );
            aSet.Put( aLR );
            aSet.Put( aUL );
            bSetLeft = false;
            pNewPgDsc->SetUseOn( UseOnPage::Right );
            pNewPgDsc->SetFollow( GetPageDescFromPool( RES_POOLPAGE_LEFT ));
        }
        break;

    case RES_POOLPAGE_ENVELOPE:        // "Envelope"
        {
            Size aPSize( SvxPaperInfo::GetPaperSize( PAPER_ENV_C65 ) );
            LandscapeSwap( aPSize );
            aSet.Put( SwFormatFrameSize( SwFrameSize::Fixed, aPSize.Width(), aPSize.Height() ));
            aLR.SetLeft( 0 ); aLR.SetRight( 0 );
            aUL.SetUpper( 0 ); aUL.SetLower( 0 );
            aSet.Put( aLR );
            aSet.Put( aUL );

            pNewPgDsc->SetUseOn( UseOnPage::All );
            pNewPgDsc->SetLandscape( true );
        }
        break;

    case RES_POOLPAGE_HTML:         // "HTML"
        {
            lcl_PutStdPageSizeIntoItemSet( m_rDoc, aSet );
            aLR.SetRight(o3tl::convert(1, o3tl::Length::cm, o3tl::Length::twip));
            aUL.SetUpper( o3tl::narrowing<sal_uInt16>(aLR.GetRight()) );
            aUL.SetLower( o3tl::narrowing<sal_uInt16>(aLR.GetRight()) );
            aSet.Put( aLR );
            aSet.Put( aUL );

            pNewPgDsc->SetUseOn( UseOnPage::All );
        }
        break;

    case RES_POOLPAGE_FOOTNOTE:     // "Footnote"
    case RES_POOLPAGE_ENDNOTE:      // "Endnote"
        {
            lcl_PutStdPageSizeIntoItemSet( m_rDoc, aSet );
            aSet.Put( aLR );
            aSet.Put( aUL );
            pNewPgDsc->SetUseOn( UseOnPage::All );
            SwPageFootnoteInfo aInf( pNewPgDsc->GetFootnoteInfo() );
            aInf.SetLineWidth( 0 );
            aInf.SetTopDist( 0 );
            aInf.SetBottomDist( 0 );
            pNewPgDsc->SetFootnoteInfo( aInf );
        }
        break;

    case RES_POOLPAGE_LANDSCAPE:    // "Landscape"
        {
            SwPageDesc* pStdPgDsc = GetPageDescFromPool( RES_POOLPAGE_STANDARD );
            SwFormatFrameSize aFrameSz( pStdPgDsc->GetMaster().GetFrameSize() );
            if ( !pStdPgDsc->GetLandscape() )
            {
                const SwTwips nTmp = aFrameSz.GetHeight();
                aFrameSz.SetHeight( aFrameSz.GetWidth() );
                aFrameSz.SetWidth( nTmp );
            }
            aSet.Put( aFrameSz );
            aSet.Put( aLR );
            aSet.Put( aUL );
            pNewPgDsc->SetUseOn( UseOnPage::All );
            pNewPgDsc->SetLandscape( true );
        }
        break;

    }

    if( aSet.Count() )
    {
        if( bSetLeft )
        {
            pNewPgDsc->GetLeft().SetFormatAttr( aSet );
            pNewPgDsc->GetFirstLeft().SetFormatAttr( aSet );
        }
        pNewPgDsc->GetMaster().SetFormatAttr( aSet );
        pNewPgDsc->GetFirstMaster().SetFormatAttr( aSet );
    }
    return pNewPgDsc;
}

SwNumRule* DocumentStylePoolManager::GetNumRuleFromPool( sal_uInt16 nId )
{
    OSL_ENSURE( RES_POOLNUMRULE_BEGIN <= nId && nId < RES_POOLNUMRULE_END,
            "Wrong AutoFormat Id" );

    SwNumRule* pNewRule;

    for (size_t n = 0; n < m_rDoc.GetNumRuleTable().size(); ++n )
    {
        pNewRule = m_rDoc.GetNumRuleTable()[ n ];
        if (nId == pNewRule->GetPoolFormatId())
        {
            return pNewRule;
        }
    }

    // error: unknown Pool style
    if( RES_POOLNUMRULE_BEGIN > nId ||  nId >= RES_POOLNUMRULE_END )
    {
        OSL_ENSURE( false, "invalid Id" );
        nId = RES_POOLNUMRULE_BEGIN;
    }

    static_assert(SAL_N_ELEMENTS(STR_POOLNUMRULE_NUM_ARY) == RES_POOLNUMRULE_END - RES_POOLNUMRULE_BEGIN, "### unexpected size!");
    OUString aNm(SwResId(STR_POOLNUMRULE_NUM_ARY[nId - RES_POOLNUMRULE_BEGIN]));

    SwCharFormat *pNumCFormat = nullptr, *pBullCFormat = nullptr;

    const SvxNumberFormat::SvxNumPositionAndSpaceMode eNumberFormatPositionAndSpaceMode
                                  = numfunc::GetDefaultPositionAndSpaceMode(); //#i89178#
    {
        bool bIsModified = m_rDoc.getIDocumentState().IsModified();

        sal_uInt16 n = m_rDoc.MakeNumRule( aNm, nullptr, false, eNumberFormatPositionAndSpaceMode );

        pNewRule = m_rDoc.GetNumRuleTable()[ n ];
        pNewRule->SetPoolFormatId( nId );
        pNewRule->SetAutoRule( false );

        if( RES_POOLNUMRULE_NUM1 <= nId && nId <= RES_POOLNUMRULE_NUM5 )
            pNumCFormat = GetCharFormatFromPool( RES_POOLCHR_NUM_LEVEL );

        if( ( RES_POOLNUMRULE_BUL1 <= nId && nId <= RES_POOLNUMRULE_BUL5 ) ||
            RES_POOLNUMRULE_NUM5 == nId )
            pBullCFormat = GetCharFormatFromPool( RES_POOLCHR_NUM_LEVEL );

        if( !bIsModified )
            m_rDoc.getIDocumentState().ResetModified();
    }

    switch( nId )
    {
    case RES_POOLNUMRULE_NUM1:
        {
            SwNumFormat aFormat;
            aFormat.SetPositionAndSpaceMode( eNumberFormatPositionAndSpaceMode );
            aFormat.SetNumberingType(SVX_NUM_ARABIC);
            aFormat.SetCharFormat( pNumCFormat );
            aFormat.SetStart( 1 );
            aFormat.SetIncludeUpperLevels( 1 );

            static const sal_uInt16 aAbsSpace[ MAXLEVEL ] =
                {
//              cm: 0.7 cm intervals, with 1 cm = 567
                    397, 794, 1191, 1588, 1985, 2381, 2778, 3175, 3572, 3969
                };
            const sal_uInt16* pArr = aAbsSpace;

            if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_WIDTH_AND_POSITION )
            {
                aFormat.SetFirstLineOffset( - (*pArr) );
            }
            else if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_ALIGNMENT )
            {
                aFormat.SetLabelFollowedBy( SvxNumberFormat::LISTTAB );
                aFormat.SetFirstLineIndent( - (*pArr) );
            }

            for (sal_uInt16 n = 0; n < MAXLEVEL; ++n, ++pArr)
            {
                aFormat.SetListFormat(u""_ustr, u"."_ustr, n);
                if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_WIDTH_AND_POSITION )
                {
                    aFormat.SetAbsLSpace( *pArr + 357 ); // 357 is indent of 0.63 cm
                }
                else if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_ALIGNMENT )
                {
                    aFormat.SetListtabPos( *pArr );
                    aFormat.SetIndentAt( *pArr + 357 );
                }

                pNewRule->Set( n, aFormat );
            }
        }
        break;

    case RES_POOLNUMRULE_NUM2:
        {
            static const sal_uInt16 aAbsSpace[ MAXLEVEL ] =
                {
                    397,  397,  397,  397,      // 0.70 cm intervals
                    397, 397, 397, 397,
                   397, 397
                };

            const sal_uInt16* pArr = aAbsSpace;
            SwNumFormat aFormat;

            aFormat.SetPositionAndSpaceMode( eNumberFormatPositionAndSpaceMode );
            aFormat.SetNumberingType(SVX_NUM_CHARS_UPPER_LETTER);
            aFormat.SetCharFormat( pNumCFormat );
            aFormat.SetStart( 1 );
            aFormat.SetIncludeUpperLevels( 1 );

            if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_ALIGNMENT )
            {
                aFormat.SetLabelFollowedBy( SvxNumberFormat::LISTTAB );
            }

            sal_uInt16 nSpace = 357; // indent of 0.63 cm
            for (sal_uInt16 n = 0; n < MAXLEVEL; ++n)
            {
                aFormat.SetListFormat(u""_ustr, u"."_ustr, n);
                if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_WIDTH_AND_POSITION )
                {
                    nSpace += pArr[ n ];
                    aFormat.SetAbsLSpace( nSpace );
                    aFormat.SetFirstLineOffset( - pArr[ n ] );
                }
                else if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_ALIGNMENT )
                {
                    nSpace += pArr[ n ];
                    aFormat.SetListtabPos( nSpace );
                    aFormat.SetIndentAt( nSpace );
                    aFormat.SetFirstLineIndent( - pArr[ n ] );
                }

                pNewRule->Set( n, aFormat );
            }
        }
        break;
    case RES_POOLNUMRULE_NUM3:
        {
            SwNumFormat aFormat;

            aFormat.SetPositionAndSpaceMode( eNumberFormatPositionAndSpaceMode );
            aFormat.SetNumberingType(SVX_NUM_CHARS_LOWER_LETTER);
            aFormat.SetCharFormat( pNumCFormat );
            aFormat.SetStart( 1 );
            aFormat.SetIncludeUpperLevels( 1 );

            tools::Long const nOffs = 397; // 0.70 cm

            if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_WIDTH_AND_POSITION )
            {
                aFormat.SetFirstLineOffset( - nOffs );
            }
            else if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_ALIGNMENT )
            {
                aFormat.SetLabelFollowedBy( SvxNumberFormat::LISTTAB );
                aFormat.SetFirstLineIndent( - nOffs );
            }

            for (sal_uInt16 n = 0; n < MAXLEVEL; ++n)
            {
                aFormat.SetListFormat(u""_ustr, u"."_ustr, n);
                if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_WIDTH_AND_POSITION )
                {
                    aFormat.SetAbsLSpace( (n+1) * nOffs + 357 ); // 357 is indent of 0.63 cm
                }
                else if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_ALIGNMENT )
                {
                    tools::Long nPos = (n+1) * nOffs;
                    aFormat.SetListtabPos(nPos + 357);
                    aFormat.SetIndentAt(nPos + 357);
                }

                pNewRule->Set( n, aFormat );
            }
        }
        break;
    case RES_POOLNUMRULE_NUM4:
        {
            SwNumFormat aFormat;

            aFormat.SetPositionAndSpaceMode( eNumberFormatPositionAndSpaceMode );
            aFormat.SetNumberingType(SVX_NUM_ROMAN_UPPER);
            aFormat.SetCharFormat( pNumCFormat );
            aFormat.SetStart( 1 );
            aFormat.SetIncludeUpperLevels( 1 );
            aFormat.SetNumAdjust( SvxAdjust::Right );

            static const sal_uInt16 aAbsSpace[ MAXLEVEL ] =
                {
//              cm: 1.33 cm intervals
                    754, 1508, 2262, 3016, 3771, 4525, 5279, 6033, 6787, 7541
                };
            const sal_uInt16* pArr = aAbsSpace;

            if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_WIDTH_AND_POSITION )
            {
                aFormat.SetFirstLineOffset( 580 - (*pArr) ); // 1 cm space
            }
            else if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_ALIGNMENT )
            {
                aFormat.SetLabelFollowedBy( SvxNumberFormat::LISTTAB );
                aFormat.SetFirstLineIndent( 580 - (*pArr) );
            }

            for (sal_uInt16 n = 0; n < MAXLEVEL; ++n, ++pArr)
            {
                aFormat.SetListFormat(u""_ustr, u"."_ustr, n);
                if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_WIDTH_AND_POSITION )
                {
                    aFormat.SetAbsLSpace( *pArr );
                }
                else if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_ALIGNMENT )
                {
                    aFormat.SetListtabPos( *pArr );
                    aFormat.SetIndentAt( *pArr );
                }

                pNewRule->Set( n, aFormat );
            }
        }
        break;
    case RES_POOLNUMRULE_NUM5:
        {
            // [ First, LSpace ]
            static const sal_uInt16 aAbsSpace0to2[] =
                {
                    174,  754,  // 0.33, 1.33cm
                    174,  1508, // 0.33, 2.66cm
                    397,  2262  // 0.70, 4.00cm
                };

            const sal_uInt16* pArr0to2 = aAbsSpace0to2;
            SwNumFormat aFormat;

            aFormat.SetPositionAndSpaceMode( eNumberFormatPositionAndSpaceMode );
            aFormat.SetNumberingType(SVX_NUM_ROMAN_LOWER);
            aFormat.SetStart( 1 );
            aFormat.SetIncludeUpperLevels( 1 );
            aFormat.SetNumAdjust( SvxAdjust::Right );
            aFormat.SetListFormat(u""_ustr, u"."_ustr, 0);

            if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_ALIGNMENT )
            {
                aFormat.SetLabelFollowedBy( SvxNumberFormat::LISTTAB );
            }

            if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_WIDTH_AND_POSITION )
            {
                aFormat.SetFirstLineOffset(- pArr0to2[0]); // num ends at 1.00 cm
                aFormat.SetAbsLSpace(pArr0to2[1]);         // text starts at 1.33 cm
            }
            else if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_ALIGNMENT )
            {
                aFormat.SetFirstLineIndent( -pArr0to2[0] );
                aFormat.SetListtabPos( pArr0to2[1] );
                aFormat.SetIndentAt( pArr0to2[1] );
            }

            aFormat.SetCharFormat( pNumCFormat );
            pNewRule->Set( 0, aFormat );

            aFormat.SetNumberingType(SVX_NUM_ROMAN_UPPER);
            aFormat.SetIncludeUpperLevels( 1 );
            aFormat.SetListFormat(u""_ustr, u"."_ustr, 1);

            if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_WIDTH_AND_POSITION )
            {
                aFormat.SetFirstLineOffset(- pArr0to2[2]); // num ends at 2.33 cm
                aFormat.SetAbsLSpace(pArr0to2[3]);         // text starts at 2.66 cm
            }
            else if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_ALIGNMENT )
            {
                aFormat.SetFirstLineIndent( -pArr0to2[2] );
                aFormat.SetListtabPos( pArr0to2[3] );
                aFormat.SetIndentAt( pArr0to2[3] );
            }

            pNewRule->Set( 1, aFormat );

            aFormat.SetNumberingType(SVX_NUM_CHARS_LOWER_LETTER);
            aFormat.SetIncludeUpperLevels( 1 );
            aFormat.SetListFormat(u""_ustr, u")"_ustr, 2);
            aFormat.SetNumAdjust( SvxAdjust::Left );

            if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_WIDTH_AND_POSITION )
            {
                aFormat.SetFirstLineOffset(- pArr0to2[4]); // num starts at 3.30 cm
                aFormat.SetAbsLSpace(pArr0to2[5]);         // text starts at 4.00 cm
            }
            else if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_ALIGNMENT )
            {
                aFormat.SetFirstLineIndent( -pArr0to2[4] );
                aFormat.SetListtabPos( pArr0to2[5] );
                aFormat.SetIndentAt( pArr0to2[5] );
            }

            pNewRule->Set( 2, aFormat );

            aFormat.SetNumberingType(SVX_NUM_CHAR_SPECIAL);
            aFormat.SetCharFormat( pBullCFormat );
            aFormat.SetBulletFont(  &numfunc::GetDefBulletFont() );
            aFormat.SetBulletChar( cBulletChar );
            sal_Int16 nOffs = o3tl::convert(660, o3tl::Length::mm100, o3tl::Length::twip),
                      nOffs2 = o3tl::convert(4000, o3tl::Length::mm100, o3tl::Length::twip);

            if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_WIDTH_AND_POSITION )
            {
                aFormat.SetFirstLineOffset( - nOffs );
            }
            else if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_ALIGNMENT )
            {
                aFormat.SetFirstLineIndent( - nOffs );
            }

            for (sal_uInt16 n = 3; n < MAXLEVEL; ++n)
            {
                aFormat.SetStart( n+1 );
                aFormat.SetListFormat(u""_ustr, u""_ustr, n);

                if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_WIDTH_AND_POSITION )
                {
                    aFormat.SetAbsLSpace(nOffs2 + ((n - 2) * static_cast<tools::Long>(nOffs)));
                }
                else if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_ALIGNMENT )
                {
                    tools::Long nPos = nOffs2 + ((n - 2) * static_cast<tools::Long>(nOffs));
                    aFormat.SetListtabPos(nPos);
                    aFormat.SetIndentAt(nPos);
                }

                pNewRule->Set( n, aFormat );
            }
        }
        break;

    case RES_POOLNUMRULE_BUL1:
        {
            SwNumFormat aFormat;

            aFormat.SetPositionAndSpaceMode( eNumberFormatPositionAndSpaceMode );
            aFormat.SetNumberingType(SVX_NUM_CHAR_SPECIAL);
            aFormat.SetCharFormat( pBullCFormat );
            aFormat.SetStart( 1 );
            aFormat.SetIncludeUpperLevels( 1 );
            aFormat.SetBulletFont( &numfunc::GetDefBulletFont() );
            aFormat.SetBulletChar( cBulletChar );

            static const sal_uInt16 aAbsSpace[ MAXLEVEL ] =
                {
//              cm: 0,4  0,8  1,2  1,6  2,0   2,4   2,8   3,2   3,6   4,0
                    227, 454, 680, 907, 1134, 1361, 1587, 1814, 2041, 2268
                };
            const sal_uInt16* pArr = aAbsSpace;

            if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_WIDTH_AND_POSITION )
            {
                aFormat.SetFirstLineOffset( - (*pArr) );
            }
            else if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_ALIGNMENT )
            {
                aFormat.SetLabelFollowedBy( SvxNumberFormat::LISTTAB );
                aFormat.SetFirstLineIndent( - (*pArr) );
            }

            for (sal_uInt16 n = 0; n < MAXLEVEL; ++n, ++pArr)
            {
                if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_WIDTH_AND_POSITION )
                {
                    aFormat.SetAbsLSpace( *pArr );
                }
                else if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_ALIGNMENT )
                {
                    aFormat.SetListtabPos( *pArr );
                    aFormat.SetIndentAt( *pArr );
                }

                pNewRule->Set( n, aFormat );
            }
        }
        break;
    case RES_POOLNUMRULE_BUL2:
        {
            SwNumFormat aFormat;

            aFormat.SetPositionAndSpaceMode( eNumberFormatPositionAndSpaceMode );
            aFormat.SetNumberingType(SVX_NUM_CHAR_SPECIAL);
            aFormat.SetCharFormat( pBullCFormat );
            aFormat.SetStart( 1 );
            aFormat.SetIncludeUpperLevels( 1 );
            aFormat.SetBulletFont(  &numfunc::GetDefBulletFont() );
            aFormat.SetBulletChar( 0x2013 );

            static const sal_uInt16 aAbsSpace[ MAXLEVEL ] =
                {
//              cm: 0,3  0,6  0,9  1,2  1,5  1,8   2,1   2,4   2,7   3,0
                    170, 340, 510, 680, 850, 1020, 1191, 1361, 1531, 1701
                };
            const sal_uInt16* pArr = aAbsSpace;

            if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_WIDTH_AND_POSITION )
            {
                aFormat.SetFirstLineOffset( - (*pArr) );
            }
            else if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_ALIGNMENT )
            {
                aFormat.SetLabelFollowedBy( SvxNumberFormat::LISTTAB );
                aFormat.SetFirstLineIndent( - (*pArr) );
            }

            for (sal_uInt16 n = 0; n < MAXLEVEL; ++n, ++pArr)
            {
                if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_WIDTH_AND_POSITION )
                {
                    aFormat.SetAbsLSpace( *pArr );
                }
                else if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_ALIGNMENT )
                {
                    aFormat.SetListtabPos( *pArr );
                    aFormat.SetIndentAt( *pArr );
                }

                pNewRule->Set( n, aFormat );
            }
        }
        break;
    case RES_POOLNUMRULE_BUL3:
        {
            SwNumFormat aFormat;

            aFormat.SetPositionAndSpaceMode( eNumberFormatPositionAndSpaceMode );

            aFormat.SetNumberingType(SVX_NUM_CHAR_SPECIAL);
            aFormat.SetCharFormat( pBullCFormat );
            aFormat.SetStart( 1 );
            aFormat.SetIncludeUpperLevels( 1 );
            aFormat.SetBulletFont(  &numfunc::GetDefBulletFont() );

            sal_uInt16 nOffs = o3tl::convert(4, o3tl::Length::mm, o3tl::Length::twip);

            if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_WIDTH_AND_POSITION )
            {
                aFormat.SetFirstLineOffset( - nOffs );
            }
            else if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_ALIGNMENT )
            {
                aFormat.SetLabelFollowedBy( SvxNumberFormat::LISTTAB );
                aFormat.SetFirstLineIndent( - nOffs );
            }

            for (sal_uInt16 n = 0; n < MAXLEVEL; ++n)
            {
                aFormat.SetBulletChar( (n & 1) ? 0x25a1 : 0x2611 );

                if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_WIDTH_AND_POSITION )
                {
                    aFormat.SetAbsLSpace( ((n & 1) +1) * nOffs );
                }
                else if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_ALIGNMENT )
                {
                    tools::Long nPos = ((n & 1) +1) * static_cast<tools::Long>(nOffs);
                    aFormat.SetListtabPos(nPos);
                    aFormat.SetIndentAt(nPos);
                }

                pNewRule->Set( n, aFormat );
            }
        }
        break;
    case RES_POOLNUMRULE_BUL4:
        {
            SwNumFormat aFormat;

            aFormat.SetPositionAndSpaceMode( eNumberFormatPositionAndSpaceMode );
            aFormat.SetNumberingType(SVX_NUM_CHAR_SPECIAL);
            aFormat.SetCharFormat( pBullCFormat );
            aFormat.SetStart( 1 );
            aFormat.SetIncludeUpperLevels( 1 );
            aFormat.SetBulletFont(  &numfunc::GetDefBulletFont() );

            static const sal_uInt16 aAbsSpace[ MAXLEVEL ] =
                {
//              cm: 0,4  0,8  1,2  1,6  2,0   2,4   2,8   3,2   3,6   4,0
                    227, 454, 680, 907, 1134, 1361, 1587, 1814, 2041, 2268
                };

            const sal_uInt16* pArr = aAbsSpace;

            if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_WIDTH_AND_POSITION )
            {
                aFormat.SetFirstLineOffset( - (*pArr) );
            }
            else if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_ALIGNMENT )
            {
                aFormat.SetLabelFollowedBy( SvxNumberFormat::SPACE );
                aFormat.SetFirstLineIndent( - (*pArr) );
            }

            for (sal_uInt16 n = 0; n < MAXLEVEL; ++n, ++pArr)
            {
                switch( n )
                {
                case 0:     aFormat.SetBulletChar( 0x27a2 );   break;
                case 1:     aFormat.SetBulletChar( 0xE006 );   break;
                default:    aFormat.SetBulletChar( 0xE004 );   break;
                }

                if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_WIDTH_AND_POSITION )
                {
                    aFormat.SetAbsLSpace( *pArr );
                }
                else if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_ALIGNMENT )
                {
                    aFormat.SetListtabPos( *pArr );
                    aFormat.SetIndentAt( *pArr );
                }

                pNewRule->Set( n, aFormat );
            }
        }
        break;
    case RES_POOLNUMRULE_BUL5:
        {
            SwNumFormat aFormat;

            aFormat.SetPositionAndSpaceMode( eNumberFormatPositionAndSpaceMode );
            aFormat.SetNumberingType(SVX_NUM_CHAR_SPECIAL);
            aFormat.SetCharFormat( pBullCFormat );
            aFormat.SetStart( 1 );
            aFormat.SetIncludeUpperLevels( 1 );
            aFormat.SetBulletChar( 0x2717 );
            aFormat.SetBulletFont( &numfunc::GetDefBulletFont() );

            static const sal_uInt16 aAbsSpace[ MAXLEVEL ] =
                {
//              cm: 0,4  0,8  1,2  1,6  2,0   2,4   2,8   3,2   3,6   4,0
                    227, 454, 680, 907, 1134, 1361, 1587, 1814, 2041, 2268
                };
            const sal_uInt16* pArr = aAbsSpace;

            if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_WIDTH_AND_POSITION )
            {
                aFormat.SetFirstLineOffset( - (*pArr) );
            }
            else if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_ALIGNMENT )
            {
                aFormat.SetLabelFollowedBy( SvxNumberFormat::LISTTAB );
                aFormat.SetFirstLineIndent( - (*pArr) );
            }

            for (sal_uInt16 n = 0; n < MAXLEVEL; ++n, ++pArr)
            {
                if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_WIDTH_AND_POSITION )
                {
                    aFormat.SetAbsLSpace( *pArr );
                }
                else if ( eNumberFormatPositionAndSpaceMode == SvxNumberFormat::LABEL_ALIGNMENT )
                {
                    aFormat.SetListtabPos( *pArr );
                    aFormat.SetIndentAt( *pArr );
                }

                pNewRule->Set( n, aFormat );
            }
        }
        break;
    }

    return pNewRule;
}

/// Check if this AutoCollection is already/still in use in this Document
bool DocumentStylePoolManager::IsPoolTextCollUsed( sal_uInt16 nId ) const
{
    OSL_ENSURE(
        (RES_POOLCOLL_TEXT_BEGIN <= nId && nId < RES_POOLCOLL_TEXT_END) ||
        (RES_POOLCOLL_LISTS_BEGIN <= nId && nId < RES_POOLCOLL_LISTS_END) ||
        (RES_POOLCOLL_EXTRA_BEGIN <= nId && nId < RES_POOLCOLL_EXTRA_END) ||
        (RES_POOLCOLL_REGISTER_BEGIN <= nId && nId < RES_POOLCOLL_REGISTER_END) ||
        (RES_POOLCOLL_DOC_BEGIN <= nId && nId < RES_POOLCOLL_DOC_END) ||
        (RES_POOLCOLL_HTML_BEGIN <= nId && nId < RES_POOLCOLL_HTML_END),
            "Wrong AutoFormat Id" );

    SwTextFormatColl* pNewColl = nullptr;
    bool bFnd = false;
    for( SwTextFormatColls::size_type n = 0; !bFnd && n < m_rDoc.GetTextFormatColls()->size(); ++n )
    {
        pNewColl = (*m_rDoc.GetTextFormatColls())[ n ];
        if( nId == pNewColl->GetPoolFormatId() )
            bFnd = true;
    }

    if( !bFnd || !pNewColl->HasWriterListeners() )
        return false;

    bool isUsed = false;
    sw::AutoFormatUsedHint aHint(isUsed, m_rDoc.GetNodes());
    pNewColl->CallSwClientNotify(aHint);
    return isUsed;
}

/// Check if this AutoCollection is already/still in use
bool DocumentStylePoolManager::IsPoolFormatUsed( sal_uInt16 nId ) const
{
    const SwFormat *pNewFormat = nullptr;
    const SwFormatsBase* pArray[ 2 ];
    sal_uInt16 nArrCnt = 1;
    bool bFnd = true;

    if (RES_POOLCHR_BEGIN <= nId && nId < RES_POOLCHR_END)
    {
        pArray[0] = m_rDoc.GetCharFormats();
    }
    else if (RES_POOLFRM_BEGIN <= nId && nId < RES_POOLFRM_END)
    {
        pArray[0] = m_rDoc.GetFrameFormats();
        pArray[1] = m_rDoc.GetSpzFrameFormats();
        nArrCnt = 2;
    }
    else
    {
        SAL_WARN("sw.core", "Invalid Pool Id: " << nId << " should be within "
            "[" << int(RES_POOLCHR_BEGIN) << "," << int(RES_POOLCHR_END) << ") or "
            "[" << int(RES_POOLFRM_BEGIN) << "," << int(RES_POOLFRM_END) << ")");
        bFnd = false;
    }

    if( bFnd )
    {
        bFnd = false;
        while( nArrCnt-- && !bFnd )
            for( size_t n = 0; !bFnd && n < (*pArray[nArrCnt]).GetFormatCount(); ++n )
            {
                pNewFormat = (*pArray[ nArrCnt ] ).GetFormat( n );
                if( nId == pNewFormat->GetPoolFormatId() )
                    bFnd = true;
            }
    }

    // Not found or no dependencies?
    if(!bFnd || !pNewFormat->HasWriterListeners() )
        return false;
    // Check if we have dependent ContentNodes in the Nodes array
    // (also indirect ones for derived Formats)
    return pNewFormat->IsUsed();
}

/// Check if this AutoCollection is already/still in use in this Document
bool DocumentStylePoolManager::IsPoolPageDescUsed( sal_uInt16 nId ) const
{
    OSL_ENSURE( RES_POOLPAGE_BEGIN <= nId && nId < RES_POOLPAGE_END,
            "Wrong AutoFormat Id" );
    SwPageDesc *pNewPgDsc = nullptr;
    bool bFnd = false;
    for( size_t n = 0; !bFnd && n < m_rDoc.GetPageDescCnt(); ++n )
    {
        pNewPgDsc = &m_rDoc.GetPageDesc(n);
        if( nId == pNewPgDsc->GetPoolFormatId() )
            bFnd = true;
    }

    // Not found or no dependencies?
    if( !bFnd || !pNewPgDsc->HasWriterListeners() )     // ??????
        return false;

    // Check if we have dependent ContentNodes in the Nodes array
    // (also indirect ones for derived Formats)
    bool isUsed = false;
    sw::AutoFormatUsedHint aHint(isUsed, m_rDoc.GetNodes());
    pNewPgDsc->CallSwClientNotify(aHint);
    return isUsed;
}

DocumentStylePoolManager::~DocumentStylePoolManager()
{
}

}

static std::vector<OUString>
lcl_NewUINameArray(const TranslateId* pIds, const size_t nLen, const size_t nSvxIds = 0)
{
    assert(nSvxIds <= nLen);
    const size_t nWriterIds = nLen - nSvxIds;
    std::vector<OUString> aNameArray;
    aNameArray.reserve(nLen);
    for (size_t i = 0; i < nWriterIds; ++i)
        aNameArray.push_back(SwResId(pIds[i]));
    for (size_t i = nWriterIds; i < nLen; ++i)
        aNameArray.push_back(SvxResId(pIds[i]));
    return aNameArray;
}

const std::vector<OUString>& SwStyleNameMapper::GetTextUINameArray()
{
    SvtSysLocale aSysLocale;
    const LanguageTag& rCurrentLanguage = aSysLocale.GetUILanguageTag();
    static std::map<LanguageTag, std::vector<OUString>> s_aTextUINameArray;

    auto it = s_aTextUINameArray.find(rCurrentLanguage);
    if (it == s_aTextUINameArray.end())
        it = s_aTextUINameArray.emplace(rCurrentLanguage,
                lcl_NewUINameArray(STR_POOLCOLL_TEXT_ARY, SAL_N_ELEMENTS(STR_POOLCOLL_TEXT_ARY))).first;

    return it->second;
}

const std::vector<OUString>& SwStyleNameMapper::GetListsUINameArray()
{
    SvtSysLocale aSysLocale;
    const LanguageTag& rCurrentLanguage = aSysLocale.GetUILanguageTag();
    static std::map<LanguageTag, std::vector<OUString>> s_aListsUINameArray;

    auto it = s_aListsUINameArray.find(rCurrentLanguage);
    if (it == s_aListsUINameArray.end())
        it = s_aListsUINameArray.emplace(rCurrentLanguage,
            lcl_NewUINameArray(STR_POOLCOLL_LISTS_ARY, SAL_N_ELEMENTS(STR_POOLCOLL_LISTS_ARY))).first;

    return it->second;
}

const std::vector<OUString>& SwStyleNameMapper::GetExtraUINameArray()
{
    SvtSysLocale aSysLocale;
    const LanguageTag& rCurrentLanguage = aSysLocale.GetUILanguageTag();
    static std::map<LanguageTag, std::vector<OUString>> s_aExtraUINameArray;

    auto it = s_aExtraUINameArray.find(rCurrentLanguage);
    if (it == s_aExtraUINameArray.end())
        it = s_aExtraUINameArray.emplace(rCurrentLanguage,
            lcl_NewUINameArray(STR_POOLCOLL_EXTRA_ARY, SAL_N_ELEMENTS(STR_POOLCOLL_EXTRA_ARY))).first;

    return it->second;
}

const std::vector<OUString>& SwStyleNameMapper::GetRegisterUINameArray()
{
    SvtSysLocale aSysLocale;
    const LanguageTag& rCurrentLanguage = aSysLocale.GetUILanguageTag();
    static std::map<LanguageTag, std::vector<OUString>> s_aRegisterUINameArray;

    auto it = s_aRegisterUINameArray.find(rCurrentLanguage);
    if (it == s_aRegisterUINameArray.end())
        it = s_aRegisterUINameArray.emplace(rCurrentLanguage,
            lcl_NewUINameArray(STR_POOLCOLL_REGISTER_ARY, SAL_N_ELEMENTS(STR_POOLCOLL_REGISTER_ARY))).first;

    return it->second;
}

const std::vector<OUString>& SwStyleNameMapper::GetDocUINameArray()
{
    SvtSysLocale aSysLocale;
    const LanguageTag& rCurrentLanguage = aSysLocale.GetUILanguageTag();
    static std::map<LanguageTag, std::vector<OUString>> s_aDocUINameArray;

    auto it = s_aDocUINameArray.find(rCurrentLanguage);
    if (it == s_aDocUINameArray.end())
        it = s_aDocUINameArray.emplace(rCurrentLanguage,
            lcl_NewUINameArray(STR_POOLCOLL_DOC_ARY, SAL_N_ELEMENTS(STR_POOLCOLL_DOC_ARY))).first;

    return it->second;
}

const std::vector<OUString>& SwStyleNameMapper::GetHTMLUINameArray()
{
    SvtSysLocale aSysLocale;
    const LanguageTag& rCurrentLanguage = aSysLocale.GetUILanguageTag();
    static std::map<LanguageTag, std::vector<OUString>> s_aHTMLUINameArray;

    auto it = s_aHTMLUINameArray.find(rCurrentLanguage);
    if (it == s_aHTMLUINameArray.end())
        it = s_aHTMLUINameArray.emplace(rCurrentLanguage,
            lcl_NewUINameArray(STR_POOLCOLL_HTML_ARY, SAL_N_ELEMENTS(STR_POOLCOLL_HTML_ARY))).first;

    return it->second;
}

const std::vector<OUString>& SwStyleNameMapper::GetFrameFormatUINameArray()
{
    SvtSysLocale aSysLocale;
    const LanguageTag& rCurrentLanguage = aSysLocale.GetUILanguageTag();
    static std::map<LanguageTag, std::vector<OUString>> s_aFrameFormatUINameArray;

    auto it = s_aFrameFormatUINameArray.find(rCurrentLanguage);
    if (it == s_aFrameFormatUINameArray.end())
        it = s_aFrameFormatUINameArray.emplace(rCurrentLanguage,
            lcl_NewUINameArray(STR_POOLFRM_ARY, SAL_N_ELEMENTS(STR_POOLFRM_ARY))).first;

    return it->second;
}

const std::vector<OUString>& SwStyleNameMapper::GetChrFormatUINameArray()
{
    SvtSysLocale aSysLocale;
    const LanguageTag& rCurrentLanguage = aSysLocale.GetUILanguageTag();
    static std::map<LanguageTag, std::vector<OUString>> s_aChrFormatUINameArray;

    auto it = s_aChrFormatUINameArray.find(rCurrentLanguage);
    if (it == s_aChrFormatUINameArray.end())
        it = s_aChrFormatUINameArray.emplace(rCurrentLanguage,
            lcl_NewUINameArray(STR_POOLCHR_ARY, SAL_N_ELEMENTS(STR_POOLCHR_ARY))).first;

    return it->second;
}

const std::vector<OUString>& SwStyleNameMapper::GetHTMLChrFormatUINameArray()
{
    SvtSysLocale aSysLocale;
    const LanguageTag& rCurrentLanguage = aSysLocale.GetUILanguageTag();
    static std::map<LanguageTag, std::vector<OUString>> s_aHTMLChrFormatUINameArray;

    auto it = s_aHTMLChrFormatUINameArray.find(rCurrentLanguage);
    if (it == s_aHTMLChrFormatUINameArray.end())
        it = s_aHTMLChrFormatUINameArray.emplace(rCurrentLanguage,
            lcl_NewUINameArray(STR_POOLCHR_HTML_ARY, SAL_N_ELEMENTS(STR_POOLCHR_HTML_ARY))).first;

    return it->second;
}

const std::vector<OUString>& SwStyleNameMapper::GetPageDescUINameArray()
{
    SvtSysLocale aSysLocale;
    const LanguageTag& rCurrentLanguage = aSysLocale.GetUILanguageTag();
    static std::map<LanguageTag, std::vector<OUString>> s_aPageDescUINameArray;

    auto it = s_aPageDescUINameArray.find(rCurrentLanguage);
    if (it == s_aPageDescUINameArray.end())
        it = s_aPageDescUINameArray.emplace(rCurrentLanguage,
            lcl_NewUINameArray(STR_POOLPAGE_ARY, SAL_N_ELEMENTS(STR_POOLPAGE_ARY))).first;

    return it->second;
}

const std::vector<OUString>& SwStyleNameMapper::GetNumRuleUINameArray()
{
    SvtSysLocale aSysLocale;
    const LanguageTag& rCurrentLanguage = aSysLocale.GetUILanguageTag();
    static std::map<LanguageTag, std::vector<OUString>> s_aNumRuleUINameArray;

    auto it = s_aNumRuleUINameArray.find(rCurrentLanguage);
    if (it == s_aNumRuleUINameArray.end())
        it = s_aNumRuleUINameArray.emplace(rCurrentLanguage,
            lcl_NewUINameArray(STR_POOLNUMRULE_NUM_ARY, SAL_N_ELEMENTS(STR_POOLNUMRULE_NUM_ARY))).first;

    return it->second;
}

const std::vector<OUString>& SwStyleNameMapper::GetTableStyleUINameArray()
{
    SvtSysLocale aSysLocale;
    const LanguageTag& rCurrentLanguage = aSysLocale.GetUILanguageTag();
    static std::map<LanguageTag, std::vector<OUString>> s_aTableStyleUINameArray;

    auto it = s_aTableStyleUINameArray.find(rCurrentLanguage);
    if (it == s_aTableStyleUINameArray.end())
        it = s_aTableStyleUINameArray.emplace(rCurrentLanguage,
            // 1 Writer resource string (XXX if this ever changes rather use offset math)
            lcl_NewUINameArray(STR_TABSTYLE_ARY, SAL_N_ELEMENTS(STR_TABSTYLE_ARY),
                    static_cast<size_t>(SAL_N_ELEMENTS(STR_TABSTYLE_ARY) - 1))).first;

    return it->second;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

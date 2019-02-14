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

#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <cppuhelper/supportsservice.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/contouritem.hxx>
#include <editeng/shdditem.hxx>
#include <editeng/crossedoutitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/fontitem.hxx>
#include <svx/xfillit0.hxx>
#include <svx/xlineit0.hxx>
#include <svx/sdtaitm.hxx>
#include <svx/sdtfsitm.hxx>
#include <svx/sdtagitm.hxx>
#include <svx/sdshitm.hxx>
#include <svx/sdshcitm.hxx>
#include <svx/sdsxyitm.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/numitem.hxx>
#include <editeng/cmapitem.hxx>
#include <svl/hint.hxx>
#include <editeng/charreliefitem.hxx>
#include <editeng/emphasismarkitem.hxx>
#include <svx/sdr/table/tabledesign.hxx>
#include <editeng/autokernitem.hxx>

#include <editeng/lrspitem.hxx>
#include <editeng/adjustitem.hxx>
#include <editeng/numdef.hxx>
#include <svl/itempool.hxx>
#include <svl/IndexedStyleSheets.hxx>

#include <stlpool.hxx>
#include <sdresid.hxx>
#include <stlsheet.hxx>
#include <strings.hrc>
#include <glob.hxx>
#include <drawdoc.hxx>
#include <svl/itemset.hxx>
#include <app.hrc>
#include <strings.hxx>

#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/drawing/TextFitToSizeType.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::style;
using namespace ::com::sun::star::container;

namespace
{

OUString lcl_findRenamedStyleName(std::vector< std::pair< OUString, OUString > > &rRenamedList, OUString const & aOriginalName )
{
    auto aIter = std::find_if(rRenamedList.begin(), rRenamedList.end(),
        [&aOriginalName](const std::pair<OUString, OUString>& rItem) { return rItem.first == aOriginalName; });
    if (aIter != rRenamedList.end())
        return (*aIter).second;
    return OUString();
}

SfxStyleSheet *lcl_findStyle(StyleSheetCopyResultVector& rStyles, const OUString& aStyleName)
{
    if( aStyleName.isEmpty() )
        return nullptr;
    for (const auto& a : rStyles)
    {
        if (a.m_xStyleSheet->GetName() == aStyleName)
            return a.m_xStyleSheet.get();
    }
    return nullptr;
}

}

SdStyleSheetPool::SdStyleSheetPool(SfxItemPool const& _rPool, SdDrawDocument* pDocument)
:   SdStyleSheetPoolBase( _rPool )
,   mpActualStyleSheet(nullptr)
,   mpDoc(pDocument)
{
    if( !mpDoc )
        return;

    rtl::Reference< SfxStyleSheetPool > xPool( this );

    // create graphics family
    mxGraphicFamily = new SdStyleFamily( xPool, SfxStyleFamily::Para );
    mxCellFamily = new SdStyleFamily( xPool, SfxStyleFamily::Frame );

    mxTableFamily = sdr::table::CreateTableDesignFamily();
    Reference< XNamed > xNamed( mxTableFamily, UNO_QUERY );
    if( xNamed.is() )
        msTableFamilyName = xNamed->getName();

    // create presentation families, one for each master page
    const sal_uInt16 nCount = mpDoc->GetMasterSdPageCount(PageKind::Standard);
    for( sal_uInt16 nPage = 0; nPage < nCount; ++nPage )
        AddStyleFamily( mpDoc->GetMasterSdPage(nPage,PageKind::Standard) );
}

SdStyleSheetPool::~SdStyleSheetPool()
{
    DBG_ASSERT( mpDoc == nullptr, "sd::SdStyleSheetPool::~SdStyleSheetPool(), dispose me first!" );
}

SfxStyleSheetBase* SdStyleSheetPool::Create(const OUString& rName, SfxStyleFamily eFamily, SfxStyleSearchBits _nMask )
{
    return new SdStyleSheet(rName, *this, eFamily, _nMask);
}

SfxStyleSheetBase* SdStyleSheetPool::GetTitleSheet(const OUString& rLayoutName)
{
    OUString aName(rLayoutName);
    aName += SD_LT_SEPARATOR;
    aName += STR_LAYOUT_TITLE;
    SfxStyleSheetBase* pResult = Find(aName, SfxStyleFamily::Page);
    return pResult;
}

/*************************************************************************
|*
|* Create a list of outline text templates for a presentation layout.
|* The caller has to delete the list.
|*
\************************************************************************/

void SdStyleSheetPool::CreateOutlineSheetList (const OUString& rLayoutName, std::vector<SfxStyleSheetBase*> &rOutlineStyles)
{
    OUString aName(rLayoutName);
    aName += SD_LT_SEPARATOR;
    aName += STR_LAYOUT_OUTLINE;

    for (sal_Int32 nSheet = 1; nSheet < 10; nSheet++)
    {
        OUString aFullName(aName + " " + OUString::number( nSheet ) );
        SfxStyleSheetBase* pSheet = Find(aFullName, SfxStyleFamily::Page);

        if (pSheet)
            rOutlineStyles.push_back(pSheet);
    }
}

/*************************************************************************
|*
|* Create style sheets with default values for the named presentation layout
|*
\************************************************************************/

void SdStyleSheetPool::CreateLayoutStyleSheets(const OUString& rLayoutName, bool bCheck /*= sal_False*/ )
{
    const SfxStyleSearchBits nUsedMask = SfxStyleSearchBits::All & ~SfxStyleSearchBits::UserDefined;

    bool bCreated = false;

    SfxStyleSheetBase* pSheet = nullptr;

    OUString aPrefix(rLayoutName + SD_LT_SEPARATOR);

    vcl::Font aLatinFont, aCJKFont, aCTLFont;

    mpDoc->getDefaultFonts( aLatinFont, aCJKFont, aCTLFont );

    // Font for title and outline
    SvxFontItem aSvxFontItem( aLatinFont.GetFamilyType(), aLatinFont.GetFamilyName(), aLatinFont.GetStyleName(), aLatinFont.GetPitch(),
                              aLatinFont.GetCharSet(), EE_CHAR_FONTINFO );

    SvxFontItem aSvxFontItemCJK( aCJKFont.GetFamilyType(), aCJKFont.GetFamilyName(), aCJKFont.GetStyleName(), aCJKFont.GetPitch(),
                                 aCJKFont.GetCharSet(), EE_CHAR_FONTINFO_CJK );

    SvxFontItem aSvxFontItemCTL( aCTLFont.GetFamilyType(), aCTLFont.GetFamilyName(), aCTLFont.GetStyleName(), aCTLFont.GetPitch(),
                                 aCTLFont.GetCharSet(), EE_CHAR_FONTINFO_CTL );

    vcl::Font aBulletFont( GetBulletFont() );

    /**************************************************************************
    * outline levels
    **************************************************************************/
    OUString aName(STR_LAYOUT_OUTLINE);
    OUString aHelpFile;

    SvxLRSpaceItem aSvxLRSpaceItem( EE_PARA_LRSPACE );
    SvxULSpaceItem aSvxULSpaceItem( EE_PARA_ULSPACE );

    for( sal_Int32 nLevel = 1; nLevel < 10; nLevel++)
    {
        OUString aLevelName( aPrefix + aName + " " + OUString::number( nLevel ) ) ;

        if (!Find(aLevelName, SfxStyleFamily::Page))
        {
            bCreated = true;
            pSheet = &Make(aLevelName, SfxStyleFamily::Page,nUsedMask);
            pSheet->SetHelpId( aHelpFile, HID_PSEUDOSHEET_OUTLINE + nLevel );

            pSheet->SetParent( OUString() );

            // attributing for level 1, the others levels inherit
            if (nLevel == 1)
            {
                SfxItemSet&     rSet = pSheet->GetItemSet();

                rSet.Put(aSvxFontItem);
                rSet.Put(aSvxFontItemCJK);
                rSet.Put(aSvxFontItemCTL);
                rSet.Put( SvxPostureItem( ITALIC_NONE, EE_CHAR_ITALIC ) );
                rSet.Put( SvxPostureItem( ITALIC_NONE, EE_CHAR_ITALIC_CJK ) );
                rSet.Put( SvxPostureItem( ITALIC_NONE, EE_CHAR_ITALIC_CTL ) );
                rSet.Put( SvxWeightItem( WEIGHT_NORMAL, EE_CHAR_WEIGHT ) );
                rSet.Put( SvxWeightItem( WEIGHT_NORMAL, EE_CHAR_WEIGHT_CJK ) );
                rSet.Put( SvxWeightItem( WEIGHT_NORMAL, EE_CHAR_WEIGHT_CTL ) );
                rSet.Put( SvxUnderlineItem(LINESTYLE_NONE, EE_CHAR_UNDERLINE ) );
                rSet.Put( SvxOverlineItem(LINESTYLE_NONE, EE_CHAR_OVERLINE ) );
                rSet.Put( SvxCrossedOutItem(STRIKEOUT_NONE, EE_CHAR_STRIKEOUT ) );
                rSet.Put( SvxCaseMapItem(SvxCaseMap::NotMapped, EE_CHAR_CASEMAP ) );
                rSet.Put( SvxShadowedItem(false, EE_CHAR_SHADOW ) );
                rSet.Put( SvxContourItem(false, EE_CHAR_OUTLINE ) );
                rSet.Put( SvxEmphasisMarkItem(FontEmphasisMark::NONE, EE_CHAR_EMPHASISMARK ) );
                rSet.Put( SvxCharReliefItem(FontRelief::NONE, EE_CHAR_RELIEF) );
                rSet.Put( SvxColorItem( COL_AUTO, EE_CHAR_COLOR) );
                rSet.Put( SvxBackgroundColorItem( COL_AUTO, EE_CHAR_BKGCOLOR )  );
                rSet.Put( XLineStyleItem(css::drawing::LineStyle_NONE) );
                rSet.Put( XFillStyleItem(drawing::FillStyle_NONE) );
                rSet.Put( SdrTextFitToSizeTypeItem(drawing::TextFitToSizeType_AUTOFIT) );
                rSet.Put( makeSdrTextAutoGrowHeightItem(false) );
                // #i16874# enable kerning by default but only for new documents
                rSet.Put( SvxAutoKernItem( true, EE_CHAR_PAIRKERNING ) );

                vcl::Font f( GetBulletFont() );
                PutNumBulletItem( pSheet, f );
            }

            sal_uLong nFontSize = 20;
            sal_uInt16 nUpper = 100;

            switch (nLevel)
            {
                case 1:
                {
                    nFontSize = 32;
                    nUpper = 500;
                }
                break;

                case 2:
                {
                    nFontSize = 28;
                    nUpper = 400;
                }
                break;

                case 3:
                {
                    nFontSize = 24;
                    nUpper = 300;
                }
                break;

                case 4:
                {
                    nUpper = 200;
                }
                break;
            }

            // FontSize
            nFontSize = static_cast<sal_uInt16>((nFontSize * 2540L) / 72);  // Pt --> 1/100 mm
            SfxItemSet& rOutlineSet = pSheet->GetItemSet();
            rOutlineSet.Put( SvxFontHeightItem( nFontSize, 100, EE_CHAR_FONTHEIGHT ) );
            rOutlineSet.Put( SvxFontHeightItem( nFontSize, 100, EE_CHAR_FONTHEIGHT_CJK ) );
            rOutlineSet.Put( SvxFontHeightItem( SdDrawDocument::convertFontHeightToCTL( nFontSize ), 100, EE_CHAR_FONTHEIGHT_CTL ) );

            // Line distance (upwards). Stuff around here cleaned up in i35937
            aSvxULSpaceItem.SetUpper(nUpper);
            pSheet->GetItemSet().Put(aSvxULSpaceItem);
        }
    }

    // if we created outline styles, we need to chain them
    if( bCreated )
    {
        SfxStyleSheetBase* pParent = nullptr;
        for (sal_Int32 nLevel = 1; nLevel < 10; nLevel++)
        {
            OUString aLevelName( aPrefix + aName + " " + OUString::number( nLevel ) );

            pSheet = Find(aLevelName, SfxStyleFamily::Page);

            DBG_ASSERT( pSheet, "missing layout style!");

            if( pSheet )
            {
                if (pParent)
                    pSheet->SetParent(pParent->GetName());
                pParent = pSheet;
            }
        }
    }

    /**************************************************************************
    * Title
    **************************************************************************/
    aName = aPrefix + STR_LAYOUT_TITLE;

    if (!Find(aName, SfxStyleFamily::Page))
    {
        bCreated = true;

        pSheet = &Make(aName, SfxStyleFamily::Page,nUsedMask);
        pSheet->SetHelpId( aHelpFile, HID_PSEUDOSHEET_TITLE );
        pSheet->SetParent( OUString() );
        SfxItemSet& rTitleSet = pSheet->GetItemSet();
        rTitleSet.Put(XLineStyleItem(drawing::LineStyle_NONE));
        rTitleSet.Put(XFillStyleItem(drawing::FillStyle_NONE));
        rTitleSet.Put(aSvxFontItem);
        rTitleSet.Put(aSvxFontItemCJK);
        rTitleSet.Put(aSvxFontItemCTL);
        rTitleSet.Put(SvxPostureItem( ITALIC_NONE, EE_CHAR_ITALIC ) );
        rTitleSet.Put(SvxPostureItem( ITALIC_NONE, EE_CHAR_ITALIC_CJK ) );
        rTitleSet.Put(SvxPostureItem( ITALIC_NONE, EE_CHAR_ITALIC_CTL ) );
        rTitleSet.Put(SvxWeightItem( WEIGHT_NORMAL, EE_CHAR_WEIGHT ) );
        rTitleSet.Put(SvxWeightItem( WEIGHT_NORMAL, EE_CHAR_WEIGHT_CJK ) );
        rTitleSet.Put(SvxWeightItem( WEIGHT_NORMAL, EE_CHAR_WEIGHT_CTL ) );
        rTitleSet.Put(SvxFontHeightItem( 1552, 100, EE_CHAR_FONTHEIGHT ) );                 // 44 pt
        rTitleSet.Put(SvxFontHeightItem( 1552, 100, EE_CHAR_FONTHEIGHT_CJK ) );                 // 44 pt
        rTitleSet.Put(SvxFontHeightItem( SdDrawDocument::convertFontHeightToCTL( 1552 ), 100, EE_CHAR_FONTHEIGHT_CTL ) );                   // 44 pt
        rTitleSet.Put(SvxUnderlineItem(LINESTYLE_NONE, EE_CHAR_UNDERLINE ));
        rTitleSet.Put(SvxOverlineItem(LINESTYLE_NONE, EE_CHAR_OVERLINE ));
        rTitleSet.Put(SvxCrossedOutItem(STRIKEOUT_NONE, EE_CHAR_STRIKEOUT ));
        rTitleSet.Put(SvxCaseMapItem(SvxCaseMap::NotMapped, EE_CHAR_CASEMAP ));
        rTitleSet.Put(SvxShadowedItem(false, EE_CHAR_SHADOW ));
        rTitleSet.Put(SvxContourItem(false, EE_CHAR_OUTLINE ));
        rTitleSet.Put( SvxEmphasisMarkItem(FontEmphasisMark::NONE, EE_CHAR_EMPHASISMARK ) );
        rTitleSet.Put( SvxCharReliefItem(FontRelief::NONE, EE_CHAR_RELIEF ) );
        rTitleSet.Put(SvxColorItem( COL_AUTO, EE_CHAR_COLOR ));
        rTitleSet.Put(SvxBackgroundColorItem( COL_AUTO, EE_CHAR_BKGCOLOR ));
        rTitleSet.Put(SvxAdjustItem(SvxAdjust::Center, EE_PARA_JUST ));
        rTitleSet.Put( SdrTextVertAdjustItem( SDRTEXTVERTADJUST_CENTER ) );
        // #i16874# enable kerning by default but only for new documents
        rTitleSet.Put( SvxAutoKernItem( true, EE_CHAR_PAIRKERNING ) );

        aBulletFont.SetFontSize(Size(0,1552));                  // 44 pt
        PutNumBulletItem( pSheet, aBulletFont );
    }

    /**************************************************************************
    * Subtitle
    **************************************************************************/
    aName = aPrefix + STR_LAYOUT_SUBTITLE;

    if (!Find(aName, SfxStyleFamily::Page))
    {
        bCreated = true;

        pSheet = &Make(aName, SfxStyleFamily::Page,nUsedMask);
        pSheet->SetHelpId( aHelpFile, HID_PSEUDOSHEET_SUBTITLE );
        pSheet->SetParent( OUString() );
        SfxItemSet& rSubtitleSet = pSheet->GetItemSet();
        rSubtitleSet.Put(XLineStyleItem(drawing::LineStyle_NONE));
        rSubtitleSet.Put(XFillStyleItem(drawing::FillStyle_NONE));
        rSubtitleSet.Put(aSvxFontItem);
        rSubtitleSet.Put(aSvxFontItemCJK);
        rSubtitleSet.Put(aSvxFontItemCTL);
        rSubtitleSet.Put(SvxPostureItem( ITALIC_NONE, EE_CHAR_ITALIC ) );
        rSubtitleSet.Put(SvxPostureItem( ITALIC_NONE, EE_CHAR_ITALIC_CJK ) );
        rSubtitleSet.Put(SvxPostureItem( ITALIC_NONE, EE_CHAR_ITALIC_CTL ) );
        rSubtitleSet.Put(SvxWeightItem( WEIGHT_NORMAL, EE_CHAR_WEIGHT ) );
        rSubtitleSet.Put(SvxWeightItem( WEIGHT_NORMAL, EE_CHAR_WEIGHT_CJK ) );
        rSubtitleSet.Put(SvxWeightItem( WEIGHT_NORMAL, EE_CHAR_WEIGHT_CTL ) );
        rSubtitleSet.Put( SvxFontHeightItem( 1129, 100, EE_CHAR_FONTHEIGHT ) );     // 32 pt
        rSubtitleSet.Put( SvxFontHeightItem( 1129, 100, EE_CHAR_FONTHEIGHT_CJK ) ); // 32 pt
        rSubtitleSet.Put( SvxFontHeightItem( SdDrawDocument::convertFontHeightToCTL( 1129 ), 100, EE_CHAR_FONTHEIGHT_CTL ) ); // 32 pt
        rSubtitleSet.Put(SvxUnderlineItem(LINESTYLE_NONE, EE_CHAR_UNDERLINE ));
        rSubtitleSet.Put(SvxOverlineItem(LINESTYLE_NONE, EE_CHAR_OVERLINE ));
        rSubtitleSet.Put(SvxCrossedOutItem(STRIKEOUT_NONE, EE_CHAR_STRIKEOUT ));
        rSubtitleSet.Put(SvxCaseMapItem(SvxCaseMap::NotMapped, EE_CHAR_CASEMAP ));
        rSubtitleSet.Put(SvxShadowedItem(false, EE_CHAR_SHADOW ));
        rSubtitleSet.Put(SvxContourItem(false, EE_CHAR_OUTLINE ));
        rSubtitleSet.Put( SvxEmphasisMarkItem(FontEmphasisMark::NONE, EE_CHAR_EMPHASISMARK ) );
        rSubtitleSet.Put( SvxCharReliefItem(FontRelief::NONE, EE_CHAR_RELIEF ) );
        rSubtitleSet.Put(SvxColorItem( COL_AUTO, EE_CHAR_COLOR ));
        rSubtitleSet.Put(SvxBackgroundColorItem( COL_AUTO, EE_CHAR_BKGCOLOR ));
        rSubtitleSet.Put(SvxAdjustItem(SvxAdjust::Center, EE_PARA_JUST ));
        rSubtitleSet.Put( SdrTextVertAdjustItem( SDRTEXTVERTADJUST_CENTER ) );
        // #i16874# enable kerning by default but only for new documents
        rSubtitleSet.Put( SvxAutoKernItem( true, EE_CHAR_PAIRKERNING ) );
        aSvxLRSpaceItem.SetTextLeft(0);
        rSubtitleSet.Put(aSvxLRSpaceItem);

        vcl::Font aTmpFont( GetBulletFont() );
        aTmpFont.SetFontSize(Size(0, 1129));        // 32 pt
        PutNumBulletItem( pSheet, aTmpFont );
    }

    /**************************************************************************
    * Notes
    **************************************************************************/
    aName = aPrefix + STR_LAYOUT_NOTES;

    if (!Find(aName, SfxStyleFamily::Page))
    {
        bCreated = true;

        pSheet = &Make(aName, SfxStyleFamily::Page,nUsedMask);
        pSheet->SetHelpId( aHelpFile, HID_PSEUDOSHEET_NOTES );
        pSheet->SetParent( OUString() );
        SfxItemSet& rNotesSet = pSheet->GetItemSet();
        rNotesSet.Put(XLineStyleItem(drawing::LineStyle_NONE));
        rNotesSet.Put(XFillStyleItem(drawing::FillStyle_NONE));
        rNotesSet.Put(aSvxFontItem);
        rNotesSet.Put(aSvxFontItemCJK);
        rNotesSet.Put(aSvxFontItemCTL);
        rNotesSet.Put( SvxPostureItem( ITALIC_NONE, EE_CHAR_ITALIC ) );
        rNotesSet.Put( SvxPostureItem( ITALIC_NONE, EE_CHAR_ITALIC_CJK ) );
        rNotesSet.Put( SvxPostureItem( ITALIC_NONE, EE_CHAR_ITALIC_CTL ) );
        rNotesSet.Put( SvxWeightItem( WEIGHT_NORMAL, EE_CHAR_WEIGHT ) );
        rNotesSet.Put( SvxWeightItem( WEIGHT_NORMAL, EE_CHAR_WEIGHT_CJK ) );
        rNotesSet.Put( SvxWeightItem( WEIGHT_NORMAL, EE_CHAR_WEIGHT_CTL ) );
        rNotesSet.Put( SvxFontHeightItem( 705, 100, EE_CHAR_FONTHEIGHT ) );     // 20 pt
        rNotesSet.Put( SvxFontHeightItem( 705, 100, EE_CHAR_FONTHEIGHT_CJK ) ); // 20 pt
        rNotesSet.Put( SvxFontHeightItem( SdDrawDocument::convertFontHeightToCTL( 705 ), 100, EE_CHAR_FONTHEIGHT_CTL ) ); // 20 pt
        rNotesSet.Put( SvxUnderlineItem(LINESTYLE_NONE, EE_CHAR_UNDERLINE ) );
        rNotesSet.Put( SvxOverlineItem(LINESTYLE_NONE, EE_CHAR_OVERLINE ) );
        rNotesSet.Put( SvxCrossedOutItem(STRIKEOUT_NONE, EE_CHAR_STRIKEOUT ) );
        rNotesSet.Put( SvxCaseMapItem(SvxCaseMap::NotMapped, EE_CHAR_CASEMAP ) );
        rNotesSet.Put( SvxShadowedItem(false, EE_CHAR_SHADOW ) );
        rNotesSet.Put( SvxContourItem(false, EE_CHAR_OUTLINE ) );
        rNotesSet.Put( SvxEmphasisMarkItem(FontEmphasisMark::NONE, EE_CHAR_EMPHASISMARK ) );
        rNotesSet.Put( SvxCharReliefItem(FontRelief::NONE, EE_CHAR_RELIEF) );
        rNotesSet.Put( SvxColorItem( COL_AUTO, EE_CHAR_COLOR ) );
        rNotesSet.Put( SvxBackgroundColorItem( COL_AUTO, EE_CHAR_BKGCOLOR ) );
        rNotesSet.Put( SvxLRSpaceItem( 0, 0, 600, -600, EE_PARA_LRSPACE  ) );
        // #i16874# enable kerning by default but only for new documents
        rNotesSet.Put( SvxAutoKernItem( true, EE_CHAR_PAIRKERNING ) );

/* #i35937# */

    }

    /**************************************************************************
    * Background objects
    **************************************************************************/
    aName = aPrefix + STR_LAYOUT_BACKGROUNDOBJECTS;

    if (!Find(aName, SfxStyleFamily::Page))
    {
        bCreated = true;

        pSheet = &Make(aName, SfxStyleFamily::Page,nUsedMask);
        pSheet->SetHelpId( aHelpFile, HID_PSEUDOSHEET_BACKGROUNDOBJECTS );
        pSheet->SetParent( OUString() );
        SfxItemSet& rBackgroundObjectsSet = pSheet->GetItemSet();
        rBackgroundObjectsSet.Put(makeSdrShadowItem(false));
        rBackgroundObjectsSet.Put(makeSdrShadowColorItem(COL_GRAY));
        rBackgroundObjectsSet.Put(makeSdrShadowXDistItem(200)); // 3 mm shadow distance
        rBackgroundObjectsSet.Put(makeSdrShadowYDistItem(200));
        // #i16874# enable kerning by default but only for new documents
        rBackgroundObjectsSet.Put( SvxAutoKernItem( true, EE_CHAR_PAIRKERNING ) );
        rBackgroundObjectsSet.Put(SdrTextHorzAdjustItem(SDRTEXTHORZADJUST_BLOCK));
    }

    /**************************************************************************
    * Background
    **************************************************************************/
    aName = aPrefix + STR_LAYOUT_BACKGROUND;

    if (!Find(aName, SfxStyleFamily::Page))
    {
        bCreated = true;

        pSheet = &Make(aName, SfxStyleFamily::Page,nUsedMask);
        pSheet->SetHelpId( aHelpFile, HID_PSEUDOSHEET_BACKGROUND );
        pSheet->SetParent( OUString() );
        SfxItemSet& rBackgroundSet = pSheet->GetItemSet();
        rBackgroundSet.Put(XLineStyleItem(drawing::LineStyle_NONE));
        rBackgroundSet.Put(XFillStyleItem(drawing::FillStyle_NONE));
        // #i16874# enable kerning by default but only for new documents
        rBackgroundSet.Put( SvxAutoKernItem( true, EE_CHAR_PAIRKERNING ) );
    }

    DBG_ASSERT( !bCheck || !bCreated, "missing layout style sheets detected!" );
}

/*************************************************************************
|*
|* Copy graphic style sheets from source pool into this pool
|*
|* (rSourcePool can not be const since SfxStyleSheetPoolBase::Find isn't const)
|*
\************************************************************************/

void SdStyleSheetPool::CopyGraphicSheets(SdStyleSheetPool& rSourcePool)
{
    CopySheets( rSourcePool, SfxStyleFamily::Para );
}

void SdStyleSheetPool::CopyCellSheets(SdStyleSheetPool& rSourcePool)
{
    CopySheets( rSourcePool, SfxStyleFamily::Frame );
}

void SdStyleSheetPool::CopyTableStyles(SdStyleSheetPool const & rSourcePool)
{
    Reference< XIndexAccess > xSource( rSourcePool.mxTableFamily, UNO_QUERY );
    Reference< XNameContainer > xTarget( mxTableFamily, UNO_QUERY );
    Reference< XSingleServiceFactory > xFactory( mxTableFamily, UNO_QUERY );

    if( !(xSource.is() && xFactory.is() && mxTableFamily.is()) )
        return;

    for( sal_Int32 nIndex = 0; nIndex < xSource->getCount(); nIndex++ ) try
    {
        Reference< XStyle > xSourceTableStyle( xSource->getByIndex( nIndex ), UNO_QUERY );
        if( xSourceTableStyle.is() )
        {
            Reference< XStyle > xNewTableStyle( xFactory->createInstance(), UNO_QUERY );
            if( xNewTableStyle.is() )
            {
                Reference< XNameAccess> xSourceNames( xSourceTableStyle, UNO_QUERY_THROW );

                Sequence< OUString > aStyleNames( xSourceNames->getElementNames() );
                OUString* pStyleNames( aStyleNames.getArray() );

                Reference< XNameReplace > xTargetNames( xNewTableStyle, UNO_QUERY );

                sal_Int32 nNames = aStyleNames.getLength();
                while( nNames-- )
                {
                    const OUString aName( *pStyleNames++ );
                    Reference< XStyle > xSourceStyle( xSourceNames->getByName( aName ), UNO_QUERY );
                    Reference< XStyle > xTargetStyle;
                    if( xSourceStyle.is() ) try
                    {
                        mxCellFamily->getByName( xSourceStyle->getName() ) >>= xTargetStyle;
                    }
                    catch( Exception& )
                    {
                        OSL_FAIL( "sd::SdStyleSheetPool::CopyTableStyles(), exception caught!" );
                    }

                    if( xTargetStyle.is() )
                        xTargetNames->replaceByName( aName, Any( xTargetStyle ) );
                }
            }

            OUString sName( xSourceTableStyle->getName() );
            if( xTarget->hasByName( sName ) )
                xTarget->replaceByName( sName, Any( xNewTableStyle ) );
            else
                xTarget->insertByName( sName, Any( xNewTableStyle ) );
        }
    }
    catch( Exception& )
    {
        OSL_FAIL("sd::SdStyleSheetPool::CopyTableStyles(), exception caught!");
    }
}

void SdStyleSheetPool::CopyCellSheets(SdStyleSheetPool& rSourcePool, StyleSheetCopyResultVector& rCreatedSheets)
{
    CopySheets( rSourcePool, SfxStyleFamily::Frame, rCreatedSheets );
}

void SdStyleSheetPool::RenameAndCopyGraphicSheets(SdStyleSheetPool& rSourcePool, StyleSheetCopyResultVector& rCreatedSheets, OUString const &rRenameSuffix)
{
    CopySheets( rSourcePool, SfxStyleFamily::Para, rCreatedSheets, rRenameSuffix );
}

void SdStyleSheetPool::CopySheets(SdStyleSheetPool& rSourcePool, SfxStyleFamily eFamily )
{
    StyleSheetCopyResultVector aTmpSheets;
    CopySheets(rSourcePool, eFamily, aTmpSheets);
}

void SdStyleSheetPool::CopySheets(SdStyleSheetPool& rSourcePool, SfxStyleFamily eFamily, StyleSheetCopyResultVector& rCreatedSheets)
{
    CopySheets(rSourcePool, eFamily, rCreatedSheets, "");
}

namespace
{

struct HasFamilyPredicate : svl::StyleSheetPredicate
{
    explicit HasFamilyPredicate(SfxStyleFamily eFamily)
    : meFamily(eFamily) {}

    bool Check(const SfxStyleSheetBase& sheet) override
    {
        return sheet.GetFamily() == meFamily;
    }
    SfxStyleFamily const meFamily;
};

}

void SdStyleSheetPool::CopySheets(SdStyleSheetPool& rSourcePool, SfxStyleFamily eFamily, StyleSheetCopyResultVector& rCreatedSheets, const OUString& rRenameSuffix)
{
    std::vector< std::pair< rtl::Reference< SfxStyleSheetBase >, OUString > > aNewStyles;
    std::vector< std::pair< OUString, OUString > > aRenamedList;

    // find all style sheets of the source pool which have the same family
    HasFamilyPredicate aHasFamilyPredicate(eFamily);
    std::vector<unsigned> aSheetsWithFamily = rSourcePool.GetIndexedStyleSheets().FindPositionsByPredicate(aHasFamilyPredicate);

    for (const auto& rPos : aSheetsWithFamily)
    {
        SfxStyleSheetBase* pSheet = rSourcePool.GetStyleSheetByPositionInIndex( rPos );
        if( !pSheet )
            continue;
        OUString aName( pSheet->GetName() );

        // now check whether we already have a sheet with the same name
        std::vector<unsigned> aSheetsWithName = GetIndexedStyleSheets().FindPositionsByName(aName);
        bool bAddToList = false;
        SfxStyleSheetBase * pExistingSheet = nullptr;
        if (!aSheetsWithName.empty())
        {
            // if we have a rename suffix, try to find a new name
            pExistingSheet =
                GetStyleSheetByPositionInIndex(aSheetsWithName.front());
            if (!rRenameSuffix.isEmpty() &&
                !pExistingSheet->GetItemSet().Equals(pSheet->GetItemSet(), false))
            {
                // we have found a sheet with the same name, but different contents. Try to find a new name.
                // If we already have a sheet with the new name, and it is equal to the one in the source pool,
                // do nothing.
                OUString aTmpName = aName + rRenameSuffix;
                sal_Int32 nSuffix = 1;
                do
                {
                    aTmpName = aName + rRenameSuffix + OUString::number(nSuffix);
                    pExistingSheet = Find(aTmpName, eFamily);
                    nSuffix++;
                } while (pExistingSheet &&
                        !pExistingSheet->GetItemSet().Equals(pSheet->GetItemSet(), false));
                aName = aTmpName;
                bAddToList = true;
            }
        }
        // we do not already have a sheet with the same name and contents. Create a new one.
        if (!pExistingSheet)
        {
            assert(!Find(aName, eFamily));
            rtl::Reference< SfxStyleSheetBase > xNewSheet( &Make( aName, eFamily ) );

            xNewSheet->SetMask( pSheet->GetMask() );

            // Also set parent relation for copied style sheets
            OUString aParent( pSheet->GetParent() );
            if( !aParent.isEmpty() )
                aNewStyles.emplace_back( xNewSheet, aParent );

            if( !bAddToList )
            {
                OUString aHelpFile;
                xNewSheet->SetHelpId( aHelpFile, pSheet->GetHelpId( aHelpFile ) );
            }
            xNewSheet->GetItemSet().Put( pSheet->GetItemSet() );

            rCreatedSheets.emplace_back(static_cast<SdStyleSheet*>(xNewSheet.get()), true);
            aRenamedList.emplace_back( pSheet->GetName(), aName );
        }
        else if (bAddToList)
        {
            // Add to list - used for renaming
            rCreatedSheets.emplace_back(static_cast<SdStyleSheet*>(pExistingSheet), false);
            aRenamedList.emplace_back( pSheet->GetName(), aName );
        }
    }

    // set parents on newly added stylesheets
    for( auto& rStyle : aNewStyles )
    {
        if( !rRenameSuffix.isEmpty() )
        {
            SfxStyleSheet *pParent = lcl_findStyle(rCreatedSheets, lcl_findRenamedStyleName(aRenamedList, rStyle.second));
            if( pParent )
            {
                rStyle.first->SetParent( pParent->GetName() );
                continue;
            }
        }
        DBG_ASSERT( rSourcePool.Find( rStyle.second, eFamily ), "StyleSheet has invalid parent: Family mismatch" );
        rStyle.first->SetParent( rStyle.second );
    }
    // we have changed names of style sheets. Trigger reindexing.
    Reindex();
}

/*************************************************************************
|*
|* Copy style sheets of the named presentation layout from the source pool into
|* this pool. Copies only the style sheets which aren't yet in this pool.
|* If not NULL, pCreatedSheets is filled with pointers to the created style
|* sheets.
|*
|* (rSourcePool can not be const since SfxStyleSheetPoolBase::Find isn't const)
|*
\************************************************************************/

void SdStyleSheetPool::CopyLayoutSheets(const OUString& rLayoutName, SdStyleSheetPool& rSourcePool, StyleSheetCopyResultVector& rCreatedSheets)
{
    SfxStyleSheetBase* pSheet = nullptr;

    std::vector<OUString> aNameList;
    CreateLayoutSheetNames(rLayoutName,aNameList);

    for (const auto& rName : aNameList)
    {
        pSheet = Find(rName, SfxStyleFamily::Page);
        if (!pSheet)
        {
            SfxStyleSheetBase* pSourceSheet = rSourcePool.Find(rName, SfxStyleFamily::Page);
            DBG_ASSERT(pSourceSheet, "CopyLayoutSheets: Style sheet missing");
            if (pSourceSheet)
            {
                // In the case one comes with Methusalem-Docs.
                SfxStyleSheetBase& rNewSheet = Make(rName, SfxStyleFamily::Page);
                OUString file;
                rNewSheet.SetHelpId( file, pSourceSheet->GetHelpId( file ) );
                rNewSheet.GetItemSet().Put(pSourceSheet->GetItemSet());
                rCreatedSheets.emplace_back(static_cast<SdStyleSheet*>(&rNewSheet), true);
            }
        }
    }

    // Special treatment for outline templates: create parent relation
    std::vector<SfxStyleSheetBase*> aOutlineSheets;
    CreateOutlineSheetList(rLayoutName,aOutlineSheets);

    if( aOutlineSheets.empty() )
        return;

    std::vector<SfxStyleSheetBase*>::iterator it = aOutlineSheets.begin();
    SfxStyleSheetBase* pParent = *it;
    ++it;

    while (it != aOutlineSheets.end())
    {
        pSheet = *it;

        if (!pSheet)
            break;

        if (pSheet->GetParent().isEmpty())
            pSheet->SetParent(pParent->GetName());

        pParent = pSheet;

        ++it;
    }
}

/*************************************************************************
|*
|* Create list with names of the presentation templates of a layout.
|* The list and the containing strings are owned by the caller!
|*
\************************************************************************/

void SdStyleSheetPool::CreateLayoutSheetNames(const OUString& rLayoutName, std::vector<OUString> &aNameList)
{
    OUString aPrefix(rLayoutName + SD_LT_SEPARATOR);

    for (sal_Int32 nLevel = 1; nLevel < 10; nLevel++)
        aNameList.emplace_back(aPrefix + STR_LAYOUT_OUTLINE " " + OUString::number( nLevel ) );

    aNameList.emplace_back(aPrefix + STR_LAYOUT_TITLE);
    aNameList.emplace_back(aPrefix + STR_LAYOUT_SUBTITLE);
    aNameList.emplace_back(aPrefix + STR_LAYOUT_NOTES);
    aNameList.emplace_back(aPrefix + STR_LAYOUT_BACKGROUNDOBJECTS);
    aNameList.emplace_back(aPrefix + STR_LAYOUT_BACKGROUND);
}

/*************************************************************************
|*
|* Create a list with pointer to presentation templates of a layout.
|* The list is owned by the caller!
|*
\************************************************************************/

void SdStyleSheetPool::CreateLayoutSheetList(const OUString& rLayoutName, SdStyleSheetVector& rLayoutSheets )
{
    OUString aLayoutNameWithSep(rLayoutName + SD_LT_SEPARATOR);

    SfxStyleSheetIterator aIter(this, SfxStyleFamily::Page);
    SfxStyleSheetBase* pSheet = aIter.First();

    while (pSheet)
    {
        if (pSheet->GetName().startsWith(aLayoutNameWithSep))
            rLayoutSheets.emplace_back( static_cast< SdStyleSheet* >( pSheet ) );
        pSheet = aIter.Next();
    }
}

/*************************************************************************
|*
|* Create pseudo style sheets if necessary
|*
\************************************************************************/

void SdStyleSheetPool::CreatePseudosIfNecessary()
{
    OUString aName;
    OUString aHelpFile;
    SfxStyleSheetBase* pSheet = nullptr;
    SfxStyleSheetBase* pParent = nullptr;

    SfxStyleSearchBits nUsedMask = SfxStyleSearchBits::Used;

    aName = SdResId(STR_PSEUDOSHEET_TITLE);
    if( (pSheet = Find(aName, SfxStyleFamily::Pseudo)) == nullptr )
    {
        pSheet = &Make(aName, SfxStyleFamily::Pseudo, nUsedMask);
        pSheet->SetParent( OUString() );
        static_cast<SfxStyleSheet*>(pSheet)->StartListening(*this);
    }
    pSheet->SetHelpId( aHelpFile, HID_PSEUDOSHEET_TITLE );

    aName = SdResId(STR_PSEUDOSHEET_SUBTITLE);
    if( (pSheet = Find(aName, SfxStyleFamily::Pseudo)) == nullptr )
    {
        pSheet = &Make(aName, SfxStyleFamily::Pseudo, nUsedMask);
        pSheet->SetParent( OUString() );
        static_cast<SfxStyleSheet*>(pSheet)->StartListening(*this);
    }
    pSheet->SetHelpId( aHelpFile, HID_PSEUDOSHEET_SUBTITLE );

    aName = SdResId(STR_PSEUDOSHEET_BACKGROUNDOBJECTS);
    if( (pSheet = Find(aName, SfxStyleFamily::Pseudo)) == nullptr )
    {
        pSheet = &Make(aName, SfxStyleFamily::Pseudo, nUsedMask);
        pSheet->SetParent( OUString() );
        static_cast<SfxStyleSheet*>(pSheet)->StartListening(*this);
    }
    pSheet->SetHelpId( aHelpFile, HID_PSEUDOSHEET_BACKGROUNDOBJECTS );

    aName = SdResId(STR_PSEUDOSHEET_BACKGROUND);
    if( (pSheet = Find(aName, SfxStyleFamily::Pseudo)) == nullptr )
    {
        pSheet = &Make(aName, SfxStyleFamily::Pseudo, nUsedMask);
        pSheet->SetParent( OUString() );
        static_cast<SfxStyleSheet*>(pSheet)->StartListening(*this);
    }
    pSheet->SetHelpId( aHelpFile, HID_PSEUDOSHEET_BACKGROUND );

    aName = SdResId(STR_PSEUDOSHEET_NOTES);
    if( (pSheet = Find(aName, SfxStyleFamily::Pseudo)) == nullptr )
    {
        pSheet = &Make(aName, SfxStyleFamily::Pseudo, nUsedMask);
        pSheet->SetParent( OUString() );
        static_cast<SfxStyleSheet*>(pSheet)->StartListening(*this);
    }
    pSheet->SetHelpId( aHelpFile, HID_PSEUDOSHEET_NOTES );

    pParent = nullptr;
    SetSearchMask(SfxStyleFamily::Pseudo);
    aName = SdResId(STR_PSEUDOSHEET_OUTLINE);
    for (sal_Int32 nLevel = 1; nLevel < 10; nLevel++)
    {
        OUString aLevelName( aName + " " + OUString::number( nLevel ) );

        if( (pSheet = Find(aLevelName, SfxStyleFamily::Pseudo)) == nullptr )
        {
            pSheet = &Make(aLevelName, SfxStyleFamily::Pseudo, nUsedMask);

            if (pParent)
                pSheet->SetParent(pParent->GetName());
            pParent = pSheet;
            static_cast<SfxStyleSheet*>(pSheet)->StartListening(*this);
        }
        pSheet->SetHelpId( aHelpFile, HID_PSEUDOSHEET_OUTLINE + nLevel );
    }
}

/*************************************************************************
|*
|* Set the correct name in the program language to the standard styles
|*
\************************************************************************/

namespace
{
struct StyleSheetIsUserDefinedPredicate : svl::StyleSheetPredicate
{
    StyleSheetIsUserDefinedPredicate()
    {}

    bool Check(const SfxStyleSheetBase& sheet) override
    {
        return sheet.IsUserDefined();
    }
};
}

void SdStyleSheetPool::UpdateStdNames()
{
    OUString aHelpFile;
    StyleSheetIsUserDefinedPredicate aPredicate;
    std::vector<SfxStyleSheetBase*> aEraseList;
    std::vector<unsigned> aUserDefinedStyles = GetIndexedStyleSheets().FindPositionsByPredicate(aPredicate);
    for (const auto& rStyle : aUserDefinedStyles)
    {
        SfxStyleSheetBase* pStyle = GetStyleSheetByPositionInIndex(rStyle);

        if( !pStyle->IsUserDefined() )
        {
            OUString aOldName   = pStyle->GetName();
            sal_uLong nHelpId   = pStyle->GetHelpId( aHelpFile );
            SfxStyleFamily eFam = pStyle->GetFamily();

            bool bHelpKnown = true;
            OUString aNewName;
            const char* pNameId = nullptr;
            switch( nHelpId )
            {
                case HID_STANDARD_STYLESHEET_NAME:  pNameId = STR_STANDARD_STYLESHEET_NAME; break;
                case HID_POOLSHEET_OBJWITHOUTFILL:  pNameId = STR_POOLSHEET_OBJWITHOUTFILL; break;
                case HID_POOLSHEET_OBJNOLINENOFILL: pNameId = STR_POOLSHEET_OBJNOLINENOFILL;break;
                case HID_POOLSHEET_TEXT:            pNameId = STR_POOLSHEET_TEXT;           break;
                case HID_POOLSHEET_A4:              pNameId = STR_POOLSHEET_A4;             break;
                case HID_POOLSHEET_A4_TITLE:        pNameId = STR_POOLSHEET_A4_TITLE;       break;
                case HID_POOLSHEET_A4_HEADLINE:     pNameId = STR_POOLSHEET_A4_HEADLINE;    break;
                case HID_POOLSHEET_A4_TEXT:         pNameId = STR_POOLSHEET_A4_TEXT;        break;
                case HID_POOLSHEET_A0:              pNameId = STR_POOLSHEET_A0;             break;
                case HID_POOLSHEET_A0_TITLE:        pNameId = STR_POOLSHEET_A0_TITLE;       break;
                case HID_POOLSHEET_A0_HEADLINE:     pNameId = STR_POOLSHEET_A0_HEADLINE;    break;
                case HID_POOLSHEET_A0_TEXT:         pNameId = STR_POOLSHEET_A0_TEXT;        break;
                case HID_POOLSHEET_GRAPHIC:         pNameId = STR_POOLSHEET_GRAPHIC;        break;
                case HID_POOLSHEET_SHAPES:          pNameId = STR_POOLSHEET_SHAPES;         break;
                case HID_POOLSHEET_FILLED:          pNameId = STR_POOLSHEET_FILLED;         break;
                case HID_POOLSHEET_FILLED_BLUE:     pNameId = STR_POOLSHEET_FILLED_BLUE;    break;
                case HID_POOLSHEET_FILLED_GREEN:    pNameId = STR_POOLSHEET_FILLED_GREEN;   break;
                case HID_POOLSHEET_FILLED_RED:      pNameId = STR_POOLSHEET_FILLED_RED;     break;
                case HID_POOLSHEET_FILLED_YELLOW:   pNameId = STR_POOLSHEET_FILLED_YELLOW;  break;
                case HID_POOLSHEET_OUTLINE:         pNameId = STR_POOLSHEET_OUTLINE;        break;
                case HID_POOLSHEET_OUTLINE_BLUE:    pNameId = STR_POOLSHEET_OUTLINE_BLUE;   break;
                case HID_POOLSHEET_OUTLINE_GREEN:   pNameId = STR_POOLSHEET_OUTLINE_GREEN;  break;
                case HID_POOLSHEET_OUTLINE_RED:     pNameId = STR_POOLSHEET_OUTLINE_RED;    break;
                case HID_POOLSHEET_OUTLINE_YELLOW:  pNameId = STR_POOLSHEET_OUTLINE_YELLOW; break;
                case HID_POOLSHEET_LINES:           pNameId = STR_POOLSHEET_LINES;          break;
                case HID_POOLSHEET_MEASURE:         pNameId = STR_POOLSHEET_MEASURE;        break;
                case HID_POOLSHEET_LINES_DASHED:    pNameId = STR_POOLSHEET_LINES_DASHED;   break;

                case HID_PSEUDOSHEET_OUTLINE1:
                case HID_PSEUDOSHEET_OUTLINE2:
                case HID_PSEUDOSHEET_OUTLINE3:
                case HID_PSEUDOSHEET_OUTLINE4:
                case HID_PSEUDOSHEET_OUTLINE5:
                case HID_PSEUDOSHEET_OUTLINE6:
                case HID_PSEUDOSHEET_OUTLINE7:
                case HID_PSEUDOSHEET_OUTLINE8:
                case HID_PSEUDOSHEET_OUTLINE9:      pNameId = STR_PSEUDOSHEET_OUTLINE;      break;
                case HID_PSEUDOSHEET_BACKGROUNDOBJECTS: pNameId = STR_PSEUDOSHEET_BACKGROUNDOBJECTS; break;
                case HID_PSEUDOSHEET_BACKGROUND:    pNameId = STR_PSEUDOSHEET_BACKGROUND;   break;
                case HID_PSEUDOSHEET_NOTES:         pNameId = STR_PSEUDOSHEET_NOTES;        break;

                case HID_SD_CELL_STYLE_DEFAULT:         pNameId = STR_STANDARD_STYLESHEET_NAME; break;
                case HID_SD_CELL_STYLE_BANDED:          pNameId = STR_POOLSHEET_BANDED_CELL; break;
                case HID_SD_CELL_STYLE_HEADER:          pNameId = STR_POOLSHEET_HEADER; break;
                case HID_SD_CELL_STYLE_TOTAL:           pNameId = STR_POOLSHEET_TOTAL; break;
                case HID_SD_CELL_STYLE_FIRST_COLUMN:    pNameId = STR_POOLSHEET_FIRST_COLUMN; break;
                case HID_SD_CELL_STYLE_LAST_COLUMN:     pNameId = STR_POOLSHEET_LAST_COLUMN; break;

                default:
                    // 0 or wrong (old) HelpId
                    bHelpKnown = false;
            }
            if( bHelpKnown )
            {
                if (pNameId)
                {
                    aNewName = SdResId(pNameId);
                    if (strcmp(pNameId, STR_PSEUDOSHEET_OUTLINE) == 0)
                    {
                        aNewName += " " + OUString::number( sal_Int32( nHelpId - HID_PSEUDOSHEET_OUTLINE ) );
                    }
                }

                if( !aNewName.isEmpty() && aNewName != aOldName )
                {
                    SfxStyleSheetBase* pSheetFound = Find( aNewName, eFam );

                    if ( !pSheetFound )
                    {
                        // Sheet does not yet exist: rename old sheet
                        pStyle->SetName( aNewName );    // transform also parents
                    }
                    else
                    {
                        // Sheet does exist: old sheet has to be removed
                        aEraseList.push_back( pStyle );
                    }
                }
            }
        }
    }

    if (!aEraseList.empty())
    {
        // styles that could not be renamed, must be removed
        for (SfxStyleSheetBase* p : aEraseList)
            Remove( p );
        Reindex();
    }
}

void SdStyleSheetPool::setDefaultOutlineNumberFormatBulletAndIndent(sal_uInt16 i, SvxNumberFormat &rNumberFormat)
{
    rNumberFormat.SetBulletChar( 0x25CF );  // StarBats: 0xF000 + 34
    rNumberFormat.SetBulletRelSize(45);
    const auto nLSpace = (i + 1) * 1200;
    rNumberFormat.SetAbsLSpace(nLSpace);
    sal_Int32 nFirstLineOffset = -600;

    switch(i)
    {
        case 0:
        {
            nFirstLineOffset = -900;
        }
        break;

        case 1:
        {
            rNumberFormat.SetBulletChar( 0x2013 );  // StarBats: 0xF000 + 150
            rNumberFormat.SetBulletRelSize(75);
            nFirstLineOffset = -900;
        }
        break;

        case 2:
        {
            nFirstLineOffset = -800;
        }
        break;

        case 3:
        {
            rNumberFormat.SetBulletChar( 0x2013 );  // StarBats: 0xF000 + 150
            rNumberFormat.SetBulletRelSize(75);
        }
        break;
    }

    rNumberFormat.SetFirstLineOffset(nFirstLineOffset);
}

// Set new SvxNumBulletItem for the respective style sheet
void SdStyleSheetPool::PutNumBulletItem( SfxStyleSheetBase* pSheet,
                                         vcl::Font& rBulletFont )
{
    OUString aHelpFile;
    sal_uLong nHelpId = pSheet->GetHelpId( aHelpFile );
    SfxItemSet& rSet = pSheet->GetItemSet();

    switch ( nHelpId )
    {
        case HID_STANDARD_STYLESHEET_NAME :
        {
            // Standard template
            SvxNumberFormat aNumberFormat(SVX_NUM_CHAR_SPECIAL);
            aNumberFormat.SetBulletFont(&rBulletFont);
            aNumberFormat.SetBulletChar( 0x25CF ); // StarBats: 0xF000 + 34
            aNumberFormat.SetBulletRelSize(45);
            aNumberFormat.SetBulletColor(COL_AUTO);
            aNumberFormat.SetStart(1);
            aNumberFormat.SetNumAdjust(SvxAdjust::Left);

            SvxNumRule aNumRule( SvxNumRuleFlags::BULLET_REL_SIZE | SvxNumRuleFlags::BULLET_COLOR, SVX_MAX_NUM, false);

            for( sal_uInt16 i = 0; i < aNumRule.GetLevelCount(); i++ )
            {
                const auto nLSpace = (i + 1) * 600;
                aNumberFormat.SetAbsLSpace(nLSpace);
                aNumberFormat.SetFirstLineOffset(-600);
                aNumRule.SetLevel( i, aNumberFormat );
            }

            rSet.Put( SvxNumBulletItem( aNumRule, EE_PARA_NUMBULLET ) );
            static_cast<SfxStyleSheet*>(pSheet)->Broadcast(SfxHint( SfxHintId::DataChanged ) );
        }
        break;

        case HID_PSEUDOSHEET_TITLE:
            /* title gets same bullet as subtitle and not that page symbol anymore */
        case HID_PSEUDOSHEET_SUBTITLE :
        {
            // Subtitle template
            SvxNumBulletItem const*const pItem(
                    rSet.GetPool()->GetSecondaryPool()->GetPoolDefaultItem(EE_PARA_NUMBULLET));
            SvxNumRule *const pDefaultRule = pItem ? pItem->GetNumRule() : nullptr;
            DBG_ASSERT( pDefaultRule, "Where is my default template? [CL]" );

            if(pDefaultRule)
            {
                SvxNumRule aNumRule(pDefaultRule->GetFeatureFlags(), 10, false);
                for(sal_uInt16 i=0; i < aNumRule.GetLevelCount(); i++)
                {
                    SvxNumberFormat aFrmt( pDefaultRule->GetLevel(i) );
                    aFrmt.SetNumberingType(SVX_NUM_CHAR_SPECIAL);
                    // #i93908# clear suffix for bullet lists
                    aFrmt.SetPrefix(OUString());
                    aFrmt.SetSuffix(OUString());
                    aFrmt.SetStart(1);
                    aFrmt.SetBulletRelSize(45);
                    aFrmt.SetBulletChar( 0x25CF );  // StarBats: 0xF000 + 34
                    aFrmt.SetBulletFont(&rBulletFont);
                    aNumRule.SetLevel(i, aFrmt);
                }

                rSet.Put( SvxNumBulletItem( aNumRule, EE_PARA_NUMBULLET ) );
                static_cast<SfxStyleSheet*>(pSheet)->Broadcast(SfxHint( SfxHintId::DataChanged ) );
            }
        }
        break;

        case HID_PSEUDOSHEET_OUTLINE + 1 :
        {
            // Outline template
            SvxNumberFormat aNumberFormat(SVX_NUM_CHAR_SPECIAL);
            aNumberFormat.SetBulletColor(COL_AUTO);
            aNumberFormat.SetStart(1);
            aNumberFormat.SetNumAdjust(SvxAdjust::Left);

            SvxNumRule aNumRule( SvxNumRuleFlags::BULLET_REL_SIZE | SvxNumRuleFlags::BULLET_COLOR,
                                 SVX_MAX_NUM, false );
            for( sal_uInt16 i = 0; i < aNumRule.GetLevelCount(); i++ )
            {
                setDefaultOutlineNumberFormatBulletAndIndent(i, aNumberFormat);
                rBulletFont.SetFontSize(Size(0,846));       // 24 pt
                aNumberFormat.SetBulletFont(&rBulletFont);
                aNumRule.SetLevel( i, aNumberFormat );
            }

            rSet.Put( SvxNumBulletItem( aNumRule, EE_PARA_NUMBULLET ) );
            static_cast<SfxStyleSheet*>(pSheet)->Broadcast(SfxHint( SfxHintId::DataChanged ) );
        }
        break;
    }
}

/*************************************************************************
|*
|* Create standard bullet font (without size)
|*
\************************************************************************/

vcl::Font SdStyleSheetPool::GetBulletFont()
{
    vcl::Font aBulletFont( OUString( "StarSymbol" ), Size(0, 1000) );
    aBulletFont.SetCharSet(RTL_TEXTENCODING_UNICODE);
    aBulletFont.SetWeight(WEIGHT_NORMAL);
    aBulletFont.SetUnderline(LINESTYLE_NONE);
    aBulletFont.SetOverline(LINESTYLE_NONE);
    aBulletFont.SetStrikeout(STRIKEOUT_NONE);
    aBulletFont.SetItalic(ITALIC_NONE);
    aBulletFont.SetOutline(false);
    aBulletFont.SetShadow(false);
    aBulletFont.SetColor(COL_AUTO);
    aBulletFont.SetTransparent(true);

    return aBulletFont;
}

void SdStyleSheetPool::AddStyleFamily( const SdPage* pPage )
{
    rtl::Reference< SfxStyleSheetPool > xPool( this );
    maStyleFamilyMap[pPage] = new SdStyleFamily( xPool, pPage );
}

void SdStyleSheetPool::RemoveStyleFamily( const SdPage* pPage )
{
    SdStyleFamilyMap::iterator iter( maStyleFamilyMap.find( pPage ) );
    if( iter == maStyleFamilyMap.end() )
        return;

    SdStyleFamilyRef xStyle( (*iter).second );
    maStyleFamilyMap.erase( iter );

    if( xStyle.is() ) try
    {
        xStyle->dispose();
    }
    catch( Exception& )
    {
    }
}

void SdStyleSheetPool::throwIfDisposed()
{
    if( mpDoc == nullptr )
        throw DisposedException();
}

// XServiceInfo
OUString SAL_CALL SdStyleSheetPool::getImplementationName()
{
    return OUString( "SdStyleSheetPool" );
}

sal_Bool SAL_CALL SdStyleSheetPool::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService(this, ServiceName);
}

Sequence< OUString > SAL_CALL SdStyleSheetPool::getSupportedServiceNames()
{
    OUString aStr("com.sun.star.style.StyleFamilies");
    return Sequence< OUString >( &aStr, 1 );
}

// XNameAccess
Any SAL_CALL SdStyleSheetPool::getByName( const OUString& aName )
{
    throwIfDisposed();

    if( mxGraphicFamily->getName() == aName )
        return Any( Reference< XNameAccess >( static_cast< XNameAccess* >( mxGraphicFamily.get() ) ) );

    if( mxCellFamily->getName() == aName )
        return Any( Reference< XNameAccess >( static_cast< XNameAccess* >( mxCellFamily.get() ) ) );

    if( msTableFamilyName == aName )
        return Any( mxTableFamily );

    auto iter = std::find_if(maStyleFamilyMap.begin(), maStyleFamilyMap.end(),
            [&aName](const SdStyleFamilyMap::value_type& rEntry) { return rEntry.second->getName() == aName; });
    if (iter != maStyleFamilyMap.end())
        return Any( Reference< XNameAccess >( static_cast< XNameAccess* >( (*iter).second.get() ) ) );

    throw NoSuchElementException();
}

Sequence< OUString > SAL_CALL SdStyleSheetPool::getElementNames()
{
    throwIfDisposed();

    Sequence< OUString > aNames( maStyleFamilyMap.size() + 3 );
    OUString* pNames = aNames.getArray();

    *pNames++ = mxGraphicFamily->getName();
    *pNames++ = mxCellFamily->getName();
    *pNames++ = msTableFamilyName;

    for( const auto& rEntry : maStyleFamilyMap )
    {
        *pNames++ = rEntry.second->getName();
    }

    return aNames;
}

sal_Bool SAL_CALL SdStyleSheetPool::hasByName( const OUString& aName )
{
    throwIfDisposed();

    if( mxGraphicFamily->getName() == aName )
        return true;

    if( mxCellFamily->getName() == aName )
        return true;

    if( msTableFamilyName == aName )
        return true;

    return std::any_of(maStyleFamilyMap.begin(), maStyleFamilyMap.end(),
        [&aName](const SdStyleFamilyMap::value_type& rEntry) { return rEntry.second->getName() == aName; });
}

// XElementAccess

Type SAL_CALL SdStyleSheetPool::getElementType()
{
    throwIfDisposed();

    return cppu::UnoType<XNameAccess>::get();
}

sal_Bool SAL_CALL SdStyleSheetPool::hasElements()
{
    return true;
}

// XIndexAccess

sal_Int32 SAL_CALL SdStyleSheetPool::getCount()
{
    throwIfDisposed();

    return maStyleFamilyMap.size() + 3;
}

Any SAL_CALL SdStyleSheetPool::getByIndex( sal_Int32 Index )
{
    switch( Index )
    {
    case 0:
        return Any( Reference< XNameAccess >( static_cast< XNameAccess* >( mxGraphicFamily.get() ) ) );

    case 1:
        return Any( Reference< XNameAccess >( static_cast< XNameAccess* >( mxCellFamily.get() ) ) );

    case 2:
        return Any( mxTableFamily );

    default:
        {
            Index -= 3;
            if( (Index < 0) || (Index >= sal::static_int_cast<sal_Int32>(maStyleFamilyMap.size())) )
                throw IndexOutOfBoundsException();
            SdStyleFamilyMap::iterator iter( maStyleFamilyMap.begin() );
            std::advance(iter, Index);

            return Any( Reference< XNameAccess >( static_cast< XNameAccess* >( (*iter).second.get() ) ) );
        }
    }
}

// XComponent

void SAL_CALL SdStyleSheetPool::dispose()
{
    if( !mpDoc )
        return;

    mxGraphicFamily->dispose();
    mxGraphicFamily.clear();
    mxCellFamily->dispose();
    mxCellFamily.clear();

    Reference< XComponent > xComp( mxTableFamily, UNO_QUERY );
    if( xComp.is() )
        xComp->dispose();
    mxTableFamily = nullptr;

    SdStyleFamilyMap aTempMap;
    aTempMap.swap( maStyleFamilyMap );

    for( auto& rEntry : aTempMap ) try
    {
        rEntry.second->dispose();
    }
    catch( Exception& )
    {
    }

    mpDoc = nullptr;

    Clear();
}

void SAL_CALL SdStyleSheetPool::addEventListener( const Reference< XEventListener >& /*xListener*/ )
{
}

void SAL_CALL SdStyleSheetPool::removeEventListener( const Reference< XEventListener >& /*aListener*/ )
{
}

SdStyleSheetVector SdStyleSheetPool::CreateChildList( SdStyleSheet const * pSheet )
{
    SdStyleSheetVector aResult;

    const size_t nListenerCount = pSheet->GetSizeOfVector();
    for (size_t n = 0; n < nListenerCount; ++n)
    {
        SdStyleSheet* pChild = dynamic_cast< SdStyleSheet* >( pSheet->GetListener(n) );
        if(pChild && pChild->GetParent() == pSheet->GetName())
        {
            aResult.emplace_back( pChild );
        }
    }

    return aResult;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

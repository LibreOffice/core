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

#include <sal/config.h>

#include <com/sun/star/style/XStyle.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <i18nlangtag/mslangid.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/dispatch.hxx>
#include "Outliner.hxx"
#include <comphelper/processfactory.hxx>
#include <editeng/outliner.hxx>

#include "../ui/inc/DrawDocShell.hxx"
#include <editeng/eeitem.hxx>

#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>

#include <editeng/autokernitem.hxx>

#include <svx/svxids.hrc>
#include <svl/srchitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/lspcitem.hxx>
#include <editeng/adjustitem.hxx>
#include <editeng/numdef.hxx>
#include <svx/dialogs.hrc>
#include <svx/dialmgr.hxx>
#include <editeng/bulletitem.hxx>
#include <svx/xtable.hxx>
#include <editeng/borderline.hxx>
#include <editeng/boxitem.hxx>
#include <svx/xit.hxx>
#include <svx/xlineit0.hxx>
#include <svx/sdshitm.hxx>
#include <svx/svdotext.hxx>
#include <svx/xfillit0.hxx>
#include <svx/sdshcitm.hxx>
#include <editeng/editstat.hxx>
#include <editeng/colritem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/crossedoutitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/contouritem.hxx>
#include <editeng/emphasismarkitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/shdditem.hxx>
#include <editeng/cmapitem.hxx>
#include <svx/xbtmpit.hxx>
#include <svx/xflhtit.hxx>
#include <svx/xflgrit.hxx>
#include <svx/xflclit.hxx>
#include <svx/xlnedcit.hxx>
#include <svx/xlnstcit.hxx>
#include <svx/xlnedwit.hxx>
#include <svx/xlnstwit.hxx>
#include <svx/xlnedit.hxx>
#include <editeng/charreliefitem.hxx>
#include <svx/xlnstit.hxx>
#include <svx/xlndsit.hxx>
#include <svx/xlnwtit.hxx>
#include <svx/xlnclit.hxx>
#include <svx/svditer.hxx>
#include <svx/svdogrp.hxx>
#include <tools/shl.hxx>
#include <editeng/numitem.hxx>
#include <editeng/editeng.hxx>
#include <editeng/unolingu.hxx>
#include <com/sun/star/linguistic2/XHyphenator.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <svl/itempool.hxx>
#include <editeng/outlobj.hxx>
#include <sfx2/viewfrm.hxx>
#include <editeng/langitem.hxx>
#include <editeng/frmdiritem.hxx>

#include "sdresid.hxx"
#include "drawdoc.hxx"
#include "sdpage.hxx"
#include "glob.hrc"
#include "glob.hxx"
#include "stlpool.hxx"
#include "helpids.h"
#include "sdiocmpt.hxx"
#include "shapelist.hxx"
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <svl/itemset.hxx>
#include "app.hrc"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::style;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::linguistic2;
using namespace ::sd;

// CreateLayoutTemplates
// At the moment (31.03.1995), the StyleSheetPool only saves styleheets that
// have an ItemSet. To save all stylesheets, we force the creation of an ItemSet
// with a GetItemSet call.
// We can remove this behavior once the pool saves styleheets even without an ItemSet
void SdDrawDocument::CreateLayoutTemplates()
{
    SdStyleSheetPool*       pSSPool = static_cast<SdStyleSheetPool*>(GetStyleSheetPool());
    SfxStyleSheetBase*      pSheet = nullptr;
    OUString                aHelpFile;
    OUString                aStdName(SD_RESSTR(STR_STANDARD_STYLESHEET_NAME));

    // Default style

    sal_uInt16 nMask = SFXSTYLEBIT_AUTO;

    OUString aName(aStdName);
    pSheet = &(pSSPool->Make(aName, SD_STYLE_FAMILY_GRAPHICS, nMask));
    pSheet->SetHelpId( aHelpFile, HID_STANDARD_STYLESHEET_NAME );
    SfxItemSet& rISet = pSheet->GetItemSet();
    SfxItemPool* pPool = rISet.GetPool();

    ::basegfx::B2DPolyPolygon aNullPolyPolygon;
    Color    aNullCol(RGB_Color(COL_DEFAULT_SHAPE_STROKE));

    XDash     aNullDash;
    XGradient aNullGrad(aNullCol,RGB_Color(COL_WHITE));
              aNullGrad.SetStartIntens( 100 );
              aNullGrad.SetEndIntens( 100 );
    XHatch    aNullHatch(aNullCol);

                    // Line attributes (Extended OutputDevice)
    rISet.Put(XLineStyleItem(drawing::LineStyle_SOLID));
    rISet.Put(XLineColorItem(OUString(), RGB_Color(COL_DEFAULT_SHAPE_STROKE)));
    rISet.Put(XLineWidthItem(0));
    rISet.Put(XLineDashItem(pPool,aNullDash));
    rISet.Put(XLineStartItem(pPool,aNullPolyPolygon));
    rISet.Put(XLineEndItem(pPool,aNullPolyPolygon));
    rISet.Put(XLineStartWidthItem(200));
    rISet.Put(XLineEndWidthItem(200));
    rISet.Put(XLineStartCenterItem());
    rISet.Put(XLineEndCenterItem());
    rISet.Put(SdrTextHorzAdjustItem(SDRTEXTHORZADJUST_BLOCK));

                    // Fill attributes (Extended OutputDevice)
    rISet.Put(XFillStyleItem(drawing::FillStyle_SOLID));
    rISet.Put(XFillColorItem(OUString(), RGB_Color(COL_DEFAULT_SHAPE_FILLING)));

    rISet.Put( XFillGradientItem( aNullGrad) );
    rISet.Put(XFillHatchItem(pPool,aNullHatch));
    Size    aNullSize( 32, 32 );
    Color   aNullColor( COL_WHITE );
    Bitmap  aNullBmp( aNullSize, 8 );
    aNullBmp.Erase( aNullColor );
    rISet.Put(XFillBitmapItem(pPool, Graphic(aNullBmp)));

                    // Shadow attributes (Drawing Engine)
    rISet.Put(makeSdrShadowItem(false));
    rISet.Put(makeSdrShadowColorItem(RGB_Color(COL_GRAY)));
    rISet.Put(makeSdrShadowXDistItem(200));         // 3 mm Shadow distance
    rISet.Put(makeSdrShadowYDistItem(200));

    vcl::Font aLatinFont, aCJKFont, aCTLFont;

    getDefaultFonts( aLatinFont, aCJKFont, aCTLFont );

    SvxFontItem aSvxFontItem( aLatinFont.GetFamily(), aLatinFont.GetFamilyName(), aLatinFont.GetStyleName(), aLatinFont.GetPitch(),
                              aLatinFont.GetCharSet(), EE_CHAR_FONTINFO );

    SvxFontItem aSvxFontItemCJK( aCJKFont.GetFamily(), aCJKFont.GetFamilyName(), aCJKFont.GetStyleName(), aCJKFont.GetPitch(),
                                 aCJKFont.GetCharSet(), EE_CHAR_FONTINFO_CJK );

    SvxFontItem aSvxFontItemCTL( aCTLFont.GetFamily(), aCTLFont.GetFamilyName(), aCTLFont.GetStyleName(), aCTLFont.GetPitch(),
                                 aCTLFont.GetCharSet(), EE_CHAR_FONTINFO_CTL );

    rISet.Put( aSvxFontItem );
    rISet.Put( aSvxFontItemCJK );
    rISet.Put( aSvxFontItemCTL );

    rISet.Put( SvxFontHeightItem( 635, 100, EE_CHAR_FONTHEIGHT ) );     // sj: (i33745) changed default from 24 to 18 pt
    rISet.Put( SvxFontHeightItem( 635, 100, EE_CHAR_FONTHEIGHT_CJK ) ); // 18 pt
    rISet.Put( SvxFontHeightItem( convertFontHeightToCTL( 635 ), 100, EE_CHAR_FONTHEIGHT_CTL ) ); // 18 pt

    rISet.Put( SvxWeightItem( WEIGHT_NORMAL, EE_CHAR_WEIGHT ) );
    rISet.Put( SvxWeightItem( WEIGHT_NORMAL, EE_CHAR_WEIGHT_CJK ) );
    rISet.Put( SvxWeightItem( WEIGHT_NORMAL, EE_CHAR_WEIGHT_CTL ) );

    rISet.Put( SvxPostureItem( ITALIC_NONE, EE_CHAR_ITALIC ) );
    rISet.Put( SvxPostureItem( ITALIC_NONE, EE_CHAR_ITALIC_CJK ) );
    rISet.Put( SvxPostureItem( ITALIC_NONE, EE_CHAR_ITALIC_CTL ) );

    rISet.Put(SvxContourItem(false, EE_CHAR_OUTLINE ));
    rISet.Put(SvxShadowedItem(false, EE_CHAR_SHADOW ));
    rISet.Put(SvxUnderlineItem(UNDERLINE_NONE, EE_CHAR_UNDERLINE));
    rISet.Put(SvxOverlineItem(UNDERLINE_NONE, EE_CHAR_OVERLINE));
    rISet.Put(SvxCrossedOutItem(STRIKEOUT_NONE, EE_CHAR_STRIKEOUT ));
    rISet.Put(SvxCaseMapItem(SVX_CASEMAP_NOT_MAPPED, EE_CHAR_CASEMAP ));
    rISet.Put(SvxEmphasisMarkItem(EMPHASISMARK_NONE, EE_CHAR_EMPHASISMARK));
    rISet.Put(SvxCharReliefItem(RELIEF_NONE, EE_CHAR_RELIEF));
    rISet.Put(SvxColorItem(Color(COL_AUTO), EE_CHAR_COLOR ));

    // Paragraph attributes (Edit Engine)
    rISet.Put(SvxLRSpaceItem(EE_PARA_LRSPACE));
    rISet.Put(SvxULSpaceItem(EE_PARA_ULSPACE));

    rISet.Put( makeSdrTextLeftDistItem( 250 ) );    // sj: (i33745) using text frame distances seems to be a better default
    rISet.Put( makeSdrTextRightDistItem( 250 ) );
    rISet.Put( makeSdrTextUpperDistItem( 125 ) );
    rISet.Put( makeSdrTextLowerDistItem( 125 ) );

    rISet.Put( SvxLineSpacingItem( LINE_SPACE_DEFAULT_HEIGHT, EE_PARA_SBL ) );

    // #i16874# enable kerning by default but only for new documents
    rISet.Put( SvxAutoKernItem( true, EE_CHAR_PAIRKERNING ) );

    // Bullet
    // BulletItem and BulletFont for title and outline
    SvxBulletItem aBulletItem(EE_PARA_BULLET);
                            // Identical in all layers
    aBulletItem.SetStyle(SvxBulletStyle::BULLET);
    aBulletItem.SetStart(1);
    aBulletItem.SetScale(45);           // In percent

    vcl::Font aBulletFont( SdStyleSheetPool::GetBulletFont() );

    aBulletFont.SetSize(Size(0,635));   // sj: (i33745) changed default from 24 to 18 pt

    aBulletItem.SetFont(aBulletFont);
    aBulletItem.SetSymbol( 0x25CF );                    // In points
    rISet.Put(aBulletItem);

    // New BulletItem
    SdStyleSheetPool::PutNumBulletItem( pSheet, aBulletFont );

    SfxItemSet* pISet = nullptr;

    // Object with arrowhead
    aName = SD_RESSTR(STR_POOLSHEET_OBJWITHARROW);
    pSheet = &(pSSPool->Make(aName, SD_STYLE_FAMILY_GRAPHICS, nMask));
    pSheet->SetParent(aStdName);
    pSheet->SetHelpId( aHelpFile, HID_POOLSHEET_OBJWITHARROW );
    pISet = &pSheet->GetItemSet();

    pISet->Put(XLineStyleItem(drawing::LineStyle_SOLID));
    pISet->Put(XLineColorItem(OUString(), RGB_Color(COL_BLACK)));
    pISet->Put(XLineWidthItem(150));

    ::basegfx::B2DPolygon aArrow;
    aArrow.append(::basegfx::B2DPoint(10.0, 0.0));
    aArrow.append(::basegfx::B2DPoint(0.0, 30.0));
    aArrow.append(::basegfx::B2DPoint(20.0, 30.0));
    aArrow.setClosed(true);
    pISet->Put(XLineStartItem(SVX_RESSTR(RID_SVXSTR_ARROW),::basegfx::B2DPolyPolygon(aArrow)));

    pISet->Put(XLineStartWidthItem(700));
    pISet->Put(XLineEndWidthItem(300));
    pISet->Put(XLineStartCenterItem(true));

    // Object with Shadow
    aName = SD_RESSTR(STR_POOLSHEET_OBJWITHSHADOW);
    pSheet = &(pSSPool->Make(aName, SD_STYLE_FAMILY_GRAPHICS, nMask));
    pSheet->SetParent(aStdName);
    pSheet->SetHelpId( aHelpFile, HID_POOLSHEET_OBJWITHSHADOW );
    pISet = &pSheet->GetItemSet();

    pISet->Put(makeSdrShadowItem(true));
    pISet->Put(makeSdrShadowColorItem(RGB_Color(COL_GRAY)));
    pISet->Put(makeSdrShadowXDistItem(200));        // 3 mm shadow distance
    pISet->Put(makeSdrShadowYDistItem(200));

    // Object without fillung
    aName = SD_RESSTR(STR_POOLSHEET_OBJWITHOUTFILL);
    pSheet = &(pSSPool->Make(aName, SD_STYLE_FAMILY_GRAPHICS, nMask));
    pSheet->SetParent(aStdName);
    pSheet->SetHelpId( aHelpFile, HID_POOLSHEET_OBJWITHOUTFILL );
    pISet = &pSheet->GetItemSet();

    pISet->Put(XFillStyleItem(drawing::FillStyle_NONE));
    pISet->Put(XLineColorItem(OUString(), RGB_Color(COL_BLACK)));

    // Object no fill no line

    aName = SD_RESSTR(STR_POOLSHEET_OBJNOLINENOFILL);
    pSheet = &(pSSPool->Make(aName, SD_STYLE_FAMILY_GRAPHICS, nMask));
    pSheet->SetParent(aStdName);
    pSheet->SetHelpId( aHelpFile, HID_POOLSHEET_OBJNOLINENOFILL );
    pISet = &pSheet->GetItemSet();

    pISet->Put(XFillStyleItem(drawing::FillStyle_NONE));
    pISet->Put(XLineStyleItem(drawing::LineStyle_NONE));

    // Text

    aName = SD_RESSTR(STR_POOLSHEET_TEXT);
    pSheet = &(pSSPool->Make(aName, SD_STYLE_FAMILY_GRAPHICS, nMask));
    pSheet->SetParent(aStdName);
    pSheet->SetHelpId( aHelpFile, HID_POOLSHEET_TEXT );
    pISet = &pSheet->GetItemSet();

    pISet->Put(XLineStyleItem(drawing::LineStyle_NONE));
    pISet->Put(XFillStyleItem(drawing::FillStyle_NONE));

    // Text body
    aName = SD_RESSTR(STR_POOLSHEET_TEXTBODY);
    pSheet = &(pSSPool->Make(aName, SD_STYLE_FAMILY_GRAPHICS, nMask));
    pSheet->SetParent(aStdName);
    pSheet->SetHelpId( aHelpFile, HID_POOLSHEET_TEXTBODY );
    pISet = &pSheet->GetItemSet();

    pISet->Put(XLineStyleItem(drawing::LineStyle_NONE));
    pISet->Put(XFillStyleItem(drawing::FillStyle_NONE));

    pISet->Put(SvxFontHeightItem(564, 100, EE_CHAR_FONTHEIGHT));        // 16 pt

    // Text body, justified
    aName = SD_RESSTR(STR_POOLSHEET_TEXTBODY_JUSTIFY);
    pSheet = &(pSSPool->Make(aName, SD_STYLE_FAMILY_GRAPHICS, nMask));
    pSheet->SetParent(aStdName);
    pSheet->SetHelpId( aHelpFile, HID_POOLSHEET_TEXTBODY_JUSTIFY );
    pISet = &pSheet->GetItemSet();

    pISet->Put(XLineStyleItem(drawing::LineStyle_NONE));
    pISet->Put(XFillStyleItem(drawing::FillStyle_NONE));

    pISet->Put(SvxAdjustItem(SVX_ADJUST_BLOCK, EE_PARA_JUST ));

    // Text body, indented
    aName = SD_RESSTR(STR_POOLSHEET_TEXTBODY_INDENT);
    pSheet = &(pSSPool->Make(aName, SD_STYLE_FAMILY_GRAPHICS, nMask));
    pSheet->SetParent(aStdName);
    pSheet->SetHelpId( aHelpFile, HID_POOLSHEET_TEXTBODY_INDENT );
    pISet = &pSheet->GetItemSet();

    pISet->Put(XLineStyleItem(drawing::LineStyle_NONE));
    pISet->Put(XFillStyleItem(drawing::FillStyle_NONE));

    SvxLRSpaceItem aLRSpaceItem( EE_PARA_LRSPACE );
    aLRSpaceItem.SetTextFirstLineOfst(600);      // Indentation of first line: 6mm; right: 0
    pISet->Put(aLRSpaceItem);

    // Title

    aName = SD_RESSTR(STR_POOLSHEET_TITLE);
    pSheet = &(pSSPool->Make(aName, SD_STYLE_FAMILY_GRAPHICS, nMask));
    pSheet->SetParent(aStdName);
    pSheet->SetHelpId( aHelpFile, HID_POOLSHEET_TITLE );
    pISet = &pSheet->GetItemSet();

    pISet->Put(XLineStyleItem(drawing::LineStyle_NONE));
    pISet->Put(XFillStyleItem(drawing::FillStyle_NONE));

    pISet->Put(SvxFontHeightItem(1551, 100, EE_CHAR_FONTHEIGHT ));      // 44 pt

    // Title1
    aName = SD_RESSTR(STR_POOLSHEET_TITLE1);
    pSheet = &(pSSPool->Make(aName, SD_STYLE_FAMILY_GRAPHICS, nMask));
    pSheet->SetParent(aStdName);
    pSheet->SetHelpId( aHelpFile, HID_POOLSHEET_TITLE1 );
    pISet = &pSheet->GetItemSet();

    pISet->Put(XLineStyleItem(drawing::LineStyle_NONE));
    pISet->Put(XFillStyleItem(drawing::FillStyle_SOLID));
    pISet->Put(XFillColorItem(OUString(), RGB_Color(COL_CYAN)));

    pISet->Put(makeSdrShadowItem(true));
    pISet->Put(makeSdrShadowColorItem(RGB_Color(COL_GRAY)));
    pISet->Put(makeSdrShadowXDistItem(200));        // 2 mm shadow distance
    pISet->Put(makeSdrShadowYDistItem(200));

    pISet->Put(SvxFontHeightItem(846, 100, EE_CHAR_FONTHEIGHT ));       // 24 pt

    pISet->Put(SvxAdjustItem(SVX_ADJUST_CENTER, EE_PARA_JUST ));

    // Title2

    aName = SD_RESSTR(STR_POOLSHEET_TITLE2);
    pSheet = &(pSSPool->Make(aName, SD_STYLE_FAMILY_GRAPHICS, nMask));
    pSheet->SetParent(aStdName);
    pSheet->SetHelpId( aHelpFile, HID_POOLSHEET_TITLE2 );
    pISet = &pSheet->GetItemSet();

    pISet->Put(XLineWidthItem(50));

    // Don't get color from the color table, because the color might have been
    // deleted or changed there
    Color aOrange4(255, 204, 153);
    pISet->Put(XFillColorItem(OUString(), aOrange4));

    pISet->Put(makeSdrShadowItem(true));
    pISet->Put(makeSdrShadowColorItem(RGB_Color(COL_GRAY)));
    pISet->Put(makeSdrShadowXDistItem(200));        // 2 mm shadow distance
    pISet->Put(makeSdrShadowYDistItem(200));

    pISet->Put(SvxFontHeightItem(1270, 100, EE_CHAR_FONTHEIGHT ));      // 36 pt

    SvxLRSpaceItem aLRSpItem( 200, 200, 0, 0, EE_PARA_LRSPACE);
    pISet->Put( aLRSpItem );    // Indentation of first line: 0 mm; left and right: 2 mm

    pISet->Put(SvxULSpaceItem(100, 100, EE_PARA_ULSPACE ));      // Paragraph margin above/below: 1 mm

    pISet->Put(SvxAdjustItem(SVX_ADJUST_CENTER, EE_PARA_JUST ));

    // Headline

    aName = SD_RESSTR(STR_POOLSHEET_HEADLINE);
    pSheet = &(pSSPool->Make(aName, SD_STYLE_FAMILY_GRAPHICS, nMask));
    pSheet->SetParent(aStdName);
    pSheet->SetHelpId( aHelpFile, HID_POOLSHEET_HEADLINE );
    pISet = &pSheet->GetItemSet();

    pISet->Put(XLineStyleItem(drawing::LineStyle_NONE));
    pISet->Put(XFillStyleItem(drawing::FillStyle_NONE));

    pISet->Put(SvxFontHeightItem(846, 100, EE_CHAR_FONTHEIGHT ));        // 24 pt

    pISet->Put(SvxULSpaceItem(420, 210, EE_PARA_ULSPACE ));      // Paragraph margin above: 4,2 mm,
                                                // Paragraph margin below: 2,1 mm

    // Headline1
    aName = SD_RESSTR(STR_POOLSHEET_HEADLINE1);
    pSheet = &(pSSPool->Make(aName, SD_STYLE_FAMILY_GRAPHICS, nMask));
    pSheet->SetParent(aStdName);
    pSheet->SetHelpId( aHelpFile, HID_POOLSHEET_HEADLINE1 );
    pISet = &pSheet->GetItemSet();

    pISet->Put(XLineStyleItem(drawing::LineStyle_NONE));
    pISet->Put(XFillStyleItem(drawing::FillStyle_NONE));

    pISet->Put(SvxWeightItem(WEIGHT_BOLD, EE_CHAR_WEIGHT ));

    pISet->Put(SvxFontHeightItem(635, 100, EE_CHAR_FONTHEIGHT ));       // 18 pt

    pISet->Put(SvxULSpaceItem(420, 210, EE_PARA_ULSPACE ));      // Paragraph margin above: 4,2 mm,
                                                // Paragraph margin below: 2,1 mm

    // Headline2
    aName = SD_RESSTR(STR_POOLSHEET_HEADLINE2);
    pSheet = &(pSSPool->Make(aName, SD_STYLE_FAMILY_GRAPHICS, nMask));
    pSheet->SetParent(aStdName);
    pSheet->SetHelpId( aHelpFile, HID_POOLSHEET_HEADLINE2 );
    pISet = &pSheet->GetItemSet();

    pISet->Put(XLineStyleItem(drawing::LineStyle_NONE));
    pISet->Put(XFillStyleItem(drawing::FillStyle_NONE));

    pISet->Put(SvxPostureItem(ITALIC_NORMAL, EE_CHAR_ITALIC ));
    pISet->Put(SvxWeightItem(WEIGHT_BOLD, EE_CHAR_WEIGHT));

    pISet->Put(SvxFontHeightItem(494, 100, EE_CHAR_FONTHEIGHT ));        // 14 pt

    pISet->Put(SvxULSpaceItem(420, 210, EE_PARA_ULSPACE ));      // Paragraph margin above: 4,2 mm,
                                                // Paragraph margin below: 2,1 mm

    // Measurements
    aName = SD_RESSTR(STR_POOLSHEET_MEASURE);
    pSheet = &(pSSPool->Make(aName, SD_STYLE_FAMILY_GRAPHICS, nMask));
    pSheet->SetParent(aStdName);
    pSheet->SetHelpId( aHelpFile, HID_POOLSHEET_MEASURE );
    pISet = &pSheet->GetItemSet();

    pISet->Put(XFillStyleItem(drawing::FillStyle_NONE));
    pISet->Put(XLineColorItem(OUString(), RGB_Color(COL_BLACK)));

    pISet->Put(SvxFontHeightItem(423, 100, EE_CHAR_FONTHEIGHT ));         // 12 pt

    pISet->Put(XLineStartItem(SVX_RESSTR(RID_SVXSTR_ARROW),::basegfx::B2DPolyPolygon(aArrow)));
    pISet->Put(XLineStartWidthItem(200));
    pISet->Put(XLineEndItem(SVX_RESSTR(RID_SVXSTR_ARROW),::basegfx::B2DPolyPolygon(aArrow)));
    pISet->Put(XLineEndWidthItem(200));
    pISet->Put(XLineStyleItem(drawing::LineStyle_SOLID));
    pISet->Put(SdrYesNoItem(SDRATTR_MEASURESHOWUNIT, true));

    // Generate presentation templates for default layout.
    OUString aPrefix = SD_RESSTR(STR_LAYOUT_DEFAULT_NAME);
    pSSPool->CreateLayoutStyleSheets(aPrefix);
}

static Any implMakeSolidCellStyle( SdStyleSheetPool* pSSPool, const OUString& rName, const OUString& rParent, const Color& rColor )
{
    SfxStyleSheetBase* pSheet = &(pSSPool->Make(rName, SD_STYLE_FAMILY_CELL, SFXSTYLEBIT_AUTO));
    pSheet->SetParent(rParent);
    SfxItemSet* pISet = &pSheet->GetItemSet();
    pISet->Put(XFillStyleItem(drawing::FillStyle_SOLID));
    pISet->Put(XFillColorItem(OUString(), rColor));

    return Any( Reference< XStyle >( static_cast< XWeak* >( pSheet ), UNO_QUERY ) );
}

static void implCreateTableTemplate( const Reference< XNameContainer >& xTableFamily, const OUString& rName, const Any& rBody, const Any& rHeading, const Any& rBanding )
{
    if( xTableFamily.is() ) try
    {
        if( !xTableFamily->hasByName( rName ) )
        {
            Reference< XSingleServiceFactory > xFactory( xTableFamily, UNO_QUERY_THROW );
            Reference< XNameReplace > xDefaultTableStyle( xFactory->createInstance(), UNO_QUERY_THROW );
            xTableFamily->insertByName( rName, Any( xDefaultTableStyle ) );

            xDefaultTableStyle->replaceByName( "body", rBody  );
            xDefaultTableStyle->replaceByName( "odd-rows" , rBanding );
            xDefaultTableStyle->replaceByName( "odd-columns" , rBanding );
            xDefaultTableStyle->replaceByName( "first-row" , rHeading );
            xDefaultTableStyle->replaceByName( "first-column" , rHeading );
            xDefaultTableStyle->replaceByName( "last-row" , rHeading );
            xDefaultTableStyle->replaceByName( "last-column" , rHeading );
        }
    }
    catch( Exception& )
    {
        OSL_FAIL("sd::implCreateTableTemplate(), exception caught!");
    }
}

void SdDrawDocument::CreateDefaultCellStyles()
{
    SdStyleSheetPool*       pSSPool = static_cast< SdStyleSheetPool* >(GetStyleSheetPool());
    SfxStyleSheetBase*      pSheet = nullptr;
    OUString                aHelpFile;

    Reference< XNameContainer > xTableFamily( pSSPool->getByName( "table" ), UNO_QUERY );

    // ---- Default -----------------------------------------------

    sal_uInt16 nMask = SFXSTYLEBIT_AUTO;

    OUString aDefaultCellStyleName( "default" );

    pSheet = &(pSSPool->Make(aDefaultCellStyleName, SD_STYLE_FAMILY_CELL, nMask));
    pSheet->SetHelpId( aHelpFile, HID_SD_CELL_STYLE_DEFAULT );
    SfxItemSet& rISet = pSheet->GetItemSet();

    Color    aNullCol(RGB_Color(COL_BLACK));

    XGradient aNullGrad(aNullCol,RGB_Color(COL_WHITE));
              aNullGrad.SetStartIntens( 100 );
              aNullGrad.SetEndIntens( 100 );
    XHatch    aNullHatch(aNullCol);

    rISet.Put(XFillStyleItem(drawing::FillStyle_SOLID));
    rISet.Put(XFillColorItem(OUString(), RGB_Color(0x00ccccff)));

    vcl::Font aLatinFont, aCJKFont, aCTLFont;

    getDefaultFonts( aLatinFont, aCJKFont, aCTLFont );

    SvxFontItem aSvxFontItem( aLatinFont.GetFamily(), aLatinFont.GetFamilyName(), aLatinFont.GetStyleName(), aLatinFont.GetPitch(),
                              aLatinFont.GetCharSet(), EE_CHAR_FONTINFO );

    SvxFontItem aSvxFontItemCJK( aCJKFont.GetFamily(), aCJKFont.GetFamilyName(), aCJKFont.GetStyleName(), aCJKFont.GetPitch(),
                                 aCJKFont.GetCharSet(), EE_CHAR_FONTINFO_CJK );

    SvxFontItem aSvxFontItemCTL( aCTLFont.GetFamily(), aCTLFont.GetFamilyName(), aCTLFont.GetStyleName(), aCTLFont.GetPitch(),
                                 aCTLFont.GetCharSet(), EE_CHAR_FONTINFO_CTL );

    rISet.Put( aSvxFontItem );
    rISet.Put( aSvxFontItemCJK );
    rISet.Put( aSvxFontItemCTL );

    rISet.Put( SvxFontHeightItem( 635, 100, EE_CHAR_FONTHEIGHT ) );     // sj: (i33745) changed default from 24 to 18 pt
    rISet.Put( SvxFontHeightItem( 635, 100, EE_CHAR_FONTHEIGHT_CJK ) ); // 18 pt
    rISet.Put( SvxFontHeightItem( convertFontHeightToCTL( 635 ), 100, EE_CHAR_FONTHEIGHT_CTL ) ); // 18 pt

    rISet.Put(SvxColorItem(Color(COL_AUTO), EE_CHAR_COLOR ));

    // Paragraph attributes (Edit Engine)
    rISet.Put(SvxLRSpaceItem(EE_PARA_LRSPACE));
    rISet.Put(SvxULSpaceItem(EE_PARA_ULSPACE));

    rISet.Put( makeSdrTextLeftDistItem( 250 ) );
    rISet.Put( makeSdrTextRightDistItem( 250 ) );
    rISet.Put( makeSdrTextUpperDistItem( 130 ) );
    rISet.Put( makeSdrTextLowerDistItem( 130 ) );

    rISet.Put( SvxLineSpacingItem( LINE_SPACE_DEFAULT_HEIGHT, EE_PARA_SBL ) );
    rISet.Put( SvxAutoKernItem( true, EE_CHAR_PAIRKERNING ) );
    rISet.Put( SdrTextVertAdjustItem(SDRTEXTVERTADJUST_TOP) );
    rISet.Put( SdrTextHorzAdjustItem(SDRTEXTHORZADJUST_LEFT) );

    Color aWhite( COL_WHITE );
    ::editeng::SvxBorderLine aBorderLine(
            &aWhite, 1, table::BorderLineStyle::SOLID);

    SvxBoxItem aBoxItem( SDRATTR_TABLE_BORDER );
    aBoxItem.SetLine( &aBorderLine, SvxBoxItemLine::TOP );
    aBoxItem.SetLine( &aBorderLine, SvxBoxItemLine::BOTTOM );
    aBoxItem.SetLine( &aBorderLine, SvxBoxItemLine::LEFT );
    aBoxItem.SetLine( &aBorderLine, SvxBoxItemLine::RIGHT );

    rISet.Put( aBoxItem );

    // ---- default --------------------------------------------------

    Any aGray1( implMakeSolidCellStyle( pSSPool, "gray1" , aDefaultCellStyleName, RGB_COLORDATA(230,230,230)));
    Any aGray2( implMakeSolidCellStyle( pSSPool, "gray2" , aDefaultCellStyleName, RGB_COLORDATA(204,204,204)));
    Any aGray3( implMakeSolidCellStyle( pSSPool, "gray3" , aDefaultCellStyleName, RGB_COLORDATA(179,179,179)));

    implCreateTableTemplate( xTableFamily, "default" , aGray1, aGray3, aGray2 );

    // ---- BW ------------------------------------------------

    Any aBW1( implMakeSolidCellStyle( pSSPool, "bw1" , aDefaultCellStyleName, RGB_COLORDATA(255,255,255)));
    Any aBW2( implMakeSolidCellStyle( pSSPool, "bw2" , aDefaultCellStyleName, RGB_COLORDATA(230,230,230)));
    Any aBW3( implMakeSolidCellStyle( pSSPool, "bw3" , aDefaultCellStyleName, RGB_COLORDATA(0,0,0)));

    implCreateTableTemplate( xTableFamily, "bw" , aBW1, aBW3, aBW2 );

    // ---- Orange --------------------------------------------------

    Any aOrange1( implMakeSolidCellStyle( pSSPool, "orange1" , aDefaultCellStyleName, RGB_COLORDATA(255,204,153)));
    Any aOrange2( implMakeSolidCellStyle( pSSPool, "orange2" , aDefaultCellStyleName, RGB_COLORDATA(255,153,102)));
    Any aOrange3( implMakeSolidCellStyle( pSSPool, "orange3" , aDefaultCellStyleName, RGB_COLORDATA(255,102,51)));

    implCreateTableTemplate( xTableFamily, "orange" , aOrange1, aOrange3, aOrange2 );

    // ---- Turquoise --------------------------------------------------

    Any aTurquoise1( implMakeSolidCellStyle( pSSPool, "turquoise1" , aDefaultCellStyleName, RGB_COLORDATA(71,184,184)));
    Any aTurquoise2( implMakeSolidCellStyle( pSSPool, "turquoise2" , aDefaultCellStyleName, RGB_COLORDATA(51,163,163)));
    Any aTurquoise3( implMakeSolidCellStyle( pSSPool, "turquoise3" , aDefaultCellStyleName, RGB_COLORDATA(25,138,138)));

    implCreateTableTemplate( xTableFamily, "turquoise" , aTurquoise1, aTurquoise3, aTurquoise2 );

    // ---- Gray ------------------------------------------------

    Any aBlue1( implMakeSolidCellStyle( pSSPool, "blue1" , aDefaultCellStyleName, RGB_COLORDATA(153,204,255)));
    Any aBlue2( implMakeSolidCellStyle( pSSPool, "blue2" , aDefaultCellStyleName, RGB_COLORDATA(0,153,255)));
    Any aBlue3( implMakeSolidCellStyle( pSSPool, "blue3" , aDefaultCellStyleName, RGB_COLORDATA(0,102,204)));

    implCreateTableTemplate( xTableFamily, "blue" , aBlue1, aBlue3, aBlue2 );

    // ---- Sun ------------------------------------------------

    Any aSun1( implMakeSolidCellStyle( pSSPool, "sun1" , aDefaultCellStyleName, RGB_COLORDATA(230,230,255)));
    Any aSun2( implMakeSolidCellStyle( pSSPool, "sun2" , aDefaultCellStyleName, RGB_COLORDATA(204,204,255)));
    Any aSun3( implMakeSolidCellStyle( pSSPool, "sun3" , aDefaultCellStyleName, RGB_COLORDATA(153,153,255)));

    implCreateTableTemplate( xTableFamily, "sun" , aSun1, aSun3, aSun2 );

    // ---- Earth ----------------------------------------------

    Any aEarth1( implMakeSolidCellStyle( pSSPool, "earth1" , aDefaultCellStyleName, RGB_COLORDATA(255,255,204)));
    Any aEarth2( implMakeSolidCellStyle( pSSPool, "earth2" , aDefaultCellStyleName, RGB_COLORDATA(255,204,153)));
    Any aEarth3( implMakeSolidCellStyle( pSSPool, "earth3" , aDefaultCellStyleName, RGB_COLORDATA(204,102,51)));

    implCreateTableTemplate( xTableFamily, "earth" , aEarth1, aEarth3, aEarth2 );

    // ---- Green ----------------------------------------------

    Any aGreen1( implMakeSolidCellStyle( pSSPool, "green1" , aDefaultCellStyleName, RGB_COLORDATA(255,255,204)));
    Any aGreen2( implMakeSolidCellStyle( pSSPool, "green2" , aDefaultCellStyleName, RGB_COLORDATA(148,189,94)));
    Any aGreen3( implMakeSolidCellStyle( pSSPool, "green3" , aDefaultCellStyleName, RGB_COLORDATA(92,133,38)));

    implCreateTableTemplate( xTableFamily, "green" , aGreen1, aGreen3, aGreen2 );

    // ---- Seaweed ----------------------------------------------

    Any aSeetang1( implMakeSolidCellStyle( pSSPool, "seetang1" , aDefaultCellStyleName, RGB_COLORDATA(204,255,255)));
    Any aSeetang2( implMakeSolidCellStyle( pSSPool, "seetang2" , aDefaultCellStyleName, RGB_COLORDATA(71,184,184)));
    Any aSeetang3( implMakeSolidCellStyle( pSSPool, "seetang3" , aDefaultCellStyleName, RGB_COLORDATA(51,163,163)));

    implCreateTableTemplate( xTableFamily, "seetang" , aSeetang1, aSeetang3, aSeetang2 );

    // ---- LightBlue ----------------------------------------------

    Any aLightBlue1( implMakeSolidCellStyle( pSSPool, "lightblue1" , aDefaultCellStyleName, RGB_COLORDATA(255,255,255)));
    Any aLightBlue2( implMakeSolidCellStyle( pSSPool, "lightblue2" , aDefaultCellStyleName, RGB_COLORDATA(230,230,255)));
    Any aLightBlue3( implMakeSolidCellStyle( pSSPool, "lightblue3" , aDefaultCellStyleName, RGB_COLORDATA(153,153,204)));

    implCreateTableTemplate( xTableFamily, "lightblue" , aLightBlue1, aLightBlue3, aLightBlue2 );

    // ---- Yellow ----------------------------------------------

    Any aYellow1( implMakeSolidCellStyle( pSSPool, "yellow1" , aDefaultCellStyleName, RGB_COLORDATA(255,255,204)));
    Any aYellow2( implMakeSolidCellStyle( pSSPool, "yellow2" , aDefaultCellStyleName, RGB_COLORDATA(255,255,153)));
    Any aYellow3( implMakeSolidCellStyle( pSSPool, "yellow3" , aDefaultCellStyleName, RGB_COLORDATA(255,204,153)));

    implCreateTableTemplate( xTableFamily, "yellow" , aYellow1, aYellow3, aYellow2 );
}

// Number of pages that reference a master page
sal_uInt16 SdDrawDocument::GetMasterPageUserCount(SdrPage* pMaster) const
{
    sal_uInt16 nResult = 0;
    sal_uInt16 nPage;
    sal_uInt16 nPageCount = GetPageCount();

    for (nPage = 0; nPage < nPageCount; nPage++)
    {
        const SdrPage* pPage = GetPage(nPage);

        if(pPage->TRG_HasMasterPage())
        {
            if(&(pPage->TRG_GetMasterPage()) == pMaster)
            {
                nResult++;
            }
        }
    }
    return nResult;
}

// Finish OnlineSpelling in the background

void SdDrawDocument::StopOnlineSpelling()
{
    if (mpOnlineSpellingIdle && mpOnlineSpellingIdle->IsActive())
    {
        mpOnlineSpellingIdle->Stop();
    }

    delete mpOnlineSpellingIdle;
    mpOnlineSpellingIdle = nullptr;

    delete mpOnlineSpellingList;
    mpOnlineSpellingList = nullptr;
}

// Start OnlineSpelling in the background
void SdDrawDocument::StartOnlineSpelling(bool bForceSpelling)
{
    if (mbOnlineSpell && (bForceSpelling || mbInitialOnlineSpellingEnabled) &&
        mpDocSh && !mpDocSh->IsReadOnly() )
    {
        StopOnlineSpelling();

        ::sd::Outliner* pOutl = GetInternalOutliner();

        Reference< XSpellChecker1 > xSpellChecker( LinguMgr::GetSpellChecker() );
        if ( xSpellChecker.is() )
            pOutl->SetSpeller( xSpellChecker );

        Reference< XHyphenator > xHyphenator( LinguMgr::GetHyphenator() );
        if( xHyphenator.is() )
            pOutl->SetHyphenator( xHyphenator );

        pOutl->SetDefaultLanguage( meLanguage );

        mpOnlineSpellingList = new ShapeList;
        sal_uInt16 nPage;

        for ( nPage = 0; nPage < GetPageCount(); nPage++ )
        {
            // Search in all pages
            FillOnlineSpellingList(static_cast<SdPage*>(GetPage(nPage)));
        }

        for (nPage = 0; nPage < GetMasterPageCount(); nPage++)
        {
            // Search all master pages
            FillOnlineSpellingList(static_cast<SdPage*>( GetMasterPage(nPage) ));
        }

        mpOnlineSpellingList->seekShape(0);
        mpOnlineSpellingIdle = new Idle();
        mpOnlineSpellingIdle->SetIdleHdl( LINK(this, SdDrawDocument, OnlineSpellingHdl) );
        mpOnlineSpellingIdle->SetPriority(SchedulerPriority::LOWEST);
        mpOnlineSpellingIdle->Start();
    }
}

// Fill OnlineSpelling list
void SdDrawDocument::FillOnlineSpellingList(SdPage* pPage)
{
    SdrObjListIter aIter(*pPage, IM_FLAT);

    while (aIter.IsMore())
    {
        SdrObject* pObj = aIter.Next();

        if( !pObj )
            continue;

        if (pObj->GetOutlinerParaObject())
        {
            // Found a text object
            mpOnlineSpellingList->addShape(*pObj);
        }
        else if (pObj->GetObjIdentifier() == OBJ_GRUP)
        {
            // Found a group object
            SdrObjListIter aGroupIter(*static_cast<SdrObjGroup*>(pObj)->GetSubList(),
                                      IM_DEEPNOGROUPS);

            bool bSubTextObjFound = false;

            while (aGroupIter.IsMore() && !bSubTextObjFound)
            {
                if (aGroupIter.Next()->GetOutlinerParaObject())
                {
                    // Found a text object in a group object
                    bSubTextObjFound = true;
                }
            }

            if (bSubTextObjFound)
            {
                mpOnlineSpellingList->addShape(*pObj);
            }
        }
    }
}

// OnlineSpelling in the background
IMPL_LINK_NOARG_TYPED(SdDrawDocument, OnlineSpellingHdl, Idle *, void)
{
    if (mpOnlineSpellingList!=nullptr
        && ( !mbOnlineSpell || mpOnlineSpellingList->hasMore()))
    {
        // Spell next object
        SdrObject* pObj = mpOnlineSpellingList->getNextShape();

        if (pObj)
        {
            if (pObj->GetOutlinerParaObject() && dynamic_cast< const SdrTextObj *>( pObj ) !=  nullptr)
            {
                // Spell text object
                SpellObject(static_cast<SdrTextObj*>(pObj));
            }
            else if (pObj->GetObjIdentifier() == OBJ_GRUP)
            {
                // Found a group object
                SdrObjListIter aGroupIter(*static_cast<SdrObjGroup*>(pObj)->GetSubList(),
                                          IM_DEEPNOGROUPS);


                while (aGroupIter.IsMore())
                {
                    SdrObject* pSubObj = aGroupIter.Next();

                    if (pSubObj->GetOutlinerParaObject() && dynamic_cast< SdrTextObj *>( pSubObj ) !=  nullptr)
                    {
                        // Found a text object in a group object
                        SpellObject(static_cast<SdrTextObj*>(pSubObj));
                    }
                }
            }
        }

        // Continue search
        mpOnlineSpellingIdle->Start();
    }
    else
    {
        // Initial spelling has finished
        mbInitialOnlineSpellingEnabled = false;

        // Stop search
        StopOnlineSpelling();

        delete mpOnlineSearchItem;
        mpOnlineSearchItem = nullptr;
    }
}

// Spell object (for OnlineSpelling)
void SdDrawDocument::SpellObject(SdrTextObj* pObj)
{
    if (pObj && pObj->GetOutlinerParaObject() /* && pObj != pView->GetTextEditObject() */)
    {
        mbHasOnlineSpellErrors = false;
        ::sd::Outliner* pOutl = GetInternalOutliner();
        pOutl->SetUpdateMode(true);
        Link<EditStatus&,void> aEvtHdl = pOutl->GetStatusEventHdl();
        pOutl->SetStatusEventHdl(LINK(this, SdDrawDocument, OnlineSpellEventHdl));

        sal_uInt16 nOldOutlMode = pOutl->GetMode();
        sal_uInt16 nOutlMode = OUTLINERMODE_TEXTOBJECT;
        if (pObj->GetObjInventor() == SdrInventor &&
            pObj->GetObjIdentifier() == OBJ_OUTLINETEXT)
        {
            nOutlMode = OUTLINERMODE_OUTLINEOBJECT;
        }
        pOutl->Init( nOutlMode );

        // Put text into the outliner
        pOutl->SetText(*pObj->GetOutlinerParaObject());

        if (!mpOnlineSearchItem || pOutl->HasText(*mpOnlineSearchItem))
        {
            // Spelling
            pOutl->CompleteOnlineSpelling();

            if (mbHasOnlineSpellErrors)
            {
                sd::ModifyGuard aGuard( this );
                SdrModel* pModel = pObj->GetModel();
                bool bLock = false;
                if ( pModel )
                {
                    bLock = pModel->isLocked();
                    pModel->setLock(true);
                }
                // taking text from the outliner
                pObj->SetOutlinerParaObject( pOutl->CreateParaObject() );

                pObj->BroadcastObjectChange();
                if ( pModel )
                    pModel->setLock(bLock);
            }
        }

        pOutl->SetStatusEventHdl(aEvtHdl);
        pOutl->SetUpdateMode(false);
        pOutl->Init( nOldOutlMode );
        mbHasOnlineSpellErrors = false;
    }
}

// Object was inserted into model
void SdDrawDocument::InsertObject(SdrObject* pObj, SdPage* /*pPage*/)
{
    if(mpOnlineSpellingList && pObj)
    {
        if (pObj->GetOutlinerParaObject() || (pObj->GetObjIdentifier() == OBJ_GRUP))
        {
            // Add object to OnlineSpelling list
            mpOnlineSpellingList->addShape(*pObj);
        }
    }
}

// Object removed from model
void SdDrawDocument::RemoveObject(SdrObject* pObj, SdPage* /*pPage*/)
{
    if(mpOnlineSpellingList && pObj)
    {
        if (pObj->GetOutlinerParaObject() || (pObj->GetObjIdentifier() == OBJ_GRUP))
        {
            // Replace object in OnlineSpelling list by 0 pointer
            mpOnlineSpellingList->removeShape(*pObj);
        }
    }
}

// Callback for ExecuteSpellPopup()
IMPL_LINK_TYPED(SdDrawDocument, OnlineSpellEventHdl, EditStatus&, rEditStat, void)
{
    EditStatusFlags nStat = rEditStat.GetStatusWord();
    mbHasOnlineSpellErrors = bool(nStat & EditStatusFlags::WRONGWORDCHANGED);
}

// Callback for ExecuteSpellPopup()

// removed link and replaced with Imp method
void SdDrawDocument::ImpOnlineSpellCallback(SpellCallbackInfo* pInfo, SdrObject* pObj, SdrOutliner* pOutl)
{
    delete mpOnlineSearchItem;
    mpOnlineSearchItem = nullptr;

    SpellCallbackCommand nCommand = pInfo->nCommand;

    if (nCommand == SpellCallbackCommand::IGNOREWORD
        // restart when add to dictionary takes place, too.
        || nCommand == SpellCallbackCommand::ADDTODICTIONARY)
    {
        if(pObj && pOutl && dynamic_cast< const SdrTextObj *>( pObj ) !=  nullptr)
        {
            bool bModified(IsChanged());
            static_cast<SdrTextObj*>(pObj)->SetOutlinerParaObject(pOutl->CreateParaObject());
            SetChanged(bModified);
            pObj->BroadcastObjectChange();
        }

        mpOnlineSearchItem = new SvxSearchItem( SID_SEARCH_ITEM );
        mpOnlineSearchItem->SetSearchString(pInfo->aWord);
        StartOnlineSpelling();
    }
    else if (nCommand == SpellCallbackCommand::STARTSPELLDLG)
        SfxViewFrame::Current()->GetDispatcher()->Execute( SID_SPELL_DIALOG, SfxCallMode::ASYNCHRON );
    else if (nCommand == SpellCallbackCommand::AUTOCORRECT_OPTIONS)
        SfxViewFrame::Current()->GetDispatcher()->Execute( SID_AUTO_CORRECT_DLG, SfxCallMode::ASYNCHRON );
}

// Replace the unambiguous names of the default layers by their names in the
// native language
void SdDrawDocument::RestoreLayerNames()
{
    SdrLayerAdmin& rLayerAdmin = GetLayerAdmin();
    sal_uInt16 nLayerCount = rLayerAdmin.GetLayerCount();

    for (sal_uInt16 nLayer = 0; nLayer < nLayerCount; nLayer++)
    {
        SdrLayer* pLayer = rLayerAdmin.GetLayer(nLayer);

        if (pLayer)
        {
            OUString aLayerName(pLayer->GetName());

            if (aLayerName == "LAYER_LAYOUT")
            {
                pLayer->SetName(SD_RESSTR(STR_LAYER_LAYOUT));
            }
            else if (aLayerName == "LAYER_BCKGRND")
            {
                pLayer->SetName(SD_RESSTR(STR_LAYER_BCKGRND));
            }
            else if (aLayerName == "LAYER_BACKGRNDOBJ")
            {
                pLayer->SetName(SD_RESSTR(STR_LAYER_BCKGRNDOBJ));
            }
            else if (aLayerName == "LAYER_CONTROLS")
            {
                pLayer->SetName(SD_RESSTR(STR_LAYER_CONTROLS));
            }
            else if (aLayerName == "LAYER_MEASURELINES")
            {
                pLayer->SetName(SD_RESSTR(STR_LAYER_MEASURELINES));
            }
        }
    }
}

// Return formatted page number (1, I, i, a, etc.)
OUString SdDrawDocument::CreatePageNumValue(sal_uInt16 nNum) const
{
    OUString aPageNumValue;
    bool bUpper = false;

    switch (mePageNumType)
    {
        case SVX_CHARS_UPPER_LETTER:
            aPageNumValue += OUString( (sal_Unicode)(char)((nNum - 1) % 26 + 'A') );
            break;
        case SVX_CHARS_LOWER_LETTER:
            aPageNumValue += OUString( (sal_Unicode)(char)((nNum - 1) % 26 + 'a') );
            break;
        case SVX_ROMAN_UPPER:
            bUpper = true;
            SAL_FALLTHROUGH;
        case SVX_ROMAN_LOWER:
            aPageNumValue += SvxNumberFormat::CreateRomanString(nNum, bUpper);
            break;
        case SVX_NUMBER_NONE:
            aPageNumValue = " ";
            break;
        default:
            aPageNumValue += OUString::number(nNum);
    }

    return aPageNumValue;
}

// Rename layout template
// Keep in mind that rOldLayoutName contains the _complete_ name of the layout
// (including ~LT~). This is unlike rNewName.
void SdDrawDocument::RenameLayoutTemplate(const OUString& rOldLayoutName, const OUString& rNewName)
{
    OUString aSep(SD_LT_SEPARATOR);
    OUString aOldName(rOldLayoutName);
    sal_Int32 nPos = aOldName.indexOf( aSep );

    // erase everything after '~LT~'
    if (nPos != -1)
        aOldName = aOldName.copy(0, nPos + aSep.getLength());

    std::vector<StyleReplaceData> aReplList;
    SfxStyleSheetIterator aIter(mxStyleSheetPool.get(), SD_STYLE_FAMILY_MASTERPAGE);
    SfxStyleSheetBase* pSheet = aIter.First();

    while (pSheet)
    {
        OUString aSheetName = pSheet->GetName();

        // if the sheetname starts with aOldName + "~LT~"
        if (aSheetName.startsWith(aOldName))
        {
            aSheetName = aSheetName.replaceAt(0, aOldName.getLength() - aSep.getLength(), rNewName);

            StyleReplaceData aReplData;
            aReplData.nFamily     = pSheet->GetFamily();
            aReplData.nNewFamily = pSheet->GetFamily();
            aReplData.aName   = pSheet->GetName();
            aReplData.aNewName   = aSheetName;
            aReplList.push_back(aReplData);

            pSheet->SetName(aSheetName);
        }

        pSheet = aIter.Next();
    }

    // Now set the layout name of the drawing and the notes page, as well as
    // their master pages.
    OUString aPageLayoutName(rNewName);
    aPageLayoutName += aSep + SD_RESSTR(STR_LAYOUT_OUTLINE);

    // Inform all text objects on pages that use the renamed layout and set the
    // new name.
    sal_uInt16 nPage;
    for (nPage = 0; nPage < GetPageCount(); nPage++)
    {
        SdPage* pPage = static_cast<SdPage*>(GetPage(nPage));
        OUString aTemp(pPage->GetLayoutName());

        if (aTemp == rOldLayoutName)
        {
            pPage->SetLayoutName(aPageLayoutName);

            for (size_t nObj = 0; nObj < pPage->GetObjCount(); ++nObj)
            {
                SdrObject* pObj = pPage->GetObj(nObj);

                if (pObj->GetObjInventor() == SdrInventor)
                {
                    switch( pObj->GetObjIdentifier() )
                    {
                        case OBJ_TEXT:
                        case OBJ_OUTLINETEXT:
                        case OBJ_TITLETEXT:
                        {
                            OutlinerParaObject* pOPO = static_cast<SdrTextObj*>(pObj)->GetOutlinerParaObject();

                            if (pOPO)
                            {
                                std::vector<StyleReplaceData>::iterator it;
                                for (it = aReplList.begin(); it != aReplList.end(); ++it)
                                    pOPO->ChangeStyleSheets( it->aName, it->nFamily, it->aNewName, it->nNewFamily );
                            }
                        }
                        break;

                        default:
                        break;
                    }
                }
            }
        }
    }

    // Now do this again for all master pages.
    // The affected master pages get the name of the layout as their page name.
    for (nPage = 0; nPage < GetMasterPageCount(); nPage++)
    {
        SdPage* pPage = static_cast<SdPage*>( GetMasterPage(nPage) );
        OUString aTemp(pPage->GetLayoutName());

        if (aTemp == rOldLayoutName)
        {
            pPage->SetLayoutName(aPageLayoutName);
            pPage->SetName(rNewName);

            for (size_t nObj = 0; nObj < pPage->GetObjCount(); ++nObj)
            {
                SdrObject* pObj = pPage->GetObj(nObj);

                if (pObj->GetObjInventor() == SdrInventor)
                {
                    switch(pObj->GetObjIdentifier())
                    {
                        case OBJ_TEXT:
                        case OBJ_OUTLINETEXT:
                        case OBJ_TITLETEXT:
                        {
                            OutlinerParaObject* pOPO = static_cast<SdrTextObj*>(pObj)->GetOutlinerParaObject();

                            if (pOPO)
                            {
                                std::vector<StyleReplaceData>::iterator it;
                                for (it = aReplList.begin(); it != aReplList.end(); ++it)
                                    pOPO->ChangeStyleSheets( it->aName, it->nFamily, it->aNewName, it->nNewFamily );
                            }
                        }
                        break;

                        default:
                        break;
                    }
                }
            }
        }
    }
}

// Set outliner defaults (pool defaults)
void SdDrawDocument::SetTextDefaults() const
{
    // BulletItem and BulletFont for Titel and Outline
    SvxBulletItem aBulletItem(EE_PARA_BULLET);
    vcl::Font aBulletFont( SdStyleSheetPool::GetBulletFont() );
    aBulletFont.SetSize(Size(0,846));       // 24 pt
    aBulletItem.SetFont(aBulletFont);
    aBulletItem.SetStyle(SvxBulletStyle::BULLET);
    aBulletItem.SetStart(1);
    aBulletItem.SetScale(45);               // In percent
    aBulletItem.SetSymbol( 0x25CF );                // In points
    pItemPool->SetPoolDefaultItem( aBulletItem );

    // New BulletItem
    SvxNumberFormat aNumberFormat(SVX_NUM_CHAR_SPECIAL);
    aNumberFormat.SetBulletFont(&aBulletFont);
    aNumberFormat.SetBulletChar( 0x25CF );  // StarBats: 0xF000 + 34
    aNumberFormat.SetBulletRelSize(45);
    aNumberFormat.SetBulletColor(Color(COL_AUTO));
    aNumberFormat.SetStart(1);
    aNumberFormat.SetNumAdjust(SVX_ADJUST_LEFT);

    SvxNumRule aNumRule( SvxNumRuleFlags::BULLET_REL_SIZE | SvxNumRuleFlags::BULLET_COLOR | SvxNumRuleFlags::CHAR_TEXT_DISTANCE, SVX_MAX_NUM, false);

    //aNumberFormat.SetAbsLSpace( 0 );
    //aNumberFormat.SetFirstLineOffset( 0 );
    //aNumRule.SetLevel( 0, aNumberFormat );

    for( sal_uInt16 i = 0; i < aNumRule.GetLevelCount(); i++ )
    {
        const short nLSpace = (i + 1) * 600;
        aNumberFormat.SetAbsLSpace(nLSpace);
        aNumberFormat.SetFirstLineOffset(-600);
        aNumRule.SetLevel( i, aNumberFormat );
    }

    SvxNumBulletItem aNumBulletItem( aNumRule, EE_PARA_NUMBULLET );
    pItemPool->SetPoolDefaultItem( aNumBulletItem );
}

css::text::WritingMode SdDrawDocument::GetDefaultWritingMode() const
{
    const SfxPoolItem*                  pItem = ( pItemPool ? pItemPool->GetPoolDefaultItem( EE_PARA_WRITINGDIR ) : nullptr );
    css::text::WritingMode eRet = css::text::WritingMode_LR_TB;

    if( pItem )
    {
        switch( static_cast<const SvxFrameDirectionItem&>( *pItem ).GetValue() )
        {
            case( FRMDIR_HORI_LEFT_TOP ): eRet = css::text::WritingMode_LR_TB; break;
            case( FRMDIR_HORI_RIGHT_TOP ): eRet = css::text::WritingMode_RL_TB; break;
            case( FRMDIR_VERT_TOP_RIGHT ): eRet = css::text::WritingMode_TB_RL; break;

            default:
                OSL_FAIL( "Frame direction not supported yet" );
            break;
        }
    }

    return eRet;
}

void SdDrawDocument::SetDefaultWritingMode(css::text::WritingMode eMode )
{
    if( pItemPool )
    {
        SvxFrameDirection nVal;
        switch( eMode )
        {
        case css::text::WritingMode_LR_TB: nVal = FRMDIR_HORI_LEFT_TOP; break;
        case css::text::WritingMode_RL_TB: nVal = FRMDIR_HORI_RIGHT_TOP; break;
        case css::text::WritingMode_TB_RL: nVal = FRMDIR_VERT_TOP_RIGHT; break;
        default:
            OSL_FAIL( "Frame direction not supported yet" );
            return;
        }

        SvxFrameDirectionItem aModeItem( nVal, EE_PARA_WRITINGDIR );
        pItemPool->SetPoolDefaultItem( aModeItem );

        SvxAdjustItem aAdjust( SVX_ADJUST_LEFT, EE_PARA_JUST );

        if( eMode == css::text::WritingMode_RL_TB )
            aAdjust.SetEnumValue( SVX_ADJUST_RIGHT );

        pItemPool->SetPoolDefaultItem( aAdjust );

    }
}

void SdDrawDocument::getDefaultFonts( vcl::Font& rLatinFont, vcl::Font& rCJKFont, vcl::Font& rCTLFont )
{
    LanguageType eLatin = GetLanguage( EE_CHAR_LANGUAGE );

    //  If the UI language is Korean, the default Latin font has to
    //  be queried for Korean, too (the Latin language from the document can't be Korean).
    //  This is the same logic as in SwDocShell::InitNew.
    LanguageType eUiLanguage = Application::GetSettings().GetUILanguageTag().getLanguageType();
    if (MsLangId::isKorean(eUiLanguage))
        eLatin = eUiLanguage;

    rLatinFont = OutputDevice::GetDefaultFont( DefaultFontType::LATIN_PRESENTATION, eLatin, GetDefaultFontFlags::OnlyOne );
    rCJKFont = OutputDevice::GetDefaultFont( DefaultFontType::CJK_PRESENTATION, GetLanguage( EE_CHAR_LANGUAGE_CJK ), GetDefaultFontFlags::OnlyOne );
    rCTLFont = OutputDevice::GetDefaultFont( DefaultFontType::CTL_PRESENTATION, GetLanguage( EE_CHAR_LANGUAGE_CTL ), GetDefaultFontFlags::OnlyOne ) ;
}

/* converts the given western font height to a corresponding ctl font height, depending on the system language */
sal_uInt32 SdDrawDocument::convertFontHeightToCTL( sal_uInt32 nWesternFontHeight )
{
    LanguageType eRealCTLLanguage = Application::GetSettings().GetLanguageTag().getLanguageType();
    if( LANGUAGE_THAI == eRealCTLLanguage )
    {
        // http://specs.openoffice.org/g11n/font_sizes/42775_42725_Individual_configurable_font_size_for_default_fonts.odt
        double fTemp = double(nWesternFontHeight) * 1.333;
        nWesternFontHeight = (sal_uInt32)fTemp;
        // make some nice values for UI that displays PT instead of 1/100th mm
        nWesternFontHeight = ((nWesternFontHeight * 72) + 1270) / 2540L;
        nWesternFontHeight = ((nWesternFontHeight * 2540L) + 36) / 72;
    }
    return nWesternFontHeight;
}

SdStyleSheetPool* SdDrawDocument::GetSdStyleSheetPool() const
{
    return dynamic_cast< SdStyleSheetPool* >( GetStyleSheetPool() );
}

ModifyGuard::ModifyGuard( SdDrawDocument* pDoc )
: mpDocShell( nullptr ), mpDoc( pDoc )
{
    init();
}

void ModifyGuard::init()
{
    if( mpDocShell )
    {
        mpDoc = mpDocShell->GetDoc();
    }
    else if( mpDoc )
    {
        mpDocShell = mpDoc->GetDocSh();
    }

    mbIsEnableSetModified = mpDocShell && mpDocShell->IsEnableSetModified();
    mbIsDocumentChanged = mpDoc && mpDoc->IsChanged();

    if( mbIsEnableSetModified )
        mpDocShell->EnableSetModified( false );
}

ModifyGuard::~ModifyGuard()
{
    if( mbIsEnableSetModified )
        mpDocShell->EnableSetModified();

    if( mpDoc && (mpDoc->IsChanged() != mbIsDocumentChanged) )
        mpDoc->SetChanged(mbIsDocumentChanged);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

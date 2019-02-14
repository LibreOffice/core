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
#include <i18nlangtag/languagetag.hxx>
#include <i18nlangtag/mslangid.hxx>
#include <sfx2/dispatch.hxx>
#include <Outliner.hxx>
#include <editeng/outliner.hxx>

#include <DrawDocShell.hxx>
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
#include <svx/strings.hrc>
#include <svx/dialmgr.hxx>
#include <editeng/bulletitem.hxx>
#include <editeng/borderline.hxx>
#include <editeng/boxitem.hxx>
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
#include <editeng/numitem.hxx>
#include <editeng/unolingu.hxx>
#include <svl/itempool.hxx>
#include <editeng/outlobj.hxx>
#include <sfx2/viewfrm.hxx>
#include <editeng/frmdiritem.hxx>

#include <sdresid.hxx>
#include <drawdoc.hxx>
#include <sdpage.hxx>
#include <strings.hrc>
#include <glob.hxx>
#include <stlpool.hxx>
#include <shapelist.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <svl/itemset.hxx>
#include <app.hrc>
#include <strings.hxx>

namespace com { namespace sun { namespace star { namespace linguistic2 { class XHyphenator; } } } }

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
    OUString                aStdName(SdResId(STR_STANDARD_STYLESHEET_NAME));

    // Default style

    SfxStyleSearchBits nMask = SfxStyleSearchBits::Auto;

    OUString aName(aStdName);
    pSheet = &(pSSPool->Make(aName, SfxStyleFamily::Para, nMask));
    pSheet->SetHelpId( aHelpFile, HID_STANDARD_STYLESHEET_NAME );
    SfxItemSet& rISet = pSheet->GetItemSet();

    ::basegfx::B2DPolyPolygon aNullPolyPolygon;
    Color    aNullCol(COL_DEFAULT_SHAPE_STROKE);

    XDash     aNullDash;
    XGradient aNullGrad(aNullCol,COL_WHITE);
    aNullGrad.SetStartIntens( 100 );
    aNullGrad.SetEndIntens( 100 );
    XHatch    aNullHatch(aNullCol);

                    // Line attributes (Extended OutputDevice)
    rISet.Put(XLineStyleItem(drawing::LineStyle_SOLID));
    rISet.Put(XLineColorItem(OUString(), COL_DEFAULT_SHAPE_STROKE));
    rISet.Put(XLineWidthItem(0));
    rISet.Put(XLineDashItem(aNullDash));
    rISet.Put(XLineStartItem(aNullPolyPolygon));
    rISet.Put(XLineEndItem(aNullPolyPolygon));
    rISet.Put(XLineStartWidthItem(200));
    rISet.Put(XLineEndWidthItem(200));
    rISet.Put(XLineStartCenterItem());
    rISet.Put(XLineEndCenterItem());
    rISet.Put(SdrTextHorzAdjustItem(SDRTEXTHORZADJUST_BLOCK));

                    // Fill attributes (Extended OutputDevice)
    rISet.Put(XFillStyleItem(drawing::FillStyle_SOLID));
    rISet.Put(XFillColorItem(OUString(), COL_DEFAULT_SHAPE_FILLING));

    rISet.Put( XFillGradientItem( aNullGrad) );
    rISet.Put(XFillHatchItem(aNullHatch));
    Size    aNullSize( 32, 32 );
    Bitmap  aNullBmp( aNullSize, 8 );
    aNullBmp.Erase( COL_WHITE );
    rISet.Put(XFillBitmapItem(Graphic(aNullBmp)));

                    // Shadow attributes (Drawing Engine)
    rISet.Put(makeSdrShadowItem(false));
    rISet.Put(makeSdrShadowColorItem(COL_GRAY));
    rISet.Put(makeSdrShadowXDistItem(200));         // 3 mm Shadow distance
    rISet.Put(makeSdrShadowYDistItem(200));

    vcl::Font aLatinFont, aCJKFont, aCTLFont;

    getDefaultFonts( aLatinFont, aCJKFont, aCTLFont );

    SvxFontItem aSvxFontItem( aLatinFont.GetFamilyType(), aLatinFont.GetFamilyName(), aLatinFont.GetStyleName(), aLatinFont.GetPitch(),
                              aLatinFont.GetCharSet(), EE_CHAR_FONTINFO );

    SvxFontItem aSvxFontItemCJK( aCJKFont.GetFamilyType(), aCJKFont.GetFamilyName(), aCJKFont.GetStyleName(), aCJKFont.GetPitch(),
                                 aCJKFont.GetCharSet(), EE_CHAR_FONTINFO_CJK );

    SvxFontItem aSvxFontItemCTL( aCTLFont.GetFamilyType(), aCTLFont.GetFamilyName(), aCTLFont.GetStyleName(), aCTLFont.GetPitch(),
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
    rISet.Put(SvxUnderlineItem(LINESTYLE_NONE, EE_CHAR_UNDERLINE));
    rISet.Put(SvxOverlineItem(LINESTYLE_NONE, EE_CHAR_OVERLINE));
    rISet.Put(SvxCrossedOutItem(STRIKEOUT_NONE, EE_CHAR_STRIKEOUT ));
    rISet.Put(SvxCaseMapItem(SvxCaseMap::NotMapped, EE_CHAR_CASEMAP ));
    rISet.Put(SvxEmphasisMarkItem(FontEmphasisMark::NONE, EE_CHAR_EMPHASISMARK));
    rISet.Put(SvxCharReliefItem(FontRelief::NONE, EE_CHAR_RELIEF));
    rISet.Put(SvxColorItem(COL_AUTO, EE_CHAR_COLOR ));

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

    aBulletFont.SetFontSize(Size(0,635));   // sj: (i33745) changed default from 24 to 18 pt

    aBulletItem.SetFont(aBulletFont);
    aBulletItem.SetSymbol( 0x25CF );                    // In points
    rISet.Put(aBulletItem);

    // New BulletItem
    SdStyleSheetPool::PutNumBulletItem( pSheet, aBulletFont );

    SfxItemSet* pISet = nullptr;

    // Default > Object without filling
    {
        aName = SdResId(STR_POOLSHEET_OBJWITHOUTFILL);
        pSheet = &(pSSPool->Make(aName, SfxStyleFamily::Para, nMask));
        pSheet->SetParent(aStdName);
        pSheet->SetHelpId( aHelpFile, HID_POOLSHEET_OBJWITHOUTFILL );
    }
    // Default > Object no fill no line
    {
        aName = SdResId(STR_POOLSHEET_OBJNOLINENOFILL);
        pSheet = &(pSSPool->Make(aName, SfxStyleFamily::Para, nMask));
        pSheet->SetParent(aStdName);
        pSheet->SetHelpId( aHelpFile, HID_POOLSHEET_OBJNOLINENOFILL );
        pISet = &pSheet->GetItemSet();

        pISet->Put(XFillStyleItem(drawing::FillStyle_NONE));
        pISet->Put(XLineStyleItem(drawing::LineStyle_NONE));
    }

    // tdf#94369

    // Text
    OUString aTextName;
    {
        aTextName = SdResId(STR_POOLSHEET_TEXT);
        pSheet = &(pSSPool->Make(aTextName, SfxStyleFamily::Para, nMask));
        pSheet->SetHelpId( aHelpFile, HID_POOLSHEET_TEXT );
        pISet = &pSheet->GetItemSet();
        aSvxFontItem.SetFamilyName("Noto Sans");
        pISet->Put( aSvxFontItem );                                        // Noto Sans
        pISet->Put(XFillStyleItem(drawing::FillStyle_SOLID));              // solid fill
        pISet->Put(XFillColorItem(OUString(), Color(0xeeeeee)));           // light gray 5
        pISet->Put(XLineStyleItem(drawing::LineStyle_SOLID));              // solid fill
        pISet->Put(XLineColorItem(OUString(), Color(0xcccccc)));           // light gray 3
    }
    // Text > A4
    OUString aA4Name;
    {
        aA4Name = SdResId(STR_POOLSHEET_A4);
        pSheet = &(pSSPool->Make(aA4Name, SfxStyleFamily::Para, nMask));
        pSheet->SetParent( aTextName );
        pSheet->SetHelpId( aHelpFile, HID_POOLSHEET_A4 );
        pISet = &pSheet->GetItemSet();
        pISet->Put(SvxFontHeightItem(635, 100, EE_CHAR_FONTHEIGHT ));      // 18 pt
        pISet->Put(XFillStyleItem(drawing::FillStyle_NONE));               // no filling
    }
    // Text > A4 > Title
    {

        aName = SdResId(STR_POOLSHEET_A4_TITLE);
        pSheet = &(pSSPool->Make(aName, SfxStyleFamily::Para, nMask));
        pSheet->SetParent( aA4Name );
        pSheet->SetHelpId( aHelpFile, HID_POOLSHEET_A4_TITLE );
        pISet = &pSheet->GetItemSet();
        pISet->Put(SvxFontHeightItem(1551, 100, EE_CHAR_FONTHEIGHT ));     // 44 pt
        pISet->Put(XLineStyleItem(drawing::LineStyle_NONE));               // no border
    }
    // Text > A4 > Headline
    {
        aName = SdResId(STR_POOLSHEET_A4_HEADLINE);
        pSheet = &(pSSPool->Make(aName, SfxStyleFamily::Para, nMask));
        pSheet->SetParent( aA4Name );
        pSheet->SetHelpId( aHelpFile, HID_POOLSHEET_A4_HEADLINE );
        pISet = &pSheet->GetItemSet();
        pISet->Put(SvxFontHeightItem(847, 100, EE_CHAR_FONTHEIGHT ));      // 24 pt
        pISet->Put(XLineStyleItem(drawing::LineStyle_NONE));               // no border
    }
    // Text > A4 > Text
    {
        aName = SdResId(STR_POOLSHEET_A4_TEXT);
        pSheet = &(pSSPool->Make(aName, SfxStyleFamily::Para, nMask));
        pSheet->SetParent(aA4Name);
        pSheet->SetHelpId( aHelpFile, HID_POOLSHEET_A4_TEXT );
        pISet = &pSheet->GetItemSet();
        pISet->Put(XLineStyleItem(drawing::LineStyle_NONE));               // no border
    }
    // Text > A0
    OUString aA0Name;
    {
        aA0Name = SdResId(STR_POOLSHEET_A0);
        pSheet = &(pSSPool->Make(aA0Name, SfxStyleFamily::Para, nMask));
        pSheet->SetParent(aTextName);
        pSheet->SetHelpId( aHelpFile, HID_POOLSHEET_A0 );
        pISet = &pSheet->GetItemSet();
        pISet->Put(SvxFontHeightItem(1692, 100, EE_CHAR_FONTHEIGHT ));     // 48 pt
        pISet->Put(XFillStyleItem(drawing::FillStyle_NONE));               // no filling
    }
    // Text > A0 > Title
    {
        aName = SdResId(STR_POOLSHEET_A0_TITLE);
        pSheet = &(pSSPool->Make(aName, SfxStyleFamily::Para, nMask));
        pSheet->SetParent(aA0Name);
        pSheet->SetHelpId( aHelpFile, HID_POOLSHEET_A0_TITLE );
        pISet = &pSheet->GetItemSet();
        pISet->Put(SvxFontHeightItem(3385, 100, EE_CHAR_FONTHEIGHT ));      // 96 pt
        pISet->Put(XLineStyleItem(drawing::LineStyle_NONE));               // no border
    }
    // Text > A0 > Headline
    {
        aName = SdResId(STR_POOLSHEET_A0_HEADLINE);
        pSheet = &(pSSPool->Make(aName, SfxStyleFamily::Para, nMask));
        pSheet->SetParent(aA0Name);
        pSheet->SetHelpId( aHelpFile, HID_POOLSHEET_A0_HEADLINE );
        pISet = &pSheet->GetItemSet();
        pISet->Put(SvxFontHeightItem(2538, 100, EE_CHAR_FONTHEIGHT ));       // 72 pt
        pISet->Put(XLineStyleItem(drawing::LineStyle_NONE));               // no border
    }
    // Text > A0 > Text
    {
        aName = SdResId(STR_POOLSHEET_A0_TEXT);
        pSheet = &(pSSPool->Make(aName, SfxStyleFamily::Para, nMask));
        pSheet->SetParent(aA0Name);
        pSheet->SetHelpId( aHelpFile, HID_POOLSHEET_A0_TEXT );
        pISet = &pSheet->GetItemSet();
        pISet->Put(XLineStyleItem(drawing::LineStyle_NONE));               // no border
    }

    // Graphic
    OUString aGraphicName;
    XFillGradientItem aFillGradient;
    XGradient aGradient;

    {
        aGraphicName = SdResId(STR_POOLSHEET_GRAPHIC);
        pSheet = &(pSSPool->Make(aGraphicName, SfxStyleFamily::Para, nMask));
        pSheet->SetHelpId( aHelpFile, HID_POOLSHEET_GRAPHIC );
        pISet = &pSheet->GetItemSet();
        aSvxFontItem.SetFamilyName("Liberation Sans");                     // Liberation Sans
        pISet->Put( aSvxFontItem );
        pISet->Put( SvxFontHeightItem(635, 100, EE_CHAR_FONTHEIGHT) );     // 18 pt
        pISet->Put( XFillStyleItem(drawing::FillStyle_SOLID) );            // solid fill
        pISet->Put( XFillColorItem(OUString(), COL_WHITE) );               // filled white

    }
    // Graphic > Shapes
    OUString aShapesName;
    {
        aShapesName = SdResId(STR_POOLSHEET_SHAPES);
        pSheet = &(pSSPool->Make(aShapesName, SfxStyleFamily::Para, nMask));
        pSheet->SetParent( aGraphicName );
        pSheet->SetHelpId( aHelpFile, HID_POOLSHEET_SHAPES);
        pISet = &pSheet->GetItemSet();
        pISet->Put(XFillStyleItem(drawing::FillStyle_GRADIENT));           // fill with gradient
        aGradient.SetGradientStyle( ::awt::GradientStyle_RECT);            // square type
        aGradient.SetAngle( 0 );                                           // 0° angle
        aGradient.SetStartColor( Color(0xcccccc) );                        // white
        aGradient.SetEndColor( COL_WHITE );                                // light gray 3
        aFillGradient.SetName( aShapesName );
        aFillGradient.SetGradientValue(aGradient);
        pISet->Put( aFillGradient );
        pISet->Put( XLineStyleItem(drawing::LineStyle_NONE) );             // no border
        pISet->Put( SvxFontHeightItem(494, 100, EE_CHAR_FONTHEIGHT) );     // 14 pt
        pISet->Put( SvxWeightItem(WEIGHT_BOLD, EE_CHAR_WEIGHT) );          // bold
    }
    // Graphic > Shapes > Filled
    OUString  aFilledName(SdResId(STR_POOLSHEET_FILLED));
    {
        aName = aFilledName;
        pSheet = &(pSSPool->Make(aName, SfxStyleFamily::Para, nMask));
        pSheet->SetHelpId( aHelpFile, HID_POOLSHEET_FILLED );
        pSheet->SetParent( aShapesName );
        pISet = &pSheet->GetItemSet();

        aGradient.SetGradientStyle( ::awt::GradientStyle_LINEAR );
        aGradient.SetAngle( 300 );
        aGradient.SetStartColor( COL_WHITE );                              // white
        aGradient.SetEndColor( Color(0xcccccc) );                          // light gray 3
        aFillGradient.SetName( aName );
        aFillGradient.SetGradientValue(aGradient);
        pISet->Put( XFillStyleItem(drawing::FillStyle_GRADIENT) );
        pISet->Put( aFillGradient );
    }
    // Graphic > Shapes > Filled > Blue
    {
        aName =SdResId(STR_POOLSHEET_FILLED_BLUE);
        pSheet = &(pSSPool->Make(aName, SfxStyleFamily::Para, nMask));
        pSheet->SetParent(aFilledName);
        pSheet->SetHelpId( aHelpFile, HID_POOLSHEET_FILLED_BLUE );
        pISet = &pSheet->GetItemSet();

        aGradient.SetStartColor( Color(0x00729fcf) );                   // light blue 2
        aGradient.SetEndColor( Color(0x00355269) );                     // dark blue 2
        aFillGradient.SetName( aName );
        aFillGradient.SetGradientValue(aGradient);
        pISet->Put( aFillGradient );
        pISet->Put( SvxColorItem(COL_WHITE, EE_CHAR_COLOR ));           // font white
    }
    // Graphic > Shapes > Filled > Green
    {
        aName =SdResId(STR_POOLSHEET_FILLED_GREEN);
        pSheet = &(pSSPool->Make(aName, SfxStyleFamily::Para, nMask));
        pSheet->SetParent(aFilledName);
        pSheet->SetHelpId( aHelpFile, HID_POOLSHEET_FILLED_GREEN );
        pISet = &pSheet->GetItemSet();

        aGradient.SetStartColor( Color(0x0077bc65) );                   // light green 2
        aGradient.SetEndColor( Color(0x00127622) );                     // dark green 2
        aFillGradient.SetName( aName );
        aFillGradient.SetGradientValue(aGradient);
        pISet->Put( aFillGradient );
        pISet->Put( aSvxFontItem );                                     // font name
        pISet->Put( SvxColorItem(COL_WHITE, EE_CHAR_COLOR ));           // font white
    }
    // Graphic > Shapes > Filled > Red
    {
        aName =SdResId(STR_POOLSHEET_FILLED_RED);
        pSheet = &(pSSPool->Make(aName, SfxStyleFamily::Para, nMask));
        pSheet->SetParent(aFilledName);
        pSheet->SetHelpId( aHelpFile, HID_POOLSHEET_FILLED_RED );
        pISet = &pSheet->GetItemSet();

        aGradient.SetStartColor( Color(0x00ff6d6d) );                   // light red 2
        aGradient.SetEndColor( Color(0x00c9211e) );                     // dark red 2
        aFillGradient.SetName( aName );
        aFillGradient.SetGradientValue(aGradient);
        pISet->Put( aFillGradient );
        pISet->Put( SvxColorItem(COL_WHITE, EE_CHAR_COLOR ));           // font white
    }
    // Graphic > Shapes > Filled > Yellow
    {
        aName =SdResId(STR_POOLSHEET_FILLED_YELLOW);
        pSheet = &(pSSPool->Make(aName, SfxStyleFamily::Para, nMask));
        pSheet->SetParent(aFilledName);
        pSheet->SetHelpId( aHelpFile, HID_POOLSHEET_FILLED_YELLOW );
        pISet = &pSheet->GetItemSet();

        aGradient.SetStartColor( Color(0x00ffde59) );                   // light gold 2
        aGradient.SetEndColor( Color(0x00b47804) );                     // dark gold 2
        aFillGradient.SetName( aName );
        aFillGradient.SetGradientValue(aGradient);
        pISet->Put( aFillGradient );
        pISet->Put( SvxColorItem(COL_WHITE, EE_CHAR_COLOR ));           // font white
    }
    // Graphic > Shapes > Outlines
    OUString aOutlineName(SdResId(STR_POOLSHEET_OUTLINE));
    {
        aName = aOutlineName;
        pSheet = &(pSSPool->Make(aName, SfxStyleFamily::Para, nMask));
        pSheet->SetHelpId( aHelpFile, HID_POOLSHEET_OUTLINE );
        pSheet->SetParent( aShapesName );
        pISet = &pSheet->GetItemSet();
        pISet->Put( XFillStyleItem(drawing::FillStyle_NONE) );             // clear
        pISet->Put( XLineStyleItem(drawing::LineStyle_SOLID) );            // solide line
        pISet->Put( XLineWidthItem(81) );                                  // 2.3 pt
        pISet->Put( XLineColorItem(OUString(), COL_BLACK) );               // b/w
    }
    // Graphic > Shapes > Outlines > Blue
    {
        aName =SdResId(STR_POOLSHEET_OUTLINE_BLUE);
        pSheet = &(pSSPool->Make(aName, SfxStyleFamily::Para, nMask));
        pSheet->SetParent(aOutlineName);
        pSheet->SetHelpId( aHelpFile, HID_POOLSHEET_OUTLINE_BLUE );
        pISet = &pSheet->GetItemSet();
        pISet->Put( XLineColorItem(OUString(), Color(0x00355269)) );       // dark blue 2
        pISet->Put( SvxColorItem(Color(0x00355269), EE_CHAR_COLOR ));      // font color
    }
    // Graphic > Shapes > Outlines > Green
    {
        aName =SdResId(STR_POOLSHEET_OUTLINE_GREEN);
        pSheet = &(pSSPool->Make(aName, SfxStyleFamily::Para, nMask));
        pSheet->SetParent(aOutlineName);
        pSheet->SetHelpId( aHelpFile, HID_POOLSHEET_OUTLINE_GREEN );
        pISet = &pSheet->GetItemSet();
        pISet->Put( XLineColorItem(OUString(), Color(0x00127622)) );       // dark green 2
        pISet->Put( SvxColorItem(Color(0x00127622), EE_CHAR_COLOR ));      // font color
    }
    // Graphic > Shapes > Outlines > Red
    {
        aName =SdResId(STR_POOLSHEET_OUTLINE_RED);
        pSheet = &(pSSPool->Make(aName, SfxStyleFamily::Para, nMask));
        pSheet->SetParent(aOutlineName);
        pSheet->SetHelpId( aHelpFile, HID_POOLSHEET_OUTLINE_RED );
        pISet = &pSheet->GetItemSet();
        pISet->Put( XLineColorItem(OUString(), Color(0x00c9211e)) );       // dark red 2
        pISet->Put( SvxColorItem(Color(0x00c9211e), EE_CHAR_COLOR ));      // font color
    }
    // Graphic > Shapes > Outlines > Yellow
    {
        aName =SdResId(STR_POOLSHEET_OUTLINE_YELLOW);
        pSheet = &(pSSPool->Make(aName, SfxStyleFamily::Para, nMask));
        pSheet->SetParent(aOutlineName);
        pSheet->SetHelpId( aHelpFile, HID_POOLSHEET_OUTLINE_YELLOW );
        pISet = &pSheet->GetItemSet();
        pISet->Put( XLineStyleItem(drawing::LineStyle_SOLID));
        pISet->Put( XLineColorItem(OUString(), Color(0x00b47804)) );       // dark gold 2
        pISet->Put( SvxColorItem(Color(0x00b47804), EE_CHAR_COLOR ));      // font color
    }
    // Graphic > Lines
    OUString aLinesName;
    {
        aLinesName = SdResId(STR_POOLSHEET_LINES);
        pSheet = &(pSSPool->Make(aLinesName, SfxStyleFamily::Para, nMask));
        pSheet->SetParent( aGraphicName );
        pSheet->SetHelpId( aHelpFile, HID_POOLSHEET_LINES);
        pISet = &pSheet->GetItemSet();
        pISet->Put( XFillStyleItem(drawing::FillStyle_NONE) );             // clear
        pISet->Put( XLineStyleItem(drawing::LineStyle_SOLID) );            // solide line
        pISet->Put( XLineColorItem(OUString(), COL_BLACK) );               // b/w
    }
    // Graphic > Lines > Measurements
    {
        aName = SdResId(STR_POOLSHEET_MEASURE);
        pSheet = &(pSSPool->Make(aName, SfxStyleFamily::Para, nMask));
        pSheet->SetParent(aLinesName);
        pSheet->SetHelpId( aHelpFile, HID_POOLSHEET_MEASURE );
        pISet = &pSheet->GetItemSet();

        ::basegfx::B2DPolygon aArrow;                                      // arrows
        aArrow.append(::basegfx::B2DPoint(10.0, 0.0));
        aArrow.append(::basegfx::B2DPoint(0.0, 30.0));
        aArrow.append(::basegfx::B2DPoint(20.0, 30.0));
        aArrow.setClosed(true);

        pISet->Put(XLineStartItem(SvxResId(RID_SVXSTR_ARROW),::basegfx::B2DPolyPolygon(aArrow)));
        pISet->Put(XLineStartWidthItem(200));
        pISet->Put(XLineEndItem(SvxResId(RID_SVXSTR_ARROW),::basegfx::B2DPolyPolygon(aArrow)));
        pISet->Put(XLineEndWidthItem(200));
        pISet->Put(SdrYesNoItem(SDRATTR_MEASURESHOWUNIT, true));
    }
    // Graphic > Lines > Dashed
    {
        aName = SdResId(STR_POOLSHEET_LINES_DASHED);
        pSheet = &(pSSPool->Make(aName, SfxStyleFamily::Para, nMask));
        pSheet->SetParent(aLinesName);
        pSheet->SetHelpId( aHelpFile, HID_POOLSHEET_LINES_DASHED );
        pISet = &pSheet->GetItemSet();
        pISet->Put( XLineStyleItem(drawing::LineStyle_DASH) );            // dashed line
    }

    // Generate presentation templates for default layout.
    OUString aPrefix = SdResId(STR_LAYOUT_DEFAULT_NAME);
    pSSPool->CreateLayoutStyleSheets(aPrefix);
}

static Any implMakeSolidCellStyle( SdStyleSheetPool* pSSPool, const OUString& rName, const OUString& rParent, const Color& rColor )
{
    SfxStyleSheetBase* pSheet = &(pSSPool->Make(rName, SfxStyleFamily::Frame, SfxStyleSearchBits::Auto));
    pSheet->SetParent(rParent);
    SfxItemSet* pISet = &pSheet->GetItemSet();
    pISet->Put(XFillStyleItem(drawing::FillStyle_SOLID));
    pISet->Put(XFillColorItem(OUString(), rColor));

    return Any( Reference< XStyle >( static_cast< XWeak* >( pSheet ), UNO_QUERY ) );
}

static void implCreateTableTemplate( const Reference< XNameContainer >& xTableFamily, const OUString& rName, const Any& rBody, const Any& rHeading, const Any& rBanding )
{
    if( !xTableFamily.is() )
        return;

    try
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

    Reference< XNameContainer > xTableFamily( pSSPool->getByName( "table" ), UNO_QUERY );

    // ---- Default -----------------------------------------------

    OUString aDefaultCellStyleName( "default" );

    pSheet = &(pSSPool->Make(aDefaultCellStyleName, SfxStyleFamily::Frame, SfxStyleSearchBits::Auto));
    pSheet->SetHelpId( OUString(), HID_SD_CELL_STYLE_DEFAULT );
    SfxItemSet& rISet = pSheet->GetItemSet();

    XHatch    aNullHatch(COL_BLACK);

    rISet.Put(XFillStyleItem(drawing::FillStyle_SOLID));
    rISet.Put(XFillColorItem(OUString(), Color(0x00ccccff)));

    vcl::Font aLatinFont, aCJKFont, aCTLFont;

    getDefaultFonts( aLatinFont, aCJKFont, aCTLFont );

    SvxFontItem aSvxFontItem( aLatinFont.GetFamilyType(), aLatinFont.GetFamilyName(), aLatinFont.GetStyleName(), aLatinFont.GetPitch(),
                              aLatinFont.GetCharSet(), EE_CHAR_FONTINFO );

    SvxFontItem aSvxFontItemCJK( aCJKFont.GetFamilyType(), aCJKFont.GetFamilyName(), aCJKFont.GetStyleName(), aCJKFont.GetPitch(),
                                 aCJKFont.GetCharSet(), EE_CHAR_FONTINFO_CJK );

    SvxFontItem aSvxFontItemCTL( aCTLFont.GetFamilyType(), aCTLFont.GetFamilyName(), aCTLFont.GetStyleName(), aCTLFont.GetPitch(),
                                 aCTLFont.GetCharSet(), EE_CHAR_FONTINFO_CTL );

    rISet.Put( aSvxFontItem );
    rISet.Put( aSvxFontItemCJK );
    rISet.Put( aSvxFontItemCTL );

    rISet.Put( SvxFontHeightItem( 635, 100, EE_CHAR_FONTHEIGHT ) );     // sj: (i33745) changed default from 24 to 18 pt
    rISet.Put( SvxFontHeightItem( 635, 100, EE_CHAR_FONTHEIGHT_CJK ) ); // 18 pt
    rISet.Put( SvxFontHeightItem( convertFontHeightToCTL( 635 ), 100, EE_CHAR_FONTHEIGHT_CTL ) ); // 18 pt

    rISet.Put(SvxColorItem(COL_AUTO, EE_CHAR_COLOR ));

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
            &aWhite, 1, SvxBorderLineStyle::SOLID);

    SvxBoxItem aBoxItem( SDRATTR_TABLE_BORDER );
    aBoxItem.SetLine( &aBorderLine, SvxBoxItemLine::TOP );
    aBoxItem.SetLine( &aBorderLine, SvxBoxItemLine::BOTTOM );
    aBoxItem.SetLine( &aBorderLine, SvxBoxItemLine::LEFT );
    aBoxItem.SetLine( &aBorderLine, SvxBoxItemLine::RIGHT );

    rISet.Put( aBoxItem );

    // ---- default --------------------------------------------------

    Any aGray1( implMakeSolidCellStyle( pSSPool, "gray1" , aDefaultCellStyleName, Color(230,230,230)));
    Any aGray2( implMakeSolidCellStyle( pSSPool, "gray2" , aDefaultCellStyleName, Color(204,204,204)));
    Any aGray3( implMakeSolidCellStyle( pSSPool, "gray3" , aDefaultCellStyleName, Color(179,179,179)));

    implCreateTableTemplate( xTableFamily, "default" , aGray1, aGray3, aGray2 );

    // ---- BW ------------------------------------------------

    Any aBW1( implMakeSolidCellStyle( pSSPool, "bw1" , aDefaultCellStyleName, Color(255,255,255)));
    Any aBW2( implMakeSolidCellStyle( pSSPool, "bw2" , aDefaultCellStyleName, Color(230,230,230)));
    Any aBW3( implMakeSolidCellStyle( pSSPool, "bw3" , aDefaultCellStyleName, Color(0,0,0)));

    implCreateTableTemplate( xTableFamily, "bw" , aBW1, aBW3, aBW2 );

    // ---- Orange --------------------------------------------------

    Any aOrange1( implMakeSolidCellStyle( pSSPool, "orange1" , aDefaultCellStyleName, Color(255,204,153)));
    Any aOrange2( implMakeSolidCellStyle( pSSPool, "orange2" , aDefaultCellStyleName, Color(255,153,102)));
    Any aOrange3( implMakeSolidCellStyle( pSSPool, "orange3" , aDefaultCellStyleName, Color(255,102,51)));

    implCreateTableTemplate( xTableFamily, "orange" , aOrange1, aOrange3, aOrange2 );

    // ---- Turquoise --------------------------------------------------

    Any aTurquoise1( implMakeSolidCellStyle( pSSPool, "turquoise1" , aDefaultCellStyleName, Color(71,184,184)));
    Any aTurquoise2( implMakeSolidCellStyle( pSSPool, "turquoise2" , aDefaultCellStyleName, Color(51,163,163)));
    Any aTurquoise3( implMakeSolidCellStyle( pSSPool, "turquoise3" , aDefaultCellStyleName, Color(25,138,138)));

    implCreateTableTemplate( xTableFamily, "turquoise" , aTurquoise1, aTurquoise3, aTurquoise2 );

    // ---- Gray ------------------------------------------------

    Any aBlue1( implMakeSolidCellStyle( pSSPool, "blue1" , aDefaultCellStyleName, Color(153,204,255)));
    Any aBlue2( implMakeSolidCellStyle( pSSPool, "blue2" , aDefaultCellStyleName, Color(0,153,255)));
    Any aBlue3( implMakeSolidCellStyle( pSSPool, "blue3" , aDefaultCellStyleName, Color(0,102,204)));

    implCreateTableTemplate( xTableFamily, "blue" , aBlue1, aBlue3, aBlue2 );

    // ---- Sun ------------------------------------------------

    Any aSun1( implMakeSolidCellStyle( pSSPool, "sun1" , aDefaultCellStyleName, Color(230,230,255)));
    Any aSun2( implMakeSolidCellStyle( pSSPool, "sun2" , aDefaultCellStyleName, Color(204,204,255)));
    Any aSun3( implMakeSolidCellStyle( pSSPool, "sun3" , aDefaultCellStyleName, Color(153,153,255)));

    implCreateTableTemplate( xTableFamily, "sun" , aSun1, aSun3, aSun2 );

    // ---- Earth ----------------------------------------------

    Any aEarth1( implMakeSolidCellStyle( pSSPool, "earth1" , aDefaultCellStyleName, Color(255,255,204)));
    Any aEarth2( implMakeSolidCellStyle( pSSPool, "earth2" , aDefaultCellStyleName, Color(255,204,153)));
    Any aEarth3( implMakeSolidCellStyle( pSSPool, "earth3" , aDefaultCellStyleName, Color(204,102,51)));

    implCreateTableTemplate( xTableFamily, "earth" , aEarth1, aEarth3, aEarth2 );

    // ---- Green ----------------------------------------------

    Any aGreen1( implMakeSolidCellStyle( pSSPool, "green1" , aDefaultCellStyleName, Color(255,255,204)));
    Any aGreen2( implMakeSolidCellStyle( pSSPool, "green2" , aDefaultCellStyleName, Color(148,189,94)));
    Any aGreen3( implMakeSolidCellStyle( pSSPool, "green3" , aDefaultCellStyleName, Color(92,133,38)));

    implCreateTableTemplate( xTableFamily, "green" , aGreen1, aGreen3, aGreen2 );

    // ---- Seaweed ----------------------------------------------

    Any aSeetang1( implMakeSolidCellStyle( pSSPool, "seetang1" , aDefaultCellStyleName, Color(204,255,255)));
    Any aSeetang2( implMakeSolidCellStyle( pSSPool, "seetang2" , aDefaultCellStyleName, Color(71,184,184)));
    Any aSeetang3( implMakeSolidCellStyle( pSSPool, "seetang3" , aDefaultCellStyleName, Color(51,163,163)));

    implCreateTableTemplate( xTableFamily, "seetang" , aSeetang1, aSeetang3, aSeetang2 );

    // ---- LightBlue ----------------------------------------------

    Any aLightBlue1( implMakeSolidCellStyle( pSSPool, "lightblue1" , aDefaultCellStyleName, Color(255,255,255)));
    Any aLightBlue2( implMakeSolidCellStyle( pSSPool, "lightblue2" , aDefaultCellStyleName, Color(230,230,255)));
    Any aLightBlue3( implMakeSolidCellStyle( pSSPool, "lightblue3" , aDefaultCellStyleName, Color(153,153,204)));

    implCreateTableTemplate( xTableFamily, "lightblue" , aLightBlue1, aLightBlue3, aLightBlue2 );

    // ---- Yellow ----------------------------------------------

    Any aYellow1( implMakeSolidCellStyle( pSSPool, "yellow1" , aDefaultCellStyleName, Color(255,255,204)));
    Any aYellow2( implMakeSolidCellStyle( pSSPool, "yellow2" , aDefaultCellStyleName, Color(255,255,153)));
    Any aYellow3( implMakeSolidCellStyle( pSSPool, "yellow3" , aDefaultCellStyleName, Color(255,204,153)));

    implCreateTableTemplate( xTableFamily, "yellow" , aYellow1, aYellow3, aYellow2 );
}

// Number of pages that reference a master page
sal_uInt16 SdDrawDocument::GetMasterPageUserCount(SdrPage const * pMaster) const
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

    mpOnlineSpellingIdle.reset();
    mpOnlineSpellingList.reset();
}

// Start OnlineSpelling in the background
void SdDrawDocument::StartOnlineSpelling(bool bForceSpelling)
{
    if (!(mbOnlineSpell && (bForceSpelling || mbInitialOnlineSpellingEnabled) &&
        mpDocSh && !mpDocSh->IsReadOnly()) )
        return;

    StopOnlineSpelling();

    SdOutliner* pOutl = GetInternalOutliner();

    Reference< XSpellChecker1 > xSpellChecker( LinguMgr::GetSpellChecker() );
    if ( xSpellChecker.is() )
        pOutl->SetSpeller( xSpellChecker );

    Reference< XHyphenator > xHyphenator( LinguMgr::GetHyphenator() );
    if( xHyphenator.is() )
        pOutl->SetHyphenator( xHyphenator );

    pOutl->SetDefaultLanguage( meLanguage );

    mpOnlineSpellingList.reset(new ShapeList);
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
    mpOnlineSpellingIdle.reset(new Idle("OnlineSpelling"));
    mpOnlineSpellingIdle->SetInvokeHandler( LINK(this, SdDrawDocument, OnlineSpellingHdl) );
    mpOnlineSpellingIdle->SetPriority(TaskPriority::LOWEST);
    mpOnlineSpellingIdle->Start();
}

// Fill OnlineSpelling list
void SdDrawDocument::FillOnlineSpellingList(SdPage const * pPage)
{
    SdrObjListIter aIter(pPage, SdrIterMode::Flat);

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
            SdrObjListIter aGroupIter(static_cast< SdrObjGroup* >(pObj)->GetSubList(), SdrIterMode::DeepNoGroups);

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
IMPL_LINK_NOARG(SdDrawDocument, OnlineSpellingHdl, Timer *, void)
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
                SdrObjListIter aGroupIter(static_cast< SdrObjGroup* >(pObj)->GetSubList(), SdrIterMode::DeepNoGroups);


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

        mpOnlineSearchItem.reset();
    }
}

// Spell object (for OnlineSpelling)
void SdDrawDocument::SpellObject(SdrTextObj* pObj)
{
    if (!(pObj && pObj->GetOutlinerParaObject()) /* && pObj != pView->GetTextEditObject() */)
        return;

    mbHasOnlineSpellErrors = false;
    SdOutliner* pOutl = GetInternalOutliner();
    pOutl->SetUpdateMode(true);
    Link<EditStatus&,void> aEvtHdl = pOutl->GetStatusEventHdl();
    pOutl->SetStatusEventHdl(LINK(this, SdDrawDocument, OnlineSpellEventHdl));

    OutlinerMode nOldOutlMode = pOutl->GetMode();
    OutlinerMode nOutlMode = OutlinerMode::TextObject;
    if (pObj->GetObjInventor() == SdrInventor::Default &&
        pObj->GetObjIdentifier() == OBJ_OUTLINETEXT)
    {
        nOutlMode = OutlinerMode::OutlineObject;
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
            std::unique_ptr<OutlinerParaObject> pOPO = pOutl->CreateParaObject();
            if (pOPO)
            {
                if (!( *pOPO == *pObj->GetOutlinerParaObject() ) ||
                     !pObj->GetOutlinerParaObject()->isWrongListEqual( *pOPO ))
                {
                    sd::ModifyGuard aGuard( this );

                    // taking text from the outliner
                    // use non-broadcasting version to avoid O(n^2)
                    pObj->NbcSetOutlinerParaObject( std::move(pOPO) );
                }
            }
        }
    }

    pOutl->SetStatusEventHdl(aEvtHdl);
    pOutl->SetUpdateMode(false);
    pOutl->Init( nOldOutlMode );
    mbHasOnlineSpellErrors = false;
}

// Object was inserted into model
void SdDrawDocument::InsertObject(SdrObject* pObj)
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
void SdDrawDocument::RemoveObject(SdrObject* pObj)
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
IMPL_LINK(SdDrawDocument, OnlineSpellEventHdl, EditStatus&, rEditStat, void)
{
    EditStatusFlags nStat = rEditStat.GetStatusWord();
    mbHasOnlineSpellErrors = bool(nStat & EditStatusFlags::WRONGWORDCHANGED);
}

// Callback for ExecuteSpellPopup()

// removed link and replaced with Imp method
void SdDrawDocument::ImpOnlineSpellCallback(SpellCallbackInfo const * pInfo, SdrObject* pObj, SdrOutliner const * pOutl)
{
    mpOnlineSearchItem.reset();

    SpellCallbackCommand nCommand = pInfo->nCommand;

    if (nCommand == SpellCallbackCommand::IGNOREWORD
        // restart when add to dictionary takes place, too.
        || nCommand == SpellCallbackCommand::ADDTODICTIONARY)
    {
        if(pOutl && dynamic_cast< const SdrTextObj *>( pObj ))
        {
            bool bModified(IsChanged());
            static_cast<SdrTextObj*>(pObj)->SetOutlinerParaObject(pOutl->CreateParaObject());
            SetChanged(bModified);
            pObj->BroadcastObjectChange();
        }

        mpOnlineSearchItem.reset(new SvxSearchItem( SID_SEARCH_ITEM ) );
        mpOnlineSearchItem->SetSearchString(pInfo->aWord);
        StartOnlineSpelling();
    }
    else if (nCommand == SpellCallbackCommand::STARTSPELLDLG)
        SfxViewFrame::Current()->GetDispatcher()->Execute( SID_SPELL_DIALOG, SfxCallMode::ASYNCHRON );
    else if (nCommand == SpellCallbackCommand::AUTOCORRECT_OPTIONS)
        SfxViewFrame::Current()->GetDispatcher()->Execute( SID_AUTO_CORRECT_DLG, SfxCallMode::ASYNCHRON );
}

// Return formatted page number (1, I, i, a, etc.)
OUString SdDrawDocument::CreatePageNumValue(sal_uInt16 nNum) const
{
    OUString aPageNumValue;
    bool bUpper = false;

    switch (mePageNumType)
    {
        case css::style::NumberingType::CHARS_UPPER_LETTER:
            aPageNumValue += OUStringLiteral1( (nNum - 1) % 26 + 'A' );
            break;
        case css::style::NumberingType::CHARS_LOWER_LETTER:
            aPageNumValue += OUStringLiteral1( (nNum - 1) % 26 + 'a' );
            break;
        case css::style::NumberingType::ROMAN_UPPER:
            bUpper = true;
            [[fallthrough]];
        case css::style::NumberingType::ROMAN_LOWER:
            aPageNumValue += SvxNumberFormat::CreateRomanString(nNum, bUpper);
            break;
        case css::style::NumberingType::NUMBER_NONE:
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
    SfxStyleSheetIterator aIter(mxStyleSheetPool.get(), SfxStyleFamily::Page);
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
    aPageLayoutName += aSep + STR_LAYOUT_OUTLINE;

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

                if (pObj->GetObjInventor() == SdrInventor::Default)
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
                                for (const auto& rRepl : aReplList)
                                    pOPO->ChangeStyleSheets( rRepl.aName, rRepl.nFamily, rRepl.aNewName, rRepl.nNewFamily );
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

                if (pObj->GetObjInventor() == SdrInventor::Default)
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
                                for (const auto& rRepl : aReplList)
                                    pOPO->ChangeStyleSheets( rRepl.aName, rRepl.nFamily, rRepl.aNewName, rRepl.nNewFamily );
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
    // BulletItem and BulletFont for Title and Outline
    SvxBulletItem aBulletItem(EE_PARA_BULLET);
    vcl::Font aBulletFont( SdStyleSheetPool::GetBulletFont() );
    aBulletFont.SetFontSize(Size(0,846));       // 24 pt
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
    aNumberFormat.SetBulletColor(COL_AUTO);
    aNumberFormat.SetStart(1);
    aNumberFormat.SetNumAdjust(SvxAdjust::Left);

    SvxNumRule aNumRule( SvxNumRuleFlags::BULLET_REL_SIZE | SvxNumRuleFlags::BULLET_COLOR, SVX_MAX_NUM, false);

    //aNumberFormat.SetAbsLSpace( 0 );
    //aNumberFormat.SetFirstLineOffset( 0 );
    //aNumRule.SetLevel( 0, aNumberFormat );

    for( sal_uInt16 i = 0; i < aNumRule.GetLevelCount(); i++ )
    {
        const auto nLSpace = (i + 1) * 600;
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
            case SvxFrameDirection::Horizontal_LR_TB: eRet = css::text::WritingMode_LR_TB; break;
            case SvxFrameDirection::Horizontal_RL_TB: eRet = css::text::WritingMode_RL_TB; break;
            case SvxFrameDirection::Vertical_RL_TB: eRet = css::text::WritingMode_TB_RL; break;

            default:
                OSL_FAIL( "Frame direction not supported yet" );
            break;
        }
    }

    return eRet;
}

void SdDrawDocument::SetDefaultWritingMode(css::text::WritingMode eMode )
{
    if( !pItemPool )
        return;

    SvxFrameDirection nVal;
    switch( eMode )
    {
    case css::text::WritingMode_LR_TB: nVal = SvxFrameDirection::Horizontal_LR_TB; break;
    case css::text::WritingMode_RL_TB: nVal = SvxFrameDirection::Horizontal_RL_TB; break;
    case css::text::WritingMode_TB_RL: nVal = SvxFrameDirection::Vertical_RL_TB; break;
    default:
        OSL_FAIL( "Frame direction not supported yet" );
        return;
    }

    SvxFrameDirectionItem aModeItem( nVal, EE_PARA_WRITINGDIR );
    pItemPool->SetPoolDefaultItem( aModeItem );

    SvxAdjustItem aAdjust( SvxAdjust::Left, EE_PARA_JUST );

    if( eMode == css::text::WritingMode_RL_TB )
        aAdjust.SetAdjust( SvxAdjust::Right );

    pItemPool->SetPoolDefaultItem( aAdjust );
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
        nWesternFontHeight = static_cast<sal_uInt32>(fTemp);
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

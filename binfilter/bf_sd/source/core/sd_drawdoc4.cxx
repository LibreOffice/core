/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include <bf_offmgr/app.hxx>
#include <bf_svx/eeitem.hxx>

#include "bf_sd/docshell.hxx"
#include <eetext.hxx>

#define ITEMID_SEARCH               SID_SEARCH_ITEM
#define ITEMID_LANGUAGE             EE_CHAR_LANGUAGE
#define ITEMID_EMPHASISMARK         EE_CHAR_EMPHASISMARK
#define ITEMID_CHARRELIEF           EE_CHAR_RELIEF
#define ITEMID_FRAMEDIR             EE_PARA_WRITINGDIR

#include <bf_svx/svxids.hrc>
#include <bf_svx/lrspitem.hxx>
#include <bf_svx/ulspitem.hxx>
#include <bf_svx/lspcitem.hxx>
#include <bf_svx/adjitem.hxx>
#include <bf_svx/dialogs.hrc>
#include <bf_svx/dialmgr.hxx>					// SVX_RESSTR
#include <bf_svx/bulitem.hxx>
#include <bf_svx/xtable.hxx>

#include <bf_svx/xdef.hxx>

#include <bf_svx/svdotext.hxx>
#include <bf_svx/colritem.hxx>
#include <bf_svx/fhgtitem.hxx>
#include <bf_svx/wghtitem.hxx>
#include <bf_svx/postitem.hxx>
#include <bf_svx/crsditem.hxx>
#include <bf_svx/udlnitem.hxx>
#include <bf_svx/cntritem.hxx>
#include <bf_svx/emphitem.hxx>
#include <bf_svx/fontitem.hxx>
#include <bf_svx/shdditem.hxx>
#include <bf_svx/xbtmpit.hxx>
#include <bf_svx/xflhtit.hxx>
#include <bf_svx/xflgrit.hxx>
#include <bf_svx/xflclit.hxx>
#include <bf_svx/xlnedcit.hxx>
#include <bf_svx/xlnstcit.hxx>
#include <bf_svx/xlnedwit.hxx>
#include <bf_svx/xlnstwit.hxx>
#include <bf_svx/xlnedit.hxx>
#include <bf_svx/charreliefitem.hxx>
#include <bf_svx/xlnstit.hxx>
#include <bf_svx/xlndsit.hxx>
#include <bf_svx/xlnwtit.hxx>
#include <bf_svx/xlnclit.hxx>
#include <tools/shl.hxx>
#include <bf_svx/numitem.hxx>
#include <bf_svx/editeng.hxx>
#include <bf_svtools/itempool.hxx>
#include <bf_svx/outlobj.hxx>
#include <bf_svx/frmdiritem.hxx>

#include "sdresid.hxx"
#include "drawdoc.hxx"
#include "sdpage.hxx"
#include "glob.hrc"
#include "stlpool.hxx"
#include "helpids.h"
namespace binfilter {

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::linguistic2;

void SdDrawDocument::CreateLayoutTemplates()
{
    SdStyleSheetPool*       pStyleSheetPool = (SdStyleSheetPool*)GetStyleSheetPool();
    SfxStyleSheetBase*      pSheet = NULL;
    String                  aHelpFile;
    String                  aStdName = String(SdResId(STR_STANDARD_STYLESHEET_NAME));

    // ---- Standardvorlage -----------------------------------------------

    // nicht benutzt, nicht benutzerdefiniert
    // SB hatte wahrscheinlich Probleme mit SFXSTYLEBIT_AUTO, da dann gar nichts
    // mehr im Gestalter angezeigt wird. Dieses Problem ist zu 364 j behoben worden
    // USHORT nMask = SFXSTYLEBIT_ALL & ~(SFXSTYLEBIT_USED | SFXSTYLEBIT_USERDEF);
    USHORT nMask = SFXSTYLEBIT_AUTO;

    String aName(aStdName);
    pSheet = &(pStyleSheetPool->Make(aName, SFX_STYLE_FAMILY_PARA, nMask));
    pSheet->SetHelpId( aHelpFile, HID_STANDARD_STYLESHEET_NAME );
    SfxItemSet& rISet = pSheet->GetItemSet();
    SfxItemPool* pPool = rISet.GetPool();

    String	 aNullStr;

    XPolygon aNullPol;
    Color	 aNullCol(RGB_Color(COL_BLACK));

    XDash	  aNullDash;
    XGradient aNullGrad(aNullCol,RGB_Color(COL_WHITE));
              aNullGrad.SetStartIntens( 100 );
              aNullGrad.SetEndIntens( 100 );
    XHatch	  aNullHatch(aNullCol);

                    // Linienattribute (Extended OutputDevice)
    rISet.Put(XLineStyleItem(XLINE_SOLID));
    rISet.Put(XLineColorItem(String(), RGB_Color(COL_BLACK)));
    rISet.Put(XLineWidthItem(0));
    rISet.Put(XLineDashItem(pPool,aNullDash));
    rISet.Put(XLineStartItem(pPool,aNullPol));
    rISet.Put(XLineEndItem(pPool,aNullPol));
    rISet.Put(XLineStartWidthItem(300));
    rISet.Put(XLineEndWidthItem(300));
    rISet.Put(XLineStartCenterItem());
    rISet.Put(XLineEndCenterItem());

                    // Fuellattribute (Extended OutputDevice)
    rISet.Put(XFillStyleItem(XFILL_SOLID));
    rISet.Put(XFillColorItem(String(), Color(0,184,255))); // "Blau 7"

    rISet.Put(XFillGradientItem(pPool,aNullGrad));
    rISet.Put(XFillHatchItem(pPool,aNullHatch));
    Size    aNullSize( 32, 32 );
    Color   aNullColor( COL_WHITE );
    Bitmap  aNullBmp( aNullSize, 8 );
    aNullBmp.Erase( aNullColor );
    rISet.Put(XFillBitmapItem(pPool,aNullBmp));

    // Schattenattribute (Drawing Engine)
    rISet.Put(SdrShadowItem(FALSE));
    rISet.Put(SdrShadowColorItem(aNullStr, RGB_Color(COL_GRAY)));
    rISet.Put(SdrShadowXDistItem(300)); 		// 3 mm Schattendistanz
    rISet.Put(SdrShadowYDistItem(300));

    Font aLatinFont, aCJKFont, aCTLFont;

    getDefaultFonts( aLatinFont, aCJKFont, aCTLFont );

    SvxFontItem aSvxFontItem( aLatinFont.GetFamily(), aLatinFont.GetName(), aLatinFont.GetStyleName(), aLatinFont.GetPitch(),
                              aLatinFont.GetCharSet(), EE_CHAR_FONTINFO );

    SvxFontItem aSvxFontItemCJK( aCJKFont.GetFamily(), aCJKFont.GetName(), aCJKFont.GetStyleName(), aCJKFont.GetPitch(),
                                 aCJKFont.GetCharSet(), EE_CHAR_FONTINFO_CJK );

    SvxFontItem aSvxFontItemCTL( aCTLFont.GetFamily(), aCTLFont.GetName(), aCTLFont.GetStyleName(), aCTLFont.GetPitch(),
                                 aCTLFont.GetCharSet(), EE_CHAR_FONTINFO_CTL );

    rISet.Put( aSvxFontItem );
    rISet.Put( aSvxFontItemCJK );
    rISet.Put( aSvxFontItemCTL );

    rISet.Put( SvxFontHeightItem( 846, 100, EE_CHAR_FONTHEIGHT ) );     // 24 pt
    rISet.Put( SvxFontHeightItem( 846, 100, EE_CHAR_FONTHEIGHT_CJK ) ); // 24 pt
    rISet.Put( SvxFontHeightItem( 846, 100, EE_CHAR_FONTHEIGHT_CTL ) ); // 24 pt

    rISet.Put( SvxWeightItem( WEIGHT_NORMAL, EE_CHAR_WEIGHT ) );
    rISet.Put( SvxWeightItem( WEIGHT_NORMAL, EE_CHAR_WEIGHT_CJK ) );
    rISet.Put( SvxWeightItem( WEIGHT_NORMAL, EE_CHAR_WEIGHT_CTL ) );

    rISet.Put( SvxPostureItem( ITALIC_NONE, EE_CHAR_ITALIC ) );
    rISet.Put( SvxPostureItem( ITALIC_NONE, EE_CHAR_ITALIC_CJK ) );
    rISet.Put( SvxPostureItem( ITALIC_NONE, EE_CHAR_ITALIC_CTL ) );

    rISet.Put(SvxContourItem(FALSE));
    rISet.Put(SvxShadowedItem(FALSE));
    rISet.Put(SvxUnderlineItem(UNDERLINE_NONE));
    rISet.Put(SvxCrossedOutItem(STRIKEOUT_NONE));
    rISet.Put(SvxEmphasisMarkItem(EMPHASISMARK_NONE));
    rISet.Put(SvxCharReliefItem(RELIEF_NONE));
    rISet.Put(SvxColorItem(Color(COL_AUTO)));

    // Absatzattribute (Edit Engine)
    rISet.Put(SvxLRSpaceItem());
    rISet.Put(SvxULSpaceItem());

    rISet.Put(SvxLineSpacingItem());

    SvxBulletItem aBulletItem(EE_PARA_BULLET);
                            // die sind in allen Ebenen identisch
    aBulletItem.SetStyle(BS_BULLET);
    aBulletItem.SetStart(1);
    aBulletItem.SetScale(45);			// in Prozent

    Font aBulletFont( pStyleSheetPool->GetBulletFont() );
    aBulletFont.SetSize(Size(0,846));		// 24 pt
    aBulletItem.SetFont(aBulletFont);
    aBulletItem.SetSymbol( 0x25CF );					// Punkt
    rISet.Put(aBulletItem);

    SfxUInt16Item aBulletStateItem(EE_PARA_BULLETSTATE, 0); // Bullets nicht sichtbar
    rISet.Put(aBulletStateItem);

    // Neues BulletItem
    pStyleSheetPool->PutNumBulletItem( pSheet, aBulletFont );

    SfxItemSet* pISet = NULL;

    // ---- Objekt mit Pfeilspitze ----------------------------------------

    aName = String(SdResId(STR_POOLSHEET_OBJWITHARROW));
    pSheet = &(pStyleSheetPool->Make(aName, SFX_STYLE_FAMILY_PARA, nMask));
    pSheet->SetParent(aStdName);
    pSheet->SetHelpId( aHelpFile, HID_POOLSHEET_OBJWITHARROW );
    pISet = &pSheet->GetItemSet();

    pISet->Put(XLineStyleItem(XLINE_SOLID));
    pISet->Put(XLineColorItem(String(), RGB_Color(COL_BLACK)));
    pISet->Put(XLineWidthItem(150));

    XPolygon aArrow(4);                          //      []
    aArrow[0]=Point(10,0);                        // 0,4__[]__2,4
    aArrow[1]=Point(0,30);                      //    \    /
    aArrow[2]=Point(20,30);                        //     \  /
    aArrow[3]=Point(10,0);                        //      \/1,0
    pISet->Put(XLineStartItem(SdResId(STR_POOLSHEET_ARROW),aArrow));

    pISet->Put(XLineStartWidthItem(700));
    pISet->Put(XLineEndWidthItem(300));
    pISet->Put(XLineStartCenterItem(TRUE));

    // ---- Objekt mit Schatten -------------------------------------------

    aName = String(SdResId(STR_POOLSHEET_OBJWITHSHADOW));
    pSheet = &(pStyleSheetPool->Make(aName, SFX_STYLE_FAMILY_PARA, nMask));
    pSheet->SetParent(aStdName);
    pSheet->SetHelpId( aHelpFile, HID_POOLSHEET_OBJWITHSHADOW );
    pISet = &pSheet->GetItemSet();

    pISet->Put(SdrShadowItem(TRUE));
    pISet->Put(SdrShadowColorItem(aNullStr, RGB_Color(COL_GRAY)));
    pISet->Put(SdrShadowXDistItem(300));		// 3 mm Schattendistanz
    pISet->Put(SdrShadowYDistItem(300));

    // ---- Objekt ohne Fllung -------------------------------------------

    aName = String(SdResId(STR_POOLSHEET_OBJWITHOUTFILL));
    pSheet = &(pStyleSheetPool->Make(aName, SFX_STYLE_FAMILY_PARA, nMask));
    pSheet->SetParent(aStdName);
    pSheet->SetHelpId( aHelpFile, HID_POOLSHEET_OBJWITHOUTFILL );
    pISet = &pSheet->GetItemSet();

    pISet->Put(XFillStyleItem(XFILL_NONE));

    // ---- Text ----------------------------------------------------------

    aName = String(SdResId(STR_POOLSHEET_TEXT));
    pSheet = &(pStyleSheetPool->Make(aName, SFX_STYLE_FAMILY_PARA, nMask));
    pSheet->SetParent(aStdName);
    pSheet->SetHelpId( aHelpFile, HID_POOLSHEET_TEXT );
    pISet = &pSheet->GetItemSet();

    pISet->Put(XLineStyleItem(XLINE_NONE));
    pISet->Put(XFillStyleItem(XFILL_NONE));

    // ---- Textk”rper ----------------------------------------------------

    aName = String(SdResId(STR_POOLSHEET_TEXTBODY));
    pSheet = &(pStyleSheetPool->Make(aName, SFX_STYLE_FAMILY_PARA, nMask));
    pSheet->SetParent(aStdName);
    pSheet->SetHelpId( aHelpFile, HID_POOLSHEET_TEXTBODY );
    pISet = &pSheet->GetItemSet();

    pISet->Put(XLineStyleItem(XLINE_NONE));
    pISet->Put(XFillStyleItem(XFILL_NONE));

    pISet->Put(SvxFontHeightItem(564)); 		// 16 pt

    // ---- Textk”rper mit Blocksatz --------------------------------------

    aName = String(SdResId(STR_POOLSHEET_TEXTBODY_JUSTIFY));
    pSheet = &(pStyleSheetPool->Make(aName, SFX_STYLE_FAMILY_PARA, nMask));
    pSheet->SetParent(aStdName);
    pSheet->SetHelpId( aHelpFile, HID_POOLSHEET_TEXTBODY_JUSTIFY );
    pISet = &pSheet->GetItemSet();

    pISet->Put(XLineStyleItem(XLINE_NONE));
    pISet->Put(XFillStyleItem(XFILL_NONE));

    pISet->Put(SvxAdjustItem(SVX_ADJUST_BLOCK));

    // ---- Textkoerper mit Einzug -----------------------------------------

    aName = String(SdResId(STR_POOLSHEET_TEXTBODY_INDENT));
    pSheet = &(pStyleSheetPool->Make(aName, SFX_STYLE_FAMILY_PARA, nMask));
    pSheet->SetParent(aStdName);
    pSheet->SetHelpId( aHelpFile, HID_POOLSHEET_TEXTBODY_INDENT );
    pISet = &pSheet->GetItemSet();

    pISet->Put(XLineStyleItem(XLINE_NONE));
    pISet->Put(XFillStyleItem(XFILL_NONE));

    SvxLRSpaceItem aLRSpaceItem;
    aLRSpaceItem.SetTxtFirstLineOfst(600); 		// Erstzeileneinzug 6mm, rechts 0
    pISet->Put(aLRSpaceItem);

    // SvxLRSpaceItem hart gesetzt: NumBulletItem anpassen
    SvxNumBulletItem aNumBullet( (const SvxNumBulletItem&) pISet->Get(EE_PARA_NUMBULLET) );
    EditEngine::ImportBulletItem( aNumBullet, 0, NULL, &aLRSpaceItem );
    pISet->Put( aNumBullet );

    // ---- Titel ---------------------------------------------------------

    aName = String(SdResId(STR_POOLSHEET_TITLE));
    pSheet = &(pStyleSheetPool->Make(aName, SFX_STYLE_FAMILY_PARA, nMask));
    pSheet->SetParent(aStdName);
    pSheet->SetHelpId( aHelpFile, HID_POOLSHEET_TITLE );
    pISet = &pSheet->GetItemSet();

    pISet->Put(XLineStyleItem(XLINE_NONE));
    pISet->Put(XFillStyleItem(XFILL_NONE));

    pISet->Put(SvxFontHeightItem(1551));		// 44 pt

    // ---- Titel1 --------------------------------------------------------

    aName = String(SdResId(STR_POOLSHEET_TITLE1));
    pSheet = &(pStyleSheetPool->Make(aName, SFX_STYLE_FAMILY_PARA, nMask));
    pSheet->SetParent(aStdName);
    pSheet->SetHelpId( aHelpFile, HID_POOLSHEET_TITLE1 );
    pISet = &pSheet->GetItemSet();

    pISet->Put(XLineStyleItem(XLINE_NONE));
    pISet->Put(XFillStyleItem(XFILL_SOLID));
    pISet->Put(XFillColorItem(String(), RGB_Color(COL_CYAN)));

    pISet->Put(SdrShadowItem(TRUE));
    pISet->Put(SdrShadowColorItem(aNullStr, RGB_Color(COL_GRAY)));
    pISet->Put(SdrShadowXDistItem(200));		// 2 mm Schattendistanz
    pISet->Put(SdrShadowYDistItem(200));

    pISet->Put(SvxFontHeightItem(846)); 		// 24 pt

    pISet->Put(SvxAdjustItem(SVX_ADJUST_CENTER));

    // ---- Titel2 --------------------------------------------------------

    aName = String(SdResId(STR_POOLSHEET_TITLE2));
    pSheet = &(pStyleSheetPool->Make(aName, SFX_STYLE_FAMILY_PARA, nMask));
    pSheet->SetParent(aStdName);
    pSheet->SetHelpId( aHelpFile, HID_POOLSHEET_TITLE2 );
    pISet = &pSheet->GetItemSet();

    pISet->Put(XLineWidthItem(50));

    // Farbe nicht aus der Farbtabelle holen, denn da kann diese Farbe
    // geloescht oder veraendert sein
    Color aOrange4(255, 204, 153);
    pISet->Put(XFillColorItem(String(), aOrange4));

    pISet->Put(SdrShadowItem(TRUE));
    pISet->Put(SdrShadowColorItem(aNullStr, RGB_Color(COL_GRAY)));
    pISet->Put(SdrShadowXDistItem(200));		// 2 mm Schattendistanz
    pISet->Put(SdrShadowYDistItem(200));

    pISet->Put(SvxFontHeightItem(1270));		// 36 pt

    SvxLRSpaceItem aLRSpItem(0, 200, 200);
    pISet->Put( aLRSpItem );	// Erstzeileneinzug 0 mm, links und rechts 2 mm
    // SvxLRSpaceItem hart gesetzt: NumBulletItem anpassen
    SvxNumBulletItem aNmBullet( (const SvxNumBulletItem&) pISet->Get(EE_PARA_NUMBULLET) );
    EditEngine::ImportBulletItem( aNmBullet, 0, NULL, &aLRSpItem );
    pISet->Put( aNmBullet );

    pISet->Put(SvxULSpaceItem(100, 100));		// Absatzrand oben/unten 1 mm

    pISet->Put(SvxAdjustItem(SVX_ADJUST_CENTER));

    // ---- Ueberschrift ---------------------------------------------------

    aName = String(SdResId(STR_POOLSHEET_HEADLINE));
    pSheet = &(pStyleSheetPool->Make(aName, SFX_STYLE_FAMILY_PARA, nMask));
    pSheet->SetParent(aStdName);
    pSheet->SetHelpId( aHelpFile, HID_POOLSHEET_HEADLINE );
    pISet = &pSheet->GetItemSet();

    pISet->Put(XLineStyleItem(XLINE_NONE));
    pISet->Put(XFillStyleItem(XFILL_NONE));

    pISet->Put(SvxFontHeightItem(846)); 		// 24 pt

    pISet->Put(SvxULSpaceItem(420, 210));		// Absatzrand oben 4,2 mm,
                                                // unten 2,1 mm

    // ---- Ueberschrift1 --------------------------------------------------

    aName = String(SdResId(STR_POOLSHEET_HEADLINE1));
    pSheet = &(pStyleSheetPool->Make(aName, SFX_STYLE_FAMILY_PARA, nMask));
    pSheet->SetParent(aStdName);
    pSheet->SetHelpId( aHelpFile, HID_POOLSHEET_HEADLINE1 );
    pISet = &pSheet->GetItemSet();

    pISet->Put(XLineStyleItem(XLINE_NONE));
    pISet->Put(XFillStyleItem(XFILL_NONE));

    pISet->Put(SvxWeightItem(WEIGHT_BOLD));

    pISet->Put(SvxFontHeightItem(635)); 		// 18 pt

    pISet->Put(SvxULSpaceItem(420, 210));		// Absatzrand oben 4,2 mm,
                                                // unten 2,1 mm

    // ---- Ueberschrift2 --------------------------------------------------

    aName = String(SdResId(STR_POOLSHEET_HEADLINE2));
    pSheet = &(pStyleSheetPool->Make(aName, SFX_STYLE_FAMILY_PARA, nMask));
    pSheet->SetParent(aStdName);
    pSheet->SetHelpId( aHelpFile, HID_POOLSHEET_HEADLINE2 );
    pISet = &pSheet->GetItemSet();

    pISet->Put(XLineStyleItem(XLINE_NONE));
    pISet->Put(XFillStyleItem(XFILL_NONE));

    pISet->Put(SvxPostureItem(ITALIC_NORMAL));
    pISet->Put(SvxWeightItem(WEIGHT_BOLD));

    pISet->Put(SvxFontHeightItem(494)); 		// 14 pt

    pISet->Put(SvxULSpaceItem(420, 210));		// Absatzrand oben 4,2 mm,
                                                // unten 2,1 mm

    // ---- Bemassung --------------------------------------------------

    aName = String(SdResId(STR_POOLSHEET_MEASURE));
    pSheet = &(pStyleSheetPool->Make(aName, SFX_STYLE_FAMILY_PARA, nMask));
    pSheet->SetParent(aStdName);
    pSheet->SetHelpId( aHelpFile, HID_POOLSHEET_MEASURE );
    pISet = &pSheet->GetItemSet();

    pISet->Put(XFillStyleItem(XFILL_NONE));

    pISet->Put(SvxFontHeightItem(423));         // 12 pt

    pISet->Put(XLineStartItem(SVX_RESSTR(RID_SVXSTR_ARROW),aArrow));
    pISet->Put(XLineEndItem(SVX_RESSTR(RID_SVXSTR_ARROW),aArrow));
    pISet->Put(XLineEndWidthItem(200));
    pISet->Put(XLineStyleItem(XLINE_SOLID));

    // Praesentationsvorlagen fuer das Standardlayout erzeugen
    String aPrefix = String(SdResId(STR_LAYOUT_DEFAULT_NAME));
    pStyleSheetPool->CreateLayoutStyleSheets(aPrefix);
}

USHORT SdDrawDocument::GetMasterPageUserCount(SdrPage* pMaster) const
{
    USHORT nResult = 0;
    USHORT nPage;
    USHORT nPageCount = GetPageCount();

    for (nPage = 0; nPage < nPageCount; nPage++)
    {
        const SdrPage* pPage = GetPage(nPage);
        for (USHORT nPos = 0; nPos < pPage->GetMasterPageCount(); nPos++)
        {
            if (pMaster == pPage->GetMasterPage(nPos))
            {
                nResult++;
            }
        }
    }
    return nResult;
}

void SdDrawDocument::RestoreLayerNames()
{
    SdrLayerAdmin& rLayerAdmin = GetLayerAdmin();
    USHORT nLayerCount = rLayerAdmin.GetLayerCount();

    if (nFileFormatVersion < 13)
    {
        USHORT nStandardLayer = 5;
        nLayerCount = Min(nLayerCount, nStandardLayer);

        for (USHORT nLayer = 0; nLayer < nLayerCount; nLayer++)
        {
            SdrLayer* pLayer = rLayerAdmin.GetLayer(nLayer);

            if (pLayer)
            {
                String aDefaultLayerName;

                if (nLayer == 0)
                {
                    aDefaultLayerName = String( RTL_CONSTASCII_USTRINGPARAM( "LAYER_LAYOUT" ));
                }
                else if (nLayer == 1)
                {
                    aDefaultLayerName = String( RTL_CONSTASCII_USTRINGPARAM( "LAYER_BCKGRND" ));
                }
                else if (nLayer == 2)
                {
                    aDefaultLayerName = String( RTL_CONSTASCII_USTRINGPARAM( "LAYER_BACKGRNDOBJ" ));
                }
                else if (nLayer == 3)
                {
                    aDefaultLayerName = String( RTL_CONSTASCII_USTRINGPARAM( "LAYER_CONTROLS" ));
                }
                else if (nLayer == 4)
                {
                    aDefaultLayerName = String( RTL_CONSTASCII_USTRINGPARAM( "LAYER_MEASURELINES" ));
                }

                if (pLayer->GetName() != aDefaultLayerName)
                {
                    pLayer->SetName(aDefaultLayerName);
                }
            }
        }
    }
}

String SdDrawDocument::CreatePageNumValue(USHORT nNum) const
{
    String aPageNumValue;
    BOOL bUpper = FALSE;

    switch (ePageNumType)
    {
         case SVX_CHARS_UPPER_LETTER:
             aPageNumValue += (sal_Unicode)(char)((nNum - 1) % 26 + 'A');
             break;
         case SVX_CHARS_LOWER_LETTER:
             aPageNumValue += (sal_Unicode)(char)((nNum - 1) % 26 + 'a');
             break;
         case SVX_ROMAN_UPPER:
             bUpper = TRUE;
         case SVX_ROMAN_LOWER:
             aPageNumValue += SvxNumberFormat::CreateRomanString(nNum, bUpper);
             break;
         case SVX_NUMBER_NONE:
             aPageNumValue.Erase();
             aPageNumValue += sal_Unicode(' ');
             break;
        default:
            aPageNumValue += String::CreateFromInt32( (sal_Int32)nNum );
    }

    return(aPageNumValue);
}

void SdDrawDocument::RenameLayoutTemplate(const String& rOldLayoutName, const String& rNewName)
{
    String aOldName(rOldLayoutName);
    USHORT nPos = aOldName.SearchAscii( SD_LT_SEPARATOR );

    // erase everything after '~LT~' 
    aOldName.Erase(nPos + sizeof(SD_LT_SEPARATOR) - 1 );
    USHORT nLen = aOldName.Len();

    List aReplList;
    SfxStyleSheetIterator aIter(pStyleSheetPool, SD_LT_FAMILY);
    SfxStyleSheetBase* pSheet = aIter.First();

    while (pSheet)
    {
        String aSheetName = pSheet->GetName();

        // if the sheetname starts with aOldName + "~LT~"
        if (aSheetName.Match(aOldName) == nLen)
        {
            aSheetName.Erase(0, nLen - sizeof(SD_LT_SEPARATOR) + 1 );
            aSheetName.Insert(rNewName, 0);

            StyleReplaceData* pReplData = new StyleReplaceData;
            pReplData->nFamily	  = pSheet->GetFamily();
            pReplData->nNewFamily = pSheet->GetFamily();
            pReplData->aName	  = pSheet->GetName();
            pReplData->aNewName   = aSheetName;
            aReplList.Insert(pReplData, LIST_APPEND);

            pSheet->SetName(aSheetName);
        }

        pSheet = aIter.Next();
    }

    // jetzt noch den Layoutnamen der Zeichen- und der Notizseite
    // sowie ihrer Masterpages setzen
    String aPageLayoutName(rNewName);
    aPageLayoutName.AppendAscii( RTL_CONSTASCII_STRINGPARAM( SD_LT_SEPARATOR ));
    aPageLayoutName += String(SdResId(STR_LAYOUT_OUTLINE));

    // an allen Seiten, die das jetzt umbeannte Layout benutzen, die
    // Textobjekte von der Aenderung unterrichten und Layoutnamen setzen
    USHORT nPage;
    for (nPage = 0; nPage < GetPageCount(); nPage++)
    {
        SdPage* pPage = (SdPage*) GetPage(nPage);
        String aTemp(pPage->GetLayoutName());

        if (aTemp == rOldLayoutName)
        {
            pPage->SetLayoutName(aPageLayoutName);

            for (ULONG nObj = 0; nObj < pPage->GetObjCount(); nObj++)
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
                            OutlinerParaObject* pOPO = ((SdrTextObj*) pObj)->GetOutlinerParaObject();

                            if (pOPO)
                            {
                                StyleReplaceData* pReplData = (StyleReplaceData*) aReplList.First();

                                while( pReplData )
                                {
                                    pOPO->ChangeStyleSheets( pReplData->aName, pReplData->nFamily, pReplData->aNewName, pReplData->nNewFamily );
                                    pReplData = (StyleReplaceData*) aReplList.Next();
                                }
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

    // und nochmal fuer die Masterpages
    // die betroffenen Masterpages erhalten als Seitennamen den Namen
    // des Layouts
    for (nPage = 0; nPage < GetMasterPageCount(); nPage++)
    {
        SdPage* pPage = (SdPage*) GetMasterPage(nPage);
        String aTemp(pPage->GetLayoutName());

        if (aTemp == rOldLayoutName)
        {
            pPage->SetLayoutName(aPageLayoutName);
            pPage->SetName(rNewName);

            for (ULONG nObj = 0; nObj < pPage->GetObjCount(); nObj++)
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
                            OutlinerParaObject* pOPO = ((SdrTextObj*)pObj)->GetOutlinerParaObject();

                            if (pOPO)
                            {
                                StyleReplaceData* pReplData = (StyleReplaceData*) aReplList.First();

                                while( pReplData )
                                {
                                    pOPO->ChangeStyleSheets( pReplData->aName, pReplData->nFamily, pReplData->aNewName, pReplData->nNewFamily );
                                    pReplData = (StyleReplaceData*) aReplList.Next();
                                }
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

void SdDrawDocument::SetTextDefaults() const
{
    // BulletItem und BulletFont fuer Titel und Gliederung
    SvxBulletItem aBulletItem(EE_PARA_BULLET);
    Font aBulletFont( ((SdStyleSheetPool*) pStyleSheetPool)->GetBulletFont() );
    aBulletFont.SetSize(Size(0,846));		// 24 pt
    aBulletItem.SetFont(aBulletFont);
    aBulletItem.SetStyle(BS_BULLET);
    aBulletItem.SetStart(1);
    aBulletItem.SetScale(45);				// in Prozent
    aBulletItem.SetSymbol( 0x25CF );				// Punkt
    pItemPool->SetPoolDefaultItem( aBulletItem );

    // Bullets nicht sichtbar
    SfxUInt16Item aBulletStateItem(EE_PARA_BULLETSTATE, 0);
    pItemPool->SetPoolDefaultItem( aBulletStateItem );

    // Neues BulletItem
    SvxNumberFormat aNumberFormat(SVX_NUM_CHAR_SPECIAL);
    aNumberFormat.SetBulletFont(&aBulletFont);
    aNumberFormat.SetBulletChar( 0x25CF );  // StarBats: 0xF000 + 34
    aNumberFormat.SetBulletRelSize(45);
    aNumberFormat.SetBulletColor(Color(COL_AUTO));
    aNumberFormat.SetStart(1);
    aNumberFormat.SetNumAdjust(SVX_ADJUST_LEFT);

    SvxNumRule aNumRule( NUM_BULLET_REL_SIZE|NUM_BULLET_COLOR|NUM_CHAR_TEXT_DISTANCE, 10 , FALSE);

    aNumberFormat.SetLSpace( 0 );
    aNumberFormat.SetAbsLSpace( 0 );
    aNumberFormat.SetFirstLineOffset( 0 );
    aNumRule.SetLevel( 0, aNumberFormat );

    for( USHORT i = 1; i < 10; i++ )
    {
        const short nLSpace = (i + 1) * 600;
        aNumberFormat.SetLSpace(nLSpace);
        aNumberFormat.SetAbsLSpace(nLSpace);
        aNumberFormat.SetFirstLineOffset(-600);
        aNumRule.SetLevel( i, aNumberFormat );
    }

    SvxNumBulletItem aNumBulletItem( aNumRule, EE_PARA_NUMBULLET );
    pItemPool->SetPoolDefaultItem( aNumBulletItem );
}

::com::sun::star::text::WritingMode SdDrawDocument::GetDefaultWritingMode() const
{
    const SfxPoolItem*                  pItem = ( pItemPool ? pItemPool->GetPoolDefaultItem( EE_PARA_WRITINGDIR ) : NULL );
    ::com::sun::star::text::WritingMode eRet = ::com::sun::star::text::WritingMode_LR_TB;
    
    if( pItem )
    {
        switch( ( (SvxFrameDirectionItem&)( *pItem ) ).GetValue() )
        {
            case( FRMDIR_HORI_LEFT_TOP ): eRet = ::com::sun::star::text::WritingMode_LR_TB; break;
            case( FRMDIR_HORI_RIGHT_TOP ): eRet = ::com::sun::star::text::WritingMode_RL_TB; break;
            case( FRMDIR_VERT_TOP_RIGHT ): eRet = ::com::sun::star::text::WritingMode_TB_RL; break;
            default:
            break;
        }
    }

    return eRet;
}


void SdDrawDocument::getDefaultFonts( Font& rLatinFont, Font& rCJKFont, Font& rCTLFont )
{
    LanguageType eLatin = GetLanguage( EE_CHAR_LANGUAGE );

    LanguageType eUiLanguage = Application::GetSettings().GetUILanguage();
    switch( eUiLanguage )
    {
        case LANGUAGE_KOREAN:
        case LANGUAGE_KOREAN_JOHAB:
            eLatin = eUiLanguage;
        break;
    }

    rLatinFont = OutputDevice::GetDefaultFont( DEFAULTFONT_LATIN_PRESENTATION, eLatin, DEFAULTFONT_FLAGS_ONLYONE );
    rCJKFont = OutputDevice::GetDefaultFont( DEFAULTFONT_CJK_PRESENTATION, GetLanguage( EE_CHAR_LANGUAGE_CJK ), DEFAULTFONT_FLAGS_ONLYONE );
    rCTLFont = OutputDevice::GetDefaultFont( DEFAULTFONT_CTL_PRESENTATION, GetLanguage( EE_CHAR_LANGUAGE_CTL ), DEFAULTFONT_FLAGS_ONLYONE ) ;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

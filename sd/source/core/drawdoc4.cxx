/*************************************************************************
 *
 *  $RCSfile: drawdoc4.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: dl $ $Date: 2001-03-21 16:24:40 $
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

#ifndef SVX_LIGHT
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _OFF_APP_HXX //autogen
#include <offmgr/app.hxx>
#endif
#ifndef _OSPLCFG_HXX
#include <offmgr/osplcfg.hxx>
#endif
#include "sdoutl.hxx"
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif#else
#ifndef _OUTLINER_HXX //autogen wg. Outliner
#include <svx/outliner.hxx>
#endif
#endif // !SVX_LIGHT

#ifndef _EEITEM_HXX //autogen
#include <svx/eeitem.hxx>
#endif

#include <eetext.hxx>

#define ITEMID_SEARCH           SID_SEARCH_ITEM
#include <svx/svxids.hrc>
#include <svx/srchitem.hxx>
#include <svx/lrspitem.hxx>
#include <svx/ulspitem.hxx>
#include <svx/lspcitem.hxx>
#include <svx/adjitem.hxx>
#include <svx/dialogs.hrc>
#include <svx/dialmgr.hxx>                  // SVX_RESSTR
#include <svx/bulitem.hxx>
#include <svx/xtable.hxx>
#ifndef _SVX_XIT_HXX //autogen
#include <svx/xit.hxx>
#endif
#ifndef _SVX_XLINEIT0_HXX //autogen
#include <svx/xlineit0.hxx>
#endif
#ifndef _SDSHITM_HXX //autogen
#include <svx/sdshitm.hxx>
#endif
#ifndef _SVDOTEXT_HXX //autogen
#include <svx/svdotext.hxx>
#endif
#ifndef SVX_XFILLIT0_HXX //autogen
#include <svx/xfillit0.hxx>
#endif
#ifndef _SDSHCITM_HXX //autogen
#include <svx/sdshcitm.hxx>
#endif
#ifndef _EDITSTAT_HXX //autogen
#include <svx/editstat.hxx>
#endif
#ifndef _SVX_COLRITEM_HXX //autogen
#include <svx/colritem.hxx>
#endif
#ifndef _SVX_FHGTITEM_HXX //autogen
#include <svx/fhgtitem.hxx>
#endif
#ifndef _SVX_WGHTITEM_HXX //autogen
#include <svx/wghtitem.hxx>
#endif
#ifndef _SVX_POSTITEM_HXX //autogen
#include <svx/postitem.hxx>
#endif
#ifndef _SVX_CRSDITEM_HXX //autogen
#include <svx/crsditem.hxx>
#endif
#ifndef _SVX_UDLNITEM_HXX //autogen
#include <svx/udlnitem.hxx>
#endif
#ifndef _SVX_ITEM_HXX //autogen
#include <svx/cntritem.hxx>
#endif
#ifndef _SVX_FONTITEM_HXX //autogen
#include <svx/fontitem.hxx>
#endif
#ifndef _SVX_SHDDITEM_HXX //autogen
#include <svx/shdditem.hxx>
#endif
#ifndef _SVX_XBTMPIT_HXX //autogen
#include <svx/xbtmpit.hxx>
#endif
#ifndef _SVX_XFLHTIT_HXX //autogen
#include <svx/xflhtit.hxx>
#endif
#ifndef _SVX_XFLGRIT_HXX //autogen
#include <svx/xflgrit.hxx>
#endif
#ifndef _SVX_XFLCLIT_HXX //autogen
#include <svx/xflclit.hxx>
#endif
#ifndef _SVX_XLNEDCIT_HXX //autogen
#include <svx/xlnedcit.hxx>
#endif
#ifndef _SVX_XLNSTCIT_HXX //autogen
#include <svx/xlnstcit.hxx>
#endif
#ifndef _SVX_XLNEDWIT_HXX //autogen
#include <svx/xlnedwit.hxx>
#endif
#ifndef _SVX_XLNSTWIT_HXX //autogen
#include <svx/xlnstwit.hxx>
#endif
#ifndef _SVX_XLNEDIT_HXX //autogen
#include <svx/xlnedit.hxx>
#endif
#ifndef _SVX_XLNSTIT_HXX //autogen
#include <svx/xlnstit.hxx>
#endif
#ifndef _SVX_XLNDSIT_HXX //autogen
#include <svx/xlndsit.hxx>
#endif
#ifndef _SVX_XLNWTIT_HXX //autogen
#include <svx/xlnwtit.hxx>
#endif
#ifndef _SVX_XLNCLIT_HXX //autogen
#include <svx/xlnclit.hxx>
#endif
#ifndef _SVDITER_HXX //autogen
#include <svx/svditer.hxx>
#endif
#ifndef _SVDOGRP_HXX //autogen
#include <svx/svdogrp.hxx>
#endif
#ifndef _SHL_HXX //autogen
#include <tools/shl.hxx>
#endif
#ifndef _SVX_NUMITEM_HXX
#include <svx/numitem.hxx>
#endif
#ifndef _MyEDITENG_HXX //autogen
#include <svx/editeng.hxx>
#endif
#ifndef _UNO_LINGU_HXX
#include <svx/unolingu.hxx>
#endif
#ifndef _COM_SUN_STAR_LINGUISTIC2_XHYPHENATOR_HPP_
#include <com/sun/star/linguistic2/XHyphenator.hpp>
#endif
#ifndef _COM_SUN_STAR_LINGUISTIC2_XLINGUSERVICEMANAGER_HPP_
#include <com/sun/star/linguistic2/XLinguServiceManager.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _SFXITEMPOOL_HXX
#include <svtools/itempool.hxx>
#endif
#ifndef _OUTLOBJ_HXX
#include <svx/outlobj.hxx>
#endif
#ifndef _SFXVIEWFRM_HXX
#include <sfx2/viewfrm.hxx>
#endif
#define ITEMID_LANGUAGE           EE_CHAR_LANGUAGE
#ifndef _SVX_LANGITEM_HXX
#include <svx/langitem.hxx>
#endif

#include "sdresid.hxx"
#include "drawdoc.hxx"
#include "sdpage.hxx"
#include "glob.hrc"
#include "glob.hxx"
#include "stlpool.hxx"
#include "helpids.h"
#include "sdiocmpt.hxx"

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::linguistic2;

/*************************************************************************
|*
|* CreateLayoutTemplates, Layoutvorlagen erzeugen
|*
|* Z.Zt. (31.03.95) speichert der StyleSheetPool nur diejenigen Sheets, die
|* ein ItemSet haben. Damit alle Sheets gespeichert werden, wird die ItemSet-
|* Erzeugung mit einem GetItemSet-Aufruf erzwungen.
|* Dies kann entfallen, sobald der Pool auch Sheets ohne ItemSet speichert.
|*
\************************************************************************/

#ifndef SVX_LIGHT
void SdDrawDocument::CreateLayoutTemplates()
{
    SdStyleSheetPool* pStyleSheetPool = (SdStyleSheetPool*)GetStyleSheetPool();
    SfxStyleSheetBase* pSheet = NULL;
    String aHelpFile;
    String aStdName = String(SdResId(STR_STANDARD_STYLESHEET_NAME));

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

    String   aNullStr;

    Bitmap   aNullBmp(SdResId(BMP_PRESOBJ_OBJECT));   // irgendeine

    XPolygon aNullPol;
    Color    aNullCol(RGB_Color(COL_BLACK));

    XDash     aNullDash;
    XGradient aNullGrad(aNullCol,RGB_Color(COL_WHITE));
              aNullGrad.SetStartIntens( 100 );
              aNullGrad.SetEndIntens( 100 );
    XHatch    aNullHatch(aNullCol);

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
    rISet.Put(XFillBitmapItem(pPool,aNullBmp));

                    // Schattenattribute (Drawing Engine)
    rISet.Put(SdrShadowItem(FALSE));
    rISet.Put(SdrShadowColorItem(aNullStr, RGB_Color(COL_GRAY)));
    rISet.Put(SdrShadowXDistItem(300));         // 3 mm Schattendistanz
    rISet.Put(SdrShadowYDistItem(300));

                    // Zeichenattribute (Edit Engine)
    SvxFontItem aSvxFontItem;
    SvxFontItem aSvxFontItemCJK( EE_CHAR_FONTINFO_CJK );
    SvxFontItem aSvxFontItemCTL( EE_CHAR_FONTINFO_CTL );
    GetDefaultFonts( aSvxFontItem, aSvxFontItemCJK, aSvxFontItemCTL );
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
    rISet.Put(SvxColorItem(RGB_Color(COL_BLACK)));

                    // Absatzattribute (Edit Engine)
    rISet.Put(SvxLRSpaceItem());
    rISet.Put(SvxULSpaceItem());
    rISet.Put(SvxAdjustItem());
    rISet.Put(SvxLineSpacingItem());

                    // Bullet
    // BulletItem und BulletFont fuer Titel und Gliederung
    SvxBulletItem aBulletItem(EE_PARA_BULLET);
                            // die sind in allen Ebenen identisch
    aBulletItem.SetStyle(BS_BULLET);
    aBulletItem.SetStart(1);
    aBulletItem.SetScale(45);           // in Prozent

    Font aBulletFont( pStyleSheetPool->GetBulletFont() );
    aBulletFont.SetSize(Size(0,846));       // 24 pt
    aBulletItem.SetFont(aBulletFont);
    aBulletItem.SetSymbol(34);                  // Punkt
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
    aArrow[0]=Point(100,0);                        // 0,4__[]__2,4
    aArrow[1]=Point(200,400);                      //    \    /
    aArrow[2]=Point(0,400);                        //     \  /
    aArrow[3]=Point(100,0);                        //      \/1,0
    pISet->Put(XLineStartItem(SVX_RESSTR(RID_SVXSTR_ARROW),aArrow));

    pISet->Put(XLineStartWidthItem(700));
    pISet->Put(XLineEndWidthItem(300));         // wollte Kohse das wirklich?
    pISet->Put(XLineStartCenterItem(TRUE));

    // ---- Objekt mit Schatten -------------------------------------------

    aName = String(SdResId(STR_POOLSHEET_OBJWITHSHADOW));
    pSheet = &(pStyleSheetPool->Make(aName, SFX_STYLE_FAMILY_PARA, nMask));
    pSheet->SetParent(aStdName);
    pSheet->SetHelpId( aHelpFile, HID_POOLSHEET_OBJWITHSHADOW );
    pISet = &pSheet->GetItemSet();

    pISet->Put(SdrShadowItem(TRUE));
    pISet->Put(SdrShadowColorItem(aNullStr, RGB_Color(COL_GRAY)));
    pISet->Put(SdrShadowXDistItem(300));        // 3 mm Schattendistanz
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

    pISet->Put(SvxFontHeightItem(564));         // 16 pt

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
    aLRSpaceItem.SetTxtFirstLineOfst(600);      // Erstzeileneinzug 6mm, rechts 0
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

    pISet->Put(SvxFontHeightItem(1551));        // 44 pt

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
    pISet->Put(SdrShadowXDistItem(200));        // 2 mm Schattendistanz
    pISet->Put(SdrShadowYDistItem(200));

    pISet->Put(SvxFontHeightItem(846));         // 24 pt

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
    pISet->Put(SdrShadowXDistItem(200));        // 2 mm Schattendistanz
    pISet->Put(SdrShadowYDistItem(200));

    pISet->Put(SvxFontHeightItem(1270));        // 36 pt

    SvxLRSpaceItem aLRSpItem(0, 200, 200);
    pISet->Put( aLRSpItem );    // Erstzeileneinzug 0 mm, links und rechts 2 mm
    // SvxLRSpaceItem hart gesetzt: NumBulletItem anpassen
    SvxNumBulletItem aNmBullet( (const SvxNumBulletItem&) pISet->Get(EE_PARA_NUMBULLET) );
    EditEngine::ImportBulletItem( aNmBullet, 0, NULL, &aLRSpItem );
    pISet->Put( aNmBullet );

    pISet->Put(SvxULSpaceItem(100, 100));       // Absatzrand oben/unten 1 mm

    pISet->Put(SvxAdjustItem(SVX_ADJUST_CENTER));

    // ---- Ueberschrift ---------------------------------------------------

    aName = String(SdResId(STR_POOLSHEET_HEADLINE));
    pSheet = &(pStyleSheetPool->Make(aName, SFX_STYLE_FAMILY_PARA, nMask));
    pSheet->SetParent(aStdName);
    pSheet->SetHelpId( aHelpFile, HID_POOLSHEET_HEADLINE );
    pISet = &pSheet->GetItemSet();

    pISet->Put(XLineStyleItem(XLINE_NONE));
    pISet->Put(XFillStyleItem(XFILL_NONE));

    pISet->Put(SvxFontHeightItem(846));         // 24 pt

    pISet->Put(SvxULSpaceItem(420, 210));       // Absatzrand oben 4,2 mm,
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

    pISet->Put(SvxFontHeightItem(635));         // 18 pt

    pISet->Put(SvxULSpaceItem(420, 210));       // Absatzrand oben 4,2 mm,
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

    pISet->Put(SvxFontHeightItem(494));         // 14 pt

    pISet->Put(SvxULSpaceItem(420, 210));       // Absatzrand oben 4,2 mm,
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
    pISet->Put(XLineStartWidthItem(200));
    pISet->Put(XLineEndItem(SVX_RESSTR(RID_SVXSTR_ARROW),aArrow));
    pISet->Put(XLineEndWidthItem(200));
    pISet->Put(XLineStyleItem(XLINE_SOLID));

/*

    invalidierte Items koennen nicht gespeichert werden;
    da muessen wir uns was anderes ueberlegen

    // ---- leere Vorlage --------------------------------------------------

    aName = String(SdResId(STR_EMPTY_STYLESHEET_NAME));
    pSheet = &(pStyleSheetPool->Make(aName, SFX_STYLE_FAMILY_PARA, nMask));
    pISet = &pSheet->GetItemSet();
    pISet->InvalidateAllItems();      // alle auf DONTCARE setzen
*/


    // Praesentationsvorlagen fuer das Standardlayout erzeugen
    String aPrefix = String(SdResId(STR_LAYOUT_DEFAULT_NAME));
    pStyleSheetPool->CreateLayoutStyleSheets(aPrefix);
}
#endif // !SVX_LIGHT


/*************************************************************************
|*
|* Anzahl der Seiten, die eine masterPage referenzieren
|*
\************************************************************************/

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


/*************************************************************************
|*
|* OnlineSpelling im Hintergrund beenden
|*
\************************************************************************/

#ifndef SVX_LIGHT
void SdDrawDocument::StopOnlineSpelling()
{
    if (pOnlineSpellingTimer && pOnlineSpellingTimer->IsActive())
    {
        pOnlineSpellingTimer->Stop();
    }

    delete pOnlineSpellingTimer;
    pOnlineSpellingTimer = NULL;

    delete pOnlineSpellingList;
    pOnlineSpellingList = NULL;
}
#endif // !SVX_LIGHT


/*************************************************************************
|*
|* OnlineSpelling im Hintergrund starten
|*
\************************************************************************/

#ifndef SVX_LIGHT
void SdDrawDocument::StartOnlineSpelling(BOOL bForceSpelling)
{
    if (bOnlineSpell && (bForceSpelling || bInitialOnlineSpellingEnabled))
    {
        StopOnlineSpelling();

        SdOutliner* pOutl = GetInternalOutliner(TRUE);

        Reference< XMultiServiceFactory > xMgr( ::comphelper::getProcessServiceFactory() );
        Reference< XLinguServiceManager > xLinguServiceManager( xMgr->createInstance(
            OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.linguistic2.LinguServiceManager" ))),
                                                            uno::UNO_QUERY );

        if ( xLinguServiceManager.is() )
        {
            Reference< XSpellChecker1 > xSpellChecker( xLinguServiceManager->getSpellChecker(), UNO_QUERY );
            if ( xSpellChecker.is() )
                pOutl->SetSpeller( xSpellChecker );

            Reference< XHyphenator > xHyphenator( xLinguServiceManager->getHyphenator(), UNO_QUERY );
            if( xHyphenator.is() )
                pOutl->SetHyphenator( xHyphenator );
        }

        pOutl->SetDefaultLanguage( eLanguage );

        SdPage* pPage = NULL;
        SdrObject* pObj = NULL;
        pOnlineSpellingList = new List(64, 32);

        // Um im OnlineSpellingHdl mit List::Next() arbeiten zu  koennen,
        // wird ein Position 0 ein Dummy-Objekt (NULL-Pointer) eingefuegt
        pOnlineSpellingList->Insert(NULL, LIST_APPEND);

        for (USHORT nPage = 0; nPage < GetPageCount(); nPage++)
        {
            // Alle Pages durchsuchen
            FillOnlineSpellingList((SdPage*) GetPage(nPage));
        }

        for (nPage = 0; nPage < GetMasterPageCount(); nPage++)
        {
            // Alle MasterPages durchsuchen
            FillOnlineSpellingList((SdPage*) GetMasterPage(nPage));
        }

        pOnlineSpellingList->Seek(ULONG(0));
        pOnlineSpellingTimer = new Timer();
        pOnlineSpellingTimer->SetTimeoutHdl( LINK(this, SdDrawDocument, OnlineSpellingHdl) );
        pOnlineSpellingTimer->SetTimeout(250);
        pOnlineSpellingTimer->Start();
    }
}
#endif // !SVX_LIGHT


/*************************************************************************
|*
|* OnlineSpelling-Liste fuellen
|*
\************************************************************************/

#ifndef SVX_LIGHT
void SdDrawDocument::FillOnlineSpellingList(SdPage* pPage)
{
    SdrObject* pObj = NULL;
    SdrObjListIter aIter(*pPage, IM_FLAT);

    while (aIter.IsMore())
    {
        pObj = aIter.Next();

        if (pObj->GetOutlinerParaObject())
        {
            // Textobjekt gefunden
            pOnlineSpellingList->Insert(pObj, LIST_APPEND);
        }
        else if (pObj->GetObjIdentifier() == OBJ_GRUP)
        {
            // Gruppenobjekt gefunden
            SdrObjListIter aGroupIter(*((SdrObjGroup*)pObj)->GetSubList(),
                                      IM_DEEPNOGROUPS);

            BOOL bSubTextObjFound = FALSE;

            while (aGroupIter.IsMore() && !bSubTextObjFound)
            {
                if (aGroupIter.Next()->GetOutlinerParaObject())
                {
                    // Textobjekt im Gruppenobjekt gefunden
                    bSubTextObjFound = TRUE;
                }
            }

            if (bSubTextObjFound)
            {
                pOnlineSpellingList->Insert(pObj, LIST_APPEND);
            }
        }
    }
}
#endif // !SVX_LIGHT


/*************************************************************************
|*
|* OnlineSpelling im Hintergrund
|*
\************************************************************************/

#ifndef SVX_LIGHT
IMPL_LINK(SdDrawDocument, OnlineSpellingHdl, Timer*, pTimer)
{
    if (!pOnlineSpellingList || !bOnlineSpell ||
        pOnlineSpellingList->GetCurPos() < pOnlineSpellingList->Count() + 1)
    {
        /**********************************************************************
        * Naechstes Objekt spellen
        **********************************************************************/
        SdrObject* pObj = (SdrObject*) pOnlineSpellingList->Next();

        if (pObj)
        {
            if (pObj->GetOutlinerParaObject() && pObj->ISA(SdrTextObj))
            {
                // Textobjekt spellen
                SpellObject((SdrTextObj*) pObj);
            }
            else if (pObj->GetObjIdentifier() == OBJ_GRUP)
            {
                // Gruppenobjekt gefunden
                SdrObjListIter aGroupIter(*((SdrObjGroup*)pObj)->GetSubList(),
                                          IM_DEEPNOGROUPS);

                SdrObject* pSubObj = NULL;

                while (aGroupIter.IsMore())
                {
                    pSubObj = aGroupIter.Next();

                    if (pSubObj->GetOutlinerParaObject() && pSubObj->ISA(SdrTextObj))
                    {
                        // Textobjekt im Gruppenobjekt gefunden
                        SpellObject((SdrTextObj*) pSubObj);
                    }
                }
            }
        }

        // Weitersuchen
        pOnlineSpellingTimer->Start();
    }
    else
    {
        // Spelling wurde initial durchgefuehrt
        bInitialOnlineSpellingEnabled = FALSE;

        // Suche beenden
        StopOnlineSpelling();

        delete pOnlineSearchItem;
        pOnlineSearchItem = NULL;
    }

    return(0);
}
#endif // !SVX_LIGHT


/*************************************************************************
|*
|* Objekt spellen (fuer OnlineSpelling)
|*
\************************************************************************/

#ifndef SVX_LIGHT
void SdDrawDocument::SpellObject(SdrTextObj* pObj)
{
    if (pObj && pObj->GetOutlinerParaObject() /* && pObj != pView->GetTextEditObject() */)
    {
        bHasOnlineSpellErrors = FALSE;
        SdOutliner* pOutl = GetInternalOutliner(TRUE);
        pOutl->SetUpdateMode(TRUE);
        Link aEvtHdl = pOutl->GetStatusEventHdl();
        pOutl->SetStatusEventHdl(LINK(this, SdDrawDocument, OnlineSpellEventHdl));

        USHORT nOldOutlMode = pOutl->GetMode();
        USHORT nOutlMode = OUTLINERMODE_TEXTOBJECT;
        if (((SdrTextObj*) pObj)->GetObjInventor() == SdrInventor &&
            ((SdrTextObj*) pObj)->GetObjIdentifier() == OBJ_OUTLINETEXT)
        {
            nOutlMode = OUTLINERMODE_OUTLINEOBJECT;
            pOutl->SetMinDepth(1);
        }
        pOutl->Init( nOutlMode );

        // Text in den Outliner setzen
        pOutl->SetText(*((SdrTextObj*) pObj)->GetOutlinerParaObject());

        if (!pOnlineSearchItem || pOutl->HasText(*pOnlineSearchItem))
        {
            // Spelling
            pOutl->CompleteOnlineSpelling();

            if (bHasOnlineSpellErrors)
            {
                // Text aus Outliner holen
                BOOL bModified = IsChanged();
                ((SdrTextObj*) pObj)->SetOutlinerParaObject( pOutl->CreateParaObject() );
                SetChanged(bModified);
                pObj->SendRepaintBroadcast();
            }
        }

        pOutl->SetStatusEventHdl(aEvtHdl);
        pOutl->SetUpdateMode(FALSE);
        pOutl->Init( nOldOutlMode );
        bHasOnlineSpellErrors = FALSE;
    }
}
#endif // !SVX_LIGHT


/*************************************************************************
|*
|* Objekt wurde ins Model eingefuegt
|*
\************************************************************************/

#ifndef SVX_LIGHT
void SdDrawDocument::InsertObject(SdrObject* pObj, SdPage* pPage)
{
    if (pOnlineSpellingList)
    {
        if (pObj->GetOutlinerParaObject())
        {
            // Objekt in OnlineSpelling-Liste aufnehmen
            pOnlineSpellingList->Insert(pObj, LIST_APPEND);
        }
    }
}
#endif // !SVX_LIGHT



/*************************************************************************
|*
|* Objekt wurde aus dem Model entfernt
|*
\************************************************************************/

#ifndef SVX_LIGHT
void SdDrawDocument::RemoveObject(SdrObject* pObj, SdPage* pPage)
{
    if (pOnlineSpellingList)
    {
        if (pObj->GetOutlinerParaObject())
        {
            // Objekt in OnlineSpelling-Liste durch NULL-Pointer ersetzt
            pOnlineSpellingList->Replace(NULL, pObj);
        }
    }
}
#endif // !SVX_LIGHT



/*************************************************************************
|*
|* Callback fuer ExecuteSpellPopup()
|*
\************************************************************************/

#ifndef SVX_LIGHT
IMPL_LINK(SdDrawDocument, OnlineSpellEventHdl, EditStatus*, pEditStat)
{
    ULONG nStat = pEditStat->GetStatusWord();
    bHasOnlineSpellErrors = (nStat & EE_STAT_WRONGWORDCHANGED) != 0;

    return(0);
}
#endif // !SVX_LIGHT


/*************************************************************************
|*
|* Callback fuer ExecuteSpellPopup()
|*
\************************************************************************/

#ifndef SVX_LIGHT
IMPL_LINK(SdDrawDocument, OnlineSpellCallback, SpellCallbackInfo*, pInfo)
{
    delete pOnlineSearchItem;
    pOnlineSearchItem = NULL;

    USHORT nCommand = pInfo->nCommand;

    if (nCommand == SPELLCMD_IGNOREWORD)
    {
        pOnlineSearchItem = new SvxSearchItem();
        pOnlineSearchItem->SetSearchString(pInfo->aWord);
        StartOnlineSpelling();
    }
    else if (nCommand == SPELLCMD_STARTSPELLDLG)
    {
        SfxViewFrame::Current()->GetDispatcher()->Execute( SID_SPELLING, SFX_CALLMODE_ASYNCHRON );
    }

    return(0);
}
#endif // !SVX_LIGHT


/*************************************************************************
|*
|* Sprachabhaengige Namen der StandardLayer durch eindeutigen Namen ersetzen
|*
\************************************************************************/

#ifndef SVX_LIGHT
void SdDrawDocument::MakeUniqueLayerNames()
{
    String aLayerLayout(SdResId(STR_LAYER_LAYOUT));
    String aLayerBckgrnd(SdResId(STR_LAYER_BCKGRND));
    String aLayerBckgrndObj(SdResId(STR_LAYER_BCKGRNDOBJ));
    String aLayerControls(SdResId(STR_LAYER_CONTROLS));
    String aLayerMeasurelines(SdResId(STR_LAYER_MEASURELINES));
    SdrLayerAdmin& rLayerAdmin = GetLayerAdmin();
    USHORT nStandardLayer = 5;
    USHORT nLayerCount = Min(rLayerAdmin.GetLayerCount(), nStandardLayer);

    for (USHORT nLayer = 0; nLayer < nLayerCount; nLayer++)
    {
        // Die sprachabhaengigen Namen der Default-Layer werden nicht mehr
        // gespeichert. Es werden stattdessen eindeutige Namen verwendet.
        SdrLayer* pLayer = rLayerAdmin.GetLayer(nLayer);

        if (pLayer)
        {
            String aLayerName(pLayer->GetName());

            if (aLayerName == aLayerLayout)
            {
                pLayer->SetName( String( RTL_CONSTASCII_USTRINGPARAM( "LAYER_LAYOUT" )));
            }
            else if (aLayerName == aLayerBckgrnd)
            {
                pLayer->SetName( String( RTL_CONSTASCII_USTRINGPARAM( "LAYER_BCKGRND" )));
            }
            else if (aLayerName == aLayerBckgrndObj)
            {
                pLayer->SetName( String( RTL_CONSTASCII_USTRINGPARAM( "LAYER_BACKGRNDOBJ" )));
            }
            else if (aLayerName == aLayerControls)
            {
                pLayer->SetName( String( RTL_CONSTASCII_USTRINGPARAM( "LAYER_CONTROLS" )));
            }
            else if (aLayerName == aLayerMeasurelines)
            {
                pLayer->SetName( String( RTL_CONSTASCII_USTRINGPARAM( "LAYER_MEASURELINES" )));
            }
        }
    }
}
#endif // !SVX_LIGHT



/*************************************************************************
|*
|* Eindeutige Namen der StandardLayer durch sprachabhaengige Namen ersetzen
|*
\************************************************************************/

#ifndef SVX_LIGHT
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
                    aDefaultLayerName = String(SdResId(STR_LAYER_LAYOUT));
                }
                else if (nLayer == 1)
                {
                    aDefaultLayerName = String(SdResId(STR_LAYER_BCKGRND));
                }
                else if (nLayer == 2)
                {
                    aDefaultLayerName = String(SdResId(STR_LAYER_BCKGRNDOBJ));
                }
                else if (nLayer == 3)
                {
                    aDefaultLayerName = String(SdResId(STR_LAYER_CONTROLS));
                }
                else if (nLayer == 4)
                {
                    aDefaultLayerName = String(SdResId(STR_LAYER_MEASURELINES));
                }

                if (pLayer->GetName() != aDefaultLayerName)
                {
                    pLayer->SetName(aDefaultLayerName);
                }
            }
        }
    }
    else
    {
        for (USHORT nLayer = 0; nLayer < nLayerCount; nLayer++)
        {
            SdrLayer* pLayer = rLayerAdmin.GetLayer(nLayer);

            if (pLayer)
            {
                String aLayerName(pLayer->GetName());

                if (aLayerName.EqualsAscii( "LAYER_LAYOUT" ))
                {
                    pLayer->SetName(String(SdResId(STR_LAYER_LAYOUT)));
                }
                else if (aLayerName.EqualsAscii( "LAYER_BCKGRND" ))
                {
                    pLayer->SetName(String(SdResId(STR_LAYER_BCKGRND)));
                }
                else if (aLayerName.EqualsAscii( "LAYER_BACKGRNDOBJ" ))
                {
                    pLayer->SetName(String(SdResId(STR_LAYER_BCKGRNDOBJ)));
                }
                else if (aLayerName.EqualsAscii( "LAYER_CONTROLS" ))
                {
                    pLayer->SetName(String(SdResId(STR_LAYER_CONTROLS)));
                }
                else if (aLayerName.EqualsAscii( "LAYER_MEASURELINES" ))
                {
                    pLayer->SetName(String(SdResId(STR_LAYER_MEASURELINES)));
                }
            }
        }
    }
}
#endif // !SVX_LIGHT


/*************************************************************************
|*
|* Formatierte Seitennummer zurueckgeben (1, I, i, a, usw.)
|*
\************************************************************************/

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



/*************************************************************************
|*
|* Layout-Template umbenennen
|* Zu beachten ist, das rOldLayoutName im Gegensatz zu rNewName den
|* kompletten Layout(!)-Namen enthaelt (inkl. ~LT~)!
|*
\************************************************************************/

void SdDrawDocument::RenameLayoutTemplate(const String& rOldLayoutName, const String& rNewName)
{
    String aOldName(rOldLayoutName);
    USHORT nPos = aOldName.SearchAscii( SD_LT_SEPARATOR );
    aOldName.Erase(nPos);
    USHORT nLen = aOldName.Len();

    List aReplList;
    SfxStyleSheetIterator aIter(pStyleSheetPool, SD_LT_FAMILY);
    SfxStyleSheetBase* pSheet = aIter.First();

    while (pSheet)
    {
        String aSheetName = pSheet->GetName();

        // wenn aSheetName mit aOldName beginnt
        if (aSheetName.Match(aOldName) == nLen)
        {
            aSheetName.Erase(0, nLen);
            aSheetName.Insert(rNewName, 0);

            StyleReplaceData* pReplData = new StyleReplaceData;
            pReplData->nFamily    = pSheet->GetFamily();
            pReplData->nNewFamily = pSheet->GetFamily();
            pReplData->aName      = pSheet->GetName();
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
        pPage->SetName(rNewName);
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

/*************************************************************************
|*
|* Outliner-Defaults setzen (Pool-Defaults)
|*
\************************************************************************/
void SdDrawDocument::SetTextDefaults() const
{
    // BulletItem und BulletFont fuer Titel und Gliederung
    SvxBulletItem aBulletItem(EE_PARA_BULLET);
    Font aBulletFont( ((SdStyleSheetPool*) pStyleSheetPool)->GetBulletFont() );
    aBulletFont.SetSize(Size(0,846));       // 24 pt
    aBulletItem.SetFont(aBulletFont);
    aBulletItem.SetStyle(BS_BULLET);
    aBulletItem.SetStart(1);
    aBulletItem.SetScale(45);               // in Prozent
    aBulletItem.SetSymbol(34);              // Punkt
    pItemPool->SetPoolDefaultItem( aBulletItem );

    // Bullets nicht sichtbar
    SfxUInt16Item aBulletStateItem(EE_PARA_BULLETSTATE, 0);
    pItemPool->SetPoolDefaultItem( aBulletStateItem );

    // Neues BulletItem
    SvxNumberFormat aNumberFormat(SVX_NUM_CHAR_SPECIAL);
    aNumberFormat.SetBulletFont(&aBulletFont);
    aNumberFormat.SetBulletChar( 0xF000 + 34 );
    aNumberFormat.SetBulletRelSize(45);
    aNumberFormat.SetBulletColor(RGB_Color(COL_BLACK));
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

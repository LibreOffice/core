/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: stlpool.cxx,v $
 *
 *  $Revision: 1.33 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-12 16:33:59 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"


#ifndef _EEITEMID_HXX //autogen
#include <svx/eeitemid.hxx>
#endif
#ifndef _EEITEM_HXX //autogen
#include <svx/eeitem.hxx>
#endif
#ifndef _SVX_FHGTITEM_HXX //autogen
#include <svx/fhgtitem.hxx>
#endif
#ifndef _SVX_COLRITEM_HXX //autogen
#include <svx/colritem.hxx>
#endif
#ifndef _SVX_ITEM_HXX //autogen
#include <svx/cntritem.hxx>
#endif
#ifndef _SVX_SHDDITEM_HXX //autogen
#include <svx/shdditem.hxx>
#endif
#ifndef _SVX_CRSDITEM_HXX //autogen
#include <svx/crsditem.hxx>
#endif
#ifndef _SVX_UDLNITEM_HXX //autogen
#include <svx/udlnitem.hxx>
#endif
#ifndef _SVX_WGHTITEM_HXX //autogen
#include <svx/wghtitem.hxx>
#endif
#ifndef _SVX_POSTITEM_HXX //autogen
#include <svx/postitem.hxx>
#endif
#ifndef _SVX_FONTITEM_HXX //autogen
#include <svx/fontitem.hxx>
#endif
#ifndef _SFXPOOLITEM_HXX //autogen
#include <svtools/poolitem.hxx>
#endif
#ifndef SVX_XFILLIT0_HXX //autogen
#include <svx/xfillit0.hxx>
#endif
#ifndef _SVX_XLINEIT0_HXX //autogen
#include <svx/xlineit0.hxx>
#endif
#ifndef _SVX_ULSPITEM_HXX //autogen
#include <svx/ulspitem.hxx>
#endif
#ifndef _SVX_NUMITEM_HXX //autogen
#include <svx/numitem.hxx>
#endif
#define ITEMID_BRUSH    0
#ifndef _SVX_BRSHITEM_HXX //autogen
#include <svx/brshitem.hxx>
#endif
#ifndef _MyEDITENG_HXX //autogen
#include <svx/editeng.hxx>
#endif
#ifndef _SFXSMPLHINT_HXX //autogen
#include <svtools/smplhint.hxx>
#endif
#ifndef _SVX_LANGITEM_HXX
#include <svx/langitem.hxx>
#endif
#define ITEMID_EMPHASISMARK       EE_CHAR_EMPHASISMARK
#define ITEMID_CHARRELIEF         EE_CHAR_RELIEF
#ifndef _SVX_CHARRELIEFITEM_HXX
#include <svx/charreliefitem.hxx>
#endif
#ifndef _SVX_EMPHITEM_HXX
#include <svx/emphitem.hxx>
#endif


#include <svx/svdattr.hxx>
#include "eetext.hxx"
#include <svx/xtable.hxx>           // fuer RGB_Color
#include <svx/bulitem.hxx>
#include <svx/lrspitem.hxx>
#include <svx/adjitem.hxx>
#ifndef _SFXITEMPOOL_HXX //autogen
#include <svtools/itempool.hxx>
#endif

#define _SDR_POSITIVE
#define _SDR_ITEMS

#include "stlpool.hxx"
#include "sdresid.hxx"
#include "stlsheet.hxx"
#include "glob.hrc"
#include "glob.hxx"
#include "drawdoc.hxx"
#include "sdmod.hxx"
#include "sdpage.hxx"
#include "helpids.h"

#ifndef _SFXITEMSET_HXX
#include <svtools/itemset.hxx>
#endif

/*************************************************************************
|*
|* Ctor
|*
\************************************************************************/

SdStyleSheetPool::SdStyleSheetPool(SfxItemPool& _rPool, SdDrawDocument* pDocument)
:   SfxStyleSheetPool(_rPool)
,   mpActualStyleSheet(NULL)
,   mpDoc(pDocument)
{
}

/*************************************************************************
|*
|* Dtor
|*
\************************************************************************/

SdStyleSheetPool::~SdStyleSheetPool()
{
}

/*************************************************************************
|*
|* Vorlage erzeugen
|*
\************************************************************************/

SfxStyleSheetBase* SdStyleSheetPool::Create(const String& rName,
                                            SfxStyleFamily eFamily,
                                            USHORT _nMask )
{
    return new SdStyleSheet(rName, *this, eFamily, _nMask);
}


/*************************************************************************
|*
|* Vorlage nach Vorbild erzeugen
|*
\************************************************************************/

SfxStyleSheetBase* SdStyleSheetPool::Create(const SdStyleSheet& rStyle)
{
    return new SdStyleSheet(rStyle);
}

/*************************************************************************
|*
|* eine Titelvorlage fuer ein Praesentationslayout suchen
|*
\************************************************************************/

SfxStyleSheetBase* SdStyleSheetPool::GetTitleSheet(const String& rLayoutName)
{
    String aName(rLayoutName);
    aName.AppendAscii( RTL_CONSTASCII_STRINGPARAM( SD_LT_SEPARATOR ));
    aName += String(SdResId(STR_LAYOUT_TITLE));
    SfxStyleSheetBase* pResult = Find(aName, SD_LT_FAMILY);
    return pResult;
}

/*************************************************************************
|*
|* LayoutName des ersten Layouts ermitteln
|*
\************************************************************************/

String SdStyleSheetPool::GetLayoutName() const
{
    String aName( SdResId(STR_LAYOUT_DEFAULT_NAME ) );
    ULONG  nCount = aStyles.Count();

    for( ULONG n = 0; n < nCount; n++ )
    {
        aName = aStyles.GetObject( n )->GetName();
        USHORT nPos = aName.SearchAscii( SD_LT_SEPARATOR );
        if( nPos != STRING_NOTFOUND )
            break;
    }

    USHORT nPos = aName.Search( sal_Unicode( ' ' ));
    if( nPos != STRING_NOTFOUND )
        aName.Erase( nPos );       // removing blanks and number (e.g. "Gliederung 1")

    return aName;
}

/*************************************************************************
|*
|* eine Liste der Gliederungstextvorlagen fuer ein Praesentationlayout
|* erstellen, der Aufrufer muss die Liste wieder loeschen
|*
\************************************************************************/

List* SdStyleSheetPool::CreateOutlineSheetList (const String& rLayoutName)
{
    String aName(rLayoutName);
    aName.AppendAscii( RTL_CONSTASCII_STRINGPARAM( SD_LT_SEPARATOR ));
    aName += String(SdResId(STR_LAYOUT_OUTLINE));
    List* pList = new List;
    for (USHORT nSheet = 1; nSheet < 10; nSheet++)
    {
        String aFullName(aName);
        aFullName.Append( sal_Unicode( ' ' ));
        aFullName.Append( String::CreateFromInt32( (sal_Int32)nSheet ));
        SfxStyleSheetBase* pSheet = Find(aFullName, SD_LT_FAMILY);
        pList->Insert(pSheet, LIST_APPEND);
    }
    return pList;
}

/*************************************************************************
|*
|* StyleSheets mit Defaultweren fuer das genannte Praesentationslayout erzeugen
|*
\************************************************************************/

void SdStyleSheetPool::CreateLayoutStyleSheets(const String& rLayoutName, sal_Bool bCheck /*= sal_False*/ )
{
    (void)bCheck;
    sal_Bool bCreated = sal_False;

    SfxStyleSheetBase* pSheet = NULL;

    String aPrefix(rLayoutName);
    String aSep( RTL_CONSTASCII_USTRINGPARAM( SD_LT_SEPARATOR ));
    aPrefix.Insert(aSep);

    Font aLatinFont, aCJKFont, aCTLFont;

    mpDoc->getDefaultFonts( aLatinFont, aCJKFont, aCTLFont );

    // Font fuer Titel und Gliederung
    SvxFontItem aSvxFontItem( aLatinFont.GetFamily(), aLatinFont.GetName(), aLatinFont.GetStyleName(), aLatinFont.GetPitch(),
                              aLatinFont.GetCharSet(), EE_CHAR_FONTINFO );

    SvxFontItem aSvxFontItemCJK( aCJKFont.GetFamily(), aCJKFont.GetName(), aCJKFont.GetStyleName(), aCJKFont.GetPitch(),
                                 aCJKFont.GetCharSet(), EE_CHAR_FONTINFO_CJK );

    SvxFontItem aSvxFontItemCTL( aCTLFont.GetFamily(), aCTLFont.GetName(), aCTLFont.GetStyleName(), aCTLFont.GetPitch(),
                                 aCTLFont.GetCharSet(), EE_CHAR_FONTINFO_CTL );

    Font aBulletFont( GetBulletFont() );

    /**************************************************************************
    * Gliederungsebenen
    **************************************************************************/
    String aName(SdResId(STR_LAYOUT_OUTLINE));
    String aHelpFile;

    SfxStyleSheetBase* pParent = NULL;
    SvxLRSpaceItem aSvxLRSpaceItem;
    SvxULSpaceItem aSvxULSpaceItem;
    USHORT nLevel;

    for( nLevel = 1; nLevel < 10; nLevel++)
    {
        String aLevelName(aName);
        aLevelName.Append( sal_Unicode( ' ' ));
        aLevelName.Append( String::CreateFromInt32( sal_Int32( nLevel )));

        aLevelName.Insert(aPrefix, 0);

        if (!Find(aLevelName, SD_LT_FAMILY))
        {
            bCreated = sal_True;
            pSheet = &Make(aLevelName, SD_LT_FAMILY);
            pSheet->SetHelpId( aHelpFile, HID_PSEUDOSHEET_OUTLINE + nLevel );

            pSheet->SetParent( String() );

            // Attributierung fuer Level 1, die anderen Ebenen "erben"
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
                rSet.Put( SvxUnderlineItem(UNDERLINE_NONE) );
                rSet.Put( SvxCrossedOutItem(STRIKEOUT_NONE) );
                rSet.Put( SvxShadowedItem(FALSE) );
                rSet.Put( SvxContourItem(FALSE) );
                rSet.Put( SvxEmphasisMarkItem(EMPHASISMARK_NONE) );
                rSet.Put( SvxCharReliefItem(RELIEF_NONE) );
                rSet.Put( SvxColorItem( Color(COL_AUTO)) );
                rSet.Put( XLineStyleItem(XLINE_NONE) );
                rSet.Put( XFillStyleItem(XFILL_NONE) );

                if( nLevel == 1 )
                {
                    Font f( GetBulletFont() );
                    PutNumBulletItem( pSheet, f );
                }
            }

            ULONG nFontSize = 20;
            short nFirstIndent = -600;
            USHORT nIndent = nLevel * 1200;
            USHORT nLower = 100;

            switch (nLevel)
            {
                case 1:
                {
                    nFontSize = 32;
                    nLower = 500;
                    nFirstIndent = -900;
                }
                break;

                case 2:
                {
                    nFontSize = 28;
                    nLower = 400;
                    nFirstIndent = -800;
                }
                break;

                case 3:
                {
                    nFontSize = 24;
                    nLower = 300;
                }
                break;

                case 4:
                {
                    nLower = 200;
                }
                break;
            }

            // FontSize
            nFontSize = (USHORT)((nFontSize * 2540L) / 72);  // Pt --> 1/100 mm
            SfxItemSet& rOutlineSet = pSheet->GetItemSet();
            rOutlineSet.Put( SvxFontHeightItem( nFontSize, 100, EE_CHAR_FONTHEIGHT ) );
            rOutlineSet.Put( SvxFontHeightItem( nFontSize, 100, EE_CHAR_FONTHEIGHT_CJK ) );
            rOutlineSet.Put( SvxFontHeightItem( SdDrawDocument::convertFontHeightToCTL( nFontSize ), 100, EE_CHAR_FONTHEIGHT_CTL ) );

            // Einzuege
            aSvxLRSpaceItem.SetTxtFirstLineOfst(nFirstIndent);
            aSvxLRSpaceItem.SetTxtLeft(nIndent);
            aSvxLRSpaceItem.SetRight(0);
            aSvxLRSpaceItem.SetBulletFI(TRUE);
            pSheet->GetItemSet().Put(aSvxLRSpaceItem);

            // Zeilendurchschuss (Abstand nach unten)
            aSvxULSpaceItem.SetLower(nLower);
            pSheet->GetItemSet().Put(aSvxULSpaceItem);

            if (nLevel == 1)
            {
                SfxUInt16Item aBulletStateItem(EE_PARA_BULLETSTATE, 1); // Bullets sichtbar
                pSheet->GetItemSet().Put(aBulletStateItem);
            }

/*
            // Gliederungsvorlagen haben die naechsthoehere Ebene als Parent
            if (pParent)
                pSheet->SetParent(pParent->GetName());
            pParent = pSheet;
*/
        }
    }

    // if we created outline styles, we need to chain them
    if( bCreated )
    {
        pParent = NULL;
        for (nLevel = 1; nLevel < 10; nLevel++)
        {
            String aLevelName(aName);
            aLevelName.Append( sal_Unicode( ' ' ));
            aLevelName.Append( String::CreateFromInt32( sal_Int32( nLevel )));

            aLevelName.Insert(aPrefix, 0);

            pSheet = Find(aLevelName, SD_LT_FAMILY);

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
    * Titel
    **************************************************************************/
    aName = String(SdResId(STR_LAYOUT_TITLE));
    aName.Insert(aPrefix, 0);

    if (!Find(aName, SD_LT_FAMILY))
    {
        bCreated = sal_True;

        pSheet = &Make(aName, SD_LT_FAMILY);
        pSheet->SetHelpId( aHelpFile, HID_PSEUDOSHEET_TITLE );
        pSheet->SetParent(String());
        SfxItemSet& rTitleSet = pSheet->GetItemSet();
        rTitleSet.Put(XLineStyleItem(XLINE_NONE));
        rTitleSet.Put(XFillStyleItem(XFILL_NONE));
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
        rTitleSet.Put(SvxUnderlineItem(UNDERLINE_NONE));
        rTitleSet.Put(SvxCrossedOutItem(STRIKEOUT_NONE));
        rTitleSet.Put(SvxShadowedItem(FALSE));
        rTitleSet.Put(SvxContourItem(FALSE));
        rTitleSet.Put( SvxEmphasisMarkItem(EMPHASISMARK_NONE) );
        rTitleSet.Put( SvxCharReliefItem(RELIEF_NONE) );
        rTitleSet.Put(SvxColorItem( Color(COL_AUTO)));
        rTitleSet.Put(SvxAdjustItem(SVX_ADJUST_CENTER));
        rTitleSet.Put( SdrTextVertAdjustItem( SDRTEXTVERTADJUST_CENTER ) );
        rTitleSet.Put( SfxUInt16Item(EE_PARA_BULLETSTATE, 0) );
        aBulletFont.SetSize(Size(0,1552));                  // 44 pt
        PutNumBulletItem( pSheet, aBulletFont );
    }

    /**************************************************************************
    * Untertitel
    **************************************************************************/
    aName = String(SdResId(STR_LAYOUT_SUBTITLE));
    aName.Insert(aPrefix, 0);

    if (!Find(aName, SD_LT_FAMILY))
    {
        bCreated = sal_True;

        pSheet = &Make(aName, SD_LT_FAMILY);
        pSheet->SetHelpId( aHelpFile, HID_PSEUDOSHEET_SUBTITLE );
        pSheet->SetParent(String());
        SfxItemSet& rSubtitleSet = pSheet->GetItemSet();
        rSubtitleSet.Put(XLineStyleItem(XLINE_NONE));
        rSubtitleSet.Put(XFillStyleItem(XFILL_NONE));
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
        rSubtitleSet.Put(SvxUnderlineItem(UNDERLINE_NONE));
        rSubtitleSet.Put(SvxCrossedOutItem(STRIKEOUT_NONE));
        rSubtitleSet.Put(SvxShadowedItem(FALSE));
        rSubtitleSet.Put(SvxContourItem(FALSE));
        rSubtitleSet.Put( SvxEmphasisMarkItem(EMPHASISMARK_NONE) );
        rSubtitleSet.Put( SvxCharReliefItem(RELIEF_NONE) );
        rSubtitleSet.Put(SvxColorItem( Color(COL_AUTO)));
        rSubtitleSet.Put(SvxAdjustItem(SVX_ADJUST_CENTER));
        rSubtitleSet.Put( SdrTextVertAdjustItem( SDRTEXTVERTADJUST_CENTER ) );
        rSubtitleSet.Put( SfxUInt16Item(EE_PARA_BULLETSTATE, 0) );
        aSvxLRSpaceItem.SetTxtLeft(0);
        rSubtitleSet.Put(aSvxLRSpaceItem);

        Font aTmpFont( GetBulletFont() );
        aTmpFont.SetSize(Size(0, 1129));        // 32 pt
        PutNumBulletItem( pSheet, aTmpFont );
    }

    /**************************************************************************
    * Notizen
    **************************************************************************/
    aName = String(SdResId(STR_LAYOUT_NOTES));
    aName.Insert(aPrefix, 0);

    if (!Find(aName, SD_LT_FAMILY))
    {
        bCreated = sal_True;

        pSheet = &Make(aName, SD_LT_FAMILY);
        pSheet->SetHelpId( aHelpFile, HID_PSEUDOSHEET_NOTES );
        pSheet->SetParent(String());
        SfxItemSet& rNotesSet = pSheet->GetItemSet();
        rNotesSet.Put(XLineStyleItem(XLINE_NONE));
        rNotesSet.Put(XFillStyleItem(XFILL_NONE));
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
        rNotesSet.Put( SvxUnderlineItem(UNDERLINE_NONE) );
        rNotesSet.Put( SvxCrossedOutItem(STRIKEOUT_NONE) );
        rNotesSet.Put( SvxShadowedItem(FALSE) );
        rNotesSet.Put( SvxContourItem(FALSE) );
        rNotesSet.Put( SvxEmphasisMarkItem(EMPHASISMARK_NONE) );
        rNotesSet.Put( SvxCharReliefItem(RELIEF_NONE) );
        rNotesSet.Put( SvxColorItem( Color(COL_AUTO)) );
        rNotesSet.Put( SfxUInt16Item(EE_PARA_BULLETSTATE, 0) );
        rNotesSet.Put( SvxLRSpaceItem( 0, 0, 600, -600 ) );

        SvxNumBulletItem aNumBullet( (const SvxNumBulletItem&) rNotesSet.Get(EE_PARA_NUMBULLET) );

        EditEngine::ImportBulletItem( aNumBullet, 0, NULL,
                                &(const SvxLRSpaceItem&) rNotesSet.Get( EE_PARA_LRSPACE ) );

        ( (SfxItemSet&) rNotesSet).Put( aNumBullet );

    }

    /**************************************************************************
    * Hintergrundobjekte
    **************************************************************************/
    aName = String(SdResId(STR_LAYOUT_BACKGROUNDOBJECTS));
    aName.Insert(aPrefix, 0);

    if (!Find(aName, SD_LT_FAMILY))
    {
        bCreated = sal_True;

        pSheet = &Make(aName, SD_LT_FAMILY);
        pSheet->SetHelpId( aHelpFile, HID_PSEUDOSHEET_BACKGROUNDOBJECTS );
        pSheet->SetParent(String());
        SfxItemSet& rBackgroundObjectsSet = pSheet->GetItemSet();
        rBackgroundObjectsSet.Put(SdrShadowItem(FALSE));
        rBackgroundObjectsSet.Put(SdrShadowColorItem(String(), Color(COL_GRAY)));
        rBackgroundObjectsSet.Put(SdrShadowXDistItem(300)); // 3 mm Schattendistanz
        rBackgroundObjectsSet.Put(SdrShadowYDistItem(300));
    }

    /**************************************************************************
    * Hintergrund
    **************************************************************************/
    aName = String(SdResId(STR_LAYOUT_BACKGROUND));
    aName.Insert(aPrefix, 0);

    if (!Find(aName, SD_LT_FAMILY))
    {
        bCreated = sal_True;

        pSheet = &Make(aName, SD_LT_FAMILY);
        pSheet->SetHelpId( aHelpFile, HID_PSEUDOSHEET_BACKGROUND );
        pSheet->SetParent(String());
        SfxItemSet& rBackgroundSet = pSheet->GetItemSet();
        rBackgroundSet.Put(XLineStyleItem(XLINE_NONE));
        rBackgroundSet.Put(XFillStyleItem(XFILL_NONE));
    }

    DBG_ASSERT( !bCheck || !bCreated, "missing layout style sheets detected!" );
}

/*************************************************************************
|*
|* StyleSheets des genannten Praesentationslayouts loeschen
|*
\************************************************************************/

void SdStyleSheetPool::EraseLayoutStyleSheets(const String& rLayoutName)
{
    SfxStyleSheetBase* pSheet = NULL;

    List* pNameList = CreateLayoutSheetNames(rLayoutName);

    String* pName = (String*)pNameList->First();
    while (pName)
    {
        pSheet = Find(*pName, SD_LT_FAMILY);
        DBG_ASSERT(pSheet, "EraseLayoutStyleSheets: Vorlage nicht gefunden");
        if (pSheet)
            Erase(pSheet);
        delete pName;
        pName = (String*)pNameList->Next();
    }
    delete pNameList;
}


/*************************************************************************
|*
|* Graphik-StyleSheets  aus dem Quellpool in diesen Pool kopieren
|*
|* (rSourcePool kann nicht const sein, weil SfxStyleSheetPoolBase::Find
|*  nicht const ist)
|*
\************************************************************************/

void SdStyleSheetPool::CopyGraphicSheets(SdStyleSheetPool& rSourcePool)
{
    ULONG nCount = rSourcePool.aStyles.Count();

    std::vector< std::pair< SfxStyleSheetBase*, String > > aNewStyles;

    for (ULONG n = 0; n < nCount; n++)
    {
        SfxStyleSheet* pSheet = (SfxStyleSheet*) rSourcePool.aStyles.GetObject(n);

        if( pSheet->GetFamily() == SFX_STYLE_FAMILY_PARA )
        {
            String aName( pSheet->GetName() );
            if ( !Find( aName, SFX_STYLE_FAMILY_PARA ) )
            {
                SfxStyleSheetBase& rNewSheet = Make( aName, SFX_STYLE_FAMILY_PARA );

                rNewSheet.SetMask( pSheet->GetMask() );

                // #91588# Also set parent relation for copied style sheets
                String aParent( pSheet->GetParent() );
                if( aParent.Len() )
                    aNewStyles.push_back( std::pair< SfxStyleSheetBase*, String >( &rNewSheet, aParent ) );

                rNewSheet.GetItemSet().Put( pSheet->GetItemSet() );
            }
        }
    }

    // set parents on newly added stylesheets
    std::vector< std::pair< SfxStyleSheetBase*, String > >::iterator aIter;
    for( aIter = aNewStyles.begin(); aIter != aNewStyles.end(); aIter++ )
    {
        DBG_ASSERT( rSourcePool.Find( (*aIter).second, SFX_STYLE_FAMILY_PARA ), "StyleSheet has invalid parent: Family mismatch" );
        (*aIter).first->SetParent( (*aIter).second );
    }
}


/*************************************************************************
|*
|* StyleSheets des genannten Praesentationslayouts aus dem Quellpool in diesen
|* Pool kopieren. Kopiert werden nur solche StyleSheets, die in diesem Pool
|* noch nicht vorhanden sind.
|* pCreatedSheets wird - wenn ungleich NULL - mit Zeigern auf die erzeugten
|* StyleSheets gefuellt.
|*
|* (rSourcePool kann nicht const sein, weil SfxStyleSheetPoolBase::Find
|*  nicht const ist)
|*
\************************************************************************/

void SdStyleSheetPool::CopyLayoutSheets(const String& rLayoutName,
                                        SdStyleSheetPool& rSourcePool,
                                        List* pCreatedSheets)
{
    SfxStyleSheetBase* pSheet = NULL;

    String aOutlineTag(SdResId(STR_LAYOUT_OUTLINE));

    List* pNameList = CreateLayoutSheetNames(rLayoutName);

    String* pName = (String*)pNameList->First();
    while (pName)
    {
        pSheet = Find(*pName, SD_LT_FAMILY);
        if (!pSheet)
        {
            SfxStyleSheetBase* pSourceSheet =
                                rSourcePool.Find(*pName, SD_LT_FAMILY);
            DBG_ASSERT(pSourceSheet,
                       "CopyLayoutSheets: Quellvorlage nicht gefunden");
            if (pSourceSheet)   // falls einer mit Methusalem-Doks. ankommt
            {
                SfxStyleSheetBase& rNewSheet = Make(*pName, SD_LT_FAMILY);
                rNewSheet.GetItemSet().Put(pSourceSheet->GetItemSet());
                if (pCreatedSheets)
                    pCreatedSheets->Insert(&rNewSheet, LIST_APPEND);
            }
        }
        delete pName;
        pName = (String*)pNameList->Next();
    }
    delete pNameList;

    // Sonderbehandlung fuer Gliederungsvorlagen: Parentbeziehungen aufbauen
    List* pOutlineSheets = CreateOutlineSheetList(rLayoutName);
    SfxStyleSheetBase* pParent = (SfxStyleSheetBase*)pOutlineSheets->First();
    pSheet = (SfxStyleSheetBase*)pOutlineSheets->Next();
    while (pSheet)
    {
        // kein Parent?
        if (pSheet->GetParent().Len() == 0)
            pSheet->SetParent(pParent->GetName());
        pParent = pSheet;
        pSheet = (SfxStyleSheetBase*)pOutlineSheets->Next();
    }
    delete pOutlineSheets;
}

/*************************************************************************
|*
|* Liste mit den Namen der Praesentationsvorlagen eines Layouts erzeugen.
|* Die Liste und die enthaltenen Strings gehoeren dem Caller!
|*
\************************************************************************/

List* SdStyleSheetPool::CreateLayoutSheetNames(const String& rLayoutName) const
{
    String aPrefix(rLayoutName);
    String aSep( RTL_CONSTASCII_USTRINGPARAM( SD_LT_SEPARATOR ));
    aPrefix.Insert(aSep);

    List* pNameList = new List;

    String aName(SdResId(STR_LAYOUT_OUTLINE));
    String* pName = NULL;

    for (USHORT nLevel = 1; nLevel < 10; nLevel++)
    {
        pName = new String(aName);
        pName->Append( sal_Unicode( ' ' ));
        pName->Append( String::CreateFromInt32( sal_Int32( nLevel )));
        pName->Insert(aPrefix, 0);
        pNameList->Insert(pName, LIST_APPEND);
    }

    pName = new String(SdResId(STR_LAYOUT_TITLE));
    pName->Insert(aPrefix, 0);
    pNameList->Insert(pName, LIST_APPEND);

    pName = new String(SdResId(STR_LAYOUT_SUBTITLE));
    pName->Insert(aPrefix, 0);
    pNameList->Insert(pName, LIST_APPEND);

    pName = new String(SdResId(STR_LAYOUT_NOTES));
    pName->Insert(aPrefix, 0);
    pNameList->Insert(pName, LIST_APPEND);

    pName = new String(SdResId(STR_LAYOUT_BACKGROUNDOBJECTS));
    pName->Insert(aPrefix, 0);
    pNameList->Insert(pName, LIST_APPEND);

    pName = new String(SdResId(STR_LAYOUT_BACKGROUND));
    pName->Insert(aPrefix, 0);
    pNameList->Insert(pName, LIST_APPEND);

    return pNameList;
}

/*************************************************************************
|*
|* Liste mit Zeigern auf Praesentationsvorlagen eines Layouts erzeugen.
|* Die Liste gehoert dem Caller!
|*
\************************************************************************/

List* SdStyleSheetPool::CreateLayoutSheetList(const String& rLayoutName)
{
    List* pList = new List;             // die kriegt der Caller
    String aLayoutNameWithSep(rLayoutName);
    aLayoutNameWithSep.AppendAscii( RTL_CONSTASCII_STRINGPARAM( SD_LT_SEPARATOR ));
    USHORT nLen = aLayoutNameWithSep.Len();

    SfxStyleSheetIterator aIter(this, SD_LT_FAMILY);
    SfxStyleSheetBase* pSheet = aIter.First();

    while (pSheet)
    {
        if (pSheet->GetName().Match(aLayoutNameWithSep) == nLen)
            pList->Insert(pSheet, LIST_APPEND);
        pSheet = aIter.Next();
    }
    return pList;
}

/*************************************************************************
|*
|* ggfs. PseudoStyleSheets erzeugen
|*
\************************************************************************/

void SdStyleSheetPool::CreatePseudosIfNecessary()
{
    String aName;
    String aHelpFile;
    SfxStyleSheetBase* pSheet = NULL;
    SfxStyleSheetBase* pParent = NULL;

    //USHORT nUsedMask = SFXSTYLEBIT_ALL & ~SFXSTYLEBIT_USERDEF;
    USHORT nUsedMask = SFXSTYLEBIT_USED;

    aName = String(SdResId(STR_PSEUDOSHEET_TITLE));
    if( (pSheet = Find(aName, SFX_STYLE_FAMILY_PSEUDO)) == 0 )
    {
        pSheet = &Make(aName, SFX_STYLE_FAMILY_PSEUDO, nUsedMask);
        pSheet->SetParent( String() );
        ((SfxStyleSheet*)pSheet)->StartListening(*this);
    }
    pSheet->SetHelpId( aHelpFile, HID_PSEUDOSHEET_TITLE );

    aName = String(SdResId(STR_PSEUDOSHEET_SUBTITLE));
    if( (pSheet = Find(aName, SFX_STYLE_FAMILY_PSEUDO)) == 0 )
    {
        pSheet = &Make(aName, SFX_STYLE_FAMILY_PSEUDO, nUsedMask);
        pSheet->SetParent(String());
        ((SfxStyleSheet*)pSheet)->StartListening(*this);
    }
    pSheet->SetHelpId( aHelpFile, HID_PSEUDOSHEET_SUBTITLE );

    aName = String(SdResId(STR_PSEUDOSHEET_BACKGROUNDOBJECTS));
    if( (pSheet = Find(aName, SFX_STYLE_FAMILY_PSEUDO)) == 0 )
    {
        pSheet = &Make(aName, SFX_STYLE_FAMILY_PSEUDO, nUsedMask);
        pSheet->SetParent( String() );
        ((SfxStyleSheet*)pSheet)->StartListening(*this);
    }
    pSheet->SetHelpId( aHelpFile, HID_PSEUDOSHEET_BACKGROUNDOBJECTS );

    aName = String(SdResId(STR_PSEUDOSHEET_BACKGROUND));
    if( (pSheet = Find(aName, SFX_STYLE_FAMILY_PSEUDO)) == 0 )
    {
        pSheet = &Make(aName, SFX_STYLE_FAMILY_PSEUDO, nUsedMask);
        pSheet->SetParent( String() );
        ((SfxStyleSheet*)pSheet)->StartListening(*this);
    }
    pSheet->SetHelpId( aHelpFile, HID_PSEUDOSHEET_BACKGROUND );

    aName = String(SdResId(STR_PSEUDOSHEET_NOTES));
    if( (pSheet = Find(aName, SFX_STYLE_FAMILY_PSEUDO)) == 0 )
    {
        pSheet = &Make(aName, SFX_STYLE_FAMILY_PSEUDO, nUsedMask);
        pSheet->SetParent( String() );
        ((SfxStyleSheet*)pSheet)->StartListening(*this);
    }
    pSheet->SetHelpId( aHelpFile, HID_PSEUDOSHEET_NOTES );

    pParent = NULL;
    SetSearchMask(SFX_STYLE_FAMILY_PSEUDO);
    aName = String(SdResId(STR_PSEUDOSHEET_OUTLINE));
    for (USHORT nLevel = 1; nLevel < 10; nLevel++)
    {
        String aLevelName(aName);
        aLevelName.Append( sal_Unicode( ' ' ));
        aLevelName.Append( String::CreateFromInt32( sal_Int32( nLevel )));

        if( (pSheet = Find(aLevelName, SFX_STYLE_FAMILY_PSEUDO)) == 0 )
        {
            pSheet = &Make(aLevelName, SFX_STYLE_FAMILY_PSEUDO, nUsedMask);

            if (pSheet)
            {
                if (pParent)
                    pSheet->SetParent(pParent->GetName());
                pParent = pSheet;
                ((SfxStyleSheet*)pSheet)->StartListening(*this);
            }
        }
        pSheet->SetHelpId( aHelpFile, HID_PSEUDOSHEET_OUTLINE + nLevel );
    }
}


/*************************************************************************
|*
|* Standard-Styles den richtigen Namen in der Programm-Sprache geben
|*
\************************************************************************/

void SdStyleSheetPool::UpdateStdNames()
{
    BOOL bNewHelpIds = FALSE;
    String aHelpFile;
    ULONG  nCount = aStyles.Count();
    List* pEraseList = NULL;

    for( ULONG n=0; n < nCount; n++ )
    {
        SfxStyleSheetBase* pStyle = aStyles.GetObject( n );

        if( !pStyle->IsUserDefined() )
        {
            String aOldName     = pStyle->GetName();
            ULONG nHelpId       = pStyle->GetHelpId( aHelpFile );
            SfxStyleFamily eFam = pStyle->GetFamily();

            BOOL bHelpKnown = TRUE;
            String aNewName;
            USHORT nNameId = 0;
            switch( nHelpId )
            {
                case HID_STANDARD_STYLESHEET_NAME:  nNameId = STR_STANDARD_STYLESHEET_NAME; break;
                case HID_POOLSHEET_OBJWITHARROW:    nNameId = STR_POOLSHEET_OBJWITHARROW;   break;
                case HID_POOLSHEET_OBJWITHSHADOW:   nNameId = STR_POOLSHEET_OBJWITHSHADOW;  break;
                case HID_POOLSHEET_OBJWITHOUTFILL:  nNameId = STR_POOLSHEET_OBJWITHOUTFILL; break;
                case HID_POOLSHEET_TEXT:            nNameId = STR_POOLSHEET_TEXT;           break;
                case HID_POOLSHEET_TEXTBODY:        nNameId = STR_POOLSHEET_TEXTBODY;       break;
                case HID_POOLSHEET_TEXTBODY_JUSTIFY:nNameId = STR_POOLSHEET_TEXTBODY_JUSTIFY;break;
                case HID_POOLSHEET_TEXTBODY_INDENT: nNameId = STR_POOLSHEET_TEXTBODY_INDENT;break;
                case HID_POOLSHEET_TITLE:           nNameId = STR_POOLSHEET_TITLE;          break;
                case HID_POOLSHEET_TITLE1:          nNameId = STR_POOLSHEET_TITLE1;         break;
                case HID_POOLSHEET_TITLE2:          nNameId = STR_POOLSHEET_TITLE2;         break;
                case HID_POOLSHEET_HEADLINE:        nNameId = STR_POOLSHEET_HEADLINE;       break;
                case HID_POOLSHEET_HEADLINE1:       nNameId = STR_POOLSHEET_HEADLINE1;      break;
                case HID_POOLSHEET_HEADLINE2:       nNameId = STR_POOLSHEET_HEADLINE2;      break;
                case HID_POOLSHEET_MEASURE:         nNameId = STR_POOLSHEET_MEASURE;        break;

                case HID_PSEUDOSHEET_TITLE:         nNameId = STR_PSEUDOSHEET_TITLE;        break;
                case HID_PSEUDOSHEET_SUBTITLE:      nNameId = STR_PSEUDOSHEET_SUBTITLE;     break;
                case HID_PSEUDOSHEET_OUTLINE1:
                case HID_PSEUDOSHEET_OUTLINE2:
                case HID_PSEUDOSHEET_OUTLINE3:
                case HID_PSEUDOSHEET_OUTLINE4:
                case HID_PSEUDOSHEET_OUTLINE5:
                case HID_PSEUDOSHEET_OUTLINE6:
                case HID_PSEUDOSHEET_OUTLINE7:
                case HID_PSEUDOSHEET_OUTLINE8:
                case HID_PSEUDOSHEET_OUTLINE9:      nNameId = STR_PSEUDOSHEET_OUTLINE;      break;
                case HID_PSEUDOSHEET_BACKGROUNDOBJECTS: nNameId = STR_PSEUDOSHEET_BACKGROUNDOBJECTS; break;
                case HID_PSEUDOSHEET_BACKGROUND:    nNameId = STR_PSEUDOSHEET_BACKGROUND;   break;
                case HID_PSEUDOSHEET_NOTES:         nNameId = STR_PSEUDOSHEET_NOTES;        break;

                default:
                    // 0 oder falsche (alte) HelpId
                    bHelpKnown = FALSE;
            }
            if( bHelpKnown )
            {
                if( nNameId )
                {
                    aNewName = String( SdResId( nNameId ) );
                    if( nNameId == STR_PSEUDOSHEET_OUTLINE )
                    {
                        aNewName.Append( sal_Unicode( ' ' ));
                        aNewName.Append( String::CreateFromInt32( sal_Int32( nHelpId - HID_PSEUDOSHEET_OUTLINE )));
                    }
                }

                if( aNewName.Len() && aNewName != aOldName )
                {
                    SfxStyleSheetBase* pSheetFound = Find( aNewName, eFam );

                    if ( !pSheetFound )
                    {
                        // Sheet existiert noch nicht: Altes Sheet wird umbenannt
                        pStyle->SetName( aNewName );    // setzt auch Parents um
                    }
                    else
                    {
                        // Sheet existiert schon: Altes Sheet muss entfernt werden
                        if( !pEraseList )
                        {
                            pEraseList = new List();
                        }

                        pEraseList->Insert( pStyle );
                    }
                }
            }
            else
            {
                //  falsche oder keine HelpId -> neue generieren

                DBG_WARNING("Default-Vorlage mit falscher oder keiner HelpId");

                // Da bisher nur deutsche Dokumente erzeugt worden sein koennen,
                // kann hier auch der deutsche Text abgefragt werden, damit man
                // nicht in einer anderssprachigen Version auf die deutsche
                // Version zurueckgreifen muss!

                USHORT nNewId = 0;
                if( eFam == SFX_STYLE_FAMILY_PARA )
                {
                    if( aOldName == String( SdResId( STR_STANDARD_STYLESHEET_NAME ) ) ||
                        aOldName.EqualsAscii( "Standard", 0, RTL_CONSTASCII_LENGTH( "Standard" )) )
                        nNewId = HID_STANDARD_STYLESHEET_NAME;
                    else if( aOldName == String( SdResId( STR_POOLSHEET_OBJWITHARROW ) ) ||
                        aOldName.EqualsAscii( "Objekt mit Pfeilspitze", 0, RTL_CONSTASCII_LENGTH( "Objekt mit Pfeilspitze" )) )
                        nNewId = HID_POOLSHEET_OBJWITHARROW;
                    else if( aOldName == String( SdResId( STR_POOLSHEET_OBJWITHSHADOW ) ) ||
                        aOldName.EqualsAscii( "Objekt mit Schatten", 0, RTL_CONSTASCII_LENGTH( "Objekt mit Schatten" )) )
                        nNewId = HID_POOLSHEET_OBJWITHSHADOW;
                    else if( aOldName == String( SdResId( STR_POOLSHEET_OBJWITHOUTFILL ) ) ||
                        aOldName == String( SdResId( STR_POOLSHEET_OBJWITHOUTFILL_NT ) ) )
                        //aOldName.EqualsAscii( "Objekt ohne Füllung", 0, RTL_CONSTASCII_LENGTH( "Objekt ohne Füllung" )) )
                        nNewId = HID_POOLSHEET_OBJWITHOUTFILL;
                    else if( aOldName == String( SdResId( STR_POOLSHEET_TEXT ) ) ||
                        aOldName.EqualsAscii( "Text", 0, RTL_CONSTASCII_LENGTH( "Text" )) )
                        nNewId = HID_POOLSHEET_TEXT;
                    else if( aOldName == String( SdResId( STR_POOLSHEET_TEXTBODY ) ) ||
                        aOldName == String( SdResId( STR_POOLSHEET_TEXTBODY_NT ) ) )
                        //aOldName.EqualsAscii( "Textkörper", 0, RTL_CONSTASCII_LENGTH( "Textkörper" )) )
                        nNewId = HID_POOLSHEET_TEXTBODY;
                    else if( aOldName == String( SdResId( STR_POOLSHEET_TEXTBODY_JUSTIFY ) ) ||
                        aOldName == String( SdResId( STR_POOLSHEET_TEXTBODY_JUSTIFY_NT ) ) )
                        //aOldName.EqualsAscii( "Textkörper Blocksatz", 0, RTL_CONSTASCII_LENGTH( "Textkörper Blocksatz" )) )
                        nNewId = HID_POOLSHEET_TEXTBODY_JUSTIFY;
                    else if( aOldName == String( SdResId( STR_POOLSHEET_TEXTBODY_INDENT ) ) ||
                        aOldName == String( SdResId( STR_POOLSHEET_TEXTBODY_INDENT_NT ) ) )
                        //aOldName.EqualsAscii( "Textkörper Einzug", 0, RTL_CONSTASCII_LENGTH( "Textkörper Einzug" )) )
                        nNewId = HID_POOLSHEET_TEXTBODY_INDENT;
                    else if( aOldName == String( SdResId( STR_POOLSHEET_TITLE ) ) ||
                        aOldName.EqualsAscii( "Titel", 0, RTL_CONSTASCII_LENGTH( "Titel" )) )
                        nNewId = HID_POOLSHEET_TITLE;
                    else if( aOldName == String( SdResId( STR_POOLSHEET_TITLE1 ) ) ||
                        aOldName.EqualsAscii( "Titel1", 0, RTL_CONSTASCII_LENGTH( "Titel1" )) )
                        nNewId = HID_POOLSHEET_TITLE1;
                    else if( aOldName == String( SdResId( STR_POOLSHEET_TITLE2 ) ) ||
                        aOldName.EqualsAscii( "Titel2", 0, RTL_CONSTASCII_LENGTH( "Titel2" )) )
                        nNewId = HID_POOLSHEET_TITLE2;
                    else if( aOldName == String( SdResId( STR_POOLSHEET_HEADLINE ) ) ||
                        aOldName == String( SdResId( STR_POOLSHEET_HEADLINE_NT ) ) )
                        //aOldName.EqualsAscii( "Überschrift", 0, RTL_CONSTASCII_LENGTH( "Überschrift" )) )
                        nNewId = HID_POOLSHEET_HEADLINE;
                    else if( aOldName == String( SdResId( STR_POOLSHEET_HEADLINE1 ) ) ||
                        aOldName == String( SdResId( STR_POOLSHEET_HEADLINE1_NT ) ) )
                        //aOldName.EqualsAscii( "Überschrift1", 0, RTL_CONSTASCII_LENGTH( "Überschrift1" )) )
                        nNewId = HID_POOLSHEET_HEADLINE1;
                    else if( aOldName == String( SdResId( STR_POOLSHEET_HEADLINE2 ) ) ||
                        aOldName == String( SdResId( STR_POOLSHEET_HEADLINE2_NT ) ) )
                        //aOldName.EqualsAscii( "Überschrift2", 0, RTL_CONSTASCII_LENGTH( "Überschrift2" )) )
                        nNewId = HID_POOLSHEET_HEADLINE2;
                    else if( aOldName == String( SdResId( STR_POOLSHEET_MEASURE ) ) ||
                        aOldName == String( SdResId( STR_POOLSHEET_MEASURE_NT ) ) )
                        //aOldName.EqualsAscii( "Maßlinie", 0, RTL_CONSTASCII_LENGTH( "Maßlinie" )) )
                        nNewId = HID_POOLSHEET_MEASURE;
                }
                else if( eFam == SFX_STYLE_FAMILY_PSEUDO )
                {
                    String aStr( SdResId( STR_PSEUDOSHEET_OUTLINE ) );
                    aStr.Append( sal_Unicode( ' ' ));
                    String aStr2( RTL_CONSTASCII_USTRINGPARAM( "Gliederung " ));

                    if( aOldName == String( SdResId( STR_PSEUDOSHEET_TITLE ) ) ||
                        aOldName.EqualsAscii( "Titel", 0, RTL_CONSTASCII_LENGTH( "Titel" )))
                        nNewId = HID_PSEUDOSHEET_TITLE;
                    else if( aOldName == String( SdResId( STR_PSEUDOSHEET_SUBTITLE ) ) ||
                        aOldName.EqualsAscii( "Untertitel", 0, RTL_CONSTASCII_LENGTH( "Untertitel" )) )
                        nNewId = HID_PSEUDOSHEET_SUBTITLE;
                    else if( aOldName == String( SdResId( STR_PSEUDOSHEET_BACKGROUNDOBJECTS ) ) ||
                        aOldName.EqualsAscii( "Hintergrundobjekte", 0, RTL_CONSTASCII_LENGTH( "Hintergrundobjekte" )) )
                        nNewId = HID_PSEUDOSHEET_BACKGROUNDOBJECTS;
                    else if( aOldName == String( SdResId( STR_PSEUDOSHEET_BACKGROUND ) ) ||
                        aOldName.EqualsAscii( "Hintergrund", 0, RTL_CONSTASCII_LENGTH( "Hintergrund" )) )
                        nNewId = HID_PSEUDOSHEET_BACKGROUND;
                    else if( aOldName == String( SdResId( STR_PSEUDOSHEET_NOTES ) ) ||
                        aOldName.EqualsAscii( "Notizen", 0, RTL_CONSTASCII_LENGTH( "Notizen" )) )
                        nNewId = HID_PSEUDOSHEET_NOTES;
                    else if( aOldName.Equals( String( String( aStr ).Append( String( UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "1" ) ) ) ) ) ) ||
                             aOldName.Equals( String( String( aStr2 ).Append( String( UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "1" ) ) ) ) ) ) )
                        nNewId = HID_PSEUDOSHEET_OUTLINE1;
                    else if( aOldName.Equals( String( String( aStr ).Append( String( UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "2" ) ) ) ) ) ) ||
                             aOldName.Equals( String( String( aStr2 ).Append( String( UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "2" ) ) ) ) ) ) )
                        nNewId = HID_PSEUDOSHEET_OUTLINE2;
                    else if( aOldName.Equals( String( String( aStr ).Append( String( UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "3" ) ) ) ) ) ) ||
                             aOldName.Equals( String( String( aStr2 ).Append( String( UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "3" ) ) ) ) ) ) )
                        nNewId = HID_PSEUDOSHEET_OUTLINE3;
                    else if( aOldName.Equals( String( String( aStr ).Append( String( UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "4" ) ) ) ) ) ) ||
                             aOldName.Equals( String( String( aStr2 ).Append( String( UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "4" ) ) ) ) ) ) )
                        nNewId = HID_PSEUDOSHEET_OUTLINE4;
                    else if( aOldName.Equals( String( String( aStr ).Append( String( UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "5" ) ) ) ) ) ) ||
                             aOldName.Equals( String( String( aStr2 ).Append( String( UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "5" ) ) ) ) ) ) )
                        nNewId = HID_PSEUDOSHEET_OUTLINE5;
                    else if( aOldName.Equals( String( String( aStr ).Append( String( UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "6" ) ) ) ) ) ) ||
                             aOldName.Equals( String( String( aStr2 ).Append( String( UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "6" ) ) ) ) ) ) )
                        nNewId = HID_PSEUDOSHEET_OUTLINE6;
                    else if( aOldName.Equals( String( String( aStr ).Append( String( UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "7" ) ) ) ) ) ) ||
                             aOldName.Equals( String( String( aStr2 ).Append( String( UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "7" ) ) ) ) ) ) )
                        nNewId = HID_PSEUDOSHEET_OUTLINE7;
                    else if( aOldName.Equals( String( String( aStr ).Append( String( UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "8" ) ) ) ) ) ) ||
                             aOldName.Equals( String( String( aStr2 ).Append( String( UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "8" ) ) ) ) ) ) )
                        nNewId = HID_PSEUDOSHEET_OUTLINE8;
                    else if( aOldName.Equals( String( String( aStr ).Append( String( UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "9" ) ) ) ) ) ) ||
                             aOldName.Equals( String( String( aStr2 ).Append( String( UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( "9" ) ) ) ) ) ) )
                        nNewId = HID_PSEUDOSHEET_OUTLINE9;
                }

                if ( nNewId )
                {
                    bNewHelpIds = TRUE;
                    pStyle->SetHelpId( aHelpFile, nNewId );
                }
            }
        }
    }

    if ( pEraseList )
    {
        // Styles, welche nicht umbenannt werden konnten, muessen entfernt werden
        for ( ULONG i = 0; i < pEraseList->Count(); i++ )
        {
            SfxStyleSheetBase* pEraseSheet = ( SfxStyleSheetBase* ) pEraseList->GetObject( i );
            Erase( pEraseSheet );
        }

        delete pEraseList;
        pEraseList = NULL;
    }

    // Wenn neue HelpIDs gefunden wurden koennen die Namen auch gleich ersetzt werden
    if( bNewHelpIds )
        UpdateStdNames();
}

/*************************************************************************
|*
|* Neues SvxNumBulletItem fuer das jeweilige StyleSheet setzen
|*
\************************************************************************/

void SdStyleSheetPool::PutNumBulletItem( SfxStyleSheetBase* pSheet,
                                         Font& rBulletFont )
{
    String aHelpFile;
    ULONG nHelpId = pSheet->GetHelpId( aHelpFile );
    SfxItemSet& rSet = pSheet->GetItemSet();

    switch ( nHelpId )
    {
        case HID_STANDARD_STYLESHEET_NAME :
        {
            // Standard-Vorlage
            SvxNumberFormat aNumberFormat(SVX_NUM_CHAR_SPECIAL);
            aNumberFormat.SetBulletFont(&rBulletFont);
            aNumberFormat.SetBulletChar( 0x25CF ); // StarBats: 0xF000 + 34
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

            rSet.Put( SvxNumBulletItem( aNumRule, EE_PARA_NUMBULLET ) );
            ((SfxStyleSheet*)pSheet)->Broadcast(SfxSimpleHint( SFX_HINT_DATACHANGED ) );
        }
        break;

        case HID_PSEUDOSHEET_TITLE:
            /* #84013# title gets same bullet as subtitle and not that page symbol anymore */
        case HID_PSEUDOSHEET_SUBTITLE :
        {
            // Untertitel-Vorlage
            SvxNumRule* pDefaultRule = ((SvxNumBulletItem*) rSet.GetPool()->GetSecondaryPool()->GetPoolDefaultItem(EE_PARA_NUMBULLET))->GetNumRule();
            DBG_ASSERT( pDefaultRule, "Wo ist mein Default? [CL]" );

            if(pDefaultRule)
            {
                SvxNumRule aNumRule(pDefaultRule->GetFeatureFlags(), 9, FALSE);
                for(USHORT i=0; i < aNumRule.GetLevelCount(); i++)
                {
                    SvxNumberFormat aFrmt( pDefaultRule->GetLevel(i) );
                    aFrmt.SetNumberingType(SVX_NUM_CHAR_SPECIAL);
                    aFrmt.SetStart(1);
                    aFrmt.SetBulletRelSize(45);
                    aFrmt.SetBulletChar( 0x25CF );  // StarBats: 0xF000 + 34
                    aFrmt.SetBulletFont(&rBulletFont);
                    aNumRule.SetLevel(i, aFrmt);
                }

                rSet.Put( SvxNumBulletItem( aNumRule, EE_PARA_NUMBULLET ) );
                ((SfxStyleSheet*)pSheet)->Broadcast(SfxSimpleHint( SFX_HINT_DATACHANGED ) );
            }
        }
        break;

        case HID_PSEUDOSHEET_OUTLINE + 1 :
        {
            // Gliederungs-Vorlage
            SvxNumberFormat aNumberFormat(SVX_NUM_CHAR_SPECIAL);
            aNumberFormat.SetBulletColor(Color(COL_AUTO));
            aNumberFormat.SetStart(1);
            aNumberFormat.SetNumAdjust(SVX_ADJUST_LEFT);

            SvxNumRule aNumRule( NUM_BULLET_REL_SIZE|NUM_BULLET_COLOR|NUM_CHAR_TEXT_DISTANCE|NUM_SYMBOL_ALIGNMENT,
                                 10, FALSE, SVX_RULETYPE_PRESENTATION_NUMBERING );
            for( UINT16 i = 0; i < 10; i++ )
            {
                aNumberFormat.SetBulletChar( 0x25CF );  // StarBats: 0xF000 + 34
                aNumberFormat.SetBulletRelSize(45);
                const short nLSpace = i ? i * 1200 : 1200;
                aNumberFormat.SetLSpace(nLSpace);
                aNumberFormat.SetAbsLSpace(nLSpace);
                short nFirstLineOffset = -600;

                ULONG nFontSize = 20;
                switch(i)
                {
                    case 0:
                    case 1:
                    {
                        nFontSize = 32;
                        nFirstLineOffset = -900;
                    }
                    break;

                    case 2:
                    {
                        nFontSize = 28;
                        nFirstLineOffset = -800;
                        aNumberFormat.SetBulletChar( 0x2013 );  // StarBats: 0xF000 + 150
                        aNumberFormat.SetBulletRelSize(75);
                    }
                    break;

                    case 3:
                    {
                        nFontSize = 24;
                    }
                    break;

                    case 4:
                    {
                        aNumberFormat.SetBulletChar( 0x2013 ); // StarBats: 0xF000 + 150
                        aNumberFormat.SetBulletRelSize(75);
                    }
                    break;
                }

                aNumberFormat.SetFirstLineOffset(nFirstLineOffset);
                nFontSize = (USHORT)((nFontSize * 2540L) / 72);  // Pt --> 1/100 mm
                rBulletFont.SetSize(Size(0,846));       // 24 pt
                aNumberFormat.SetBulletFont(&rBulletFont);
                aNumRule.SetLevel( i, aNumberFormat );
            }

            rSet.Put( SvxNumBulletItem( aNumRule, EE_PARA_NUMBULLET ) );
            ((SfxStyleSheet*)pSheet)->Broadcast(SfxSimpleHint( SFX_HINT_DATACHANGED ) );
        }
        break;
    }
}

/*************************************************************************
|*
|* Standard Bullet-Font erzeugen (ohne Groesse)
|*
\************************************************************************/

Font SdStyleSheetPool::GetBulletFont() const
{
    Font aBulletFont( String( RTL_CONSTASCII_USTRINGPARAM( "StarSymbol" )), Size(0, 1000) );
    aBulletFont.SetCharSet(RTL_TEXTENCODING_UNICODE);
    aBulletFont.SetWeight(WEIGHT_NORMAL);
    aBulletFont.SetUnderline(UNDERLINE_NONE);
    aBulletFont.SetStrikeout(STRIKEOUT_NONE);
    aBulletFont.SetItalic(ITALIC_NONE);
    aBulletFont.SetOutline(FALSE);
    aBulletFont.SetShadow(FALSE);
    aBulletFont.SetColor(Color(COL_AUTO));
    aBulletFont.SetTransparent(TRUE);

    return aBulletFont;
}






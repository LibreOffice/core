/*************************************************************************
 *
 *  $RCSfile: chardlg.cxx,v $
 *
 *  $Revision: 1.21 $
 *
 *  last change: $Author: pb $ $Date: 2001-03-19 12:33:03 $
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

// include ---------------------------------------------------------------

#ifndef _UNO_LINGU_HXX
#include <unolingu.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_PATHOPTIONS_HXX
#include <svtools/pathoptions.hxx>
#endif
#ifndef _CTRLTOOL_HXX //autogen
#include <svtools/ctrltool.hxx>
#endif
#ifndef _SFONTITM_HXX
#include <svtools/sfontitm.hxx>
#endif
#ifndef _SFX_PRINTER_HXX //autogen
#include <sfx2/printer.hxx>
#endif
#ifndef _SFX_OBJSH_HXX //autogen
#include <sfx2/objsh.hxx>
#endif
#ifndef _SFXVIEWSH_HXX
#include <sfx2/viewsh.hxx>
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif

#pragma hdrstop

#define _SVX_CHARDLG_CXX

#include "dialogs.hrc"
#include "svxitems.hrc"
#include "chardlg.hrc"

//Erstmal definieren, damit die Klassendeklarionen angezogen werden.
#define ITEMID_FONT             SID_ATTR_CHAR_FONT
#define ITEMID_POSTURE          SID_ATTR_CHAR_POSTURE
#define ITEMID_WEIGHT           SID_ATTR_CHAR_WEIGHT
#define ITEMID_SHADOWED         SID_ATTR_CHAR_SHADOWED
#define ITEMID_WORDLINEMODE     SID_ATTR_CHAR_WORDLINEMODE
#define ITEMID_CONTOUR          SID_ATTR_CHAR_CONTOUR
#define ITEMID_CROSSEDOUT       SID_ATTR_CHAR_STRIKEOUT
#define ITEMID_UNDERLINE        SID_ATTR_CHAR_UNDERLINE
#define ITEMID_FONTHEIGHT       SID_ATTR_CHAR_FONTHEIGHT
#define ITEMID_PROPSIZE         SID_ATTR_CHAR_PROPSIZE
#define ITEMID_COLOR            SID_ATTR_CHAR_COLOR
#define ITEMID_KERNING          SID_ATTR_CHAR_KERNING
#define ITEMID_CASEMAP          SID_ATTR_CHAR_CASEMAP
#define ITEMID_LANGUAGE         SID_ATTR_CHAR_LANGUAGE
#define ITEMID_ESCAPEMENT       SID_ATTR_CHAR_ESCAPEMENT
#define ITEMID_FONTLIST         SID_ATTR_CHAR_FONTLIST
#define ITEMID_AUTOKERN         SID_ATTR_CHAR_AUTOKERN
#define ITEMID_COLOR_TABLE      SID_COLOR_TABLE
#define ITEMID_BLINK            SID_ATTR_FLASH
#define ITEMID_BRUSH            SID_ATTR_BRUSH
#define ITEMID_EMPHASISMARK     SID_ATTR_CHAR_EMPHASISMARK
#define ITEMID_TWOLINES         SID_ATTR_CHAR_TWO_LINES
#define ITEMID_CHARROTATE       SID_ATTR_CHAR_ROTATED
#define ITEMID_CHARSCALE_W      SID_ATTR_CHAR_SCALEWIDTH
#define ITEMID_CHARRELIEF       SID_ATTR_CHAR_RELIEF

#include "xtable.hxx"       // XColorTable
#include "chardlg.hxx"
#include "fontitem.hxx"
#include "postitem.hxx"
#include "udlnitem.hxx"
#include "crsditem.hxx"
#include "cntritem.hxx"
#include "langitem.hxx"
#include "wghtitem.hxx"
#include "fhgtitem.hxx"
#include "shdditem.hxx"
#include "escpitem.hxx"
#include "prszitem.hxx"
#include "wrlmitem.hxx"
#include "cmapitem.hxx"
#include "kernitem.hxx"
#include "blnkitem.hxx"
#include "flstitem.hxx"
#include "akrnitem.hxx"
#include "brshitem.hxx"
#include "colritem.hxx"
#include "drawitem.hxx"
#include "dlgutil.hxx"
#include "dialmgr.hxx"
#include "htmlmode.hxx"
#include "charmap.hxx"
#include "chardlg.h"
#include "emphitem.hxx"
#include <charreliefitem.hxx>
#include "twolinesitem.hxx"

#ifndef _SVX_CHARSCALEITEM_HXX
#include <charscaleitem.hxx>
#endif
#ifndef _SVX_CHARROTATEITEM_HXX
#include <charrotateitem.hxx>
#endif

using namespace ::com::sun::star;

// define ----------------------------------------------------------------

#define LW_NORMAL   0
#define LW_GESPERRT 1
#define LW_SCHMAL   2

// static ----------------------------------------------------------------

static USHORT pStdRanges[] =
{
    SID_ATTR_CHAR_FONT,
    SID_ATTR_CHAR_SHADOWED,
    SID_ATTR_CHAR_CONTOUR,
    SID_ATTR_CHAR_COLOR,
    SID_ATTR_CHAR_CASEMAP,
    SID_ATTR_CHAR_FONTLIST,
    0
};

static USHORT pExtRanges[] =
{
    SID_ATTR_CHAR_WORDLINEMODE,
    SID_ATTR_CHAR_WORDLINEMODE,
    SID_ATTR_CHAR_STRIKEOUT,
    SID_ATTR_CHAR_UNDERLINE,
    SID_ATTR_CHAR_PROPSIZE,
    SID_ATTR_CHAR_PROPSIZE,
    SID_ATTR_CHAR_KERNING,
    SID_ATTR_CHAR_CASEMAP,
    SID_ATTR_CHAR_ESCAPEMENT,
    SID_ATTR_CHAR_ESCAPEMENT,
    0
};

// C-Funktion ------------------------------------------------------------

inline BOOL StateToAttr( TriState aState )
{
    return ( STATE_CHECK == aState );
}

// class SvxCharStdPage --------------------------------------------------

SfxTabPage* SvxCharStdPage::Create( Window* pParent, const SfxItemSet& rSet )
{
    return new SvxCharStdPage( pParent, rSet );
}

// -----------------------------------------------------------------------

BOOL SvxCharStdPage::FillItemSet( SfxItemSet& rSet )
{
    const SfxPoolItem* pOld = 0, *pItem;
    const SfxItemSet& rOldSet = GetItemSet();
    BOOL bModified = FALSE;
    const SfxItemSet* pExampleSet = GetTabDialog() ? GetTabDialog()->GetExampleSet() : NULL;

    // "Anderungen beim Font?
    FASTBOOL bChanged = TRUE;
    USHORT nWhich = GetWhich( SID_ATTR_CHAR_FONT );
    const String& rFntName  = aFontBox.GetText();
    const FontList* pFontList = GetFontList();
    String aStyleBoxText = aStyleBox.GetText();
    USHORT nEntryPos = aStyleBox.GetEntryPos( aStyleBoxText );
    if ( nEntryPos >= nExtraEntryPos )
        aStyleBoxText.Erase();
    FontInfo aInfo( pFontList->Get( rFntName, aStyleBoxText ) );

    SvxFontItem aFmtFont( aInfo.GetFamily(), aInfo.GetName(),
                          aInfo.GetStyleName(), aInfo.GetPitch(),
                          aInfo.GetCharSet(), nWhich );
    pOld = GetOldItem( rSet, SID_ATTR_CHAR_FONT );

    if ( pOld )
    {
        const SvxFontItem& rItem = *( (const SvxFontItem*)pOld );

        if ( rItem.GetFamilyName() == aFmtFont.GetFamilyName() )
            bChanged = FALSE;
    }

    if ( !bChanged )
        bChanged = !aFontBox.GetSavedValue().Len();

    if( !bChanged && pExampleSet &&
        pExampleSet->GetItemState(nWhich, FALSE, &pItem) == SFX_ITEM_SET &&
        ((SvxFontItem*)pItem)->GetFamilyName() != aFmtFont.GetFamilyName() )
            bChanged = TRUE;

    if ( bChanged && rFntName.Len() )
    {
        rSet.Put( aFmtFont );
        bModified |= TRUE;
    }
    else if ( SFX_ITEM_DEFAULT == rOldSet.GetItemState( nWhich, FALSE ) )
        rSet.ClearItem( nWhich );

    bChanged = TRUE;
    nWhich = GetWhich( SID_ATTR_CHAR_WEIGHT );
    FontWeight eWeight = aInfo.GetWeight();
    if ( nEntryPos >= nExtraEntryPos )
        eWeight = WEIGHT_NORMAL;
    SvxWeightItem aFmtWeight( eWeight, nWhich );
    pOld = GetOldItem( rSet, SID_ATTR_CHAR_WEIGHT );

    if ( pOld )
    {
        const SvxWeightItem& rItem = *( (const SvxWeightItem*)pOld );

        if ( rItem.GetValue() == aFmtWeight.GetValue() )
            bChanged = FALSE;
    }

    if ( !bChanged )
    {
        bChanged = !aStyleBox.GetSavedValue().Len();

        if ( bInSearchMode && bChanged &&
             aInfo.GetWeight() == WEIGHT_NORMAL && aInfo.GetItalic() != ITALIC_NONE )
            bChanged = FALSE;
    }

    if ( !bChanged && pExampleSet &&
        pExampleSet->GetItemState(nWhich, FALSE, &pItem) == SFX_ITEM_SET &&
        ((SvxWeightItem*)pItem)->GetValue() != aFmtWeight.GetValue() )
        bChanged = TRUE;

    if ( nEntryPos >= nExtraEntryPos )
        bChanged = ( nEntryPos == nExtraEntryPos );

    String aText( aStyleBox.GetText() ); // Tristate, dann Text leer

    if ( bChanged && aText.Len() )
    {
        rSet.Put( aFmtWeight );
        bModified |= TRUE;
    }
    else if ( SFX_ITEM_DEFAULT == rOldSet.GetItemState( nWhich, FALSE ) )
        rSet.ClearItem( nWhich );

    bChanged = TRUE;
    nWhich = GetWhich( SID_ATTR_CHAR_POSTURE );
    FontItalic eItalic = aInfo.GetItalic();
    if ( nEntryPos >= nExtraEntryPos )
        eItalic = ITALIC_NONE;
    SvxPostureItem aFmtPosture( eItalic, nWhich );
    pOld = GetOldItem( rSet, SID_ATTR_CHAR_POSTURE );

    if ( pOld )
    {
        const SvxPostureItem& rItem = *( (const SvxPostureItem*)pOld );

        if ( rItem.GetValue() == aFmtPosture.GetValue() )
            bChanged = FALSE;
    }

    if ( !bChanged )
    {
        bChanged = !aStyleBox.GetSavedValue().Len();

        if ( bInSearchMode && bChanged &&
             aInfo.GetItalic() == ITALIC_NONE && aInfo.GetWeight() != WEIGHT_NORMAL )
            bChanged = FALSE;
    }

    if ( !bChanged && pExampleSet &&
        pExampleSet->GetItemState(nWhich, FALSE, &pItem) == SFX_ITEM_SET &&
        ((SvxPostureItem*)pItem)->GetValue() != aFmtPosture.GetValue() )
            bChanged = TRUE;

    if ( nEntryPos >= nExtraEntryPos )
        bChanged = ( nEntryPos == ( nExtraEntryPos + 1 ) );

    if ( bChanged && aText.Len() )
    {
        rSet.Put( aFmtPosture );
        bModified |= TRUE;
    }
    else if ( SFX_ITEM_DEFAULT == rOldSet.GetItemState( nWhich, FALSE ) )
        rSet.ClearItem( nWhich );

    // FontSize
    long nSize = aSizeBox.GetValue();

    if ( !aSizeBox.GetText().Len() )    // GetValue() gibt dann Min-Wert zurueck
        nSize = 0;
    long nSavedSize = (long)aSizeBox.GetSavedValue().ToInt32();
    FASTBOOL bRel = TRUE;

    if ( !aSizeBox.IsRelative() )
    {
        nSavedSize *= 10;
        bRel = FALSE;
    }

    nWhich = GetWhich( SID_ATTR_CHAR_FONTHEIGHT );
    const SvxFontHeightItem* pOldHeight =
        (const SvxFontHeightItem*)GetOldItem( rSet, SID_ATTR_CHAR_FONTHEIGHT );
    bChanged = ( nSize != nSavedSize );

    if ( !bChanged && pExampleSet &&
        pExampleSet->GetItemState(nWhich, FALSE, &pItem) == SFX_ITEM_SET )
    {
        float fSize = (float)nSize / 10;
        long nVal = CalcToUnit( fSize, rSet.GetPool()->GetMetric( nWhich ) );
        if ( ( (SvxFontHeightItem*)pItem)->GetHeight() != (UINT32)nVal )
            bChanged = TRUE;
    }

    if( bChanged || !pOldHeight ||
        bRel != ( SFX_MAPUNIT_RELATIVE != pOldHeight->GetPropUnit() ||
                    100 != pOldHeight->GetProp() ) )
    {
        SfxMapUnit eUnit = rSet.GetPool()->GetMetric( nWhich );
        if( aSizeBox.IsRelative() )
        {
            DBG_ASSERT( GetItemSet().GetParent(), "No parent set" );
            const SvxFontHeightItem& rOldItem = (const SvxFontHeightItem&)
                GetItemSet().GetParent()->Get( nWhich );

            SvxFontHeightItem aHeight( 240, 100, nWhich );
            if( aSizeBox.IsPtRelative() )
                aHeight.SetHeight( rOldItem.GetHeight(),
                                   (USHORT)( nSize / 10 ),
                                   SFX_MAPUNIT_POINT, eUnit );

            else
                aHeight.SetHeight( rOldItem.GetHeight(),
                                   (USHORT)nSize, SFX_MAPUNIT_RELATIVE );
            rSet.Put( aHeight );
        }
        else
        {
            float fSize = (float)nSize / 10;
            SvxFontHeightItem aFmtSize(
                CalcToUnit( fSize, eUnit ), 100, nWhich );
            rSet.Put( aFmtSize );
        }
        bModified |= TRUE;
    }
    else if ( SFX_ITEM_DEFAULT == rOldSet.GetItemState( nWhich, FALSE ) )
        rSet.ClearItem( nWhich );

    // Farbe
    bModified |= FillItemSet_Impl( aColorBox, rSet, SID_ATTR_CHAR_COLOR );
    if ( aFillColorBox.IsVisible() )
        bModified |= FillItemSet_Impl( aFillColorBox, rSet, SID_ATTR_CHAR_FILLCOLOR );

    bChanged = TRUE;
    // Unterstreichen
    nWhich = GetWhich( SID_ATTR_CHAR_UNDERLINE );
    pOld = GetOldItem( rSet, SID_ATTR_CHAR_UNDERLINE );
    TriState eState = aUnderlineBtn.GetState();

    if ( pOld )
    {
        const SvxUnderlineItem& rItem = *( (const SvxUnderlineItem*)pOld );

        if ( (FontUnderline)rItem.GetValue() == eUnderline &&
             aUnderlineBtn.GetSavedValue() == eState )
            bChanged = FALSE;
    }

    if ( !bChanged && pExampleSet &&
        pExampleSet->GetItemState(nWhich, FALSE, &pItem) == SFX_ITEM_SET &&
        !StateToAttr( eState ) &&
        ((SvxUnderlineItem*)pItem)->GetUnderline() != UNDERLINE_NONE )
            bChanged = TRUE;

    if ( bChanged && eState != STATE_DONTKNOW )
    {
        SvxUnderlineItem aFmtUnderl(
            StateToAttr( eState ) ? eUnderline : UNDERLINE_NONE, nWhich );
        rSet.Put( aFmtUnderl );
        bModified |= TRUE;
    }
    else if ( SFX_ITEM_DEFAULT == rOldSet.GetItemState( nWhich, FALSE ) )
        rSet.ClearItem( nWhich );
    bChanged = TRUE;

    // Durchstreichen
    nWhich = GetWhich( SID_ATTR_CHAR_STRIKEOUT );
    pOld = GetOldItem( rSet, SID_ATTR_CHAR_STRIKEOUT );
    eState = aStrikeoutBtn.GetState();

    if ( pOld )
    {
        const SvxCrossedOutItem& rItem = *( (const SvxCrossedOutItem*)pOld );

        if ( (FontStrikeout)rItem.GetValue() == eStrikeout &&
             aStrikeoutBtn.GetSavedValue() == eState )
            bChanged = FALSE;
    }

    if ( !bChanged && pExampleSet &&
        pExampleSet->GetItemState(nWhich, FALSE, &pItem) == SFX_ITEM_SET &&
        !StateToAttr( eState ) &&
            ((SvxCrossedOutItem*)pItem)->GetStrikeout() != STRIKEOUT_NONE )
        bChanged = TRUE;

    if ( bChanged && eState != STATE_DONTKNOW )
    {
        SvxCrossedOutItem aFmtCrossedOut(
            StateToAttr( eState ) ? eStrikeout : STRIKEOUT_NONE, nWhich );
        rSet.Put( aFmtCrossedOut );
        bModified |= TRUE;
    }
    else if ( SFX_ITEM_DEFAULT == rOldSet.GetItemState( nWhich, FALSE ) )
        rSet.ClearItem( nWhich );
    bChanged = TRUE;

    // Kontour
    nWhich = GetWhich( SID_ATTR_CHAR_CONTOUR );
    pOld = GetOldItem( rSet, SID_ATTR_CHAR_CONTOUR );
    eState = aOutlineBtn.GetState();

    if ( pOld )
    {
        const SvxContourItem& rItem = *( (const SvxContourItem*)pOld );

        if ( rItem.GetValue() == StateToAttr( eState ) &&
             aOutlineBtn.GetSavedValue() == eState )
            bChanged = FALSE;
    }

    if ( !bChanged && pExampleSet &&
        pExampleSet->GetItemState(nWhich, FALSE, &pItem) == SFX_ITEM_SET &&
        !StateToAttr( eState ) && ((SvxContourItem*)pItem)->GetValue() )
        bChanged = TRUE;

    if ( bChanged && eState != STATE_DONTKNOW )
    {
        rSet.Put( SvxContourItem( StateToAttr( eState ), nWhich ) );
        bModified |= TRUE;
    }
    else if ( SFX_ITEM_DEFAULT == rOldSet.GetItemState( nWhich, FALSE ) )
        rSet.ClearItem( nWhich );
    bChanged = TRUE;

    // Schatten
    nWhich = GetWhich( SID_ATTR_CHAR_SHADOWED );
    pOld = GetOldItem( rSet, SID_ATTR_CHAR_SHADOWED );
    eState = aShadowBtn.GetState();

    if ( pOld )
    {
        const SvxShadowedItem& rItem = *( (const SvxShadowedItem*)pOld );

        if ( rItem.GetValue() == StateToAttr( eState ) &&
             aShadowBtn.GetSavedValue() == eState )
            bChanged = FALSE;
    }

    if ( !bChanged && pExampleSet &&
        pExampleSet->GetItemState(nWhich, FALSE, &pItem) == SFX_ITEM_SET &&
        !StateToAttr( eState ) && ((SvxShadowedItem*)pItem)->GetValue() )
        bChanged = TRUE;

    if ( bChanged && eState != STATE_DONTKNOW )
    {
        rSet.Put( SvxShadowedItem( StateToAttr( eState ), nWhich ) );
        bModified |= TRUE;
    }
    else if ( SFX_ITEM_DEFAULT == rOldSet.GetItemState( nWhich, FALSE ) )
        rSet.ClearItem( nWhich );
    bChanged = TRUE;

    // Sprache
    if( aLanguageBox.IsVisible() )
    {
        nWhich = GetWhich( SID_ATTR_CHAR_LANGUAGE );
        pOld = GetOldItem( rSet, SID_ATTR_CHAR_LANGUAGE );
        USHORT nLPos = aLanguageBox.GetSelectEntryPos();
        USHORT nLang = (USHORT)(ULONG)aLanguageBox.GetEntryData(nLPos);
        util::Language nLanguage = SvxGetSelectableLanguages().getConstArray()[ nLang ];

        if ( pOld )
        {
            const SvxLanguageItem& rItem = *( (const SvxLanguageItem*)pOld );

            if ( nLPos == LISTBOX_ENTRY_NOTFOUND ||
                rItem.GetValue() == nLanguage )
                bChanged = FALSE;
        }

        if ( !bChanged )
            bChanged = ( aLanguageBox.GetSavedValue() == LISTBOX_ENTRY_NOTFOUND );

        if ( bChanged && nLPos != LISTBOX_ENTRY_NOTFOUND )
        {
            rSet.Put( SvxLanguageItem( (LanguageType) nLanguage, nWhich ) );
            bModified |= TRUE;
        }
        else if ( SFX_ITEM_DEFAULT == rOldSet.GetItemState( nWhich, FALSE ) )
            rSet.ClearItem( nWhich );
    }

    return bModified;
}

// -----------------------------------------------------------------------

void SvxCharStdPage::Reset( const SfxItemSet& rSet )
{
    // die FontListBox fuellen
    const FontList* pFontList = GetFontList();
    aFontBox.Fill( pFontList );

    // Font ermitteln
    const SvxFontItem* pFontItem = 0;
    USHORT nWhich = GetWhich( SID_ATTR_CHAR_FONT );
    SfxItemState eState = rSet.GetItemState( nWhich );

    if ( eState >= SFX_ITEM_DEFAULT )
    {
        pFontItem = (const SvxFontItem*)&( rSet.Get( nWhich ) );
        aFontBox.SetText( pFontItem->GetFamilyName() );
    }
    else
        aFontBox.SetText( String() );

    // Alle Styles zu einem selektierten Font
    FillStyleBox_Impl();

    FASTBOOL bStyle = FALSE;
    FontItalic eItalic = ITALIC_NONE;
    FontWeight eWeight = WEIGHT_NORMAL;
    nWhich = GetWhich( SID_ATTR_CHAR_POSTURE );
    eState = rSet.GetItemState( nWhich );

    if ( eState >= SFX_ITEM_DEFAULT )
    {
        const SvxPostureItem& rItem = (SvxPostureItem&)rSet.Get( nWhich );
        eItalic = (FontItalic)rItem.GetValue();
        bStyle = TRUE;
    }

    nWhich = GetWhich( SID_ATTR_CHAR_WEIGHT );
    eState = rSet.GetItemState( nWhich );

    if ( eState >= SFX_ITEM_DEFAULT )
    {
        SvxWeightItem& rItem = (SvxWeightItem&)rSet.Get( nWhich );
        eWeight = (FontWeight)rItem.GetValue();
    }
    else if ( bStyle )
        bStyle = FALSE;

    // Aktuell eingestellter Font
    if ( bStyle && pFontItem )
    {
/*!!!
        String aStyleStr( pFontItem->GetStyleName() );

        if ( aStyleStr.Len() )
            aStyleBox.SetText( aStyleStr );
        else
        {
            FontInfo aInfo = pFontList->Get( pFontItem->GetFamilyName(), eWeight, eItalic );
            aStyleBox.SetText( pFontList->GetStyleName( aInfo ) );
        }
*/
        FontInfo aInfo = pFontList->Get( pFontItem->GetFamilyName(),
                                         eWeight, eItalic );
        aStyleBox.SetText( pFontList->GetStyleName( aInfo ) );
    }
    else if ( !bInSearchMode || !bStyle )
        aStyleBox.SetText( String() );
    else if ( bStyle )
    {
        FontInfo aInfo = pFontList->Get( String(), eWeight, eItalic );
        aStyleBox.SetText( pFontList->GetStyleName( aInfo ) );
    }

    // SizeBox fuellen
    FillSizeBox_Impl();
    nWhich = GetWhich( SID_ATTR_CHAR_FONTHEIGHT );
    eState = rSet.GetItemState( nWhich );

    if ( aSizeBox.IsRelativeMode() )
    {
        SfxMapUnit eUnit = rSet.GetPool()->GetMetric( nWhich );
        const SvxFontHeightItem& rItem =
            (SvxFontHeightItem&)rSet.Get( nWhich );

        if( rItem.GetProp() != 100 ||
            SFX_MAPUNIT_RELATIVE != rItem.GetPropUnit() )
        {
            BOOL bPtRel = SFX_MAPUNIT_POINT == rItem.GetPropUnit();
            aSizeBox.SetPtRelative( bPtRel );
            aSizeBox.SetValue( bPtRel ? ((short)rItem.GetProp()) * 10
                                      : rItem.GetProp() );
        }
        else
        {
            aSizeBox.SetRelative();
            aSizeBox.SetValue( (long)CalcToPoint( rItem.GetHeight(), eUnit, 10 ) );
        }
    }
    else if ( eState >= SFX_ITEM_DEFAULT )
    {
        SfxMapUnit eUnit = rSet.GetPool()->GetMetric( nWhich );
        const SvxFontHeightItem& rItem =
            (SvxFontHeightItem&)rSet.Get( nWhich );
        aSizeBox.SetValue(
            (long)CalcToPoint( rItem.GetHeight(), eUnit, 10 ) );
    }
    else
        aSizeBox.SetText( String() );

    // Schattiert
    nWhich = GetWhich( SID_ATTR_CHAR_SHADOWED );
    eState = rSet.GetItemState( nWhich );

    switch ( eState )
    {
        case SFX_ITEM_UNKNOWN:
            aShadowBtn.Hide();
            break;

        case SFX_ITEM_DISABLED:
        case SFX_ITEM_READONLY:
            aShadowBtn.Disable();
            break;

        case SFX_ITEM_DONTCARE:
            aShadowBtn.SetState( STATE_DONTKNOW );
            break;

        case SFX_ITEM_DEFAULT:
        case SFX_ITEM_SET:
        {
            const SvxShadowedItem& rItem =
                (SvxShadowedItem&)rSet.Get( nWhich );
            aShadowBtn.SetState( (TriState)rItem.GetValue() );
            aShadowBtn.EnableTriState( FALSE );
            break;
        }
    }

    // Kontour
    nWhich = GetWhich( SID_ATTR_CHAR_CONTOUR );
    eState = rSet.GetItemState( nWhich );

    switch ( eState )
    {
        case SFX_ITEM_UNKNOWN:
            aOutlineBtn.Hide();
            break;

        case SFX_ITEM_DISABLED:
        case SFX_ITEM_READONLY:
            aOutlineBtn.Disable();
            break;

        case SFX_ITEM_DONTCARE:
            aOutlineBtn.SetState( STATE_DONTKNOW );
            break;

        case SFX_ITEM_DEFAULT:
        case SFX_ITEM_SET:
        {
            const SvxContourItem& rItem = (SvxContourItem&)rSet.Get( nWhich );
            aOutlineBtn.SetState( (TriState)rItem.GetValue() );
            aOutlineBtn.EnableTriState( FALSE );
            break;
        }
    }

    // Underline
    nWhich = GetWhich( SID_ATTR_CHAR_UNDERLINE );
    eState = rSet.GetItemState( nWhich );

    switch ( eState )
    {
        case SFX_ITEM_UNKNOWN:
            aUnderlineBtn.Hide();
            break;

        case SFX_ITEM_DISABLED:
        case SFX_ITEM_READONLY:
            aUnderlineBtn.Disable();
            break;

        case SFX_ITEM_DONTCARE:
            aUnderlineBtn.SetState( STATE_DONTKNOW );
            break;

        case SFX_ITEM_DEFAULT:
        case SFX_ITEM_SET:
        {
            const SvxUnderlineItem& rItem =
                (SvxUnderlineItem&)rSet.Get( nWhich );
            eUnderline = (FontUnderline)rItem.GetValue();

            if ( eUnderline == UNDERLINE_NONE )
                aUnderlineBtn.SetState( STATE_NOCHECK );
            else
                aUnderlineBtn.SetState( STATE_CHECK );
            aUnderlineBtn.EnableTriState( FALSE );
            break;
        }
    }

    //  Strikeout
    nWhich = GetWhich( SID_ATTR_CHAR_STRIKEOUT );
    eState = rSet.GetItemState( nWhich );

    switch ( eState )
    {
        case SFX_ITEM_UNKNOWN:
            aStrikeoutBtn.Hide();
            break;

        case SFX_ITEM_DISABLED:
        case SFX_ITEM_READONLY:
            aStrikeoutBtn.Disable();
            break;

        case SFX_ITEM_DONTCARE:
            aStrikeoutBtn.SetState( STATE_DONTKNOW );
            break;

        case SFX_ITEM_DEFAULT:
        case SFX_ITEM_SET:
        {
            const SvxCrossedOutItem& rItem =
                (SvxCrossedOutItem&)rSet.Get( nWhich );
            eStrikeout = (FontStrikeout)rItem.GetValue();

            if ( eStrikeout == STRIKEOUT_NONE )
                aStrikeoutBtn.SetState( STATE_NOCHECK );
            else
                aStrikeoutBtn.SetState( STATE_CHECK );
            aStrikeoutBtn.EnableTriState( FALSE );
            break;
        }
    }

    // Farbe
    Reset_Impl( aColorBox, aColorText, rSet, SID_ATTR_CHAR_COLOR);
    if( aFillColorBox.IsVisible() )
        Reset_Impl( aFillColorBox, aFillColorText, rSet, SID_ATTR_CHAR_FILLCOLOR);

    // Sprache
    if( aLanguageBox.IsVisible() )
    {
        nWhich = GetWhich( SID_ATTR_CHAR_LANGUAGE );
        aLanguageBox.SetNoSelection();
        eState = rSet.GetItemState( nWhich );

        switch ( eState )
        {
            case SFX_ITEM_UNKNOWN:
                aLanguageText.Hide();
                aLanguageBox.Hide();
                break;

            case SFX_ITEM_DISABLED:
            case SFX_ITEM_READONLY:
                aLanguageText.Disable();
                aLanguageBox.Disable();
                break;

            case SFX_ITEM_DONTCARE:
                aLanguageBox.SetNoSelection();
                break;

            case SFX_ITEM_DEFAULT:
            case SFX_ITEM_SET:
            {
                const SvxLanguageItem& rItem =
                    (SvxLanguageItem&)rSet.Get( nWhich );
                DBG_ASSERT( (LanguageType)rItem.GetValue() != LANGUAGE_SYSTEM,
                            "LANGUAGE_SYSTEM nicht erlaubt!" );
                if ( (LanguageType)rItem.GetValue() != LANGUAGE_DONTKNOW )
                {
                    USHORT nLang = (USHORT)SvxGetLanguagePos( SvxGetSelectableLanguages(), rItem.GetValue() );
                    for ( USHORT i = 0; i < aLanguageBox.GetEntryCount(); ++i )
                    {
                        if ( (USHORT)(ULONG)aLanguageBox.GetEntryData(i) == nLang )
                        {
                            aLanguageBox.SelectEntryPos(i);
                            break;
                        }
                    }
                }
                break;
            }
        }
    }


    // alle Attribute des Beispiel-Window-Fonts einstellen
    short       nEsc        = 0;
    BYTE        nEscProp    = 100;
    BYTE        nProp       = 100;
    SvxCaseMap  eCaseMap    = SVX_CASEMAP_NOT_MAPPED;

    // HochTief-Stellung
    nWhich = GetWhich( SID_ATTR_CHAR_ESCAPEMENT );
    eState = rSet.GetItemState( nWhich );

    if ( eState >= SFX_ITEM_DEFAULT )
    {
        const SvxEscapementItem& rItem =
            (SvxEscapementItem&)rSet.Get( nWhich );
        nEsc = rItem.GetEsc();
        nEscProp = rItem.GetProp();
    }

    nWhich = GetWhich( SID_ATTR_CHAR_PROPSIZE );
    eState = rSet.GetItemState( nWhich );

    if ( eState >= SFX_ITEM_DEFAULT )
    {
        const SvxPropSizeItem& rItem = (SvxPropSizeItem&)rSet.Get( nWhich );
        nProp = (BYTE)rItem.GetValue();
    }

    // Auszeichnung
    nWhich = GetWhich( SID_ATTR_CHAR_CASEMAP );
    eState = rSet.GetItemState( nWhich );

    if ( eState >= SFX_ITEM_DEFAULT )
    {
        const SvxCaseMapItem& rItem = (const SvxCaseMapItem&)rSet.Get( nWhich );
        eCaseMap = (SvxCaseMap)rItem.GetValue();
    }

    SvxFont& rFont = aExampleWin.GetFont();
    nWhich = GetWhich( SID_ATTR_CHAR_KERNING );
    eState = rSet.GetItemState( nWhich );

    if ( eState >= SFX_ITEM_DEFAULT )
    {
        MapUnit eUnit = (MapUnit)rSet.GetPool()->GetMetric( nWhich );
        const SvxKerningItem& rItem = (SvxKerningItem&)rSet.Get( nWhich );
        long nKern = LogicToLogic( rItem.GetValue(), eUnit, MAP_TWIP );
        rFont.SetFixKerning( (short)nKern );
    }
    else
        rFont.SetFixKerning( 0 );

    /*
     * Fontattribute aus den Attr-Controls
     * ExampleFont in Klammern, damit diese Variable nicht mehr
     * verwendet wird.  Update() und andere holen sich den Font
     * aus dem Beispielcontrol; das gibt nette Seiteneffekte, wenn
     * dann mit diesem Font(aExampleFont) weitergearbeitet wird und
     * dieser anschliessend in das Beispielcontrol selektiert wird.
     */
    if ( pFontItem )
    {
        rFont.SetFamily( pFontItem->GetFamily() );
        rFont.SetName( pFontItem->GetFamilyName() );
        rFont.SetPitch( pFontItem->GetPitch() );
        rFont.SetCharSet( pFontItem->GetCharSet() );
    }

    nWhich = GetWhich( SID_ATTR_CHAR_FONTHEIGHT );
    eState = rSet.GetItemState( nWhich );

    if ( aSizeBox.IsRelativeMode() )
    {
        DBG_ASSERT( rSet.GetParent(), "No parent set" );

        // SetValue wuerde nur einen Prozent-Wert zurueckgeben, deshalb
        // den Wert vom Parent des Items abholen
        SfxMapUnit eUnit = rSet.GetPool()->GetMetric( nWhich );
        const SvxFontHeightItem& rItem =
            (SvxFontHeightItem&)rSet.GetParent()->Get( nWhich );
        rFont.SetSize( Size( 0, PointToTwips(
            (long)CalcToPoint( rItem.GetHeight(), eUnit, 1 ) ) ) );
    }
    else if ( aSizeBox.GetText().Len() )
        rFont.SetSize( Size( 0, PointToTwips( aSizeBox.GetValue() ) / 10 ) );
    else
        // als Default 10pt
        rFont.SetSize( Size( 0, 200 ) );

    rFont.SetWeight(
        eWeight != WEIGHT_NORMAL ? WEIGHT_BOLD : WEIGHT_NORMAL );

    rFont.SetItalic(
        eItalic != ITALIC_NONE ? ITALIC_NORMAL : ITALIC_NONE );

    rFont.SetUnderline(
        StateToAttr( aUnderlineBtn.GetState() ) ? eUnderline : UNDERLINE_NONE );

    rFont.SetStrikeout(
        StateToAttr( aStrikeoutBtn.GetState() ) ? eStrikeout : STRIKEOUT_NONE );

    // Outline Shadow
    rFont.SetOutline( StateToAttr( aOutlineBtn.GetState() ) );
    rFont.SetShadow( StateToAttr( aShadowBtn.GetState() ) );

    // Color
    rFont.SetColor( aColorBox.GetSelectEntryColor() );
    if( aFillColorBox.IsVisible() )
    {
        Color aSelectedColor;
        if( aFillColorBox.GetSelectEntry() == aStrTransparent )
            aSelectedColor = Color( COL_TRANSPARENT );
        else
            aSelectedColor = aFillColorBox.GetSelectEntryColor();
        rFont.SetFillColor( aSelectedColor );
    }

    // Kapitaelchen
    rFont.SetCaseMap( eCaseMap );

    // erst Relative Groesse dann Escapement
    rFont.SetPropr( nProp );
    rFont.SetProprRel( nEscProp );
    rFont.SetEscapement( nEsc );

    //  WordLineMode
    nWhich = GetWhich( SID_ATTR_CHAR_WORDLINEMODE );
    eState = rSet.GetItemState( nWhich );

    if ( eState >= SFX_ITEM_DEFAULT )
    {
        const SvxWordLineModeItem& rItem =
            (SvxWordLineModeItem&)rSet.Get( nWhich );
        rFont.SetWordLineMode( rItem.GetValue() );
    }

    // Hintergrund umstellen
    aExampleWin.Invalidate();

    // Hilfetext
    aFontTypeText.SetText( pFontList->GetFontMapText(
            pFontList->Get( aFontBox.GetText(), aStyleBox.GetText() ) ) );

    // alte Einstellungen merken
    aFontBox.SaveValue();
    aStyleBox.SaveValue();
    aSizeBox.SaveValue();
    aOutlineBtn.SaveValue();
    aShadowBtn.SaveValue();
    aUnderlineBtn.SaveValue();
    aStrikeoutBtn.SaveValue();
    aColorBox.SaveValue();
    aFillColorBox.SaveValue();
    aLanguageBox.SaveValue();
}

// -----------------------------------------------------------------------

void SvxCharStdPage::DataChanged( const DataChangedEvent& rDCEvt )
{
    SfxTabPage::DataChanged( rDCEvt );
    if ( bDeleteFontList )
        delete (FontList*)_pFontList;
    _pFontList = NULL;
}

// -----------------------------------------------------------------------

void SvxCharStdPage::EnableRelativeMode()
{
    DBG_ASSERT( GetItemSet().GetParent(), "RelativeMode, but no ParentSet!" );
    aSizeBox.EnableRelativeMode( 0, 999 ); // min 0%, max 999%, step 5

    USHORT nWhich = GetWhich( SID_ATTR_CHAR_FONTHEIGHT );
    const SvxFontHeightItem& rItem =
            (SvxFontHeightItem&)GetItemSet().GetParent()->Get( nWhich );
    SfxMapUnit eUnit = GetItemSet().GetPool()->GetMetric( nWhich );
    short nCurHeight = CalcToPoint( rItem.GetHeight(), eUnit, 1 ) * 10;

    // ausgehend von der akt. Hoehe:
    //      - neagtiv bis minimal 2 pt
    //      - positiv bis maximal 999 pt
    aSizeBox.EnablePtRelativeMode( -(nCurHeight - 20),
                                    (9999 - nCurHeight), 10 );
}

// -----------------------------------------------------------------------

void SvxCharStdPage::SetInfoItem( const SvxFontListItem& rItem )
{
    if( bDeleteFontList )
    {
        delete (FontList*)_pFontList;
        bDeleteFontList = FALSE;
    }
    _pFontList = rItem.GetFontList();
}

// -----------------------------------------------------------------------

const FontList* SvxCharStdPage::GetFontList() const
{
    if ( !_pFontList )
    {
        SfxObjectShell* pDocSh = SfxObjectShell::Current();
        const SfxPoolItem* pItem;

        if ( pDocSh && ( pItem = pDocSh->GetItem( SID_ATTR_CHAR_FONTLIST ) ) )
            ((SvxCharStdPage*)this)->_pFontList = ( (SvxFontListItem*)pItem )->GetFontList();
        else
        {
            ((SvxCharStdPage*)this)->_pFontList = new FontList( Application::GetDefaultDevice() );
            ((SvxCharStdPage*)this)->bDeleteFontList = TRUE;
        }
    }
    return _pFontList;
}

// -----------------------------------------------------------------------

SvxCharStdPage::SvxCharStdPage( Window* pParent,
                                const SfxItemSet& rAttr ) :

    SfxTabPage( pParent, SVX_RES( RID_SVXPAGE_CHAR_STD ), rAttr ),

    aFontText       ( this, ResId( FT_FONT ) ),
    aFontBox        ( this, ResId( LB_FONT ) ),
    aStyleText      ( this, ResId( FT_STYLE ) ),
    aStyleBox       ( this, ResId( LB_STYLE ) ),
    aSizeText       ( this, ResId( FT_SIZE ) ),
    aSizeBox        ( this, ResId( LB_SIZE ) ),
    aOutlineBtn     ( this, ResId( BTN_OUTLINE ) ),
    aShadowBtn      ( this, ResId( BTN_SHADOW ) ),
    aUnderlineBtn   ( this, ResId( BTN_UNDERLINE ) ),
    aStrikeoutBtn   ( this, ResId( BTN_STRIKEOUT ) ),
    aCasemapBox     ( this, ResId( GB_CASEMAP ) ),
    aColorText      ( this, ResId( FT_COLOR ) ),
    aColorBox       ( this, ResId( LB_COLOR ) ),
    aFillColorText  ( this, ResId( FT_FILLCOLOR ) ),
    aFillColorBox   ( this, ResId( LB_FILLCOLOR ) ),
    aLanguageText   ( this, ResId( FT_LANGUAGE ) ),
    aLanguageBox    ( this, ResId( LB_LANGUAGE ) ),
    aFontTypeText   ( this, ResId( FT_FONTTYPE ) ),
    aExampleWin     ( this, ResId( WIN_EXAMPLE ) ),
    aExampleBox     ( this, ResId( GB_EXAMPLE ) ),
    aStrTransparent ( ResId( STR_TRANSPARENT ) ),
    aNoStyleStr     ( ResId( STR_NOSTYLE ) ),
    _pFontList      ( NULL ),
    eUnderline      ( UNDERLINE_NONE ),
    eStrikeout      ( STRIKEOUT_NONE ),
    bDeleteFontList ( FALSE ),
    bInSearchMode   ( FALSE ),
    nExtraEntryPos  ( LISTBOX_ENTRY_NOTFOUND )

{
    FreeResource();

    const SfxPoolItem* pItem;
    SfxObjectShell* pShell;
    if(SFX_ITEM_SET == rAttr.GetItemState(SID_HTML_MODE, FALSE, &pItem) ||
        ( 0 != ( pShell = SfxObjectShell::Current()) &&
                    0 != (pItem = pShell->GetItem(SID_HTML_MODE))))
    {
        USHORT nHtmlMode = ((const SfxUInt16Item*)pItem)->GetValue();
        if(nHtmlMode & HTMLMODE_ON)
        {
            aOutlineBtn     .Hide();
            aShadowBtn      .Hide();
            aUnderlineBtn   .SetPosPixel(aOutlineBtn .GetPosPixel());
            aStrikeoutBtn   .SetPosPixel(aShadowBtn  .GetPosPixel());
        }
    }

    // diese Page braucht ExchangeSupport
    SetExchangeSupport();

    SetHandler_Impl();

    // ColorBox aus der XColorTable fuellen.
    SfxObjectShell* pDocSh = SfxObjectShell::Current();
    DBG_ASSERT( pDocSh, "DocShell not found!" );
    XColorTable* pColorTable = NULL;
    FASTBOOL bKillTable = FALSE;

    if ( pDocSh && ( pItem = pDocSh->GetItem( SID_COLOR_TABLE ) ) )
        pColorTable = ( (SvxColorTableItem*)pItem )->GetColorTable();

    if ( !pColorTable )
    {
        pColorTable = new XColorTable( SvtPathOptions().GetPalettePath() );
        bKillTable = TRUE;
    }

    aColorBox.SetUpdateMode( FALSE );
    aColorBox.InsertEntry(Color(COL_AUTO), SVX_RESSTR( RID_SVXSTR_AUTOMATIC ));
    for ( long i = 0; i < pColorTable->Count(); i++ )
    {
        XColorEntry* pEntry = pColorTable->Get(i);
        aColorBox.InsertEntry( pEntry->GetColor(), pEntry->GetName() );
    }
    aColorBox.SetUpdateMode( TRUE );

    if( SFX_ITEM_SET == rAttr.GetItemState(SID_ATTR_CHAR_FILLCOLOR, FALSE ) )
    {
        aFillColorBox.SetUpdateMode( FALSE );
        aFillColorBox.CopyEntries( aColorBox );
        aFillColorBox.RemoveEntry(0);
        aFillColorBox.InsertEntry( aStrTransparent, 0 );
        aFillColorBox.SetUpdateMode( TRUE );
        aLanguageBox.Hide();
        aLanguageText.Hide();
        aFillColorBox.Show();
        aFillColorText.Show();
    }

    if ( bKillTable )
        delete pColorTable;

    aTimer.Start();
}

// -----------------------------------------------------------------------

SvxCharStdPage::~SvxCharStdPage()
{
    if( bDeleteFontList )
        delete (FontList*)_pFontList;
}

// -----------------------------------------------------------------------

USHORT* SvxCharStdPage::GetRanges()
{
    return pStdRanges;
}

// -----------------------------------------------------------------------

void SvxCharStdPage::Reset_Impl( ColorListBox& rColorBox,
    FixedText& rColorText, const SfxItemSet& rSet, USHORT nSlot )
{
    USHORT nWhich = GetWhich( nSlot );
    SfxItemState eState = rSet.GetItemState( nWhich );

    switch ( eState )
    {
        case SFX_ITEM_UNKNOWN:
            rColorText.Hide();
            rColorBox.Hide();
            break;

        case SFX_ITEM_DISABLED:
        case SFX_ITEM_READONLY:
            rColorText.Disable();
            rColorBox.Disable();
            break;

        case SFX_ITEM_DONTCARE:
            rColorBox.SetNoSelection();
            break;

        case SFX_ITEM_DEFAULT:
        case SFX_ITEM_SET:
        {
            const SvxColorItem& rItem = (SvxColorItem&)rSet.Get( nWhich );
            Color aColor = rItem.GetValue();
            USHORT nSelPos = rColorBox.GetEntryPos( aColor );
            if( nSelPos == LISTBOX_ENTRY_NOTFOUND &&
                aColor == Color(COL_TRANSPARENT) )
                nSelPos = rColorBox.GetEntryPos( aStrTransparent );

            if ( LISTBOX_ENTRY_NOTFOUND != nSelPos )
                rColorBox.SelectEntryPos( nSelPos );
            else
            {
                nSelPos = rColorBox.GetEntryPos( aColor );
                if ( LISTBOX_ENTRY_NOTFOUND != nSelPos )
                    rColorBox.SelectEntryPos( nSelPos );
                else
                    rColorBox.SelectEntryPos( rColorBox.InsertEntry(
                        aColor, String( SVX_RES( RID_SVXSTR_COLOR_USER ) ) ) );
            }
            break;
        }
    }
}

// -----------------------------------------------------------------------

BOOL SvxCharStdPage::FillItemSet_Impl( const ColorListBox& rColorBox,
    SfxItemSet& rSet, USHORT nSlot )
{
    USHORT nWhich = GetWhich( nSlot );
    const SvxColorItem* pOld = (const SvxColorItem*)GetOldItem( rSet, nSlot );
    const SvxColorItem* pItem = 0;
    BOOL bChanged = TRUE;
    const SfxItemSet* pExampleSet =
        GetTabDialog() ? GetTabDialog()->GetExampleSet() : NULL;
    const SfxItemSet& rOldSet = GetItemSet();

    Color aSelectedColor;
    if( rColorBox.GetSelectEntry() == aStrTransparent )
        aSelectedColor = Color( COL_TRANSPARENT );
    else
        aSelectedColor = rColorBox.GetSelectEntryColor();

    if ( pOld && pOld->GetValue() == aSelectedColor )
        bChanged = FALSE;

    if ( !bChanged )
        bChanged = ( rColorBox.GetSavedValue() == LISTBOX_ENTRY_NOTFOUND );

    if ( !bChanged && pExampleSet &&
        pExampleSet->GetItemState(nWhich, FALSE, (const SfxPoolItem**)&pItem) == SFX_ITEM_SET &&
        ( (SvxColorItem*)pItem)->GetValue() != aSelectedColor )
        bChanged = TRUE;

    BOOL bModified = FALSE;

    if ( bChanged && rColorBox.GetSelectEntryPos() != LISTBOX_ENTRY_NOTFOUND )
    {
        rSet.Put( SvxColorItem( aSelectedColor, nWhich ) );
        bModified = TRUE;
    }
    else if ( SFX_ITEM_DEFAULT == rOldSet.GetItemState( nWhich, FALSE ) )
        rSet.ClearItem( nWhich );

    return bModified;
}


// -----------------------------------------------------------------------

void SvxCharStdPage::ActivatePage( const SfxItemSet& rSet )
{
    SvxFont& rFont = aExampleWin.GetFont();

    // WordLineMode
    USHORT nWhich = GetWhich( SID_ATTR_CHAR_WORDLINEMODE );

    if ( rSet.GetItemState( nWhich ) == SFX_ITEM_SET )
    {
        const SvxWordLineModeItem& rItem =
            (SvxWordLineModeItem&)rSet.Get( nWhich );
        rFont.SetWordLineMode( rItem.GetValue() );
    }

    short       nEsc        = 0;
    BYTE        nEscProp    = 100;
    BYTE        nProp       = 100;
    SvxCaseMap  eCaseMap    = SVX_CASEMAP_NOT_MAPPED;

    // Proportionale Textgroesse
    nWhich = GetWhich( SID_ATTR_CHAR_PROPSIZE );

    if ( rSet.GetItemState( nWhich ) == SFX_ITEM_SET )
    {
        const SvxPropSizeItem& rItem = (SvxPropSizeItem&)rSet.Get( nWhich );
        nProp = (BYTE)rItem.GetValue();
    }

    // HochTief-Stellung
    nWhich = GetWhich( SID_ATTR_CHAR_ESCAPEMENT );

    if ( rSet.GetItemState( nWhich ) == SFX_ITEM_SET )
    {
        const SvxEscapementItem& rItem = (SvxEscapementItem&)rSet.Get( nWhich );
        nEsc = rItem.GetEsc();
        nEscProp = rItem.GetProp();
    }

    // BspFont setzen
    rFont.SetPropr( nProp );
    rFont.SetProprRel( nEscProp );
    rFont.SetEscapement( nEsc );

    // Auszeichnung
    nWhich = GetWhich( SID_ATTR_CHAR_CASEMAP );

    if ( rSet.GetItemState( nWhich ) == SFX_ITEM_SET )
    {
        const SvxCaseMapItem& rItem = (const SvxCaseMapItem&)rSet.Get( nWhich );
        eCaseMap = (SvxCaseMap)rItem.GetValue();
    }
    rFont.SetCaseMap( eCaseMap );

    // Kerning
    nWhich = GetWhich( SID_ATTR_CHAR_KERNING );

    if ( rSet.GetItemState( nWhich ) == SFX_ITEM_SET )
    {
        const SvxKerningItem& rItem = (SvxKerningItem&)rSet.Get( nWhich );
        MapUnit eUnit = (MapUnit)rSet.GetPool()->GetMetric( nWhich );
        long nKern = LogicToLogic( rItem.GetValue(), eUnit, MAP_TWIP );
        rFont.SetFixKerning( (short)nKern );
    }

    // Unterstrichen
    nWhich = GetWhich( SID_ATTR_CHAR_UNDERLINE );

    if ( rSet.GetItemState( nWhich ) == SFX_ITEM_SET )
    {
        const SvxUnderlineItem& rItem = (SvxUnderlineItem&)rSet.Get( nWhich );
        eUnderline = (FontUnderline)rItem.GetValue();
        rFont.SetUnderline( eUnderline );
        if ( eUnderline == UNDERLINE_NONE )
            aUnderlineBtn.SetState( STATE_NOCHECK );
    }

    // Durchgestrichen
    nWhich = GetWhich( SID_ATTR_CHAR_STRIKEOUT );

    if ( rSet.GetItemState( nWhich ) == SFX_ITEM_SET )
    {
        const SvxCrossedOutItem& rItem = (SvxCrossedOutItem&)rSet.Get( nWhich );
        eStrikeout = (FontStrikeout)rItem.GetValue();
        rFont.SetStrikeout( eStrikeout );
        if ( eStrikeout == STRIKEOUT_NONE )
            aStrikeoutBtn.SetState( STATE_NOCHECK );
    }

    // Hintergrundbrush
    nWhich = GetWhich( SID_ATTR_BRUSH );

    if ( rSet.GetItemState( nWhich ) == SFX_ITEM_SET )
    {
        const SvxBrushItem& rItem = (SvxBrushItem&)rSet.Get( nWhich );
        aExampleWin.SetColor( rItem.GetColor() );
    }

    UpdateExample_Impl();
}

// -----------------------------------------------------------------------

int SvxCharStdPage::DeactivatePage( SfxItemSet* pSet )
{
    if ( pSet )
        FillItemSet( *pSet );
    return LEAVE_PAGE;
}

// -----------------------------------------------------------------------

void SvxCharStdPage::SetHandler_Impl()
{
    // Handler jeweils fuer Update der Beispielbox
    aTimer.SetTimeout( 350 );
    aTimer.SetTimeoutHdl( LINK( this, SvxCharStdPage, TimeoutHdl_Impl ) );

    Link aModLink = LINK( this, SvxCharStdPage, BoxModifyHdl_Impl );
    aFontBox.SetModifyHdl( aModLink );
    aStyleBox.SetModifyHdl( aModLink );
    aSizeBox.SetModifyHdl( aModLink );

    aOutlineBtn.SetClickHdl(
        LINK( this, SvxCharStdPage, OutlineClickHdl_Impl ) );
    aShadowBtn.SetClickHdl(
        LINK( this, SvxCharStdPage, ShadowClickHdl_Impl ) );
    aUnderlineBtn.SetClickHdl(
        LINK( this, SvxCharStdPage, UnderlineClickHdl_Impl ) );
    aStrikeoutBtn.SetClickHdl(
        LINK( this, SvxCharStdPage, StrikeoutClickHdl_Impl ) );

    aColorBox.SetSelectHdl(
        LINK( this, SvxCharStdPage, ColorBoxSelectHdl_Impl ) );
    aFillColorBox.SetSelectHdl(
        LINK( this, SvxCharStdPage, ColorBoxSelectHdl_Impl ) );

    // initialisieren Sprachen
    const sal_Int32 nLanguageCount = SvxGetSelectableLanguages().getLength();
    const util::Language *pLang = SvxGetSelectableLanguages().getConstArray();
    for ( sal_Int32 i = 0; i < nLanguageCount; ++i )
    {
        LanguageType eType = pLang[i];
        USHORT nPos;

        nPos = aLanguageBox.InsertEntry( ::GetLanguageString( eType ) );
        aLanguageBox.SetEntryData( nPos, (void*)(ULONG)i );
    }
 }

// -----------------------------------------------------------------------

void SvxCharStdPage::UpdateExample_Impl()
{
    SvxFont& rFont = aExampleWin.GetFont();
    Size aSize = rFont.GetSize();
    aSize.Width() = 0;

    // Font
    const FontList* pFontList = GetFontList();
    FontInfo aFontInfo( pFontList->Get( aFontBox.GetText(),
                                        aStyleBox.GetText() ) );
    // Size
    if ( aSizeBox.IsRelative() )
    {
        DBG_ASSERT( GetItemSet().GetParent(), "No parent set" );

        USHORT nWhich = GetWhich( SID_ATTR_CHAR_FONTHEIGHT );
        const SvxFontHeightItem& rOldItem =
            (SvxFontHeightItem&)GetItemSet().GetParent()->Get( nWhich );

        // alter Wert, skaliert
        long nHeight;
        if( aSizeBox.IsPtRelative() )
            nHeight = rOldItem.GetHeight() +
                            PointToTwips( aSizeBox.GetValue() / 10 );
        else
            nHeight = rOldItem.GetHeight() * aSizeBox.GetValue() / 100;

        // Umrechnung in twips fuer das Beispiel-Window
        aSize.Height() = ItemToControl( nHeight,
            GetItemSet().GetPool()->GetMetric( nWhich ), SFX_FUNIT_TWIP );
    }
    else if ( aSizeBox.GetText().Len() )
        aSize.Height() = PointToTwips( aSizeBox.GetValue() / 10 );
    else
        aSize.Height() = 200;   // als Default 10pt
    aFontInfo.SetSize( aSize );

    rFont.SetFamily( aFontInfo.GetFamily() );
    rFont.SetName( aFontInfo.GetName() );
    rFont.SetStyleName( aFontInfo.GetStyleName() );
    rFont.SetPitch( aFontInfo.GetPitch() );
    rFont.SetCharSet( aFontInfo.GetCharSet() );
    rFont.SetWeight( aFontInfo.GetWeight() );
    rFont.SetItalic( aFontInfo.GetItalic() );
    rFont.SetSize( aFontInfo.GetSize() );

    aExampleWin.Invalidate();
    aFontTypeText.SetText( pFontList->GetFontMapText( aFontInfo ) );
}

// -----------------------------------------------------------------------

void SvxCharStdPage::FillStyleBox_Impl()
{
    const FontList* pFontList = GetFontList();
    DBG_ASSERT( pFontList, "no fontlist" );
    // "Uber FontInfo und Fontlist die StyleBox f"ullen
    aStyleBox.Fill( aFontBox.GetText(), pFontList );

    if ( bInSearchMode )
    {
        // Bei der Suche zus"atzliche Eintr"age:
        // "Nicht Fett" und "Nicht Kursiv"
        String aEntry = aNoStyleStr;
        const sal_Char sS[] = "%s";
        aEntry.SearchAndReplaceAscii( sS, pFontList->GetBoldStr() );
        nExtraEntryPos = aStyleBox.InsertEntry( aEntry );
        aEntry = aNoStyleStr;
        aEntry.SearchAndReplaceAscii( sS, pFontList->GetItalicStr() );
        aStyleBox.InsertEntry( aEntry );
    }
}

// -----------------------------------------------------------------------

void SvxCharStdPage::FillSizeBox_Impl()
{
    const FontList* pFontList = GetFontList();
    // Ueber FontInfo und Fontlist Size fuellen
    aSizeBox.Fill( pFontList->Get( aFontBox.GetText(), aStyleBox.GetText() ),
                   pFontList );
}

// -----------------------------------------------------------------------

IMPL_LINK_INLINE_START( SvxCharStdPage, TimeoutHdl_Impl, Timer *, EMPTYARG )
{
    UpdateExample_Impl();
    return 0;
}
IMPL_LINK_INLINE_END( SvxCharStdPage, TimeoutHdl_Impl, Timer *, EMPTYARG )

// -----------------------------------------------------------------------

IMPL_LINK( SvxCharStdPage, BoxModifyHdl_Impl, void *, pBox )
{
    aTimer.Start();

    if ( &aFontBox == pBox )
    {
        FillStyleBox_Impl();
        FillSizeBox_Impl();
    }
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK_INLINE_START( SvxCharStdPage, OutlineClickHdl_Impl, TriStateBox *, pBtn )
{
    pBtn->EnableTriState( FALSE );
    SvxFont& rFont = aExampleWin.GetFont();
    rFont.SetOutline( StateToAttr( pBtn->GetState() ) );
    aExampleWin.Invalidate();
    return 0;
}
IMPL_LINK_INLINE_END( SvxCharStdPage, OutlineClickHdl_Impl, TriStateBox *, pBtn )

// -----------------------------------------------------------------------

IMPL_LINK_INLINE_START( SvxCharStdPage, ShadowClickHdl_Impl, TriStateBox *, pBtn )
{
    pBtn->EnableTriState( FALSE );
    SvxFont& rFont = aExampleWin.GetFont();
    rFont.SetShadow( StateToAttr( pBtn->GetState() ) );
    aExampleWin.Invalidate();
    return 0;
}
IMPL_LINK_INLINE_END( SvxCharStdPage, ShadowClickHdl_Impl, TriStateBox *, pBtn )

// -----------------------------------------------------------------------

IMPL_LINK( SvxCharStdPage, UnderlineClickHdl_Impl, TriStateBox *, pBtn )
{
    pBtn->EnableTriState( FALSE );
    SvxFont& rFont = aExampleWin.GetFont();
    eUnderline = StateToAttr( pBtn->GetState() ) == TRUE ? UNDERLINE_SINGLE
                                                         : UNDERLINE_NONE;
    rFont.SetUnderline( eUnderline );
    UpdateExample_Impl();
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxCharStdPage, StrikeoutClickHdl_Impl, TriStateBox *, pBtn )
{
    pBtn->EnableTriState( FALSE );
    SvxFont& rFont = aExampleWin.GetFont();
    eStrikeout = StateToAttr( pBtn->GetState() ) == TRUE ? STRIKEOUT_SINGLE
                                                         : STRIKEOUT_NONE;
    rFont.SetStrikeout( eStrikeout );
    UpdateExample_Impl();
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK_INLINE_START( SvxCharStdPage, ColorBoxSelectHdl_Impl, ColorListBox *, pBox )
{
    SvxFont& rFont = aExampleWin.GetFont();
    Color aSelectedColor;
    if( pBox->GetSelectEntry() == aStrTransparent )
        aSelectedColor = Color( COL_TRANSPARENT );
    else
        aSelectedColor = pBox->GetSelectEntryColor();

    if( pBox == &aColorBox )
        rFont.SetColor( aSelectedColor );
    else
        rFont.SetFillColor( aSelectedColor );
    UpdateExample_Impl();
    return 0;
}
IMPL_LINK_INLINE_END( SvxCharStdPage, ColorBoxSelectHdl_Impl, ColorListBox *, pBox )

// class SvxCharExtPage --------------------------------------------------

SfxTabPage* SvxCharExtPage::Create( Window* pParent, const SfxItemSet& rSet )
{
    return new SvxCharExtPage( pParent, rSet );
}

// -----------------------------------------------------------------------

BOOL SvxCharExtPage::FillItemSet( SfxItemSet& rSet )
{
    const SfxPoolItem* pOld = 0;
    const SfxItemSet& rOldSet = GetItemSet();
    BOOL bModified = FALSE;
    FASTBOOL bChanged = TRUE;

    // Wortweise
    USHORT nWhich = GetWhich( SID_ATTR_CHAR_WORDLINEMODE );
    pOld = GetOldItem( rSet, SID_ATTR_CHAR_WORDLINEMODE );

    if ( pOld )
    {
        const SvxWordLineModeItem& rItem = *( (const SvxWordLineModeItem*)pOld );

        if ( rItem.GetValue() == aWordBox.IsChecked() )
            bChanged = FALSE;
    }

    if ( rOldSet.GetItemState( nWhich ) == SFX_ITEM_DONTCARE &&
         aWordBox.IsChecked() == aWordBox.GetSavedValue() )
        bChanged = FALSE;

    if ( bChanged )
    {
        rSet.Put( SvxWordLineModeItem( aWordBox.IsChecked(), nWhich ) );
        bModified |= TRUE;
    }
    else if ( SFX_ITEM_DEFAULT == rOldSet.GetItemState( nWhich, FALSE ) )
        rSet.ClearItem( nWhich );
    bChanged = TRUE;

    // Auszeichnung
    nWhich = GetWhich( SID_ATTR_CHAR_CASEMAP );
    pOld = GetOldItem( rSet, SID_ATTR_CHAR_CASEMAP );
    SvxCaseMap eCaseMap = SVX_CASEMAP_NOT_MAPPED;
    FASTBOOL bChecked = FALSE;

#if ( SUPD < 590 )
    if ( aCapsBtn.IsChecked() )
    {
        eCaseMap = SVX_CASEMAP_VERSALIEN;
        bChecked = TRUE;
    }
    else if ( aLowercaseBtn.IsChecked() )
    {
        eCaseMap = SVX_CASEMAP_GEMEINE;
        bChecked = TRUE;
    }
    else if ( aSmallcapsBtn.IsChecked() )
    {
        eCaseMap = SVX_CASEMAP_KAPITAELCHEN;
        bChecked = TRUE;
    }
    else if ( aTitelBtn.IsChecked() )
    {
        eCaseMap = SVX_CASEMAP_TITEL;
        bChecked = TRUE;
    }
    else if ( aNoneBtn.IsChecked() )
        bChecked = TRUE;
#else
    USHORT nCapsPos = aCapsLB.GetSelectEntryPos();
    if ( nCapsPos != LISTBOX_ENTRY_NOTFOUND )
    {
        eCaseMap = (SvxCaseMap)nCapsPos;
        bChecked = TRUE;
    }
#endif

    if ( pOld )
    {
        const SvxCaseMapItem& rItem = *( (const SvxCaseMapItem*)pOld );

        if ( (SvxCaseMap)rItem.GetValue() == eCaseMap )
            bChanged = FALSE;
    }

#if ( SUPD < 590 )
    if ( !bChanged && !aNoneBtn.GetSavedValue() &&
         !aCapsBtn.GetSavedValue() && !aLowercaseBtn.GetSavedValue() &&
         !aSmallcapsBtn.GetSavedValue() && !aTitelBtn.GetSavedValue() )
        bChanged = TRUE;
#endif

    if ( bChanged && bChecked )
    {
        rSet.Put( SvxCaseMapItem( eCaseMap, nWhich ) );
        bModified |= TRUE;
    }
    else if ( SFX_ITEM_DEFAULT == rOldSet.GetItemState( nWhich, FALSE ) )
        rSet.ClearItem( nWhich );
    bChanged = TRUE;

    //  Position (hoch, normal oder tief)
    nWhich = GetWhich( SID_ATTR_CHAR_ESCAPEMENT );
    pOld = GetOldItem( rSet, SID_ATTR_CHAR_ESCAPEMENT );
    const FASTBOOL bHigh = aHighBtn.IsChecked();
    short nEsc;
    BYTE  nEscProp;

    if ( bHigh || aDeepBtn.IsChecked() )
    {
        if(aAutoPosBox.IsChecked())
        {
            nEsc = bHigh ? DFLT_ESC_AUTO_SUPER : DFLT_ESC_AUTO_SUB;
        }
        else
        {
            nEsc = (short)aEscRelPosEdit.Denormalize( aEscRelPosEdit.GetValue() );
            nEsc *= bHigh ? 1 : -1;
        }
        nEscProp = (BYTE)aEscRelSizeEdit.Denormalize(
            aEscRelSizeEdit.GetValue() );
    }
    else
    {
        nEsc  = 0;
        nEscProp = 100;
    }

    if ( pOld )
    {
        const SvxEscapementItem& rItem = *( (const SvxEscapementItem*)pOld );

        // Wert gleich geblieben oder kein Button gecheckt?
        if ( rItem.GetEsc() == nEsc && rItem.GetProp() == nEscProp  )
            bChanged = FALSE;
    }

    if ( !bChanged && !aHighBtn.GetSavedValue() &&
         !aNormalBtn.GetSavedValue() && !aDeepBtn.GetSavedValue() )
        bChanged = TRUE;

    if ( bChanged &&
         ( aHighBtn.IsChecked()     ||
           aNormalBtn.IsChecked()   ||
           aDeepBtn.IsChecked() ) )
    {
        rSet.Put( SvxEscapementItem( nEsc, nEscProp, nWhich ) );
        bModified |= TRUE;
    }
    else if ( SFX_ITEM_DEFAULT == rOldSet.GetItemState( nWhich, FALSE ) )
        rSet.ClearItem( nWhich );
    bChanged = TRUE;

    // Kerning
    nWhich = GetWhich( SID_ATTR_CHAR_KERNING );
    pOld = GetOldItem( rSet, SID_ATTR_CHAR_KERNING );
    USHORT nPos = aKerningListBox.GetSelectEntryPos();
    short nKerning = 0;
    SfxMapUnit eUnit = rSet.GetPool()->GetMetric( nWhich );

    if ( nPos == LW_GESPERRT || nPos == LW_SCHMAL )
    {
        long nTmp = aKerningEdit.GetValue();
        long nVal = LogicToLogic( nTmp, MAP_POINT, (MapUnit)eUnit );
        nKerning = (short)aKerningEdit.Denormalize( nVal );

        if ( nPos == LW_SCHMAL )
            nKerning *= - 1;
    }

    if ( pOld )
    {
        const SvxKerningItem& rItem = *( (const SvxKerningItem*)pOld );

        if ( rItem.GetValue() == nKerning )
            bChanged = FALSE;
    }

    if ( !bChanged &&
         ( aKerningListBox.GetSavedValue() == LISTBOX_ENTRY_NOTFOUND ||
           !aKerningEdit.GetSavedValue().Len() ) )
        bChanged = TRUE;

    if ( bChanged && nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        rSet.Put( SvxKerningItem( nKerning, nWhich ) );
        bModified |= TRUE;
    }
    else if ( SFX_ITEM_DEFAULT == rOldSet.GetItemState( nWhich, FALSE ) )
        rSet.ClearItem( nWhich );
    bChanged = TRUE;

    // Pair-Kerning
    nWhich = GetWhich( SID_ATTR_CHAR_AUTOKERN );

    if ( aAutoKernBtn.IsChecked() != aAutoKernBtn.GetSavedValue() )
    {
        rSet.Put( SvxAutoKernItem( aAutoKernBtn.IsChecked(), nWhich ) );
        bModified |= TRUE;
    }
    else if ( SFX_ITEM_DEFAULT == rOldSet.GetItemState( nWhich, FALSE ) )
        rSet.ClearItem( nWhich );

    // Unterstrichen
    nWhich = GetWhich( SID_ATTR_CHAR_UNDERLINE );
    pOld = GetOldItem( rSet, SID_ATTR_CHAR_UNDERLINE );
    nPos = aUnderlineBox.GetSelectEntryPos();
    FontUnderline eUnder = (FontUnderline)(ULONG)aUnderlineBox.GetEntryData( nPos );

    if ( pOld )
    {
        const SvxUnderlineItem& rItem = *( (const SvxUnderlineItem*)pOld );

        if ( !aUnderlineBox.IsEnabled() ||
             (FontUnderline)rItem.GetValue() == eUnder )
            bChanged = FALSE;
    }

    FASTBOOL bTmp = FALSE;
    SfxTabDialog* pDlg = GetTabDialog();
    const SfxItemSet* pExSet = NULL;

    if ( pDlg )
        pExSet = pDlg->GetExampleSet();

    if ( aUnderlineBox.IsEnabled() )
    {
        if ( bChanged )
        {
            const SvxUnderlineItem* pTmp = NULL;
            SfxItemState eState = SFX_ITEM_UNKNOWN;

            if ( pExSet )
                eState = pExSet->GetItemState( nWhich, TRUE, (const SfxPoolItem**)&pTmp );

            if ( pTmp && pTmp->GetUnderline() != UNDERLINE_NONE )
                bTmp = TRUE;
            else if ( !pTmp )
            {
                pTmp = (SvxUnderlineItem*)GetItem( rOldSet, SID_ATTR_CHAR_UNDERLINE );
                if ( pTmp && pTmp->GetUnderline() != UNDERLINE_NONE )
                    bTmp = TRUE;
            }

            if ( bTmp || SFX_ITEM_DONTCARE == eState )
            {
                rSet.Put( SvxUnderlineItem( eUnder, nWhich ) );
                bModified |= TRUE;
            }
        }

        if ( ( !bChanged || !bTmp ) &&
             SFX_ITEM_DEFAULT == rOldSet.GetItemState( nWhich, FALSE ) )
            rSet.ClearItem( nWhich );
    }
    bChanged = TRUE;

    // Durchgestrichen
    nWhich = GetWhich( SID_ATTR_CHAR_STRIKEOUT );
    pOld = GetOldItem( rSet, SID_ATTR_CHAR_UNDERLINE );
    nPos = aStrikeoutBox.GetSelectEntryPos();
    FontStrikeout eStrike = (FontStrikeout)(ULONG)aStrikeoutBox.GetEntryData( nPos );

    if ( pOld )
    {
        const SvxCrossedOutItem& rItem = *( (const SvxCrossedOutItem*)pOld );

        if ( !aStrikeoutBox.IsEnabled() || (FontStrikeout)rItem.GetValue() == eStrike )
            bChanged = FALSE;
    }

    bTmp = FALSE;

    if ( aStrikeoutBox.IsEnabled() )
    {
        if ( bChanged )
        {
            const SvxCrossedOutItem* pTmp = NULL;
            SfxItemState eState = SFX_ITEM_UNKNOWN;

            if ( pExSet )
                eState = pExSet->GetItemState( nWhich, TRUE, (const SfxPoolItem**)&pTmp );

            if ( pTmp && pTmp->GetStrikeout() != STRIKEOUT_NONE )
                bTmp = TRUE;
            else if ( !pTmp )
            {
                pTmp = (SvxCrossedOutItem*)GetItem( rOldSet, SID_ATTR_CHAR_STRIKEOUT );
                if ( pTmp && pTmp->GetStrikeout() != STRIKEOUT_NONE )
                    bTmp = TRUE;
            }

            if ( bTmp || SFX_ITEM_DONTCARE == eState )
            {
                rSet.Put( SvxCrossedOutItem( eStrike, nWhich ) );
                bModified |= TRUE;
            }
        }

        if ( ( !bChanged || !bTmp ) &&
             SFX_ITEM_DEFAULT == rOldSet.GetItemState( nWhich, FALSE ) )
            rSet.ClearItem( nWhich );
    }

    //blinken

    if ( aFlashBox.IsChecked() != aFlashBox.GetSavedValue())
    {
        nWhich = GetWhich( SID_ATTR_FLASH );
        rSet.Put(SvxBlinkItem(aFlashBox.IsChecked(), nWhich));
        bModified |= TRUE;
    }

    return bModified;
}

// -----------------------------------------------------------------------

void SvxCharExtPage::Reset( const SfxItemSet& rSet )
{
    bInReset = TRUE;
    String sUser = GetUserData();

    if ( sUser.Len() )
    {
        nSuperEsc = (short)sUser.GetToken( 0 ).ToInt32();
        nSubEsc = (short)sUser.GetToken( 1 ).ToInt32();
        nSuperProp = (BYTE)sUser.GetToken( 2 ).ToInt32();
        nSubProp = (BYTE)sUser.GetToken( 3 ).ToInt32();
    }
    ActivatePage( rSet );

    SvxFont& rFont = aExampleWin.GetFont();
    USHORT nWhich;

    // WordLineMode
    nWhich = GetWhich( SID_ATTR_CHAR_WORDLINEMODE );

    if ( rSet.GetItemState( nWhich ) >= SFX_ITEM_DEFAULT )
    {
        const SvxWordLineModeItem& rItem =
            (SvxWordLineModeItem&)rSet.Get( nWhich );
        rFont.SetWordLineMode( rItem.GetValue() );
        aWordBox.Check( rItem.GetValue() );
    }

    short       nEsc        = 0;
    BYTE        nEscProp    = 100;
    BYTE        nProp       = 100;

    aEscRelPosEdit.Disable();
    aEscRelPosText.Disable();
    aEscRelSizeEdit.Disable();
    aEscRelSizeText.Disable();

    // HochTief-Stellung
    nWhich = GetWhich( SID_ATTR_CHAR_ESCAPEMENT );

    if ( rSet.GetItemState( nWhich ) >= SFX_ITEM_DEFAULT )
    {
        const SvxEscapementItem& rItem = (SvxEscapementItem&)rSet.Get( nWhich );
        nEsc = rItem.GetEsc();
        nEscProp = rItem.GetProp();

        if ( nEsc != 0 )
        {
            aEscRelPosEdit.Enable();
            aEscRelPosText.Enable();
            aEscRelSizeEdit.Enable();
            aEscRelSizeText.Enable();

            short nFac;

            if ( nEsc > 0 )
            {
                nFac = 1;
                aHighBtn.Check( TRUE );
                if(nEsc == DFLT_ESC_AUTO_SUPER)
                {
                    aAutoPosBox.Check(TRUE);
                    nEsc = DFLT_ESC_SUPER;
                }
            }
            else
            {
                nFac = -1;
                aDeepBtn.Check( TRUE );
                if(nEsc == DFLT_ESC_AUTO_SUB)
                {
                    aAutoPosBox.Check(TRUE);
                    nEsc = DFLT_ESC_SUB;

                }
            }
            if(aAutoPosBox.IsChecked())
            {
                aEscRelPosText .Disable();
                aEscRelPosEdit .Disable();
            }
            aEscRelPosEdit.SetValue( aEscRelPosEdit.Normalize( nFac * nEsc ) );
            aEscRelSizeEdit.SetValue( aEscRelSizeEdit.Normalize( nEscProp ) );
        }
        else
        {
            aNormalBtn.Check( TRUE );
            aAutoPosBox.Check( TRUE );
            EscapementHdl_Impl( 0 );
        }
    }
    else
    {
        aHighBtn.Check( FALSE );
        aNormalBtn.Check( FALSE );
        aDeepBtn.Check( FALSE );
    }

    // BspFont setzen
    rFont.SetPropr(nProp);
    rFont.SetProprRel(nEscProp);
    rFont.SetEscapement(nEsc);

    // Auszeichnung
    //
    nWhich = GetWhich( SID_ATTR_CHAR_CASEMAP );
    SvxCaseMap eCaseMap = SVX_CASEMAP_END;  // dont care

    if ( rSet.GetItemState( nWhich ) >= SFX_ITEM_DEFAULT )
    {
        const SvxCaseMapItem& rItem = (const SvxCaseMapItem&)rSet.Get( nWhich );
        eCaseMap = (SvxCaseMap)rItem.GetValue();
    }
    SetCaseMap_Impl( eCaseMap );

    // Kerning
    //
    nWhich = GetWhich( SID_ATTR_CHAR_KERNING );

    if ( rSet.GetItemState( nWhich ) >= SFX_ITEM_DEFAULT )
    {
        const SvxKerningItem& rItem = (SvxKerningItem&)rSet.Get( nWhich );
        SfxMapUnit eUnit = rSet.GetPool()->GetMetric( nWhich );
        MapUnit eOrgUnit = (MapUnit)eUnit;
        MapUnit ePntUnit( MAP_POINT );
        long nBig = aKerningEdit.Normalize( (long)rItem.GetValue() );
        long nKerning = LogicToLogic( nBig, eOrgUnit, ePntUnit );

        // Kerning am Font setzen, vorher in Twips umrechnen
        long nKern =
            LogicToLogic( rItem.GetValue(), (MapUnit)eUnit, MAP_TWIP );
        rFont.SetFixKerning( (short)nKern );

        if ( nKerning > 0 )
        {
            aKerningEdit.Enable();
            aKerningEdit.SetValue( nKerning );
            aKerningListBox.SelectEntryPos( LW_GESPERRT );
        }
        else if ( nKerning < 0 )
        {
            aKerningText.Enable();
            aKerningEdit.Enable();
            aKerningEdit.SetValue( -nKerning );
            aKerningListBox.SelectEntryPos( LW_SCHMAL );
            // beim schmalen Kerning den Maximal-Wert auf
            // 1/6 der Font-Hoehe setzen
            long nMax = rFont.GetSize().Height() / 6;
            aKerningEdit.SetMax( aKerningEdit.Normalize( nMax ), FUNIT_TWIP );
            aKerningEdit.SetLast(
                aKerningEdit.GetMax( aKerningEdit.GetUnit() ) );
        }
        else
        {
            aKerningText.Disable();
            aKerningEdit.Disable();
            aKerningEdit.SetValue( 0 );
            aKerningListBox.SelectEntryPos( LW_NORMAL );
            aKerningEdit.SetMax( 9999 );
            aKerningEdit.SetLast( 9999 );
        }
    }
    else
        aKerningEdit.SetText( String() );

    nWhich = GetWhich( SID_ATTR_CHAR_AUTOKERN );

    if ( rSet.GetItemState( nWhich ) >= SFX_ITEM_DEFAULT )
    {
        const SvxAutoKernItem& rItem = (SvxAutoKernItem&)rSet.Get( nWhich );
        aAutoKernBtn.Check( rItem.GetValue() );
    }
    else
        aAutoKernBtn.Check( FALSE );

    FASTBOOL bWord = FALSE;

    // Underline
    nWhich = GetWhich( SID_ATTR_CHAR_UNDERLINE );

    rFont.SetUnderline( UNDERLINE_NONE );
    aUnderlineBox.SelectEntryPos( 0 );
    aUnderlineBox.Disable();
    aUnderlineText.Disable();
    SfxItemState eState = rSet.GetItemState( nWhich );

    if ( eState >= SFX_ITEM_DONTCARE )
    {
        if ( eState == SFX_ITEM_DONTCARE )
        {
            aUnderlineBox.Enable();
            aUnderlineBox.SetNoSelection();
            aUnderlineText.Enable();
        }
        else
        {
            const SvxUnderlineItem& rItem = (SvxUnderlineItem&)rSet.Get( nWhich );
            FontUnderline eUnderline = (FontUnderline)rItem.GetValue();
            rFont.SetUnderline( eUnderline );

            if ( eUnderline != UNDERLINE_NONE )
            {
                aUnderlineBox.Enable();
                aUnderlineText.Enable();

                for ( USHORT i = 0; i < aUnderlineBox.GetEntryCount(); ++i )
                {
                    if ( (FontUnderline)(ULONG)aUnderlineBox.GetEntryData(i) == eUnderline )
                    {
                        aUnderlineBox.SelectEntryPos(i);
                        break;
                    }
                }
                bWord |= TRUE;
            }
        }
    }

    //  Strikeout
    nWhich = GetWhich( SID_ATTR_CHAR_STRIKEOUT );

    rFont.SetStrikeout( STRIKEOUT_NONE );
    aStrikeoutBox.SelectEntryPos( 0 );
    aStrikeoutBox.Disable();
    aStrikeoutText.Disable();
    eState = rSet.GetItemState( nWhich );

    if ( eState >= SFX_ITEM_DONTCARE )
    {
        if ( eState == SFX_ITEM_DONTCARE )
        {
            aStrikeoutBox.Enable();
            aStrikeoutBox.SetNoSelection();
            aStrikeoutText.Enable();
        }
        else
        {
            const SvxCrossedOutItem& rItem = (SvxCrossedOutItem&)rSet.Get( nWhich );
            FontStrikeout eStrikeout = (FontStrikeout)rItem.GetValue();
            rFont.SetStrikeout( eStrikeout );

            if ( eStrikeout != STRIKEOUT_NONE )
            {
                aStrikeoutBox.Enable();
                aStrikeoutText.Enable();

                for ( USHORT i = 0; i < aStrikeoutBox.GetEntryCount(); ++i )
                {
                    if ( (FontStrikeout)(ULONG)aStrikeoutBox.GetEntryData(i) == eStrikeout )
                    {
                        aStrikeoutBox.SelectEntryPos(i);
                        break;
                    }
                }
                bWord |= TRUE;
            }
        }
    }

    if ( !bWord )
        aWordBox.Disable();
    else
        aWordBox.Enable();

    //blinken
    nWhich = GetWhich( SID_ATTR_FLASH );
    if ( rSet.GetItemState( nWhich ) >= SFX_ITEM_DEFAULT )
    {
        const SvxBlinkItem& rItem = (SvxBlinkItem&)rSet.Get(nWhich);
        aFlashBox.Check(rItem.GetValue());
    }

    aExampleWin.Invalidate();

    // Alte Einstellungen merken
#if ( SUPD < 590 )
    aNoneBtn.SaveValue();
    aCapsBtn.SaveValue();
    aLowercaseBtn.SaveValue();
    aSmallcapsBtn.SaveValue();
    aTitelBtn.SaveValue();
#else
    aCapsLB.SaveValue();
#endif
    aHighBtn.SaveValue();
    aNormalBtn.SaveValue();
    aDeepBtn.SaveValue();
    aEscRelPosEdit.SaveValue();
    aEscRelSizeEdit.SaveValue();
    aKerningListBox.SaveValue();
    aKerningEdit.SaveValue();
    aAutoKernBtn.SaveValue();
    aUnderlineBox.SaveValue();
    aStrikeoutBox.SaveValue();
    aWordBox.SaveValue();
    aFlashBox.SaveValue();
    bInReset = FALSE;
}

// -----------------------------------------------------------------------

void SvxCharExtPage::FillUserData()
{
    const sal_Unicode cTok = ';';

    String sUser( UniString::CreateFromInt32( nSuperEsc ) );
    sUser.Append( cTok );
    sUser.Append( UniString::CreateFromInt32( nSubEsc ) );
    sUser.Append( cTok );
    sUser.Append( UniString::CreateFromInt32( nSuperProp ) );
    sUser.Append( cTok );
    sUser.Append( UniString::CreateFromInt32( nSubProp ) );
    SetUserData( sUser );
}

// -----------------------------------------------------------------------

void SvxCharExtPage::DisableControls( USHORT nDisable )
{
    if ( DISABLE_CASEMAP & nDisable )
    {
#if ( SUPD < 590 )
        aNoneBtn.Disable();
        aCapsBtn.Disable();
        aLowercaseBtn.Disable();
        aSmallcapsBtn.Disable();
        aTitelBtn.Disable();
        aEffectBox.Disable();
#else
        aCapsLB.Disable();
#endif
    }

    if ( DISABLE_WORDLINE & nDisable )
        aWordBox.Hide();

    if ( DISABLE_BLINK & nDisable )
        aFlashBox.Disable();
}
// -----------------------------------------------------------------------

void SvxCharExtPage::EnableFlash()
{
    if(!((nHtmlMode & HTMLMODE_ON) && !(nHtmlMode & HTMLMODE_BLINK)))
        aFlashBox.Show();
}

// -----------------------------------------------------------------------

SvxCharExtPage::SvxCharExtPage( Window* pParent, const SfxItemSet& rAttr ):

    SfxTabPage( pParent, SVX_RES( RID_SVXPAGE_CHAR_EXT ), rAttr ),

#if ( SUPD < 590 )
    aNoneBtn        ( this, ResId( BTN_NONE ) ),
    aCapsBtn        ( this, ResId( BTN_CAPS ) ),
    aLowercaseBtn   ( this, ResId( BTN_LOWERCASE ) ),
    aSmallcapsBtn   ( this, ResId( BTN_SMALLCAPS ) ),
    aTitelBtn       ( this, ResId( BTN_TITEL ) ),
#else
    aCapsLB         ( this, ResId( LB_CAPS ) ),
#endif
    aFlashBox       ( this, ResId( CB_FLASH ) ),
    aEffectBox      ( this, ResId( GB_EFFECT ) ),
    aHighBtn        ( this, ResId( BTN_HIGH ) ),
    aNormalBtn      ( this, ResId( BTN_NORMAL ) ),
    aDeepBtn        ( this, ResId( BTN_DEEP ) ),
    aPositionBox    ( this, ResId( GB_POSITION ) ),
    aEscRelPosText  ( this, ResId( FT_ESCDEEPHIGH ) ),
    aEscRelPosEdit  ( this, ResId( ED_ESCDEEPHIGH ) ),
    aEscRelSizeText ( this, ResId( FT_ESCRELSIZE ) ),
    aEscRelSizeEdit ( this, ResId( ED_ESCRELSIZE ) ),
    aAutoPosBox     ( this, ResId( CB_AUTOPOS ) ),
    aPosExtBox      ( this, ResId( GB_POSEXT ) ),
    aKerningListBox ( this, ResId( LB_KERNING ) ),
    aKerningText    ( this, ResId( FT_KERNING ) ),
    aKerningEdit    ( this, ResId( ED_KERNING ) ),
    aAutoKernBtn    ( this, ResId( BTN_AUTOKERN ) ),
    aKerningBox     ( this, ResId( GB_KERNING ) ),
    aUnderlineText  ( this, ResId( FT_UNDERLINECNT ) ),
    aUnderlineBox   ( this, ResId( LB_UNDERLINECNT ) ),
    aStrikeoutText  ( this, ResId( FT_STRIKEOUTCNT ) ),
    aStrikeoutBox   ( this, ResId( LB_STRIKEOUTCNT ) ),
    aWordBox        ( this, ResId( CB_WORD ) ),
    aExampleWin     ( this, ResId( WIN_EXAMPLE ) ),
    aExampleBox     ( this, ResId( GB_EXAMPLE ) ),

    bInReset        ( FALSE ),
    nSuperEsc       ( (short)DFLT_ESC_SUPER ),
    nSubEsc         ( (short)DFLT_ESC_SUB ),
    nSuperProp      ( (BYTE)DFLT_ESC_PROP ),
    nSubProp        ( (BYTE)DFLT_ESC_PROP ),
    nHtmlMode       (0)

{
    FreeResource();

    const SfxPoolItem* pItem;
    SfxObjectShell* pShell;
    if ( SFX_ITEM_SET == rAttr.GetItemState( SID_HTML_MODE, FALSE, &pItem ) ||
         ( 0 != ( pShell = SfxObjectShell::Current() ) &&
           0 != ( pItem = pShell->GetItem( SID_HTML_MODE ) ) ) )
    {
        nHtmlMode = (BOOL)( (const SfxUInt16Item*)pItem )->GetValue();
        if ( nHtmlMode & HTMLMODE_ON )
        {
            aStrikeoutText   .Hide();
            aStrikeoutBox    .Hide();
            aPosExtBox       .Hide();
            aAutoPosBox      .Hide();
            aEscRelPosText   .Hide();
            aEscRelPosEdit   .Hide();
            aEscRelSizeText  .Hide();
            aEscRelSizeEdit  .Hide();
#if ( SUPD < 590 )
            aCapsBtn         .Hide();
            aLowercaseBtn    .Hide();
            aTitelBtn        .Hide();
#endif
            aUnderlineText   .Hide();
            aUnderlineBox    .Hide();
            aWordBox         .Hide();

            if ( !( nHtmlMode & HTMLMODE_FULL_STYLES ) )
            {
                aKerningListBox.Hide();
                aKerningText   .Hide();
                aKerningEdit   .Hide();
                aAutoKernBtn   .Hide();
                aKerningBox    .Hide();
            }
            if ( !( nHtmlMode & HTMLMODE_BLINK ) )
                aFlashBox.Hide();
#if ( SUPD < 590 )
            else
                aFlashBox.SetPosPixel( aLowercaseBtn.GetPosPixel() );
            if ( nHtmlMode & HTMLMODE_SMALL_CAPS )
                aSmallcapsBtn.SetPosPixel( aCapsBtn.GetPosPixel() );
            else
                aSmallcapsBtn.Hide();
#endif
            Point aPos = aPosExtBox.GetPosPixel();
            Point aXPos = aExampleBox.GetPosPixel();
            int nYDiff = aXPos.Y() - aPos.Y();
            aExampleBox.SetPosPixel(aPos);
            aXPos = aExampleWin.GetPosPixel();
            aXPos.Y() -= nYDiff;
            aExampleWin.SetPosPixel(aXPos);
        }
    }

    // diese Page braucht ExchangeSupport
    SetExchangeSupport();

    SetHandler_Impl();
}

// -----------------------------------------------------------------------

USHORT* SvxCharExtPage::GetRanges()
{
    return pExtRanges;
}

// -----------------------------------------------------------------------

void SvxCharExtPage::ActivatePage( const SfxItemSet& rSet )
{
    SfxItemState eState = bInReset ? SFX_ITEM_DEFAULT : SFX_ITEM_SET;
    SvxFont& rFont = aExampleWin.GetFont();
    const SvxFontItem* pFontItem = 0;
    USHORT nWhich = GetWhich( SID_ATTR_CHAR_FONT );

    if ( rSet.GetItemState( nWhich ) >= eState )
    {
        pFontItem = (const SvxFontItem*)&rSet.Get( nWhich );
        rFont.SetFamily( pFontItem->GetFamily() );
        rFont.SetName( pFontItem->GetFamilyName() );
        rFont.SetPitch( pFontItem->GetPitch() );
        rFont.SetCharSet( pFontItem->GetCharSet() );
        rFont.SetStyleName( pFontItem->GetStyleName() );
    }

    nWhich = GetWhich( SID_ATTR_CHAR_POSTURE );

    if ( rSet.GetItemState( nWhich ) >= eState )
    {
        const SvxPostureItem& rItem = (SvxPostureItem&)rSet.Get( nWhich );
        rFont.SetItalic( (FontItalic)rItem.GetValue() != ITALIC_NONE ?
            ITALIC_NORMAL : ITALIC_NONE );
    }

    nWhich = GetWhich( SID_ATTR_CHAR_WEIGHT );

    if ( rSet.GetItemState( nWhich ) >= eState )
    {
        SvxWeightItem& rItem = (SvxWeightItem&)rSet.Get( nWhich );
        rFont.SetWeight( (FontWeight)rItem.GetValue() != WEIGHT_NORMAL ?
            WEIGHT_BOLD : WEIGHT_NORMAL );
    }

    nWhich = GetWhich( SID_ATTR_CHAR_FONTHEIGHT );

    if ( rSet.GetItemState( nWhich ) >= eState )
    {
        MapUnit eUnit = (MapUnit)rSet.GetPool()->GetMetric( nWhich );
        SvxFontHeightItem& rItem = (SvxFontHeightItem&)rSet.Get( nWhich );

        Size aSize( rFont.GetSize() );
        long nH = LogicToLogic( rItem.GetHeight(), eUnit, MAP_TWIP );
        aSize.Height() = nH;
        aSize.Width() = 0; //!!!
        rFont.SetSize( aSize );
    }
    else
        // als Default 12pt
        rFont.SetSize( Size( 0, 240 ) );

    // Schattiert
    nWhich = GetWhich( SID_ATTR_CHAR_SHADOWED );

    if ( rSet.GetItemState( nWhich ) >= eState )
    {
        const SvxShadowedItem& rItem = (SvxShadowedItem&)rSet.Get( nWhich );
        rFont.SetShadow(rItem.GetValue());
    }

    // Kontour
    nWhich = GetWhich( SID_ATTR_CHAR_CONTOUR );

    if ( rSet.GetItemState( nWhich ) >= eState )
    {
        const SvxContourItem& rItem = (SvxContourItem&)rSet.Get( nWhich );
        rFont.SetOutline( rItem.GetValue() );
    }

    // Farbe
    nWhich = GetWhich( SID_ATTR_CHAR_COLOR );
    if ( rSet.GetItemState( nWhich ) >= eState )
    {
        const SvxColorItem& rItem = (SvxColorItem&)rSet.Get( nWhich );
        rFont.SetColor( rItem.GetValue() );
    }
    nWhich = GetWhich( SID_ATTR_CHAR_FILLCOLOR );
    if ( rSet.GetItemState( nWhich ) >= eState )
    {
        const SvxColorItem& rItem = (SvxColorItem&)rSet.Get( nWhich );
        rFont.SetFillColor( rItem.GetValue() );
    }

    FASTBOOL bWord = FALSE;

    // Durchstreichen
    nWhich = GetWhich( SID_ATTR_CHAR_STRIKEOUT );

    if ( rSet.GetItemState( nWhich ) >= eState )
    {
        const SvxCrossedOutItem& rItem =
            (SvxCrossedOutItem&)rSet.Get( nWhich );
        FontStrikeout eStrikeout = (FontStrikeout)rItem.GetValue();
        rFont.SetStrikeout( eStrikeout );

        if ( eStrikeout == STRIKEOUT_NONE )
        {
            aStrikeoutText.Disable();
            aStrikeoutBox.Disable();
            aStrikeoutBox.SelectEntryPos( 0 );
        }
        else
        {
            aStrikeoutText.Enable();
            aStrikeoutBox.Enable();

            for ( USHORT i = 0; i < aStrikeoutBox.GetEntryCount(); ++i )
            {
                if ( (FontStrikeout)(ULONG)aStrikeoutBox.GetEntryData(i) == eStrikeout )
                {
                    aStrikeoutBox.SelectEntryPos(i);
                    break;
                }
            }
            bWord |= TRUE;
        }
    }

    // Underline
    nWhich = GetWhich( SID_ATTR_CHAR_UNDERLINE );

    if ( rSet.GetItemState( nWhich ) >= eState )
    {
        const SvxUnderlineItem& rItem = (SvxUnderlineItem&)rSet.Get( nWhich );
        FontUnderline eUnderline = (FontUnderline)rItem.GetValue();
        rFont.SetUnderline( eUnderline );

        if ( eUnderline == UNDERLINE_NONE )
        {
            aUnderlineBox.SelectEntryPos( 0 );
            aUnderlineBox.Disable();
            aUnderlineText.Disable();
        }
        else
        {
            for ( USHORT i = 0; i < aUnderlineBox.GetEntryCount(); ++i )
            {
                if ( (FontUnderline)(ULONG)aUnderlineBox.GetEntryData(i) == eUnderline )
                {
                    aUnderlineBox.SelectEntryPos(i);
                    break;
                }
            }
            aUnderlineBox.Enable();
            aUnderlineText.Enable();
            bWord |= TRUE;
        }
    }

    if ( !bWord )
        aWordBox.Disable();
    else
        aWordBox.Enable();

    // Hintergrundfarbe
    nWhich = GetWhich( SID_ATTR_BRUSH );

    if ( rSet.GetItemState( nWhich ) >= eState )
    {
        const SvxBrushItem& rItem = (SvxBrushItem&)rSet.Get( nWhich );
        aExampleWin.SetColor( rItem.GetColor() );
    }

    aExampleWin.Invalidate();
    KerningSelectHdl_Impl( 0 );
    aDelayTimer.Start();
}

// -----------------------------------------------------------------------

int SvxCharExtPage::DeactivatePage( SfxItemSet* pSet )
{
    if ( pSet )
        FillItemSet( *pSet );
    return LEAVE_PAGE;
}

// -----------------------------------------------------------------------

void SvxCharExtPage::SetHandler_Impl()
{
    Link aLink = LINK( this, SvxCharExtPage, CaseMapHdl_Impl );
#if ( SUPD < 590 )
    aNoneBtn.SetClickHdl( aLink );
    aCapsBtn.SetClickHdl( aLink );
    aLowercaseBtn.SetClickHdl( aLink );
    aSmallcapsBtn.SetClickHdl( aLink );
    aTitelBtn.SetClickHdl( aLink );
#else
    aCapsLB.SetSelectHdl( aLink );
#endif

    aLink = LINK( this, SvxCharExtPage, EscapementHdl_Impl );
    aHighBtn.SetClickHdl( aLink );
    aNormalBtn.SetClickHdl( aLink );
    aDeepBtn.SetClickHdl( aLink );
    aAutoKernBtn.SetClickHdl( LINK( this, SvxCharExtPage, AutoKernHdl_Impl ) );
    aKerningListBox.SetSelectHdl(
        LINK( this, SvxCharExtPage, KerningSelectHdl_Impl ) );
    aUnderlineBox.SetSelectHdl(
        LINK( this, SvxCharExtPage, UnderlineSelectHdl_Impl ) );
    aStrikeoutBox.SetSelectHdl(
        LINK( this, SvxCharExtPage, StrikeoutSelectHdl_Impl ) );

    aUnderlineBox.SelectEntryPos( 0 );
    aStrikeoutBox.SelectEntryPos( 0 );

    aEscRelPosEdit.SetModifyHdl(
        LINK( this, SvxCharExtPage, FontExtModifyHdl_Impl ) );
    aEscRelSizeEdit.SetModifyHdl(
        LINK( this, SvxCharExtPage, FontExtModifyHdl_Impl ) );

    aEscRelPosEdit.SetLoseFocusHdl(
        LINK( this, SvxCharExtPage, LoseFocusHdl_Impl ) );
    aEscRelSizeEdit.SetLoseFocusHdl(
        LINK( this, SvxCharExtPage, LoseFocusHdl_Impl ) );

    aKerningEdit.SetModifyHdl(
        LINK( this, SvxCharExtPage, KerningModifyHdl_Impl ) );
    aWordBox.SetClickHdl(
        LINK( this, SvxCharExtPage, WordLineClickHdl_Impl ) );

    aAutoPosBox.SetClickHdl(
        LINK( this, SvxCharExtPage, AutoPosHdl_Impl ) );

    aDelayTimer.SetTimeout( 300 );
    aDelayTimer.SetTimeoutHdl( LINK( this, SvxCharExtPage, TimeoutHdl_Impl ) );
}


// -----------------------------------------------------------------------

#if ( SUPD < 590 )
IMPL_LINK( SvxCharExtPage, CaseMapHdl_Impl, RadioButton *, pBox )
#else
IMPL_LINK( SvxCharExtPage, CaseMapHdl_Impl, ListBox *, EMPTYARG )
#endif
{
    SvxCaseMap eMap = SVX_CASEMAP_NOT_MAPPED;

#if ( SUPD < 590 )
    if ( !pBox->IsChecked() )
        return 0;

    if ( pBox == &aCapsBtn )
        eMap = SVX_CASEMAP_VERSALIEN;
    else if ( pBox == &aLowercaseBtn )
        eMap = SVX_CASEMAP_GEMEINE;
    else if ( pBox == &aSmallcapsBtn )
        eMap = SVX_CASEMAP_KAPITAELCHEN;
    else if ( pBox == &aTitelBtn )
        eMap = SVX_CASEMAP_TITEL;
#else
    USHORT nCapsPos = aCapsLB.GetSelectEntryPos();
    if ( nCapsPos != LISTBOX_ENTRY_NOTFOUND )
        eMap = (SvxCaseMap)nCapsPos;
#endif

    SetCaseMap_Impl( eMap );
    return 0;
}

// -----------------------------------------------------------------------

void SvxCharExtPage::SetCaseMap_Impl( USHORT eMap )
{
#if ( SUPD < 590 )
    switch ( eMap  )
    {
        case SVX_CASEMAP_VERSALIEN:     aCapsBtn.Check( TRUE );         break;
        case SVX_CASEMAP_GEMEINE:       aLowercaseBtn.Check( TRUE );    break;
        case SVX_CASEMAP_TITEL:         aTitelBtn.Check( TRUE );        break;
        case SVX_CASEMAP_KAPITAELCHEN:  aSmallcapsBtn.Check( TRUE );    break;
        case SVX_CASEMAP_NOT_MAPPED:    aNoneBtn.Check( TRUE );         break;
        case SVX_CASEMAP_END:
            aCapsBtn.Check( FALSE );
            aLowercaseBtn.Check( FALSE );
            aTitelBtn.Check( FALSE );
            aSmallcapsBtn.Check( FALSE );
            aNoneBtn.Check( FALSE );
            break;
        default: DBG_ASSERT(FALSE, "Falscher CaseMap"); return;
    }
    if ( eMap == SVX_CASEMAP_END )
        eMap = SVX_CASEMAP_NOT_MAPPED;
#else
    if ( SVX_CASEMAP_END > (SvxCaseMap)eMap )
        aCapsLB.SelectEntryPos( eMap );
    else
    {
        aCapsLB.SetNoSelection();
        eMap = SVX_CASEMAP_NOT_MAPPED;
    }
#endif

    aExampleWin.GetFont().SetCaseMap( (SvxCaseMap)eMap );
    aExampleWin.Invalidate();
}

// -----------------------------------------------------------------------

void SvxCharExtPage::EnablePairKerning_Impl()
{
    SvxFont& rFont = aExampleWin.GetFont();
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxCharExtPage, EscapementHdl_Impl, RadioButton *, pBtn )
{
    USHORT nEsc = SVX_ESCAPEMENT_OFF;   // auch wenn pBtn == 0

    if ( &aHighBtn == pBtn )
        nEsc = SVX_ESCAPEMENT_SUPERSCRIPT;
    else if ( &aDeepBtn == pBtn )
        nEsc = SVX_ESCAPEMENT_SUBSCRIPT;

    SetEscapement_Impl( nEsc );
    return 0;
}

// -----------------------------------------------------------------------

void SvxCharExtPage::SetEscapement_Impl( USHORT eEsc )
{
    SvxEscapementItem aEsc( (SvxEscapement)eEsc );

    if ( SVX_ESCAPEMENT_SUPERSCRIPT == eEsc )
    {
        aEsc.GetEsc() = nSuperEsc;
        aEsc.GetProp() = nSuperProp;
    }
    else if ( SVX_ESCAPEMENT_SUBSCRIPT == eEsc )
    {
        aEsc.GetEsc() = nSubEsc;
        aEsc.GetProp() = nSubProp;
    }

    short nFac = aEsc.GetEsc() < 0 ? -1 : 1;

    aEscRelPosEdit.SetValue( aEsc.GetEsc() * nFac );
    aEscRelSizeEdit.SetValue( aEsc.GetProp() );

    if ( eEsc == SVX_ESCAPEMENT_OFF )
    {
        aEscRelSizeText.Disable();
        aEscRelSizeEdit.Disable();
        aEscRelPosEdit.Disable();
        aEscRelPosText.Disable();
        aAutoPosBox.Disable();
    }
    else
    {
        aEscRelSizeText.Enable();
        aEscRelSizeEdit.Enable();
        aAutoPosBox.Enable();
        if(!aAutoPosBox.IsChecked())
        {
            aEscRelPosEdit.Enable();
            aEscRelPosText.Enable();
        }
        else
        {
            AutoPosHdl_Impl(&aAutoPosBox);
        }
    }
    UpdatePos_Impl( 100, aEsc.GetEsc(), aEsc.GetProp() );
}

// -----------------------------------------------------------------------

IMPL_LINK_INLINE_START( SvxCharExtPage, FontExtModifyHdl_Impl, MetricField *, EMPTYARG )
{
    short nEsc  = (short)aEscRelPosEdit.GetValue();
    BYTE nEscProp = (BYTE)aEscRelSizeEdit.GetValue();
    nEsc *= aDeepBtn.IsChecked() ? -1 : 1;
    UpdatePos_Impl( 100, nEsc, nEscProp );
    return 0;
}
IMPL_LINK_INLINE_END( SvxCharExtPage, FontExtModifyHdl_Impl, MetricField *, EMPTYARG )

// -----------------------------------------------------------------------

void SvxCharExtPage::UpdatePos_Impl( BYTE nProp, short nEsc, BYTE nEscProp )
{
    SvxFont& rFont = aExampleWin.GetFont();

    rFont.SetPropr( nProp );
    rFont.SetProprRel( nEscProp );
    rFont.SetEscapement( nEsc );
    aExampleWin.Invalidate();
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxCharExtPage, KerningModifyHdl_Impl, MetricField *, EMPTYARG )
{
    long nTmp = aKerningEdit.GetValue();
    long nVal = LogicToLogic( nTmp, MAP_POINT, MAP_TWIP );
    long nKern = (short)aKerningEdit.Denormalize( nVal );

    // Schmal?, dann negativ
    if ( aKerningListBox.GetSelectEntryPos() == 2 )
        nKern *= -1;

    SvxFont& rFont = aExampleWin.GetFont();
    rFont.SetFixKerning( (short)nKern );
    aExampleWin.Invalidate();
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxCharExtPage, KerningSelectHdl_Impl, ListBox *, EMPTYARG )
{
    if ( aKerningListBox.GetSelectEntryPos() > 0 )
    {
        aKerningText.Enable();
        aKerningEdit.Enable();

        if ( aKerningListBox.GetSelectEntryPos() == 2 )
        {
            // beim schmalen Kerning den Maximal-Wert auf
            // 1/6 der Font-Hoehe setzen
            SvxFont& rFont = aExampleWin.GetFont();
            long nMax = rFont.GetSize().Height() / 6;
            aKerningEdit.SetMax( aKerningEdit.Normalize( nMax ), FUNIT_TWIP );
            aKerningEdit.SetLast(
                aKerningEdit.GetMax( aKerningEdit.GetUnit() ) );
        }
        else
        {
            aKerningEdit.SetMax( 9999 );
            aKerningEdit.SetLast( 9999 );
        }
    }
    else
    {
        aKerningEdit.SetValue(0);
        aKerningText.Disable();
        aKerningEdit.Disable();
    }
    KerningModifyHdl_Impl(0);
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxCharExtPage, StrikeoutSelectHdl_Impl, ListBox *, EMPTYARG )
{
    if ( aStrikeoutBox.GetSelectEntryPos() != LISTBOX_ENTRY_NOTFOUND )
    {
        USHORT nPos = aStrikeoutBox.GetSelectEntryPos();
        FontStrikeout eStrikeout = (FontStrikeout)(ULONG)aStrikeoutBox.GetEntryData( nPos );
        aExampleWin.GetFont().SetStrikeout(eStrikeout);
        aExampleWin.Invalidate();
    }
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxCharExtPage, UnderlineSelectHdl_Impl, ListBox *, EMPTYARG )
{
    if ( aUnderlineBox.GetSelectEntryPos() != LISTBOX_ENTRY_NOTFOUND )
    {
        USHORT nPos = aUnderlineBox.GetSelectEntryPos();
        FontUnderline eUnderline = (FontUnderline)(ULONG)aUnderlineBox.GetEntryData( nPos );
        aExampleWin.GetFont().SetUnderline(eUnderline);
        aExampleWin.Invalidate();
    }
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK_INLINE_START( SvxCharExtPage, WordLineClickHdl_Impl, CheckBox *, pBox )
{
    aExampleWin.GetFont().SetWordLineMode( pBox->IsChecked() );
    aExampleWin.Invalidate();
    return 0;
}
IMPL_LINK_INLINE_END( SvxCharExtPage, WordLineClickHdl_Impl, CheckBox *, pBox )

// -----------------------------------------------------------------------

IMPL_LINK( SvxCharExtPage, TimeoutHdl_Impl, Timer *, EMPTYARG )
{
    // "Uberpr"ufung, ob Pairkerning am eingestellten Font m"oglich ist.

    if ( aAutoKernBtn.IsChecked() )
        return 0;

    SfxViewShell* pSh = SfxViewShell::Current();
    Printer* pPrinter = pSh ? pSh->GetPrinter() : 0;

    if ( pPrinter )
    {
        Font aOldFont = pPrinter->GetFont();
        Font aTestFont = aExampleWin.GetFont();
        aTestFont.SetKerning( TRUE );
        pPrinter->SetFont( aTestFont );
        FontMetric aMetric = pPrinter->GetFontMetric();
        pPrinter->SetFont( aOldFont );

        if ( aMetric.IsKerning() )
            aAutoKernBtn.Enable();
        else
            aAutoKernBtn.Disable();
    }
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxCharExtPage, LoseFocusHdl_Impl, MetricField *, pField )
{
    FASTBOOL bDeep = aDeepBtn.IsChecked(),
             bHigh = aHighBtn.IsChecked();

    DBG_ASSERT( !( bDeep && bHigh ), "das kann nicht sein" );

    if ( &aEscRelPosEdit == pField && ( bDeep || bHigh ) )
    {
        if ( bDeep )
            nSubEsc = (short)aEscRelPosEdit.GetValue() * -1;
        else
            nSuperEsc = (short)aEscRelPosEdit.GetValue();
    }
    else if ( &aEscRelSizeEdit == pField && ( bDeep || bHigh ) )
    {
        if ( bDeep )
            nSubProp = (BYTE)aEscRelSizeEdit.GetValue();
        else
            nSuperProp = (BYTE)aEscRelSizeEdit.GetValue();
    }
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK_INLINE_START(SvxCharExtPage, AutoKernHdl_Impl, CheckBox*, EMTPYARG )
{
    if ( !aAutoKernBtn.IsChecked() )
        TimeoutHdl_Impl( 0 );
    return 0;
}
IMPL_LINK_INLINE_END(SvxCharExtPage, AutoKernHdl_Impl, CheckBox*, EMPTYARG )

// -----------------------------------------------------------------------

IMPL_LINK( SvxCharExtPage, AutoPosHdl_Impl, CheckBox*, pBox)
{
    if(pBox->IsChecked())
    {
        aEscRelPosText .Disable();
        aEscRelPosEdit .Disable();
    }
    else
    {
        RadioButton* pRadio =   aHighBtn.IsChecked() ? &aHighBtn :
                                    aDeepBtn.IsChecked() ? &aDeepBtn :
                                        &aNormalBtn;
        EscapementHdl_Impl(pRadio);
    }
    return 0;
}

// SvxCharNamePage_Impl --------------------------------------------------

struct SvxCharNamePage_Impl
{
    Timer           m_aUpdateTimer;
    String          m_aNoStyleText;
    String          m_aTransparentText;
    const FontList* m_pFontList;
    USHORT          m_nExtraEntryPos;
    BOOL            m_bMustDelete;
    BOOL            m_bInSearchMode;

    SvxCharNamePage_Impl() :

        m_pFontList     ( NULL ),
        m_nExtraEntryPos( LISTBOX_ENTRY_NOTFOUND ),
        m_bMustDelete   ( FALSE ),
        m_bInSearchMode ( FALSE )

    {
        m_aUpdateTimer.SetTimeout( 350 );
    }

    ~SvxCharNamePage_Impl()
    {
        if ( m_bMustDelete )
            delete m_pFontList;
    }
};

// class SvxCharNamePage -------------------------------------------------

SvxCharNamePage::SvxCharNamePage( Window* pParent, const SfxItemSet& rInSet ) :

    SfxTabPage( pParent, SVX_RES( RID_SVXPAGE_CHAR_NAME ), rInSet ),

    m_aWestLine             ( this, ResId( FL_WEST ) ),
    m_aWestFontNameFT       ( this, ResId( FT_WEST_NAME ) ),
    m_aWestFontNameLB       ( this, ResId( LB_WEST_NAME ) ),
    m_aWestFontStyleFT      ( this, ResId( FT_WEST_STYLE ) ),
    m_aWestFontStyleLB      ( this, ResId( LB_WEST_STYLE ) ),
    m_aWestFontSizeFT       ( this, ResId( FT_WEST_SIZE ) ),
    m_aWestFontSizeLB       ( this, ResId( LB_WEST_SIZE ) ),
    m_aWestFontLanguageFT   ( this, ResId( FT_WEST_LANG ) ),
    m_aWestFontLanguageLB   ( this, ResId( LB_WEST_LANG ), TRUE ),

    m_aEastLine             ( this, ResId( FL_EAST ) ),
    m_aEastFontNameFT       ( this, ResId( FT_EAST_NAME ) ),
    m_aEastFontNameLB       ( this, ResId( LB_EAST_NAME ) ),
    m_aEastFontStyleFT      ( this, ResId( FT_EAST_STYLE ) ),
    m_aEastFontStyleLB      ( this, ResId( LB_EAST_STYLE ) ),
    m_aEastFontSizeFT       ( this, ResId( FT_EAST_SIZE ) ),
    m_aEastFontSizeLB       ( this, ResId( LB_EAST_SIZE ) ),
    m_aEastFontLanguageFT   ( this, ResId( FT_EAST_LANG ) ),
    m_aEastFontLanguageLB   ( this, ResId( LB_EAST_LANG ), TRUE ),

    m_aPreviewLine          ( this, ResId( FL_CHAR_PREVIEW ) ),
    m_aPreviewWin           ( this, ResId( WIN_CHAR_PREVIEW ) ),
    m_aFontTypeFT           ( this, ResId( FT_CHAR_FONTTYPE ) ),

    m_aSeparateLine         ( this, ResId( FL_SEPARATE ) ),

    m_aColorFT              ( this, ResId( FT_COLOR2 ) ),
    m_aColorLB              ( this, ResId( LB_COLOR2 ) ),

    m_pImpl                 ( new SvxCharNamePage_Impl )

{
    m_pImpl->m_aNoStyleText = String( ResId( STR_CHARNAME_NOSTYLE ) );
    m_pImpl->m_aTransparentText = String( ResId( STR_CHARNAME_TRANSPARENT ) );

    FreeResource();
    Initialize();
}

// -----------------------------------------------------------------------

SvxCharNamePage::~SvxCharNamePage()
{
    if ( m_pImpl->m_bMustDelete )
        delete m_pImpl->m_pFontList;
    delete m_pImpl;
}

// -----------------------------------------------------------------------

void SvxCharNamePage::Initialize()
{
    // to handle the changes of the other pages
    SetExchangeSupport();

    // fill the color box
    SfxObjectShell* pDocSh = SfxObjectShell::Current();
    DBG_ASSERT( pDocSh, "DocShell not found!" );
    XColorTable* pColorTable = NULL;
    FASTBOOL bKillTable = FALSE;
    const SfxPoolItem* pItem = NULL;

    if ( pDocSh && ( pItem = pDocSh->GetItem( SID_COLOR_TABLE ) ) )
        pColorTable = ( (SvxColorTableItem*)pItem )->GetColorTable();

    if ( !pColorTable )
    {
        pColorTable = new XColorTable( SvtPathOptions().GetPalettePath() );
        bKillTable = TRUE;
    }

    m_aColorLB.SetUpdateMode( FALSE );

    m_aColorLB.InsertEntry(Color(COL_AUTO), SVX_RESSTR( RID_SVXSTR_AUTOMATIC ));
    for ( long i = 0; i < pColorTable->Count(); i++ )
    {
        XColorEntry* pEntry = pColorTable->Get(i);
        m_aColorLB.InsertEntry( pEntry->GetColor(), pEntry->GetName() );
    }

    m_aColorLB.SetUpdateMode( TRUE );

    if ( bKillTable )
        delete pColorTable;

    m_aSeparateLine.SetStyle( m_aSeparateLine.GetStyle() | WB_VERT );
    m_aColorLB.SetSelectHdl( LINK( this, SvxCharNamePage, ColorBoxSelectHdl_Impl ) );

    Link aLink = LINK( this, SvxCharNamePage, FontModifyHdl_Impl );
    m_aWestFontNameLB.SetModifyHdl( aLink );
    m_aWestFontStyleLB.SetModifyHdl( aLink );
    m_aWestFontSizeLB.SetModifyHdl( aLink );

    m_pImpl->m_aUpdateTimer.SetTimeoutHdl( LINK( this, SvxCharNamePage, UpdateHdl_Impl ) );
    m_pImpl->m_aUpdateTimer.Start();
}

// -----------------------------------------------------------------------

const FontList* SvxCharNamePage::GetFontList() const
{
    if ( !m_pImpl->m_pFontList )
    {
        SfxObjectShell* pDocSh = SfxObjectShell::Current();
        const SfxPoolItem* pItem;

        if ( pDocSh && ( pItem = pDocSh->GetItem( SID_ATTR_CHAR_FONTLIST ) ) )
            m_pImpl->m_pFontList = ( (SvxFontListItem*)pItem )->GetFontList();
        else
        {
            m_pImpl->m_pFontList = new FontList( Application::GetDefaultDevice() );
            m_pImpl->m_bMustDelete = TRUE;
        }
    }

    return m_pImpl->m_pFontList;
}

// -----------------------------------------------------------------------

void SvxCharNamePage::UpdatePreview_Impl()
{
    SvxFont& rFont = m_aPreviewWin.GetFont();
    Size aSize = rFont.GetSize();
    aSize.Width() = 0;
    // Font
    const FontList* pFontList = GetFontList();
    FontInfo aFontInfo( pFontList->Get( m_aWestFontNameLB.GetText(), m_aWestFontStyleLB.GetText() ) );
    // Size
    if ( m_aWestFontSizeLB.IsRelative() )
    {
        DBG_ASSERT( GetItemSet().GetParent(), "No parent set" );
        USHORT nWhich = GetWhich( SID_ATTR_CHAR_FONTHEIGHT );
        const SvxFontHeightItem& rOldItem = (SvxFontHeightItem&)GetItemSet().GetParent()->Get( nWhich );

        // alter Wert, skaliert
        long nHeight;
        if ( m_aWestFontSizeLB.IsPtRelative() )
            nHeight = rOldItem.GetHeight() + PointToTwips( m_aWestFontSizeLB.GetValue() / 10 );
        else
            nHeight = rOldItem.GetHeight() * m_aWestFontSizeLB.GetValue() / 100;

        // Umrechnung in twips fuer das Beispiel-Window
        aSize.Height() =
            ItemToControl( nHeight, GetItemSet().GetPool()->GetMetric( nWhich ), SFX_FUNIT_TWIP );
    }
    else if ( m_aWestFontSizeLB.GetText().Len() )
        aSize.Height() = PointToTwips( m_aWestFontSizeLB.GetValue() / 10 );
    else
        aSize.Height() = 200;   // default 10pt
    aFontInfo.SetSize( aSize );

    rFont.SetFamily( aFontInfo.GetFamily() );
    rFont.SetName( aFontInfo.GetName() );
    rFont.SetStyleName( aFontInfo.GetStyleName() );
    rFont.SetPitch( aFontInfo.GetPitch() );
    rFont.SetCharSet( aFontInfo.GetCharSet() );
    rFont.SetWeight( aFontInfo.GetWeight() );
    rFont.SetItalic( aFontInfo.GetItalic() );
    rFont.SetSize( aFontInfo.GetSize() );

    m_aPreviewWin.Invalidate();
    m_aFontTypeFT.SetText( pFontList->GetFontMapText( aFontInfo ) );
}

// -----------------------------------------------------------------------

void SvxCharNamePage::FillStyleBox_Impl( const FontNameBox* pBox )
{
    const FontList* pFontList = GetFontList();
    DBG_ASSERT( pFontList, "no fontlist" );

    if ( &m_aWestFontNameLB == pBox )
        m_aWestFontStyleLB.Fill( m_aWestFontNameLB.GetText(), pFontList );
    else if ( &m_aEastFontNameLB == pBox )
        m_aEastFontStyleLB.Fill( m_aEastFontNameLB.GetText(), pFontList );
    else
    {
        DBG_ERRORFILE( "invalid font name box" );
    }

    if ( m_pImpl->m_bInSearchMode )
    {
        // Bei der Suche zus"atzliche Eintr"age:
        // "Nicht Fett" und "Nicht Kursiv"
        String aEntry = m_pImpl->m_aNoStyleText;
        const sal_Char sS[] = "%s";
        aEntry.SearchAndReplaceAscii( sS, pFontList->GetBoldStr() );
        m_pImpl->m_nExtraEntryPos = ( &m_aWestFontNameLB == pBox )
            ? m_aWestFontStyleLB.InsertEntry( aEntry ) : m_aEastFontStyleLB.InsertEntry( aEntry );
        aEntry = m_pImpl->m_aNoStyleText;
        aEntry.SearchAndReplaceAscii( sS, pFontList->GetItalicStr() );
        ( &m_aWestFontNameLB == pBox ) ? m_aWestFontStyleLB.InsertEntry( aEntry )
                                       : m_aEastFontStyleLB.InsertEntry( aEntry );
    }
}

// -----------------------------------------------------------------------

void SvxCharNamePage::FillSizeBox_Impl( const FontNameBox* pBox )
{
    const FontList* pFontList = GetFontList();
    DBG_ASSERT( pFontList, "no fontlist" );

    if ( &m_aWestFontNameLB == pBox )
        m_aWestFontSizeLB.Fill( pFontList->Get( m_aWestFontNameLB.GetText(),
                                                m_aWestFontStyleLB.GetText() ), pFontList );
    else if ( &m_aEastFontNameLB == pBox )
        m_aEastFontSizeLB.Fill( pFontList->Get( m_aEastFontNameLB.GetText(),
                                                m_aEastFontStyleLB.GetText() ), pFontList );
    else
    {
        DBG_ERRORFILE( "invalid font name box" );
    }
}

// -----------------------------------------------------------------------

void SvxCharNamePage::ResetWestOrEast_Impl( const SfxItemSet& rSet, BOOL bWest )
{
    FontNameBox* pNameBox = bWest ? &m_aWestFontNameLB : &m_aEastFontNameLB;
    FontStyleBox* pStyleBox = bWest ? &m_aWestFontStyleLB : &m_aEastFontStyleLB;
    FontSizeBox* pSizeBox = bWest ? &m_aWestFontSizeLB : &m_aEastFontSizeLB;
    FixedText* pLangFT = bWest ? &m_aWestFontLanguageFT : &m_aEastFontLanguageFT;
    SvxLanguageBox* pLangBox = bWest ? &m_aWestFontLanguageLB : &m_aEastFontLanguageLB;

    // die FontListBox fuellen
    const FontList* pFontList = GetFontList();
    pNameBox->Fill( pFontList );

    // Font ermitteln
    const SvxFontItem* pFontItem = NULL;
    USHORT nWhich = GetWhich( bWest ? SID_ATTR_CHAR_FONT : SID_ATTR_CHAR_CJK_FONT );
    SfxItemState eState = rSet.GetItemState( nWhich );

    if ( eState >= SFX_ITEM_DEFAULT )
    {
        pFontItem = (const SvxFontItem*)&( rSet.Get( nWhich ) );
        pNameBox->SetText( pFontItem->GetFamilyName() );
    }
    else
        pNameBox->SetText( String() );

    FillStyleBox_Impl( pNameBox );

    FASTBOOL bStyle = FALSE;
    FontItalic eItalic = ITALIC_NONE;
    FontWeight eWeight = WEIGHT_NORMAL;
    nWhich = GetWhich( bWest ? SID_ATTR_CHAR_POSTURE : SID_ATTR_CHAR_CJK_POSTURE );
    eState = rSet.GetItemState( nWhich );

    if ( eState >= SFX_ITEM_DEFAULT )
    {
        const SvxPostureItem& rItem = (SvxPostureItem&)rSet.Get( nWhich );
        eItalic = (FontItalic)rItem.GetValue();
        bStyle = TRUE;
    }

    nWhich = GetWhich( bWest ? SID_ATTR_CHAR_WEIGHT : SID_ATTR_CHAR_CJK_WEIGHT );
    eState = rSet.GetItemState( nWhich );

    if ( eState >= SFX_ITEM_DEFAULT )
    {
        SvxWeightItem& rItem = (SvxWeightItem&)rSet.Get( nWhich );
        eWeight = (FontWeight)rItem.GetValue();
    }
    else if ( bStyle )
        bStyle = FALSE;

    // Aktuell eingestellter Font
    if ( bStyle && pFontItem )
    {
        FontInfo aInfo = pFontList->Get( pFontItem->GetFamilyName(), eWeight, eItalic );
        pStyleBox->SetText( pFontList->GetStyleName( aInfo ) );
    }
    else if ( !m_pImpl->m_bInSearchMode || !bStyle )
        pStyleBox->SetText( String() );
    else if ( bStyle )
    {
        FontInfo aInfo = pFontList->Get( String(), eWeight, eItalic );
        pStyleBox->SetText( pFontList->GetStyleName( aInfo ) );
    }

    // SizeBox fuellen
    FillSizeBox_Impl( pNameBox );
    nWhich = GetWhich( bWest ? SID_ATTR_CHAR_FONTHEIGHT : SID_ATTR_CHAR_CJK_FONTHEIGHT );
    eState = rSet.GetItemState( nWhich );

    if ( pSizeBox->IsRelativeMode() )
    {
        SfxMapUnit eUnit = rSet.GetPool()->GetMetric( nWhich );
        const SvxFontHeightItem& rItem = (SvxFontHeightItem&)rSet.Get( nWhich );

        if( rItem.GetProp() != 100 || SFX_MAPUNIT_RELATIVE != rItem.GetPropUnit() )
        {
            BOOL bPtRel = SFX_MAPUNIT_POINT == rItem.GetPropUnit();
            pSizeBox->SetPtRelative( bPtRel );
            pSizeBox->SetValue( bPtRel ? ((short)rItem.GetProp()) * 10 : rItem.GetProp() );
        }
        else
        {
            pSizeBox->SetRelative();
            pSizeBox->SetValue( (long)CalcToPoint( rItem.GetHeight(), eUnit, 10 ) );
        }
    }
    else if ( eState >= SFX_ITEM_DEFAULT )
    {
        SfxMapUnit eUnit = rSet.GetPool()->GetMetric( nWhich );
        const SvxFontHeightItem& rItem = (SvxFontHeightItem&)rSet.Get( nWhich );
        pSizeBox->SetValue( (long)CalcToPoint( rItem.GetHeight(), eUnit, 10 ) );
    }
    else
        pSizeBox->SetText( String() );

    nWhich = GetWhich( bWest ? SID_ATTR_CHAR_LANGUAGE : SID_ATTR_CHAR_CJK_LANGUAGE );
    pLangBox->SetNoSelection();
    eState = rSet.GetItemState( nWhich );

    switch ( eState )
    {
        case SFX_ITEM_UNKNOWN:
            pLangFT->Hide();
            pLangBox->Hide();
            break;

        case SFX_ITEM_DISABLED:
        case SFX_ITEM_READONLY:
            pLangFT->Disable();
            pLangBox->Disable();
            break;

        case SFX_ITEM_DEFAULT:
        case SFX_ITEM_SET:
        {
            const SvxLanguageItem& rItem = (SvxLanguageItem&)rSet.Get( nWhich );
            LanguageType eLangType = (LanguageType)rItem.GetValue();
            DBG_ASSERT( eLangType != LANGUAGE_SYSTEM, "LANGUAGE_SYSTEM not allowed" );
            if ( eLangType != LANGUAGE_DONTKNOW )
            {
                for ( USHORT i = 0; i < pLangBox->GetEntryCount(); ++i )
                {
                    if ( (LanguageType)(ULONG)pLangBox->GetEntryData(i) == eLangType )
                    {
                        pLangBox->SelectEntryPos(i);
                        break;
                    }
                }
            }
            break;
        }
    }

    if ( bWest )
        m_aFontTypeFT.SetText( pFontList->GetFontMapText(
            pFontList->Get( pNameBox->GetText(), pStyleBox->GetText() ) ) );

    // save these settings
    pNameBox->SaveValue();
    pStyleBox->SaveValue();
    pSizeBox->SaveValue();
    pLangBox->SaveValue();
}

// -----------------------------------------------------------------------

BOOL SvxCharNamePage::FillItemSetWestOrEast_Impl( SfxItemSet& rSet, BOOL bWest )
{
    BOOL bModified = FALSE;

    FontNameBox* pNameBox = bWest ? &m_aWestFontNameLB : &m_aEastFontNameLB;
    FontStyleBox* pStyleBox = bWest ? &m_aWestFontStyleLB : &m_aEastFontStyleLB;
    FontSizeBox* pSizeBox = bWest ? &m_aWestFontSizeLB : &m_aEastFontSizeLB;
    SvxLanguageBox* pLangBox = bWest ? &m_aWestFontLanguageLB : &m_aEastFontLanguageLB;

    const SfxPoolItem* pItem = NULL;
    const SfxItemSet& rOldSet = GetItemSet();
    const SfxPoolItem* pOld = NULL;

    const SfxItemSet* pExampleSet = GetTabDialog() ? GetTabDialog()->GetExampleSet() : NULL;

    FASTBOOL bChanged = TRUE;
    USHORT nWhich = GetWhich( bWest ? SID_ATTR_CHAR_FONT : SID_ATTR_CHAR_CJK_FONT );
    const String& rFontName  = pNameBox->GetText();
    const FontList* pFontList = GetFontList();
    String aStyleBoxText =pStyleBox->GetText();
    USHORT nEntryPos = pStyleBox->GetEntryPos( aStyleBoxText );
    if ( nEntryPos >= m_pImpl->m_nExtraEntryPos )
        aStyleBoxText.Erase();
    FontInfo aInfo( pFontList->Get( rFontName, aStyleBoxText ) );
    SvxFontItem aFontItem( aInfo.GetFamily(), aInfo.GetName(), aInfo.GetStyleName(),
                           aInfo.GetPitch(), aInfo.GetCharSet(), nWhich );
    pOld = GetOldItem( rSet, bWest ? SID_ATTR_CHAR_FONT : SID_ATTR_CHAR_CJK_FONT );

    if ( pOld )
    {
        const SvxFontItem& rItem = *( (const SvxFontItem*)pOld );

        if ( rItem.GetFamilyName() == aFontItem.GetFamilyName() )
            bChanged = FALSE;
    }

    if ( !bChanged )
        bChanged = !pNameBox->GetSavedValue().Len();

    if ( !bChanged && pExampleSet &&
         pExampleSet->GetItemState( nWhich, FALSE, &pItem ) == SFX_ITEM_SET &&
         ( (SvxFontItem*)pItem )->GetFamilyName() != aFontItem.GetFamilyName() )
        bChanged = TRUE;

    if ( bChanged && rFontName.Len() )
    {
        rSet.Put( aFontItem );
        bModified |= TRUE;
    }
    else if ( SFX_ITEM_DEFAULT == rOldSet.GetItemState( nWhich, FALSE ) )
        rSet.ClearItem( nWhich );

    bChanged = TRUE;
    nWhich = GetWhich( bWest ? SID_ATTR_CHAR_WEIGHT : SID_ATTR_CHAR_CJK_WEIGHT );
    FontWeight eWeight = aInfo.GetWeight();
    if ( nEntryPos >= m_pImpl->m_nExtraEntryPos )
        eWeight = WEIGHT_NORMAL;
    SvxWeightItem aWeightItem( eWeight, nWhich );
    pOld = GetOldItem( rSet, bWest ? SID_ATTR_CHAR_WEIGHT : SID_ATTR_CHAR_CJK_WEIGHT );

    if ( pOld )
    {
        const SvxWeightItem& rItem = *( (const SvxWeightItem*)pOld );

        if ( rItem.GetValue() == aWeightItem.GetValue() )
            bChanged = FALSE;
    }

    if ( !bChanged )
    {
        bChanged = !pStyleBox->GetSavedValue().Len();

        if ( m_pImpl->m_bInSearchMode && bChanged &&
             aInfo.GetWeight() == WEIGHT_NORMAL && aInfo.GetItalic() != ITALIC_NONE )
            bChanged = FALSE;
    }

    if ( !bChanged && pExampleSet &&
         pExampleSet->GetItemState( nWhich, FALSE, &pItem ) == SFX_ITEM_SET &&
         ( (SvxWeightItem*)pItem )->GetValue() != aWeightItem.GetValue() )
        bChanged = TRUE;

    if ( nEntryPos >= m_pImpl->m_nExtraEntryPos )
        bChanged = ( nEntryPos == m_pImpl->m_nExtraEntryPos );

    String aText( pStyleBox->GetText() ); // Tristate, dann Text leer

    if ( bChanged && aText.Len() )
    {
        rSet.Put( aWeightItem );
        bModified |= TRUE;
    }
    else if ( SFX_ITEM_DEFAULT == rOldSet.GetItemState( nWhich, FALSE ) )
        rSet.ClearItem( nWhich );

    bChanged = TRUE;
    nWhich = GetWhich( bWest ? SID_ATTR_CHAR_POSTURE : SID_ATTR_CHAR_CJK_POSTURE );
    FontItalic eItalic = aInfo.GetItalic();
    if ( nEntryPos >= m_pImpl->m_nExtraEntryPos )
        eItalic = ITALIC_NONE;
    SvxPostureItem aPostureItem( eItalic, nWhich );
    pOld = GetOldItem( rSet, bWest ? SID_ATTR_CHAR_POSTURE : SID_ATTR_CHAR_CJK_POSTURE );

    if ( pOld )
    {
        const SvxPostureItem& rItem = *( (const SvxPostureItem*)pOld );

        if ( rItem.GetValue() == aPostureItem.GetValue() )
            bChanged = FALSE;
    }

    if ( !bChanged )
    {
        bChanged = !pStyleBox->GetSavedValue().Len();

        if ( m_pImpl->m_bInSearchMode && bChanged &&
             aInfo.GetItalic() == ITALIC_NONE && aInfo.GetWeight() != WEIGHT_NORMAL )
            bChanged = FALSE;
    }

    if ( !bChanged && pExampleSet &&
         pExampleSet->GetItemState( nWhich, FALSE, &pItem ) == SFX_ITEM_SET &&
         ( (SvxPostureItem*)pItem )->GetValue() != aPostureItem.GetValue() )
        bChanged = TRUE;

    if ( nEntryPos >= m_pImpl->m_nExtraEntryPos )
        bChanged = ( nEntryPos == ( m_pImpl->m_nExtraEntryPos + 1 ) );

    if ( bChanged && aText.Len() )
    {
        rSet.Put( aPostureItem );
        bModified |= TRUE;
    }
    else if ( SFX_ITEM_DEFAULT == rOldSet.GetItemState( nWhich, FALSE ) )
        rSet.ClearItem( nWhich );

    // FontSize
    long nSize = pSizeBox->GetValue();

    if ( !pSizeBox->GetText().Len() )   // GetValue() gibt dann Min-Wert zurueck
        nSize = 0;
    long nSavedSize = pSizeBox->GetSavedValue().ToInt32();
    FASTBOOL bRel = TRUE;

    if ( !pSizeBox->IsRelative() )
    {
        nSavedSize *= 10;
        bRel = FALSE;
    }

    nWhich = GetWhich( bWest ? SID_ATTR_CHAR_FONTHEIGHT : SID_ATTR_CHAR_CJK_FONTHEIGHT );
    const SvxFontHeightItem* pOldHeight = (const SvxFontHeightItem*)
        GetOldItem( rSet, bWest ? SID_ATTR_CHAR_FONTHEIGHT : SID_ATTR_CHAR_CJK_FONTHEIGHT );
    bChanged = ( nSize != nSavedSize );

    if ( !bChanged && pExampleSet &&
         pExampleSet->GetItemState( nWhich, FALSE, &pItem ) == SFX_ITEM_SET )
    {
        float fSize = (float)nSize / 10;
        long nVal = CalcToUnit( fSize, rSet.GetPool()->GetMetric( nWhich ) );
        if ( ( (SvxFontHeightItem*)pItem )->GetHeight() != (UINT32)nVal )
            bChanged = TRUE;
    }

    if ( bChanged || !pOldHeight ||
         bRel != ( SFX_MAPUNIT_RELATIVE != pOldHeight->GetPropUnit() || 100 != pOldHeight->GetProp() ) )
    {
        SfxMapUnit eUnit = rSet.GetPool()->GetMetric( nWhich );
        if ( pSizeBox->IsRelative() )
        {
            DBG_ASSERT( GetItemSet().GetParent(), "No parent set" );
            const SvxFontHeightItem& rOldItem =
                (const SvxFontHeightItem&)GetItemSet().GetParent()->Get( nWhich );

            SvxFontHeightItem aHeight( 240, 100, nWhich );
            if ( pSizeBox->IsPtRelative() )
                aHeight.SetHeight( rOldItem.GetHeight(), (USHORT)( nSize / 10 ), SFX_MAPUNIT_POINT, eUnit );
            else
                aHeight.SetHeight( rOldItem.GetHeight(), (USHORT)nSize, SFX_MAPUNIT_RELATIVE );
            rSet.Put( aHeight );
        }
        else
        {
            float fSize = (float)nSize / 10;
            rSet.Put( SvxFontHeightItem( CalcToUnit( fSize, eUnit ), 100, nWhich ) );
        }
        bModified |= TRUE;
    }
    else if ( SFX_ITEM_DEFAULT == rOldSet.GetItemState( nWhich, FALSE ) )
        rSet.ClearItem( nWhich );

    bChanged = TRUE;
    nWhich = GetWhich( bWest ? SID_ATTR_CHAR_LANGUAGE : SID_ATTR_CHAR_CJK_LANGUAGE );
    pOld = GetOldItem( rSet, bWest ? SID_ATTR_CHAR_LANGUAGE : SID_ATTR_CHAR_CJK_LANGUAGE );
    USHORT nLangPos = pLangBox->GetSelectEntryPos();
    LanguageType eLangType = (LanguageType)(ULONG)pLangBox->GetEntryData( nLangPos );

    if ( pOld )
    {
        const SvxLanguageItem& rItem = *( (const SvxLanguageItem*)pOld );

        if ( nLangPos == LISTBOX_ENTRY_NOTFOUND || eLangType == (LanguageType)rItem.GetValue() )
            bChanged = FALSE;
    }

    if ( !bChanged )
        bChanged = ( pLangBox->GetSavedValue() == LISTBOX_ENTRY_NOTFOUND );

    if ( bChanged && nLangPos != LISTBOX_ENTRY_NOTFOUND )
    {
        rSet.Put( SvxLanguageItem( eLangType, nWhich ) );
        bModified |= TRUE;
    }
    else if ( SFX_ITEM_DEFAULT == rOldSet.GetItemState( nWhich, FALSE ) )
        rSet.ClearItem( nWhich );

    return bModified;
}

// -----------------------------------------------------------------------

void SvxCharNamePage::ResetColor_Impl( const SfxItemSet& rSet )
{
    USHORT nWhich = GetWhich( SID_ATTR_CHAR_COLOR );
    SfxItemState eState = rSet.GetItemState( nWhich );

    switch ( eState )
    {
        case SFX_ITEM_UNKNOWN:
            m_aColorLB.Hide();
            break;

        case SFX_ITEM_DISABLED:
        case SFX_ITEM_READONLY:
            m_aColorLB.Disable();
            break;

        case SFX_ITEM_DONTCARE:
            m_aColorLB.SetNoSelection();
            break;

        case SFX_ITEM_DEFAULT:
        case SFX_ITEM_SET:
        {
            SvxFont& rFont = m_aPreviewWin.GetFont();
            const SvxColorItem& rItem = (SvxColorItem&)rSet.Get( nWhich );
            Color aColor = rItem.GetValue();
            rFont.SetColor( aColor.GetColor() == COL_AUTO ? Color(COL_BLACK) : aColor );
            m_aPreviewWin.Invalidate();
            USHORT nSelPos = m_aColorLB.GetEntryPos( aColor );
            if ( nSelPos == LISTBOX_ENTRY_NOTFOUND && aColor == Color( COL_TRANSPARENT ) )
                nSelPos = m_aColorLB.GetEntryPos( m_pImpl->m_aTransparentText );

            if ( LISTBOX_ENTRY_NOTFOUND != nSelPos )
                m_aColorLB.SelectEntryPos( nSelPos );
            else
            {
                nSelPos = m_aColorLB.GetEntryPos( aColor );
                if ( LISTBOX_ENTRY_NOTFOUND != nSelPos )
                    m_aColorLB.SelectEntryPos( nSelPos );
                else
                    m_aColorLB.SelectEntryPos(
                        m_aColorLB.InsertEntry( aColor, String( SVX_RES( RID_SVXSTR_COLOR_USER ) ) ) );
            }
            break;
        }
    }
}

// -----------------------------------------------------------------------

BOOL SvxCharNamePage::FillItemSetColor_Impl( SfxItemSet& rSet )
{
    USHORT nWhich = GetWhich( SID_ATTR_CHAR_COLOR );
    const SvxColorItem* pOld = (const SvxColorItem*)GetOldItem( rSet, SID_ATTR_CHAR_COLOR );
    const SvxColorItem* pItem = NULL;
    BOOL bChanged = TRUE;
    const SfxItemSet* pExampleSet = GetTabDialog() ? GetTabDialog()->GetExampleSet() : NULL;
    const SfxItemSet& rOldSet = GetItemSet();

    Color aSelectedColor;
    if ( m_aColorLB.GetSelectEntry() == m_pImpl->m_aTransparentText )
        aSelectedColor = Color( COL_TRANSPARENT );
    else
        aSelectedColor = m_aColorLB.GetSelectEntryColor();

    if ( pOld && pOld->GetValue() == aSelectedColor )
        bChanged = FALSE;

    if ( !bChanged )
        bChanged = ( m_aColorLB.GetSavedValue() == LISTBOX_ENTRY_NOTFOUND );

    if ( !bChanged && pExampleSet &&
         pExampleSet->GetItemState( nWhich, FALSE, (const SfxPoolItem**)&pItem ) == SFX_ITEM_SET &&
         ( (SvxColorItem*)pItem )->GetValue() != aSelectedColor )
        bChanged = TRUE;

    BOOL bModified = FALSE;

    if ( bChanged && m_aColorLB.GetSelectEntryPos() != LISTBOX_ENTRY_NOTFOUND )
    {
        rSet.Put( SvxColorItem( aSelectedColor, nWhich ) );
        bModified = TRUE;
    }
    else if ( SFX_ITEM_DEFAULT == rOldSet.GetItemState( nWhich, FALSE ) )
        rSet.ClearItem( nWhich );

    return bModified;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxCharNamePage, UpdateHdl_Impl, Timer*, EMPTYARG )
{
    UpdatePreview_Impl();
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxCharNamePage, FontModifyHdl_Impl, void*, pBox )
{
    m_pImpl->m_aUpdateTimer.Start();

    if ( &m_aWestFontLanguageLB == pBox || &m_aEastFontLanguageLB == pBox )
    {
        FillStyleBox_Impl( (FontNameBox*)pBox );
        FillSizeBox_Impl( (FontNameBox*)pBox );
    }
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxCharNamePage, ColorBoxSelectHdl_Impl, ColorListBox*, pBox )
{
    SvxFont& rFont = m_aPreviewWin.GetFont();
    Color aSelectedColor;
    if ( pBox->GetSelectEntry() == m_pImpl->m_aTransparentText )
        aSelectedColor = Color( COL_TRANSPARENT );
    else
        aSelectedColor = pBox->GetSelectEntryColor();
    rFont.SetColor( aSelectedColor.GetColor() == COL_AUTO ? Color(COL_BLACK) : aSelectedColor );
    m_aPreviewWin.Invalidate();
    return 0;
}

// -----------------------------------------------------------------------

void SvxCharNamePage::ActivatePage( const SfxItemSet& rSet )
{
    SvxFont& rFont = m_aPreviewWin.GetFont();

    // Underline
    USHORT nWhich = GetWhich( SID_ATTR_CHAR_UNDERLINE );
    if ( rSet.GetItemState( nWhich ) >= SFX_ITEM_DEFAULT )
    {
        const SvxUnderlineItem& rItem = (SvxUnderlineItem&)rSet.Get( nWhich );
        FontUnderline eUnderline = (FontUnderline)rItem.GetValue();
        rFont.SetUnderline( eUnderline );
    }
    else
        rFont.SetUnderline( UNDERLINE_NONE );

    //  Strikeout
    nWhich = GetWhich( SID_ATTR_CHAR_STRIKEOUT );
    if ( rSet.GetItemState( nWhich ) >= SFX_ITEM_DEFAULT )
    {
        const SvxCrossedOutItem& rItem = (SvxCrossedOutItem&)rSet.Get( nWhich );
        FontStrikeout eStrikeout = (FontStrikeout)rItem.GetValue();
        rFont.SetStrikeout( eStrikeout );
    }
    else
        rFont.SetStrikeout( STRIKEOUT_NONE );

    // WordLineMode
    nWhich = GetWhich( SID_ATTR_CHAR_WORDLINEMODE );
    if ( rSet.GetItemState( nWhich ) >= SFX_ITEM_DEFAULT )
    {
        const SvxWordLineModeItem& rItem = (SvxWordLineModeItem&)rSet.Get( nWhich );
        rFont.SetWordLineMode( rItem.GetValue() );
    }

    // Emphasis
    nWhich = GetWhich( SID_ATTR_CHAR_EMPHASISMARK );
    if ( rSet.GetItemState( nWhich ) >= SFX_ITEM_DEFAULT )
    {
        const SvxEmphasisMarkItem& rItem = (SvxEmphasisMarkItem&)rSet.Get( nWhich );
        FontEmphasisMark eMark = rItem.GetEmphasisMark();
        rFont.SetEmphasisMark( eMark );
    }

    // Effects
    nWhich = GetWhich( SID_ATTR_CHAR_CASEMAP );

    if ( rSet.GetItemState( nWhich ) >= SFX_ITEM_DEFAULT )
    {
        const SvxCaseMapItem& rItem = (const SvxCaseMapItem&)rSet.Get( nWhich );
        SvxCaseMap eCaseMap = (SvxCaseMap)rItem.GetValue();
        rFont.SetCaseMap( eCaseMap );
    }

    // Outline
    nWhich = GetWhich( SID_ATTR_CHAR_CONTOUR );
    if ( rSet.GetItemState( nWhich ) >= SFX_ITEM_DEFAULT )
    {
        const SvxContourItem& rItem = (SvxContourItem&)rSet.Get( nWhich );
        rFont.SetOutline( rItem.GetValue() );
    }

    // Shadow
    nWhich = GetWhich( SID_ATTR_CHAR_SHADOWED );
    if ( rSet.GetItemState( nWhich ) >= SFX_ITEM_DEFAULT )
    {
        const SvxShadowedItem& rItem = (SvxShadowedItem&)rSet.Get( nWhich );
        rFont.SetShadow( rItem.GetValue() );
    }

    m_aPreviewWin.Invalidate();
}

// -----------------------------------------------------------------------

int SvxCharNamePage::DeactivatePage( SfxItemSet* pSet )
{
    if ( pSet )
        FillItemSet( *pSet );
    return LEAVE_PAGE;
}

// -----------------------------------------------------------------------

SfxTabPage* SvxCharNamePage::Create( Window* pParent, const SfxItemSet& rSet )
{
    return new SvxCharNamePage( pParent, rSet );
}

// -----------------------------------------------------------------------

USHORT* SvxCharNamePage::GetRanges()
{
    return NULL;
}

// -----------------------------------------------------------------------

void SvxCharNamePage::Reset( const SfxItemSet& rSet )
{
    ResetWestOrEast_Impl( rSet, TRUE );
    ResetWestOrEast_Impl( rSet, FALSE );
    ResetColor_Impl( rSet );
    m_aColorLB.SaveValue();
}

// -----------------------------------------------------------------------

BOOL SvxCharNamePage::FillItemSet( SfxItemSet& rSet )
{
    BOOL bModified = FillItemSetWestOrEast_Impl( rSet, TRUE );
    bModified |= FillItemSetWestOrEast_Impl( rSet, FALSE );
    bModified |= FillItemSetColor_Impl( rSet );
    return bModified;
}

// -----------------------------------------------------------------------

void SvxCharNamePage::SetFontList( const SvxFontListItem& rItem )
{
    if ( m_pImpl->m_bMustDelete )
    {
        delete m_pImpl->m_pFontList;
        m_pImpl->m_bMustDelete = FALSE;
    }
    m_pImpl->m_pFontList = rItem.GetFontList();
}

// -----------------------------------------------------------------------

void SvxCharNamePage::EnableRelativeMode()
{
    DBG_ASSERT( GetItemSet().GetParent(), "RelativeMode, but no ParentSet!" );
    m_aWestFontSizeLB.EnableRelativeMode( 0, 999 ); // min 0%, max 999%, step 5

    USHORT nWhich = GetWhich( SID_ATTR_CHAR_FONTHEIGHT );
    const SvxFontHeightItem& rWestItem = (SvxFontHeightItem&)GetItemSet().GetParent()->Get( nWhich );
    SfxMapUnit eUnit = GetItemSet().GetPool()->GetMetric( nWhich );
    short nCurHeight = CalcToPoint( rWestItem.GetHeight(), eUnit, 1 ) * 10;

    // ausgehend von der akt. Hoehe:
    //      - negativ bis minimal 2 pt
    //      - positiv bis maximal 999 pt
    m_aWestFontSizeLB.EnablePtRelativeMode( -(nCurHeight - 20), (9999 - nCurHeight), 10 );

    nWhich = GetWhich( SID_ATTR_CHAR_CJK_FONTHEIGHT );
    const SvxFontHeightItem& rEastItem = (SvxFontHeightItem&)GetItemSet().GetParent()->Get( nWhich );
    eUnit = GetItemSet().GetPool()->GetMetric( nWhich );
    nCurHeight = CalcToPoint( rEastItem.GetHeight(), eUnit, 1 ) * 10;
    m_aEastFontSizeLB.EnablePtRelativeMode( -(nCurHeight - 20), (9999 - nCurHeight), 10 );
}

// -----------------------------------------------------------------------

void SvxCharNamePage::EnableSearchMode()
{
    m_pImpl->m_bInSearchMode = TRUE;
}

// class SvxCharEffectsPage ----------------------------------------------

SvxCharEffectsPage::SvxCharEffectsPage( Window* pParent, const SfxItemSet& rInSet ) :

    SfxTabPage( pParent, SVX_RES( RID_SVXPAGE_CHAR_EFFECTS ), rInSet ),

    m_aUnderlineFT          ( this, ResId( FT_UNDERLINE ) ),
    m_aUnderlineLB          ( this, ResId( LB_UNDERLINE ) ),
    m_aColorFT              ( this, ResId( FT_UNDERLINE_COLOR ) ),
    m_aColorLB              ( this, ResId( LB_UNDERLINE_COLOR ) ),
    m_aStrikeoutFT          ( this, ResId( FT_STRIKEOUT ) ),
    m_aStrikeoutLB          ( this, ResId( LB_STRIKEOUT ) ),
    m_aIndividualWordsBtn   ( this, ResId( CB_INDIVIDUALWORDS ) ),
    m_aEmphasisFT           ( this, ResId( FT_EMPHASIS ) ),
    m_aEmphasisLB           ( this, ResId( LB_EMPHASIS ) ),
    m_aPositionFT           ( this, ResId( FT_POSITION ) ),
    m_aPositionLB           ( this, ResId( LB_POSITION ) ),

    m_aEffectsFT            ( this, ResId( FT_EFFECTS ) ),
    m_aEffectsLB            ( this, ResId( LB_EFFECTS ) ),

    m_aEffects2LB           ( this, ResId( LB_EFFECTS2 ) ),

    m_aReliefFT             ( this, ResId( FT_RELIEF ) ),
    m_aReliefLB             ( this, ResId( LB_RELIEF ) ),

    m_aOutlineBtn           ( this, ResId( CB_OUTLINE ) ),
    m_aShadowBtn            ( this, ResId( CB_SHADOW ) ),
    m_aBlinkingBtn          ( this, ResId( CB_BLINKING ) ),

    m_aPreviewLine          ( this, ResId( FL_EFFECTS_PREVIEW ) ),
    m_aPreviewWin           ( this, ResId( WIN_EFFECTS_PREVIEW ) ),
    m_aFontTypeFT           ( this, ResId( FT_EFFECTS_FONTTYPE ) ),

    m_aTransparentColorName (       ResId( STR_EFFECTS_TRANSPARENT ) )

{
    m_aEffectsLB.InsertEntry( String( ResId( STR_EFFECTS_CAPITALS ) ) );
    m_aEffectsLB.SetEntryData( 0, (void*)(ULONG)STR_EFFECTS_CAPITALS );
    m_aEffectsLB.InsertEntry( String( ResId( STR_EFFECTS_LOWERCASE ) ) );
    m_aEffectsLB.SetEntryData( 1, (void*)(ULONG)STR_EFFECTS_LOWERCASE );
    m_aEffectsLB.InsertEntry( String( ResId( STR_EFFECTS_TITLE ) ) );
    m_aEffectsLB.SetEntryData( 2, (void*)(ULONG)STR_EFFECTS_TITLE );
    m_aEffectsLB.InsertEntry( String( ResId( STR_EFFECTS_SMALL ) ) );
    m_aEffectsLB.SetEntryData( 3, (void*)(ULONG)STR_EFFECTS_SMALL );
    m_aEffectsLB.InsertEntry( String( ResId( STR_EFFECTS_OUTLINE ) ) );
    m_aEffectsLB.SetEntryData( 4, (void*)(ULONG)STR_EFFECTS_OUTLINE );
    m_aEffectsLB.InsertEntry( String( ResId( STR_EFFECTS_SHADOW ) ) );
    m_aEffectsLB.SetEntryData( 5, (void*)(ULONG)STR_EFFECTS_SHADOW );
    m_aEffectsLB.InsertEntry( String( ResId( STR_EFFECTS_BLINKING ) ) );
    m_aEffectsLB.SetEntryData( 6, (void*)(ULONG)STR_EFFECTS_BLINKING );

    m_aEffectsLB.SetHighlightRange();
    m_aEffectsLB.SelectEntryPos( 0 );

    FreeResource();
    Initialize();
}

// -----------------------------------------------------------------------

void SvxCharEffectsPage::Initialize()
{
    // to handle the changes of the other pages
    SetExchangeSupport();

    // HTML-Mode
    const SfxPoolItem* pItem;
    SfxObjectShell* pShell;
    if ( SFX_ITEM_SET == GetItemSet().GetItemState( SID_HTML_MODE, FALSE, &pItem ) ||
         ( NULL != ( pShell = SfxObjectShell::Current() ) &&
           NULL != ( pItem = pShell->GetItem( SID_HTML_MODE ) ) ) )
    {
        m_nHtmlMode = ( (const SfxUInt16Item*)pItem )->GetValue();
        if ( ( m_nHtmlMode & HTMLMODE_ON ) == HTMLMODE_ON )
        {
            //!!! hide some controls please
        }
    }

    // fill the color box
    SfxObjectShell* pDocSh = SfxObjectShell::Current();
    DBG_ASSERT( pDocSh, "DocShell not found!" );
    XColorTable* pColorTable = NULL;
    FASTBOOL bKillTable = FALSE;

    if ( pDocSh && ( pItem = pDocSh->GetItem( SID_COLOR_TABLE ) ) )
        pColorTable = ( (SvxColorTableItem*)pItem )->GetColorTable();

    if ( !pColorTable )
    {
        pColorTable = new XColorTable( SvtPathOptions().GetPalettePath() );
        bKillTable = TRUE;
    }

    m_aColorLB.SetUpdateMode( FALSE );

    for ( long i = 0; i < pColorTable->Count(); i++ )
    {
        XColorEntry* pEntry = pColorTable->Get(i);
        m_aColorLB.InsertEntry( pEntry->GetColor(), pEntry->GetName() );
    }

    m_aColorLB.SetUpdateMode( TRUE );

    if ( bKillTable )
        delete pColorTable;

    // handler
    Link aLink = LINK( this, SvxCharEffectsPage, SelectHdl_Impl );
    m_aUnderlineLB.SetSelectHdl( aLink );
    m_aStrikeoutLB.SetSelectHdl( aLink );
    m_aEmphasisLB.SetSelectHdl( aLink );
    m_aPositionLB.SetSelectHdl( aLink );
    m_aEffects2LB.SetSelectHdl( aLink );
    m_aReliefLB.SetSelectHdl( aLink );

    m_aUnderlineLB.SelectEntryPos( 0 );
    m_aStrikeoutLB.SelectEntryPos( 0 );
    m_aEmphasisLB.SelectEntryPos( 0 );
    m_aPositionLB.SelectEntryPos( 0 );
    SelectHdl_Impl( NULL );
    SelectHdl_Impl( &m_aEmphasisLB );

    m_aEffects2LB.SelectEntryPos( 0 );

    aLink = LINK( this, SvxCharEffectsPage, ClickHdl_Impl );
    m_aOutlineBtn.SetClickHdl( aLink );
    m_aShadowBtn.SetClickHdl( aLink );
}

// -----------------------------------------------------------------------

void SvxCharEffectsPage::UpdatePreview_Impl()
{
    SvxFont& rFont = m_aPreviewWin.GetFont();
    USHORT nPos = m_aUnderlineLB.GetSelectEntryPos();
    FontUnderline eUnderline = (FontUnderline)(ULONG)m_aUnderlineLB.GetEntryData( nPos );
    nPos = m_aStrikeoutLB.GetSelectEntryPos();
    FontStrikeout eStrikeout = (FontStrikeout)(ULONG)m_aStrikeoutLB.GetEntryData( nPos );
    rFont.SetUnderline( eUnderline );
    rFont.SetStrikeout( eStrikeout );
    nPos = m_aPositionLB.GetSelectEntryPos();
    BOOL bUnder = ( CHRDLG_POSITION_UNDER == (ULONG)m_aPositionLB.GetEntryData( nPos ) );
    FontEmphasisMark eMark = (FontEmphasisMark)m_aEmphasisLB.GetSelectEntryPos();
    eMark |= bUnder ? EMPHASISMARK_POS_BELOW : EMPHASISMARK_POS_ABOVE;
    rFont.SetEmphasisMark( eMark );
    short nRelief = m_aReliefLB.GetSelectEntryPos();
    rFont.SetRelief( (FontRelief)nRelief );
    rFont.SetOutline( StateToAttr( m_aOutlineBtn.GetState() ) );
    rFont.SetShadow( StateToAttr( m_aShadowBtn.GetState() ) );
    USHORT nCapsPos = m_aEffects2LB.GetSelectEntryPos();
    if ( nCapsPos != LISTBOX_ENTRY_NOTFOUND )
        rFont.SetCaseMap( (SvxCaseMap)nCapsPos );
    m_aPreviewWin.Invalidate();
}

// -----------------------------------------------------------------------

void SvxCharEffectsPage::SetCaseMap_Impl( SvxCaseMap eCaseMap )
{
    if ( SVX_CASEMAP_END > eCaseMap )
        m_aEffects2LB.SelectEntryPos( eCaseMap );
    else
    {
        m_aEffects2LB.SetNoSelection();
        eCaseMap = SVX_CASEMAP_NOT_MAPPED;
    }

    UpdatePreview_Impl();
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxCharEffectsPage, SelectHdl_Impl, ListBox*, pBox )
{
    if ( &m_aEmphasisLB == pBox )
    {
        BOOL bEnable = ( m_aEmphasisLB.GetSelectEntryPos() > 0 );
        m_aPositionFT.Enable( bEnable );
        m_aPositionLB.Enable( bEnable );
    }
    else if( &m_aReliefLB == pBox)
    {
        BOOL bEnable = ( pBox->GetSelectEntryPos() == 0 );
        m_aOutlineBtn.Enable(bEnable);
        m_aShadowBtn.Enable(bEnable);
    }
    else if ( &m_aPositionLB != pBox )
    {
        BOOL bEnable = ( ( m_aUnderlineLB.GetSelectEntryPos() > 0 ) |
                         ( m_aStrikeoutLB.GetSelectEntryPos() > 0 ) );
        m_aColorFT.Enable( bEnable );
        m_aColorLB.Enable( bEnable );
        m_aIndividualWordsBtn.Enable( bEnable );
    }
    UpdatePreview_Impl();
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxCharEffectsPage, ClickHdl_Impl, TriStateBox*, EMPTYARG )
{
    UpdatePreview_Impl();
    return 0;
}

// -----------------------------------------------------------------------

void SvxCharEffectsPage::ActivatePage( const SfxItemSet& rSet )
{
    BOOL bInReset = FALSE; //!!!!
    SfxItemState eState = bInReset ? SFX_ITEM_DEFAULT : SFX_ITEM_SET;
    SvxFont& rFont = m_aPreviewWin.GetFont();

    // Font
    USHORT nWhich = GetWhich( SID_ATTR_CHAR_FONT );
    const SvxFontItem* pFontItem = NULL;
    if ( rSet.GetItemState( nWhich ) >= SFX_ITEM_DEFAULT )
    {
        pFontItem = (const SvxFontItem*)&rSet.Get( nWhich );
        rFont.SetFamily( pFontItem->GetFamily() );
        rFont.SetName( pFontItem->GetFamilyName() );
        rFont.SetPitch( pFontItem->GetPitch() );
        rFont.SetCharSet( pFontItem->GetCharSet() );
        rFont.SetStyleName( pFontItem->GetStyleName() );
    }

    // Style
    nWhich = GetWhich( SID_ATTR_CHAR_POSTURE );
    if ( rSet.GetItemState( nWhich ) >= eState )
    {
        const SvxPostureItem& rItem = (SvxPostureItem&)rSet.Get( nWhich );
        rFont.SetItalic( (FontItalic)rItem.GetValue() != ITALIC_NONE ? ITALIC_NORMAL : ITALIC_NONE );
    }
    nWhich = GetWhich( SID_ATTR_CHAR_WEIGHT );
    if ( rSet.GetItemState( nWhich ) >= eState )
    {
        SvxWeightItem& rItem = (SvxWeightItem&)rSet.Get( nWhich );
        rFont.SetWeight( (FontWeight)rItem.GetValue() != WEIGHT_NORMAL ? WEIGHT_BOLD : WEIGHT_NORMAL );
    }

    // Size
    nWhich = GetWhich( SID_ATTR_CHAR_FONTHEIGHT );
    if ( rSet.GetItemState( nWhich ) >= eState )
    {
        MapUnit eUnit = (MapUnit)rSet.GetPool()->GetMetric( nWhich );
        SvxFontHeightItem& rItem = (SvxFontHeightItem&)rSet.Get( nWhich );
        Size aSize( rFont.GetSize() );
        long nH = LogicToLogic( rItem.GetHeight(), eUnit, MAP_TWIP );
        aSize.Height() = nH;
        aSize.Width() = 0;
        rFont.SetSize( aSize );
    }
    else
        // as default 12pt
        rFont.SetSize( Size( 0, 240 ) );

    // Color
    nWhich = GetWhich( SID_ATTR_CHAR_COLOR );
    if ( rSet.GetItemState( nWhich ) >= eState )
    {
        const SvxColorItem& rItem = (SvxColorItem&)rSet.Get( nWhich );
        rFont.SetColor( rItem.GetValue() );
    }

    m_aPreviewWin.Invalidate();
}

// -----------------------------------------------------------------------

int SvxCharEffectsPage::DeactivatePage( SfxItemSet* pSet )
{
    if ( pSet )
        FillItemSet( *pSet );
    return LEAVE_PAGE;
}

// -----------------------------------------------------------------------

SfxTabPage* SvxCharEffectsPage::Create( Window* pParent, const SfxItemSet& rSet )
{
    return new SvxCharEffectsPage( pParent, rSet );
}

// -----------------------------------------------------------------------

USHORT* SvxCharEffectsPage::GetRanges()
{
    return NULL;
}

// -----------------------------------------------------------------------

void SvxCharEffectsPage::Reset( const SfxItemSet& rSet )
{
    SvxFont& rFont = m_aPreviewWin.GetFont();
    BOOL bEnable = FALSE;

    // Underline
    USHORT nWhich = GetWhich( SID_ATTR_CHAR_UNDERLINE );
    rFont.SetUnderline( UNDERLINE_NONE );
    m_aUnderlineLB.SelectEntryPos( 0 );
    SfxItemState eState = rSet.GetItemState( nWhich );

    if ( eState >= SFX_ITEM_DONTCARE )
    {
        if ( eState == SFX_ITEM_DONTCARE )
            m_aUnderlineLB.SetNoSelection();
        else
        {
            const SvxUnderlineItem& rItem = (SvxUnderlineItem&)rSet.Get( nWhich );
            FontUnderline eUnderline = (FontUnderline)rItem.GetValue();
            rFont.SetUnderline( eUnderline );

            if ( eUnderline != UNDERLINE_NONE )
            {
                for ( USHORT i = 0; i < m_aUnderlineLB.GetEntryCount(); ++i )
                {
                    if ( (FontUnderline)(ULONG)m_aUnderlineLB.GetEntryData(i) == eUnderline )
                    {
                        m_aUnderlineLB.SelectEntryPos(i);
                        bEnable |= TRUE;
                        break;
                    }
                }

                Color aColor = rItem.GetColor();
                USHORT nPos = m_aColorLB.GetEntryPos( aColor );
                if ( nPos == LISTBOX_ENTRY_NOTFOUND && aColor == Color( COL_TRANSPARENT ) )
                    nPos = m_aColorLB.GetEntryPos( m_aTransparentColorName );

                if ( LISTBOX_ENTRY_NOTFOUND != nPos )
                    m_aColorLB.SelectEntryPos( nPos );
                else
                {
                    nPos = m_aColorLB.GetEntryPos( aColor );
                    if ( LISTBOX_ENTRY_NOTFOUND != nPos )
                        m_aColorLB.SelectEntryPos( nPos );
                    else
                        m_aColorLB.SelectEntryPos(
                            m_aColorLB.InsertEntry( aColor, String( SVX_RES( RID_SVXSTR_COLOR_USER ) ) ) );
                }
            }
            else
            {
                m_aColorLB.SetNoSelection();
                m_aColorLB.Disable();
            }
        }
    }

    //  Strikeout
    nWhich = GetWhich( SID_ATTR_CHAR_STRIKEOUT );
    rFont.SetStrikeout( STRIKEOUT_NONE );
    m_aStrikeoutLB.SelectEntryPos( 0 );
    eState = rSet.GetItemState( nWhich );

    if ( eState >= SFX_ITEM_DONTCARE )
    {
        if ( eState == SFX_ITEM_DONTCARE )
            m_aStrikeoutLB.SetNoSelection();
        else
        {
            const SvxCrossedOutItem& rItem = (SvxCrossedOutItem&)rSet.Get( nWhich );
            FontStrikeout eStrikeout = (FontStrikeout)rItem.GetValue();
            rFont.SetStrikeout( eStrikeout );

            if ( eStrikeout != STRIKEOUT_NONE )
            {
                for ( USHORT i = 0; i < m_aStrikeoutLB.GetEntryCount(); ++i )
                {
                    if ( (FontStrikeout)(ULONG)m_aStrikeoutLB.GetEntryData(i) == eStrikeout )
                    {
                        m_aStrikeoutLB.SelectEntryPos(i);
                        bEnable |= TRUE;
                        break;
                    }
                }
            }
        }
    }

    // WordLineMode
    nWhich = GetWhich( SID_ATTR_CHAR_WORDLINEMODE );

    if ( rSet.GetItemState( nWhich ) >= SFX_ITEM_DEFAULT )
    {
        const SvxWordLineModeItem& rItem = (SvxWordLineModeItem&)rSet.Get( nWhich );
        rFont.SetWordLineMode( rItem.GetValue() );
        m_aIndividualWordsBtn.Check( rItem.GetValue() );
        m_aIndividualWordsBtn.Enable( bEnable );
    }

    // Emphasis
    nWhich = GetWhich( SID_ATTR_CHAR_EMPHASISMARK );

    if ( rSet.GetItemState( nWhich ) >= SFX_ITEM_DEFAULT )
    {
        const SvxEmphasisMarkItem& rItem = (SvxEmphasisMarkItem&)rSet.Get( nWhich );
        FontEmphasisMark eMark = rItem.GetEmphasisMark();
        rFont.SetEmphasisMark( eMark );
        m_aEmphasisLB.SelectEntryPos( (USHORT)( eMark & EMPHASISMARK_STYLE ) );
    }

    SelectHdl_Impl( NULL );
    SelectHdl_Impl( &m_aEmphasisLB );

    // Effects
    nWhich = GetWhich( SID_ATTR_CHAR_CASEMAP );
    SvxCaseMap eCaseMap = SVX_CASEMAP_END;

    if ( rSet.GetItemState( nWhich ) >= SFX_ITEM_DEFAULT )
    {
        const SvxCaseMapItem& rItem = (const SvxCaseMapItem&)rSet.Get( nWhich );
        eCaseMap = (SvxCaseMap)rItem.GetValue();
    }
    SetCaseMap_Impl( eCaseMap );

    //Relief
    nWhich = GetWhich(SID_ATTR_CHAR_RELIEF);
    if ( rSet.GetItemState( nWhich ) >= SFX_ITEM_DEFAULT )
    {
        const SvxCharReliefItem& rItem = (const SvxCharReliefItem&)rSet.Get( nWhich );
        m_aReliefLB.SelectEntryPos(rItem.GetValue());
        SelectHdl_Impl(&m_aReliefLB);
    }

    // Outline
    nWhich = GetWhich( SID_ATTR_CHAR_CONTOUR );
    eState = rSet.GetItemState( nWhich );

    switch ( eState )
    {
        case SFX_ITEM_UNKNOWN:
            m_aOutlineBtn.Hide();
            break;

        case SFX_ITEM_DISABLED:
        case SFX_ITEM_READONLY:
            m_aOutlineBtn.Disable();
            break;

        case SFX_ITEM_DONTCARE:
            m_aOutlineBtn.SetState( STATE_DONTKNOW );
            break;

        case SFX_ITEM_DEFAULT:
        case SFX_ITEM_SET:
        {
            const SvxContourItem& rItem = (SvxContourItem&)rSet.Get( nWhich );
            m_aOutlineBtn.SetState( (TriState)rItem.GetValue() );
            m_aOutlineBtn.EnableTriState( FALSE );
            break;
        }
    }

    // Shadow
    nWhich = GetWhich( SID_ATTR_CHAR_SHADOWED );
    eState = rSet.GetItemState( nWhich );

    switch ( eState )
    {
        case SFX_ITEM_UNKNOWN:
            m_aShadowBtn.Hide();
            break;

        case SFX_ITEM_DISABLED:
        case SFX_ITEM_READONLY:
            m_aShadowBtn.Disable();
            break;

        case SFX_ITEM_DONTCARE:
            m_aShadowBtn.SetState( STATE_DONTKNOW );
            break;

        case SFX_ITEM_DEFAULT:
        case SFX_ITEM_SET:
        {
            const SvxShadowedItem& rItem = (SvxShadowedItem&)rSet.Get( nWhich );
            m_aShadowBtn.SetState( (TriState)rItem.GetValue() );
            m_aShadowBtn.EnableTriState( FALSE );
            break;
        }
    }

    // Blinking
    nWhich = GetWhich( SID_ATTR_FLASH );

    if ( rSet.GetItemState( nWhich ) >= SFX_ITEM_DEFAULT )
    {
        const SvxBlinkItem& rItem = (SvxBlinkItem&)rSet.Get( nWhich );
        m_aBlinkingBtn.Check( rItem.GetValue() );
    }

    // preview update
    m_aPreviewWin.Invalidate();

    // save this settings
    m_aUnderlineLB.SaveValue();
    m_aColorLB.SaveValue();
    m_aStrikeoutLB.SaveValue();
    m_aIndividualWordsBtn.SaveValue();
    m_aEmphasisLB.SaveValue();
    m_aPositionLB.SaveValue();
    m_aEffects2LB.SaveValue();
    m_aReliefLB.SaveValue();
    m_aOutlineBtn.SaveValue();
    m_aShadowBtn.SaveValue();
    m_aBlinkingBtn.SaveValue();
}

// -----------------------------------------------------------------------

BOOL SvxCharEffectsPage::FillItemSet( SfxItemSet& rSet )
{
    const SfxPoolItem* pOld = 0;
    const SfxItemSet& rOldSet = GetItemSet();
    BOOL bModified = FALSE;
    FASTBOOL bChanged = TRUE;

    // Underline
    USHORT nWhich = GetWhich( SID_ATTR_CHAR_UNDERLINE );
    pOld = GetOldItem( rSet, SID_ATTR_CHAR_UNDERLINE );
    USHORT nPos = m_aUnderlineLB.GetSelectEntryPos();
    FontUnderline eUnder = (FontUnderline)(ULONG)m_aUnderlineLB.GetEntryData( nPos );

    if ( pOld )
    {
        const SvxUnderlineItem& rItem = *( (const SvxUnderlineItem*)pOld );
        if ( (FontUnderline)rItem.GetValue() == eUnder &&
             ( UNDERLINE_NONE == eUnder || rItem.GetColor() == m_aColorLB.GetSelectEntryColor() ) )
            bChanged = FALSE;
    }

    if ( bChanged )
    {
        SvxUnderlineItem aNewItem( eUnder, nWhich );
        aNewItem.SetColor( m_aColorLB.GetSelectEntryColor() );
        rSet.Put( aNewItem );
        bModified |= TRUE;
    }
     else if ( SFX_ITEM_DEFAULT == rOldSet.GetItemState( nWhich, FALSE ) )
        rSet.ClearItem( nWhich );

    bChanged = TRUE;

    // Strikeout
    nWhich = GetWhich( SID_ATTR_CHAR_STRIKEOUT );
    pOld = GetOldItem( rSet, SID_ATTR_CHAR_STRIKEOUT );
    nPos = m_aStrikeoutLB.GetSelectEntryPos();
    FontStrikeout eStrike = (FontStrikeout)(ULONG)m_aStrikeoutLB.GetEntryData( nPos );

    if ( pOld )
    {
        const SvxCrossedOutItem& rItem = *( (const SvxCrossedOutItem*)pOld );
        if ( !m_aStrikeoutLB.IsEnabled() || (FontStrikeout)rItem.GetValue() == eStrike )
            bChanged = FALSE;
    }

    if ( bChanged )
    {
        rSet.Put( SvxCrossedOutItem( eStrike, nWhich ) );
        bModified |= TRUE;
    }
    else if ( SFX_ITEM_DEFAULT == rOldSet.GetItemState( nWhich, FALSE ) )
        rSet.ClearItem( nWhich );

    bChanged = TRUE;

    // Individual words
    nWhich = GetWhich( SID_ATTR_CHAR_WORDLINEMODE );
    pOld = GetOldItem( rSet, SID_ATTR_CHAR_WORDLINEMODE );

    if ( pOld )
    {
        const SvxWordLineModeItem& rItem = *( (const SvxWordLineModeItem*)pOld );
        if ( rItem.GetValue() == m_aIndividualWordsBtn.IsChecked() )
            bChanged = FALSE;
    }

    if ( rOldSet.GetItemState( nWhich ) == SFX_ITEM_DONTCARE &&
         m_aIndividualWordsBtn.IsChecked() == m_aIndividualWordsBtn.GetSavedValue() )
        bChanged = FALSE;

    if ( bChanged )
    {
        rSet.Put( SvxWordLineModeItem( m_aIndividualWordsBtn.IsChecked(), nWhich ) );
        bModified |= TRUE;
    }
    else if ( SFX_ITEM_DEFAULT == rOldSet.GetItemState( nWhich, FALSE ) )
        rSet.ClearItem( nWhich );

    bChanged = TRUE;

    // Emphasis
    nWhich = GetWhich( SID_ATTR_CHAR_EMPHASISMARK );
    pOld = GetOldItem( rSet, SID_ATTR_CHAR_EMPHASISMARK );
    USHORT nMarkPos = m_aEmphasisLB.GetSelectEntryPos();
    USHORT nPosPos = m_aPositionLB.GetSelectEntryPos();
    FontEmphasisMark eMark = (FontEmphasisMark)nMarkPos;
    if ( m_aPositionLB.IsEnabled() )
    {
        eMark |= ( CHRDLG_POSITION_UNDER == (ULONG)m_aPositionLB.GetEntryData( nPosPos ) )
            ? EMPHASISMARK_POS_BELOW : EMPHASISMARK_POS_ABOVE;
    }

    if ( pOld )
    {
        const SvxEmphasisMarkItem& rItem = *( (const SvxEmphasisMarkItem*)pOld );
        if ( rItem.GetEmphasisMark() == eMark )
            bChanged = FALSE;
    }

    if ( rOldSet.GetItemState( nWhich ) == SFX_ITEM_DONTCARE &&
         m_aEmphasisLB.GetSavedValue() == nMarkPos && m_aPositionLB.GetSavedValue() == nPosPos )
        bChanged = FALSE;

    if ( bChanged )
    {
        rSet.Put( SvxEmphasisMarkItem( eMark, nWhich ) );
        bModified |= TRUE;
    }
    else if ( SFX_ITEM_DEFAULT == rOldSet.GetItemState( nWhich, FALSE ) )
        rSet.ClearItem( nWhich );

    bChanged = TRUE;

    // Effects
    nWhich = GetWhich( SID_ATTR_CHAR_CASEMAP );
    pOld = GetOldItem( rSet, SID_ATTR_CHAR_CASEMAP );
    SvxCaseMap eCaseMap = SVX_CASEMAP_NOT_MAPPED;
    FASTBOOL bChecked = FALSE;
    USHORT nCapsPos = m_aEffects2LB.GetSelectEntryPos();
    if ( nCapsPos != LISTBOX_ENTRY_NOTFOUND )
    {
        eCaseMap = (SvxCaseMap)nCapsPos;
        bChecked = TRUE;
    }

    if ( pOld )
    {
        const SvxCaseMapItem& rItem = *( (const SvxCaseMapItem*)pOld );

        if ( (SvxCaseMap)rItem.GetValue() == eCaseMap )
            bChanged = FALSE;
    }

    if ( bChanged && bChecked )
    {
        rSet.Put( SvxCaseMapItem( eCaseMap, nWhich ) );
        bModified |= TRUE;
    }
    else if ( SFX_ITEM_DEFAULT == rOldSet.GetItemState( nWhich, FALSE ) )
        rSet.ClearItem( nWhich );

    bChanged = TRUE;

    //Relief
    nWhich = GetWhich(SID_ATTR_CHAR_RELIEF);
    if(m_aReliefLB.GetSelectEntryPos() != m_aReliefLB.GetSavedValue())
    {
        SvxCharReliefItem aRelief((FontRelief)m_aReliefLB.GetSelectEntryPos(), nWhich);
        rSet.Put(aRelief);
    }

    // Outline
    const SfxItemSet* pExampleSet = GetTabDialog() ? GetTabDialog()->GetExampleSet() : NULL;
    nWhich = GetWhich( SID_ATTR_CHAR_CONTOUR );
    pOld = GetOldItem( rSet, SID_ATTR_CHAR_CONTOUR );
    TriState eState = m_aOutlineBtn.GetState();
    const SfxPoolItem* pItem;

    if ( pOld )
    {
        const SvxContourItem& rItem = *( (const SvxContourItem*)pOld );
        if ( rItem.GetValue() == StateToAttr( eState ) && m_aOutlineBtn.GetSavedValue() == eState )
            bChanged = FALSE;
    }

    if ( !bChanged && pExampleSet && pExampleSet->GetItemState( nWhich, FALSE, &pItem ) == SFX_ITEM_SET &&
         !StateToAttr( eState ) && ( (SvxContourItem*)pItem )->GetValue() )
        bChanged = TRUE;

    if ( bChanged && eState != STATE_DONTKNOW )
    {
        rSet.Put( SvxContourItem( StateToAttr( eState ), nWhich ) );
        bModified |= TRUE;
    }
    else if ( SFX_ITEM_DEFAULT == rOldSet.GetItemState( nWhich, FALSE ) )
        rSet.ClearItem( nWhich );

    bChanged = TRUE;

    // Shadow
    nWhich = GetWhich( SID_ATTR_CHAR_SHADOWED );
    pOld = GetOldItem( rSet, SID_ATTR_CHAR_SHADOWED );
    eState = m_aShadowBtn.GetState();

    if ( pOld )
    {
        const SvxShadowedItem& rItem = *( (const SvxShadowedItem*)pOld );
        if ( rItem.GetValue() == StateToAttr( eState ) && m_aShadowBtn.GetSavedValue() == eState )
            bChanged = FALSE;
    }

    if ( !bChanged && pExampleSet && pExampleSet->GetItemState( nWhich, FALSE, &pItem ) == SFX_ITEM_SET &&
         !StateToAttr( eState ) && ( (SvxShadowedItem*)pItem )->GetValue() )
        bChanged = TRUE;

    if ( bChanged && eState != STATE_DONTKNOW )
    {
        rSet.Put( SvxShadowedItem( StateToAttr( eState ), nWhich ) );
        bModified |= TRUE;
    }
    else if ( SFX_ITEM_DEFAULT == rOldSet.GetItemState( nWhich, FALSE ) )
        rSet.ClearItem( nWhich );

    // Blinking
    if ( m_aBlinkingBtn.IsChecked() != m_aBlinkingBtn.GetSavedValue() )
    {
        nWhich = GetWhich( SID_ATTR_FLASH );
        rSet.Put( SvxBlinkItem( m_aBlinkingBtn.IsChecked(), nWhich ) );
        bModified |= TRUE;
    }

    return bModified;
}

void SvxCharEffectsPage::DisableControls( USHORT nDisable )
{
    if ( ( DISABLE_CASEMAP & nDisable ) == DISABLE_CASEMAP )
    {
        m_aEffectsFT.Disable();
        m_aEffects2LB.Disable();
    }

    if ( ( DISABLE_WORDLINE & nDisable ) == DISABLE_WORDLINE )
        m_aIndividualWordsBtn.Disable();

    if ( ( DISABLE_BLINK & nDisable ) == DISABLE_BLINK )
        m_aBlinkingBtn.Disable();
}

void SvxCharEffectsPage::EnableFlash()
{
    if ( !( ( m_nHtmlMode & HTMLMODE_ON ) && !( m_nHtmlMode & HTMLMODE_BLINK ) ) )
        m_aBlinkingBtn.Show();
}

// class SvxCharPositionPage ---------------------------------------------

SvxCharPositionPage::SvxCharPositionPage( Window* pParent, const SfxItemSet& rInSet ) :

    SfxTabPage( pParent, SVX_RES( RID_SVXPAGE_CHAR_POSITION ), rInSet ),

    m_aPositionLine     ( this, ResId( FL_POSITION ) ),
    m_aHighPosBtn       ( this, ResId( RB_HIGHPOS ) ),
    m_aNormalPosBtn     ( this, ResId( RB_NORMALPOS ) ),
    m_aLowPosBtn        ( this, ResId( RB_LOWPOS ) ),
    m_aHighLowFT        ( this, ResId( FT_HIGHLOW ) ),
    m_aHighLowEdit      ( this, ResId( ED_HIGHLOW ) ),
    m_aHighLowRB        ( this, ResId( CB_HIGHLOW ) ),
    m_aFontSizeFT       ( this, ResId( FT_FONTSIZE ) ),
    m_aFontSizeEdit     ( this, ResId( ED_FONTSIZE ) ),
    m_aRotationScalingFL( this, ResId( FL_ROTATION_SCALING ) ),
    m_a0degRB           ( this, ResId( RB_0_DEG ) ),
    m_a90degRB          ( this, ResId( RB_90_DEG ) ),
    m_a270degRB         ( this, ResId( RB_270_DEG ) ),
    m_aFitToLineCB      ( this, ResId( CB_FIT_TO_LINE ) ),
    m_aScaleWidthFT     ( this, ResId( FT_SCALE_WIDTH ) ),
    m_aScaleWidthMF     ( this, ResId( MF_SCALE_WIDTH ) ),

    m_aKerningLine      ( this, ResId( FL_KERNING2 ) ),
    m_aKerningLB        ( this, ResId( LB_KERNING2 ) ),
    m_aKerningFT        ( this, ResId( FT_KERNING2 ) ),
    m_aKerningEdit      ( this, ResId( ED_KERNING2 ) ),
    m_aPairKerningBtn   ( this, ResId( CB_PAIRKERNING ) ),

    m_aPreviewLine      ( this, ResId( FL_POS_PREVIEW ) ),
    m_aPreviewWin       ( this, ResId( WIN_POS_PREVIEW ) ),
    m_aFontTypeFT       ( this, ResId( FT_POS_FONTTYPE ) ),

    m_nSuperEsc         ( (short)DFLT_ESC_SUPER ),
    m_nSubEsc           ( (short)DFLT_ESC_SUB ),
    m_nScaleWidthItemSetVal ( 100 ),
    m_nScaleWidthInitialVal ( 100 ),
    m_nSuperProp        ( (BYTE)DFLT_ESC_PROP ),
    m_nSubProp          ( (BYTE)DFLT_ESC_PROP )
{
    FreeResource();
    Initialize();
}

// -----------------------------------------------------------------------

void SvxCharPositionPage::Initialize()
{
    // to handle the changes of the other pages
    SetExchangeSupport();

    m_aPreviewWin.GetFont().SetSize( Size( 0, 240 ) );

    m_aNormalPosBtn.Check();
    PositionHdl_Impl( &m_aNormalPosBtn );
    m_aKerningLB.SelectEntryPos( 0 );
    KerningSelectHdl_Impl( NULL );

    Link aLink = LINK( this, SvxCharPositionPage, PositionHdl_Impl );
    m_aHighPosBtn.SetClickHdl( aLink );
    m_aNormalPosBtn.SetClickHdl( aLink );
    m_aLowPosBtn.SetClickHdl( aLink );

    aLink = LINK( this, SvxCharPositionPage, RotationHdl_Impl );
    m_a0degRB  .SetClickHdl( aLink );
    m_a90degRB .SetClickHdl( aLink );
    m_a270degRB.SetClickHdl( aLink );

    aLink = LINK( this, SvxCharPositionPage, FontModifyHdl_Impl );
    m_aHighLowEdit.SetModifyHdl( aLink );
    m_aFontSizeEdit.SetModifyHdl( aLink );

    m_aHighLowRB.SetClickHdl( LINK( this, SvxCharPositionPage, AutoPositionHdl_Impl ) );
    m_aFitToLineCB.SetClickHdl( LINK( this, SvxCharPositionPage, FitToLineHdl_Impl ) );
    m_aKerningLB.SetSelectHdl( LINK( this, SvxCharPositionPage, KerningSelectHdl_Impl ) );
    m_aKerningEdit.SetModifyHdl( LINK( this, SvxCharPositionPage, KerningModifyHdl_Impl ) );
    m_aPairKerningBtn.SetClickHdl( LINK( this, SvxCharPositionPage, PairKerningHdl_Impl ) );
}

// -----------------------------------------------------------------------

void SvxCharPositionPage::UpdatePreview_Impl( BYTE nProp, BYTE nEscProp, short nEsc )
{
    SvxFont& rFont = m_aPreviewWin.GetFont();
    rFont.SetPropr( nProp );
    rFont.SetProprRel( nEscProp );
    rFont.SetEscapement( nEsc );
    m_aPreviewWin.Invalidate();
}

// -----------------------------------------------------------------------

void SvxCharPositionPage::SetEscapement_Impl( USHORT nEsc )
{
    SvxEscapementItem aEscItm( (SvxEscapement)nEsc );

    if ( SVX_ESCAPEMENT_SUPERSCRIPT == nEsc )
    {
        aEscItm.GetEsc() = m_nSuperEsc;
        aEscItm.GetProp() = m_nSuperProp;
    }
    else if ( SVX_ESCAPEMENT_SUBSCRIPT == nEsc )
    {
        aEscItm.GetEsc() = m_nSubEsc;
        aEscItm.GetProp() = m_nSubProp;
    }

    short nFac = aEscItm.GetEsc() < 0 ? -1 : 1;

    m_aHighLowEdit.SetValue( aEscItm.GetEsc() * nFac );
    m_aFontSizeEdit.SetValue( aEscItm.GetProp() );

    if ( SVX_ESCAPEMENT_OFF == nEsc )
    {
        m_aHighLowFT.Disable();
        m_aHighLowEdit.Disable();
        m_aFontSizeFT.Disable();
        m_aFontSizeEdit.Disable();
        m_aHighLowRB.Disable();
    }
    else
    {
        m_aFontSizeFT.Enable();
        m_aFontSizeEdit.Enable();
        m_aHighLowRB.Enable();

        if ( !m_aHighLowRB.IsChecked() )
        {
            m_aHighLowFT.Enable();
            m_aHighLowEdit.Enable();
        }
        else
            AutoPositionHdl_Impl( &m_aHighLowRB );
    }

    UpdatePreview_Impl( 100, aEscItm.GetProp(), aEscItm.GetEsc() );
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxCharPositionPage, PositionHdl_Impl, RadioButton*, pBtn )
{
    USHORT nEsc = SVX_ESCAPEMENT_OFF;   // also when pBtn == NULL

    if ( &m_aHighPosBtn == pBtn )
        nEsc = SVX_ESCAPEMENT_SUPERSCRIPT;
    else if ( &m_aLowPosBtn == pBtn )
        nEsc = SVX_ESCAPEMENT_SUBSCRIPT;

    SetEscapement_Impl( nEsc );
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxCharPositionPage, RotationHdl_Impl, RadioButton*, pBtn )
{
    BOOL bEnable = FALSE;
    if (&m_a90degRB == pBtn  ||  &m_a270degRB == pBtn)
        bEnable = TRUE;
    else if (&m_a0degRB != pBtn)
        DBG_ERROR( "unexpected button" );
    m_aFitToLineCB.Enable( bEnable );
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxCharPositionPage, FontModifyHdl_Impl, MetricField*, EMPTYARG )
{
    BYTE nEscProp = (BYTE)m_aFontSizeEdit.GetValue();
    short nEsc  = (short)m_aHighLowEdit.GetValue();
    nEsc *= m_aLowPosBtn.IsChecked() ? -1 : 1;
    UpdatePreview_Impl( 100, nEscProp, nEsc );
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxCharPositionPage, AutoPositionHdl_Impl, CheckBox*, pBox )
{
    if ( pBox->IsChecked() )
    {
        m_aHighLowFT.Disable();
        m_aHighLowEdit.Disable();
    }
    else
        PositionHdl_Impl( m_aHighPosBtn.IsChecked() ? &m_aHighPosBtn
                                                      : m_aLowPosBtn.IsChecked() ? &m_aLowPosBtn
                                                                                   : &m_aNormalPosBtn );
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxCharPositionPage, FitToLineHdl_Impl, CheckBox*, pBox )
{
    if ( &m_aFitToLineCB == pBox)
    {
        UINT16 nVal = m_nScaleWidthInitialVal;
        if (m_aFitToLineCB.IsChecked())
            nVal = m_nScaleWidthItemSetVal;
        m_aScaleWidthMF.SetValue( nVal );
    }
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxCharPositionPage, KerningSelectHdl_Impl, ListBox*, EMPTYARG )
{
    if ( m_aKerningLB.GetSelectEntryPos() > 0 )
    {
        m_aKerningFT.Enable();
        m_aKerningEdit.Enable();

        if ( m_aKerningLB.GetSelectEntryPos() == 2 )
        {
            // Condensed -> max value == 1/6 of the current font height
            SvxFont& rFont = m_aPreviewWin.GetFont();
            long nMax = rFont.GetSize().Height() / 6;
            m_aKerningEdit.SetMax( m_aKerningEdit.Normalize( nMax ), FUNIT_TWIP );
            m_aKerningEdit.SetLast( m_aKerningEdit.GetMax( m_aKerningEdit.GetUnit() ) );
        }
        else
        {
            m_aKerningEdit.SetMax( 9999 );
            m_aKerningEdit.SetLast( 9999 );
        }
    }
    else
    {
        m_aKerningEdit.SetValue( 0 );
        m_aKerningFT.Disable();
        m_aKerningEdit.Disable();
    }

    KerningModifyHdl_Impl( NULL );

    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxCharPositionPage, KerningModifyHdl_Impl, MetricField*, EMPTYARG )
{
    long nVal = m_aKerningEdit.GetValue();
    nVal = LogicToLogic( nVal, MAP_POINT, MAP_TWIP );
    long nKern = (short)m_aKerningEdit.Denormalize( nVal );

    // Condensed? -> then negative
    if ( m_aKerningLB.GetSelectEntryPos() == 2 )
        nKern *= -1;

    SvxFont& rFont = m_aPreviewWin.GetFont();
    rFont.SetFixKerning( (short)nKern );
    m_aPreviewWin.Invalidate();
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxCharPositionPage, PairKerningHdl_Impl, CheckBox*, EMPTYARG )
{
    return 0;
}

// -----------------------------------------------------------------------

void SvxCharPositionPage::ActivatePage( const SfxItemSet& rSet )
{
    BOOL bInReset = FALSE; //!!!!
    SfxItemState eState = bInReset ? SFX_ITEM_DEFAULT : SFX_ITEM_SET;
    SvxFont& rFont = m_aPreviewWin.GetFont();

    // Font
    USHORT nWhich = GetWhich( SID_ATTR_CHAR_FONT );
    const SvxFontItem* pFontItem = NULL;
    if ( rSet.GetItemState( nWhich ) >= SFX_ITEM_DEFAULT )
    {
        pFontItem = (const SvxFontItem*)&rSet.Get( nWhich );
        rFont.SetFamily( pFontItem->GetFamily() );
        rFont.SetName( pFontItem->GetFamilyName() );
        rFont.SetPitch( pFontItem->GetPitch() );
        rFont.SetCharSet( pFontItem->GetCharSet() );
        rFont.SetStyleName( pFontItem->GetStyleName() );
    }

    // Style
    nWhich = GetWhich( SID_ATTR_CHAR_POSTURE );
    if ( rSet.GetItemState( nWhich ) >= eState )
    {
        const SvxPostureItem& rItem = (SvxPostureItem&)rSet.Get( nWhich );
        rFont.SetItalic( (FontItalic)rItem.GetValue() != ITALIC_NONE ? ITALIC_NORMAL : ITALIC_NONE );
    }
    nWhich = GetWhich( SID_ATTR_CHAR_WEIGHT );
    if ( rSet.GetItemState( nWhich ) >= eState )
    {
        SvxWeightItem& rItem = (SvxWeightItem&)rSet.Get( nWhich );
        rFont.SetWeight( (FontWeight)rItem.GetValue() != WEIGHT_NORMAL ? WEIGHT_BOLD : WEIGHT_NORMAL );
    }

    // Size
    nWhich = GetWhich( SID_ATTR_CHAR_FONTHEIGHT );
    if ( rSet.GetItemState( nWhich ) >= eState )
    {
        MapUnit eUnit = (MapUnit)rSet.GetPool()->GetMetric( nWhich );
        SvxFontHeightItem& rItem = (SvxFontHeightItem&)rSet.Get( nWhich );
        Size aSize( rFont.GetSize() );
        long nH = LogicToLogic( rItem.GetHeight(), eUnit, MAP_TWIP );
        aSize.Height() = nH;
        aSize.Width() = 0;
        rFont.SetSize( aSize );
    }
    else
        // as default 12pt
        rFont.SetSize( Size( 0, 240 ) );

    // Color
    nWhich = GetWhich( SID_ATTR_CHAR_COLOR );
    if ( rSet.GetItemState( nWhich ) >= eState )
    {
        const SvxColorItem& rItem = (SvxColorItem&)rSet.Get( nWhich );
        rFont.SetColor( rItem.GetValue() );
    }

    // Underline
    nWhich = GetWhich( SID_ATTR_CHAR_UNDERLINE );
    if ( rSet.GetItemState( nWhich ) >= SFX_ITEM_DEFAULT )
    {
        const SvxUnderlineItem& rItem = (SvxUnderlineItem&)rSet.Get( nWhich );
        FontUnderline eUnderline = (FontUnderline)rItem.GetValue();
        rFont.SetUnderline( eUnderline );
    }
    else
        rFont.SetUnderline( UNDERLINE_NONE );

    //  Strikeout
    nWhich = GetWhich( SID_ATTR_CHAR_STRIKEOUT );
    if ( rSet.GetItemState( nWhich ) >= SFX_ITEM_DEFAULT )
    {
        const SvxCrossedOutItem& rItem = (SvxCrossedOutItem&)rSet.Get( nWhich );
        FontStrikeout eStrikeout = (FontStrikeout)rItem.GetValue();
        rFont.SetStrikeout( eStrikeout );
    }
    else
        rFont.SetStrikeout( STRIKEOUT_NONE );

    // WordLineMode
    nWhich = GetWhich( SID_ATTR_CHAR_WORDLINEMODE );
    if ( rSet.GetItemState( nWhich ) >= SFX_ITEM_DEFAULT )
    {
        const SvxWordLineModeItem& rItem = (SvxWordLineModeItem&)rSet.Get( nWhich );
        rFont.SetWordLineMode( rItem.GetValue() );
    }

    // Emphasis
    nWhich = GetWhich( SID_ATTR_CHAR_EMPHASISMARK );
    if ( rSet.GetItemState( nWhich ) >= SFX_ITEM_DEFAULT )
    {
        const SvxEmphasisMarkItem& rItem = (SvxEmphasisMarkItem&)rSet.Get( nWhich );
        FontEmphasisMark eMark = rItem.GetEmphasisMark();
        rFont.SetEmphasisMark( eMark );
    }

    // Effects
    nWhich = GetWhich( SID_ATTR_CHAR_CASEMAP );

    if ( rSet.GetItemState( nWhich ) >= SFX_ITEM_DEFAULT )
    {
        const SvxCaseMapItem& rItem = (const SvxCaseMapItem&)rSet.Get( nWhich );
        SvxCaseMap eCaseMap = (SvxCaseMap)rItem.GetValue();
        rFont.SetCaseMap( eCaseMap );
    }

    // Outline
    nWhich = GetWhich( SID_ATTR_CHAR_CONTOUR );
    if ( rSet.GetItemState( nWhich ) >= SFX_ITEM_DEFAULT )
    {
        const SvxContourItem& rItem = (SvxContourItem&)rSet.Get( nWhich );
        rFont.SetOutline( rItem.GetValue() );
    }

    // Shadow
    nWhich = GetWhich( SID_ATTR_CHAR_SHADOWED );
    if ( rSet.GetItemState( nWhich ) >= SFX_ITEM_DEFAULT )
    {
        const SvxShadowedItem& rItem = (SvxShadowedItem&)rSet.Get( nWhich );
        rFont.SetShadow( rItem.GetValue() );
    }

    m_aPreviewWin.Invalidate();
}

// -----------------------------------------------------------------------

int SvxCharPositionPage::DeactivatePage( SfxItemSet* pSet )
{
    if ( pSet )
        FillItemSet( *pSet );
    return LEAVE_PAGE;
}

// -----------------------------------------------------------------------

SfxTabPage* SvxCharPositionPage::Create( Window* pParent, const SfxItemSet& rSet )
{
    return new SvxCharPositionPage( pParent, rSet );
}

// -----------------------------------------------------------------------

USHORT* SvxCharPositionPage::GetRanges()
{
    return NULL;
}

// -----------------------------------------------------------------------

void SvxCharPositionPage::Reset( const SfxItemSet& rSet )
{
    String sUser = GetUserData();

    if ( sUser.Len() )
    {
        m_nSuperEsc = (short)sUser.GetToken( 0 ).ToInt32();
        m_nSubEsc = (short)sUser.GetToken( 1 ).ToInt32();
        m_nSuperProp = (BYTE)sUser.GetToken( 2 ).ToInt32();
        m_nSubProp = (BYTE)sUser.GetToken( 3 ).ToInt32();
    }

    short nEsc = 0;
    BYTE nEscProp = 100;
    BYTE nProp = 100;

    m_aHighLowFT.Disable();
    m_aHighLowEdit.Disable();
    m_aFontSizeFT.Disable();
    m_aFontSizeEdit.Disable();

    SvxFont& rFont = m_aPreviewWin.GetFont();
    USHORT nWhich = GetWhich( SID_ATTR_CHAR_ESCAPEMENT );

    if ( rSet.GetItemState( nWhich ) >= SFX_ITEM_DEFAULT )
    {
        const SvxEscapementItem& rItem = (SvxEscapementItem&)rSet.Get( nWhich );
        nEsc = rItem.GetEsc();
        nEscProp = rItem.GetProp();

        if ( nEsc != 0 )
        {
            m_aHighLowFT.Enable();
            m_aHighLowEdit.Enable();
            m_aFontSizeFT.Enable();
            m_aFontSizeEdit.Enable();

            short nFac;

            if ( nEsc > 0 )
            {
                nFac = 1;
                m_aHighPosBtn.Check( TRUE );
                if ( nEsc == DFLT_ESC_AUTO_SUPER )
                {
                    m_aHighLowRB.Check( TRUE );
                    nEsc = DFLT_ESC_SUPER;
                }
            }
            else
            {
                nFac = -1;
                m_aLowPosBtn.Check( TRUE );
                if ( nEsc == DFLT_ESC_AUTO_SUB )
                {
                    m_aHighLowRB.Check(TRUE);
                    nEsc = DFLT_ESC_SUB;
                }
            }

            if ( m_aHighLowRB.IsChecked() )
            {
                m_aHighLowFT.Disable();
                m_aHighLowEdit.Disable();
            }
            m_aHighLowEdit.SetValue( m_aHighLowEdit.Normalize( nFac * nEsc ) );
            m_aFontSizeEdit.SetValue( m_aFontSizeEdit.Normalize( nEscProp ) );
        }
        else
        {
            m_aNormalPosBtn.Check( TRUE );
            m_aHighLowRB.Check( TRUE );
            PositionHdl_Impl( NULL );
        }
    }
    else
    {
        m_aHighPosBtn.Check( FALSE );
        m_aNormalPosBtn.Check( FALSE );
        m_aLowPosBtn.Check( FALSE );
    }

    // BspFont setzen
    rFont.SetPropr( nProp );
    rFont.SetProprRel( nEscProp );
    rFont.SetEscapement( nEsc );

    // Kerning
    nWhich = GetWhich( SID_ATTR_CHAR_KERNING );

    if ( rSet.GetItemState( nWhich ) >= SFX_ITEM_DEFAULT )
    {
        const SvxKerningItem& rItem = (SvxKerningItem&)rSet.Get( nWhich );
        SfxMapUnit eUnit = rSet.GetPool()->GetMetric( nWhich );
        MapUnit eOrgUnit = (MapUnit)eUnit;
        MapUnit ePntUnit( MAP_POINT );
        long nBig = m_aKerningEdit.Normalize( (long)rItem.GetValue() );
        long nKerning = LogicToLogic( nBig, eOrgUnit, ePntUnit );

        // Kerning am Font setzen, vorher in Twips umrechnen
        long nKern = LogicToLogic( rItem.GetValue(), (MapUnit)eUnit, MAP_TWIP );
        rFont.SetFixKerning( (short)nKern );

        if ( nKerning > 0 )
        {
            m_aKerningFT.Enable();
            m_aKerningEdit.Enable();
            m_aKerningEdit.SetValue( nKerning );
            m_aKerningLB.SelectEntryPos( LW_GESPERRT );
        }
        else if ( nKerning < 0 )
        {
            m_aKerningFT.Enable();
            m_aKerningEdit.Enable();
            m_aKerningEdit.SetValue( -nKerning );
            m_aKerningLB.SelectEntryPos( LW_SCHMAL );
            long nMax = rFont.GetSize().Height() / 6;
            m_aKerningEdit.SetMax( m_aKerningEdit.Normalize( nMax ), FUNIT_TWIP );
            m_aKerningEdit.SetLast( m_aKerningEdit.GetMax( m_aKerningEdit.GetUnit() ) );
        }
        else
        {
            m_aKerningFT.Disable();
            m_aKerningEdit.Disable();
            m_aKerningEdit.SetValue( 0 );
            m_aKerningLB.SelectEntryPos( LW_NORMAL );
            m_aKerningEdit.SetMax( 9999 );
            m_aKerningEdit.SetLast( 9999 );
        }
    }
    else
        m_aKerningEdit.SetText( String() );

    // Pair kerning
    nWhich = GetWhich( SID_ATTR_CHAR_AUTOKERN );

    if ( rSet.GetItemState( nWhich ) >= SFX_ITEM_DEFAULT )
    {
        const SvxAutoKernItem& rItem = (SvxAutoKernItem&)rSet.Get( nWhich );
        m_aPairKerningBtn.Check( rItem.GetValue() );
    }
    else
        m_aPairKerningBtn.Check( FALSE );

    // Scale Width
    nWhich = GetWhich( SID_ATTR_CHAR_SCALEWIDTH );
    if ( rSet.GetItemState( nWhich ) >= SFX_ITEM_DEFAULT )
    {
        const SvxCharScaleWidthItem& rItem =
                (SvxCharScaleWidthItem&) rSet.Get( nWhich );
        m_nScaleWidthInitialVal = rItem.GetValue();
        m_aScaleWidthMF.SetValue( m_nScaleWidthInitialVal );
    }
    else
        m_aScaleWidthMF.SetValue( 100 );

    nWhich = GetWhich( SID_ATTR_CHAR_WIDTH_FIT_TO_LINE );
    if ( rSet.GetItemState( nWhich ) >= SFX_ITEM_DEFAULT )
    {
        m_nScaleWidthItemSetVal = ((SfxUInt16Item&) rSet.Get( nWhich )).GetValue();
    }

    // Rotation
    nWhich = GetWhich( SID_ATTR_CHAR_ROTATED );
    Link aOldLink( m_aFitToLineCB.GetClickHdl() );
    m_aFitToLineCB.SetClickHdl( Link() );
    if ( rSet.GetItemState( nWhich ) >= SFX_ITEM_DEFAULT )
    {
        const SvxCharRotateItem& rItem =
                (SvxCharRotateItem&) rSet.Get( nWhich );
        if (rItem.IsBottomToTop())
            m_a90degRB.Check( TRUE );
        else if (rItem.IsTopToBotton())
            m_a270degRB.Check( TRUE );
        else
        {
            DBG_ASSERT( 0 == rItem.GetValue(), "incorrect value" );
            m_a0degRB.Check( TRUE );
        }
        m_aFitToLineCB.Check( rItem.IsFitToLine() );
    }
    else
    {
        m_a0degRB.Check( TRUE );
        m_aFitToLineCB.Check( FALSE );
    }
    m_aFitToLineCB.SetClickHdl( aOldLink );

    m_aFitToLineCB.Enable( !m_a0degRB.IsChecked() );

    m_aHighPosBtn.SaveValue();
    m_aNormalPosBtn.SaveValue();
    m_aLowPosBtn.SaveValue();
    m_a0degRB.SaveValue();
    m_a90degRB.SaveValue();
    m_a270degRB.SaveValue();
    m_aFitToLineCB.SaveValue();
    m_aScaleWidthMF.SaveValue();
    m_aKerningLB.SaveValue();
    m_aKerningEdit.SaveValue();
    m_aPairKerningBtn.SaveValue();
}

// -----------------------------------------------------------------------

BOOL SvxCharPositionPage::FillItemSet( SfxItemSet& rSet )
{
    //  Position (hoch, normal oder tief)
    const SfxItemSet& rOldSet = GetItemSet();
    BOOL bModified = FALSE, bChanged = TRUE;
    USHORT nWhich = GetWhich( SID_ATTR_CHAR_ESCAPEMENT );
    const SfxPoolItem* pOld = GetOldItem( rSet, SID_ATTR_CHAR_ESCAPEMENT );
    const FASTBOOL bHigh = m_aHighPosBtn.IsChecked();
    short nEsc;
    BYTE  nEscProp;

    if ( bHigh || m_aLowPosBtn.IsChecked() )
    {
        if ( m_aHighLowRB.IsChecked() )
            nEsc = bHigh ? DFLT_ESC_AUTO_SUPER : DFLT_ESC_AUTO_SUB;
        else
        {
            nEsc = (short)m_aHighLowEdit.Denormalize( m_aHighLowEdit.GetValue() );
            nEsc *= bHigh ? 1 : -1;
        }
        nEscProp = (BYTE)m_aFontSizeEdit.Denormalize( m_aFontSizeEdit.GetValue() );
    }
    else
    {
        nEsc  = 0;
        nEscProp = 100;
    }

    if ( pOld )
    {
        const SvxEscapementItem& rItem = *( (const SvxEscapementItem*)pOld );
        if ( rItem.GetEsc() == nEsc && rItem.GetProp() == nEscProp  )
            bChanged = FALSE;
    }

    if ( !bChanged && !m_aHighPosBtn.GetSavedValue() &&
         !m_aNormalPosBtn.GetSavedValue() && !m_aLowPosBtn.GetSavedValue() )
        bChanged = TRUE;

    if ( bChanged &&
         ( m_aHighPosBtn.IsChecked() || m_aNormalPosBtn.IsChecked() || m_aLowPosBtn.IsChecked() ) )
    {
        rSet.Put( SvxEscapementItem( nEsc, nEscProp, nWhich ) );
        bModified |= TRUE;
    }
    else if ( SFX_ITEM_DEFAULT == rOldSet.GetItemState( nWhich, FALSE ) )
        rSet.ClearItem( nWhich );

    bChanged = TRUE;

    // Kerning
    nWhich = GetWhich( SID_ATTR_CHAR_KERNING );
    pOld = GetOldItem( rSet, SID_ATTR_CHAR_KERNING );
    USHORT nPos = m_aKerningLB.GetSelectEntryPos();
    short nKerning = 0;
    SfxMapUnit eUnit = rSet.GetPool()->GetMetric( nWhich );

    if ( nPos == LW_GESPERRT || nPos == LW_SCHMAL )
    {
        long nTmp = m_aKerningEdit.GetValue();
        long nVal = LogicToLogic( nTmp, MAP_POINT, (MapUnit)eUnit );
        nKerning = (short)m_aKerningEdit.Denormalize( nVal );

        if ( nPos == LW_SCHMAL )
            nKerning *= - 1;
    }

    if ( pOld )
    {
        const SvxKerningItem& rItem = *( (const SvxKerningItem*)pOld );
        if ( rItem.GetValue() == nKerning )
            bChanged = FALSE;
    }

    if ( !bChanged &&
         ( m_aKerningLB.GetSavedValue() == LISTBOX_ENTRY_NOTFOUND ||
           !m_aKerningEdit.GetSavedValue().Len() ) )
        bChanged = TRUE;

    if ( bChanged && nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        rSet.Put( SvxKerningItem( nKerning, nWhich ) );
        bModified |= TRUE;
    }
    else if ( SFX_ITEM_DEFAULT == rOldSet.GetItemState( nWhich, FALSE ) )
        rSet.ClearItem( nWhich );

    bChanged = TRUE;

    // Pair-Kerning
    nWhich = GetWhich( SID_ATTR_CHAR_AUTOKERN );

    if ( m_aPairKerningBtn.IsChecked() != m_aPairKerningBtn.GetSavedValue() )
    {
        rSet.Put( SvxAutoKernItem( m_aPairKerningBtn.IsChecked(), nWhich ) );
        bModified |= TRUE;
    }
    else if ( SFX_ITEM_DEFAULT == rOldSet.GetItemState( nWhich, FALSE ) )
        rSet.ClearItem( nWhich );

    // Scale Width
    nWhich = GetWhich( SID_ATTR_CHAR_SCALEWIDTH );
    if ( m_aScaleWidthMF.GetText() != m_aScaleWidthMF.GetSavedValue() )
    {
        rSet.Put( SvxCharScaleWidthItem( m_aScaleWidthMF.GetValue(), nWhich ) );
        bModified |= TRUE;
    }
    else if ( SFX_ITEM_DEFAULT == rOldSet.GetItemState( nWhich, FALSE ) )
        rSet.ClearItem( nWhich );

    // Rotation
    nWhich = GetWhich( SID_ATTR_CHAR_ROTATED );
    if ( m_a0degRB     .IsChecked() != m_a0degRB     .GetSavedValue()  ||
         m_a90degRB    .IsChecked() != m_a90degRB    .GetSavedValue()  ||
         m_a270degRB   .IsChecked() != m_a270degRB   .GetSavedValue()  ||
         m_aFitToLineCB.IsChecked() != m_aFitToLineCB.GetSavedValue() )
    {
        SvxCharRotateItem aItem( 0, m_aFitToLineCB.IsChecked(), nWhich );
        sal_uInt16 nVal = 0;
        if (m_a90degRB.IsChecked())
            aItem.SetBottomToTop();
        else if (m_a270degRB.IsChecked())
            aItem.SetTopToBotton();
        rSet.Put( aItem );
        bModified |= TRUE;
    }
    else if ( SFX_ITEM_DEFAULT == rOldSet.GetItemState( nWhich, FALSE ) )
        rSet.ClearItem( nWhich );

    return bModified;
}

// -----------------------------------------------------------------------

void SvxCharPositionPage::FillUserData()
{
    const sal_Unicode cTok = ';';

    String sUser( UniString::CreateFromInt32( m_nSuperEsc ) );
    sUser.Append( cTok );
    sUser.Append( UniString::CreateFromInt32( m_nSubEsc ) );
    sUser.Append( cTok );
    sUser.Append( UniString::CreateFromInt32( m_nSuperProp ) );
    sUser.Append( cTok );
    sUser.Append( UniString::CreateFromInt32( m_nSubProp ) );
    SetUserData( sUser );
}

// class SvxCharTwoLinesPage ------------------------------------------------

SvxCharTwoLinesPage::SvxCharTwoLinesPage( Window* pParent, const SfxItemSet& rInSet ) :

    SfxTabPage( pParent, SVX_RES( RID_SVXPAGE_CHAR_TWOLINES ), rInSet ),

    m_aSwitchOnLine     ( this, ResId( FL_SWITCHON ) ),
    m_aTwoLinesBtn      ( this, ResId( CB_TWOLINES ) ),

    m_aEncloseLine      ( this, ResId( FL_ENCLOSE ) ),
    m_aStartBracketFT   ( this, ResId( FT_STARTBRACKET ) ),
    m_aStartBracketLB   ( this, ResId( ED_STARTBRACKET ) ),
    m_aEndBracketFT     ( this, ResId( FT_ENDBRACKET ) ),
    m_aEndBracketLB     ( this, ResId( ED_ENDBRACKET ) ),

    m_aPreviewLine      ( this, ResId( FL_TWOLINES_PREVIEW ) ),
    m_aPreviewWin       ( this, ResId( WIN_TWOLINES_PREVIEW ) ),
    m_aFontTypeFT       ( this, ResId( FT_TWOLINES_FONTTYPE ) )

{
    FreeResource();
    Initialize();
}

// -----------------------------------------------------------------------

void SvxCharTwoLinesPage::Initialize()
{
    Size aSize = m_aStartBracketLB.GetSizePixel();
    aSize.Height() = m_aStartBracketLB.CalcSize( 1, 2 ).Height();
    m_aStartBracketLB.SetSizePixel( aSize );
    aSize = m_aEndBracketLB.GetSizePixel();
    aSize.Height() = m_aEndBracketLB.CalcSize( 1, 2 ).Height();
    m_aEndBracketLB.SetSizePixel( aSize );

    m_aTwoLinesBtn.Check( FALSE );
    TwoLinesHdl_Impl( NULL );

    m_aTwoLinesBtn.SetClickHdl( LINK( this, SvxCharTwoLinesPage, TwoLinesHdl_Impl ) );

    Link aLink = LINK( this, SvxCharTwoLinesPage, CharacterMapHdl_Impl );
    m_aStartBracketLB.SetSelectHdl( aLink );
    m_aEndBracketLB.SetSelectHdl( aLink );

    SvxFont& rFont = m_aPreviewWin.GetFont();
    rFont.SetSize( Size( 0, 220 ) );
}

// -----------------------------------------------------------------------

void SvxCharTwoLinesPage::SelectCharacter( ListBox* pBox )
{
    SvxCharacterMap aDlg( this );
    aDlg.DisableFontSelection();

    if ( aDlg.Execute() == RET_OK )
    {
        sal_Unicode cChar = aDlg.GetChar();
        USHORT nPos = pBox->InsertEntry( String( cChar ) );
        pBox->SelectEntryPos( nPos );
    }
}

// -----------------------------------------------------------------------

void SvxCharTwoLinesPage::SetBracket( sal_Unicode cBracket, BOOL bStart )
{
    ListBox* pBox = bStart ? &m_aStartBracketLB : &m_aEndBracketLB;
    if ( 0 == cBracket )
        pBox->SelectEntryPos(0);
    else
    {
        FASTBOOL bFound = FALSE;
        for ( USHORT i = 1; i < pBox->GetEntryCount(); ++i )
        {
            if ( (ULONG)pBox->GetEntryData(i) != CHRDLG_ENCLOSE_SPECIAL_CHAR )
            {
                const sal_Unicode cChar = pBox->GetEntry(i).GetChar(0);
                if ( cChar == cBracket )
                {
                    pBox->SelectEntryPos(i);
                    bFound = TRUE;
                    break;
                }
            }
        }

        if ( !bFound )
            pBox->SelectEntryPos( pBox->InsertEntry( String( cBracket ) ) );
    }
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxCharTwoLinesPage, TwoLinesHdl_Impl, CheckBox*, EMPTYARG )
{
    BOOL bChecked = m_aTwoLinesBtn.IsChecked();
    m_aStartBracketFT.Enable( bChecked );
    m_aStartBracketLB.Enable( bChecked );
    m_aEndBracketFT.Enable( bChecked );
    m_aEndBracketLB.Enable( bChecked );

    UpdatePreview_Impl();
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxCharTwoLinesPage, CharacterMapHdl_Impl, ListBox*, pBox )
{
    USHORT nPos = pBox->GetSelectEntryPos();
    if ( CHRDLG_ENCLOSE_SPECIAL_CHAR == (ULONG)pBox->GetEntryData( nPos ) )
        SelectCharacter( pBox );
    UpdatePreview_Impl();
    return 0;
}

// -----------------------------------------------------------------------

void SvxCharTwoLinesPage::ActivatePage( const SfxItemSet& rSet )
{
    BOOL bInReset = FALSE; //!!!!
    SfxItemState eState = bInReset ? SFX_ITEM_DEFAULT : SFX_ITEM_SET;
    SvxFont& rFont = m_aPreviewWin.GetFont();

    // Font
    USHORT nWhich = GetWhich( SID_ATTR_CHAR_FONT );
    const SvxFontItem* pFontItem = NULL;
    if ( rSet.GetItemState( nWhich ) >= SFX_ITEM_DEFAULT )
    {
        pFontItem = (const SvxFontItem*)&rSet.Get( nWhich );
        rFont.SetFamily( pFontItem->GetFamily() );
        rFont.SetName( pFontItem->GetFamilyName() );
        rFont.SetPitch( pFontItem->GetPitch() );
        rFont.SetCharSet( pFontItem->GetCharSet() );
        rFont.SetStyleName( pFontItem->GetStyleName() );
    }

    // Style
    nWhich = GetWhich( SID_ATTR_CHAR_POSTURE );
    if ( rSet.GetItemState( nWhich ) >= eState )
    {
        const SvxPostureItem& rItem = (SvxPostureItem&)rSet.Get( nWhich );
        rFont.SetItalic( (FontItalic)rItem.GetValue() != ITALIC_NONE ? ITALIC_NORMAL : ITALIC_NONE );
    }
    nWhich = GetWhich( SID_ATTR_CHAR_WEIGHT );
    if ( rSet.GetItemState( nWhich ) >= eState )
    {
        SvxWeightItem& rItem = (SvxWeightItem&)rSet.Get( nWhich );
        rFont.SetWeight( (FontWeight)rItem.GetValue() != WEIGHT_NORMAL ? WEIGHT_BOLD : WEIGHT_NORMAL );
    }

    // Size
    nWhich = GetWhich( SID_ATTR_CHAR_FONTHEIGHT );
    if ( rSet.GetItemState( nWhich ) >= eState )
    {
        MapUnit eUnit = (MapUnit)rSet.GetPool()->GetMetric( nWhich );
        SvxFontHeightItem& rItem = (SvxFontHeightItem&)rSet.Get( nWhich );
        Size aSize( rFont.GetSize() );
        long nH = LogicToLogic( rItem.GetHeight(), eUnit, MAP_TWIP );
        aSize.Height() = nH;
        aSize.Width() = 0;
        rFont.SetSize( aSize );
    }
    else
        // as default 12pt
        rFont.SetSize( Size( 0, 240 ) );

    // Color
    nWhich = GetWhich( SID_ATTR_CHAR_COLOR );
    if ( rSet.GetItemState( nWhich ) >= eState )
    {
        const SvxColorItem& rItem = (SvxColorItem&)rSet.Get( nWhich );
        rFont.SetColor( rItem.GetValue() );
    }

    // Underline
    nWhich = GetWhich( SID_ATTR_CHAR_UNDERLINE );
    if ( rSet.GetItemState( nWhich ) >= SFX_ITEM_DEFAULT )
    {
        const SvxUnderlineItem& rItem = (SvxUnderlineItem&)rSet.Get( nWhich );
        FontUnderline eUnderline = (FontUnderline)rItem.GetValue();
        rFont.SetUnderline( eUnderline );
    }
    else
        rFont.SetUnderline( UNDERLINE_NONE );

    //  Strikeout
    nWhich = GetWhich( SID_ATTR_CHAR_STRIKEOUT );
    if ( rSet.GetItemState( nWhich ) >= SFX_ITEM_DEFAULT )
    {
        const SvxCrossedOutItem& rItem = (SvxCrossedOutItem&)rSet.Get( nWhich );
        FontStrikeout eStrikeout = (FontStrikeout)rItem.GetValue();
        rFont.SetStrikeout( eStrikeout );
    }
    else
        rFont.SetStrikeout( STRIKEOUT_NONE );

    // WordLineMode
    nWhich = GetWhich( SID_ATTR_CHAR_WORDLINEMODE );
    if ( rSet.GetItemState( nWhich ) >= SFX_ITEM_DEFAULT )
    {
        const SvxWordLineModeItem& rItem = (SvxWordLineModeItem&)rSet.Get( nWhich );
        rFont.SetWordLineMode( rItem.GetValue() );
    }

    // Emphasis
    nWhich = GetWhich( SID_ATTR_CHAR_EMPHASISMARK );
    if ( rSet.GetItemState( nWhich ) >= SFX_ITEM_DEFAULT )
    {
        const SvxEmphasisMarkItem& rItem = (SvxEmphasisMarkItem&)rSet.Get( nWhich );
        FontEmphasisMark eMark = rItem.GetEmphasisMark();
        rFont.SetEmphasisMark( eMark );
    }

    // Effects
    nWhich = GetWhich( SID_ATTR_CHAR_CASEMAP );

    if ( rSet.GetItemState( nWhich ) >= SFX_ITEM_DEFAULT )
    {
        const SvxCaseMapItem& rItem = (const SvxCaseMapItem&)rSet.Get( nWhich );
        SvxCaseMap eCaseMap = (SvxCaseMap)rItem.GetValue();
        rFont.SetCaseMap( eCaseMap );
    }

    // Outline
    nWhich = GetWhich( SID_ATTR_CHAR_CONTOUR );
    if ( rSet.GetItemState( nWhich ) >= SFX_ITEM_DEFAULT )
    {
        const SvxContourItem& rItem = (SvxContourItem&)rSet.Get( nWhich );
        rFont.SetOutline( rItem.GetValue() );
    }

    // Shadow
    nWhich = GetWhich( SID_ATTR_CHAR_SHADOWED );
    if ( rSet.GetItemState( nWhich ) >= SFX_ITEM_DEFAULT )
    {
        const SvxShadowedItem& rItem = (SvxShadowedItem&)rSet.Get( nWhich );
        rFont.SetShadow( rItem.GetValue() );
    }

    m_aPreviewWin.Invalidate();
}

// -----------------------------------------------------------------------

int SvxCharTwoLinesPage::DeactivatePage( SfxItemSet* pSet )
{
    if ( pSet )
        FillItemSet( *pSet );
    return LEAVE_PAGE;
}

// -----------------------------------------------------------------------

SfxTabPage* SvxCharTwoLinesPage::Create( Window* pParent, const SfxItemSet& rSet )
{
    return new SvxCharTwoLinesPage( pParent, rSet );
}

// -----------------------------------------------------------------------

USHORT* SvxCharTwoLinesPage::GetRanges()
{
    return NULL;
}

// -----------------------------------------------------------------------

void SvxCharTwoLinesPage::Reset( const SfxItemSet& rSet )
{
    m_aTwoLinesBtn.Check( FALSE );
    USHORT nWhich = GetWhich( SID_ATTR_CHAR_TWO_LINES );
    SfxItemState eState = rSet.GetItemState( nWhich );

    if ( eState >= SFX_ITEM_DONTCARE )
    {
        const SvxTwoLinesItem& rItem = (SvxTwoLinesItem&)rSet.Get( nWhich );
        m_aTwoLinesBtn.Check( rItem.GetValue() );

        if ( rItem.GetValue() )
        {
            SetBracket( rItem.GetStartBracket(), TRUE );
            SetBracket( rItem.GetEndBracket(), FALSE );
        }
    }
    TwoLinesHdl_Impl( NULL );
}

// -----------------------------------------------------------------------

BOOL SvxCharTwoLinesPage::FillItemSet( SfxItemSet& rSet )
{
    const SfxItemSet& rOldSet = GetItemSet();
    BOOL bModified = FALSE, bChanged = TRUE;
    USHORT nWhich = GetWhich( SID_ATTR_CHAR_TWO_LINES );
    const SfxPoolItem* pOld = GetOldItem( rSet, SID_ATTR_CHAR_TWO_LINES );
    sal_Bool bOn = m_aTwoLinesBtn.IsChecked();
    sal_Unicode cStart = ( bOn && m_aStartBracketLB.GetSelectEntryPos() > 0 )
        ? m_aStartBracketLB.GetSelectEntry().GetChar(0) : 0;
    sal_Unicode cEnd = ( bOn && m_aEndBracketLB.GetSelectEntryPos() > 0 )
        ? m_aEndBracketLB.GetSelectEntry().GetChar(0) : 0;

    if ( pOld )
    {
        const SvxTwoLinesItem& rItem = *( (const SvxTwoLinesItem*)pOld );
        if ( rItem.GetValue() ==  bOn &&
             ( !bOn || ( rItem.GetStartBracket() == cStart && rItem.GetEndBracket() == cEnd ) ) )
            bChanged = FALSE;
    }

    if ( bChanged )
    {
        rSet.Put( SvxTwoLinesItem( bOn, cStart, cEnd, nWhich ) );
        bModified |= TRUE;
    }
    else if ( SFX_ITEM_DEFAULT == rOldSet.GetItemState( nWhich, FALSE ) )
        rSet.ClearItem( nWhich );

    return bModified;
}
/* -----------------------------04.12.00 09:48--------------------------------

 ---------------------------------------------------------------------------*/
void    SvxCharTwoLinesPage::UpdatePreview_Impl()
{
    sal_Unicode cStart = m_aStartBracketLB.GetSelectEntryPos() > 0
        ? m_aStartBracketLB.GetSelectEntry().GetChar(0) : 0;
    sal_Unicode cEnd = m_aEndBracketLB.GetSelectEntryPos() > 0
        ? m_aEndBracketLB.GetSelectEntry().GetChar(0) : 0;
    m_aPreviewWin.SetBrackets(cStart, cEnd);
    m_aPreviewWin.SetTwoLines(m_aTwoLinesBtn.IsChecked());
    m_aPreviewWin.Invalidate();
}

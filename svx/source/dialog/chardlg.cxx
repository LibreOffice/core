/*************************************************************************
 *
 *  $RCSfile: chardlg.cxx,v $
 *
 *  $Revision: 1.78 $
 *
 *  last change: $Author: kz $ $Date: 2003-09-11 09:44:14 $
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
#ifndef _SFX_BINDINGS_HXX
#include <sfx2/bindings.hxx>
#endif
#ifndef _SFXVIEWFRM_HXX
#include <sfx2/viewfrm.hxx>
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif

#pragma hdrstop

#define _SVX_CHARDLG_CXX

#ifndef _SVTOOLS_LANGUAGEOPTIONS_HXX
#include <svtools/languageoptions.hxx>
#endif

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

#define ISITEMSET   rSet.GetItemState(nWhich)>=SFX_ITEM_DEFAULT

#define CLEARTITEM  rSet.InvalidateItem(nWhich)

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

static USHORT pNameRanges[] =
{
    SID_ATTR_CHAR_FONT,
    SID_ATTR_CHAR_WEIGHT,
    SID_ATTR_CHAR_FONTHEIGHT,
    SID_ATTR_CHAR_FONTHEIGHT,
    SID_ATTR_CHAR_COLOR,
    SID_ATTR_CHAR_COLOR,
    SID_ATTR_CHAR_LANGUAGE,
    SID_ATTR_CHAR_LANGUAGE,
    SID_ATTR_CHAR_CJK_FONT,
    SID_ATTR_CHAR_CJK_WEIGHT,
    SID_ATTR_CHAR_CTL_FONT,
    SID_ATTR_CHAR_CTL_WEIGHT,
    0
};

static USHORT pEffectsRanges[] =
{
    SID_ATTR_CHAR_SHADOWED,
    SID_ATTR_CHAR_UNDERLINE,
    SID_ATTR_CHAR_CASEMAP,
    SID_ATTR_CHAR_CASEMAP,
    SID_ATTR_FLASH,
    SID_ATTR_FLASH,
    SID_ATTR_CHAR_EMPHASISMARK,
    SID_ATTR_CHAR_EMPHASISMARK,
    SID_ATTR_CHAR_RELIEF,
    SID_ATTR_CHAR_RELIEF,
    0
};

static USHORT pPositionRanges[] =
{
    SID_ATTR_CHAR_KERNING,
    SID_ATTR_CHAR_KERNING,
    SID_ATTR_CHAR_ESCAPEMENT,
    SID_ATTR_CHAR_ESCAPEMENT,
    SID_ATTR_CHAR_AUTOKERN,
    SID_ATTR_CHAR_AUTOKERN,
    SID_ATTR_CHAR_ROTATED,
    SID_ATTR_CHAR_SCALEWIDTH,
    SID_ATTR_CHAR_WIDTH_FIT_TO_LINE,
    SID_ATTR_CHAR_WIDTH_FIT_TO_LINE,
    0
};

static USHORT pTwoLinesRanges[] =
{
    SID_ATTR_CHAR_TWO_LINES,
    SID_ATTR_CHAR_TWO_LINES,
    0
};

// C-Funktion ------------------------------------------------------------

inline BOOL StateToAttr( TriState aState )
{
    return ( STATE_CHECK == aState );
}

// class SvxCharBasePage -------------------------------------------------

inline SvxFont& SvxCharBasePage::GetPreviewFont()
{
    return m_aPreviewWin.GetFont();
}

// -----------------------------------------------------------------------

inline SvxFont& SvxCharBasePage::GetPreviewCJKFont()
{
    return m_aPreviewWin.GetCJKFont();
}
// -----------------------------------------------------------------------

inline SvxFont& SvxCharBasePage::GetPreviewCTLFont()
{
    return m_aPreviewWin.GetCTLFont();
}

// -----------------------------------------------------------------------

SvxCharBasePage::SvxCharBasePage( Window* pParent, const ResId& rResId, const SfxItemSet& rItemset,
                                 USHORT nResIdPrewievWin, USHORT nResIdFontTypeFT ):
    SfxTabPage( pParent, rResId, rItemset ),
    m_aPreviewWin( this, ResId( nResIdPrewievWin ) ),
    m_aFontTypeFT( this, ResId( nResIdFontTypeFT ) ),
    m_bPreviewBackgroundToCharacter( FALSE )
{
}

// -----------------------------------------------------------------------

SvxCharBasePage::~SvxCharBasePage()
{
}

// -----------------------------------------------------------------------

//void SvxCharBasePage::SetPrevFontAttributes( const SfxItemSet& rSet )
void SvxCharBasePage::ActivatePage( const SfxItemSet& rSet )
{
    SvxFont&        rFont = GetPreviewFont();
    SvxFont&        rCJKFont = GetPreviewCJKFont();
    SvxFont&        rCTLFont = GetPreviewCTLFont();
    USHORT          nWhich;

    // Underline
    FontUnderline eUnderline;
    nWhich = GetWhich( SID_ATTR_CHAR_UNDERLINE );
    if( ISITEMSET )
    {
        const SvxUnderlineItem& rItem = ( SvxUnderlineItem& ) rSet.Get( nWhich );
        eUnderline = ( FontUnderline ) rItem.GetValue();
        m_aPreviewWin.SetTextLineColor( rItem.GetColor() );
    }
    else
        eUnderline = UNDERLINE_NONE;

    rFont.SetUnderline( eUnderline );
    rCJKFont.SetUnderline( eUnderline );
    rCTLFont.SetUnderline( eUnderline );

    //  Strikeout
    FontStrikeout eStrikeout;
    nWhich = GetWhich( SID_ATTR_CHAR_STRIKEOUT );
    if( ISITEMSET )
    {
        const SvxCrossedOutItem& rItem = ( SvxCrossedOutItem& ) rSet.Get( nWhich );
        eStrikeout = ( FontStrikeout ) rItem.GetValue();
    }
    else
        eStrikeout = STRIKEOUT_NONE;

    rFont.SetStrikeout( eStrikeout );
    rCJKFont.SetStrikeout( eStrikeout );
    rCTLFont.SetStrikeout( eStrikeout );

    // WordLineMode
    nWhich = GetWhich( SID_ATTR_CHAR_WORDLINEMODE );
    if( ISITEMSET )
    {
        const SvxWordLineModeItem& rItem = ( SvxWordLineModeItem& ) rSet.Get( nWhich );
        rFont.SetWordLineMode( rItem.GetValue() );
        rCJKFont.SetWordLineMode( rItem.GetValue() );
        rCTLFont.SetWordLineMode( rItem.GetValue() );
    }

    // Emphasis
    nWhich = GetWhich( SID_ATTR_CHAR_EMPHASISMARK );
    if( ISITEMSET )
    {
        const SvxEmphasisMarkItem& rItem = ( SvxEmphasisMarkItem& ) rSet.Get( nWhich );
        FontEmphasisMark eMark = rItem.GetEmphasisMark();
        rFont.SetEmphasisMark( eMark );
        rCJKFont.SetEmphasisMark( eMark );
        rCTLFont.SetEmphasisMark( eMark );
    }

    // Relief
    nWhich = GetWhich( SID_ATTR_CHAR_RELIEF );
    if( ISITEMSET )
    {
        const SvxCharReliefItem& rItem = ( SvxCharReliefItem& ) rSet.Get( nWhich );
        FontRelief eFontRelief = ( FontRelief ) rItem.GetValue();
        rFont.SetRelief( eFontRelief );
        rCJKFont.SetRelief( eFontRelief );
        rCTLFont.SetRelief( eFontRelief );
    }

    // Effects
    nWhich = GetWhich( SID_ATTR_CHAR_CASEMAP );
    if( ISITEMSET )
    {
        const SvxCaseMapItem& rItem = ( SvxCaseMapItem& ) rSet.Get( nWhich );
        SvxCaseMap eCaseMap = ( SvxCaseMap ) rItem.GetValue();
        rFont.SetCaseMap( eCaseMap );
        rCJKFont.SetCaseMap( eCaseMap );
        rCTLFont.SetCaseMap( eCaseMap );
    }

    // Outline
    nWhich = GetWhich( SID_ATTR_CHAR_CONTOUR );
    if( ISITEMSET )
    {
        const SvxContourItem& rItem = ( SvxContourItem& ) rSet.Get( nWhich );
        BOOL bOutline = rItem.GetValue();
        rFont.SetOutline( bOutline );
        rCJKFont.SetOutline( bOutline );
        rCTLFont.SetOutline( bOutline );
    }

    // Shadow
    nWhich = GetWhich( SID_ATTR_CHAR_SHADOWED );
    if( ISITEMSET )
    {
        const SvxShadowedItem& rItem = ( SvxShadowedItem& ) rSet.Get( nWhich );
        BOOL bShadow = rItem.GetValue();
        rFont.SetShadow( bShadow );
        rCJKFont.SetShadow( bShadow );
        rCTLFont.SetShadow( bShadow );
    }

    // Background
    BOOL bTransparent;
    nWhich = GetWhich( m_bPreviewBackgroundToCharacter ? SID_ATTR_BRUSH : SID_ATTR_BRUSH_CHAR );
    if( ISITEMSET )
    {
         const SvxBrushItem& rBrush = ( SvxBrushItem& ) rSet.Get( nWhich );
         const Color& rColor = rBrush.GetColor();
         bTransparent = rColor.GetTransparency() > 0;
         rFont.SetFillColor( rColor );
         rCJKFont.SetFillColor( rColor );
         rCTLFont.SetFillColor( rColor );
    }
    else
        bTransparent = TRUE;

    rFont.SetTransparent( bTransparent );
    rCJKFont.SetTransparent( bTransparent );
    rCTLFont.SetTransparent( bTransparent );

    Color aBackCol( COL_TRANSPARENT );
    if( !m_bPreviewBackgroundToCharacter )
    {
        nWhich = GetWhich( SID_ATTR_BRUSH );
        if( ISITEMSET )
        {
            const SvxBrushItem& rBrush = ( SvxBrushItem& ) rSet.Get( nWhich );
            if( GPOS_NONE == rBrush.GetGraphicPos() )
                aBackCol = rBrush.GetColor();
        }
    }
    m_aPreviewWin.SetBackColor( aBackCol );

    // Font
    SetPrevFont( rSet, SID_ATTR_CHAR_FONT, rFont );
    SetPrevFont( rSet, SID_ATTR_CHAR_CJK_FONT, rCJKFont );
    SetPrevFont( rSet, SID_ATTR_CHAR_CTL_FONT, rCTLFont );

    // Style
    SetPrevFontStyle( rSet, SID_ATTR_CHAR_POSTURE, SID_ATTR_CHAR_WEIGHT, rFont );
    SetPrevFontStyle( rSet, SID_ATTR_CHAR_CJK_POSTURE, SID_ATTR_CHAR_CJK_WEIGHT, rCJKFont );
    SetPrevFontStyle( rSet, SID_ATTR_CHAR_CTL_POSTURE, SID_ATTR_CHAR_CTL_WEIGHT, rCTLFont );

    // Size
    SetPrevFontSize( rSet, SID_ATTR_CHAR_FONTHEIGHT, rFont );
    SetPrevFontSize( rSet, SID_ATTR_CHAR_CJK_FONTHEIGHT, rCJKFont );
    SetPrevFontSize( rSet, SID_ATTR_CHAR_CTL_FONTHEIGHT, rCTLFont );

    // Color
    nWhich = GetWhich( SID_ATTR_CHAR_COLOR );
    if( ISITEMSET )
    {
        const SvxColorItem& rItem = ( SvxColorItem& ) rSet.Get( nWhich );
        Color aCol( rItem.GetValue() );
        rFont.SetColor( aCol );
        rCJKFont.SetColor( aCol );
        rCTLFont.SetColor( aCol );

        m_aPreviewWin.AutoCorrectFontColor();   // handle color COL_AUTO
    }

    // Kerning
    nWhich = GetWhich( SID_ATTR_CHAR_KERNING );
    if( ISITEMSET )
    {
        const SvxKerningItem& rItem = ( SvxKerningItem& ) rSet.Get( nWhich );
        short nKern = ( short )
                        LogicToLogic( rItem.GetValue(), ( MapUnit ) rSet.GetPool()->GetMetric( nWhich ), MAP_TWIP );
        rFont.SetFixKerning( nKern );
        rCJKFont.SetFixKerning( nKern );
        rCTLFont.SetFixKerning( nKern );
    }

    // Escapement
    nWhich = GetWhich( SID_ATTR_CHAR_ESCAPEMENT );
    const BYTE nProp = 100;
    short nEsc;
    BYTE nEscProp;
    if( ISITEMSET )
    {
        const SvxEscapementItem& rItem = ( SvxEscapementItem& ) rSet.Get( nWhich );
        nEsc = rItem.GetEsc();
        nEscProp = rItem.GetProp();

        if( nEsc == DFLT_ESC_AUTO_SUPER )
            nEsc = DFLT_ESC_SUPER;
        else if( nEsc == DFLT_ESC_AUTO_SUB )
            nEsc = DFLT_ESC_SUB;
    }
    else
    {
        nEsc  = 0;
        nEscProp = 100;
    }

    SetPrevFontEscapement( nProp, nEscProp, nEsc );

    // Font width scale
    SetPrevFontWidthScale( rSet );

    m_aPreviewWin.Invalidate();
}

// -----------------------------------------------------------------------

void SvxCharBasePage::SetPrevFontSize( const SfxItemSet& rSet, USHORT nSlot, SvxFont& rFont )
{
    USHORT nWhich = GetWhich( nSlot );
    long nH;
    if( rSet.GetItemState( nWhich ) >= SFX_ITEM_SET )
    {
        nH = LogicToLogic(  ( ( SvxFontHeightItem& ) rSet.Get( nWhich ) ).GetHeight(),
                            ( MapUnit ) rSet.GetPool()->GetMetric( nWhich ),
                            MAP_TWIP );
    }
    else
        nH = 240;   // as default 12pt

    rFont.SetSize( Size( 0, nH ) );
}

// -----------------------------------------------------------------------

void SvxCharBasePage::SetPrevFont( const SfxItemSet& rSet, USHORT nSlot, SvxFont& rFont )
{
    USHORT nWhich = GetWhich( nSlot );
    if( ISITEMSET )
    {
        const SvxFontItem& rFontItem = ( SvxFontItem& ) rSet.Get( nWhich );
        rFont.SetFamily( rFontItem.GetFamily() );
        rFont.SetName( rFontItem.GetFamilyName() );
        rFont.SetPitch( rFontItem.GetPitch() );
        rFont.SetCharSet( rFontItem.GetCharSet() );
        rFont.SetStyleName( rFontItem.GetStyleName() );
    }
}

// -----------------------------------------------------------------------

void SvxCharBasePage::SetPrevFontStyle( const SfxItemSet& rSet, USHORT nPosture, USHORT nWeight, SvxFont& rFont )
{
    USHORT nWhich = GetWhich( nPosture );
    if( ISITEMSET )
    {
        const SvxPostureItem& rItem = ( SvxPostureItem& ) rSet.Get( nWhich );
        rFont.SetItalic( ( FontItalic ) rItem.GetValue() != ITALIC_NONE ? ITALIC_NORMAL : ITALIC_NONE );
    }

    nWhich = GetWhich( nWeight );
    if( ISITEMSET )
    {
        SvxWeightItem& rItem = ( SvxWeightItem& ) rSet.Get( nWhich );
        rFont.SetWeight( ( FontWeight ) rItem.GetValue() != WEIGHT_NORMAL ? WEIGHT_BOLD : WEIGHT_NORMAL );
    }
}

// -----------------------------------------------------------------------

void SvxCharBasePage::SetPrevFontWidthScale( const SfxItemSet& rSet )
{
    USHORT  nWhich = GetWhich( SID_ATTR_CHAR_SCALEWIDTH );
    if( ISITEMSET )
    {
        const SvxCharScaleWidthItem&    rItem = ( SvxCharScaleWidthItem& ) rSet.Get( nWhich );

        m_aPreviewWin.SetFontWidthScale( rItem.GetValue() );
    }
}

// -----------------------------------------------------------------------
namespace
{
    // -----------------------------------------------------------------------
    void setPrevFontEscapement(SvxFont& _rFont,BYTE nProp, BYTE nEscProp, short nEsc )
    {
        _rFont.SetPropr( nProp );
        _rFont.SetProprRel( nEscProp );
        _rFont.SetEscapement( nEsc );
    }
    // -----------------------------------------------------------------------
    // -----------------------------------------------------------------------
}
// -----------------------------------------------------------------------

void SvxCharBasePage::SetPrevFontEscapement( BYTE nProp, BYTE nEscProp, short nEsc )
{
    setPrevFontEscapement(GetPreviewFont(),nProp,nEscProp,nEsc);
    setPrevFontEscapement(GetPreviewCJKFont(),nProp,nEscProp,nEsc);
    setPrevFontEscapement(GetPreviewCTLFont(),nProp,nEscProp,nEsc);
    m_aPreviewWin.Invalidate();
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

    SvxCharBasePage( pParent, SVX_RES( RID_SVXPAGE_CHAR_NAME ), rInSet, WIN_CHAR_PREVIEW, FT_CHAR_FONTTYPE ),
    m_pImpl                 ( new SvxCharNamePage_Impl )
{
    m_pImpl->m_aNoStyleText = String( ResId( STR_CHARNAME_NOSTYLE ) );
    m_pImpl->m_aTransparentText = String( ResId( STR_CHARNAME_TRANSPARENT ) );

    SvtLanguageOptions aLanguageOptions;
    sal_Bool bCJK = ( aLanguageOptions.IsCJKFontEnabled() || aLanguageOptions.IsCTLFontEnabled() );

    m_pWestLine         = new FixedLine( this, ResId( FL_WEST ) );
    m_pWestFontNameFT   = new FixedText( this, ResId( bCJK ? FT_WEST_NAME : FT_WEST_NAME_NOCJK ) );
    m_pWestFontNameLB   = new FontNameBox( this, ResId( bCJK ? LB_WEST_NAME : LB_WEST_NAME_NOCJK ) );
    m_pWestFontStyleFT  = new FixedText( this, ResId( bCJK ? FT_WEST_STYLE : FT_WEST_STYLE_NOCJK ) );
    m_pWestFontStyleLB  = new FontStyleBox( this, ResId( bCJK ? LB_WEST_STYLE : LB_WEST_STYLE_NOCJK ) );
    m_pWestFontSizeFT   = new FixedText( this, ResId( bCJK ? FT_WEST_SIZE : FT_WEST_SIZE_NOCJK ) );
    m_pWestFontSizeLB   = new FontSizeBox( this, ResId( bCJK ? LB_WEST_SIZE : LB_WEST_SIZE_NOCJK ) );

    if( !bCJK )
    {
        m_pColorFL  = new FixedLine( this, ResId( FL_COLOR2 ) );
        m_pColorFT  = new FixedText( this, ResId( FT_COLOR2 ) );
        m_pColorLB  = new ColorListBox( this, ResId( LB_COLOR2 ) );
    }

    m_pWestFontLanguageFT   = new FixedText( this, ResId( bCJK ? FT_WEST_LANG : FT_WEST_LANG_NOCJK ) );
    m_pWestFontLanguageLB   = new SvxLanguageBox( this, ResId( bCJK ? LB_WEST_LANG : LB_WEST_LANG_NOCJK ) );

    m_pEastLine             = new FixedLine( this, ResId( FL_EAST ) );
    m_pEastFontNameFT       = new FixedText( this, ResId( FT_EAST_NAME ) );
    m_pEastFontNameLB       = new FontNameBox( this, ResId( LB_EAST_NAME ) );
    m_pEastFontStyleFT      = new FixedText( this, ResId( FT_EAST_STYLE ) );
    m_pEastFontStyleLB      = new FontStyleBox( this, ResId( LB_EAST_STYLE ) );
    m_pEastFontSizeFT       = new FixedText( this, ResId( FT_EAST_SIZE ) );
    m_pEastFontSizeLB       = new FontSizeBox( this, ResId( LB_EAST_SIZE ) );
    m_pEastFontLanguageFT   = new FixedText( this, ResId( FT_EAST_LANG ) );
    m_pEastFontLanguageLB   = new SvxLanguageBox( this, ResId( LB_EAST_LANG ) );

    m_pCTLLine              = new FixedLine( this, ResId( FL_CTL ) );
    m_pCTLFontNameFT        = new FixedText( this, ResId( FT_CTL_NAME ) );
    m_pCTLFontNameLB        = new FontNameBox( this, ResId( LB_CTL_NAME ) );
    m_pCTLFontStyleFT       = new FixedText( this, ResId( FT_CTL_STYLE ) );
    m_pCTLFontStyleLB       = new FontStyleBox( this, ResId( LB_CTL_STYLE ) );
    m_pCTLFontSizeFT        = new FixedText( this, ResId( FT_CTL_SIZE ) );
    m_pCTLFontSizeLB        = new FontSizeBox( this, ResId( LB_CTL_SIZE ) );
    m_pCTLFontLanguageFT    = new FixedText( this, ResId( FT_CTL_LANG ) );
    m_pCTLFontLanguageLB    = new SvxLanguageBox( this, ResId( LB_CTL_LANG ) );

    if( bCJK )
    {
        m_pColorFL  = new FixedLine( this, ResId( FL_COLOR2 ) );
        m_pColorFT  = new FixedText( this, ResId( FT_COLOR2 ) );
        m_pColorLB  = new ColorListBox( this, ResId( LB_COLOR2 ) );
    }

    m_pWestLine             ->Show( bCJK );
    m_pColorFL              ->Show( bCJK );

    bCJK = aLanguageOptions.IsCJKFontEnabled();
    m_pEastLine             ->Show( bCJK );
    m_pEastFontNameFT       ->Show( bCJK );
    m_pEastFontNameLB       ->Show( bCJK );
    m_pEastFontStyleFT      ->Show( bCJK );
    m_pEastFontStyleLB      ->Show( bCJK );
    m_pEastFontSizeFT       ->Show( bCJK );
    m_pEastFontSizeLB       ->Show( bCJK );
    m_pEastFontLanguageFT   ->Show( bCJK );
    m_pEastFontLanguageLB   ->Show( bCJK );

    sal_Bool bShowCTL = aLanguageOptions.IsCTLFontEnabled();
    if ( bShowCTL && !bCJK )
    {
        // move CTL controls to the places of the CJK controls, if these controls aren't visible
        m_pCTLLine             ->SetPosPixel( m_pEastLine->GetPosPixel() );
        m_pCTLFontNameFT       ->SetPosPixel( m_pEastFontNameFT->GetPosPixel() );
        m_pCTLFontNameLB       ->SetPosPixel( m_pEastFontNameLB->GetPosPixel() );
        m_pCTLFontStyleFT      ->SetPosPixel( m_pEastFontStyleFT->GetPosPixel() );
        m_pCTLFontStyleLB      ->SetPosPixel( m_pEastFontStyleLB->GetPosPixel() );
        m_pCTLFontSizeFT       ->SetPosPixel( m_pEastFontSizeFT->GetPosPixel() );
        m_pCTLFontSizeLB       ->SetPosPixel( m_pEastFontSizeLB->GetPosPixel() );
        m_pCTLFontLanguageFT   ->SetPosPixel( m_pEastFontLanguageFT->GetPosPixel() );
        m_pCTLFontLanguageLB   ->SetPosPixel( m_pEastFontLanguageLB->GetPosPixel() );
    }
    m_pCTLLine             ->Show( bShowCTL );
    m_pCTLFontNameFT       ->Show( bShowCTL );
    m_pCTLFontNameLB       ->Show( bShowCTL );
    m_pCTLFontStyleFT      ->Show( bShowCTL );
    m_pCTLFontStyleLB      ->Show( bShowCTL );
    m_pCTLFontSizeFT       ->Show( bShowCTL );
    m_pCTLFontSizeLB       ->Show( bShowCTL );
    m_pCTLFontLanguageFT   ->Show( bShowCTL );
    m_pCTLFontLanguageLB   ->Show( bShowCTL );

    FreeResource();

    m_pWestFontLanguageLB->SetLanguageList( LANG_LIST_WESTERN,  TRUE, FALSE, TRUE );
    m_pEastFontLanguageLB->SetLanguageList( LANG_LIST_CJK,      TRUE, FALSE, TRUE );
    m_pCTLFontLanguageLB->SetLanguageList( LANG_LIST_CTL,       TRUE, FALSE, TRUE );

    Initialize();
}

// -----------------------------------------------------------------------

SvxCharNamePage::~SvxCharNamePage()
{
    if ( m_pImpl->m_bMustDelete )
        delete m_pImpl->m_pFontList;
    delete m_pImpl;

    delete m_pWestLine;
    delete m_pWestFontNameFT;
    delete m_pWestFontNameLB;
    delete m_pWestFontStyleFT;
    delete m_pWestFontStyleLB;
    delete m_pWestFontSizeFT;
    delete m_pWestFontSizeLB;
    delete m_pWestFontLanguageFT;
    delete m_pWestFontLanguageLB;

    delete m_pEastLine;
    delete m_pEastFontNameFT;
    delete m_pEastFontNameLB;
    delete m_pEastFontStyleFT;
    delete m_pEastFontStyleLB;
    delete m_pEastFontSizeFT;
    delete m_pEastFontSizeLB;
    delete m_pEastFontLanguageFT;
    delete m_pEastFontLanguageLB;

    delete m_pCTLLine;
    delete m_pCTLFontNameFT;
    delete m_pCTLFontNameLB;
    delete m_pCTLFontStyleFT;
    delete m_pCTLFontStyleLB;
    delete m_pCTLFontSizeFT;
    delete m_pCTLFontSizeLB;
    delete m_pCTLFontLanguageFT;
    delete m_pCTLFontLanguageLB;

    delete m_pColorFL;
    delete m_pColorFT;
    delete m_pColorLB;
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

    m_pColorLB->SetUpdateMode( FALSE );

    {
        SfxPoolItem* pDummy;
        SfxViewFrame* pFrame = SfxViewFrame::GetFirst( pDocSh );
        if( !pFrame || SFX_ITEM_DEFAULT > pFrame->GetBindings().QueryState(
                                    SID_ATTR_AUTO_COLOR_INVALID, pDummy ))
            m_pColorLB->InsertEntry( Color( COL_AUTO ),
                                     SVX_RESSTR( RID_SVXSTR_AUTOMATIC ));
    }
    for ( long i = 0; i < pColorTable->Count(); i++ )
    {
        XColorEntry* pEntry = pColorTable->Get(i);
        m_pColorLB->InsertEntry( pEntry->GetColor(), pEntry->GetName() );
    }

    m_pColorLB->SetUpdateMode( TRUE );

    if ( bKillTable )
        delete pColorTable;

    m_pColorLB->SetSelectHdl( LINK( this, SvxCharNamePage, ColorBoxSelectHdl_Impl ) );

    Link aLink = LINK( this, SvxCharNamePage, FontModifyHdl_Impl );
    m_pWestFontNameLB->SetModifyHdl( aLink );
    m_pWestFontStyleLB->SetModifyHdl( aLink );
    m_pWestFontSizeLB->SetModifyHdl( aLink );
    m_pEastFontNameLB->SetModifyHdl( aLink );
    m_pEastFontStyleLB->SetModifyHdl( aLink );
    m_pEastFontSizeLB->SetModifyHdl( aLink );
    m_pCTLFontNameLB->SetModifyHdl( aLink );
    m_pCTLFontStyleLB->SetModifyHdl( aLink );
    m_pCTLFontSizeLB->SetModifyHdl( aLink );

    m_pImpl->m_aUpdateTimer.SetTimeoutHdl( LINK( this, SvxCharNamePage, UpdateHdl_Impl ) );

    m_pColorFL->Hide();
    m_pColorFT->Hide();
    m_pColorLB->Hide();
}

// -----------------------------------------------------------------------

const FontList* SvxCharNamePage::GetFontList() const
{
    if ( !m_pImpl->m_pFontList )
    {
        SfxObjectShell* pDocSh = SfxObjectShell::Current();
        const SfxPoolItem* pItem;

        /* #110771# SvxFontListItem::GetFontList can return NULL */
        if ( pDocSh && ( pItem = pDocSh->GetItem( SID_ATTR_CHAR_FONTLIST ) ) )
        {
            m_pImpl->m_pFontList = ( (SvxFontListItem*)pItem )->GetFontList();
            DBG_ASSERT(NULL != m_pImpl->m_pFontList,
                       "Where is the font list?")
        }
        if(!m_pImpl->m_pFontList)
        {
            m_pImpl->m_pFontList =
                new FontList( Application::GetDefaultDevice() );
            m_pImpl->m_bMustDelete = TRUE;
        }
    }

    return m_pImpl->m_pFontList;
}

// -----------------------------------------------------------------------------
namespace
{
    FontInfo calcFontInfo(  SvxFont& _rFont,
                    SvxCharNamePage* _pPage,
                    const FontNameBox* _pFontNameLB,
                    const FontStyleBox* _pFontStyleLB,
                    const FontSizeBox* _pFontSizeLB,
                    const FontList* _pFontList,
                    USHORT _nFontWhich,
                    USHORT _nFontHeightWhich)
    {
        Size aSize = _rFont.GetSize();
        aSize.Width() = 0;
        FontInfo aFontInfo;
        String sFontName(_pFontNameLB->GetText());
        BOOL bFontAvailable = _pFontList->IsAvailable( sFontName );
        if(bFontAvailable  || _pFontNameLB->GetSavedValue() != sFontName)
            aFontInfo = _pFontList->Get( sFontName, _pFontStyleLB->GetText() );
        else
        {
            //get the font from itemset
            SfxItemState eState = _pPage->GetItemSet().GetItemState( _nFontWhich );
            if ( eState >= SFX_ITEM_DEFAULT )
            {
                const SvxFontItem* pFontItem = (const SvxFontItem*)&( _pPage->GetItemSet().Get( _nFontWhich ) );
                aFontInfo.SetName(pFontItem->GetFamilyName());
                aFontInfo.SetStyleName(pFontItem->GetStyleName());
                aFontInfo.SetFamily(pFontItem->GetFamily());
                aFontInfo.SetPitch(pFontItem->GetPitch());
                aFontInfo.SetCharSet(pFontItem->GetCharSet());
            }
        }
        if ( _pFontSizeLB->IsRelative() )
        {
            DBG_ASSERT( _pPage->GetItemSet().GetParent(), "No parent set" );
            const SvxFontHeightItem& rOldItem = (SvxFontHeightItem&)_pPage->GetItemSet().GetParent()->Get( _nFontHeightWhich );

            // alter Wert, skaliert
            long nHeight;
            if ( _pFontSizeLB->IsPtRelative() )
                nHeight = rOldItem.GetHeight() + PointToTwips( _pFontSizeLB->GetValue() / 10 );
            else
                nHeight = rOldItem.GetHeight() * _pFontSizeLB->GetValue() / 100;

            // Umrechnung in twips fuer das Beispiel-Window
            aSize.Height() =
                ItemToControl( nHeight, _pPage->GetItemSet().GetPool()->GetMetric( _nFontHeightWhich ), SFX_FUNIT_TWIP );
        }
        else if ( _pFontSizeLB->GetText().Len() )
            aSize.Height() = PointToTwips( _pFontSizeLB->GetValue() / 10 );
        else
            aSize.Height() = 200;   // default 10pt
        aFontInfo.SetSize( aSize );

        _rFont.SetFamily( aFontInfo.GetFamily() );
        _rFont.SetName( aFontInfo.GetName() );
        _rFont.SetStyleName( aFontInfo.GetStyleName() );
        _rFont.SetPitch( aFontInfo.GetPitch() );
        _rFont.SetCharSet( aFontInfo.GetCharSet() );
        _rFont.SetWeight( aFontInfo.GetWeight() );
        _rFont.SetItalic( aFontInfo.GetItalic() );
        _rFont.SetSize( aFontInfo.GetSize() );

        return aFontInfo;
    }
}

// -----------------------------------------------------------------------

void SvxCharNamePage::UpdatePreview_Impl()
{
    SvxFont& rFont = GetPreviewFont();
    SvxFont& rCJKFont = GetPreviewCJKFont();
    SvxFont& rCTLFont = GetPreviewCTLFont();
    // Size
    Size aSize = rFont.GetSize();
    aSize.Width() = 0;
    Size aCJKSize = rCJKFont.GetSize();
    aCJKSize.Width() = 0;
    Size aCTLSize = rCTLFont.GetSize();
    aCTLSize.Width() = 0;
    // Font
    const FontList* pFontList = GetFontList();
    FontInfo aFontInfo =
        calcFontInfo(rFont,this,m_pWestFontNameLB,m_pWestFontStyleLB,m_pWestFontSizeLB,pFontList,GetWhich( SID_ATTR_CHAR_FONT ),GetWhich( SID_ATTR_CHAR_FONTHEIGHT ));

    calcFontInfo(rCJKFont,this,m_pEastFontNameLB,m_pEastFontStyleLB,m_pEastFontSizeLB,pFontList,GetWhich( SID_ATTR_CHAR_CJK_FONT ),GetWhich( SID_ATTR_CHAR_CJK_FONTHEIGHT ));

    calcFontInfo(rCTLFont,this,m_pCTLFontNameLB,m_pCTLFontStyleLB,m_pCTLFontSizeLB,pFontList,GetWhich( SID_ATTR_CHAR_CTL_FONT ),GetWhich( SID_ATTR_CHAR_CTL_FONTHEIGHT ));

    m_aPreviewWin.Invalidate();
    m_aFontTypeFT.SetText( pFontList->GetFontMapText( aFontInfo ) );
}

// -----------------------------------------------------------------------

void SvxCharNamePage::FillStyleBox_Impl( const FontNameBox* pNameBox )
{
    const FontList* pFontList = GetFontList();
    DBG_ASSERT( pFontList, "no fontlist" );

    FontStyleBox* pStyleBox = NULL;

    if ( m_pWestFontNameLB == pNameBox )
        pStyleBox = m_pWestFontStyleLB;
    else if ( m_pEastFontNameLB == pNameBox )
        pStyleBox = m_pEastFontStyleLB;
    else if ( m_pCTLFontNameLB == pNameBox )
        pStyleBox = m_pCTLFontStyleLB;
    else
    {
        DBG_ERRORFILE( "invalid font name box" );
    }

    pStyleBox->Fill( pNameBox->GetText(), pFontList );

    if ( m_pImpl->m_bInSearchMode )
    {
        // Bei der Suche zus"atzliche Eintr"age:
        // "Nicht Fett" und "Nicht Kursiv"
        String aEntry = m_pImpl->m_aNoStyleText;
        const sal_Char sS[] = "%s";
        aEntry.SearchAndReplaceAscii( sS, pFontList->GetBoldStr() );
        m_pImpl->m_nExtraEntryPos = pStyleBox->InsertEntry( aEntry );
        aEntry = m_pImpl->m_aNoStyleText;
        aEntry.SearchAndReplaceAscii( sS, pFontList->GetItalicStr() );
        pStyleBox->InsertEntry( aEntry );
    }
}

// -----------------------------------------------------------------------

void SvxCharNamePage::FillSizeBox_Impl( const FontNameBox* pNameBox )
{
    const FontList* pFontList = GetFontList();
    DBG_ASSERT( pFontList, "no fontlist" );

    FontStyleBox* pStyleBox = NULL;
    FontSizeBox* pSizeBox = NULL;

    if ( m_pWestFontNameLB == pNameBox )
    {
        pStyleBox = m_pWestFontStyleLB;
        pSizeBox = m_pWestFontSizeLB;
    }
    else if ( m_pEastFontNameLB == pNameBox )
    {
        pStyleBox = m_pEastFontStyleLB;
        pSizeBox = m_pEastFontSizeLB;
    }
    else if ( m_pCTLFontNameLB == pNameBox )
    {
        pStyleBox = m_pCTLFontStyleLB;
        pSizeBox = m_pCTLFontSizeLB;
    }
    else
    {
        DBG_ERRORFILE( "invalid font name box" );
    }

    pSizeBox->Fill( pFontList->Get( pNameBox->GetText(), pStyleBox->GetText() ), pFontList );
}

// -----------------------------------------------------------------------

void SvxCharNamePage::Reset_Impl( const SfxItemSet& rSet, LanguageGroup eLangGrp )
{
    FixedText* pNameLabel = NULL;
    FontNameBox* pNameBox = NULL;
    FixedText* pStyleLabel = NULL;
    FontStyleBox* pStyleBox = NULL;
    FixedText* pSizeLabel = NULL;
    FontSizeBox* pSizeBox = NULL;
    FixedText* pLangFT = NULL;
    SvxLanguageBox* pLangBox = NULL;
    USHORT nWhich = 0;

    switch ( eLangGrp )
    {
        case Western :
            pNameLabel = m_pWestFontNameFT;
            pNameBox = m_pWestFontNameLB;
            pStyleLabel = m_pWestFontStyleFT;
            pStyleBox = m_pWestFontStyleLB;
            pSizeLabel = m_pWestFontSizeFT;
            pSizeBox = m_pWestFontSizeLB;
            pLangFT = m_pWestFontLanguageFT;
            pLangBox = m_pWestFontLanguageLB;
            nWhich = GetWhich( SID_ATTR_CHAR_FONT );
            break;

        case Asian :
            pNameLabel = m_pEastFontNameFT;
            pNameBox = m_pEastFontNameLB;
            pStyleLabel = m_pEastFontStyleFT;
            pStyleBox = m_pEastFontStyleLB;
            pSizeLabel = m_pEastFontSizeFT;
            pSizeBox = m_pEastFontSizeLB;
            pLangFT = m_pEastFontLanguageFT;
            pLangBox = m_pEastFontLanguageLB;
            nWhich = GetWhich( SID_ATTR_CHAR_CJK_FONT );
            break;

        case Ctl :
            pNameLabel = m_pCTLFontNameFT;
            pNameBox = m_pCTLFontNameLB;
            pStyleLabel = m_pCTLFontStyleFT;
            pStyleBox = m_pCTLFontStyleLB;
            pSizeLabel = m_pCTLFontSizeFT;
            pSizeBox = m_pCTLFontSizeLB;
            pLangFT = m_pCTLFontLanguageFT;
            pLangBox = m_pCTLFontLanguageLB;
            nWhich = GetWhich( SID_ATTR_CHAR_CTL_FONT );
            break;
    }

    // die FontListBox fuellen
    const FontList* pFontList = GetFontList();
    pNameBox->Fill( pFontList );

    // Font ermitteln
    const SvxFontItem* pFontItem = NULL;
    SfxItemState eState = rSet.GetItemState( nWhich );

    if ( eState >= SFX_ITEM_DEFAULT )
    {
        pFontItem = (const SvxFontItem*)&( rSet.Get( nWhich ) );
        pNameBox->SetText( pFontItem->GetFamilyName() );
    }
    else
    {
        pNameBox->SetText( String() );
    }

    FillStyleBox_Impl( pNameBox );

    FASTBOOL bStyle = FALSE;
    FASTBOOL bStyleAvailable = TRUE;
    FontItalic eItalic = ITALIC_NONE;
    FontWeight eWeight = WEIGHT_NORMAL;
    switch ( eLangGrp )
    {
        case Western : nWhich = GetWhich( SID_ATTR_CHAR_POSTURE ); break;
        case Asian : nWhich = GetWhich( SID_ATTR_CHAR_CJK_POSTURE ); break;
        case Ctl : nWhich = GetWhich( SID_ATTR_CHAR_CTL_POSTURE ); break;
    }
    eState = rSet.GetItemState( nWhich );

    if ( eState >= SFX_ITEM_DEFAULT )
    {
        const SvxPostureItem& rItem = (SvxPostureItem&)rSet.Get( nWhich );
        eItalic = (FontItalic)rItem.GetValue();
        bStyle = TRUE;
    }
    bStyleAvailable = bStyleAvailable && (eState >= SFX_ITEM_DONTCARE);

    switch ( eLangGrp )
    {
        case Western : nWhich = GetWhich( SID_ATTR_CHAR_WEIGHT ); break;
        case Asian : nWhich = GetWhich( SID_ATTR_CHAR_CJK_WEIGHT ); break;
        case Ctl : nWhich = GetWhich( SID_ATTR_CHAR_CTL_WEIGHT ); break;
    }
    eState = rSet.GetItemState( nWhich );

    if ( eState >= SFX_ITEM_DEFAULT )
    {
        SvxWeightItem& rItem = (SvxWeightItem&)rSet.Get( nWhich );
        eWeight = (FontWeight)rItem.GetValue();
    }
    else
        bStyle = FALSE;
    bStyleAvailable = bStyleAvailable && (eState >= SFX_ITEM_DONTCARE);

    // Aktuell eingestellter Font
    if ( bStyle && pFontItem )
    {
        FontInfo aInfo = pFontList->Get( pFontItem->GetFamilyName(), eWeight, eItalic );
        pStyleBox->SetText( pFontList->GetStyleName( aInfo ) );
    }
    else if ( !m_pImpl->m_bInSearchMode || !bStyle )
    {
        pStyleBox->SetText( String() );
    }
    else if ( bStyle )
    {
        FontInfo aInfo = pFontList->Get( String(), eWeight, eItalic );
        pStyleBox->SetText( pFontList->GetStyleName( aInfo ) );
    }
    if (!bStyleAvailable)
    {
        pStyleBox->Disable( );
        pStyleLabel->Disable( );
    }

    // SizeBox fuellen
    FillSizeBox_Impl( pNameBox );
    switch ( eLangGrp )
    {
        case Western : nWhich = GetWhich( SID_ATTR_CHAR_FONTHEIGHT ); break;
        case Asian : nWhich = GetWhich( SID_ATTR_CHAR_CJK_FONTHEIGHT ); break;
        case Ctl : nWhich = GetWhich( SID_ATTR_CHAR_CTL_FONTHEIGHT ); break;
    }
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
    {
        pSizeBox->SetText( String() );
        if ( eState <= SFX_ITEM_READONLY )
        {
            pSizeBox->Disable( );
            pSizeLabel->Disable( );
        }
    }

    switch ( eLangGrp )
    {
        case Western : nWhich = GetWhich( SID_ATTR_CHAR_LANGUAGE ); break;
        case Asian : nWhich = GetWhich( SID_ATTR_CHAR_CJK_LANGUAGE ); break;
        case Ctl : nWhich = GetWhich( SID_ATTR_CHAR_CTL_LANGUAGE ); break;
    }
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

    if ( Western == eLangGrp )
        m_aFontTypeFT.SetText( pFontList->GetFontMapText(
            pFontList->Get( pNameBox->GetText(), pStyleBox->GetText() ) ) );

    // save these settings
    pNameBox->SaveValue();
    pStyleBox->SaveValue();
    pSizeBox->SaveValue();
    pLangBox->SaveValue();
}

// -----------------------------------------------------------------------

BOOL SvxCharNamePage::FillItemSet_Impl( SfxItemSet& rSet, LanguageGroup eLangGrp )
{
    BOOL bModified = FALSE;

    FontNameBox* pNameBox = NULL;
    FontStyleBox* pStyleBox = NULL;
    FontSizeBox* pSizeBox = NULL;
    SvxLanguageBox* pLangBox = NULL;
    USHORT nWhich = 0;
    USHORT nSlot = 0;

    switch ( eLangGrp )
    {
        case Western :
            pNameBox = m_pWestFontNameLB;
            pStyleBox = m_pWestFontStyleLB;
            pSizeBox = m_pWestFontSizeLB;
            pLangBox = m_pWestFontLanguageLB;
            nSlot = SID_ATTR_CHAR_FONT;
            break;

        case Asian :
            pNameBox = m_pEastFontNameLB;
            pStyleBox = m_pEastFontStyleLB;
            pSizeBox = m_pEastFontSizeLB;
            pLangBox = m_pEastFontLanguageLB;
            nSlot = SID_ATTR_CHAR_CJK_FONT;
            break;

        case Ctl :
            pNameBox = m_pCTLFontNameLB;
            pStyleBox = m_pCTLFontStyleLB;
            pSizeBox = m_pCTLFontSizeLB;
            pLangBox = m_pCTLFontLanguageLB;
            nSlot = SID_ATTR_CHAR_CTL_FONT;
            break;
    }

    nWhich = GetWhich( nSlot );
    const SfxPoolItem* pItem = NULL;
    const SfxItemSet& rOldSet = GetItemSet();
    const SfxPoolItem* pOld = NULL;

    const SfxItemSet* pExampleSet = GetTabDialog() ? GetTabDialog()->GetExampleSet() : NULL;

    FASTBOOL bChanged = TRUE;
    const String& rFontName  = pNameBox->GetText();
    const FontList* pFontList = GetFontList();
    String aStyleBoxText =pStyleBox->GetText();
    USHORT nEntryPos = pStyleBox->GetEntryPos( aStyleBoxText );
    if ( nEntryPos >= m_pImpl->m_nExtraEntryPos )
        aStyleBoxText.Erase();
    FontInfo aInfo( pFontList->Get( rFontName, aStyleBoxText ) );
    SvxFontItem aFontItem( aInfo.GetFamily(), aInfo.GetName(), aInfo.GetStyleName(),
                           aInfo.GetPitch(), aInfo.GetCharSet(), nWhich );
    pOld = GetOldItem( rSet, nSlot );

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
    switch ( eLangGrp )
    {
        case Western : nSlot = SID_ATTR_CHAR_WEIGHT; break;
        case Asian : nSlot = SID_ATTR_CHAR_CJK_WEIGHT; break;
        case Ctl : nSlot = SID_ATTR_CHAR_CTL_WEIGHT; break;
    }
    nWhich = GetWhich( nSlot );
    FontWeight eWeight = aInfo.GetWeight();
    if ( nEntryPos >= m_pImpl->m_nExtraEntryPos )
        eWeight = WEIGHT_NORMAL;
    SvxWeightItem aWeightItem( eWeight, nWhich );
    pOld = GetOldItem( rSet, nSlot );

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
        CLEARTITEM;

    bChanged = TRUE;
    switch ( eLangGrp )
    {
        case Western : nSlot = SID_ATTR_CHAR_POSTURE; break;
        case Asian : nSlot = SID_ATTR_CHAR_CJK_POSTURE; break;
        case Ctl : nSlot = SID_ATTR_CHAR_CTL_POSTURE; break;
    }
    nWhich = GetWhich( nSlot );
    FontItalic eItalic = aInfo.GetItalic();
    if ( nEntryPos >= m_pImpl->m_nExtraEntryPos )
        eItalic = ITALIC_NONE;
    SvxPostureItem aPostureItem( eItalic, nWhich );
    pOld = GetOldItem( rSet, nSlot );

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
        CLEARTITEM;

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

    switch ( eLangGrp )
    {
        case Western : nSlot = SID_ATTR_CHAR_FONTHEIGHT; break;
        case Asian : nSlot = SID_ATTR_CHAR_CJK_FONTHEIGHT; break;
        case Ctl : nSlot = SID_ATTR_CHAR_CTL_FONTHEIGHT; break;
    }
    nWhich = GetWhich( nSlot );
    const SvxFontHeightItem* pOldHeight = (const SvxFontHeightItem*)GetOldItem( rSet, nSlot );
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
        CLEARTITEM;

    bChanged = TRUE;
    switch ( eLangGrp )
    {
        case Western : nSlot = SID_ATTR_CHAR_LANGUAGE; break;
        case Asian : nSlot = SID_ATTR_CHAR_CJK_LANGUAGE; break;
        case Ctl : nSlot = SID_ATTR_CHAR_CTL_LANGUAGE; break;
    }
    nWhich = GetWhich( nSlot );
    pOld = GetOldItem( rSet, nSlot );
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
        CLEARTITEM;

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
            m_pColorLB->Hide();
            break;

        case SFX_ITEM_DISABLED:
        case SFX_ITEM_READONLY:
            m_pColorLB->Disable();
            break;

        case SFX_ITEM_DONTCARE:
            m_pColorLB->SetNoSelection();
            break;

        case SFX_ITEM_DEFAULT:
        case SFX_ITEM_SET:
        {
            SvxFont& rFont = GetPreviewFont();
            SvxFont& rCJKFont = GetPreviewCJKFont();
            SvxFont& rCTLFont = GetPreviewCTLFont();
            const SvxColorItem& rItem = (SvxColorItem&)rSet.Get( nWhich );
            Color aColor = rItem.GetValue();
            rFont.SetColor( aColor.GetColor() == COL_AUTO ? Color(COL_BLACK) : aColor );
            rCJKFont.SetColor( aColor.GetColor() == COL_AUTO ? Color(COL_BLACK) : aColor );
            rCTLFont.SetColor( aColor.GetColor() == COL_AUTO ? Color(COL_BLACK) : aColor );
            m_aPreviewWin.Invalidate();
            USHORT nSelPos = m_pColorLB->GetEntryPos( aColor );
            if ( nSelPos == LISTBOX_ENTRY_NOTFOUND && aColor == Color( COL_TRANSPARENT ) )
                nSelPos = m_pColorLB->GetEntryPos( m_pImpl->m_aTransparentText );

            if ( LISTBOX_ENTRY_NOTFOUND != nSelPos )
                m_pColorLB->SelectEntryPos( nSelPos );
            else
            {
                nSelPos = m_pColorLB->GetEntryPos( aColor );
                if ( LISTBOX_ENTRY_NOTFOUND != nSelPos )
                    m_pColorLB->SelectEntryPos( nSelPos );
                else
                    m_pColorLB->SelectEntryPos(
                        m_pColorLB->InsertEntry( aColor, String( SVX_RES( RID_SVXSTR_COLOR_USER ) ) ) );
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
    if ( m_pColorLB->GetSelectEntry() == m_pImpl->m_aTransparentText )
        aSelectedColor = Color( COL_TRANSPARENT );
    else
        aSelectedColor = m_pColorLB->GetSelectEntryColor();

    if ( pOld && pOld->GetValue() == aSelectedColor )
        bChanged = FALSE;

    if ( !bChanged )
        bChanged = ( m_pColorLB->GetSavedValue() == LISTBOX_ENTRY_NOTFOUND );

    if ( !bChanged && pExampleSet &&
         pExampleSet->GetItemState( nWhich, FALSE, (const SfxPoolItem**)&pItem ) == SFX_ITEM_SET &&
         ( (SvxColorItem*)pItem )->GetValue() != aSelectedColor )
        bChanged = TRUE;

    BOOL bModified = FALSE;

    if ( bChanged && m_pColorLB->GetSelectEntryPos() != LISTBOX_ENTRY_NOTFOUND )
    {
        rSet.Put( SvxColorItem( aSelectedColor, nWhich ) );
        bModified = TRUE;
    }
    else if ( SFX_ITEM_DEFAULT == rOldSet.GetItemState( nWhich, FALSE ) )
        CLEARTITEM;

    return bModified;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxCharNamePage, UpdateHdl_Impl, Timer*, EMPTYARG )
{
    UpdatePreview_Impl();
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxCharNamePage, FontModifyHdl_Impl, void*, pNameBox )
{
    m_pImpl->m_aUpdateTimer.Start();

    if ( m_pWestFontNameLB == pNameBox || m_pEastFontNameLB == pNameBox || m_pCTLFontNameLB == pNameBox )
    {
        FillStyleBox_Impl( (FontNameBox*)pNameBox );
        FillSizeBox_Impl( (FontNameBox*)pNameBox );
    }
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxCharNamePage, ColorBoxSelectHdl_Impl, ColorListBox*, pBox )
{
    SvxFont& rFont = GetPreviewFont();
    SvxFont& rCJKFont = GetPreviewCJKFont();
    SvxFont& rCTLFont = GetPreviewCTLFont();
    Color aSelectedColor;
    if ( pBox->GetSelectEntry() == m_pImpl->m_aTransparentText )
        aSelectedColor = Color( COL_TRANSPARENT );
    else
        aSelectedColor = pBox->GetSelectEntryColor();
    rFont.SetColor( aSelectedColor.GetColor() == COL_AUTO ? Color(COL_BLACK) : aSelectedColor );
    rCJKFont.SetColor( aSelectedColor.GetColor() == COL_AUTO ? Color(COL_BLACK) : aSelectedColor );
    rCTLFont.SetColor( aSelectedColor.GetColor() == COL_AUTO ? Color(COL_BLACK) : aSelectedColor );
    m_aPreviewWin.Invalidate();
    return 0;
}

// -----------------------------------------------------------------------

void SvxCharNamePage::ActivatePage( const SfxItemSet& rSet )
{
    SvxCharBasePage::ActivatePage( rSet );

    UpdatePreview_Impl();       // instead of asynchronous calling in ctor
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
    return pNameRanges;
}

// -----------------------------------------------------------------------

void SvxCharNamePage::Reset( const SfxItemSet& rSet )
{
    Reset_Impl( rSet, Western );
    Reset_Impl( rSet, Asian );
    Reset_Impl( rSet, Ctl );
    ResetColor_Impl( rSet );
    m_pColorLB->SaveValue();

    SetPrevFontWidthScale( rSet );
    UpdatePreview_Impl();
}

// -----------------------------------------------------------------------

BOOL SvxCharNamePage::FillItemSet( SfxItemSet& rSet )
{
    BOOL bModified = FillItemSet_Impl( rSet, Western );
    bModified |= FillItemSet_Impl( rSet, Asian );
    bModified |= FillItemSet_Impl( rSet, Ctl );
//! bModified |= FillItemSetColor_Impl( rSet );
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
namespace
{
    void enableRelativeMode(SvxCharNamePage* _pPage,
                            FontSizeBox* _pFontSizeLB,
                            USHORT _nHeightWhich)
    {
        _pFontSizeLB->EnableRelativeMode( 5, 995, 5 ); // min 5%, max 995%, step 5

        const SvxFontHeightItem& rHeightItem = (SvxFontHeightItem&)_pPage->GetItemSet().GetParent()->Get( _nHeightWhich );
        SfxMapUnit eUnit = _pPage->GetItemSet().GetPool()->GetMetric( _nHeightWhich );
        short nCurHeight = CalcToPoint( rHeightItem.GetHeight(), eUnit, 1 ) * 10;

        // ausgehend von der akt. Hoehe:
        //      - negativ bis minimal 2 pt
        //      - positiv bis maximal 999 pt
        _pFontSizeLB->EnablePtRelativeMode( -(nCurHeight - 20), (9999 - nCurHeight), 10 );
    }
}
// -----------------------------------------------------------------------------

void SvxCharNamePage::EnableRelativeMode()
{
    DBG_ASSERT( GetItemSet().GetParent(), "RelativeMode, but no ParentSet!" );
    enableRelativeMode(this,m_pWestFontSizeLB,GetWhich( SID_ATTR_CHAR_FONTHEIGHT ));
    enableRelativeMode(this,m_pEastFontSizeLB,GetWhich( SID_ATTR_CHAR_CJK_FONTHEIGHT ));
    enableRelativeMode(this,m_pCTLFontSizeLB,GetWhich( SID_ATTR_CHAR_CTL_FONTHEIGHT ));
}

// -----------------------------------------------------------------------

void SvxCharNamePage::EnableSearchMode()
{
    m_pImpl->m_bInSearchMode = TRUE;
}
// -----------------------------------------------------------------------
void SvxCharNamePage::DisableControls( USHORT nDisable )
{
    if ( DISABLE_LANGUAGE & nDisable )
    {
        if ( m_pWestFontLanguageFT ) m_pWestFontLanguageFT->Disable();
        if ( m_pWestFontLanguageLB ) m_pWestFontLanguageLB->Disable();
        if ( m_pEastFontLanguageFT ) m_pEastFontLanguageFT->Disable();
        if ( m_pEastFontLanguageLB ) m_pEastFontLanguageLB->Disable();
        if ( m_pCTLFontLanguageFT ) m_pCTLFontLanguageFT->Disable();
        if ( m_pCTLFontLanguageLB ) m_pCTLFontLanguageLB->Disable();
    }

    if ( DISABLE_HIDE_LANGUAGE & nDisable )
    {
        if ( m_pWestFontLanguageFT ) m_pWestFontLanguageFT->Hide();
        if ( m_pWestFontLanguageLB ) m_pWestFontLanguageLB->Hide();
        if ( m_pEastFontLanguageFT ) m_pEastFontLanguageFT->Hide();
        if ( m_pEastFontLanguageLB ) m_pEastFontLanguageLB->Hide();
        if ( m_pCTLFontLanguageFT ) m_pCTLFontLanguageFT->Hide();
        if ( m_pCTLFontLanguageLB ) m_pCTLFontLanguageLB->Hide();
    }
}

// -----------------------------------------------------------------------
void SvxCharNamePage::SetPreviewBackgroundToCharacter()
{
    m_bPreviewBackgroundToCharacter = sal_True;
}

// class SvxCharEffectsPage ----------------------------------------------

SvxCharEffectsPage::SvxCharEffectsPage( Window* pParent, const SfxItemSet& rInSet ) :

    SvxCharBasePage( pParent, SVX_RES( RID_SVXPAGE_CHAR_EFFECTS ), rInSet, WIN_EFFECTS_PREVIEW, FT_EFFECTS_FONTTYPE ),

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
    m_aFontColorFT          ( this, ResId( FT_FONTCOLOR ) ),
    m_aFontColorLB          ( this, ResId( LB_FONTCOLOR ) ),

    m_aEffectsFT            ( this, ResId( FT_EFFECTS ) ),
    m_aEffectsLB            ( this, 0 ),

    m_aEffects2LB           ( this, ResId( LB_EFFECTS2 ) ),

    m_aReliefFT             ( this, ResId( FT_RELIEF ) ),
    m_aReliefLB             ( this, ResId( LB_RELIEF ) ),

    m_aOutlineBtn           ( this, ResId( CB_OUTLINE ) ),
    m_aShadowBtn            ( this, ResId( CB_SHADOW ) ),
    m_aBlinkingBtn          ( this, ResId( CB_BLINKING ) ),

    m_aTransparentColorName ( ResId( STR_CHARNAME_TRANSPARENT ) )

{
    m_aEffectsLB.Hide();
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
    m_aFontColorLB.SetUpdateMode( FALSE );

    {
        SfxPoolItem* pDummy;
        SfxViewFrame* pFrame = SfxViewFrame::GetFirst( pDocSh );
        if ( !pFrame ||
             SFX_ITEM_DEFAULT > pFrame->GetBindings().QueryState( SID_ATTR_AUTO_COLOR_INVALID, pDummy ) )
        {
            m_aColorLB.InsertAutomaticEntry();
            m_aFontColorLB.InsertAutomaticEntry();
        }
    }
    for ( long i = 0; i < pColorTable->Count(); i++ )
    {
        XColorEntry* pEntry = pColorTable->Get(i);
        m_aColorLB.InsertEntry( pEntry->GetColor(), pEntry->GetName() );
        m_aFontColorLB.InsertEntry( pEntry->GetColor(), pEntry->GetName() );
    }

    m_aColorLB.SetUpdateMode( TRUE );
    m_aFontColorLB.SetUpdateMode( TRUE );
    m_aFontColorLB.SetSelectHdl( LINK( this, SvxCharEffectsPage, ColorBoxSelectHdl_Impl ) );

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
    m_aColorLB.SetSelectHdl( aLink );

    m_aUnderlineLB.SelectEntryPos( 0 );
    m_aStrikeoutLB.SelectEntryPos( 0 );
    m_aEmphasisLB.SelectEntryPos( 0 );
    m_aPositionLB.SelectEntryPos( 0 );
    m_aColorLB.SelectEntryPos( 0 );
    SelectHdl_Impl( NULL );
    SelectHdl_Impl( &m_aEmphasisLB );

    m_aEffects2LB.SelectEntryPos( 0 );

    m_aIndividualWordsBtn.SetClickHdl( LINK( this, SvxCharEffectsPage, CbClickHdl_Impl ) );
    aLink = LINK( this, SvxCharEffectsPage, TristClickHdl_Impl );
    m_aOutlineBtn.SetClickHdl( aLink );
    m_aShadowBtn.SetClickHdl( aLink );

    if ( !SvtLanguageOptions().IsAsianTypographyEnabled() )
    {
        m_aEmphasisFT.Hide();
        m_aEmphasisLB.Hide();
        m_aPositionFT.Hide();
        m_aPositionLB.Hide();

        m_aFontColorFT.SetPosPixel( m_aEmphasisFT.GetPosPixel() );
        m_aFontColorLB.SetPosPixel( m_aEmphasisLB.GetPosPixel() );
    }
}
// -----------------------------------------------------------------------

void SvxCharEffectsPage::UpdatePreview_Impl()
{
    SvxFont& rFont = GetPreviewFont();
    SvxFont& rCJKFont = GetPreviewCJKFont();
    SvxFont& rCTLFont = GetPreviewCTLFont();

    USHORT nPos = m_aUnderlineLB.GetSelectEntryPos();
    FontUnderline eUnderline = (FontUnderline)(ULONG)m_aUnderlineLB.GetEntryData( nPos );
    nPos = m_aStrikeoutLB.GetSelectEntryPos();
    FontStrikeout eStrikeout = (FontStrikeout)(ULONG)m_aStrikeoutLB.GetEntryData( nPos );
    rFont.SetUnderline( eUnderline );
    rCJKFont.SetUnderline( eUnderline );
    rCTLFont.SetUnderline( eUnderline );

    m_aPreviewWin.SetTextLineColor( m_aColorLB.GetSelectEntryColor() );
    rFont.SetStrikeout( eStrikeout );
    rCJKFont.SetStrikeout( eStrikeout );
    rCTLFont.SetStrikeout( eStrikeout );

    nPos = m_aPositionLB.GetSelectEntryPos();
    BOOL bUnder = ( CHRDLG_POSITION_UNDER == (ULONG)m_aPositionLB.GetEntryData( nPos ) );
    FontEmphasisMark eMark = (FontEmphasisMark)m_aEmphasisLB.GetSelectEntryPos();
    eMark |= bUnder ? EMPHASISMARK_POS_BELOW : EMPHASISMARK_POS_ABOVE;
    rFont.SetEmphasisMark( eMark );
    rCJKFont.SetEmphasisMark( eMark );
    rCTLFont.SetEmphasisMark( eMark );

    USHORT nRelief = m_aReliefLB.GetSelectEntryPos();
    if(LISTBOX_ENTRY_NOTFOUND != nRelief)
    {
        rFont.SetRelief( (FontRelief)nRelief );
        rCJKFont.SetRelief( (FontRelief)nRelief );
        rCTLFont.SetRelief( (FontRelief)nRelief );
    }

    rFont.SetOutline( StateToAttr( m_aOutlineBtn.GetState() ) );
    rCJKFont.SetOutline( rFont.IsOutline() );
    rCTLFont.SetOutline( rFont.IsOutline() );

    rFont.SetShadow( StateToAttr( m_aShadowBtn.GetState() ) );
    rCJKFont.SetShadow( rFont.IsShadow() );
    rCTLFont.SetShadow( rFont.IsShadow() );

    USHORT nCapsPos = m_aEffects2LB.GetSelectEntryPos();
    if ( nCapsPos != LISTBOX_ENTRY_NOTFOUND )
    {
        rFont.SetCaseMap( (SvxCaseMap)nCapsPos );
        rCJKFont.SetCaseMap( (SvxCaseMap)nCapsPos );
        rCTLFont.SetCaseMap( (SvxCaseMap)nCapsPos );
    }

    BOOL bWordLine = m_aIndividualWordsBtn.IsChecked();
    rFont.SetWordLineMode( bWordLine );
    rCJKFont.SetWordLineMode( bWordLine );
    rCTLFont.SetWordLineMode( bWordLine );

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

void SvxCharEffectsPage::ResetColor_Impl( const SfxItemSet& rSet )
{
    USHORT nWhich = GetWhich( SID_ATTR_CHAR_COLOR );
    SfxItemState eState = rSet.GetItemState( nWhich );

    switch ( eState )
    {
        case SFX_ITEM_UNKNOWN:
            m_aFontColorFT.Hide();
            m_aFontColorLB.Hide();
            break;

        case SFX_ITEM_DISABLED:
        case SFX_ITEM_READONLY:
            m_aFontColorFT.Disable();
            m_aFontColorLB.Disable();
            break;

        case SFX_ITEM_DONTCARE:
            m_aFontColorLB.SetNoSelection();
            break;

        case SFX_ITEM_DEFAULT:
        case SFX_ITEM_SET:
        {
            SvxFont& rFont = GetPreviewFont();
            SvxFont& rCJKFont = GetPreviewCJKFont();
            SvxFont& rCTLFont = GetPreviewCTLFont();

            const SvxColorItem& rItem = (SvxColorItem&)rSet.Get( nWhich );
            Color aColor = rItem.GetValue();
            rFont.SetColor( aColor.GetColor() == COL_AUTO ? Color(COL_BLACK) : aColor );
            rCJKFont.SetColor( aColor.GetColor() == COL_AUTO ? Color(COL_BLACK) : aColor );
            rCTLFont.SetColor( aColor.GetColor() == COL_AUTO ? Color(COL_BLACK) : aColor );

            m_aPreviewWin.Invalidate();
            USHORT nSelPos = m_aFontColorLB.GetEntryPos( aColor );
            if ( nSelPos == LISTBOX_ENTRY_NOTFOUND && aColor == Color( COL_TRANSPARENT ) )
                nSelPos = m_aFontColorLB.GetEntryPos( m_aTransparentColorName );

            if ( LISTBOX_ENTRY_NOTFOUND != nSelPos )
                m_aFontColorLB.SelectEntryPos( nSelPos );
            else
            {
                nSelPos = m_aFontColorLB.GetEntryPos( aColor );
                if ( LISTBOX_ENTRY_NOTFOUND != nSelPos )
                    m_aFontColorLB.SelectEntryPos( nSelPos );
                else
                    m_aFontColorLB.SelectEntryPos(
                        m_aFontColorLB.InsertEntry( aColor, String( SVX_RES( RID_SVXSTR_COLOR_USER ) ) ) );
            }
            break;
        }
    }
}

// -----------------------------------------------------------------------

BOOL SvxCharEffectsPage::FillItemSetColor_Impl( SfxItemSet& rSet )
{
    USHORT nWhich = GetWhich( SID_ATTR_CHAR_COLOR );
    const SvxColorItem* pOld = (const SvxColorItem*)GetOldItem( rSet, SID_ATTR_CHAR_COLOR );
    const SvxColorItem* pItem = NULL;
    BOOL bChanged = TRUE;
    const SfxItemSet* pExampleSet = GetTabDialog() ? GetTabDialog()->GetExampleSet() : NULL;
    const SfxItemSet& rOldSet = GetItemSet();

    Color aSelectedColor;
    if ( m_aFontColorLB.GetSelectEntry() == m_aTransparentColorName )
        aSelectedColor = Color( COL_TRANSPARENT );
    else
        aSelectedColor = m_aFontColorLB.GetSelectEntryColor();

    if ( pOld && pOld->GetValue() == aSelectedColor )
        bChanged = FALSE;

    if ( !bChanged )
        bChanged = ( m_aFontColorLB.GetSavedValue() == LISTBOX_ENTRY_NOTFOUND );

    if ( !bChanged && pExampleSet &&
         pExampleSet->GetItemState( nWhich, FALSE, (const SfxPoolItem**)&pItem ) == SFX_ITEM_SET &&
         ( (SvxColorItem*)pItem )->GetValue() != aSelectedColor )
        bChanged = TRUE;

    BOOL bModified = FALSE;

    if ( bChanged && m_aFontColorLB.GetSelectEntryPos() != LISTBOX_ENTRY_NOTFOUND )
    {
        rSet.Put( SvxColorItem( aSelectedColor, nWhich ) );
        bModified = TRUE;
    }
    else if ( SFX_ITEM_DEFAULT == rOldSet.GetItemState( nWhich, FALSE ) )
        CLEARTITEM;

    return bModified;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxCharEffectsPage, SelectHdl_Impl, ListBox*, pBox )
{
    if ( &m_aEmphasisLB == pBox )
    {
        USHORT nEPos = m_aEmphasisLB.GetSelectEntryPos();
        BOOL bEnable = ( nEPos > 0 && nEPos != LISTBOX_ENTRY_NOTFOUND );
        m_aPositionFT.Enable( bEnable );
        m_aPositionLB.Enable( bEnable );
    }
    else if( &m_aReliefLB == pBox)
    {
        BOOL bEnable = ( pBox->GetSelectEntryPos() == 0 );
        m_aOutlineBtn.Enable( bEnable );
        m_aShadowBtn.Enable( bEnable );
    }
    else if ( &m_aPositionLB != pBox )
    {
        USHORT nUPos = m_aUnderlineLB.GetSelectEntryPos(), nSPos = m_aStrikeoutLB.GetSelectEntryPos();
        BOOL bEnable = ( nUPos > 0 && nUPos != LISTBOX_ENTRY_NOTFOUND );
        m_aColorFT.Enable( bEnable );
        m_aColorLB.Enable( bEnable );
        m_aIndividualWordsBtn.Enable( bEnable || ( nSPos > 0 && nSPos != LISTBOX_ENTRY_NOTFOUND ) );
    }
    UpdatePreview_Impl();
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxCharEffectsPage, UpdatePreview_Impl, ListBox*, EMPTYARG )
{
    BOOL bEnable = ( ( m_aUnderlineLB.GetSelectEntryPos() > 0 ) |
                    ( m_aStrikeoutLB.GetSelectEntryPos() > 0 ) );
    m_aIndividualWordsBtn.Enable( bEnable );

    UpdatePreview_Impl();
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxCharEffectsPage, CbClickHdl_Impl, CheckBox*, EMPTYARG )
{
    UpdatePreview_Impl();
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxCharEffectsPage, TristClickHdl_Impl, TriStateBox*, EMPTYARG )
{
    UpdatePreview_Impl();
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxCharEffectsPage, ColorBoxSelectHdl_Impl, ColorListBox*, pBox )
{
    SvxFont& rFont = GetPreviewFont();
    SvxFont& rCJKFont = GetPreviewCJKFont();
    SvxFont& rCTLFont = GetPreviewCTLFont();

    Color aSelectedColor;
    if ( pBox->GetSelectEntry() == m_aTransparentColorName )
        aSelectedColor = Color( COL_TRANSPARENT );
    else
        aSelectedColor = pBox->GetSelectEntryColor();
    rFont.SetColor( aSelectedColor.GetColor() == COL_AUTO ? Color(COL_BLACK) : aSelectedColor );
    rCJKFont.SetColor( aSelectedColor.GetColor() == COL_AUTO ? Color(COL_BLACK) : aSelectedColor );
    rCTLFont.SetColor( aSelectedColor.GetColor() == COL_AUTO ? Color(COL_BLACK) : aSelectedColor );

    m_aPreviewWin.Invalidate();
    return 0;
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
    return pEffectsRanges;
}

// -----------------------------------------------------------------------

void SvxCharEffectsPage::Reset( const SfxItemSet& rSet )
{
    SvxFont& rFont = GetPreviewFont();
    SvxFont& rCJKFont = GetPreviewCJKFont();
    SvxFont& rCTLFont = GetPreviewCTLFont();

    BOOL bEnable = FALSE;

    // Underline
    USHORT nWhich = GetWhich( SID_ATTR_CHAR_UNDERLINE );
    rFont.SetUnderline( UNDERLINE_NONE );
    rCJKFont.SetUnderline( UNDERLINE_NONE );
    rCTLFont.SetUnderline( UNDERLINE_NONE );

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
            rCJKFont.SetUnderline( eUnderline );
            rCTLFont.SetUnderline( eUnderline );

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

                if ( LISTBOX_ENTRY_NOTFOUND != nPos )
                    m_aColorLB.SelectEntryPos( nPos );
                else
                {
                    nPos = m_aColorLB.GetEntryPos( aColor );
                    if ( LISTBOX_ENTRY_NOTFOUND != nPos )
                        m_aColorLB.SelectEntryPos( nPos );
                    else
                        m_aColorLB.SelectEntryPos(
                            m_aColorLB.InsertEntry( aColor,
                                String( SVX_RES( RID_SVXSTR_COLOR_USER ) ) ) );
                }
            }
            else
            {
                m_aColorLB.SelectEntry( Color( COL_AUTO ));
                m_aColorLB.Disable();
            }
        }
    }

    //  Strikeout
    nWhich = GetWhich( SID_ATTR_CHAR_STRIKEOUT );
    rFont.SetStrikeout( STRIKEOUT_NONE );
    rCJKFont.SetStrikeout( STRIKEOUT_NONE );
    rCTLFont.SetStrikeout( STRIKEOUT_NONE );

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
            rCJKFont.SetStrikeout( eStrikeout );
            rCTLFont.SetStrikeout( eStrikeout );

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
    switch ( eState )
    {
        case SFX_ITEM_UNKNOWN:
            m_aIndividualWordsBtn.Hide();
            break;

        case SFX_ITEM_DISABLED:
        case SFX_ITEM_READONLY:
            m_aIndividualWordsBtn.Disable();
            break;

        case SFX_ITEM_DONTCARE:
            m_aIndividualWordsBtn.SetState( STATE_DONTKNOW );
            break;

        case SFX_ITEM_DEFAULT:
        case SFX_ITEM_SET:
        {
            const SvxWordLineModeItem& rItem = (SvxWordLineModeItem&)rSet.Get( nWhich );
            rFont.SetWordLineMode( rItem.GetValue() );
            rCJKFont.SetWordLineMode( rItem.GetValue() );
            rCTLFont.SetWordLineMode( rItem.GetValue() );

            m_aIndividualWordsBtn.Check( rItem.GetValue() );
            m_aIndividualWordsBtn.Enable( bEnable );
            break;
        }
    }

    // Emphasis
    nWhich = GetWhich( SID_ATTR_CHAR_EMPHASISMARK );
    eState = rSet.GetItemState( nWhich );

    if ( eState >= SFX_ITEM_DEFAULT )
    {
        const SvxEmphasisMarkItem& rItem = (SvxEmphasisMarkItem&)rSet.Get( nWhich );
        FontEmphasisMark eMark = rItem.GetEmphasisMark();
        rFont.SetEmphasisMark( eMark );
        rCJKFont.SetEmphasisMark( eMark );
        rCTLFont.SetEmphasisMark( eMark );

        m_aEmphasisLB.SelectEntryPos( (USHORT)( eMark & EMPHASISMARK_STYLE ) );
        eMark &= ~EMPHASISMARK_STYLE;
        ULONG nEntryData = ( eMark == EMPHASISMARK_POS_ABOVE )
            ? CHRDLG_POSITION_OVER
            : ( eMark == EMPHASISMARK_POS_BELOW ) ? CHRDLG_POSITION_UNDER : 0;

        for ( int i = 0; i < m_aPositionLB.GetEntryCount(); i++ )
        {
            if ( nEntryData == (ULONG)m_aPositionLB.GetEntryData(i) )
            {
                m_aPositionLB.SelectEntryPos(i);
                break;
            }
        }
    }
    else if ( eState == SFX_ITEM_DONTCARE )
        m_aEmphasisLB.SetNoSelection( );
    else if ( eState == SFX_ITEM_UNKNOWN )
    {
        m_aEmphasisFT.Hide();
        m_aEmphasisLB.Hide();
    }
    else // SFX_ITEM_DISABLED or SFX_ITEM_READONLY
    {
        m_aEmphasisFT.Disable();
        m_aEmphasisLB.Disable();
    }

    // the select handler for the underline/strikeout list boxes
//  SelectHdl_Impl( NULL );
    DBG_ASSERT(m_aUnderlineLB.GetSelectHdl() == m_aStrikeoutLB.GetSelectHdl(),
        "SvxCharEffectsPage::Reset: inconsistence (1)!");
    m_aUnderlineLB.GetSelectHdl().Call(NULL);
        // don't call SelectHdl_Impl directly!
        // in DisableControls, we may have re-reouted the select handler
        // 30.05.2001 - 86262 - frank.schoenheit@germany.sun.com

    // the select handler for the emphasis listbox
//  SelectHdl_Impl( &m_aEmphasisLB );
    DBG_ASSERT(m_aEmphasisLB.GetSelectHdl() == LINK(this, SvxCharEffectsPage, SelectHdl_Impl),
        "SvxCharEffectsPage::Reset: inconsistence (2)!");
    m_aEmphasisLB.GetSelectHdl().Call( &m_aEmphasisLB );
        // this is for consistency only. Here it would be allowed to call SelectHdl_Impl directly ...

    // Effects
    SvxCaseMap eCaseMap = SVX_CASEMAP_END;
    nWhich = GetWhich( SID_ATTR_CHAR_CASEMAP );
    eState = rSet.GetItemState( nWhich );
    switch ( eState )
    {
        case SFX_ITEM_UNKNOWN:
            m_aEffectsFT.Hide();
            m_aEffects2LB.Hide();
            break;

        case SFX_ITEM_DISABLED:
        case SFX_ITEM_READONLY:
            m_aEffectsFT.Disable();
            m_aEffects2LB.Disable();
            break;

        case SFX_ITEM_DONTCARE:
            m_aEffects2LB.SetNoSelection();
            break;

        case SFX_ITEM_DEFAULT:
        case SFX_ITEM_SET:
        {
            const SvxCaseMapItem& rItem = (const SvxCaseMapItem&)rSet.Get( nWhich );
            eCaseMap = (SvxCaseMap)rItem.GetValue();
            break;
        }
    }
    SetCaseMap_Impl( eCaseMap );

    //Relief
    nWhich = GetWhich(SID_ATTR_CHAR_RELIEF);
    eState = rSet.GetItemState( nWhich );
    switch ( eState )
    {
        case SFX_ITEM_UNKNOWN:
            m_aReliefFT.Hide();
            m_aReliefLB.Hide();
            break;

        case SFX_ITEM_DISABLED:
        case SFX_ITEM_READONLY:
            m_aReliefFT.Disable();
            m_aReliefLB.Disable();
            break;

        case SFX_ITEM_DONTCARE:
            m_aReliefLB.SetNoSelection();
            break;

        case SFX_ITEM_DEFAULT:
        case SFX_ITEM_SET:
        {
            const SvxCharReliefItem& rItem = (const SvxCharReliefItem&)rSet.Get( nWhich );
            m_aReliefLB.SelectEntryPos(rItem.GetValue());
            SelectHdl_Impl(&m_aReliefLB);
            break;
        }
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
    eState = rSet.GetItemState( nWhich );

    switch ( eState )
    {
        case SFX_ITEM_UNKNOWN:
            m_aBlinkingBtn.Hide();
            break;

        case SFX_ITEM_DISABLED:
        case SFX_ITEM_READONLY:
            m_aBlinkingBtn.Disable();
            break;

        case SFX_ITEM_DONTCARE:
            m_aBlinkingBtn.SetState( STATE_DONTKNOW );
            break;

        case SFX_ITEM_DEFAULT:
        case SFX_ITEM_SET:
        {
            const SvxBlinkItem& rItem = (SvxBlinkItem&)rSet.Get( nWhich );
            m_aBlinkingBtn.SetState( (TriState)rItem.GetValue() );
            m_aBlinkingBtn.EnableTriState( FALSE );
            break;
        }
    }

    SetPrevFontWidthScale( rSet );
    ResetColor_Impl( rSet );

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
    m_aFontColorLB.SaveValue();
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
        //! if there are different underline styles in the selection the
        //! item-state in the 'rOldSet' will be invalid. In this case
        //! changing the underline style will be allowed if a style is
        //! selected in the listbox.
        BOOL bAllowChg = LISTBOX_ENTRY_NOTFOUND != nPos  &&
                         SFX_ITEM_DEFAULT > rOldSet.GetItemState( nWhich, TRUE );

        const SvxUnderlineItem& rItem = *( (const SvxUnderlineItem*)pOld );
        if ( (FontUnderline)rItem.GetValue() == eUnder &&
             ( UNDERLINE_NONE == eUnder || rItem.GetColor() == m_aColorLB.GetSelectEntryColor() ) &&
             ! bAllowChg )
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
        CLEARTITEM;

    bChanged = TRUE;

    // Strikeout
    nWhich = GetWhich( SID_ATTR_CHAR_STRIKEOUT );
    pOld = GetOldItem( rSet, SID_ATTR_CHAR_STRIKEOUT );
    nPos = m_aStrikeoutLB.GetSelectEntryPos();
    FontStrikeout eStrike = (FontStrikeout)(ULONG)m_aStrikeoutLB.GetEntryData( nPos );

    if ( pOld )
    {
        //! if there are different strikeout styles in the selection the
        //! item-state in the 'rOldSet' will be invalid. In this case
        //! changing the strikeout style will be allowed if a style is
        //! selected in the listbox.
        BOOL bAllowChg = LISTBOX_ENTRY_NOTFOUND != nPos  &&
                         SFX_ITEM_DEFAULT > rOldSet.GetItemState( nWhich, TRUE );

        const SvxCrossedOutItem& rItem = *( (const SvxCrossedOutItem*)pOld );
        if ( !m_aStrikeoutLB.IsEnabled()
            || ((FontStrikeout)rItem.GetValue() == eStrike  && !bAllowChg) )
            bChanged = FALSE;
    }

    if ( bChanged )
    {
        rSet.Put( SvxCrossedOutItem( eStrike, nWhich ) );
        bModified |= TRUE;
    }
    else if ( SFX_ITEM_DEFAULT == rOldSet.GetItemState( nWhich, FALSE ) )
        CLEARTITEM;

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
        CLEARTITEM;

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
        if( rOldSet.GetItemState( nWhich ) != SFX_ITEM_DONTCARE )
        {
            const SvxEmphasisMarkItem& rItem = *( (const SvxEmphasisMarkItem*)pOld );
            if ( rItem.GetEmphasisMark() == eMark )
                bChanged = FALSE;
        }
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
        CLEARTITEM;

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
        //! if there are different effect styles in the selection the
        //! item-state in the 'rOldSet' will be invalid. In this case
        //! changing the effect style will be allowed if a style is
        //! selected in the listbox.
        BOOL bAllowChg = LISTBOX_ENTRY_NOTFOUND != nPos  &&
                         SFX_ITEM_DEFAULT > rOldSet.GetItemState( nWhich, TRUE );

        const SvxCaseMapItem& rItem = *( (const SvxCaseMapItem*)pOld );
        if ( (SvxCaseMap)rItem.GetValue() == eCaseMap  &&  !bAllowChg )
            bChanged = FALSE;
    }

    if ( bChanged && bChecked )
    {
        rSet.Put( SvxCaseMapItem( eCaseMap, nWhich ) );
        bModified |= TRUE;
    }
    else if ( SFX_ITEM_DEFAULT == rOldSet.GetItemState( nWhich, FALSE ) )
        CLEARTITEM;

    bChanged = TRUE;

    //Relief
    nWhich = GetWhich(SID_ATTR_CHAR_RELIEF);
    if(m_aReliefLB.GetSelectEntryPos() != m_aReliefLB.GetSavedValue())
    {
        m_aReliefLB.SaveValue();
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
        CLEARTITEM;

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
        bModified = TRUE;
    }
    else if ( SFX_ITEM_DEFAULT == rOldSet.GetItemState( nWhich, FALSE ) )
        CLEARTITEM;

    bChanged = TRUE;

    // Blinking
    nWhich = GetWhich( SID_ATTR_FLASH );
    pOld = GetOldItem( rSet, SID_ATTR_FLASH );
    eState = m_aBlinkingBtn.GetState();

    if ( pOld )
    {
        const SvxBlinkItem& rItem = *( (const SvxBlinkItem*)pOld );
        if ( rItem.GetValue() == StateToAttr( eState ) && m_aBlinkingBtn.GetSavedValue() == eState )
            bChanged = FALSE;
    }

    if ( !bChanged && pExampleSet && pExampleSet->GetItemState( nWhich, FALSE, &pItem ) == SFX_ITEM_SET &&
         !StateToAttr( eState ) && ( (SvxBlinkItem*)pItem )->GetValue() )
        bChanged = TRUE;

    if ( bChanged && eState != STATE_DONTKNOW )
    {
        rSet.Put( SvxBlinkItem( StateToAttr( eState ), nWhich ) );
        bModified = TRUE;
    }
    else if ( SFX_ITEM_DEFAULT == rOldSet.GetItemState( nWhich, FALSE ) )
        CLEARTITEM;


    bModified |= FillItemSetColor_Impl( rSet );

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

    if ( ( DISABLE_UNDERLINE_COLOR & nDisable ) == DISABLE_UNDERLINE_COLOR )
    {
        // disable the controls
        m_aColorFT.Disable( );
        m_aColorLB.Disable( );
        // and reroute the selection handler of the controls which normally would affect the color box dis-/enabling
        m_aUnderlineLB.SetSelectHdl(LINK(this, SvxCharEffectsPage, UpdatePreview_Impl));
        m_aStrikeoutLB.SetSelectHdl(LINK(this, SvxCharEffectsPage, UpdatePreview_Impl));
    }
}

void SvxCharEffectsPage::EnableFlash()
{
    if ( !( ( m_nHtmlMode & HTMLMODE_ON ) && !( m_nHtmlMode & HTMLMODE_BLINK ) ) )
        m_aBlinkingBtn.Show();
}

// -----------------------------------------------------------------------
void SvxCharEffectsPage::SetPreviewBackgroundToCharacter()
{
    m_bPreviewBackgroundToCharacter = TRUE;
}

// class SvxCharPositionPage ---------------------------------------------

SvxCharPositionPage::SvxCharPositionPage( Window* pParent, const SfxItemSet& rInSet ) :

    SvxCharBasePage( pParent, SVX_RES( RID_SVXPAGE_CHAR_POSITION ), rInSet, WIN_POS_PREVIEW, FT_POS_FONTTYPE ),

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
    m_aScalingFL        ( this, ResId( FL_SCALING ) ),
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

    GetPreviewFont().SetSize( Size( 0, 240 ) );
    GetPreviewCJKFont().SetSize( Size( 0, 240 ) );
    GetPreviewCTLFont().SetSize( Size( 0, 240 ) );

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

    aLink = LINK( this, SvxCharPositionPage, LoseFocusHdl_Impl );
    m_aHighLowEdit.SetLoseFocusHdl( aLink );
    m_aFontSizeEdit.SetLoseFocusHdl( aLink );

    m_aHighLowRB.SetClickHdl( LINK( this, SvxCharPositionPage, AutoPositionHdl_Impl ) );
    m_aFitToLineCB.SetClickHdl( LINK( this, SvxCharPositionPage, FitToLineHdl_Impl ) );
    m_aKerningLB.SetSelectHdl( LINK( this, SvxCharPositionPage, KerningSelectHdl_Impl ) );
    m_aKerningEdit.SetModifyHdl( LINK( this, SvxCharPositionPage, KerningModifyHdl_Impl ) );
    m_aPairKerningBtn.SetClickHdl( LINK( this, SvxCharPositionPage, PairKerningHdl_Impl ) );
    m_aScaleWidthMF.SetModifyHdl( LINK( this, SvxCharPositionPage, ScaleWidthModifyHdl_Impl ) );
}

// -----------------------------------------------------------------------

void SvxCharPositionPage::UpdatePreview_Impl( BYTE nProp, BYTE nEscProp, short nEsc )
{
    SetPrevFontEscapement( nProp, nEscProp, nEsc );
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

        m_aPreviewWin.SetFontWidthScale( nVal );
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
            SvxFont& rFont = GetPreviewFont();
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

    SvxFont& rFont = GetPreviewFont();
    SvxFont& rCJKFont = GetPreviewCJKFont();
    SvxFont& rCTLFont = GetPreviewCTLFont();

    rFont.SetFixKerning( (short)nKern );
    rCJKFont.SetFixKerning( (short)nKern );
    rCTLFont.SetFixKerning( (short)nKern );
    m_aPreviewWin.Invalidate();
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxCharPositionPage, PairKerningHdl_Impl, CheckBox*, EMPTYARG )
{
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxCharPositionPage, LoseFocusHdl_Impl, MetricField*, pField )
{
    sal_Bool bHigh = m_aHighPosBtn.IsChecked();
    sal_Bool bLow = m_aLowPosBtn.IsChecked();
    DBG_ASSERT( bHigh || bLow, "normal position is not valid" );

    if ( &m_aHighLowEdit == pField )
    {
        if ( bLow )
            m_nSubEsc = (short)m_aHighLowEdit.GetValue() * -1;
        else
            m_nSuperEsc = (short)m_aHighLowEdit.GetValue();
    }
    else if ( &m_aFontSizeEdit == pField )
    {
        if ( bLow )
            m_nSubProp = (BYTE)m_aFontSizeEdit.GetValue();
        else
            m_nSuperProp = (BYTE)m_aFontSizeEdit.GetValue();
    }
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxCharPositionPage, ScaleWidthModifyHdl_Impl, MetricField*, pField )
{
    m_aPreviewWin.SetFontWidthScale( USHORT( m_aScaleWidthMF.GetValue() ) );

    return 0;
}

/* -----------------28.08.2003 12:12-----------------

 --------------------------------------------------*/
void  SvxCharPositionPage::ActivatePage( const SfxItemSet& rSet )
{
    //update the preview
    SvxCharBasePage::ActivatePage( rSet );

    //the only thing that has to be checked is the max. allowed value for the
    //condense edit field
    if ( m_aKerningLB.GetSelectEntryPos() == 2 )
    {
        // Condensed -> max value == 1/6 of the current font height
        SvxFont& rFont = GetPreviewFont();
        long nMax = rFont.GetSize().Height() / 6;
        long nKern = (short)m_aKerningEdit.Denormalize( LogicToLogic( m_aKerningEdit.GetValue(), MAP_POINT, MAP_TWIP ) );
        m_aKerningEdit.SetMax( m_aKerningEdit.Normalize( nKern > nMax ? nKern : nMax ), FUNIT_TWIP );
        m_aKerningEdit.SetLast( m_aKerningEdit.GetMax( m_aKerningEdit.GetUnit() ) );
    }
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
    return pPositionRanges;
}

// -----------------------------------------------------------------------
#define ENABLE_AND_CHECK( aCheckBox )   \
    if ( !aCheckBox.IsEnabled() )       \
        aCheckBox.Enable();             \
    aCheckBox.Check( TRUE )

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

    SvxFont& rFont = GetPreviewFont();
    SvxFont& rCJKFont = GetPreviewCJKFont();
    SvxFont& rCTLFont = GetPreviewCTLFont();
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
                    ENABLE_AND_CHECK( m_aHighLowRB );
                    nEsc = DFLT_ESC_SUPER;
                }
            }
            else
            {
                nFac = -1;
                m_aLowPosBtn.Check( TRUE );
                if ( nEsc == DFLT_ESC_AUTO_SUB )
                {
                    ENABLE_AND_CHECK( m_aHighLowRB );
                    nEsc = DFLT_ESC_SUB;
                }
            }

            if ( m_aHighLowRB.IsChecked() )
            {
                m_aHighLowFT.Disable();
                m_aHighLowEdit.Disable();
            }
            m_aHighLowEdit.SetValue( m_aHighLowEdit.Normalize( nFac * nEsc ) );
        }
        else
        {
            m_aNormalPosBtn.Check( TRUE );
            m_aHighLowRB.Check( TRUE );
            PositionHdl_Impl( NULL );
        }
        //the height has to be set after the handler is called to keep the value also if the escapement is zero
        m_aFontSizeEdit.SetValue( m_aFontSizeEdit.Normalize( nEscProp ) );
    }
    else
    {
        m_aHighPosBtn.Check( FALSE );
        m_aNormalPosBtn.Check( FALSE );
        m_aLowPosBtn.Check( FALSE );
    }

    // BspFont setzen
    SetPrevFontEscapement( nProp, nEscProp, nEsc );

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
        rCJKFont.SetFixKerning( (short)nKern );
        rCTLFont.SetFixKerning( (short)nKern );

        if ( nKerning > 0 )
        {
            m_aKerningLB.SelectEntryPos( LW_GESPERRT );
        }
        else if ( nKerning < 0 )
        {
            m_aKerningLB.SelectEntryPos( LW_SCHMAL );
            nKerning = -nKerning;
        }
        else
        {
            nKerning = 0;
            m_aKerningLB.SelectEntryPos( LW_NORMAL );
        }
        //enable/disable and set min/max of the Edit
        KerningSelectHdl_Impl(&m_aKerningLB);
        //the attribute value must be displayed also if it's above the maximum allowed value
        long nVal = m_aKerningEdit.GetMax();
        if(nVal < nKerning)
            m_aKerningEdit.SetMax( nKerning );
        m_aKerningEdit.SetValue( nKerning );
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
        const SvxCharScaleWidthItem& rItem = ( SvxCharScaleWidthItem& ) rSet.Get( nWhich );
        m_nScaleWidthInitialVal = rItem.GetValue();
        m_aScaleWidthMF.SetValue( m_nScaleWidthInitialVal );
    }
    else
        m_aScaleWidthMF.SetValue( 100 );

    nWhich = GetWhich( SID_ATTR_CHAR_WIDTH_FIT_TO_LINE );
    if ( rSet.GetItemState( nWhich ) >= SFX_ITEM_DEFAULT )
        m_nScaleWidthItemSetVal = ((SfxUInt16Item&) rSet.Get( nWhich )).GetValue();

    // Rotation
    nWhich = GetWhich( SID_ATTR_CHAR_ROTATED );
    SfxItemState eState = rSet.GetItemState( nWhich );
    if( SFX_ITEM_UNKNOWN == eState )
    {
        m_aRotationScalingFL.Hide();
        m_aScalingFL.Show();
        m_a0degRB.Hide();
        m_a90degRB.Hide();
        m_a270degRB.Hide();
        m_aFitToLineCB.Hide();
        m_aFitToLineCB .Hide();


        // move the following controls upwards
        Window* aCntrlArr[] = {
            &m_aScaleWidthFT, &m_aScaleWidthMF, &m_aKerningLine,
            &m_aKerningLB, &m_aKerningFT, &m_aKerningEdit, &m_aPairKerningBtn,
            0 };

        long nDiff = m_aScaleWidthMF.GetPosPixel().Y() -
                        m_a0degRB.GetPosPixel().Y();

        for( Window** ppW = aCntrlArr; *ppW; ++ppW )
        {
            Point aPnt( (*ppW)->GetPosPixel() );
            aPnt.Y() -= nDiff;
            (*ppW)->SetPosPixel( aPnt );
        }
    }
    else
    {
        m_aScalingFL.Hide();

        Link aOldLink( m_aFitToLineCB.GetClickHdl() );
        m_aFitToLineCB.SetClickHdl( Link() );
        if( eState >= SFX_ITEM_DEFAULT )
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
            if( eState == SFX_ITEM_DONTCARE )
            {
                m_a0degRB.Check( FALSE );
                m_a90degRB.Check( FALSE );
                m_a270degRB.Check( FALSE );
            }
            else
                m_a0degRB.Check( TRUE );

            m_aFitToLineCB.Check( FALSE );
        }
        m_aFitToLineCB.SetClickHdl( aOldLink );
        m_aFitToLineCB.Enable( !m_a0degRB.IsChecked() );

        // is this value set?
        if( SFX_ITEM_UNKNOWN == rSet.GetItemState( GetWhich(
                                        SID_ATTR_CHAR_WIDTH_FIT_TO_LINE ) ))
            m_aFitToLineCB.Hide();
    }

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
        bModified = TRUE;
    }
    else if ( SFX_ITEM_DEFAULT == rOldSet.GetItemState( nWhich, FALSE ) )
        CLEARTITEM;

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
           ( !m_aKerningEdit.GetSavedValue().Len() && m_aKerningEdit.IsEnabled() ) ) )
        bChanged = TRUE;

    if ( bChanged && nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        rSet.Put( SvxKerningItem( nKerning, nWhich ) );
        bModified |= TRUE;
    }
    else if ( SFX_ITEM_DEFAULT == rOldSet.GetItemState( nWhich, FALSE ) )
        CLEARTITEM;

    bChanged = TRUE;

    // Pair-Kerning
    nWhich = GetWhich( SID_ATTR_CHAR_AUTOKERN );

    if ( m_aPairKerningBtn.IsChecked() != m_aPairKerningBtn.GetSavedValue() )
    {
        rSet.Put( SvxAutoKernItem( m_aPairKerningBtn.IsChecked(), nWhich ) );
        bModified |= TRUE;
    }
    else if ( SFX_ITEM_DEFAULT == rOldSet.GetItemState( nWhich, FALSE ) )
        CLEARTITEM;

    // Scale Width
    nWhich = GetWhich( SID_ATTR_CHAR_SCALEWIDTH );
    if ( m_aScaleWidthMF.GetText() != m_aScaleWidthMF.GetSavedValue() )
    {
        rSet.Put( SvxCharScaleWidthItem( (USHORT)m_aScaleWidthMF.GetValue(), nWhich ) );
        bModified |= TRUE;
    }
    else if ( SFX_ITEM_DEFAULT == rOldSet.GetItemState( nWhich, FALSE ) )
        CLEARTITEM;

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
        CLEARTITEM;

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

// -----------------------------------------------------------------------
void SvxCharPositionPage::SetPreviewBackgroundToCharacter()
{
    m_bPreviewBackgroundToCharacter = TRUE;
}
// class SvxCharTwoLinesPage ------------------------------------------------

SvxCharTwoLinesPage::SvxCharTwoLinesPage( Window* pParent, const SfxItemSet& rInSet ) :

    SvxCharBasePage( pParent, SVX_RES( RID_SVXPAGE_CHAR_TWOLINES ), rInSet, WIN_TWOLINES_PREVIEW, FT_TWOLINES_FONTTYPE ),

    m_aSwitchOnLine     ( this, ResId( FL_SWITCHON ) ),
    m_aTwoLinesBtn      ( this, ResId( CB_TWOLINES ) ),

    m_aEncloseLine      ( this, ResId( FL_ENCLOSE ) ),
    m_aStartBracketFT   ( this, ResId( FT_STARTBRACKET ) ),
    m_aStartBracketLB   ( this, ResId( ED_STARTBRACKET ) ),
    m_aEndBracketFT     ( this, ResId( FT_ENDBRACKET ) ),
    m_aEndBracketLB     ( this, ResId( ED_ENDBRACKET ) )
{
    FreeResource();
    Initialize();
}

// -----------------------------------------------------------------------

void SvxCharTwoLinesPage::Initialize()
{
    Size aSize = m_aStartBracketLB.GetSizePixel();
    aSize.Height() = m_aStartBracketLB.CalcSize( 1, 6 ).Height();
    m_aStartBracketLB.SetSizePixel( aSize );
    aSize = m_aEndBracketLB.GetSizePixel();
    aSize.Height() = m_aEndBracketLB.CalcSize( 1, 6 ).Height();
    m_aEndBracketLB.SetSizePixel( aSize );

    m_aTwoLinesBtn.Check( FALSE );
    TwoLinesHdl_Impl( NULL );

    m_aTwoLinesBtn.SetClickHdl( LINK( this, SvxCharTwoLinesPage, TwoLinesHdl_Impl ) );

    Link aLink = LINK( this, SvxCharTwoLinesPage, CharacterMapHdl_Impl );
    m_aStartBracketLB.SetSelectHdl( aLink );
    m_aEndBracketLB.SetSelectHdl( aLink );

    SvxFont& rFont = GetPreviewFont();
    SvxFont& rCJKFont = GetPreviewCJKFont();
    SvxFont& rCTLFont = GetPreviewCTLFont();
    rFont.SetSize( Size( 0, 220 ) );
    rCJKFont.SetSize( Size( 0, 220 ) );
    rCTLFont.SetSize( Size( 0, 220 ) );
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
    return pTwoLinesRanges;
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

    SetPrevFontWidthScale( rSet );
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
        CLEARTITEM;

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
// -----------------------------------------------------------------------
void SvxCharTwoLinesPage::SetPreviewBackgroundToCharacter()
{
    m_bPreviewBackgroundToCharacter = TRUE;
}


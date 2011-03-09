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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_cui.hxx"

// include ---------------------------------------------------------------

#include "align.hxx"

#include <editeng/svxenum.hxx>
#include <svx/dialogs.hrc>
#include <cuires.hrc>
#include "align.hrc"
#include <svx/rotmodit.hxx>

#include <svx/algitem.hxx>
#include <editeng/frmdiritem.hxx>
#include <dialmgr.hxx>
#include <svx/dlgutil.hxx>
#include <tools/shl.hxx>
#include <sfx2/app.hxx>
#include <sfx2/module.hxx>
#include <sfx2/itemconnect.hxx>
#include <svl/cjkoptions.hxx>
#include <svl/languageoptions.hxx>
#include <svtools/localresaccess.hxx>
#include <svx/flagsdef.hxx> //CHINA001
#include <svl/intitem.hxx> //CHINA001
#include <sfx2/request.hxx> //CHINA001

namespace svx {

// item connections ===========================================================

// horizontal alignment -------------------------------------------------------

typedef sfx::ValueItemWrapper< SvxHorJustifyItem, SvxCellHorJustify, sal_uInt16 > HorJustItemWrapper;
typedef sfx::ListBoxConnection< HorJustItemWrapper > HorJustConnection;

static const HorJustConnection::MapEntryType s_pHorJustMap[] =
{
    { ALIGNDLG_HORALIGN_STD,    SVX_HOR_JUSTIFY_STANDARD    },
    { ALIGNDLG_HORALIGN_LEFT,   SVX_HOR_JUSTIFY_LEFT        },
    { ALIGNDLG_HORALIGN_CENTER, SVX_HOR_JUSTIFY_CENTER      },
    { ALIGNDLG_HORALIGN_RIGHT,  SVX_HOR_JUSTIFY_RIGHT       },
    { ALIGNDLG_HORALIGN_BLOCK,  SVX_HOR_JUSTIFY_BLOCK       },
    { ALIGNDLG_HORALIGN_FILL,   SVX_HOR_JUSTIFY_REPEAT      },
    { LISTBOX_ENTRY_NOTFOUND,   SVX_HOR_JUSTIFY_STANDARD    }
};

// vertical alignment ---------------------------------------------------------

typedef sfx::ValueItemWrapper< SvxVerJustifyItem, SvxCellVerJustify, sal_uInt16 > VerJustItemWrapper;
typedef sfx::ListBoxConnection< VerJustItemWrapper > VerJustConnection;

static const VerJustConnection::MapEntryType s_pVerJustMap[] =
{
    { ALIGNDLG_VERALIGN_STD,    SVX_VER_JUSTIFY_STANDARD    },
    { ALIGNDLG_VERALIGN_TOP,    SVX_VER_JUSTIFY_TOP         },
    { ALIGNDLG_VERALIGN_MID,    SVX_VER_JUSTIFY_CENTER      },
    { ALIGNDLG_VERALIGN_BOTTOM, SVX_VER_JUSTIFY_BOTTOM      },
    { LISTBOX_ENTRY_NOTFOUND,   SVX_VER_JUSTIFY_STANDARD    }
};

// cell rotate mode -----------------------------------------------------------

typedef sfx::ValueItemWrapper< SvxRotateModeItem, SvxRotateMode, sal_uInt16 > RotateModeItemWrapper;
typedef sfx::ValueSetConnection< RotateModeItemWrapper > RotateModeConnection;

static const RotateModeConnection::MapEntryType s_pRotateModeMap[] =
{
    { IID_BOTTOMLOCK,           SVX_ROTATE_MODE_BOTTOM      },
    { IID_TOPLOCK,              SVX_ROTATE_MODE_TOP         },
    { IID_CELLLOCK,             SVX_ROTATE_MODE_STANDARD    },
    { VALUESET_ITEM_NOTFOUND,   SVX_ROTATE_MODE_STANDARD    }
};

// ============================================================================

static sal_uInt16 s_pRanges[] =
{
    SID_ATTR_ALIGN_HOR_JUSTIFY,SID_ATTR_ALIGN_VER_JUSTIFY,
    SID_ATTR_ALIGN_STACKED,SID_ATTR_ALIGN_LINEBREAK,
    SID_ATTR_ALIGN_INDENT,SID_ATTR_ALIGN_INDENT,
    SID_ATTR_ALIGN_DEGREES,SID_ATTR_ALIGN_DEGREES,
    SID_ATTR_ALIGN_LOCKPOS,SID_ATTR_ALIGN_LOCKPOS,
    SID_ATTR_ALIGN_HYPHENATION,SID_ATTR_ALIGN_HYPHENATION,
    SID_ATTR_ALIGN_ASIANVERTICAL,SID_ATTR_ALIGN_ASIANVERTICAL,
    SID_ATTR_FRAMEDIRECTION,SID_ATTR_FRAMEDIRECTION,
    SID_ATTR_ALIGN_SHRINKTOFIT,SID_ATTR_ALIGN_SHRINKTOFIT,
    0
};

// ============================================================================

AlignmentTabPage::AlignmentTabPage( Window* pParent, const SfxItemSet& rCoreAttrs ) :

    SfxTabPage( pParent, CUI_RES( RID_SVXPAGE_ALIGNMENT ), rCoreAttrs ),

    maFlAlignment   ( this, CUI_RES( FL_ALIGNMENT ) ),
    maFtHorAlign    ( this, CUI_RES( FT_HORALIGN ) ),
    maLbHorAlign    ( this, CUI_RES( LB_HORALIGN ) ),
    maFtIndent      ( this, CUI_RES( FT_INDENT ) ),
    maEdIndent      ( this, CUI_RES( ED_INDENT ) ),
    maFtVerAlign    ( this, CUI_RES( FT_VERALIGN ) ),
    maLbVerAlign    ( this, CUI_RES( LB_VERALIGN ) ),

    maFlOrient      ( this, CUI_RES( FL_ORIENTATION ) ),
    maCtrlDial      ( this, CUI_RES( CTR_DIAL ) ),
    maFtRotate      ( this, CUI_RES( FT_DEGREES ) ),
    maNfRotate      ( this, CUI_RES( NF_DEGREES ) ),
    maFtRefEdge     ( this, CUI_RES( FT_BORDER_LOCK ) ),
    maVsRefEdge     ( this, CUI_RES( CTR_BORDER_LOCK ) ),
    maCbStacked     ( this, CUI_RES( BTN_TXTSTACKED ) ),
    maCbAsianMode   ( this, CUI_RES( BTN_ASIAN_VERTICAL ) ),
    maOrientHlp     ( maCtrlDial, maNfRotate, maCbStacked ),

    maFlProperties  ( this, CUI_RES( FL_WRAP ) ),
    maBtnWrap       ( this, CUI_RES( BTN_WRAP ) ),
    maBtnHyphen     ( this, CUI_RES( BTN_HYPH ) ),
    maBtnShrink     ( this, CUI_RES( BTN_SHRINK ) ),
    maFtFrameDir    ( this, CUI_RES( FT_TEXTFLOW ) ),
    maLbFrameDir    ( this, CUI_RES( LB_FRAMEDIR ) )
{
    InitVsRefEgde();

    // windows to be disabled, if stacked text is turned ON
    maOrientHlp.AddDependentWindow( maFtRotate,     STATE_CHECK );
    maOrientHlp.AddDependentWindow( maFtRefEdge,    STATE_CHECK );
    maOrientHlp.AddDependentWindow( maVsRefEdge,    STATE_CHECK );
    // windows to be disabled, if stacked text is turned OFF
    maOrientHlp.AddDependentWindow( maCbAsianMode,  STATE_NOCHECK );

    Link aLink = LINK( this, AlignmentTabPage, UpdateEnableHdl );

    maLbHorAlign.SetSelectHdl( aLink );
    maBtnWrap.SetClickHdl( aLink );

    // Asian vertical mode
    maCbAsianMode.Show( SvtCJKOptions().IsVerticalTextEnabled() );

    // CTL frame direction
    maLbFrameDir.InsertEntryValue( CUI_RESSTR( RID_SVXSTR_FRAMEDIR_LTR ), FRMDIR_HORI_LEFT_TOP );
    maLbFrameDir.InsertEntryValue( CUI_RESSTR( RID_SVXSTR_FRAMEDIR_RTL ), FRMDIR_HORI_RIGHT_TOP );
    maLbFrameDir.InsertEntryValue( CUI_RESSTR( RID_SVXSTR_FRAMEDIR_SUPER ), FRMDIR_ENVIRONMENT );
    if( !SvtLanguageOptions().IsCTLFontEnabled() )
    {
        maFtFrameDir.Hide();
        maLbFrameDir.Hide();
    }

    // diese Page braucht ExchangeSupport
    SetExchangeSupport();

    FreeResource();

    AddItemConnection( new sfx::DummyItemConnection( SID_ATTR_ALIGN_HOR_JUSTIFY, maFtHorAlign, sfx::ITEMCONN_HIDE_UNKNOWN ) );
    AddItemConnection( new HorJustConnection( SID_ATTR_ALIGN_HOR_JUSTIFY, maLbHorAlign, s_pHorJustMap, sfx::ITEMCONN_HIDE_UNKNOWN ) );
    AddItemConnection( new sfx::DummyItemConnection( SID_ATTR_ALIGN_INDENT, maFtIndent, sfx::ITEMCONN_HIDE_UNKNOWN ) );
    AddItemConnection( new sfx::UInt16MetricConnection( SID_ATTR_ALIGN_INDENT, maEdIndent, FUNIT_TWIP, sfx::ITEMCONN_HIDE_UNKNOWN ) );
    AddItemConnection( new sfx::DummyItemConnection( SID_ATTR_ALIGN_VER_JUSTIFY, maFtVerAlign, sfx::ITEMCONN_HIDE_UNKNOWN ) );
    AddItemConnection( new VerJustConnection( SID_ATTR_ALIGN_VER_JUSTIFY, maLbVerAlign, s_pVerJustMap, sfx::ITEMCONN_HIDE_UNKNOWN ) );
    AddItemConnection( new DialControlConnection( SID_ATTR_ALIGN_DEGREES, maCtrlDial, sfx::ITEMCONN_HIDE_UNKNOWN ) );
    AddItemConnection( new sfx::DummyItemConnection( SID_ATTR_ALIGN_DEGREES, maFtRotate, sfx::ITEMCONN_HIDE_UNKNOWN ) );
    AddItemConnection( new sfx::DummyItemConnection( SID_ATTR_ALIGN_LOCKPOS, maFtRefEdge, sfx::ITEMCONN_HIDE_UNKNOWN ) );
    AddItemConnection( new RotateModeConnection( SID_ATTR_ALIGN_LOCKPOS, maVsRefEdge, s_pRotateModeMap, sfx::ITEMCONN_HIDE_UNKNOWN ) );
    AddItemConnection( new OrientStackedConnection( SID_ATTR_ALIGN_STACKED, maOrientHlp ) );
    AddItemConnection( new sfx::DummyItemConnection( SID_ATTR_ALIGN_STACKED, maCbStacked, sfx::ITEMCONN_HIDE_UNKNOWN ) );
    AddItemConnection( new sfx::CheckBoxConnection( SID_ATTR_ALIGN_ASIANVERTICAL, maCbAsianMode, sfx::ITEMCONN_HIDE_UNKNOWN ) );
    AddItemConnection( new sfx::CheckBoxConnection( SID_ATTR_ALIGN_LINEBREAK, maBtnWrap, sfx::ITEMCONN_HIDE_UNKNOWN ) );
    AddItemConnection( new sfx::CheckBoxConnection( SID_ATTR_ALIGN_HYPHENATION, maBtnHyphen, sfx::ITEMCONN_HIDE_UNKNOWN ) );
    AddItemConnection( new sfx::CheckBoxConnection( SID_ATTR_ALIGN_SHRINKTOFIT, maBtnShrink, sfx::ITEMCONN_HIDE_UNKNOWN ) );
    AddItemConnection( new sfx::DummyItemConnection( SID_ATTR_FRAMEDIRECTION, maFtFrameDir, sfx::ITEMCONN_HIDE_UNKNOWN ) );
    AddItemConnection( new FrameDirListBoxConnection( SID_ATTR_FRAMEDIRECTION, maLbFrameDir, sfx::ITEMCONN_HIDE_UNKNOWN ) );

    maLbHorAlign.SetAccessibleRelationMemberOf( &maFlAlignment );
    maEdIndent.SetAccessibleRelationMemberOf( &maFlAlignment );
    maLbVerAlign.SetAccessibleRelationMemberOf( &maFlAlignment );
}

AlignmentTabPage::~AlignmentTabPage()
{
}

SfxTabPage* AlignmentTabPage::Create( Window* pParent, const SfxItemSet& rAttrSet )
{
    return new AlignmentTabPage( pParent, rAttrSet );
}

sal_uInt16* AlignmentTabPage::GetRanges()
{
    return s_pRanges;
}

void AlignmentTabPage::Reset( const SfxItemSet& rCoreAttrs )
{
    SfxTabPage::Reset( rCoreAttrs );
    UpdateEnableControls();
}

int AlignmentTabPage::DeactivatePage( SfxItemSet* _pSet )
{
    if( _pSet )
        FillItemSet( *_pSet );
    return LEAVE_PAGE;
}

void AlignmentTabPage::DataChanged( const DataChangedEvent& rDCEvt )
{
    SfxTabPage::DataChanged( rDCEvt );
    if( (rDCEvt.GetType() == DATACHANGED_SETTINGS) && (rDCEvt.GetFlags() & SETTINGS_STYLE) )
    {
        svt::OLocalResourceAccess aLocalResAcc( CUI_RES( RID_SVXPAGE_ALIGNMENT ), RSC_TABPAGE );
        InitVsRefEgde();
    }
}

void AlignmentTabPage::InitVsRefEgde()
{
    // remember selection - is deleted in call to ValueSet::Clear()
    sal_uInt16 nSel = maVsRefEdge.GetSelectItemId();

    ResId aResId( GetSettings().GetStyleSettings().GetHighContrastMode() ? IL_LOCK_BMPS_HC : IL_LOCK_BMPS, CUI_MGR() );
    ImageList aImageList( aResId );
    Size aItemSize( aImageList.GetImage( IID_BOTTOMLOCK ).GetSizePixel() );

    maVsRefEdge.Clear();
    maVsRefEdge.SetStyle( maVsRefEdge.GetStyle() | WB_ITEMBORDER | WB_DOUBLEBORDER );

    maVsRefEdge.SetColCount( 3 );
    maVsRefEdge.InsertItem( IID_BOTTOMLOCK, aImageList.GetImage( IID_BOTTOMLOCK ),  String( CUI_RES( STR_BOTTOMLOCK ) ) );
    maVsRefEdge.InsertItem( IID_TOPLOCK,    aImageList.GetImage( IID_TOPLOCK ),     String( CUI_RES( STR_TOPLOCK ) ) );
    maVsRefEdge.InsertItem( IID_CELLLOCK,   aImageList.GetImage( IID_CELLLOCK ),    String( CUI_RES( STR_CELLLOCK ) ) );

    maVsRefEdge.SetSizePixel( maVsRefEdge.CalcWindowSizePixel( aItemSize ) );

    maVsRefEdge.SelectItem( nSel );
}

void AlignmentTabPage::UpdateEnableControls()
{
    sal_uInt16 nHorAlign = maLbHorAlign.GetSelectEntryPos();
    bool bHorLeft  = (nHorAlign == ALIGNDLG_HORALIGN_LEFT);
    bool bHorBlock = (nHorAlign == ALIGNDLG_HORALIGN_BLOCK);
    bool bHorFill  = (nHorAlign == ALIGNDLG_HORALIGN_FILL);

    // indent edit field only for left alignment
    maFtIndent.Enable( bHorLeft );
    maEdIndent.Enable( bHorLeft );

    // rotation/stacked disabled for fill alignment
    maOrientHlp.Enable( !bHorFill );

    // hyphenation only for automatic line breaks or for block alignment
    maBtnHyphen.Enable( maBtnWrap.IsChecked() || bHorBlock );

    // shrink only without automatic line break, and not for block and fill
    maBtnShrink.Enable( (maBtnWrap.GetState() == STATE_NOCHECK) && !bHorBlock && !bHorFill );

    // visibility of fixed lines
    maFlAlignment.Show( maLbHorAlign.IsVisible() || maEdIndent.IsVisible() || maLbVerAlign.IsVisible() );
    maFlOrient.Show( maCtrlDial.IsVisible() || maVsRefEdge.IsVisible() || maCbStacked.IsVisible() || maCbAsianMode.IsVisible() );
    maFlProperties.Show( maBtnWrap.IsVisible() || maBtnHyphen.IsVisible() || maBtnShrink.IsVisible() || maLbFrameDir.IsVisible() );
}

IMPL_LINK( AlignmentTabPage, UpdateEnableHdl, void*, EMPTYARG )
{
    UpdateEnableControls();
    return 0;
}

// ============================================================================

} // namespace svx


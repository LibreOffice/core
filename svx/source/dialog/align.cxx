/*************************************************************************
 *
 *  $RCSfile: align.cxx,v $
 *
 *  $Revision: 1.17 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 17:39:45 $
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
 *  WITHOUT WARRUNTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRUNTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
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

#include <svxenum.hxx>
#pragma hdrstop

#include "dialogs.hrc"
#include "align.hrc"
#include "rotmodit.hxx"

#define ITEMID_HORJUSTIFY       SID_ATTR_ALIGN_HOR_JUSTIFY
#define ITEMID_VERJUSTIFY       SID_ATTR_ALIGN_VER_JUSTIFY
#define ITEMID_LINEBREAK        SID_ATTR_ALIGN_LINEBREAK
#define ITEMID_MARGIN           SID_ATTR_ALIGN_MARGIN
#define ITEMID_FRAMEDIR         SID_ATTR_FRAMEDIRECTION

#include "algitem.hxx"
#include "frmdiritem.hxx"
#include "align.hxx"
#include "dialmgr.hxx"
#include "dlgutil.hxx"

#ifndef _SHL_HXX //autogen
#include <tools/shl.hxx>
#endif
#ifndef _SFXAPP_HXX //autogen
#include <sfx2/app.hxx>
#endif
#ifndef _SFXMODULE_HXX
#include <sfx2/module.hxx>
#endif
#ifndef SFX_ITEMCONNECT_HXX
#include <sfx2/itemconnect.hxx>
#endif
#ifndef _SVTOOLS_CJKOPTIONS_HXX
#include <svtools/cjkoptions.hxx>
#endif
#ifndef _SVTOOLS_LANGUAGEOPTIONS_HXX
#include <svtools/languageoptions.hxx>
#endif
#ifndef _SVTOOLS_LOCALRESACCESS_HXX_
#include <svtools/localresaccess.hxx>
#endif
#include "svxids.hrc" //CHINA001
#include "flagsdef.hxx" //CHINA001
#include <svtools/intitem.hxx> //CHINA001
#include <sfx2/request.hxx> //CHINA001

namespace svx {

// item connections ===========================================================

// horizontal alignment -------------------------------------------------------

typedef sfx::ValueItemWrapper< SvxHorJustifyItem, SvxCellHorJustify > HorJustItemWrapper;
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

typedef sfx::ValueItemWrapper< SvxVerJustifyItem, SvxCellVerJustify > VerJustItemWrapper;
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

typedef sfx::ValueItemWrapper< SvxRotateModeItem, SvxRotateMode > RotateModeItemWrapper;
typedef sfx::ValueSetConnection< RotateModeItemWrapper > RotateModeConnection;

static const RotateModeConnection::MapEntryType s_pRotateModeMap[] =
{
    { IID_BOTTOMLOCK,           SVX_ROTATE_MODE_BOTTOM      },
    { IID_TOPLOCK,              SVX_ROTATE_MODE_TOP         },
    { IID_CELLLOCK,             SVX_ROTATE_MODE_STANDARD    },
    { VALUESET_ITEM_NOTFOUND,   SVX_ROTATE_MODE_STANDARD    }
};

// ============================================================================

static USHORT s_pRanges[] =
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

    SfxTabPage( pParent, SVX_RES( RID_SVXPAGE_ALIGNMENT ), rCoreAttrs ),

    maFlAlignment   ( this, ResId( FL_ALIGNMENT ) ),
    maFtHorAlign    ( this, ResId( FT_HORALIGN ) ),
    maLbHorAlign    ( this, ResId( LB_HORALIGN ) ),
    maFtIndent      ( this, ResId( FT_INDENT ) ),
    maEdIndent      ( this, ResId( ED_INDENT ) ),
    maFtVerAlign    ( this, ResId( FT_VERALIGN ) ),
    maLbVerAlign    ( this, ResId( LB_VERALIGN ) ),

    maFlOrient      ( this, ResId( FL_ORIENTATION ) ),
    maCtrlDial      ( this, ResId( CTR_DIAL ) ),
    maFtRotate      ( this, ResId( FT_DEGREES ) ),
    maNfRotate      ( this, ResId( NF_DEGREES ) ),
    maFtRefEdge     ( this, ResId( FT_BORDER_LOCK ) ),
    maVsRefEdge     ( this, ResId( CTR_BORDER_LOCK ) ),
    maCbStacked     ( this, ResId( BTN_TXTSTACKED ) ),
    maCbAsianMode   ( this, ResId( BTN_ASIAN_VERTICAL ) ),
    maOrientHlp     ( this, maCtrlDial, maNfRotate, maCbStacked ),

    maFlProperties  ( this, ResId( FL_WRAP ) ),
    maBtnWrap       ( this, ResId( BTN_WRAP ) ),
    maBtnHyphen     ( this, ResId( BTN_HYPH ) ),
    maBtnShrink     ( this, ResId( BTN_SHRINK ) ),
    maFtFrameDir    ( this, ResId( FT_TEXTFLOW ) ),
    maLbFrameDir    ( this, ResId( LB_FRAMEDIR ) )
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
    maLbFrameDir.InsertEntryValue( SVX_RESSTR( RID_SVXSTR_FRAMEDIR_LTR ), FRMDIR_HORI_LEFT_TOP );
    maLbFrameDir.InsertEntryValue( SVX_RESSTR( RID_SVXSTR_FRAMEDIR_RTL ), FRMDIR_HORI_RIGHT_TOP );
    maLbFrameDir.InsertEntryValue( SVX_RESSTR( RID_SVXSTR_FRAMEDIR_SUPER ), FRMDIR_ENVIRONMENT );
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
}

AlignmentTabPage::~AlignmentTabPage()
{
}

SfxTabPage* AlignmentTabPage::Create( Window* pParent, const SfxItemSet& rAttrSet )
{
    return new AlignmentTabPage( pParent, rAttrSet );
}

USHORT* AlignmentTabPage::GetRanges()
{
    return s_pRanges;
}

void AlignmentTabPage::Reset( const SfxItemSet& rCoreAttrs )
{
    SfxTabPage::Reset( rCoreAttrs );
    UpdateEnableControls();
}

int AlignmentTabPage::DeactivatePage( SfxItemSet* pSet )
{
    if( pSet )
        FillItemSet( *pSet );
    return LEAVE_PAGE;
}

void AlignmentTabPage::DataChanged( const DataChangedEvent& rDCEvt )
{
    SfxTabPage::DataChanged( rDCEvt );
    if( (rDCEvt.GetType() == DATACHANGED_SETTINGS) && (rDCEvt.GetFlags() & SETTINGS_STYLE) )
    {
        svt::OLocalResourceAccess aLocalResAcc( SVX_RES( RID_SVXPAGE_ALIGNMENT ), RSC_TABPAGE );
        InitVsRefEgde();
    }
}

void AlignmentTabPage::InitVsRefEgde()
{
    // remember selection - is deleted in call to ValueSet::Clear()
    USHORT nSel = maVsRefEdge.GetSelectItemId();

    ResId aResId( GetBackground().GetColor().IsDark() ? IL_LOCK_BMPS_HC : IL_LOCK_BMPS );
    ImageList aImageList( aResId );
    Size aItemSize( aImageList.GetImage( IID_BOTTOMLOCK ).GetSizePixel() );

    maVsRefEdge.Clear();
    maVsRefEdge.SetStyle( maVsRefEdge.GetStyle() | WB_ITEMBORDER | WB_DOUBLEBORDER );

    maVsRefEdge.SetColCount( 3 );
    maVsRefEdge.InsertItem( IID_BOTTOMLOCK, aImageList.GetImage( IID_BOTTOMLOCK ),  String( ResId( STR_BOTTOMLOCK ) ) );
    maVsRefEdge.InsertItem( IID_TOPLOCK,    aImageList.GetImage( IID_TOPLOCK ),     String( ResId( STR_TOPLOCK ) ) );
    maVsRefEdge.InsertItem( IID_CELLLOCK,   aImageList.GetImage( IID_CELLLOCK ),    String( ResId( STR_CELLLOCK ) ) );

    maVsRefEdge.SetSizePixel( maVsRefEdge.CalcWindowSizePixel( aItemSize ) );

    maVsRefEdge.SelectItem( nSel );
}

void AlignmentTabPage::UpdateEnableControls()
{
    USHORT nHorAlign = maLbHorAlign.GetSelectEntryPos();
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


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

#include "align.hxx"

#include <editeng/svxenum.hxx>
#include <svx/dialogs.hrc>
#include <cuires.hrc>
#include "align.hrc"
#include <svx/rotmodit.hxx>

#include <svx/algitem.hxx>
#include <editeng/frmdiritem.hxx>
#include <editeng/justifyitem.hxx>
#include <dialmgr.hxx>
#include <svx/dlgutil.hxx>
#include <tools/shl.hxx>
#include <sfx2/app.hxx>
#include <sfx2/module.hxx>
#include <sfx2/itemconnect.hxx>
#include <svl/cjkoptions.hxx>
#include <svl/languageoptions.hxx>
#include <svtools/localresaccess.hxx>
#include <svx/flagsdef.hxx>
#include <svl/intitem.hxx>
#include <sfx2/request.hxx>

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
    { ALIGNDLG_HORALIGN_DISTRIBUTED, SVX_HOR_JUSTIFY_BLOCK  },
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
    { ALIGNDLG_VERALIGN_BLOCK,  SVX_VER_JUSTIFY_BLOCK       },
    { ALIGNDLG_VERALIGN_DISTRIBUTED, SVX_VER_JUSTIFY_BLOCK  },
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

namespace {

template<typename _JustContainerType, typename _JustEnumType>
void lcl_MaybeResetAlignToDistro(
    ListBox& rLB, sal_uInt16 nListPos, const SfxItemSet& rCoreAttrs, sal_uInt16 nWhichAlign, sal_uInt16 nWhichJM, _JustEnumType eBlock)
{
    const SfxPoolItem* pItem;
    if (rCoreAttrs.GetItemState(nWhichAlign, sal_True, &pItem) != SFX_ITEM_SET)
        // alignment not set.
        return;

    const SfxEnumItem* p = static_cast<const SfxEnumItem*>(pItem);
    _JustContainerType eVal = static_cast<_JustContainerType>(p->GetEnumValue());
    if (eVal != eBlock)
        // alignment is not 'justify'.  No need to go further.
        return;

    if (rCoreAttrs.GetItemState(nWhichJM, sal_True, &pItem) != SFX_ITEM_SET)
        // justification method is not set.
        return;

    p = static_cast<const SfxEnumItem*>(pItem);
    SvxCellJustifyMethod eMethod = static_cast<SvxCellJustifyMethod>(p->GetEnumValue());
    if (eMethod == SVX_JUSTIFY_METHOD_DISTRIBUTE)
        // Select the 'distribute' entry in the specified list box.
        rLB.SelectEntryPos(nListPos);
}

void lcl_SetJustifyMethodToItemSet(SfxItemSet& rSet, sal_uInt16 nWhichJM, const ListBox& rLB, sal_uInt16 nListPos)
{
    SvxCellJustifyMethod eJM = SVX_JUSTIFY_METHOD_AUTO;
    if (rLB.GetSelectEntryPos() == nListPos)
        eJM = SVX_JUSTIFY_METHOD_DISTRIBUTE;

    SvxJustifyMethodItem aItem(eJM, nWhichJM);
    rSet.Put(aItem);
}

}

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

    // This page needs ExchangeSupport.
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

sal_Bool AlignmentTabPage::FillItemSet( SfxItemSet& rSet )
{
    bool bChanged = SfxTabPage::FillItemSet(rSet);

    // Special treatment for distributed alignment; we need to set the justify
    // method to 'distribute' to distinguish from the normal justification.

    sal_uInt16 nWhichHorJM = GetWhich(SID_ATTR_ALIGN_HOR_JUSTIFY_METHOD);
    lcl_SetJustifyMethodToItemSet(rSet, nWhichHorJM, maLbHorAlign, ALIGNDLG_HORALIGN_DISTRIBUTED);
    if (!bChanged)
        bChanged = HasAlignmentChanged(rSet, nWhichHorJM);

    sal_uInt16 nWhichVerJM = GetWhich(SID_ATTR_ALIGN_VER_JUSTIFY_METHOD);
    lcl_SetJustifyMethodToItemSet(rSet, nWhichVerJM, maLbVerAlign, ALIGNDLG_VERALIGN_DISTRIBUTED);
    if (!bChanged)
        bChanged = HasAlignmentChanged(rSet, nWhichVerJM);

    return bChanged;
}

void AlignmentTabPage::Reset( const SfxItemSet& rCoreAttrs )
{
    SfxTabPage::Reset( rCoreAttrs );

    // Special treatment for distributed alignment; we need to set the justify
    // method to 'distribute' to distinguish from the normal justification.

    lcl_MaybeResetAlignToDistro<SvxCellHorJustify, SvxCellHorJustify>(
        maLbHorAlign, ALIGNDLG_HORALIGN_DISTRIBUTED, rCoreAttrs,
        GetWhich(SID_ATTR_ALIGN_HOR_JUSTIFY), GetWhich(SID_ATTR_ALIGN_HOR_JUSTIFY_METHOD),
        SVX_HOR_JUSTIFY_BLOCK);

    lcl_MaybeResetAlignToDistro<SvxCellVerJustify, SvxCellVerJustify>(
        maLbVerAlign, ALIGNDLG_VERALIGN_DISTRIBUTED, rCoreAttrs,
        GetWhich(SID_ATTR_ALIGN_VER_JUSTIFY), GetWhich(SID_ATTR_ALIGN_VER_JUSTIFY_METHOD),
        SVX_VER_JUSTIFY_BLOCK);

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

    ResId aResId( IL_LOCK_BMPS, CUI_MGR() );
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
    bool bHorDist  = (nHorAlign == ALIGNDLG_HORALIGN_DISTRIBUTED);

    // indent edit field only for left alignment
    maFtIndent.Enable( bHorLeft );
    maEdIndent.Enable( bHorLeft );

    // rotation/stacked disabled for fill alignment
    maOrientHlp.Enable( !bHorFill );

    // hyphenation only for automatic line breaks or for block alignment
    maBtnHyphen.Enable( maBtnWrap.IsChecked() || bHorBlock );

    // shrink only without automatic line break, and not for block, fill or distribute.
    maBtnShrink.Enable( (maBtnWrap.GetState() == STATE_NOCHECK) && !bHorBlock && !bHorFill && !bHorDist );

    // visibility of fixed lines
    maFlAlignment.Show( maLbHorAlign.IsVisible() || maEdIndent.IsVisible() || maLbVerAlign.IsVisible() );
    maFlOrient.Show( maCtrlDial.IsVisible() || maVsRefEdge.IsVisible() || maCbStacked.IsVisible() || maCbAsianMode.IsVisible() );
    maFlProperties.Show( maBtnWrap.IsVisible() || maBtnHyphen.IsVisible() || maBtnShrink.IsVisible() || maLbFrameDir.IsVisible() );
}

bool AlignmentTabPage::HasAlignmentChanged( const SfxItemSet& rNew, sal_uInt16 nWhich ) const
{
    const SfxItemSet& rOld = GetItemSet();
    const SfxPoolItem* pItem;
    SvxCellJustifyMethod eMethodOld = SVX_JUSTIFY_METHOD_AUTO;
    SvxCellJustifyMethod eMethodNew = SVX_JUSTIFY_METHOD_AUTO;
    if (rOld.GetItemState(nWhich, sal_True, &pItem) == SFX_ITEM_SET)
    {
        const SfxEnumItem* p = static_cast<const SfxEnumItem*>(pItem);
        eMethodOld = static_cast<SvxCellJustifyMethod>(p->GetEnumValue());
    }

    if (rNew.GetItemState(nWhich, sal_True, &pItem) == SFX_ITEM_SET)
    {
        const SfxEnumItem* p = static_cast<const SfxEnumItem*>(pItem);
        eMethodNew = static_cast<SvxCellJustifyMethod>(p->GetEnumValue());
    }

    return eMethodOld != eMethodNew;
}

IMPL_LINK_NOARG(AlignmentTabPage, UpdateEnableHdl)
{
    UpdateEnableControls();
    return 0;
}

// ============================================================================

} // namespace svx

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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
#include <strings.hrc>
#include <bitmaps.hlst>
#include <svx/rotmodit.hxx>

#include <svx/algitem.hxx>
#include <editeng/frmdiritem.hxx>
#include <editeng/justifyitem.hxx>
#include <dialmgr.hxx>
#include <svx/dlgutil.hxx>
#include <sfx2/app.hxx>
#include <sfx2/module.hxx>
#include <sfx2/itemconnect.hxx>
#include <svl/cjkoptions.hxx>
#include <svl/languageoptions.hxx>
#include <svx/flagsdef.hxx>
#include <svl/intitem.hxx>
#include <sfx2/request.hxx>
#include <vcl/settings.hxx>

#define IID_BOTTOMLOCK 1
#define IID_TOPLOCK    2
#define IID_CELLLOCK   3

namespace svx {

// item connections ===========================================================

// horizontal alignment -------------------------------------------------------

typedef sfx::ValueItemWrapper< SvxHorJustifyItem, SvxCellHorJustify > HorJustItemWrapper;
typedef sfx::ListBoxConnection< HorJustItemWrapper > HorJustConnection;

static const HorJustConnection::MapEntryType s_pHorJustMap[] =
{
    { ALIGNDLG_HORALIGN_STD,    SvxCellHorJustify::Standard    },
    { ALIGNDLG_HORALIGN_LEFT,   SvxCellHorJustify::Left        },
    { ALIGNDLG_HORALIGN_CENTER, SvxCellHorJustify::Center      },
    { ALIGNDLG_HORALIGN_RIGHT,  SvxCellHorJustify::Right       },
    { ALIGNDLG_HORALIGN_BLOCK,  SvxCellHorJustify::Block       },
    { ALIGNDLG_HORALIGN_FILL,   SvxCellHorJustify::Repeat      },
    { ALIGNDLG_HORALIGN_DISTRIBUTED, SvxCellHorJustify::Block  },
    { WRAPPER_LISTBOX_ENTRY_NOTFOUND,   SvxCellHorJustify::Standard    }
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
    { ALIGNDLG_VERALIGN_BLOCK,  SVX_VER_JUSTIFY_BLOCK       },
    { ALIGNDLG_VERALIGN_DISTRIBUTED, SVX_VER_JUSTIFY_BLOCK  },
    { WRAPPER_LISTBOX_ENTRY_NOTFOUND,   SVX_VER_JUSTIFY_STANDARD    }
};

// cell rotate mode -----------------------------------------------------------

typedef sfx::ValueItemWrapper< SvxRotateModeItem, SvxRotateMode > RotateModeItemWrapper;
typedef sfx::ValueSetConnection< RotateModeItemWrapper > RotateModeConnection;

static const RotateModeConnection::MapEntryType s_pRotateModeMap[] =
{
    { IID_BOTTOMLOCK,           SVX_ROTATE_MODE_BOTTOM      },
    { IID_TOPLOCK,              SVX_ROTATE_MODE_TOP         },
    { IID_CELLLOCK,             SVX_ROTATE_MODE_STANDARD    },
    { WRAPPER_VALUESET_ITEM_NOTFOUND,   SVX_ROTATE_MODE_STANDARD    }
};


const sal_uInt16 AlignmentTabPage::s_pRanges[] =
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


namespace {

template<typename JustContainerType, typename JustEnumType>
void lcl_MaybeResetAlignToDistro(
    ListBox& rLB, sal_uInt16 nListPos, const SfxItemSet& rCoreAttrs, sal_uInt16 nWhichAlign, sal_uInt16 nWhichJM, JustEnumType eBlock)
{
    const SfxPoolItem* pItem;
    if (rCoreAttrs.GetItemState(nWhichAlign, true, &pItem) != SfxItemState::SET)
        // alignment not set.
        return;

    const SfxEnumItemInterface* p = static_cast<const SfxEnumItemInterface*>(pItem);
    JustContainerType eVal = static_cast<JustContainerType>(p->GetEnumValue());
    if (eVal != eBlock)
        // alignment is not 'justify'.  No need to go further.
        return;

    if (rCoreAttrs.GetItemState(nWhichJM, true, &pItem) != SfxItemState::SET)
        // justification method is not set.
        return;

    p = static_cast<const SfxEnumItemInterface*>(pItem);
    SvxCellJustifyMethod eMethod = static_cast<SvxCellJustifyMethod>(p->GetEnumValue());
    if (eMethod == SvxCellJustifyMethod::Distribute)
        // Select the 'distribute' entry in the specified list box.
        rLB.SelectEntryPos(nListPos);
}

void lcl_SetJustifyMethodToItemSet(SfxItemSet& rSet, sal_uInt16 nWhichJM, const ListBox& rLB, sal_uInt16 nListPos)
{
    SvxCellJustifyMethod eJM = SvxCellJustifyMethod::Auto;
    if (rLB.GetSelectEntryPos() == nListPos)
        eJM = SvxCellJustifyMethod::Distribute;

    SvxJustifyMethodItem aItem(eJM, nWhichJM);
    rSet.Put(aItem);
}

}//namespace


AlignmentTabPage::AlignmentTabPage( vcl::Window* pParent, const SfxItemSet& rCoreAttrs ) :

    SfxTabPage( pParent, "CellAlignPage","cui/ui/cellalignment.ui", &rCoreAttrs )

{
    // text alignment
    get(m_pLbHorAlign,"comboboxHorzAlign");
    get(m_pFtIndent,"labelIndent");
    get(m_pEdIndent,"spinIndentFrom");
    get(m_pFtVerAlign,"labelVertAlign");
    get(m_pLbVerAlign,"comboboxVertAlign");

    //text rotation
    get(m_pNfRotate,"spinDegrees");
    get(m_pCtrlDial,"dialcontrol");
    get(m_pFtRotate,"labelDegrees");
    get(m_pFtRefEdge,"labelRefEdge");
    get(m_pVsRefEdge,"references");
    get(m_pBoxDirection,"boxDirection");

    //Asian mode
    get(m_pCbStacked,"checkVertStack");
    get(m_pCbAsianMode,"checkAsianMode");

    m_pOrientHlp = new OrientationHelper(*m_pCtrlDial, *m_pNfRotate, *m_pCbStacked);

    // Properties
    get(m_pBtnWrap,"checkWrapTextAuto");
    get(m_pBtnHyphen,"checkHyphActive");
    get(m_pBtnShrink,"checkShrinkFitCellSize");
    get(m_pLbFrameDir,"comboTextDirBox");

    //ValueSet hover strings
    get(m_pFtBotLock,"labelSTR_BOTTOMLOCK");
    get(m_pFtTopLock,"labelSTR_TOPLOCK");
    get(m_pFtCelLock,"labelSTR_CELLLOCK");
    get(m_pFtABCD,"labelABCD");

    get(m_pAlignmentFrame, "alignment");
    get(m_pOrientFrame, "orientation");
    get(m_pPropertiesFrame, "properties");

    m_pCtrlDial->SetText(m_pFtABCD->GetText());

    InitVsRefEgde();

    // windows to be disabled, if stacked text is turned ON
    m_pOrientHlp->AddDependentWindow( *m_pFtRotate,     TRISTATE_TRUE );
    m_pOrientHlp->AddDependentWindow( *m_pFtRefEdge,    TRISTATE_TRUE );
    m_pOrientHlp->AddDependentWindow( *m_pVsRefEdge,    TRISTATE_TRUE );
    // windows to be disabled, if stacked text is turned OFF
    m_pOrientHlp->AddDependentWindow( *m_pCbAsianMode,  TRISTATE_FALSE );

    Link<ListBox&,void> aLink = LINK( this, AlignmentTabPage, UpdateEnableHdl );

    m_pLbHorAlign->SetSelectHdl( aLink );
    m_pBtnWrap->SetClickHdl( LINK( this, AlignmentTabPage, UpdateEnableClickHdl ) );

    // Asian vertical mode
    m_pCbAsianMode->Show( SvtCJKOptions().IsVerticalTextEnabled() );

    m_pLbFrameDir->InsertEntryValue( CuiResId( RID_SVXSTR_FRAMEDIR_LTR ), SvxFrameDirection::Horizontal_LR_TB );
    m_pLbFrameDir->InsertEntryValue( CuiResId( RID_SVXSTR_FRAMEDIR_RTL ), SvxFrameDirection::Horizontal_RL_TB );
    m_pLbFrameDir->InsertEntryValue( CuiResId( RID_SVXSTR_FRAMEDIR_SUPER ), SvxFrameDirection::Environment );

    // This page needs ExchangeSupport.
    SetExchangeSupport();

    AddItemConnection( new HorJustConnection( SID_ATTR_ALIGN_HOR_JUSTIFY, *m_pLbHorAlign, s_pHorJustMap, ItemConnFlags::HideUnknown ) );
    AddItemConnection( new sfx::DummyItemConnection( SID_ATTR_ALIGN_INDENT, *m_pFtIndent, ItemConnFlags::HideUnknown ) );
    AddItemConnection( new sfx::MetricConnection<sfx::UInt16ItemWrapper>( SID_ATTR_ALIGN_INDENT, *m_pEdIndent, FUNIT_TWIP, ItemConnFlags::HideUnknown ) );
    AddItemConnection( new sfx::DummyItemConnection( SID_ATTR_ALIGN_VER_JUSTIFY, *m_pFtVerAlign, ItemConnFlags::HideUnknown ) );
    AddItemConnection( new VerJustConnection( SID_ATTR_ALIGN_VER_JUSTIFY, *m_pLbVerAlign, s_pVerJustMap, ItemConnFlags::HideUnknown ) );
    AddItemConnection( new DialControlConnection( SID_ATTR_ALIGN_DEGREES, *m_pCtrlDial, ItemConnFlags::HideUnknown ) );
    AddItemConnection( new sfx::DummyItemConnection( SID_ATTR_ALIGN_DEGREES, *m_pFtRotate, ItemConnFlags::HideUnknown ) );
    AddItemConnection( new sfx::DummyItemConnection( SID_ATTR_ALIGN_LOCKPOS, *m_pFtRefEdge, ItemConnFlags::HideUnknown ) );
    AddItemConnection( new RotateModeConnection( SID_ATTR_ALIGN_LOCKPOS, *m_pVsRefEdge, s_pRotateModeMap, ItemConnFlags::HideUnknown ) );
    AddItemConnection( new OrientStackedConnection( SID_ATTR_ALIGN_STACKED, *m_pOrientHlp ) );
    AddItemConnection( new sfx::DummyItemConnection( SID_ATTR_ALIGN_STACKED, *m_pCbStacked, ItemConnFlags::HideUnknown ) );
    AddItemConnection( new sfx::CheckBoxConnection( SID_ATTR_ALIGN_ASIANVERTICAL, *m_pCbAsianMode, ItemConnFlags::HideUnknown ) );
    AddItemConnection( new sfx::CheckBoxConnection( SID_ATTR_ALIGN_LINEBREAK, *m_pBtnWrap, ItemConnFlags::HideUnknown ) );
    AddItemConnection( new sfx::CheckBoxConnection( SID_ATTR_ALIGN_HYPHENATION, *m_pBtnHyphen, ItemConnFlags::HideUnknown ) );
    AddItemConnection( new sfx::CheckBoxConnection( SID_ATTR_ALIGN_SHRINKTOFIT, *m_pBtnShrink, ItemConnFlags::HideUnknown ) );
    AddItemConnection( new sfx::DummyItemConnection( SID_ATTR_FRAMEDIRECTION, *m_pBoxDirection, ItemConnFlags::HideUnknown ) );
    AddItemConnection( new FrameDirListBoxConnection( SID_ATTR_FRAMEDIRECTION, *m_pLbFrameDir, ItemConnFlags::HideUnknown ) );

}

AlignmentTabPage::~AlignmentTabPage()
{
    disposeOnce();
}

void AlignmentTabPage::dispose()
{
    delete m_pOrientHlp;
    m_pOrientHlp = nullptr;
    m_pLbHorAlign.clear();
    m_pFtIndent.clear();
    m_pEdIndent.clear();
    m_pFtVerAlign.clear();
    m_pLbVerAlign.clear();
    m_pCtrlDial.clear();
    m_pFtRotate.clear();
    m_pNfRotate.clear();
    m_pFtRefEdge.clear();
    m_pVsRefEdge.clear();
    m_pCbStacked.clear();
    m_pCbAsianMode.clear();
    m_pBoxDirection.clear();
    m_pBtnWrap.clear();
    m_pBtnHyphen.clear();
    m_pBtnShrink.clear();
    m_pLbFrameDir.clear();
    m_pFtBotLock.clear();
    m_pFtTopLock.clear();
    m_pFtCelLock.clear();
    m_pFtABCD.clear();
    m_pAlignmentFrame.clear();
    m_pOrientFrame.clear();
    m_pPropertiesFrame.clear();
    SfxTabPage::dispose();
}

VclPtr<SfxTabPage> AlignmentTabPage::Create( vcl::Window* pParent, const SfxItemSet* rAttrSet )
{
    return VclPtr<AlignmentTabPage>::Create( pParent, *rAttrSet );
}

bool AlignmentTabPage::FillItemSet( SfxItemSet* rSet )
{
    bool bChanged = SfxTabPage::FillItemSet(rSet);

    // Special treatment for distributed alignment; we need to set the justify
    // method to 'distribute' to distinguish from the normal justification.

    sal_uInt16 nWhichHorJM = GetWhich(SID_ATTR_ALIGN_HOR_JUSTIFY_METHOD);
    lcl_SetJustifyMethodToItemSet(*rSet, nWhichHorJM, *m_pLbHorAlign, ALIGNDLG_HORALIGN_DISTRIBUTED);
    if (!bChanged)
        bChanged = HasAlignmentChanged(*rSet, nWhichHorJM);

    sal_uInt16 nWhichVerJM = GetWhich(SID_ATTR_ALIGN_VER_JUSTIFY_METHOD);
    lcl_SetJustifyMethodToItemSet(*rSet, nWhichVerJM, *m_pLbVerAlign, ALIGNDLG_VERALIGN_DISTRIBUTED);
    if (!bChanged)
        bChanged = HasAlignmentChanged(*rSet, nWhichVerJM);

    return bChanged;
}

void AlignmentTabPage::Reset( const SfxItemSet* rCoreAttrs )
{
    SfxTabPage::Reset( rCoreAttrs );

    // Special treatment for distributed alignment; we need to set the justify
    // method to 'distribute' to distinguish from the normal justification.

    lcl_MaybeResetAlignToDistro<SvxCellHorJustify, SvxCellHorJustify>(
        *m_pLbHorAlign, ALIGNDLG_HORALIGN_DISTRIBUTED, *rCoreAttrs,
        GetWhich(SID_ATTR_ALIGN_HOR_JUSTIFY), GetWhich(SID_ATTR_ALIGN_HOR_JUSTIFY_METHOD),
        SvxCellHorJustify::Block);

    lcl_MaybeResetAlignToDistro<SvxCellVerJustify, SvxCellVerJustify>(
        *m_pLbVerAlign, ALIGNDLG_VERALIGN_DISTRIBUTED, *rCoreAttrs,
        GetWhich(SID_ATTR_ALIGN_VER_JUSTIFY), GetWhich(SID_ATTR_ALIGN_VER_JUSTIFY_METHOD),
        SVX_VER_JUSTIFY_BLOCK);

    UpdateEnableControls();
}

DeactivateRC AlignmentTabPage::DeactivatePage( SfxItemSet* _pSet )
{
    if( _pSet )
        FillItemSet( _pSet );
    return DeactivateRC::LeavePage;
}

void AlignmentTabPage::DataChanged( const DataChangedEvent& rDCEvt )
{
    SfxTabPage::DataChanged( rDCEvt );
    if( (rDCEvt.GetType() == DataChangedEventType::SETTINGS) && (rDCEvt.GetFlags() & AllSettingsFlags::STYLE) )
    {
        InitVsRefEgde();
    }
}

void AlignmentTabPage::InitVsRefEgde()
{
    // remember selection - is deleted in call to ValueSet::Clear()
    sal_uInt16 nSel = m_pVsRefEdge->GetSelectItemId();

    BitmapEx aBottomLock(RID_SVXBMP_BOTTOMLOCK);
    BitmapEx aTopLock(RID_SVXBMP_TOPLOCK);
    BitmapEx aCellLock(RID_SVXBMP_CELLLOCK);

    if( GetDPIScaleFactor() > 1 )
    {
        aBottomLock.Scale(GetDPIScaleFactor(), GetDPIScaleFactor(), BmpScaleFlag::Fast);
        aTopLock.Scale(GetDPIScaleFactor(), GetDPIScaleFactor(), BmpScaleFlag::Fast);
        aCellLock.Scale(GetDPIScaleFactor(), GetDPIScaleFactor(), BmpScaleFlag::Fast);
    }

    Size aItemSize(aBottomLock.GetSizePixel());

    m_pVsRefEdge->Clear();
    m_pVsRefEdge->SetStyle( m_pVsRefEdge->GetStyle() | WB_ITEMBORDER | WB_DOUBLEBORDER );

    m_pVsRefEdge->SetColCount( 3 );
    m_pVsRefEdge->InsertItem(IID_BOTTOMLOCK, Image(aBottomLock),  m_pFtBotLock->GetText());
    m_pVsRefEdge->InsertItem(IID_TOPLOCK,    Image(aTopLock),     m_pFtTopLock->GetText());
    m_pVsRefEdge->InsertItem(IID_CELLLOCK,   Image(aCellLock),    m_pFtCelLock->GetText());

    m_pVsRefEdge->SetSizePixel( m_pVsRefEdge->CalcWindowSizePixel( aItemSize ) );

    m_pVsRefEdge->SelectItem( nSel );
}

void AlignmentTabPage::UpdateEnableControls()
{
    const sal_Int32 nHorAlign = m_pLbHorAlign->GetSelectEntryPos();
    bool bHorLeft  = (nHorAlign == ALIGNDLG_HORALIGN_LEFT);
    bool bHorBlock = (nHorAlign == ALIGNDLG_HORALIGN_BLOCK);
    bool bHorFill  = (nHorAlign == ALIGNDLG_HORALIGN_FILL);
    bool bHorDist  = (nHorAlign == ALIGNDLG_HORALIGN_DISTRIBUTED);

    // indent edit field only for left alignment
    m_pFtIndent->Enable( bHorLeft );
    m_pEdIndent->Enable( bHorLeft );

    // rotation/stacked disabled for fill alignment
    m_pOrientHlp->Enable( !bHorFill );

    // hyphenation only for automatic line breaks or for block alignment
    m_pBtnHyphen->Enable( m_pBtnWrap->IsChecked() || bHorBlock );

    // shrink only without automatic line break, and not for block, fill or distribute.
    m_pBtnShrink->Enable( (m_pBtnWrap->GetState() == TRISTATE_FALSE) && !bHorBlock && !bHorFill && !bHorDist );

    // visibility of frames
    m_pAlignmentFrame->Show(m_pLbHorAlign->IsVisible() || m_pEdIndent->IsVisible() ||
        m_pLbVerAlign->IsVisible());
    m_pOrientFrame->Show(m_pCtrlDial->IsVisible() || m_pVsRefEdge->IsVisible() ||
        m_pCbStacked->IsVisible() || m_pCbAsianMode->IsVisible());
    m_pPropertiesFrame->Show(m_pBtnWrap->IsVisible() || m_pBtnHyphen->IsVisible() ||
        m_pBtnShrink->IsVisible() || m_pLbFrameDir->IsVisible());
}

bool AlignmentTabPage::HasAlignmentChanged( const SfxItemSet& rNew, sal_uInt16 nWhich ) const
{
    const SfxItemSet& rOld = GetItemSet();
    const SfxPoolItem* pItem;
    SvxCellJustifyMethod eMethodOld = SvxCellJustifyMethod::Auto;
    SvxCellJustifyMethod eMethodNew = SvxCellJustifyMethod::Auto;
    if (rOld.GetItemState(nWhich, true, &pItem) == SfxItemState::SET)
    {
        const SfxEnumItemInterface* p = static_cast<const SfxEnumItemInterface*>(pItem);
        eMethodOld = static_cast<SvxCellJustifyMethod>(p->GetEnumValue());
    }

    if (rNew.GetItemState(nWhich, true, &pItem) == SfxItemState::SET)
    {
        const SfxEnumItemInterface* p = static_cast<const SfxEnumItemInterface*>(pItem);
        eMethodNew = static_cast<SvxCellJustifyMethod>(p->GetEnumValue());
    }

    return eMethodOld != eMethodNew;
}

IMPL_LINK_NOARG(AlignmentTabPage, UpdateEnableClickHdl, Button*, void)
{
    UpdateEnableControls();
}

IMPL_LINK_NOARG(AlignmentTabPage, UpdateEnableHdl, ListBox&, void)
{
    UpdateEnableControls();
}


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

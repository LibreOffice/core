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

#include <comphelper/propertyvalue.hxx>
#include <o3tl/string_view.hxx>
#include <vcl/toolbox.hxx>
#include <svl/intitem.hxx>
#include <svl/itempool.hxx>
#include <svl/eitem.hxx>
#include <svl/whiter.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/request.hxx>
#include <sfx2/basedlgs.hxx>
#include <vcl/InterimItemWindow.hxx>
#include <sfx2/sfxdlg.hxx>
#include <tools/urlobj.hxx>

#include <svx/dialogs.hrc>
#include <svx/svxids.hrc>
#include <svx/strings.hrc>
#include <editeng/brushitem.hxx>
#include <editeng/sizeitem.hxx>
#include <svx/sdgcpitm.hxx>

#include <svx/dialmgr.hxx>
#include <svx/svdview.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdundo.hxx>
#include <svx/svdtrans.hxx>
#include <svx/grafctrl.hxx>
#include <svx/tbxcolor.hxx>
#include <sdgcoitm.hxx>
#include <svx/sdggaitm.hxx>
#include <svx/sdgluitm.hxx>
#include <svx/sdgmoitm.hxx>
#include <sdgtritm.hxx>
#include <bitmaps.hlst>

#include <com/sun/star/frame/XDispatchProvider.hpp>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::beans;

constexpr OUString TOOLBOX_NAME = u"colorbar"_ustr;
#define RID_SVXSTR_UNDO_GRAFCROP    RID_SVXSTR_GRAFCROP

namespace {

class ImplGrafControl final : public InterimItemWindow
{
private:
    OUString maCommand;
    Reference<XFrame> mxFrame;
    std::unique_ptr<weld::Image> mxImage;
    std::unique_ptr<weld::MetricSpinButton> mxField;

    DECL_LINK(ValueChangedHdl, weld::MetricSpinButton&, void);
    DECL_LINK(KeyInputHdl, const KeyEvent&, bool);
    void ImplModify();

public:
    ImplGrafControl( vcl::Window* pParent, const OUString& rCmd, const Reference< XFrame >& rFrame );
    virtual ~ImplGrafControl() override;
    virtual void            dispose() override;

    void Update( const SfxPoolItem* pItem );
    void set_field_text(const OUString& rStr) { mxField->set_text(rStr); }
    void set_sensitive(bool bSensitive)
    {
        Enable(bSensitive);
        mxImage->set_sensitive(bSensitive);
        mxField->set_sensitive(bSensitive);
    }
};

}

IMPL_LINK_NOARG(ImplGrafControl, ValueChangedHdl, weld::MetricSpinButton&, void)
{
    ImplModify();
}

void ImplGrafControl::ImplModify()
{
    const sal_Int64 nVal = mxField->get_value(FieldUnit::NONE);

    // Convert value to an any to be usable with dispatch API
    Any a;
    if ( maCommand == ".uno:GrafRed" ||
         maCommand == ".uno:GrafGreen" ||
         maCommand == ".uno:GrafBlue" ||
         maCommand == ".uno:GrafLuminance" ||
         maCommand == ".uno:GrafContrast" )
        a <<= sal_Int16( nVal );
    else if ( maCommand == ".uno:GrafGamma" ||
              maCommand == ".uno:GrafTransparence" )
        a <<= sal_Int32( nVal );

    if ( !a.hasValue() )
        return;

    INetURLObject aObj( maCommand );

    Sequence< PropertyValue > aArgs{ comphelper::makePropertyValue(aObj.GetURLPath(), a) };

    SfxToolBoxControl::Dispatch(
        Reference< XDispatchProvider >( mxFrame->getController(), UNO_QUERY ),
        maCommand,
        aArgs );
}

void ImplGrafControl::Update( const SfxPoolItem* pItem )
{
    if( pItem )
    {
        tools::Long nValue;

        if ( maCommand == ".uno:GrafTransparence" )
            nValue = static_cast<const SfxUInt16Item*>( pItem )->GetValue();
        else if ( maCommand == ".uno:GrafGamma" )
            nValue = static_cast<const SfxUInt32Item*>( pItem )->GetValue();
        else
            nValue = static_cast<const SfxInt16Item*>( pItem )->GetValue();

        mxField->set_value(nValue, FieldUnit::NONE);
    }
    else
        mxField->set_text(OUString());
}

namespace {

struct CommandToRID
{
    const char* pCommand;
    OUString sResId;
};

}

static OUString ImplGetRID( std::u16string_view aCommand )
{
    static constexpr OUString EMPTY = u""_ustr;
    static constexpr CommandToRID aImplCommandToResMap[] =
    {
        { ".uno:GrafRed",           RID_SVXBMP_GRAF_RED             },
        { ".uno:GrafGreen",         RID_SVXBMP_GRAF_GREEN           },
        { ".uno:GrafBlue",          RID_SVXBMP_GRAF_BLUE            },
        { ".uno:GrafLuminance",     RID_SVXBMP_GRAF_LUMINANCE       },
        { ".uno:GrafContrast",      RID_SVXBMP_GRAF_CONTRAST        },
        { ".uno:GrafGamma",         RID_SVXBMP_GRAF_GAMMA           },
        { ".uno:GrafTransparence",  RID_SVXBMP_GRAF_TRANSPARENCE    },
        { nullptr, EMPTY }
    };

    OUString sRID;

    sal_Int32 i( 0 );
    while ( aImplCommandToResMap[ i ].pCommand )
    {
        if ( o3tl::equalsAscii( aCommand, aImplCommandToResMap[ i ].pCommand ))
        {
            sRID = aImplCommandToResMap[i].sResId;
            break;
        }
        ++i;
    }

    return sRID;
}

ImplGrafControl::ImplGrafControl(
    vcl::Window* pParent,
    const OUString& rCmd,
    const Reference< XFrame >& rFrame)
    : InterimItemWindow(pParent, "svx/ui/grafctrlbox.ui", "GrafCtrlBox")
    , maCommand(rCmd)
    , mxFrame(rFrame)
    , mxImage(m_xBuilder->weld_image("image"))
    , mxField(m_xBuilder->weld_metric_spin_button("spinfield", FieldUnit::NONE))
{
    InitControlBase(&mxField->get_widget());

    OUString sResId(ImplGetRID(rCmd));
    mxImage->set_from_icon_name(sResId);
    mxImage->set_toolbar_background();

    SetBackground( Wallpaper() ); // transparent background

    mxField->set_help_id(rCmd);
    mxField->get_widget().connect_key_press(LINK(this, ImplGrafControl, KeyInputHdl));
    mxField->connect_value_changed(LINK(this, ImplGrafControl, ValueChangedHdl));

    if (maCommand == ".uno:GrafGamma")
    {
        mxField->set_digits(2);

        mxField->set_range(10, 1000, FieldUnit::NONE);
        mxField->set_increments(10, 100, FieldUnit::NONE);
    }
    else
    {
        const tools::Long nMinVal = maCommand == ".uno:GrafTransparence" ? 0 : -100;

        mxField->set_unit(FieldUnit::PERCENT);
        mxField->set_digits(0);

        mxField->set_range(nMinVal, 100, FieldUnit::PERCENT);
        mxField->set_increments(1, 10, FieldUnit::PERCENT);
    }

    SetSizePixel(m_xContainer->get_preferred_size());
}

IMPL_LINK(ImplGrafControl, KeyInputHdl, const KeyEvent&, rKEvt, bool)
{
    return ChildKeyInput(rKEvt);
}

ImplGrafControl::~ImplGrafControl()
{
    disposeOnce();
}

void ImplGrafControl::dispose()
{
    mxImage.reset();
    mxField.reset();
    InterimItemWindow::dispose();
}

namespace {

class ImplGrafModeControl final : public InterimItemWindow
{
private:
    sal_uInt16 mnCurPos;
    Reference< XFrame > mxFrame;
    std::unique_ptr<weld::ComboBox> m_xWidget;

    DECL_LINK(SelectHdl, weld::ComboBox&, void);
    DECL_LINK(KeyInputHdl, const KeyEvent&, bool);
    DECL_LINK(FocusInHdl, weld::Widget&, void);

    static void     ImplReleaseFocus();

public:
    ImplGrafModeControl( vcl::Window* pParent, const Reference< XFrame >& rFrame );
    virtual void dispose() override;
    virtual ~ImplGrafModeControl() override;

    void set_sensitive(bool bSensitive)
    {
        Enable(bSensitive);
        m_xWidget->set_sensitive(true);
    }

    void set_active(int nActive)
    {
        m_xWidget->set_active(nActive);
    }

    void            Update( const SfxPoolItem* pItem );
};

}

ImplGrafModeControl::ImplGrafModeControl(vcl::Window* pParent, const Reference<XFrame>& rFrame)
    : InterimItemWindow(pParent, "svx/ui/grafmodebox.ui", "GrafModeBox")
    , mnCurPos(0)
    , mxFrame(rFrame)
    , m_xWidget(m_xBuilder->weld_combo_box("grafmode"))
{
    InitControlBase(m_xWidget.get());

    m_xWidget->append_text( SvxResId( RID_SVXSTR_GRAFMODE_STANDARD  ) );
    m_xWidget->append_text( SvxResId( RID_SVXSTR_GRAFMODE_GREYS     ) );
    m_xWidget->append_text( SvxResId( RID_SVXSTR_GRAFMODE_MONO      ) );
    m_xWidget->append_text( SvxResId( RID_SVXSTR_GRAFMODE_WATERMARK ) );

    m_xWidget->connect_changed(LINK(this, ImplGrafModeControl, SelectHdl));
    m_xWidget->connect_key_press(LINK(this, ImplGrafModeControl, KeyInputHdl));
    m_xWidget->connect_focus_in(LINK(this, ImplGrafModeControl, FocusInHdl));

    SetSizePixel(m_xWidget->get_preferred_size());
}

void ImplGrafModeControl::dispose()
{
    m_xWidget.reset();
    InterimItemWindow::dispose();
}

ImplGrafModeControl::~ImplGrafModeControl()
{
    disposeOnce();
}

IMPL_LINK(ImplGrafModeControl, SelectHdl, weld::ComboBox&, rBox, void)
{
    Sequence< PropertyValue > aArgs{ comphelper::makePropertyValue("GrafMode",
                                                                   sal_Int16(rBox.get_active())) };

    /*  #i33380# DR 2004-09-03 Moved the following line above the Dispatch() call.
        This instance may be deleted in the meantime (i.e. when a dialog is opened
        while in Dispatch()), accessing members will crash in this case. */
    ImplReleaseFocus();

    SfxToolBoxControl::Dispatch(
        Reference< XDispatchProvider >( mxFrame->getController(), UNO_QUERY ),
        ".uno:GrafMode",
        aArgs );
}

IMPL_LINK(ImplGrafModeControl, KeyInputHdl, const KeyEvent&, rKEvt, bool)
{
    bool bHandled(false);

    if (rKEvt.GetKeyCode().GetCode() == KEY_ESCAPE)
    {
        m_xWidget->set_active(mnCurPos);
        ImplReleaseFocus();
        bHandled = true;
    }

    return bHandled || ChildKeyInput(rKEvt);
}

IMPL_LINK_NOARG(ImplGrafModeControl, FocusInHdl, weld::Widget&, void)
{
    mnCurPos = m_xWidget->get_active();
}

void ImplGrafModeControl::ImplReleaseFocus()
{
    if (const SfxViewShell* pViewShell = SfxViewShell::Current())
    {
        vcl::Window* pShellWnd = pViewShell->GetWindow();

        if( pShellWnd )
            pShellWnd->GrabFocus();
    }
}

void ImplGrafModeControl::Update( const SfxPoolItem* pItem )
{
    if( pItem )
        m_xWidget->set_active(static_cast<const SfxUInt16Item*>(pItem)->GetValue());
    else
        m_xWidget->set_active(-1);
}

SvxGrafToolBoxControl::SvxGrafToolBoxControl( sal_uInt16 nSlotId, ToolBoxItemId nId, ToolBox& rTbx) :
    SfxToolBoxControl( nSlotId, nId, rTbx )
{
    rTbx.SetItemBits( nId, ToolBoxItemBits::DROPDOWN | rTbx.GetItemBits( nId ) );
    rTbx.Invalidate();
}

SvxGrafToolBoxControl::~SvxGrafToolBoxControl()
{
}

void SvxGrafToolBoxControl::StateChangedAtToolBoxControl( sal_uInt16, SfxItemState eState, const SfxPoolItem* pState )
{
    ImplGrafControl* pCtrl = static_cast<ImplGrafControl*>( GetToolBox().GetItemWindow( GetId() ) );
    DBG_ASSERT( pCtrl, "Control not found" );

    if( eState == SfxItemState::DISABLED )
    {
        pCtrl->set_sensitive(false);
        pCtrl->set_field_text( OUString() );
    }
    else
    {
        pCtrl->set_sensitive(true);

        if( eState == SfxItemState::DEFAULT )
            pCtrl->Update( pState );
        else
            pCtrl->Update( nullptr );
    }
}

VclPtr<InterimItemWindow> SvxGrafToolBoxControl::CreateItemWindow( vcl::Window *pParent )
{
    return VclPtr<ImplGrafControl>::Create( pParent, m_aCommandURL, m_xFrame ).get();
}

SFX_IMPL_TOOLBOX_CONTROL( SvxGrafRedToolBoxControl, SfxInt16Item );

SvxGrafRedToolBoxControl::SvxGrafRedToolBoxControl( sal_uInt16 nSlotId, ToolBoxItemId nId, ToolBox& rTbx ) :
    SvxGrafToolBoxControl( nSlotId, nId, rTbx )
{
}

SFX_IMPL_TOOLBOX_CONTROL( SvxGrafGreenToolBoxControl, SfxInt16Item );

SvxGrafGreenToolBoxControl::SvxGrafGreenToolBoxControl( sal_uInt16 nSlotId, ToolBoxItemId nId, ToolBox& rTbx ) :
    SvxGrafToolBoxControl( nSlotId, nId, rTbx )
{
}

SFX_IMPL_TOOLBOX_CONTROL( SvxGrafBlueToolBoxControl, SfxInt16Item );

SvxGrafBlueToolBoxControl::SvxGrafBlueToolBoxControl( sal_uInt16 nSlotId, ToolBoxItemId nId, ToolBox& rTbx ) :
    SvxGrafToolBoxControl( nSlotId, nId, rTbx )
{
}

SFX_IMPL_TOOLBOX_CONTROL( SvxGrafLuminanceToolBoxControl, SfxInt16Item );

SvxGrafLuminanceToolBoxControl::SvxGrafLuminanceToolBoxControl( sal_uInt16 nSlotId, ToolBoxItemId nId, ToolBox& rTbx ) :
    SvxGrafToolBoxControl( nSlotId, nId, rTbx )
{
}

SFX_IMPL_TOOLBOX_CONTROL( SvxGrafContrastToolBoxControl, SfxInt16Item );

SvxGrafContrastToolBoxControl::SvxGrafContrastToolBoxControl( sal_uInt16 nSlotId, ToolBoxItemId nId, ToolBox& rTbx ) :
    SvxGrafToolBoxControl( nSlotId, nId, rTbx )
{
}

SFX_IMPL_TOOLBOX_CONTROL( SvxGrafGammaToolBoxControl, SfxUInt32Item );

SvxGrafGammaToolBoxControl::SvxGrafGammaToolBoxControl( sal_uInt16 nSlotId, ToolBoxItemId nId, ToolBox& rTbx ) :
    SvxGrafToolBoxControl( nSlotId, nId, rTbx )
{
}

SFX_IMPL_TOOLBOX_CONTROL( SvxGrafTransparenceToolBoxControl, SfxUInt16Item );

SvxGrafTransparenceToolBoxControl::SvxGrafTransparenceToolBoxControl( sal_uInt16 nSlotId, ToolBoxItemId nId, ToolBox& rTbx ) :
    SvxGrafToolBoxControl( nSlotId, nId, rTbx )
{
}

SFX_IMPL_TOOLBOX_CONTROL( SvxGrafModeToolBoxControl, SfxUInt16Item );

SvxGrafModeToolBoxControl::SvxGrafModeToolBoxControl( sal_uInt16 nSlotId, ToolBoxItemId nId, ToolBox& rTbx ) :
    SfxToolBoxControl( nSlotId, nId, rTbx )
{
}

SvxGrafModeToolBoxControl::~SvxGrafModeToolBoxControl()
{
}

void SvxGrafModeToolBoxControl::StateChangedAtToolBoxControl( sal_uInt16, SfxItemState eState, const SfxPoolItem* pState )

{
    ImplGrafModeControl* pCtrl = static_cast<ImplGrafModeControl*>( GetToolBox().GetItemWindow( GetId() ) );
    DBG_ASSERT( pCtrl, "Control not found" );

    if( eState == SfxItemState::DISABLED )
    {
        pCtrl->set_sensitive(false);
        pCtrl->set_active(-1);
    }
    else
    {
        pCtrl->set_sensitive(true);

        if( eState == SfxItemState::DEFAULT )
            pCtrl->Update( pState );
        else
            pCtrl->Update( nullptr );
    }
}

VclPtr<InterimItemWindow> SvxGrafModeToolBoxControl::CreateItemWindow( vcl::Window *pParent )
{
    return VclPtr<ImplGrafModeControl>::Create( pParent, m_xFrame ).get();
}

void SvxGrafAttrHelper::ExecuteGrafAttr( SfxRequest& rReq, SdrView& rView )
{
    SfxItemPool&    rPool = rView.GetModel().GetItemPool();
    SfxItemSetFixed<SDRATTR_GRAF_FIRST, SDRATTR_GRAF_LAST> aSet( rPool );
    OUString        aUndoStr;
    const bool      bUndo = rView.IsUndoEnabled();

    if( bUndo )
    {
        aUndoStr = rView.GetDescriptionOfMarkedObjects() + " ";
    }

    const SfxItemSet*   pArgs = rReq.GetArgs();
    const SfxPoolItem*  pItem;
    sal_uInt16              nSlot = rReq.GetSlot();

    if( !pArgs || SfxItemState::SET != pArgs->GetItemState( nSlot, false, &pItem ))
        pItem = nullptr;

    switch( nSlot )
    {
        case SID_ATTR_GRAF_RED:
        {
            if( pItem )
            {
                aSet.Put( SdrGrafRedItem( static_cast<const SfxInt16Item*>(pItem)->GetValue() ));
                if( bUndo )
                    aUndoStr += SvxResId( RID_SVXSTR_UNDO_GRAFRED );
            }
        }
        break;

        case SID_ATTR_GRAF_GREEN:
        {
            if( pItem )
            {
                aSet.Put( SdrGrafGreenItem( static_cast<const SfxInt16Item*>(pItem)->GetValue() ));
                if( bUndo )
                    aUndoStr += SvxResId( RID_SVXSTR_UNDO_GRAFGREEN );
            }
        }
        break;

        case SID_ATTR_GRAF_BLUE:
        {
            if( pItem )
            {
                aSet.Put( SdrGrafBlueItem( static_cast<const SfxInt16Item*>(pItem)->GetValue() ));
                if( bUndo )
                    aUndoStr += SvxResId( RID_SVXSTR_UNDO_GRAFBLUE );
            }
        }
        break;

        case SID_ATTR_GRAF_LUMINANCE:
        {
            if( pItem )
            {
                aSet.Put( SdrGrafLuminanceItem( static_cast<const SfxInt16Item*>(pItem)->GetValue() ));
                if( bUndo )
                    aUndoStr += SvxResId( RID_SVXSTR_UNDO_GRAFLUMINANCE );
            }
        }
        break;

        case SID_ATTR_GRAF_CONTRAST:
        {
            if( pItem )
            {
                aSet.Put( SdrGrafContrastItem( static_cast<const SfxInt16Item*>(pItem)->GetValue() ));
                if( bUndo )
                    aUndoStr += SvxResId( RID_SVXSTR_UNDO_GRAFCONTRAST );
            }
        }
        break;

        case SID_ATTR_GRAF_GAMMA:
        {
            if( pItem )
            {
                aSet.Put( SdrGrafGamma100Item( static_cast<const SfxUInt32Item*>(pItem)->GetValue() ));
                if( bUndo )
                    aUndoStr += SvxResId( RID_SVXSTR_UNDO_GRAFGAMMA );
            }
        }
        break;

        case SID_ATTR_GRAF_TRANSPARENCE:
        {
            if( pItem )
            {
                aSet.Put( SdrGrafTransparenceItem( static_cast<const SfxUInt16Item*>(pItem)->GetValue() ));
                if( bUndo )
                    aUndoStr += SvxResId( RID_SVXSTR_UNDO_GRAFTRANSPARENCY );
            }
        }
        break;

        case SID_ATTR_GRAF_MODE:
        {
            if( pItem )
            {
                aSet.Put( SdrGrafModeItem( static_cast<GraphicDrawMode>(static_cast<const SfxUInt16Item*>(pItem)->GetValue()) ));
                if( bUndo )
                    aUndoStr += SvxResId( RID_SVXSTR_UNDO_GRAFMODE );
            }
        }
        break;

        case SID_ATTR_GRAF_CROP:
        {
            const SdrMarkList& rMarkList = rView.GetMarkedObjectList();

            if( 0 < rMarkList.GetMarkCount() )
            {
                SdrGrafObj* pObj = static_cast<SdrGrafObj*>( rMarkList.GetMark( 0 )->GetMarkedSdrObj() );

                if( ( pObj->GetGraphicType() != GraphicType::NONE ) &&
                    ( pObj->GetGraphicType() != GraphicType::Default ) )
                {
                    SfxItemSetFixed<SDRATTR_GRAFCROP, SDRATTR_GRAFCROP> aGrfAttr( rPool );
                    const MapUnit       eOldMetric = rPool.GetMetric( 0 );

                    aGrfAttr.Put(pObj->GetMergedItemSet());
                    rPool.SetDefaultMetric( MapUnit::MapTwip );

                    SfxItemSetFixed<
                            SDRATTR_GRAFCROP, SDRATTR_GRAFCROP,
                            SID_ATTR_PAGE_SIZE, SID_ATTR_PAGE_SIZE,
                            SID_ATTR_GRAF_CROP, SID_ATTR_GRAF_FRMSIZE,
                            SID_ATTR_GRAF_GRAPHIC, SID_ATTR_GRAF_GRAPHIC>
                        aCropDlgAttr(rPool);

                    aCropDlgAttr.Put( SvxBrushItem( pObj->GetGraphic(), GPOS_MM, SID_ATTR_GRAF_GRAPHIC ) );
                    aCropDlgAttr.Put( SvxSizeItem( SID_ATTR_PAGE_SIZE,
                            o3tl::convert(Size(200000, 200000), o3tl::Length::mm100, o3tl::Length::twip)));
                    aCropDlgAttr.Put( SvxSizeItem( SID_ATTR_GRAF_FRMSIZE,
                            o3tl::convert(pObj->GetLogicRect().GetSize(), o3tl::Length::mm100, o3tl::Length::twip)));

                    const SdrGrafCropItem&  rCrop = aGrfAttr.Get( SDRATTR_GRAFCROP );
                    Size aLTSize = o3tl::convert(Size(rCrop.GetLeft(), rCrop.GetTop()), o3tl::Length::mm100, o3tl::Length::twip);
                    Size aRBSize = o3tl::convert(Size(rCrop.GetRight(), rCrop.GetBottom()), o3tl::Length::mm100, o3tl::Length::twip);

                    aCropDlgAttr.Put( SdrGrafCropItem( aLTSize.Width(), aLTSize.Height(),
                                                    aRBSize.Width(), aRBSize.Height() ) );

                    vcl::Window* pParent(SfxViewShell::Current() ? SfxViewShell::Current()->GetWindow() : nullptr);
                    SfxSingleTabDialogController aCropDialog(pParent ? pParent->GetFrameWeld() : nullptr,
                        &aCropDlgAttr);
                    const OUString aCropStr(SvxResId(RID_SVXSTR_GRAFCROP));

                    SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();
                    ::CreateTabPage fnCreatePage = pFact->GetTabPageCreatorFunc( RID_SVXPAGE_GRFCROP );
                    std::unique_ptr<SfxTabPage> xTabPage = (*fnCreatePage)(aCropDialog.get_content_area(), &aCropDialog, &aCropDlgAttr);
                    sal_Int32 nPreferredDPI = rView.getSdrModelFromSdrView().getImagePreferredDPI();
                    xTabPage->getAdditionalProperties().emplace("PreferredDPI", css::uno::Any(nPreferredDPI));
                    xTabPage->SetPageTitle(aCropStr);
                    aCropDialog.SetTabPage(std::move(xTabPage));

                    if (aCropDialog.run() == RET_OK)
                    {
                        const SfxItemSet* pOutAttr = aCropDialog.GetOutputItemSet();

                        if( pOutAttr )
                        {
                            aUndoStr += SvxResId( RID_SVXSTR_UNDO_GRAFCROP );

                            // set crop attributes
                            if( SfxItemState::SET <= pOutAttr->GetItemState( SDRATTR_GRAFCROP ) )
                            {
                                const SdrGrafCropItem& rNewCrop = pOutAttr->Get( SDRATTR_GRAFCROP );

                                aLTSize = o3tl::convert(Size(rNewCrop.GetLeft(), rNewCrop.GetTop()), o3tl::Length::twip, o3tl::Length::mm100);
                                aRBSize = o3tl::convert(Size(rNewCrop.GetRight(), rNewCrop.GetBottom()), o3tl::Length::twip, o3tl::Length::mm100);
                                aSet.Put( SdrGrafCropItem( aLTSize.Width(), aLTSize.Height(), aRBSize.Width(), aRBSize.Height() ) );
                            }

                            // set new logic rect
                            if( SfxItemState::SET <= pOutAttr->GetItemState( SID_ATTR_GRAF_FRMSIZE ) )
                            {
                                Point       aNewOrigin( pObj->GetLogicRect().TopLeft() );
                                const Size& rGrfSize = pOutAttr->Get( SID_ATTR_GRAF_FRMSIZE ).GetSize();
                                Size aNewGrfSize = o3tl::convert(rGrfSize, o3tl::Length::twip, o3tl::Length::mm100);
                                Size        aOldGrfSize( pObj->GetLogicRect().GetSize() );

                                tools::Rectangle aNewRect( aNewOrigin, aNewGrfSize );
                                Point aOffset( (aNewGrfSize.Width() - aOldGrfSize.Width()) >> 1,
                                            (aNewGrfSize.Height() - aOldGrfSize.Height()) >> 1 );

                                // #106181# rotate snap rect before setting it
                                const GeoStat& aGeo = pObj->GetGeoStat();

                                if (aGeo.m_nRotationAngle || aGeo.m_nShearAngle)
                                {
                                    tools::Polygon aPol(aNewRect);

                                    // also transform origin offset
                                    if (aGeo.m_nShearAngle)
                                    {
                                        ShearPoly(aPol,
                                                aNewRect.TopLeft(),
                                                aGeo.mfTanShearAngle);
                                        ShearPoint(aOffset, Point(0,0), aGeo.mfTanShearAngle);
                                    }
                                    if (aGeo.m_nRotationAngle)
                                    {
                                        RotatePoly(aPol,
                                                aNewRect.TopLeft(),
                                                aGeo.mfSinRotationAngle,aGeo.mfCosRotationAngle);
                                        RotatePoint(aOffset, Point(0,0), aGeo.mfSinRotationAngle,aGeo.mfCosRotationAngle);
                                    }

                                    // apply offset
                                    aPol.Move( -aOffset.X(), -aOffset.Y() );
                                    aNewRect=aPol.GetBoundRect();
                                }
                                else
                                {
                                    aNewRect.Move( -aOffset.X(), -aOffset.Y() );
                                }

                                if( !aSet.Count() )
                                    rView.SetMarkedObjRect( aNewRect );
                                else
                                {
                                    if( bUndo )
                                    {
                                        rView.BegUndo( aUndoStr );
                                        rView.AddUndo(rView.GetModel().GetSdrUndoFactory().CreateUndoGeoObject(*pObj));
                                    }
                                    pObj->SetSnapRect( aNewRect );
                                    rView.SetAttributes( aSet );

                                    if( bUndo )
                                        rView.EndUndo();
                                    aSet.ClearItem();
                                }
                            }
                        }
                    }

                    rPool.SetDefaultMetric( eOldMetric );
                }
            }
        }
        break;

        case SID_COLOR_SETTINGS:
        {
            svx::ToolboxAccess aToolboxAccess( TOOLBOX_NAME );
            aToolboxAccess.toggleToolbox();
            rReq.Done();
            break;
        }

        default:
            break;
    }

    if( aSet.Count() )
    {
        if( bUndo )
            rView.BegUndo( aUndoStr );

        rView.SetAttributes( aSet );

        if( bUndo )
            rView.EndUndo();
    }
}

void SvxGrafAttrHelper::GetGrafAttrState( SfxItemSet& rSet, SdrView const & rView )
{
    SfxItemPool&    rPool = rView.GetModel().GetItemPool();
    SfxItemSet      aAttrSet( rPool );
    SfxWhichIter    aIter( rSet );
    sal_uInt16      nWhich = aIter.FirstWhich();
    const           SdrMarkList& rMarkList = rView.GetMarkedObjectList();
    bool            bEnableColors = true;
    bool            bEnableTransparency = true;
    bool            bEnableCrop = ( 1 == rMarkList.GetMarkCount() );

    for( size_t i = 0, nCount = rMarkList.GetMarkCount(); i < nCount; ++i )
    {
        SdrGrafObj* pGrafObj = dynamic_cast< SdrGrafObj* >( rMarkList.GetMark( i )->GetMarkedSdrObj() );

        if( !pGrafObj ||
            ( pGrafObj->GetGraphicType() == GraphicType::NONE ) ||
            ( pGrafObj->GetGraphicType() == GraphicType::Default  ))
        {
            bEnableColors = bEnableTransparency = bEnableCrop = false;
            break;
        }
        else if( bEnableTransparency && ( pGrafObj->HasGDIMetaFile() || pGrafObj->IsAnimated() ) )
        {
            bEnableTransparency = false;
        }
    }

    rView.GetAttributes( aAttrSet );

    while( nWhich )
    {
        sal_uInt16 nSlotId = SfxItemPool::IsWhich( nWhich ) ? rPool.GetSlotId( nWhich ) : nWhich;

        switch( nSlotId )
        {
            case SID_ATTR_GRAF_MODE:
            {
                if( SfxItemState::DEFAULT <= aAttrSet.GetItemState( SDRATTR_GRAFMODE ) )
                {
                    if( bEnableColors )
                    {
                        rSet.Put( SfxUInt16Item( nSlotId,
                            sal::static_int_cast< sal_uInt16 >( aAttrSet.Get(SDRATTR_GRAFMODE).GetValue() ) ) );
                    }
                    else
                    {
                        rSet.DisableItem( SID_ATTR_GRAF_MODE );
                    }
                }
            }
            break;

            case SID_ATTR_GRAF_RED:
            {
                if( SfxItemState::DEFAULT <= aAttrSet.GetItemState( SDRATTR_GRAFRED ) )
                {
                    if( bEnableColors )
                    {
                        rSet.Put( SfxInt16Item( nSlotId, aAttrSet.Get(SDRATTR_GRAFRED).GetValue() ) );
                    }
                    else
                    {
                        rSet.DisableItem( SID_ATTR_GRAF_RED );
                    }
                }
            }
            break;

            case SID_ATTR_GRAF_GREEN:
            {
                if( SfxItemState::DEFAULT <= aAttrSet.GetItemState( SDRATTR_GRAFGREEN ) )
                {
                    if( bEnableColors )
                    {
                        rSet.Put( SfxInt16Item( nSlotId, aAttrSet.Get(SDRATTR_GRAFGREEN).GetValue()) );
                    }
                    else
                    {
                        rSet.DisableItem( SID_ATTR_GRAF_GREEN );
                    }
                }
            }
            break;

            case SID_ATTR_GRAF_BLUE:
            {
                if( SfxItemState::DEFAULT <= aAttrSet.GetItemState( SDRATTR_GRAFBLUE ) )
                {
                    if( bEnableColors )
                    {
                        rSet.Put( SfxInt16Item( nSlotId, aAttrSet.Get(SDRATTR_GRAFBLUE).GetValue()) );
                    }
                    else
                    {
                        rSet.DisableItem( SID_ATTR_GRAF_BLUE );
                    }
                }
            }
            break;

            case SID_ATTR_GRAF_LUMINANCE:
            {
                if( SfxItemState::DEFAULT <= aAttrSet.GetItemState( SDRATTR_GRAFLUMINANCE ) )
                {
                    if( bEnableColors )
                    {
                        rSet.Put( SfxInt16Item( nSlotId, aAttrSet.Get(SDRATTR_GRAFLUMINANCE).GetValue()) );
                    }
                    else
                    {
                        rSet.DisableItem( SID_ATTR_GRAF_LUMINANCE );
                    }
                }
            }
            break;

            case SID_ATTR_GRAF_CONTRAST:
            {
                if( SfxItemState::DEFAULT <= aAttrSet.GetItemState( SDRATTR_GRAFCONTRAST ) )
                {
                    if( bEnableColors )
                    {
                        rSet.Put( SfxInt16Item( nSlotId,
                            aAttrSet.Get(SDRATTR_GRAFCONTRAST).GetValue()) );
                    }
                    else
                    {
                        rSet.DisableItem( SID_ATTR_GRAF_CONTRAST );
                    }
                }
            }
            break;

            case SID_ATTR_GRAF_GAMMA:
            {
                if( SfxItemState::DEFAULT <= aAttrSet.GetItemState( SDRATTR_GRAFGAMMA ) )
                {
                    if( bEnableColors )
                    {
                        rSet.Put( SfxUInt32Item( nSlotId,
                            aAttrSet.Get(SDRATTR_GRAFGAMMA).GetValue() ) );
                    }
                    else
                    {
                        rSet.DisableItem( SID_ATTR_GRAF_GAMMA );
                    }
                }
            }
            break;

            case SID_ATTR_GRAF_TRANSPARENCE:
            {
                if( SfxItemState::DEFAULT <= aAttrSet.GetItemState( SDRATTR_GRAFTRANSPARENCE ) )
                {
                    if( bEnableTransparency )
                    {
                        rSet.Put( SfxUInt16Item( nSlotId,
                            aAttrSet.Get(SDRATTR_GRAFTRANSPARENCE).GetValue() ) );
                    }
                    else
                    {
                        rSet.DisableItem( SID_ATTR_GRAF_TRANSPARENCE );
                    }
                }
            }
            break;

            case SID_ATTR_GRAF_CROP:
            {
                if( !bEnableCrop )
                    rSet.DisableItem( nSlotId );
            }
            break;

            case SID_COLOR_SETTINGS :
            {
                svx::ToolboxAccess aToolboxAccess( TOOLBOX_NAME );
                rSet.Put( SfxBoolItem( nWhich, aToolboxAccess.isToolboxVisible() ) );
                break;
            }

            default:
            break;
        }

        nWhich = aIter.NextWhich();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

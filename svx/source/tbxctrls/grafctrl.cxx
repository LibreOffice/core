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

#include <string>

#include <vcl/toolbox.hxx>
#include <vcl/field.hxx>
#include <vcl/fixed.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/idle.hxx>
#include <svl/intitem.hxx>
#include <svl/eitem.hxx>
#include <svl/whiter.hxx>
#include <sfx2/app.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/request.hxx>
#include <sfx2/basedlgs.hxx>
#include <tools/urlobj.hxx>
#include <comphelper/processfactory.hxx>

#include <svx/svxids.hrc>
#include <svx/dialogs.hrc>
#include <editeng/brushitem.hxx>
#include <editeng/sizeitem.hxx>
#include <svx/sdgcpitm.hxx>

#include <svx/itemwin.hxx>
#include <svx/dialmgr.hxx>
#include <svx/svdview.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdundo.hxx>
#include <svx/svdtrans.hxx>
#include "svx/grafctrl.hxx"
#include "svx/tbxcolor.hxx"

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;

#include <svx/svxdlg.hxx>

#define SYMBOL_TO_FIELD_OFFSET      4
#define ITEMVALUE(ItemSet,Id,Cast)  static_cast<const Cast&>((ItemSet).Get(Id)).GetValue()
#define TOOLBOX_NAME                "colorbar"


class ImplGrafMetricField : public MetricField
{
    using Window::Update;

private:
    Idle                maIdle;
    OUString            maCommand;
    Reference< XFrame > mxFrame;

                    DECL_LINK_TYPED(ImplModifyHdl, Idle *, void);

protected:

    virtual void    Modify() override;

public:
                    ImplGrafMetricField( vcl::Window* pParent, const OUString& aCmd, const Reference< XFrame >& rFrame );

    void            Update( const SfxPoolItem* pItem );
};

ImplGrafMetricField::ImplGrafMetricField( vcl::Window* pParent, const OUString& rCmd, const Reference< XFrame >& rFrame ) :
    MetricField( pParent, WB_BORDER | WB_SPIN | WB_REPEAT | WB_3DLOOK ),
    maCommand( rCmd ),
    mxFrame( rFrame )
{
    Size aSize( GetTextWidth( "-100 %" ), GetTextHeight() );

    aSize.Width() += 20;
    aSize.Height() += 6;
    SetSizePixel( aSize );

    if ( maCommand == ".uno:GrafGamma" )
    {
        SetDecimalDigits( 2 );

        SetMin( 10 );
        SetFirst( 10 );
        SetMax( 1000 );
        SetLast( 1000 );
        SetSpinSize( 10 );
    }
    else
    {
        const long nMinVal = maCommand == ".uno:GrafTransparence" ? 0 : -100;

        SetUnit(FUNIT_PERCENT);
        SetDecimalDigits( 0 );

        SetMin( nMinVal );
        SetFirst( nMinVal );
        SetMax( 100 );
        SetLast( 100 );
        SetSpinSize( 1 );
    }

    maIdle.SetIdleHdl( LINK( this, ImplGrafMetricField, ImplModifyHdl ) );
}

void ImplGrafMetricField::Modify()
{
    maIdle.Start();
}

IMPL_LINK_NOARG_TYPED(ImplGrafMetricField, ImplModifyHdl, Idle *, void)
{
    const sal_Int64 nVal = GetValue();

    // Convert value to an any to be usable with dispatch API
    Any a;
    if ( maCommand == ".uno:GrafRed" ||
         maCommand == ".uno:GrafGreen" ||
         maCommand == ".uno:GrafBlue" ||
         maCommand == ".uno:GrafLuminance" ||
         maCommand == ".uno:GrafContrast" )
        a = makeAny( sal_Int16( nVal ));
    else if ( maCommand == ".uno:GrafGamma" ||
              maCommand == ".uno:GrafTransparence" )
        a = makeAny( sal_Int32( nVal ));

    if ( a.hasValue() )
    {
        INetURLObject aObj( maCommand );

        Sequence< PropertyValue > aArgs( 1 );
        aArgs[0].Name = aObj.GetURLPath();
        aArgs[0].Value = a;

        SfxToolBoxControl::Dispatch(
            Reference< XDispatchProvider >( mxFrame->getController(), UNO_QUERY ),
            maCommand,
            aArgs );
    }
}

void ImplGrafMetricField::Update( const SfxPoolItem* pItem )
{
    if( pItem )
    {
        long nValue;

        if ( maCommand == ".uno:GrafTransparence" )
            nValue = static_cast<const SfxUInt16Item*>( pItem )->GetValue();
        else if ( maCommand == ".uno:GrafGamma" )
            nValue = static_cast<const SfxUInt32Item*>( pItem )->GetValue();
        else
            nValue = static_cast<const SfxInt16Item*>( pItem )->GetValue();

        SetValue( nValue );
    }
    else
        SetText( OUString() );
}

struct CommandToRID
{
    const char* pCommand;
    sal_uInt16      nResId;
};

static sal_uInt16 ImplGetRID( const OUString& aCommand )
{
    static const CommandToRID aImplCommandToResMap[] =
    {
        { ".uno:GrafRed",           RID_SVXIMG_GRAF_RED             },
        { ".uno:GrafGreen",         RID_SVXIMG_GRAF_GREEN           },
        { ".uno:GrafBlue",          RID_SVXIMG_GRAF_BLUE            },
        { ".uno:GrafLuminance",     RID_SVXIMG_GRAF_LUMINANCE       },
        { ".uno:GrafContrast",      RID_SVXIMG_GRAF_CONTRAST        },
        { ".uno:GrafGamma",         RID_SVXIMG_GRAF_GAMMA           },
        { ".uno:GrafTransparence",  RID_SVXIMG_GRAF_TRANSPARENCE    },
        { nullptr, 0 }
    };

    sal_uInt16 nRID = 0;

    sal_Int32 i( 0 );
    while ( aImplCommandToResMap[ i ].pCommand )
    {
        if ( aCommand.equalsAscii( aImplCommandToResMap[ i ].pCommand ))
        {
            nRID = aImplCommandToResMap[ i ].nResId;
            break;
        }
        ++i;
    }

    return nRID;
}

class ImplGrafControl : public Control
{
    using Window::Update;
private:
    VclPtr<FixedImage>          maImage;
    VclPtr<ImplGrafMetricField> maField;

protected:

    virtual void            GetFocus() override;

public:

                            ImplGrafControl( vcl::Window* pParent, const OUString& rCmd, const Reference< XFrame >& rFrame );
                            virtual ~ImplGrafControl() override;
    virtual void            dispose() override;

    void                    Update( const SfxPoolItem* pItem ) { maField->Update( pItem ); }
    void                    SetText( const OUString& rStr ) override { maField->SetText( rStr ); }
    virtual void            Resize() override;
};

ImplGrafControl::ImplGrafControl(
    vcl::Window* pParent,
    const OUString& rCmd,
    const Reference< XFrame >& rFrame
)   : Control( pParent, WB_TABSTOP )
    , maImage( VclPtr<FixedImage>::Create(this) )
    , maField( VclPtr<ImplGrafMetricField>::Create(this, rCmd, rFrame) )
{
    ResId   aResId( ImplGetRID( rCmd ), DIALOG_MGR() ) ;
    Image   aImage( aResId );

    Size    aImgSize( aImage.GetSizePixel() );
    Size    aFldSize( maField->GetSizePixel() );
    long    nFldY, nImgY;

    maImage->SetImage( aImage );
    maImage->SetSizePixel( aImgSize );
    // we want to see the background of the toolbox, not of the FixedImage or Control
    maImage->SetBackground( Wallpaper( COL_TRANSPARENT ) );
    SetBackground( Wallpaper( COL_TRANSPARENT ) );

    if( aImgSize.Height() > aFldSize.Height() )
    {
        nImgY = 0;
        nFldY = ( aImgSize.Height() - aFldSize.Height() ) >> 1;
    }
    else
    {
        nFldY = 0;
        nImgY = ( aFldSize.Height() - aImgSize.Height() ) >> 1;
    }

    long nOffset = SYMBOL_TO_FIELD_OFFSET / 2;
    maImage->SetPosPixel( Point( nOffset, nImgY ) );
    maField->SetPosPixel( Point( aImgSize.Width() + SYMBOL_TO_FIELD_OFFSET, nFldY ) );
    SetSizePixel( Size( aImgSize.Width() + aFldSize.Width() + SYMBOL_TO_FIELD_OFFSET + nOffset,
                  std::max( aImgSize.Height(), aFldSize.Height() ) ) );

    SetBackground( Wallpaper() ); // transparent background

    maImage->Show();

    maField->SetHelpId( OUStringToOString( rCmd, RTL_TEXTENCODING_UTF8 ) );
    maField->Show();
}

ImplGrafControl::~ImplGrafControl()
{
    disposeOnce();
}

void ImplGrafControl::dispose()
{
    maImage.disposeAndClear();
    maField.disposeAndClear();
    Control::dispose();
}

void ImplGrafControl::GetFocus()
{
    if (maField)
        maField->GrabFocus();
}

void ImplGrafControl::Resize()
{
    Size aFldSize(maField->GetSizePixel());
    aFldSize.Width() = GetSizePixel().Width() - SYMBOL_TO_FIELD_OFFSET - maImage->GetSizePixel().Width();
    maField->SetSizePixel(aFldSize);

    Control::Resize();
}

class ImplGrafModeControl : public ListBox
{
    using Window::Update;
private:
    sal_uInt16              mnCurPos;
    Reference< XFrame > mxFrame;

    virtual void    Select() override;
    virtual bool    PreNotify( NotifyEvent& rNEvt ) override;
    virtual bool    Notify( NotifyEvent& rNEvt ) override;
    static void     ImplReleaseFocus();

public:
                    ImplGrafModeControl( vcl::Window* pParent, const Reference< XFrame >& rFrame );

    void            Update( const SfxPoolItem* pItem );
};

ImplGrafModeControl::ImplGrafModeControl( vcl::Window* pParent, const Reference< XFrame >& rFrame ) :
    ListBox( pParent, WB_BORDER | WB_DROPDOWN | WB_AUTOHSCROLL ),
    mnCurPos( 0 ),
    mxFrame( rFrame )
{
    SetSizePixel( Size( 100, 260 ) );

    InsertEntry( SVX_RESSTR( RID_SVXSTR_GRAFMODE_STANDARD  ) );
    InsertEntry( SVX_RESSTR( RID_SVXSTR_GRAFMODE_GREYS     ) );
    InsertEntry( SVX_RESSTR( RID_SVXSTR_GRAFMODE_MONO      ) );
    InsertEntry( SVX_RESSTR( RID_SVXSTR_GRAFMODE_WATERMARK ) );

    Show();
}

void ImplGrafModeControl::Select()
{
    if ( !IsTravelSelect() )
    {
        Sequence< PropertyValue > aArgs( 1 );
        aArgs[0].Name = "GrafMode";
        aArgs[0].Value = makeAny( sal_Int16( GetSelectEntryPos() ));

        /*  #i33380# DR 2004-09-03 Moved the following line above the Dispatch() call.
            This instance may be deleted in the meantime (i.e. when a dialog is opened
            while in Dispatch()), accessing members will crash in this case. */
        ImplReleaseFocus();

        SfxToolBoxControl::Dispatch(
            Reference< XDispatchProvider >( mxFrame->getController(), UNO_QUERY ),
            ".uno:GrafMode",
            aArgs );
    }
}

bool ImplGrafModeControl::PreNotify( NotifyEvent& rNEvt )
{
    MouseNotifyEvent nType = rNEvt.GetType();

    if( MouseNotifyEvent::MOUSEBUTTONDOWN == nType || MouseNotifyEvent::GETFOCUS == nType )
        mnCurPos = GetSelectEntryPos();

    return ListBox::PreNotify( rNEvt );
}

bool ImplGrafModeControl::Notify( NotifyEvent& rNEvt )
{
    bool bHandled = ListBox::Notify( rNEvt );

    if( rNEvt.GetType() == MouseNotifyEvent::KEYINPUT )
    {
        const KeyEvent* pKEvt = rNEvt.GetKeyEvent();

        switch( pKEvt->GetKeyCode().GetCode() )
        {
            case KEY_RETURN:
            {
                Select();
                bHandled = true;
            }
            break;

            case KEY_ESCAPE:
            {
                SelectEntryPos( mnCurPos );
                ImplReleaseFocus();
                bHandled = true;
            }
            break;
        }
    }

    return bHandled;
}

void ImplGrafModeControl::ImplReleaseFocus()
{
    if( SfxViewShell::Current() )
    {
        vcl::Window* pShellWnd = SfxViewShell::Current()->GetWindow();

        if( pShellWnd )
            pShellWnd->GrabFocus();
    }
}

void ImplGrafModeControl::Update( const SfxPoolItem* pItem )
{
    if( pItem )
        SelectEntryPos( static_cast<const SfxUInt16Item*>(pItem)->GetValue() );
    else
        SetNoSelection();
}

SvxGrafToolBoxControl::SvxGrafToolBoxControl( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx) :
    SfxToolBoxControl( nSlotId, nId, rTbx )
{
    rTbx.SetItemBits( nId, ToolBoxItemBits::DROPDOWN | rTbx.GetItemBits( nId ) );
    rTbx.Invalidate();
}

SvxGrafToolBoxControl::~SvxGrafToolBoxControl()
{
}

void SvxGrafToolBoxControl::StateChanged( sal_uInt16, SfxItemState eState, const SfxPoolItem* pState )

{
    ImplGrafControl* pCtrl = static_cast<ImplGrafControl*>( GetToolBox().GetItemWindow( GetId() ) );
    DBG_ASSERT( pCtrl, "Control not found" );

    if( eState == SfxItemState::DISABLED )
    {
        pCtrl->Disable();
        pCtrl->SetText( OUString() );
    }
    else
    {
        pCtrl->Enable();

        if( eState == SfxItemState::DEFAULT )
            pCtrl->Update( pState );
        else
            pCtrl->Update( nullptr );
    }
}

VclPtr<vcl::Window> SvxGrafToolBoxControl::CreateItemWindow( vcl::Window *pParent )
{
    return VclPtr<ImplGrafControl>::Create( pParent, m_aCommandURL, m_xFrame ).get();
}

SFX_IMPL_TOOLBOX_CONTROL( SvxGrafRedToolBoxControl, SfxInt16Item );

SvxGrafRedToolBoxControl::SvxGrafRedToolBoxControl( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx ) :
    SvxGrafToolBoxControl( nSlotId, nId, rTbx )
{
}

SFX_IMPL_TOOLBOX_CONTROL( SvxGrafGreenToolBoxControl, SfxInt16Item );

SvxGrafGreenToolBoxControl::SvxGrafGreenToolBoxControl( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx ) :
    SvxGrafToolBoxControl( nSlotId, nId, rTbx )
{
}

SFX_IMPL_TOOLBOX_CONTROL( SvxGrafBlueToolBoxControl, SfxInt16Item );

SvxGrafBlueToolBoxControl::SvxGrafBlueToolBoxControl( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx ) :
    SvxGrafToolBoxControl( nSlotId, nId, rTbx )
{
}

SFX_IMPL_TOOLBOX_CONTROL( SvxGrafLuminanceToolBoxControl, SfxInt16Item );

SvxGrafLuminanceToolBoxControl::SvxGrafLuminanceToolBoxControl( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx ) :
    SvxGrafToolBoxControl( nSlotId, nId, rTbx )
{
}

SFX_IMPL_TOOLBOX_CONTROL( SvxGrafContrastToolBoxControl, SfxInt16Item );

SvxGrafContrastToolBoxControl::SvxGrafContrastToolBoxControl( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx ) :
    SvxGrafToolBoxControl( nSlotId, nId, rTbx )
{
}

SFX_IMPL_TOOLBOX_CONTROL( SvxGrafGammaToolBoxControl, SfxUInt32Item );

SvxGrafGammaToolBoxControl::SvxGrafGammaToolBoxControl( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx ) :
    SvxGrafToolBoxControl( nSlotId, nId, rTbx )
{
}

SFX_IMPL_TOOLBOX_CONTROL( SvxGrafTransparenceToolBoxControl, SfxUInt16Item );

SvxGrafTransparenceToolBoxControl::SvxGrafTransparenceToolBoxControl( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx ) :
    SvxGrafToolBoxControl( nSlotId, nId, rTbx )
{
}

SFX_IMPL_TOOLBOX_CONTROL( SvxGrafModeToolBoxControl, SfxUInt16Item );

SvxGrafModeToolBoxControl::SvxGrafModeToolBoxControl( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx ) :
    SfxToolBoxControl( nSlotId, nId, rTbx )
{
}

SvxGrafModeToolBoxControl::~SvxGrafModeToolBoxControl()
{
}

void SvxGrafModeToolBoxControl::StateChanged( sal_uInt16, SfxItemState eState, const SfxPoolItem* pState )

{
    ImplGrafModeControl* pCtrl = static_cast<ImplGrafModeControl*>( GetToolBox().GetItemWindow( GetId() ) );
    DBG_ASSERT( pCtrl, "Control not found" );

    if( eState == SfxItemState::DISABLED )
    {
        pCtrl->Disable();
        pCtrl->SetText( OUString() );
    }
    else
    {
        pCtrl->Enable();

        if( eState == SfxItemState::DEFAULT )
            pCtrl->Update( pState );
        else
            pCtrl->Update( nullptr );
    }
}

VclPtr<vcl::Window> SvxGrafModeToolBoxControl::CreateItemWindow( vcl::Window *pParent )
{
    return VclPtr<ImplGrafModeControl>::Create( pParent, m_xFrame ).get();
}

void SvxGrafAttrHelper::ExecuteGrafAttr( SfxRequest& rReq, SdrView& rView )
{
    SfxItemPool&    rPool = rView.GetModel()->GetItemPool();
    SfxItemSet      aSet( rPool, SDRATTR_GRAF_FIRST, SDRATTR_GRAF_LAST );
    OUString        aUndoStr;
    const bool      bUndo = rView.IsUndoEnabled();

    if( bUndo )
    {
        aUndoStr = rView.GetDescriptionOfMarkedObjects();
        aUndoStr += " ";
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
                    aUndoStr += SVX_RESSTR( RID_SVXSTR_UNDO_GRAFRED );
            }
        }
        break;

        case SID_ATTR_GRAF_GREEN:
        {
            if( pItem )
            {
                aSet.Put( SdrGrafGreenItem( static_cast<const SfxInt16Item*>(pItem)->GetValue() ));
                if( bUndo )
                    aUndoStr += SVX_RESSTR( RID_SVXSTR_UNDO_GRAFGREEN );
            }
        }
        break;

        case SID_ATTR_GRAF_BLUE:
        {
            if( pItem )
            {
                aSet.Put( SdrGrafBlueItem( static_cast<const SfxInt16Item*>(pItem)->GetValue() ));
                if( bUndo )
                    aUndoStr += SVX_RESSTR( RID_SVXSTR_UNDO_GRAFBLUE );
            }
        }
        break;

        case SID_ATTR_GRAF_LUMINANCE:
        {
            if( pItem )
            {
                aSet.Put( SdrGrafLuminanceItem( static_cast<const SfxInt16Item*>(pItem)->GetValue() ));
                if( bUndo )
                    aUndoStr += SVX_RESSTR( RID_SVXSTR_UNDO_GRAFLUMINANCE );
            }
        }
        break;

        case SID_ATTR_GRAF_CONTRAST:
        {
            if( pItem )
            {
                aSet.Put( SdrGrafContrastItem( static_cast<const SfxInt16Item*>(pItem)->GetValue() ));
                if( bUndo )
                    aUndoStr += SVX_RESSTR( RID_SVXSTR_UNDO_GRAFCONTRAST );
            }
        }
        break;

        case SID_ATTR_GRAF_GAMMA:
        {
            if( pItem )
            {
                aSet.Put( SdrGrafGamma100Item( static_cast<const SfxUInt32Item*>(pItem)->GetValue() ));
                if( bUndo )
                    aUndoStr += SVX_RESSTR( RID_SVXSTR_UNDO_GRAFGAMMA );
            }
        }
        break;

        case SID_ATTR_GRAF_TRANSPARENCE:
        {
            if( pItem )
            {
                aSet.Put( SdrGrafTransparenceItem( static_cast<const SfxUInt16Item*>(pItem)->GetValue() ));
                if( bUndo )
                    aUndoStr += SVX_RESSTR( RID_SVXSTR_UNDO_GRAFTRANSPARENCY );
            }
        }
        break;

        case SID_ATTR_GRAF_MODE:
        {
            if( pItem )
            {
                aSet.Put( SdrGrafModeItem( (GraphicDrawMode) static_cast<const SfxUInt16Item*>(pItem)->GetValue() ));
                if( bUndo )
                    aUndoStr += SVX_RESSTR( RID_SVXSTR_UNDO_GRAFMODE );
            }
        }
        break;

        case SID_ATTR_GRAF_CROP:
        {
            const SdrMarkList& rMarkList = rView.GetMarkedObjectList();

            if( 0 < rMarkList.GetMarkCount() )
            {
                SdrGrafObj* pObj = static_cast<SdrGrafObj*>( rMarkList.GetMark( 0 )->GetMarkedSdrObj() );

                if( pObj && dynamic_cast<const SdrGrafObj*>( pObj) !=  nullptr &&
                    ( pObj->GetGraphicType() != GraphicType::NONE ) &&
                    ( pObj->GetGraphicType() != GraphicType::Default ) )
                {
                    SfxItemSet          aGrfAttr( rPool, SDRATTR_GRAFCROP, SDRATTR_GRAFCROP, 0 );
                    const MapUnit       eOldMetric = rPool.GetMetric( 0 );
                    const MapMode       aMap100( MAP_100TH_MM );
                    const MapMode       aMapTwip( MAP_TWIP );

                    aGrfAttr.Put(pObj->GetMergedItemSet());
                    rPool.SetDefaultMetric( MAP_TWIP );

                    SfxItemSet  aCropDlgAttr( rPool,
                                            SDRATTR_GRAFCROP, SDRATTR_GRAFCROP,
                                            SID_ATTR_GRAF_GRAPHIC, SID_ATTR_GRAF_GRAPHIC,
                                            SID_ATTR_PAGE_SIZE, SID_ATTR_PAGE_SIZE,
                                            SID_ATTR_GRAF_FRMSIZE, SID_ATTR_GRAF_FRMSIZE,
                                            SID_ATTR_GRAF_CROP, SID_ATTR_GRAF_CROP, 0 );

                    aCropDlgAttr.Put( SvxBrushItem( pObj->GetGraphic(), GPOS_MM, SID_ATTR_GRAF_GRAPHIC ) );
                    aCropDlgAttr.Put( SvxSizeItem( SID_ATTR_PAGE_SIZE,
                                                Size( OutputDevice::LogicToLogic(
                                                        Size( 200000, 200000 ), aMap100, aMapTwip ) ) ) );
                    aCropDlgAttr.Put( SvxSizeItem( SID_ATTR_GRAF_FRMSIZE, OutputDevice::LogicToLogic(
                                                pObj->GetLogicRect().GetSize(), aMap100, aMapTwip ) ) );

                    const SdrGrafCropItem&  rCrop = static_cast<const SdrGrafCropItem&>( aGrfAttr.Get( SDRATTR_GRAFCROP ) );
                    Size                    aLTSize( OutputDevice::LogicToLogic(
                                                    Size( rCrop.GetLeft(), rCrop.GetTop() ), aMap100, aMapTwip ) );
                    Size                    aRBSize( OutputDevice::LogicToLogic(
                                                    Size( rCrop.GetRight(), rCrop.GetBottom() ), aMap100, aMapTwip ) );

                    aCropDlgAttr.Put( SdrGrafCropItem( aLTSize.Width(), aLTSize.Height(),
                                                    aRBSize.Width(), aRBSize.Height() ) );

                    ScopedVclPtrInstance<SfxSingleTabDialog> aCropDialog(
                        SfxViewShell::Current() ? SfxViewShell::Current()->GetWindow() : nullptr,
                        aCropDlgAttr);
                    const OUString aCropStr(SVX_RESSTR(RID_SVXSTR_GRAFCROP));

                    SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();
                    assert(pFact && "Dialog creation failed!");
                    ::CreateTabPage fnCreatePage = pFact->GetTabPageCreatorFunc( RID_SVXPAGE_GRFCROP );
                    assert(fnCreatePage && "Dialog creation failed!");
                    SfxTabPage* pTabPage = (*fnCreatePage)( aCropDialog->get_content_area(), &aCropDlgAttr );

                    pTabPage->SetText( aCropStr );
                    aCropDialog->SetTabPage( pTabPage );

                    if( aCropDialog->Execute() == RET_OK )
                    {
                        const SfxItemSet* pOutAttr = aCropDialog->GetOutputItemSet();

                        if( pOutAttr )
                        {
                            aUndoStr += SVX_RESSTR( RID_SVXSTR_UNDO_GRAFCROP );

                            // set crop attributes
                            if( SfxItemState::SET <= pOutAttr->GetItemState( SDRATTR_GRAFCROP ) )
                            {
                                const SdrGrafCropItem& rNewCrop = static_cast<const SdrGrafCropItem&>( pOutAttr->Get( SDRATTR_GRAFCROP ) );

                                aLTSize = OutputDevice::LogicToLogic( Size( rNewCrop.GetLeft(), rNewCrop.GetTop() ), aMapTwip, aMap100 );
                                aRBSize = OutputDevice::LogicToLogic( Size( rNewCrop.GetRight(), rNewCrop.GetBottom() ), aMapTwip, aMap100 );
                                aSet.Put( SdrGrafCropItem( aLTSize.Width(), aLTSize.Height(), aRBSize.Width(), aRBSize.Height() ) );
                            }

                            // set new logic rect
                            if( SfxItemState::SET <= pOutAttr->GetItemState( SID_ATTR_GRAF_FRMSIZE ) )
                            {
                                Point       aNewOrigin( pObj->GetLogicRect().TopLeft() );
                                const Size& rGrfSize = static_cast<const SvxSizeItem&>( pOutAttr->Get( SID_ATTR_GRAF_FRMSIZE ) ).GetSize();
                                Size        aNewGrfSize( OutputDevice::LogicToLogic( rGrfSize, aMapTwip, aMap100 ) );
                                Size        aOldGrfSize( pObj->GetLogicRect().GetSize() );

                                Rectangle aNewRect( aNewOrigin, aNewGrfSize );
                                Point aOffset( (aNewGrfSize.Width() - aOldGrfSize.Width()) >> 1,
                                            (aNewGrfSize.Height() - aOldGrfSize.Height()) >> 1 );

                                // #106181# rotate snap rect before setting it
                                const GeoStat& aGeo = pObj->GetGeoStat();

                                if (aGeo.nRotationAngle!=0 || aGeo.nShearAngle!=0)
                                {
                                    tools::Polygon aPol(aNewRect);

                                    // also transform origin offset
                                    if (aGeo.nShearAngle!=0)
                                    {
                                        ShearPoly(aPol,
                                                aNewRect.TopLeft(),
                                                aGeo.nTan);
                                        ShearPoint(aOffset, Point(0,0), aGeo.nTan);
                                    }
                                    if (aGeo.nRotationAngle!=0)
                                    {
                                        RotatePoly(aPol,
                                                aNewRect.TopLeft(),
                                                aGeo.nSin,aGeo.nCos);
                                        RotatePoint(aOffset, Point(0,0), aGeo.nSin,aGeo.nCos);
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
                                        rView.AddUndo( rView.GetModel()->GetSdrUndoFactory().CreateUndoGeoObject( *pObj ) );
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

void SvxGrafAttrHelper::GetGrafAttrState( SfxItemSet& rSet, SdrView& rView )
{
    SfxItemPool&    rPool = rView.GetModel()->GetItemPool();
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
                            sal::static_int_cast< sal_uInt16 >( ITEMVALUE( aAttrSet, SDRATTR_GRAFMODE, SdrGrafModeItem ) ) ) );
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
                        rSet.Put( SfxInt16Item( nSlotId,
                            ITEMVALUE( aAttrSet, SDRATTR_GRAFRED, SdrGrafRedItem ) ) );
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
                        rSet.Put( SfxInt16Item( nSlotId,
                            ITEMVALUE( aAttrSet, SDRATTR_GRAFGREEN, SdrGrafGreenItem ) ) );
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
                        rSet.Put( SfxInt16Item( nSlotId,
                            ITEMVALUE( aAttrSet, SDRATTR_GRAFBLUE, SdrGrafBlueItem ) ) );
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
                        rSet.Put( SfxInt16Item( nSlotId,
                            ITEMVALUE( aAttrSet, SDRATTR_GRAFLUMINANCE, SdrGrafLuminanceItem ) ) );
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
                            ITEMVALUE( aAttrSet, SDRATTR_GRAFCONTRAST, SdrGrafContrastItem ) ) );
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
                            ITEMVALUE( aAttrSet, SDRATTR_GRAFGAMMA, SdrGrafGamma100Item ) ) );
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
                            ITEMVALUE( aAttrSet, SDRATTR_GRAFTRANSPARENCE, SdrGrafTransparenceItem ) ) );
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

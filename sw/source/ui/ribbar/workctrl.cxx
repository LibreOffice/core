/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#include "precompiled_sw.hxx"


#include <string>
#include <svl/eitem.hxx>
#include <svx/htmlmode.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/imagemgr.hxx>
#include <swmodule.hxx>
#include <view.hxx>
#include <initui.hxx>
#include <viewopt.hxx>
#include <docsh.hxx>
#include <gloshdl.hxx>
#include <glosdoc.hxx>
#include <gloslst.hxx>
#include <workctrl.hxx>
#include <workctrl.hrc>
#include <cmdid.h>
#include <helpid.h>
#include <wrtsh.hxx>
#include <svl/imageitm.hxx>
#include <vcl/lstbox.hxx>
#include <rtl/ustring.hxx>
#include "swabstdlg.hxx"
#include <misc.hrc>

#include <vcl/svapp.hxx>

// Size Abpruefung
#define NAVI_ENTRIES 20
#if NAVI_ENTRIES != NID_COUNT
#error SwScrollNaviPopup-CTOR static Array falsche Size. Wurden neue IDs zugefuegt ??
#endif

using ::rtl::OUString;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::frame;

SFX_IMPL_TOOLBOX_CONTROL( SwTbxInsertCtrl, SfxImageItem);
SFX_IMPL_TOOLBOX_CONTROL( SwTbxAutoTextCtrl, SfxBoolItem );

SwTbxInsertCtrl::SwTbxInsertCtrl(
    USHORT nSlotId,
    USHORT nId,
    ToolBox& rTbx ) :
        SfxToolBoxControl( nSlotId, nId, rTbx ),
        nLastSlotId(FN_INSERT_CTRL == nSlotId ? FN_INSERT_TABLE : SID_INSERT_DIAGRAM)
{
    rTbx.SetItemBits( nId, TIB_DROPDOWN | rTbx.GetItemBits( nId ) );
}

SwTbxInsertCtrl::~SwTbxInsertCtrl()
{
}

void SAL_CALL SwTbxInsertCtrl::update() throw (uno::RuntimeException)
{
    ToolBox& rTbx = GetToolBox();
    rtl::OUString aSlotURL( RTL_CONSTASCII_USTRINGPARAM( "slot:" ));
    aSlotURL += rtl::OUString::valueOf( sal_Int32( nLastSlotId ));
    Image aImage = GetImage( m_xFrame, aSlotURL, hasBigImages() );

    rTbx.SetItemImage(GetId(), aImage);
    rTbx.Invalidate();

    SfxToolBoxControl::update();
}

void SwTbxInsertCtrl::StateChanged( USHORT /*nSID*/,
                                      SfxItemState eState,
                                      const SfxPoolItem* pState )
{
    USHORT nId = GetId();
    GetToolBox().EnableItem( nId, (GetItemState(pState) != SFX_ITEM_DISABLED) );

    if( eState == SFX_ITEM_AVAILABLE )
    {
        const SfxImageItem* pItem = PTR_CAST( SfxImageItem, pState );
        if(pItem)
        {
            nLastSlotId = pItem->GetValue();
            if( nLastSlotId )
                nId = nLastSlotId;

            rtl::OUString aSlotURL( RTL_CONSTASCII_USTRINGPARAM( "slot:" ));
            aSlotURL += rtl::OUString::valueOf( sal_Int32( nId ));
            ToolBox& rBox = GetToolBox();
            Image aImage = GetImage( m_xFrame, aSlotURL, hasBigImages() );
            rBox.SetItemImage(GetId(), aImage);
            rBox.SetItemImageMirrorMode( GetId(), FALSE );
            rBox.SetItemImageAngle( GetId(), pItem->GetRotation() );
            rBox.SetItemImageMirrorMode( GetId(), pItem->IsMirrored() );
        }
    }

}

SfxPopupWindow* SwTbxInsertCtrl::CreatePopupWindow()
{
    if(GetSlotId() == FN_INSERT_CTRL)
    {
        OUString aToolBarResStr( RTL_CONSTASCII_USTRINGPARAM( "private:resource/toolbar/insertbar" ));
        createAndPositionSubToolBar( aToolBarResStr );
    }
    else /* FN_INSERT_OBJ_CTRL */
    {
        OUString aToolBarResStr( RTL_CONSTASCII_USTRINGPARAM( "private:resource/toolbar/insertobjectbar" ));
        createAndPositionSubToolBar( aToolBarResStr );
    }
    return NULL;
}

SfxPopupWindowType  SwTbxInsertCtrl::GetPopupWindowType() const
{
    return nLastSlotId ? SFX_POPUPWINDOW_ONTIMEOUT : SFX_POPUPWINDOW_ONCLICK;
}


void SwTbxInsertCtrl::Select( BOOL /*bMod1*/ )
{
    if( nLastSlotId )
    {
        SfxViewShell*   pCurSh( SfxViewShell::Current() );
        SfxDispatcher*  pDispatch( 0 );

        if ( pCurSh )
        {
            SfxViewFrame*   pViewFrame = pCurSh->GetViewFrame();
            if ( pViewFrame )
                pDispatch = pViewFrame->GetDispatcher();
        }

        if ( pDispatch )
            pDispatch->Execute(nLastSlotId);
    }
}

SwTbxAutoTextCtrl::SwTbxAutoTextCtrl(
    USHORT nSlotId,
    USHORT nId,
    ToolBox& rTbx ) :
    SfxToolBoxControl( nSlotId, nId, rTbx ),
    pPopup(0),
    pView(0)
{
    rTbx.SetItemBits( nId, TIB_DROPDOWN | rTbx.GetItemBits( nId ) );
}

SwTbxAutoTextCtrl::~SwTbxAutoTextCtrl()
{
    DelPopup();
}

SfxPopupWindow* SwTbxAutoTextCtrl::CreatePopupWindow()
{
    pView = ::GetActiveView();
    if(pView && !pView->GetDocShell()->IsReadOnly() &&
       !pView->GetWrtShell().HasReadonlySel() )
    {
        ToolBox& rBox = GetToolBox();

        Rectangle aItemRect( rBox.GetItemRect( GetId() ) );
        Point aPt(rBox.OutputToScreenPixel(aItemRect.TopLeft()));
        aPt.X() += aItemRect.GetWidth()/2;
        aPt.Y() += aItemRect.GetHeight()/2;
        if(pView)
        {
            Link aLnk = LINK(this, SwTbxAutoTextCtrl, PopupHdl);

            if (GetSlotId() == FN_INSERT_FIELD_CTRL)
            {
                pPopup = new PopupMenu(SW_RES(RID_INSERT_FIELD_CTRL));
                pPopup->SetSelectHdl(aLnk);

                if (::GetHtmlMode(pView->GetDocShell()) & HTMLMODE_ON)
                {
                    pPopup->RemoveItem(pPopup->GetItemPos(FN_INSERT_FLD_PGCOUNT));
                    pPopup->RemoveItem(pPopup->GetItemPos(FN_INSERT_FLD_TOPIC));
                }
            }
            else
            {
                pPopup = new PopupMenu;
                SwGlossaryList* pGlossaryList = ::GetGlossaryList();
                USHORT nGroupCount = pGlossaryList->GetGroupCount();
                for(USHORT i = 1; i <= nGroupCount; i++)
                {
                    // Gruppenname mit Pfad-Extension besorgen
                    String sTitle;
                    String sGroupName = pGlossaryList->GetGroupName(i - 1, FALSE, &sTitle);
                    USHORT nBlockCount = pGlossaryList->GetBlockCount(i -1);
                    if(nBlockCount)
                    {
                        USHORT nIndex = 100 * (i);
                        // aber ohne extension einfuegen
                        pPopup->InsertItem( i, sTitle);//sGroupName.GetToken(0, GLOS_DELIM));
                        PopupMenu* pSub = new PopupMenu;
                        pSub->SetSelectHdl(aLnk);
                        pPopup->SetPopupMenu(i, pSub);
                        for(USHORT j = 0; j < nBlockCount; j++)
                        {
                            String sEntry;
                            String sLongName(pGlossaryList->GetBlockName(i - 1, j, sEntry));
                            sEntry.AppendAscii(" - ");
                            sEntry += sLongName;
                            pSub->InsertItem(++nIndex, sEntry);
                        }
                    }
                }
            }
        }
        ToolBox* pToolBox = &GetToolBox();
        USHORT nId = GetId();
        pToolBox->SetItemDown( nId, TRUE );

        pPopup->Execute( pToolBox, pToolBox->GetItemRect( nId ),
            (pToolBox->GetAlign() == WINDOWALIGN_TOP || pToolBox->GetAlign() == WINDOWALIGN_BOTTOM) ?
                POPUPMENU_EXECUTE_DOWN : POPUPMENU_EXECUTE_RIGHT );

        pToolBox->SetItemDown( nId, FALSE );
    }
    GetToolBox().EndSelection();
    DelPopup();
    return 0;

}


SfxPopupWindowType SwTbxAutoTextCtrl::GetPopupWindowType() const
{
    return SFX_POPUPWINDOW_ONTIMEOUT;
}

void SwTbxAutoTextCtrl::StateChanged( USHORT nSID,
                                              SfxItemState eState,
                                              const SfxPoolItem* pState )
{
    GetToolBox().EnableItem( GetId(), (GetItemState(pState) != SFX_ITEM_DISABLED) );
    if(FN_INSERT_FIELD_CTRL == nSID && eState >= SFX_ITEM_DEFAULT)
    {
        GetToolBox().CheckItem( GetId(), ((SfxBoolItem*)pState)->GetValue() );
    }
}

IMPL_LINK(SwTbxAutoTextCtrl, PopupHdl, PopupMenu*, pMenu)
{
    USHORT nId = pMenu->GetCurItemId();

    if ( GetSlotId() == FN_INSERT_FIELD_CTRL)
    {
        Sequence< PropertyValue > aArgs;
        const char* pChar = 0;
        switch(nId)
        {
            case FN_INSERT_FLD_DATE:
                pChar = ".uno:InsertDateField";
            break;
            case FN_INSERT_FLD_TIME:
                pChar = ".uno:InsertTimeField";
            break;
            case FN_INSERT_FLD_PGNUMBER:
                pChar = ".uno:InsertPageNumberField";
            break;
            case FN_INSERT_FLD_PGCOUNT:
                pChar = ".uno:InsertPageCountField";
            break;
            case FN_INSERT_FLD_TOPIC:
                pChar = ".uno:InsertTopicField";
            break;
            case FN_INSERT_FLD_TITLE:
                pChar = ".uno:InsertTitleField";
            break;
            case FN_INSERT_FLD_AUTHOR:
                pChar = ".uno:InsertAuthorField";
            break;
            default:
                pChar = ".uno:InsertFieldCtrl";
        }
        Dispatch( rtl::OUString::createFromAscii( pChar ),aArgs );
    }
    else
    {
        USHORT nBlock = nId / 100;

        SwGlossaryList* pGlossaryList = ::GetGlossaryList();
        String sShortName;
        String sGroup = pGlossaryList->GetGroupName(nBlock - 1, FALSE);
        String sLongName(pGlossaryList->GetBlockName(nBlock - 1, nId - (100 * nBlock) - 1, sShortName));

        SwGlossaryHdl* pGlosHdl = pView->GetGlosHdl();
        SwAbstractDialogFactory* pFact = SwAbstractDialogFactory::Create();
        OSL_ENSURE(pFact, "Dialogdiet fail!");
        ::GlossarySetActGroup fnSetActGroup = pFact->SetGlossaryActGroupFunc( DLG_RENAME_GLOS );
        if ( fnSetActGroup )
            (*fnSetActGroup)( sGroup );
        pGlosHdl->SetCurGroup(sGroup, TRUE);
        pGlosHdl->InsertGlossary(sShortName);
    }
    return 0;
}

void SwTbxAutoTextCtrl::DelPopup()
{
    if(pPopup)
    {
        if (GetSlotId() != FN_INSERT_FIELD_CTRL)
        {
            for( USHORT i = 0; i < pPopup->GetItemCount(); i ++ )
            {
                PopupMenu* pSubPopup = pPopup->GetPopupMenu(pPopup->GetItemId(i));
                delete pSubPopup;
            }
        }
        delete pPopup;
        pPopup = 0;
    }
}

/* Navigations-Popup */
// determine the order of the toolbox items
static USHORT const aNavigationInsertIds[ NAVI_ENTRIES ] =
{
    // -- first line
    NID_TBL,
    NID_FRM,
    NID_GRF,
    NID_OLE,
    NID_PGE,
    NID_OUTL,
    NID_MARK,
    NID_DRW,
    NID_CTRL,
    NID_PREV,
    // -- second line
    NID_REG,
    NID_BKM,
    NID_SEL,
    NID_FTN,
    NID_POSTIT,
    NID_SRCH_REP,
    NID_INDEX_ENTRY,
    NID_TABLE_FORMULA,
    NID_TABLE_FORMULA_ERROR,
    NID_NEXT
};
static USHORT const aNavigationHelpIds[ NAVI_ENTRIES ] =
{
    // -- first line
    HID_NID_TBL,
    HID_NID_FRM,
    HID_NID_GRF,
    HID_NID_OLE,
    HID_NID_PGE,
    HID_NID_OUTL,
    HID_NID_MARK,
    HID_NID_DRW,
    HID_NID_CTRL,
    HID_NID_PREV,
    // -- second line
    HID_NID_REG,
    HID_NID_BKM,
    HID_NID_SEL,
    HID_NID_FTN,
    HID_NID_POSTIT,
    HID_NID_SRCH_REP,
    HID_NID_INDEX_ENTRY,
    HID_NID_TABLE_FORMULA,
    HID_NID_TABLE_FORMULA_ERROR,
    HID_NID_NEXT
};

SwScrollNaviPopup::SwScrollNaviPopup( USHORT nId, const Reference< XFrame >& rFrame )
    : SfxPopupWindow(nId, rFrame, SW_RES(RID_SCROLL_NAVIGATION_WIN) ),
    aToolBox(this, 0),
    aSeparator(this, SW_RES(FL_SEP)),
    aInfoField(this, SW_RES(FI_INFO)),
    aIList(SW_RES(IL_VALUES)),
    nFwdId(FN_START_OF_NEXT_PAGE),
    nBackId(FN_START_OF_PREV_PAGE)
{
    USHORT i;

    aToolBox.SetHelpId(HID_NAVI_VS);
    aToolBox.SetLineCount( 2 );
    aToolBox.SetOutStyle(TOOLBOX_STYLE_FLAT);
    for( i = 0; i < NID_COUNT; i++)
    {
        USHORT nNaviId = aNavigationInsertIds[i];
        String sText;
        ToolBoxItemBits  nTbxBits = 0;
        if((NID_PREV != nNaviId) && (NID_NEXT != nNaviId))
        {
            // -2, there's no string for Next/Prev
            USHORT nResStr = ST_TBL - 2 + nNaviId - NID_START;
            sText = String(SW_RES(nResStr));
            nTbxBits = TIB_CHECKABLE;
        }
        aToolBox.InsertItem(nNaviId, sText, nTbxBits);
        aToolBox.SetHelpId( nNaviId, aNavigationHelpIds[i] );
    }
    ApplyImageList();
    aToolBox.InsertBreak(NID_COUNT/2);
    // don't call it before!
    FreeResource();

    // these are global strings
    for( i = 0; i < 2 * NID_COUNT; i++)
    {
        sQuickHelp[i] = String(SW_RES(STR_IMGBTN_START + i));
    }

    Size aImgSize = aIList.GetImageSize();
    aImgSize.Width() += 5;
    aImgSize.Height() += 5;
    Size aSz = aToolBox.CalcWindowSizePixel(2);
    aToolBox.SetPosSizePixel( Point(), aSz );
    USHORT nItemId = SwView::GetMoveType();
    aInfoField.SetText(aToolBox.GetItemText(nItemId));
    aToolBox.CheckItem( nItemId, sal_True );
    Size aFTSize(aInfoField.GetSizePixel());
    Size aSepSize(aSeparator.GetSizePixel());
    aSepSize.Width() = aSz.Width();

    aSz.Height() += aFTSize.Height() + aSepSize.Height();
    aInfoField.SetPosSizePixel(
        Point(0, aSz.Height() - aFTSize.Height()), Size(aSz.Width(), aFTSize.Height()));

    aSeparator.SetSizePixel(aSepSize);
    aSeparator.SetPosPixel(Point(0, aSz.Height() - aFTSize.Height() - aSepSize.Height()));

    SetOutputSizePixel(aSz);
    aToolBox.SetSelectHdl(LINK(this, SwScrollNaviPopup, SelectHdl));
    aToolBox.StartSelection();
    aToolBox.Show();
}

SwScrollNaviPopup::~SwScrollNaviPopup()
{
}

void SwScrollNaviPopup::DataChanged( const DataChangedEvent& rDCEvt )
{
    if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
         (rDCEvt.GetFlags() & SETTINGS_STYLE) )
            ApplyImageList();

    Window::DataChanged( rDCEvt );
}

void SwScrollNaviPopup::ApplyImageList()
{
    ImageList& rImgLst = aIList;
    for(USHORT i = 0; i < NID_COUNT; i++)
    {
        USHORT nNaviId = aNavigationInsertIds[i];
        aToolBox.SetItemImage(nNaviId, rImgLst.GetImage(nNaviId));
    }
}

SfxPopupWindow* SwScrollNaviPopup::Clone() const
{
    return new SwScrollNaviPopup( GetId(), GetFrame() );
}

IMPL_LINK(SwScrollNaviPopup, SelectHdl, ToolBox*, pSet)
{
    USHORT nSet = pSet->GetCurItemId();
    if( nSet != NID_PREV && nSet != NID_NEXT )
    {
        SwView::SetMoveType(nSet);
        aToolBox.SetItemText(NID_NEXT, sQuickHelp[nSet - NID_START]);
        aToolBox.SetItemText(NID_PREV, sQuickHelp[nSet - NID_START + NID_COUNT]);
        aInfoField.SetText(aToolBox.GetItemText(nSet));
        //check the current button only
        for(USHORT i = 0; i < NID_COUNT; i++)
        {
            USHORT nItemId = aToolBox.GetItemId( i );
            aToolBox.CheckItem( nItemId, nItemId == nSet );
        }
    }
    else
    {
        SfxBoolItem aNext(FN_SCROLL_NEXT_PREV, NID_NEXT == nSet);
        Any a;
        Sequence< PropertyValue > aArgs( 1 );
        aArgs[0].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ScrollNextPrev" ));
        aNext.QueryValue( a );
        aArgs[0].Value = a;
        SfxToolBoxControl::Dispatch( Reference< XDispatchProvider >( GetFrame()->getController(), UNO_QUERY ),
                                     rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:ScrollNextPrev" )),
                                     aArgs );
    }
    return 0;
}

void SwScrollNaviToolBox::MouseButtonUp( const MouseEvent& rMEvt )
{
    ToolBox::MouseButtonUp(rMEvt);
    if ( ((SwScrollNaviPopup*)GetParent())->IsInPopupMode() )
        ((SwScrollNaviPopup*)GetParent())->EndPopupMode( FLOATWIN_POPUPMODEEND_CLOSEALL );
}

void  SwScrollNaviToolBox::RequestHelp( const HelpEvent& rHEvt )
{
    SetItemText(NID_NEXT, SwScrollNaviPopup::GetQuickHelpText(TRUE));
    SetItemText(NID_PREV, SwScrollNaviPopup::GetQuickHelpText(FALSE));
    ToolBox::RequestHelp( rHEvt );

}

String  SwScrollNaviPopup::GetQuickHelpText(BOOL bNext)
{
    USHORT nResId = STR_IMGBTN_START;
    nResId += SwView::GetMoveType() - NID_START;
    if(!bNext)
        nResId += NID_COUNT;
    return String(SW_RES(nResId));
}

void SwNaviImageButton::Click()
{
    pPopup = new
        SwScrollNaviPopup( FN_SCROLL_NAVIGATION,
                           m_xFrame );
    Point aPos = OutputToScreenPixel(Point(0,0));
    Rectangle aRect(aPos, GetSizePixel());
    SetPopupWindow( pPopup );
    pPopup->StartPopupMode(aRect, FLOATWIN_POPUPMODE_LEFT|FLOATWIN_POPUPMODE_ALLOWTEAROFF);
}

//--------------------------------------------------------------------

void SwNaviImageButton::SetPopupWindow( SfxPopupWindow* pWindow )
{
    pPopupWindow = pWindow;
    pPopupWindow->SetPopupModeEndHdl( LINK( this, SwNaviImageButton, PopupModeEndHdl ));
    pPopupWindow->SetDeleteLink_Impl( LINK( this, SwNaviImageButton, ClosePopupWindow ));
}

//--------------------------------------------------------------------

IMPL_LINK( SwNaviImageButton, PopupModeEndHdl, void *, EMPTYARG )
{
    if ( pPopupWindow->IsVisible() )
    {
        // Replace floating window with popup window and destroy
        // floating window instance.
        delete pFloatingWindow;
        pFloatingWindow = pPopupWindow;
        pPopupWindow    = 0;
    }
    else
    {
        // Popup window has been closed by the user. No replacement, instance
        // will destroy itself.
        pPopupWindow = 0;
    }

    return 1;
}

//--------------------------------------------------------------------
IMPL_LINK( SwNaviImageButton, ClosePopupWindow, SfxPopupWindow *, pWindow )
{
    if ( pWindow == pFloatingWindow )
        pFloatingWindow = 0;
    else
        pPopupWindow = 0;

    return 1;
}

void SwHlpImageButton::RequestHelp( const HelpEvent& rHEvt )
{

    SetQuickHelpText(SwScrollNaviPopup::GetQuickHelpText(!bUp));

    ImageButton::RequestHelp(rHEvt);
}

SwNaviImageButton::SwNaviImageButton(
    Window* pParent,
    const Reference< XFrame >& rFrame ) :
    ImageButton(pParent, SW_RES(BTN_NAVI)),
        pPopup(0),
        aImage(SW_RES(IMG_BTN)),
        sQuickText(SW_RES(ST_QUICK)),
        pPopupWindow(0),
        pFloatingWindow(0),
        m_xFrame( rFrame )
{
    FreeResource();
    SetStyle(GetStyle()|WB_NOPOINTERFOCUS);
    SetQuickHelpText(sQuickText);
    SetModeImage( aImage );
}

void SwNaviImageButton::DataChanged( const DataChangedEvent& rDCEvt )
{
    if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) &&
         (rDCEvt.GetFlags() & SETTINGS_STYLE) )
            SetModeImage( aImage );

    Window::DataChanged( rDCEvt );
}

class SwZoomBox_Impl : public ComboBox
{
    USHORT          nSlotId;
    BOOL            bRelease;
    uno::Reference< frame::XDispatchProvider > m_xDispatchProvider;

public:
    SwZoomBox_Impl(
        Window* pParent,
        USHORT nSlot,
        const Reference< XDispatchProvider >& rDispatchProvider );
    ~SwZoomBox_Impl();

protected:
    virtual void    Select();
    virtual long    Notify( NotifyEvent& rNEvt );

    void ReleaseFocus();

};

SwZoomBox_Impl::SwZoomBox_Impl(
    Window* pParent,
    USHORT nSlot,
    const Reference< XDispatchProvider >& rDispatchProvider ):
    ComboBox( pParent, SW_RES(RID_PVIEW_ZOOM_LB)),
    nSlotId(nSlot),
    bRelease(TRUE),
    m_xDispatchProvider( rDispatchProvider )
{
    EnableAutocomplete( FALSE );
    USHORT aZoomValues[] =
    {   25, 50, 75, 100, 150, 200 };
    for(USHORT i = 0; i < sizeof(aZoomValues)/sizeof(USHORT); i++)
    {
        String sEntry = String::CreateFromInt32(aZoomValues[i]);
        sEntry += '%';
        InsertEntry(sEntry);
    }
}

SwZoomBox_Impl::~SwZoomBox_Impl()
{}

void    SwZoomBox_Impl::Select()
{
    if ( !IsTravelSelect() )
    {
        String sEntry(GetText());
        sEntry.EraseAllChars( '%' );
        USHORT nZoom = (USHORT)sEntry.ToInt32();
        if(nZoom < MINZOOM)
            nZoom = MINZOOM;
        if(nZoom > MAXZOOM)
            nZoom = MAXZOOM;

        SfxUInt16Item aItem( nSlotId, nZoom );
        if ( FN_PREVIEW_ZOOM == nSlotId )
        {
            Any a;
            Sequence< PropertyValue > aArgs( 1 );
            aArgs[0].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "PreviewZoom" ));
            aItem.QueryValue( a );
            aArgs[0].Value = a;
            SfxToolBoxControl::Dispatch(
                m_xDispatchProvider,
                rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:PreviewZoom" )),
                aArgs );
        }

        ReleaseFocus();
    }
}

long SwZoomBox_Impl::Notify( NotifyEvent& rNEvt )
{
    long nHandled = 0;

    if ( rNEvt.GetType() == EVENT_KEYINPUT )
    {
        USHORT nCode = rNEvt.GetKeyEvent()->GetKeyCode().GetCode();

        switch ( nCode )
        {
            case KEY_RETURN:
            case KEY_TAB:
            {
                if ( KEY_TAB == nCode )
                    bRelease = FALSE;
                else
                    nHandled = 1;
                Select();
                break;
            }

            case KEY_ESCAPE:
                SetText( GetSavedValue() );
                ReleaseFocus();
                break;
        }
    }
    else if ( EVENT_LOSEFOCUS == rNEvt.GetType() )
    {
        Window* pFocusWin = Application::GetFocusWindow();
        if ( !HasFocus() && GetSubEdit() != pFocusWin )
            SetText( GetSavedValue() );
    }

    return nHandled ? nHandled : ComboBox::Notify( rNEvt );
}

void SwZoomBox_Impl::ReleaseFocus()
{
    if ( !bRelease )
    {
        bRelease = TRUE;
        return;
    }
    SfxViewShell* pCurSh = SfxViewShell::Current();

    if ( pCurSh )
    {
        Window* pShellWnd = pCurSh->GetWindow();

        if ( pShellWnd )
            pShellWnd->GrabFocus();
    }
}

SFX_IMPL_TOOLBOX_CONTROL( SwPreviewZoomControl, SfxUInt16Item);

SwPreviewZoomControl::SwPreviewZoomControl(
    USHORT nSlotId,
    USHORT nId,
    ToolBox& rTbx) :
    SfxToolBoxControl( nSlotId, nId, rTbx )
{
}

SwPreviewZoomControl::~SwPreviewZoomControl()
{
}

void SwPreviewZoomControl::StateChanged( USHORT /*nSID*/,
                                         SfxItemState eState,
                                         const SfxPoolItem* pState )
{
    USHORT nId = GetId();
    GetToolBox().EnableItem( nId, (GetItemState(pState) != SFX_ITEM_DISABLED) );
    SwZoomBox_Impl* pBox = (SwZoomBox_Impl*)GetToolBox().GetItemWindow( GetId() );
    if(SFX_ITEM_AVAILABLE <= eState)
    {
        String sZoom(String::CreateFromInt32(((const SfxUInt16Item*)pState)->GetValue()));
        sZoom += '%';
        pBox->SetText(sZoom);
        pBox->SaveValue();
    }
}

Window* SwPreviewZoomControl::CreateItemWindow( Window *pParent )
{
    SwZoomBox_Impl* pRet = new SwZoomBox_Impl( pParent, GetSlotId(), Reference< XDispatchProvider >( m_xFrame->getController(), UNO_QUERY ));
    return pRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

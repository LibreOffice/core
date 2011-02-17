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
#include "precompiled_svx.hxx"

#include <string> // HACK: prevent conflict between STLPORT and Workshop headers

#ifndef _TOOLBOX_HXX //autogen
#include <vcl/toolbox.hxx>
#endif
#ifndef _FIELD_HXX //autogen
#include <vcl/field.hxx>
#endif
#include <vcl/fixed.hxx>
#include <vcl/msgbox.hxx>
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
#include "grafctrl.hrc"
#include <svx/dialogs.hrc>
#include <editeng/brshitem.hxx>
#include <editeng/sizeitem.hxx>
#include <svx/sdgcpitm.hxx>
//CHINA001 #include "../dialog/grfpage.hxx"
#include <svx/itemwin.hxx>
#include <svx/dialmgr.hxx>
#include <svx/svdview.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdundo.hxx>
#include <svx/svdtrans.hxx>
#include "svx/grafctrl.hxx"
#include "svx/tbxcolor.hxx"

// namespaces
using ::rtl::OUString;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;

#include <svx/svxdlg.hxx> //CHINA001
// -----------
// - Defines -
// -----------

#define SYMBOL_TO_FIELD_OFFSET      4
#define ITEMVALUE(ItemSet,Id,Cast)  ((const Cast&)(ItemSet).Get(Id)).GetValue()
#define TOOLBOX_NAME                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "colorbar" ) )

// ----------------
// - TbxImageItem -
// ----------------

TYPEINIT1_AUTOFACTORY( TbxImageItem, SfxUInt16Item );

//---------------------------------------------------------

TbxImageItem::TbxImageItem( sal_uInt16 _nWhich, sal_uInt16 nImage ) :
    SfxUInt16Item( _nWhich, nImage )
{
}

//---------------------------------------------------------

SfxPoolItem* TbxImageItem::Clone( SfxItemPool* ) const
{
    return new TbxImageItem( *this );
}

//---------------------------------------------------------

int TbxImageItem::operator==( const SfxPoolItem& rItem ) const
{
    return( ( (TbxImageItem&) rItem ).GetValue() == GetValue() );
}

// -----------------------
// - ImplGrafMetricField -
// -----------------------

class ImplGrafMetricField : public MetricField
{
    using Window::Update;

private:
    Timer               maTimer;
    OUString            maCommand;
    Reference< XFrame > mxFrame;

                    DECL_LINK( ImplModifyHdl, Timer* );

protected:

    virtual void    Modify();

public:

                    ImplGrafMetricField( Window* pParent, const rtl::OUString& aCmd, const Reference< XFrame >& rFrame );
                    ~ImplGrafMetricField();

    void            Update( const SfxPoolItem* pItem );
    const OUString& GetCommand() const { return maCommand; }
};

// -----------------------------------------------------------------------------

ImplGrafMetricField::ImplGrafMetricField( Window* pParent, const rtl::OUString& rCmd, const Reference< XFrame >& rFrame ) :
    MetricField( pParent, WB_BORDER | WB_SPIN | WB_REPEAT | WB_3DLOOK ),
    maCommand( rCmd ),
    mxFrame( rFrame )
{
    Size aSize( GetTextWidth( String::CreateFromAscii("-100 %") ), GetTextHeight() );

    aSize.Width() += 20, aSize.Height() += 6;
    SetSizePixel( aSize );

    if ( maCommand.equalsAscii( ".uno:GrafGamma" ))
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
        const long nMinVal = ( maCommand.equalsAscii( ".uno:GrafTransparence" )) ? 0 : -100;

        SetUnit( FUNIT_CUSTOM );
        SetCustomUnitText( String::CreateFromAscii(" %") );
        SetDecimalDigits( 0 );

        SetMin( nMinVal );
        SetFirst( nMinVal );
        SetMax( 100 );
        SetLast( 100 );
        SetSpinSize( 1 );
    }

    maTimer.SetTimeout( 100 );
    maTimer.SetTimeoutHdl( LINK( this, ImplGrafMetricField, ImplModifyHdl ) );
}

// -----------------------------------------------------------------------------

ImplGrafMetricField::~ImplGrafMetricField()
{
}

// -----------------------------------------------------------------------------

void ImplGrafMetricField::Modify()
{
    maTimer.Start();
}

// -----------------------------------------------------------------------------

IMPL_LINK( ImplGrafMetricField, ImplModifyHdl, Timer*, EMPTYARG )
{
    const sal_Int64 nVal = GetValue();

    // Convert value to an any to be usable with dispatch API
    Any a;
    if ( maCommand.equalsAscii( ".uno:GrafRed" ) ||
         maCommand.equalsAscii( ".uno:GrafGreen" ) ||
         maCommand.equalsAscii( ".uno:GrafBlue" ) ||
         maCommand.equalsAscii( ".uno:GrafLuminance" ) ||
         maCommand.equalsAscii( ".uno:GrafContrast" ))
        a = makeAny( sal_Int16( nVal ));
    else if ( maCommand.equalsAscii( ".uno:GrafGamma" ) ||
              maCommand.equalsAscii( ".uno:GrafTransparence" ))
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
    return 0L;
}

// -----------------------------------------------------------------------------

void ImplGrafMetricField::Update( const SfxPoolItem* pItem )
{
    if( pItem )
    {
        long nValue;

        if ( maCommand.equalsAscii( ".uno:GrafTransparence" ))
            nValue = ( (SfxUInt16Item*) pItem )->GetValue();
        else if ( maCommand.equalsAscii( ".uno:GrafGamma" ))
            nValue = ( (SfxUInt32Item*) pItem )->GetValue();
        else
            nValue = ( (SfxInt16Item*) pItem )->GetValue();

        SetValue( nValue );
    }
    else
        SetText( String() );
}

// --------------------
// - ImplGrafControl  -
// --------------------

struct CommandToRID
{
    const char* pCommand;
    sal_uInt16      nResId;
    sal_uInt16      nHCResId;
};

static sal_uInt16 ImplGetRID( const OUString& aCommand, bool bHighContrast )
{
    static const CommandToRID aImplCommandToResMap[] =
    {
        { ".uno:GrafRed", RID_SVXIMG_GRAF_RED, RID_SVXIMG_GRAF_RED_H },
        { ".uno:GrafGreen", RID_SVXIMG_GRAF_GREEN, RID_SVXIMG_GRAF_GREEN_H },
        { ".uno:GrafBlue", RID_SVXIMG_GRAF_BLUE, RID_SVXIMG_GRAF_BLUE_H },
        { ".uno:GrafLuminance", RID_SVXIMG_GRAF_LUMINANCE, RID_SVXIMG_GRAF_LUMINANCE_H },
        { ".uno:GrafContrast", RID_SVXIMG_GRAF_CONTRAST, RID_SVXIMG_GRAF_CONTRAST_H },
        { ".uno:GrafGamma", RID_SVXIMG_GRAF_GAMMA, RID_SVXIMG_GRAF_GAMMA_H },
        { ".uno:GrafTransparence", RID_SVXIMG_GRAF_TRANSPARENCE, RID_SVXIMG_GRAF_TRANSPARENCE_H },
        { 0, 0, 0 }
    };

    sal_uInt16 nRID = 0;

    sal_Int32 i( 0 );
    while ( aImplCommandToResMap[ i ].pCommand )
    {
        if ( aCommand.equalsAscii( aImplCommandToResMap[ i ].pCommand ))
        {
            if ( bHighContrast )
                nRID = aImplCommandToResMap[ i ].nHCResId;
            else
                nRID = aImplCommandToResMap[ i ].nResId;
            break;
        }
        ++i;
    }

    return nRID;
}

// -----------------------------------------------------------------------------

class ImplGrafControl : public Control
{
    using Window::Update;
private:
    FixedImage              maImage;
    ImplGrafMetricField     maField;

protected:

    virtual void            GetFocus();

public:

                            ImplGrafControl( Window* pParent, sal_uInt16 nSlotId, const rtl::OUString& rCmd, const Reference< XFrame >& rFrame );
                            ~ImplGrafControl();

    void                    Update( const SfxPoolItem* pItem ) { maField.Update( pItem ); }
    void                    SetText( const String& rStr ) { maField.SetText( rStr ); }
};

// -----------------------------------------------------------------------------

ImplGrafControl::ImplGrafControl( Window* pParent, sal_uInt16, const rtl::OUString& rCmd, const Reference< XFrame >& rFrame ) :
    Control( pParent, WB_TABSTOP ),
    maImage     ( this ),
    maField     ( this, rCmd, rFrame )
{
    ResId   aResId( ImplGetRID( rCmd, false ), DIALOG_MGR() ) ;
    Image   aImage( aResId );

    ResId   aResIdHC( ImplGetRID( rCmd, true ), DIALOG_MGR() ) ;
    Image   aImageHC( aResIdHC );

    Size    aImgSize( aImage.GetSizePixel() );
    Size    aFldSize( maField.GetSizePixel() );
    long    nFldY, nImgY;

    maImage.SetImage( aImage );
    maImage.SetModeImage( aImageHC, BMP_COLOR_HIGHCONTRAST );
    maImage.SetSizePixel( aImgSize );
    // we want to see the backbround of the toolbox, not of the FixedImage or Control
    maImage.SetBackground( Wallpaper( COL_TRANSPARENT ) );
    SetBackground( Wallpaper( COL_TRANSPARENT ) );

    if( aImgSize.Height() > aFldSize.Height() )
        nImgY = 0, nFldY = ( aImgSize.Height() - aFldSize.Height() ) >> 1;
    else
        nFldY = 0, nImgY = ( aFldSize.Height() - aImgSize.Height() ) >> 1;

    long nOffset = SYMBOL_TO_FIELD_OFFSET / 2;
    maImage.SetPosPixel( Point( nOffset, nImgY ) );
    maField.SetPosPixel( Point( aImgSize.Width() + SYMBOL_TO_FIELD_OFFSET, nFldY ) );
    SetSizePixel( Size( aImgSize.Width() + aFldSize.Width() + SYMBOL_TO_FIELD_OFFSET + nOffset,
                  Max( aImgSize.Height(), aFldSize.Height() ) ) );

    SetBackground( Wallpaper() ); // transparent background

    maImage.Show();

    maField.SetHelpId( rtl::OUStringToOString( rCmd, RTL_TEXTENCODING_UTF8 ) );
    maField.Show();
}

// -----------------------------------------------------------------------------

ImplGrafControl::~ImplGrafControl()
{
}

// -----------------------------------------------------------------------------

void ImplGrafControl::GetFocus()
{
    maField.GrabFocus();
}

// -----------------------
// - ImplGrafModeControl -
// -----------------------

class ImplGrafModeControl : public ListBox
{
    using Window::Update;
private:
    sal_uInt16              mnCurPos;
    Reference< XFrame > mxFrame;

    virtual void    Select();
    virtual long    PreNotify( NotifyEvent& rNEvt );
    virtual long    Notify( NotifyEvent& rNEvt );
    void            ImplReleaseFocus();

public:

                    ImplGrafModeControl( Window* pParent, const Reference< XFrame >& rFrame );
                    ~ImplGrafModeControl();

    void            Update( const SfxPoolItem* pItem );
};

// -----------------------------------------------------------------------------

ImplGrafModeControl::ImplGrafModeControl( Window* pParent, const Reference< XFrame >& rFrame ) :
    ListBox( pParent, WB_BORDER | WB_DROPDOWN | WB_AUTOHSCROLL ),
    mnCurPos( 0 ),
    mxFrame( rFrame )
{
    SetSizePixel( Size( 100, 260 ) );

    InsertEntry( SVX_RESSTR( RID_SVXSTR_GRAFMODE_STANDARD ) );
    InsertEntry( SVX_RESSTR( RID_SVXSTR_GRAFMODE_GREYS ) );
    InsertEntry( SVX_RESSTR( RID_SVXSTR_GRAFMODE_MONO ) );
    InsertEntry( SVX_RESSTR( RID_SVXSTR_GRAFMODE_WATERMARK ) );

    Show();
}

// -----------------------------------------------------------------------

ImplGrafModeControl::~ImplGrafModeControl()
{
}

// -----------------------------------------------------------------------

void ImplGrafModeControl::Select()
{
    if ( !IsTravelSelect() )
    {
        Sequence< PropertyValue > aArgs( 1 );
        aArgs[0].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "GrafMode" ));
        aArgs[0].Value = makeAny( sal_Int16( GetSelectEntryPos() ));

        /*  #i33380# DR 2004-09-03 Moved the following line above the Dispatch() call.
            This instance may be deleted in the meantime (i.e. when a dialog is opened
            while in Dispatch()), accessing members will crash in this case. */
        ImplReleaseFocus();

        SfxToolBoxControl::Dispatch(
            Reference< XDispatchProvider >( mxFrame->getController(), UNO_QUERY ),
            OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:GrafMode" )),
            aArgs );
    }
}

// -----------------------------------------------------------------------

long ImplGrafModeControl::PreNotify( NotifyEvent& rNEvt )
{
    sal_uInt16 nType = rNEvt.GetType();

    if( EVENT_MOUSEBUTTONDOWN == nType || EVENT_GETFOCUS == nType )
        mnCurPos = GetSelectEntryPos();

    return ListBox::PreNotify( rNEvt );
}

// -----------------------------------------------------------------------

long ImplGrafModeControl::Notify( NotifyEvent& rNEvt )
{
    long nHandled = ListBox::Notify( rNEvt );

    if( rNEvt.GetType() == EVENT_KEYINPUT )
    {
        const KeyEvent* pKEvt = rNEvt.GetKeyEvent();

        switch( pKEvt->GetKeyCode().GetCode() )
        {
            case KEY_RETURN:
            {
                Select();
                nHandled = 1;
            }
            break;

            case KEY_ESCAPE:
            {
                SelectEntryPos( mnCurPos );
                ImplReleaseFocus();
                nHandled = 1;
            }
            break;
        }
    }

    return nHandled;
}

// -----------------------------------------------------------------------

void ImplGrafModeControl::ImplReleaseFocus()
{
    if( SfxViewShell::Current() )
    {
        Window* pShellWnd = SfxViewShell::Current()->GetWindow();

        if( pShellWnd )
            pShellWnd->GrabFocus();
    }
}

// -----------------------------------------------------------------------

void ImplGrafModeControl::Update( const SfxPoolItem* pItem )
{
    if( pItem )
        SelectEntryPos( ((SfxUInt16Item*)pItem)->GetValue() );
    else
        SetNoSelection();
}

// -----------------------
// - ImplGrafFilterPopup -
// -----------------------
/*
CD!!!
class ImplGrafFilterPopup : public SfxPopupWindow
{
private:

    SvxGrafFilterToolBoxControl*        mpParent;
    Reference< XConfigurableUIElement > m_xToolBar;
//  SfxToolBoxManager                   maTbxMgr;
    ResId                               maResIdWin;
    ResId                               maResIdTbx;
    WindowAlign                         meTbxAlign;
    Link                                maSelectHdl;

                                    DECL_LINK( TbxSelectHdl, void* );

public:
                                    ImplGrafFilterPopup( sal_uInt16 nId, SvxGrafFilterToolBoxControl* pParent,
                                                         WindowAlign eAlign,
                                                         const ResId& rResIdWin, const ResId& rResIdTbx,
                                                         SfxBindings& rBindings );
                                    ~ImplGrafFilterPopup();

    virtual SfxPopupWindow*         Clone() const;
    virtual void                    PopupModeEnd();

    void                            StartSelection() { maTbxMgr.GetToolBox().StartSelection(); }
    void                            Update();
};

// -----------------------------------------------------------------------------

ImplGrafFilterPopup::ImplGrafFilterPopup( sal_uInt16 nId, SvxGrafFilterToolBoxControl* pParent,
                                          WindowAlign eAlign,
                                          const ResId& rResIdWin, const ResId& rResIdTbx ) :
    SfxPopupWindow  ( nId, rResIdWin ),
    mpParent        ( pParent ),
    maTbxMgr        ( this, GetBindings(), rResIdTbx ),
    maResIdWin      ( rResIdWin ),
    maResIdTbx      ( rResIdTbx ),
    meTbxAlign      ( eAlign )
{
    maTbxMgr.UseDefault();

    maSelectHdl = maTbxMgr.GetToolBox().GetSelectHdl();
    maTbxMgr.GetToolBox().SetSelectHdl( LINK( this, ImplGrafFilterPopup, TbxSelectHdl ) );

    FreeResource();

    const Size aSize( maTbxMgr.CalcWindowSizePixel() );
    maTbxMgr.SetPosSizePixel( Point(), aSize );
    SetOutputSizePixel( aSize );
}

// -----------------------------------------------------------------------------

ImplGrafFilterPopup::~ImplGrafFilterPopup()
{
}

// -----------------------------------------------------------------------------

SfxPopupWindow* ImplGrafFilterPopup::Clone() const
{
    return( new ImplGrafFilterPopup( GetId(), mpParent, meTbxAlign,
                                     maResIdWin, maResIdTbx,
                                     (SfxBindings&) GetBindings() ) );
}

// -----------------------------------------------------------------------------

void ImplGrafFilterPopup::Update()
{
    ToolBox* pBox = &maTbxMgr.GetToolBox();
    maTbxMgr.Activate( pBox );
    maTbxMgr.Deactivate( pBox );
}

// -----------------------------------------------------------------------------

void ImplGrafFilterPopup::PopupModeEnd()
{
    maTbxMgr.GetToolBox().EndSelection();
    SfxPopupWindow::PopupModeEnd();
}

// -----------------------------------------------------------------------------

IMPL_LINK( ImplGrafFilterPopup, TbxSelectHdl, void*, EMPTYARG )
{
    const sal_uInt16 nSlotId = maTbxMgr.GetToolBox().GetCurItemId();

    if( IsInPopupMode() )
        EndPopupMode();

    GetBindings().GetDispatcher()->Execute( nSlotId, SFX_CALLMODE_ASYNCHRON );

    return 0;
}
*/
// -------------------------------
// - SvxGrafFilterToolBoxControl -
// -------------------------------

SFX_IMPL_TOOLBOX_CONTROL( SvxGrafFilterToolBoxControl, TbxImageItem );

// -----------------------------------------------------------------------------

SvxGrafFilterToolBoxControl::SvxGrafFilterToolBoxControl( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx ) :
    SfxToolBoxControl( nSlotId, nId, rTbx )
{
    rTbx.SetItemBits( nId, TIB_DROPDOWNONLY | rTbx.GetItemBits( nId ) );
    rTbx.Invalidate();
}

// -----------------------------------------------------------------------------

SvxGrafFilterToolBoxControl::~SvxGrafFilterToolBoxControl()
{
}

// -----------------------------------------------------------------------------

void SvxGrafFilterToolBoxControl::StateChanged( sal_uInt16, SfxItemState eState, const SfxPoolItem* )
{
    GetToolBox().EnableItem( GetId(), ( eState != SFX_ITEM_DISABLED ) );
}

// -----------------------------------------------------------------------------

SfxPopupWindowType SvxGrafFilterToolBoxControl::GetPopupWindowType() const
{
    return SFX_POPUPWINDOW_ONCLICK;
}

// -----------------------------------------------------------------------------

SfxPopupWindow* SvxGrafFilterToolBoxControl::CreatePopupWindow()
{
    rtl::OUString aSubTbxResName(
        RTL_CONSTASCII_USTRINGPARAM( "private:resource/toolbar/graffilterbar" ) );
    createAndPositionSubToolBar( aSubTbxResName );

    return NULL;
}

// -------------------------
// - SvxGrafToolBoxControl -
// -------------------------

SFX_IMPL_TOOLBOX_CONTROL( SvxGrafToolBoxControl, SfxVoidItem );

// -----------------------------------------------------------------------------

SvxGrafToolBoxControl::SvxGrafToolBoxControl( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx) :
    SfxToolBoxControl( nSlotId, nId, rTbx )
{
    rTbx.SetItemBits( nId, TIB_DROPDOWN | rTbx.GetItemBits( nId ) );
    rTbx.Invalidate();
}

// -----------------------------------------------------------------------------

SvxGrafToolBoxControl::~SvxGrafToolBoxControl()
{
}

// -----------------------------------------------------------------------------

void SvxGrafToolBoxControl::StateChanged( sal_uInt16, SfxItemState eState, const SfxPoolItem* pState )

{
    ImplGrafControl* pCtrl = (ImplGrafControl*) GetToolBox().GetItemWindow( GetId() );
    DBG_ASSERT( pCtrl, "Control not found" );

    if( eState == SFX_ITEM_DISABLED )
    {
        pCtrl->Disable();
        pCtrl->SetText( String() );
    }
    else
    {
        pCtrl->Enable();

        if( eState == SFX_ITEM_AVAILABLE )
            pCtrl->Update( pState );
        else
            pCtrl->Update( NULL );
    }
}

// -----------------------------------------------------------------------------

Window* SvxGrafToolBoxControl::CreateItemWindow( Window *pParent )
{
    return( new ImplGrafControl( pParent, GetSlotId(), m_aCommandURL, m_xFrame ) );
}

// ----------------------------
// - SvxGrafRedToolBoxControl -
// ----------------------------

SFX_IMPL_TOOLBOX_CONTROL( SvxGrafRedToolBoxControl, SfxInt16Item );

// -----------------------------------------------------------------------------

SvxGrafRedToolBoxControl::SvxGrafRedToolBoxControl( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx ) :
    SvxGrafToolBoxControl( nSlotId, nId, rTbx )
{
}

// ------------------------------
// - SvxGrafGreenToolBoxControl -
// ------------------------------

SFX_IMPL_TOOLBOX_CONTROL( SvxGrafGreenToolBoxControl, SfxInt16Item );

// -----------------------------------------------------------------------------

SvxGrafGreenToolBoxControl::SvxGrafGreenToolBoxControl( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx ) :
    SvxGrafToolBoxControl( nSlotId, nId, rTbx )
{
}

// -----------------------------
// - SvxGrafBlueToolBoxControl -
// -----------------------------

SFX_IMPL_TOOLBOX_CONTROL( SvxGrafBlueToolBoxControl, SfxInt16Item );

// -----------------------------------------------------------------------------

SvxGrafBlueToolBoxControl::SvxGrafBlueToolBoxControl( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx ) :
    SvxGrafToolBoxControl( nSlotId, nId, rTbx )
{
}

// ----------------------------------
// - SvxGrafLuminanceToolBoxControl -
// ----------------------------------

SFX_IMPL_TOOLBOX_CONTROL( SvxGrafLuminanceToolBoxControl, SfxInt16Item );

// -----------------------------------------------------------------------------

SvxGrafLuminanceToolBoxControl::SvxGrafLuminanceToolBoxControl( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx ) :
    SvxGrafToolBoxControl( nSlotId, nId, rTbx )
{
}

// ----------------------------------
// - SvxGrafContrastToolBoxControl -
// ----------------------------------

SFX_IMPL_TOOLBOX_CONTROL( SvxGrafContrastToolBoxControl, SfxInt16Item );

// -----------------------------------------------------------------------------

SvxGrafContrastToolBoxControl::SvxGrafContrastToolBoxControl( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx ) :
    SvxGrafToolBoxControl( nSlotId, nId, rTbx )
{
}

// ------------------------------
// - SvxGrafGammaToolBoxControl -
// ------------------------------

SFX_IMPL_TOOLBOX_CONTROL( SvxGrafGammaToolBoxControl, SfxUInt32Item );

// -----------------------------------------------------------------------------

SvxGrafGammaToolBoxControl::SvxGrafGammaToolBoxControl( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx ) :
    SvxGrafToolBoxControl( nSlotId, nId, rTbx )
{
}

// -------------------------------------
// - SvxGrafTransparenceToolBoxControl -
// -------------------------------------

SFX_IMPL_TOOLBOX_CONTROL( SvxGrafTransparenceToolBoxControl, SfxUInt16Item );

// -----------------------------------------------------------------------------

SvxGrafTransparenceToolBoxControl::SvxGrafTransparenceToolBoxControl( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx ) :
    SvxGrafToolBoxControl( nSlotId, nId, rTbx )
{
}

// -----------------------------
// - SvxGrafModeToolBoxControl -
// -----------------------------

SFX_IMPL_TOOLBOX_CONTROL( SvxGrafModeToolBoxControl, SfxUInt16Item );

// -----------------------------------------------------------------------------

SvxGrafModeToolBoxControl::SvxGrafModeToolBoxControl( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx ) :
    SfxToolBoxControl( nSlotId, nId, rTbx )
{
}

// -----------------------------------------------------------------------------

SvxGrafModeToolBoxControl::~SvxGrafModeToolBoxControl()
{
}

// -----------------------------------------------------------------------------

void SvxGrafModeToolBoxControl::StateChanged( sal_uInt16, SfxItemState eState, const SfxPoolItem* pState )

{
    ImplGrafModeControl* pCtrl = (ImplGrafModeControl*) GetToolBox().GetItemWindow( GetId() );
    DBG_ASSERT( pCtrl, "Control not found" );

    if( eState == SFX_ITEM_DISABLED )
    {
        pCtrl->Disable();
        pCtrl->SetText( String() );
    }
    else
    {
        pCtrl->Enable();

        if( eState == SFX_ITEM_AVAILABLE )
            pCtrl->Update( pState );
        else
            pCtrl->Update( NULL );
    }
}

// -----------------------------------------------------------------------------

Window* SvxGrafModeToolBoxControl::CreateItemWindow( Window *pParent )
{
    return( new ImplGrafModeControl( pParent, m_xFrame ) );
}

// ---------------------
// - SvxGrafAttrHelper -
// ---------------------

void SvxGrafAttrHelper::ExecuteGrafAttr( SfxRequest& rReq, SdrView& rView )
{
    SfxItemPool&    rPool = rView.GetModel()->GetItemPool();
    SfxItemSet      aSet( rPool, SDRATTR_GRAF_FIRST, SDRATTR_GRAF_LAST );
    String          aUndoStr;
    const bool      bUndo = rView.IsUndoEnabled();

    if( bUndo )
    {
        aUndoStr = rView.GetDescriptionOfMarkedObjects();
        aUndoStr.Append( sal_Unicode(' ') );
    }

    const SfxItemSet*   pArgs = rReq.GetArgs();
    const SfxPoolItem*  pItem;
    sal_uInt16              nSlot = rReq.GetSlot();

    if( !pArgs || SFX_ITEM_SET != pArgs->GetItemState( nSlot, sal_False, &pItem ))
        pItem = 0;

    switch( nSlot )
    {
        case SID_ATTR_GRAF_RED:
        {
            if( pItem )
            {
                aSet.Put( SdrGrafRedItem( ((SfxInt16Item*)pItem)->GetValue() ));
                if( bUndo )
                    aUndoStr.Append( String( SVX_RESSTR( RID_SVXSTR_UNDO_GRAFRED ) ) );
            }
        }
        break;

        case SID_ATTR_GRAF_GREEN:
        {
            if( pItem )
            {
                aSet.Put( SdrGrafGreenItem( ((SfxInt16Item*)pItem)->GetValue() ));
                if( bUndo )
                    aUndoStr.Append( String( SVX_RESSTR( RID_SVXSTR_UNDO_GRAFGREEN ) ) );
            }
        }
        break;

        case SID_ATTR_GRAF_BLUE:
        {
            if( pItem )
            {
                aSet.Put( SdrGrafBlueItem( ((SfxInt16Item*)pItem)->GetValue() ));
                if( bUndo )
                    aUndoStr.Append( String( SVX_RESSTR( RID_SVXSTR_UNDO_GRAFBLUE ) ) );
            }
        }
        break;

        case SID_ATTR_GRAF_LUMINANCE:
        {
            if( pItem )
            {
                aSet.Put( SdrGrafLuminanceItem( ((SfxInt16Item*)pItem)->GetValue() ));
                if( bUndo )
                    aUndoStr.Append( String( SVX_RESSTR( RID_SVXSTR_UNDO_GRAFLUMINANCE ) ) );
            }
        }
        break;

        case SID_ATTR_GRAF_CONTRAST:
        {
            if( pItem )
            {
                aSet.Put( SdrGrafContrastItem( ((SfxInt16Item*)pItem)->GetValue() ));
                if( bUndo )
                    aUndoStr.Append( String( SVX_RESSTR( RID_SVXSTR_UNDO_GRAFCONTRAST ) ) );
            }
        }
        break;

        case SID_ATTR_GRAF_GAMMA:
        {
            if( pItem )
            {
                aSet.Put( SdrGrafGamma100Item( ((SfxUInt32Item*)pItem)->GetValue() ));
                if( bUndo )
                    aUndoStr.Append( String( SVX_RESSTR( RID_SVXSTR_UNDO_GRAFGAMMA ) ) );
            }
        }
        break;

        case SID_ATTR_GRAF_TRANSPARENCE:
        {
            if( pItem )
            {
                aSet.Put( SdrGrafTransparenceItem( ((SfxUInt16Item*)pItem)->GetValue() ));
                if( bUndo )
                    aUndoStr.Append( String( SVX_RESSTR( RID_SVXSTR_UNDO_GRAFTRANSPARENCY ) ) );
            }
        }
        break;

        case SID_ATTR_GRAF_MODE:
        {
            if( pItem )
            {
                aSet.Put( SdrGrafModeItem( (GraphicDrawMode) ((SfxUInt16Item*)pItem)->GetValue() ));
                if( bUndo )
                    aUndoStr.Append( String( SVX_RESSTR( RID_SVXSTR_UNDO_GRAFMODE ) ) );
            }
        }
        break;

        case( SID_ATTR_GRAF_CROP ):
        {
            const SdrMarkList& rMarkList = rView.GetMarkedObjectList();

            if( 0 < rMarkList.GetMarkCount() )
            {
                SdrGrafObj* pObj = (SdrGrafObj*) rMarkList.GetMark( 0 )->GetMarkedSdrObj();

                if( pObj && pObj->ISA( SdrGrafObj ) &&
                    ( pObj->GetGraphicType() != GRAPHIC_NONE ) &&
                    ( pObj->GetGraphicType() != GRAPHIC_DEFAULT ) )
                {
                    SfxItemSet          aGrfAttr( rPool, SDRATTR_GRAFCROP, SDRATTR_GRAFCROP, 0 );
                    const SfxMapUnit    eOldMetric = rPool.GetMetric( 0 );
                    const MapMode       aMap100( MAP_100TH_MM );
                    const MapMode       aMapTwip( MAP_TWIP );

                    aGrfAttr.Put(pObj->GetMergedItemSet());
                    rPool.SetDefaultMetric( SFX_MAPUNIT_TWIP );

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

                    const SdrGrafCropItem&  rCrop = (const SdrGrafCropItem&) aGrfAttr.Get( SDRATTR_GRAFCROP );
                    Size                    aLTSize( OutputDevice::LogicToLogic(
                                                    Size( rCrop.GetLeft(), rCrop.GetTop() ), aMap100, aMapTwip ) );
                    Size                    aRBSize( OutputDevice::LogicToLogic(
                                                    Size( rCrop.GetRight(), rCrop.GetBottom() ), aMap100, aMapTwip ) );

                    aCropDlgAttr.Put( SdrGrafCropItem( aLTSize.Width(), aLTSize.Height(),
                                                    aRBSize.Width(), aRBSize.Height() ) );

                    SfxSingleTabDialog  aCropDialog( SfxViewShell::Current() ? SfxViewShell::Current()->GetWindow() : NULL,
                                                    aCropDlgAttr, 950 );
                    const String        aCropStr = SVX_RESSTR( RID_SVXSTR_GRAFCROP );
                    //CHINA001 SfxTabPage*          pTabPage = SvxGrfCropPage::Create( &aCropDialog, aCropDlgAttr );
                    SfxAbstractDialogFactory* pFact = SfxAbstractDialogFactory::Create();
                    DBG_ASSERT(pFact, "Dialogdiet error!");//CHINA001
                    ::CreateTabPage fnCreatePage = pFact->GetTabPageCreatorFunc( RID_SVXPAGE_GRFCROP );
                    DBG_ASSERT(fnCreatePage, "Dialogdiet error!");//CHINA001
                    SfxTabPage* pTabPage = (*fnCreatePage)( &aCropDialog, aCropDlgAttr );
                    //CHINA001 end
                    pTabPage->SetText( aCropStr );
                    aCropDialog.SetTabPage( pTabPage );

                    if( aCropDialog.Execute() == RET_OK )
                    {
                        const SfxItemSet* pOutAttr = aCropDialog.GetOutputItemSet();

                        if( pOutAttr )
                        {
                            aUndoStr.Append( String( SVX_RESSTR( RID_SVXSTR_UNDO_GRAFCROP ) ) );

                            // set crop attributes
                            if( SFX_ITEM_SET <= pOutAttr->GetItemState( SDRATTR_GRAFCROP ) )
                            {
                                const SdrGrafCropItem& rNewCrop = (const SdrGrafCropItem&) pOutAttr->Get( SDRATTR_GRAFCROP );

                                aLTSize = OutputDevice::LogicToLogic( Size( rNewCrop.GetLeft(), rNewCrop.GetTop() ), aMapTwip, aMap100 );
                                aRBSize = OutputDevice::LogicToLogic( Size( rNewCrop.GetRight(), rNewCrop.GetBottom() ), aMapTwip, aMap100 );
                                aSet.Put( SdrGrafCropItem( aLTSize.Width(), aLTSize.Height(), aRBSize.Width(), aRBSize.Height() ) );
                            }

                            // set new logic rect
                            if( SFX_ITEM_SET <= pOutAttr->GetItemState( SID_ATTR_GRAF_FRMSIZE ) )
                            {
                                Point       aNewOrigin( pObj->GetLogicRect().TopLeft() );
                                const Size& rGrfSize = ( (const SvxSizeItem&) pOutAttr->Get( SID_ATTR_GRAF_FRMSIZE ) ).GetSize();
                                Size        aNewGrfSize( OutputDevice::LogicToLogic( rGrfSize, aMapTwip, aMap100 ) );
                                Size        aOldGrfSize( pObj->GetLogicRect().GetSize() );

                                Rectangle aNewRect( aNewOrigin, aNewGrfSize );
                                Point aOffset( (aNewGrfSize.Width() - aOldGrfSize.Width()) >> 1,
                                            (aNewGrfSize.Height() - aOldGrfSize.Height()) >> 1 );

                                // #106181# rotate snap rect before setting it
                                const GeoStat& aGeo = pObj->GetGeoStat();

                                if (aGeo.nDrehWink!=0 || aGeo.nShearWink!=0)
                                {
                                    Polygon aPol(aNewRect);

                                    // also transform origin offset
                                    if (aGeo.nShearWink!=0)
                                    {
                                        ShearPoly(aPol,
                                                aNewRect.TopLeft(),
                                                aGeo.nTan);
                                        ShearPoint(aOffset, Point(0,0), aGeo.nTan);
                                    }
                                    if (aGeo.nDrehWink!=0)
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

// -----------------------------------------------------------------------------

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

    for( int i = 0, nCount = rMarkList.GetMarkCount(); i < nCount; ++i )
    {
        SdrGrafObj* pGrafObj = dynamic_cast< SdrGrafObj* >( rMarkList.GetMark( i )->GetMarkedSdrObj() );

        if( !pGrafObj ||
            ( pGrafObj->GetGraphicType() == GRAPHIC_NONE ) ||
            ( pGrafObj->GetGraphicType() == GRAPHIC_DEFAULT  ) ||
            pGrafObj->HasRenderGraphic() )
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
            case( SID_ATTR_GRAF_MODE ):
            {
                if( SFX_ITEM_AVAILABLE <= aAttrSet.GetItemState( SDRATTR_GRAFMODE ) )
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

            case( SID_ATTR_GRAF_RED ):
            {
                if( SFX_ITEM_AVAILABLE <= aAttrSet.GetItemState( SDRATTR_GRAFRED ) )
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

            case( SID_ATTR_GRAF_GREEN ):
            {
                if( SFX_ITEM_AVAILABLE <= aAttrSet.GetItemState( SDRATTR_GRAFGREEN ) )
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

            case( SID_ATTR_GRAF_BLUE ):
            {
                if( SFX_ITEM_AVAILABLE <= aAttrSet.GetItemState( SDRATTR_GRAFBLUE ) )
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

            case( SID_ATTR_GRAF_LUMINANCE ):
            {
                if( SFX_ITEM_AVAILABLE <= aAttrSet.GetItemState( SDRATTR_GRAFLUMINANCE ) )
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

            case( SID_ATTR_GRAF_CONTRAST ):
            {
                if( SFX_ITEM_AVAILABLE <= aAttrSet.GetItemState( SDRATTR_GRAFCONTRAST ) )
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

            case( SID_ATTR_GRAF_GAMMA ):
            {
                if( SFX_ITEM_AVAILABLE <= aAttrSet.GetItemState( SDRATTR_GRAFGAMMA ) )
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

            case( SID_ATTR_GRAF_TRANSPARENCE ):
            {
                if( SFX_ITEM_AVAILABLE <= aAttrSet.GetItemState( SDRATTR_GRAFTRANSPARENCE ) )
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

            case( SID_ATTR_GRAF_CROP ):
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

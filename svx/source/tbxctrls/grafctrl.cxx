/*************************************************************************
 *
 *  $RCSfile: grafctrl.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ka $ $Date: 2000-11-18 11:23:34 $
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

#include <string> // HACK: prevent conflict between STLPORT and Workshop headers

#ifndef _TOOLBOX_HXX //autogen
#include <vcl/toolbox.hxx>
#endif
#ifndef _FIELD_HXX //autogen
#include <vcl/field.hxx>
#endif
#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif
#ifndef _SFXINTITEM_HXX
#include <svtools/intitem.hxx>
#endif
#ifndef _SFXENUMITEM_HXX
#include <svtools/eitem.hxx>
#endif
#ifndef _SFXAPP_HXX //autogen
#include <sfx2/app.hxx>
#endif
#ifndef _SFXDISPATCH_HXX //autogen
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SFX_OBJSH_HXX //autogen
#include <sfx2/objsh.hxx>
#endif
#ifndef _SFXVIEWSH_HXX //autogen
#include <sfx2/viewsh.hxx>
#endif
#ifndef _SFXTBXMGR_HXX //autogen
#include <sfx2/tbxmgr.hxx>
#endif

#include "svxids.hrc"
#include "grafctrl.hrc"
#include "dialogs.hrc"
#include "itemwin.hxx"
#include "dialmgr.hxx"
#include "grafctrl.hxx"

// -----------
// - Defines -
// -----------

#define SYMBOL_TO_FIELD_OFFSET  4

// -----------------------
// - ImplGrafMetricField -
// -----------------------

class ImplGrafMetricField : public MetricField
{
private:
    Timer           maTimer;
    USHORT          mnSID;
    SfxBindings&    mrBindings;

                    DECL_LINK( ImplModifyHdl, Timer* );

protected:

    virtual void    Modify();

public:

                    ImplGrafMetricField( Window* pParent, USHORT nSID, SfxBindings& rBindings );
                    ~ImplGrafMetricField();

    void            Update( const SfxPoolItem* pItem );
    USHORT          GetSID() const { return mnSID; }
};

// -----------------------------------------------------------------------------

ImplGrafMetricField::ImplGrafMetricField( Window* pParent, USHORT nSID, SfxBindings& rBindings ) :

    MetricField( pParent, WB_BORDER | WB_SPIN | WB_REPEAT | WB_3DLOOK ),

    mnSID       ( nSID ),
    mrBindings  ( rBindings )

{
    Size aSize( GetTextWidth( String::CreateFromAscii("-100 %") ), GetTextHeight() );

    aSize.Width() += 20, aSize.Height() += 6;
    SetSizePixel( aSize );

    if( SID_ATTR_GRAF_GAMMA == mnSID )
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
        const long nMinVal = ( SID_ATTR_GRAF_TRANSPARENCE == mnSID ) ? 0 : -100;

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

IMPL_LINK( ImplGrafMetricField, ImplModifyHdl, Timer*, pTimer )
{
    SfxPoolItem*    pItem;
    const long      nVal = GetValue();

    switch( mnSID )
    {
    case SID_ATTR_GRAF_RED:
    case SID_ATTR_GRAF_GREEN:
    case SID_ATTR_GRAF_BLUE:
    case SID_ATTR_GRAF_LUMINANCE:
    case SID_ATTR_GRAF_CONTRAST:
        pItem = new SfxInt16Item( mnSID, (short) nVal );
        break;

    case SID_ATTR_GRAF_GAMMA:
        pItem = new SfxUInt32Item( mnSID, nVal );
        break;

    case SID_ATTR_GRAF_TRANSPARENCE:
        pItem = new SfxUInt16Item( mnSID, (USHORT) nVal );
        break;

    default:
        pItem = 0;
        break;
    }

    if( pItem )
    {
        mrBindings.GetDispatcher()->Execute( mnSID, SFX_CALLMODE_RECORD, pItem, 0L );
        delete pItem;
    }
    return 0L;
}

// -----------------------------------------------------------------------------

void ImplGrafMetricField::Update( const SfxPoolItem* pItem )
{
    if( pItem )
    {
        long nValue;

        switch( mnSID )
        {
        case SID_ATTR_GRAF_TRANSPARENCE:
            nValue = ( (SfxUInt16Item*) pItem )->GetValue();
            break;

        case SID_ATTR_GRAF_GAMMA:
            nValue = ( (SfxUInt32Item*) pItem )->GetValue();
            break;

        default:
            nValue = ( (SfxInt16Item*) pItem )->GetValue();
            break;
        }

        SetValue( nValue );
    }
    else
        SetText( String() );
}

// --------------------
// - ImplGrafControl  -
// --------------------

static USHORT ImplGetRID( USHORT nSID )
{
    static const USHORT aImplSlotToResMap[][2] =
    {
        { SID_ATTR_GRAF_RED, RID_SVXIMG_GRAF_RED },
        { SID_ATTR_GRAF_GREEN, RID_SVXIMG_GRAF_GREEN },
        { SID_ATTR_GRAF_BLUE, RID_SVXIMG_GRAF_BLUE },
        { SID_ATTR_GRAF_LUMINANCE, RID_SVXIMG_GRAF_LUMINANCE },
        { SID_ATTR_GRAF_CONTRAST, RID_SVXIMG_GRAF_CONTRAST },
        { SID_ATTR_GRAF_GAMMA, RID_SVXIMG_GRAF_GAMMA },
        { SID_ATTR_GRAF_TRANSPARENCE, RID_SVXIMG_GRAF_TRANSPARENCE }
    };

    USHORT nRID = 0;

    for( int i = 0,
        nCount = sizeof( aImplSlotToResMap ) / ( sizeof( USHORT ) * 2 );
            i < nCount; i++ )
        if( aImplSlotToResMap[ i ][ 0 ] == nSID )
        {
            nRID = aImplSlotToResMap[ i ][ 1 ];
            break;
        }

    return nRID;
}

// -----------------------------------------------------------------------------

class ImplGrafControl : public Control
{
private:
    FixedImage              maImage;
    ImplGrafMetricField     maField;

protected:

    virtual void            GetFocus();

public:

                            ImplGrafControl( Window* pParent, USHORT nSID, SfxBindings& rBindings );
                            ~ImplGrafControl();

    void                    Update( const SfxPoolItem* pItem ) { maField.Update( pItem ); }
    void                    SetText( const String& rStr ) { maField.SetText( rStr ); }
};

// -----------------------------------------------------------------------------

ImplGrafControl::ImplGrafControl( Window* pParent, USHORT nSID, SfxBindings& rBindings ) :

    Control( pParent, WB_TABSTOP ),

    maImage     ( this ),
    maField     ( this, nSID, rBindings )

{
    ResId   aResId( ImplGetRID( nSID ), DIALOG_MGR() ) ;
    Image   aImage( aResId );
    Size    aImgSize( aImage.GetSizePixel() );
    Size    aFldSize( maField.GetSizePixel() );
    long    nFldY, nImgY;

    maImage.SetImage( aImage );
    maImage.SetSizePixel( aImgSize );

    if( aImgSize.Height() > aFldSize.Height() )
        nImgY = 0, nFldY = ( aImgSize.Height() - aFldSize.Height() ) >> 1;
    else
        nFldY = 0, nImgY = ( aFldSize.Height() - aImgSize.Height() ) >> 1;

    maImage.SetPosPixel( Point( 0, nImgY ) );
    maField.SetPosPixel( Point( aImgSize.Width() + SYMBOL_TO_FIELD_OFFSET, nFldY ) );
    SetSizePixel( Size( aImgSize.Width() + aFldSize.Width() + SYMBOL_TO_FIELD_OFFSET,
                  Max( aImgSize.Height(), aFldSize.Height() ) ) );

    maImage.SetHelpId( nSID );
    maImage.Show();

    maField.SetHelpId( nSID );
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
private:
    USHORT          mnCurPos;
    SfxBindings&    mrBindings;

    virtual void    Select();
    virtual long    PreNotify( NotifyEvent& rNEvt );
    virtual long    Notify( NotifyEvent& rNEvt );
    void            ImplReleaseFocus();

public:

                    ImplGrafModeControl( Window* pParent, SfxBindings& rBindings );
                    ~ImplGrafModeControl();

    void            Update( const SfxPoolItem* pItem );
};

// -----------------------------------------------------------------------------

ImplGrafModeControl::ImplGrafModeControl( Window* pParent, SfxBindings& rBindings ) :

    ListBox( pParent, WB_BORDER | WB_DROPDOWN | WB_AUTOHSCROLL ),

    mnCurPos    ( 0 ),
    mrBindings  ( rBindings )

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
        SfxUInt16Item aGrafModeItem( SID_ATTR_GRAF_MODE, GetSelectEntryPos() );
        mrBindings.GetDispatcher()->Execute( SID_ATTR_GRAF_MODE, SFX_CALLMODE_RECORD, &aGrafModeItem, 0L );
        ImplReleaseFocus();
    }
}

// -----------------------------------------------------------------------

long ImplGrafModeControl::PreNotify( NotifyEvent& rNEvt )
{
    USHORT nType = rNEvt.GetType();

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
    Window* pShellWnd = SfxViewShell::Current()->GetWindow();

    if( pShellWnd )
        pShellWnd->GrabFocus();
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

class ImplGrafFilterPopup : public SfxPopupWindow
{
private:

    SvxGrafFilterToolBoxControl*    mpParent;
    SfxToolBoxManager               maTbxMgr;
    ResId                           maResIdWin;
    ResId                           maResIdTbx;
    WindowAlign                     meTbxAlign;
    Link                            maSelectHdl;

                                    DECL_LINK( TbxSelectHdl, void* );

public:
                                    ImplGrafFilterPopup( USHORT nId, SvxGrafFilterToolBoxControl* pParent,
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

ImplGrafFilterPopup::ImplGrafFilterPopup( USHORT nId, SvxGrafFilterToolBoxControl* pParent,
                                          WindowAlign eAlign,
                                          const ResId& rResIdWin, const ResId& rResIdTbx,
                                          SfxBindings& rBindings ) :
    SfxPopupWindow  ( nId, rResIdWin, rBindings ),
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
    const USHORT nSlotId = maTbxMgr.GetToolBox().GetCurItemId();

    if( IsInPopupMode() )
        EndPopupMode();

    GetBindings().GetDispatcher()->Execute( nSlotId, SFX_CALLMODE_ASYNCHRON );

    return 0;
}

// -------------------------------
// - SvxGrafFilterToolBoxControl -
// -------------------------------

SFX_IMPL_TOOLBOX_CONTROL( SvxGrafFilterToolBoxControl, SfxEnumItem );

// -----------------------------------------------------------------------------

SvxGrafFilterToolBoxControl::SvxGrafFilterToolBoxControl( USHORT nId, ToolBox& rTbx, SfxBindings& rBind ) :
    SfxToolBoxControl( nId, rTbx, rBind )
{
}

// -----------------------------------------------------------------------------

SvxGrafFilterToolBoxControl::~SvxGrafFilterToolBoxControl()
{
}

// -----------------------------------------------------------------------------

void SvxGrafFilterToolBoxControl::StateChanged( USHORT nSID, SfxItemState eState, const SfxPoolItem* pState )
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
    ImplGrafFilterPopup* pWin = new ImplGrafFilterPopup( GetId(), this, GetToolBox().GetAlign(),
                                                         SVX_RES( RID_SVXTBX_GRFFILTER ),
                                                         SVX_RES( TBX_GRFFILTER ),
                                                         GetBindings() );
    pWin->StartPopupMode( &GetToolBox(), TRUE );
    pWin->StartSelection();
    pWin->Show();

    return pWin;
}

// -------------------------
// - SvxGrafToolBoxControl -
// -------------------------

SFX_IMPL_TOOLBOX_CONTROL( SvxGrafToolBoxControl, SfxVoidItem );

// -----------------------------------------------------------------------------

SvxGrafToolBoxControl::SvxGrafToolBoxControl( USHORT nId, ToolBox& rTbx, SfxBindings& rBind ) :
    SfxToolBoxControl   ( nId, rTbx, rBind )
{
    SfxApplication* pSfxApp = SFX_APP();
    SfxModule*      pModule = pSfxApp->GetActiveModule();

    if( pModule )
        StartListening( rBind, TRUE );
    else
        StartListening( *pSfxApp , TRUE );

}

// -----------------------------------------------------------------------------

SvxGrafToolBoxControl::~SvxGrafToolBoxControl()
{
}

// -----------------------------------------------------------------------------

void SvxGrafToolBoxControl::StateChanged( USHORT nSID, SfxItemState eState, const SfxPoolItem* pState )

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

void SvxGrafToolBoxControl::SFX_NOTIFY( SfxBroadcaster& rBC, const TypeId& rBCType,
                                        const SfxHint& rHint, const TypeId& rHintType )
{
}

// -----------------------------------------------------------------------------

Window* SvxGrafToolBoxControl::CreateItemWindow( Window *pParent )
{
    return( new ImplGrafControl( pParent, GetId(), GetBindings() ) );
}

// ----------------------------
// - SvxGrafRedToolBoxControl -
// ----------------------------

SFX_IMPL_TOOLBOX_CONTROL( SvxGrafRedToolBoxControl, SfxInt16Item );

// -----------------------------------------------------------------------------

SvxGrafRedToolBoxControl::SvxGrafRedToolBoxControl( USHORT nId, ToolBox& rTbx, SfxBindings& rBind ) :

    SvxGrafToolBoxControl( nId, rTbx, rBind )

{
}

// ------------------------------
// - SvxGrafGreenToolBoxControl -
// ------------------------------

SFX_IMPL_TOOLBOX_CONTROL( SvxGrafGreenToolBoxControl, SfxInt16Item );

// -----------------------------------------------------------------------------

SvxGrafGreenToolBoxControl::SvxGrafGreenToolBoxControl( USHORT nId, ToolBox& rTbx, SfxBindings& rBind ) :

    SvxGrafToolBoxControl( nId, rTbx, rBind )

{
}

// -----------------------------
// - SvxGrafBlueToolBoxControl -
// -----------------------------

SFX_IMPL_TOOLBOX_CONTROL( SvxGrafBlueToolBoxControl, SfxInt16Item );

// -----------------------------------------------------------------------------

SvxGrafBlueToolBoxControl::SvxGrafBlueToolBoxControl( USHORT nId, ToolBox& rTbx, SfxBindings& rBind ) :

    SvxGrafToolBoxControl( nId, rTbx, rBind )

{
}

// ----------------------------------
// - SvxGrafLuminanceToolBoxControl -
// ----------------------------------

SFX_IMPL_TOOLBOX_CONTROL( SvxGrafLuminanceToolBoxControl, SfxInt16Item );

// -----------------------------------------------------------------------------

SvxGrafLuminanceToolBoxControl::SvxGrafLuminanceToolBoxControl( USHORT nId, ToolBox& rTbx,
                                                                SfxBindings& rBind ) :

    SvxGrafToolBoxControl( nId, rTbx, rBind )

{
}

// ----------------------------------
// - SvxGrafContrastToolBoxControl -
// ----------------------------------

SFX_IMPL_TOOLBOX_CONTROL( SvxGrafContrastToolBoxControl, SfxInt16Item );

// -----------------------------------------------------------------------------

SvxGrafContrastToolBoxControl::SvxGrafContrastToolBoxControl( USHORT nId, ToolBox& rTbx,
                                                              SfxBindings& rBind ) :

    SvxGrafToolBoxControl( nId, rTbx, rBind )

{
}

// ------------------------------
// - SvxGrafGammaToolBoxControl -
// ------------------------------

SFX_IMPL_TOOLBOX_CONTROL( SvxGrafGammaToolBoxControl, SfxUInt32Item );

// -----------------------------------------------------------------------------

SvxGrafGammaToolBoxControl::SvxGrafGammaToolBoxControl( USHORT nId, ToolBox& rTbx, SfxBindings& rBind ) :

    SvxGrafToolBoxControl( nId, rTbx, rBind )

{
}

// -------------------------------------
// - SvxGrafTransparenceToolBoxControl -
// -------------------------------------

SFX_IMPL_TOOLBOX_CONTROL( SvxGrafTransparenceToolBoxControl, SfxUInt16Item );

// -----------------------------------------------------------------------------

SvxGrafTransparenceToolBoxControl::SvxGrafTransparenceToolBoxControl( USHORT nId, ToolBox& rTbx,
                                                                      SfxBindings& rBind ) :

    SvxGrafToolBoxControl( nId, rTbx, rBind )

{
}

// -----------------------------
// - SvxGrafModeToolBoxControl -
// -----------------------------

SFX_IMPL_TOOLBOX_CONTROL( SvxGrafModeToolBoxControl, SfxUInt16Item );

// -----------------------------------------------------------------------------

SvxGrafModeToolBoxControl::SvxGrafModeToolBoxControl( USHORT nId, ToolBox& rTbx, SfxBindings& rBind ) :

    SfxToolBoxControl( nId, rTbx, rBind )

{
    SfxApplication* pSfxApp = SFX_APP();
    SfxModule*      pModule = pSfxApp->GetActiveModule();

    if( pModule )
        StartListening( rBind, TRUE );
    else
        StartListening( *pSfxApp , TRUE );
}

// -----------------------------------------------------------------------------

SvxGrafModeToolBoxControl::~SvxGrafModeToolBoxControl()
{
}

// -----------------------------------------------------------------------------

void SvxGrafModeToolBoxControl::StateChanged( USHORT nSID, SfxItemState eState, const SfxPoolItem* pState )

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

void SvxGrafModeToolBoxControl::SFX_NOTIFY( SfxBroadcaster& rBC, const TypeId& rBCType,
                                            const SfxHint& rHint, const TypeId& rHintType )
{
}

// -----------------------------------------------------------------------------

Window* SvxGrafModeToolBoxControl::CreateItemWindow( Window *pParent )
{
    return( new ImplGrafModeControl( pParent, GetBindings() ) );
}


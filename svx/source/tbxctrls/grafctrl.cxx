/*************************************************************************
 *
 *  $RCSfile: grafctrl.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: ka $ $Date: 2000-11-26 15:14:03 $
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

#define ITEMID_SIZE 0
#define ITEMID_BRUSH 0
#define ITEMID_GRF_CROP SID_ATTR_GRAF_CROP

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
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef _SFXINTITEM_HXX
#include <svtools/intitem.hxx>
#endif
#ifndef _SFXENUMITEM_HXX
#include <svtools/eitem.hxx>
#endif
#ifndef _SFX_WHITER_HXX
#include <svtools/whiter.hxx>
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
#ifndef _SFXREQUEST_HXX
#include <sfx2/request.hxx>
#endif
#ifndef _BASEDLGS_HXX
#include <sfx2/basedlgs.hxx>
#endif

#include "svxids.hrc"
#include "grafctrl.hrc"
#include "dialogs.hrc"
#include "brshitem.hxx"
#include "sizeitem.hxx"
#include "sdgcpitm.hxx"
#include "../dialog/grfpage.hxx"
#include "itemwin.hxx"
#include "dialmgr.hxx"
#include "svdview.hxx"
#include "svdmodel.hxx"
#include "svdograf.hxx"
#include "grafctrl.hxx"

// -----------
// - Defines -
// -----------

#define SYMBOL_TO_FIELD_OFFSET      4
#define ITEMVALUE(ItemSet,Id,Cast)  ((const Cast&)(ItemSet).Get(Id)).GetValue()

// ----------------
// - TbxImageItem -
// ----------------

TYPEINIT1( TbxImageItem, SfxUInt16Item );

//---------------------------------------------------------

TbxImageItem::TbxImageItem( USHORT nWhich, UINT16 nImage ) :
    SfxUInt16Item( nWhich, nImage )
{
}

//---------------------------------------------------------

SfxPoolItem* TbxImageItem::Clone( SfxItemPool* pPool ) const
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

SFX_IMPL_TOOLBOX_CONTROL( SvxGrafFilterToolBoxControl, TbxImageItem );

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

// ---------------------
// - SvxGrafAttrHelper -
// ---------------------

void SvxGrafAttrHelper::ExecuteGrafAttr( SfxRequest& rReq, SdrView& rView )
{
    SfxItemPool&    rPool = rView.GetModel()->GetItemPool();
    SfxItemSet      aSet( rPool, SDRATTR_GRAF_FIRST, SDRATTR_GRAF_LAST-1 );

    String              aUndoStr( rView.GetMarkDescription() );
    const SfxItemSet*   pArgs = rReq.GetArgs();
    const SfxPoolItem*  pItem;
    USHORT              nSlot = rReq.GetSlot();
    BOOL                bGeometryChanged = FALSE;

    if( !pArgs || SFX_ITEM_SET != pArgs->GetItemState( nSlot, FALSE, &pItem ))
        pItem = 0;

    aUndoStr.Append( sal_Unicode(' ') );

    switch( nSlot )
    {
        case SID_ATTR_GRAF_RED:
        {
            if( pItem )
            {
                aSet.Put( SdrGrafRedItem( ((SfxInt16Item*)pItem)->GetValue() ));
                aUndoStr.Append( String( SVX_RESSTR( RID_SVXSTR_UNDO_GRAFRED ) ) );
            }
        }
        break;

        case SID_ATTR_GRAF_GREEN:
        {
            if( pItem )
            {
                aSet.Put( SdrGrafGreenItem( ((SfxInt16Item*)pItem)->GetValue() ));
                aUndoStr.Append( String( SVX_RESSTR( RID_SVXSTR_UNDO_GRAFGREEN ) ) );
            }
        }
        break;

        case SID_ATTR_GRAF_BLUE:
        {
            if( pItem )
            {
                aSet.Put( SdrGrafBlueItem( ((SfxInt16Item*)pItem)->GetValue() ));
                aUndoStr.Append( String( SVX_RESSTR( RID_SVXSTR_UNDO_GRAFBLUE ) ) );
            }
        }
        break;

        case SID_ATTR_GRAF_LUMINANCE:
        {
            if( pItem )
            {
                aSet.Put( SdrGrafLuminanceItem( ((SfxInt16Item*)pItem)->GetValue() ));
                aUndoStr.Append( String( SVX_RESSTR( RID_SVXSTR_UNDO_GRAFLUMINANCE ) ) );
            }
        }
        break;

        case SID_ATTR_GRAF_CONTRAST:
        {
            if( pItem )
            {
                aSet.Put( SdrGrafContrastItem( ((SfxInt16Item*)pItem)->GetValue() ));
                aUndoStr.Append( String( SVX_RESSTR( RID_SVXSTR_UNDO_GRAFCONTRAST ) ) );
            }
        }
        break;

        case SID_ATTR_GRAF_GAMMA:
        {
            if( pItem )
            {
                aSet.Put( SdrGrafGamma100Item( ((SfxUInt32Item*)pItem)->GetValue() ));
                aUndoStr.Append( String( SVX_RESSTR( RID_SVXSTR_UNDO_GRAFGAMMA ) ) );
            }
        }
        break;

        case SID_ATTR_GRAF_TRANSPARENCE:
        {
            if( pItem )
            {
                aSet.Put( SdrGrafTransparenceItem( ((SfxUInt16Item*)pItem)->GetValue() ));
                aUndoStr.Append( String( SVX_RESSTR( RID_SVXSTR_UNDO_GRAFTRANSPARENCY ) ) );
            }
        }
        break;

        case SID_ATTR_GRAF_MODE:
        {
            if( pItem )
            {
                aSet.Put( SdrGrafModeItem( (GraphicDrawMode) ((SfxUInt16Item*)pItem)->GetValue() ));
                aUndoStr.Append( String( SVX_RESSTR( RID_SVXSTR_UNDO_GRAFMODE ) ) );
            }
        }
        break;

        case( SID_ATTR_GRAF_CROP ):
        {
            SdrGrafObj* pObj = (SdrGrafObj*) rView.GetMarkList().GetMark( 0 )->GetObj();

            if( pObj && pObj->ISA( SdrGrafObj ) )
            {
                SfxItemSet          aGrfAttr( rPool, SDRATTR_GRAFCROP, SDRATTR_GRAFCROP, 0 );
                const SfxMapUnit    eOldMetric = rPool.GetMetric( 0 );
                const MapMode       aMap100( MAP_100TH_MM );
                const MapMode       aMapTwip( MAP_TWIP );

                aGrfAttr.Put(pObj->GetItemSet());
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
                                                     Size( 100000, 100000 ), aMap100, aMapTwip ) ) ) );
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
                                                 aCropDlgAttr, 950, FALSE );
                const String        aCropStr( SVX_RESSTR( RID_SVXSTR_GRAFCROP ) );
                SfxTabPage*         pTabPage = SvxGrfCropPage::Create( &aCropDialog, aCropDlgAttr );

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

                            aNewOrigin.X() -= ( aNewGrfSize.Width() - aOldGrfSize.Width() ) >> 1;
                            aNewOrigin.Y() -= ( aNewGrfSize.Height() - aOldGrfSize.Height() ) >> 1;

                            const Rectangle aNewRect( aNewOrigin, aNewGrfSize );

                            if( !aSet.Count() )
                                rView.SetMarkedObjRect( aNewRect );
                            else
                                pObj->SetSnapRect( aNewRect );
                        }
                    }
                }

                rPool.SetDefaultMetric( eOldMetric );
            }
        }
        break;

        default:
            break;
    }

    if( aSet.Count() )
    {
        rView.BegUndo( aUndoStr );
        rView.SetAttributes( aSet );
        rView.EndUndo();
    }
}

// -----------------------------------------------------------------------------

void SvxGrafAttrHelper::GetGrafAttrState( SfxItemSet& rSet, SdrView& rView )
{
    SfxItemPool&    rPool = rView.GetModel()->GetItemPool();
    SfxItemSet      aAttrSet( rPool );
    SfxWhichIter    aIter( rSet );
    USHORT          nWhich = aIter.FirstWhich();

    rView.GetAttributes( aAttrSet );

    while( nWhich )
    {
        USHORT nSlotId = SfxItemPool::IsWhich( nWhich ) ? rPool.GetSlotId( nWhich ) : nWhich;

        switch( nSlotId )
        {
            case( SID_ATTR_GRAF_MODE ):
            {
                if( SFX_ITEM_AVAILABLE <= aAttrSet.GetItemState( SDRATTR_GRAFMODE ) )
                {
                    rSet.Put( SfxUInt16Item( nSlotId,
                        ITEMVALUE( aAttrSet, SDRATTR_GRAFMODE, SdrGrafModeItem ) ) );
                }
            }
            break;

            case( SID_ATTR_GRAF_RED ):
            {
                if( SFX_ITEM_AVAILABLE <= aAttrSet.GetItemState( SDRATTR_GRAFRED ) )
                {
                    rSet.Put( SfxInt16Item( nSlotId,
                        ITEMVALUE( aAttrSet, SDRATTR_GRAFRED, SdrGrafRedItem ) ) );
                }
            }
            break;

            case( SID_ATTR_GRAF_GREEN ):
            {
                if( SFX_ITEM_AVAILABLE <= aAttrSet.GetItemState( SDRATTR_GRAFGREEN ) )
                {
                    rSet.Put( SfxInt16Item( nSlotId,
                        ITEMVALUE( aAttrSet, SDRATTR_GRAFGREEN, SdrGrafGreenItem ) ) );
                }
            }
            break;

            case( SID_ATTR_GRAF_BLUE ):
            {
                if( SFX_ITEM_AVAILABLE <= aAttrSet.GetItemState( SDRATTR_GRAFBLUE ) )
                {
                    rSet.Put( SfxInt16Item( nSlotId,
                        ITEMVALUE( aAttrSet, SDRATTR_GRAFBLUE, SdrGrafBlueItem ) ) );
                }
            }
            break;

            case( SID_ATTR_GRAF_LUMINANCE ):
            {
                if( SFX_ITEM_AVAILABLE <= aAttrSet.GetItemState( SDRATTR_GRAFLUMINANCE ) )
                {
                    rSet.Put( SfxInt16Item( nSlotId,
                        ITEMVALUE( aAttrSet, SDRATTR_GRAFLUMINANCE, SdrGrafLuminanceItem ) ) );
                }
            }
            break;

            case( SID_ATTR_GRAF_CONTRAST ):
            {
                if( SFX_ITEM_AVAILABLE <= aAttrSet.GetItemState( SDRATTR_GRAFCONTRAST ) )
                {
                    rSet.Put( SfxInt16Item( nSlotId,
                        ITEMVALUE( aAttrSet, SDRATTR_GRAFCONTRAST, SdrGrafContrastItem ) ) );
                }
            }
            break;

            case( SID_ATTR_GRAF_GAMMA ):
            {
                if( SFX_ITEM_AVAILABLE <= aAttrSet.GetItemState( SDRATTR_GRAFGAMMA ) )
                {
                    rSet.Put( SfxUInt32Item( nSlotId,
                        ITEMVALUE( aAttrSet, SDRATTR_GRAFGAMMA, SdrGrafGamma100Item ) ) );
                }
            }
            break;

            case( SID_ATTR_GRAF_TRANSPARENCE ):
            {
                if( SFX_ITEM_AVAILABLE <= aAttrSet.GetItemState( SDRATTR_GRAFTRANSPARENCE ) )
                {
                    const SdrMarkList&  rMarkList = rView.GetMarkList();
                    BOOL                bEnable = TRUE;

                    for( USHORT i = 0, nCount = (USHORT) rMarkList.GetMarkCount();
                         ( i < nCount ) && bEnable; i++ )
                    {
                        SdrObject* pObj = rMarkList.GetMark( 0 )->GetObj();

                        if( !pObj || !pObj->ISA( SdrGrafObj ) ||
                            ( (SdrGrafObj*) pObj )->HasGDIMetaFile() ||
                            ( (SdrGrafObj*) pObj )->IsAnimated() )
                        {
                            bEnable = FALSE;
                        }
                    }

                    if( bEnable )
                        rSet.Put( SfxUInt16Item( nSlotId,
                            ITEMVALUE( aAttrSet, SDRATTR_GRAFTRANSPARENCE, SdrGrafTransparenceItem ) ) );
                    else
                        rSet.DisableItem( SID_ATTR_GRAF_TRANSPARENCE );
                }
            }
            break;

            case( SID_ATTR_GRAF_CROP ):
            {
                const SdrMarkList&  rMarkList = rView.GetMarkList();
                BOOL                bDisable = TRUE;

                if( 1 == rMarkList.GetMarkCount() )
                {
                    SdrObject* pObj = rMarkList.GetMark( 0 )->GetObj();

                    if( pObj && pObj->ISA( SdrGrafObj ) )
                        bDisable = FALSE;
                }

                if( bDisable )
                    rSet.DisableItem( SID_ATTR_GRAF_CROP );
            }
            break;

            default:
            break;
        }

        nWhich = aIter.NextWhich();
    }
}

/*************************************************************************
 *
 *  $RCSfile: grafctrl.cxx,v $
 *
 *  $Revision: 1.18 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-03 13:22:38 $
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
#include <tools/urlobj.hxx>
#include <comphelper/processfactory.hxx>

#include "svxids.hrc"
#include "grafctrl.hrc"
#include "dialogs.hrc"
#include "brshitem.hxx"
#include "sizeitem.hxx"
#include "sdgcpitm.hxx"
//CHINA001 #include "../dialog/grfpage.hxx"
#include "itemwin.hxx"
#include "dialmgr.hxx"
#include "svdview.hxx"
#include "svdmodel.hxx"
#include "svdograf.hxx"
#include "svdundo.hxx"
#include "svdtrans.hxx"
#include "grafctrl.hxx"

// namespaces
using namespace ::rtl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;

#include "svxdlg.hxx" //CHINA001
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

IMPL_LINK( ImplGrafMetricField, ImplModifyHdl, Timer*, pTimer )
{
    const long nVal = GetValue();

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
    USHORT      nResId;
    USHORT      nHCResId;
};

static USHORT ImplGetRID( const OUString& aCommand, bool bHighContrast )
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

    USHORT nRID = 0;

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
private:
    FixedImage              maImage;
    ImplGrafMetricField     maField;

protected:

    virtual void            GetFocus();

public:

                            ImplGrafControl( Window* pParent, USHORT nSlotId, const rtl::OUString& rCmd, const Reference< XFrame >& rFrame );
                            ~ImplGrafControl();

    void                    Update( const SfxPoolItem* pItem ) { maField.Update( pItem ); }
    void                    SetText( const String& rStr ) { maField.SetText( rStr ); }
};

// -----------------------------------------------------------------------------

ImplGrafControl::ImplGrafControl( Window* pParent, USHORT nSlotId, const rtl::OUString& rCmd, const Reference< XFrame >& rFrame ) :
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

    if( aImgSize.Height() > aFldSize.Height() )
        nImgY = 0, nFldY = ( aImgSize.Height() - aFldSize.Height() ) >> 1;
    else
        nFldY = 0, nImgY = ( aFldSize.Height() - aImgSize.Height() ) >> 1;

    maImage.SetPosPixel( Point( 0, nImgY ) );
    maField.SetPosPixel( Point( aImgSize.Width() + SYMBOL_TO_FIELD_OFFSET, nFldY ) );
    SetSizePixel( Size( aImgSize.Width() + aFldSize.Width() + SYMBOL_TO_FIELD_OFFSET,
                  Max( aImgSize.Height(), aFldSize.Height() ) ) );

    maImage.Show();

    maField.SetHelpId( nSlotId );
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
    USHORT              mnCurPos;
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

        SfxToolBoxControl::Dispatch(
            Reference< XDispatchProvider >( mxFrame->getController(), UNO_QUERY ),
            OUString( RTL_CONSTASCII_USTRINGPARAM( ".uno:GrafMode" )),
            aArgs );

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
    const USHORT nSlotId = maTbxMgr.GetToolBox().GetCurItemId();

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

SvxGrafFilterToolBoxControl::SvxGrafFilterToolBoxControl( USHORT nSlotId, USHORT nId, ToolBox& rTbx ) :
    SfxToolBoxControl( nSlotId, nId, rTbx )
{
    rTbx.SetItemBits( nId, TIB_DROPDOWN | rTbx.GetItemBits( nId ) );
    rTbx.Invalidate();
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
/*  CD!!
    ImplGrafFilterPopup* pWin = new ImplGrafFilterPopup( GetId(), this, GetToolBox().GetAlign(),
                                                         SVX_RES( RID_SVXTBX_GRFFILTER ),
                                                         SVX_RES( TBX_GRFFILTER ),
                                                         GetBindings() );
    pWin->StartPopupMode( &GetToolBox(), TRUE );
    pWin->StartSelection();
    pWin->Show();

    return pWin;
*/
    return NULL;
}

// -------------------------
// - SvxGrafToolBoxControl -
// -------------------------

SFX_IMPL_TOOLBOX_CONTROL( SvxGrafToolBoxControl, SfxVoidItem );

// -----------------------------------------------------------------------------

SvxGrafToolBoxControl::SvxGrafToolBoxControl( USHORT nSlotId, USHORT nId, ToolBox& rTbx) :
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

Window* SvxGrafToolBoxControl::CreateItemWindow( Window *pParent )
{
    return( new ImplGrafControl( pParent, GetSlotId(), m_aCommandURL, m_xFrame ) );
}

// ----------------------------
// - SvxGrafRedToolBoxControl -
// ----------------------------

SFX_IMPL_TOOLBOX_CONTROL( SvxGrafRedToolBoxControl, SfxInt16Item );

// -----------------------------------------------------------------------------

SvxGrafRedToolBoxControl::SvxGrafRedToolBoxControl( USHORT nSlotId, USHORT nId, ToolBox& rTbx ) :
    SvxGrafToolBoxControl( nSlotId, nId, rTbx )
{
}

// ------------------------------
// - SvxGrafGreenToolBoxControl -
// ------------------------------

SFX_IMPL_TOOLBOX_CONTROL( SvxGrafGreenToolBoxControl, SfxInt16Item );

// -----------------------------------------------------------------------------

SvxGrafGreenToolBoxControl::SvxGrafGreenToolBoxControl( USHORT nSlotId, USHORT nId, ToolBox& rTbx ) :
    SvxGrafToolBoxControl( nSlotId, nId, rTbx )
{
}

// -----------------------------
// - SvxGrafBlueToolBoxControl -
// -----------------------------

SFX_IMPL_TOOLBOX_CONTROL( SvxGrafBlueToolBoxControl, SfxInt16Item );

// -----------------------------------------------------------------------------

SvxGrafBlueToolBoxControl::SvxGrafBlueToolBoxControl( USHORT nSlotId, USHORT nId, ToolBox& rTbx ) :
    SvxGrafToolBoxControl( nSlotId, nId, rTbx )
{
}

// ----------------------------------
// - SvxGrafLuminanceToolBoxControl -
// ----------------------------------

SFX_IMPL_TOOLBOX_CONTROL( SvxGrafLuminanceToolBoxControl, SfxInt16Item );

// -----------------------------------------------------------------------------

SvxGrafLuminanceToolBoxControl::SvxGrafLuminanceToolBoxControl( USHORT nSlotId, USHORT nId, ToolBox& rTbx ) :
    SvxGrafToolBoxControl( nSlotId, nId, rTbx )
{
}

// ----------------------------------
// - SvxGrafContrastToolBoxControl -
// ----------------------------------

SFX_IMPL_TOOLBOX_CONTROL( SvxGrafContrastToolBoxControl, SfxInt16Item );

// -----------------------------------------------------------------------------

SvxGrafContrastToolBoxControl::SvxGrafContrastToolBoxControl( USHORT nSlotId, USHORT nId, ToolBox& rTbx ) :
    SvxGrafToolBoxControl( nSlotId, nId, rTbx )
{
}

// ------------------------------
// - SvxGrafGammaToolBoxControl -
// ------------------------------

SFX_IMPL_TOOLBOX_CONTROL( SvxGrafGammaToolBoxControl, SfxUInt32Item );

// -----------------------------------------------------------------------------

SvxGrafGammaToolBoxControl::SvxGrafGammaToolBoxControl( USHORT nSlotId, USHORT nId, ToolBox& rTbx ) :
    SvxGrafToolBoxControl( nSlotId, nId, rTbx )
{
}

// -------------------------------------
// - SvxGrafTransparenceToolBoxControl -
// -------------------------------------

SFX_IMPL_TOOLBOX_CONTROL( SvxGrafTransparenceToolBoxControl, SfxUInt16Item );

// -----------------------------------------------------------------------------

SvxGrafTransparenceToolBoxControl::SvxGrafTransparenceToolBoxControl( USHORT nSlotId, USHORT nId, ToolBox& rTbx ) :
    SvxGrafToolBoxControl( nSlotId, nId, rTbx )
{
}

// -----------------------------
// - SvxGrafModeToolBoxControl -
// -----------------------------

SFX_IMPL_TOOLBOX_CONTROL( SvxGrafModeToolBoxControl, SfxUInt16Item );

// -----------------------------------------------------------------------------

SvxGrafModeToolBoxControl::SvxGrafModeToolBoxControl( USHORT nSlotId, USHORT nId, ToolBox& rTbx ) :
    SfxToolBoxControl( nSlotId, nId, rTbx )
{
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
    SfxItemSet      aSet( rPool, SDRATTR_GRAF_FIRST, SDRATTR_GRAF_LAST-1 );

    String              aUndoStr( rView.GetDescriptionOfMarkedObjects() );
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
            SdrGrafObj* pObj = (SdrGrafObj*) rView.GetMarkedObjectList().GetMark( 0 )->GetObj();

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
                                rView.BegUndo( aUndoStr );
                                rView.AddUndo( new SdrUndoGeoObj( *pObj ) );
                                pObj->SetSnapRect( aNewRect );
                                rView.SetAttributes( aSet );
                                rView.EndUndo();
                                aSet.ClearItem();
                            }
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
                    const SdrMarkList&  rMarkList = rView.GetMarkedObjectList();
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
                const SdrMarkList&  rMarkList = rView.GetMarkedObjectList();
                BOOL                bDisable = TRUE;

                if( 1 == rMarkList.GetMarkCount() )
                {
                    SdrObject* pObj = rMarkList.GetMark( 0 )->GetObj();

                    if( pObj && pObj->ISA( SdrGrafObj ) )
                    {
                        SdrGrafObj* pGrafObj = (SdrGrafObj*) pObj;

                        if( ( pGrafObj->GetGraphicType() != GRAPHIC_NONE ) &&
                            ( pGrafObj->GetGraphicType() != GRAPHIC_DEFAULT ) )
                        {
                            bDisable = FALSE;
                        }
                    }
                }

                if( bDisable )
                    rSet.DisableItem( nSlotId );
            }
            break;

            default:
            break;
        }

        nWhich = aIter.NextWhich();
    }
}

/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dialog.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 17:51:15 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _SV_DIALOG_HXX
#define _SV_DIALOG_HXX

#ifndef _SV_SV_H
#include <vcl/sv.h>
#endif

#ifndef _VCL_DLLAPI_H
#include <vcl/dllapi.h>
#endif

#ifndef _SV_SYSWIN_HXX
#include <vcl/syswin.hxx>
#endif

// parameter to pass to the dialogue constructor if really no parent is wanted
// whereas NULL chooses the deafult dialogue parent
#define DIALOG_NO_PARENT ((Window*)0xffffffff)

// ----------
// - Dialog -
// ----------

struct DialogImpl;

class VCL_DLLPUBLIC Dialog : public SystemWindow
{
private:
    Window*         mpDialogParent;
    Dialog*         mpPrevExecuteDlg;
    DialogImpl*     mpDialogImpl;
    long            mnMousePositioned;
    BOOL            mbInExecute;
    BOOL            mbOldSaveBack;
    BOOL            mbInClose;
    BOOL            mbModalMode;

    SAL_DLLPRIVATE void    ImplInitDialogData();
    SAL_DLLPRIVATE void    ImplInitSettings();

//#if 0 // _SOLAR__PRIVATE
    // Copy assignment is forbidden and not implemented.
    SAL_DLLPRIVATE         Dialog (const Dialog &);
    SAL_DLLPRIVATE         Dialog & operator= (const Dialog &);

protected:
    using Window::ImplInit;
    SAL_DLLPRIVATE void    ImplInit( Window* pParent, WinBits nStyle );
    SAL_DLLPRIVATE void    ImplDialogRes( const ResId& rResId );
    SAL_DLLPRIVATE void    ImplCenterDialog();

public:
    SAL_DLLPRIVATE BOOL    IsInClose() const { return mbInClose; }
//#endif

protected:
                    Dialog( WindowType nType );
    virtual void    Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize, ULONG nFlags );

public:
                    Dialog( Window* pParent, WinBits nStyle = WB_STDDIALOG );
                    Dialog( Window* pParent, const ResId& rResId );
    virtual         ~Dialog();

    virtual long    Notify( NotifyEvent& rNEvt );
    virtual void    StateChanged( StateChangedType nStateChange );
    virtual void    DataChanged( const DataChangedEvent& rDCEvt );

    virtual BOOL    Close();

    virtual short   Execute();
    BOOL            IsInExecute() const { return mbInExecute; }

    ////////////////////////////////////////
    // Dialog::Execute replacement API
public:
    // Link impl: DECL_LINK( MyEndDialogHdl, Dialog* ); <= param is dialog just ended
    virtual void    StartExecuteModal( const Link& rEndDialogHdl );
    BOOL            IsStartedModal() const;
    long            GetResult() const;
private:
    BOOL            ImplStartExecuteModal();
    void            ImplEndExecuteModal();
public:

    // Dialog::Execute replacement API
    ////////////////////////////////////////

    void            EndDialog( long nResult = 0 );
    static void     EndAllDialogs( Window* pParent=NULL );

    void            GetDrawWindowBorder( sal_Int32& rLeftBorder, sal_Int32& rTopBorder,
                                         sal_Int32& rRightBorder, sal_Int32& rBottomBorder ) const;

    void            SetModalInputMode( BOOL bModal );
    void            SetModalInputMode( BOOL bModal, BOOL bSubModalDialogs );
    BOOL            IsModalInputMode() const { return mbModalMode; }

    void            GrabFocusToFirstControl();
};

// ------------------
// - ModelessDialog -
// ------------------

class VCL_DLLPUBLIC ModelessDialog : public Dialog
{
//#if 0 // _SOLAR__PRIVATE
    // Copy assignment is forbidden and not implemented.
    SAL_DLLPRIVATE         ModelessDialog (const ModelessDialog &);
    SAL_DLLPRIVATE         ModelessDialog & operator= (const ModelessDialog &);
//#endif

public:
                    ModelessDialog( Window* pParent, WinBits nStyle = WB_STDMODELESS );
                    ModelessDialog( Window* pParent, const ResId& rResId );
};

// ---------------
// - ModalDialog -
// ---------------

class VCL_DLLPUBLIC ModalDialog : public Dialog
{
public:
                    ModalDialog( Window* pParent, WinBits nStyle = WB_STDMODAL );
                    ModalDialog( Window* pParent, const ResId& rResId );

private:
    using Window::Show;
    void            Show( BOOL bVisible = TRUE );
    using Window::Hide;
    void            Hide();

//#if 0 // _SOLAR__PRIVATE
    // Copy assignment is forbidden and not implemented.
    SAL_DLLPRIVATE         ModalDialog (const ModalDialog &);
    SAL_DLLPRIVATE         ModalDialog & operator= (const ModalDialog &);
//#endif
};

#endif  // _SV_DIALOG_HXX

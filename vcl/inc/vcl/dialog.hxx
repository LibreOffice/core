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

#ifndef _SV_DIALOG_HXX
#define _SV_DIALOG_HXX

#include <tools/solar.h>
#include <vcl/dllapi.h>
#include <vcl/builder.hxx>
#include <vcl/syswin.hxx>
#include <vcl/timer.hxx>

// parameter to pass to the dialog constructor if really no parent is wanted
// whereas NULL chooses the default dialog parent
#define DIALOG_NO_PARENT ((Window*)0xffffffff)

// ----------
// - Dialog -
// ----------
struct DialogImpl;
class VclBox;
class VclButtonBox;
class VclContainer;

class VCL_DLLPUBLIC Dialog
    : public SystemWindow
    , public VclBuilderContainer
{
private:
    Window*         mpDialogParent;
    Dialog*         mpPrevExecuteDlg;
    DialogImpl*     mpDialogImpl;
    long            mnMousePositioned;
    sal_Bool            mbInExecute;
    sal_Bool            mbOldSaveBack;
    sal_Bool            mbInClose;
    sal_Bool            mbModalMode;
    bool            mbIsDefferedInit;
    bool            mbIsCalculatingInitialLayoutSize;
    Timer           maLayoutTimer;
    VclButtonBox*   mpActionArea;
    VclBox*         mpContentArea;

    SAL_DLLPRIVATE void    ImplInitDialogData();
    SAL_DLLPRIVATE void    ImplInitSettings();

    // Copy assignment is forbidden and not implemented.
    SAL_DLLPRIVATE         Dialog (const Dialog &);
    SAL_DLLPRIVATE         Dialog & operator= (const Dialog &);

    DECL_DLLPRIVATE_LINK( ImplAsyncCloseHdl, void* );
    DECL_DLLPRIVATE_LINK( ImplHandleLayoutTimerHdl, void* );

protected:
    using Window::ImplInit;
    SAL_DLLPRIVATE void    ImplInit( Window* pParent, WinBits nStyle );
    SAL_DLLPRIVATE void    ImplDialogRes( const ResId& rResId );
    SAL_DLLPRIVATE WinBits init(Window *pParent, const ResId& rResId);

    SAL_DLLPRIVATE void    setPosSizeOnContainee(Size aSize, VclContainer &rBox);
public:
    SAL_DLLPRIVATE sal_Bool    IsInClose() const { return mbInClose; }
    SAL_DLLPRIVATE bool hasPendingLayout() const { return maLayoutTimer.IsActive(); }
    SAL_DLLPRIVATE void doDeferredInit(bool bResizable);
    SAL_DLLPRIVATE bool isDeferredInit() const { return mbIsDefferedInit; }

protected:
                    Dialog( WindowType nType );
                    Dialog( Window* pParent, const rtl::OString& rID, const rtl::OUString& rUIXMLDescription, WindowType nType );
    virtual void    Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize, sal_uLong nFlags );

protected:
    friend class VclBuilder;
    void set_action_area(VclButtonBox *pBox);
    void set_content_area(VclBox *pBox);

public:
                    Dialog( Window* pParent, WinBits nStyle = WB_STDDIALOG );
                    Dialog( Window* pParent, const rtl::OString& rID, const rtl::OUString& rUIXMLDescription );
    virtual         ~Dialog();

    virtual long    Notify( NotifyEvent& rNEvt );
    virtual void    StateChanged( StateChangedType nStateChange );
    virtual void    DataChanged( const DataChangedEvent& rDCEvt );

    virtual Size    GetOptimalSize() const;
    virtual void    Resize();
    bool            isLayoutEnabled() const;
    void            setOptimalLayoutSize();
    bool            isCalculatingInitialLayoutSize() const { return mbIsCalculatingInitialLayoutSize; }
    virtual void queue_layout();
    virtual bool set_property(const rtl::OString &rKey, const rtl::OString &rValue);
    VclButtonBox* get_action_area();
    VclBox* get_content_area();

    virtual sal_Bool    Close();

    virtual short   Execute();
    sal_Bool            IsInExecute() const { return mbInExecute; }

    ////////////////////////////////////////
    // Dialog::Execute replacement API
public:
    // Link impl: DECL_LINK( MyEndDialogHdl, Dialog* ); <= param is dialog just ended
    virtual void    StartExecuteModal( const Link& rEndDialogHdl );
    long            GetResult() const;
private:
    sal_Bool            ImplStartExecuteModal();
    void            ImplEndExecuteModal();
public:

    // Dialog::Execute replacement API
    ////////////////////////////////////////

    void            EndDialog( long nResult = 0 );
    static void     EndAllDialogs( Window* pParent=NULL );

    void            GetDrawWindowBorder( sal_Int32& rLeftBorder, sal_Int32& rTopBorder,
                                         sal_Int32& rRightBorder, sal_Int32& rBottomBorder ) const;

    void            SetModalInputMode( sal_Bool bModal );
    void            SetModalInputMode( sal_Bool bModal, sal_Bool bSubModalDialogs );
    sal_Bool            IsModalInputMode() const { return mbModalMode; }

    void            GrabFocusToFirstControl();
};

// ------------------
// - ModelessDialog -
// ------------------

class VCL_DLLPUBLIC ModelessDialog : public Dialog
{
    // Copy assignment is forbidden and not implemented.
    SAL_DLLPRIVATE         ModelessDialog (const ModelessDialog &);
    SAL_DLLPRIVATE         ModelessDialog & operator= (const ModelessDialog &);

public:
                    ModelessDialog( Window* pParent, const ResId& rResId );
                    ModelessDialog( Window* pParent, const rtl::OString& rID, const rtl::OUString& rUIXMLDescription );
};

// ---------------
// - ModalDialog -
// ---------------

class VCL_DLLPUBLIC ModalDialog : public Dialog
{
public:
                    ModalDialog( Window* pParent, WinBits nStyle = WB_STDMODAL );
                    ModalDialog( Window* pParent, const rtl::OString& rID, const rtl::OUString& rUIXMLDescription );
                    ModalDialog( Window* pParent, const ResId& rResId );

protected:
    using Window::Show;
    void            Show( sal_Bool bVisible = sal_True );
    using Window::Hide;
    void            Hide();

private:

    // Copy assignment is forbidden and not implemented.
    SAL_DLLPRIVATE         ModalDialog (const ModalDialog &);
    SAL_DLLPRIVATE         ModalDialog & operator= (const ModalDialog &);
};

#endif  // _SV_DIALOG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

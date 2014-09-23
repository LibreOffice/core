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

#ifndef INCLUDED_VCL_DIALOG_HXX
#define INCLUDED_VCL_DIALOG_HXX

#include <tools/solar.h>
#include <vcl/dllapi.h>
#include <vcl/syswin.hxx>
#include <vcl/timer.hxx>

// parameter to pass to the dialog constructor if really no parent is wanted
// whereas NULL chooses the default dialog parent
#define DIALOG_NO_PARENT ((vcl::Window*)0xffffffff)


// - Dialog -

struct DialogImpl;
class VclBox;
class VclButtonBox;

class VCL_DLLPUBLIC Dialog : public SystemWindow
{
private:
    Dialog*         mpPrevExecuteDlg;
    DialogImpl*     mpDialogImpl;
    long            mnMousePositioned;
    bool            mbInExecute;
    bool            mbOldSaveBack;
    bool            mbInClose;
    bool            mbModalMode;

    VclButtonBox*   mpActionArea;
    VclBox*         mpContentArea;

    SAL_DLLPRIVATE void    ImplInitDialogData();
    SAL_DLLPRIVATE void    ImplInitSettings();

    // Copy assignment is forbidden and not implemented.
    SAL_DLLPRIVATE         Dialog (const Dialog &);
    SAL_DLLPRIVATE         Dialog & operator= (const Dialog &);

    DECL_DLLPRIVATE_LINK( ImplAsyncCloseHdl, void* );

protected:
    using Window::ImplInit;
    SAL_DLLPRIVATE void    ImplInit( vcl::Window* pParent, WinBits nStyle );

public:
    SAL_DLLPRIVATE bool    IsInClose() const { return mbInClose; }
    virtual        void    doDeferredInit(WinBits nBits) SAL_OVERRIDE;

protected:
    explicit        Dialog( WindowType nType );
    explicit        Dialog( vcl::Window* pParent, const OString& rID, const OUString& rUIXMLDescription, WindowType nType );
    virtual void    Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize, sal_uLong nFlags ) SAL_OVERRIDE;
    virtual void    settingOptimalLayoutSize(VclBox *pBox) SAL_OVERRIDE;

protected:
    friend class VclBuilder;
    void set_action_area(VclButtonBox *pBox);
    void set_content_area(VclBox *pBox);

public:
    explicit        Dialog( vcl::Window* pParent, WinBits nStyle = WB_STDDIALOG );
    explicit        Dialog( vcl::Window* pParent, const OString& rID, const OUString& rUIXMLDescription );
    virtual         ~Dialog();

    virtual bool    Notify( NotifyEvent& rNEvt ) SAL_OVERRIDE;
    virtual void    StateChanged( StateChangedType nStateChange ) SAL_OVERRIDE;
    virtual void    DataChanged( const DataChangedEvent& rDCEvt ) SAL_OVERRIDE;

    virtual void queue_resize(StateChangedType eReason = STATE_CHANGE_LAYOUT) SAL_OVERRIDE;
    virtual bool set_property(const OString &rKey, const OString &rValue) SAL_OVERRIDE;
    VclButtonBox* get_action_area() { return mpActionArea;}
    VclBox* get_content_area() { return mpContentArea;}

    virtual bool    Close() SAL_OVERRIDE;

    virtual short   Execute();
    bool            IsInExecute() const { return mbInExecute; }

    // Dialog::Execute replacement API
public:
    // Link impl: DECL_LINK( MyEndDialogHdl, Dialog* ); <= param is dialog just ended
    virtual void    StartExecuteModal( const Link& rEndDialogHdl );
    long            GetResult() const;
private:
    bool            ImplStartExecuteModal();
    void            ImplEndExecuteModal();
public:

    // Dialog::Execute replacement API


    void            EndDialog( long nResult = 0 );
    static void     EndAllDialogs( vcl::Window* pParent=NULL );

    void            GetDrawWindowBorder( sal_Int32& rLeftBorder, sal_Int32& rTopBorder,
                                         sal_Int32& rRightBorder, sal_Int32& rBottomBorder ) const;

    void            SetModalInputMode( bool bModal );
    void            SetModalInputMode( bool bModal, bool bSubModalDialogs );
    bool            IsModalInputMode() const { return mbModalMode; }

    void            GrabFocusToFirstControl();
};

// - ModelessDialog -
class VCL_DLLPUBLIC ModelessDialog : public Dialog
{
    // Copy assignment is forbidden and not implemented.
    SAL_DLLPRIVATE         ModelessDialog (const ModelessDialog &);
    SAL_DLLPRIVATE         ModelessDialog & operator= (const ModelessDialog &);

public:
    explicit        ModelessDialog( vcl::Window* pParent, const OString& rID, const OUString& rUIXMLDescription );
};

// - ModalDialog -
class VCL_DLLPUBLIC ModalDialog : public Dialog
{
public:
    explicit        ModalDialog( vcl::Window* pParent, WinBits nStyle = WB_STDMODAL );
    explicit        ModalDialog( vcl::Window* pParent, const OString& rID, const OUString& rUIXMLDescription );

protected:
    using Window::Show;
    void            Show( bool bVisible = true );
    using Window::Hide;
    void            Hide();

private:

    // Copy assignment is forbidden and not implemented.
    SAL_DLLPRIVATE         ModalDialog (const ModalDialog &);
    SAL_DLLPRIVATE         ModalDialog & operator= (const ModalDialog &);
};

#endif // INCLUDED_VCL_DIALOG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

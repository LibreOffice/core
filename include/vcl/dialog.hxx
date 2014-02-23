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
#include <vcl/builder.hxx>
#include <vcl/syswin.hxx>
#include <vcl/timer.hxx>

// parameter to pass to the dialog constructor if really no parent is wanted
// whereas NULL chooses the default dialog parent
#define DIALOG_NO_PARENT ((Window*)0xffffffff)


// - Dialog -

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
    bool            mbInExecute;
    bool            mbOldSaveBack;
    bool            mbInClose;
    bool            mbModalMode;
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
    SAL_DLLPRIVATE bool    IsInClose() const { return mbInClose; }
    SAL_DLLPRIVATE bool hasPendingLayout() const { return maLayoutTimer.IsActive(); }
    SAL_DLLPRIVATE void doDeferredInit(bool bResizable);
    SAL_DLLPRIVATE bool isDeferredInit() const { return mbIsDefferedInit; }

protected:
    explicit        Dialog( WindowType nType );
    explicit        Dialog( Window* pParent, const OString& rID, const OUString& rUIXMLDescription, WindowType nType );
    virtual void    Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize, sal_uLong nFlags );

protected:
    friend class VclBuilder;
    void set_action_area(VclButtonBox *pBox);
    void set_content_area(VclBox *pBox);

public:
    explicit        Dialog( Window* pParent, WinBits nStyle = WB_STDDIALOG );
    explicit        Dialog( Window* pParent, const OString& rID, const OUString& rUIXMLDescription );
    virtual         ~Dialog();

    virtual bool    Notify( NotifyEvent& rNEvt );
    virtual void    StateChanged( StateChangedType nStateChange );
    virtual void    DataChanged( const DataChangedEvent& rDCEvt );

    virtual Size    GetOptimalSize() const;
    virtual void    Resize();
    bool            isLayoutEnabled() const;
    void            setOptimalLayoutSize();
    bool            isCalculatingInitialLayoutSize() const { return mbIsCalculatingInitialLayoutSize; }
    virtual void queue_resize();
    virtual bool set_property(const OString &rKey, const OString &rValue);
    VclButtonBox* get_action_area();
    VclBox* get_content_area();

    virtual bool    Close();

    virtual short   Execute();
    bool            IsInExecute() const { return mbInExecute; }

    virtual void      SetText( const OUString& rStr );
    virtual OUString  GetText() const;


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
    static void     EndAllDialogs( Window* pParent=NULL );

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
    explicit        ModelessDialog( Window* pParent, const ResId& rResId );
    explicit        ModelessDialog( Window* pParent, const OString& rID, const OUString& rUIXMLDescription );
};


// - ModalDialog -


class VCL_DLLPUBLIC ModalDialog : public Dialog
{
public:
    explicit        ModalDialog( Window* pParent, WinBits nStyle = WB_STDMODAL );
    explicit        ModalDialog( Window* pParent, const OString& rID, const OUString& rUIXMLDescription );
    explicit        ModalDialog( Window* pParent, const ResId& rResId );

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

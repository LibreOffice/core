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
#include <vcl/vclptr.hxx>

// - Dialog -

struct DialogImpl;
class VclBox;
class VclButtonBox;

class VCL_DLLPUBLIC Dialog : public SystemWindow
{
public:
    enum class InitFlag { Default, NoParent };

private:
    VclPtr<Dialog>  mpPrevExecuteDlg;
    DialogImpl*     mpDialogImpl;
    long            mnMousePositioned;
    bool            mbInExecute;
    bool            mbOldSaveBack;
    bool            mbInClose;
    bool            mbModalMode;
    InitFlag        mnInitFlag; // used for deferred init

    VclPtr<VclButtonBox> mpActionArea;
    VclPtr<VclBox>       mpContentArea;

    SAL_DLLPRIVATE void    ImplInitDialogData();
    SAL_DLLPRIVATE void    ImplInitSettings();

    virtual void ApplySettings(vcl::RenderContext& rRenderContext) override;

    SAL_DLLPRIVATE         Dialog (const Dialog &) = delete;
    SAL_DLLPRIVATE         Dialog & operator= (const Dialog &) = delete;

    DECL_DLLPRIVATE_LINK_TYPED( ImplAsyncCloseHdl, void*, void );

protected:
    using Window::ImplInit;
    SAL_DLLPRIVATE void    ImplInit( vcl::Window* pParent, WinBits nStyle, InitFlag eFlag = InitFlag::Default );

public:
    SAL_DLLPRIVATE bool    IsInClose() const { return mbInClose; }
    virtual        void    doDeferredInit(WinBits nBits) override;

protected:
    explicit        Dialog( WindowType nType );
    explicit        Dialog( vcl::Window* pParent, const OUString& rID, const OUString& rUIXMLDescription, WindowType nType, InitFlag eFlag = InitFlag::Default );
    virtual void    Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize, DrawFlags nFlags ) override;
    virtual void    settingOptimalLayoutSize(Window *pBox) override;

protected:
    friend class VclBuilder;
    void set_action_area(VclButtonBox* pBox);
    void set_content_area(VclBox* pBox);

public:
    explicit        Dialog( vcl::Window* pParent, WinBits nStyle = WB_STDDIALOG, InitFlag eFlag = InitFlag::Default );
    explicit        Dialog( vcl::Window* pParent, const OUString& rID, const OUString& rUIXMLDescription );
    virtual         ~Dialog();
    virtual void    dispose() override;

    virtual bool    Notify( NotifyEvent& rNEvt ) override;
    virtual void    StateChanged( StateChangedType nStateChange ) override;
    virtual void    DataChanged( const DataChangedEvent& rDCEvt ) override;

    virtual void queue_resize(StateChangedType eReason = StateChangedType::Layout) override;
    virtual bool set_property(const OString &rKey, const OString &rValue) override;
    VclButtonBox* get_action_area() { return mpActionArea; }
    VclBox* get_content_area() { return mpContentArea; }

    virtual bool    Close() override;

    virtual short   Execute();
    bool            IsInExecute() const { return mbInExecute; }

    // Dialog::Execute replacement API
public:
    // Link impl: DECL_LINK( MyEndDialogHdl, Dialog* ); <= param is dialog just ended
    virtual void    StartExecuteModal( const Link<Dialog&,void>& rEndDialogHdl );
    long            GetResult() const;
private:
    bool            ImplStartExecuteModal();
    static void     ImplEndExecuteModal();
    bool            ImplHandleCmdEvent ( const CommandEvent& rCEvent );
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
                    ModelessDialog (const ModelessDialog &) = delete;
                    ModelessDialog & operator= (const ModelessDialog &) = delete;

public:
    explicit        ModelessDialog( vcl::Window* pParent, const OUString& rID, const OUString& rUIXMLDescription, Dialog::InitFlag eFlag = Dialog::InitFlag::Default );
};

// - ModalDialog -
class VCL_DLLPUBLIC ModalDialog : public Dialog
{
public:
    explicit        ModalDialog( vcl::Window* pParent, WinBits nStyle = WB_STDMODAL );
    explicit        ModalDialog( vcl::Window* pParent, const OUString& rID, const OUString& rUIXMLDescription );

protected:
    using Window::Show;
    using Window::Hide;

private:

    SAL_DLLPRIVATE         ModalDialog (const ModalDialog &) = delete;
    SAL_DLLPRIVATE         ModalDialog & operator= (const ModalDialog &) = delete;
};

#endif // INCLUDED_VCL_DIALOG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

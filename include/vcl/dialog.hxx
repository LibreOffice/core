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

#include <memory>
#include <tools/solar.h>
#include <vcl/dllapi.h>
#include <vcl/syswin.hxx>
#include <vcl/vclptr.hxx>
#include <vcl/IDialogRenderable.hxx>
#include <vcl/abstdlg.hxx>

struct DialogImpl;
class VclBox;
class VclButtonBox;

class VCL_DLLPUBLIC Dialog : public SystemWindow
{
public:
    enum class InitFlag
    {
        /** Use given parent or get a default one using GetDefaultParent(...) */
        Default,

        /** No Parent */
        NoParent
    };

private:
    std::unique_ptr<DialogImpl> mpDialogImpl;
    long            mnMousePositioned;
    bool            mbInExecute;
    bool            mbInSyncExecute;
    bool            mbInClose;
    bool            mbModalMode;
    bool            mbPaintComplete;
    bool            mbForceBorderWindow;
    InitFlag        mnInitFlag; // used for deferred init

    VclPtr<VclButtonBox> mpActionArea;
    VclPtr<VclBox>       mpContentArea;

    SAL_DLLPRIVATE void    RemoveFromDlgList();
    SAL_DLLPRIVATE void    ImplInitDialogData();
    SAL_DLLPRIVATE void    ImplInitSettings();
    SAL_DLLPRIVATE VclPtr<vcl::Window> AddBorderWindow(vcl::Window* pParent, WinBits nBits);

    virtual void ApplySettings(vcl::RenderContext& rRenderContext) override;

    SAL_DLLPRIVATE         Dialog (const Dialog &) = delete;
    SAL_DLLPRIVATE         Dialog & operator= (const Dialog &) = delete;

    DECL_DLLPRIVATE_LINK( ImplAsyncCloseHdl, void*, void );

protected:
    using Window::ImplInit;
    SAL_DLLPRIVATE void    ImplInit( vcl::Window* pParent, WinBits nStyle, InitFlag eFlag = InitFlag::Default );

    /// Find and set the LOK notifier according to the pParent.
    void ImplLOKNotifier(vcl::Window* pParent);

public:
    SAL_DLLPRIVATE bool    IsInClose() const { return mbInClose; }
    virtual        void    doDeferredInit(WinBits nBits) override;

protected:
    explicit        Dialog( WindowType nType );
    explicit        Dialog( vcl::Window* pParent, const OUString& rID, const OUString& rUIXMLDescription, WindowType nType, InitFlag eFlag = InitFlag::Default, bool bBorder = false );
    virtual void    Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize, DrawFlags nFlags ) override;
    virtual void    settingOptimalLayoutSize(Window *pBox) override;

protected:
    friend class VclBuilder;
    void set_action_area(VclButtonBox* pBox);
    void set_content_area(VclBox* pBox);

public:
    explicit        Dialog( vcl::Window* pParent, WinBits nStyle = WB_STDDIALOG, InitFlag eFlag = InitFlag::Default );
    explicit        Dialog( vcl::Window* pParent, const OUString& rID, const OUString& rUIXMLDescription );
    virtual         ~Dialog() override;
    virtual void    dispose() override;

    // get the default parent for a dialog as is done in standard initialization
    static vcl::Window* GetDefaultParent(WinBits nStyle);

    virtual bool    EventNotify( NotifyEvent& rNEvt ) override;
    virtual void    StateChanged( StateChangedType nStateChange ) override;
    virtual void    DataChanged( const DataChangedEvent& rDCEvt ) override;

    virtual void queue_resize(StateChangedType eReason = StateChangedType::Layout) override;
    virtual bool set_property(const OString &rKey, const OUString &rValue) override;
    VclButtonBox* get_action_area() { return mpActionArea; }
    VclBox* get_content_area() { return mpContentArea; }

    virtual bool    Close() override;

    // try to extract content and return as Bitmap. To do that reliably, a Yield-loop
    // like in Execute() has to be executed and it is necessary to detect when the
    // paint is finished
    virtual void PrePaint(vcl::RenderContext& rRenderContext) override;
    virtual void PostPaint(vcl::RenderContext& rRenderContext) override;

    // ensureRepaint - triggers Application::Yield until the dialog is
    // completely repainted. Sometimes needed for dialogs showing progress
    // during actions
    void ensureRepaint();

    // Screenshot interface
    virtual std::vector<OString> getAllPageUIXMLDescriptions() const;
    virtual bool selectPageByUIXMLDescription(const OString& rUIXMLDescription);
    Bitmap createScreenshot();

    virtual short   Execute();
    bool            IsInExecute() const { return mbInExecute; }
    // Return true when dialog is synchronously executed (calling ::Execute())
    bool            IsInSyncExecute() const { return mbInSyncExecute; };

    virtual FactoryFunction GetUITestFactory() const override;

    // Dialog::Execute replacement API
public:
    virtual void    StartExecuteModal( const Link<Dialog&,void>& rEndDialogHdl );
    long            GetResult() const;
private:
    bool            ImplStartExecuteModal();
    static void     ImplEndExecuteModal();
    void            ImplSetModalInputMode(bool bModal);
public:

    // FIXME: Need to remove old StartExecuteModal in favour of this one.
    /// Returns true if the dialog successfully starts
    virtual bool StartExecuteAsync(const std::function<void(sal_Int32)> &rEndDialogFn)
    {
        VclAbstractDialog::AsyncContext aCtx;
        aCtx.mxOwner = this;
        aCtx.maEndDialogFn = rEndDialogFn;
        return StartExecuteAsync(aCtx);
    }

    /// Commence execution of a modal dialog, disposes owner on failure
    virtual bool    StartExecuteAsync(VclAbstractDialog::AsyncContext &rCtx);

    // Dialog::Execute replacement API


    void            EndDialog( long nResult = 0 );
    static void     EndAllDialogs( vcl::Window const * pParent );

    void            GetDrawWindowBorder( sal_Int32& rLeftBorder, sal_Int32& rTopBorder,
                                         sal_Int32& rRightBorder, sal_Int32& rBottomBorder ) const;

    void            SetModalInputMode( bool bModal );
    bool            IsModalInputMode() const { return mbModalMode; }

    void            GrabFocusToFirstControl();
    virtual void    Resize() override;
};

class VCL_DLLPUBLIC ModelessDialog : public Dialog
{
                    ModelessDialog (const ModelessDialog &) = delete;
                    ModelessDialog & operator= (const ModelessDialog &) = delete;

public:
    explicit        ModelessDialog( vcl::Window* pParent, const OUString& rID, const OUString& rUIXMLDescription, Dialog::InitFlag eFlag = Dialog::InitFlag::Default );
};

class VCL_DLLPUBLIC ModalDialog : public Dialog
{
public:
    explicit        ModalDialog( vcl::Window* pParent, WinBits nStyle = WB_STDMODAL );
    explicit        ModalDialog( vcl::Window* pParent, const OUString& rID, const OUString& rUIXMLDescription, bool bBorder = false );

protected:
    using Window::Show;
    using Window::Hide;

private:

    SAL_DLLPRIVATE         ModalDialog (const ModalDialog &) = delete;
    SAL_DLLPRIVATE         ModalDialog & operator= (const ModalDialog &) = delete;
};

#endif // INCLUDED_VCL_DIALOG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

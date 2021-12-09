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

#if !defined(VCL_DLLIMPLEMENTATION) && !defined(TOOLKIT_DLLIMPLEMENTATION) && !defined(VCL_INTERNALS)
#error "don't use this in new code"
#endif

#include <memory>
#include <vcl/dllapi.h>
#include <vcl/syswin.hxx>
#include <vcl/vclptr.hxx>
#include <vcl/abstdlg.hxx>

struct DialogImpl;
class PushButton;
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
    tools::Long            mnMousePositioned;
    bool            mbInExecute;
    bool            mbInSyncExecute;
    bool            mbInClose;
    bool            mbModalMode;
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

    DECL_DLLPRIVATE_LINK(ImplAsyncCloseHdl, void*, void);
    DECL_DLLPRIVATE_LINK(ResponseHdl, Button*, void);

protected:
    void    ImplInitDialog( vcl::Window* pParent, WinBits nStyle, InitFlag eFlag = InitFlag::Default );

    /// Find and set the LOK notifier according to the pParent.
    void ImplLOKNotifier(vcl::Window* pParent);

public:
    SAL_DLLPRIVATE bool    IsInClose() const { return mbInClose; }
    virtual        void    doDeferredInit(WinBits nBits) override;
    SAL_DLLPRIVATE void    disposeOwnedButtons();

protected:
    explicit        Dialog( WindowType nType );
    explicit        Dialog( vcl::Window* pParent, const OUString& rID, const OUString& rUIXMLDescription);
    virtual void    Draw( OutputDevice* pDev, const Point& rPos, DrawFlags nFlags ) override;
    virtual void    settingOptimalLayoutSize(Window *pBox) override;

protected:
    friend class VclBuilder;
    friend class SalInstanceBuilder;
    void set_action_area(VclButtonBox* pBox);
    virtual void set_content_area(VclBox* pBox);

public:
    explicit        Dialog( vcl::Window* pParent, WinBits nStyle = WB_STDDIALOG, InitFlag eFlag = InitFlag::Default );
    virtual         ~Dialog() override;
    virtual void    dispose() override;

    // get the default parent for a dialog as is done in standard initialization
    static vcl::Window* GetDefaultParent(WinBits nStyle);

    virtual bool    EventNotify( NotifyEvent& rNEvt ) override;
    virtual void    StateChanged( StateChangedType nStateChange ) override;
    virtual void    DataChanged( const DataChangedEvent& rDCEvt ) override;
    virtual void    Command( const CommandEvent& rCEvt ) override;
    virtual void    PixelInvalidate(const tools::Rectangle* pRectangle) override;

    virtual void queue_resize(StateChangedType eReason = StateChangedType::Layout) override;
    virtual bool set_property(const OString &rKey, const OUString &rValue) override;
    VclButtonBox* get_action_area() { return mpActionArea; }
    VclBox* get_content_area() { return mpContentArea; }

    virtual bool    Close() override;

    short           Execute();
    bool            IsInExecute() const { return mbInExecute; }
    // Return true when dialog is synchronously executed (calling ::Execute())
    bool            IsInSyncExecute() const { return mbInSyncExecute; };

    virtual FactoryFunction GetUITestFactory() const override;

private:
    bool            ImplStartExecute();
    static void     ImplEndExecuteModal();
    void            ImplSetModalInputMode(bool bModal);

    vcl::Window*    GetFirstControlForFocus();
public:

    /// Commence execution of a modal dialog, disposes owner on failure
    bool            StartExecuteAsync(VclAbstractDialog::AsyncContext &rCtx);

    // Dialog::Execute replacement API


    void            EndDialog( tools::Long nResult = RET_CANCEL );

    void            GetDrawWindowBorder( sal_Int32& rLeftBorder, sal_Int32& rTopBorder,
                                         sal_Int32& rRightBorder, sal_Int32& rBottomBorder ) const;

    void            SetModalInputMode( bool bModal );
    bool            IsModalInputMode() const { return mbModalMode; }

    void            GrabFocusToFirstControl();
    virtual void    Resize() override;

    void            Activate() override;

    void            SetPopupMenuHdl(const Link<const CommandEvent&, bool>& rLink);
    void            SetInstallLOKNotifierHdl(const Link<void*, vcl::ILibreOfficeKitNotifier*>& rLink);
    void            SetLOKTunnelingState(bool bEnabled);

    void            add_button(PushButton* pButton, int nResponse, bool bTransferOwnership);
    void            set_default_response(int nResponse);
    int             get_default_response() const;
    vcl::Window*    get_widget_for_response(int nResponse);

    void            DumpAsPropertyTree(tools::JsonWriter& rJsonWriter) override;
};

#endif // INCLUDED_VCL_DIALOG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

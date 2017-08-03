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

        /** Suppress Parent so that Parent is not blocked (kind of modal mode) */
        NoParent,

        /** Suppress Parent (no modal, see above) and additionally center on default parent */
        NoParentCentered
    };

private:
    VclPtr<Dialog>  mpPrevExecuteDlg;
    std::unique_ptr<DialogImpl>     mpDialogImpl;
    long            mnMousePositioned;
    bool            mbInExecute;
    bool            mbInClose;
    bool            mbModalMode;
    bool            mbPaintComplete;
    InitFlag        mnInitFlag; // used for deferred init

    VclPtr<VclButtonBox> mpActionArea;
    VclPtr<VclBox>       mpContentArea;

    vcl::IDialogRenderable*   mpDialogRenderable; // to emit LOK callbacks

    SAL_DLLPRIVATE void    ImplInitDialogData();
    SAL_DLLPRIVATE void    ImplInitSettings();

    virtual void ApplySettings(vcl::RenderContext& rRenderContext) override;

    SAL_DLLPRIVATE         Dialog (const Dialog &) = delete;
    SAL_DLLPRIVATE         Dialog & operator= (const Dialog &) = delete;

    DECL_DLLPRIVATE_LINK( ImplAsyncCloseHdl, void*, void );

protected:
    using Window::ImplInit;
    SAL_DLLPRIVATE void    ImplInit( vcl::Window* pParent, WinBits nStyle, InitFlag eFlag = InitFlag::Default );
    OUString               maID; // identifier for this dialog

public:
    SAL_DLLPRIVATE bool    IsInClose() const { return mbInClose; }
    virtual        void    doDeferredInit(WinBits nBits) override;
    virtual        void    LogicInvalidate(const tools::Rectangle* pRectangle) override;

    /// Necessary to register dialog renderable instance to emit LOK callbacks
    void registerDialogRenderable(vcl::IDialogRenderable* pDialogRenderable);
    /// Paints the current dialog to the given virtual device
    void paintDialog(VirtualDevice& rDevice);
    void LogicMouseButtonDown(const MouseEvent& rMouseEvent);
    void LogicMouseButtonUp(const MouseEvent& rMouseEvent);
    void LogicMouseMove(const MouseEvent& rMouseEvent);

    void LOKKeyInput(const KeyEvent& rKeyEvent);
    void LOKKeyUp(const KeyEvent& rKeyEvent);

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

    virtual FactoryFunction GetUITestFactory() const override;

    // Dialog::Execute replacement API
public:
    virtual void    StartExecuteModal( const Link<Dialog&,void>& rEndDialogHdl );
    long            GetResult() const;
private:
    bool            ImplStartExecuteModal();
    static void     ImplEndExecuteModal();
public:

    // Dialog::Execute replacement API


    void            EndDialog( long nResult = 0 );
    static void     EndAllDialogs( vcl::Window const * pParent );
    static bool     AreDialogsOpen();

    void            GetDrawWindowBorder( sal_Int32& rLeftBorder, sal_Int32& rTopBorder,
                                         sal_Int32& rRightBorder, sal_Int32& rBottomBorder ) const;

    void            SetModalInputMode( bool bModal );
    bool            IsModalInputMode() const { return mbModalMode; }

    void            GrabFocusToFirstControl();
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

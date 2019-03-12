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
#ifndef INCLUDED_SFX2_BASEDLGS_HXX
#define INCLUDED_SFX2_BASEDLGS_HXX

#include <memory>
#include <sal/config.h>
#include <sfx2/dllapi.h>
#include <sal/types.h>
#include <vcl/builder.hxx>
#include <vcl/dialog.hxx>
#include <vcl/floatwin.hxx>
#include <vcl/timer.hxx>
#include <vcl/weld.hxx>

class TabPage;
class SfxTabPage;
class SfxBindings;
class SfxChildWindow;
struct SfxChildWinInfo;
class SfxItemSet;
class SfxItemPool;
class OKButton;
class CancelButton;
class HelpButton;
class Button;

// class SfxModalDialog --------------------------------------------------

class SFX2_DLLPUBLIC SfxModalDialog: public ModalDialog
{
    OUString                aExtraData;
    const SfxItemSet*       pInputSet;
    std::unique_ptr<SfxItemSet> pOutputSet;

private:
    SfxModalDialog(SfxModalDialog const &) = delete;
    SfxModalDialog& operator =(SfxModalDialog const &) = delete;

    SAL_DLLPRIVATE void SetDialogData_Impl();
    SAL_DLLPRIVATE void GetDialogData_Impl();
    DECL_DLLPRIVATE_STATIC_LINK(SfxModalDialog, InstallLOKNotifierHdl, void*, vcl::ILibreOfficeKitNotifier*);

protected:
    SfxModalDialog(vcl::Window *pParent, const OUString& rID, const OUString& rUIXMLDescription);

    OUString&           GetExtraData()      { return aExtraData; }
    void                CreateOutputItemSet( const SfxItemSet& rInput );
    void                SetInputSet( const SfxItemSet* pInSet ) { pInputSet = pInSet; }
    SfxItemSet*         GetOutputSetImpl() { return pOutputSet.get(); }

public:
    virtual ~SfxModalDialog() override;
    virtual void dispose() override;

    const SfxItemSet*   GetOutputItemSet() const { return pOutputSet.get(); }
    const SfxItemSet*   GetInputItemSet() const { return pInputSet; }
};

// class SfxModelessDialog --------------------------------------------------
class SfxModelessDialog_Impl;
class SFX2_DLLPUBLIC SfxModelessDialog: public ModelessDialog
{
    SfxBindings*            pBindings;
    Size                    aSize;
    std::unique_ptr< SfxModelessDialog_Impl > pImpl;

    SfxModelessDialog(SfxModelessDialog const &) = delete;
    SfxModelessDialog& operator =(SfxModelessDialog const &) = delete;

    void Init(SfxBindings *pBindinx, SfxChildWindow *pCW);

    DECL_DLLPRIVATE_STATIC_LINK(SfxModelessDialog, InstallLOKNotifierHdl, void*, vcl::ILibreOfficeKitNotifier*);
protected:
    SfxModelessDialog( SfxBindings*, SfxChildWindow*,
        vcl::Window*, const OUString& rID, const OUString& rUIXMLDescription );
    virtual ~SfxModelessDialog() override;
    virtual void dispose() override;
    virtual bool            Close() override;
    virtual void            Resize() override;
    virtual void            Move() override;
    virtual void            StateChanged( StateChangedType nStateChange ) override;

public:
    virtual void            FillInfo(SfxChildWinInfo&) const;
    void                    Initialize (SfxChildWinInfo const * pInfo);
    virtual bool            EventNotify( NotifyEvent& rNEvt ) override;
    SfxBindings&            GetBindings()
                            { return *pBindings; }

    DECL_LINK(TimerHdl, Timer *, void);
};

class SFX2_DLLPUBLIC SfxDialogController : public weld::GenericDialogController
{
private:
    DECL_DLLPRIVATE_STATIC_LINK(SfxDialogController, InstallLOKNotifierHdl, void*, vcl::ILibreOfficeKitNotifier*);
public:
    SfxDialogController(weld::Widget* pParent, const OUString& rUIFile, const OString& rDialogId);
};

class SfxModelessDialog_Impl;
class SFX2_DLLPUBLIC SfxModelessDialogController : public SfxDialogController
{
    SfxBindings* m_pBindings;
    std::unique_ptr<SfxModelessDialog_Impl> m_xImpl;

    SfxModelessDialogController(SfxModelessDialogController&) = delete;
    void operator =(SfxModelessDialogController&) = delete;

    void Init(SfxBindings *pBindinx, SfxChildWindow *pCW);

    DECL_DLLPRIVATE_LINK(FocusInHdl, weld::Widget&, void);
    DECL_DLLPRIVATE_LINK(FocusOutHdl, weld::Widget&, void);
protected:
    SfxModelessDialogController(SfxBindings*, SfxChildWindow* pChildWin,
        weld::Window* pParent, const OUString& rUIXMLDescription, const OString& rID);
    virtual ~SfxModelessDialogController() override;

public:
    void                    FillInfo(SfxChildWinInfo&) const;
    virtual void            Activate() {}
    void                    Initialize (SfxChildWinInfo const * pInfo);
    void                    Close();
    void                    DeInit();
    void                    EndDialog();
    SfxBindings&            GetBindings() { return *m_pBindings; }
};

// class SfxFloatingWindow --------------------------------------------------
class SfxFloatingWindow_Impl;
class SFX2_DLLPUBLIC SfxFloatingWindow: public FloatingWindow
{
    SfxBindings*            pBindings;
    Size                    aSize;
    std::unique_ptr< SfxFloatingWindow_Impl > pImpl;

    SfxFloatingWindow(SfxFloatingWindow const &) = delete;
    SfxFloatingWindow& operator =(SfxFloatingWindow const &) = delete;

protected:
                            SfxFloatingWindow( SfxBindings *pBindings,
                                              SfxChildWindow *pCW,
                                              vcl::Window* pParent,
                                              WinBits nWinBits);
                            SfxFloatingWindow( SfxBindings *pBindings,
                                              SfxChildWindow *pCW,
                                              vcl::Window* pParent,
                                              const OString& rID, const OUString& rUIXMLDescription,
                                              const css::uno::Reference<css::frame::XFrame> &rFrame = css::uno::Reference<css::frame::XFrame>());
                            virtual ~SfxFloatingWindow() override;
    virtual void            dispose() override;

    virtual void            StateChanged( StateChangedType nStateChange ) override;
    virtual bool            Close() override;
    virtual void            Resize() override;
    virtual void            Move() override;
    virtual bool            EventNotify( NotifyEvent& rNEvt ) override;
    SfxBindings&            GetBindings()
                            { return *pBindings; }

public:
    virtual void            FillInfo(SfxChildWinInfo&) const;
    void                    Initialize (SfxChildWinInfo const * pInfo);

    DECL_LINK(TimerHdl, Timer *, void);

};

// class SfxNoLayoutSingleTabDialog --------------------------------------------------

struct SingleTabDlgImpl
{
    VclPtr<SfxTabPage>          m_pSfxPage;

    SingleTabDlgImpl();
};

typedef const sal_uInt16* (*GetTabPageRanges)(); // provides international Which values

class SFX2_DLLPUBLIC SfxSingleTabDialog : public SfxModalDialog
{
public:
    SfxSingleTabDialog(vcl::Window *pParent, const SfxItemSet& rOptionsSet,
        const OUString& rID = OUString("SingleTabDialog"),
        const OUString& rUIXMLDescription = OUString("sfx/ui/singletabdialog.ui"));

    SfxSingleTabDialog(vcl::Window *pParent, const SfxItemSet* pInSet,
        const OUString& rID = OUString("SingleTabDialog"),
        const OUString& rUIXMLDescription = OUString("sfx/ui/singletabdialog.ui"));

    virtual             ~SfxSingleTabDialog() override;
    virtual void        dispose() override;

    void                SetTabPage(SfxTabPage* pTabPage);
    SfxTabPage*         GetTabPage() const { return pImpl->m_pSfxPage; }

    OKButton*           GetOKButton() const { return pOKBtn; }

private:
    VclPtr<OKButton>      pOKBtn;
    VclPtr<CancelButton>  pCancelBtn;
    VclPtr<HelpButton>    pHelpBtn;

    DECL_DLLPRIVATE_LINK(OKHdl_Impl, Button*, void);

    std::unique_ptr<SingleTabDlgImpl>   pImpl;
};

class SFX2_DLLPUBLIC SfxOkDialogController : public SfxDialogController
{
public:
    SfxOkDialogController(weld::Widget* pParent, const OUString& rUIXMLDescription,
                          const OString& rID)
        : SfxDialogController(pParent, rUIXMLDescription, rID)
    {
    }

    virtual weld::Button&       GetOKButton() const = 0;
    virtual const SfxItemSet* GetExampleSet() const = 0;
};

class SFX2_DLLPUBLIC SfxSingleTabDialogController : public SfxOkDialogController
{
private:
    std::unique_ptr<SfxItemSet> m_xOutputSet;
    const SfxItemSet* m_pInputSet;

public:
    SfxSingleTabDialogController(weld::Widget* pParent, const SfxItemSet& rOptionsSet,
        const OUString& rUIXMLDescription = OUString("sfx/ui/singletabdialog.ui"),
        const OString& rID = OString("SingleTabDialog"));

    weld::Container* get_content_area() { return m_xContainer.get(); }

    virtual             ~SfxSingleTabDialogController() override;

    void                SetTabPage(SfxTabPage* pTabPage);

    virtual weld::Button& GetOKButton() const override { return *m_xOKBtn; }
    virtual const SfxItemSet* GetExampleSet() const override { return nullptr; }

    const SfxItemSet*   GetOutputItemSet() const { return m_xOutputSet.get(); }
    const SfxItemSet*   GetInputItemSet() const { return m_pInputSet; }

protected:
    VclPtr<SfxTabPage> m_xSfxPage;
    std::unique_ptr<weld::Container> m_xContainer;
    std::unique_ptr<weld::Button> m_xOKBtn;
    std::unique_ptr<weld::Button> m_xHelpBtn;

    void                CreateOutputItemSet(const SfxItemSet& rInput);
    DECL_DLLPRIVATE_LINK(OKHdl_Impl, weld::Button&, void);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

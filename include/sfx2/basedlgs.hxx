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
#include <vcl/weld.hxx>

class SfxTabPage;
class SfxBindings;
class SfxChildWindow;
struct SfxChildWinInfo;
class SfxItemSet;
class WhichRangesContainer;

class SFX2_DLLPUBLIC SfxDialogController : public weld::GenericDialogController
{
private:
    DECL_DLLPRIVATE_STATIC_LINK(SfxDialogController, InstallLOKNotifierHdl, void*, vcl::ILibreOfficeKitNotifier*);

    DECL_DLLPRIVATE_LINK(FocusChangeHdl, weld::Container&, void);

public:
    SfxDialogController(weld::Widget* pParent, const OUString& rUIFile, const OUString& rDialogId);
    // dialog gets focus
    virtual void Activate() {}
    // dialog loses focus
    virtual void Deactivate() {}

    // when the dialog has an associated SfxChildWin, typically for Modeless interaction
    virtual void ChildWinDispose() {} // called from the associated SfxChildWin dtor
    virtual void Close(); // called by the SfxChildWin when the dialog is closed
    virtual void EndDialog(int nResponse); // called by the SfxChildWin to close the dialog
    virtual bool CloseOnHide() const { return true; } // called from ScValidationDlg
};

class SfxModelessDialog_Impl;

class SFX2_DLLPUBLIC SfxModelessDialogController : public SfxDialogController
{
    SfxBindings* m_pBindings;
    std::unique_ptr<SfxModelessDialog_Impl> m_xImpl;

    SfxModelessDialogController(SfxModelessDialogController&) = delete;
    void operator =(SfxModelessDialogController&) = delete;

    void Init(SfxBindings *pBindinx, SfxChildWindow *pCW);

protected:
    SfxModelessDialogController(SfxBindings*, SfxChildWindow* pChildWin,
        weld::Window* pParent, const OUString& rUIXMLDescription, const OUString& rID);

public:
    virtual ~SfxModelessDialogController() override;

    void                    Initialize (SfxChildWinInfo const * pInfo);
    bool                    IsClosing() const;
    virtual void            Close() override;
    virtual void            EndDialog(int nResponse) override;
    virtual void            Activate() override;
    virtual void            Deactivate() override;
    virtual void            ChildWinDispose() override;
    virtual void            FillInfo(SfxChildWinInfo&) const;
    SfxBindings&            GetBindings() const { return *m_pBindings; }
};

typedef WhichRangesContainer (*GetTabPageRanges)(); // provides international Which values

class SFX2_DLLPUBLIC SfxOkDialogController : public SfxDialogController
{
public:
    SfxOkDialogController(weld::Widget* pParent, const OUString& rUIXMLDescription,
                          const OUString& rID)
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
    SfxSingleTabDialogController(weld::Widget* pParent, const SfxItemSet* pOptionsSet,
        const OUString& rUIXMLDescription = u"sfx/ui/singletabdialog.ui"_ustr,
        const OUString& rID = u"SingleTabDialog"_ustr);

    SfxSingleTabDialogController(weld::Widget* pParent, const SfxItemSet* pOptionsSet,
        const OUString& rContainerId, const OUString& rUIXMLDescription,
        const OUString& rID);

    weld::Container* get_content_area() { return m_xContainer.get(); }

    virtual             ~SfxSingleTabDialogController() override;

    void                SetTabPage(std::unique_ptr<SfxTabPage> xTabPage);
    SfxTabPage*         GetTabPage() const { return m_xSfxPage.get(); }

    virtual weld::Button& GetOKButton() const override { return *m_xOKBtn; }
    virtual const SfxItemSet* GetExampleSet() const override { return nullptr; }

    const SfxItemSet*   GetOutputItemSet() const { return m_xOutputSet.get(); }
    const SfxItemSet*   GetInputItemSet() const { return m_pInputSet; }

protected:
    std::unique_ptr<SfxTabPage> m_xSfxPage;
    std::unique_ptr<weld::Container> m_xContainer;
    std::unique_ptr<weld::Button> m_xOKBtn;
    std::unique_ptr<weld::Button> m_xHelpBtn;

    void                CreateOutputItemSet(const SfxItemSet& rInput);
    void                SetInputSet(const SfxItemSet* pInSet) { m_pInputSet = pInSet; }
    DECL_DLLPRIVATE_LINK(OKHdl_Impl, weld::Button&, void);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

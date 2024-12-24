/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <vcl/syswin.hxx>
#include <vcl/ctrl.hxx>
#include <vcl/notebookbar/NotebookBarAddonsMerger.hxx>
#include <vcl/settings.hxx>
#include <vector>

class NotebookBarContextChangeEventListener;
class NotebookbarContextControl;
class SfxViewShell;

/// This implements Widget Layout-based notebook-like menu bar.
class UNLESS_MERGELIBS(VCL_DLLPUBLIC) NotebookBar final : public Control, public VclBuilderContainer
{
friend class NotebookBarContextChangeEventListener;
public:
    NotebookBar(Window* pParent, const OUString& rID, const OUString& rUIXMLDescription,
                const css::uno::Reference<css::frame::XFrame>& rFrame,
                const NotebookBarAddonsItem& aNotebookBarAddonsItem);
    virtual ~NotebookBar() override;
    virtual void dispose() override;

    virtual bool PreNotify( NotifyEvent& rNEvt ) override;
    virtual Size GetOptimalSize() const override;
    virtual void setPosSizePixel(tools::Long nX, tools::Long nY, tools::Long nWidth, tools::Long nHeight, PosSizeFlags nFlags = PosSizeFlags::All) override;
    virtual void Resize() override;

    void SetSystemWindow(SystemWindow* pSystemWindow);

    void StateChanged(const StateChangedType nStateChange ) override;

    void DataChanged(const DataChangedEvent& rDCEvt) override;

    void SetupListener(bool bListen);

    bool IsWelded() const { return m_bIsWelded; }
    VclPtr<vcl::Window>& GetMainContainer() { return m_xVclContentArea; }
    const OUString & GetUIFilePath() const { return m_sUIXMLDescription; }
    void SetDisposeCallback(const Link<const SfxViewShell*, void> rDisposeCallback, const SfxViewShell* pViewShell);

private:
    VclPtr<SystemWindow> m_pSystemWindow;
    rtl::Reference<NotebookBarContextChangeEventListener> m_pEventListener;
    std::vector<NotebookbarContextControl*> m_pContextContainers;
    const SfxViewShell* m_pViewShell;

    VclPtr<vcl::Window> m_xVclContentArea;
    bool m_bIsWelded;
    OUString m_sUIXMLDescription;
    Link<const SfxViewShell*, void> m_rDisposeLink;

    AllSettings DefaultSettings;
    AllSettings PersonaSettings;

    void UpdateBackground();

    void UpdateDefaultSettings();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

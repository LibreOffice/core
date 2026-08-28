/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <vcl/ctrl.hxx>
#include <vcl/settings.hxx>

class SfxViewShell;

/// Container for the welded notebookbar that Collabora Online builds
/// from notebookbar_online.ui via jsdialog.
class UNLESS_MERGELIBS(VCL_DLLPUBLIC) NotebookBar final : public Control
{
public:
    NotebookBar(Window* pParent, OUString aUIXMLDescription);
    virtual ~NotebookBar() override;
    virtual void dispose() override;

    virtual Size GetOptimalSize() const override;
    virtual void Resize() override;

    void StateChanged(const StateChangedType nStateChange) override;

    void DataChanged(const DataChangedEvent& rDCEvt) override;

    VclPtr<vcl::Window>& GetMainContainer() { return m_xVclContentArea; }
    const OUString & GetUIFilePath() const { return m_sUIXMLDescription; }
    void SetDisposeCallback(const Link<const SfxViewShell*, void> rDisposeCallback, const SfxViewShell* pViewShell);

private:
    const SfxViewShell* m_pViewShell;

    VclPtr<vcl::Window> m_xVclContentArea;
    OUString m_sUIXMLDescription;
    Link<const SfxViewShell*, void> m_rDisposeLink;

    AllSettings DefaultSettings;

    void UpdateBackground();

    void UpdateDefaultSettings();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

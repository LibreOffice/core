/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_VCL_NOTEBOOKBAR_HXX
#define INCLUDED_VCL_NOTEBOOKBAR_HXX

#include <vcl/builder.hxx>
#include <vcl/ctrl.hxx>
#include <vcl/settings.hxx>
#include <vector>

namespace com { namespace sun { namespace star { namespace ui { class XContextChangeEventListener; } } } }

class NotebookbarContextControl;
class SystemWindow;

/// This implements Widget Layout-based notebook-like menu bar.
class VCL_DLLPUBLIC NotebookBar : public Control, public VclBuilderContainer
{
friend class NotebookBarContextChangeEventListener;
public:
    NotebookBar(Window* pParent, const OString& rID, const OUString& rUIXMLDescription, const css::uno::Reference<css::frame::XFrame> &rFrame);
    virtual ~NotebookBar() override;
    virtual void dispose() override;

    virtual bool PreNotify( NotifyEvent& rNEvt ) override;
    virtual Size GetOptimalSize() const override;
    virtual void setPosSizePixel(long nX, long nY, long nWidth, long nHeight, PosSizeFlags nFlags = PosSizeFlags::All) override;
    virtual void Resize() override;

    void SetSystemWindow(SystemWindow* pSystemWindow);

    const css::uno::Reference<css::ui::XContextChangeEventListener>& getContextChangeEventListener() const { return m_pEventListener; }

    void StateChanged(const StateChangedType nStateChange ) override;

    void DataChanged(const DataChangedEvent& rDCEvt) override;

    void ControlListener(bool bListen);

private:
    VclPtr<SystemWindow> m_pSystemWindow;
    css::uno::Reference<css::ui::XContextChangeEventListener> m_pEventListener;
    std::vector<NotebookbarContextControl*> m_pContextContainers;
    css::uno::Reference<css::frame::XFrame> mxFrame;

    AllSettings DefaultSettings;
    AllSettings PersonaSettings;

    void UpdateBackground();

    void UpdateDefaultSettings();
    void UpdatePersonaSettings();

};


#endif // INCLUDED_VCL_NOTEBOOKBAR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

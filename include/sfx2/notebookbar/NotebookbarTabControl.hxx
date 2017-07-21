/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SFX2_NOTEBOOKBAR_NOTEBOOKBARTABCONTROL_HXX
#define INCLUDED_SFX2_NOTEBOOKBAR_NOTEBOOKBARTABCONTROL_HXX

#include <vcl/toolbox.hxx>
#include <sfx2/dllapi.h>
#include <vcl/tabctrl.hxx>

class ChangedUIEventListener;
namespace com { namespace sun { namespace star { namespace ui {
    class XUIConfigurationListener;
} } } }

class SFX2_DLLPUBLIC NotebookbarTabControl : public NotebookbarTabControlBase
{
friend class ChangedUIEventListener;

public:
    NotebookbarTabControl( Window* pParent );
    ~NotebookbarTabControl() override;

    virtual void StateChanged(StateChangedType nStateChange) override;
    virtual Size calculateRequisition() const override;

private:
    static void FillShortcutsToolBox(css::uno::Reference<css::uno::XComponentContext> const & xContext,
                                          const css::uno::Reference<css::frame::XFrame>& xFrame,
                                          const OUString& aModuleName,
                                          ToolBox* pShortcuts
                             );
    DECL_LINK(OpenNotebookbarPopupMenu, NotebookBar*, void);

    css::uno::Reference<css::ui::XUIConfigurationListener> m_pListener;
    css::uno::Reference<css::frame::XFrame> m_xFrame;

protected:
    bool m_bInitialized;
    bool m_bInvalidate;
};

#endif // INCLUDED_SFX2_NOTEBOOKBAR_NOTEBOOKBARTABCONTROL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

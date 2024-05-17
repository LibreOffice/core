/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <com/sun/star/drawing/framework/XConfigurationChangeListener.hpp>
#include <comphelper/compbase.hxx>
#include <rtl/ref.hxx>
#include <tools/link.hxx>
#include <set>

namespace com::sun::star::drawing::framework
{
class XConfigurationController;
class XView;
}
namespace com::sun::star::frame
{
class XController;
}
namespace sd
{
class DrawController;
class ViewShellBase;
}
namespace sd::tools
{
class EventMultiplexerEvent;
}

namespace sd::framework
{
/** This module is responsible for handling visibility of NotesPane across modes
*/
class NotesPaneModule : public comphelper::WeakComponentImplHelper<
                            css::drawing::framework::XConfigurationChangeListener>
{
public:
    /** Create a new module that controls the view tab bar above the view
        in the specified pane.
        @param rxController
            This is the access point to the drawing framework.
    */
    NotesPaneModule(const rtl::Reference<::sd::DrawController>& rxController);
    virtual ~NotesPaneModule() override;

    void AddActiveMainView(const OUString& rsMainViewURL);
    bool IsResourceActive(const OUString& rsMainViewURL);
    void SaveResourceState();

    virtual void disposing(std::unique_lock<std::mutex>&) override;

    // XConfigurationChangeListener

    virtual void SAL_CALL notifyConfigurationChange(
        const css::drawing::framework::ConfigurationChangeEvent& rEvent) override;

    // XEventListener

    virtual void SAL_CALL disposing(const css::lang::EventObject& rEvent) override;

private:
    css::uno::Reference<css::drawing::framework::XConfigurationController>
        mxConfigurationController;

    css::uno::Reference<css::drawing::framework::XResourceId> mxBottomImpressPaneId;
    css::uno::Reference<css::drawing::framework::XResourceId> mxMainViewAnchorId;

    std::set<OUString> maActiveMainViewContainer;
    OUString msCurrentMainViewURL;
    ViewShellBase* mpViewShellBase = nullptr;
    bool mbListeningEventMultiplexer = false;
    bool mbInMasterEditMode = false;

    void onMainViewSwitch(const OUString& rsViewURL, const bool bIsActivated);
    void onResourceRequest(
        bool bActivation,
        const css::uno::Reference<css::drawing::framework::XConfiguration>& rxConfiguration);
    bool IsMasterView(const css::uno::Reference<css::drawing::framework::XView>& xView);

    DECL_LINK(EventMultiplexerListener, ::sd::tools::EventMultiplexerEvent&, void);
};

} // end of namespace sd::framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */

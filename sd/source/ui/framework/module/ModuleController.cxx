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

#include <framework/ModuleController.hxx>
#include <framework/PresentationFactory.hxx>
#include <framework/factories/BasicPaneFactory.hxx>
#include <framework/factories/BasicViewFactory.hxx>
#include <framework/factories/BasicToolBarFactory.hxx>
#include <DrawController.hxx>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/container/XNameAccess.hpp>

#include <tools/ConfigurationAccess.hxx>
#include <comphelper/processfactory.hxx>

#include <comphelper/diagnose_ex.hxx>
#include <rtl/ref.hxx>
#include <sal/log.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing::framework;

namespace sd::framework {

ModuleController::ModuleController(const rtl::Reference<::sd::DrawController>& rxController)
{
    assert(rxController);

    /** Load a list of URL to service mappings.
        The mappings are stored in the
        mpResourceToFactoryMap member.
    */
    ProcessFactory(
        "com.sun.star.drawing.framework.BasicPaneFactory",
        { "private:resource/pane/CenterPane",
          "private:resource/pane/LeftImpressPane",
          "private:resource/pane/BottomImpressPane",
          "private:resource/pane/LeftDrawPane" });
    ProcessFactory(
        "com.sun.star.drawing.framework.BasicViewFactory",
        { "private:resource/view/ImpressView",
          "private:resource/view/GraphicView",
          "private:resource/view/OutlineView",
          "private:resource/view/NotesView",
          "private:resource/view/NotesPanelView",
          "private:resource/view/HandoutView",
          "private:resource/view/SlideSorter",
        "private:resource/view/PresentationView" });
    ProcessFactory(
        u"com.sun.star.drawing.framework.BasicToolBarFactory"_ustr,
        { u"private:resource/toolbar/ViewTabBar"_ustr });

    try
    {
        mxController = rxController;

        InstantiateStartupServices();
    }
    catch (RuntimeException&)
    {}
}

ModuleController::~ModuleController() noexcept
{
}

void ModuleController::disposing(std::unique_lock<std::mutex>&)
{
    // Break the cyclic reference back to DrawController object
    maLoadedFactories.clear();
    maResourceToFactoryMap.clear();
    mxController.clear();
}

void ModuleController::ProcessFactory (const OUString& sServiceName, ::std::vector<OUString> aURLs)
{
    // Get all resource URLs that are created by the factory.

    SAL_INFO("sd.fwk", __func__ << ": ModuleController::adding factory " << sServiceName);

    // Add the resource URLs to the map.
    for (const auto& rResource : aURLs)
    {
        maResourceToFactoryMap[rResource] = sServiceName;
        SAL_INFO("sd.fwk", __func__ << ":    " << rResource);
    }
}

void ModuleController::InstantiateStartupServices()
{
    try
    {
        // Instantiate service.
        // Note that when the new object will be destroyed at the end of
        // this scope when it does not register itself anywhere.
        // Typically it will add itself as ConfigurationChangeListener
        // at the configuration controller.
        sd::framework::PresentationFactory::install(mxController);
    }
    catch (Exception&)
    {
        SAL_WARN("sd.fwk", "ERROR in ModuleController::InstantiateStartupServices");
    }
}

//----- XModuleController -----------------------------------------------------

void SAL_CALL ModuleController::requestResource (const OUString& rsResourceURL)
{
    auto iFactory = maResourceToFactoryMap.find(rsResourceURL);
    if (iFactory == maResourceToFactoryMap.end())
        return;

    // Check that the factory has already been loaded and not been
    // destroyed in the meantime.
    Reference<XInterface> xFactory;
    auto iLoadedFactory = maLoadedFactories.find(iFactory->second);
    if (iLoadedFactory != maLoadedFactories.end())
        xFactory.set(iLoadedFactory->second, UNO_QUERY);
    if (  xFactory.is())
        return;

    // Create a new instance of the factory.
    Reference<uno::XComponentContext> xContext =
        ::comphelper::getProcessComponentContext();

    // Create the factory service.
    if (iFactory->second == "com.sun.star.drawing.framework.BasicPaneFactory")
        xFactory = uno::Reference<css::drawing::framework::XResourceFactory>(new BasicPaneFactory(xContext, mxController));
    else if (iFactory->second == "com.sun.star.drawing.framework.BasicViewFactory")
        xFactory = uno::Reference<css::drawing::framework::XResourceFactory>(new BasicViewFactory(mxController));
    else if (iFactory->second == "com.sun.star.drawing.framework.BasicToolBarFactory")
        xFactory = uno::Reference<css::drawing::framework::XResourceFactory>(new BasicToolBarFactory(mxController));
    else
        throw RuntimeException(u"unknown factory"_ustr);

    // Remember that this factory has been instanced.
    maLoadedFactories[iFactory->second] = xFactory;
}

} // end of namespace sd::framework


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

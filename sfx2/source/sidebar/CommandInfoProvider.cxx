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

#include <sfx2/sidebar/CommandInfoProvider.hxx>

#include <comphelper/processfactory.hxx>
#include <svtools/acceleratorexecute.hxx>
#include <cppuhelper/compbase1.hxx>
#include <cppuhelper/basemutex.hxx>

#include <com/sun/star/frame/ModuleManager.hpp>
#include <com/sun/star/frame/theUICommandDescription.hpp>
#include <com/sun/star/ui/GlobalAcceleratorConfiguration.hpp>
#include <com/sun/star/ui/XUIConfigurationManagerSupplier.hpp>
#include <com/sun/star/ui/theModuleUIConfigurationManagerSupplier.hpp>

using namespace css;
using namespace css::uno;
using ::rtl::OUString;


namespace
{
    typedef ::cppu::WeakComponentImplHelper1 <
        css::lang::XEventListener
        > FrameListenerInterfaceBase;
    class FrameListener
        : public ::cppu::BaseMutex,
          public FrameListenerInterfaceBase
    {
    public:
        FrameListener (sfx2::sidebar::CommandInfoProvider& rInfoProvider, const Reference<frame::XFrame>& rxFrame)
            : FrameListenerInterfaceBase(m_aMutex),
              mrInfoProvider(rInfoProvider),
              mxFrame(rxFrame)
        {
            if (mxFrame.is())
                mxFrame->addEventListener(this);
        }
        virtual ~FrameListener (void)
        {
        }
        virtual void SAL_CALL disposing (void) SAL_OVERRIDE
        {
            if (mxFrame.is())
                mxFrame->removeEventListener(this);
        }
        virtual void SAL_CALL disposing (const css::lang::EventObject& rEvent)
            throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE
        {
            (void)rEvent;
            mrInfoProvider.SetFrame(NULL);
            mxFrame = NULL;
        }

    private:
        sfx2::sidebar::CommandInfoProvider& mrInfoProvider;
        Reference<frame::XFrame> mxFrame;
    };
}



namespace sfx2 { namespace sidebar {

CommandInfoProvider& CommandInfoProvider::Instance (void)
{
    static CommandInfoProvider aProvider;
    return aProvider;
}




CommandInfoProvider::CommandInfoProvider (void)
    : mxContext(comphelper::getProcessComponentContext()),
      mxCachedDataFrame(),
      mxCachedDocumentAcceleratorConfiguration(),
      mxCachedModuleAcceleratorConfiguration(),
      mxCachedGlobalAcceleratorConfiguration(),
      msCachedModuleIdentifier(),
      mxFrameListener()
{
}




CommandInfoProvider::~CommandInfoProvider (void)
{
    if (mxFrameListener.is())
    {
        mxFrameListener->dispose();
        mxFrameListener = NULL;
    }
}




OUString CommandInfoProvider::GetLabelForCommand (
    const OUString& rsCommandName,
    const Reference<frame::XFrame>& rxFrame)
{
    SetFrame(rxFrame);

    const OUString sLabel (GetCommandLabel(rsCommandName));
    const OUString sShortCut (GetCommandShortcut(rsCommandName));
    if (sShortCut.getLength() > 0)
        return sLabel + " (" + sShortCut + ")";
    else
        return sLabel;
}




void CommandInfoProvider::SetFrame (const Reference<frame::XFrame>& rxFrame)
{
    if (rxFrame != mxCachedDataFrame)
    {
        // Detach from the old frame.
        if (mxFrameListener.is())
        {
            mxFrameListener->dispose();
            mxFrameListener = NULL;
        }

        // Release objects that are tied to the old frame.
        mxCachedDocumentAcceleratorConfiguration = NULL;
        mxCachedModuleAcceleratorConfiguration = NULL;
        msCachedModuleIdentifier.clear();
        mxCachedDataFrame = rxFrame;

        // Connect to the new frame.
        if (rxFrame.is())
            mxFrameListener = new FrameListener(*this, rxFrame);
    }
}




Reference<ui::XAcceleratorConfiguration> CommandInfoProvider::GetDocumentAcceleratorConfiguration (void)
{
    if ( ! mxCachedDocumentAcceleratorConfiguration.is())
    {
        // Get the accelerator configuration for the document.
        if (mxCachedDataFrame.is())
        {
            Reference<frame::XController> xController = mxCachedDataFrame->getController();
            if (xController.is())
            {
                Reference<frame::XModel> xModel (xController->getModel());
                if (xModel.is())
                {
                    Reference<ui::XUIConfigurationManagerSupplier> xSupplier (xModel, UNO_QUERY);
                    if (xSupplier.is())
                    {
                        Reference<ui::XUIConfigurationManager> xConfigurationManager(
                            xSupplier->getUIConfigurationManager(),
                            UNO_QUERY);
                        if (xConfigurationManager.is())
                        {
                            mxCachedDocumentAcceleratorConfiguration = xConfigurationManager->getShortCutManager();
                        }
                    }
                }
            }
        }
    }
    return mxCachedDocumentAcceleratorConfiguration;
}




Reference<ui::XAcceleratorConfiguration> CommandInfoProvider::GetModuleAcceleratorConfiguration (void)
{
    if ( ! mxCachedModuleAcceleratorConfiguration.is())
    {
        try
        {
            Reference<ui::XModuleUIConfigurationManagerSupplier> xSupplier  = ui::theModuleUIConfigurationManagerSupplier::get(mxContext);
            Reference<ui::XUIConfigurationManager> xManager (
                xSupplier->getUIConfigurationManager(GetModuleIdentifier()));
            if (xManager.is())
            {
                mxCachedModuleAcceleratorConfiguration = xManager->getShortCutManager();
            }
        }
        catch (Exception&)
        {
        }
    }
    return mxCachedModuleAcceleratorConfiguration;
}




Reference<ui::XAcceleratorConfiguration> CommandInfoProvider::GetGlobalAcceleratorConfiguration (void)
{
    // Get the global accelerator configuration.
    if ( ! mxCachedGlobalAcceleratorConfiguration.is())
    {
        mxCachedGlobalAcceleratorConfiguration = ui::GlobalAcceleratorConfiguration::create(mxContext);
    }

    return mxCachedGlobalAcceleratorConfiguration;
}




OUString CommandInfoProvider::GetModuleIdentifier (void)
{
    if (msCachedModuleIdentifier.getLength() == 0)
    {
        Reference<frame::XModuleManager2> xModuleManager = frame::ModuleManager::create(mxContext);
        msCachedModuleIdentifier = xModuleManager->identify(mxCachedDataFrame);
    }
    return msCachedModuleIdentifier;
}




OUString CommandInfoProvider::GetCommandShortcut (const OUString& rsCommandName)
{
    OUString sShortcut;

    sShortcut = RetrieveShortcutsFromConfiguration(GetDocumentAcceleratorConfiguration(), rsCommandName);
    if (sShortcut.getLength() > 0)
        return sShortcut;

    sShortcut = RetrieveShortcutsFromConfiguration(GetModuleAcceleratorConfiguration(), rsCommandName);
    if (sShortcut.getLength() > 0)
        return sShortcut;

    sShortcut = RetrieveShortcutsFromConfiguration(GetGlobalAcceleratorConfiguration(), rsCommandName);
    if (sShortcut.getLength() > 0)
        return sShortcut;

    return OUString();
}




OUString CommandInfoProvider::RetrieveShortcutsFromConfiguration(
    const Reference<ui::XAcceleratorConfiguration>& rxConfiguration,
    const OUString& rsCommandName)
{
    if (rxConfiguration.is())
    {
        try
        {
            Sequence<OUString> aCommands(1);
            aCommands[0] = rsCommandName;

            Sequence<Any> aKeyCodes (rxConfiguration->getPreferredKeyEventsForCommandList(aCommands));
            if (aCommands.getLength() == 1)
            {
                css::awt::KeyEvent aKeyEvent;
                if (aKeyCodes[0] >>= aKeyEvent)
                {
                    return svt::AcceleratorExecute::st_AWTKey2VCLKey(aKeyEvent).GetName();
                }
            }
        }
        catch (lang::IllegalArgumentException&)
        {
        }
    }
    return OUString();
}




Sequence<beans::PropertyValue> CommandInfoProvider::GetCommandProperties (const OUString& rsCommandName)
{
    Sequence<beans::PropertyValue> aProperties;

    try
    {
        const OUString sModuleIdentifier (GetModuleIdentifier());
        if (sModuleIdentifier.getLength() > 0)
        {
            Reference<container::XNameAccess> xNameAccess  = frame::theUICommandDescription::get(mxContext);
            Reference<container::XNameAccess> xUICommandLabels;
            if (xNameAccess->getByName(sModuleIdentifier) >>= xUICommandLabels)
                xUICommandLabels->getByName(rsCommandName) >>= aProperties;
        }
    }
    catch (Exception&)
    {
    }

    return aProperties;
}




OUString CommandInfoProvider::GetCommandLabel (const OUString& rsCommandName)
{
    const Sequence<beans::PropertyValue> aProperties (GetCommandProperties(rsCommandName));
    for (sal_Int32 nIndex=0; nIndex<aProperties.getLength(); ++nIndex)
    {
        if (aProperties[nIndex].Name.equalsAscii("Name"))
        {
            OUString sLabel;
            aProperties[nIndex].Value >>= sLabel;
            return sLabel;
        }
    }
    return OUString();
}


} } // end of namespace sfx2/framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

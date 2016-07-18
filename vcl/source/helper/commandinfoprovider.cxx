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

#include <vcl/commandinfoprovider.hxx>
#include <vcl/mnemonic.hxx>
#include <comphelper/string.hxx>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/basemutex.hxx>

#include <com/sun/star/frame/ModuleManager.hpp>
#include <com/sun/star/frame/theUICommandDescription.hpp>
#include <com/sun/star/ui/GlobalAcceleratorConfiguration.hpp>
#include <com/sun/star/ui/XUIConfigurationManagerSupplier.hpp>
#include <com/sun/star/ui/theModuleUIConfigurationManagerSupplier.hpp>
#include <com/sun/star/ui/ImageType.hpp>
#include <com/sun/star/ui/XImageManager.hpp>
#include <com/sun/star/awt/KeyModifier.hpp>

#include "svdata.hxx"

using namespace css;
using namespace css::uno;


namespace
{
    typedef ::cppu::WeakComponentImplHelper <
        css::frame::XFrameActionListener
        > FrameListenerInterfaceBase;
    class FrameListener
        : public ::cppu::BaseMutex,
          public FrameListenerInterfaceBase
    {
    public:
        FrameListener (vcl::CommandInfoProvider& rInfoProvider, const Reference<frame::XFrame>& rxFrame)
            : FrameListenerInterfaceBase(m_aMutex),
              mrInfoProvider(rInfoProvider),
              mxFrame(rxFrame)
        {
            if (mxFrame.is())
                mxFrame->addFrameActionListener(this);
        }
        virtual ~FrameListener()
        {
        }
        virtual void SAL_CALL frameAction(const css::frame::FrameActionEvent& aEvent)
            throw (css::uno::RuntimeException, std::exception) override
        {
            // The same frame can be reused for a different component, e.g.
            // starting component from the start center, so need to re-init the cached data.
            if (aEvent.Action == css::frame::FrameAction_COMPONENT_DETACHING)
                mrInfoProvider.SetFrame(nullptr);
        }
        virtual void SAL_CALL disposing() override
        {
            if (mxFrame.is())
                mxFrame->removeFrameActionListener(this);
        }
        virtual void SAL_CALL disposing (const css::lang::EventObject& rEvent)
            throw (RuntimeException, std::exception) override
        {
            (void)rEvent;
            mrInfoProvider.SetFrame(nullptr);
            mxFrame = nullptr;
        }

    private:
        vcl::CommandInfoProvider& mrInfoProvider;
        Reference<frame::XFrame> mxFrame;
    };
}

namespace vcl {

CommandInfoProvider& CommandInfoProvider::Instance()
{
    static CommandInfoProvider aProvider;
    return aProvider;
}

CommandInfoProvider::CommandInfoProvider()
    : mxContext(comphelper::getProcessComponentContext()),
      mxCachedDataFrame(),
      mxCachedDocumentAcceleratorConfiguration(),
      mxCachedModuleAcceleratorConfiguration(),
      mxCachedGlobalAcceleratorConfiguration(),
      msCachedModuleIdentifier(),
      mxFrameListener()
{
    ImplGetSVData()->mpCommandInfoProvider = this;
}

void CommandInfoProvider::dispose()
{
    if (mxFrameListener.is())
    {
        mxFrameListener->dispose();
        mxFrameListener.clear();
    }
    mxCachedGlobalAcceleratorConfiguration.clear();
    mxCachedModuleAcceleratorConfiguration.clear();
    mxCachedDocumentAcceleratorConfiguration.clear();
    mxCachedDataFrame.clear();
    mxContext.clear();
}

CommandInfoProvider::~CommandInfoProvider()
{
    dispose();
}

OUString CommandInfoProvider::GetLabelForCommand (
    const OUString& rsCommandName,
    const Reference<frame::XFrame>& rxFrame)
{
    SetFrame(rxFrame);

    return GetCommandProperty("Name", rsCommandName);
}

OUString CommandInfoProvider::GetMenuLabelForCommand (
    const OUString& rsCommandName,
    const Reference<frame::XFrame>& rxFrame)
{
    SetFrame(rxFrame);

    // Here we want to use "Label", not "Name". "Name" is a stripped-down version of "Label" without accelerators
    // and ellipsis. In the menu, we want to have those accelerators and ellipsis.
    return GetCommandProperty("Label", rsCommandName);
}

OUString CommandInfoProvider::GetPopupLabelForCommand (
    const OUString& rsCommandName,
    const css::uno::Reference<css::frame::XFrame>& rxFrame)
{
    SetFrame(rxFrame);

    OUString sPopupLabel(GetCommandProperty("PopupLabel", rsCommandName));
    if (!sPopupLabel.isEmpty())
        return sPopupLabel;
    return GetCommandProperty("Label", rsCommandName);
}

OUString CommandInfoProvider::GetTooltipForCommand (
    const OUString& rsCommandName,
    const Reference<frame::XFrame>& rxFrame)
{
    SetFrame(rxFrame);

    OUString sLabel (GetCommandProperty("TooltipLabel", rsCommandName));
    if (sLabel.isEmpty()) {
        sLabel = GetPopupLabelForCommand(rsCommandName, rxFrame);
        // Remove '...' at the end and mnemonics (we don't want those in tooltips)
        sLabel = comphelper::string::stripEnd(sLabel, '.');
        sLabel = MnemonicGenerator::EraseAllMnemonicChars(sLabel);
    }

    // Command can be just an alias to another command,
    // so need to get the shortcut of the "real" command.
    const OUString sRealCommand(GetRealCommandForCommand(rsCommandName, rxFrame));
    const OUString sShortCut(GetCommandShortcut(!sRealCommand.isEmpty() ? sRealCommand : rsCommandName, rxFrame));
    if (!sShortCut.isEmpty())
        return sLabel + " (" + sShortCut + ")";
    return sLabel;
}

OUString CommandInfoProvider::GetCommandShortcut (const OUString& rsCommandName,
                                                  const Reference<frame::XFrame>& rxFrame)
{
    SetFrame(rxFrame);

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

OUString CommandInfoProvider::GetRealCommandForCommand(const OUString& rCommandName,
                                                       const css::uno::Reference<frame::XFrame>& rxFrame)
{
    SetFrame(rxFrame);

    return GetCommandProperty("TargetURL", rCommandName);
}

Image CommandInfoProvider::GetImageForCommand(const OUString& rsCommandName, bool bLarge,
                                              const Reference<frame::XFrame>& rxFrame)
{
    SetFrame(rxFrame);

    if (rsCommandName.isEmpty())
        return Image();

    sal_Int16 nImageType(ui::ImageType::COLOR_NORMAL | ui::ImageType::SIZE_DEFAULT);
    if (bLarge)
        nImageType |= ui::ImageType::SIZE_LARGE;

    try
    {
        Reference<frame::XController> xController(rxFrame->getController());
        Reference<frame::XModel> xModel(xController->getModel());

        Reference<ui::XUIConfigurationManagerSupplier> xSupplier(xModel, UNO_QUERY);
        if (xSupplier.is())
        {
            Reference<ui::XUIConfigurationManager> xDocUICfgMgr(xSupplier->getUIConfigurationManager(), UNO_QUERY);
            Reference<ui::XImageManager> xDocImgMgr(xDocUICfgMgr->getImageManager(), UNO_QUERY);

            Sequence< Reference<graphic::XGraphic> > aGraphicSeq;
            Sequence<OUString> aImageCmdSeq { rsCommandName };

            aGraphicSeq = xDocImgMgr->getImages( nImageType, aImageCmdSeq );
            Reference<graphic::XGraphic> xGraphic = aGraphicSeq[0];
            Image aImage(xGraphic);

            if (!!aImage)
                return aImage;
        }
    }
    catch (Exception&)
    {
    }

    try {
        Reference<ui::XModuleUIConfigurationManagerSupplier> xModuleCfgMgrSupplier(ui::theModuleUIConfigurationManagerSupplier::get(mxContext));
        Reference<ui::XUIConfigurationManager> xUICfgMgr(xModuleCfgMgrSupplier->getUIConfigurationManager(GetModuleIdentifier()));

        Sequence< Reference<graphic::XGraphic> > aGraphicSeq;
        Reference<ui::XImageManager> xModuleImageManager(xUICfgMgr->getImageManager(), UNO_QUERY);

        Sequence<OUString> aImageCmdSeq { rsCommandName };

        aGraphicSeq = xModuleImageManager->getImages(nImageType, aImageCmdSeq);

        Reference<graphic::XGraphic> xGraphic(aGraphicSeq[0]);

        return Image(xGraphic);
    }
    catch (Exception&)
    {
    }

    return Image();
}

sal_Int32 CommandInfoProvider::GetPropertiesForCommand (
    const OUString& rsCommandName,
    const Reference<frame::XFrame>& rxFrame)
{
    SetFrame(rxFrame);

    sal_Int32 nValue = 0;
    const Sequence<beans::PropertyValue> aProperties (GetCommandProperties(rsCommandName));
    for (sal_Int32 nIndex=0; nIndex<aProperties.getLength(); ++nIndex)
    {
        if (aProperties[nIndex].Name == "Properties")
        {
            aProperties[nIndex].Value >>= nValue;
            break;
        }
    }
    return nValue;
}

bool CommandInfoProvider::IsRotated(const OUString& rsCommandName)
{
    return ResourceHasKey("private:resource/image/commandrotateimagelist", rsCommandName);
}

bool CommandInfoProvider::IsMirrored(const OUString& rsCommandName)
{
    return ResourceHasKey("private:resource/image/commandmirrorimagelist", rsCommandName);
}

void CommandInfoProvider::SetFrame (const Reference<frame::XFrame>& rxFrame)
{
    if (rxFrame != mxCachedDataFrame)
    {
        // Detach from the old frame.
        if (mxFrameListener.is())
        {
            mxFrameListener->dispose();
            mxFrameListener = nullptr;
        }

        // Release objects that are tied to the old frame.
        mxCachedDocumentAcceleratorConfiguration = nullptr;
        mxCachedModuleAcceleratorConfiguration = nullptr;
        msCachedModuleIdentifier.clear();
        mxCachedDataFrame = rxFrame;

        // Connect to the new frame.
        if (rxFrame.is())
            mxFrameListener = new FrameListener(*this, rxFrame);
    }
}

Reference<ui::XAcceleratorConfiguration> const & CommandInfoProvider::GetDocumentAcceleratorConfiguration()
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

Reference<ui::XAcceleratorConfiguration> const & CommandInfoProvider::GetModuleAcceleratorConfiguration()
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

Reference<ui::XAcceleratorConfiguration> const & CommandInfoProvider::GetGlobalAcceleratorConfiguration()
{
    // Get the global accelerator configuration.
    if ( ! mxCachedGlobalAcceleratorConfiguration.is())
    {
        mxCachedGlobalAcceleratorConfiguration = ui::GlobalAcceleratorConfiguration::create(mxContext);
    }

    return mxCachedGlobalAcceleratorConfiguration;
}

OUString const & CommandInfoProvider::GetModuleIdentifier()
{
    if (msCachedModuleIdentifier.getLength() == 0)
    {
        Reference<frame::XModuleManager2> xModuleManager = frame::ModuleManager::create(mxContext);
        msCachedModuleIdentifier = xModuleManager->identify(mxCachedDataFrame);
    }
    return msCachedModuleIdentifier;
}

OUString CommandInfoProvider::RetrieveShortcutsFromConfiguration(
    const Reference<ui::XAcceleratorConfiguration>& rxConfiguration,
    const OUString& rsCommandName)
{
    if (rxConfiguration.is())
    {
        try
        {
            Sequence<OUString> aCommands { rsCommandName };

            Sequence<Any> aKeyCodes (rxConfiguration->getPreferredKeyEventsForCommandList(aCommands));
            if (aCommands.getLength() == 1)
            {
                awt::KeyEvent aKeyEvent;
                if (aKeyCodes[0] >>= aKeyEvent)
                {
                    return AWTKey2VCLKey(aKeyEvent).GetName();
                }
            }
        }
        catch (css::lang::IllegalArgumentException&)
        {
        }
    }
    return OUString();
}

bool CommandInfoProvider::ResourceHasKey(const OUString& rsResourceName, const OUString& rsCommandName)
{
    Sequence< OUString > aSequence;
    try
    {
        const OUString sModuleIdentifier (GetModuleIdentifier());
        if (!sModuleIdentifier.isEmpty())
        {
            Reference<container::XNameAccess> xNameAccess  = frame::theUICommandDescription::get(mxContext);
            Reference<container::XNameAccess> xUICommandLabels;
            if (xNameAccess->getByName(sModuleIdentifier) >>= xUICommandLabels) {
                xUICommandLabels->getByName(rsResourceName) >>= aSequence;
                for ( sal_Int32 i = 0; i < aSequence.getLength(); i++ )
                {
                    if (aSequence[i] == rsCommandName)
                        return true;
                }
            }
        }
    }
    catch (Exception&)
    {
    }
    return false;
}

Sequence<beans::PropertyValue> CommandInfoProvider::GetCommandProperties(const OUString& rsCommandName)
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

OUString CommandInfoProvider::GetCommandProperty(const OUString& rsProperty, const OUString& rsCommandName)
{
    const Sequence<beans::PropertyValue> aProperties (GetCommandProperties(rsCommandName));
    for (sal_Int32 nIndex=0; nIndex<aProperties.getLength(); ++nIndex)
    {
        if (aProperties[nIndex].Name == rsProperty)
        {
            OUString sLabel;
            aProperties[nIndex].Value >>= sLabel;
            return sLabel;
        }
    }
    return OUString();
}

OUString CommandInfoProvider::GetCommandPropertyFromModule( const OUString& rCommandName, const OUString& rModuleName )
{
    OUString sLabel;
    if ( rCommandName.isEmpty() )
        return sLabel;

    Sequence<beans::PropertyValue> aProperties;
    try
    {
        if( rModuleName.getLength() > 0)
        {
            Reference<container::XNameAccess> xNameAccess  = frame::theUICommandDescription::get(mxContext);
            Reference<container::XNameAccess> xUICommandLabels;
            if (xNameAccess->getByName( rModuleName ) >>= xUICommandLabels )
                xUICommandLabels->getByName(rCommandName) >>= aProperties;

            for (sal_Int32 nIndex=0; nIndex<aProperties.getLength(); ++nIndex)
            {
                if(aProperties[nIndex].Name == "Label")
                {
                    aProperties[nIndex].Value >>= sLabel;
                    return sLabel;
                }
            }
        }
    }
    catch (Exception&)
    {
    }
    return OUString();
}

vcl::KeyCode CommandInfoProvider::AWTKey2VCLKey(const awt::KeyEvent& aAWTKey)
{
    bool bShift = ((aAWTKey.Modifiers & awt::KeyModifier::SHIFT) == awt::KeyModifier::SHIFT );
    bool bMod1  = ((aAWTKey.Modifiers & awt::KeyModifier::MOD1 ) == awt::KeyModifier::MOD1  );
    bool bMod2  = ((aAWTKey.Modifiers & awt::KeyModifier::MOD2 ) == awt::KeyModifier::MOD2  );
    bool bMod3  = ((aAWTKey.Modifiers & awt::KeyModifier::MOD3 ) == awt::KeyModifier::MOD3  );
    sal_uInt16   nKey   = (sal_uInt16)aAWTKey.KeyCode;

    return vcl::KeyCode(nKey, bShift, bMod1, bMod2, bMod3);
}


} // end of namespace vcl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

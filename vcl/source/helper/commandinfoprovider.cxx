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


namespace vcl {

CommandInfoProvider::CommandInfoProvider() { }

CommandInfoProvider::~CommandInfoProvider()
{
}

OUString CommandInfoProvider::GetLabelForCommand (
    const OUString& rsCommandName,
    const Reference<frame::XFrame>& rxFrame)
{

    return GetCommandProperty("Name", rsCommandName, rxFrame);
}

OUString CommandInfoProvider::GetMenuLabelForCommand (
    const OUString& rsCommandName,
    const Reference<frame::XFrame>& rxFrame)
{

    // Here we want to use "Label", not "Name". "Name" is a stripped-down version of "Label" without accelerators
    // and ellipsis. In the menu, we want to have those accelerators and ellipsis.
    return GetCommandProperty("Label", rsCommandName, rxFrame);
}

OUString CommandInfoProvider::GetPopupLabelForCommand (
    const OUString& rsCommandName,
    const css::uno::Reference<css::frame::XFrame>& rxFrame)
{

    OUString sPopupLabel(GetCommandProperty("PopupLabel", rsCommandName, rxFrame));
    if (!sPopupLabel.isEmpty())
        return sPopupLabel;
    return GetCommandProperty("Label", rsCommandName, rxFrame);
}

OUString CommandInfoProvider::GetTooltipForCommand (
    const OUString& rsCommandName,
    const Reference<frame::XFrame>& rxFrame)
{

    OUString sLabel (GetCommandProperty("TooltipLabel", rsCommandName, rxFrame));
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

    OUString sShortcut;

    sShortcut = RetrieveShortcutsFromConfiguration(GetDocumentAcceleratorConfiguration(rxFrame), rsCommandName);
    if (sShortcut.getLength() > 0)
        return sShortcut;

    sShortcut = RetrieveShortcutsFromConfiguration(GetModuleAcceleratorConfiguration(rxFrame), rsCommandName);
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

    return GetCommandProperty("TargetURL", rCommandName, rxFrame);
}

BitmapEx CommandInfoProvider::GetBitmapForCommand(const OUString& rsCommandName,
                                                 const Reference<frame::XFrame>& rxFrame,
                                                 vcl::ImageType eImageType)
{

    if (rsCommandName.isEmpty())
        return BitmapEx();

    sal_Int16 nImageType(ui::ImageType::COLOR_NORMAL | ui::ImageType::SIZE_DEFAULT);

    if (eImageType == vcl::ImageType::Size26)
        nImageType |= ui::ImageType::SIZE_LARGE;
    else if (eImageType == vcl::ImageType::Size32)
        nImageType |= ui::ImageType::SIZE_32;

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
            const Graphic aGraphic(xGraphic);
            BitmapEx aBitmap(aGraphic.GetBitmapEx());

            if (!!aBitmap)
                return aBitmap;
        }
    }
    catch (Exception&)
    {
    }

    try {
        Reference<ui::XModuleUIConfigurationManagerSupplier> xModuleCfgMgrSupplier(ui::theModuleUIConfigurationManagerSupplier::get(comphelper::getProcessComponentContext()));
        Reference<ui::XUIConfigurationManager> xUICfgMgr(xModuleCfgMgrSupplier->getUIConfigurationManager(GetModuleIdentifier(rxFrame)));

        Sequence< Reference<graphic::XGraphic> > aGraphicSeq;
        Reference<ui::XImageManager> xModuleImageManager(xUICfgMgr->getImageManager(), UNO_QUERY);

        Sequence<OUString> aImageCmdSeq { rsCommandName };

        aGraphicSeq = xModuleImageManager->getImages(nImageType, aImageCmdSeq);

        Reference<graphic::XGraphic> xGraphic(aGraphicSeq[0]);

        const Graphic aGraphic(xGraphic);

        return aGraphic.GetBitmapEx();
    }
    catch (Exception&)
    {
    }

    return BitmapEx();
}

Image CommandInfoProvider::GetImageForCommand(const OUString& rsCommandName,
                                              const Reference<frame::XFrame>& rxFrame,
                                              vcl::ImageType eImageType)
{
    return Image(GetBitmapForCommand(rsCommandName, rxFrame, eImageType));
}

sal_Int32 CommandInfoProvider::GetPropertiesForCommand (
    const OUString& rsCommandName,
    const Reference<frame::XFrame>& rxFrame)
{

    sal_Int32 nValue = 0;
    const Sequence<beans::PropertyValue> aProperties (GetCommandProperties(rsCommandName, rxFrame));
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

bool CommandInfoProvider::IsRotated(const OUString& rsCommandName, const Reference<frame::XFrame>& rxFrame)
{
    return ResourceHasKey("private:resource/image/commandrotateimagelist", rsCommandName, rxFrame);
}

bool CommandInfoProvider::IsMirrored(const OUString& rsCommandName, const Reference<frame::XFrame>& rxFrame)
{
    return ResourceHasKey("private:resource/image/commandmirrorimagelist", rsCommandName, rxFrame);
}

bool CommandInfoProvider::IsExperimental(const OUString& rsCommandName,
                                         const OUString& rModuleName)
{
    Sequence<beans::PropertyValue> aProperties;
    try
    {
        if( rModuleName.getLength() > 0)
        {
            Reference<container::XNameAccess> xNameAccess  = frame::theUICommandDescription::get(comphelper::getProcessComponentContext());
            Reference<container::XNameAccess> xUICommandLabels;
            if (xNameAccess->getByName( rModuleName ) >>= xUICommandLabels )
                xUICommandLabels->getByName(rsCommandName) >>= aProperties;

            for (sal_Int32 nIndex=0; nIndex<aProperties.getLength(); ++nIndex)
            {
                if (aProperties[nIndex].Name == "IsExperimental")
                {
                    bool bValue;
                    return (aProperties[nIndex].Value >>= bValue) && bValue;
                }
            }
        }
    }
    catch (Exception&)
    {
    }
    return false;
}

Reference<ui::XAcceleratorConfiguration> const CommandInfoProvider::GetDocumentAcceleratorConfiguration(const Reference<frame::XFrame>& rxFrame)
{
    Reference<frame::XController> xController = rxFrame->getController();
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
                    return xConfigurationManager->getShortCutManager();
                }
            }
        }
    }
    return nullptr;
}

Reference<ui::XAcceleratorConfiguration> const CommandInfoProvider::GetModuleAcceleratorConfiguration(const Reference<frame::XFrame>& rxFrame)
{
    css::uno::Reference<css::ui::XAcceleratorConfiguration> curModuleAcceleratorConfiguration;
    try
    {
        Reference<ui::XModuleUIConfigurationManagerSupplier> xSupplier  = ui::theModuleUIConfigurationManagerSupplier::get(comphelper::getProcessComponentContext());
        Reference<ui::XUIConfigurationManager> xManager (
            xSupplier->getUIConfigurationManager(GetModuleIdentifier(rxFrame)));
        if (xManager.is())
        {
            curModuleAcceleratorConfiguration = xManager->getShortCutManager();
        }
    }
    catch (Exception&)
    {
    }
    return curModuleAcceleratorConfiguration;
}

Reference<ui::XAcceleratorConfiguration> const CommandInfoProvider::GetGlobalAcceleratorConfiguration()
{
    // Get the global accelerator configuration.
    return ui::GlobalAcceleratorConfiguration::create(comphelper::getProcessComponentContext());

}

OUString const CommandInfoProvider::GetModuleIdentifier(const Reference<frame::XFrame>& rxFrame)
{
    Reference<frame::XModuleManager2> xModuleManager = frame::ModuleManager::create(comphelper::getProcessComponentContext());
    return xModuleManager->identify(rxFrame);
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

bool CommandInfoProvider::ResourceHasKey(const OUString& rsResourceName, const OUString& rsCommandName, const Reference<frame::XFrame>& rxFrame)
{
    Sequence< OUString > aSequence;
    try
    {
        const OUString sModuleIdentifier (GetModuleIdentifier(rxFrame));
        if (!sModuleIdentifier.isEmpty())
        {
            Reference<container::XNameAccess> xNameAccess  = frame::theUICommandDescription::get(comphelper::getProcessComponentContext());
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

Sequence<beans::PropertyValue> CommandInfoProvider::GetCommandProperties(const OUString& rsCommandName, const Reference<frame::XFrame>& rxFrame)
{
    Sequence<beans::PropertyValue> aProperties;

    try
    {
        const OUString sModuleIdentifier (GetModuleIdentifier(rxFrame));
        if (sModuleIdentifier.getLength() > 0)
        {
            Reference<container::XNameAccess> xNameAccess  = frame::theUICommandDescription::get(comphelper::getProcessComponentContext());
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

OUString CommandInfoProvider::GetCommandProperty(const OUString& rsProperty, const OUString& rsCommandName, const Reference<frame::XFrame>& rxFrame)
{
    const Sequence<beans::PropertyValue> aProperties (GetCommandProperties(rsCommandName, rxFrame));
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
            Reference<container::XNameAccess> xNameAccess  = frame::theUICommandDescription::get(comphelper::getProcessComponentContext());
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

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
#include <vcl/graph.hxx>
#include <vcl/keycod.hxx>
#include <vcl/mnemonic.hxx>
#include <comphelper/string.hxx>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/weakref.hxx>

#include <com/sun/star/frame/ModuleManager.hpp>
#include <com/sun/star/frame/theUICommandDescription.hpp>
#include <com/sun/star/ui/GlobalAcceleratorConfiguration.hpp>
#include <com/sun/star/ui/XUIConfigurationManagerSupplier.hpp>
#include <com/sun/star/ui/theModuleUIConfigurationManagerSupplier.hpp>
#include <com/sun/star/ui/ImageType.hpp>
#include <com/sun/star/ui/XImageManager.hpp>
#include <com/sun/star/awt/KeyModifier.hpp>

using namespace css;
using namespace css::uno;

namespace vcl { namespace CommandInfoProvider {

Reference<container::XNameAccess> const GetCommandDescription()
{
    static WeakReference<container::XNameAccess> xWeakRef;
    css::uno::Reference<container::XNameAccess> xRef(xWeakRef);

    if (!xRef.is())
    {
        xRef = frame::theUICommandDescription::get(comphelper::getProcessComponentContext());
        xWeakRef = xRef;
    }

    return xRef;
}

Reference<ui::XModuleUIConfigurationManagerSupplier> const GetModuleConfigurationSupplier()
{
    static WeakReference<ui::XModuleUIConfigurationManagerSupplier> xWeakRef;
    css::uno::Reference<ui::XModuleUIConfigurationManagerSupplier> xRef(xWeakRef);

    if (!xRef.is())
    {
        xRef = ui::theModuleUIConfigurationManagerSupplier::get(comphelper::getProcessComponentContext());
        xWeakRef = xRef;
    }

    return xRef;
}

Reference<ui::XAcceleratorConfiguration> const GetGlobalAcceleratorConfiguration()
{
    static WeakReference<ui::XAcceleratorConfiguration> xWeakRef;
    css::uno::Reference<ui::XAcceleratorConfiguration> xRef(xWeakRef);

    if (!xRef.is())
    {
        xRef = ui::GlobalAcceleratorConfiguration::create(comphelper::getProcessComponentContext());
        xWeakRef = xRef;
    }

    return xRef;
}

Reference<ui::XAcceleratorConfiguration> const GetDocumentAcceleratorConfiguration(const Reference<frame::XFrame>& rxFrame)
{
    Reference<frame::XController> xController = rxFrame->getController();
    if (xController.is())
    {
        Reference<ui::XUIConfigurationManagerSupplier> xSupplier(xController->getModel(), UNO_QUERY);
        if (xSupplier.is())
        {
            Reference<ui::XUIConfigurationManager> xConfigurationManager(
                xSupplier->getUIConfigurationManager());
            if (xConfigurationManager.is())
            {
                return xConfigurationManager->getShortCutManager();
            }
        }
    }
    return nullptr;
}

Reference<ui::XAcceleratorConfiguration> const GetModuleAcceleratorConfiguration(const Reference<frame::XFrame>& rxFrame)
{
    css::uno::Reference<css::ui::XAcceleratorConfiguration> curModuleAcceleratorConfiguration;
    try
    {
        Reference<ui::XModuleUIConfigurationManagerSupplier> xSupplier(GetModuleConfigurationSupplier());
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

vcl::KeyCode AWTKey2VCLKey(const awt::KeyEvent& aAWTKey)
{
    bool bShift = ((aAWTKey.Modifiers & awt::KeyModifier::SHIFT) == awt::KeyModifier::SHIFT );
    bool bMod1  = ((aAWTKey.Modifiers & awt::KeyModifier::MOD1 ) == awt::KeyModifier::MOD1  );
    bool bMod2  = ((aAWTKey.Modifiers & awt::KeyModifier::MOD2 ) == awt::KeyModifier::MOD2  );
    bool bMod3  = ((aAWTKey.Modifiers & awt::KeyModifier::MOD3 ) == awt::KeyModifier::MOD3  );
    sal_uInt16   nKey   = static_cast<sal_uInt16>(aAWTKey.KeyCode);

    return vcl::KeyCode(nKey, bShift, bMod1, bMod2, bMod3);
}

OUString RetrieveShortcutsFromConfiguration(
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

bool ResourceHasKey(const OUString& rsResourceName, const OUString& rsCommandName, const OUString& rsModuleName)
{
    Sequence< OUString > aSequence;
    try
    {
        if (!rsModuleName.isEmpty())
        {
            Reference<container::XNameAccess> xNameAccess(GetCommandDescription());
            Reference<container::XNameAccess> xUICommandLabels;
            if (xNameAccess->getByName(rsModuleName) >>= xUICommandLabels)
            {
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

Sequence<beans::PropertyValue> GetCommandProperties(const OUString& rsCommandName, const OUString& rsModuleName)
{
    Sequence<beans::PropertyValue> aProperties;

    try
    {
        if (!rsModuleName.isEmpty())
        {
            Reference<container::XNameAccess> xNameAccess(GetCommandDescription());
            Reference<container::XNameAccess> xUICommandLabels;
            if (xNameAccess->getByName(rsModuleName) >>= xUICommandLabels)
                xUICommandLabels->getByName(rsCommandName) >>= aProperties;
        }
    }
    catch (Exception&)
    {
    }

    return aProperties;
}

OUString GetCommandProperty(const OUString& rsProperty, const OUString& rsCommandName, const OUString& rsModuleName)
{
    const Sequence<beans::PropertyValue> aProperties (GetCommandProperties(rsCommandName, rsModuleName));
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

OUString GetLabelForCommand (
    const OUString& rsCommandName,
    const OUString& rsModuleName)
{
    return GetCommandProperty("Name", rsCommandName, rsModuleName);
}

OUString GetMenuLabelForCommand (
    const OUString& rsCommandName,
    const OUString& rsModuleName)
{
    // Here we want to use "Label", not "Name". "Name" is a stripped-down version of "Label" without accelerators
    // and ellipsis. In the menu, we want to have those accelerators and ellipsis.
    return GetCommandProperty("Label", rsCommandName, rsModuleName);
}

OUString GetPopupLabelForCommand (
    const OUString& rsCommandName,
    const OUString& rsModuleName)
{
    OUString sPopupLabel(GetCommandProperty("PopupLabel", rsCommandName, rsModuleName));
    if (!sPopupLabel.isEmpty())
        return sPopupLabel;
    return GetCommandProperty("Label", rsCommandName, rsModuleName);
}

OUString GetTooltipForCommand (
    const OUString& rsCommandName,
    const Reference<frame::XFrame>& rxFrame)
{
    OUString sModuleName(GetModuleIdentifier(rxFrame));
    OUString sLabel (GetCommandProperty("TooltipLabel", rsCommandName, sModuleName));
    if (sLabel.isEmpty()) {
        sLabel = GetPopupLabelForCommand(rsCommandName, sModuleName);
        // Remove '...' at the end and mnemonics (we don't want those in tooltips)
        sLabel = comphelper::string::stripEnd(sLabel, '.');
        sLabel = MnemonicGenerator::EraseAllMnemonicChars(sLabel);
    }

    // Command can be just an alias to another command,
    // so need to get the shortcut of the "real" command.
    const OUString sRealCommand(GetRealCommandForCommand(rsCommandName, sModuleName));
    const OUString sShortCut(GetCommandShortcut(!sRealCommand.isEmpty() ? sRealCommand : rsCommandName, rxFrame));
    if (!sShortCut.isEmpty())
        return sLabel + " (" + sShortCut + ")";
    return sLabel;
}

OUString GetCommandShortcut (const OUString& rsCommandName,
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

OUString GetRealCommandForCommand(const OUString& rCommandName,
                                  const OUString& rsModuleName)
{
    return GetCommandProperty("TargetURL", rCommandName, rsModuleName);
}

BitmapEx GetBitmapForCommand(const OUString& rsCommandName,
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
        Reference<frame::XController> xController(rxFrame->getController(), UNO_SET_THROW);
        Reference<ui::XUIConfigurationManagerSupplier> xSupplier(xController->getModel(), UNO_QUERY);
        if (xSupplier.is())
        {
            Reference<ui::XUIConfigurationManager> xDocUICfgMgr(xSupplier->getUIConfigurationManager());
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
        Reference<ui::XModuleUIConfigurationManagerSupplier> xModuleCfgMgrSupplier(GetModuleConfigurationSupplier());
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

Image GetImageForCommand(const OUString& rsCommandName,
                         const Reference<frame::XFrame>& rxFrame,
                         vcl::ImageType eImageType)
{
    return Image(GetBitmapForCommand(rsCommandName, rxFrame, eImageType));
}

sal_Int32 GetPropertiesForCommand (
    const OUString& rsCommandName,
    const OUString& rsModuleName)
{

    sal_Int32 nValue = 0;
    const Sequence<beans::PropertyValue> aProperties (GetCommandProperties(rsCommandName, rsModuleName));
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

bool IsRotated(const OUString& rsCommandName, const OUString& rsModuleName)
{
    return ResourceHasKey("private:resource/image/commandrotateimagelist", rsCommandName, rsModuleName);
}

bool IsMirrored(const OUString& rsCommandName, const OUString& rsModuleName)
{
    return ResourceHasKey("private:resource/image/commandmirrorimagelist", rsCommandName, rsModuleName);
}

bool IsExperimental(const OUString& rsCommandName, const OUString& rModuleName)
{
    Sequence<beans::PropertyValue> aProperties;
    try
    {
        if( rModuleName.getLength() > 0)
        {
            Reference<container::XNameAccess> xNameAccess(GetCommandDescription());
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

OUString const GetModuleIdentifier(const Reference<frame::XFrame>& rxFrame)
{
    static WeakReference<frame::XModuleManager2> xWeakRef;
    css::uno::Reference<frame::XModuleManager2> xRef(xWeakRef);

    if (!xRef.is())
    {
        xRef = frame::ModuleManager::create(comphelper::getProcessComponentContext());
        xWeakRef = xRef;
    }

    try
    {
        return xRef->identify(rxFrame);
    }
    catch (const Exception&)
    {}

    return OUString();
}

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

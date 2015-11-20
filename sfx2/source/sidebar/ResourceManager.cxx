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

#include <sfx2/sidebar/ResourceManager.hxx>
#include <sfx2/sidebar/Tools.hxx>

#include <unotools/confignode.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/namedvaluecollection.hxx>
#include <comphelper/types.hxx>

#include <rtl/ustrbuf.hxx>
#include <tools/diagnose_ex.h>

#include <com/sun/star/frame/ModuleManager.hpp>
#include <com/sun/star/ui/XUpdateModel.hpp>

#include <map>

using namespace css;
using namespace css::uno;

namespace sfx2 { namespace sidebar {

namespace
{

OUString getString(utl::OConfigurationNode const & aNode, const char* pNodeName)
{
    return comphelper::getString(aNode.getNodeValue(pNodeName));
}
sal_Int32 getInt32(utl::OConfigurationNode const & aNode, const char* pNodeName)
{
    return comphelper::getINT32(aNode.getNodeValue(pNodeName));
}
bool getBool(utl::OConfigurationNode const & aNode, const char* pNodeName)
{
    return comphelper::getBOOL(aNode.getNodeValue(pNodeName));
}

} //end anonymous namespace

ResourceManager& ResourceManager::Instance()
{
    static ResourceManager s_SidebarResourceManagerInstance;
    return s_SidebarResourceManagerInstance;
}

ResourceManager::ResourceManager()
    : maDecks(),
      maPanels(),
      maProcessedApplications(),
      maMiscOptions()
{
    ReadDeckList();
    ReadPanelList();
}

ResourceManager::~ResourceManager()
{
}

const DeckDescriptor* ResourceManager::GetDeckDescriptor(const OUString& rsDeckId) const
{
    DeckContainer::const_iterator iDeck;
    for (iDeck = maDecks.begin(); iDeck != maDecks.end(); ++iDeck)
    {
        if (iDeck->mbExperimental && !maMiscOptions.IsExperimentalMode())
            continue;
        if (iDeck->msId.equals(rsDeckId))
            return &*iDeck;
    }
    return nullptr;
}

const PanelDescriptor* ResourceManager::GetPanelDescriptor(const OUString& rsPanelId) const
{
    PanelContainer::const_iterator iPanel;
    for (iPanel = maPanels.begin(); iPanel != maPanels.end(); ++iPanel)
    {
        if (iPanel->msId.equals(rsPanelId))
            return &*iPanel;
    }
    return nullptr;
}

void ResourceManager::SetIsDeckEnabled(const OUString& rsDeckId, const bool bIsEnabled)
{
    DeckContainer::iterator iDeck;
    for (iDeck = maDecks.begin(); iDeck != maDecks.end(); ++iDeck)
    {
        if (iDeck->mbExperimental && !maMiscOptions.IsExperimentalMode())
            continue;
        if (iDeck->msId.equals(rsDeckId))
        {
            iDeck->mbIsEnabled = bIsEnabled;
            return;
        }
    }
}

void ResourceManager::SetDeckTitle(const OUString& rsDeckId, const OUString& sTitle)
{
    DeckContainer::iterator iDeck;
    for (iDeck = maDecks.begin(); iDeck != maDecks.end(); ++iDeck)
    {
        if (iDeck->mbExperimental && !maMiscOptions.IsExperimentalMode())
            continue;
        if (iDeck->msId.equals(rsDeckId))
        {
            iDeck->msTitle = sTitle;
            iDeck->msHelpText = sTitle;
            return;
        }
    }
}

void ResourceManager::SetDeckToDescriptor(const OUString& rsDeckId, VclPtr<Deck> aDeck)
{
    DeckContainer::iterator iDeck;
    for (iDeck = maDecks.begin(); iDeck != maDecks.end(); ++iDeck)
    {
        if (iDeck->mbExperimental && !maMiscOptions.IsExperimentalMode())
            continue;
        if (iDeck->msId.equals(rsDeckId))
        {
            iDeck->mpDeck = aDeck;
            return;
        }
    }
}

void ResourceManager::SetDeckOrderIndex(const OUString& rsDeckId, const sal_Int32 orderIndex)
{
    DeckContainer::iterator iDeck;
    for (iDeck = maDecks.begin(); iDeck != maDecks.end(); ++iDeck)
    {
        if (iDeck->mbExperimental && !maMiscOptions.IsExperimentalMode())
            continue;
        if (iDeck->msId.equals(rsDeckId))
        {
            iDeck->mnOrderIndex = orderIndex;
            return;
        }
    }
}

void ResourceManager::SetPanelOrderIndex(const OUString& rsPanelId, const sal_Int32 orderIndex)
{
    PanelContainer::iterator iPanel;
    for (iPanel = maPanels.begin(); iPanel != maPanels.end(); ++iPanel)
    {
        if (iPanel->mbExperimental && !maMiscOptions.IsExperimentalMode())
            continue;
        if (iPanel->msId.equals(rsPanelId))
        {
            iPanel->mnOrderIndex = orderIndex;
            return;
        }
    }
}

const ResourceManager::DeckContextDescriptorContainer& ResourceManager::GetMatchingDecks (
                                                            DeckContextDescriptorContainer& rDecks,
                                                            const Context& rContext,
                                                            const bool bIsDocumentReadOnly,
                                                            const Reference<frame::XController>& rxController)
{
    ReadLegacyAddons(rxController);

    std::multimap<sal_Int32,DeckContextDescriptor> aOrderedIds;
    DeckContainer::const_iterator iDeck;
    for (iDeck = maDecks.begin(); iDeck != maDecks.end(); ++iDeck)
    {
        if (iDeck->mbExperimental && !maMiscOptions.IsExperimentalMode())
            continue;
        const DeckDescriptor& rDeckDescriptor (*iDeck);
        if (rDeckDescriptor.maContextList.GetMatch(rContext) == nullptr)
            continue;
        DeckContextDescriptor aDeckContextDescriptor;
        aDeckContextDescriptor.msId = rDeckDescriptor.msId;
        aDeckContextDescriptor.mbIsEnabled =
            ! bIsDocumentReadOnly
            || IsDeckEnabled(rDeckDescriptor.msId, rContext, rxController);
        aOrderedIds.insert(::std::multimap<sal_Int32,DeckContextDescriptor>::value_type(
                rDeckDescriptor.mnOrderIndex,
                aDeckContextDescriptor));
    }

    std::multimap<sal_Int32,DeckContextDescriptor>::const_iterator iId;
    for (iId = aOrderedIds.begin(); iId != aOrderedIds.end(); ++iId)
    {
        rDecks.push_back(iId->second);
    }

    return rDecks;
}

const ResourceManager::PanelContextDescriptorContainer& ResourceManager::GetMatchingPanels (
                                                            PanelContextDescriptorContainer& rPanelIds,
                                                            const Context& rContext,
                                                            const OUString& rsDeckId,
                                                            const Reference<frame::XController>& rxController)
{
    ReadLegacyAddons(rxController);

    std::multimap<sal_Int32, PanelContextDescriptor> aOrderedIds;
    PanelContainer::const_iterator iPanel;
    for (iPanel = maPanels.begin(); iPanel != maPanels.end(); ++iPanel)
    {
        const PanelDescriptor& rPanelDescriptor (*iPanel);
        if (rPanelDescriptor.mbExperimental && !maMiscOptions.IsExperimentalMode())
            continue;
        if ( ! rPanelDescriptor.msDeckId.equals(rsDeckId))
            continue;

        const ContextList::Entry* pEntry = rPanelDescriptor.maContextList.GetMatch(rContext);
        if (pEntry == nullptr)
            continue;

        PanelContextDescriptor aPanelContextDescriptor;
        aPanelContextDescriptor.msId = rPanelDescriptor.msId;
        aPanelContextDescriptor.msMenuCommand = pEntry->msMenuCommand;
        aPanelContextDescriptor.mbIsInitiallyVisible = pEntry->mbIsInitiallyVisible;
        aPanelContextDescriptor.mbShowForReadOnlyDocuments = rPanelDescriptor.mbShowForReadOnlyDocuments;
        aOrderedIds.insert(std::multimap<sal_Int32, PanelContextDescriptor>::value_type(
                                                    rPanelDescriptor.mnOrderIndex,
                                                    aPanelContextDescriptor));
    }

    std::multimap<sal_Int32,PanelContextDescriptor>::const_iterator iId;
    for (iId = aOrderedIds.begin(); iId != aOrderedIds.end(); ++iId)
    {
        rPanelIds.push_back(iId->second);
    }

    return rPanelIds;
}

void ResourceManager::ReadDeckList()
{
    const utl::OConfigurationTreeRoot aDeckRootNode(
                                        comphelper::getProcessComponentContext(),
                                        OUString("org.openoffice.Office.UI.Sidebar/Content/DeckList"),
                                        false);
    if (!aDeckRootNode.isValid())
        return;

    const Sequence<OUString> aDeckNodeNames (aDeckRootNode.getNodeNames());
    const sal_Int32 nCount(aDeckNodeNames.getLength());
    maDecks.resize(nCount);
    sal_Int32 nWriteIndex(0);
    for (sal_Int32 nReadIndex(0); nReadIndex<nCount; ++nReadIndex)
    {
        const utl::OConfigurationNode aDeckNode(aDeckRootNode.openNode(aDeckNodeNames[nReadIndex]));
        if (!aDeckNode.isValid())
            continue;

        DeckDescriptor& rDeckDescriptor (maDecks[nWriteIndex++]);

        rDeckDescriptor.msTitle = getString(aDeckNode, "Title");
        rDeckDescriptor.msId = getString(aDeckNode, "Id");
        rDeckDescriptor.msIconURL = getString(aDeckNode, "IconURL");
        rDeckDescriptor.msHighContrastIconURL = getString(aDeckNode, "HighContrastIconURL");
        rDeckDescriptor.msTitleBarIconURL = getString(aDeckNode, "TitleBarIconURL");
        rDeckDescriptor.msHighContrastTitleBarIconURL = getString(aDeckNode, "HighContrastTitleBarIconURL");
        rDeckDescriptor.msHelpURL = getString(aDeckNode, "HelpURL");
        rDeckDescriptor.msHelpText = rDeckDescriptor.msTitle;
        rDeckDescriptor.mbIsEnabled = true;
        rDeckDescriptor.mnOrderIndex = getInt32(aDeckNode, "OrderIndex");
        rDeckDescriptor.mbExperimental = getBool(aDeckNode, "IsExperimental");

        ReadContextList(
            aDeckNode,
            rDeckDescriptor.maContextList,
            OUString());
    }

    // When there where invalid nodes then we have to adapt the size
    // of the deck vector.
    if (nWriteIndex<nCount)
        maDecks.resize(nWriteIndex);
}

void ResourceManager::ReadPanelList()
{
    const utl::OConfigurationTreeRoot aPanelRootNode(
                                        comphelper::getProcessComponentContext(),
                                        OUString("org.openoffice.Office.UI.Sidebar/Content/PanelList"),
                                        false);
    if (!aPanelRootNode.isValid())
        return;

    const Sequence<OUString> aPanelNodeNames (aPanelRootNode.getNodeNames());
    const sal_Int32 nCount (aPanelNodeNames.getLength());
    maPanels.resize(nCount);
    sal_Int32 nWriteIndex (0);
    for (sal_Int32 nReadIndex(0); nReadIndex<nCount; ++nReadIndex)
    {
        const utl::OConfigurationNode aPanelNode (aPanelRootNode.openNode(aPanelNodeNames[nReadIndex]));
        if (!aPanelNode.isValid())
            continue;

        PanelDescriptor& rPanelDescriptor (maPanels[nWriteIndex++]);

        rPanelDescriptor.msTitle = getString(aPanelNode, "Title");
        rPanelDescriptor.mbIsTitleBarOptional = getBool(aPanelNode, "TitleBarIsOptional");
        rPanelDescriptor.msId = getString(aPanelNode, "Id");
        rPanelDescriptor.msDeckId = getString(aPanelNode, "DeckId");
        rPanelDescriptor.msTitleBarIconURL = getString(aPanelNode, "TitleBarIconURL");
        rPanelDescriptor.msHighContrastTitleBarIconURL = getString(aPanelNode, "HighContrastTitleBarIconURL");
        rPanelDescriptor.msHelpURL = getString(aPanelNode, "HelpURL");
        rPanelDescriptor.msImplementationURL = getString(aPanelNode, "ImplementationURL");
        rPanelDescriptor.mnOrderIndex = getInt32(aPanelNode, "OrderIndex");
        rPanelDescriptor.mbShowForReadOnlyDocuments = getBool(aPanelNode, "ShowForReadOnlyDocument");
        rPanelDescriptor.mbWantsCanvas = getBool(aPanelNode, "WantsCanvas");
        rPanelDescriptor.mbExperimental = getBool(aPanelNode, "IsExperimental");
        const OUString sDefaultMenuCommand(getString(aPanelNode, "DefaultMenuCommand"));

        ReadContextList(aPanelNode, rPanelDescriptor.maContextList, sDefaultMenuCommand);
    }

    // When there where invalid nodes then we have to adapt the size
    // of the deck vector.
    if (nWriteIndex<nCount)
        maPanels.resize(nWriteIndex);
}

void ResourceManager::ReadContextList (
                        const utl::OConfigurationNode& rParentNode,
                        ContextList& rContextList,
                        const OUString& rsDefaultMenuCommand)
{
    const Any aValue = rParentNode.getNodeValue("ContextList");
    Sequence<OUString> aValues;
    sal_Int32 nCount;
    if (aValue >>= aValues)
        nCount = aValues.getLength();
    else
        nCount = 0;

    for (sal_Int32 nIndex=0; nIndex<nCount; ++nIndex)
    {
        const OUString sValue (aValues[nIndex]);
        sal_Int32 nCharacterIndex (0);
        const OUString sApplicationName (sValue.getToken(0, ',', nCharacterIndex).trim());
        if (nCharacterIndex < 0)
        {
            if (sApplicationName.getLength() == 0)
            {
                // This is a valid case: in the XML file the separator
                // was used as terminator.  Using it in the last line
                // creates an additional but empty entry.
                break;
            }
            else
            {
                OSL_FAIL("expecting three or four values per ContextList entry, separated by comma");
                continue;
            }
        }

        const OUString sContextName(sValue.getToken(0, ',', nCharacterIndex).trim());
        if (nCharacterIndex < 0)
        {
            OSL_FAIL("expecting three or four values per ContextList entry, separated by comma");
            continue;
        }

        const OUString sInitialState(sValue.getToken(0, ',', nCharacterIndex).trim());

        // The fourth argument is optional.
        const OUString sMenuCommandOverride(
            nCharacterIndex < 0
                ? OUString()
                : sValue.getToken(0, ',', nCharacterIndex).trim());

        const OUString sMenuCommand(
            sMenuCommandOverride.getLength() > 0
                ? (sMenuCommandOverride == "none"
                    ? OUString()
                    : sMenuCommandOverride)
                : rsDefaultMenuCommand);

        // Setup a list of application enums.  Note that the
        // application name may result in more than one value (eg
        // DrawImpress will result in two enums, one for Draw and one
        // for Impress).
        std::vector<EnumContext::Application> aApplications;
        EnumContext::Application eApplication (EnumContext::GetApplicationEnum(sApplicationName));
        if (eApplication == EnumContext::Application_None
            && !sApplicationName.equals(EnumContext::GetApplicationName(EnumContext::Application_None)))
        {
            // Handle some special names: abbreviations that make
            // context descriptions more readable.
            if (sApplicationName == "Writer")
                aApplications.push_back(EnumContext::Application_Writer);
            else if (sApplicationName == "Calc")
                aApplications.push_back(EnumContext::Application_Calc);
            else if (sApplicationName == "Draw")
                aApplications.push_back(EnumContext::Application_Draw);
            else if (sApplicationName == "Impress")
                aApplications.push_back(EnumContext::Application_Impress);
            else if (sApplicationName == "Chart")
                aApplications.push_back(EnumContext::Application_Chart);
            else if (sApplicationName == "DrawImpress")
            {
                // A special case among the special names:  it is
                // common to use the same context descriptions for
                // both Draw and Impress.  This special case helps to
                // avoid duplication in the .xcu file.
                aApplications.push_back(EnumContext::Application_Draw);
                aApplications.push_back(EnumContext::Application_Impress);
            }
            else if (sApplicationName == "WriterVariants")
            {
                // Another special case for all Writer variants.
                aApplications.push_back(EnumContext::Application_Writer);
                aApplications.push_back(EnumContext::Application_WriterGlobal);
                aApplications.push_back(EnumContext::Application_WriterWeb);
                aApplications.push_back(EnumContext::Application_WriterXML);
                aApplications.push_back(EnumContext::Application_WriterForm);
                aApplications.push_back(EnumContext::Application_WriterReport);
            }
            else
            {
                SAL_WARN("sfx.sidebar", "application name " << sApplicationName << " not recognized");
                continue;
            }
        }
        else
        {
            // No conversion of the application name necessary.
            aApplications.push_back(eApplication);
        }

        // Setup the actual context enum.
        const EnumContext::Context eContext (EnumContext::GetContextEnum(sContextName));
        if (eContext == EnumContext::Context_Unknown)
        {
            SAL_WARN("sfx.sidebar", "context name " << sContextName << " not recognized");
            continue;
        }

        // Setup the flag that controls whether a deck/pane is
        // initially visible/expanded.
        bool bIsInitiallyVisible;
        if (sInitialState == "visible")
            bIsInitiallyVisible = true;
        else if (sInitialState == "hidden")
            bIsInitiallyVisible = false;
        else
        {
            OSL_FAIL("unrecognized state");
            continue;
        }

        // Add context descriptors.
        std::vector<EnumContext::Application>::const_iterator iApplication;
        for (iApplication = aApplications.begin(); iApplication != aApplications.end(); ++iApplication)
        {
            if (*iApplication != EnumContext::Application_None)
            {
                rContextList.AddContextDescription(
                    Context(
                        EnumContext::GetApplicationName(*iApplication),
                        EnumContext::GetContextName(eContext)),
                    bIsInitiallyVisible,
                    sMenuCommand);
            }
        }
    }
}

void ResourceManager::ReadLegacyAddons (const Reference<frame::XController>& rxController)
{
    // Get module name for given frame.
    OUString sModuleName (Tools::GetModuleName(rxController));
    if (sModuleName.getLength() == 0)
        return;
    if (maProcessedApplications.find(sModuleName) != maProcessedApplications.end())
    {
        // Addons for this application have already been read.
        // There is nothing more to do.
        return;
    }

    // Mark module as processed.  Even when there is an error that
    // prevents the configuration data from being read, this error
    // will not be triggered a second time.
    maProcessedApplications.insert(sModuleName);

    // Get access to the configuration root node for the application.
    utl::OConfigurationTreeRoot aLegacyRootNode (GetLegacyAddonRootNode(sModuleName));
    if (!aLegacyRootNode.isValid())
        return;

    // Process child nodes.
    std::vector<OUString> aMatchingNodeNames;
    GetToolPanelNodeNames(aMatchingNodeNames, aLegacyRootNode);
    const sal_Int32 nCount (aMatchingNodeNames.size());
    size_t nDeckWriteIndex (maDecks.size());
    size_t nPanelWriteIndex (maPanels.size());
    maDecks.resize(maDecks.size() + nCount);
    maPanels.resize(maPanels.size() + nCount);
    for (sal_Int32 nReadIndex(0); nReadIndex<nCount; ++nReadIndex)
    {
        const OUString& rsNodeName (aMatchingNodeNames[nReadIndex]);
        const utl::OConfigurationNode aChildNode (aLegacyRootNode.openNode(rsNodeName));
        if (!aChildNode.isValid())
            continue;

        if ( rsNodeName == "private:resource/toolpanel/DrawingFramework/CustomAnimations" ||
             rsNodeName == "private:resource/toolpanel/DrawingFramework/Layouts" ||
             rsNodeName == "private:resource/toolpanel/DrawingFramework/MasterPages" ||
             rsNodeName == "private:resource/toolpanel/DrawingFramework/SlideTransitions" ||
             rsNodeName == "private:resource/toolpanel/DrawingFramework/TableDesign" )
          continue;

        DeckDescriptor& rDeckDescriptor (maDecks[nDeckWriteIndex++]);
        rDeckDescriptor.msTitle = getString(aChildNode, "UIName");
        rDeckDescriptor.msId = rsNodeName;
        rDeckDescriptor.msIconURL = getString(aChildNode, "ImageURL");
        rDeckDescriptor.msHighContrastIconURL = rDeckDescriptor.msIconURL;
        rDeckDescriptor.msTitleBarIconURL.clear();
        rDeckDescriptor.msHighContrastTitleBarIconURL.clear();
        rDeckDescriptor.msHelpURL = getString(aChildNode, "HelpURL");
        rDeckDescriptor.msHelpText = rDeckDescriptor.msTitle;
        rDeckDescriptor.mbIsEnabled = true;
        rDeckDescriptor.mnOrderIndex = 100000 + nReadIndex;
        rDeckDescriptor.maContextList.AddContextDescription(Context(sModuleName, OUString("any")), true, OUString());

        PanelDescriptor& rPanelDescriptor (maPanels[nPanelWriteIndex++]);
        rPanelDescriptor.msTitle = getString(aChildNode, "UIName");
        rPanelDescriptor.mbIsTitleBarOptional = true;
        rPanelDescriptor.msId = rsNodeName;
        rPanelDescriptor.msDeckId = rsNodeName;
        rPanelDescriptor.msTitleBarIconURL.clear();
        rPanelDescriptor.msHighContrastTitleBarIconURL.clear();
        rPanelDescriptor.msHelpURL = getString(aChildNode, "HelpURL");
        rPanelDescriptor.msImplementationURL = rsNodeName;
        rPanelDescriptor.mnOrderIndex = 100000 + nReadIndex;
        rPanelDescriptor.mbShowForReadOnlyDocuments = false;
        rPanelDescriptor.mbWantsCanvas = false;
        rPanelDescriptor.maContextList.AddContextDescription(Context(sModuleName, OUString("any")), true, OUString());
    }

    // When there where invalid nodes then we have to adapt the size
    // of the deck and panel vectors.
    if (nDeckWriteIndex < maDecks.size())
        maDecks.resize(nDeckWriteIndex);
    if (nPanelWriteIndex < maPanels.size())
        maPanels.resize(nPanelWriteIndex);
}

void ResourceManager::StorePanelExpansionState (
                        const OUString& rsPanelId,
                        const bool bExpansionState,
                        const Context& rContext)
{
    PanelContainer::iterator iPanel;
    for (iPanel = maPanels.begin(); iPanel != maPanels.end(); ++iPanel)
    {
        if (iPanel->msId.equals(rsPanelId))
        {
            ContextList::Entry* pEntry(iPanel->maContextList.GetMatch(rContext));
            if (pEntry != nullptr)
                pEntry->mbIsInitiallyVisible = bExpansionState;
        }
    }
}

utl::OConfigurationTreeRoot ResourceManager::GetLegacyAddonRootNode (const OUString& rsModuleName)
{
    try
    {
        const Reference<XComponentContext> xContext(comphelper::getProcessComponentContext());
        const Reference<frame::XModuleManager2> xModuleAccess = frame::ModuleManager::create(xContext);
        const comphelper::NamedValueCollection aModuleProperties(xModuleAccess->getByName(rsModuleName));
        const OUString sWindowStateRef(aModuleProperties.getOrDefault(
                                       "ooSetupFactoryWindowStateConfigRef",
                                       OUString()));

        OUStringBuffer aPathComposer;
        aPathComposer.append("org.openoffice.Office.UI.");
        aPathComposer.append(sWindowStateRef);
        aPathComposer.append("/UIElements/States");

        return utl::OConfigurationTreeRoot(xContext, aPathComposer.makeStringAndClear(), false);
    }
    catch (const Exception&)
    {
        DBG_UNHANDLED_EXCEPTION();
    }

    return utl::OConfigurationTreeRoot();
}

void ResourceManager::GetToolPanelNodeNames (
                        std::vector<OUString>& rMatchingNames,
                        const utl::OConfigurationTreeRoot& aRoot)
{
    Sequence<OUString> aChildNodeNames (aRoot.getNodeNames());
    const sal_Int32 nCount (aChildNodeNames.getLength());
    for (sal_Int32 nIndex(0); nIndex<nCount; ++nIndex)
    {
        if (aChildNodeNames[nIndex].startsWith( "private:resource/toolpanel/" ))
            rMatchingNames.push_back(aChildNodeNames[nIndex]);
    }
}

bool ResourceManager::IsDeckEnabled (
                        const OUString& rsDeckId,
                        const Context& rContext,
                        const Reference<frame::XController>& rxController)
{
    // Check if any panel that matches the current context can be
    // displayed.
    ResourceManager::PanelContextDescriptorContainer aPanelContextDescriptors;

    ResourceManager::Instance().GetMatchingPanels(aPanelContextDescriptors,
                                                  rContext, rsDeckId, rxController);

    ResourceManager::PanelContextDescriptorContainer::const_iterator iPanel;
    for (iPanel = aPanelContextDescriptors.begin(); iPanel != aPanelContextDescriptors.end(); ++iPanel)
    {
        if (iPanel->mbShowForReadOnlyDocuments)
            return true;
    }

    return false;
}

void ResourceManager::UpdateModel(css::uno::Reference<css::frame::XModel> xModel)
{
    for (DeckContainer::iterator itr = maDecks.begin(); itr != maDecks.end(); ++itr)
    {
        if (!itr->mpDeck)
            continue;

        const SharedPanelContainer& rContainer = itr->mpDeck->GetPanels();

        for (SharedPanelContainer::const_iterator it = rContainer.begin(); it != rContainer.end(); ++it) {
            css::uno::Reference<css::ui::XUpdateModel> xPanel((*it)->GetPanelComponent(), css::uno::UNO_QUERY);
            xPanel->updateModel(xModel);
        }

    }
}

void ResourceManager::disposeDecks()
{
    for (DeckContainer::iterator itr = maDecks.begin(); itr != maDecks.end(); ++itr)
        itr->mpDeck.disposeAndClear();
}

} } // end of namespace sfx2::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

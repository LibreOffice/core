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

#include <officecfg/Office/UI/Sidebar.hxx>
#include <unotools/confignode.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/namedvaluecollection.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/types.hxx>

#include <rtl/ustrbuf.hxx>
#include <tools/diagnose_ex.h>
#include <sal/log.hxx>

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

css::uno::Sequence<OUString> BuildContextList (const ContextList& rContextList)
{
    const ::std::vector<ContextList::Entry>& entries = rContextList.GetEntries();

    css::uno::Sequence<OUString> result(entries.size());
    long i = 0;

    for (auto const& entry : entries)
    {
        OUString appName = entry.maContext.msApplication;
        OUString contextName = entry.maContext.msContext;
        OUString menuCommand = entry.msMenuCommand;

        OUString visibility;
        if (entry.mbIsInitiallyVisible)
            visibility = "visible";
        else
            visibility = "hidden";

        OUString element = appName + ", " + contextName +", " + visibility;

        if (!menuCommand.isEmpty())
            element += ", "+menuCommand;

        result[i] = element;

        ++i;
    }

    return result;

}

} //end anonymous namespace

ResourceManager::ResourceManager()
    : maDecks(),
      maPanels(),
      maProcessedApplications(),
      maMiscOptions()
{
    ReadDeckList();
    ReadPanelList();
    ReadLastActive();
}

ResourceManager::~ResourceManager()
{
}

void ResourceManager::InitDeckContext(const Context& rContext)
{
    for (auto const& deck : maDecks)
    {
        const ContextList::Entry* pMatchingEntry = deck->maContextList.GetMatch(rContext);

        bool bIsEnabled;
        if (pMatchingEntry)
            bIsEnabled = pMatchingEntry->mbIsInitiallyVisible;
        else
            bIsEnabled = false;

        deck->mbIsEnabled = bIsEnabled;
    }
}

std::shared_ptr<DeckDescriptor> ResourceManager::ImplGetDeckDescriptor(const OUString& rsDeckId) const
{
    for (auto const& deck : maDecks)
    {
        if (deck->mbExperimental && !maMiscOptions.IsExperimentalMode())
            continue;
        if (deck->msId == rsDeckId)
            return deck;
    }
    return nullptr;
}

std::shared_ptr<DeckDescriptor> ResourceManager::GetDeckDescriptor(const OUString& rsDeckId) const
{
    return ImplGetDeckDescriptor( rsDeckId );
}

std::shared_ptr<PanelDescriptor> ResourceManager::ImplGetPanelDescriptor(const OUString& rsPanelId) const
{
    for (auto const& panel : maPanels)
    {
        if (panel->msId == rsPanelId)
            return panel;
    }
    return nullptr;
}

std::shared_ptr<PanelDescriptor> ResourceManager::GetPanelDescriptor(const OUString& rsPanelId) const
{
    return ImplGetPanelDescriptor( rsPanelId );
}

const ResourceManager::DeckContextDescriptorContainer& ResourceManager::GetMatchingDecks (
                                                            DeckContextDescriptorContainer& rDecks,
                                                            const Context& rContext,
                                                            const bool bIsDocumentReadOnly,
                                                            const Reference<frame::XController>& rxController)
{
    ReadLegacyAddons(rxController);

    std::multimap<sal_Int32,DeckContextDescriptor> aOrderedIds;
    for (auto const& deck : maDecks)
    {
        if (deck->mbExperimental && !maMiscOptions.IsExperimentalMode())
            continue;

        const DeckDescriptor& rDeckDescriptor (*deck);
        if (rDeckDescriptor.maContextList.GetMatch(rContext) == nullptr)
            continue;

        DeckContextDescriptor aDeckContextDescriptor;
        aDeckContextDescriptor.msId = rDeckDescriptor.msId;

        aDeckContextDescriptor.mbIsEnabled = (! bIsDocumentReadOnly || IsDeckEnabled(rDeckDescriptor.msId, rContext, rxController) )
                                             && rDeckDescriptor.mbIsEnabled;


        aOrderedIds.emplace(rDeckDescriptor.mnOrderIndex, aDeckContextDescriptor);
    }

    for (auto const& orderId : aOrderedIds)
    {
        rDecks.push_back(orderId.second);
    }

    return rDecks;
}

const ResourceManager::PanelContextDescriptorContainer& ResourceManager::GetMatchingPanels (
                                                            PanelContextDescriptorContainer& rPanelIds,
                                                            const Context& rContext,
                                                            const OUString& sDeckId,
                                                            const Reference<frame::XController>& rxController)
{
    ReadLegacyAddons(rxController);

    std::multimap<sal_Int32, PanelContextDescriptor> aOrderedIds;
    for (auto const& panel : maPanels)
    {
        const PanelDescriptor& rPanelDescriptor (*panel);
        if (rPanelDescriptor.mbExperimental && !maMiscOptions.IsExperimentalMode())
            continue;
        if ( rPanelDescriptor.msDeckId != sDeckId )
            continue;

        const ContextList::Entry* pEntry = rPanelDescriptor.maContextList.GetMatch(rContext);
        if (pEntry == nullptr)
            continue;

        PanelContextDescriptor aPanelContextDescriptor;
        aPanelContextDescriptor.msId = rPanelDescriptor.msId;
        aPanelContextDescriptor.msMenuCommand = pEntry->msMenuCommand;
        aPanelContextDescriptor.mbIsInitiallyVisible = pEntry->mbIsInitiallyVisible;
        aPanelContextDescriptor.mbShowForReadOnlyDocuments = rPanelDescriptor.mbShowForReadOnlyDocuments;
        aOrderedIds.emplace(rPanelDescriptor.mnOrderIndex, aPanelContextDescriptor);
    }

    for (auto const& orderId : aOrderedIds)
    {
        rPanelIds.push_back(orderId.second);
    }

    return rPanelIds;
}

const OUString& ResourceManager::GetLastActiveDeck( const Context& rContext )
{
    if( maLastActiveDecks.find( rContext.msApplication ) == maLastActiveDecks.end())
        return maLastActiveDecks["any"];
    else
        return maLastActiveDecks[rContext.msApplication];
}

void ResourceManager::SetLastActiveDeck( const Context& rContext, const OUString &rsDeckId )
{
    maLastActiveDecks[rContext.msApplication] = rsDeckId;
}

void ResourceManager::ReadDeckList()
{
    const utl::OConfigurationTreeRoot aDeckRootNode(
                                        comphelper::getProcessComponentContext(),
                                        "org.openoffice.Office.UI.Sidebar/Content/DeckList",
                                        false);
    if (!aDeckRootNode.isValid())
        return;

    const Sequence<OUString> aDeckNodeNames (aDeckRootNode.getNodeNames());
    const sal_Int32 nCount(aDeckNodeNames.getLength());
    maDecks.clear();
    for (sal_Int32 nReadIndex(0); nReadIndex<nCount; ++nReadIndex)
    {
        const utl::OConfigurationNode aDeckNode(aDeckRootNode.openNode(aDeckNodeNames[nReadIndex]));
        if (!aDeckNode.isValid())
            continue;

        maDecks.push_back(std::make_shared<DeckDescriptor>());
        DeckDescriptor& rDeckDescriptor (*maDecks.back());

        rDeckDescriptor.msTitle = getString(aDeckNode, "Title");
        rDeckDescriptor.msId = getString(aDeckNode, "Id");
        rDeckDescriptor.msIconURL = getString(aDeckNode, "IconURL");
        rDeckDescriptor.msHighContrastIconURL = getString(aDeckNode, "HighContrastIconURL");
        rDeckDescriptor.msTitleBarIconURL = getString(aDeckNode, "TitleBarIconURL");
        rDeckDescriptor.msHighContrastTitleBarIconURL = getString(aDeckNode, "HighContrastTitleBarIconURL");
        rDeckDescriptor.msHelpText = rDeckDescriptor.msTitle;
        rDeckDescriptor.mnOrderIndex = getInt32(aDeckNode, "OrderIndex");
        rDeckDescriptor.mbExperimental = getBool(aDeckNode, "IsExperimental");

        rDeckDescriptor.msNodeName = aDeckNodeNames[nReadIndex];

        ReadContextList(
            aDeckNode,
            rDeckDescriptor.maContextList,
            OUString());

    }
}

void ResourceManager::SaveDecksSettings(const Context& rContext)
{
    for (auto const& deck : maDecks)
    {
       const ContextList::Entry* pMatchingEntry = deck->maContextList.GetMatch(rContext);
       if (pMatchingEntry)
       {
            std::shared_ptr<DeckDescriptor> xDeckDesc = GetDeckDescriptor(deck->msId);
            if (xDeckDesc)
                SaveDeckSettings(xDeckDesc.get());
       }

    }
}

void ResourceManager::SaveDeckSettings(const DeckDescriptor* pDeckDesc)
{
    const utl::OConfigurationTreeRoot aDeckRootNode(
                                    comphelper::getProcessComponentContext(),
                                    "org.openoffice.Office.UI.Sidebar/Content/DeckList",
                                    true);
    if (!aDeckRootNode.isValid())
        return;

    // save deck settings

    ::uno::Sequence< OUString > sContextList = BuildContextList(pDeckDesc->maContextList);

    utl::OConfigurationNode aDeckNode (aDeckRootNode.openNode(pDeckDesc->msNodeName));

    css::uno::Any aTitle(makeAny(pDeckDesc->msTitle));
    css::uno::Any aOrder(makeAny(pDeckDesc->mnOrderIndex));
    css::uno::Any aContextList(makeAny(sContextList));

    bool bChanged = false;
    if (aTitle != aDeckNode.getNodeValue("Title"))
    {
        aDeckNode.setNodeValue("Title", aTitle);
        bChanged = true;
    }
    if (aOrder != aDeckNode.getNodeValue("OrderIndex"))
    {
        aDeckNode.setNodeValue("OrderIndex", aOrder);
        bChanged = true;
    }
    if (aContextList != aDeckNode.getNodeValue("ContextList"))
    {
        aDeckNode.setNodeValue("ContextList", aContextList);
        bChanged = true;
    }

    if (bChanged)
        aDeckRootNode.commit();

    // save panel settings

    const utl::OConfigurationTreeRoot aPanelRootNode(
                                    comphelper::getProcessComponentContext(),
                                    "org.openoffice.Office.UI.Sidebar/Content/PanelList",
                                    true);

    if (!aPanelRootNode.isValid())
        return;

    if (!pDeckDesc->mpDeck) // the deck has not been edited
        return;

    SharedPanelContainer rPanels = pDeckDesc->mpDeck->GetPanels();

    bChanged = false;
    for (auto const& panel : rPanels)
    {
        OUString panelId = panel->GetId();
        std::shared_ptr<PanelDescriptor> xPanelDesc = GetPanelDescriptor(panelId);

        ::uno::Sequence< OUString > sPanelContextList = BuildContextList(xPanelDesc->maContextList);

        utl::OConfigurationNode aPanelNode (aPanelRootNode.openNode(xPanelDesc->msNodeName));

        aTitle <<= xPanelDesc->msTitle;
        aOrder <<= xPanelDesc->mnOrderIndex;
        aContextList <<= sPanelContextList;

        if (aTitle != aPanelNode.getNodeValue("Title"))
        {
            aPanelNode.setNodeValue("Title", aTitle);
            bChanged = true;
        }
        if (aOrder != aPanelNode.getNodeValue("OrderIndex"))
        {
            aPanelNode.setNodeValue("OrderIndex", aOrder);
            bChanged = true;
        }
        if (aContextList != aPanelNode.getNodeValue("ContextList"))
        {
            aPanelNode.setNodeValue("ContextList", aContextList);
            bChanged = true;
        }
    }

    if (bChanged)
        aPanelRootNode.commit();
}

void ResourceManager::SaveLastActiveDeck(const Context& rContext, const OUString& rActiveDeck)
{
    maLastActiveDecks[rContext.msApplication] = rActiveDeck;

    std::set<OUString> aLastActiveDecks;
    for ( auto const & rEntry : maLastActiveDecks )
        aLastActiveDecks.insert( rEntry.first + "," +  rEntry.second);

    std::shared_ptr<comphelper::ConfigurationChanges> cfgWriter( comphelper::ConfigurationChanges::create() );

    officecfg::Office::UI::Sidebar::Content::LastActiveDeck::set(comphelper::containerToSequence(aLastActiveDecks), cfgWriter);
    cfgWriter->commit();

}

void ResourceManager::ReadPanelList()
{
    const utl::OConfigurationTreeRoot aPanelRootNode(
                                        comphelper::getProcessComponentContext(),
                                        "org.openoffice.Office.UI.Sidebar/Content/PanelList",
                                        false);
    if (!aPanelRootNode.isValid())
        return;

    const Sequence<OUString> aPanelNodeNames (aPanelRootNode.getNodeNames());
    const sal_Int32 nCount (aPanelNodeNames.getLength());
    maPanels.clear();
    for (sal_Int32 nReadIndex(0); nReadIndex<nCount; ++nReadIndex)
    {
        const utl::OConfigurationNode aPanelNode (aPanelRootNode.openNode(aPanelNodeNames[nReadIndex]));
        if (!aPanelNode.isValid())
            continue;

        maPanels.push_back(std::make_shared<PanelDescriptor>());
        PanelDescriptor& rPanelDescriptor(*maPanels.back());

        rPanelDescriptor.msTitle = getString(aPanelNode, "Title");
        rPanelDescriptor.mbIsTitleBarOptional = getBool(aPanelNode, "TitleBarIsOptional");
        rPanelDescriptor.msId = getString(aPanelNode, "Id");
        rPanelDescriptor.msDeckId = getString(aPanelNode, "DeckId");
        rPanelDescriptor.msTitleBarIconURL = getString(aPanelNode, "TitleBarIconURL");
        rPanelDescriptor.msHighContrastTitleBarIconURL = getString(aPanelNode, "HighContrastTitleBarIconURL");
        rPanelDescriptor.msImplementationURL = getString(aPanelNode, "ImplementationURL");
        rPanelDescriptor.mnOrderIndex = getInt32(aPanelNode, "OrderIndex");
        rPanelDescriptor.mbShowForReadOnlyDocuments = getBool(aPanelNode, "ShowForReadOnlyDocument");
        rPanelDescriptor.mbWantsCanvas = getBool(aPanelNode, "WantsCanvas");
        rPanelDescriptor.mbExperimental = getBool(aPanelNode, "IsExperimental");
        const OUString sDefaultMenuCommand(getString(aPanelNode, "DefaultMenuCommand"));

        rPanelDescriptor.msNodeName = aPanelNodeNames[nReadIndex];

        ReadContextList(aPanelNode, rPanelDescriptor.maContextList, sDefaultMenuCommand);
    }
}

void ResourceManager::ReadLastActive()
{
    Sequence <OUString> aLastActive (officecfg::Office::UI::Sidebar::Content::LastActiveDeck::get());

    for (auto& rDeckInfo : aLastActive)
    {
        sal_Int32 nCharIdx = rDeckInfo.lastIndexOf(',');
        if ( nCharIdx <= 0 || (nCharIdx == rDeckInfo.getLength() - 1) )
        {
            SAL_WARN("sfx.sidebar", "Expecting 2 values separated by comma");
            continue;
        }

        const OUString sApplicationName = rDeckInfo.copy( 0, nCharIdx );
        vcl::EnumContext::Application eApplication (vcl::EnumContext::GetApplicationEnum(sApplicationName));
        const OUString sLastUsed = rDeckInfo.copy( nCharIdx + 1 );

        // guard against garbage in place of application
        if (eApplication != vcl::EnumContext::Application::NONE)
            maLastActiveDecks.insert( std::make_pair(sApplicationName, sLastUsed ) );
    }
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
        std::vector<vcl::EnumContext::Application> aApplications;
        vcl::EnumContext::Application eApplication (vcl::EnumContext::GetApplicationEnum(sApplicationName));

        if (eApplication == vcl::EnumContext::Application::NONE
            && sApplicationName != vcl::EnumContext::GetApplicationName(vcl::EnumContext::Application::NONE))
        {
            // Handle some special names: abbreviations that make
            // context descriptions more readable.
            if (sApplicationName == "Writer")
                aApplications.push_back(vcl::EnumContext::Application::Writer);
            else if (sApplicationName == "Calc")
                aApplications.push_back(vcl::EnumContext::Application::Calc);
            else if (sApplicationName == "Draw")
                aApplications.push_back(vcl::EnumContext::Application::Draw);
            else if (sApplicationName == "Impress")
                aApplications.push_back(vcl::EnumContext::Application::Impress);
            else if (sApplicationName == "Chart")
                aApplications.push_back(vcl::EnumContext::Application::Chart);
            else if (sApplicationName == "DrawImpress")
            {
                // A special case among the special names:  it is
                // common to use the same context descriptions for
                // both Draw and Impress.  This special case helps to
                // avoid duplication in the .xcu file.
                aApplications.push_back(vcl::EnumContext::Application::Draw);
                aApplications.push_back(vcl::EnumContext::Application::Impress);
            }
            else if (sApplicationName == "WriterVariants")
            {
                // Another special case for all Writer variants.
                aApplications.push_back(vcl::EnumContext::Application::Writer);
                aApplications.push_back(vcl::EnumContext::Application::WriterGlobal);
                aApplications.push_back(vcl::EnumContext::Application::WriterWeb);
                aApplications.push_back(vcl::EnumContext::Application::WriterXML);
                aApplications.push_back(vcl::EnumContext::Application::WriterForm);
                aApplications.push_back(vcl::EnumContext::Application::WriterReport);
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
        const vcl::EnumContext::Context eContext (vcl::EnumContext::GetContextEnum(sContextName));
        if (eContext == vcl::EnumContext::Context::Unknown)
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
        for (auto const& application : aApplications)
        {
            if (application != vcl::EnumContext::Application::NONE)
            {
                rContextList.AddContextDescription(
                    Context(
                        vcl::EnumContext::GetApplicationName(application),
                        vcl::EnumContext::GetContextName(eContext)),
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

        maDecks.push_back(std::make_shared<DeckDescriptor>());
        DeckDescriptor& rDeckDescriptor(*maDecks.back());
        rDeckDescriptor.msTitle = getString(aChildNode, "UIName");
        rDeckDescriptor.msId = rsNodeName;
        rDeckDescriptor.msIconURL = getString(aChildNode, "ImageURL");
        rDeckDescriptor.msHighContrastIconURL = rDeckDescriptor.msIconURL;
        rDeckDescriptor.msTitleBarIconURL.clear();
        rDeckDescriptor.msHighContrastTitleBarIconURL.clear();
        rDeckDescriptor.msHelpText = rDeckDescriptor.msTitle;
        rDeckDescriptor.mbIsEnabled = true;
        rDeckDescriptor.mnOrderIndex = 100000 + nReadIndex;
        rDeckDescriptor.maContextList.AddContextDescription(Context(sModuleName, "any"), true, OUString());

        maPanels.push_back(std::make_shared<PanelDescriptor>());
        PanelDescriptor& rPanelDescriptor(*maPanels.back());
        rPanelDescriptor.msTitle = getString(aChildNode, "UIName");
        rPanelDescriptor.mbIsTitleBarOptional = true;
        rPanelDescriptor.msId = rsNodeName;
        rPanelDescriptor.msDeckId = rsNodeName;
        rPanelDescriptor.msTitleBarIconURL.clear();
        rPanelDescriptor.msHighContrastTitleBarIconURL.clear();
        rPanelDescriptor.msImplementationURL = rsNodeName;
        rPanelDescriptor.mnOrderIndex = 100000 + nReadIndex;
        rPanelDescriptor.mbShowForReadOnlyDocuments = false;
        rPanelDescriptor.mbWantsCanvas = false;
        rPanelDescriptor.maContextList.AddContextDescription(Context(sModuleName, "any"), true, OUString());
    }
}

void ResourceManager::StorePanelExpansionState (
                        const OUString& rsPanelId,
                        const bool bExpansionState,
                        const Context& rContext)
{
    for (auto const& panel : maPanels)
    {
        if (panel->msId == rsPanelId)
        {
            ContextList::Entry* pEntry(panel->maContextList.GetMatch(rContext));
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
        DBG_UNHANDLED_EXCEPTION("sfx.sidebar");
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
    PanelContextDescriptorContainer aPanelContextDescriptors;

    GetMatchingPanels(aPanelContextDescriptors, rContext, rsDeckId, rxController);

    for (auto const& panelContextDescriptor : aPanelContextDescriptors)
    {
        if (panelContextDescriptor.mbShowForReadOnlyDocuments)
            return true;
    }
    return false;
}

void ResourceManager::UpdateModel(const css::uno::Reference<css::frame::XModel>& xModel)
{
    for (auto const& deck : maDecks)
    {
        if (!deck->mpDeck)
            continue;

        const SharedPanelContainer& rContainer = deck->mpDeck->GetPanels();

        for (auto const& elem : rContainer)
        {
            css::uno::Reference<css::ui::XUpdateModel> xPanel(elem->GetPanelComponent(), css::uno::UNO_QUERY);
            if (xPanel.is()) // tdf#108814 interface is optional
            {
                xPanel->updateModel(xModel);
            }
        }
    }
}

void ResourceManager::disposeDecks()
{
    for (auto const& deck : maDecks)
    {
        deck->mpDeck.disposeAndClear();
    }
}

} } // end of namespace sfx2::sidebar

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

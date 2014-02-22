/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "ResourceManager.hxx"
#include <sfx2/sidebar/Tools.hxx>

#include <unotools/confignode.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/namedvaluecollection.hxx>
#include <comphelper/types.hxx>

#include <rtl/ustrbuf.hxx>
#include <tools/diagnose_ex.h>

#include <com/sun/star/frame/ModuleManager.hpp>

#include <map>

using ::rtl::OUString;
using namespace css;
using namespace cssu;

namespace sfx2 { namespace sidebar {

class ResourceManager::Deleter
{
public:
    void operator() (ResourceManager* pObject)
    {
        delete pObject;
    }
};

ResourceManager& ResourceManager::Instance (void)
{
    static ResourceManager maInstance;
    return maInstance;
}




ResourceManager::ResourceManager (void)
    : maDecks(),
      maPanels(),
      maProcessedApplications()
{
    ReadDeckList();
    ReadPanelList();
}




ResourceManager::~ResourceManager (void)
{
    maPanels.clear();
    maDecks.clear();
}




const DeckDescriptor* ResourceManager::GetDeckDescriptor (
    const ::rtl::OUString& rsDeckId) const
{
    for (DeckContainer::const_iterator
             iDeck(maDecks.begin()),
             iEnd(maDecks.end());
         iDeck!=iEnd;
         ++iDeck)
    {
        if (iDeck->msId.equals(rsDeckId))
            return &*iDeck;
    }
    return NULL;
}




const PanelDescriptor* ResourceManager::GetPanelDescriptor (
    const ::rtl::OUString& rsPanelId) const
{
    for (PanelContainer::const_iterator
             iPanel(maPanels.begin()),
             iEnd(maPanels.end());
         iPanel!=iEnd;
         ++iPanel)
    {
        if (iPanel->msId.equals(rsPanelId))
            return &*iPanel;
    }
    return NULL;
}




void ResourceManager::SetIsDeckEnabled (
    const ::rtl::OUString& rsDeckId,
    const bool bIsEnabled)
{
    for (DeckContainer::iterator
             iDeck(maDecks.begin()),
             iEnd(maDecks.end());
         iDeck!=iEnd;
         ++iDeck)
    {
        if (iDeck->msId.equals(rsDeckId))
        {
            iDeck->mbIsEnabled = bIsEnabled;
            return;
        }
    }
}




const ResourceManager::DeckContextDescriptorContainer& ResourceManager::GetMatchingDecks (
    DeckContextDescriptorContainer& rDecks,
    const Context& rContext,
    const bool bIsDocumentReadOnly,
    const Reference<frame::XFrame>& rxFrame)
{
    ReadLegacyAddons(rxFrame);

    ::std::multimap<sal_Int32,DeckContextDescriptor> aOrderedIds;
    for (DeckContainer::const_iterator
             iDeck(maDecks.begin()),
             iEnd (maDecks.end());
         iDeck!=iEnd;
         ++iDeck)
    {
        const DeckDescriptor& rDeckDescriptor (*iDeck);
        if (rDeckDescriptor.maContextList.GetMatch(rContext) == NULL)
            continue;
        DeckContextDescriptor aDeckContextDescriptor;
        aDeckContextDescriptor.msId = rDeckDescriptor.msId;
        aDeckContextDescriptor.mbIsEnabled =
            ! bIsDocumentReadOnly
            || IsDeckEnabled(rDeckDescriptor.msId, rContext, rxFrame);
        aOrderedIds.insert(::std::multimap<sal_Int32,DeckContextDescriptor>::value_type(
                rDeckDescriptor.mnOrderIndex,
                aDeckContextDescriptor));
    }

    for (::std::multimap<sal_Int32,DeckContextDescriptor>::const_iterator
             iId(aOrderedIds.begin()),
             iEnd(aOrderedIds.end());
         iId!=iEnd;
         ++iId)
    {
        rDecks.push_back(iId->second);
    }

    return rDecks;
}




const ResourceManager::PanelContextDescriptorContainer& ResourceManager::GetMatchingPanels (
    PanelContextDescriptorContainer& rPanelIds,
    const Context& rContext,
    const ::rtl::OUString& rsDeckId,
    const Reference<frame::XFrame>& rxFrame)
{
    ReadLegacyAddons(rxFrame);

    ::std::multimap<sal_Int32,PanelContextDescriptor> aOrderedIds;
    for (PanelContainer::const_iterator
             iPanel(maPanels.begin()),
             iEnd(maPanels.end());
         iPanel!=iEnd;
         ++iPanel)
    {
        const PanelDescriptor& rPanelDescriptor (*iPanel);
        if ( ! rPanelDescriptor.msDeckId.equals(rsDeckId))
            continue;

        const ContextList::Entry* pEntry = rPanelDescriptor.maContextList.GetMatch(rContext);
        if (pEntry == NULL)
            continue;

        PanelContextDescriptor aPanelContextDescriptor;
        aPanelContextDescriptor.msId = rPanelDescriptor.msId;
        aPanelContextDescriptor.msMenuCommand = pEntry->msMenuCommand;
        aPanelContextDescriptor.mbIsInitiallyVisible = pEntry->mbIsInitiallyVisible;
        aPanelContextDescriptor.mbShowForReadOnlyDocuments = rPanelDescriptor.mbShowForReadOnlyDocuments;
        aOrderedIds.insert(::std::multimap<sal_Int32,PanelContextDescriptor>::value_type(
                rPanelDescriptor.mnOrderIndex,
                aPanelContextDescriptor));
    }

    for (::std::multimap<sal_Int32,PanelContextDescriptor>::const_iterator
             iId(aOrderedIds.begin()),
             iEnd(aOrderedIds.end());
         iId!=iEnd;
         ++iId)
    {
        rPanelIds.push_back(iId->second);
    }

    return rPanelIds;
}




void ResourceManager::ReadDeckList (void)
{
    const ::utl::OConfigurationTreeRoot aDeckRootNode (
        ::comphelper::getProcessComponentContext(),
        OUString("org.openoffice.Office.UI.Sidebar/Content/DeckList"),
        false);
    if ( ! aDeckRootNode.isValid() )
        return;

    const Sequence<OUString> aDeckNodeNames (aDeckRootNode.getNodeNames());
    const sal_Int32 nCount (aDeckNodeNames.getLength());
    maDecks.resize(nCount);
    sal_Int32 nWriteIndex(0);
    for (sal_Int32 nReadIndex(0); nReadIndex<nCount; ++nReadIndex)
    {
        const ::utl::OConfigurationNode aDeckNode (aDeckRootNode.openNode(aDeckNodeNames[nReadIndex]));
        if ( ! aDeckNode.isValid())
            continue;

        DeckDescriptor& rDeckDescriptor (maDecks[nWriteIndex++]);

        rDeckDescriptor.msTitle = ::comphelper::getString(
            aDeckNode.getNodeValue("Title"));
        rDeckDescriptor.msId = ::comphelper::getString(
            aDeckNode.getNodeValue("Id"));
        rDeckDescriptor.msIconURL = ::comphelper::getString(
            aDeckNode.getNodeValue("IconURL"));
        rDeckDescriptor.msHighContrastIconURL = ::comphelper::getString(
            aDeckNode.getNodeValue("HighContrastIconURL"));
        rDeckDescriptor.msTitleBarIconURL = ::comphelper::getString(
            aDeckNode.getNodeValue("TitleBarIconURL"));
        rDeckDescriptor.msHighContrastTitleBarIconURL = ::comphelper::getString(
            aDeckNode.getNodeValue("HighContrastTitleBarIconURL"));
        rDeckDescriptor.msHelpURL = ::comphelper::getString(
            aDeckNode.getNodeValue("HelpURL"));
        rDeckDescriptor.msHelpText = rDeckDescriptor.msTitle;
        rDeckDescriptor.mbIsEnabled = true;
        rDeckDescriptor.mnOrderIndex = ::comphelper::getINT32(
            aDeckNode.getNodeValue("OrderIndex"));

        ReadContextList(
            aDeckNode,
            rDeckDescriptor.maContextList,
            OUString());
    }

    
    
    if (nWriteIndex<nCount)
        maDecks.resize(nWriteIndex);
}




void ResourceManager::ReadPanelList (void)
{
    const ::utl::OConfigurationTreeRoot aPanelRootNode (
        ::comphelper::getProcessComponentContext(),
        OUString("org.openoffice.Office.UI.Sidebar/Content/PanelList"),
        false);
    if ( ! aPanelRootNode.isValid() )
        return;

    const Sequence<OUString> aPanelNodeNames (aPanelRootNode.getNodeNames());
    const sal_Int32 nCount (aPanelNodeNames.getLength());
    maPanels.resize(nCount);
    sal_Int32 nWriteIndex (0);
    for (sal_Int32 nReadIndex(0); nReadIndex<nCount; ++nReadIndex)
    {
        const ::utl::OConfigurationNode aPanelNode (aPanelRootNode.openNode(aPanelNodeNames[nReadIndex]));
        if ( ! aPanelNode.isValid())
            continue;

        PanelDescriptor& rPanelDescriptor (maPanels[nWriteIndex++]);

        rPanelDescriptor.msTitle = ::comphelper::getString(
            aPanelNode.getNodeValue("Title"));
        rPanelDescriptor.mbIsTitleBarOptional = ::comphelper::getBOOL(
            aPanelNode.getNodeValue("TitleBarIsOptional"));
        rPanelDescriptor.msId = ::comphelper::getString(
            aPanelNode.getNodeValue("Id"));
        rPanelDescriptor.msDeckId = ::comphelper::getString(
            aPanelNode.getNodeValue("DeckId"));
        rPanelDescriptor.msTitleBarIconURL = ::comphelper::getString(
            aPanelNode.getNodeValue("TitleBarIconURL"));
        rPanelDescriptor.msHighContrastTitleBarIconURL = ::comphelper::getString(
            aPanelNode.getNodeValue("HighContrastTitleBarIconURL"));
        rPanelDescriptor.msHelpURL = ::comphelper::getString(
            aPanelNode.getNodeValue("HelpURL"));
        rPanelDescriptor.msImplementationURL = ::comphelper::getString(
            aPanelNode.getNodeValue("ImplementationURL"));
        rPanelDescriptor.mnOrderIndex = ::comphelper::getINT32(
            aPanelNode.getNodeValue("OrderIndex"));
        rPanelDescriptor.mbShowForReadOnlyDocuments = ::comphelper::getBOOL(
            aPanelNode.getNodeValue("ShowForReadOnlyDocument"));
        rPanelDescriptor.mbWantsCanvas = ::comphelper::getBOOL(
            aPanelNode.getNodeValue("WantsCanvas"));
        const OUString sDefaultMenuCommand (::comphelper::getString(
                aPanelNode.getNodeValue("DefaultMenuCommand")));

        ReadContextList(
            aPanelNode,
            rPanelDescriptor.maContextList,
            sDefaultMenuCommand);
    }

    
    
    if (nWriteIndex<nCount)
        maPanels.resize(nWriteIndex);
}




void ResourceManager::ReadContextList (
    const ::utl::OConfigurationNode& rParentNode,
    ContextList& rContextList,
    const OUString& rsDefaultMenuCommand) const
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
                
                
                
                break;
            }
            else
            {
                OSL_FAIL("expecting three or four values per ContextList entry, separated by comma");
                continue;
            }
        }

        const OUString sContextName (sValue.getToken(0, ',', nCharacterIndex).trim());
        if (nCharacterIndex < 0)
        {
            OSL_FAIL("expecting three or four values per ContextList entry, separated by comma");
            continue;
        }

        const OUString sInitialState (sValue.getToken(0, ',', nCharacterIndex).trim());

        
        const OUString sMenuCommandOverride (
            nCharacterIndex<0
                ? OUString()
                : sValue.getToken(0, ',', nCharacterIndex).trim());
        const OUString sMenuCommand (
            sMenuCommandOverride.getLength()>0
                ? (sMenuCommandOverride.equalsAscii("none")
                    ? OUString()
                    : sMenuCommandOverride)
                : rsDefaultMenuCommand);

        
        
        
        
        ::std::vector<EnumContext::Application> aApplications;
        EnumContext::Application eApplication (EnumContext::GetApplicationEnum(sApplicationName));
        if (eApplication == EnumContext::Application_None
            && !sApplicationName.equals(EnumContext::GetApplicationName(EnumContext::Application_None)))
        {
            
            
            if (sApplicationName.equalsAscii("Writer"))
                aApplications.push_back(EnumContext::Application_Writer);
            else if (sApplicationName.equalsAscii("Calc"))
                aApplications.push_back(EnumContext::Application_Calc);
            else if (sApplicationName.equalsAscii("Draw"))
                aApplications.push_back(EnumContext::Application_Draw);
            else if (sApplicationName.equalsAscii("Impress"))
                aApplications.push_back(EnumContext::Application_Impress);
            else if (sApplicationName.equalsAscii("DrawImpress"))
            {
                
                
                
                
                aApplications.push_back(EnumContext::Application_Draw);
                aApplications.push_back(EnumContext::Application_Impress);
            }
            else if (sApplicationName.equalsAscii("WriterVariants"))
            {
                
                aApplications.push_back(EnumContext::Application_Writer);
                aApplications.push_back(EnumContext::Application_WriterGlobal);
                aApplications.push_back(EnumContext::Application_WriterWeb);
                aApplications.push_back(EnumContext::Application_WriterXML);
                aApplications.push_back(EnumContext::Application_WriterForm);
                aApplications.push_back(EnumContext::Application_WriterReport);
            }
            else
            {
                OSL_FAIL("application name not recognized");
                continue;
            }
        }
        else
        {
            
            aApplications.push_back(eApplication);
        }

        
        const EnumContext::Context eContext (EnumContext::GetContextEnum(sContextName));
        if (eContext == EnumContext::Context_Unknown)
        {
            OSL_FAIL("context name not recognized");
            continue;
        }

        
        
        bool bIsInitiallyVisible;
        if (sInitialState.equalsAscii("visible"))
            bIsInitiallyVisible = true;
        else if (sInitialState.equalsAscii("hidden"))
            bIsInitiallyVisible = false;
        else
        {
            OSL_FAIL("unrecognized state");
            continue;
        }

        
        for (::std::vector<EnumContext::Application>::const_iterator
                 iApplication(aApplications.begin()),
                 iEnd(aApplications.end());
             iApplication!=iEnd;
             ++iApplication)
        {
            if (*iApplication != EnumContext::Application_None)
                rContextList.AddContextDescription(
                    Context(
                        EnumContext::GetApplicationName(*iApplication),
                        EnumContext::GetContextName(eContext)),
                    bIsInitiallyVisible,
                    sMenuCommand);
        }
    }
}




void ResourceManager::ReadLegacyAddons (const Reference<frame::XFrame>& rxFrame)
{
    
    ::rtl::OUString sModuleName (Tools::GetModuleName(rxFrame));
    if (sModuleName.getLength() == 0)
        return;
    if (maProcessedApplications.find(sModuleName) != maProcessedApplications.end())
    {
        
        
        return;
    }

    
    
    
    maProcessedApplications.insert(sModuleName);

    
    ::utl::OConfigurationTreeRoot aLegacyRootNode (GetLegacyAddonRootNode(sModuleName));
    if ( ! aLegacyRootNode.isValid())
        return;

    
    ::std::vector<OUString> aMatchingNodeNames;
    GetToolPanelNodeNames(aMatchingNodeNames, aLegacyRootNode);
    const sal_Int32 nCount (aMatchingNodeNames.size());
    size_t nDeckWriteIndex (maDecks.size());
    size_t nPanelWriteIndex (maPanels.size());
    maDecks.resize(maDecks.size() + nCount);
    maPanels.resize(maPanels.size() + nCount);
    for (sal_Int32 nReadIndex(0); nReadIndex<nCount; ++nReadIndex)
    {
        const OUString& rsNodeName (aMatchingNodeNames[nReadIndex]);
        const ::utl::OConfigurationNode aChildNode (aLegacyRootNode.openNode(rsNodeName));
        if ( ! aChildNode.isValid())
            continue;

        if ( rsNodeName == "private:resource/toolpanel/DrawingFramework/CustomAnimations" ||
             rsNodeName == "private:resource/toolpanel/DrawingFramework/Layouts" ||
             rsNodeName == "private:resource/toolpanel/DrawingFramework/MasterPages" ||
             rsNodeName == "private:resource/toolpanel/DrawingFramework/SlideTransitions" ||
             rsNodeName == "private:resource/toolpanel/DrawingFramework/TableDesign" )
          continue;

        DeckDescriptor& rDeckDescriptor (maDecks[nDeckWriteIndex++]);
        rDeckDescriptor.msTitle = ::comphelper::getString(aChildNode.getNodeValue("UIName"));
        rDeckDescriptor.msId = rsNodeName;
        rDeckDescriptor.msIconURL = ::comphelper::getString(aChildNode.getNodeValue("ImageURL"));
        rDeckDescriptor.msHighContrastIconURL = rDeckDescriptor.msIconURL;
        rDeckDescriptor.msTitleBarIconURL = OUString();
        rDeckDescriptor.msHighContrastTitleBarIconURL = OUString();
        rDeckDescriptor.msHelpURL = ::comphelper::getString(aChildNode.getNodeValue("HelpURL"));
        rDeckDescriptor.msHelpText = rDeckDescriptor.msTitle;
        rDeckDescriptor.mbIsEnabled = true;
        rDeckDescriptor.mnOrderIndex = 100000 + nReadIndex;
        rDeckDescriptor.maContextList.AddContextDescription(Context(sModuleName, OUString("any")), true, OUString());

        PanelDescriptor& rPanelDescriptor (maPanels[nPanelWriteIndex++]);
        rPanelDescriptor.msTitle = ::comphelper::getString(aChildNode.getNodeValue("UIName"));
        rPanelDescriptor.mbIsTitleBarOptional = true;
        rPanelDescriptor.msId = rsNodeName;
        rPanelDescriptor.msDeckId = rsNodeName;
        rPanelDescriptor.msTitleBarIconURL = OUString();
        rPanelDescriptor.msHighContrastTitleBarIconURL = OUString();
        rPanelDescriptor.msHelpURL = ::comphelper::getString(aChildNode.getNodeValue("HelpURL"));
        rPanelDescriptor.msImplementationURL = rsNodeName;
        rPanelDescriptor.mnOrderIndex = 100000 + nReadIndex;
        rPanelDescriptor.mbShowForReadOnlyDocuments = false;
        rPanelDescriptor.mbWantsCanvas = false;
        rPanelDescriptor.maContextList.AddContextDescription(Context(sModuleName, OUString("any")), true, OUString());
    }

    
    
    if (nDeckWriteIndex < maDecks.size())
        maDecks.resize(nDeckWriteIndex);
    if (nPanelWriteIndex < maPanels.size())
        maPanels.resize(nPanelWriteIndex);
}




void ResourceManager::StorePanelExpansionState (
    const ::rtl::OUString& rsPanelId,
    const bool bExpansionState,
    const Context& rContext)
{
    for (PanelContainer::iterator
             iPanel(maPanels.begin()),
             iEnd(maPanels.end());
         iPanel!=iEnd;
         ++iPanel)
    {
        if (iPanel->msId.equals(rsPanelId))
        {
            ContextList::Entry* pEntry (
                iPanel->maContextList.GetMatch (rContext));
            if (pEntry != NULL)
                pEntry->mbIsInitiallyVisible = bExpansionState;
        }
    }
}




::utl::OConfigurationTreeRoot ResourceManager::GetLegacyAddonRootNode (
    const ::rtl::OUString& rsModuleName) const
{
    try
    {
        const Reference<XComponentContext> xContext (::comphelper::getProcessComponentContext() );
        const Reference<frame::XModuleManager2> xModuleAccess =
            frame::ModuleManager::create( xContext );
        const ::comphelper::NamedValueCollection aModuleProperties (xModuleAccess->getByName(rsModuleName));
        const ::rtl::OUString sWindowStateRef (aModuleProperties.getOrDefault(
                "ooSetupFactoryWindowStateConfigRef",
                ::rtl::OUString()));

        ::rtl::OUStringBuffer aPathComposer;
        aPathComposer.appendAscii("org.openoffice.Office.UI.");
        aPathComposer.append(sWindowStateRef);
        aPathComposer.appendAscii("/UIElements/States");

        return ::utl::OConfigurationTreeRoot(xContext,
            aPathComposer.makeStringAndClear(), false);
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }

    return ::utl::OConfigurationTreeRoot();
}




void ResourceManager::GetToolPanelNodeNames (
    ::std::vector<OUString>& rMatchingNames,
    const ::utl::OConfigurationTreeRoot aRoot) const
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
    const Reference<frame::XFrame>& rxFrame) const
{
    
    
    ResourceManager::PanelContextDescriptorContainer aPanelContextDescriptors;
    ResourceManager::Instance().GetMatchingPanels(
        aPanelContextDescriptors,
        rContext,
        rsDeckId,
        rxFrame);

    for (ResourceManager::PanelContextDescriptorContainer::const_iterator
             iPanel(aPanelContextDescriptors.begin()),
             iEnd(aPanelContextDescriptors.end());
         iPanel!=iEnd;
         ++iPanel)
    {
        if (iPanel->mbShowForReadOnlyDocuments)
            return true;
    }

    return false;
}


} } 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

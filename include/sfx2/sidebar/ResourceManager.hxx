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
#ifndef INCLUDED_SFX2_SOURCE_SIDEBAR_RESOURCEMANAGER_HXX
#define INCLUDED_SFX2_SOURCE_SIDEBAR_RESOURCEMANAGER_HXX

#include "DeckDescriptor.hxx"
#include "PanelDescriptor.hxx"
#include <sfx2/sidebar/Context.hxx>
#include <unotools/confignode.hxx>
#include <com/sun/star/frame/XController.hpp>
#include <set>
#include <svtools/miscopt.hxx>

namespace sfx2 { namespace sidebar {

class Context;
class ContextList;

/** Read the content of the Sidebar.xcu file and provide access
    methods so that the sidebar can easily decide which content panels
    to display for a certain context.
*/
class ResourceManager
{
public:

     ResourceManager();
    ~ResourceManager();

    static ResourceManager& Instance();

    const DeckDescriptor* GetDeckDescriptor(const OUString& rsDeckId) const;
    const PanelDescriptor* GetPanelDescriptor(const OUString& rsPanelId) const;

    /** Excluded or include a deck from being displayed in the tab
        bar.
        Note that this value is not persistent.
        The flag can not be set directly at a DeckDescriptor object
        because the ResourceManager gives access to them only
        read-only.
    */
    void SetIsDeckEnabled(const OUString& rsDeckId, const bool bIsEnabled);

    void SetDeckTitle(const OUString& rsDeckId, const OUString& sTitle);

    void SetDeckToDescriptor(const OUString& rsDeckId, VclPtr<Deck> aDeck);

    void SetDeckOrderIndex(const OUString& rsDeckId, const sal_Int32 orderIndex);

    void SetPanelOrderIndex(const OUString& rsPanelId, const sal_Int32 orderIndex);

    void UpdateModel(css::uno::Reference<css::frame::XModel> xModel);

    void disposeDecks();

    class DeckContextDescriptor
    {
    public:
        OUString msId;
        bool mbIsEnabled;
    };
    typedef std::vector<DeckContextDescriptor> DeckContextDescriptorContainer;

    class PanelContextDescriptor
    {
    public:
        OUString msId;
        OUString msMenuCommand;
        bool mbIsInitiallyVisible;
        bool mbShowForReadOnlyDocuments;
    };
    typedef std::vector<PanelContextDescriptor> PanelContextDescriptorContainer;

    const DeckContextDescriptorContainer& GetMatchingDecks(
                                            DeckContextDescriptorContainer& rDeckDescriptors,
                                            const Context& rContext,
                                            const bool bIsDocumentReadOnly,
                                            const css::uno::Reference<css::frame::XController>& rxController);

    const PanelContextDescriptorContainer& GetMatchingPanels(
                                            PanelContextDescriptorContainer& rPanelDescriptors,
                                            const Context& rContext,
                                            const OUString& rsDeckId,
                                            const css::uno::Reference<css::frame::XController>& rxController);

    /** Remember the expansions state per panel and context.
        This is not persistent past application end.
    */
    void StorePanelExpansionState(const OUString& rsPanelId,
                                  const bool bExpansionState,
                                  const Context& rContext);

private:


    typedef std::vector<DeckDescriptor> DeckContainer;
    DeckContainer maDecks;

    typedef std::vector<PanelDescriptor> PanelContainer;
    PanelContainer maPanels;
    mutable std::set<rtl::OUString> maProcessedApplications;

    SvtMiscOptions maMiscOptions;

    void ReadDeckList();
    void ReadPanelList();
    static void ReadContextList(const utl::OConfigurationNode& rNode,
                         ContextList& rContextList,
                         const OUString& rsDefaultMenuCommand);
    void ReadLegacyAddons(const css::uno::Reference<css::frame::XController>& rxController);
    static utl::OConfigurationTreeRoot GetLegacyAddonRootNode(const OUString& rsModuleName);
    static void GetToolPanelNodeNames(std::vector<OUString>& rMatchingNames,
                               const utl::OConfigurationTreeRoot& aRoot);
    static bool IsDeckEnabled(const OUString& rsDeckId,
                       const Context& rContext,
                       const css::uno::Reference<css::frame::XController>& rxController);
};

} } // end of namespace sfx2::sidebar

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

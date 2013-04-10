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
#ifndef SFX_SIDEBAR_RESOURCE_MANAGER_HXX
#define SFX_SIDEBAR_RESOURCE_MANAGER_HXX

#include "DeckDescriptor.hxx"
#include "PanelDescriptor.hxx"
#include "Context.hxx"
#include <unotools/confignode.hxx>
#include <com/sun/star/frame/XFrame.hpp>
#include <set>
#include <boost/shared_ptr.hpp>


namespace css = ::com::sun::star;
namespace cssu = ::com::sun::star::uno;

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
    static ResourceManager& Instance (void);

    const DeckDescriptor* GetBestMatchingDeck (
        const Context& rContext,
        const cssu::Reference<css::frame::XFrame>& rxFrame);

    const DeckDescriptor* GetDeckDescriptor (
        const ::rtl::OUString& rsDeckId) const;
    const PanelDescriptor* GetPanelDescriptor (
        const ::rtl::OUString& rsPanelId) const;

    /** Excluded or include a deck from being displayed in the tab
        bar.
        Note that this value is not persistent.
        The flag can not be set directly at a DeckDescriptor object
        because the ResourceManager gives access to to them only
        read-only.
    */
    void SetIsDeckEnabled (
        const ::rtl::OUString& rsDeckId,
        const bool bIsEnabled);

    typedef ::std::vector<rtl::OUString> IdContainer;
    class PanelContextDescriptor
    {
    public:
        ::rtl::OUString msId;
        ::rtl::OUString msMenuCommand;
        bool mbIsInitiallyVisible;
    };
    typedef ::std::vector<PanelContextDescriptor> PanelContextDescriptorContainer;

    const IdContainer& GetMatchingDecks (
        IdContainer& rDeckDescriptors,
        const Context& rContext,
        const cssu::Reference<css::frame::XFrame>& rxFrame);

    const PanelContextDescriptorContainer& GetMatchingPanels (
        PanelContextDescriptorContainer& rPanelDescriptors,
        const Context& rContext,
        const ::rtl::OUString& rsDeckId,
        const cssu::Reference<css::frame::XFrame>& rxFrame);

    static ::rtl::OUString GetModuleName (
        const cssu::Reference<css::frame::XFrame>& rxFrame);

private:
    ResourceManager (void);
    ~ResourceManager (void);
    class Deleter;
    friend class Deleter;

    typedef ::std::vector<DeckDescriptor> DeckContainer;
    DeckContainer maDecks;
    typedef ::std::vector<PanelDescriptor> PanelContainer;
    PanelContainer maPanels;
    mutable ::std::set<rtl::OUString> maProcessedApplications;

    void ReadDeckList (void);
    void ReadPanelList (void);
    void ReadContextList (
        const ::utl::OConfigurationNode& rNode,
        ContextList& rContextList,
        const ::rtl::OUString& rsDefaultMenuCommand) const;
    void ReadLegacyAddons (
        const cssu::Reference<css::frame::XFrame>& rxFrame);
    ::utl::OConfigurationTreeRoot GetLegacyAddonRootNode (
        const ::rtl::OUString& rsModuleName) const;
    void GetToolPanelNodeNames (
        ::std::vector<rtl::OUString>& rMatchingNames,
        const ::utl::OConfigurationTreeRoot aRoot) const;
};


} } // end of namespace sfx2::sidebar

#endif

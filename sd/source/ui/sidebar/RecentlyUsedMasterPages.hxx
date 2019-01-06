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

#ifndef INCLUDED_SD_SOURCE_UI_SIDEBAR_RECENTLYUSEDMASTERPAGES_HXX
#define INCLUDED_SD_SOURCE_UI_SIDEBAR_RECENTLYUSEDMASTERPAGES_HXX

#include <tools/SdGlobalResourceContainer.hxx>
#include <tools/link.hxx>
#include <vector>

#include "MasterPageContainer.hxx"

namespace sd {
class MasterPageObserverEvent;
}

namespace sd { namespace sidebar {

/** This singleton holds a list of the most recently used master pages.
*/
class RecentlyUsedMasterPages
    : public SdGlobalResource
{
public:
    /** Return the single instance of this class.
    */
    static RecentlyUsedMasterPages& Instance();

    void AddEventListener (const Link<LinkParamNone*,void>& rEventListener);
    void RemoveEventListener (const Link<LinkParamNone*,void>& rEventListener);

    int GetMasterPageCount() const;
    MasterPageContainer::Token GetTokenForIndex (sal_uInt32 nIndex) const;

private:
    class Descriptor
    {
    public:
        OUString msURL;
        OUString msName;
        ::sd::sidebar::MasterPageContainer::Token maToken;
        Descriptor (::sd::sidebar::MasterPageContainer::Token aToken,
                    const OUString& rsURL, const OUString& rsName)
            : msURL(rsURL),
              msName(rsName),
              maToken(aToken)
        {}

        class TokenComparator
        {
        public:
            explicit TokenComparator(::sd::sidebar::MasterPageContainer::Token aToken)
                : maToken(aToken) {}
            bool operator () (const Descriptor& rDescriptor)
            { return maToken==rDescriptor.maToken; }

        private:
            ::sd::sidebar::MasterPageContainer::Token const maToken;
        };
    };

    /** The single instance of this class.  It is created on demand when
        Instance() is called for the first time.
    */
    static RecentlyUsedMasterPages* mpInstance;

    ::std::vector<Link<LinkParamNone*,void>> maListeners;

    typedef ::std::vector<Descriptor> MasterPageList;
    MasterPageList mvMasterPages;
    std::shared_ptr<MasterPageContainer> mpContainer;

    RecentlyUsedMasterPages();
    virtual ~RecentlyUsedMasterPages() override;

    /** Call this method after a new object has been created.
    */
    void LateInit();

    RecentlyUsedMasterPages (const RecentlyUsedMasterPages&) = delete;

    RecentlyUsedMasterPages& operator= (const RecentlyUsedMasterPages&) = delete;

    void SendEvent();
    DECL_LINK(MasterPageChangeListener, MasterPageObserverEvent&, void);
    DECL_LINK(MasterPageContainerChangeListener, MasterPageContainerChangeEvent&, void);

    /** Add a descriptor for the specified master page to the end of the
        list of most recently used master pages.  When the page is already a
        member of that list the associated descriptor is moved to the end of
        the list to make it the most recently used entry.
    */
    void AddMasterPage(MasterPageContainer::Token aToken);

    /** Load the list of recently used master pages from the registry where
        it was saved to make it persistent.
    */
    void LoadPersistentValues();

    /** Save the list of recently used master pages to the registry to make
        it presistent.
    */
    void SavePersistentValues();

    void ResolveList();
};

} } // end of namespace sd::sidebar

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

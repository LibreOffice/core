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

#ifndef SD_SIDEBAR_PANELS_RECENTLY_USED_MASTER_PAGES_HXX
#define SD_SIDEBAR_PANELS_RECENTLY_USED_MASTER_PAGES_HXX

#include "tools/SdGlobalResourceContainer.hxx"
#include <osl/mutex.hxx>
#include <tools/link.hxx>
#include <vcl/image.hxx>
#include <vector>

#include "DrawDocShell.hxx"
#include "MasterPageContainer.hxx"
#include <com/sun/star/uno/XInterface.hpp>

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
    static RecentlyUsedMasterPages& Instance (void);

    void AddEventListener (const Link& rEventListener);
    void RemoveEventListener (const Link& rEventListener);

    int GetMasterPageCount (void) const;
    MasterPageContainer::Token GetTokenForIndex (sal_uInt32 nIndex) const;

private:
    /** The single instance of this class.  It is created on demand when
        Instance() is called for the first time.
    */
    static RecentlyUsedMasterPages* mpInstance;

    ::std::vector<Link> maListeners;

    class MasterPageList;
    ::std::auto_ptr<MasterPageList> mpMasterPages;
    unsigned long int mnMaxListSize;
    ::boost::shared_ptr<MasterPageContainer> mpContainer;

    RecentlyUsedMasterPages (void);
    virtual ~RecentlyUsedMasterPages (void);

    /** Call this method after a new object has been created.
    */
    void LateInit (void);

    /// The copy constructor is not implemented.  Do not use!
    RecentlyUsedMasterPages (const RecentlyUsedMasterPages&);

    /// The assignment operator is not implemented.  Do not use!
    RecentlyUsedMasterPages& operator= (const RecentlyUsedMasterPages&);

    void SendEvent (void);
    DECL_LINK(MasterPageChangeListener, MasterPageObserverEvent*);
    DECL_LINK(MasterPageContainerChangeListener, MasterPageContainerChangeEvent*);

    /** Add a descriptor for the specified master page to the end of the
        list of most recently used master pages.  When the page is already a
        member of that list the associated descriptor is moved to the end of
        the list to make it the most recently used entry.
        @param bMakePersistent
            When <TRUE/> is given then the new list of recently used master
            pages is written back into the configuration to make it
            persistent.  Giving <FALSE/> to ommit this is used while loading
            the persistent list from the configuration.
    */
    void AddMasterPage (
        MasterPageContainer::Token aToken,
        bool bMakePersistent = true);

    /** Load the list of recently used master pages from the registry where
        it was saved to make it persistent.
    */
    void LoadPersistentValues (void);

    /** Save the list of recently used master pages to the registry to make
        it presistent.
    */
    void SavePersistentValues (void);

    void ResolveList (void);
};



} } // end of namespace sd::sidebar

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

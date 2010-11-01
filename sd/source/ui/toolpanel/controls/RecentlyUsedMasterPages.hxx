/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef SD_TOOLPANEL_CONTROLS_RECENTLY_USED_MASTER_PAGES_HXX
#define SD_TOOLPANEL_CONTROLS_RECENTLY_USED_MASTER_PAGES_HXX

#include "tools/SdGlobalResourceContainer.hxx"
#include <osl/mutex.hxx>
#include <tools/link.hxx>
#include <vcl/image.hxx>
#include <vector>
#include <tools/string.hxx>

#include "DrawDocShell.hxx"
#include "MasterPageContainer.hxx"
#include <com/sun/star/uno/XInterface.hpp>

class SdPage;

namespace sd {
class MasterPageObserverEvent;
}


namespace sd { namespace toolpanel { namespace controls {

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



} } } // end of namespace ::sd::toolpanel::controls

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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

#ifndef INCLUDED_SD_SOURCE_UI_INC_MASTERPAGEOBSERVER_HXX
#define INCLUDED_SD_SOURCE_UI_INC_MASTERPAGEOBSERVER_HXX

#include <rtl/ustring.hxx>
#include "tools/SdGlobalResourceContainer.hxx"
#include <osl/mutex.hxx>
#include <memory>
#include <set>

class SdDrawDocument;

namespace sd {

class MasterPageObserverEvent;

/** This singleton observes all registered documents for changes in the used
    master pages and in turn informs its listeners about it.  One such
    listener is the master page selector control in the tool panel that
    shows the recently used master pages.
*/
class MasterPageObserver
    : public SdGlobalResource
{
public:
    typedef ::std::set<OUString> MasterPageNameSet;

    /** Return the single instance of this class.
    */
    static MasterPageObserver& Instance();

    /** The master page observer will listen to events of this document and
        detect changes of the use of master pages.
    */
    void RegisterDocument (SdDrawDocument& rDocument);

    /** The master page observer will stop to listen to events of this
        document.
    */
    void UnregisterDocument (SdDrawDocument& rDocument);

    /** Add a listener that is informed of master pages that are newly
        assigned to slides or become unassigned.
        @param rEventListener
            The event listener to call for future events.  Call
            RemoveEventListener() before the listener is destroyed.
    */
    void AddEventListener (const Link<MasterPageObserverEvent&,void>& rEventListener);

    /** Remove the given listener from the list of listeners.
        @param rEventListener
            After this method returns the given listener is not called back
            from this object.  Passing a listener that has not
            been registered before is safe and is silently ignored.
    */
    void RemoveEventListener (const Link<MasterPageObserverEvent&,void>& rEventListener);

private:
    static ::osl::Mutex maMutex;

    class Implementation;
    ::std::unique_ptr<Implementation> mpImpl;

    MasterPageObserver();
    virtual ~MasterPageObserver();

    MasterPageObserver (const MasterPageObserver&) = delete;

    MasterPageObserver& operator= (const MasterPageObserver&) = delete;
};

/** Objects of this class are sent to listeners of the MasterPageObserver
    singleton when the list of master pages of one document has changed.
*/
class MasterPageObserverEvent
{
public:
    enum EventType {
        /// Master page already exists when document is registered.
        ET_MASTER_PAGE_EXISTS,
        /// Master page has been added to a document.
        ET_MASTER_PAGE_ADDED,
        /// Master page has been removed from to a document.
        ET_MASTER_PAGE_REMOVED
    };

    EventType meType;
    const OUString& mrMasterPageName;

    MasterPageObserverEvent (
        EventType eType,
        const OUString& rMasterPageName)
        : meType(eType),
          mrMasterPageName(rMasterPageName)
    {}

};

} // end of namespace sd

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

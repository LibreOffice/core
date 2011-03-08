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

#ifndef SD_VIEW_MASTER_PAGE_OBSERVER_HXX
#define SD_VIEW_MASTER_PAGE_OBSERVER_HXX

#include "tools/SdGlobalResourceContainer.hxx"
#include <osl/mutex.hxx>
#include <tools/link.hxx>
#include <memory>
#include <set>

class SdDrawDocument;
class String;

namespace sd {

/** This singleton observes all registered documents for changes in the used
    master pages and in turn informs its listeners about it.  One such
    listener is the master page selector control in the tool panel that
    shows the recently used master pages.
*/
class MasterPageObserver
    : public SdGlobalResource
{
public:
    typedef ::std::set<String> MasterPageNameSet;

    /** Return the single instance of this class.
    */
    static MasterPageObserver& Instance (void);

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
    void AddEventListener (const Link& rEventListener);

    /** Remove the given listener from the list of listeners.
        @param rEventListener
            After this method returns the given listener is not called back
            from this object.  Passing a listener that has not
            been registered before is safe and is silently ignored.
    */
    void RemoveEventListener (const Link& rEventListener);

private:
    static ::osl::Mutex maMutex;

    class Implementation;
    ::std::auto_ptr<Implementation> mpImpl;

    MasterPageObserver (void);
    virtual ~MasterPageObserver (void);

    /// The copy constructor is not implemented.  Do not use!
    MasterPageObserver (const MasterPageObserver&);

    /// The assignment operator is not implemented.  Do not use!
    MasterPageObserver& operator= (const MasterPageObserver&);
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
    SdDrawDocument& mrDocument;
    const String& mrMasterPageName;

    MasterPageObserverEvent (
        EventType eType,
        SdDrawDocument& rDocument,
        const String& rMasterPageName)
        : meType(eType),
          mrDocument(rDocument),
          mrMasterPageName(rMasterPageName)
    {}

};

} // end of namespace sd

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

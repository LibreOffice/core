/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/*
 * This API is asynchronous, and will emit its callbacks in the LibreOffice
 * main-thread, ie. it is not necessarily thread-safe, take the solar mutex
 * to use it. All methods should be fast - ie. can be called in a user
 * interface context without significant blocking. Any threading used by
 * implementations should not be exposed to the caller directly.
 */

#include <boost/signals2.hpp>

/*
 * FIXME: need to use smart ptrs everywhere.
 */

namespace SlideHack {

/// version history
class VersionData
{
    virtual OUString getVersion() = 0;
    virtual Date     getCheckinDate() = 0;
    virtual OUString getCheckinComment() = 0;
};

/// Defines information about a group of slides
class GroupMeta
{
public:
    virtual OUString getName() = 0;
    virtual OUString getUserName() = 0;
    virtual OUString getTopic() = 0;

    /// number of slides
    virtual int      getLength() = 0;

    // Cedric: can this be easily fetched in one chunk ?
    virtual VersionData *getVersionData() = 0;
};

/// Defines a group of slides on a related topic
class Group
{
public:
    virtual GroupMeta *getMetaData() = 0;

    /// fetches slide data from the potentially remote store

    ///    FIXME: <we want a boost callback - this is lame !>
    virtual void   readData( GroupDataCallback *pCallback ) = 0;
    virtual void   cancelReadData( GroupDataCallback *pCallback ) = 0;

    /// retrieve slide data - but only after a readData has completed
    virtual BitmapEx getSlide( sal_uInt32 nSlide ) = 0;
    /// get slide data as flat odp ?
    virtual OUString getSlideFodp( sal_uInt32 nSlide ) = 0;

    /// start fetch of all version numbers
    virtual void     getVersions() = 0;
    /// version number fetch completed - ver history is linear
    boost::signals2< VersionData * > maVersions;
};

/// We can have multiple (different length) pitches for the same topic
class Alternatives
{
public:
    std::vector< Group * > getAlternatives() = 0;
};

/// Overall factory and store for these guys
class Store
{
public:
    /// initiate search returns a handle
    virtual sal_uInt32 search( OUString aSearchEntry ) = 0;
    /// cancel a running search
    virtual void  cancelSearch( sal_uInt32 nHandle ) = 0;
    /// search completed signal - always in the main thread
    boost::signals2< sal_uInt32, std::vector<Alternatives *> > maSearchCompleted;

    /// used to create a group handle from a stored slide, so we can
    /// check for updated versions etc.
    virtual Group *createGroup( OUString aName, OUString aVersion ) = 0;

    /// factory function: to get the root
    static SlideHackStore *getStore();
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

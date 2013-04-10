/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef _SD_SLIDEHACK_HXX
#define _SD_SLIDEHACK_HXX

/*
 * This API is asynchronous, and will emit its callbacks in the LibreOffice
 * main-thread, ie. it is not necessarily thread-safe, take the solar mutex
 * to use it. All methods should be fast - ie. can be called in a user
 * interface context without significant blocking. Any threading used by
 * implementations should not be exposed to the caller directly.
 */

// Would be nice if boost::signals2 was less dirty ...
#ifdef __GNUC__
#  pragma GCC diagnostic ignored "-Wshadow"
#  pragma GCC diagnostic ignored "-Wundef"
#  pragma GCC diagnostic ignored "-Wempty-body"
#endif

#include <vector>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/noncopyable.hpp>
#include <boost/signals2.hpp>

#ifdef __GNUC__
#  pragma GCC diagnostic pop
#  pragma GCC diagnostic pop
#  pragma GCC diagnostic pop
#endif

#include <rtl/ustring.hxx>
#include <tools/datetime.hxx>
#include <vcl/bitmapex.hxx>

class SdPage;

namespace SlideHack {

typedef boost::shared_ptr< class Store > StorePtr;
typedef boost::shared_ptr< class Group > GroupPtr;
typedef boost::shared_ptr< class Origin > OriginPtr;
typedef boost::shared_ptr< class GroupMeta > GroupMetaPtr;
typedef boost::shared_ptr< class VersionData > VersionDataPtr;
typedef boost::shared_ptr< class OriginDetails > OriginDetailsPtr;

/// version history
class VersionData : public boost::enable_shared_from_this< VersionData >,
                    private boost::noncopyable
{
public:
    virtual ~VersionData() {}
    virtual OUString getVersion() = 0;
    virtual DateTime getCheckinTime() = 0;
    virtual OUString getCheckinComment() = 0;
};

/// Tracking where a single slide came from
class Origin
{
    friend class Store;
private:
    Origin( const OUString &rURL );
public:
    virtual ~Origin() {}
    /// get a URL for our origin, which encodes version, alternative, etc.
    OUString getURL();
};

/// Tracking where a single slide came from and some policy around that
class OriginDetails : public Origin
{
public:
    /// how should we set about updating data from this origin ?
    enum UpdateType { ORIGIN_UPDATE_NEVER, ORIGIN_UPDATE_AUTO, ORIGIN_UPDATE_MANUAL };

    OriginDetails( const Origin &rOrigin, UpdateType eUpdateType );
    virtual ~OriginDetails() {}

    UpdateType getUpdateType() { return meUpdateType; }
    void setUpdateType( UpdateType eUpdateType) { meUpdateType = eUpdateType; }

    /// for save
    OUString toString();
    /// for load
    OUString fromString();
private:
    UpdateType meUpdateType;
};

/// Defines information about a group of slides
class GroupMeta : public boost::enable_shared_from_this< GroupMeta >,
                  private boost::noncopyable
{
public:
    virtual ~GroupMeta() {}
    virtual OUString   getName() = 0;
    virtual OUString   getUserName() = 0;
    virtual OUString   getTopic() = 0;

    /// number of slides
    virtual int        getLength() = 0;
    virtual OriginPtr  getOriginForSlide( sal_uInt32 nSlide ) = 0;

    // Cedric: can this be easily fetched in one chunk ?
    virtual VersionDataPtr getVersionData() = 0;
};

/// Defines a group of slides on a related topic
class Group : public boost::enable_shared_from_this< Group >,
              private boost::noncopyable
{
public:
    virtual ~Group() {}

    virtual GroupMetaPtr getMetaData() = 0;

    /// fetches slide data from the potentially remote store

    /// initiate reading the slide thumbnail and/or ODP
    virtual void fetchData( bool bThumbnail, bool bODPStream ) = 0;
    /// data fetch completed signal - always in the main thread
    boost::signals2::signal< void (BitmapEx &, SvStream *) > maDataComplete;

    /// start fetch of all version numbers
    virtual void     getVersions() = 0;
    /// version number fetch completed - ver history is linear
    boost::signals2::signal< void (const std::vector< VersionDataPtr > &) > maVersions;
};

/// Overall factory and store for these guys
class Store : public boost::enable_shared_from_this< Store >,
              private boost::noncopyable
{
public:
    virtual ~Store() {}

    /// initiate search returns a handle
    virtual sal_uInt32 search( OUString aSearchEntry ) = 0;
    /// cancel a running search
    virtual void  cancelSearch( sal_uInt32 nHandle ) = 0;
    /// search completed signal - always in the main thread
    boost::signals2::signal< void(sal_uInt32, std::vector< GroupPtr >) > maSearchCompleted;

    /// used to create a group handle from a stored slide, so we can
    /// check for updated versions etc.
    virtual GroupPtr lookupGroup( OriginPtr pOrigin ) = 0;
    virtual GroupPtr createGroup( const OUString &rName,
                                  const OUString &rKeywords,
                                  const std::vector< SdPage * > &rPages ) = 0;

    /// factory function: to get the root
    static StorePtr getStore();
};

}

#endif // _SD_SLIDEHACK_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

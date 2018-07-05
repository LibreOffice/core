/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_UCB_SOURCE_UCP_WEBDAV_NEON_PROPFINDCACHE_HXX
#define INCLUDED_UCB_SOURCE_UCP_WEBDAV_NEON_PROPFINDCACHE_HXX

#include <sal/types.h>
#include <rtl/ustring.hxx>
#include <osl/mutex.hxx>
#include <list>
#include <map>
#include <vector>

#include "DAVResource.hxx"

namespace webdav_ucp
{
    // A property names cache mechanism, URL driven.
    // It is used to cache the property names received
    // from the WebDAV server, to minimize the need of
    // net transactions (e.g. PROPFIND).
    // The cache lifetime should be short
    // just to remove the annoying slowness when
    // typing text or moving cursor around when the
    // net link is slow.

    // Define the properties cache element
    class PropertyNames final
    {
        /// target time when this element becomes stale
        sal_uInt32 m_nStaleTime;
        OUString    m_sURL;
        // the property name list received from WebDAV server
        std::vector< DAVResourceInfo > m_aPropertiesNames;

    public:
        PropertyNames();
        explicit PropertyNames( const OUString& rURL );

        sal_uInt32 getStaleTime() const { return m_nStaleTime; };
        void setStaleTime( const sal_uInt32 nStaleTime ) { m_nStaleTime = nStaleTime; };

        OUString& getURL() { return m_sURL; };

        const std::vector< DAVResourceInfo >& getPropertiesNames() { return m_aPropertiesNames; };
        void setPropertiesNames( const std::vector< DAVResourceInfo >& aPropertiesNames ) { m_aPropertiesNames = aPropertiesNames; };
    };

    // Define the PropertyNames cache
    // TODO: the OUString key element in std::map needs to be changed with a URI representation
    // with a specific compare (std::less) implementation, this last one implementing
    // as suggested in <https://tools.ietf.org/html/rfc3986#section-6>.
    // To find by URI and not by string equality.
    typedef std::map< OUString, PropertyNames,
                      std::less< OUString > >PropNameCache;

    class PropertyNamesCache final
    {
        PropNameCache       m_aTheCache;
        osl::Mutex          m_aMutex;

    public:
        PropertyNamesCache();
        ~PropertyNamesCache();

        bool getCachedPropertyNames( const OUString& URL, PropertyNames& rCacheElement );
        void removeCachedPropertyNames( const OUString& URL );
        void addCachePropertyNames( PropertyNames& rCacheElement );
    };

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */

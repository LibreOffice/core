/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef SVL_STRINGPOOL_HXX
#define SVL_STRINGPOOL_HXX

#include "svl/svldllapi.h"
#include "rtl/ustring.hxx"

#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>

class CharClass;

namespace svl {

/**
 * Storage for pool of shared strings.  It also provides mapping from
 * original-cased strings to upper-cased strings for case insensitive
 * operations.
 */
class SVL_DLLPUBLIC StringPool
{
    typedef boost::unordered_set<OUString, OUStringHash> StrHashType;
    typedef std::pair<StrHashType::iterator, bool> InsertResultType;
    typedef boost::unordered_map<const rtl_uString*, OUString> StrIdMapType;

    StrHashType maStrPool;
    StrHashType maStrPoolUpper;
    StrIdMapType maToUpperMap;
    const CharClass* mpCharClass;

public:
    typedef sal_uIntPtr StrIdType;

    StringPool();
    StringPool( const CharClass* pCharClass );

    /**
     * Intern a string object into the shared string pool.
     *
     * @param rStr string object to intern.
     *
     * @return a pointer to the string object stored inside the pool, or NULL
     *         if the insertion fails.
     */
    rtl_uString* intern( const OUString& rStr );

    /**
     * Get a unique ID of string object that's expected to be in the shared
     * string pool. If the string is not in the pool, NULL is returned.  The
     * ID obtained by this method can be used for case sensitive comparison.
     *
     * @param rStr string object to get the ID of.
     *
     * @return unique ID of the string object.
     */
    StrIdType getIdentifier( const OUString& rStr ) const;

    /**
     * Get a unique ID of string object for case insensitive comparison. The
     * string object is expected to be in the pool.
     *
     * @param rStr string object to get the ID of.
     *
     * @return unique ID of the string object usable for case insensitive
     *         comparison.
     */
    StrIdType getIdentifierIgnoreCase( const OUString& rStr ) const;

    /**
     * Go through all string objects in the pool, and clear those that are no
     * longer used outside of the pool.
     */
    void purge();

    size_t getCount() const;

    size_t getCountIgnoreCase() const;

private:
    InsertResultType findOrInsert( StrHashType& rPool, const OUString& rStr ) const;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SVL_SHAREDSTRINGPOOL_HXX
#define INCLUDED_SVL_SHAREDSTRINGPOOL_HXX

#include <svl/svldllapi.h>
#include <rtl/ustring.hxx>

class CharClass;

namespace svl {

class SharedString;

/**
 * Storage for pool of shared strings.  It also provides mapping from
 * original-cased strings to upper-cased strings for case insensitive
 * operations.
 */
class SVL_DLLPUBLIC SharedStringPool
{
    struct Impl;
    Impl* mpImpl;

    SharedStringPool( const SharedStringPool& ) = delete;
    SharedStringPool& operator=( const SharedStringPool& ) = delete;

public:
    SharedStringPool( const CharClass* pCharClass );
    ~SharedStringPool();

    /**
     * Intern a string object into the shared string pool.
     *
     * @param rStr string object to intern.
     *
     * @return a pointer to the string object stored inside the pool, or NULL
     *         if the insertion fails.
     */
    SharedString intern( const OUString& rStr );

    /**
     * Go through all string objects in the pool, and clear those that are no
     * longer used outside of the pool.
     */
    void purge();

    size_t getCount() const;

    size_t getCountIgnoreCase() const;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

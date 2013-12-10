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

#ifndef INCLUDED_WW8_FKP_HXX
#define INCLUDED_WW8_FKP_HXX

#include <WW8CpAndFc.hxx>

namespace writerfilter {
namespace doctok
{

/**
   A formated diskpage (FKP).

   Formatted diskpages are used by Word to store runs of SPRMs (single
   property modifier). A FKP contains a list of file character
   positions (FC). For each FC there is an entry containing the run of
   SPRMs stored for that FC.
 */
class WW8FKP : public WW8StructBase
{
public:
    /**
       Shared pointer to an FKP.
     */
    typedef boost::shared_ptr<WW8FKP> Pointer_t;

    /**
       Create FKP from stream.

       @param rStream    stream to create FKP from
       @param nOffset    offset in stream where the FKP starts.
     */
    WW8FKP(WW8Stream & rStream, sal_uInt32 nOffset)
    : WW8StructBase(rStream, nOffset, 512)
    {
    }

    /**
       Return count of entries.
    */
    virtual sal_uInt32 getEntryCount() const = 0;

    /**
       Return an FC.

       @param nIndex   index of the FC to return
     */
    virtual Fc getFc(sal_uInt32 nIndex) const = 0;

    /**
       Returns the first FC of the FKP.
    */
    virtual Fc getFirstFc() const = 0;

    /**
       Returns the last FC of the FKP.
     */
    virtual Fc getLastFc() const = 0;

    /**
       Check if an FKP contains an FC.

       @param rFc   FC to look for

       @retval true    FKP contains FC
       @retval false   else
     */
    virtual bool contains(const Fc & rFc) const = 0;

    /**
       Return properties stored in an FKP for a FC.

       @param rFc   FC to look for
     */
    virtual writerfilter::Reference<Properties>::Pointer_t
    getProperties(const Fc & rFc)
        const = 0;
};

/**
   Cache providing FKPs.
 */
class WW8FKPCache
{
public:
    /**
       Shared pointer to cache.
     */
    typedef boost::shared_ptr<WW8FKPCache> Pointer_t;

    virtual ~WW8FKPCache();

    /**
       Return FKP.

       @param nPageNumber    number of page to return
     */
    virtual WW8FKP::Pointer_t get(sal_uInt32 nPageNumber, bool bComplex) = 0;
};
}}

#endif // INCLUDED_WW8_FKP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

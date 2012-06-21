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

#ifndef INCLUDED_WW8_BIN_TABLE_HXX
#define INCLUDED_WW8_BIN_TABLE_HXX

#include <boost/shared_ptr.hpp>
#include <string>
#include <WW8FKP.hxx>

namespace writerfilter {
namespace doctok
{

/**
   A bintable.

   Word uses bintables to associate FC ranges with FKPs. A bintable
   has a list of FCs. At each FC a range begins. The length of the
   range is given by the distance of the according CPs.
 */
class WW8BinTable
{
public:
    virtual ~WW8BinTable() {};
    /**
       Shared pointer to a bintable.
     */
    typedef boost::shared_ptr<WW8BinTable> Pointer_t;

    /**
       Return count of entries.
     */
    virtual sal_uInt32 getEntryCount() const = 0;

    /**
       Return FC from bintable.

       @param nIndex    index in bintable to return FC from
     */
    virtual Fc getFc(sal_uInt32 nIndex) const = 0;

    /**
       Return page number.

       @param nIndex    index in bintable to return page number from
     */
    virtual sal_uInt32 getPageNumber(sal_uInt32 nIndex) const = 0;

    /**
       Return page number associated with FC.

       @param rFc      FC to return page number for
     */
    virtual sal_uInt32 getPageNumber(const Fc & rFc) const = 0;

    /**
       Return string representation of bintable.
     */
    virtual string toString() const = 0;
};
}}

#endif // INCLUDED_WW8_BIN_TABLE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

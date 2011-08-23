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

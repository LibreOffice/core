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

#ifndef ARY_LOC_LOCS_LE_HXX
#define ARY_LOC_LOCS_LE_HXX

// BASE CLASSES
#include <store/s_storage.hxx>
// USED SERVICES
#include <cosv/tpl/tpltools.hxx>
#include <ary/loc/loc_le.hxx>
#include <ary/loc/loc_root.hxx>
#include <sortedids.hxx>




namespace ary
{
namespace loc
{


/** The data base for all ->ary::cpp::CodeEntity objects.
*/
class Le_Storage : public ::ary::stg::Storage<LocationEntity>
{
  public:
    typedef SortedIds<Le_Compare>           Index;

                        Le_Storage();
    virtual             ~Le_Storage();

    const Index &       RootIndex() const         { return aRoots; }
    Index &             RootIndex()               { return aRoots; }

    static Le_Storage & Instance_()               { csv_assert(pInstance_ != 0);
                                                    return *pInstance_; }
  private:
    // DATA
    Index               aRoots;

    static Le_Storage * pInstance_;
};




namespace predefined
{

enum E_LocationEntity
{
    le_MAX = 1
};

}   // namespace predefined




}   // namespace cpp
}   // namespace ary
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

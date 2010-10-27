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

#ifndef ARY_LOC_LOCA_LE_HXX
#define ARY_LOC_LOCA_LE_HXX

// BASE CLASSES
#include <ary/loc/locp_le.hxx>

namespace ary
{
namespace loc
{
    class Le_Storage;
}
}




namespace ary
{
namespace loc
{


/** Provides access to files and directories stored in the
    repository.
*/
class LocationAdmin : public LocationPilot
{
  public:
                        LocationAdmin();
    virtual              ~LocationAdmin();

    // INHERITED
        // Interface LocationPilot:
    virtual Root &      CheckIn_Root(
                            const csv::ploc::Path &
                                                i_rPath );
    virtual File &      CheckIn_File(
                            const String  &     i_name,
                            const csv::ploc::DirectoryChain &
                                                i_subPath,
                            Le_id               i_root );

    virtual Root &      Find_Root(
                            Le_id               i_id ) const;
    virtual Directory & Find_Directory(
                            Le_id               i_id ) const;
    virtual File &      Find_File(
                            Le_id               i_id ) const;
  private:
    // Locals
    Le_Storage &        Storage() const;
    Directory &         CheckIn_Directory(
                            Directory &         io_parent,
                            const String &      i_name );
    Directory &         CheckIn_Directories(
                            Directory &         io_root,
                            StringVector::const_iterator
                                                i_beginSubPath,
                            StringVector::const_iterator
                                                i_endSubPath );
    // DATA
    Dyn<Le_Storage>     pStorage;
};




}   // namespace loc
}   // namespace ary
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

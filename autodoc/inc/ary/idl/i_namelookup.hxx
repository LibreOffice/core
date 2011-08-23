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

#ifndef ARY_IDL_I_NAMELOOKUP_HXX
#define ARY_IDL_I_NAMELOOKUP_HXX

// BASE CLASSES
#include <ary/idl/i_types4idl.hxx>

// USED SERVICES
#include <ary/stdconstiter.hxx>
#include <ary/itrange.hxx>
#include <vector>
#include <map>




namespace ary
{
namespace idl
{


/** This class finds all occurrences in the current language of a
    name in the repository.
*/
class NameLookup
{
  public:
    struct NameProperties
    {
                            NameProperties()
                                :   nId(0),
                                    nClass(0),
                                    nOwner(0) {}
                            NameProperties(
                                Ce_id               i_id,
                                ClassId             i_class,
                                Ce_id               i_owner )
                                :   nId(i_id),
                                    nClass(i_class),
                                    nOwner(i_owner) {}
        Ce_id               nId;
        ClassId             nClass;
        Ce_id               nOwner;
    };

    /// Map from Name to NameProperties.
    typedef std::multimap<String, NameProperties>   Map_Names;

    // LIFECYCLE
                        NameLookup();
                        ~NameLookup();
    // OPERATIONS
    void                Add_Name(
                            const String &      i_name,
                            Ce_id               i_id,
                            ClassId             i_class,
                            Ce_id               i_owner );
  private:
    // DATA
    Map_Names           aNames;
};




}   // namespace idl
}   // namespace ary
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

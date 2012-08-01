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

#ifndef ARY_IDL_I_NAMELOOKUP_HXX
#define ARY_IDL_I_NAMELOOKUP_HXX

// BASE CLASSES
#include <ary/idl/i_types4idl.hxx>

// USED SERVICES
#include <ary/stdconstiter.hxx>
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

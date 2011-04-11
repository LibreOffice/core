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

#ifndef ARY_LOC_LE_HXX
#define ARY_LOC_LE_HXX

// USED SERVICES
    // BASE CLASSES
#include <ary/entity.hxx>
    // COMPONENTS
    // PARAMETERS
#include <ary/loc/loc_types4loc.hxx>
#include <ary/loc/loc_traits.hxx>



namespace ary
{
namespace loc
{


/** Base class for all file locations in the Autodoc repository.
*/
class LocationEntity : public ::ary::Entity
{
  public:
    typedef Le_Traits       traits_t;

    virtual             ~LocationEntity() {}

    Le_id               LeId() const;
    const String &      LocalName() const;
    Le_id               ParentDirectory() const;

  private:
    virtual const String &
                        inq_LocalName() const = 0;
    virtual Le_id       inq_ParentDirectory() const = 0;
};




// IMPLEMENTATION
inline Le_id
LocationEntity::LeId() const
{
    return TypedId<LocationEntity>(Id());
}

inline const String &
LocationEntity::LocalName() const
{
    return inq_LocalName();
}

inline Le_id
LocationEntity::ParentDirectory() const
{
    return inq_ParentDirectory();
}





} // namespace loc
} // namespace ary
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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

#ifndef ARY_CESSENTL_HXX
#define ARY_CESSENTL_HXX
//  KORR_DEPRECATED_3.0


// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
    // PARAMETERS
#include <ary/cpp/c_types4cpp.hxx>
#include <ary/loc/loc_types4loc.hxx>



namespace ary
{
namespace cpp
{


class CeEssentials
{	// Non inline functions are implemented in ceworker.cxx .
  public:
    // LIFECYCLE
                        CeEssentials();
                        CeEssentials(
                            const String  &     i_sLocalName,
                            Ce_id               i_nOwner,
                            loc::Le_id          i_nLocation );
                        ~CeEssentials();
    // INQUIRY
    const String  &     LocalName() const;
    Ce_id     		    Owner() const;
    loc::Le_id          Location() const;

    // ACCESS
  private:
    String              sLocalName;
    Ce_id			    nOwner;
    loc::Le_id          nLocation;
};



// IMPLEMENTATION
inline const String 	&
CeEssentials::LocalName() const
    { return sLocalName; }
inline Ce_id
CeEssentials::Owner() const
    { return nOwner; }
inline loc::Le_id
CeEssentials::Location() const
    { return nLocation; }




}   // namespace cpp
}   // namespace ary
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

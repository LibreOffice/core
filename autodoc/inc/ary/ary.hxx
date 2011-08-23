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

#ifndef ARY_ARY_HXX
#define ARY_ARY_HXX


// USED SERVICES
    // BASE CLASSES
    // OTHER

namespace ary
{
namespace idl
{
    class Gate;
}

namespace cpp
{
    class Gate;
}
}



namespace ary
{

/** Starting point for all work with the
    Autodoc Sourcecode Repository.

    Create and destroy the repository and
    give access to the "Gates" for different tasks.

    @collab ::ary::cpp::Gate
    @collab ::ary::idl::Gate
*/

class Repository
{
  public:
    //  LIFECYCLE
    virtual             ~Repository() {}
    static DYN Repository &
                        Create_();
    // INQUIRY
    virtual const String &
                        Title() const = 0;
    virtual const ::ary::cpp::Gate &
                        Gate_Cpp() const = 0;
    virtual const ::ary::idl::Gate &
                        Gate_Idl() const = 0;
    // ACCESS
    virtual ::ary::cpp::Gate &
                        Gate_Cpp() = 0;
    virtual ::ary::idl::Gate &
                        Gate_Idl() = 0;
    virtual void        Set_Title(
                            const String &      i_sName ) = 0;
};



} // namespace ary
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

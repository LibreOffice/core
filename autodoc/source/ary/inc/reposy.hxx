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

#ifndef ARY_REPOSY_HXX
#define ARY_REPOSY_HXX

// BASE CLASSES
#include <ary/ary.hxx>
// USED SERVICES
#include <cosv/ploc_dir.hxx>

namespace ary
{
namespace cpp
{
    class InternalGate;
}
namespace idl
{
    class InternalGate;
}
}   // namespace ary




namespace ary
{


/** Implements ::ary::Repository.

    @see Repository
*/

class RepositoryCenter : public ::ary::Repository
{
  public:
    //  LIFECYCLE
                        RepositoryCenter();
    virtual             ~RepositoryCenter();

    // INHERITED
        // Interface Repository:
    virtual const cpp::Gate &   Gate_Cpp() const;
    virtual const idl::Gate &   Gate_Idl() const;
    virtual const String &      Title() const;
    virtual cpp::Gate &         Gate_Cpp();
    virtual idl::Gate &         Gate_Idl();
    virtual void                Set_Title(const String & i_sName );

  private:
    // DATA
    String              sDisplayedName;     /// Name to be displayed for human users.
    csv::ploc::Directory
                        aLocation;
    Dyn<cpp::InternalGate>
                        pCppPartition;
    Dyn<idl::InternalGate>
                        pIdlPartition;
};




}   // namespace ary
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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

#ifndef ARY_IDL_I_REPOSYPART_HXX
#define ARY_IDL_I_REPOSYPART_HXX



// USED SERVICES
    // BASE CLASSES
#include <idl_internalgate.hxx>
    // OTHER


namespace ary
{
namespace idl
{
    class CeAdmin;
    class TypeAdmin;
    class NameLookup;
}
}





namespace ary
{
namespace idl
{



/** The idl part of the Autodoc repository.
*/
class RepositoryPartition : public InternalGate
{
  public:
    // LIFECYCLE
                        RepositoryPartition(
                            RepositoryCenter &  i_repository );
                        ~RepositoryPartition();
    // INHERITED
        // Interface Gate:
    virtual void        Calculate_AllSecondaryInformation(
                            const String &      i_devman_reffilepath );
//                            const ::autodoc::Options &
//                                                i_options );
    virtual const CePilot &
                        Ces() const;
    virtual const TypePilot &
                        Types() const;
    virtual CePilot &   Ces();
    virtual TypePilot & Types();

  private:
    // DATA
    RepositoryCenter *  pCenter;

    Dyn<CeAdmin>        pCes;
    Dyn<TypeAdmin>      pTypes;
    Dyn<NameLookup>     pNamesDictionary;
};




}   //  namespace idl
}   //  namespace ary
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

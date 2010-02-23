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

#ifndef ARY_IDL_I_GATE_HXX
#define ARY_IDL_I_GATE_HXX

// USED SERVICES

namespace autodoc
{
    class Options;
}
namespace ary
{
namespace idl
{
    class CePilot;
    class TypePilot;
}
}




namespace ary
{
namespace idl
{


/** Main entry to access the IDL parts of the repository.
*/
class Gate
{
  public:
    // LIFECYCLE
    virtual             ~Gate() {}

    // OPERATIONS
    virtual void        Calculate_AllSecondaryInformation(
                            const String &      i_devman_reffilepath ) = 0;
//                            const ::autodoc::Options &
//                                                i_options ) = 0;
    // INQUIRY
    virtual const CePilot &
                        Ces() const = 0;
    virtual const TypePilot &
                        Types() const = 0;
    // ACCESS
    virtual CePilot &   Ces() = 0;
    virtual TypePilot & Types() = 0;
};




}   // namespace idl
}   // namespace ary
#endif

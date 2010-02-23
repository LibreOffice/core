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

#ifndef ARY_CPP_CP_DEF_HXX
#define ARY_CPP_CP_DEF_HXX


// USED SERVICES
    // BASE CLASSES
    // OTHER
#include <ary/cpp/c_types4cpp.hxx>

namespace ary
{
namespace cpp
{
    class Define;
    class InputContext;
    class Macro;
}
}




namespace ary
{
namespace cpp
{



/** Acess to C++ defines and macros in the repository.
*/
class DefPilot
{
  public:
    typedef DefsList::const_iterator                DefsIterator;



    // LIFECYCLE
    virtual             ~DefPilot() {}


    // OPERATIONS
    virtual Define &    Store_Define(
                            const InputContext& i_rContext,
                            const String  &     i_sName,
                            const StringVector &
                                                i_rDefinition ) = 0;
    virtual Macro &     Store_Macro(
                            const InputContext& i_rContext,
                            const String  &     i_sName,
                            const StringVector &
                                                i_rParams,
                            const StringVector &
                                                i_rDefinition ) = 0;
    // INQUIRY
    virtual const DefineEntity &
                        Find_Def(
                            De_id               i_id ) const = 0;
    virtual DefsResultList
                        AllDefines() const = 0;
    virtual DefsResultList
                        AllMacros() const = 0;
};





}   // namespace cpp
}   // namespace ary
#endif

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

#ifndef ARY_IDL_IK_MODULE_HXX
#define ARY_IDL_IK_MODULE_HXX



// USED SERVICES
    // BASE CLASSES
#include <ary/idl/ik_ce.hxx>
    // COMPONENTS
    // PARAMETERS


namespace ary
{
namespace idl
{

class CePilot;

namespace ifc_module
{

using ifc_ce::Dyn_CeIterator;
using ifc_ce::DocText;


struct attr : public ifc_ce::attr
{
    // KORR_FUTURE
    //   This has to be changed that way, that the differencing takes place
    //   within hfi_module.cxx and not here.
    //   So the class CePilot is not needed here, etc.
    //   Too much scope pollution.
    static void         Get_AllChildrenSeparated(
                            std::vector< const CodeEntity* > & o_nestedModules,
                            std::vector< const CodeEntity* > & o_services,
                            std::vector< const CodeEntity* > & o_interfaces,
                            std::vector< const CodeEntity* > & o_structs,
                            std::vector< const CodeEntity* > & o_exceptions,
                            std::vector< const CodeEntity* > & o_enums,
                            std::vector< const CodeEntity* > & o_typedefs,
                            std::vector< const CodeEntity* > & o_constantGroups,
                            std::vector< const CodeEntity* > & o_singletons,
                            const CePilot &     i_pilot,
                            const CodeEntity &  i_ce );

    static void         Get_SubModules(
                            Dyn_CeIterator &    o_result,
                            const CodeEntity &  i_ce );
    static void         Get_Services(
                            Dyn_CeIterator &    o_result,
                            const CodeEntity &  i_ce );
    static void         Get_Interfaces(
                            Dyn_CeIterator &    o_result,
                            const CodeEntity &  i_ce );
    static void         Get_Structs(
                            Dyn_CeIterator &    o_result,
                            const CodeEntity &  i_ce );
    static void         Get_Exceptions(
                            Dyn_CeIterator &    o_result,
                            const CodeEntity &  i_ce );
    static void         Get_Enums(
                            Dyn_CeIterator &    o_result,
                            const CodeEntity &  i_ce );
    static void         Get_Typedefs(
                            Dyn_CeIterator &    o_result,
                            const CodeEntity &  i_ce );
    static void         Get_ConstantsGroups(
                            Dyn_CeIterator &    o_result,
                            const CodeEntity &  i_ce );
};

struct xref : public ifc_ce::xref
{
};

struct doc : public ifc_ce::doc
{
};

}   // namespace ifc_module

}   // namespace idl
}   // namespace ary


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

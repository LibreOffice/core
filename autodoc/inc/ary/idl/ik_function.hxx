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

#ifndef ARY_IDL_IK_SERVICE_HXX
#define ARY_IDL_IK_SERVICE_HXX



// USED SERVICES
    // BASE CLASSES
#include <ary/idl/i_param.hxx>
#include <ary/idl/ik_ce.hxx>
    // COMPONENTS
    // PARAMETERS


namespace ary
{
namespace idl
{



namespace ifc_function
{

using ::ary::idl::ifc_ce::Dyn_CeIterator;
using ::ary::idl::ifc_ce::Dyn_TypeIterator;
using ::ary::idl::ifc_ce::DocText;


struct attr: public ifc_ce::attr
{
    static Type_id      ReturnType(
                            const CodeEntity &  i_ce );
    static bool         IsOneway(
                            const CodeEntity &  i_ce );
    static bool         HasEllipse(
                            const CodeEntity &  i_ce );
    static void         Get_Parameters(
                            Dyn_StdConstIterator<ary::idl::Parameter> &
                                                o_result,
                            const CodeEntity &  i_ce );
    static void         Get_Exceptions(
                            Dyn_TypeIterator &  o_result,
                            const CodeEntity &  i_ce );
};

struct xref : public ifc_ce::xref
{
};

struct doc : public ifc_ce::doc
{
//  aStateMachine.AddToken( "@param",   nTok_at_param,       A_nAtTagDefStatus, finAtTag );
//  aStateMachine.AddToken( "@throws",  nTok_at_throws,      A_nAtTagDefStatus, finAtTag );
//  aStateMachine.AddToken( "@exception",
//  aStateMachine.AddToken( "@return",  nTok_at_return,      A_nAtTagDefStatus, finAtTag );
//  aStateMachine.AddToken( "@returns", nTok_at_return,      A_nAtTagDefStatus, finAtTag );
};


}   // namespace ifc_function



}   // namespace idl
}   // namespace ary


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

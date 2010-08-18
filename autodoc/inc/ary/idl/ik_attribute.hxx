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

#ifndef ARY_IDL_IK_ATTRIBUTE_HXX
#define ARY_IDL_IK_ATTRIBUTE_HXX
//  KORR_DEPRECATED_3.0


// USED SERVICES
    // BASE CLASSES
#include <ary/idl/ik_ce.hxx>
    // COMPONENTS
    // PARAMETERS


namespace ary
{
namespace idl
{

namespace ifc_attribute
{

using ifc_ce::DocText;
using ::ary::idl::ifc_ce::Dyn_TypeIterator;


struct attr: public ifc_ce::attr
{
    static bool         HasAnyStereotype(
                            const CodeEntity &  i_ce );
    static bool         IsReadOnly(
                            const CodeEntity &  i_ce );
    static bool         IsBound(
                            const CodeEntity &  i_ce );
    static Type_id      Type(
                            const CodeEntity &  i_ce );
    static void         Get_GetExceptions(
                            Dyn_TypeIterator &  o_result,
                            const CodeEntity &  i_ce );
    static void         Get_SetExceptions(
                            Dyn_TypeIterator &  o_result,
                            const CodeEntity &  i_ce );
};

struct xref : public ifc_ce::xref
{
};

struct doc : public ifc_ce::doc
{
};


}   // namespace ifc_attribute

}   // namespace idl
}   // namespace ary


#endif

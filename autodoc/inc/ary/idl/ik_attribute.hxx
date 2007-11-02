/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ik_attribute.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 15:13:03 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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

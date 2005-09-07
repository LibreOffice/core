/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ik_property.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 16:16:35 $
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

#ifndef ARY_IDL_IK_PROPERTY_HXX
#define ARY_IDL_IK_PROPERTY_HXX


// USED SERVICES
    // BASE CLASSES
#include <ary/idl/ik_ce.hxx>
    // COMPONENTS
    // PARAMETERS


namespace ary
{
namespace idl
{

namespace ifc_property
{

using ifc_ce::DocText;


struct attr: public ifc_ce::attr
{
    static bool         HasAnyStereotype(
                            const CodeEntity &  i_ce );
    static bool         IsReadOnly(
                            const CodeEntity &  i_ce );
    static bool         IsBound(
                            const CodeEntity &  i_ce );
    static bool         IsConstrained(
                            const CodeEntity &  i_ce );
    static bool         IsMayBeAmbiguous(
                            const CodeEntity &  i_ce );
    static bool         IsMayBeDefault(
                            const CodeEntity &  i_ce );
    static bool         IsMayBeVoid(
                            const CodeEntity &  i_ce );
    static bool         IsRemovable(
                            const CodeEntity &  i_ce );
    static bool         IsTransient(
                            const CodeEntity &  i_ce );
    static Type_id      Type(
                            const CodeEntity &  i_ce );
};

struct xref : public ifc_ce::xref
{
};

struct doc : public ifc_ce::doc
{
};


}   // namespace ifc_property

}   // namespace idl
}   // namespace ary


#endif

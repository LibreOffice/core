/*************************************************************************
 *
 *  $RCSfile: ik_module.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: np $ $Date: 2002-11-01 17:11:59 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
    // KORR
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

/*************************************************************************
 *
 *  $RCSfile: ik_function.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-12 15:08:57 $
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


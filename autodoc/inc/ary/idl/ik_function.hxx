/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ik_function.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 16:15:43 $
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


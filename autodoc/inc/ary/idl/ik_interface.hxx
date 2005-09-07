/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ik_interface.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 16:15:59 $
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

#ifndef ARY_IDL_IK_INTERFACE_HXX
#define ARY_IDL_IK_INTERFACE_HXX


// USED SERVICES
    // BASE CLASSES
#include <ary/idl/ik_ce.hxx>
    // COMPONENTS
    // PARAMETERS


namespace ary
{
namespace idl
{
class CommentedRelation;

namespace ifc_interface
{

using ifc_ce::Dyn_CeIterator;
using ifc_ce::DocText;


struct attr: public ifc_ce::attr
{
    static void         Get_Bases(
                            Dyn_StdConstIterator<CommentedRelation> &
                                                o_result,
                            const CodeEntity &  i_ce );
    static void         Get_Functions(
                            Dyn_CeIterator &    o_result,
                            const CodeEntity &  i_ce );
    static void         Get_Attributes(
                            Dyn_CeIterator &    o_result,
                            const CodeEntity &  i_ce );
};

struct xref : public ifc_ce::xref
{
    static void         Get_Derivations(
                            Dyn_CeIterator &    o_result,
                            const CodeEntity &  i_ce );
    static void         Get_SynonymTypedefs(     /// like: typedef i_ce.LocalName() newName;
                            Dyn_CeIterator &    o_result,
                            const CodeEntity &  i_ce );
    static void         Get_ExportingServices(
                            Dyn_CeIterator &    o_result,
                            const CodeEntity &  i_ce );
    static void         Get_ExportingSingletons(
                            Dyn_CeIterator &    o_result,
                            const CodeEntity &  i_ce );
    static void         Get_AsReturns(
                            Dyn_CeIterator &    o_result,
                            const CodeEntity &  i_ce );
    static void         Get_AsParameters(
                            Dyn_CeIterator &    o_result,
                            const CodeEntity &  i_ce );
    static void         Get_AsDataTypes(
                            Dyn_CeIterator &    o_result,
                            const CodeEntity &  i_ce );

#if 0
    static void         Get_UsingTypedefs(      /// like: typedef sequence<i_ce.LocalName()> newNameSeq;
                            Dyn_CeIterator &    o_result,
                            const CodeEntity &  i_ce );
    static void         Get_AsIndirectReturns(
                            Dyn_CeIterator &    o_result,
                            const CodeEntity &  i_ce );
    static void         Get_AsIndirectParameters(
                            Dyn_CeIterator &    o_result,
                            const CodeEntity &  i_ce );
#endif // 0
};

struct doc : public ifc_ce::doc
{
};

}   // namespace ifc_interface

}   // namespace idl
}   // namespace ary

#endif

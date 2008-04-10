/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: d_namedlist.hxx,v $
 * $Revision: 1.3 $
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

#ifndef ARY_DOC_D_NAMEDLIST_HXX
#define ARY_DOC_D_NAMEDLIST_HXX

// BASE CLASSES
#include <ary/doc/d_node.hxx>

// USED SERVICES




namespace ary
{
namespace doc
{


/** Represents a list of documentation elements of the same type, which
    are accessible by their names as a key.

    Actually it is not necessary (by the interface of the class) that
    all elements have the same type, but that is its intended purpose
    and other uses should be carefully reconsidered.

    @example
    Elements could be for example a list of base classes or parameters
    which are just a list on one hand, but may be accessed via the name
    of the base class/parameter as well.

*/
class NamedList : public Node
{
  public:
    enum E_HandleDuplicateNames
    {
        hdn_normal = 0,
        hdn_put_together,
        hdn_keep_only_first
    };

    //  LIFECYCLE
    explicit            NamedList(
                            nodetype::id    i_id );
    virtual             ~NamedList();

    //  OPERATORS
    const Node *        operator[](
                            const String &  i_name ) const;
    //  OPERATIONS
    /** When Node name already exists, the new ->Node is discarded.
    */
    void                Add_Node(
                            const String &  i_name,
                            DYN Node &      pass_Element,
                            E_HandleDuplicateNames
                                            eHandleDuplicateNames = hdn_normal );
  private:
    // Interface csv::ConstProcessorClient:
    virtual void        do_Accept(
                            csv::ProcessorIfc & io_processor ) const;
    // Locals
    void                AddNode_normal(
                            const String &  i_name,
                            DYN Node &      pass_Element );
    void                AddNode_put_together(
                            const String &  i_name,
                            DYN Node &      pass_Element );
    void                AddNode_keep_only_first(
                            const String &  i_name,
                            DYN Node &      pass_Element );

    // DATA
    /** aNames is built exactly parallel to ->aElements. Each
        element of aNames contains the key to the corresponding
        ->Node in ->aElements.
    */
    StringVector        aNames;
    NodeList            aElements;
};




}   // doc
}   // ary
#endif

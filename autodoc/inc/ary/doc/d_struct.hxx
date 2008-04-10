/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: d_struct.hxx,v $
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

#ifndef ARY_DOC_D_STRUCT_HXX
#define ARY_DOC_D_STRUCT_HXX

// USED SERVICES
    // BASE CLASSES
#include <ary/doc/d_node.hxx>




namespace ary
{
namespace doc
{


/** Represents a documentation structure whose elements
    are accessible by a slot id.

    @attention
    Maximum possible number of slots is 254.
*/
class Struct : public Node
{
  public:
    typedef nodetype::id                slot_id;

    // LIFECYCLE
    explicit            Struct(
                            nodetype::id        i_type );
    virtual             ~Struct();

    // OPERATIONS
    /** Sorts the new node into the right slot.

        @return pass_node
    */
    Node &              Add_Node(
                            DYN Node &          pass_node );

    // INQUIRY
    const Node *        Slot(
                            slot_id             i_slot ) const;
    // ACCESS
    Node *              Slot(
                            slot_id             i_slot );
  private:
    // Interface csv::ConstProcessorClient:
    virtual void        do_Accept(
                            csv::ProcessorIfc & io_processor ) const;
    // DATA
    NodeList            aElements;
};




}   // namespace doc
}   // namespace ary
#endif

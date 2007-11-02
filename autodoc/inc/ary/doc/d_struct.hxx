/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: d_struct.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 15:00:40 $
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

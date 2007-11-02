/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: d_boolean.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 14:58:07 $
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

#ifndef ARY_DOC_D_BOOLEAN_HXX
#define ARY_DOC_D_BOOLEAN_HXX

// BASE CLASSES
#include <ary/doc/d_node.hxx>

// USED SERVICES




namespace ary
{
namespace doc
{


/** Repesents a boolean documentation item like "optional" or "not optional".
*/
class Boolean : public Node
{
  public:
    // LIFECYCLE
    explicit            Boolean(
                            nodetype::id        i_type );
    virtual             ~Boolean();

    // OPERATIONS
    void                Set(
                            bool                i_b );
    // INQUIRY
    bool                IsTrue() const;

  private:
    // Interface csv::ConstProcessorClient:
    virtual void        do_Accept(
                            csv::ProcessorIfc & io_processor ) const;
    // DATA
    bool                b;
};




// IMPLEMENTATION
inline
Boolean::Boolean(nodetype::id i_type)
    :   Node(i_type),
        b(false)
{
}

inline bool
Boolean::IsTrue() const
{
    return b;
}

inline void
Boolean::Set( bool i_b )
{
    b = i_b;
}



}   // namespace doc
}   // namespace ary
#endif

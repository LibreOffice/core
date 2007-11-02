/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: d_unit.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 15:01:35 $
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

#ifndef ARY_DOC_D_UNIT_HXX
#define ARY_DOC_D_UNIT_HXX

// BASE CLASSES
#include <ary/doc/d_node.hxx>

// USED SERVICES
#include <ary/doc/d_hypertext.hxx>




namespace ary
{
namespace doc
{


/** A documentation with a title and a text.

    @example
    ReturnValue
        The number of items counted.
*/
class Unit : public Node
{
  public:

    // LIFECYCLE
    explicit            Unit(
                            nodetype::id        i_id );
    virtual             ~Unit();

    // INQUIRY
    const HyperText &   Doc() const;

    // ACESS
    HyperText &         Doc();

  private:
    // Interface csv::ConstProcessorClient:
    virtual void        do_Accept(
                            csv::ProcessorIfc & io_processor ) const;
    // DATA
    HyperText           aDoc;
};




// IMPLEMENTATION
inline const HyperText &
Unit::Doc() const
{
    return aDoc;
}

inline HyperText &
Unit::Doc()
{
    return aDoc;
}




}   // namespace doc
}   // namespace ary
#endif

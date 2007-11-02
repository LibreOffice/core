/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: d_summary.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 15:01:01 $
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

#ifndef ARY_DOC_D_SUMMARY_HXX
#define ARY_DOC_D_SUMMARY_HXX

// BASE CLASSES
#include <ary/doc/d_node.hxx>
// USED SERVICES
#include <ary/doc/d_hypertext.hxx>




namespace ary
{
namespace doc
{


/** The summary of a documentation block.

    The summary can be:
     - the first paragraph of a description (standard),
       which should be contained within 2 lines,
     - else the first three lines of the description followed by a "(...)".

    If there is no description, first the @return unit will be chosen, if
    both are not there, the @resp unit, else the first unit of the docu.

    With an explicit @summary unit the summary can be forced to be longer than
    3 lines.
*/
class Summary : public Node
{
  public:
    // LIFECYCLE
                        Summary(
                            nodetype::id        i_slot );
    virtual             ~Summary();

    // INQUIRY
    const HyperText &   Text() const;
    bool                IsComplete() const;
    nodetype::id        Origin() const;

    // ACESS
    void                Set(
                            const HyperText &   i_text,
                            bool                i_isIncomplete,
                            nodetype::id        i_origin );
  private:
    // Interface csv::ConstProcessorClient:
    virtual void        do_Accept(
                            csv::ProcessorIfc & io_processor ) const;
    // DATA
    const HyperText *   pText;
    bool                bIncomplete;
    nodetype::id        nOrigin;        /// For example from: description, @return, @responsibility.
};




// IMPLEMENTATION
inline bool
Summary::IsComplete() const
{
    return bIncomplete;
}

inline nodetype::id
Summary::Origin() const
{
    return nOrigin;
}




}   // namespace doc
}   // namespace ary
#endif

/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: d_summary.hxx,v $
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

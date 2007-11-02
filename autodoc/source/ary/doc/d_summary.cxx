/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: d_summary.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 15:38:51 $
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

#include <precomp.h>
#include <ary/doc/d_summary.hxx>


// NOT FULLY DEFINED SERVICES



namespace ary
{
namespace doc
{



Summary::Summary(nodetype::id i_slot)
    :   Node(i_slot),
        pText(0),
        bIncomplete(false),
        nOrigin(0)
{
}

Summary::~Summary()
{
}

const HyperText &
Summary::Text() const
{
    static const HyperText  aTextNull_;
    return pText
                ?   *pText
                :   aTextNull_;
}


void
Summary::Set( const HyperText &   i_text,
              bool                i_isIncomplete,
              nodetype::id        i_origin )
{
    pText = &i_text;
    bIncomplete = i_isIncomplete;
    nOrigin = i_origin;
}

void
Summary::do_Accept(csv::ProcessorIfc & io_processor ) const
{
    csv::CheckedCall(io_processor,*this);
}




}   // namespace doc
}   // namespace ary

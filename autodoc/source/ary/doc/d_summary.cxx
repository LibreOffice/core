/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: d_summary.cxx,v $
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

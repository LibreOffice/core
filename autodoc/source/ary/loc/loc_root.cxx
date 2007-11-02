/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: loc_root.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 16:16:53 $
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
#include <ary/loc/loc_root.hxx>


// NOT FULLY DEFINED SERVICES


namespace ary
{
namespace loc
{


Root::Root(const csv::ploc::Path & i_path)
    :   aPath(i_path),
        sPathAsString(),
        aMyDirectory(0)
{
    StreamLock
        path_string(700);
    path_string() << i_path;
    sPathAsString = path_string().c_str();
}

Root::~Root()
{
}

void
Root::do_Accept(csv::ProcessorIfc & io_processor) const
{
    csv::CheckedCall(io_processor,*this);
}

ClassId
Root::get_AryClass() const
{
    return class_id;
}

const String &
Root::inq_LocalName() const
{
    return sPathAsString;
}

Le_id
Root::inq_ParentDirectory() const
{
    return Le_id::Null_();
}



} // namespace loc
} // namespace ary

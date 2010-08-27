/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

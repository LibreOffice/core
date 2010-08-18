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
#include <ary/info/all_dts.hxx>


// NOT FULLY DEFINED SERVICES
#include <ary/info/infodisp.hxx>


namespace ary
{
namespace info
{


void
DT_Text::do_StoreAt( DocuDisplay & o_rDisplay ) const
{
    o_rDisplay.Display_DT_Text(*this);
}

bool
DT_Text::inq_IsWhite() const
{
     return false;
}

void
DT_MaybeLink::do_StoreAt( DocuDisplay & o_rDisplay ) const
{
    o_rDisplay.Display_DT_MaybeLink(*this);
}

bool
DT_MaybeLink::inq_IsWhite() const
{
     return false;
}

void
DT_Whitespace::do_StoreAt( DocuDisplay & o_rDisplay ) const
{
    o_rDisplay.Display_DT_Whitespace(*this);
}

bool
DT_Whitespace::inq_IsWhite() const
{
     return true;
}

void
DT_Eol::do_StoreAt( DocuDisplay & o_rDisplay ) const
{
    o_rDisplay.Display_DT_Eol(*this);
}

bool
DT_Eol::inq_IsWhite() const
{
     return true;
}

void
DT_Xml::do_StoreAt( DocuDisplay & o_rDisplay ) const
{
    o_rDisplay.Display_DT_Xml(*this);
}

bool
DT_Xml::inq_IsWhite() const
{
     return false;
}


}   // namespace info
}   // namespace ary


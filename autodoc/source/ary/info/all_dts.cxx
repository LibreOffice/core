/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: all_dts.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: vg $ $Date: 2007-09-18 13:40:13 $
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


/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: it_builtin.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-16 16:28:17 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_autodoc.hxx"


#include <precomp.h>
#include "it_builtin.hxx"


// NOT FULLY DEFINED SERVICES
#include "i_strconst.hxx"



namespace ary
{
namespace idl
{



BuiltInType::BuiltInType( const char * i_sName )
    :   Named_Type(i_sName)
{
}

BuiltInType::~BuiltInType()
{
}

void
BuiltInType::do_Visit( Host & ) const
{
    // yet unused.
}

RCid
BuiltInType::inq_ClassId() const
{
    return class_id;
}

void
BuiltInType::inq_Get_Text( StringVector &      ,
                           String &            o_name,
                           Ce_id &             ,
                           int &               ,
                           const Gate &        ) const
{
    o_name = Name();
}



}   // namespace idl
}   // namespace ary


/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dht_link2param.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 15:40:46 $
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
#include <ary/doc/ht/dht_link2param.hxx>


// NOT FULLY DEFINED SERVICES
#include <ary/doc/ht/dht_component.hxx>
#include <ary/doc/ht/dht_processor.hxx>
#include "dht_helper.hxx"


namespace ary
{
namespace doc
{
namespace ht
{

const char  C_cSeparator = '#';



Link2Param::~Link2Param()
{
}

Component
Link2Param::Create_Component_( const String &   i_parameterName,
                               const String &   i_display )
{
    static const Link2Param
        aTheInstance_;
    return Create_Component(
                aTheInstance_,
                i_parameterName,
                C_cSeparator,
                i_display );
}

void
Link2Param::do_Accept( Processor &         io_processor,
                       const String &      i_data ) const
{
    io_processor.Process(*this, i_data);
}



}   // namespace ht
}   // namespace doc
}   // namespace ary

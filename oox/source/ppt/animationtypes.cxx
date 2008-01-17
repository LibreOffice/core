/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: animationtypes.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-17 08:06:00 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2007 by Sun Microsystems, Inc.
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


#include "animationtypes.hxx"

#include <com/sun/star/animations/Timing.hpp>

#include "oox/helper/attributelist.hxx"

#include "tokens.hxx"

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::animations;
using namespace ::com::sun::star::xml::sax;

namespace oox { namespace ppt {

// ST_TLTime
Any GetTime(  const ::rtl::OUString & val )
{
    Any aDuration;
    if( val.compareToAscii( "indefinite" ) == 0 )
    {
        aDuration <<= Timing_INDEFINITE;
    }
    else
    {
        aDuration <<= val.toFloat() / 1000.0;
    }
    return aDuration;
}


// ST_TLTimeAnimateValueTime
Any GetTimeAnimateValueTime( const ::rtl::OUString & val )
{
    Any aPercent;
    if( val.compareToAscii( "indefinite" ) == 0 )
    {
        aPercent <<= Timing_INDEFINITE;
    }
    else
    {
        aPercent <<= val.toFloat() / 100000.0;
    }
    return aPercent;
}

} }

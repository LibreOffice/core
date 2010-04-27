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

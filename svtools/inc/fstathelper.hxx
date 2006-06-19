/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fstathelper.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-19 20:18:13 $
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

#ifndef _SVTOOLS_FSTATHELPER_HXX
#define _SVTOOLS_FSTATHELPER_HXX

#ifndef INCLUDED_SVLDLLAPI_H
#include "svtools/svldllapi.h"
#endif

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif

class UniString;
class Date;
class Time;

namespace FStatHelper {

/** Return the modified time and date stamp for this URL.

    @param URL  the asking URL

    @param pDate if unequal 0, the function set the date stamp

    @param pTime if unequal 0, the function set the time stamp

    @return     it was be able to get the date/time stamp
*/
SVL_DLLPUBLIC sal_Bool GetModifiedDateTimeOfFile( const UniString& rURL,
                                    Date* pDate, Time* pTime );

/** Return if under the URL a document exist. This is only a wrapper for the
    UCB.IsContent.
*/
SVL_DLLPUBLIC sal_Bool IsDocument( const UniString& rURL );

/** Return if under the URL a folder exist. This is only a wrapper for the
    UCB.isFolder.
*/
SVL_DLLPUBLIC sal_Bool IsFolder( const UniString& rURL );

}

#endif

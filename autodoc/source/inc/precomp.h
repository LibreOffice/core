/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: precomp.h,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 18:09:56 $
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
#ifndef __ADC_PRECOMP_H_06071998__
#define __ADC_PRECOMP_H_06071998__


// For en/disabling csv_assertions:
#ifndef DEBUG
#define CSV_NO_ASSERTIONS
#endif

#ifdef WNT
#pragma warning( disable : 4786 )
#endif

#include <cosv/csv_precomp.h>

#include <vector>
#include <map>
#include <set>



// Shortcuts to access csv::-types:
using csv::String;
using csv::StringVector;
using csv::StreamStr;
using csv::ios;
using csv::ostream;
using csv::c_str;
typedef csv::StreamStrLock  StreamLock;



// As long, as appearances of udmstri are not changed yet:
typedef String udmstri;


/** @attention
    Has to be changed to returning csv::Cout(),if
        1)  iostreams are not used ( #ifdef CSV_NO_IOSTREAM )
        2)  used for an GUI-application.
*/
inline ostream &
Cout() { return std::cout; }

/** @attention
    Has to be changed to returning csv::Cerr(),if
        1)  iostreams are not used ( #ifdef CSV_NO_IOSTREAM )
        2)  used for an GUI-application.
*/
inline ostream &
Cerr() { return std::cerr; }


inline csv::F_FLUSHING_FUNC
Endl()  { return csv::Endl; }
inline csv::F_FLUSHING_FUNC
Flush() { return csv::Flush; }


#endif



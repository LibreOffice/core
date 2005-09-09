/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: precomp.h,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 14:34:37 $
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
#if OSL_DEBUG_LEVEL == 0
#define CSV_NO_ASSERTIONS
#else
#define CSV_USE_CSV_ASSERTIONS
#endif

#ifdef NP_LOCALBUILD
#pragma warning( disable : 4786 )
#endif

#include <cosv/csv_precomp.h>

#include <vector>
#include <map>



// Shortcuts to access csv::-types:
using csv::String;
using csv::StringVector;
using csv::StreamStr;
using csv::ios;
using csv::ostream;
using csv::c_str;
typedef csv::StreamStrLock  StreamLock;



inline ostream &
Cout() { return csv::Cout(); }

inline ostream &
Cerr() { return csv::Cerr(); }


inline csv::F_FLUSHING_FUNC
Endl()  { return csv::Endl; }
inline csv::F_FLUSHING_FUNC
Flush() { return csv::Flush; }



#endif



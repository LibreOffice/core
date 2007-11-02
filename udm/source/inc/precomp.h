/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: precomp.h,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 17:32:48 $
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
#ifndef UDM_PRECOMP_H
#define UDM_PRECOMP_H

// For en/disabling csv_assertions:
#ifndef DEBUG
#define CSV_NO_ASSERTIONS
#endif


#include <cosv/csv_precomp.h>
#include <vector>
#include <map>



// Shortcuts to access csv::-types:
using csv::String;
using csv::StringVector;
using csv::StreamStr;
using csv::c_str;
typedef csv::StreamStrLock  StreamLock;



inline std::ostream &
Cout() { return std::cout; }

inline std::ostream &
Cerr() { return std::cerr; }


inline csv::F_FLUSHING_FUNC
Endl()  { return csv::Endl; }
inline csv::F_FLUSHING_FUNC
Flush() { return csv::Flush; }




#endif

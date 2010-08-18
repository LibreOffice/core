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
#ifndef ADC_PRECOMP_H_06071998
#define ADC_PRECOMP_H_06071998


// For en/disabling csv_assertions:
#ifndef DEBUG
#define CSV_NO_ASSERTIONS
#endif

#include <cosv/csv_precomp.h>

#include <vector>
#include <map>
#include <set>



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

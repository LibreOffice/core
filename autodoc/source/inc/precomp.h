/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

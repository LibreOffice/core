/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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

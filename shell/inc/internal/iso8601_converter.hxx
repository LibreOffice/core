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



#ifndef ISO8601_CONVERTER_HXX_INCLUDED
#define ISO8601_CONVERTER_HXX_INCLUDED

#include <string>

//-----------------------------------
/* Converts ISO 8601 conform date/time
   represenation to the representation
   conforming to the current locale
*/
std::wstring iso8601_date_to_local_date(const std::wstring& iso8601date);

//------------------------------------
/* Converts ISO 8601 conform duration
   representation to the representation
   conforming to the current locale
*/
std::wstring iso8601_duration_to_local_duration(const std::wstring& iso8601duration);

#endif

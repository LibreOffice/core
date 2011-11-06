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



#ifndef _XMLOFF_XMLEHELP_HXX
#define _XMLOFF_XMLEHELP_HXX

#include <tools/mapunit.hxx>

namespace rtl { class OUStringBuffer; }

class SvXMLExportHelper
{
public:
    static void AddLength( sal_Int32 nValue, MapUnit eValueUnit,
                           ::rtl::OUStringBuffer& rOut,
                           MapUnit eOutUnit );
    static void AddPercentage( sal_Int32 nVal, ::rtl::OUStringBuffer& rOut );
    static double GetConversionFactor(::rtl::OUStringBuffer& rUnit,
        const MapUnit eCoreUnit, const MapUnit eDestUnit);
    static MapUnit GetUnitFromString(const ::rtl::OUString& rString,
        MapUnit eDefaultUnit);
};



#endif  //  _XMLOFF_XMLEHELP_HXX


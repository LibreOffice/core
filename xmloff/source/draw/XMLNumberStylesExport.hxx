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



#ifndef _XMLOFF_NUMBERSTYLESEXPORT_HXX
#define _XMLOFF_NUMBERSTYLESEXPORT_HXX

#include <sal/types.h>
#include <rtl/ustring.hxx>

class SdXMLExport;

const sal_Int16 SdXMLDateFormatCount = 8;
const sal_Int16 SdXMLTimeFormatCount = 7;

class SdXMLNumberStylesExporter
{
public:
    static void exportTimeStyle( SdXMLExport& rExport, sal_Int32 nStyle );
    static void exportDateStyle( SdXMLExport& rExport, sal_Int32 nStyle );

    static sal_Int32 getDateStyleCount() { return SdXMLDateFormatCount; }
    static sal_Int32 getTimeStyleCount() { return SdXMLTimeFormatCount; }

    static rtl::OUString getTimeStyleName(const sal_Int32 nTimeFormat );
    static rtl::OUString getDateStyleName(const sal_Int32 nDateFormat );
};

#endif  //  _XMLOFF_NUMBERSTYLESEXPORT_HXX


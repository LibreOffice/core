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



#ifndef _XMLOFF_FORMENUMS_HXX_
#define _XMLOFF_FORMENUMS_HXX_

#include <xmloff/xmlement.hxx>

//.........................................................................
namespace xmloff
{
//.........................................................................

    //=====================================================================
    //= OEnumMapper
    //=====================================================================
    class OEnumMapper
    {
    public:
        enum EnumProperties
        {
            epSubmitEncoding = 0,
            epSubmitMethod,
            epCommandType,
            epNavigationType,
            epTabCyle,
            epButtonType,
            epListSourceType,
            epCheckState,
            epTextAlign,
            epBorderWidth,
            epFontEmphasis,
            epFontRelief,
            epListLinkageType,
            epOrientation,
            epVisualEffect,
            epImagePosition,
            epImageAlign,
            epImageScaleMode,

            KNOWN_ENUM_PROPERTIES
        };

    private:
        static const SvXMLEnumMapEntry* s_pEnumMap[KNOWN_ENUM_PROPERTIES];

    public:
        static const SvXMLEnumMapEntry* getEnumMap(EnumProperties _eProperty);
    };

//.........................................................................
}   // namespace xmloff
//.........................................................................

#endif // _XMLOFF_FORMENUMS_HXX_


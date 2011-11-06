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



#ifndef SC_STRINGUTIL_HXX
#define SC_STRINGUTIL_HXX

#include "rtl/ustring.hxx"

class ScStringUtil
{
public:
    /**
     * Check if a given string is a simple decimal number (e.g. 12.345). We
     * don't do any elaborate parsing here; we only check for the simplest
     * case of decimal number format.
     *
     * @param rStr string to parse
     * @param dsep decimal separator
     * @param gsep group separator (aka thousands separator)
     * @param rVal value of successfully parsed number
     *
     * @return true if the string is a valid number, false otherwise.
     */
    static bool parseSimpleNumber(
        const ::rtl::OUString& rStr, sal_Unicode dsep, sal_Unicode gsep, double& rVal);
};


#endif

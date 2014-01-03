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



#ifndef _SV_SALATSUIFONTUTILS_HXX
#define _SV_SALATSUIFONTUTILS_HXX

class ImplMacFontData;
class ImplDevFontList;

#include <premac.h>
#include <ApplicationServices/ApplicationServices.h>
#include <postmac.h>

#include <map>

/* This class has the responsibility of assembling a list
   of atsui compatible fonts available on the system and
   enabling access to that list.
 */
class SystemFontList
{
public:
    SystemFontList();
    ~SystemFontList();

    void AnnounceFonts( ImplDevFontList& ) const;
    ImplMacFontData* GetFontDataFromId( ATSUFontID ) const;

    ATSUFontFallbacks maFontFallbacks;

private:
    typedef std::hash_map<ATSUFontID,ImplMacFontData*> MacFontContainer;
    MacFontContainer maFontContainer;

    void InitGlyphFallbacks();
};

#endif  // _SV_SALATSUIFONTUTILS_HXX


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


#ifndef _SFXMNUCFGA_HXX
#define _SFXMNUCFGA_HXX

#include <tools/string.hxx>
#ifndef _SFXMINARRAY_HXX
#include <sfx2/minarray.hxx>
#endif

//==================================================================

class SfxMenuCfgItemArr;

struct SfxMenuCfgItem
{
    sal_uInt16             nId;        // id of the binding or 0 if none
    String             aTitle;     // title of the item
    String             aHelpText;  // Hilfetext
    String              aCommand;
    SfxMenuCfgItemArr* pPopup;     // pointer to a popup menu (if any)
};

DECL_PTRARRAY(SfxMenuCfgItemArr, SfxMenuCfgItem*, 4, 4 )


#endif


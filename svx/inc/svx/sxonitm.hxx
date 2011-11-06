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


#ifndef _SXONITM_HXX
#define _SXONITM_HXX

#include <svx/svddef.hxx>
#include <svl/stritem.hxx>

//------------------------------
// class SdrObjectNameItem
//------------------------------

class SdrObjectNameItem: public SfxStringItem {
public:
    SdrObjectNameItem()                   : SfxStringItem() { SetWhich(SDRATTR_OBJECTNAME); }
    SdrObjectNameItem(const String& rStr) : SfxStringItem(SDRATTR_OBJECTNAME,rStr) {}
    SdrObjectNameItem(SvStream& rIn)      : SfxStringItem(SDRATTR_OBJECTNAME,rIn)  {}
};

#endif

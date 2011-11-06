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


#ifndef _SDSHCITM_HXX
#define _SDSHCITM_HXX

#include <svx/xcolit.hxx>
#include <svx/svddef.hxx>

//---------------------
// class SdrShadowColorItem
//---------------------
class SdrShadowColorItem: public XColorItem {
public:
    SdrShadowColorItem() {}
    SdrShadowColorItem(long nIndex, const Color& rTheColor):          XColorItem(SDRATTR_SHADOWCOLOR,nIndex,rTheColor) {}

    SdrShadowColorItem(const String& rName, const Color& rTheColor):  XColorItem(SDRATTR_SHADOWCOLOR,rName,rTheColor) {}

//  SdrShadowColorItem(SvStream& rIn):                                XColorItem(SDRATTR_SHADOWCOLOR,rIn) {}
};

#endif

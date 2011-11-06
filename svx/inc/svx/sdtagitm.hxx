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


#ifndef _SDTAGITM_HXX
#define _SDTAGITM_HXX

#include <svx/sdooitm.hxx>
#include <svx/svddef.hxx>

//------------------------------------
// class SdrTextAutoGrowHeightItem
//------------------------------------
class SdrTextAutoGrowHeightItem: public SdrOnOffItem {
public:
    SdrTextAutoGrowHeightItem(sal_Bool bAuto=sal_True): SdrOnOffItem(SDRATTR_TEXT_AUTOGROWHEIGHT,bAuto) {}
    SdrTextAutoGrowHeightItem(SvStream& rIn)  : SdrOnOffItem(SDRATTR_TEXT_AUTOGROWHEIGHT,rIn)   {}
};

//------------------------------------
// class SdrTextAutoGrowWidthItem
//------------------------------------
class SdrTextAutoGrowWidthItem: public SdrOnOffItem {
public:
    SdrTextAutoGrowWidthItem(sal_Bool bAuto=sal_False): SdrOnOffItem(SDRATTR_TEXT_AUTOGROWWIDTH,bAuto) {}
    SdrTextAutoGrowWidthItem(SvStream& rIn)   : SdrOnOffItem(SDRATTR_TEXT_AUTOGROWWIDTH,rIn)   {}
};

#endif

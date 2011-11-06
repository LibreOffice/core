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


#ifndef _SXONEITM_HXX
#define _SXONEITM_HXX

#include <svx/svddef.hxx>
#include <svx/sdmetitm.hxx>

//------------------------------
// class SdrOnePositionXItem
//------------------------------
class SdrOnePositionXItem: public SdrMetricItem {
public:
    SdrOnePositionXItem(long nPosX=0): SdrMetricItem(SDRATTR_ONEPOSITIONX,nPosX) {}
    SdrOnePositionXItem(SvStream& rIn): SdrMetricItem(SDRATTR_ONEPOSITIONX,rIn)  {}
};

//------------------------------
// class SdrOnePositionYItem
//------------------------------
class SdrOnePositionYItem: public SdrMetricItem {
public:
    SdrOnePositionYItem(long nPosY=0): SdrMetricItem(SDRATTR_ONEPOSITIONY,nPosY) {}
    SdrOnePositionYItem(SvStream& rIn): SdrMetricItem(SDRATTR_ONEPOSITIONY,rIn)  {}
};

//------------------------------
// class SdrOneSizeWidthItem
//------------------------------
class SdrOneSizeWidthItem: public SdrMetricItem {
public:
    SdrOneSizeWidthItem(long nWdt=0): SdrMetricItem(SDRATTR_ONESIZEWIDTH,nWdt)  {}
    SdrOneSizeWidthItem(SvStream& rIn): SdrMetricItem(SDRATTR_ONESIZEWIDTH,rIn) {}
};

//------------------------------
// class SdrOneSizeWidthItem
//------------------------------
class SdrOneSizeHeightItem: public SdrMetricItem {
public:
    SdrOneSizeHeightItem(long nHgt=0): SdrMetricItem(SDRATTR_ONESIZEHEIGHT,nHgt)  {}
    SdrOneSizeHeightItem(SvStream& rIn): SdrMetricItem(SDRATTR_ONESIZEHEIGHT,rIn) {}
};

#endif

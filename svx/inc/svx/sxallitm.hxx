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


#ifndef _SXALLITM_HXX
#define _SXALLITM_HXX

#include <svx/svddef.hxx>
#include <svx/sdmetitm.hxx>

//------------------------------
// class SdrAllPositionXItem
//------------------------------
class SdrAllPositionXItem: public SdrMetricItem {
public:
    SdrAllPositionXItem(long nPosX=0): SdrMetricItem(SDRATTR_ALLPOSITIONX,nPosX) {}
    SdrAllPositionXItem(SvStream& rIn): SdrMetricItem(SDRATTR_ALLPOSITIONX,rIn)  {}
};

//------------------------------
// class SdrAllPositionYItem
//------------------------------
class SdrAllPositionYItem: public SdrMetricItem {
public:
    SdrAllPositionYItem(long nPosY=0): SdrMetricItem(SDRATTR_ALLPOSITIONY,nPosY) {}
    SdrAllPositionYItem(SvStream& rIn): SdrMetricItem(SDRATTR_ALLPOSITIONY,rIn)  {}
};

//------------------------------
// class SdrAllSizeWidthItem
//------------------------------
class SdrAllSizeWidthItem: public SdrMetricItem {
public:
    SdrAllSizeWidthItem(long nWdt=0): SdrMetricItem(SDRATTR_ALLSIZEWIDTH,nWdt)  {}
    SdrAllSizeWidthItem(SvStream& rIn): SdrMetricItem(SDRATTR_ALLSIZEWIDTH,rIn) {}
};

//------------------------------
// class SdrAllSizeWidthItem
//------------------------------
class SdrAllSizeHeightItem: public SdrMetricItem {
public:
    SdrAllSizeHeightItem(long nHgt=0): SdrMetricItem(SDRATTR_ALLSIZEHEIGHT,nHgt)  {}
    SdrAllSizeHeightItem(SvStream& rIn): SdrMetricItem(SDRATTR_ALLSIZEHEIGHT,rIn) {}
};

#endif

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


#ifndef _SXCGITM_HXX
#define _SXCGITM_HXX

#include <svx/svddef.hxx>
#include <svx/sdmetitm.hxx>

//------------------------------------
// class SdrCaptionGapItem
// Abstand des Linienanfangs vom Rect
//------------------------------------
class SdrCaptionGapItem: public SdrMetricItem {
public:
    SdrCaptionGapItem(long nGap=0)  : SdrMetricItem(SDRATTR_CAPTIONGAP,nGap) {}
    SdrCaptionGapItem(SvStream& rIn): SdrMetricItem(SDRATTR_CAPTIONGAP,rIn)  {}
};

#endif

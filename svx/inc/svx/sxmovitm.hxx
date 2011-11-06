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


#ifndef _SXMOVITM_HXX
#define _SXMOVITM_HXX

#include <svx/svddef.hxx>
#include <svx/sdmetitm.hxx>

//------------------------------
// class SdrMoveXItem
//------------------------------
class SdrMoveXItem: public SdrMetricItem {
public:
    SdrMoveXItem(long n=0): SdrMetricItem(SDRATTR_MOVEX,n)        {}
    SdrMoveXItem(SvStream& rIn): SdrMetricItem(SDRATTR_MOVEX,rIn) {}
};

//------------------------------
// class SdrMoveYItem
//------------------------------
class SdrMoveYItem: public SdrMetricItem {
public:
    SdrMoveYItem(long n=0): SdrMetricItem(SDRATTR_MOVEY,n)        {}
    SdrMoveYItem(SvStream& rIn): SdrMetricItem(SDRATTR_MOVEY,rIn) {}
};

#endif

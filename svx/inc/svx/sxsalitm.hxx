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


#ifndef _SXSALITM_HXX
#define _SXSALITM_HXX

#include <svx/svddef.hxx>
#include <svx/sdangitm.hxx>

//------------------------------
// class SdrHorzShearAllItem
//------------------------------
class SdrHorzShearAllItem: public SdrAngleItem {
public:
    SdrHorzShearAllItem(long nAngle=0): SdrAngleItem(SDRATTR_HORZSHEARALL,nAngle) {}
    SdrHorzShearAllItem(SvStream& rIn): SdrAngleItem(SDRATTR_HORZSHEARALL,rIn)    {}
};

//------------------------------
// class SdrVertShearAllItem
//------------------------------
class SdrVertShearAllItem: public SdrAngleItem {
public:
    SdrVertShearAllItem(long nAngle=0): SdrAngleItem(SDRATTR_VERTSHEARALL,nAngle) {}
    SdrVertShearAllItem(SvStream& rIn): SdrAngleItem(SDRATTR_VERTSHEARALL,rIn)    {}
};

#endif

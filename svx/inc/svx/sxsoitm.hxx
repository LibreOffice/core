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


#ifndef _SXSOITM_HXX
#define _SXSOITM_HXX

#include <svx/svddef.hxx>
#include <svx/sdangitm.hxx>

//------------------------------
// class SdrHorzShearOneItem
//------------------------------
class SdrHorzShearOneItem: public SdrAngleItem {
public:
    SdrHorzShearOneItem(long nAngle=0): SdrAngleItem(SDRATTR_HORZSHEARONE,nAngle) {}
    SdrHorzShearOneItem(SvStream& rIn): SdrAngleItem(SDRATTR_HORZSHEARONE,rIn)    {}
};

//------------------------------
// class SdrVertShearOneItem
//------------------------------
class SdrVertShearOneItem: public SdrAngleItem {
public:
    SdrVertShearOneItem(long nAngle=0): SdrAngleItem(SDRATTR_VERTSHEARONE,nAngle) {}
    SdrVertShearOneItem(SvStream& rIn): SdrAngleItem(SDRATTR_VERTSHEARONE,rIn)    {}
};

#endif

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


#ifndef _SXREOITM_HXX
#define _SXREOITM_HXX

#include <svx/svddef.hxx>

#ifndef _SXFIITM_HXX
#include <svx/sxfiitm.hxx>
#endif

//------------------------------
// class SdrResizeXOneItem
//------------------------------
class SdrResizeXOneItem: public SdrFractionItem {
public:
    SdrResizeXOneItem(): SdrFractionItem(SDRATTR_RESIZEXONE,Fraction(1,1)) {}
    SdrResizeXOneItem(const Fraction& rFact): SdrFractionItem(SDRATTR_RESIZEXONE,rFact) {}
    SdrResizeXOneItem(SvStream& rIn): SdrFractionItem(SDRATTR_RESIZEXONE,rIn)    {}
};

//------------------------------
// class SdrResizeYOneItem
//------------------------------
class SdrResizeYOneItem: public SdrFractionItem {
public:
    SdrResizeYOneItem(): SdrFractionItem(SDRATTR_RESIZEYONE,Fraction(1,1)) {}
    SdrResizeYOneItem(const Fraction& rFact): SdrFractionItem(SDRATTR_RESIZEYONE,rFact) {}
    SdrResizeYOneItem(SvStream& rIn): SdrFractionItem(SDRATTR_RESIZEYONE,rIn)    {}
};

#endif

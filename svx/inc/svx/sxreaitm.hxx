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


#ifndef _SXREAITM_HXX
#define _SXREAITM_HXX

#include <svx/svddef.hxx>

#ifndef _SXFIITM_HXX
#include <svx/sxfiitm.hxx>
#endif

//------------------------------
// class SdrResizeXAllItem
//------------------------------
class SdrResizeXAllItem: public SdrFractionItem {
public:
    SdrResizeXAllItem(): SdrFractionItem(SDRATTR_RESIZEXALL,Fraction(1,1)) {}
    SdrResizeXAllItem(const Fraction& rFact): SdrFractionItem(SDRATTR_RESIZEXALL,rFact) {}
    SdrResizeXAllItem(SvStream& rIn): SdrFractionItem(SDRATTR_RESIZEXALL,rIn)    {}
};

//------------------------------
// class SdrResizeYAllItem
//------------------------------
class SdrResizeYAllItem: public SdrFractionItem {
public:
    SdrResizeYAllItem(): SdrFractionItem(SDRATTR_RESIZEYALL,Fraction(1,1)) {}
    SdrResizeYAllItem(const Fraction& rFact): SdrFractionItem(SDRATTR_RESIZEYALL,rFact) {}
    SdrResizeYAllItem(SvStream& rIn): SdrFractionItem(SDRATTR_RESIZEYALL,rIn)    {}
};

#endif

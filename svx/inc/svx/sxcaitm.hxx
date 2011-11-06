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


#ifndef _SXCAITM_HXX
#define _SXCAITM_HXX

#include <svx/svddef.hxx>

#ifndef _SDRANGITM_HXX
#include <svx/sdangitm.hxx>
#endif
#include <svx/sdooitm.hxx>

//------------------------------------
// class SdrCaptionFixedAngleItem
// sal_True=Winkel durch SdrCaptionAngleItem vorgegeben
// sal_False=Freier Winkel
//------------------------------------
class SdrCaptionFixedAngleItem: public SdrOnOffItem {
public:
    SdrCaptionFixedAngleItem(sal_Bool bFix=sal_True): SdrOnOffItem(SDRATTR_CAPTIONFIXEDANGLE,bFix) {}
    SdrCaptionFixedAngleItem(SvStream& rIn) : SdrOnOffItem(SDRATTR_CAPTIONFIXEDANGLE,rIn)  {}
};

//------------------------------
// class SdrCaptionAngleItem
// Winkelangabe in 1/100 Degree
// Nur bei Type2, Type3 und Type4
// und nur wenn SdrCaptionIsFixedAngleItem=TRUE
//------------------------------
class SdrCaptionAngleItem: public SdrAngleItem {
public:
    SdrCaptionAngleItem(long nAngle=0): SdrAngleItem(SDRATTR_CAPTIONANGLE,nAngle) {}
    SdrCaptionAngleItem(SvStream& rIn): SdrAngleItem(SDRATTR_CAPTIONANGLE,rIn)    {}
};

#endif

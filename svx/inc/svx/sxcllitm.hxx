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


#ifndef _SXCLLITM_HXX
#define _SXCLLITM_HXX

#include <svx/svddef.hxx>
#include <svx/sdynitm.hxx>
#include <svx/sdmetitm.hxx>

//------------------------------------
// class SdrCaptionLineLenItem
// Laenge der ersten Austrittslinie
// Nur bei Type3 und Type4
// Nur relevant, wenn SdrCaptionFitLineLenItem=FALSE
//------------------------------------
class SdrCaptionLineLenItem: public SdrMetricItem {
public:
    SdrCaptionLineLenItem(long nLineLen=0): SdrMetricItem(SDRATTR_CAPTIONLINELEN,nLineLen) {}
    SdrCaptionLineLenItem(SvStream& rIn)  : SdrMetricItem(SDRATTR_CAPTIONLINELEN,rIn)      {}
};

//------------------------------------
// class SdrCaptionFitLineLenItem
// Laenge der ersten Austrittslinie automatisch berechnen oder
// Vorgabe (SdrCaptionLineLenItem) verwenden.
// Nur bei Type3 und Type4
//------------------------------------
class SdrCaptionFitLineLenItem: public SdrYesNoItem {
public:
    SdrCaptionFitLineLenItem(sal_Bool bBestFit=sal_True): SdrYesNoItem(SDRATTR_CAPTIONFITLINELEN,bBestFit) {}
    SdrCaptionFitLineLenItem(SvStream& rIn)     : SdrYesNoItem(SDRATTR_CAPTIONFITLINELEN,rIn)      {}
};

#endif

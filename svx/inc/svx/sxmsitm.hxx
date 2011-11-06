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


#ifndef _SXMSITM_HXX
#define _SXMSITM_HXX

#include <svx/svddef.hxx>
#include <svx/sxsiitm.hxx>

// Massstabsvorgabe. Wenn bereits am Model ein Massstab (UIScale) gesetzt
// ist wird dieser nicht ueberschrieben. Stattdessen werden beide Massstaebe
// miteinander multipliziert.
class SdrMeasureScaleItem: public SdrScaleItem {
public:
    SdrMeasureScaleItem()                   : SdrScaleItem(SDRATTR_MEASURESCALE,Fraction(1,1)) {}
    SdrMeasureScaleItem(const Fraction& rFr): SdrScaleItem(SDRATTR_MEASURESCALE,rFr) {}
    SdrMeasureScaleItem(SvStream& rIn)      : SdrScaleItem(SDRATTR_MEASURESCALE,rIn) {}
};

#endif

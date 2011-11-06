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


#ifndef _SXMTAITM_HXX
#define _SXMTAITM_HXX

#include <svx/svddef.hxx>
#include <svx/sdynitm.hxx>
#include <svx/sdangitm.hxx>

// Den Text automatisch zurechtdrehen (Automatisches UpsideDown).
// TextUpsideDown bleibt trotzdem weiterhin wirksam und dreht
// den Text bei sal_True nochmal.
class SdrMeasureTextAutoAngleItem: public SdrYesNoItem {
public:
    SdrMeasureTextAutoAngleItem(bool bOn=true): SdrYesNoItem(SDRATTR_MEASURETEXTAUTOANGLE,bOn) {}
    SdrMeasureTextAutoAngleItem(SvStream& rIn): SdrYesNoItem(SDRATTR_MEASURETEXTAUTOANGLE,rIn) {}
};

// Der bevorzugte Blickwinkel zum lesen des Textes. Wird nur ausgewertet, wenn
// TextAutoAngle=TRUE. Winkel in 1/100deg aus der Zeichnung zum Betrachter.
class SdrMeasureTextAutoAngleViewItem: public SdrAngleItem {
public:
    SdrMeasureTextAutoAngleViewItem(long nVal=31500): SdrAngleItem(SDRATTR_MEASURETEXTAUTOANGLEVIEW,nVal)  {}
    SdrMeasureTextAutoAngleViewItem(SvStream& rIn): SdrAngleItem(SDRATTR_MEASURETEXTAUTOANGLEVIEW,rIn) {}
};

#endif

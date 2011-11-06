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


#ifndef _SXMFSITM_HXX
#define _SXMFSITM_HXX

#include <svx/svddef.hxx>
#include <svl/stritem.hxx>

// Formatstring (aehnl. Calc), z.B. "#.###,00km"
// ueberschreibt SdrMeasureUnitItem und SdrMeasureShowUnitItem (n.i.)
class SdrMeasureFormatStringItem: public SfxStringItem {
public:
    SdrMeasureFormatStringItem()                   : SfxStringItem() { SetWhich(SDRATTR_MEASUREFORMATSTRING); }
    SdrMeasureFormatStringItem(const String& rStr) : SfxStringItem(SDRATTR_MEASUREFORMATSTRING,rStr) {}
    SdrMeasureFormatStringItem(SvStream& rIn)      : SfxStringItem(SDRATTR_MEASUREFORMATSTRING,rIn)  {}
};

#endif

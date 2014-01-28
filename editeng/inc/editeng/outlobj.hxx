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



#ifndef _OUTLOBJ_HXX
#define _OUTLOBJ_HXX

//////////////////////////////////////////////////////////////////////////////

#include <tools/solar.h>
#include <tools/string.hxx>
#include <rsc/rscsfx.hxx>
#include <editeng/paragraphdata.hxx>
#include "editeng/editengdllapi.h"

class EditTextObject;
class ImplOutlinerParaObject;

//////////////////////////////////////////////////////////////////////////////

class EDITENG_DLLPUBLIC OutlinerParaObject
{
private:
    ImplOutlinerParaObject*        mpImplOutlinerParaObject;

    void ImplMakeUnique();

public:
    // constructors/destructor
    OutlinerParaObject( const EditTextObject&, const ParagraphDataVector&, bool bIsEditDoc = true);
    OutlinerParaObject( const EditTextObject&);
    OutlinerParaObject( const OutlinerParaObject&);
    ~OutlinerParaObject();

    // assignment operator
    OutlinerParaObject& operator=(const OutlinerParaObject& rCandidate);

    // compare operator
    bool operator==(const OutlinerParaObject& rCandidate) const;
    bool operator!=(const OutlinerParaObject& rCandidate) const { return !operator==(rCandidate); }

    // #i102062#
    bool isWrongListEqual(const OutlinerParaObject& rCompare) const;

    // outliner mode access
    sal_uInt16 GetOutlinerMode() const;
    void SetOutlinerMode(sal_uInt16 nNew);

    // vertical access
    bool IsVertical() const;
    void SetVertical(bool bNew);

    // data read access
    sal_uInt32 Count() const;
    sal_Int16 GetDepth(sal_uInt16 nPara) const;
    const EditTextObject& GetTextObject() const;
    bool IsEditDoc() const;
    const ParagraphData& GetParagraphData(sal_uInt32 nIndex) const;

    // portion info support
    void ClearPortionInfo();

    // StyleSheet support
    bool ChangeStyleSheets(const XubString& rOldName, SfxStyleFamily eOldFamily, const XubString& rNewName, SfxStyleFamily eNewFamily);
    void ChangeStyleSheetName(SfxStyleFamily eFamily, const XubString& rOldName, const XubString& rNewName);
    void SetStyleSheets(sal_uInt16 nLevel, const XubString rNewName, const SfxStyleFamily& rNewFamily);
};

//////////////////////////////////////////////////////////////////////////////

#endif

/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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
    OutlinerParaObject(
        const EditTextObject& rEditTextObject,
        const ParagraphDataVector& rParagraphDataVector = ParagraphDataVector(),
        bool bIsEditDoc = true);
    OutlinerParaObject(const OutlinerParaObject& rCandidate);
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
    bool ChangeStyleSheets(const OUString& rOldName, SfxStyleFamily eOldFamily,
        const OUString& rNewName, SfxStyleFamily eNewFamily);
    void ChangeStyleSheetName(SfxStyleFamily eFamily, const OUString& rOldName,
        const OUString& rNewName);
    void SetStyleSheets(sal_uInt16 nLevel, const OUString rNewName,
        const SfxStyleFamily& rNewFamily);
};

//////////////////////////////////////////////////////////////////////////////

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

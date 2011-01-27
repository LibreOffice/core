/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
    bool ChangeStyleSheets(const XubString& rOldName, SfxStyleFamily eOldFamily, const XubString& rNewName, SfxStyleFamily eNewFamily);
    void ChangeStyleSheetName(SfxStyleFamily eFamily, const XubString& rOldName, const XubString& rNewName);
    void SetStyleSheets(sal_uInt16 nLevel, const XubString rNewName, const SfxStyleFamily& rNewFamily);
};

//////////////////////////////////////////////////////////////////////////////

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

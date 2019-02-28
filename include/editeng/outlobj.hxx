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

#ifndef INCLUDED_EDITENG_OUTLOBJ_HXX
#define INCLUDED_EDITENG_OUTLOBJ_HXX

#include <editeng/paragraphdata.hxx>
#include <editeng/editengdllapi.h>
#include <rtl/ustring.hxx>
#include <svl/style.hxx>
#include <o3tl/cow_wrapper.hxx>
#include <memory>

class EditTextObject;
enum class OutlinerMode;

/**
 * This is the guts of OutlinerParaObject, refcounted and shared among
 * multiple instances of OutlinerParaObject.
 */
struct OutlinerParaObjData
{
    // data members
    std::unique_ptr<EditTextObject>  mpEditTextObject;
    ParagraphDataVector              maParagraphDataVector;
    bool                             mbIsEditDoc;

    // constructor
    OutlinerParaObjData( std::unique_ptr<EditTextObject> pEditTextObject, const ParagraphDataVector& rParagraphDataVector, bool bIsEditDoc );

    OutlinerParaObjData( const OutlinerParaObjData& r );

    // assignment operator
    OutlinerParaObjData& operator=(const OutlinerParaObjData& rCandidate) = delete;

    // destructor
    ~OutlinerParaObjData();

    bool operator==(const OutlinerParaObjData& rCandidate) const;

    // #i102062#
    bool isWrongListEqual(const OutlinerParaObjData& rCompare) const;
};

class EDITENG_DLLPUBLIC OutlinerParaObject
{
    ::o3tl::cow_wrapper< OutlinerParaObjData > mpImpl;

public:
    // constructors/destructor
    OutlinerParaObject( const EditTextObject&, const ParagraphDataVector&, bool bIsEditDoc);
    OutlinerParaObject( const EditTextObject&);
    OutlinerParaObject( std::unique_ptr<EditTextObject> );
    OutlinerParaObject( const OutlinerParaObject&);
    ~OutlinerParaObject();

    // assignment operator
    OutlinerParaObject& operator=(const OutlinerParaObject& rCandidate);

    // compare operator
    bool operator==(const OutlinerParaObject& rCandidate) const;

    // #i102062#
    bool isWrongListEqual(const OutlinerParaObject& rCompare) const;

    // outliner mode access
    OutlinerMode GetOutlinerMode() const;
    void SetOutlinerMode(OutlinerMode nNew);

    // vertical access
    bool IsVertical() const;
    bool IsTopToBottom() const;
    void SetVertical(bool bNew, bool bTopToBottom = true);

    // data read access
    sal_Int32 Count() const;
    sal_Int16 GetDepth(sal_Int32 nPara) const;
    const EditTextObject& GetTextObject() const;
    const ParagraphData& GetParagraphData(sal_Int32 nIndex) const;

    // portion info support
    void ClearPortionInfo();

    // StyleSheet support
    bool ChangeStyleSheets(const OUString& rOldName, SfxStyleFamily eOldFamily,
        const OUString& rNewName, SfxStyleFamily eNewFamily);
    void ChangeStyleSheetName(SfxStyleFamily eFamily, const OUString& rOldName,
        const OUString& rNewName);
    void SetStyleSheets(sal_uInt16 nLevel, const OUString& rNewName,
        const SfxStyleFamily& rNewFamily);

    void dumpAsXml(xmlTextWriterPtr pWriter) const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

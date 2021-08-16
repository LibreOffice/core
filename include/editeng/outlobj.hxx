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

#pragma once

#include <editeng/paragraphdata.hxx>
#include <editeng/editengdllapi.h>
#include <rtl/ustring.hxx>
#include <svl/poolitem.hxx>
#include <svl/style.hxx>
#include <o3tl/cow_wrapper.hxx>
#include <stdexcept>
#include <memory>

class EditTextObject;
enum class OutlinerMode;
enum class TextRotation;

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

    OutlinerParaObjData( OutlinerParaObjData&& r ) = default;

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
friend class std::optional<OutlinerParaObject>;
    ::o3tl::cow_wrapper< OutlinerParaObjData > mpImpl;

    OutlinerParaObject(std::nullopt_t) noexcept
        : mpImpl(std::nullopt) {}
    OutlinerParaObject( const OutlinerParaObject& other, std::nullopt_t ) noexcept
        : mpImpl(other.mpImpl, std::nullopt) {}

public:
    // constructors/destructor
    OutlinerParaObject(std::unique_ptr<EditTextObject>, const ParagraphDataVector&, bool bIsEditDoc);
    OutlinerParaObject( std::unique_ptr<EditTextObject> );
    OutlinerParaObject( const OutlinerParaObject&);
    OutlinerParaObject(OutlinerParaObject&&) noexcept;
    ~OutlinerParaObject();

    // assignment operator
    OutlinerParaObject& operator=(const OutlinerParaObject& rCandidate);
    OutlinerParaObject& operator=(OutlinerParaObject&&) noexcept;

    // compare operator
    bool operator==(const OutlinerParaObject& rCandidate) const;
    bool operator!=(const OutlinerParaObject& rCandidate) const { return !operator==(rCandidate); }

    // #i102062#
    bool isWrongListEqual(const OutlinerParaObject& rCompare) const;

    // outliner mode access
    OutlinerMode GetOutlinerMode() const;
    void SetOutlinerMode(OutlinerMode nNew);

    // vertical access
    bool IsEffectivelyVertical() const;
    bool GetVertical() const;
    bool IsTopToBottom() const;
    void SetVertical(bool bNew);
    void SetRotation(TextRotation nRotation);
    TextRotation GetRotation() const;

    // data read access
    sal_Int32 Count() const;
    sal_Int16 GetDepth(sal_Int32 nPara) const;
    const EditTextObject& GetTextObject() const;
    const ParagraphData& GetParagraphData(sal_Int32 nIndex) const;

    // portion info support
    void ClearPortionInfo();

    // StyleSheet support
    bool ChangeStyleSheets(std::u16string_view rOldName, SfxStyleFamily eOldFamily,
        const OUString& rNewName, SfxStyleFamily eNewFamily);
    void ChangeStyleSheetName(SfxStyleFamily eFamily, std::u16string_view rOldName,
        const OUString& rNewName);
    void SetStyleSheets(sal_uInt16 nLevel, const OUString& rNewName,
        const SfxStyleFamily& rNewFamily);

    void dumpAsXml(xmlTextWriterPtr pWriter) const;
};

namespace std
{
    template<>
    class optional<OutlinerParaObject>
    {
    public:
        optional() noexcept : maParaObject(std::nullopt) {}
        optional(std::nullopt_t) noexcept : maParaObject(std::nullopt) {}
        optional(const optional& other) :
            maParaObject(other.maParaObject, std::nullopt) {}
        optional(optional&& other) noexcept :
            maParaObject(std::move(other.maParaObject)) {}
        optional(OutlinerParaObject&& para) noexcept :
            maParaObject(std::move(para)) {}
        optional(const OutlinerParaObject& para) noexcept :
            maParaObject(para) {}

        optional& operator=(optional const & other)
        {
            maParaObject = other.maParaObject;
            return *this;
        }
        optional& operator=(optional&& other) noexcept
        {
            maParaObject = std::move(other.maParaObject);
            return *this;
        }

        bool has_value() const noexcept { return maParaObject.mpImpl.empty(); }
        explicit operator bool() const noexcept { return maParaObject.mpImpl.empty(); }

        OutlinerParaObject& operator*()
        {
            checkNotEmpty();
            return maParaObject;
        }
        const OutlinerParaObject& operator*() const
        {
            checkNotEmpty();
            return maParaObject;
        }
        OutlinerParaObject& operator->()
        {
            checkNotEmpty();
            return maParaObject;
        }
        const OutlinerParaObject& operator->() const
        {
            checkNotEmpty();
            return maParaObject;
        }
    private:
        void checkNotEmpty() const
        {
            if (maParaObject.mpImpl.empty())
                throw std::logic_error("empty");
        }
        OutlinerParaObject maParaObject;
    };
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

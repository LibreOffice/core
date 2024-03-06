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

#ifndef INCLUDED_EDITENG_EDITOBJ_HXX
#define INCLUDED_EDITENG_EDITOBJ_HXX

#include <svl/style.hxx>
#include <editeng/editengdllapi.h>
#include <editeng/macros.hxx>
#include <svl/languageoptions.hxx>

#include <com/sun/star/text/textfield/Type.hpp>

#include <vector>
#include <memory>

class SfxItemPool;
class SfxItemSet;
class SvxFieldItem;
class SvxFieldData;
enum class OutlinerMode;
struct EECharAttrib;
typedef struct _xmlTextWriter* xmlTextWriterPtr;

namespace editeng {

class FieldUpdater;
struct Section;

}

namespace svl {

class SharedString;
class SharedStringPool;

}

enum class TextRotation { NONE, TOPTOBOTTOM, BOTTOMTOTOP };

class EDITENG_DLLPUBLIC EditTextObject
{
public:
    virtual ~EditTextObject();

    /**
     * Set paragraph strings to the shared string pool.
     *
     * @param rPool shared string pool.
     */
    virtual void NormalizeString( svl::SharedStringPool& rPool ) = 0;

    virtual std::vector<svl::SharedString> GetSharedStrings() const = 0;

    virtual const SfxItemPool* GetPool() const = 0;
    virtual OutlinerMode GetUserType() const = 0;    // For OutlinerMode, it can however not save in compatible format
    virtual void SetUserType( OutlinerMode n ) = 0;

    virtual bool IsEffectivelyVertical() const = 0;
    virtual bool GetVertical() const = 0;
    virtual bool IsTopToBottom() const = 0;
    virtual void SetVertical( bool bVertical ) = 0;
    virtual void SetRotation( TextRotation nRotation ) = 0;
    virtual TextRotation    GetRotation() const = 0;

    virtual SvtScriptType GetScriptType() const = 0;

    virtual std::unique_ptr<EditTextObject> Clone() const = 0;

    virtual sal_Int32 GetParagraphCount() const = 0;

    virtual OUString GetText(sal_Int32 nPara) const = 0;

    virtual sal_Int32 GetTextLen(sal_Int32 nPara) const = 0;

    bool HasText(sal_Int32 nPara) const { return GetTextLen(nPara) > 0; }

    virtual void ClearPortionInfo() = 0;

    virtual bool HasOnlineSpellErrors() const = 0;

    virtual void GetCharAttribs( sal_Int32 nPara, std::vector<EECharAttrib>& rLst ) const = 0;

    virtual bool RemoveCharAttribs( sal_uInt16 nWhich ) = 0;

    /**
     * Get all text sections in this content.  Sections are non-overlapping
     * segments of text split either by paragraph boundaries or format
     * boundaries.  Each section object contains all applied formats and/or a
     * field item.
     */
    virtual void GetAllSections( std::vector<editeng::Section>& rAttrs ) const = 0;

    virtual bool IsFieldObject() const = 0;
    virtual const SvxFieldItem* GetField() const = 0;
    virtual const SvxFieldData* GetFieldData(sal_Int32 nPara, size_t nPos, sal_Int32 nType) const = 0;
    virtual bool HasField( sal_Int32 nType = css::text::textfield::Type::UNSPECIFIED ) const = 0;

    virtual const SfxItemSet& GetParaAttribs(sal_Int32 nPara) const = 0;

    virtual void GetStyleSheet(sal_Int32 nPara, OUString& rName, SfxStyleFamily& eFamily) const = 0;
    virtual void SetStyleSheet(sal_Int32 nPara, const OUString& rName, const SfxStyleFamily& eFamily) = 0;
    virtual bool ChangeStyleSheets(
        std::u16string_view rOldName, SfxStyleFamily eOldFamily, const OUString& rNewName, SfxStyleFamily eNewFamily) = 0;
    virtual void ChangeStyleSheetName(SfxStyleFamily eFamily, std::u16string_view rOldName, const OUString& rNewName) = 0;

    virtual editeng::FieldUpdater GetFieldUpdater() = 0;

    virtual bool operator==( const EditTextObject& rCompare ) const = 0;

    /** Compare, ignoring SfxItemPool pointer.
     */
    bool Equals( const EditTextObject& rCompare ) const;

    // #i102062#
    virtual bool isWrongListEqual(const EditTextObject& rCompare) const = 0;

#if DEBUG_EDIT_ENGINE
    virtual void Dump() const = 0;
#endif
    virtual void dumpAsXml(xmlTextWriterPtr pWriter) const = 0;
};

#endif // INCLUDED_EDITENG_EDITOBJ_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

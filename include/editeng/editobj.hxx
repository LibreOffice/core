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
#include <svl/itempool.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/editdata.hxx>
#include <editeng/editengdllapi.h>
#include <editeng/macros.hxx>
#include <svl/languageoptions.hxx>

#include <com/sun/star/text/textfield/Type.hpp>

#include <vector>
#include <memory>

class SfxItemSet;
class SvxFieldItem;
class SvxFieldData;
class SvStream;
enum class OutlinerMode;

namespace editeng {

class FieldUpdater;
class FieldUpdaterImpl;
struct Section;

}

namespace svl {

class SharedString;
class SharedStringPool;

}

class EditTextObjectImpl;

class EDITENG_DLLPUBLIC EditTextObject : public SfxItemPoolUser
{
    friend class EditTextObjectImpl;
    friend class editeng::FieldUpdaterImpl;
    friend class ImpEditEngine;

    std::unique_ptr<EditTextObjectImpl> mpImpl;

    EditTextObject( SfxItemPool* pPool );

public:
    EditTextObject( const EditTextObject& r );
    virtual ~EditTextObject();
    EditTextObject&      operator=( const EditTextObject& ) = delete;

    /**
     * Set paragraph strings to the shared string pool.
     *
     * @param rPool shared string pool.
     */
    void NormalizeString( svl::SharedStringPool& rPool );

    std::vector<svl::SharedString> GetSharedStrings() const;

    const SfxItemPool* GetPool() const;
    OutlinerMode GetUserType() const;    // For OutlinerMode, it can however not save in compatible format
    void SetUserType( OutlinerMode n );

    bool IsVertical() const;
    bool IsTopToBottom() const;
    void SetVertical( bool bVertical, bool bTopToBottom = true);

    SvtScriptType GetScriptType() const;

    std::unique_ptr<EditTextObject> Clone() const;

    sal_Int32 GetParagraphCount() const;

    OUString GetText(sal_Int32 nPara) const;

    void ClearPortionInfo();

    bool HasOnlineSpellErrors() const;

    void GetCharAttribs( sal_Int32 nPara, std::vector<EECharAttrib>& rLst ) const;

    bool RemoveCharAttribs( sal_uInt16 nWhich );

    /**
     * Get all text sections in this content.  Sections are non-overlapping
     * segments of text split either by paragraph boundaries or format
     * boundaries.  Each section object contains all applied formats and/or a
     * field item.
     */
    void GetAllSections( std::vector<editeng::Section>& rAttrs ) const;

    bool IsFieldObject() const;
    const SvxFieldItem* GetField() const;
    const SvxFieldData* GetFieldData(sal_Int32 nPara, size_t nPos, sal_Int32 nType) const;
    bool HasField( sal_Int32 nType = css::text::textfield::Type::UNSPECIFIED ) const;

    const SfxItemSet& GetParaAttribs(sal_Int32 nPara) const;

    void GetStyleSheet(sal_Int32 nPara, OUString& rName, SfxStyleFamily& eFamily) const;
    void SetStyleSheet(sal_Int32 nPara, const OUString& rName, const SfxStyleFamily& eFamily);
    bool ChangeStyleSheets(
        const OUString& rOldName, SfxStyleFamily eOldFamily, const OUString& rNewName, SfxStyleFamily eNewFamily);
    void ChangeStyleSheetName(SfxStyleFamily eFamily, const OUString& rOldName, const OUString& rNewName);

    editeng::FieldUpdater GetFieldUpdater();

    bool operator==( const EditTextObject& rCompare ) const;

    /** Compare, ignoring SfxItemPool pointer.
     */
    bool Equals( const EditTextObject& rCompare ) const;

    // #i102062#
    bool isWrongListEqual(const EditTextObject& rCompare) const;

    virtual void ObjectInDestruction(const SfxItemPool& rSfxItemPool) override;

#if DEBUG_EDIT_ENGINE
    void Dump() const;
#endif
    void dumpAsXml(xmlTextWriterPtr pWriter) const;
};

#endif // INCLUDED_EDITENG_EDITOBJ_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

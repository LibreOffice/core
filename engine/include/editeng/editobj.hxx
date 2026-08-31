/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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
#include <svl/itemset.hxx>
#include <svl/languageoptions.hxx>
#include <tools/lineend.hxx>

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
class ContentInfo;
class XParaPortionList;

namespace editeng {
class FieldUpdater;
struct Section;
}

namespace svl {
class SharedString;
class SharedStringPool;
}

enum class TextRotation { NONE, TOPTOBOTTOM, BOTTOMTOTOP };

class XEditAttribute
{
private:
    SfxPoolItemHolder   maItemHolder;
    sal_Int32           nStart;
    sal_Int32           nEnd;

public:
    XEditAttribute(SfxItemPool&, const SfxPoolItem&, sal_Int32 nStart, sal_Int32 nEnd );

    const SfxPoolItem*      GetItem() const             { return maItemHolder.getItem(); }
    SfxPoolItemHolder&      GetItemHolder()             { return maItemHolder; }

    sal_Int32&              GetStart()                  { return nStart; }
    sal_Int32&              GetEnd()                    { return nEnd; }

    sal_Int32               GetStart() const            { return nStart; }
    sal_Int32               GetEnd() const              { return nEnd; }

    sal_Int32               GetLen() const              { return nEnd-nStart; }

    bool IsFeature() const;
    void SetItem(SfxItemPool&, const SfxPoolItem&);

    inline bool operator==( const XEditAttribute& rCompare ) const;
};



class EDITENG_DLLPUBLIC EditTextObject final
{
public:
    typedef std::vector<std::unique_ptr<ContentInfo> > ContentInfosType;

    EditTextObject(SfxItemPool* pPool, MapUnit eDefaultMetric, bool bVertical,
                   TextRotation eRotation, SvtScriptType eScriptType);
    EditTextObject( const EditTextObject& r );
    ~EditTextObject();

    /**
     * Set paragraph strings to the shared string pool.
     *
     * @param rPool shared string pool.
     */
    void NormalizeString( svl::SharedStringPool& rPool );

    std::vector<svl::SharedString> GetSharedStrings() const;

    const SfxItemPool* GetPool() const { return mpPool.get(); }
    SfxItemPool*       GetPool()       { return mpPool.get(); }

    OutlinerMode GetUserType() const { return meUserType;} // For OutlinerMode, it can however not save in compatible format
    void SetUserType( OutlinerMode n );

    bool IsEffectivelyVertical() const;
    bool GetVertical() const;
    bool IsTopToBottom() const;
    void SetVertical( bool bVertical );
    void SetRotation( TextRotation nRotation );
    TextRotation    GetRotation() const;

    SvtScriptType GetScriptType() const { return meScriptType;}

    sal_Int32 GetParagraphCount() const;

    OUString GetText(sal_Int32 nPara) const;

    OUString GetText(LineEnd eEnd = LINEEND_LF) const;

    sal_Int32 GetTextLen(sal_Int32 nPara) const;

    bool HasText(sal_Int32 nPara) const { return GetTextLen(nPara) > 0; }

    XParaPortionList*       GetPortionInfo() const  { return mpPortionInfo.get(); }
    void                    SetPortionInfo( std::unique_ptr<XParaPortionList> pP );
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
        std::u16string_view rOldName, SfxStyleFamily eOldFamily, const OUString& rNewName, SfxStyleFamily eNewFamily);
    void ChangeStyleSheetName(SfxStyleFamily eFamily, std::u16string_view rOldName, const OUString& rNewName);

    editeng::FieldUpdater GetFieldUpdater();

    bool operator==( const EditTextObject& rCompare ) const;

    /** Compare, ignoring SfxItemPool pointer.
     */
    bool Equals( const EditTextObject& rCompare ) const;

    bool Equals( const EditTextObject& rCompare, bool bComparePool ) const;

    // #i102062#
    bool isWrongListEqual(const EditTextObject& rCompare) const;

    ContentInfo*            CreateAndInsertContent();
    XEditAttribute          CreateAttrib( const SfxPoolItem& rItem, sal_Int32 nStart, sal_Int32 nEnd );
    ContentInfosType&       GetContents() { return maContents;}
    const ContentInfosType& GetContents() const { return maContents;}

    bool                    HasMetric() const { return meMetric != MapUnit::LASTENUMDUMMY; }
    MapUnit                 GetMetric() const { return meMetric; }

#if DEBUG_EDIT_ENGINE
    void Dump() const;
#endif
    void dumpAsXml(xmlTextWriterPtr pWriter) const;

private:
    bool ImpChangeStyleSheets( std::u16string_view rOldName, SfxStyleFamily eOldFamily,
                               const OUString& rNewName, SfxStyleFamily eNewFamily );

    ContentInfosType        maContents;
    rtl::Reference<SfxItemPool>       mpPool;
    std::unique_ptr<XParaPortionList> mpPortionInfo;
    OutlinerMode            meUserType;
    SvtScriptType           meScriptType;
    TextRotation            meRotation;
    MapUnit                 meMetric;
    bool                    mbVertical;
};

#endif // INCLUDED_EDITENG_EDITOBJ_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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

#include "editattr.hxx"
#include "edtspell.hxx"
#include <editeng/svxfont.hxx>
#include <svl/itemset.hxx>
#include <svl/style.hxx>
#include <svl/itempool.hxx>
#include <svl/languageoptions.hxx>
#include <tools/lineend.hxx>

#include <memory>
#include <string_view>
#include <vector>

class SvxTabStop;

class ContentAttribs
{
private:
    SfxStyleSheet* pStyle;
    SfxItemSetFixed<EE_PARA_START, EE_CHAR_END> aAttribSet;

public:
    ContentAttribs(SfxItemPool& rItemPool);

    void dumpAsXml(xmlTextWriterPtr pWriter) const;

    SvxTabStop FindTabStop(sal_Int32 nCurPos, sal_uInt16 nDefTab);
    SfxItemSet& GetItems() { return aAttribSet; }
    const SfxItemSet& GetItems() const { return aAttribSet; }
    const SfxStyleSheet* GetStyleSheet() const { return pStyle; }
    SfxStyleSheet* GetStyleSheet() { return pStyle; }
    void SetStyleSheet(SfxStyleSheet* pS);

    const SfxPoolItem& GetItem(sal_uInt16 nWhich) const;
    template <class T> const T& GetItem(TypedWhichId<T> nWhich) const
    {
        return static_cast<const T&>(GetItem(sal_uInt16(nWhich)));
    }
    bool HasItem(sal_uInt16 nWhich) const;
};

class CharAttribList
{
public:
    typedef std::vector<std::unique_ptr<EditCharAttrib>> AttribsType;

private:
    AttribsType aAttribs;
    SvxFont aDefFont; // faster than ever from the pool!
    bool bHasEmptyAttribs;

public:
    CharAttribList();
    ~CharAttribList();

    void dumpAsXml(xmlTextWriterPtr pWriter) const;

    void DeleteEmptyAttribs();

    const EditCharAttrib* FindAttrib(sal_uInt16 nWhich, sal_Int32 nPos) const;
    EditCharAttrib* FindAttrib(sal_uInt16 nWhich, sal_Int32 nPos);
    const EditCharAttrib* FindNextAttrib(sal_uInt16 nWhich, sal_Int32 nFromPos) const;
    EditCharAttrib* FindEmptyAttrib(sal_uInt16 nWhich, sal_Int32 nPos);
    const EditCharAttrib* FindFeature(sal_Int32 nPos) const;

    void ResortAttribs();
    void OptimizeRanges();

    sal_Int32 Count() const;

    void InsertAttrib(EditCharAttrib* pAttrib);

    SvxFont& GetDefFont() { return aDefFont; }

    bool HasEmptyAttribs() const { return bHasEmptyAttribs; }
    void SetHasEmptyAttribs(bool b);
    bool HasBoundingAttrib(sal_Int32 nBound) const;
    bool HasAttrib(sal_Int32 nStartPos, sal_Int32 nEndPos) const;

    AttribsType& GetAttribs() { return aAttribs; }
    const AttribsType& GetAttribs() const { return aAttribs; }

    void Remove(const EditCharAttrib* p);
    void Remove(sal_Int32 nPos);

#if OSL_DEBUG_LEVEL > 0 && !defined NDEBUG
    static void DbgCheckAttribs(CharAttribList const& rAttribs);
#endif
};

class ContentNode
{
private:
    OUString maString;
    ContentAttribs aContentAttribs;
    CharAttribList aCharAttribList;
    std::unique_ptr<WrongList> mpWrongList;

    void UnExpandPosition(sal_Int32& rStartPos, bool bBiasStart);

public:
    ContentNode(SfxItemPool& rItemPool);
    ContentNode(const OUString& rStr, const ContentAttribs& rContentAttribs);
    ~ContentNode();
    ContentNode(const ContentNode&) = delete;
    ContentNode& operator=(const ContentNode&) = delete;

    void dumpAsXml(xmlTextWriterPtr pWriter) const;

    ContentAttribs& GetContentAttribs() { return aContentAttribs; }
    const ContentAttribs& GetContentAttribs() const { return aContentAttribs; }
    CharAttribList& GetCharAttribs() { return aCharAttribList; }
    const CharAttribList& GetCharAttribs() const { return aCharAttribList; }

    void ExpandAttribs(sal_Int32 nIndex, sal_Int32 nNewChars);
    void CollapseAttribs(sal_Int32 nIndex, sal_Int32 nDelChars);
    void AppendAttribs(ContentNode* pNextNode);
    void CopyAndCutAttribs(ContentNode* pPrevNode, SfxItemPool& rPool, bool bKeepEndingAttribs);

    void SetStyleSheet(SfxStyleSheet* pS, bool bRecalcFont = true);
    void SetStyleSheet(SfxStyleSheet* pS, const SvxFont& rFontFromStyle);
    SfxStyleSheet* GetStyleSheet() { return aContentAttribs.GetStyleSheet(); }

    void CreateDefFont();

    void EnsureWrongList();
    WrongList* GetWrongList();
    const WrongList* GetWrongList() const;
    void SetWrongList(WrongList* p);

    void CreateWrongList();
    void DestroyWrongList();

    bool IsFeature(sal_Int32 nPos) const;

    sal_Int32 Len() const;
    const OUString& GetString() const { return maString; }

    /// return length including expanded fields
    sal_Int32 GetExpandedLen() const;
    /// return content including expanded fields
    OUString GetExpandedText(sal_Int32 nStartPos = 0, sal_Int32 nEndPos = -1) const;
    /// re-write offsets in the expanded text to string offsets
    void UnExpandPositions(sal_Int32& rStartPos, sal_Int32& rEndPos);

    void SetChar(sal_Int32 nPos, sal_Unicode c);
    void Insert(std::u16string_view rStr, sal_Int32 nPos);
    void Append(std::u16string_view rStr);
    void Erase(sal_Int32 nPos);
    void Erase(sal_Int32 nPos, sal_Int32 nCount);
    OUString Copy(sal_Int32 nPos) const;
    OUString Copy(sal_Int32 nPos, sal_Int32 nCount) const;
    sal_Unicode GetChar(sal_Int32 nPos) const;

    void checkAndDeleteEmptyAttribs() const;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

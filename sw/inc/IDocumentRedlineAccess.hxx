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

#include <sal/config.h>

#include <cstddef>

#include <sal/types.h>

#include <com/sun/star/uno/Sequence.h>
#include <o3tl/typed_flags_set.hxx>
#include <svx/ctredlin.hxx>

#include "docary.hxx"

class SwRangeRedline;
class SwTableRowRedline;
class SwTableCellRedline;
class SwPaM;
struct SwPosition;
class SwStartNode;
class SwNode;
class SwViewShell;

enum class RedlineFlags
{
    NONE                 = 0x000, ///< no RedlineFlags
    On                   = 0x001, ///< RedlineFlags on
    Ignore               = 0x002, ///< ignore Redlines (only set from code, temporarily)
    ShowInsert           = 0x010, ///< show all inserts
    ShowDelete           = 0x020, ///< show all deletes
    ShowMask             = ShowInsert | ShowDelete,

    // For internal management:
    // remove the original Redlines together with their content
    // (Clipboard/text modules).
    DeleteRedlines       = 0x100,
    // don't combine any redlines. This flag may be only used in Undo.
    DontCombineRedlines  = 0x400,
};
namespace o3tl
{
    template<> struct typed_flags<RedlineFlags> : is_typed_flags<RedlineFlags, 0x533> {};
}

inline OUString SwRedlineTypeToOUString(RedlineType eType)
{
    OUString sRet;
    switch(eType)
    {
        case RedlineType::Insert: sRet = "Insert"; break;
        case RedlineType::Delete: sRet = "Delete"; break;
        case RedlineType::Format: sRet = "Format"; break;
        case RedlineType::ParagraphFormat: sRet = "ParagraphFormat"; break;
        case RedlineType::Table:  sRet = "TextTable"; break;
        case RedlineType::FmtColl:sRet = "Style"; break;
        default: break;
    }
    return sRet;
};

class IDocumentRedlineAccess
{
     // Static helper functions
public:
    static bool IsShowChanges(const RedlineFlags eM)
    { return (RedlineFlags::ShowInsert | RedlineFlags::ShowDelete) == (eM & RedlineFlags::ShowMask); }

    static bool IsHideChanges(const RedlineFlags eM)
    { return RedlineFlags::ShowInsert == (eM & RedlineFlags::ShowMask); }

    static bool IsShowOriginal(const RedlineFlags eM)
    { return RedlineFlags::ShowDelete == (eM & RedlineFlags::ShowMask); }

    static bool IsRedlineOn(const RedlineFlags eM)
    { return RedlineFlags::On == (eM & (RedlineFlags::On | RedlineFlags::Ignore )); }

public:

    /** Query the currently set redline mode

        @returns
        the currently set redline mode
    */
     virtual RedlineFlags GetRedlineFlags(const SwViewShell* pViewShell = nullptr) const = 0;

    /** Set a new redline mode.

        @param eMode
        [in] the new redline mode.
    */
    virtual void SetRedlineFlags_intern(/*[in]*/RedlineFlags eMode, bool bRecordAllViews = true) = 0;

    /** Set a new redline mode.

        @param eMode
        [in] the new redline mode.
    */
    virtual void SetRedlineFlags(/*[in]*/RedlineFlags eMode, bool bRecordAllViews = true) = 0;

    /** Query if redlining is on.

        @returns
        <TRUE/> if redlining is on <FALSE/> otherwise
    */
    virtual bool IsRedlineOn() const = 0;

    virtual bool IsIgnoreRedline() const = 0;

    virtual const SwRedlineTable& GetRedlineTable() const = 0;
    virtual SwRedlineTable& GetRedlineTable() = 0;
    virtual const SwExtraRedlineTable& GetExtraRedlineTable() const = 0;
    virtual SwExtraRedlineTable& GetExtraRedlineTable() = 0;

    virtual bool IsInRedlines(const SwNode& rNode) const = 0;

    enum class AppendResult { IGNORED, MERGED, APPENDED };
    /** Append a new redline

        @param pNewRedl redline to insert

        @param bCallDelete
            if set, then for a new DELETE redline that is inserted so that it
            overlaps an existing INSERT redline with the same author, the
            overlapping range is deleted, i.e. the new DELETE removes
            existing INSERT for that range

        @returns
            APPENDED if pNewRedl is still alive and was appended
            MERGED if pNewRedl was deleted but has been merged with existing one
            IGNORED if pNewRedl was deleted and ignored/invalid
    */
    virtual AppendResult AppendRedline(/*[in]*/ SwRangeRedline* pNewRedl, /*[in]*/ bool bCallDelete,
                                       /*[in]*/ sal_uInt32 nMoveIDToDelete = 0) = 0;

    virtual bool AppendTableRowRedline(/*[in]*/SwTableRowRedline* pPtr) = 0;
    virtual bool AppendTableCellRedline(/*[in]*/SwTableCellRedline* pPtr) = 0;

    virtual bool SplitRedline(/*[in]*/const SwPaM& rPam) = 0;

    virtual bool DeleteRedline(
        /*[in]*/const SwPaM& rPam,
        /*[in]*/bool bSaveInUndo,
        /*[in]*/RedlineType nDelType) = 0;

    virtual bool DeleteRedline(
        /*[in]*/const SwStartNode& rSection,
        /*[in]*/bool bSaveInUndo,
        /*[in]*/RedlineType nDelType) = 0;

    virtual SwRedlineTable::size_type GetRedlinePos(
        /*[in]*/const SwNode& rNode,
        /*[in]*/RedlineType nType) const = 0;

    virtual SwRedlineTable::size_type GetRedlineEndPos(
        /*[in]*/ SwRedlineTable::size_type nStartPos,
        /*[in]*/ const SwNode& rNode,
        /*[in]*/ RedlineType nType) const = 0;

    virtual bool HasRedline(
        /*[in]*/const SwPaM& rPam,
        /*[in]*/RedlineType nType,
        /*[in]*/bool bStartOrEndInRange) const = 0;

    virtual void CompressRedlines(size_t nStartIndex = 0) = 0;

    virtual const SwRangeRedline* GetRedline(
        /*[in]*/const SwPosition& rPos,
        /*[in]*/SwRedlineTable::size_type* pFndPos) const = 0;

    virtual bool IsRedlineMove() const = 0;

    virtual void SetRedlineMove(/*[in]*/bool bFlag) = 0;

    virtual bool AcceptRedline(/*[in]*/ SwRedlineTable::size_type nPos, /*[in]*/ bool bCallDelete,
                               /*[in]*/ bool bRange = false)
        = 0;

    virtual bool AcceptRedline(/*[in]*/ const SwPaM& rPam, /*[in]*/ bool bCallDelete,
                               /*[in]*/ sal_Int8 nDepth = 0)
        = 0;

    virtual void AcceptRedlineParagraphFormatting(/*[in]*/const SwPaM& rPam ) = 0;

    virtual bool RejectRedline(/*[in]*/ SwRedlineTable::size_type nPos,
                               /*[in]*/ bool bCallDelete, /*[in]*/ bool bRange = false)
        = 0;

    virtual bool RejectRedline(/*[in]*/ const SwPaM& rPam, /*[in]*/ bool bCallDelete,
                               /*[in]*/ sal_Int8 nDepth = 0)
        = 0;

    virtual const SwRangeRedline* SelNextRedline(/*[in]*/SwPaM& rPam) const = 0;

    virtual const SwRangeRedline* SelPrevRedline(/*[in]*/SwPaM& rPam) const = 0;

    virtual void AcceptAllRedline(/*[in]*/bool bAcceptReject) = 0;

    // Representation has changed, invalidate all Redlines.
    virtual void UpdateRedlineAttr() = 0;

    // Create a new Author if required.
    virtual std::size_t GetRedlineAuthor() = 0;

    // For Readers etc.: register new Author in table.
    virtual std::size_t InsertRedlineAuthor(const OUString& rAuthor) = 0;

    // Place a comment at Redline at given position.
    virtual bool SetRedlineComment(
        /*[in]*/const SwPaM& rPam,
        /*[in]*/const OUString& rComment) = 0;

    virtual const css::uno::Sequence <sal_Int8>& GetRedlinePassword() const = 0;

    virtual void SetRedlinePassword(
        /*[in]*/const css::uno::Sequence <sal_Int8>& rNewPassword) = 0;

    virtual void UpdateRedlineContentNode(/*[in]*/ SwRedlineTable::size_type nStartPos,
                                          /*[in]*/ SwRedlineTable::size_type nEndPos) const = 0;

    virtual void dumpAsXml(xmlTextWriterPtr pWriter) const = 0;


protected:
     virtual ~IDocumentRedlineAccess() {};
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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

 #ifndef IDOCUMENTREDLINE_HXX_INCLUDED
 #define IDOCUMENTREDLINE_HXX_INCLUDED

 #include <sal/types.h>
 #include <tools/solar.h>

 #include <limits.h> // USHRT_MAX

 #include <com/sun/star/uno/Sequence.hxx>

 class SwRedline;
 class SwRedlineTbl;
 class SwPaM;
 struct SwPosition;
 class SwStartNode;
 class SwNode;

typedef sal_uInt16 RedlineMode_t;
namespace nsRedlineMode_t
{
    const RedlineMode_t REDLINE_NONE = 0; ///< no RedlineMode
    const RedlineMode_t REDLINE_ON = 0x01;///< RedlineMode on
    const RedlineMode_t REDLINE_IGNORE = 0x02;///< ignore Redlines
    const RedlineMode_t REDLINE_SHOW_INSERT = 0x10;///< show all inserts
    const RedlineMode_t REDLINE_SHOW_DELETE = 0x20;///< show all deletes
    const RedlineMode_t REDLINE_SHOW_MASK = REDLINE_SHOW_INSERT | REDLINE_SHOW_DELETE;

    // For internal management:
    // remove the original Redlines together with their content
    // (Clipboard/text modules).
    const RedlineMode_t REDLINE_DELETE_REDLINES = 0x100;
    // When deleting within a RedlineObject
    // ignore the DeleteRedline during Append.
    const RedlineMode_t REDLINE_IGNOREDELETE_REDLINES = 0x200;
    // don't combine any redlines. This flag may be only used in Undo.
    const RedlineMode_t REDLINE_DONTCOMBINE_REDLINES = 0x400;
}

typedef sal_uInt16 RedlineType_t;
namespace nsRedlineType_t
{
    // Range of RedlineTypes is 0 to 127.
    const RedlineType_t REDLINE_INSERT = 0x0;// Content has been inserted.
    const RedlineType_t REDLINE_DELETE = 0x1;// Content has been deleted.
    const RedlineType_t REDLINE_FORMAT = 0x2;// Attributes have been applied.
    const RedlineType_t REDLINE_TABLE = 0x3;// Table structure has been altered.
    const RedlineType_t REDLINE_FMTCOLL = 0x4;// Style has been altered (Autoformat!).

    // When larger than 128, flags can be inserted.
    const RedlineType_t REDLINE_NO_FLAG_MASK = 0x7F;
    const RedlineType_t REDLINE_FORM_AUTOFMT = 0x80;// Can be a flag in RedlineType.
}

/** IDocumentRedlineAccess
*/
class IDocumentRedlineAccess
{
     // Static helper functions
public:
    static bool IsShowChanges(const sal_uInt16 eM)
    { return (nsRedlineMode_t::REDLINE_SHOW_INSERT | nsRedlineMode_t::REDLINE_SHOW_DELETE) == (eM & nsRedlineMode_t::REDLINE_SHOW_MASK); }

    static bool IsHideChanges(const sal_uInt16 eM)
    { return nsRedlineMode_t::REDLINE_SHOW_INSERT == (eM & nsRedlineMode_t::REDLINE_SHOW_MASK); }

    static bool IsShowOriginal(const sal_uInt16 eM)
    { return nsRedlineMode_t::REDLINE_SHOW_DELETE == (eM & nsRedlineMode_t::REDLINE_SHOW_MASK); }

    static bool IsRedlineOn(const sal_uInt16 eM)
    { return nsRedlineMode_t::REDLINE_ON == (eM & (nsRedlineMode_t::REDLINE_ON | nsRedlineMode_t::REDLINE_IGNORE )); }

public:

    /*************************************************
        Query
    *************************************************/

    /** Query the currently set redline mode

        @returns
        the currently set redline mode
    */
     virtual RedlineMode_t GetRedlineMode() const = 0;

    /** Set a new redline mode.

        @param eMode
        [in] the new redline mode.
    */
    virtual void SetRedlineMode_intern(/*[in]*/RedlineMode_t eMode) = 0;

    /** Set a new redline mode.

        @param eMode
        [in] the new redline mode.
    */
    virtual void SetRedlineMode(/*[in]*/RedlineMode_t eMode) = 0;

    /** Query if redlining is on.

        @returns
        <TRUE/> if redlining is on <FALSE/> otherwise
    */
    virtual bool IsRedlineOn() const = 0;

    virtual bool IsIgnoreRedline() const = 0;

    virtual const SwRedlineTbl& GetRedlineTbl() const = 0;

    virtual bool IsInRedlines(const SwNode& rNode) const = 0;

    /***************************************************
        Manipulation
     ***************************************************/

    /** Append a new redline

        @param pPtr

        @param bCallDelete

        @returns
    */
    virtual bool AppendRedline(/*[in]*/SwRedline* pPtr, /*[in]*/bool bCallDelete) = 0;

    virtual bool SplitRedline(/*[in]*/const SwPaM& rPam) = 0;

    virtual bool DeleteRedline(
        /*[in]*/const SwPaM& rPam,
        /*[in]*/bool bSaveInUndo,
        /*[in]*/sal_uInt16 nDelType) = 0;

    virtual bool DeleteRedline(
        /*[in]*/const SwStartNode& rSection,
        /*[in]*/bool bSaveInUndo,
        /*[in]*/sal_uInt16 nDelType) = 0;

    virtual sal_uInt16 GetRedlinePos(
        /*[in]*/const SwNode& rNode,
        /*[in]*/sal_uInt16 nType) const = 0;

    virtual void CompressRedlines() = 0;

    virtual const SwRedline* GetRedline(
        /*[in]*/const SwPosition& rPos,
        /*[in]*/sal_uInt16* pFndPos) const = 0;

    virtual bool IsRedlineMove() const = 0;

    virtual void SetRedlineMove(/*[in]*/bool bFlag) = 0;

    virtual bool AcceptRedline(/*[in]*/sal_uInt16 nPos, /*[in]*/bool bCallDelete) = 0;

    virtual bool AcceptRedline(/*[in]*/const SwPaM& rPam, /*[in]*/bool bCallDelete) = 0;

    virtual bool RejectRedline(/*[in]*/sal_uInt16 nPos, /*[in]*/bool bCallDelete) = 0;

    virtual bool RejectRedline(/*[in]*/const SwPaM& rPam, /*[in]*/bool bCallDelete) = 0;

    virtual const SwRedline* SelNextRedline(/*[in]*/SwPaM& rPam) const = 0;

    virtual const SwRedline* SelPrevRedline(/*[in]*/SwPaM& rPam) const = 0;

    // Representation has changed, invalidate all Redlines.
    virtual void UpdateRedlineAttr() = 0;

    // Create a new Author if required.
    virtual sal_uInt16 GetRedlineAuthor() = 0;

    // For Readers etc.: register new Author in table.
    virtual sal_uInt16 InsertRedlineAuthor(const OUString& rAuthor) = 0;

    // Place a comment at Redline at given position.
    virtual bool SetRedlineComment(
        /*[in]*/const SwPaM& rPam,
        /*[in]*/const OUString& rComment) = 0;

    virtual const ::com::sun::star::uno::Sequence <sal_Int8>& GetRedlinePassword() const = 0;

    virtual void SetRedlinePassword(
        /*[in]*/const ::com::sun::star::uno::Sequence <sal_Int8>& rNewPassword) = 0;

 protected:
     virtual ~IDocumentRedlineAccess() {};
 };

 #endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

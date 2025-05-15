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

#include <sal/types.h>
#include "IMark.hxx"
#include <memory>
#include <optional>

class SwPaM;
struct SwPosition;
class SwTextNode;
class SwCursorShell;

namespace sw::mark {
    class AnnotationMark;
    class SaveBookmark; // FIXME: Ugly: SaveBookmark is a core-internal class, and should not be used in the interface
}

/** Provides access to the marks of a document.
*/
class IDocumentMarkAccess
{
    public:
        enum class MarkType
        {
            UNO_BOOKMARK,
            DDE_BOOKMARK,
            BOOKMARK,
            CROSSREF_HEADING_BOOKMARK,
            CROSSREF_NUMITEM_BOOKMARK,
            ANNOTATIONMARK,
            TEXT_FIELDMARK,
            CHECKBOX_FIELDMARK,
            DROPDOWN_FIELDMARK,
            DATE_FIELDMARK,
            NAVIGATOR_REMINDER
        };

        typedef std::vector<::sw::mark::MarkBase*>::const_iterator const_iterator;
        typedef std::vector<::sw::mark::MarkBase*>::iterator iterator;

        /// To avoid recursive calls of deleteMark, the removal of dummy
        /// characters of fieldmarks has to be delayed; this is the baseclass
        /// that can be subclassed for that purpose.
        struct ILazyDeleter { virtual ~ILazyDeleter() { } };

        /** Generates a new mark in the document for a certain selection.

           @param rPaM
           [in] the selection being marked.

           @param rProposedName
           [in] the proposed name of the new mark.

           @param eMark
           [in] the type of the new mark.

           @param eMode
           [in] is the new mark part of a text copy operation

           @returns
           a pointer to the new mark (name might have changed).
        */
        virtual ::sw::mark::MarkBase* makeMark(const SwPaM& rPaM,
            const SwMarkName& rProposedName,
            MarkType eMark, ::sw::mark::InsertMode eMode,
            SwPosition const* pSepPos = nullptr) = 0;

        virtual sw::mark::Fieldmark* makeFieldBookmark( const SwPaM& rPaM,
            const SwMarkName& rName,
            const OUString& rType,
            SwPosition const* pSepPos = nullptr) = 0;
        virtual sw::mark::Fieldmark* makeNoTextFieldBookmark( const SwPaM& rPaM,
            const SwMarkName& rName,
            const OUString& rType) = 0;

        virtual sw::mark::MarkBase* makeAnnotationMark(
            const SwPaM& rPaM,
            const SwMarkName& rName ) = 0;

        /** A performance optimization mode

           Creating and inserting a lot of marks, the checks in makeMark if name is unique may
           become a bottleneck, because there we have to iterate over all marks, checking their
           names, which creates a quadratic complexity. This may e.g. slow down loading documents
           with thousands of bookmarks.

           When the check is disabled using disableUniqueNameChecks, duplicate names are allowed.
           When the check is eventually enabled using enableUniqueNameChecks, one pass over all
           marks is performed, and all duplicated names are made unique.
        */
        virtual void disableUniqueNameChecks() = 0;
        virtual void enableUniqueNameChecks() = 0;

        /** Returns a mark in the document for a paragraph.
            If there is none, a mark will be created.

           @param rTextNode
           [in] the paragraph being marked (a selection over the paragraph is marked)

           @param eMark
           [in] the type of the new mark.

           @returns
           a pointer to the new mark (name might have changed).
        */
        virtual ::sw::mark::MarkBase* getMarkForTextNode(const SwTextNode& rTextNode,
            MarkType eMark) =0;

        /** Moves an existing mark to a new selection and performs needed updates.
            @param io_pMark
            [in/out] the mark to be moved

            @param rPaM
            [in] new selection to be marked
        */

        virtual void repositionMark(::sw::mark::MarkBase* io_pMark,
            const SwPaM& rPaM) =0;

        /** Renames an existing Mark, if possible.
            @param io_pMark
            [in/out] the mark to be renamed

            @param rNewName
            [in] new name for the mark

            @returns false, if renaming failed (because the name is already in use)
        */
        virtual bool renameMark(::sw::mark::MarkBase* io_pMark,
                const SwMarkName& rNewName) =0;

        /** Corrects marks (absolute)
            This method ignores the previous position of the mark in the paragraph

            @param rOldNode
            [in] the node from which nodes should be moved

            @param rNewPos
            [in] new position to which marks will be moved, if nOffset == 0

            @param nOffset
            [in] the offset by which the mark gets positioned of rNewPos
        */
        virtual void correctMarksAbsolute(const SwNode& rOldNode,
            const SwPosition& rNewPos,
            const sal_Int32 nOffset) =0;

        /** Corrects marks (relative)
            This method uses the previous position of the mark in the paragraph as offset

            @param rOldNode
            [in] the node from which nodes should be moved

            @param rNewPos
            [in] new position to which marks from the start of the paragraph will be
                 moved, if nOffset == 0

            @param nOffset
            [in] the offset by which the mark gets positioned of rNewPos in addition to
                 its old position in the paragraph
        */
        virtual void correctMarksRelative(const SwNode& rOldNode,
            const SwPosition& rNewPos,
            const sal_Int32 nOffset) =0;

        /** Deletes marks in a range

            Note: navigator reminders are excluded

        */
        virtual void deleteMarks(
            const SwNode& rStt,
            const SwNode& rEnd,
            std::vector< ::sw::mark::SaveBookmark>* pSaveBkmk, // Ugly: SaveBookmark is core-internal
            std::optional<sal_Int32> oStartContentIdx,
            std::optional<sal_Int32> oEndContentIdx,
            bool isReplace) = 0;

        /** Deletes a mark.

            @param ppMark
            [in] an iterator pointing to the Mark to be deleted.
            @param isMoveNodes
            [in] the mark is deleted only temporarily for a node move, do not
                 remove fieldmark chars.
        */
        virtual std::unique_ptr<ILazyDeleter>
            deleteMark(const IDocumentMarkAccess::const_iterator& ppMark, bool isMoveNodes) =0;

        /** Deletes a mark.

            @param ppMark
            [in] the name of the mark to be deleted.
        */
        virtual void deleteMark(const ::sw::mark::MarkBase* const pMark) =0;

        /** Clear (deletes) all marks.
        */
        virtual void clearAllMarks() =0;

        virtual void assureSortedMarkContainers() const = 0;

        /**
         * called when we need to sort a sub-range of the container, elements starting
         * at nMinIndexModified where modified. This is used from ContentIdxStoreImpl::RestoreBkmks,
         * where we are only modifying a small range at the end of the container.
         */
        virtual void assureSortedMarkContainers(sal_Int32 nMinIndexModified) const = 0;

        /** returns a STL-like random access iterator to the begin of the sequence of marks.
        */
        virtual const_iterator getAllMarksBegin() const =0;

        /** returns a STL-like random access iterator to the end of the sequence of marks.
        */
        virtual const_iterator getAllMarksEnd() const =0;

        /** returns the number of marks.

            Note: annotation marks are excluded
        */
        virtual sal_Int32 getAllMarksCount() const =0;

        /** Finds a mark by name.

            @param rName
            [in] the name of the mark to find.

            @returns
            an iterator pointing to the mark, or pointing to getAllMarksEnd() if nothing was found.
        */
        virtual const_iterator findMark(const SwMarkName& rMark) const =0;

        /** Find the first Bookmark that does not start before.

            @returns
            an iterator pointing to the mark, or pointing to getAllMarksEnd() if nothing was found.
        */
        virtual std::vector<sw::mark::Bookmark*>::const_iterator findFirstBookmarkNotStartsBefore(const SwPosition& rPos) const =0;

        // interface Bookmarks (BOOKMARK, CROSSREF_NUMITEM_BOOKMARK, CROSSREF_HEADING_BOOKMARK )

        /** check if the selection would delete a BOOKMARK */
        virtual bool isBookmarkDeleted(SwPaM const& rPaM, bool isReplace) const =0;

        /** returns a STL-like random access iterator to the begin of the sequence the Bookmarks.
        */
        virtual std::vector<sw::mark::Bookmark*>::const_iterator getBookmarksBegin() const =0;

        /** returns a STL-like random access iterator to the end of the sequence of Bookmarks.
        */
        virtual std::vector<sw::mark::Bookmark*>::const_iterator getBookmarksEnd() const =0;

        /** returns the number of Bookmarks.
        */
        virtual sal_Int32 getBookmarksCount() const =0;

        /** Finds a bookmark by name.

            @param rName
            [in] the name of the bookmark to find.

            @returns
            an iterator pointing to the bookmark, or getBookmarksEnd() if nothing was found.
        */
        virtual std::vector<sw::mark::Bookmark*>::const_iterator findBookmark(const SwMarkName& rMark) const =0;

        /** Finds the first mark that is starting after.

            @returns
            an iterator pointing to the mark, or pointing to getBookmarksEnd() if nothing was found.
        */
        virtual std::vector<sw::mark::Bookmark*>::const_iterator findFirstBookmarkStartsAfter(const SwPosition& rPos) const =0;

        /// Get the innermost bookmark that contains rPos.
        virtual sw::mark::Bookmark* getOneInnermostBookmarkFor(const SwPosition& rPos) const = 0;

        // Fieldmarks
        /** returns a STL-like random access iterator to the begin of the sequence of fieldmarks.
        */
        virtual std::vector<sw::mark::Fieldmark*>::const_iterator getFieldmarksBegin() const =0;

        /** returns a STL-like random access iterator to the end of the sequence of fieldmarks.
        */
        virtual std::vector<sw::mark::Fieldmark*>::const_iterator getFieldmarksEnd() const =0;

        /// returns the number of Fieldmarks.
        virtual sal_Int32 getFieldmarksCount() const = 0;

        /// get Fieldmark for CH_TXT_ATR_FIELDSTART/CH_TXT_ATR_FIELDEND at rPos
        virtual ::sw::mark::Fieldmark* getFieldmarkAt(const SwPosition& rPos) const =0;
        virtual sw::mark::Fieldmark* getInnerFieldmarkFor(const SwPosition& pos) const = 0;
        virtual sw::mark::Fieldmark* getFieldmarkBefore(const SwPosition& pos, bool bLoop) const =0;
        virtual sw::mark::Fieldmark* getFieldmarkAfter(const SwPosition& pos, bool bLoop) const =0;

        virtual ::sw::mark::Fieldmark* getDropDownFor(const SwPosition& pos) const=0;
        virtual std::vector<::sw::mark::Fieldmark*> getNoTextFieldmarksIn(const SwPaM &rPaM) const=0;

        virtual void deleteFieldmarkAt(const SwPosition& rPos) = 0;
        virtual ::sw::mark::Fieldmark* changeFormFieldmarkType(::sw::mark::Fieldmark* pFieldmark, const OUString& rNewType) = 0;

        virtual void NotifyCursorUpdate(const SwCursorShell& rCursorShell) = 0;
        virtual void ClearFieldActivation() = 0;

        // Annotation Marks
        virtual std::vector<sw::mark::AnnotationMark*>::const_iterator getAnnotationMarksBegin() const = 0;
        virtual std::vector<sw::mark::AnnotationMark*>::const_iterator getAnnotationMarksEnd() const = 0;
        virtual sal_Int32 getAnnotationMarksCount() const = 0;
        virtual std::vector<sw::mark::AnnotationMark*>::const_iterator findAnnotationMark( const SwMarkName& rName ) const = 0;
        virtual sw::mark::AnnotationMark* getAnnotationMarkFor(const SwPosition& rPosition) const = 0;
        // handle and restore text ranges of annotations of tracked deletions
        // based on the helper bookmarks (which can survive I/O and hiding redlines)
        virtual ::sw::mark::Bookmark* makeAnnotationBookmark(const SwPaM& rPaM,
            const SwMarkName& rProposedName,
            ::sw::mark::InsertMode eMode,
            SwPosition const* pSepPos = nullptr) = 0;
        /** Find the first AnnotationMark that does not start before.

            @returns
            an iterator pointing to the mark, or pointing to getAnnotationMarksEnd() if nothing was found.
        */
        virtual std::vector<sw::mark::AnnotationMark*>::const_iterator findFirstAnnotationMarkNotStartsBefore(const SwPosition& rPos) const =0;
        virtual std::vector<sw::mark::AnnotationMark*>::const_iterator findFirstAnnotationMarkNotStartsBefore(const SwNode& rPos) const =0;
        virtual std::vector<sw::mark::Bookmark*>::const_iterator findAnnotationBookmark( const SwMarkName& rName ) const = 0;
        virtual void restoreAnnotationMarks(bool bDelete = true) = 0;

        /** Returns the MarkType used to create the mark
        */
        static SW_DLLPUBLIC MarkType GetType(const ::sw::mark::MarkBase& rMark);

        static SW_DLLPUBLIC OUString GetCrossRefHeadingBookmarkNamePrefix();
        static SW_DLLPUBLIC bool IsLegalPaMForCrossRefHeadingBookmark( const SwPaM& rPaM );
        static void DeleteFieldmarkCommand(::sw::mark::Fieldmark const& rMark);

    protected:
        virtual ~IDocumentMarkAccess() {};
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

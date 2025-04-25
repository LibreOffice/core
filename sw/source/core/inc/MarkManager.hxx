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

#ifndef INCLUDED_SW_SOURCE_CORE_INC_MARKMANAGER_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_MARKMANAGER_HXX

#include <IMark.hxx>
#include <IDocumentMarkAccess.hxx>
#include <unordered_map>
#include <memory>

class SwCursorShell;
class SfxViewShell;

namespace sw::mark {
    typedef std::unordered_map<SwMarkName, sal_Int32> MarkBasenameMapUniqueOffset_t;

    class AnnotationMark;
    class FieldmarkWithDropDownButton;

    class MarkManager final
        : virtual public IDocumentMarkAccess
    {
        public:
            MarkManager(/*[in/out]*/ SwDoc& rDoc);
            // IDocumentMarkAccess
            virtual ::sw::mark::MarkBase* makeMark(const SwPaM& rPaM,
                const SwMarkName& rName, IDocumentMarkAccess::MarkType eMark,
                sw::mark::InsertMode eMode,
                SwPosition const* pSepPos = nullptr) override;

            virtual sw::mark::Fieldmark* makeFieldBookmark( const SwPaM& rPaM,
                const SwMarkName& rName,
                const OUString& rType,
                SwPosition const* pSepPos = nullptr) override;
            virtual sw::mark::Fieldmark* makeNoTextFieldBookmark( const SwPaM& rPaM,
                const SwMarkName& rName,
                const OUString& rType) override;

            virtual ::sw::mark::MarkBase* getMarkForTextNode(const SwTextNode& rTextNode, IDocumentMarkAccess::MarkType eMark) override;

            virtual sw::mark::MarkBase* makeAnnotationMark(
                const SwPaM& rPaM,
                const SwMarkName& rName ) override;

            virtual void repositionMark(::sw::mark::MarkBase* io_pMark, const SwPaM& rPaM) override;
            virtual bool renameMark(::sw::mark::MarkBase* io_pMark, const SwMarkName& rNewName) override;
            virtual void correctMarksAbsolute(const SwNode& rOldNode, const SwPosition& rNewPos, const sal_Int32 nOffset) override;
            virtual void correctMarksRelative(const SwNode& rOldNode, const SwPosition& rNewPos, const sal_Int32 nOffset) override;

            virtual void deleteMarks(const SwNode& rStt,
                                    const SwNode& rEnd,
                                    std::vector< ::sw::mark::SaveBookmark>* pSaveBkmk,
                                    std::optional<sal_Int32> oStartContentIdx,
                                    std::optional<sal_Int32> oEndContentIdx,
                                    bool isReplace) override;

            // deleters
            virtual std::unique_ptr<ILazyDeleter>
                deleteMark(const const_iterator& ppMark, bool isMoveNodes) override;
            virtual void deleteMark(const ::sw::mark::MarkBase* const pMark) override;
            virtual void clearAllMarks() override;

            // marks
            virtual const_iterator getAllMarksBegin() const override;
            virtual const_iterator getAllMarksEnd() const override;
            virtual sal_Int32 getAllMarksCount() const override;
            virtual const_iterator findMark(const SwMarkName& rName) const override;
            virtual std::vector<sw::mark::Bookmark*>::const_iterator findFirstBookmarkNotStartsBefore(const SwPosition& rPos) const override;

            // bookmarks
            virtual bool isBookmarkDeleted(SwPaM const& rPaM, bool isReplace) const override;
            virtual std::vector<sw::mark::Bookmark*>::const_iterator getBookmarksBegin() const override;
            virtual std::vector<sw::mark::Bookmark*>::const_iterator getBookmarksEnd() const override;
            virtual sal_Int32 getBookmarksCount() const override;
            virtual std::vector<sw::mark::Bookmark*>::const_iterator findBookmark(const SwMarkName& rName) const override;
            virtual std::vector<sw::mark::Bookmark*>::const_iterator findFirstBookmarkStartsAfter(const SwPosition& rPos) const override;
            virtual ::sw::mark::Bookmark* getOneInnermostBookmarkFor(const SwPosition& rPos) const override;

            // Fieldmarks
            virtual std::vector<Fieldmark*>::const_iterator getFieldmarksBegin() const override;
            virtual std::vector<Fieldmark*>::const_iterator getFieldmarksEnd() const override;
            virtual sal_Int32 getFieldmarksCount() const override;
            virtual ::sw::mark::Fieldmark* getFieldmarkAt(const SwPosition& rPos) const override;
            virtual sw::mark::Fieldmark* getInnerFieldmarkFor(const SwPosition& rPos) const override;
            virtual sw::mark::Fieldmark* getFieldmarkBefore(const SwPosition& rPos, bool bLoop) const override;
            virtual sw::mark::Fieldmark* getFieldmarkAfter(const SwPosition& rPos, bool bLoop) const override;

            virtual ::sw::mark::Fieldmark* getDropDownFor(const SwPosition &rPos) const override;
            virtual std::vector<::sw::mark::Fieldmark*> getNoTextFieldmarksIn(const SwPaM &rPaM) const override;

            virtual void deleteFieldmarkAt(const SwPosition& rPos) override;
            virtual ::sw::mark::Fieldmark* changeFormFieldmarkType(::sw::mark::Fieldmark* pFieldmark, const OUString& rNewType) override;

            virtual void NotifyCursorUpdate(const SwCursorShell& rCursorShell) override;
            virtual void ClearFieldActivation() override;
            void LOKUpdateActiveField(const SfxViewShell* pViewShell);

            void dumpAsXml(xmlTextWriterPtr pWriter) const;

            // Annotation Marks
            virtual std::vector<sw::mark::AnnotationMark*>::const_iterator getAnnotationMarksBegin() const override;
            virtual std::vector<sw::mark::AnnotationMark*>::const_iterator getAnnotationMarksEnd() const override;
            virtual sal_Int32 getAnnotationMarksCount() const override;
            virtual std::vector<sw::mark::AnnotationMark*>::const_iterator findAnnotationMark( const SwMarkName& rName ) const override;
            virtual sw::mark::AnnotationMark* getAnnotationMarkFor(const SwPosition& rPos) const override;

            virtual void assureSortedMarkContainers() const override;
            virtual void assureSortedMarkContainers(sal_Int32 nMinIndexModified) const override;

            typedef std::vector<sw::mark::MarkBase*> container_t;

            // helper bookmark to store annotation range of redlines
            virtual ::sw::mark::Bookmark* makeAnnotationBookmark(const SwPaM& rPaM,
                const SwMarkName& rName,
                sw::mark::InsertMode eMode,
                SwPosition const* pSepPos = nullptr) override;
            virtual std::vector<sw::mark::AnnotationMark*>::const_iterator findFirstAnnotationMarkNotStartsBefore(const SwPosition& rPos) const override;
            virtual std::vector<sw::mark::AnnotationMark*>::const_iterator findFirstAnnotationMarkNotStartsBefore(const SwNode& rPos) const override;
            virtual std::vector<sw::mark::Bookmark*>::const_iterator findAnnotationBookmark( const SwMarkName& rName ) const override;
            virtual void restoreAnnotationMarks(bool bDelete = true) override;

        private:

            MarkManager(MarkManager const&) = delete;
            MarkManager& operator=(MarkManager const&) = delete;

            // make names
            SwMarkName getUniqueMarkName(const SwMarkName& rName) const;

            void sortSubsetMarks();
            void sortMarks();

            // container for all marks, this container owns the objects it points to
            container_t m_vAllMarks;

            // additional container for bookmarks
            std::vector<sw::mark::Bookmark*> m_vBookmarks;
            // additional container for fieldmarks
            std::vector<sw::mark::Fieldmark*> m_vFieldmarks;

            mutable MarkBasenameMapUniqueOffset_t m_aMarkBasenameMapUniqueOffset;

            // container for annotation marks
            std::vector<sw::mark::AnnotationMark*> m_vAnnotationMarks;

            SwDoc& m_rDoc;

            sw::mark::FieldmarkWithDropDownButton* m_pLastActiveFieldmark;
    };

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

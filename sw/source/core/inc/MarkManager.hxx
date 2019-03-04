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
#include <unordered_set>
#include <unordered_map>
#include <memory>

class SwCursorShell;

namespace sw {
    namespace mark {
    typedef std::unordered_map<OUString, sal_Int32> MarkBasenameMapUniqueOffset_t;

    class DropDownFieldmark;

    class MarkManager
        : virtual public IDocumentMarkAccess
    {
        public:
            MarkManager(/*[in/out]*/ SwDoc& rDoc);
            // IDocumentMarkAccess
            virtual ::sw::mark::IMark* makeMark(const SwPaM& rPaM,
                const OUString& rName, IDocumentMarkAccess::MarkType eMark,
                sw::mark::InsertMode eMode) override;

            virtual sw::mark::IFieldmark* makeFieldBookmark( const SwPaM& rPaM,
                const OUString& rName,
                const OUString& rType) override;
            virtual sw::mark::IFieldmark* makeNoTextFieldBookmark( const SwPaM& rPaM,
                const OUString& rName,
                const OUString& rType) override;

            virtual ::sw::mark::IMark* getMarkForTextNode(const SwTextNode& rTextNode, IDocumentMarkAccess::MarkType eMark) override;

            virtual sw::mark::IMark* makeAnnotationMark(
                const SwPaM& rPaM,
                const OUString& rName ) override;

            virtual void repositionMark(::sw::mark::IMark* io_pMark, const SwPaM& rPaM) override;
            virtual bool renameMark(::sw::mark::IMark* io_pMark, const OUString& rNewName) override;
            virtual void correctMarksAbsolute(const SwNodeIndex& rOldNode, const SwPosition& rNewPos, const sal_Int32 nOffset) override;
            virtual void correctMarksRelative(const SwNodeIndex& rOldNode, const SwPosition& rNewPos, const sal_Int32 nOffset) override;

            virtual void deleteMarks(const SwNodeIndex& rStt, const SwNodeIndex& rEnd, std::vector< ::sw::mark::SaveBookmark>* pSaveBkmk, const SwIndex* pSttIdx, const SwIndex* pEndIdx) override;

            // deleters
            virtual std::shared_ptr<ILazyDeleter>
                deleteMark(const const_iterator_t& ppMark) override;
            virtual void deleteMark(const ::sw::mark::IMark* const pMark) override;
            virtual void clearAllMarks() override;

            // marks
            virtual const_iterator_t getAllMarksBegin() const override;
            virtual const_iterator_t getAllMarksEnd() const override;
            virtual sal_Int32 getAllMarksCount() const override;
            virtual const_iterator_t findMark(const OUString& rName) const override;

            // bookmarks
            virtual const_iterator_t getBookmarksBegin() const override;
            virtual const_iterator_t getBookmarksEnd() const override;
            virtual sal_Int32 getBookmarksCount() const override;
            virtual const_iterator_t findBookmark(const OUString& rName) const override;

            // Fieldmarks
            virtual ::sw::mark::IFieldmark* getFieldmarkFor(const SwPosition& rPos) const override;
            virtual ::sw::mark::IFieldmark* getFieldmarkBefore(const SwPosition& rPos) const override;
            virtual ::sw::mark::IFieldmark* getFieldmarkAfter(const SwPosition& rPos) const override;

            virtual ::sw::mark::IFieldmark* getDropDownFor(const SwPosition &rPos) const override;
            virtual std::vector< ::sw::mark::IFieldmark* > getDropDownsFor(const SwPaM &rPaM) const override;

            virtual void deleteFieldmarkAt(const SwPosition& rPos) override;
            virtual ::sw::mark::IFieldmark* changeNonTextFieldmarkType(::sw::mark::IFieldmark* pFieldmark, const OUString& rNewType) override;

            virtual void NotifyCursorUpdate(const SwCursorShell& rCursorShell) override;
            virtual void ClearFieldActivation() override;

            void dumpAsXml(xmlTextWriterPtr pWriter) const;

            // Annotation Marks
            virtual const_iterator_t getAnnotationMarksBegin() const override;
            virtual const_iterator_t getAnnotationMarksEnd() const override;
            virtual sal_Int32 getAnnotationMarksCount() const override;
            virtual const_iterator_t findAnnotationMark( const OUString& rName ) const override;
            virtual sw::mark::IMark* getAnnotationMarkFor(const SwPosition& rPos) const override;

            virtual void assureSortedMarkContainers() const override;

        private:

            MarkManager(MarkManager const&) = delete;
            MarkManager& operator=(MarkManager const&) = delete;

            // make names
            OUString getUniqueMarkName(const OUString& rName) const;
        public: // FIXME should be private, needs refactor
            void sortMarks();
        private:
            void sortSubsetMarks();

            // container for all marks
            container_t m_vAllMarks;

            // additional container for bookmarks
            container_t m_vBookmarks;
            // additional container for fieldmarks
            container_t m_vFieldmarks;

            mutable MarkBasenameMapUniqueOffset_t m_aMarkBasenameMapUniqueOffset;

            // container for annotation marks
            container_t m_vAnnotationMarks;

            SwDoc * const m_pDoc;

            sw::mark::DropDownFieldmark* m_pLastActiveFieldmark;
    };
    } // namespace mark
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

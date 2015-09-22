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
#include <boost/noncopyable.hpp>

namespace sw {
    namespace mark {
    typedef std::unordered_map<OUString, sal_Int32, OUStringHash> MarkBasenameMapUniqueOffset_t;

    class MarkManager
        : private ::boost::noncopyable
        , virtual public IDocumentMarkAccess
    {
        public:
            MarkManager(/*[in/out]*/ SwDoc& rDoc);
            // IDocumentMarkAccess
            virtual ::sw::mark::IMark* makeMark(const SwPaM& rPaM, const OUString& rName, IDocumentMarkAccess::MarkType eMark) SAL_OVERRIDE;

            virtual sw::mark::IFieldmark* makeFieldBookmark( const SwPaM& rPaM,
                const OUString& rName,
                const OUString& rType) SAL_OVERRIDE;
            virtual sw::mark::IFieldmark* makeNoTextFieldBookmark( const SwPaM& rPaM,
                const OUString& rName,
                const OUString& rType) SAL_OVERRIDE;

            virtual ::sw::mark::IMark* getMarkForTextNode(const SwTextNode& rTextNode, IDocumentMarkAccess::MarkType eMark) SAL_OVERRIDE;

            virtual sw::mark::IMark* makeAnnotationMark(
                const SwPaM& rPaM,
                const OUString& rName ) SAL_OVERRIDE;

            virtual void repositionMark(::sw::mark::IMark* io_pMark, const SwPaM& rPaM) SAL_OVERRIDE;
            virtual bool renameMark(::sw::mark::IMark* io_pMark, const OUString& rNewName) SAL_OVERRIDE;
            virtual void correctMarksAbsolute(const SwNodeIndex& rOldNode, const SwPosition& rNewPos, const sal_Int32 nOffset) SAL_OVERRIDE;
            virtual void correctMarksRelative(const SwNodeIndex& rOldNode, const SwPosition& rNewPos, const sal_Int32 nOffset) SAL_OVERRIDE;

            virtual void deleteMarks(const SwNodeIndex& rStt, const SwNodeIndex& rEnd, ::std::vector< ::sw::mark::SaveBookmark>* pSaveBkmk, const SwIndex* pSttIdx, const SwIndex* pEndIdx) SAL_OVERRIDE;

            // deleters
            virtual std::shared_ptr<ILazyDeleter>
                deleteMark(const const_iterator_t& ppMark) SAL_OVERRIDE;
            virtual void deleteMark(const ::sw::mark::IMark* const pMark) SAL_OVERRIDE;
            virtual void clearAllMarks() SAL_OVERRIDE;

            // marks
            virtual const_iterator_t getAllMarksBegin() const SAL_OVERRIDE;
            virtual const_iterator_t getAllMarksEnd() const SAL_OVERRIDE;
            virtual sal_Int32 getAllMarksCount() const SAL_OVERRIDE;
            virtual const_iterator_t findMark(const OUString& rName) const SAL_OVERRIDE;

            // bookmarks
            virtual const_iterator_t getBookmarksBegin() const SAL_OVERRIDE;
            virtual const_iterator_t getBookmarksEnd() const SAL_OVERRIDE;
            virtual sal_Int32 getBookmarksCount() const SAL_OVERRIDE;
            virtual const_iterator_t findBookmark(const OUString& rName) const SAL_OVERRIDE;

            // Fieldmarks
            virtual ::sw::mark::IFieldmark* getFieldmarkFor(const SwPosition& rPos) const SAL_OVERRIDE;
            virtual ::sw::mark::IFieldmark* getFieldmarkBefore(const SwPosition& rPos) const SAL_OVERRIDE;
            virtual ::sw::mark::IFieldmark* getFieldmarkAfter(const SwPosition& rPos) const SAL_OVERRIDE;

            virtual ::sw::mark::IFieldmark* getDropDownFor(const SwPosition &rPos) const SAL_OVERRIDE;
            virtual std::vector< ::sw::mark::IFieldmark* > getDropDownsFor(const SwPaM &rPaM) const SAL_OVERRIDE;

            void dumpAsXml(struct _xmlTextWriter* pWriter) const;

            // Annotation Marks
            virtual const_iterator_t getAnnotationMarksBegin() const SAL_OVERRIDE;
            virtual const_iterator_t getAnnotationMarksEnd() const SAL_OVERRIDE;
            virtual sal_Int32 getAnnotationMarksCount() const SAL_OVERRIDE;
            virtual const_iterator_t findAnnotationMark( const OUString& rName ) const SAL_OVERRIDE;
            virtual sw::mark::IMark* getAnnotationMarkFor(const SwPosition& rPos) const SAL_OVERRIDE;

            virtual void assureSortedMarkContainers() const SAL_OVERRIDE;

        private:
            // make names
            OUString getUniqueMarkName(const OUString& rName) const;
            void sortMarks();
            void sortSubsetMarks();

            // container for all marks
            container_t m_vAllMarks;

            // additional container for bookmarks
            container_t m_vBookmarks;
            // additional container for fieldmarks
            container_t m_vFieldmarks;

            std::unordered_set<OUString, OUStringHash> m_aMarkNamesSet;
            mutable MarkBasenameMapUniqueOffset_t m_aMarkBasenameMapUniqueOffset;

            // container for annotation marks
            container_t m_vAnnotationMarks;

            // container for all marks except annotation marks
            container_t m_vCommonMarks;

            SwDoc * const m_pDoc;
    };
    } // namespace mark
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

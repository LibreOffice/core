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
#include <boost/unordered_set.hpp>
#include <boost/unordered_map.hpp>

namespace sw {
    namespace mark {
    typedef boost::unordered_map<OUString, sal_Int32, OUStringHash> MarkBasenameMapUniqueOffset_t;

    class MarkManager
        : private ::boost::noncopyable
        , virtual public IDocumentMarkAccess
    {
        public:
            MarkManager(/*[in/out]*/ SwDoc& rDoc);
            // IDocumentMarkAccess
            virtual ::sw::mark::IMark* makeMark(const SwPaM& rPaM, const OUString& rName, IDocumentMarkAccess::MarkType eMark);

            virtual sw::mark::IFieldmark* makeFieldBookmark( const SwPaM& rPaM,
                const OUString& rName,
                const OUString& rType);
            virtual sw::mark::IFieldmark* makeNoTextFieldBookmark( const SwPaM& rPaM,
                const OUString& rName,
                const OUString& rType);

            virtual ::sw::mark::IMark* getMarkForTxtNode(const SwTxtNode& rTxtNode, IDocumentMarkAccess::MarkType eMark);

            virtual sw::mark::IMark* makeAnnotationMark(
                const SwPaM& rPaM,
                const ::rtl::OUString& rName );

            virtual void repositionMark(::sw::mark::IMark* io_pMark, const SwPaM& rPaM);
            virtual bool renameMark(::sw::mark::IMark* io_pMark, const OUString& rNewName);
            virtual void correctMarksAbsolute(const SwNodeIndex& rOldNode, const SwPosition& rNewPos, const sal_Int32 nOffset);
            virtual void correctMarksRelative(const SwNodeIndex& rOldNode, const SwPosition& rNewPos, const sal_Int32 nOffset);

            virtual void deleteMarks(const SwNodeIndex& rStt, const SwNodeIndex& rEnd, ::std::vector< ::sw::mark::SaveBookmark>* pSaveBkmk, const SwIndex* pSttIdx, const SwIndex* pEndIdx);

            // deleters
            virtual ::boost::shared_ptr<ILazyDeleter>
                deleteMark(const const_iterator_t ppMark);
            virtual void deleteMark(const ::sw::mark::IMark* const pMark);
            virtual void clearAllMarks();

            // marks
            virtual const_iterator_t getAllMarksBegin() const;
            virtual const_iterator_t getAllMarksEnd() const;
            virtual sal_Int32 getAllMarksCount() const;
            virtual const_iterator_t findMark(const OUString& rName) const;
            virtual bool hasMark(const OUString& rName) const;

            // bookmarks
            virtual const_iterator_t getBookmarksBegin() const;
            virtual const_iterator_t getBookmarksEnd() const;
            virtual sal_Int32 getBookmarksCount() const;
            virtual const_iterator_t findBookmark(const OUString& rName) const;

            // Fieldmarks
            virtual ::sw::mark::IFieldmark* getFieldmarkFor(const SwPosition& rPos) const;
            virtual ::sw::mark::IFieldmark* getFieldmarkBefore(const SwPosition& rPos) const;
            virtual ::sw::mark::IFieldmark* getFieldmarkAfter(const SwPosition& rPos) const;

            void dumpAsXml(xmlTextWriterPtr w);

            // Marks exclusive annotation marks
            virtual const_iterator_t getCommonMarksBegin() const;
            virtual const_iterator_t getCommonMarksEnd() const;
            virtual sal_Int32 getCommonMarksCount() const;

            // Annotation Marks
            virtual const_iterator_t getAnnotationMarksBegin() const;
            virtual const_iterator_t getAnnotationMarksEnd() const;
            virtual sal_Int32 getAnnotationMarksCount() const;
            virtual const_iterator_t findAnnotationMark( const ::rtl::OUString& rName ) const;

            virtual void assureSortedMarkContainers() const;

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

            boost::unordered_set<OUString, OUStringHash> m_aMarkNamesSet;
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

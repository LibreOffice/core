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

#ifndef _SW_BOOKMARK_MARKMANAGER_HXX
#define _SW_BOOKMARK_MARKMANAGER_HXX

#include <IMark.hxx>
#include <IDocumentMarkAccess.hxx>
#include <boost/unordered_set.hpp>
#include <boost/unordered_map.hpp>

namespace sw {
    namespace mark {
    typedef boost::unordered_map<rtl::OUString, sal_Int32, rtl::OUStringHash> MarkBasenameMapUniqueOffset_t;

    class MarkManager
        : private ::boost::noncopyable
        , virtual public IDocumentMarkAccess
    {
        public:
            MarkManager(/*[in/out]*/ SwDoc& rDoc);
            // IDocumentMarkAccess
            virtual ::sw::mark::IMark* makeMark(const SwPaM& rPaM, const ::rtl::OUString& rName, IDocumentMarkAccess::MarkType eMark);

            virtual sw::mark::IFieldmark* makeFieldBookmark( const SwPaM& rPaM,
                const rtl::OUString& rName,
                const rtl::OUString& rType);
            virtual sw::mark::IFieldmark* makeNoTextFieldBookmark( const SwPaM& rPaM,
                const rtl::OUString& rName,
                const rtl::OUString& rType);

            virtual ::sw::mark::IMark* getMarkForTxtNode(const SwTxtNode& rTxtNode, IDocumentMarkAccess::MarkType eMark);

            virtual void repositionMark(::sw::mark::IMark* io_pMark, const SwPaM& rPaM);
            virtual bool renameMark(::sw::mark::IMark* io_pMark, const ::rtl::OUString& rNewName);
            virtual void correctMarksAbsolute(const SwNodeIndex& rOldNode, const SwPosition& rNewPos, const xub_StrLen nOffset);
            virtual void correctMarksRelative(const SwNodeIndex& rOldNode, const SwPosition& rNewPos, const xub_StrLen nOffset);

            virtual void deleteMarks(const SwNodeIndex& rStt, const SwNodeIndex& rEnd, ::std::vector< ::sw::mark::SaveBookmark>* pSaveBkmk, const SwIndex* pSttIdx, const SwIndex* pEndIdx);

            // deleters
            virtual void deleteMark(const const_iterator_t ppMark);
            virtual void deleteMark(const ::sw::mark::IMark* const pMark);
            virtual void clearAllMarks();

            // marks
            virtual const_iterator_t getMarksBegin() const;
            virtual const_iterator_t getMarksEnd() const;
            virtual sal_Int32 getMarksCount() const;
            virtual const_iterator_t findMark(const ::rtl::OUString& rName) const;
            virtual bool hasMark(const ::rtl::OUString& rName) const;

            // bookmarks
            virtual const_iterator_t getBookmarksBegin() const;
            virtual const_iterator_t getBookmarksEnd() const;
            virtual sal_Int32 getBookmarksCount() const;
            virtual const_iterator_t findBookmark(const ::rtl::OUString& rName) const;

            // Fieldmarks
            virtual ::sw::mark::IFieldmark* getFieldmarkFor(const SwPosition& rPos) const;
            virtual ::sw::mark::IFieldmark* getFieldmarkBefore(const SwPosition& rPos) const;
            virtual ::sw::mark::IFieldmark* getFieldmarkAfter(const SwPosition& rPos) const;

            void dumpAsXml(xmlTextWriterPtr w);
        private:
            // make names
            ::rtl::OUString getUniqueMarkName(const ::rtl::OUString& rName) const;
            void sortMarks();

            container_t m_vMarks;
            container_t m_vBookmarks;
            container_t m_vFieldmarks;
            boost::unordered_set<rtl::OUString, rtl::OUStringHash> m_aMarkNamesSet;
            mutable MarkBasenameMapUniqueOffset_t m_aMarkBasenameMapUniqueOffset;
            SwDoc * const m_pDoc;
    };
    } // namespace mark
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

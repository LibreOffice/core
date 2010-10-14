/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _SW_BOOKMARK_MARKMANAGER_HXX
#define _SW_BOOKMARK_MARKMANAGER_HXX

#include <IMark.hxx>
#include <IDocumentMarkAccess.hxx>

namespace sw { namespace mark
{
    class MarkManager
        : private ::boost::noncopyable
        , virtual public IDocumentMarkAccess
    {
        public:
            MarkManager(/*[in/out]*/ SwDoc& rDoc);
#if OSL_DEBUG_LEVEL > 1
            void dumpFieldmarks( ) const;
#endif
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

            // bookmarks
            virtual const_iterator_t getBookmarksBegin() const;
            virtual const_iterator_t getBookmarksEnd() const;
            virtual sal_Int32 getBookmarksCount() const;
            virtual const_iterator_t findBookmark(const ::rtl::OUString& rName) const;

            // Fieldmarks
            virtual ::sw::mark::IFieldmark* getFieldmarkFor(const SwPosition& rPos) const;
            virtual ::sw::mark::IFieldmark* getFieldmarkBefore(const SwPosition& rPos) const;
            virtual ::sw::mark::IFieldmark* getFieldmarkAfter(const SwPosition& rPos) const;

        private:
            // make names
            ::rtl::OUString getUniqueMarkName(const ::rtl::OUString& rName) const;
            void sortMarks();

            container_t m_vMarks;
            container_t m_vBookmarks;
            container_t m_vFieldmarks;
            SwDoc * const m_pDoc;
    };
}}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

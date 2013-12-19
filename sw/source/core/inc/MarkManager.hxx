/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

            virtual sw::mark::IMark* makeAnnotationMark(
                const SwPaM& rPaM,
                const ::rtl::OUString& rName );


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
            virtual const_iterator_t getAllMarksBegin() const;
            virtual const_iterator_t getAllMarksEnd() const;
            virtual sal_Int32 getAllMarksCount() const;
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

            // Marks exclusive annotation marks
            virtual const_iterator_t getCommonMarksBegin() const;
            virtual const_iterator_t getCommonMarksEnd() const;
            virtual sal_Int32 getCommonMarksCount() const;

            // Annotation Marks
            virtual const_iterator_t getAnnotationMarksBegin() const;
            virtual const_iterator_t getAnnotationMarksEnd() const;
            virtual sal_Int32 getAnnotationMarksCount() const;
            virtual const_iterator_t findAnnotationMark( const ::rtl::OUString& rName ) const;

        private:
            // make names
            ::rtl::OUString getUniqueMarkName(const ::rtl::OUString& rName) const;
            void sortMarks();

            // container for all marks
            container_t m_vAllMarks;

            // additional container for bookmarks
            container_t m_vBookmarks;
            // additional container for fieldmarks
            container_t m_vFieldmarks;

            // container for annotation marks
            container_t m_vAnnotationMarks;

            // container for all marks except annotation marks
            container_t m_vCommonMarks;

            SwDoc * const m_pDoc;
    };
}}
#endif

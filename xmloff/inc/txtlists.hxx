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
#if 1

#include <rtl/ustring.hxx>
#include <comphelper/stl_types.hxx>
#include <map>
#include <vector>
#include <stack>
#include <boost/utility.hpp>
#include <boost/tuple/tuple.hpp>
#include <com/sun/star/container/XIndexReplace.hpp>
#include <xmloff/xmlictxt.hxx>

class SvXMLImport;
class XMLTextListBlockContext;
class XMLTextListItemContext;
class XMLNumberedParaContext;

class XMLTextListsHelper : private boost::noncopyable
{
    public:
        XMLTextListsHelper();
        ~XMLTextListsHelper();

        /// list stack for importing:

        /// push a list context on the list context stack
        void PushListContext(XMLTextListBlockContext *i_pListBlock = 0);
        void PushListContext(XMLNumberedParaContext *i_pNumberedParagraph);
        /// pop the list context stack
        void PopListContext();
        /// peek at the top of the list context stack
        void ListContextTop(XMLTextListBlockContext*& o_pListBlockContext,
            XMLTextListItemContext*& o_pListItemContext,
            XMLNumberedParaContext*& o_pNumberedParagraphContext );
        /// set list item on top of the list context stack
        void SetListItem( XMLTextListItemContext *pListItem );


        // keeping track of processed lists for import and export
        // Add optional parameter <sListStyleDefaultListId> (#i92811#)
        void KeepListAsProcessed( ::rtl::OUString sListId,
                                  ::rtl::OUString sListStyleName,
                                  ::rtl::OUString sContinueListId,
                                  ::rtl::OUString sListStyleDefaultListId = ::rtl::OUString() );

        sal_Bool IsListProcessed( const ::rtl::OUString sListId ) const;
        ::rtl::OUString GetListStyleOfProcessedList(
                                        const ::rtl::OUString sListId ) const;
        ::rtl::OUString GetContinueListIdOfProcessedList(
                                        const ::rtl::OUString sListId ) const;
        const ::rtl::OUString& GetLastProcessedListId() const;
        const ::rtl::OUString& GetListStyleOfLastProcessedList() const;

        ::rtl::OUString GenerateNewListId() const;

        // Provide list id for a certain list block for import (#i92811#)
        ::rtl::OUString GetListIdForListBlock( XMLTextListBlockContext& rListBlock );

        // keep track of continue list chain for export
        void StoreLastContinuingList( ::rtl::OUString sListId,
                                      ::rtl::OUString sContinuingListId );

        ::rtl::OUString GetLastContinuingListId( ::rtl::OUString sListId ) const;

        // keep track of opened list elements of a certain list for export
        void PushListOnStack( ::rtl::OUString sListId,
                              ::rtl::OUString sListStyleName );
        void PopListFromStack();
        sal_Bool EqualsToTopListStyleOnStack( const ::rtl::OUString sListId ) const;

        /** for importing numbered-paragraph
            note that the ID namespace for numbered-paragraph and regular list
            is distinct; we never combine a list and a n-p
         */
        ::com::sun::star::uno::Reference<
                ::com::sun::star::container::XIndexReplace>
        EnsureNumberedParagraph(
            SvXMLImport & i_rImport,
            const ::rtl::OUString i_ListId,
            sal_Int16 & io_rLevel, const ::rtl::OUString i_StyleName);

        /// get ID of the last numbered-paragraph iff it has given style-name
        ::rtl::OUString GetNumberedParagraphListId(
            const sal_uInt16 i_Level,
            const ::rtl::OUString i_StyleName);

        /** Creates a NumRule from given style-name.
            @param i_rImport        the SvXMLImport
            @param i_xNumRule       parent num rule
            @param i_ParentStyleName  parent list style name
            @param i_StyleName      the list style name
            @param io_rLevel        the list level (may be reset if too large)
            @param o_rRestartNumbering   set to true if no style (defaulting)
            @param io_rSetDefaults  set to true if no style (defaulting)
         */
        static ::com::sun::star::uno::Reference<
                ::com::sun::star::container::XIndexReplace> MakeNumRule(
            SvXMLImport & i_rImport,
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::container::XIndexReplace>& i_xNumRule,
            const ::rtl::OUString i_ParentStyleName,
            const ::rtl::OUString i_StyleName,
            sal_Int16 & io_rLevel,
            sal_Bool* o_pRestartNumbering = 0,
            sal_Bool* io_pSetDefaults = 0);

    private:

        /** list context: list, list-item, numbered-paragraph
            XMLTextListBlockContext, XMLTextListItemContext,
            XMLNumberedParaContext
         */
        typedef ::boost::tuple<SvXMLImportContextRef,
            SvXMLImportContextRef, SvXMLImportContextRef> ListStackFrame_t;
        ::std::stack< ListStackFrame_t > mListStack;

        // container type for processed lists:
        // map with <ListId> as key and pair( <ListStyleName, ContinueListId> )
        // as value
        typedef ::std::map< ::rtl::OUString,
                            ::std::pair< ::rtl::OUString, ::rtl::OUString >,
                            ::comphelper::UStringLess > tMapForLists;
        tMapForLists* mpProcessedLists;
        ::rtl::OUString msLastProcessedListId;
        ::rtl::OUString msListStyleOfLastProcessedList;

        /* additional container for processed lists.
           map with <ListStyleName> as key and pair( <ListId, ListStyleDefaultListId> )
           as value. (#i92811#)
        */
        tMapForLists* mpMapListIdToListStyleDefaultListId;

        // container type to build up continue list chain:
        // map with <ListId> of master list as key and <ListId> of last list
        // continuing the master list as value
        typedef ::std::map< ::rtl::OUString, ::rtl::OUString,
                            ::comphelper::UStringLess > tMapForContinuingLists;
        tMapForContinuingLists* mpContinuingLists;

        // stack type for opened list elements and its list style:
        // vector with pair( <ListId>, <ListStyleName> ) as value
        typedef ::std::vector< ::std::pair< ::rtl::OUString, ::rtl::OUString > >
                                                                tStackForLists;
        tStackForLists* mpListStack;

        /// to connect numbered-paragraphs that have no list-id attribute:
        /// vector of pair of style-name and list-id (indexed by level)
        typedef ::std::vector< ::std::pair< ::rtl::OUString, ::rtl::OUString > >
            LastNumberedParagraphs_t;

        LastNumberedParagraphs_t mLastNumberedParagraphs;

        /// numbered-paragraphs
        typedef ::std::vector< ::std::pair< ::rtl::OUString,
            ::com::sun::star::uno::Reference<
                ::com::sun::star::container::XIndexReplace > > > NumParaList_t;
        ::std::map< ::rtl::OUString, NumParaList_t > mNPLists;

};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

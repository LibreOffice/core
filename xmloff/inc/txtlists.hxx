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

#ifndef INCLUDED_XMLOFF_INC_TXTLISTS_HXX
#define INCLUDED_XMLOFF_INC_TXTLISTS_HXX

#include <rtl/ustring.hxx>
#include <map>
#include <vector>
#include <stack>
#include <boost/noncopyable.hpp>
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
        void PushListContext(XMLTextListBlockContext *i_pListBlock = nullptr);
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
        void KeepListAsProcessed( const OUString& sListId,
                                  const OUString& sListStyleName,
                                  const OUString& sContinueListId,
                                  const OUString& sListStyleDefaultListId = OUString() );

        bool IsListProcessed( const OUString& sListId ) const;
        OUString GetListStyleOfProcessedList(
                                        const OUString& sListId ) const;
        OUString GetContinueListIdOfProcessedList(
                                        const OUString& sListId ) const;
        const OUString& GetLastProcessedListId() const { return msLastProcessedListId;}
        const OUString& GetListStyleOfLastProcessedList() const { return msListStyleOfLastProcessedList;}

        OUString GenerateNewListId() const;

        // Provide list id for a certain list block for import (#i92811#)
        OUString GetListIdForListBlock( XMLTextListBlockContext& rListBlock );

        // keep track of continue list chain for export
        void StoreLastContinuingList( const OUString& sListId,
                                      const OUString& sContinuingListId );

        OUString GetLastContinuingListId( const OUString& sListId ) const;

        // keep track of opened list elements of a certain list for export
        void PushListOnStack( const OUString& sListId,
                              const OUString& sListStyleName );
        void PopListFromStack();
        bool EqualsToTopListStyleOnStack( const OUString& sListId ) const;

        /** for importing numbered-paragraph
            note that the ID namespace for numbered-paragraph and regular list
            is distinct; we never combine a list and a n-p
         */
        css::uno::Reference< css::container::XIndexReplace>
        EnsureNumberedParagraph(
            SvXMLImport & i_rImport,
            const OUString& i_ListId,
            sal_Int16 & io_rLevel, const OUString& i_StyleName);

        /// get ID of the last numbered-paragraph iff it has given style-name
        OUString GetNumberedParagraphListId(
            const sal_uInt16 i_Level,
            const OUString& i_StyleName);

        /** Creates a NumRule from given style-name.
            @param i_rImport        the SvXMLImport
            @param i_xNumRule       parent num rule
            @param i_ParentStyleName  parent list style name
            @param i_StyleName      the list style name
            @param io_rLevel        the list level (may be reset if too large)
            @param o_rRestartNumbering   set to true if no style (defaulting)
            @param io_rSetDefaults  set to true if no style (defaulting)
         */
        static css::uno::Reference< css::container::XIndexReplace> MakeNumRule(
            SvXMLImport & i_rImport,
            const css::uno::Reference< css::container::XIndexReplace>& i_xNumRule,
            const OUString& i_ParentStyleName,
            const OUString& i_StyleName,
            sal_Int16 & io_rLevel,
            bool* o_pRestartNumbering = nullptr,
            bool* io_pSetDefaults = nullptr);

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
        typedef ::std::map< OUString,
                            ::std::pair< OUString, OUString > > tMapForLists;
        tMapForLists* mpProcessedLists;
        OUString msLastProcessedListId;
        OUString msListStyleOfLastProcessedList;

        /* additional container for processed lists.
           map with <ListStyleName> as key and pair( <ListId, ListStyleDefaultListId> )
           as value. (#i92811#)
        */
        tMapForLists* mpMapListIdToListStyleDefaultListId;

        // container type to build up continue list chain:
        // map with <ListId> of master list as key and <ListId> of last list
        // continuing the master list as value
        typedef ::std::map< OUString, OUString > tMapForContinuingLists;
        tMapForContinuingLists* mpContinuingLists;

        // stack type for opened list elements and its list style:
        // vector with pair( <ListId>, <ListStyleName> ) as value
        typedef ::std::vector< ::std::pair< OUString, OUString > >
                                                                tStackForLists;
        tStackForLists* mpListStack;

        /// to connect numbered-paragraphs that have no list-id attribute:
        /// vector of pair of style-name and list-id (indexed by level)
        typedef ::std::vector< ::std::pair< OUString, OUString > >
            LastNumberedParagraphs_t;

        LastNumberedParagraphs_t mLastNumberedParagraphs;

        /// numbered-paragraphs
        typedef ::std::vector<
                    ::std::pair<
                        OUString,
                        css::uno::Reference< css::container::XIndexReplace > > > NumParaList_t;
        ::std::map< OUString, NumParaList_t > mNPLists;

};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

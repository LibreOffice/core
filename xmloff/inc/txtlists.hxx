/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: txtlists.hxx,v $
 * $Revision: 1.2 $
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

#ifndef _XMLOFF_TXTLISTS_HXX
#define _XMLOFF_TXTLISTS_HXX

#include <rtl/ustring.hxx>
#include <comphelper/stl_types.hxx>
#include <map>
#include <vector>

class XMLTextListsHelper
{
    public:
        XMLTextListsHelper();
        ~XMLTextListsHelper();

        // keeping track of processed lists for import and export
        void KeepListAsProcessed( ::rtl::OUString sListId,
                                  ::rtl::OUString sListStyleName,
                                  ::rtl::OUString sContinueListId );

        sal_Bool IsListProcessed( const ::rtl::OUString sListId ) const;
        ::rtl::OUString GetListStyleOfProcessedList(
                                        const ::rtl::OUString sListId ) const;
        ::rtl::OUString GetContinueListIdOfProcessedList(
                                        const ::rtl::OUString sListId ) const;
        const ::rtl::OUString& GetLastProcessedListId() const;
        const ::rtl::OUString& GetListStyleOfLastProcessedList() const;

        ::rtl::OUString GenerateNewListId() const;


        // keep track of continue list chain for export
        void StoreLastContinuingList( ::rtl::OUString sListId,
                                      ::rtl::OUString sContinuingListId );

        ::rtl::OUString GetLastContinuingListId( ::rtl::OUString sListId ) const;

        // keep track of opened list elements of a certain list for export
        void PushListOnStack( ::rtl::OUString sListId,
                              ::rtl::OUString sListStyleName );
        void PopListFromStack();
        sal_Bool EqualsToTopListStyleOnStack( const ::rtl::OUString sListId ) const;

    private:
        // container type for processed lists:
        // map with <ListId> as key and pair( <ListStyleName, ContinueListId> )
        // as value
        typedef ::std::map< ::rtl::OUString,
                            ::std::pair< ::rtl::OUString, ::rtl::OUString >,
                            ::comphelper::UStringLess > tMapForLists;
        tMapForLists* mpProcessedLists;
        ::rtl::OUString msLastProcessedListId;
        ::rtl::OUString msListStyleOfLastProcessedList;

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
};
#endif

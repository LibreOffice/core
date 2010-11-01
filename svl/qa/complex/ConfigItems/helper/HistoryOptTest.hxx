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

#ifndef  SVTOOLS_HISTORYOPTTEST_HXX
#define  SVTOOLS_HISTORYOPTTEST_HXX

#include <com/sun/star/container/XNameAccess.hpp>
#include <unotools/historyoptions.hxx>

namespace css = ::com::sun::star;

class HistoryOptTest
{
    public:

        HistoryOptTest();
        virtual ~HistoryOptTest();

        //---------------------------------------------------------------------
        /** unit test of picklist */
        void checkPicklist();

        //---------------------------------------------------------------------
        /** unit test of URL list */
        void checkURLHistory();

        //---------------------------------------------------------------------
        /** unit test of Help bookmarks */
        void checkHelpBookmarks();

    private:

        //---------------------------------------------------------------------
        /** test every well known history list in the same way.
         *  Only the count of created and tested items can be defined from outside
         *  e.g. usefull for stress tests.
         *
         *  @param  eHistory
         *          specify the history list for testing.
         *
         *  @param  nMaxItems
         *          max count of new created and tested history items.
         */
        void impl_testHistory(EHistoryType eHistory ,
                              ::sal_Int32  nMaxItems);

        //---------------------------------------------------------------------
        /** try to clear the whole list and check the results.
         *  If list could not be cleared successfully an exception is thrown.
         */
        void impl_clearList();

        //---------------------------------------------------------------------
        /** define a new size for the current list and check the results.
         *  Note: The given size must match against the defined constraints.
         *  That must be checked before this method is called.
         *
         *  @param  nSize
         *          the new size.
         */
        void impl_setSize(::sal_Int32 nSize);

        //---------------------------------------------------------------------
        /** create a new item (means it's properties using a special schema using the
         *  item id).
         *
         *  Note: This method does not check if creation was successfully.
         *        Therefore exists more specialized method impl_existsItem()
         *        and impl_existsItemAtPosition().
         *
         *  @param  nItem
         *          id of the item
         */
        void impl_appendItem(::sal_Int32 nItem);

        //---------------------------------------------------------------------
        /** check if an entry for given item id realy exists (in memory and xcu file).
         *
         *  @param  nItem
         *          id of the item
         *
         *  @return true if item exists - false otherwise.
         */
        ::sal_Bool impl_existsItem(::sal_Int32 nItem);

        //---------------------------------------------------------------------
        /** check if an entry for given item id realy exists (in memory and xcu file).
         *  Further it checks if the requested item is placed at the also specified
         *  position inside history list.
         *
         *  @param  nItem
         *          id of the item
         *
         *  @param  nIndex
         *          expected position of item inside history list.
         *
         *  @return true if item exists at right position - false otherwise.
         */
        ::sal_Bool impl_existsItemAtIndex(::sal_Int32 nItem ,
                                          ::sal_Int32 nIndex);

        //---------------------------------------------------------------------
        /** create an URL suitable for the given item id.
         *
         *  @param  nItem
         *          id of the item
         *
         *  @return the new created URL.
         */
        ::rtl::OUString impl_createItemURL(::sal_Int32 nItem);

        //---------------------------------------------------------------------
        /** create a title suitable for the given item id.
         *
         *  @param  nItem
         *          id of the item
         *
         *  @return the new created title.
         */
        ::rtl::OUString impl_createItemTitle(::sal_Int32 nItem);

        //---------------------------------------------------------------------
        /** create a password suitable for the given item id.
         *
         *  @param  nItem
         *          id of the item
         *
         *  @return the new created password.
         */
        ::rtl::OUString impl_createItemPassword(::sal_Int32 nItem);

        //---------------------------------------------------------------------
        /** returns direct access to the item list inside histories.xcu
         *  suitable for the current defined list type (m_eList).
         *
         *  @return reference to the item list configuration
         */
        css::uno::Reference< css::container::XNameAccess > impl_getItemList();

        //---------------------------------------------------------------------
        /** returns direct access to the order list inside histories.xcu
         *  suitable for the current defined list type (m_eList).
         *
         *  @return reference to the order list configuration
         */
        css::uno::Reference< css::container::XNameAccess > impl_getOrderList();

        //---------------------------------------------------------------------
        /** returns direct access to the history list inside histories.xcu
         *  suitable for the current defined list type (m_eList).
         *
         *  @return reference to the history list configuration
         */
        css::uno::Reference< css::container::XNameAccess > impl_getNewHistory();

        //---------------------------------------------------------------------
        /** returns direct access to the history config inside common.xcu
         *  suitable for the current defined list type (m_eList).
         *
         *  @return reference to the history configuration
         */
        css::uno::Reference< css::container::XNameAccess > impl_getOldHistory();

    private:

        // the config item which should be tested here
        SvtHistoryOptions m_aConfigItem;

        // defines the special list for testing (picklist, history or url list)
        EHistoryType m_eList;

        // underlying configuration of the tested config items for cross over checks
        css::uno::Reference< css::container::XNameAccess > m_xHistoriesXCU;

        // underlying configuration of the tested config items for cross over checks
        css::uno::Reference< css::container::XNameAccess > m_xCommonXCU;
};

#endif // #ifndef  SVTOOLS_HISTORYOPTTEST_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

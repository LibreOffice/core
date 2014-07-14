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
         *  e.g. useful for stress tests.
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
        /** check if an entry for given item id really exists (in memory and xcu file).
         *
         *  @param  nItem
         *          id of the item
         *
         *  @return true if item exists - false otherwise.
         */
        ::sal_Bool impl_existsItem(::sal_Int32 nItem);

        //---------------------------------------------------------------------
        /** check if an entry for given item id really exists (in memory and xcu file).
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

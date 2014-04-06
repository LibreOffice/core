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
#ifndef INCLUDED_UNOTOOLS_HISTORYOPTIONS_HXX
#define INCLUDED_UNOTOOLS_HISTORYOPTIONS_HXX

#include <unotools/unotoolsdllapi.h>
#include <sal/types.h>
#include <osl/mutex.hxx>
#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <rtl/ustring.hxx>
#include <unotools/options.hxx>

/*-************************************************************************************************************
    @descr          The method GetList() returns a list of property values.
                    Use follow defines to separate values by names.
*//*-*************************************************************************************************************/

#define HISTORY_PROPERTYNAME_URL            OUString("URL")
#define HISTORY_PROPERTYNAME_FILTER         OUString("Filter")
#define HISTORY_PROPERTYNAME_TITLE          OUString("Title")
#define HISTORY_PROPERTYNAME_PASSWORD       OUString("Password")
#define HISTORY_PROPERTYNAME_THUMBNAIL      OUString("Thumbnail")

/*-************************************************************************************************************
    @descr          You can use these enum values to specify right history if you call ouer interface methods.
*//*-*************************************************************************************************************/

enum EHistoryType
{
    ePICKLIST       = 0,
    eHISTORY        = 1,
    eHELPBOOKMARKS  = 2
};

/*-************************************************************************************************************
    @short          forward declaration to our private date container implementation
    @descr          We use these class as internal member to support small memory requirements.
                    You can create the container if it is necessary. The class which use these mechanism
                    is faster and smaller then a complete implementation!
*//*-*************************************************************************************************************/

class SvtHistoryOptions_Impl;

/*-************************************************************************************************************
    @short          collect information about history features
    @descr          -

    @implements     -
    @base           -

    @devstatus      ready to use
*//*-*************************************************************************************************************/

class UNOTOOLS_DLLPUBLIC SAL_WARN_UNUSED SvtHistoryOptions : public utl::detail::Options
{
    public:
        /*-****************************************************************************************************
            @short      standard constructor and destructor
            @descr      This will initialize an instance with default values.
                        We implement these class with a refcount mechanism! Every instance of this class increase it
                        at create and decrease it at delete time - but all instances use the same data container!
                        He is implemented as a static member ...

            @seealso    member m_nRefCount
            @seealso    member m_pDataContainer

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

         SvtHistoryOptions();
        virtual ~SvtHistoryOptions();

        /*-****************************************************************************************************
            @short      interface methods to get and set value of config key "org.openoffice.Office.Common/History/..."
            @descr      key "PickList"  : The last used documents displayed in the file menu.
                        key "History"   : The last opened documents general.
        *//*-*****************************************************************************************************/

        /*-****************************************************************************************************
            @short      set/get max size of specified history
            @descr      Call this methods to get information about max. size of specified list.
                        These value lay down the max count of items in these history. If a new one
                        is add to it the oldest one is deleted automaticly!

            @seealso    -

            @param      "eHistory" select right history.
            @param      "nSize" is the new max size of specified list. If new size smaller then the old one
                        some oldest entries will be destroyed automaticly!
            @return     Current max size of specified list.

            @onerror    -
        *//*-*****************************************************************************************************/

        sal_uInt32  GetSize( EHistoryType eHistory                      ) const;

        /*-****************************************************************************************************
            @short      clear complete sepcified list
            @descr      Call this methods to clear the whole list. After that GetItemCount() will return 0 ...
                        but GetSize() will return the old value!

            @seealso    -

            @param      "eHistory" select right history.
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        void Clear( EHistoryType eHistory );

        /*-****************************************************************************************************
            @short      return complete sepcified list
            @descr      If you will show the whole list call this method to get it completely.

            @seealso    -

            @param      "eHistory" select right history.
            @return     A list of history items is returned.

            @onerror    -
        *//*-*****************************************************************************************************/

        ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > > GetList( EHistoryType eHistory ) const;

        /*-****************************************************************************************************
            @short      append a new item to specified list
            @descr      You can append items to a list only - removing isn't allowed for a special item!
                        The oldest entry is deleted automaticly if max size arrived or you can call Clear() ...
                        It exist two different overload methods to do this.
                        One for user which have an complete history item and another one for uncompletly data sets!

            @seealso    method SetSize()
            @seealso    method Clear()

            @param      "eHistory" select right history.
            @param      "sURL" URL to save in history
            @param      "sFilter" filter name to save in history
            @param      "sTitle" document title to save in history
            @param      "sPassword" password to save in history
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        void AppendItem(EHistoryType eHistory,
                const OUString& sURL, const OUString& sFilter, const OUString& sTitle,
                const OUString& sPassword, const OUString& sThumbnail);

    private:

        /*-****************************************************************************************************
            @short      return a reference to a static mutex
            @descr      Make these class threadsafe.

            @seealso    -

            @param      -
            @return     A reference to a static mutex member.

            @onerror    -
        *//*-*****************************************************************************************************/

        UNOTOOLS_DLLPRIVATE static ::osl::Mutex& GetOwnStaticMutex();

    private:

        /*Attention

            Don't initialize these static member in these header!
            a) Double dfined symbols will be detected ...
            b) and unresolved externals exist at linking time.
            Do it in your source only.
         */

        static SvtHistoryOptions_Impl*  m_pDataContainer;   /// impl. data container as dynamic pointer for smaller memory requirements!
        static sal_Int32                m_nRefCount;   /// internal ref count mechanism

};      // class SvtHistoryOptions

#endif // INCLUDED_UNOTOOLS_HISTORYOPTIONS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

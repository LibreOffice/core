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
#ifndef INCLUDED_UNOTOOLS_DYNAMICMENUOPTIONS_HXX
#define INCLUDED_UNOTOOLS_DYNAMICMENUOPTIONS_HXX

#include <unotools/unotoolsdllapi.h>
#include <sal/types.h>
#include <osl/mutex.hxx>
#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <unotools/options.hxx>

/*-************************************************************************************************************
    @descr          The method GetList() returns a list of property values.
                    Use follow defines to separate values by names.
*//*-*************************************************************************************************************/
#define DYNAMICMENU_PROPERTYNAME_URL                    "URL"
#define DYNAMICMENU_PROPERTYNAME_TITLE                  "Title"
#define DYNAMICMENU_PROPERTYNAME_IMAGEIDENTIFIER        "ImageIdentifier"
#define DYNAMICMENU_PROPERTYNAME_TARGETNAME             "TargetName"

/*-************************************************************************************************************
    @descr          You can use these enum values to specify right menu if you call our interface methods.
*//*-*************************************************************************************************************/
enum EDynamicMenuType
{
    E_NEWMENU       =   0,
    E_WIZARDMENU    =   1,
    E_HELPBOOKMARKS =   2
};

/*-************************************************************************************************************
    @short          forward declaration to our private date container implementation
    @descr          We use these class as internal member to support small memory requirements.
                    You can create the container if it is necessary. The class which use these mechanism
                    is faster and smaller then a complete implementation!
*//*-*************************************************************************************************************/

class SvtDynamicMenuOptions_Impl;

/*-************************************************************************************************************
    @short          collect information about dynamic menus
    @descr          Make it possible to configure dynamic menu structures of menus like "new" or "wizard".
    @devstatus      ready to use
*//*-*************************************************************************************************************/

class UNOTOOLS_DLLPUBLIC SAL_WARN_UNUSED SvtDynamicMenuOptions : public utl::detail::Options
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
        *//*-*****************************************************************************************************/

         SvtDynamicMenuOptions();
        virtual ~SvtDynamicMenuOptions();

        /*-****************************************************************************************************
            @short      return complete specified list
            @descr      Call it to get all entries of an dynamic menu.
                        We return a list of all nodes with his names and properties.
            @param      "eMenu" select right menu.
            @return     A list of menu items is returned.

            @onerror    We return an empty list.
        *//*-*****************************************************************************************************/

        css::uno::Sequence< css::uno::Sequence< css::beans::PropertyValue > > GetMenu( EDynamicMenuType eMenu ) const;
    private:

        /*-****************************************************************************************************
            @short      return a reference to a static mutex
            @descr      These class is partially threadsafe (for de-/initialization only).
                        All access methods are'nt safe!
                        We create a static mutex only for one ime and use at different times.
            @return     A reference to a static mutex member.
        *//*-*****************************************************************************************************/

        UNOTOOLS_DLLPRIVATE static ::osl::Mutex& GetOwnStaticMutex();

    private:

        /*Attention

            Don't initialize these static members in these headers!
            a) Double defined symbols will be detected ...
            b) and unresolved externals exist at linking time.
            Do it in your source only.
         */

        static SvtDynamicMenuOptions_Impl* m_pDataContainer;
        static sal_Int32             m_nRefCount;

};      // class SvtDynamicMenuOptions

#endif // INCLUDED_UNOTOOLS_DYNAMICMENUOPTIONS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

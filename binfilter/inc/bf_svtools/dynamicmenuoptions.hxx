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
#ifndef INCLUDED_SVTOOLS_DYNAMICMENUOPTIONS_HXX
#define INCLUDED_SVTOOLS_DYNAMICMENUOPTIONS_HXX

#include <sal/types.h>

#include <osl/mutex.hxx>

#include <com/sun/star/uno/Sequence.h>

#include <com/sun/star/beans/PropertyValue.hpp>

#include <bf_svtools/options.hxx>

namespace binfilter {

//_________________________________________________________________________________________________________________
//	types, enums, ...
//_________________________________________________________________________________________________________________

/*-************************************************************************************************************//**
    @descr			The method GetList() returns a list of property values.
                    Use follow defines to seperate values by names.
*//*-*************************************************************************************************************/
#define DYNAMICMENU_PROPERTYNAME_URL                    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("URL"             ))
#define DYNAMICMENU_PROPERTYNAME_TITLE                  ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Title"           ))
#define DYNAMICMENU_PROPERTYNAME_IMAGEIDENTIFIER        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ImageIdentifier" ))
#define DYNAMICMENU_PROPERTYNAME_TARGETNAME             ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("TargetName"      ))

/*-************************************************************************************************************//**
    @descr          You can use these enum values to specify right menu if you call our interface methods.
*//*-*************************************************************************************************************/
enum EDynamicMenuType
{
    E_NEWMENU       =   0,
    E_WIZARDMENU    =   1,
    E_HELPBOOKMARKS =   2
};
//_________________________________________________________________________________________________________________
//	forward declarations
//_________________________________________________________________________________________________________________

/*-************************************************************************************************************//**
    @short			forward declaration to our private date container implementation
    @descr			We use these class as internal member to support small memory requirements.
                    You can create the container if it is neccessary. The class which use these mechanism
                    is faster and smaller then a complete implementation!
*//*-*************************************************************************************************************/

class SvtDynamicMenuOptions_Impl;

//_________________________________________________________________________________________________________________
//	declarations
//_________________________________________________________________________________________________________________

/*-************************************************************************************************************//**
    @short          collect informations about dynamic menus
    @descr          Make it possible to configure dynamic menu structures of menus like "new" or "wizard".

    @implements		-
    @base			-

    @devstatus		ready to use
*//*-*************************************************************************************************************/

class  SvtDynamicMenuOptions: public Options
{
    //-------------------------------------------------------------------------------------------------------------
    //	public methods
    //-------------------------------------------------------------------------------------------------------------

    public:

        //---------------------------------------------------------------------------------------------------------
        //	constructor / destructor
        //---------------------------------------------------------------------------------------------------------

        /*-****************************************************************************************************//**
            @short		standard constructor and destructor
            @descr		This will initialize an instance with default values.
                        We implement these class with a refcount mechanism! Every instance of this class increase it
                        at create and decrease it at delete time - but all instances use the same data container!
                        He is implemented as a static member ...

            @seealso	member m_nRefCount
            @seealso	member m_pDataContainer

            @param		-
            @return		-

            @onerror	-
        *//*-*****************************************************************************************************/

         SvtDynamicMenuOptions();
        virtual ~SvtDynamicMenuOptions();

        //---------------------------------------------------------------------------------------------------------
        //	interface
        //---------------------------------------------------------------------------------------------------------

    //-------------------------------------------------------------------------------------------------------------
    //	private methods
    //-------------------------------------------------------------------------------------------------------------

    private:

        /*-****************************************************************************************************//**
            @short		return a reference to a static mutex
            @descr		These class is partially threadsafe (for de-/initialization only).
                        All access methods are'nt safe!
                        We create a static mutex only for one ime and use at different times.

            @seealso	-

            @param		-
            @return		A reference to a static mutex member.

            @onerror	-
        *//*-*****************************************************************************************************/

         static ::osl::Mutex& GetOwnStaticMutex();

    //-------------------------------------------------------------------------------------------------------------
    //	private member
    //-------------------------------------------------------------------------------------------------------------

    private:

        /*Attention

            Don't initialize these static member in these header!
            a) Double dfined symbols will be detected ...
            b) and unresolved externals exist at linking time.
            Do it in your source only.
         */

        static SvtDynamicMenuOptions_Impl* m_pDataContainer    ;   /// impl. data container as dynamic pointer for smaller memory requirements!
        static sal_Int32             m_nRefCount         ;   /// internal ref count mechanism

};      // class SvtDynamicMenuOptions

}

#endif  // #ifndef INCLUDED_SVTOOLS_DYNAMICMENUOPTIONS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

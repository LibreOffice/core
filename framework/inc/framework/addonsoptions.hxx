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
#ifndef __FRAMEWORK_CLASSES_ADDONSOPTIONS_HXX_
#define __FRAMEWORK_CLASSES_ADDONSOPTIONS_HXX_

#include <sal/types.h>
#include <osl/mutex.hxx>
#include <vcl/image.hxx>
#include <vcl/bitmap.hxx>
#include <vcl/svapp.hxx>
#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <framework/fwedllapi.h>
//_________________________________________________________________________________________________________________
//  types, enums, ...
//_________________________________________________________________________________________________________________

/*-************************************************************************************************************//**
    @descr          The method GetAddonsMenu() returns a list of property values.
                    Use follow defines to seperate values by names.
*//*-*************************************************************************************************************/
#define ADDONSMENUITEM_STRING_URL                       "URL"
#define ADDONSMENUITEM_STRING_TITLE                     "Title"
#define ADDONSMENUITEM_STRING_TARGET                    "Target"
#define ADDONSMENUITEM_STRING_IMAGEIDENTIFIER           "ImageIdentifier"
#define ADDONSMENUITEM_STRING_CONTEXT                   "Context"
#define ADDONSMENUITEM_STRING_SUBMENU                   "Submenu"
#define ADDONSMENUITEM_STRING_CONTROLTYPE               "ControlType"
#define ADDONSMENUITEM_STRING_WIDTH                     "Width"

#define ADDONSMENUITEM_URL_LEN                          3
#define ADDONSMENUITEM_TITLE_LEN                        5
#define ADDONSMENUITEM_TARGET_LEN                       6
#define ADDONSMENUITEM_SUBMENU_LEN                      7
#define ADDONSMENUITEM_CONTEXT_LEN                      7
#define ADDONSMENUITEM_IMAGEIDENTIFIER_LEN              15

#define ADDONSMENUITEM_PROPERTYNAME_URL                 ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(ADDONSMENUITEM_STRING_URL             ))
#define ADDONSMENUITEM_PROPERTYNAME_TITLE               ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(ADDONSMENUITEM_STRING_TITLE           ))
#define ADDONSMENUITEM_PROPERTYNAME_TARGET              ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(ADDONSMENUITEM_STRING_TARGET          ))
#define ADDONSMENUITEM_PROPERTYNAME_IMAGEIDENTIFIER     ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(ADDONSMENUITEM_STRING_IMAGEIDENTIFIER ))
#define ADDONSMENUITEM_PROPERTYNAME_CONTEXT             ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(ADDONSMENUITEM_STRING_CONTEXT         ))
#define ADDONSMENUITEM_PROPERTYNAME_SUBMENU             ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(ADDONSMENUITEM_STRING_SUBMENU         ))
#define ADDONSMENUITEM_PROPERTYNAME_CONTROLTYPE         ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(ADDONSMENUITEM_STRING_CONTROLTYPE     ))
#define ADDONSMENUITEM_PROPERTYNAME_WIDTH               ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(ADDONSMENUITEM_STRING_WIDTH           ))

#define ADDONSPOPUPMENU_URL_PREFIX_STR                  "private:menu/Addon"

#define ADDONSPOPUPMENU_URL_PREFIX                      ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( ADDONSPOPUPMENU_URL_PREFIX_STR ))

namespace framework
{

typedef ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > > MergeMenuDefinition;

struct FWE_DLLPUBLIC MergeMenuInstruction
{
    ::rtl::OUString     aMergePoint;
    ::rtl::OUString     aMergeCommand;
    ::rtl::OUString     aMergeCommandParameter;
    ::rtl::OUString     aMergeFallback;
    ::rtl::OUString     aMergeContext;
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > > aMergeMenu;
};
typedef ::std::vector< MergeMenuInstruction > MergeMenuInstructionContainer;

struct FWE_DLLPUBLIC MergeToolbarInstruction
{
    ::rtl::OUString     aMergeToolbar;
    ::rtl::OUString     aMergePoint;
    ::rtl::OUString     aMergeCommand;
    ::rtl::OUString     aMergeCommandParameter;
    ::rtl::OUString     aMergeFallback;
    ::rtl::OUString     aMergeContext;
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > > aMergeToolbarItems;
};

typedef ::std::vector< MergeToolbarInstruction > MergeToolbarInstructionContainer;

//_________________________________________________________________________________________________________________
//  forward declarations
//_________________________________________________________________________________________________________________

/*-************************************************************************************************************//**
    @short          forward declaration to our private date container implementation
    @descr          We use these class as internal member to support small memory requirements.
                    You can create the container if it is neccessary. The class which use these mechanism
                    is faster and smaller then a complete implementation!
*//*-*************************************************************************************************************/

class AddonsOptions_Impl;

//_________________________________________________________________________________________________________________
//  declarations
//_________________________________________________________________________________________________________________

/*-************************************************************************************************************//**
    @short          collect informations about menu features
    @descr          -

    @implements     -
    @base           -

    @devstatus      ready to use
*//*-*************************************************************************************************************/

class FWE_DLLPUBLIC AddonsOptions
{
    //-------------------------------------------------------------------------------------------------------------
    //  public methods
    //-------------------------------------------------------------------------------------------------------------

    public:
        //---------------------------------------------------------------------------------------------------------
        //  constructor / destructor
        //---------------------------------------------------------------------------------------------------------

        /*-****************************************************************************************************//**
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

         AddonsOptions();
        ~AddonsOptions();

        //---------------------------------------------------------------------------------------------------------
        //  interface
        //---------------------------------------------------------------------------------------------------------

        /*-****************************************************************************************************//**
            @short      clears completely the addons menu
            @descr      Call this methods to clear the addons menu
                        To fill it again use AppendItem().

            @seealso    -

            @param      "eMenu" select right menu to clear.
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        void Clear();

        /*-****************************************************************************************************//**
            @short      returns if an addons menu is available
            @descr      Call to retrieve if a addons menu is available


            @return     sal_True if there is a menu otherwise sal_False
        *//*-*****************************************************************************************************/

        sal_Bool    HasAddonsMenu() const;

        /*-****************************************************************************************************//**
            @short      returns number of addons toolbars
            @descr      Call to retrieve the number of addons toolbars


            @return     number of addons toolbars
        *//*-*****************************************************************************************************/
        sal_Int32   GetAddonsToolBarCount() const ;

        /*-****************************************************************************************************//**
            @short      returns the  complete addons menu
            @descr      Call it to get all entries of the addon menu.
                        We return a list of all nodes with his names and properties.

            @seealso    -

            @return     A list of menu items is returned.

            @onerror    We return an empty list.
        *//*-*****************************************************************************************************/

        const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > >& GetAddonsMenu() const;

        /*-****************************************************************************************************//**
            @short      Gets the menu bar part of all addon components registered
            @descr      -

            @seealso    -

            @return     A complete

            @onerror    We return sal_False
        *//*-*****************************************************************************************************/

        const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > >& GetAddonsMenuBarPart() const;

        /*-****************************************************************************************************//**
            @short      Gets a toolbar part of an single addon
            @descr      -

            @seealso    -

            @return     A complete

            @onerror    We return sal_False
        *//*-*****************************************************************************************************/

        const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > >& GetAddonsToolBarPart( sal_uInt32 nIndex ) const;

        /*-****************************************************************************************************//**
            @short      Gets a unique toolbar resource name of an single addon
            @descr      -

            @seealso    -

            @return     A complete

            @onerror    We return sal_False
        *//*-*****************************************************************************************************/

        const ::rtl::OUString GetAddonsToolbarResourceName( sal_uInt32 nIndex ) const;

        /*-****************************************************************************************************//**
            @short      Retrieves all available merge instructions for the Office menu bar
            @descr      -

            @seealso    -

            @return     The filled MergeMenuDefinitionContaier

            @onerror    We return sal_False
        *//*-*****************************************************************************************************/

        const MergeMenuInstructionContainer& GetMergeMenuInstructions() const;

        /*-****************************************************************************************************//**
            @short      Retrieves all available merge instructions for a single toolbar
            @descr      -

            @seealso    -

            @return     The filled

            @onerror    We return sal_False
        *//*-*****************************************************************************************************/
        bool GetMergeToolbarInstructions( const ::rtl::OUString& rToolbarName, MergeToolbarInstructionContainer& rToolbar ) const;

        /*-****************************************************************************************************//**
            @short      Gets the Add-On help menu part of all addon components registered
            @descr      -

            @seealso    -

            @return     A complete

            @onerror    We return sal_False
        *//*-*****************************************************************************************************/
        const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > >& GetAddonsHelpMenu() const;

        /*-****************************************************************************************************//**
            @short      Retrieve an image for a command URL which is defined inside the addon menu configuration
            @descr      Call it to retrieve an image for a command URL which is defined inside the addon menu configuration

            @seealso    -

            @return     An image which was defined in the configuration for the menu item. The image can be empty
                        no bitmap was defined for the request image properties.

            @onerror    An empty image
        *//*-*****************************************************************************************************/

        Image GetImageFromURL( const rtl::OUString& aURL, sal_Bool bBig, sal_Bool bNoScale ) const;
        Image GetImageFromURL( const rtl::OUString& aURL, sal_Bool bBig ) const;

    //-------------------------------------------------------------------------------------------------------------
    //  private methods
    //-------------------------------------------------------------------------------------------------------------

        /*-****************************************************************************************************//**
            @short      return a reference to a static mutex
            @descr      These class is partially threadsafe (for de-/initialization only).
                        All access methods are'nt safe!
                        We create a static mutex only for one ime and use at different times.

            @seealso    -

            @param      -
            @return     A reference to a static mutex member.

            @onerror    -
        *//*-*****************************************************************************************************/

        static ::osl::Mutex& GetOwnStaticMutex();

        /*-****************************************************************************************************//**
            @short      return a reference to a static mutex
            @descr      These class is partially threadsafe (for de-/initialization only).
                        All access methods are'nt safe!
                        We create a static mutex only for one ime and use at different times.

            @seealso    -

            @param      -
            @return     A reference to a static mutex member.

            @onerror    -
        *//*-*****************************************************************************************************/
        DECL_STATIC_LINK( AddonsOptions, Notify, void* );

    //-------------------------------------------------------------------------------------------------------------
    //  private member
    //-------------------------------------------------------------------------------------------------------------

    private:

        /*Attention

            Don't initialize these static member in these header!
            a) Double dfined symbols will be detected ...
            b) and unresolved externals exist at linking time.
            Do it in your source only.
         */

        static AddonsOptions_Impl*  m_pDataContainer    ;   /// impl. data container as dynamic pointer for smaller memory requirements!
        static sal_Int32            m_nRefCount         ;   /// internal ref count mechanism

};      // class SvtMenuOptions

}

#endif  // #ifndef __FRAMEWORK_CLASSES_ADDONSOPTIONS_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

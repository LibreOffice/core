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
                    Use follow defines to separate values by names.
*//*-*************************************************************************************************************/
#define ADDONSMENUITEM_STRING_URL                       "URL"
#define ADDONSMENUITEM_STRING_TITLE                     "Title"
#define ADDONSMENUITEM_STRING_TARGET                    "Target"
#define ADDONSMENUITEM_STRING_IMAGEIDENTIFIER           "ImageIdentifier"
#define ADDONSMENUITEM_STRING_CONTEXT                   "Context"
#define ADDONSMENUITEM_STRING_SUBMENU                   "Submenu"
#define ADDONSMENUITEM_STRING_CONTROLTYPE               "ControlType"
#define ADDONSMENUITEM_STRING_WIDTH                     "Width"

#define STATUSBARITEM_STRING_ALIGN                      "Alignment"
#define STATUSBARITEM_STRING_AUTOSIZE                   "AutoSize"
#define STATUSBARITEM_STRING_OWNERDRAW                  "OwnerDraw"

#define ADDONSMENUITEM_URL_LEN                          3
#define ADDONSMENUITEM_TITLE_LEN                        5
#define ADDONSMENUITEM_TARGET_LEN                       6
#define ADDONSMENUITEM_SUBMENU_LEN                      7
#define ADDONSMENUITEM_CONTEXT_LEN                      7
#define ADDONSMENUITEM_IMAGEIDENTIFIER_LEN              15

#define ADDONSMENUITEM_PROPERTYNAME_URL                 OUString(ADDONSMENUITEM_STRING_URL             )
#define ADDONSMENUITEM_PROPERTYNAME_TITLE               OUString(ADDONSMENUITEM_STRING_TITLE           )
#define ADDONSMENUITEM_PROPERTYNAME_TARGET              OUString(ADDONSMENUITEM_STRING_TARGET          )
#define ADDONSMENUITEM_PROPERTYNAME_IMAGEIDENTIFIER     OUString(ADDONSMENUITEM_STRING_IMAGEIDENTIFIER )
#define ADDONSMENUITEM_PROPERTYNAME_CONTEXT             OUString(ADDONSMENUITEM_STRING_CONTEXT         )
#define ADDONSMENUITEM_PROPERTYNAME_SUBMENU             OUString(ADDONSMENUITEM_STRING_SUBMENU         )
#define ADDONSMENUITEM_PROPERTYNAME_CONTROLTYPE         OUString(ADDONSMENUITEM_STRING_CONTROLTYPE     )
#define ADDONSMENUITEM_PROPERTYNAME_WIDTH               OUString(ADDONSMENUITEM_STRING_WIDTH           )

#define STATUSBARITEM_PROPERTYNAME_ALIGN                OUString(STATUSBARITEM_STRING_ALIGN            )
#define STATUSBARITEM_PROPERTYNAME_AUTOSIZE             OUString(STATUSBARITEM_STRING_AUTOSIZE         )
#define STATUSBARITEM_PROPERTYNAME_OWNERDRAW            OUString(STATUSBARITEM_STRING_OWNERDRAW        )

#define ADDONSPOPUPMENU_URL_PREFIX_STR                  "private:menu/Addon"

#define ADDONSPOPUPMENU_URL_PREFIX                      OUString( ADDONSPOPUPMENU_URL_PREFIX_STR )

namespace framework
{

typedef ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > > MergeMenuDefinition;

struct FWE_DLLPUBLIC MergeMenuInstruction
{
    OUString     aMergePoint;
    OUString     aMergeCommand;
    OUString     aMergeCommandParameter;
    OUString     aMergeFallback;
    OUString     aMergeContext;
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > > aMergeMenu;
};
typedef ::std::vector< MergeMenuInstruction > MergeMenuInstructionContainer;

struct FWE_DLLPUBLIC MergeToolbarInstruction
{
    OUString     aMergeToolbar;
    OUString     aMergePoint;
    OUString     aMergeCommand;
    OUString     aMergeCommandParameter;
    OUString     aMergeFallback;
    OUString     aMergeContext;
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > > aMergeToolbarItems;
};

typedef ::std::vector< MergeToolbarInstruction > MergeToolbarInstructionContainer;

struct FWE_DLLPUBLIC MergeStatusbarInstruction
{
    ::rtl::OUString     aMergePoint;
    ::rtl::OUString     aMergeCommand;
    ::rtl::OUString     aMergeCommandParameter;
    ::rtl::OUString     aMergeFallback;
    ::rtl::OUString     aMergeContext;
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > > aMergeStatusbarItems;
};

typedef ::std::vector< MergeStatusbarInstruction > MergeStatusbarInstructionContainer;

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

/*-************************************************************************************************************//**
    @short          collect information about menu features
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

        const OUString GetAddonsToolbarResourceName( sal_uInt32 nIndex ) const;

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
        bool GetMergeToolbarInstructions( const OUString& rToolbarName, MergeToolbarInstructionContainer& rToolbar ) const;

        /*-****************************************************************************************************//**
            @short      Gets the Add-On help menu part of all addon components registered
            @descr      -

            @seealso    -

            @return     A complete

            @onerror    We return sal_False
        *//*-*****************************************************************************************************/
        const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > >& GetAddonsHelpMenu() const;

        const MergeStatusbarInstructionContainer& GetMergeStatusbarInstructions() const;

        /*-****************************************************************************************************//**
            @short      Retrieve an image for a command URL which is defined inside the addon menu configuration
            @descr      Call it to retrieve an image for a command URL which is defined inside the addon menu configuration

            @seealso    -

            @return     An image which was defined in the configuration for the menu item. The image can be empty
                        no bitmap was defined for the request image properties.

            @onerror    An empty image
        *//*-*****************************************************************************************************/

        Image GetImageFromURL( const OUString& aURL, sal_Bool bBig, sal_Bool bNoScale ) const;
        Image GetImageFromURL( const OUString& aURL, sal_Bool bBig ) const;

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

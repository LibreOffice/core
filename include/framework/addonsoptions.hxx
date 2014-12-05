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
#ifndef INCLUDED_FRAMEWORK_ADDONSOPTIONS_HXX
#define INCLUDED_FRAMEWORK_ADDONSOPTIONS_HXX

#include <sal/types.h>
#include <osl/mutex.hxx>
#include <vcl/image.hxx>
#include <vcl/bitmap.hxx>
#include <vcl/svapp.hxx>
#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <framework/fwedllapi.h>

//  types, enums, ...


/*-************************************************************************************************************
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


//  forward declarations


/*-************************************************************************************************************
    @short          forward declaration to our private date container implementation
    @descr          We use these class as internal member to support small memory requirements.
                    You can create the container if it is necessary. The class which use these mechanism
                    is faster and smaller then a complete implementation!
*//*-*************************************************************************************************************/

class AddonsOptions_Impl;

/*-************************************************************************************************************
    @short          collect information about menu features
    @devstatus      ready to use
*//*-*************************************************************************************************************/

class FWE_DLLPUBLIC AddonsOptions
{

    //  public methods


    public:

        //  constructor / destructor


        /*-****************************************************************************************************
            @short      standard constructor and destructor
            @descr      This will initialize an instance with default values.
                        We implement these class with a refcount mechanism! Every instance of this class increase it
                        at create and decrease it at delete time - but all instances use the same data container!
                        He is implemented as a static member ...

            @seealso    member m_nRefCount
            @seealso    member m_pDataContainer
        *//*-*****************************************************************************************************/

         AddonsOptions();
        ~AddonsOptions();


        //  interface


        /*-****************************************************************************************************
            @short      clears completely the addons menu
            @descr      Call this methods to clear the addons menu
                        To fill it again use AppendItem().
            @param      "eMenu" select right menu to clear.
        *//*-*****************************************************************************************************/

        void Clear();

        /*-****************************************************************************************************
            @short      returns if an addons menu is available
            @descr      Call to retrieve if a addons menu is available


            @return     true if there is a menu otherwise false
        *//*-*****************************************************************************************************/

        bool    HasAddonsMenu() const;

        /*-****************************************************************************************************
            @short      returns number of addons toolbars
            @descr      Call to retrieve the number of addons toolbars


            @return     number of addons toolbars
        *//*-*****************************************************************************************************/
        sal_Int32   GetAddonsToolBarCount() const ;

        /*-****************************************************************************************************
            @short      returns the  complete addons menu
            @descr      Call it to get all entries of the addon menu.
                        We return a list of all nodes with his names and properties.
            @return     A list of menu items is returned.

            @onerror    We return an empty list.
        *//*-*****************************************************************************************************/

        const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > >& GetAddonsMenu() const;

        /*-****************************************************************************************************
            @short      Gets the menu bar part of all addon components registered
            @return     A complete

            @onerror    We return sal_False
        *//*-*****************************************************************************************************/

        const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > >& GetAddonsMenuBarPart() const;

        /*-****************************************************************************************************
            @short      Gets a toolbar part of an single addon
            @return     A complete

            @onerror    We return sal_False
        *//*-*****************************************************************************************************/

        const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > >& GetAddonsToolBarPart( sal_uInt32 nIndex ) const;

        /*-****************************************************************************************************
            @short      Gets a unique toolbar resource name of an single addon
            @return     A complete

            @onerror    We return sal_False
        *//*-*****************************************************************************************************/

        const OUString GetAddonsToolbarResourceName( sal_uInt32 nIndex ) const;

        /*-****************************************************************************************************
            @short      Retrieves all available merge instructions for the Office menu bar
            @return     The filled MergeMenuDefinitionContaier

            @onerror    We return sal_False
        *//*-*****************************************************************************************************/

        const MergeMenuInstructionContainer& GetMergeMenuInstructions() const;

        /*-****************************************************************************************************
            @short      Retrieves all available merge instructions for a single toolbar
            @return     The filled

            @onerror    We return sal_False
        *//*-*****************************************************************************************************/
        bool GetMergeToolbarInstructions( const OUString& rToolbarName, MergeToolbarInstructionContainer& rToolbar ) const;

        /*-****************************************************************************************************
            @short      Gets the Add-On help menu part of all addon components registered
            @return     A complete

            @onerror    We return sal_False
        *//*-*****************************************************************************************************/
        const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > >& GetAddonsHelpMenu() const;

        const MergeStatusbarInstructionContainer& GetMergeStatusbarInstructions() const;

        /*-****************************************************************************************************
            @short      Retrieve an image for a command URL which is defined inside the addon menu configuration
            @descr      Call it to retrieve an image for a command URL which is defined inside the addon menu configuration
            @return     An image which was defined in the configuration for the menu item. The image can be empty
                        no bitmap was defined for the request image properties.

            @onerror    An empty image
        *//*-*****************************************************************************************************/

        Image GetImageFromURL( const OUString& aURL, bool bBig, bool bNoScale ) const;
        Image GetImageFromURL( const OUString& aURL, bool bBig ) const;


    //  private methods


        /*-****************************************************************************************************
            @short      return a reference to a static mutex
            @descr      These class is partially threadsafe (for de-/initialization only).
                        All access methods are'nt safe!
                        We create a static mutex only for one ime and use at different times.
            @return     A reference to a static mutex member.
        *//*-*****************************************************************************************************/

        static ::osl::Mutex& GetOwnStaticMutex();

        /*-****************************************************************************************************
            @short      return a reference to a static mutex
            @descr      These class is partially threadsafe (for de-/initialization only).
                        All access methods are'nt safe!
                        We create a static mutex only for one ime and use at different times.
            @return     A reference to a static mutex member.
        *//*-*****************************************************************************************************/
        DECL_STATIC_LINK( AddonsOptions, Notify, void* );


    //  private member


    private:

        /*Attention

            Don't initialize these static members in these headers!
            a) Double defined symbols will be detected ...
            b) and unresolved externals exist at linking time.
            Do it in your source only.
         */

        static AddonsOptions_Impl*  m_pDataContainer    ;
        static sal_Int32            m_nRefCount         ;

};      // class SvtMenuOptions

}

#endif // INCLUDED_FRAMEWORK_ADDONSOPTIONS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

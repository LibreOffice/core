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
#include <vcl/image.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/uno/Sequence.h>
#include <framework/fwedllapi.h>
#include <memory>

namespace osl { class Mutex; }

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

#define ADDONSPOPUPMENU_URL_PREFIX_STR                  "private:menu/Addon"

namespace framework
{

struct FWE_DLLPUBLIC MergeMenuInstruction
{
    OUString     aMergePoint;
    OUString     aMergeCommand;
    OUString     aMergeCommandParameter;
    OUString     aMergeFallback;
    OUString     aMergeContext;
    css::uno::Sequence< css::uno::Sequence< css::beans::PropertyValue > > aMergeMenu;
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
    css::uno::Sequence< css::uno::Sequence< css::beans::PropertyValue > > aMergeToolbarItems;
};

typedef ::std::vector< MergeToolbarInstruction > MergeToolbarInstructionContainer;

struct FWE_DLLPUBLIC MergeStatusbarInstruction
{
    OUString     aMergePoint;
    OUString     aMergeCommand;
    OUString     aMergeCommandParameter;
    OUString     aMergeContext;
    css::uno::Sequence< css::uno::Sequence< css::beans::PropertyValue > > aMergeStatusbarItems;
};

typedef ::std::vector< MergeStatusbarInstruction > MergeStatusbarInstructionContainer;

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
    public:
         AddonsOptions();
        ~AddonsOptions();

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
                        We return a list of all nodes with its names and properties.
            @return     A list of menu items is returned.

            @onerror    We return an empty list.
        *//*-*****************************************************************************************************/

        const css::uno::Sequence< css::uno::Sequence< css::beans::PropertyValue > >& GetAddonsMenu() const;

        /*-****************************************************************************************************
            @short      Gets the menu bar part of all addon components registered
            @return     A complete

            @onerror    We return sal_False
        *//*-*****************************************************************************************************/

        const css::uno::Sequence< css::uno::Sequence< css::beans::PropertyValue > >& GetAddonsMenuBarPart() const;

        /*-****************************************************************************************************
            @short      Gets a toolbar part of an single addon
            @return     A complete

            @onerror    We return sal_False
        *//*-*****************************************************************************************************/

        const css::uno::Sequence< css::uno::Sequence< css::beans::PropertyValue > >& GetAddonsToolBarPart( sal_uInt32 nIndex ) const;

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
        const css::uno::Sequence< css::uno::Sequence< css::beans::PropertyValue > >& GetAddonsHelpMenu() const;

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
                        All access methods aren't safe!
                        We create a static mutex only for one ime and use at different times.
            @return     A reference to a static mutex member.
        *//*-*****************************************************************************************************/

        static ::osl::Mutex& GetOwnStaticMutex();

    private:
        std::shared_ptr<AddonsOptions_Impl>  m_pImpl;
};

}

#endif // INCLUDED_FRAMEWORK_ADDONSOPTIONS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

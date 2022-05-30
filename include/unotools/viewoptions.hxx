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
#ifndef INCLUDED_UNOTOOLS_VIEWOPTIONS_HXX
#define INCLUDED_UNOTOOLS_VIEWOPTIONS_HXX

#include <unotools/unotoolsdllapi.h>
#include <com/sun/star/uno/Sequence.hxx>
#include <sal/types.h>
#include <rtl/ustring.hxx>

namespace com::sun::star::beans { struct NamedValue; }
namespace com::sun::star::container { class XNameAccess; }

/*-************************************************************************************************************
    @descr          Use these enum values to specify right list in configuration in which your view data are saved.
*//*-*************************************************************************************************************/

enum class EViewType
{
    Dialog    =   0,
    TabDialog =   1,
    TabPage   =   2,
    Window    =   3
};

/*-************************************************************************************************************
    @short          collect information about view features
    @descr          We support different basetypes of views like dialogs, tab-dialogs, tab-pages and normal windows.
                    You must specify your basetype by using right enum value and must give us a valid name for your
                    subkey in registry! We support some fix features for some bastypes and user data as string for all!
                    see also configuration package "org.openoffice.Office.Views/..." for further information.

                    template of configuration:
                        DialogType
                            /WindowState    [string]
                            /UserData       [set of any scalar types]
                        TabDialogType
                            /WindowState    [string]
                            /UserData       [set of any scalar types]
                            /PageID         [int]
                        TabPageType
                            /WindowState    [string]
                            /UserData       [set of any scalar types]
                        WindowType
                            /WindowState    [string]
                            /UserData       [set of any scalar types]
                            /Visible        [boolean]

                    structure of configuration:

                        org.openoffice.Office.Views [package]
                                                    /Dialogs    [set]
                                                                /Dialog_FileOpen            [DialogType]
                                                                /Dialog_ImportGraphics      [DialogType]
                                                                ...
                                                                /Dialog_<YourName>          [DialogType]

                                                    /TabDialogs [set]
                                                                /TabDialog_001              [TabDialogType]
                                                                /TabDialog_Blubber          [TabDialogType]
                                                                ...
                                                                /TabDialog_<YourName>       [TabDialogType]

                                                    /TabPages   [set]
                                                                /TabPage_XXX                [TabPageType]
                                                                /TabPage_Date               [TabPageType]
                                                                ...
                                                                /TabPage_<YourName>         [TabPageType]

                                                    /Windows    [set]
                                                                /Window_User                [WindowType]
                                                                /Window_Options             [WindowType]
                                                                ...
                                                                /Window_<YourName>          [WindowType]
    @devstatus      ready to use
*//*-*************************************************************************************************************/

class SAL_WARN_UNUSED UNOTOOLS_DLLPUBLIC SvtViewOptions final
{

    //  public methods

    public:

        //  constructor / destructor

        /*-****************************************************************************************************
            @short      standard constructor and destructor
            @descr      This will de-/initialize an instance with default values.
                        You must give us the basic type of your view and a name which specify right entry
                        in dynamical configuration list. If entry not exist, we create a new one!

            @seealso    enum EViewType

            @param      "eType" specify type of your view and is used to use right data container!
            @param      "sViewName" specify the name of your view and is the key name in data list too.
            @onerror    An assertion is thrown in debug version. Otherwise we do nothing!
        *//*-*****************************************************************************************************/

         SvtViewOptions( EViewType eType, OUString sViewName );

        //  interface

        /*-****************************************************************************************************
            @short      use it to get information about existing entries in configuration
            @descr      The methods to set/get the position or size will create a new entry automatically if
                        it not already exist and work with default values!
                        If this a problem for you - you MUST call these method before and
                        you must make up your own mind about that.
            @onerror    No error should occur.
        *//*-*****************************************************************************************************/

        bool Exists() const;

        /*-****************************************************************************************************
            @short      use it to delete an entry of dynamic view set
            @descr      You can use this method to delete an existing node in configuration.
                        But if you call a Set- or Get- method again on this instance
                        the item is created again! If you do nothing after this call
                        your view will die relay in configuration...

            @seealso    method Exist()
        *//*-*****************************************************************************************************/

        void Delete();

        /*-****************************************************************************************************
            @short      use it to set/get the window state of your view
            @descr      These value describe position/size and some other states of a window.
                        Use it with right vcl methods directly. Normally it's not necessary to
                        parse given string!

            @seealso    vcl methods
        *//*-*****************************************************************************************************/

        OUString GetWindowState(                               ) const;
        void            SetWindowState( const OUString& sState );

        /*-****************************************************************************************************
            @short      use it to set/get the page number which was the last active one
            @descr      It's only supported for:    - tab-dialogs
                        If you call it for other ones you will get an assertion in debug version.
                        In a product version we do nothing!
            @onerror    An assertion is thrown in debug version. Otherwise we do nothing!
        *//*-*****************************************************************************************************/

        OString GetPageID() const;
        void      SetPageID(std::string_view rID);

        /*-****************************************************************************************************
            @short      use it to set/get the visual state of a window
            @descr      It's only supported for:    - windows
                        If you call it for other ones you will get an assertion in debug version.
                        In a product version we do nothing!
            @onerror    An assertion is thrown in debug version. Otherwise we do nothing!
        *//*-*****************************************************************************************************/

        bool IsVisible (                 ) const;
        void     SetVisible( bool bState );

        /** Return true if the "Visible" property actually has a non-nil value

            (IsVisible will somewhat arbitrarily return false if the property is
            nil.)
        */
        bool HasVisible() const;

        /*-****************************************************************************************************
            @short      use it to set/get the extended user data (consisting of a set of named scalar values)
            @descr      It's supported for ALL types!
                        Every view can handle its own user defined data set.
            @onerror    In the non-product version, an assertion is made. In a product version, errors are silently ignored.
        *//*-*****************************************************************************************************/
        css::uno::Sequence< css::beans::NamedValue > GetUserData(                                                                                     ) const;
        void                                         SetUserData( const css::uno::Sequence< css::beans::NamedValue >& lData );

        /*-****************************************************************************************************
            @short      use it to set/get ONE special user data item directly
            @descr      Normally you can work on full user data list by using "Set/GetUserData()".
                        With this function you have an access on special list entries directly without any
            @onerror    In the non-product version, an assertion is made. In a product version, errors are silently ignored.
        *//*-*****************************************************************************************************/

        css::uno::Any GetUserItem( const OUString&            sName  ) const;
        void          SetUserItem( const OUString&            sName  ,
                                   const css::uno::Any& aValue );

    private:
        enum State { STATE_NONE, STATE_FALSE, STATE_TRUE };

        css::uno::Reference< css::uno::XInterface > impl_getSetNode( const OUString& sNode           ,
                                                                           bool         bCreateIfMissing) const;
        State GetVisible() const;

        /// specify which list of views in configuration is used! This can't be a static value!!!
        /// ... because we need this value to work with right static data container.
        EViewType           m_eViewType;
        OUString     m_sViewName;

        OUString                                           m_sListName;
        css::uno::Reference< css::container::XNameAccess > m_xRoot;
        css::uno::Reference< css::container::XNameAccess > m_xSet;
};      // class SvtViewOptions

#endif // INCLUDED_UNOTOOLS_VIEWOPTIONS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

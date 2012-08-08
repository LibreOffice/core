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
#ifndef INCLUDED_unotools_VIEWOPTIONS_HXX
#define INCLUDED_unotools_VIEWOPTIONS_HXX

#include "unotools/unotoolsdllapi.h"
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/beans/NamedValue.hpp>
#include <unotools/configmgr.hxx>
#include <unotools/configitem.hxx>
#include <sal/types.h>
#include <osl/mutex.hxx>
#include <rtl/ustring.hxx>
#include <unotools/options.hxx>

//_________________________________________________________________________________________________________________
//  forward declarations
//_________________________________________________________________________________________________________________

class SvtViewOptionsBase_Impl;

/*-************************************************************************************************************//**
    @descr          Use these enum values to specify right list in configuration in which your view data are saved.
*//*-*************************************************************************************************************/

enum EViewType
{
    E_DIALOG    =   0,
    E_TABDIALOG =   1,
    E_TABPAGE   =   2,
    E_WINDOW    =   3
};

/*-************************************************************************************************************//**
    @short          collect informations about view features
    @descr          We support different basetypes of views like dialogs, tab-dialogs, tab-pages and normal windows.
                    You must specify your basetype by using right enum value and must give us a valid name for your
                    subkey in registry! We support some fix features for some bastypes and user data as string for all!
                    see also configuration package "org.openoffice.Office.Views/..." for further informations.

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

    @implements     -
    @base           -

    @devstatus      ready to use
*//*-*************************************************************************************************************/

class UNOTOOLS_DLLPUBLIC SvtViewOptions: public utl::detail::Options
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
            @descr      This will de-/initialize an instance with default values.
                        You must give us the basic type of your view and a name which specify right entry
                        in dynamical configuration list. If entry not exist, we create a new one!

            @seealso    enum EViewType

            @param      "eType" specify type of your view and is used to use right data container!
            @param      "sViewName" specify the name of your view and is the key name in data list too.
            @return     -

            @onerror    An assertion is thrown in debug version. Otherwise we do nothing!
        *//*-*****************************************************************************************************/

         SvtViewOptions(       EViewType        eType     ,
                         const ::rtl::OUString& sViewName );
        virtual ~SvtViewOptions();

        /*-****************************************************************************************************//**
            @short      support preload of these config item
            @descr      Sometimes we need preloading of these configuration data without real using of it.

            @seealso    -

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        static void AcquireOptions();
        static void ReleaseOptions();

        //---------------------------------------------------------------------------------------------------------
        //  interface
        //---------------------------------------------------------------------------------------------------------

        /*-****************************************************************************************************//**
            @short      use it to get information about existing entries in configuration
            @descr      The methods to set/get the position or size will create a new entry automaticly if
                        it not already exist and work with default values!
                        If this a problem for you - you MUST call these method before and
                        you must make up your own mind about that.

            @seealso    -

            @onerror    No error should occure.
        *//*-*****************************************************************************************************/

        sal_Bool Exists() const;

        /*-****************************************************************************************************//**
            @short      use it to delete an entry of dynamic view set
            @descr      You can use this method to delete anexisting node in configuration.
                        But if you call a Set- or Get- method again on this instance
                        the item is created again! If you do nothing after this call
                        your view will die relay in configuration ...

            @seealso    method Exist()

            @return     True if delete OK, False otherwise.

            @onerror    We return sal_False.
        *//*-*****************************************************************************************************/

        sal_Bool Delete();

        /*-****************************************************************************************************//**
            @short      use it to set/get the window state of your view
            @descr      These value describe position/size and some other states of a window.
                        Use it with right vcl methods directly. Normaly it's not neccessary to
                        parse given string!

            @seealso    vcl methods

            @onerror    -
        *//*-*****************************************************************************************************/

        ::rtl::OUString GetWindowState(                               ) const;
        void            SetWindowState( const ::rtl::OUString& sState );

        /*-****************************************************************************************************//**
            @short      use it to set/get the page number which was the last active one
            @descr      It's only supported for:    - tab-dialogs
                        If you call it for other ones you will get an assertion in debug version.
                        In a product version we do nothing!

            @seealso    -

            @onerror    An assertion is thrown in debug version. Otherwise we do nothing!
        *//*-*****************************************************************************************************/

        sal_Int32 GetPageID(               ) const;
        void      SetPageID( sal_Int32 nID );

        /*-****************************************************************************************************//**
            @short      use it to set/get the visual state of a window
            @descr      It's only supported for:    - windows
                        If you call it for other ones you will get an assertion in debug version.
                        In a product version we do nothing!

            @seealso    -

            @onerror    An assertion is thrown in debug version. Otherwise we do nothing!
        *//*-*****************************************************************************************************/

        sal_Bool IsVisible (                 ) const;
        void     SetVisible( sal_Bool bState );

        /*-****************************************************************************************************//**
            @short      use it to set/get the extended user data (consisting of a set of named scalar values)
            @descr      It's supported for ALL types!
                        Every view can handle its own user defined data set.

            @seealso    -

            @onerror    In the non-product version, an assertion is made. In a product version, errors are silently ignored.
        *//*-*****************************************************************************************************/
        ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue > GetUserData(                                                                                     ) const;
        void                                                                   SetUserData( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue >& lData );

        /*-****************************************************************************************************//**
            @short      use it to set/get ONE special user data item directly
            @descr      Normaly you can work on full user data list by using "Set/GetUserData()".
                        With this function you have an access on special list entries directly without any

            @seealso    -

            @onerror    In the non-product version, an assertion is made. In a product version, errors are silently ignored.
        *//*-*****************************************************************************************************/

        ::com::sun::star::uno::Any GetUserItem( const ::rtl::OUString&            sName  ) const;
        void                       SetUserItem( const ::rtl::OUString&            sName  ,
                                                const ::com::sun::star::uno::Any& aValue );

    //-------------------------------------------------------------------------------------------------------------
    //  private methods
    //-------------------------------------------------------------------------------------------------------------

    private:

        /*-****************************************************************************************************//**
            @short      return a reference to a static mutex
            @descr      These class is threadsafe.
                        We create a static mutex only for one time and use it to protect our refcount and container
                        member!

            @seealso    -

            @param      -
            @return     A reference to a static mutex member.

            @onerror    -
        *//*-*****************************************************************************************************/

        UNOTOOLS_DLLPRIVATE static ::osl::Mutex& GetOwnStaticMutex();

    //-------------------------------------------------------------------------------------------------------------
    //  private member
    //-------------------------------------------------------------------------------------------------------------

    private:

        /// specify which list of views in configuration is used! This can't be a static value!!!
        /// ... because we need this value to work with right static data container.
        EViewType           m_eViewType     ;
        ::rtl::OUString     m_sViewName     ;

        /// - impl. data container as dynamic pointer for smaller memory requirements!
        /// - internal ref count mechanism

        /*Attention

            Don't initialize these static member in these header!
            a) Double defined symbols will be detected ...
            b) and unresolved externals exist at linking time.
            Do it in your source only.
         */

        static SvtViewOptionsBase_Impl*    m_pDataContainer_Dialogs    ;   /// hold data for all dialogs
        static sal_Int32                   m_nRefCount_Dialogs         ;
        static SvtViewOptionsBase_Impl*    m_pDataContainer_TabDialogs ;   /// hold data for all tab-dialogs
        static sal_Int32                   m_nRefCount_TabDialogs      ;
        static SvtViewOptionsBase_Impl*    m_pDataContainer_TabPages   ;   /// hold data for all tab-pages
        static sal_Int32                   m_nRefCount_TabPages        ;
        static SvtViewOptionsBase_Impl*    m_pDataContainer_Windows    ;   /// hold data for all windows
        static sal_Int32                   m_nRefCount_Windows         ;

};      // class SvtViewOptions

#endif  // #ifndef INCLUDED_unotools_VIEWOPTIONS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

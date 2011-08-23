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
#ifndef INCLUDED_SVTOOLS_VIEWOPTIONS_HXX
#define INCLUDED_SVTOOLS_VIEWOPTIONS_HXX

//_________________________________________________________________________________________________________________
//	includes
//_________________________________________________________________________________________________________________

#ifndef INCLUDED_SVLDLLAPI_H

#endif

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

#ifndef _COM_SUN_STAR_BEANS_NAMEDVALUE_HPP_
#include <com/sun/star/beans/NamedValue.hpp>
#endif

#ifndef _UTL_CONFIGMGR_HXX_
#include <unotools/configmgr.hxx>
#endif

#ifndef _UTL_CONFIGITEM_HXX_
#include <unotools/configitem.hxx>
#endif

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef INCLUDED_SVTOOLS_OPTIONS_HXX
#include <bf_svtools/options.hxx>
#endif

namespace binfilter
{

//_________________________________________________________________________________________________________________
//	forward declarations
//_________________________________________________________________________________________________________________

class SvtViewOptionsBase_Impl;

//_________________________________________________________________________________________________________________
//	declarations
//_________________________________________________________________________________________________________________

/*-************************************************************************************************************//**
    @descr			Use these enum values to specify right list in configuration in which your view data are saved.
*//*-*************************************************************************************************************/

enum EViewType
{
    E_DIALOG	=	0,
    E_TABDIALOG	=	1,
    E_TABPAGE	=	2,
    E_WINDOW	=	3
};

/*-************************************************************************************************************//**
    @short			collect informations about view features
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

                        org.openoffice.Office.Views	[package]
                                                    /Dialogs	[set]
                                                                /Dialog_FileOpen			[DialogType]
                                                                /Dialog_ImportGraphics		[DialogType]
                                                                ...
                                                                /Dialog_<YourName>			[DialogType]

                                                    /TabDialogs	[set]
                                                                /TabDialog_001				[TabDialogType]
                                                                /TabDialog_Blubber			[TabDialogType]
                                                                ...
                                                                /TabDialog_<YourName>		[TabDialogType]

                                                    /TabPages	[set]
                                                                /TabPage_XXX				[TabPageType]
                                                                /TabPage_Date				[TabPageType]
                                                                ...
                                                                /TabPage_<YourName>			[TabPageType]

                                                    /Windows	[set]
                                                                /Window_User				[WindowType]
                                                                /Window_Options				[WindowType]
                                                                ...
                                                                /Window_<YourName>			[WindowType]

    @implements		-
    @base			-

    @devstatus		ready to use
*//*-*************************************************************************************************************/

class  SvtViewOptions: public Options
{
    /*
    #ifdef TF_OLDVIEW
    public:
        void                                                                        GetPosition     (       sal_Int32&                                                                  nX              ,
                                                                                                            sal_Int32&                                                                  nY              ) const;
        void                                                                        SetPosition     (       sal_Int32                                                                   nX              ,
                                                                                                            sal_Int32                                                                   nY              );
        void                                                                        GetSize         (       sal_Int32&                                                                  nWidth          ,
                                                                                                            sal_Int32&                                                                  nHeight         ) const;
        void                                                                        SetSize         (       sal_Int32                                                                   nWidth          ,
                                                                                                            sal_Int32                                                                   nHeight         );
        ::rtl::OUString                                                             GetUserData     (                                                                                                   ) const;
        void                                                                        SetUserData     ( const ::rtl::OUString&                                                            sData           );
        static ::com::sun::star::uno::Sequence< ::rtl::OUString >                   SeperateUserData( const ::rtl::OUString&                                                            sData           ,
                                                                                                            sal_Unicode                                                                 cSeperator=';'  );
        static ::rtl::OUString                                                      GenerateUserData( const ::com::sun::star::uno::Sequence< ::rtl::OUString >&                         seqData         ,
                                                                                                            sal_Unicode                                                                 cSeperator=';'  );
        ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >   GetAnyData      (                                                                                                   ) const;
        void                                                                        SetAnyData      ( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >&  lData           );
    #endif
    */

    //-------------------------------------------------------------------------------------------------------------
    //	public methods
    //-------------------------------------------------------------------------------------------------------------

    public:

        //---------------------------------------------------------------------------------------------------------
        //	constructor / destructor
        //---------------------------------------------------------------------------------------------------------

        /*-****************************************************************************************************//**
            @short		standard constructor and destructor
            @descr		This will de-/initialize an instance with default values.
                        You must give us the basic type of your view and a name which specify right entry
                        in dynamical configuration list. If entry not exist, we create a new one!

            @seealso	enum EViewType

            @param		"eType" specify type of your view and is used to use right data container!
            @param		"sViewName" specify the name of your view and is the key name in data list too.
            @return		-

            @onerror	An assertion is thrown in debug version. Otherwise we do nothing!
        *//*-*****************************************************************************************************/

         SvtViewOptions(       EViewType        eType     ,
                         const ::rtl::OUString& sViewName );
        virtual ~SvtViewOptions();

    //-------------------------------------------------------------------------------------------------------------
    //	private methods
    //-------------------------------------------------------------------------------------------------------------

    private:

        /*-****************************************************************************************************//**
            @short		return a reference to a static mutex
            @descr		These class is threadsafe.
                        We create a static mutex only for one time and use it to protect our refcount and container
                        member!

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

        /// specify which list of views in configuration is used! This can't be a static value!!!
        /// ... because we need this value to work with right static data container.
        EViewType			m_eViewType		;
        ::rtl::OUString		m_sViewName		;

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

};		// class SvtViewOptions

}

#endif	// #ifndef INCLUDED_SVTOOLS_VIEWOPTIONS_HXX

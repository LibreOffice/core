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

// MARKER(update_precomp.py): autogen include statement, do not remove

//_________________________________________________________________________________________________________________
//	includes
//_________________________________________________________________________________________________________________

#include <bf_svtools/viewoptions.hxx>

#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif

#include <hash_map>

#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#ifndef UNOTOOLS_CONFIGPATHES_HXX_INCLUDED
#include <unotools/configpathes.hxx>
#endif

#ifndef _COMPHELPER_CONFIGURATIONHELPER_HXX_
#include <comphelper/configurationhelper.hxx>
#endif

#ifndef _UNOTOOLS_PROCESSFACTORY_HXX_
#include <unotools/processfactory.hxx>
#endif

#include <itemholder1.hxx>

namespace binfilter
{

//_________________________________________________________________________________________________________________
//	namespaces
//_________________________________________________________________________________________________________________

namespace css = ::com::sun::star;

//_________________________________________________________________________________________________________________
//	const
//_________________________________________________________________________________________________________________

#ifdef CONST_ASCII
    #error  "Who define CONST_ASCII before! I use it to create const ascii strings ..."
#else
    #define CONST_ASCII(SASCIIVALUE)            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SASCIIVALUE))
#endif

#define PATHSEPERATOR                           CONST_ASCII("/")

#define PACKAGE_VIEWS                           CONST_ASCII("org.openoffice.Office.Views")

#define LIST_DIALOGS                            CONST_ASCII("Dialogs"   )
#define LIST_TABDIALOGS                         CONST_ASCII("TabDialogs")
#define LIST_TABPAGES                           CONST_ASCII("TabPages"  )
#define LIST_WINDOWS                            CONST_ASCII("Windows"   )

#define PROPERTY_WINDOWSTATE                    CONST_ASCII("WindowState")
#define PROPERTY_PAGEID                         CONST_ASCII("PageID"     )
#define PROPERTY_VISIBLE                        CONST_ASCII("Visible"    )
#define PROPERTY_USERDATA                       CONST_ASCII("UserData"   )

#define PROPCOUNT_DIALOGS                       1
#define PROPCOUNT_TABDIALOGS                    2
#define PROPCOUNT_TABPAGES                      1
#define PROPCOUNT_WINDOWS                       2

#define DEFAULT_WINDOWSTATE                     ::rtl::OUString()
#define DEFAULT_USERDATA                        css::uno::Sequence< css::beans::NamedValue >()
#define DEFAULT_PAGEID                          0
#define DEFAULT_VISIBLE                         sal_False

//#define DEBUG_VIEWOPTIONS

#ifdef DEBUG_VIEWOPTIONS
    #define _LOG_COUNTER_( _SVIEW_, _NREAD_, _NWRITE_ )                                                                                     \
                {                                                                                                                           \
                    FILE* pFile = fopen( "viewdbg.txt", "a" );                                                                              \
                    fprintf( pFile, "%s[%d, %d]\n", ::rtl::OUStringToOString(_SVIEW_, RTL_TEXTENCODING_UTF8).getStr(), _NREAD_, _NWRITE_ ); \
                    fclose( pFile );                                                                                                        \
                }
#endif // DEBUG_VIEWOPTIONS

#define SVTVIEWOPTIONS_LOG_UNEXPECTED_EXCEPTION(SVTVIEWOPTIONS_LOG_UNEXPECTED_EXCEPTION_PARAM_EXCEPTION)            \
    {                                                                                                               \
        ::rtl::OUStringBuffer sMsg(256);                                                                            \
        sMsg.appendAscii("Unexpected exception catched. Original message was:\n\""      );                          \
        sMsg.append     (SVTVIEWOPTIONS_LOG_UNEXPECTED_EXCEPTION_PARAM_EXCEPTION.Message);                          \
        sMsg.appendAscii("\""                                                           );                          \
        OSL_ENSURE(sal_False, ::rtl::OUStringToOString(sMsg.makeStringAndClear(), RTL_TEXTENCODING_UTF8).getStr()); \
    }

//_________________________________________________________________________________________________________________
//	initialization!
//_________________________________________________________________________________________________________________

SvtViewOptionsBase_Impl*     SvtViewOptions::m_pDataContainer_Dialogs    =   NULL    ;
sal_Int32                    SvtViewOptions::m_nRefCount_Dialogs         =   0       ;
SvtViewOptionsBase_Impl*     SvtViewOptions::m_pDataContainer_TabDialogs =   NULL    ;
sal_Int32                    SvtViewOptions::m_nRefCount_TabDialogs      =   0       ;
SvtViewOptionsBase_Impl*     SvtViewOptions::m_pDataContainer_TabPages   =   NULL    ;
sal_Int32                    SvtViewOptions::m_nRefCount_TabPages        =   0       ;
SvtViewOptionsBase_Impl*     SvtViewOptions::m_pDataContainer_Windows    =   NULL    ;
sal_Int32                    SvtViewOptions::m_nRefCount_Windows         =   0       ;

//_________________________________________________________________________________________________________________
//	private declarations!
//_________________________________________________________________________________________________________________

/*-************************************************************************************************************//**
    @descr  declare one configuration item
            These struct hold information about one view item. But not all member are used for all entries!
            User must decide which information are usefull and which not. We are a container iztem only and doesnt
            know anything about the context.
            But; we support a feature:
                decision between items with default values (should not realy exist in configuration!)
                and items with real values - changed by user. So user can suppress saving of realy unused items
                to disk - because; defaulted items could be restored on runtime without reading from disk!!!
                And if only items with valid information was written to cfg - we mustn't read so much and save time.
            So we start with an member m_bDefault=True and reset it to False after first set-call.
            Deficiencies of these solution - we cant allow direct read/write access to our member. We must
            support it by set/get-methods ...
*//*-*************************************************************************************************************/
class IMPL_TViewData
{
    public:
        //---------------------------------------------------------------------------------------------------------
        // create "default" item
        IMPL_TViewData()
        {
            m_sWindowState = DEFAULT_WINDOWSTATE ;
            m_lUserData    = DEFAULT_USERDATA    ;
            m_nPageID      = DEFAULT_PAGEID      ;
            m_bVisible     = DEFAULT_VISIBLE     ;

            m_bDefault     = sal_True            ;
        }

        //---------------------------------------------------------------------------------------------------------
        // write access - with reseting of default state
        void setWindowState( const ::rtl::OUString& sValue )
        {
            m_bDefault     = (
                                ( m_bDefault == sal_True            )    &&
                                ( sValue     == DEFAULT_WINDOWSTATE )
                             );
            m_sWindowState = sValue;
        }

        //---------------------------------------------------------------------------------------------------------
        void setUserData( const css::uno::Sequence< css::beans::NamedValue >& lValue )
        {
            m_bDefault  = (
                            ( m_bDefault == sal_True         )    &&
                            ( lValue     == DEFAULT_USERDATA )
                          );
            m_lUserData = lValue;
        }

        //---------------------------------------------------------------------------------------------------------
        void setPageID( sal_Int32 nValue )
        {
            m_bDefault = (
                           ( m_bDefault == sal_True       )    &&
                           ( nValue     == DEFAULT_PAGEID )
                         );
            m_nPageID  = nValue;
        }

        //---------------------------------------------------------------------------------------------------------
        void setVisible( sal_Bool bValue )
        {
            m_bDefault = (
                           ( m_bDefault == sal_True        )    &&
                           ( bValue     == DEFAULT_VISIBLE )
                         );
            m_bVisible = bValue;
        }

        //---------------------------------------------------------------------------------------------------------
        // read access
        ::rtl::OUString                              getWindowState() { return m_sWindowState; }
        css::uno::Sequence< css::beans::NamedValue > getUserData   () { return m_lUserData   ; }
        sal_Int32                                    getPageID     () { return m_nPageID     ; }
        sal_Bool                                     getVisible    () { return m_bVisible    ; }

        //---------------------------------------------------------------------------------------------------------
        // special operation for easy access on user data
        void setUserItem( const ::rtl::OUString& sName  ,
                          const css::uno::Any&   aValue )
        {
            // we change UserData in every case!
            //    a) we change already existing item
            // or b) we add a new one
            m_bDefault = sal_False;

            sal_Bool  bExist = sal_False;
            sal_Int32 nCount = m_lUserData.getLength();

            // change it, if it already exist ...
            for( sal_Int32 nStep=0; nStep<nCount; ++nStep )
            {
                if( m_lUserData[nStep].Name == sName )
                {
                    m_lUserData[nStep].Value = aValue  ;
                    bExist                   = sal_True;
                    break;
                }
            }

            // ... or create new list item
            if( bExist == sal_False )
            {
                m_lUserData.realloc( nCount+1 );
                m_lUserData[nCount].Name  = sName  ;
                m_lUserData[nCount].Value = aValue ;
            }
        }

        //---------------------------------------------------------------------------------------------------------
        css::uno::Any getUserItem( const ::rtl::OUString& sName )
        {
            // default value - if item not exist!
            css::uno::Any aValue;

            sal_Int32 nCount = m_lUserData.getLength();
            for( sal_Int32 nStep=0; nStep<nCount; ++nStep )
            {
                if( m_lUserData[nStep].Name == sName )
                {
                    aValue = m_lUserData[nStep].Value;
                    break;
                }
            }
            return aValue;
        }

        //---------------------------------------------------------------------------------------------------------
        // check for default items
        sal_Bool isDefault() { return m_bDefault; }

    private:
        ::rtl::OUString                                 m_sWindowState    ;
        css::uno::Sequence< css::beans::NamedValue >    m_lUserData       ;
        sal_Int32                                       m_nPageID         ;
        sal_Bool                                        m_bVisible        ;

        sal_Bool                                        m_bDefault        ;
};

struct IMPL_TStringHashCode
{
    size_t operator()(const ::rtl::OUString& sString) const
    {
        return sString.hashCode();
    }
};

typedef ::std::hash_map< ::rtl::OUString                    ,
                         IMPL_TViewData                     ,
                         IMPL_TStringHashCode               ,
                         ::std::equal_to< ::rtl::OUString > > IMPL_TViewHash;

/*-************************************************************************************************************//**
    @descr          Implement base data container for view options elements.
                    Every item support ALL possible configuration informations.
                    But not every superclass should use them! Because some view types don't
                    have it realy.

    @attention      We implement a write-througt-cache! We use it for reading - but write all changes directly to
                    configuration. (changes are made on internal cache too!). So it's easier to distinguish
                    between added/changed/removed elements without any complex mask or bool flag informations.
                    Caches from configuration and our own one are synchronized every time - if we do so.
*//*-*************************************************************************************************************/
class SvtViewOptionsBase_Impl
{
    //-------------------------------------------------------------------------------------------------------------
    public:
                                                        SvtViewOptionsBase_Impl ( const ::rtl::OUString&                                sList    );
        virtual                                        ~SvtViewOptionsBase_Impl (                                                                );

    //-------------------------------------------------------------------------------------------------------------
    private:
        ::rtl::OUString                                    m_sListName;
        css::uno::Reference< css::container::XNameAccess > m_xRoot;
        css::uno::Reference< css::container::XNameAccess > m_xSet;

        #ifdef DEBUG_VIEWOPTIONS
        sal_Int32           m_nReadCount    ;
        sal_Int32           m_nWriteCount   ;
        #endif
};

/*-************************************************************************************************************//**
    @descr  Implement the base data container.
*//*-*************************************************************************************************************/

/*-************************************************************************************************************//**
    @short          ctor
    @descr          We use it to open right configuration file and let configuration objects fill her caches.
                    Then we read all existing entries from right list and cached it inside our object too.
                    Normaly we should enable notifications for changes on these values too ... but these feature
                    isn't full implemented in the moment.

    @seealso        baseclass ::utl::ConfigItem
    @seealso        method Notify()

    @param          -
    @return         -

    @last change    19.10.2001 07:54
*//*-*************************************************************************************************************/
SvtViewOptionsBase_Impl::SvtViewOptionsBase_Impl( const ::rtl::OUString& sList )
        :   m_sListName  ( sList )    // we must know, which view type we must support
        #ifdef DEBUG_VIEWOPTIONS
        ,   m_nReadCount ( 0     )
        ,   m_nWriteCount( 0     )
        #endif
{
    try
    {
        m_xRoot = css::uno::Reference< css::container::XNameAccess >(
                        ::comphelper::ConfigurationHelper::openConfig(
                            ::utl::getProcessServiceFactory(),
                            PACKAGE_VIEWS,
                            ::comphelper::ConfigurationHelper::E_STANDARD),
                        css::uno::UNO_QUERY);
        if (m_xRoot.is())
            m_xRoot->getByName(sList) >>= m_xSet;
    }
    catch(const css::uno::Exception& ex)
        {
            m_xRoot.clear();
            m_xSet.clear();

            SVTVIEWOPTIONS_LOG_UNEXPECTED_EXCEPTION(ex)
        }
}

/*-************************************************************************************************************//**
    @short          dtor
    @descr          If something was changed on our internal cached values - baselcass can tell us that by return value
                    of method "IsModified()". So we should flush these changes by calling "Commit()" of our own instance.
                    It's an auto-save. Normaly user of these object should do that explicitly!

    @attention      We implement a write through cache! So we mustn't do it realy. All changes was written to cfg directly.
                    Commit isn't neccessary then.

    @seealso        baseclass ::utl::ConfigItem
    @seealso        method IsModified()
    @seealso        method SetModified()
    @seealso        method Commit()

    @param          -
    @return         -

    @last change    19.10.2001 08:02
*//*-*************************************************************************************************************/
SvtViewOptionsBase_Impl::~SvtViewOptionsBase_Impl()
{
    try
    {
        if (m_xRoot.is())
            ::comphelper::ConfigurationHelper::flush(m_xRoot);
    }
    catch(const css::uno::Exception& ex)
        {
            SVTVIEWOPTIONS_LOG_UNEXPECTED_EXCEPTION(ex)
        }
    m_xRoot.clear();
    m_xSet.clear();

    #ifdef DEBUG_VIEWOPTIONS
    _LOG_COUNTER_( m_sListName, m_nReadCount, m_nWriteCount )
    #endif // DEBUG_VIEWOPTIONS
}

//_________________________________________________________________________________________________________________
//	definitions
//_________________________________________________________________________________________________________________

//*****************************************************************************************************************
//	constructor
//*****************************************************************************************************************
SvtViewOptions::SvtViewOptions(       EViewType        eType     ,
                                const ::rtl::OUString& sViewName )
    :	m_eViewType	( eType		)
    ,	m_sViewName	( sViewName	)
{
    // Global access, must be guarded (multithreading!)
    ::osl::MutexGuard aGuard( GetOwnStaticMutex() );

    // Search for right dat container for this view type and initialize right data container or set right ref count!
    switch( eType )
    {
        case E_DIALOG		:	{
                                    // Increase ref count for dialog data container first.
                                    ++m_nRefCount_Dialogs;
                                    // If these instance the first user of the dialog data container - create these impl static container!
                                    if( m_nRefCount_Dialogs == 1 )
                                    {
                                        //m_pDataContainer_Dialogs = new SvtViewDialogOptions_Impl( LIST_DIALOGS );
                                        m_pDataContainer_Dialogs = new SvtViewOptionsBase_Impl( LIST_DIALOGS );
                                        ItemHolder1::holdConfigItem(E_VIEWOPTIONS_DIALOG);
                                    }
                                }
                                break;
        case E_TABDIALOG	:	{
                                    // Increase ref count for tab-dialog data container first.
                                    ++m_nRefCount_TabDialogs;
                                    // If these instance the first user of the tab-dialog data container - create these impl static container!
                                    if( m_nRefCount_TabDialogs == 1 )
                                    {
                                        m_pDataContainer_TabDialogs = new SvtViewOptionsBase_Impl( LIST_TABDIALOGS );
                                        ItemHolder1::holdConfigItem(E_VIEWOPTIONS_TABDIALOG);
                                    }
                                }
                                break;
        case E_TABPAGE		:	{
                                    // Increase ref count for tab-page data container first.
                                    ++m_nRefCount_TabPages;
                                    // If these instance the first user of the tab-page data container - create these impl static container!
                                    if( m_nRefCount_TabPages == 1 )
                                    {
                                        m_pDataContainer_TabPages = new SvtViewOptionsBase_Impl( LIST_TABPAGES );
                                        ItemHolder1::holdConfigItem(E_VIEWOPTIONS_TABPAGE);
                                    }
                                }
                                break;
        case E_WINDOW		:	{
                                    // Increase ref count for window data container first.
                                    ++m_nRefCount_Windows;
                                    // If these instance the first user of the window data container - create these impl static container!
                                    if( m_nRefCount_Windows == 1 )
                                    {
                                        m_pDataContainer_Windows = new SvtViewOptionsBase_Impl( LIST_WINDOWS );
                                        ItemHolder1::holdConfigItem(E_VIEWOPTIONS_WINDOW);
                                    }
                                }
                                break;
        default             :   OSL_ENSURE( sal_False, "SvtViewOptions::SvtViewOptions()\nThese view type is unknown! All following calls at these instance will do nothing!\n" );
    }
}

//*****************************************************************************************************************
//	destructor
//*****************************************************************************************************************
SvtViewOptions::~SvtViewOptions()
{
    // Global access, must be guarded (multithreading!)
    ::osl::MutexGuard aGuard( GetOwnStaticMutex() );

    // Search for right dat container for this view type and deinitialize right data container or set right ref count!
    switch( m_eViewType )
    {
        case E_DIALOG		:	{
                                    // Decrease ref count for dialog data container first.
                                    --m_nRefCount_Dialogs;
                                    // If these instance the last user of the dialog data container - delete these impl static container!
                                    if( m_nRefCount_Dialogs == 0 )
                                    {
                                        delete m_pDataContainer_Dialogs;
                                        m_pDataContainer_Dialogs = NULL;
                                    }
                                }
                                break;
        case E_TABDIALOG	:	{
                                    // Decrease ref count for tab-dialog data container first.
                                    --m_nRefCount_TabDialogs;
                                    // If these instance the last user of the tab-dialog data container - delete these impl static container!
                                    if( m_nRefCount_TabDialogs == 0 )
                                    {
                                        delete m_pDataContainer_TabDialogs;
                                        m_pDataContainer_TabDialogs = NULL;
                                    }
                                }
                                break;
        case E_TABPAGE		:	{
                                    // Decrease ref count for tab-page data container first.
                                    --m_nRefCount_TabPages;
                                    // If these instance the last user of the tab-page data container - delete these impl static container!
                                    if( m_nRefCount_TabPages == 0 )
                                    {
                                        delete m_pDataContainer_TabPages;
                                        m_pDataContainer_TabPages = NULL;
                                    }
                                }
                                break;
        case E_WINDOW		:	{
                                    // Decrease ref count for window data container first.
                                    --m_nRefCount_Windows;
                                    // If these instance the last user of the window data container - delete these impl static container!
                                    if( m_nRefCount_Windows == 0 )
                                    {
                                        delete m_pDataContainer_Windows;
                                        m_pDataContainer_Windows = NULL;
                                    }
                                }
                                break;
    }
}

//*****************************************************************************************************************
//	private method
//*****************************************************************************************************************
::osl::Mutex& SvtViewOptions::GetOwnStaticMutex()
{
    // Initialize static mutex only for one time!
    static ::osl::Mutex* pMutex = NULL;
    // If these method first called (Mutex not already exist!) ...
    if( pMutex == NULL )
    {
        // ... we must create a new one. Protect follow code with the global mutex -
        // It must be - we create a static variable!
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
        // We must check our pointer again - because it can be that another instance of ouer class will be fastr then these!
        if( pMutex == NULL )
        {
            // Create the new mutex and set it for return on static variable.
            static ::osl::Mutex aMutex;
            pMutex = &aMutex;
        }
    }
    // Return new created or already existing mutex object.
    return *pMutex;
}

}

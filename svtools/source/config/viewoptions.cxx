/*************************************************************************
 *
 *  $RCSfile: viewoptions.cxx,v $
 *
 *  $Revision: 1.18 $
 *
 *  last change: $Author: as $ $Date: 2001-10-31 12:53:53 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#pragma hdrstop

//_________________________________________________________________________________________________________________
//  includes
//_________________________________________________________________________________________________________________

#include "viewoptions.hxx"

#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif

#ifndef __SGI_STL_HASH_MAP
#include <hash_map>
#endif

#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#ifndef UNOTOOLS_CONFIGPATHES_HXX_INCLUDED
#include <unotools/configpathes.hxx>
#endif

//_________________________________________________________________________________________________________________
//  namespaces
//_________________________________________________________________________________________________________________

namespace css = ::com::sun::star;

//_________________________________________________________________________________________________________________
//  const
//_________________________________________________________________________________________________________________

#ifdef CONST_ASCII
    #error  "Who define CONST_ASCII before! I use it to create const ascii strings ..."
#else
    #define CONST_ASCII(SASCIIVALUE)            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SASCIIVALUE))
#endif

#define PATHSEPERATOR                           CONST_ASCII("/"                       )

#define LIST_DIALOGS                            CONST_ASCII("Office.Views/Dialogs"    )
#define LIST_TABDIALOGS                         CONST_ASCII("Office.Views/TabDialogs" )
#define LIST_TABPAGES                           CONST_ASCII("Office.Views/TabPages"   )
#define LIST_WINDOWS                            CONST_ASCII("Office.Views/Windows"    )

#define PROPERTY_WINDOWSTATE                    CONST_ASCII("WindowState"             )
#define PROPERTY_PAGEID                         CONST_ASCII("PageID"                  )
#define PROPERTY_VISIBLE                        CONST_ASCII("Visible"                 )
#define PROPERTY_USERDATA                       CONST_ASCII("UserData"                )

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

//_________________________________________________________________________________________________________________
//  initialization!
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
//  private declarations!
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
class SvtViewOptionsBase_Impl : public ::utl::ConfigItem
{
    //-------------------------------------------------------------------------------------------------------------
    public:
                                                        SvtViewOptionsBase_Impl ( const ::rtl::OUString&                                sList    );
        virtual                                        ~SvtViewOptionsBase_Impl (                                                                );
        virtual void                                    Notify                  ( const css::uno::Sequence< ::rtl::OUString >&          lNames   );
        virtual void                                    Commit                  (                                                                );
        sal_Bool                                        Exists                  ( const ::rtl::OUString&                                sName    );
        sal_Bool                                        Delete                  ( const ::rtl::OUString&                                sName    );
        ::rtl::OUString                                 GetWindowState          ( const ::rtl::OUString&                                sName    );
        void                                            SetWindowState          ( const ::rtl::OUString&                                sName    ,
                                                                                  const ::rtl::OUString&                                sState   );
        css::uno::Sequence< css::beans::NamedValue >    GetUserData             ( const ::rtl::OUString&                                sName    );
        void                                            SetUserData             ( const ::rtl::OUString&                                sName    ,
                                                                                  const css::uno::Sequence< css::beans::NamedValue >&   lData    );
        sal_Int32                                       GetPageID               ( const ::rtl::OUString&                                sName    );
        void                                            SetPageID               ( const ::rtl::OUString&                                sName    ,
                                                                                        sal_Int32                                       nID      );
        sal_Bool                                        GetVisible              ( const ::rtl::OUString&                                sName    );
        void                                            SetVisible              ( const ::rtl::OUString&                                sName    ,
                                                                                        sal_Bool                                        bVisible );
        css::uno::Any                                   GetUserItem             ( const ::rtl::OUString&                                sName    ,
                                                                                  const ::rtl::OUString&                                sItem    );
        void                                            SetUserItem             ( const ::rtl::OUString&                                sName    ,
                                                                                  const ::rtl::OUString&                                sItem    ,
                                                                                  const css::uno::Any&                                  aValue   );

    //-------------------------------------------------------------------------------------------------------------
    private:
        void impl_ReadWholeList     (                               );  // fill internal cache with saved information from configuration
        void impl_createEmptySetNode( const ::rtl::OUString& sNode  );  // create an empty view item on disk
        void impl_writeDirectProp   ( const ::rtl::OUString& sNode  ,   // change one property of specified view item on disk
                                      const ::rtl::OUString& sProp  ,
                                      const void*            pValue );

    //-------------------------------------------------------------------------------------------------------------
    private:
        IMPL_TViewHash      m_aList         ;
        ::rtl::OUString     m_sListName     ;

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
        :   ConfigItem   ( sList )    // open right configuration file
        ,   m_sListName  ( sList )    // we must know, which view type we must support
        #ifdef DEBUG_VIEWOPTIONS
        ,   m_nReadCount ( 0     )
        ,   m_nWriteCount( 0     )
        #endif
{
    // Read complete list from configuration.
    impl_ReadWholeList();

/*
    // Enable notification for our whole set tree!
    // use "/" to do that!
    // Attention: If you use current existing entry names to do that - you never get a notification
    // for new created items!!!
    css::uno::Sequence< ::rtl::OUString > lNotifyList(1);
    lNotifyList[0] = ROOTNODE_DIALOGS;
    ConfigItem::EnableNotification( lNotifyList );
*/
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
    #ifdef DEBUG_VIEWOPTIONS
    _LOG_COUNTER_( m_sListName, m_nReadCount, m_nWriteCount )
    #endif // DEBUG_VIEWOPTIONS
/*
    if( ConfigItem::IsModified() == sal_True )
    {
        Commit();
    }
*/
}

/*-************************************************************************************************************//**
    @short          configuration use it to notify external changes on our interessted values
    @descr          If anywhere change configuration values otside these class - configuration tell it us by calling
                    these method. To use these mechanism - we must enable it during our own ctor. See there for further
                    informations.

    @attention      Not supported in the moment!

    @seealso        ctor
    @seealso        method EnableNotification()

    @param          "lNames", names of all changed configuration entries or her subnodes
    @return         -

    @last change    19.10.2001 08:02
*//*-*************************************************************************************************************/
void SvtViewOptionsBase_Impl::Notify( const css::uno::Sequence< ::rtl::OUString >& lNames )
{
    OSL_ENSURE( sal_False, "SvtViewOptionsBase_Impl::Notify()\nNot supported yet!\n" );
}

/*-************************************************************************************************************//**
    @short          write changed internal values to configuration
    @descr          If our internal values are changed - we should flush it to configuration file.
                    But - we implement a write-through-cache ... so we mustn't do anything here!

    @seealso        dtor
    @seealso        method IsModified()

    @param          -
    @return         -

    @last change    19.10.2001 08:02
*//*-*************************************************************************************************************/
void SvtViewOptionsBase_Impl::Commit()
{
}

/*-************************************************************************************************************//**
    @short          checks for already existing entries
    @descr          If user don't know, if an entry already exist - he can get this information by calling this method.
                    Otherwhise access (read access is enough!) will create it with default values!
                    But these default items exist at runtime in our cache only. They are written to disk only, if
                    user change her values realy.

    @seealso        member m_aList

    @param          "sName", name of entry to check exist state
    @return         true , if item exist
                    false, otherwise

    @last change    19.10.2001 08:02
*//*-*************************************************************************************************************/
sal_Bool SvtViewOptionsBase_Impl::Exists( const ::rtl::OUString& sName )
{
    return( m_aList.find(sName) != m_aList.end() );
}

/*-************************************************************************************************************//**
    @short          delete entry
    @descr          Use it to delete set entry by given name.

    @seealso        member m_aList

    @param          "sName", name of entry to delete it
    @return         true , if item not exist(!) or could be deleted (should be the same!)
                    false, otherwise

    @last change    19.10.2001 08:05
*//*-*************************************************************************************************************/
sal_Bool SvtViewOptionsBase_Impl::Delete( const ::rtl::OUString& sName )
{
    #ifdef DEBUG_VIEWOPTIONS
    ++m_nWriteCount;
    #endif

    IMPL_TViewHash::iterator pItem = m_aList.find(sName);
    if( pItem != m_aList.end() )
    {
        m_aList.erase( pItem );
        ConfigItem::ClearNodeSet( ::utl::wrapConfigurationElementName(sName) );
    }
    return sal_True;
}

/*-************************************************************************************************************//**
    @short          read/write access to cache view items and her properties
    @descr          Follow methods support read/write access to all cache view items.
                    If an entry doesnt exist - we create a new one with default values in memory ... not in disk!
                    If user change values of it - we take into our internal cache AND write it to configuration
                    instandly!

    @seealso        member m_sList

    @param          -
    @return         -

    @last change    19.10.2001 08:33
*//*-*************************************************************************************************************/
::rtl::OUString SvtViewOptionsBase_Impl::GetWindowState( const ::rtl::OUString& sName )
{
    #ifdef DEBUG_VIEWOPTIONS
    ++m_nReadCount;
    #endif

    return m_aList[sName].getWindowState();
}

//*****************************************************************************************************************
void SvtViewOptionsBase_Impl::SetWindowState( const ::rtl::OUString& sName  ,
                                              const ::rtl::OUString& sState )
{
    #ifdef DEBUG_VIEWOPTIONS
    ++m_nWriteCount;
    #endif

    if( m_aList[sName].getWindowState() != sState )
    {
        m_aList[sName].setWindowState( sState );
        impl_writeDirectProp( sName, PROPERTY_WINDOWSTATE, (void*)&sState );
    }
}

//*****************************************************************************************************************
css::uno::Sequence< css::beans::NamedValue > SvtViewOptionsBase_Impl::GetUserData( const ::rtl::OUString& sName )
{
    #ifdef DEBUG_VIEWOPTIONS
    ++m_nReadCount;
    #endif

    return m_aList[sName].getUserData();
}

//*****************************************************************************************************************
void SvtViewOptionsBase_Impl::SetUserData( const ::rtl::OUString&                              sName  ,
                                           const css::uno::Sequence< css::beans::NamedValue >& lData  )
{
    #ifdef DEBUG_VIEWOPTIONS
    ++m_nWriteCount;
    #endif

    if( m_aList[sName].getUserData() != lData )
    {
        m_aList[sName].setUserData( lData );

        // User data are special properties of a view item.
        // They are a subset of a set entry in configuration.
        // Normaly a set entry is FULL created with default values,
        // if any fixed property of them is written. But subsets
        // couldnt be created so easy. That's why we must
        // check for default items ( they shouldnt exist on disk ... !)
        // but they must be created, if we whish to set new real value on
        // UserData!!!
        if( m_aList[sName].isDefault() == sal_False )
        {
            impl_createEmptySetNode( sName );
        }

        impl_writeDirectProp( sName, PROPERTY_USERDATA, &lData );
    }
}

//*****************************************************************************************************************
css::uno::Any SvtViewOptionsBase_Impl::GetUserItem( const ::rtl::OUString& sName ,
                                                    const ::rtl::OUString& sItem )
{
    #ifdef DEBUG_VIEWOPTIONS
    ++m_nReadCount;
    #endif

    return m_aList[sName].getUserItem(sItem);
}

//*****************************************************************************************************************
void SvtViewOptionsBase_Impl::SetUserItem( const ::rtl::OUString& sName  ,
                                           const ::rtl::OUString& sItem  ,
                                           const css::uno::Any&   aValue )
{
    #ifdef DEBUG_VIEWOPTIONS
    ++m_nWriteCount;
    #endif

    if( m_aList[sName].getUserItem(sItem) != aValue )
    {
        // User data are special properties of a view item.
        // They are a subset of a set entry in configuration.
        // Normaly a set entry is FULL created with default values,
        // if any fixed property of them is written. But subsets
        // couldnt be created so easy. That's why we must
        // check for default items ( they shouldnt exist on disk ... !)
        // but they must be created, if we whish to set new real value on
        // UserData!!!
        if( m_aList[sName].isDefault() == sal_True )
        {
            impl_createEmptySetNode( sName );
        }

        m_aList[sName].setUserItem(sItem, aValue);
        const css::uno::Sequence< css::beans::NamedValue > lData = m_aList[sName].getUserData();
        impl_writeDirectProp( sName, PROPERTY_USERDATA, &lData );
    }
}

//*****************************************************************************************************************
sal_Int32 SvtViewOptionsBase_Impl::GetPageID( const ::rtl::OUString& sName )
{
    #ifdef DEBUG_VIEWOPTIONS
    ++m_nReadCount;
    #endif

    return m_aList[sName].getPageID();
}

//*****************************************************************************************************************
void SvtViewOptionsBase_Impl::SetPageID( const ::rtl::OUString& sName ,
                                               sal_Int32        nID   )
{
    #ifdef DEBUG_VIEWOPTIONS
    ++m_nWriteCount;
    #endif

    if( m_aList[sName].getPageID() != nID )
    {
        m_aList[sName].setPageID( nID );
        impl_writeDirectProp( sName, PROPERTY_PAGEID, &nID );
    }
}

//*****************************************************************************************************************
sal_Bool SvtViewOptionsBase_Impl::GetVisible( const ::rtl::OUString& sName )
{
    #ifdef DEBUG_VIEWOPTIONS
    ++m_nReadCount;
    #endif

    return m_aList[sName].getVisible();
}

//*****************************************************************************************************************
void SvtViewOptionsBase_Impl::SetVisible( const ::rtl::OUString& sName    ,
                                                sal_Bool         bVisible )
{
    #ifdef DEBUG_VIEWOPTIONS
    ++m_nWriteCount;
    #endif

    if( m_aList[sName].getVisible() != bVisible )
    {
        m_aList[sName].setVisible( bVisible );
        impl_writeDirectProp( sName, PROPERTY_VISIBLE, &bVisible );
    }
}

/*-************************************************************************************************************//**
    @short          read complete configuration list
    @descr          These methods try to get all entries of right configuration list and fill it in our internal
                    cache.

    @seealso        member m_aList
    @seealso        baseclass ConfigItem

    @param          -
    @return         -

    @last change    19.10.2001 08:34
*//*-*************************************************************************************************************/
void SvtViewOptionsBase_Impl::impl_ReadWholeList()
{
    // Clear internal cache first!
    // This is the only way to clear all memory realy!
    IMPL_TViewHash().swap( m_aList );

    sal_Bool bBreaked = sal_False; // We check some invalid operation states during execution of these method.
                                   // If we found anyone - we set this value to TRUE. On the end we react
                                   // to this state by freeing internal cached values and warn programmer ...

    css::uno::Sequence< ::rtl::OUString >       lNames      = ConfigItem::GetNodeNames( ::rtl::OUString(), ::utl::CONFIG_NAME_LOCAL_PATH );
    sal_Int32                                   nNameCount  = lNames.getLength();
    sal_Int32                                   nAllCount   = 0;

    // Do nothing for empty lists!
    if( nNameCount > 0 )
    {
        // Calculate count of fix properties for current list entries.
        // So we save time by suppress reallocating of name sequence!
        if( m_sListName == LIST_DIALOGS )
            nAllCount = nNameCount*PROPCOUNT_DIALOGS;
        else
        if( m_sListName == LIST_TABDIALOGS )
            nAllCount = nNameCount*PROPCOUNT_TABDIALOGS;
        else
        if( m_sListName == LIST_TABPAGES )
            nAllCount = nNameCount*PROPCOUNT_TABPAGES;
        else
        if( m_sListName == LIST_WINDOWS )
            nAllCount = nNameCount*PROPCOUNT_WINDOWS;

        css::uno::Sequence< ::rtl::OUString > lAllNames( nAllCount );
        sal_Int32                             nAllStep = 0          ;
        ::rtl::OUString                       sPath                 ;

        for( sal_Int32 nNameStep=0; nNameStep<nNameCount; ++nNameStep )
        {
            // sPath = "*[xxx]/"
            sPath  = lNames[nNameStep] ;
            sPath += PATHSEPERATOR     ;

            // sPath     = "*[xxx]/"
            // lAllNames = "*[xxx]/WindowState"
            lAllNames[nAllStep] = sPath + PROPERTY_WINDOWSTATE;
            ++nAllStep;

            if( m_sListName == LIST_TABDIALOGS )
            {
                // sPath     = "*[xxx]/"
                // lAllNames = "*[xxx]/PageID"
                lAllNames[nAllStep] = sPath + PROPERTY_PAGEID;
                ++nAllStep;
            }

            if( m_sListName == LIST_WINDOWS )
            {
                // sPath     = "*[xxx]/"
                // lAllNames = "*[xxx]/Visible"
                lAllNames[nAllStep] = sPath + PROPERTY_VISIBLE;
                ++nAllStep;
            }

            // Handle "UserData" in a variable way. The could exist - but the must'nt!
                                                  // sPath = "*[xxx]/UserData"
                                                  sPath     += PROPERTY_USERDATA;
            css::uno::Sequence< ::rtl::OUString > lUserNames = ConfigItem::GetNodeNames( sPath, ::utl::CONFIG_NAME_LOCAL_PATH );
            sal_Int32                             nUserCount = lUserNames.getLength();
                                                  // sPath = "*[xxx]/UserData/"
                                                  sPath     += PATHSEPERATOR;

            // Reallocate name sequence, if any usre data was found and actualize nAllCount too!
            // These value is neccessary to find end of name and corresponding value list after reading values from configuration.
            if( nUserCount > 0 )
            {
                nAllCount += nUserCount;
                lAllNames.realloc( nAllCount );
            }

            for( sal_Int32 nUserStep=0; nUserStep<nUserCount; ++nUserStep )
            {
                // sPath     = "v_xxx/UserData/"
                // lAllNames = "v_xxx/UserData/<userprop>"
                lAllNames[nAllStep] = sPath + lUserNames[nUserStep];
                ++nAllStep;
            }
        }

        // Use builded list of full qualified names to get her values in corresponding list.
        css::uno::Sequence< css::uno::Any > lAllValues = ConfigItem::GetProperties( lAllNames );

        // Safe impossible cases.
        // We have asked for ALL our subtree keys and we would get all his values.
        // It's important for next loop and our index using!
        if( lAllNames.getLength()!=lAllValues.getLength() )
        {
            bBreaked = sal_True;
        }
        else
        {
            // step over both lists (name and value) and insert corresponding pairs into internal cache
            ::rtl::OUString sEntryName    ;
            ::rtl::OUString sPropertyName ;
            ::rtl::OUString sUserName     ;
            ::rtl::OUString sTemp         ;
            sal_Int32       nToken        ;
            IMPL_TViewData  aEntry        ;
                            nAllStep      = 0;

            while( nAllStep<nAllCount )
            {
                nToken        = 0;
                // lAllNames = "v_<entryname>/<propertyname[/<username>]>"
                // ... get "v_<entryname>" from lAllNames
                // ... extract "<entryname>" from sEntryName
                sEntryName    = lAllNames[nAllStep].getToken( 0, (sal_Unicode)'/', nToken );
                sEntryName    = ::utl::extractFirstFromConfigurationPath(sEntryName);

                if(
                    ( nToken                 < 0 )  ||  // nToken must be different from -1 ... because we search for sPropertyName in next line!
                    ( sEntryName.getLength() < 1 )      // An entry name is neccessary! We can't work with nothing.
                  )
                {
                    bBreaked = sal_True;
                    break;
                }

                // ... get "<propertyname>" from lAllNames
                sPropertyName = lAllNames[nAllStep].getToken( 0, (sal_Unicode)'/', nToken );

                if( sPropertyName.getLength() < 1 )
                {
                    // We must have a valid propertyname ... Otherwise we search value for "nothing"!
                    // Don't check nToken here - he could be -1 for fix properties ... but he could
                    // be >0 for UserData! Then exist some subnodes which should be readed!
                    bBreaked = sal_True;
                    break;
                }

                if( sPropertyName == PROPERTY_WINDOWSTATE )
                {
                    ::rtl::OUString sTemp;
                    lAllValues[nAllStep] >>= sTemp;
                    aEntry.setWindowState( sTemp );
                    ++nAllStep;

                }
                else
                if( sPropertyName == PROPERTY_PAGEID )
                {
                    sal_Int32 nTemp;
                    lAllValues[nAllStep] >>= nTemp;
                    aEntry.setPageID( nTemp );
                    ++nAllStep;
                }
                else
                if( sPropertyName == PROPERTY_VISIBLE )
                {
                    sal_Bool bTemp;
                    lAllValues[nAllStep] >>= bTemp;
                    aEntry.setVisible(bTemp);
                    ++nAllStep;
                }
                else
                if( sPropertyName == PROPERTY_USERDATA )
                {
                    while( sPropertyName == PROPERTY_USERDATA )
                    {
                        // ... extract "<username>" from sEntryName!
                        sUserName = lAllNames[nAllStep].getToken( 0, (sal_Unicode)'/', nToken );
                        sUserName = ::utl::extractFirstFromConfigurationPath(sUserName);
                        aEntry.setUserItem( sUserName, lAllValues[nAllStep] );

                        ++nAllStep;

                        if( nAllStep >= nAllCount )
                            break;

                        // starte new search for entry and property names.
                        // a)
                        //   If next entry in lAllNames is a fix property - these search will be superflous but
                        //   not dangerous. Because we start search on the beginning of these while-construct again.
                        //   But don't change nAllStep AFTER these lines. You must use SAME entry there!
                        // b)
                        //   If next entry is an UserData node - we get sPropertyName==PROPERTY_USERDATA!
                        //   So these "sub-while-construct" starts again to read next UserData entry ...
                        nToken        = 0;
                        sEntryName    = lAllNames[nAllStep].getToken( 0, (sal_Unicode)'/', nToken );
                        sEntryName    = ::utl::extractFirstFromConfigurationPath(sEntryName);
                        sPropertyName = lAllNames[nAllStep].getToken( 0, (sal_Unicode)'/', nToken );
                    }
                }
                else
                {
                    // There exist any unknown subnode as fix property.
                    // We can't handle these case ... so we should break operation!
                    bBreaked = sal_True;
                    break;
                }

                m_aList[sEntryName] = aEntry;
            }
        }
    }

    // If read operation was cancelled by any error - show a message for programmer and
    // clear internal caches. We can't decide between incomplete, wrong or right values.
    // So we should start with empty lists!
    if( bBreaked == sal_True )
    {
        OSL_ENSURE( sal_False, "SvtViewOptionsBase_Impl::impl_ReadWholeList()\nBreak reading completly. Unsupported configuration format found!\n" );
        IMPL_TViewHash().swap( m_aList );
        // A possible solution: SELF REPAIR!
        // Delete configuration entries in file too ...
        // It's hard - but could solve many problems.
        // Otherwhise these errors occure again and again and ...
        ConfigItem::ClearNodeSet( ::rtl::OUString() );
    }
}

/*-************************************************************************************************************//**
    @short          create new set node with default values on disk
    @descr          To create a new UserData item - the super node of these property must already exist!
                    You can call this method to create these new entry with default values and change UserData then.

    @seealso        method impl_writeDirectProp()

    @param          "sNode", name of new entry
    @return         -

    @last change    19.10.2001 08:42
*//*-*************************************************************************************************************/
void SvtViewOptionsBase_Impl::impl_createEmptySetNode( const ::rtl::OUString& sNode )
{
    css::uno::Sequence< css::beans::PropertyValue > lProps(1);
    ::rtl::OUString                                 sPath    ;

    sPath += ::utl::wrapConfigurationElementName(sNode);
    sPath += PATHSEPERATOR                             ;

    lProps[0].Name    = sPath + PROPERTY_WINDOWSTATE;
    lProps[0].Value <<= DEFAULT_WINDOWSTATE         ;

    if( m_sListName == LIST_TABDIALOGS )
    {
        lProps.realloc( lProps.getLength()+1 );
        lProps[lProps.getLength()-1].Name    = sPath + PROPERTY_PAGEID  ;
        lProps[lProps.getLength()-1].Value <<= (sal_Int32)DEFAULT_PAGEID;
    }

    if( m_sListName == LIST_WINDOWS )
    {
        lProps.realloc( lProps.getLength()+1 );
        lProps[lProps.getLength()-1].Name    = sPath + PROPERTY_VISIBLE ;
        lProps[lProps.getLength()-1].Value <<= DEFAULT_VISIBLE          ;
    }

    ConfigItem::SetSetProperties( ::rtl::OUString(), lProps );
}

/*-************************************************************************************************************//**
    @short          write one property of a view entry in cfg
    @descr          These write direct to configuration and change value of a view property.

    @seealso        method impl_createEmptySetNode()

    @param          "sNode" , name of new entry
    @param          "sProp" , name of property to change
    @param          "pValue", value of these property (real type depends on given property name)
    @return         -

    @last change    19.10.2001 08:44
*//*-*************************************************************************************************************/
void SvtViewOptionsBase_Impl::impl_writeDirectProp( const ::rtl::OUString& sNode  ,
                                                    const ::rtl::OUString& sProp  ,
                                                    const void*            pValue )
{
    ::rtl::OUStringBuffer sPath(100);
    sPath.append( ::utl::wrapConfigurationElementName(sNode));
    sPath.append( PATHSEPERATOR                             );
    sPath.append( sProp                                     );

    css::uno::Sequence< css::beans::PropertyValue > lProp(1);
    if( sProp == PROPERTY_WINDOWSTATE )
    {
        lProp[0].Name    = sPath.makeStringAndClear();
        lProp[0].Value <<= *((const ::rtl::OUString*)pValue);
        ConfigItem::SetSetProperties( ::rtl::OUString(), lProp );
    }
    else
    if( sProp == PROPERTY_PAGEID )
    {
        lProp[0].Name    = sPath.makeStringAndClear();
        lProp[0].Value <<= *((const sal_Int32*)pValue);
        ConfigItem::SetSetProperties( ::rtl::OUString(), lProp );
    }
    else
    if( sProp == PROPERTY_VISIBLE )
    {
        lProp[0].Name    = sPath.makeStringAndClear();
        lProp[0].Value <<= *((const sal_Bool*)pValue);
        ConfigItem::SetSetProperties( ::rtl::OUString(), lProp );
    }
    else
    if( sProp == PROPERTY_USERDATA )
    {
        ::rtl::OUString sBasePath = sPath.makeStringAndClear();

        const css::uno::Sequence< css::beans::NamedValue>* pData = (css::uno::Sequence< css::beans::NamedValue>*)pValue;
        sal_Int32 nCount = pData->getLength();
        sal_Int32 nStep  = 0;
        lProp.realloc(nCount);
        while( nStep<nCount )
        {
            if( (*pData)[nStep].Value.hasValue() == sal_False )
            {
                OSL_ENSURE( sal_False, "SvtViewOptionsBase_Impl::impl_writeDirectProp()\nCan't write UserData item with void-any as value!!! Item will be ignored ...\n" );
                --nCount;
                lProp.realloc(nCount);
            }
            else
            {
                lProp[nStep].Name  = sBasePath + PATHSEPERATOR + ::utl::wrapConfigurationElementName( (*pData)[nStep].Name );
                lProp[nStep].Value = (*pData)[nStep].Value;
                ++nStep;
            }
        }

        ConfigItem::ReplaceSetProperties( sBasePath, lProp );
    }
}

//_________________________________________________________________________________________________________________
//  definitions
//_________________________________________________________________________________________________________________

//*****************************************************************************************************************
//  constructor
//*****************************************************************************************************************
SvtViewOptions::SvtViewOptions(       EViewType        eType     ,
                                const ::rtl::OUString& sViewName )
    :   m_eViewType ( eType     )
    ,   m_sViewName ( sViewName )
{
    // Global access, must be guarded (multithreading!)
    ::osl::MutexGuard aGuard( GetOwnStaticMutex() );

    // Search for right dat container for this view type and initialize right data container or set right ref count!
    switch( eType )
    {
        case E_DIALOG       :   {
                                    // Increase ref count for dialog data container first.
                                    ++m_nRefCount_Dialogs;
                                    // If these instance the first user of the dialog data container - create these impl static container!
                                    if( m_nRefCount_Dialogs == 1 )
                                    {
                                        //m_pDataContainer_Dialogs = new SvtViewDialogOptions_Impl( LIST_DIALOGS );
                                        m_pDataContainer_Dialogs = new SvtViewOptionsBase_Impl( LIST_DIALOGS );
                                    }
                                }
                                break;
        case E_TABDIALOG    :   {
                                    // Increase ref count for tab-dialog data container first.
                                    ++m_nRefCount_TabDialogs;
                                    // If these instance the first user of the tab-dialog data container - create these impl static container!
                                    if( m_nRefCount_TabDialogs == 1 )
                                    {
                                        m_pDataContainer_TabDialogs = new SvtViewOptionsBase_Impl( LIST_TABDIALOGS );
                                    }
                                }
                                break;
        case E_TABPAGE      :   {
                                    // Increase ref count for tab-page data container first.
                                    ++m_nRefCount_TabPages;
                                    // If these instance the first user of the tab-page data container - create these impl static container!
                                    if( m_nRefCount_TabPages == 1 )
                                    {
                                        m_pDataContainer_TabPages = new SvtViewOptionsBase_Impl( LIST_TABPAGES );
                                    }
                                }
                                break;
        case E_WINDOW       :   {
                                    // Increase ref count for window data container first.
                                    ++m_nRefCount_Windows;
                                    // If these instance the first user of the window data container - create these impl static container!
                                    if( m_nRefCount_Windows == 1 )
                                    {
                                        m_pDataContainer_Windows = new SvtViewOptionsBase_Impl( LIST_WINDOWS );
                                    }
                                }
                                break;
        default             :   OSL_ENSURE( sal_False, "SvtViewOptions::SvtViewOptions()\nThese view type is unknown! All following calls at these instance will do nothing!\n" );
    }
}

//*****************************************************************************************************************
//  destructor
//*****************************************************************************************************************
SvtViewOptions::~SvtViewOptions()
{
    // Global access, must be guarded (multithreading!)
    ::osl::MutexGuard aGuard( GetOwnStaticMutex() );

    // Search for right dat container for this view type and deinitialize right data container or set right ref count!
    switch( m_eViewType )
    {
        case E_DIALOG       :   {
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
        case E_TABDIALOG    :   {
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
        case E_TABPAGE      :   {
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
        case E_WINDOW       :   {
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
//  public method
//*****************************************************************************************************************
sal_Bool SvtViewOptions::Exists() const
{
    // Ready for multithreading
    ::osl::MutexGuard aGuard( GetOwnStaticMutex() );

    sal_Bool bExists = sal_False;
    switch( m_eViewType )
    {
        case E_DIALOG       :   {
                                    bExists = m_pDataContainer_Dialogs->Exists( m_sViewName );
                                }
                                break;
        case E_TABDIALOG    :   {
                                    bExists = m_pDataContainer_TabDialogs->Exists( m_sViewName );
                                }
                                break;
        case E_TABPAGE      :   {
                                    bExists = m_pDataContainer_TabPages->Exists( m_sViewName );
                                }
                                break;
        case E_WINDOW       :   {
                                    bExists = m_pDataContainer_Windows->Exists( m_sViewName );
                                }
                                break;
    }
    return bExists;
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
sal_Bool SvtViewOptions::Delete()
{
    // Ready for multithreading
    ::osl::MutexGuard aGuard( GetOwnStaticMutex() );

    sal_Bool bState = sal_False;
    switch( m_eViewType )
    {
        case E_DIALOG       :   {
                                    bState = m_pDataContainer_Dialogs->Delete( m_sViewName );
                                }
                                break;
        case E_TABDIALOG    :   {
                                    bState = m_pDataContainer_TabDialogs->Delete( m_sViewName );
                                }
                                break;
        case E_TABPAGE      :   {
                                    bState = m_pDataContainer_TabPages->Delete( m_sViewName );
                                }
                                break;
        case E_WINDOW       :   {
                                    bState = m_pDataContainer_Windows->Delete( m_sViewName );
                                }
                                break;
    }
    return bState;
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
::rtl::OUString SvtViewOptions::GetWindowState() const
{
    // Ready for multithreading
    ::osl::MutexGuard aGuard( GetOwnStaticMutex() );

    ::rtl::OUString sState;
    switch( m_eViewType )
    {
        case E_DIALOG       :   {
                                    sState = m_pDataContainer_Dialogs->GetWindowState( m_sViewName );
                                }
                                break;
        case E_TABDIALOG    :   {
                                    sState = m_pDataContainer_TabDialogs->GetWindowState( m_sViewName );
                                }
                                break;
        case E_TABPAGE      :   {
                                    sState = m_pDataContainer_TabPages->GetWindowState( m_sViewName );
                                }
                                break;
        case E_WINDOW       :   {
                                    sState = m_pDataContainer_Windows->GetWindowState( m_sViewName );
                                }
                                break;
    }
    return sState;
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
void SvtViewOptions::SetWindowState( const ::rtl::OUString& sState )
{
    // Ready for multithreading
    ::osl::MutexGuard aGuard( GetOwnStaticMutex() );

    switch( m_eViewType )
    {
        case E_DIALOG       :   {
                                    m_pDataContainer_Dialogs->SetWindowState( m_sViewName, sState );
                                }
                                break;
        case E_TABDIALOG    :   {
                                    m_pDataContainer_TabDialogs->SetWindowState( m_sViewName, sState );
                                }
                                break;
        case E_TABPAGE      :   {
                                    m_pDataContainer_TabPages->SetWindowState( m_sViewName, sState );
                                }
                                break;
        case E_WINDOW       :   {
                                    m_pDataContainer_Windows->SetWindowState( m_sViewName, sState );
                                }
                                break;
    }
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
sal_Int32 SvtViewOptions::GetPageID() const
{
    // Ready for multithreading
    ::osl::MutexGuard aGuard( GetOwnStaticMutex() );

    // Safe impossible cases.
    // These call isn't allowed for dialogs, tab-pages or windows!
    OSL_ENSURE( !(m_eViewType==E_DIALOG||m_eViewType==E_TABPAGE||m_eViewType==E_WINDOW), "SvtViewOptions::GetPageID()\nCall not allowed for Dialogs, TabPages or Windows! I do nothing!\n" );

    sal_Int32 nID = 0;
    switch( m_eViewType )
    {
        case E_TABDIALOG    :   {
                                    nID = m_pDataContainer_TabDialogs->GetPageID( m_sViewName );
                                }
                                break;
    }
    return nID;
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
void SvtViewOptions::SetPageID( sal_Int32 nID )
{
    // Ready for multithreading
    ::osl::MutexGuard aGuard( GetOwnStaticMutex() );

    // Safe impossible cases.
    // These call isn't allowed for dialogs, tab-pages or windows!
    OSL_ENSURE( !(m_eViewType==E_DIALOG||m_eViewType==E_TABPAGE||m_eViewType==E_WINDOW), "SvtViewOptions::SetPageID()\nCall not allowed for Dialogs, TabPages or Windows! I do nothing!\n" );

    switch( m_eViewType )
    {
        case E_TABDIALOG    :   {
                                    m_pDataContainer_TabDialogs->SetPageID( m_sViewName, nID );
                                }
                                break;
    }
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
sal_Bool SvtViewOptions::IsVisible() const
{
    // Ready for multithreading
    ::osl::MutexGuard aGuard( GetOwnStaticMutex() );

    // Safe impossible cases.
    // These call isn't allowed for dialogs, tab-dialogs or tab-pages!
    OSL_ENSURE( !(m_eViewType==E_DIALOG||m_eViewType==E_TABDIALOG||m_eViewType==E_TABPAGE), "SvtViewOptions::IsVisible()\nCall not allowed for Dialogs, TabDialogs or TabPages! I do nothing!\n" );

    sal_Bool bState = sal_False;
    switch( m_eViewType )
    {
        case E_WINDOW       :   {
                                    bState = m_pDataContainer_Windows->GetVisible( m_sViewName );
                                }
                                break;
    }
    return bState;
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
void SvtViewOptions::SetVisible( sal_Bool bState )
{
    // Ready for multithreading
    ::osl::MutexGuard aGuard( GetOwnStaticMutex() );

    // Safe impossible cases.
    // These call isn't allowed for dialogs, tab-dialogs or tab-pages!
    OSL_ENSURE( !(m_eViewType==E_DIALOG||m_eViewType==E_TABDIALOG||m_eViewType==E_TABPAGE), "SvtViewOptions::SetVisible()\nCall not allowed for Dialogs, TabDialogs or TabPages! I do nothing!\n" );

    switch( m_eViewType )
    {
        case E_WINDOW       :   {
                                    m_pDataContainer_Windows->SetVisible( m_sViewName, bState );
                                }
                                break;
    }
}

//*****************************************************************************************************************
css::uno::Sequence< css::beans::NamedValue > SvtViewOptions::GetUserData() const
{
    // Ready for multithreading
    ::osl::MutexGuard aGuard( GetOwnStaticMutex() );

    css::uno::Sequence< css::beans::NamedValue > lData;
    switch( m_eViewType )
    {
        case E_DIALOG       :   {
                                    lData = m_pDataContainer_Dialogs->GetUserData( m_sViewName );
                                }
                                break;
        case E_TABDIALOG    :   {
                                    lData = m_pDataContainer_TabDialogs->GetUserData( m_sViewName );
                                }
                                break;
        case E_TABPAGE      :   {
                                    lData = m_pDataContainer_TabPages->GetUserData( m_sViewName );
                                }
                                break;
        case E_WINDOW       :   {
                                    lData = m_pDataContainer_Windows->GetUserData( m_sViewName );
                                }
                                break;
    }
    return lData;
}

//*****************************************************************************************************************
void SvtViewOptions::SetUserData( const css::uno::Sequence< css::beans::NamedValue >& lData )
{
    // Ready for multithreading
    ::osl::MutexGuard aGuard( GetOwnStaticMutex() );

    switch( m_eViewType )
    {
        case E_DIALOG       :   {
                                    m_pDataContainer_Dialogs->SetUserData( m_sViewName, lData );
                                }
                                break;
        case E_TABDIALOG    :   {
                                    m_pDataContainer_TabDialogs->SetUserData( m_sViewName, lData );
                                }
                                break;
        case E_TABPAGE      :   {
                                    m_pDataContainer_TabPages->SetUserData( m_sViewName, lData );
                                }
                                break;
        case E_WINDOW       :   {
                                    m_pDataContainer_Windows->SetUserData( m_sViewName, lData );
                                }
                                break;
    }
}

//*****************************************************************************************************************
css::uno::Any SvtViewOptions::GetUserItem( const ::rtl::OUString& sName ) const
{
    // Ready for multithreading
    ::osl::MutexGuard aGuard( GetOwnStaticMutex() );

    css::uno::Any aItem;
    switch( m_eViewType )
    {
        case E_DIALOG       :   {
                                    aItem = m_pDataContainer_Dialogs->GetUserItem( m_sViewName, sName );
                                }
                                break;
        case E_TABDIALOG    :   {
                                    aItem = m_pDataContainer_TabDialogs->GetUserItem( m_sViewName, sName );
                                }
                                break;
        case E_TABPAGE      :   {
                                    aItem = m_pDataContainer_TabPages->GetUserItem( m_sViewName, sName );
                                }
                                break;
        case E_WINDOW       :   {
                                    aItem = m_pDataContainer_Windows->GetUserItem( m_sViewName, sName );
                                }
                                break;
    }
    return aItem;
}

//*****************************************************************************************************************
void SvtViewOptions::SetUserItem( const ::rtl::OUString& sName  ,
                                  const css::uno::Any&   aValue )
{
    // Ready for multithreading
    ::osl::MutexGuard aGuard( GetOwnStaticMutex() );

    switch( m_eViewType )
    {
        case E_DIALOG       :   {
                                    m_pDataContainer_Dialogs->SetUserItem( m_sViewName, sName, aValue );
                                }
                                break;
        case E_TABDIALOG    :   {
                                    m_pDataContainer_TabDialogs->SetUserItem( m_sViewName, sName, aValue );
                                }
                                break;
        case E_TABPAGE      :   {
                                    m_pDataContainer_TabPages->SetUserItem( m_sViewName, sName, aValue );
                                }
                                break;
        case E_WINDOW       :   {
                                    m_pDataContainer_Windows->SetUserItem( m_sViewName, sName, aValue );
                                }
                                break;
    }
}

//*****************************************************************************************************************
//  private method
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

void SvtViewOptions::AcquireOptions()
{
    ::osl::MutexGuard aGuard( GetOwnStaticMutex() );
    if( ++m_nRefCount_Dialogs == 1 )
        m_pDataContainer_Dialogs = new SvtViewOptionsBase_Impl( LIST_DIALOGS );
    if( ++m_nRefCount_TabDialogs == 1 )
        m_pDataContainer_TabDialogs = new SvtViewOptionsBase_Impl( LIST_TABDIALOGS );
    if( ++m_nRefCount_TabPages == 1 )
        m_pDataContainer_TabPages = new SvtViewOptionsBase_Impl( LIST_TABPAGES );
    if( ++m_nRefCount_Windows == 1 )
        m_pDataContainer_Windows = new SvtViewOptionsBase_Impl( LIST_WINDOWS );
}

void SvtViewOptions::ReleaseOptions()
{
    ::osl::MutexGuard aGuard( GetOwnStaticMutex() );
    if( --m_nRefCount_Dialogs == 0 )
    {
        delete m_pDataContainer_Dialogs;
        m_pDataContainer_Dialogs = NULL;
    }
    if( --m_nRefCount_TabDialogs == 0 )
    {
        delete m_pDataContainer_TabDialogs;
        m_pDataContainer_TabDialogs = NULL;
    }
    if( --m_nRefCount_TabPages == 0 )
    {
        delete m_pDataContainer_TabPages;
        m_pDataContainer_TabPages = NULL;
    }
    if( --m_nRefCount_Windows == 0 )
    {
        delete m_pDataContainer_Windows;
        m_pDataContainer_Windows = NULL;
    }
}

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

#include <unotools/viewoptions.hxx>
#include <com/sun/star/uno/Any.hxx>

#include <boost/unordered_map.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <rtl/ustrbuf.hxx>
#include <unotools/configpaths.hxx>
#include <comphelper/configurationhelper.hxx>
#include <comphelper/processfactory.hxx>

#include <itemholder1.hxx>

//_________________________________________________________________________________________________________________
//  namespaces
//_________________________________________________________________________________________________________________

namespace css = ::com::sun::star;

#ifdef CONST_ASCII
    #error  "Who define CONST_ASCII before! I use it to create const ascii strings ..."
#else
    #define CONST_ASCII(SASCIIVALUE)            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SASCIIVALUE))
#endif

#define PACKAGE_VIEWS                           CONST_ASCII("org.openoffice.Office.Views")

#define LIST_DIALOGS                            CONST_ASCII("Dialogs"   )
#define LIST_TABDIALOGS                         CONST_ASCII("TabDialogs")
#define LIST_TABPAGES                           CONST_ASCII("TabPages"  )
#define LIST_WINDOWS                            CONST_ASCII("Windows"   )

#define PROPERTY_WINDOWSTATE                    CONST_ASCII("WindowState")
#define PROPERTY_PAGEID                         CONST_ASCII("PageID"     )
#define PROPERTY_VISIBLE                        CONST_ASCII("Visible"    )
#define PROPERTY_USERDATA                       CONST_ASCII("UserData"   )

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
    }

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

typedef ::boost::unordered_map< ::rtl::OUString                    ,
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
        enum State { STATE_NONE, STATE_FALSE, STATE_TRUE };

                                                        SvtViewOptionsBase_Impl ( const ::rtl::OUString&                                sList    );
        virtual                                        ~SvtViewOptionsBase_Impl (                                                                );
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
        State                                           GetVisible              ( const ::rtl::OUString&                                sName    );
        void                                            SetVisible              ( const ::rtl::OUString&                                sName    ,
                                                                                        sal_Bool                                        bVisible );
        css::uno::Any                                   GetUserItem             ( const ::rtl::OUString&                                sName    ,
                                                                                  const ::rtl::OUString&                                sItem    );
        void                                            SetUserItem             ( const ::rtl::OUString&                                sName    ,
                                                                                  const ::rtl::OUString&                                sItem    ,
                                                                                  const css::uno::Any&                                  aValue   );

    //-------------------------------------------------------------------------------------------------------------
    private:
        css::uno::Reference< css::uno::XInterface > impl_getSetNode( const ::rtl::OUString& sNode           ,
                                                                           sal_Bool         bCreateIfMissing);

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
                            ::comphelper::getProcessServiceFactory(),
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
    @descr          clean up something

    @attention      We implement a write through cache! So we mustn't do it realy. All changes was written to cfg directly.
                    Commit isn't neccessary then.

    @seealso        baseclass ::utl::ConfigItem
    @seealso        method IsModified()
    @seealso        method SetModified()
    @seealso        method Commit()

    @param          -
    @return         -
*//*-*************************************************************************************************************/
SvtViewOptionsBase_Impl::~SvtViewOptionsBase_Impl()
{
    // dont flush configuration changes here to m_xRoot.
    // That must be done inside every SetXXX() method already !
    // Here its to late - DisposedExceptions from used configuration access can occure otherwise.

    m_xRoot.clear();
    m_xSet.clear();

    #ifdef DEBUG_VIEWOPTIONS
    _LOG_COUNTER_( m_sListName, m_nReadCount, m_nWriteCount )
    #endif // DEBUG_VIEWOPTIONS
}

/*-************************************************************************************************************//**
    @short          checks for already existing entries
    @descr          If user don't know, if an entry already exist - he can get this information by calling this method.

    @seealso        member m_aList

    @param          "sName", name of entry to check exist state
    @return         true , if item exist
                    false, otherwise
*//*-*************************************************************************************************************/
sal_Bool SvtViewOptionsBase_Impl::Exists( const ::rtl::OUString& sName )
{
    sal_Bool bExists = sal_False;

    try
    {
        if (m_xSet.is())
            bExists = m_xSet->hasByName(sName);
    }
    catch(const css::uno::Exception& ex)
        {
            bExists = sal_False;
            SVTVIEWOPTIONS_LOG_UNEXPECTED_EXCEPTION(ex)
        }

    return bExists;
}

/*-************************************************************************************************************//**
    @short          delete entry
    @descr          Use it to delete set entry by given name.

    @seealso        member m_aList

    @param          "sName", name of entry to delete it
    @return         true , if item not exist(!) or could be deleted (should be the same!)
                    false, otherwise
*//*-*************************************************************************************************************/
sal_Bool SvtViewOptionsBase_Impl::Delete( const ::rtl::OUString& sName )
{
    #ifdef DEBUG_VIEWOPTIONS
    ++m_nWriteCount;
    #endif

    sal_Bool bDeleted = sal_False;
    try
    {
        css::uno::Reference< css::container::XNameContainer > xSet(m_xSet, css::uno::UNO_QUERY_THROW);
        xSet->removeByName(sName);
        bDeleted = sal_True;
        ::comphelper::ConfigurationHelper::flush(m_xRoot);
    }
    catch(const css::container::NoSuchElementException&)
        { bDeleted = sal_True; }
    catch(const css::uno::Exception& ex)
        {
            bDeleted = sal_False;
            SVTVIEWOPTIONS_LOG_UNEXPECTED_EXCEPTION(ex)
        }

    return bDeleted;
}

/*-************************************************************************************************************//**
    @short          read/write access to cache view items and her properties
    @descr          Follow methods support read/write access to all cache view items.

    @seealso        member m_sList

    @param          -
    @return         -
*//*-*************************************************************************************************************/
::rtl::OUString SvtViewOptionsBase_Impl::GetWindowState( const ::rtl::OUString& sName )
{
    #ifdef DEBUG_VIEWOPTIONS
    ++m_nReadCount;
    #endif

    ::rtl::OUString sWindowState;
    try
    {
        css::uno::Reference< css::beans::XPropertySet > xNode(
            impl_getSetNode(sName, sal_False),
            css::uno::UNO_QUERY);
        if (xNode.is())
            xNode->getPropertyValue(PROPERTY_WINDOWSTATE) >>= sWindowState;
    }
    catch(const css::uno::Exception& ex)
        {
            sWindowState = ::rtl::OUString();
            SVTVIEWOPTIONS_LOG_UNEXPECTED_EXCEPTION(ex)
        }

    return sWindowState;
}

//*****************************************************************************************************************
void SvtViewOptionsBase_Impl::SetWindowState( const ::rtl::OUString& sName  ,
                                              const ::rtl::OUString& sState )
{
    #ifdef DEBUG_VIEWOPTIONS
    ++m_nWriteCount;
    #endif

    try
    {
        css::uno::Reference< css::beans::XPropertySet > xNode(
            impl_getSetNode(sName, sal_True),
            css::uno::UNO_QUERY_THROW);
        xNode->setPropertyValue(PROPERTY_WINDOWSTATE, css::uno::makeAny(sState));
        ::comphelper::ConfigurationHelper::flush(m_xRoot);
    }
    catch(const css::uno::Exception& ex)
        {
            SVTVIEWOPTIONS_LOG_UNEXPECTED_EXCEPTION(ex)
        }
}

//*****************************************************************************************************************
css::uno::Sequence< css::beans::NamedValue > SvtViewOptionsBase_Impl::GetUserData( const ::rtl::OUString& sName )
{
    #ifdef DEBUG_VIEWOPTIONS
    ++m_nReadCount;
    #endif

    try
    {
        css::uno::Reference< css::container::XNameAccess > xNode(
            impl_getSetNode(sName, sal_False),
            css::uno::UNO_QUERY); // no _THROW ! because we dont create missing items here. So we have to live with zero references .-)
        css::uno::Reference< css::container::XNameAccess > xUserData;
        if (xNode.is())
            xNode->getByName(PROPERTY_USERDATA) >>= xUserData;
        if (xUserData.is())
        {
            const css::uno::Sequence< ::rtl::OUString >         lNames = xUserData->getElementNames();
            const ::rtl::OUString*                              pNames = lNames.getConstArray();
                  sal_Int32                                     c      = lNames.getLength();
                  sal_Int32                                     i      = 0;
                  css::uno::Sequence< css::beans::NamedValue >  lUserData(c);

            for (i=0; i<c; ++i)
            {
                lUserData[i].Name  = pNames[i];
                lUserData[i].Value = xUserData->getByName(pNames[i]);
            }

            return lUserData;
        }
    }
    catch(const css::uno::Exception& ex)
        {
            SVTVIEWOPTIONS_LOG_UNEXPECTED_EXCEPTION(ex)
        }

    return css::uno::Sequence< css::beans::NamedValue >();
}

//*****************************************************************************************************************
void SvtViewOptionsBase_Impl::SetUserData( const ::rtl::OUString&                              sName  ,
                                           const css::uno::Sequence< css::beans::NamedValue >& lData  )
{
    #ifdef DEBUG_VIEWOPTIONS
    ++m_nWriteCount;
    #endif

    try
    {
        css::uno::Reference< css::container::XNameAccess > xNode(
            impl_getSetNode(sName, sal_True),
            css::uno::UNO_QUERY_THROW);
        css::uno::Reference< css::container::XNameContainer > xUserData;
        xNode->getByName(PROPERTY_USERDATA) >>= xUserData;
        if (xUserData.is())
        {
            const css::beans::NamedValue* pData = lData.getConstArray();
                  sal_Int32               c     = lData.getLength();
                  sal_Int32               i     = 0;
            for (i=0; i<c; ++i)
            {
                if (xUserData->hasByName(pData[i].Name))
                    xUserData->replaceByName(pData[i].Name, pData[i].Value);
                else
                    xUserData->insertByName(pData[i].Name, pData[i].Value);
            }
        }
        ::comphelper::ConfigurationHelper::flush(m_xRoot);
    }
    catch(const css::uno::Exception& ex)
        {
            SVTVIEWOPTIONS_LOG_UNEXPECTED_EXCEPTION(ex)
        }
}

//*****************************************************************************************************************
css::uno::Any SvtViewOptionsBase_Impl::GetUserItem( const ::rtl::OUString& sName ,
                                                    const ::rtl::OUString& sItem )
{
    #ifdef DEBUG_VIEWOPTIONS
    ++m_nReadCount;
    #endif

    css::uno::Any aItem;
    try
    {
        css::uno::Reference< css::container::XNameAccess > xNode(
            impl_getSetNode(sName, sal_False),
            css::uno::UNO_QUERY);
        css::uno::Reference< css::container::XNameAccess > xUserData;
        if (xNode.is())
            xNode->getByName(PROPERTY_USERDATA) >>= xUserData;
        if (xUserData.is())
            aItem = xUserData->getByName(sItem);
    }
    catch(const css::container::NoSuchElementException&)
        { aItem.clear(); }
    catch(const css::uno::Exception& ex)
        {
            aItem.clear();
            SVTVIEWOPTIONS_LOG_UNEXPECTED_EXCEPTION(ex)
        }

    return aItem;
}

//*****************************************************************************************************************
void SvtViewOptionsBase_Impl::SetUserItem( const ::rtl::OUString& sName  ,
                                           const ::rtl::OUString& sItem  ,
                                           const css::uno::Any&   aValue )
{
    #ifdef DEBUG_VIEWOPTIONS
    ++m_nWriteCount;
    #endif

    try
    {
        css::uno::Reference< css::container::XNameAccess > xNode(
            impl_getSetNode(sName, sal_True),
            css::uno::UNO_QUERY_THROW);
        css::uno::Reference< css::container::XNameContainer > xUserData;
        xNode->getByName(PROPERTY_USERDATA) >>= xUserData;
        if (xUserData.is())
        {
            if (xUserData->hasByName(sItem))
                xUserData->replaceByName(sItem, aValue);
            else
                xUserData->insertByName(sItem, aValue);
        }
        ::comphelper::ConfigurationHelper::flush(m_xRoot);
    }
    catch(const css::uno::Exception& ex)
        {
            SVTVIEWOPTIONS_LOG_UNEXPECTED_EXCEPTION(ex)
        }
}

//*****************************************************************************************************************
sal_Int32 SvtViewOptionsBase_Impl::GetPageID( const ::rtl::OUString& sName )
{
    #ifdef DEBUG_VIEWOPTIONS
    ++m_nReadCount;
    #endif

    sal_Int32 nID = 0;
    try
    {
        css::uno::Reference< css::beans::XPropertySet > xNode(
            impl_getSetNode(sName, sal_False),
            css::uno::UNO_QUERY);
        if (xNode.is())
            xNode->getPropertyValue(PROPERTY_PAGEID) >>= nID;
    }
    catch(const css::uno::Exception& ex)
        {
            nID = 0;
            SVTVIEWOPTIONS_LOG_UNEXPECTED_EXCEPTION(ex)
        }

    return nID;
}

//*****************************************************************************************************************
void SvtViewOptionsBase_Impl::SetPageID( const ::rtl::OUString& sName ,
                                               sal_Int32        nID   )
{
    #ifdef DEBUG_VIEWOPTIONS
    ++m_nWriteCount;
    #endif

    try
    {
        css::uno::Reference< css::beans::XPropertySet > xNode(
            impl_getSetNode(sName, sal_True),
            css::uno::UNO_QUERY_THROW);
        xNode->setPropertyValue(PROPERTY_PAGEID, css::uno::makeAny(nID));
        ::comphelper::ConfigurationHelper::flush(m_xRoot);
    }
    catch(const css::uno::Exception& ex)
        {
            SVTVIEWOPTIONS_LOG_UNEXPECTED_EXCEPTION(ex)
        }
}

//*****************************************************************************************************************
SvtViewOptionsBase_Impl::State SvtViewOptionsBase_Impl::GetVisible( const ::rtl::OUString& sName )
{
    #ifdef DEBUG_VIEWOPTIONS
    ++m_nReadCount;
    #endif

    State eState = STATE_NONE;
    try
    {
        css::uno::Reference< css::beans::XPropertySet > xNode(
            impl_getSetNode(sName, sal_False),
            css::uno::UNO_QUERY);
        if (xNode.is())
        {
            sal_Bool bVisible = sal_False;
            if (xNode->getPropertyValue(PROPERTY_VISIBLE) >>= bVisible)
            {
                eState = bVisible ? STATE_TRUE : STATE_FALSE;
            }
        }
    }
    catch(const css::uno::Exception& ex)
        {
            SVTVIEWOPTIONS_LOG_UNEXPECTED_EXCEPTION(ex)
        }

    return eState;
}

//*****************************************************************************************************************
void SvtViewOptionsBase_Impl::SetVisible( const ::rtl::OUString& sName    ,
                                                sal_Bool         bVisible )
{
    #ifdef DEBUG_VIEWOPTIONS
    ++m_nWriteCount;
    #endif

    try
    {
        css::uno::Reference< css::beans::XPropertySet > xNode(
            impl_getSetNode(sName, sal_True),
            css::uno::UNO_QUERY_THROW);
        xNode->setPropertyValue(PROPERTY_VISIBLE, css::uno::makeAny(bVisible));
        ::comphelper::ConfigurationHelper::flush(m_xRoot);
    }
    catch(const css::uno::Exception& ex)
        {
            SVTVIEWOPTIONS_LOG_UNEXPECTED_EXCEPTION(ex)
        }
}

/*-************************************************************************************************************//**
    @short          create new set node with default values on disk
    @descr          To create a new UserData item - the super node of these property must already exist!
                    You can call this method to create these new entry with default values and change UserData then.

    @seealso        method impl_writeDirectProp()

    @param          "sNode", name of new entry
    @return         -
*//*-*************************************************************************************************************/
css::uno::Reference< css::uno::XInterface > SvtViewOptionsBase_Impl::impl_getSetNode( const ::rtl::OUString& sNode           ,
                                                                                            sal_Bool         bCreateIfMissing)
{
    css::uno::Reference< css::uno::XInterface > xNode;

    try
    {
        if (bCreateIfMissing)
            xNode = ::comphelper::ConfigurationHelper::makeSureSetNodeExists(m_xRoot, m_sListName, sNode);
        else
        {
            if (m_xSet.is() && m_xSet->hasByName(sNode) )
                m_xSet->getByName(sNode) >>= xNode;
        }
    }
    catch(const css::container::NoSuchElementException&)
        { xNode.clear(); }
    catch(const css::uno::Exception& ex)
        {
            xNode.clear();
            SVTVIEWOPTIONS_LOG_UNEXPECTED_EXCEPTION(ex)
        }

    return xNode;
}

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
                                        ItemHolder1::holdConfigItem(E_VIEWOPTIONS_DIALOG);
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
                                        ItemHolder1::holdConfigItem(E_VIEWOPTIONS_TABDIALOG);
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
                                        ItemHolder1::holdConfigItem(E_VIEWOPTIONS_TABPAGE);
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
                                        ItemHolder1::holdConfigItem(E_VIEWOPTIONS_WINDOW);
                                    }
                                }
                                break;
        default             :   OSL_FAIL( "SvtViewOptions::SvtViewOptions()\nThese view type is unknown! All following calls at these instance will do nothing!\n" );
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
    if( m_eViewType == E_TABDIALOG )
        nID = m_pDataContainer_TabDialogs->GetPageID( m_sViewName );
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

    if( m_eViewType == E_TABDIALOG )
        m_pDataContainer_TabDialogs->SetPageID( m_sViewName, nID );
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
    if( m_eViewType == E_WINDOW )
        bState = m_pDataContainer_Windows->GetVisible( m_sViewName ) == SvtViewOptionsBase_Impl::STATE_TRUE;

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

    if( m_eViewType == E_WINDOW )
        m_pDataContainer_Windows->SetVisible( m_sViewName, bState );
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
bool SvtViewOptions::HasVisible() const
{
    // Ready for multithreading
    ::osl::MutexGuard aGuard( GetOwnStaticMutex() );

    // Safe impossible cases.
    // These call isn't allowed for dialogs, tab-dialogs or tab-pages!
    OSL_ENSURE( !(m_eViewType==E_DIALOG||m_eViewType==E_TABDIALOG||m_eViewType==E_TABPAGE), "SvtViewOptions::IsVisible()\nCall not allowed for Dialogs, TabDialogs or TabPages! I do nothing!\n" );

    bool bState = false;
    if( m_eViewType == E_WINDOW )
        bState = m_pDataContainer_Windows->GetVisible( m_sViewName ) != SvtViewOptionsBase_Impl::STATE_NONE;

    return bState;
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

namespace
{
    class theViewOptionsMutex : public rtl::Static<osl::Mutex, theViewOptionsMutex>{};
}

//*****************************************************************************************************************
//  private method
//*****************************************************************************************************************
::osl::Mutex& SvtViewOptions::GetOwnStaticMutex()
{
    return theViewOptionsMutex::get();
}

void SvtViewOptions::AcquireOptions()
{
    ::osl::MutexGuard aGuard( GetOwnStaticMutex() );
    if( ++m_nRefCount_Dialogs == 1 )
    {
        m_pDataContainer_Dialogs = new SvtViewOptionsBase_Impl( LIST_DIALOGS );
        ItemHolder1::holdConfigItem(E_VIEWOPTIONS_DIALOG);
    }
    if( ++m_nRefCount_TabDialogs == 1 )
    {
        m_pDataContainer_TabDialogs = new SvtViewOptionsBase_Impl( LIST_TABDIALOGS );
        ItemHolder1::holdConfigItem(E_VIEWOPTIONS_TABDIALOG);
    }
    if( ++m_nRefCount_TabPages == 1 )
    {
        m_pDataContainer_TabPages = new SvtViewOptionsBase_Impl( LIST_TABPAGES );
        ItemHolder1::holdConfigItem(E_VIEWOPTIONS_TABPAGE);
    }
    if( ++m_nRefCount_Windows == 1 )
    {
        m_pDataContainer_Windows = new SvtViewOptionsBase_Impl( LIST_WINDOWS );
        ItemHolder1::holdConfigItem(E_VIEWOPTIONS_WINDOW);
    }
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

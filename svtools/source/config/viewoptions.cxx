/*************************************************************************
 *
 *  $RCSfile: viewoptions.cxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: as $ $Date: 2001-10-12 13:26:23 $
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

/*
#ifdef TF_OLDVIEW
    #ifndef _RTL_USTRBUF_HXX_
    #include <rtl/ustrbuf.hxx>
    #endif
#endif
*/
//_________________________________________________________________________________________________________________
//  namespaces
//_________________________________________________________________________________________________________________

#ifdef css
    #error  "Who define css before! I use it as namespace replacement ..."
#else
    #define css                                 ::com::sun::star
#endif

//_________________________________________________________________________________________________________________
//  const
//_________________________________________________________________________________________________________________

#ifdef CONST_ASCII
    #error  "Who define CONST_ASCII before! I use it to create const ascii strings ..."
#else
    #define CONST_ASCII( _SASCIIVALUE_ )        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(_SASCIIVALUE_))
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

#define PREFIX                                  CONST_ASCII("v_"                      )
#define PREFIXLENGTH                            2

#define CHANGED_WINDOWSTATE                     0x00000001
#define CHANGED_PAGEID                          0x00000002
#define CHANGED_VISIBLE                         0x00000004
#define CHANGED_USERDATA                        0x00000008

#define DEFAULT_WINDOWSTATE                     ::rtl::OUString()
#define DEFAULT_PAGEID                          0
#define DEFAULT_VISIBLE                         sal_True
#define DEFAULT_USERDATA                        css::uno::Sequence< css::beans::NamedValue >()

#define PROPCOUNT_DIALOGS                       2
#define PROPCOUNT_TABDIALOGS                    3
#define PROPCOUNT_TABPAGES                      2
#define PROPCOUNT_WINDOWS                       3

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
    @descr      We define different a structure which present the format of a list item of right view type.
    @attention  Not all member are used for all special view type entries!
*//*-*************************************************************************************************************/

//*****************************************************************************************************************
class IMPL_TViewData
{
    public:
        //---------------------------------------------------------------------------------------------------------
        IMPL_TViewData()
            :   m_nChangedMask( 0                   )
            ,   m_sWindowState( DEFAULT_WINDOWSTATE )
            ,   m_lUserData   ( DEFAULT_USERDATA    )
            ,   m_nPageID     ( DEFAULT_PAGEID      )
            ,   m_bVisible    ( DEFAULT_PAGEID      )
        {
        }

        //---------------------------------------------------------------------------------------------------------
        void SetWindowState( const ::rtl::OUString& sValue ,
                                   sal_Bool         bTouch )
        {
            if( m_sWindowState != sValue )
            {
                m_sWindowState = sValue;
                if( bTouch == sal_True )
                    m_nChangedMask |= CHANGED_WINDOWSTATE;
            }
        }

        //---------------------------------------------------------------------------------------------------------
        void SetUserData( const css::uno::Sequence< css::beans::NamedValue >& lValue ,
                                sal_Bool                                      bTouch )
        {
            if( m_lUserData != lValue )
            {
                m_lUserData = lValue;
                if( bTouch == sal_True )
                    m_nChangedMask |= CHANGED_USERDATA;
            }
        }

        //---------------------------------------------------------------------------------------------------------
        void SetPageID( sal_Int32 nValue ,
                        sal_Bool  bTouch )
        {
            if( m_nPageID != nValue )
            {
                m_nPageID = nValue;
                if( bTouch == sal_True )
                    m_nChangedMask |= CHANGED_PAGEID;
            }
        }

        //---------------------------------------------------------------------------------------------------------
        void SetVisible( sal_Bool bValue ,
                         sal_Bool bTouch )
        {
            if( m_bVisible != bValue )
            {
                m_bVisible = bValue;
                if( bTouch == sal_True )
                    m_nChangedMask |= CHANGED_VISIBLE;
            }
        }

        //---------------------------------------------------------------------------------------------------------
        ::rtl::OUString GetWindowState() const
        {
            return m_sWindowState;
        }

        //---------------------------------------------------------------------------------------------------------
        css::uno::Sequence< css::beans::NamedValue > GetUserData() const
        {
            return m_lUserData;
        }

        //---------------------------------------------------------------------------------------------------------
        sal_Int32 GetPageID() const
        {
            return m_nPageID;
        }

        //---------------------------------------------------------------------------------------------------------
        sal_Bool GetVisible() const
        {
            return m_bVisible;
        }

        //---------------------------------------------------------------------------------------------------------
        void AddOrReplaceUserItem( const ::rtl::OUString& sName  ,
                                   const css::uno::Any&   aValue ,
                                         sal_Bool         bTouch )
        {
            sal_Int32 nCount = m_lUserData.getLength();
            sal_Bool  bFound = sal_False;
            for( sal_Int32 nStep=0; nStep<nCount; ++nStep )
            {
                if( m_lUserData[nStep].Name == sName )
                {
                    m_lUserData[nStep].Value = aValue  ;
                    bFound                   = sal_True;
                    break;
                }
            }
            if( bFound == sal_False )
            {
                m_lUserData.realloc( nCount+1 );
                m_lUserData[nCount].Name  = sName  ;
                m_lUserData[nCount].Value = aValue ;
            }
            if( bTouch == sal_True )
            {
                m_nChangedMask |= CHANGED_USERDATA;
            }
        }

        //---------------------------------------------------------------------------------------------------------
        void RemoveUserItem( const ::rtl::OUString& sName  ,
                                   sal_Bool         bTouch )
        {
            sal_Int32 nCount = m_lUserData.getLength();
            for( sal_Int32 nStep=0; nStep<nCount; ++nStep )
            {
                if( m_lUserData[nStep].Name == sName )
                {
                    // if it is the last item - delete it by resizing!
                    if( nStep == nCount-1 )
                    {
                        m_lUserData.realloc( nCount-1 );
                    }
                    // otherwise copy last item to current position
                    // (delete by override!) and resize list
                    else
                    {
                        m_lUserData[nStep] = m_lUserData[nCount-1];
                        m_lUserData.realloc( nCount-1 );
                    }
                    break;
                }
            }
            if( bTouch == sal_True )
            {
                m_nChangedMask |= CHANGED_USERDATA;
            }
        }

        //---------------------------------------------------------------------------------------------------------
        sal_Bool IsChanged( sal_Int32 nCheckMask ) const
        {
            return(( m_nChangedMask & nCheckMask ) == nCheckMask );
        }

    //-------------------------------------------------------------------------------------------------------------
    private:
        ::rtl::OUString                                 m_sWindowState    ;
        css::uno::Sequence< css::beans::NamedValue >    m_lUserData       ;
        sal_Int32                                       m_nPageID         ;
        sal_Bool                                        m_bVisible        ;

        sal_Int32                                       m_nChangedMask    ;
};

/*-************************************************************************************************************//**
    @descr  Define hash list to hold the view type data.
*//*-*************************************************************************************************************/
struct IMPL_TStringHashCode
{
    size_t operator()(const ::rtl::OUString& sString) const
    {
        return sString.hashCode();
    }
};

typedef ::std::vector< ::rtl::OUString >                      IMPL_TStringList;

typedef ::std::hash_map< ::rtl::OUString                    ,
                         IMPL_TViewData                     ,
                         IMPL_TStringHashCode               ,
                         ::std::equal_to< ::rtl::OUString > > IMPL_TViewHash;

class IMPL_TViewList
{
    public:
        //---------------------------------------------------------------------------------------------------------
        void Add( const ::rtl::OUString& sName ,
                  const IMPL_TViewData&  rData )
        {
            // Do nothing, if item already exist!
            IMPL_TViewHash::iterator pItem = m_aList.find( sName );
            if( pItem == m_aList.end() )
            {
                // Otherwise add it to list.
                m_aList[sName] = rData;
                // Set his name in "Added" list for later flushing of data!
                m_lAdded.push_back( sName );
                // Don't forget to search it in "Deleted" list!
                // We must correct it and remove this entry there.
                IMPL_TStringList::iterator pDeleted = ::std::find( m_lDeleted.begin(), m_lDeleted.end(), sName );
                if( pDeleted != m_lDeleted.end() )
                {
                    m_lDeleted.erase( pDeleted );
                }
            }
        }

        //---------------------------------------------------------------------------------------------------------
        void Remove( const ::rtl::OUString& sName )
        {
            // If item exist in list ...
            IMPL_TViewHash::iterator pItem = m_aList.find( sName );
            if( pItem != m_aList.end() )
            {
                // ... remove it.
                // Mark it as "deleted".
                m_aList.erase( pItem );
                m_lDeleted.push_back( sName );
                // But don't forget to unset it as "added",
                // if it exist in these list too!
                IMPL_TStringList::iterator pAdded = ::std::find( m_lAdded.begin(), m_lAdded.end(), sName );
                if( pAdded != m_lAdded.end() )
                {
                    m_lAdded.erase( pAdded );
                }
            }
        }

        //---------------------------------------------------------------------------------------------------------
        void Change( const ::rtl::OUString& sName  ,
                     const IMPL_TViewData&  rData  ,
                           sal_Int32        nMask  ,
                           sal_Bool         bTouch )
        {
            // If item exist in list ...
            IMPL_TViewHash::iterator pItem = m_aList.find( sName );
            if( pItem != m_aList.end() )
            {
                if( nMask & CHANGED_WINDOWSTATE )
                    m_aList[sName].SetWindowState( rData.GetWindowState(), bTouch );
                if( nMask & CHANGED_USERDATA )
                    m_aList[sName].SetUserData( rData.GetUserData(), bTouch );
                if( nMask & CHANGED_PAGEID )
                    m_aList[sName].SetPageID( rData.GetPageID(), bTouch );
                if( nMask & CHANGED_VISIBLE )
                    m_aList[sName].SetVisible( rData.GetVisible(), bTouch );
                if(
                    ( bTouch == sal_True )  &&
                    ( nMask  != 0        )
                  )
                {
                    m_lChanged.push_back( sName );
                }
            }
        }

        //---------------------------------------------------------------------------------------------------------
        sal_Bool Find( const ::rtl::OUString& sName ,
                             IMPL_TViewData&  rData ) const
        {
            sal_Bool                       bFound = sal_False;
            IMPL_TViewHash::const_iterator pItem  = m_aList.find( sName );
            if( pItem != m_aList.end() )
            {
                rData  = pItem->second;
                bFound = sal_True;
            }
            return bFound;
        }

        //---------------------------------------------------------------------------------------------------------
        sal_Bool Exists( const ::rtl::OUString& sName ) const
        {
            IMPL_TViewHash::const_iterator pItem  = m_aList.find( sName );
            return( pItem != m_aList.end() );
        }

        //---------------------------------------------------------------------------------------------------------
        void CreateIfNotExist( const ::rtl::OUString& sName )
        {
            IMPL_TViewHash::iterator pItem  = m_aList.find( sName );
            if( pItem == m_aList.end() )
            {
                IMPL_TViewData aData;
                Add( sName, aData );
                // Don't work on list "m_lAdded/m_lChanged ..."!

                // Add() do it automaticly for us!
            }
        }

        //---------------------------------------------------------------------------------------------------------
        void Free()
        {
            IMPL_TViewHash().swap  ( m_aList    );
            IMPL_TStringList().swap( m_lAdded   );
            IMPL_TStringList().swap( m_lChanged );
            IMPL_TStringList().swap( m_lDeleted );
        }

        //---------------------------------------------------------------------------------------------------------
        const IMPL_TStringList& GetAddedList  () const { return m_lAdded  ; }
        const IMPL_TStringList& GetChangedList() const { return m_lChanged; }
        const IMPL_TStringList& GetDeletedList() const { return m_lDeleted; }

    private:
        IMPL_TViewHash      m_aList     ;
        IMPL_TStringList    m_lAdded    ;
        IMPL_TStringList    m_lChanged  ;
        IMPL_TStringList    m_lDeleted  ;
};

/*-************************************************************************************************************//**
    @descr  Implement base data container for view options elements.
            Every item knows "OwnData", "UserData" and "AnyData". So
            we can implement last two topics in baseclass .. and first one
            in different superclasses of them in a special case.
*//*-*************************************************************************************************************/
class SvtViewOptionsBase_Impl : public ::utl::ConfigItem
{
    //-------------------------------------------------------------------------------------------------------------
    public:
                                                        SvtViewOptionsBase_Impl ( const ::rtl::OUString&                                sList    );
        virtual                                        ~SvtViewOptionsBase_Impl (                                                                );
        virtual void                                    Notify                  ( const css::uno::Sequence< ::rtl::OUString >&          lNames   );
        virtual void                                    Commit                  (                                                                );
        sal_Bool                                        Exists                  ( const ::rtl::OUString&                                sName    ) const;
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
        css::uno::Any                                   GetUserItem             ( const ::rtl::OUString&                                sName    ) const;
        void                                            SetUserItem             ( const ::rtl::OUString&                                sName    ,
                                                                                  const css::uno::Any&                                  aValue   );

    //-------------------------------------------------------------------------------------------------------------
    private:
        void impl_ReadWholeList();

    //-------------------------------------------------------------------------------------------------------------
    private:
        IMPL_TViewList      m_aList     ;
        ::rtl::OUString     m_sListName ;
};

/*-************************************************************************************************************//**
    @descr  Implement the base data container.
*//*-*************************************************************************************************************/

/*-************************************************************************************************************//**
    @short      ctor
    @descr      We use it to open right configuration file and let configuration objects fill her caches.
                Then we read all existing entries from right list and cached it inside our object too.
                Normaly we should enable notifications for changes on these values too ... but these feature
                isn't full implemented in the moment.

    @seealso    baseclass ::utl::ConfigItem
    @seealso    method Notify()

    @param      -
    @return     -
*//*-*************************************************************************************************************/
SvtViewOptionsBase_Impl::SvtViewOptionsBase_Impl( const ::rtl::OUString& sList )
        :   ConfigItem ( sList )    // open right configuration file
        ,   m_sListName( sList )
{
    // Read complete list from configuration.
    impl_ReadWholeList();

    // Enable notification for our whole set tree!
    // use "/" to do that!
    // Attention: If you use current existing entry names to do that - you never get a notification
    // for new created items!!!
/*
    css::uno::Sequence< ::rtl::OUString > lNotifyList(1);
    lNotifyList[0] = ROOTNODE_DIALOGS;
    ConfigItem::EnableNotification( lNotifyList );
*/
}

/*-************************************************************************************************************//**
    @short      dtor
    @descr      If something was changed on our internal cached values - baselcass can tell us that by return value
                of method "IsModified()". So we should flush these changes by calling "Commit()" of our own instance.
                It's an auto-save. Normaly user of these object should do that explicitly!

    @attention  Don't forget to call "SetModified()" method of baseclass, if any interface methods of these object
                change internal values. Otherwise nothing will be written to configuration!

    @seealso    baseclass ::utl::ConfigItem
    @seealso    method IsModified()
    @seealso    method SetModified()
    @seealso    method Commit()

    @param      -
    @return     -
*//*-*************************************************************************************************************/
SvtViewOptionsBase_Impl::~SvtViewOptionsBase_Impl()
{
    if( ConfigItem::IsModified() == sal_True )
    {
        Commit();
    }
}

/*-************************************************************************************************************//**
    @short      configuration use it to notify external changes on our interessted values
    @descr      If anywhere change configuration values otside these class - configuration tell it us by calling
                these method. To use these mechanism - we must enable it during our own ctor. See there for further
                informations.

    @attention  Not supported in the moment!

    @seealso    ctor
    @seealso    method EnableNotification()

    @param      "lNames", names of all changed configuration entries or her subnodes
    @return     -
*//*-*************************************************************************************************************/
void SvtViewOptionsBase_Impl::Notify( const css::uno::Sequence< ::rtl::OUString >& lNames )
{
    OSL_ENSURE( sal_False, "SvtViewOptionsBase_Impl::Notify()\nNot supported yet!\n" );
}

/*-************************************************************************************************************//**
    @short      write changed internal values to configuration
    @descr      If our internal values are changed - we should flush it to configuration file.

    @seealso    dtor
    @seealso    method IsModified()

    @param      -
    @return     -
*//*-*************************************************************************************************************/
void SvtViewOptionsBase_Impl::Commit()
{
    const IMPL_TStringList& rAddedList   = m_aList.GetAddedList  ();
    const IMPL_TStringList& rChangedList = m_aList.GetChangedList();
    const IMPL_TStringList& rDeletedList = m_aList.GetDeletedList();

    IMPL_TStringList::const_iterator pItem    ;
    sal_Int32                        nStep = 0;
    ::rtl::OUString                  sPath    ;
    IMPL_TViewData                   aItem    ;

    // delete some items
    if( rDeletedList.size() > 0 )
    {
        css::uno::Sequence< ::rtl::OUString > lDeleted( rDeletedList.size() );
        nStep = 0;
        for( pItem=rDeletedList.begin(); pItem!=rDeletedList.end(); ++pItem )
        {
            lDeleted[nStep]  = PATHSEPERATOR;
            lDeleted[nStep] += PREFIX;
            lDeleted[nStep] += *pItem;
            ++nStep;
        }
        ClearNodeElements( ::rtl::OUString(), lDeleted );
    }

    // add some items
    if( rAddedList.size() > 0 )
    {
        css::uno::Sequence< css::beans::PropertyValue > lAdded;
        nStep = 0;
        for( pItem=rAddedList.begin(); pItem!=rAddedList.end(); ++pItem )
        {
            sPath  = PATHSEPERATOR ;
            sPath += PREFIX        ;
            sPath += *pItem        ;
            sPath += PATHSEPERATOR ;

            m_aList.Find( *pItem, aItem );

            lAdded.realloc( lAdded.getLength()+1 );
            lAdded[nStep].Name    = sPath + PROPERTY_WINDOWSTATE;
            lAdded[nStep].Value <<= aItem.GetWindowState();
            ++nStep;

            if( m_sListName == LIST_TABDIALOGS )
            {
                lAdded.realloc( lAdded.getLength()+1 );
                lAdded[nStep].Name    = sPath + PROPERTY_PAGEID;
                lAdded[nStep].Value <<= aItem.GetPageID();
                ++nStep;
            }

            if( m_sListName == LIST_WINDOWS )
            {
                lAdded.realloc( lAdded.getLength()+1 );
                lAdded[nStep].Name    = sPath + PROPERTY_VISIBLE;
                lAdded[nStep].Value <<= aItem.GetVisible();
                ++nStep;
            }

            css::uno::Sequence< css::beans::NamedValue > lUserData  = aItem.GetUserData();
            sal_Int32                                    nUserCount = lUserData.getLength();
            if( nUserCount > 0 )
            {
                lAdded.realloc( lAdded.getLength()+nUserCount );
            }
            for( sal_Int32 nUserStep=0; nUserStep<nUserCount; ++nUserStep )
            {
                lAdded[nStep].Name  = sPath + lUserData[nUserStep].Name ;
                lAdded[nStep].Value = lUserData[nUserStep].Value        ;
                ++nStep;
            }
        }
        ConfigItem::SetSetProperties( ::rtl::OUString(), lAdded );
    }

    // change some items
    if( rChangedList.size() > 0 )
    {
        css::uno::Sequence< css::beans::PropertyValue > lChanged;
        nStep = 0;
        for( pItem=rChangedList.begin(); pItem!=rChangedList.end(); ++pItem )
        {
            sPath  = PATHSEPERATOR ;
            sPath += PREFIX        ;
            sPath += *pItem        ;
            sPath += PATHSEPERATOR ;

            m_aList.Find( *pItem, aItem );

            if( aItem.IsChanged( CHANGED_WINDOWSTATE ) == sal_True )
            {
                lChanged.realloc( lChanged.getLength()+1 );
                lChanged[nStep].Name    = sPath + PROPERTY_WINDOWSTATE;
                lChanged[nStep].Value <<= aItem.GetWindowState();
                ++nStep;
            }

            if(
                ( m_sListName                       == LIST_TABDIALOGS )  &&
                ( aItem.IsChanged( CHANGED_PAGEID ) == sal_True        )
              )
            {
                lChanged.realloc( lChanged.getLength()+1 );
                lChanged[nStep].Name    = sPath + PROPERTY_PAGEID;
                lChanged[nStep].Value <<= aItem.GetPageID();
                ++nStep;
            }

            if(
                ( m_sListName                        == LIST_WINDOWS )  &&
                ( aItem.IsChanged( CHANGED_VISIBLE ) == sal_True     )
              )
            {
                lChanged.realloc( lChanged.getLength()+1 );
                lChanged[nStep].Name    = sPath + PROPERTY_VISIBLE;
                lChanged[nStep].Value <<= aItem.GetVisible();
                ++nStep;
            }

            if( aItem.IsChanged( CHANGED_USERDATA ) == sal_True )
            {
                css::uno::Sequence< css::beans::NamedValue > lUserData  = aItem.GetUserData();
                sal_Int32                                    nUserCount = lUserData.getLength();
                for( sal_Int32 nUserStep=0; nUserStep<nUserCount; ++nUserStep )
                {
                    lChanged.realloc( lChanged.getLength()+1 );
                    lChanged[nStep].Name  = sPath + lUserData[nUserStep].Name ;
                    lChanged[nStep].Value = lUserData[nUserStep].Value        ;
                    ++nStep;
                }
            }
        }
        ConfigItem::SetSetProperties( ::rtl::OUString(), lChanged );
    }
}

/*-************************************************************************************************************//**
    @short      checks for already existing entries
    @descr      If user don't know, if an entry already exist - he can get this information by calling this method.
                Otherwhise access (read access is enough!) will create it with default values!

    @seealso    member m_aList

    @param      "sName", name of entry to check exist state
    @return     true , if item exist
                false, otherwise
*//*-*************************************************************************************************************/
sal_Bool SvtViewOptionsBase_Impl::Exists( const ::rtl::OUString& sName ) const
{
    return( m_aList.Exists( sName ) );
}

/*-************************************************************************************************************//**
    @short      delete entry
    @descr      Use it to delete set entry by given name.

    @seealso    member m_aList

    @param      "sName", name of entry to delete it
    @return     true , if item not exist(!) or could be deleted
                false, otherwise
*//*-*************************************************************************************************************/
sal_Bool SvtViewOptionsBase_Impl::Delete( const ::rtl::OUString& sName )
{
    // Set default return value to "element no longer exist"!
    // It doesnt matter for user if element not exist or was deleted!
    // Not exist is not exist is ...
    sal_Bool bDeleteState = sal_True;

    if( m_aList.Exists( sName ) == sal_True )
    {
        bDeleteState = ConfigItem::ClearNodeSet( sName );
        if( bDeleteState == sal_True )
        {
            m_aList.Remove( sName );
            ConfigItem::SetModified();
        }
    }
    return bDeleteState;
}

/*-************************************************************************************************************//**
    @short      -
    @descr      -

    @seealso    -

    @param      -
    @return     -
*//*-*************************************************************************************************************/
::rtl::OUString SvtViewOptionsBase_Impl::GetWindowState( const ::rtl::OUString& sName )
{
    m_aList.CreateIfNotExist( sName );
    IMPL_TViewData aData;
    m_aList.Find( sName, aData );
    return aData.GetWindowState();
}

/*-************************************************************************************************************//**
    @short      -
    @descr      -

    @seealso    -

    @param      -
    @return     -
*//*-*************************************************************************************************************/
void SvtViewOptionsBase_Impl::SetWindowState( const ::rtl::OUString& sName  ,
                                              const ::rtl::OUString& sState )
{
    m_aList.CreateIfNotExist( sName );
    IMPL_TViewData aData;
    aData.SetWindowState( sState, sal_True );
    m_aList.Change( sName, aData, CHANGED_WINDOWSTATE, sal_True );
    ConfigItem::SetModified();
}

/*-************************************************************************************************************//**
    @short      returns user data list
    @descr      These method returns the user data list. If entry given by "sName" doesn't already exist - we create
                a new one with default values!
                So it could be neccessary to call "Exist()" before.

    @seealso    method impl_CreateIfNotExist()
    @seealso    member m_aList

    @param      "sName", name of entry which include property "UserData"
    @return     Valid or empty list as default!
*//*-*************************************************************************************************************/
css::uno::Sequence< css::beans::NamedValue > SvtViewOptionsBase_Impl::GetUserData( const ::rtl::OUString& sName )
{
    m_aList.CreateIfNotExist( sName );
    IMPL_TViewData aData;
    m_aList.Find( sName, aData );
    return aData.GetUserData();
}

/*-************************************************************************************************************//**
    @short      set new user data list
    @descr      These method set the new user data list. If entry given by "sName" doesn't already exist - we create
                a new one with default values!
                So it could be neccessary to call "Exist()" before.

    @seealso    method impl_CreateIfNotExist()
    @seealso    member m_aList

    @param      "sName", name of entry which include property "UserData"
    @param      "lData", new value of property
    @return     -
*//*-*************************************************************************************************************/
void SvtViewOptionsBase_Impl::SetUserData( const ::rtl::OUString&                              sName  ,
                                           const css::uno::Sequence< css::beans::NamedValue >& lData  )
{
    m_aList.CreateIfNotExist( sName );
    IMPL_TViewData aData;
    aData.SetUserData( lData, sal_True );
    m_aList.Change( sName, aData, CHANGED_USERDATA, sal_True );
    ConfigItem::SetModified();
}

css::uno::Any SvtViewOptionsBase_Impl::GetUserItem( const ::rtl::OUString& sName ) const
{
    return css::uno::Any();
}

void SvtViewOptionsBase_Impl::SetUserItem( const ::rtl::OUString& sName  ,
                                           const css::uno::Any&   aValue )
{
}

sal_Int32 SvtViewOptionsBase_Impl::GetPageID( const ::rtl::OUString& sName )
{
    m_aList.CreateIfNotExist( sName );
    IMPL_TViewData aData;
    m_aList.Find( sName, aData );
    return aData.GetPageID();
}

void SvtViewOptionsBase_Impl::SetPageID( const ::rtl::OUString& sName ,
                                               sal_Int32        nID   )
{
    m_aList.CreateIfNotExist( sName );
    IMPL_TViewData aData;
    aData.SetPageID( nID, sal_True );
    m_aList.Change( sName, aData, CHANGED_PAGEID, sal_True );
    ConfigItem::SetModified();
}

sal_Bool SvtViewOptionsBase_Impl::GetVisible( const ::rtl::OUString& sName )
{
    m_aList.CreateIfNotExist( sName );
    IMPL_TViewData aData;
    m_aList.Find( sName, aData );
    return aData.GetVisible();
}

void SvtViewOptionsBase_Impl::SetVisible( const ::rtl::OUString& sName    ,
                                                sal_Bool         bVisible )
{
    m_aList.CreateIfNotExist( sName );
    IMPL_TViewData aData;
    aData.SetVisible( bVisible, sal_True );
    m_aList.Change( sName, aData, CHANGED_VISIBLE, sal_True );
    ConfigItem::SetModified();
}

/*-************************************************************************************************************//**
    @short      read complete configuration list
    @descr      These methods try to get all entries of right configuration list and fill it in our internal
                structures. To support reading of unknown properties (which are additional ones by superclass!)
                we call pure virtual function. Programmer can override it to add his own entry names.

    @seealso    impl_AddOwnProperties()

    @param      -
    @return     -
*//*-*************************************************************************************************************/
void SvtViewOptionsBase_Impl::impl_ReadWholeList()
{
    // Clear internal cache
    m_aList.Free();

    css::uno::Sequence< ::rtl::OUString >       lNames      = ConfigItem::GetNodeNames( ::rtl::OUString() );
    sal_Int32                                   nNameCount  = lNames.getLength();
    sal_Int32                                   nAllCount   = 0;

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
        sPath  = lNames[nNameStep] ;
        sPath += PATHSEPERATOR     ;

        lAllNames[nAllStep] = sPath + PROPERTY_WINDOWSTATE;
        ++nAllStep;

        if( m_sListName == LIST_TABDIALOGS )
        {
            lAllNames[nAllStep] = sPath + PROPERTY_PAGEID;
            ++nAllStep;
        }

        if( m_sListName == LIST_WINDOWS )
        {
            lAllNames[nAllStep] = sPath + PROPERTY_VISIBLE;
            ++nAllStep;
        }

                                              sPath     += PROPERTY_USERDATA;
        css::uno::Sequence< ::rtl::OUString > lUserNames = ConfigItem::GetNodeNames( sPath );
        sal_Int32                             nUserCount = lUserNames.getLength();
                                              sPath     += PATHSEPERATOR;
        for( sal_Int32 nUserStep=0; nUserStep<nUserCount; ++nUserStep )
        {
            lAllNames[nAllStep] = sPath + lUserNames[nUserStep];
            ++nAllStep;
        }
    }

    css::uno::Sequence< css::uno::Any > lAllValues = ConfigItem::GetProperties( lAllNames );

    // Safe impossible cases.
    // We have asked for ALL our subtree keys and we would get all his values.
    // It's important for next loop and our index using!
    OSL_ENSURE( !(lAllNames.getLength()!=lAllValues.getLength()), "SvtViewDialogOptions_Impl::impl_ReadWholeList()\nMiss some configuration values for view set!\n" );

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
        sEntryName    = lAllNames[nAllStep].getToken( 0, (sal_Unicode)'/', nToken );
        sEntryName    = sEntryName.copy( PREFIXLENGTH );
        sPropertyName = lAllNames[nAllStep].getToken( 0, (sal_Unicode)'/', nToken );

        if( sPropertyName == PROPERTY_WINDOWSTATE )
        {
            ::rtl::OUString sValue = DEFAULT_WINDOWSTATE;
            lAllValues[nAllStep] >>= sValue;
            ++nAllStep;
            aEntry.SetWindowState( sValue, sal_False );
        }
        else
        if( sPropertyName == PROPERTY_PAGEID )
        {
            sal_Int32 nValue = DEFAULT_PAGEID;
            lAllValues[nAllStep] >>= nValue;
            ++nAllStep;
            aEntry.SetPageID( nValue, sal_False );
        }
        else
        if( sPropertyName == PROPERTY_VISIBLE )
        {
            sal_Bool bValue = DEFAULT_VISIBLE;
            lAllValues[nAllStep] >>= bValue;
            ++nAllStep;
            aEntry.SetVisible( bValue, sal_False );
        }
        else
        if( sPropertyName == PROPERTY_USERDATA )
        {
            css::uno::Sequence< css::beans::NamedValue > lData;
            while( sPropertyName == PROPERTY_USERDATA )
            {
                sUserName = lAllNames[nAllStep].getToken( 0, (sal_Unicode)'/', nToken );
                css::uno::Any aUserValue;
                lAllValues[nAllStep] >>= aUserValue;
                aEntry.AddOrReplaceUserItem( sUserName, aUserValue, sal_False );

                ++nAllStep;

                nToken        = 0;
                sEntryName    = lAllNames[nAllStep].getToken( 0, (sal_Unicode)'/', nToken );
                sPropertyName = lAllNames[nAllStep].getToken( 0, (sal_Unicode)'/', nToken );
            }
        }

        m_aList.Add( sEntryName, aEntry );
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
/*
#ifdef TF_OLDVIEW
css::uno::Sequence< css::beans::NamedValue > SvtViewOptions::GetUserData( sal_Bool bMakeItDifferent ) const
{
    OSL_ENSURE( !(bMakeItDifferent==sal_False), "SvtViewOptions::GetUserData()\nYour code isn't compatible! You use a default parameter without any reason ...\n" );
#else
*/
css::uno::Sequence< css::beans::NamedValue > SvtViewOptions::GetUserData() const
{
//#endif
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
    return css::uno::Any();
}

//*****************************************************************************************************************
void SvtViewOptions::SetUserItem( const ::rtl::OUString& sName  ,
                                  const css::uno::Any&   aValue )
{
}

/*#ifdef TF_OLDVIEW
//*****************************************************************************************************************
void SvtViewOptions::GetPosition( sal_Int32& nX, sal_Int32& nY ) const
{
    ::osl::MutexGuard aGuard( GetOwnStaticMutex() );

    ::rtl::OUString sWindowState;

    switch( m_eViewType )
    {
        case E_DIALOG       :   {
                                    sWindowState = m_pDataContainer_Dialogs->GetWindowState( m_sViewName );
                                }
                                break;
        case E_TABDIALOG    :   {
                                    sWindowState = m_pDataContainer_Dialogs->GetWindowState( m_sViewName );
                                }
                                break;
        case E_TABPAGE      :   {
                                    sWindowState = m_pDataContainer_Dialogs->GetWindowState( m_sViewName );
                                }
                                break;
        case E_WINDOW       :   {
                                    sWindowState = m_pDataContainer_Dialogs->GetWindowState( m_sViewName );
                                }
                                break;
    }

    sal_Int32 nToken = 0;

    nX = sWindowState.getToken( 0, (sal_Unicode)',', nToken ).toInt32();
    nY = 0;
    if( nToken > 0 )
    {
        nY = sWindowState.getToken( 0, (sal_Unicode)',', nToken ).toInt32();
    }
}

//*****************************************************************************************************************
void SvtViewOptions::SetPosition( sal_Int32  nX, sal_Int32  nY )
{
    // Not supported ...
}

//*****************************************************************************************************************
void SvtViewOptions::GetSize( sal_Int32& nWidth, sal_Int32& nHeight ) const
{
    ::osl::MutexGuard aGuard( GetOwnStaticMutex() );

    ::rtl::OUString sWindowState;

    switch( m_eViewType )
    {
        case E_DIALOG       :   {
                                    sWindowState = m_pDataContainer_Dialogs->GetWindowState( m_sViewName );
                                }
                                break;
        case E_TABDIALOG    :   {
                                    sWindowState = m_pDataContainer_Dialogs->GetWindowState( m_sViewName );
                                }
                                break;
        case E_TABPAGE      :   {
                                    sWindowState = m_pDataContainer_Dialogs->GetWindowState( m_sViewName );
                                }
                                break;
        case E_WINDOW       :   {
                                    sWindowState = m_pDataContainer_Dialogs->GetWindowState( m_sViewName );
                                }
                                break;
    }

    sal_Int32       nToken = 0;
    ::rtl::OUString sTemp ;

    nWidth  = 0;
    nHeight = 0;

    sTemp = sWindowState.getToken( 0, (sal_Unicode)',', nToken );
    if( nToken > 0 )
    {
        sTemp = sWindowState.getToken( 0, (sal_Unicode)',', nToken );
        if( nToken > 0 )
        {
            nWidth = sWindowState.getToken( 0, (sal_Unicode)',', nToken ).toInt32();
            if( nToken > 0 )
            {
                nHeight = sWindowState.getToken( 0, (sal_Unicode)',', nToken ).toInt32();
            }
        }
    }
}

//*****************************************************************************************************************
void SvtViewOptions::SetSize( sal_Int32 nWidth, sal_Int32 nHeight )
{
    // Not supported ...
}

//*****************************************************************************************************************
::rtl::OUString SvtViewOptions::GetUserData() const
{
    ::osl::MutexGuard aGuard( GetOwnStaticMutex() );
    css::uno::Sequence< css::beans::NamedValue > lData = GetUserData( sal_True );
    ::rtl::OUString sData;
    sal_Int32 nCount = lData.getLength();
    for( sal_Int32 nStep=0; nStep<nCount; ++nStep )
    {
        if( lData[nStep].Name == CONST_ASCII("compatibleUserData") )
        {
            lData[nStep].Value >>= sData;
            break;
        }
    }

    return sData;
}

//*****************************************************************************************************************
void SvtViewOptions::SetUserData( const ::rtl::OUString& sData )
{
    ::osl::MutexGuard aGuard( GetOwnStaticMutex() );
    css::uno::Sequence< css::beans::NamedValue > lData = GetUserData( sal_True );
    sal_Int32 nCount = lData.getLength();
    sal_Bool bFound = sal_False;
    for( sal_Int32 nStep=0; nStep<nCount; ++nStep )
    {
        if( lData[nStep].Name == CONST_ASCII("compatibleUserData") )
        {
            bFound = sal_True;
            lData[nStep].Value <<= sData;
            break;
        }
    }
    if( bFound == sal_False )
    {
        lData.realloc(nCount+1);
        lData[nCount].Name    = CONST_ASCII("compatibleUserData");
        lData[nCount].Value <<= sData;
    }

    SetUserData( lData );
}
//*****************************************************************************************************************
css::uno::Sequence< ::rtl::OUString > SvtViewOptions::SeperateUserData( const ::rtl::OUString& sData, sal_Unicode cSeperator )
{
    css::uno::Sequence< ::rtl::OUString > seqToken;

    sal_Int32 nLength   = sData.getLength() ;
    sal_Int32 nStart    = 0                 ;
    sal_Int32 nEnd      = 0                 ;
    sal_Int32 nToken    = 0                 ;

    while   (
                nStart < nLength
            )
    {
        nEnd = sData.indexOf( cSeperator, nStart );
        if( nEnd != -1 )
        {
            seqToken[nToken] = sData.copy( nStart, nEnd-nStart );
            nStart = nEnd;
        }
        else
        if( nStart < nLength )
        {
            seqToken[nToken] = sData.copy( nStart, nLength-nStart );
            nStart = nLength;
        }
        ++nToken;
    }

    return seqToken;
}

//*****************************************************************************************************************
::rtl::OUString SvtViewOptions::GenerateUserData( const css::uno::Sequence< ::rtl::OUString >& seqData, sal_Unicode cSeperator )
{
    ::rtl::OUStringBuffer sUserData( 1000 );
    sal_Int32 nCount = seqData.getLength();
    for( sal_Int32 nToken=0; nToken<nCount; ++nToken )
    {
        sUserData.append( seqData[nToken] );
        if( nToken < nCount )
        {
            sUserData.append( cSeperator );
        }
    }
    return sUserData.makeStringAndClear();
}

//*****************************************************************************************************************
::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > SvtViewOptions::GetAnyData() const
{
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue > lSource = GetUserData( sal_True );
    sal_Int32 nCount = lSource.getLength();
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > lDestination( nCount );
    for( sal_Int32 nStep=0; nStep<nCount; ++nStep )
    {
        lDestination[nStep].Name  = lSource[nStep].Name ;
        lDestination[nStep].Value = lSource[nStep].Value;
    }
    return lDestination;
}

//*****************************************************************************************************************
void SvtViewOptions::SetAnyData( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& lData )
{
    sal_Int32 nCount = lData.getLength();
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue > lDestination( nCount );
    for( sal_Int32 nStep=0; nStep<nCount; ++nStep )
    {
        lDestination[nStep].Name  = lData[nStep].Name ;
        lDestination[nStep].Value = lData[nStep].Value;
    }
    SetUserData( lDestination );
}
#endif
*/

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

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

#include <sal/config.h>

#include <sal/log.hxx>
#include <unotools/dynamicmenuoptions.hxx>
#include <unotools/moduleoptions.hxx>
#include <unotools/configmgr.hxx>
#include <unotools/configitem.hxx>
#include <tools/debug.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>

#include <vector>

#include "itemholder1.hxx"

#include <algorithm>

using namespace ::std;
using namespace ::utl;
using namespace ::osl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;

#define ROOTNODE_MENUS                                  "Office.Common/Menus/"
#define PATHDELIMITER                                   "/"

#define SETNODE_NEWMENU                                 "New"
#define SETNODE_WIZARDMENU                              "Wizard"
#define SETNODE_HELPBOOKMARKS                           "HelpBookmarks"

#define PROPERTYNAME_URL                                DYNAMICMENU_PROPERTYNAME_URL
#define PROPERTYNAME_TITLE                              DYNAMICMENU_PROPERTYNAME_TITLE
#define PROPERTYNAME_IMAGEIDENTIFIER                    DYNAMICMENU_PROPERTYNAME_IMAGEIDENTIFIER
#define PROPERTYNAME_TARGETNAME                         DYNAMICMENU_PROPERTYNAME_TARGETNAME

#define PROPERTYCOUNT                                   4

#define OFFSET_URL                                      0
#define OFFSET_TITLE                                    1
#define OFFSET_IMAGEIDENTIFIER                          2
#define OFFSET_TARGETNAME                               3

#define PATHPREFIX_SETUP                                "m"

/*-****************************************************************************************************************
    @descr  struct to hold information about one menu entry.
****************************************************************************************************************-*/
struct SvtDynMenuEntry
{
        OUString    sURL;
        OUString    sTitle;
        OUString    sImageIdentifier;
        OUString    sTargetName;
};

/*-****************************************************************************************************************
    @descr  support simple menu structures and operations on it
****************************************************************************************************************-*/
class SvtDynMenu
{
    public:
        // append setup written menu entry
        // Don't touch name of entry. It was defined by setup and must be the same every time!
        // Look for double menu entries here too... may be some separator items are superflous...
        void AppendSetupEntry( const SvtDynMenuEntry& rEntry )
        {
            if(
                ( lSetupEntries.size()         <  1           )  ||
                ( lSetupEntries.rbegin()->sURL != rEntry.sURL )
              )
            {
                lSetupEntries.push_back( rEntry );
            }
        }

        // convert internal list to external format
        // for using it on right menus really
        // Notice:   We build a property list with 4 entries and set it on result list then.
        //           The while-loop starts with pointer on internal member list lSetupEntries, change to
        //           lUserEntries then and stop after that with NULL!
        //           Separator entries will be packed in another way then normal entries! We define
        //           special string "sSeparator" to perform too ...
        Sequence< Sequence< PropertyValue > > GetList() const
        {
            sal_Int32                             nSetupCount = (sal_Int32)lSetupEntries.size();
            sal_Int32                             nUserCount  = (sal_Int32)lUserEntries.size();
            sal_Int32                             nStep       = 0;
            Sequence< PropertyValue >             lProperties ( PROPERTYCOUNT );
            Sequence< Sequence< PropertyValue > > lResult     ( nSetupCount+nUserCount );
            OUString                              sSeparator  ( "private:separator" );
            const vector< SvtDynMenuEntry >*            pList       = &lSetupEntries;

            lProperties[OFFSET_URL            ].Name = PROPERTYNAME_URL;
            lProperties[OFFSET_TITLE          ].Name = PROPERTYNAME_TITLE;
            lProperties[OFFSET_IMAGEIDENTIFIER].Name = PROPERTYNAME_IMAGEIDENTIFIER;
            lProperties[OFFSET_TARGETNAME     ].Name = PROPERTYNAME_TARGETNAME;

            while( pList != nullptr )
            {
                for( vector< SvtDynMenuEntry >::const_iterator pItem =pList->begin();
                                                         pItem!=pList->end();
                                                         ++pItem              )
                {
                    if( pItem->sURL == sSeparator )
                    {
                        lProperties[OFFSET_URL              ].Value <<= sSeparator;
                        lProperties[OFFSET_TITLE            ].Value <<= OUString();
                        lProperties[OFFSET_IMAGEIDENTIFIER  ].Value <<= OUString();
                        lProperties[OFFSET_TARGETNAME       ].Value <<= OUString();
                    }
                    else
                    {
                        lProperties[OFFSET_URL              ].Value <<= pItem->sURL;
                        lProperties[OFFSET_TITLE            ].Value <<= pItem->sTitle;
                        lProperties[OFFSET_IMAGEIDENTIFIER  ].Value <<= pItem->sImageIdentifier;
                        lProperties[OFFSET_TARGETNAME       ].Value <<= pItem->sTargetName;
                    }
                    lResult[nStep] = lProperties;
                    ++nStep;
                }
                if( pList == &lSetupEntries )
                    pList = &lUserEntries;
                else
                    pList = nullptr;
            }
            return lResult;
        }

    private:
        vector< SvtDynMenuEntry > lSetupEntries;
        vector< SvtDynMenuEntry > lUserEntries;
};

class SvtDynamicMenuOptions_Impl : public ConfigItem
{
    public:

         SvtDynamicMenuOptions_Impl();
        virtual ~SvtDynamicMenuOptions_Impl() override;

        /*-****************************************************************************************************
            @short      called for notify of configmanager
            @descr      This method is called from the ConfigManager before the application ends or from the
                         PropertyChangeListener if the sub tree broadcasts changes. You must update your
                        internal values.

            @seealso    baseclass ConfigItem

            @param      "lPropertyNames" is the list of properties which should be updated.
        *//*-*****************************************************************************************************/

        virtual void Notify( const Sequence< OUString >& lPropertyNames ) override;

        /*-****************************************************************************************************
            @short      base implementation of public interface for "SvtDynamicMenuOptions"!
            @descr      These class is used as static member of "SvtDynamicMenuOptions" ...
                        => The code exist only for one time and isn't duplicated for every instance!
        *//*-*****************************************************************************************************/

        Sequence< Sequence< PropertyValue > >   GetMenu     (           EDynamicMenuType    eMenu           ) const;

    private:

        virtual void ImplCommit() override;

        /*-****************************************************************************************************
            @short      return list of key names of our configuration management which represent our module tree
            @descr      This method returns the current list of key names! We need it to get needed values from our
                        configuration management and support dynamical menu item lists!
            @param      "nNewCount"     ,   returns count of menu entries for "new"
            @param      "nWizardCount"  ,   returns count of menu entries for "wizard"
            @return     A list of configuration key names is returned.
        *//*-*****************************************************************************************************/

        Sequence< OUString > impl_GetPropertyNames( sal_uInt32& nNewCount, sal_uInt32& nWizardCount, sal_uInt32& nHelpBookmarksCount );

        /*-****************************************************************************************************
            @short      sort given source list and expand it for all well known properties to destination
            @descr      We must support sets of entries with count inside the name .. but some of them could be missing!
                        e.g. s1-s2-s3-s0-u1-s6-u5-u7
                        Then we must sort it by name and expand it to the follow one:
                            sSetNode/s0/URL
                            sSetNode/s0/Title
                            sSetNode/s0/...
                            sSetNode/s1/URL
                            sSetNode/s1/Title
                            sSetNode/s1/...
                            ...
                            sSetNode/s6/URL
                            sSetNode/s6/Title
                            sSetNode/s6/...
                            sSetNode/u1/URL
                            sSetNode/u1/Title
                            sSetNode/u1/...
                            ...
                            sSetNode/u7/URL
                            sSetNode/u7/Title
                            sSetNode/u7/...
                        Rules: We start with all setup written entries names "sx" and x=[0..n].
                        Then we handle all "ux" items. Inside these blocks we sort it ascending by number.

            @attention  We add these expanded list to the end of given "lDestination" list!
                        So we must start on "lDestination.getLength()".
                        Reallocation of memory of destination list is done by us!

            @seealso    method impl_GetPropertyNames()

            @param      "lSource"      ,   original list (e.g. [m1-m2-m3-m6-m0] )
            @param      "lDestination" ,   destination of operation
            @param      "sSetNode"     ,   name of configuration set to build complete path
            @return     A list of configuration key names is returned.
        *//*-*****************************************************************************************************/

        static void impl_SortAndExpandPropertyNames( const Sequence< OUString >& lSource      ,
                                                     Sequence< OUString >& lDestination ,
                                                     const OUString&             sSetNode     );

    //  private member

    private:

        SvtDynMenu  m_aNewMenu;
        SvtDynMenu  m_aWizardMenu;
        SvtDynMenu  m_aHelpBookmarksMenu;
};

//  constructor

SvtDynamicMenuOptions_Impl::SvtDynamicMenuOptions_Impl()
    // Init baseclasses first
    :   ConfigItem( ROOTNODE_MENUS )
    // Init member then...
{
    // Get names and values of all accessible menu entries and fill internal structures.
    // See impl_GetPropertyNames() for further information.
    sal_uInt32              nNewCount           = 0;
    sal_uInt32              nWizardCount        = 0;
    sal_uInt32              nHelpBookmarksCount = 0;
    Sequence< OUString >    lNames              = impl_GetPropertyNames ( nNewCount           ,
                                                                          nWizardCount        ,
                                                                          nHelpBookmarksCount );
    Sequence< Any >         lValues             = GetProperties         ( lNames              );

    // Safe impossible cases.
    // We need values from ALL configuration keys.
    // Follow assignment use order of values in relation to our list of key names!
    DBG_ASSERT( !(lNames.getLength()!=lValues.getLength()), "SvtDynamicMenuOptions_Impl::SvtDynamicMenuOptions_Impl()\nI miss some values of configuration keys!\n" );

    // Copy values from list in right order to our internal member.
    // Attention: List for names and values have an internal construction pattern!

    // first "New" menu ...
    //      Name                            Value
    //      /New/1/URL                      "private:factory/swriter"
    //      /New/1/Title                    "Neues Writer Dokument"
    //      /New/1/ImageIdentifier          "icon_writer"
    //      /New/1/TargetName               "_blank"

    //      /New/2/URL                      "private:factory/scalc"
    //      /New/2/Title                    "Neues Calc Dokument"
    //      /New/2/ImageIdentifier          "icon_calc"
    //      /New/2/TargetName               "_blank"

    // second "Wizard" menu ...
    //      /Wizard/1/URL                   "file://b"
    //      /Wizard/1/Title                 "MalWas"
    //      /Wizard/1/ImageIdentifier       "icon_?"
    //      /Wizard/1/TargetName            "_self"

    //      ... and so on ...

    sal_uInt32  nItem     = 0;
    sal_uInt32  nPosition = 0;

    // Get names/values for new menu.
    // 4 subkeys for every item!
    for( nItem=0; nItem<nNewCount; ++nItem )
    {
        SvtDynMenuEntry   aItem;
        lValues[nPosition] >>= aItem.sURL;
        ++nPosition;
        lValues[nPosition] >>= aItem.sTitle;
        ++nPosition;
        lValues[nPosition] >>= aItem.sImageIdentifier;
        ++nPosition;
        lValues[nPosition] >>= aItem.sTargetName;
        ++nPosition;
        m_aNewMenu.AppendSetupEntry( aItem );
    }

    // Attention: Don't reset nPosition here!

    // Get names/values for wizard menu.
    // 4 subkeys for every item!
    for( nItem=0; nItem<nWizardCount; ++nItem )
    {
        SvtDynMenuEntry   aItem;
        lValues[nPosition] >>= aItem.sURL;
        ++nPosition;
        lValues[nPosition] >>= aItem.sTitle;
        ++nPosition;
        lValues[nPosition] >>= aItem.sImageIdentifier;
        ++nPosition;
        lValues[nPosition] >>= aItem.sTargetName;
        ++nPosition;
        m_aWizardMenu.AppendSetupEntry( aItem );
    }

    // Attention: Don't reset nPosition here!

    // Get names/values for wizard menu.
    // 4 subkeys for every item!
    for( nItem=0; nItem<nHelpBookmarksCount; ++nItem )
    {
        SvtDynMenuEntry   aItem;
        lValues[nPosition] >>= aItem.sURL;
        ++nPosition;
        lValues[nPosition] >>= aItem.sTitle;
        ++nPosition;
        lValues[nPosition] >>= aItem.sImageIdentifier;
        ++nPosition;
        lValues[nPosition] >>= aItem.sTargetName;
        ++nPosition;
        m_aHelpBookmarksMenu.AppendSetupEntry( aItem );
    }

/*TODO: Not used in the moment! see Notify() ...
    // Enable notification mechanism of our baseclass.
    // We need it to get information about changes outside these class on our used configuration keys!
    EnableNotification( lNames );
*/
}

//  destructor

SvtDynamicMenuOptions_Impl::~SvtDynamicMenuOptions_Impl()
{
    assert(!IsModified()); // should have been committed
}

//  public method

void SvtDynamicMenuOptions_Impl::Notify( const Sequence< OUString >& )
{
    SAL_WARN( "unotools.config", "SvtDynamicMenuOptions_Impl::Notify()\nNot implemented yet! I don't know how I can handle a dynamical list of unknown properties ...\n" );
}

//  public method

void SvtDynamicMenuOptions_Impl::ImplCommit()
{
    SAL_WARN("unotools.config", "SvtDynamicMenuOptions_Impl::ImplCommit(): Not implemented yet!");
    /*
    // Write all properties!
    // Delete complete sets first.
    ClearNodeSet( SETNODE_NEWMENU    );
    ClearNodeSet( SETNODE_WIZARDMENU );
    ClearNodeSet( SETNODE_HELPBOOKMARKS );

    MenuEntry                    aItem;
    OUString                    sNode;
    Sequence< PropertyValue >   lPropertyValues( PROPERTYCOUNT );
    sal_uInt32                  nItem          = 0;

    // Copy "new" menu entries to save-list!
    sal_uInt32 nNewCount = m_aNewMenu.size();
    for( nItem=0; nItem<nNewCount; ++nItem )
    {
        aItem = m_aNewMenu[nItem];
        // Format:  "New/1/URL"
        //          "New/1/Title"
        //          ...
        sNode = SETNODE_NEWMENU + PATHDELIMITER + PATHPREFIX + OUString::valueOf( (sal_Int32)nItem ) + PATHDELIMITER;

        lPropertyValues[OFFSET_URL             ].Name  =   sNode + PROPERTYNAME_URL;
        lPropertyValues[OFFSET_TITLE           ].Name  =   sNode + PROPERTYNAME_TITLE;
        lPropertyValues[OFFSET_IMAGEIDENTIFIER ].Name  =   sNode + PROPERTYNAME_IMAGEIDENTIFIER;
        lPropertyValues[OFFSET_TARGETNAME      ].Name  =   sNode + PROPERTYNAME_TARGETNAME;

        lPropertyValues[OFFSET_URL             ].Value <<= aItem.sURL;
        lPropertyValues[OFFSET_TITLE           ].Value <<= aItem.sTitle;
        lPropertyValues[OFFSET_IMAGEIDENTIFIER ].Value <<= aItem.sImageIdentifier;
        lPropertyValues[OFFSET_TARGETNAME      ].Value <<= aItem.sTargetName;

        SetSetProperties( SETNODE_NEWMENU, lPropertyValues );
    }

    // Copy "wizard" menu entries to save-list!
    sal_uInt32 nWizardCount = m_aWizardMenu.size();
    for( nItem=0; nItem<nWizardCount; ++nItem )
    {
        aItem = m_aWizardMenu[nItem];
        // Format:  "Wizard/1/URL"
        //          "Wizard/1/Title"
        //          ...
        sNode = SETNODE_WIZARDMENU + PATHDELIMITER + PATHPREFIX + OUString::valueOf( (sal_Int32)nItem ) + PATHDELIMITER;

        lPropertyValues[OFFSET_URL             ].Name  =   sNode + PROPERTYNAME_URL;
        lPropertyValues[OFFSET_TITLE           ].Name  =   sNode + PROPERTYNAME_TITLE;
        lPropertyValues[OFFSET_IMAGEIDENTIFIER ].Name  =   sNode + PROPERTYNAME_IMAGEIDENTIFIER;
        lPropertyValues[OFFSET_TARGETNAME      ].Name  =   sNode + PROPERTYNAME_TARGETNAME;

        lPropertyValues[OFFSET_URL             ].Value <<= aItem.sURL;
        lPropertyValues[OFFSET_TITLE           ].Value <<= aItem.sTitle;
        lPropertyValues[OFFSET_IMAGEIDENTIFIER ].Value <<= aItem.sImageIdentifier;
        lPropertyValues[OFFSET_TARGETNAME      ].Value <<= aItem.sTargetName;

        SetSetProperties( SETNODE_WIZARDMENU, lPropertyValues );
    }

    // Copy help bookmarks entries to save-list!
    sal_uInt32 nHelpBookmarksCount = m_aHelpBookmarksMenu.size();
    for( nItem=0; nItem<nHelpBookmarksCount; ++nItem )
    {
        aItem = m_aHelpBookmarksMenu[nItem];
        // Format:  "HelpBookmarks/1/URL"
        //          "HelpBookmarks/1/Title"
        //          ...
        sNode = SETNODE_HELPBOOKMARKS + PATHDELIMITER + PATHPREFIX + OUString::valueOf( (sal_Int32)nItem ) + PATHDELIMITER;

        lPropertyValues[OFFSET_URL             ].Name  =   sNode + PROPERTYNAME_URL;
        lPropertyValues[OFFSET_TITLE           ].Name  =   sNode + PROPERTYNAME_TITLE;
        lPropertyValues[OFFSET_IMAGEIDENTIFIER ].Name  =   sNode + PROPERTYNAME_IMAGEIDENTIFIER;
        lPropertyValues[OFFSET_TARGETNAME      ].Name  =   sNode + PROPERTYNAME_TARGETNAME;

        lPropertyValues[OFFSET_URL             ].Value <<= aItem.sURL;
        lPropertyValues[OFFSET_TITLE           ].Value <<= aItem.sTitle;
        lPropertyValues[OFFSET_IMAGEIDENTIFIER ].Value <<= aItem.sImageIdentifier;
        lPropertyValues[OFFSET_TARGETNAME      ].Value <<= aItem.sTargetName;

        SetSetProperties( SETNODE_HELPBOOKMARKS, lPropertyValues );
    }
    */
}

//  public method

Sequence< Sequence< PropertyValue > > SvtDynamicMenuOptions_Impl::GetMenu( EDynamicMenuType eMenu ) const
{
    Sequence< Sequence< PropertyValue > > lReturn;
    switch( eMenu )
    {
        case E_NEWMENU      :   {
                                    lReturn = m_aNewMenu.GetList();
                                }
                                break;

        case E_WIZARDMENU   :   {
                                    lReturn = m_aWizardMenu.GetList();
                                }
                                break;

        case E_HELPBOOKMARKS :  {
                                    lReturn = m_aHelpBookmarksMenu.GetList();
                                }
                                break;
    }
    return lReturn;
}

//  private method

Sequence< OUString > SvtDynamicMenuOptions_Impl::impl_GetPropertyNames( sal_uInt32& nNewCount, sal_uInt32& nWizardCount, sal_uInt32& nHelpBookmarksCount )
{
    // First get ALL names of current existing list items in configuration!
    Sequence< OUString > lNewItems           = GetNodeNames( SETNODE_NEWMENU       );
    Sequence< OUString > lWizardItems        = GetNodeNames( SETNODE_WIZARDMENU    );
    Sequence< OUString > lHelpBookmarksItems = GetNodeNames( SETNODE_HELPBOOKMARKS );

    // Get information about list counts ...
    nNewCount           = lNewItems.getLength          ();
    nWizardCount        = lWizardItems.getLength       ();
    nHelpBookmarksCount = lHelpBookmarksItems.getLength();

    // Sort and expand all three list to result list ...
    Sequence< OUString > lProperties;
    impl_SortAndExpandPropertyNames( lNewItems          , lProperties, SETNODE_NEWMENU       );
    impl_SortAndExpandPropertyNames( lWizardItems       , lProperties, SETNODE_WIZARDMENU    );
    impl_SortAndExpandPropertyNames( lHelpBookmarksItems, lProperties, SETNODE_HELPBOOKMARKS );

    // Return result.
    return lProperties;
}

//  private helper

class CountWithPrefixSort
{
    public:
        bool operator() ( const OUString& s1 ,
                         const OUString& s2 ) const
        {
            // Get order numbers from entry name without prefix.
            // e.g. "m10" => 10
            //      "m5"  => 5
            sal_Int32 n1 = s1.copy( 1, s1.getLength()-1 ).toInt32();
            sal_Int32 n2 = s2.copy( 1, s2.getLength()-1 ).toInt32();
            // MUST be in [0,1] ... because it's a difference between
            // insert-positions of given entries in sorted list!
            return( n1<n2 );
        }
};

class SelectByPrefix
{
    public:
        bool operator() ( const OUString& s ) const
        {
            // Prefer setup written entries by check first letter of given string. It must be a "s".
            return s.startsWith( PATHPREFIX_SETUP );
        }
};

//  private method

void SvtDynamicMenuOptions_Impl::impl_SortAndExpandPropertyNames( const Sequence< OUString >& lSource      ,
                                                                        Sequence< OUString >& lDestination ,
                                                                  const OUString&             sSetNode     )
{
    vector< OUString >  lTemp;
    sal_Int32           nSourceCount     = lSource.getLength();
    sal_Int32           nDestinationStep = lDestination.getLength(); // start on end of current list ...!

    lDestination.realloc( (nSourceCount*PROPERTYCOUNT)+nDestinationStep ); // get enough memory for copy operations after nDestination ...

    // Copy all items to temp. vector to use fast sort operations :-)
    for( sal_Int32 nSourceStep=0; nSourceStep<nSourceCount; ++nSourceStep )
        lTemp.push_back( lSource[nSourceStep] );

    // Sort all entries by number ...
    stable_sort( lTemp.begin(), lTemp.end(), CountWithPrefixSort() );
    // and split into setup & user written entries!
    stable_partition( lTemp.begin(), lTemp.end(), SelectByPrefix() );

    // Copy sorted entries to destination and expand every item with
    // 4 supported sub properties.
    for( vector< OUString >::const_iterator pItem =lTemp.begin();
                                            pItem!=lTemp.end();
                                            ++pItem              )
    {
        OUString sFixPath(sSetNode + PATHDELIMITER + *pItem + PATHDELIMITER);
        lDestination[nDestinationStep++] = sFixPath + PROPERTYNAME_URL;
        lDestination[nDestinationStep++] = sFixPath + PROPERTYNAME_TITLE;
        lDestination[nDestinationStep++] = sFixPath
            + PROPERTYNAME_IMAGEIDENTIFIER;
        lDestination[nDestinationStep++] = sFixPath + PROPERTYNAME_TARGETNAME;
    }
}

namespace {
    // global
    std::weak_ptr<SvtDynamicMenuOptions_Impl> g_pDynamicMenuOptions;
}

SvtDynamicMenuOptions::SvtDynamicMenuOptions()
{
    // Global access, must be guarded (multithreading!).
    MutexGuard aGuard( GetOwnStaticMutex() );

    m_pImpl = g_pDynamicMenuOptions.lock();
    if( !m_pImpl )
    {
        m_pImpl = std::make_shared<SvtDynamicMenuOptions_Impl>();
        g_pDynamicMenuOptions = m_pImpl;
        ItemHolder1::holdConfigItem(EItem::DynamicMenuOptions);
    }
}

SvtDynamicMenuOptions::~SvtDynamicMenuOptions()
{
    // Global access, must be guarded (multithreading!)
    MutexGuard aGuard( GetOwnStaticMutex() );

    m_pImpl.reset();
}

//  public method

Sequence< Sequence< PropertyValue > > SvtDynamicMenuOptions::GetMenu( EDynamicMenuType eMenu ) const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pImpl->GetMenu( eMenu );
}

namespace
{
    class theDynamicMenuOptionsMutex : public rtl::Static<osl::Mutex, theDynamicMenuOptionsMutex>{};
}

//  private method

Mutex& SvtDynamicMenuOptions::GetOwnStaticMutex()
{
    return theDynamicMenuOptionsMutex::get();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

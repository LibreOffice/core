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


#include <svtools/menuoptions.hxx>
#include <unotools/configmgr.hxx>
#include <unotools/configitem.hxx>
#include <tools/debug.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>

#include "itemholder2.hxx"

#include <list>


//  namespaces


using namespace ::utl                   ;
using namespace ::osl                   ;
using namespace ::com::sun::star::uno   ;

#define ROOTNODE_MENU                           OUString("Office.Common/View/Menu"  )
#define DEFAULT_DONTHIDEDISABLEDENTRIES         false
#define DEFAULT_FOLLOWMOUSE                     true
#define DEFAULT_MENUICONS                       TRISTATE_INDET

#define PROPERTYNAME_DONTHIDEDISABLEDENTRIES    "DontHideDisabledEntry"
#define PROPERTYNAME_FOLLOWMOUSE                "FollowMouse"
#define PROPERTYNAME_SHOWICONSINMENUES          "ShowIconsInMenues"
#define PROPERTYNAME_SYSTEMICONSINMENUES        "IsSystemIconsInMenus"

#define PROPERTYHANDLE_DONTHIDEDISABLEDENTRIES  0
#define PROPERTYHANDLE_FOLLOWMOUSE              1
#define PROPERTYHANDLE_SHOWICONSINMENUES        2
#define PROPERTYHANDLE_SYSTEMICONSINMENUES      3

#define PROPERTYCOUNT                           4

#include <tools/link.hxx>


//  private declarations!


class SvtMenuOptions_Impl : public ConfigItem
{

    //  private member


    private:
        ::std::list<Link<LinkParamNone*,void>> aList;
        bool        m_bDontHideDisabledEntries          ;   /// cache "DontHideDisabledEntries" of Menu section
        bool        m_bFollowMouse                      ;   /// cache "FollowMouse" of Menu section
        TriState    m_eMenuIcons                        ;   /// cache "MenuIcons" of Menu section


    //  public methods


    public:


        //  constructor / destructor


         SvtMenuOptions_Impl();
        virtual ~SvtMenuOptions_Impl();

        void AddListenerLink( const Link<LinkParamNone*,void>& rLink );
        void RemoveListenerLink( const Link<LinkParamNone*,void>& rLink );


        //  override methods of baseclass


        /*-****************************************************************************************************
            @short      called for notify of configmanager
            @descr      These method is called from the ConfigManager before application ends or from the
                         PropertyChangeListener if the sub tree broadcasts changes. You must update your
                        internal values.

            @seealso    baseclass ConfigItem

            @param      "seqPropertyNames" is the list of properties which should be updated.
        *//*-*****************************************************************************************************/

        virtual void Notify( const Sequence< OUString >& seqPropertyNames ) override;

        //  public interface


        /*-****************************************************************************************************
            @short      access method to get internal values
            @descr      These methods give us a chance to regulate access to our internal values.
                        It's not used in the moment - but it's possible for the future!
        *//*-*****************************************************************************************************/

        bool        IsEntryHidingEnabled() const
                    { return m_bDontHideDisabledEntries; }

        TriState    GetMenuIconsState() const
                    { return m_eMenuIcons; }

        void        SetMenuIconsState(TriState eState)
                    {
                        m_eMenuIcons = eState;
                        SetModified();
                        for ( ::std::list<Link<LinkParamNone*,void>>::const_iterator iter = aList.begin(); iter != aList.end(); ++iter )
                            iter->Call( nullptr );
                        // tdf#93451: don't Commit() here, it's too early
                    }


    //  private methods


    private:

        virtual void ImplCommit() override;

        /*-****************************************************************************************************
            @short      return list of fix key names of our configuration management which represent our module tree
            @descr      These methods return a static const list of key names. We need it to get needed values from our
                        configuration management.
            @return     A list of needed configuration keys is returned.
        *//*-*****************************************************************************************************/

        static Sequence< OUString > impl_GetPropertyNames();
};


//  constructor

SvtMenuOptions_Impl::SvtMenuOptions_Impl()
    // Init baseclasses first
    :   ConfigItem                  ( ROOTNODE_MENU                     )
    // Init member then.
    ,   m_bDontHideDisabledEntries  ( DEFAULT_DONTHIDEDISABLEDENTRIES   )
    ,   m_bFollowMouse              ( DEFAULT_FOLLOWMOUSE               )
    ,   m_eMenuIcons                ( DEFAULT_MENUICONS                 )
{
    // Use our static list of configuration keys to get his values.
    Sequence< OUString >    seqNames    = impl_GetPropertyNames();
    Sequence< Any >         seqValues   = GetProperties( seqNames ) ;

    // Safe impossible cases.
    // We need values from ALL configuration keys.
    // Follow assignment use order of values in relation to our list of key names!
    DBG_ASSERT( !(seqNames.getLength()!=seqValues.getLength()), "SvtMenuOptions_Impl::SvtMenuOptions_Impl()\nI miss some values of configuration keys!\n" );

    bool bMenuIcons = true;
    bool bSystemMenuIcons = true;
    if (m_eMenuIcons == TRISTATE_INDET)
        bMenuIcons = Application::GetSettings().GetStyleSettings().GetPreferredUseImagesInMenus();
    else
    {
        bSystemMenuIcons = false;
        bMenuIcons = m_eMenuIcons != TRISTATE_FALSE;
    }

    // Copy values from list in right order to our internal member.
    sal_Int32 nPropertyCount    =   seqValues.getLength()   ;
    sal_Int32 nProperty         =   0                       ;
    for( nProperty=0; nProperty<nPropertyCount; ++nProperty )
    {
        // Safe impossible cases.
        // Check any for valid value.
        DBG_ASSERT( seqValues[nProperty].hasValue(), "SvtMenuOptions_Impl::SvtMenuOptions_Impl()\nInvalid property value for property detected!\n" );

        if (!seqValues[nProperty].hasValue())
            continue;

        switch( nProperty )
        {
            case PROPERTYHANDLE_DONTHIDEDISABLEDENTRIES :   {
                                                                DBG_ASSERT(!(seqValues[nProperty].getValueTypeClass()!=TypeClass_BOOLEAN), "SvtMenuOptions_Impl::SvtMenuOptions_Impl()\nWho has changed the value type of \"Office.Common\\View\\Menu\\DontHideDisabledEntry\"?" );
                                                                seqValues[nProperty] >>= m_bDontHideDisabledEntries;
                                                            }
                                                            break;

            case PROPERTYHANDLE_FOLLOWMOUSE             :   {
                                                                DBG_ASSERT(!(seqValues[nProperty].getValueTypeClass()!=TypeClass_BOOLEAN), "SvtMenuOptions_Impl::SvtMenuOptions_Impl()\nWho has changed the value type of \"Office.Common\\View\\Menu\\FollowMouse\"?" );
                                                                seqValues[nProperty] >>= m_bFollowMouse;
                                                            }
                                                            break;
            case PROPERTYHANDLE_SHOWICONSINMENUES       :   {
                                                                DBG_ASSERT(!(seqValues[nProperty].getValueTypeClass()!=TypeClass_BOOLEAN), "SvtMenuOptions_Impl::SvtMenuOptions_Impl()\nWho has changed the value type of \"Office.Common\\View\\Menu\\ShowIconsInMenues\"?" );
                                                                seqValues[nProperty] >>= bMenuIcons;
                                                            }
                                                            break;
            case PROPERTYHANDLE_SYSTEMICONSINMENUES     :   {
                                                                DBG_ASSERT(!(seqValues[nProperty].getValueTypeClass()!=TypeClass_BOOLEAN), "SvtMenuOptions_Impl::SvtMenuOptions_Impl()\nWho has changed the value type of \"Office.Common\\View\\Menu\\IsSystemIconsInMenus\"?" );
                                                                seqValues[nProperty] >>= bSystemMenuIcons;
                                                            }
                                                            break;
        }
    }

    m_eMenuIcons = bSystemMenuIcons ? TRISTATE_INDET : static_cast<TriState>(bMenuIcons);

    EnableNotification( seqNames );
}


//  destructor

SvtMenuOptions_Impl::~SvtMenuOptions_Impl()
{
    assert(!IsModified()); // should have been committed
}


//  public method

void SvtMenuOptions_Impl::Notify( const Sequence< OUString >& seqPropertyNames )
{
    // Use given list of updated properties to get his values from configuration directly!
    Sequence< Any > seqValues = GetProperties( seqPropertyNames );
    // Safe impossible cases.
    // We need values from ALL notified configuration keys.
    DBG_ASSERT( !(seqPropertyNames.getLength()!=seqValues.getLength()), "SvtMenuOptions_Impl::Notify()\nI miss some values of configuration keys!\n" );

    bool bMenuSettingsChanged = false;
    bool bMenuIcons = true;
    bool bSystemMenuIcons = true;
    if (m_eMenuIcons == TRISTATE_INDET)
        bMenuIcons = Application::GetSettings().GetStyleSettings().GetUseImagesInMenus();
    else
    {
        bSystemMenuIcons = false;
        bMenuIcons = m_eMenuIcons != TRISTATE_FALSE;
    }

    // Step over list of property names and get right value from coreesponding value list to set it on internal members!
    sal_Int32 nCount = seqPropertyNames.getLength();
    for( sal_Int32 nProperty=0; nProperty<nCount; ++nProperty )
    {
        if( seqPropertyNames[nProperty] == PROPERTYNAME_DONTHIDEDISABLEDENTRIES )
        {
            DBG_ASSERT(!(seqValues[nProperty].getValueTypeClass()!=TypeClass_BOOLEAN), "SvtMenuOptions_Impl::Notify()\nWho has changed the value type of \"Office.Common\\View\\Menu\\DontHideDisabledEntry\"?" );
            seqValues[nProperty] >>= m_bDontHideDisabledEntries;
        }
        else if( seqPropertyNames[nProperty] == PROPERTYNAME_FOLLOWMOUSE )
        {
            DBG_ASSERT(!(seqValues[nProperty].getValueTypeClass()!=TypeClass_BOOLEAN), "SvtMenuOptions_Impl::Notify()\nWho has changed the value type of \"Office.Common\\View\\Menu\\FollowMouse\"?" );
            seqValues[nProperty] >>= m_bFollowMouse;
        }
        else if( seqPropertyNames[nProperty] == PROPERTYNAME_SHOWICONSINMENUES )
        {
            DBG_ASSERT(!(seqValues[nProperty].getValueTypeClass()!=TypeClass_BOOLEAN), "SvtMenuOptions_Impl::SvtMenuOptions_Impl()\nWho has changed the value type of \"Office.Common\\View\\Menu\\ShowIconsInMenues\"?" );
            bMenuSettingsChanged |= seqValues[nProperty] >>= bMenuIcons;
        }
        else if( seqPropertyNames[nProperty] == PROPERTYNAME_SYSTEMICONSINMENUES )
        {
            DBG_ASSERT(!(seqValues[nProperty].getValueTypeClass()!=TypeClass_BOOLEAN), "SvtMenuOptions_Impl::SvtMenuOptions_Impl()\nWho has changed the value type of \"Office.Common\\View\\Menu\\IsSystemIconsInMenus\"?" );
            bMenuSettingsChanged |= seqValues[nProperty] >>= bSystemMenuIcons;
        }
        else assert( false && "SvtMenuOptions_Impl::Notify()\nUnknown property detected ... I can't handle these!\n" );
    }

    if ( bMenuSettingsChanged )
        m_eMenuIcons = bSystemMenuIcons ? TRISTATE_INDET : static_cast<TriState>(bMenuIcons);

    for ( ::std::list<Link<LinkParamNone*,void>>::const_iterator iter = aList.begin(); iter != aList.end(); ++iter )
        iter->Call( nullptr );
}


//  public method

void SvtMenuOptions_Impl::ImplCommit()
{
    // Get names of supported properties, create a list for values and copy current values to it.
    Sequence< OUString >    seqNames    = impl_GetPropertyNames();
    sal_Int32               nCount      = seqNames.getLength();
    Sequence< Any >         seqValues   ( nCount );
    for( sal_Int32 nProperty=0; nProperty<nCount; ++nProperty )
    {
        switch( nProperty )
        {
            case PROPERTYHANDLE_DONTHIDEDISABLEDENTRIES :   {
                                                                seqValues[nProperty] <<= m_bDontHideDisabledEntries;
                                                            }
                                                            break;

            case PROPERTYHANDLE_FOLLOWMOUSE             :   {
                                                                seqValues[nProperty] <<= m_bFollowMouse;
                                                            }
                                                            break;
            //Output cache of current setting as possibly modified by System Theme for older version
            case PROPERTYHANDLE_SHOWICONSINMENUES       :   {
                                                                bool bValue = Application::GetSettings().GetStyleSettings().GetUseImagesInMenus();
                                                                seqValues[nProperty] <<= bValue;
                                                            }
                                                            break;
            case PROPERTYHANDLE_SYSTEMICONSINMENUES     :   {
                                                                bool bValue = m_eMenuIcons == TRISTATE_INDET;
                                                                seqValues[nProperty] <<= bValue;
                                                            }
                                                            break;
        }
    }
    // Set properties in configuration.
    PutProperties( seqNames, seqValues );
}


//  private method

Sequence< OUString > SvtMenuOptions_Impl::impl_GetPropertyNames()
{
    // Build static list of configuration key names.
    static const OUString pProperties[] =
    {
        OUString(PROPERTYNAME_DONTHIDEDISABLEDENTRIES)    ,
        OUString(PROPERTYNAME_FOLLOWMOUSE)                ,
        OUString(PROPERTYNAME_SHOWICONSINMENUES)          ,
        OUString(PROPERTYNAME_SYSTEMICONSINMENUES)
    };
    // Initialize return sequence with these list ...
    static const Sequence< OUString > seqPropertyNames( pProperties, PROPERTYCOUNT );
    // ... and return it.
    return seqPropertyNames;
}

void SvtMenuOptions_Impl::AddListenerLink( const Link<LinkParamNone*,void>& rLink )
{
    aList.push_back( rLink );
}

void SvtMenuOptions_Impl::RemoveListenerLink( const Link<LinkParamNone*,void>& rLink )
{
    aList.erase(std::remove(aList.begin(), aList.end(), rLink), aList.end());
}


//  initialize static member
//  DON'T DO IT IN YOUR HEADER!
//  see definition for further information

SvtMenuOptions_Impl*    SvtMenuOptions::m_pDataContainer    = nullptr  ;
sal_Int32               SvtMenuOptions::m_nRefCount         = 0     ;


//  constructor

SvtMenuOptions::SvtMenuOptions()
{
    // Global access, must be guarded (multithreading!).
    MutexGuard aGuard( GetOwnStaticMutex() );
    // Increase our refcount ...
    ++m_nRefCount;
    // ... and initialize our data container only if it not already!
    if( m_pDataContainer == nullptr )
    {
        m_pDataContainer = new SvtMenuOptions_Impl();

        svtools::ItemHolder2::holdConfigItem(E_MENUOPTIONS);
    }
}


//  destructor

SvtMenuOptions::~SvtMenuOptions()
{
    // Global access, must be guarded (multithreading!)
    MutexGuard aGuard( GetOwnStaticMutex() );
    // Decrease our refcount.
    --m_nRefCount;
    // If last instance was deleted ...
    // we must destroy our static data container!
    if( m_nRefCount <= 0 )
    {
        delete m_pDataContainer;
        m_pDataContainer = nullptr;
    }
}


//  public method

bool SvtMenuOptions::IsEntryHidingEnabled() const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pDataContainer->IsEntryHidingEnabled();
}


//  public method

TriState SvtMenuOptions::GetMenuIconsState() const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pDataContainer->GetMenuIconsState();
}


//  public method

void SvtMenuOptions::SetMenuIconsState(TriState eState)
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    m_pDataContainer->SetMenuIconsState(eState);
}


//  private method

Mutex& SvtMenuOptions::GetOwnStaticMutex()
{
    // Initialize static mutex only for one time!
    static Mutex* pMutex = nullptr;
    // If these method first called (Mutex not already exist!) ...
    if( pMutex == nullptr )
    {
        // ... we must create a new one. Protect follow code with the global mutex -
        // It must be - we create a static variable!
        MutexGuard aGuard( Mutex::getGlobalMutex() );
        // We must check our pointer again - because it can be that another instance of our class will be faster than these!
        if( pMutex == nullptr )
        {
            // Create the new mutex and set it for return on static variable.
            static Mutex aMutex;
            pMutex = &aMutex;
        }
    }
    // Return new created or already existing mutex object.
    return *pMutex;
}

void SvtMenuOptions::AddListenerLink( const Link<LinkParamNone*,void>& rLink )
{
    m_pDataContainer->AddListenerLink( rLink );
}

void SvtMenuOptions::RemoveListenerLink( const Link<LinkParamNone*,void>& rLink )
{
    m_pDataContainer->RemoveListenerLink( rLink );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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


#include <svtools/menuoptions.hxx>
#include <unotools/configmgr.hxx>
#include <unotools/configitem.hxx>
#include <tools/debug.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <vcl/svapp.hxx>

#include <rtl/logfile.hxx>
#include "itemholder2.hxx"

#include <list>

//_________________________________________________________________________________________________________________
//  namespaces
//_________________________________________________________________________________________________________________

using namespace ::utl                   ;
using namespace ::rtl                   ;
using namespace ::osl                   ;
using namespace ::com::sun::star::uno   ;

#define ROOTNODE_MENU                           OUString(RTL_CONSTASCII_USTRINGPARAM("Office.Common/View/Menu"  ))
#define DEFAULT_DONTHIDEDISABLEDENTRIES         sal_False
#define DEFAULT_FOLLOWMOUSE                     sal_True
#define DEFAULT_MENUICONS                       2

#define PROPERTYNAME_DONTHIDEDISABLEDENTRIES    OUString(RTL_CONSTASCII_USTRINGPARAM("DontHideDisabledEntry"    ))
#define PROPERTYNAME_FOLLOWMOUSE                OUString(RTL_CONSTASCII_USTRINGPARAM("FollowMouse"              ))
#define PROPERTYNAME_SHOWICONSINMENUES          OUString(RTL_CONSTASCII_USTRINGPARAM("ShowIconsInMenues"        ))
#define PROPERTYNAME_SYSTEMICONSINMENUES        OUString(RTL_CONSTASCII_USTRINGPARAM("IsSystemIconsInMenus"     ))

#define PROPERTYHANDLE_DONTHIDEDISABLEDENTRIES  0
#define PROPERTYHANDLE_FOLLOWMOUSE              1
#define PROPERTYHANDLE_SHOWICONSINMENUES        2
#define PROPERTYHANDLE_SYSTEMICONSINMENUES      3

#define PROPERTYCOUNT                           4

#include <tools/link.hxx>

//_________________________________________________________________________________________________________________
//  private declarations!
//_________________________________________________________________________________________________________________

class SvtMenuOptions_Impl : public ConfigItem
{
    //-------------------------------------------------------------------------------------------------------------
    //  private member
    //-------------------------------------------------------------------------------------------------------------

    private:
        ::std::list<Link> aList;
        sal_Bool    m_bDontHideDisabledEntries          ;   /// cache "DontHideDisabledEntries" of Menu section
        sal_Bool    m_bFollowMouse                      ;   /// cache "FollowMouse" of Menu section
        sal_Int16   m_nMenuIcons                        ;   /// cache "MenuIcons" of Menu section

    //-------------------------------------------------------------------------------------------------------------
    //  public methods
    //-------------------------------------------------------------------------------------------------------------

    public:

        //---------------------------------------------------------------------------------------------------------
        //  constructor / destructor
        //---------------------------------------------------------------------------------------------------------

         SvtMenuOptions_Impl();
        ~SvtMenuOptions_Impl();

        void AddListenerLink( const Link& rLink );
        void RemoveListenerLink( const Link& rLink );

        //---------------------------------------------------------------------------------------------------------
        //  overloaded methods of baseclass
        //---------------------------------------------------------------------------------------------------------

        /*-****************************************************************************************************//**
            @short      called for notify of configmanager
            @descr      These method is called from the ConfigManager before application ends or from the
                         PropertyChangeListener if the sub tree broadcasts changes. You must update your
                        internal values.

            @seealso    baseclass ConfigItem

            @param      "seqPropertyNames" is the list of properties which should be updated.
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void Notify( const Sequence< OUString >& seqPropertyNames );

        /*-****************************************************************************************************//**
            @short      write changes to configuration
            @descr      These method writes the changed values into the sub tree
                        and should always called in our destructor to guarantee consistency of config data.

            @seealso    baseclass ConfigItem

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        virtual void Commit();

        //---------------------------------------------------------------------------------------------------------
        //  public interface
        //---------------------------------------------------------------------------------------------------------

        /*-****************************************************************************************************//**
            @short      access method to get internal values
            @descr      These methods give us a chance to regulate access to our internal values.
                        It's not used in the moment - but it's possible for the future!

            @seealso    -

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        sal_Bool    IsEntryHidingEnabled() const
                    { return m_bDontHideDisabledEntries; }

        sal_Bool    IsFollowMouseEnabled() const
                    { return m_bFollowMouse; }

        sal_Int16   GetMenuIconsState() const
                    { return m_nMenuIcons; }

        void        SetEntryHidingState ( sal_Bool bState )
                    {
                        m_bDontHideDisabledEntries = bState;
                        SetModified();
                        for ( ::std::list<Link>::const_iterator iter = aList.begin(); iter != aList.end(); ++iter )
                            iter->Call( this );
                        Commit();
                    }

        void        SetFollowMouseState ( sal_Bool bState )
                    {
                        m_bFollowMouse = bState;
                        SetModified();
                        for ( ::std::list<Link>::const_iterator iter = aList.begin(); iter != aList.end(); ++iter )
                            iter->Call( this );
                        Commit();
                    }

        void        SetMenuIconsState ( sal_Int16 nState    )
                    {
                        m_nMenuIcons = nState;
                        SetModified();
                        for ( ::std::list<Link>::const_iterator iter = aList.begin(); iter != aList.end(); ++iter )
                            iter->Call( this );
                        Commit();
                    }

    //-------------------------------------------------------------------------------------------------------------
    //  private methods
    //-------------------------------------------------------------------------------------------------------------

    private:

        /*-****************************************************************************************************//**
            @short      return list of fix key names of our configuration management which represent our module tree
            @descr      These methods return a static const list of key names. We need it to get needed values from our
                        configuration management.

            @seealso    -

            @param      -
            @return     A list of needed configuration keys is returned.

            @onerror    -
        *//*-*****************************************************************************************************/

        static Sequence< OUString > impl_GetPropertyNames();
};

//*****************************************************************************************************************
//  constructor
//*****************************************************************************************************************
SvtMenuOptions_Impl::SvtMenuOptions_Impl()
    // Init baseclasses first
    :   ConfigItem                  ( ROOTNODE_MENU                     )
    // Init member then.
    ,   m_bDontHideDisabledEntries  ( DEFAULT_DONTHIDEDISABLEDENTRIES   )
    ,   m_bFollowMouse              ( DEFAULT_FOLLOWMOUSE               )
    ,   m_nMenuIcons                ( DEFAULT_MENUICONS                 )
{
    // Use our static list of configuration keys to get his values.
    Sequence< OUString >    seqNames    = impl_GetPropertyNames();
    Sequence< Any >         seqValues   = GetProperties( seqNames ) ;

    // Safe impossible cases.
    // We need values from ALL configuration keys.
    // Follow assignment use order of values in relation to our list of key names!
    DBG_ASSERT( !(seqNames.getLength()!=seqValues.getLength()), "SvtMenuOptions_Impl::SvtMenuOptions_Impl()\nI miss some values of configuration keys!\n" );

    sal_Bool bMenuIcons = sal_True;
    sal_Bool bSystemMenuIcons = sal_True;
    if (m_nMenuIcons == 2)
        bMenuIcons = (sal_Bool)(Application::GetSettings().GetStyleSettings().GetPreferredUseImagesInMenus());
    else
    {
        bSystemMenuIcons = sal_False;
        bMenuIcons = m_nMenuIcons ? sal_True : sal_False;
    }

    // Copy values from list in right order to our internal member.
    sal_Int32 nPropertyCount    =   seqValues.getLength()   ;
    sal_Int32 nProperty         =   0                       ;
    for( nProperty=0; nProperty<nPropertyCount; ++nProperty )
    {
        // Safe impossible cases.
        // Check any for valid value.
        DBG_ASSERT( !(seqValues[nProperty].hasValue()==sal_False), "SvtMenuOptions_Impl::SvtMenuOptions_Impl()\nInvalid property value for property detected!\n" );

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

    m_nMenuIcons = bSystemMenuIcons ? 2 : bMenuIcons;

    EnableNotification( seqNames );
}

//*****************************************************************************************************************
//  destructor
//*****************************************************************************************************************
SvtMenuOptions_Impl::~SvtMenuOptions_Impl()
{
    // Flush data to configuration!
    // User has no chance to do that.
    if( IsModified() == sal_True )
    {
        Commit();
    }
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
void SvtMenuOptions_Impl::Notify( const Sequence< OUString >& seqPropertyNames )
{
    // Use given list of updated properties to get his values from configuration directly!
    Sequence< Any > seqValues = GetProperties( seqPropertyNames );
    // Safe impossible cases.
    // We need values from ALL notified configuration keys.
    DBG_ASSERT( !(seqPropertyNames.getLength()!=seqValues.getLength()), "SvtMenuOptions_Impl::Notify()\nI miss some values of configuration keys!\n" );

    sal_Bool bMenuSettingsChanged = sal_False;
    sal_Bool bMenuIcons = sal_True;
    sal_Bool bSystemMenuIcons = sal_True;
    if (m_nMenuIcons == 2)
        bMenuIcons = (sal_Bool)(Application::GetSettings().GetStyleSettings().GetUseImagesInMenus());
    else
    {
        bSystemMenuIcons = sal_False;
        bMenuIcons = m_nMenuIcons ? sal_True : sal_False;
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

        #if OSL_DEBUG_LEVEL > 1
        else DBG_ASSERT( sal_False, "SvtMenuOptions_Impl::Notify()\nUnkown property detected ... I can't handle these!\n" );
        #endif
    }

    if ( bMenuSettingsChanged )
        m_nMenuIcons = bSystemMenuIcons ? 2 : bMenuIcons;

    for ( ::std::list<Link>::const_iterator iter = aList.begin(); iter != aList.end(); ++iter )
        iter->Call( this );
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
void SvtMenuOptions_Impl::Commit()
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
                                                                sal_Bool bValue = (sal_Bool)(Application::GetSettings().GetStyleSettings().GetUseImagesInMenus());
                                                                seqValues[nProperty] <<= bValue;
                                                            }
                                                            break;
            case PROPERTYHANDLE_SYSTEMICONSINMENUES     :   {
                                                                sal_Bool bValue = (m_nMenuIcons == 2 ? sal_True : sal_False) ;
                                                                seqValues[nProperty] <<= bValue;
                                                            }
                                                            break;
        }
    }
    // Set properties in configuration.
    PutProperties( seqNames, seqValues );
}

//*****************************************************************************************************************
//  private method
//*****************************************************************************************************************
Sequence< OUString > SvtMenuOptions_Impl::impl_GetPropertyNames()
{
    // Build static list of configuration key names.
    static const OUString pProperties[] =
    {
        PROPERTYNAME_DONTHIDEDISABLEDENTRIES    ,
        PROPERTYNAME_FOLLOWMOUSE                ,
        PROPERTYNAME_SHOWICONSINMENUES          ,
        PROPERTYNAME_SYSTEMICONSINMENUES
    };
    // Initialize return sequence with these list ...
    static const Sequence< OUString > seqPropertyNames( pProperties, PROPERTYCOUNT );
    // ... and return it.
    return seqPropertyNames;
}

void SvtMenuOptions_Impl::AddListenerLink( const Link& rLink )
{
    aList.push_back( rLink );
}

void SvtMenuOptions_Impl::RemoveListenerLink( const Link& rLink )
{
    for ( ::std::list<Link>::iterator iter = aList.begin(); iter != aList.end(); ++iter )
    {
        if ( *iter == rLink )
        {
            aList.erase(iter);
            break;
        }
    }
}

//*****************************************************************************************************************
//  initialize static member
//  DON'T DO IT IN YOUR HEADER!
//  see definition for further informations
//*****************************************************************************************************************
SvtMenuOptions_Impl*    SvtMenuOptions::m_pDataContainer    = NULL  ;
sal_Int32               SvtMenuOptions::m_nRefCount         = 0     ;

//*****************************************************************************************************************
//  constructor
//*****************************************************************************************************************
SvtMenuOptions::SvtMenuOptions()
{
    // Global access, must be guarded (multithreading!).
    MutexGuard aGuard( GetOwnStaticMutex() );
    // Increase our refcount ...
    ++m_nRefCount;
    // ... and initialize our data container only if it not already!
    if( m_pDataContainer == NULL )
    {
        RTL_LOGFILE_CONTEXT(aLog, "svtools ( ??? ) ::SvtMenuOptions_Impl::ctor()");
        m_pDataContainer = new SvtMenuOptions_Impl();

        svtools::ItemHolder2::holdConfigItem(E_MENUOPTIONS);
    }
}

//*****************************************************************************************************************
//  destructor
//*****************************************************************************************************************
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
        m_pDataContainer = NULL;
    }
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
sal_Bool SvtMenuOptions::IsEntryHidingEnabled() const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pDataContainer->IsEntryHidingEnabled();
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
sal_Int16 SvtMenuOptions::GetMenuIconsState() const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pDataContainer->GetMenuIconsState();
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
void SvtMenuOptions::SetMenuIconsState( sal_Int16 bState )
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    m_pDataContainer->SetMenuIconsState( bState );
}

//*****************************************************************************************************************
//  private method
//*****************************************************************************************************************
Mutex& SvtMenuOptions::GetOwnStaticMutex()
{
    // Initialize static mutex only for one time!
    static Mutex* pMutex = NULL;
    // If these method first called (Mutex not already exist!) ...
    if( pMutex == NULL )
    {
        // ... we must create a new one. Protect follow code with the global mutex -
        // It must be - we create a static variable!
        MutexGuard aGuard( Mutex::getGlobalMutex() );
        // We must check our pointer again - because it can be that another instance of our class will be faster than these!
        if( pMutex == NULL )
        {
            // Create the new mutex and set it for return on static variable.
            static Mutex aMutex;
            pMutex = &aMutex;
        }
    }
    // Return new created or already existing mutex object.
    return *pMutex;
}

void SvtMenuOptions::AddListenerLink( const Link& rLink )
{
    m_pDataContainer->AddListenerLink( rLink );
}

void SvtMenuOptions::RemoveListenerLink( const Link& rLink )
{
    m_pDataContainer->RemoveListenerLink( rLink );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

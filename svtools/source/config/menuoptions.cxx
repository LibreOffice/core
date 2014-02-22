/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
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





using namespace ::utl                   ;
using namespace ::rtl                   ;
using namespace ::osl                   ;
using namespace ::com::sun::star::uno   ;

#define ROOTNODE_MENU                           OUString("Office.Common/View/Menu"  )
#define DEFAULT_DONTHIDEDISABLEDENTRIES         sal_False
#define DEFAULT_FOLLOWMOUSE                     sal_True
#define DEFAULT_MENUICONS                       2

#define PROPERTYNAME_DONTHIDEDISABLEDENTRIES    OUString("DontHideDisabledEntry"    )
#define PROPERTYNAME_FOLLOWMOUSE                OUString("FollowMouse"              )
#define PROPERTYNAME_SHOWICONSINMENUES          OUString("ShowIconsInMenues"        )
#define PROPERTYNAME_SYSTEMICONSINMENUES        OUString("IsSystemIconsInMenus"     )

#define PROPERTYHANDLE_DONTHIDEDISABLEDENTRIES  0
#define PROPERTYHANDLE_FOLLOWMOUSE              1
#define PROPERTYHANDLE_SHOWICONSINMENUES        2
#define PROPERTYHANDLE_SYSTEMICONSINMENUES      3

#define PROPERTYCOUNT                           4

#include <tools/link.hxx>





class SvtMenuOptions_Impl : public ConfigItem
{
    
    
    

    private:
        ::std::list<Link> aList;
        sal_Bool    m_bDontHideDisabledEntries          ;   
        sal_Bool    m_bFollowMouse                      ;   
        sal_Int16   m_nMenuIcons                        ;   

    
    
    

    public:

        
        
        

         SvtMenuOptions_Impl();
        ~SvtMenuOptions_Impl();

        void AddListenerLink( const Link& rLink );
        void RemoveListenerLink( const Link& rLink );

        
        
        

        /*-****************************************************************************************************
            @short      called for notify of configmanager
            @descr      These method is called from the ConfigManager before application ends or from the
                         PropertyChangeListener if the sub tree broadcasts changes. You must update your
                        internal values.

            @seealso    baseclass ConfigItem

            @param      "seqPropertyNames" is the list of properties which should be updated.
            @return     -

            @onerror    -
        *

        virtual void Notify( const Sequence< OUString >& seqPropertyNames );

        /*-****************************************************************************************************
            @short      write changes to configuration
            @descr      These method writes the changed values into the sub tree
                        and should always called in our destructor to guarantee consistency of config data.

            @seealso    baseclass ConfigItem

            @param      -
            @return     -

            @onerror    -
        *

        virtual void Commit();

        
        
        

        /*-****************************************************************************************************
            @short      access method to get internal values
            @descr      These methods give us a chance to regulate access to our internal values.
                        It's not used in the moment - but it's possible for the future!

            @seealso    -

            @param      -
            @return     -

            @onerror    -
        *

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

    
    
    

    private:

        /*-****************************************************************************************************
            @short      return list of fix key names of our configuration management which represent our module tree
            @descr      These methods return a static const list of key names. We need it to get needed values from our
                        configuration management.

            @seealso    -

            @param      -
            @return     A list of needed configuration keys is returned.

            @onerror    -
        *

        static Sequence< OUString > impl_GetPropertyNames();
};




SvtMenuOptions_Impl::SvtMenuOptions_Impl()
    
    :   ConfigItem                  ( ROOTNODE_MENU                     )
    
    ,   m_bDontHideDisabledEntries  ( DEFAULT_DONTHIDEDISABLEDENTRIES   )
    ,   m_bFollowMouse              ( DEFAULT_FOLLOWMOUSE               )
    ,   m_nMenuIcons                ( DEFAULT_MENUICONS                 )
{
    
    Sequence< OUString >    seqNames    = impl_GetPropertyNames();
    Sequence< Any >         seqValues   = GetProperties( seqNames ) ;

    
    
    
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

    
    sal_Int32 nPropertyCount    =   seqValues.getLength()   ;
    sal_Int32 nProperty         =   0                       ;
    for( nProperty=0; nProperty<nPropertyCount; ++nProperty )
    {
        
        
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

    m_nMenuIcons = bSystemMenuIcons ? 2 : bMenuIcons;

    EnableNotification( seqNames );
}




SvtMenuOptions_Impl::~SvtMenuOptions_Impl()
{
    
    
    if( IsModified() )
    {
        Commit();
    }
}




void SvtMenuOptions_Impl::Notify( const Sequence< OUString >& seqPropertyNames )
{
    
    Sequence< Any > seqValues = GetProperties( seqPropertyNames );
    
    
    DBG_ASSERT( !(seqPropertyNames.getLength()!=seqValues.getLength()), "SvtMenuOptions_Impl::Notify()\nI miss some values of configuration keys!\n" );

    bool bMenuSettingsChanged = false;
    sal_Bool bMenuIcons = sal_True;
    sal_Bool bSystemMenuIcons = sal_True;
    if (m_nMenuIcons == 2)
        bMenuIcons = (sal_Bool)(Application::GetSettings().GetStyleSettings().GetUseImagesInMenus());
    else
    {
        bSystemMenuIcons = sal_False;
        bMenuIcons = m_nMenuIcons ? sal_True : sal_False;
    }

    
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
        else DBG_ASSERT( sal_False, "SvtMenuOptions_Impl::Notify()\nUnknown property detected ... I can't handle these!\n" );
        #endif
    }

    if ( bMenuSettingsChanged )
        m_nMenuIcons = bSystemMenuIcons ? 2 : bMenuIcons;

    for ( ::std::list<Link>::const_iterator iter = aList.begin(); iter != aList.end(); ++iter )
        iter->Call( this );
}




void SvtMenuOptions_Impl::Commit()
{
    
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
    
    PutProperties( seqNames, seqValues );
}




Sequence< OUString > SvtMenuOptions_Impl::impl_GetPropertyNames()
{
    
    static const OUString pProperties[] =
    {
        PROPERTYNAME_DONTHIDEDISABLEDENTRIES    ,
        PROPERTYNAME_FOLLOWMOUSE                ,
        PROPERTYNAME_SHOWICONSINMENUES          ,
        PROPERTYNAME_SYSTEMICONSINMENUES
    };
    
    static const Sequence< OUString > seqPropertyNames( pProperties, PROPERTYCOUNT );
    
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






SvtMenuOptions_Impl*    SvtMenuOptions::m_pDataContainer    = NULL  ;
sal_Int32               SvtMenuOptions::m_nRefCount         = 0     ;




SvtMenuOptions::SvtMenuOptions()
{
    
    MutexGuard aGuard( GetOwnStaticMutex() );
    
    ++m_nRefCount;
    
    if( m_pDataContainer == NULL )
    {
        m_pDataContainer = new SvtMenuOptions_Impl();

        svtools::ItemHolder2::holdConfigItem(E_MENUOPTIONS);
    }
}




SvtMenuOptions::~SvtMenuOptions()
{
    
    MutexGuard aGuard( GetOwnStaticMutex() );
    
    --m_nRefCount;
    
    
    if( m_nRefCount <= 0 )
    {
        delete m_pDataContainer;
        m_pDataContainer = NULL;
    }
}




sal_Bool SvtMenuOptions::IsEntryHidingEnabled() const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pDataContainer->IsEntryHidingEnabled();
}




sal_Int16 SvtMenuOptions::GetMenuIconsState() const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pDataContainer->GetMenuIconsState();
}




void SvtMenuOptions::SetMenuIconsState( sal_Int16 bState )
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    m_pDataContainer->SetMenuIconsState( bState );
}




Mutex& SvtMenuOptions::GetOwnStaticMutex()
{
    
    static Mutex* pMutex = NULL;
    
    if( pMutex == NULL )
    {
        
        
        MutexGuard aGuard( Mutex::getGlobalMutex() );
        
        if( pMutex == NULL )
        {
            
            static Mutex aMutex;
            pMutex = &aMutex;
        }
    }
    
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

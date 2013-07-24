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


#include <unotools/localisationoptions.hxx>
#include <unotools/configmgr.hxx>
#include <unotools/configitem.hxx>
#include <tools/debug.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>

#include "itemholder1.hxx"

using namespace ::utl                   ;
using namespace ::rtl                   ;
using namespace ::osl                   ;
using namespace ::com::sun::star::uno   ;

#define ROOTNODE_LOCALISATION           OUString("Office.Common/View/Localisation")
#define DEFAULT_AUTOMNEMONIC            sal_False
#define DEFAULT_DIALOGSCALE             0

#define PROPERTYNAME_AUTOMNEMONIC       OUString("AutoMnemonic")
#define PROPERTYNAME_DIALOGSCALE        OUString("DialogScale")

#define PROPERTYHANDLE_AUTOMNEMONIC     0
#define PROPERTYHANDLE_DIALOGSCALE      1

#define PROPERTYCOUNT                   2

class SvtLocalisationOptions_Impl : public ConfigItem
{
    public:

         SvtLocalisationOptions_Impl();
        ~SvtLocalisationOptions_Impl();

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

        /*-****************************************************************************************************//**
            @short      access method to get internal values
            @descr      These method give us a chance to regulate acces to ouer internal values.
                        It's not used in the moment - but it's possible for the feature!

            @seealso    -

            @param      -
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/

        sal_Bool    IsAutoMnemonic  (                   ) const ;
        sal_Int32   GetDialogScale  (                   ) const ;

    private:

        /*-****************************************************************************************************//**
            @short      return list of key names of ouer configuration management which represent oue module tree
            @descr      These methods return a static const list of key names. We need it to get needed values from our
                        configuration management.

            @seealso    -

            @param      -
            @return     A list of needed configuration keys is returned.

            @onerror    -
        *//*-*****************************************************************************************************/

        static Sequence< OUString > GetPropertyNames();

    private:

        sal_Bool    m_bAutoMnemonic     ;
        sal_Int32   m_nDialogScale      ;
};

//*****************************************************************************************************************
//  constructor
//*****************************************************************************************************************
SvtLocalisationOptions_Impl::SvtLocalisationOptions_Impl()
    // Init baseclasses first
    :   ConfigItem          ( ROOTNODE_LOCALISATION )
    // Init member then.
    ,   m_bAutoMnemonic     ( DEFAULT_AUTOMNEMONIC  )
    ,   m_nDialogScale      ( DEFAULT_DIALOGSCALE   )
{
    // Use our static list of configuration keys to get his values.
    Sequence< OUString >    seqNames    = GetPropertyNames  (           );
    Sequence< Any >         seqValues   = GetProperties     ( seqNames  );

    // Safe impossible cases.
    // We need values from ALL configuration keys.
    // Follow assignment use order of values in relation to our list of key names!
    DBG_ASSERT( !(seqNames.getLength()!=seqValues.getLength()), "SvtLocalisationOptions_Impl::SvtLocalisationOptions_Impl()\nI miss some values of configuration keys!\n" );

    // Copy values from list in right order to our internal member.
    sal_Int32 nPropertyCount = seqValues.getLength();
    for( sal_Int32 nProperty=0; nProperty<nPropertyCount; ++nProperty )
    {
        if (!seqValues[nProperty].hasValue())
            continue;
        switch( nProperty )
        {
            case PROPERTYHANDLE_AUTOMNEMONIC    :   {
                                                        DBG_ASSERT(!(seqValues[nProperty].getValueTypeClass()!=TypeClass_BOOLEAN), "SvtLocalisationOptions_Impl::SvtLocalisationOptions_Impl()\nWho has changed the value type of \"Office.Common\\View\\Localisation\\AutoMnemonic\"?" );
                                                        seqValues[nProperty] >>= m_bAutoMnemonic;
                                                    }
                                                    break;

            case PROPERTYHANDLE_DIALOGSCALE     :   {
                                                        DBG_ASSERT(!(seqValues[nProperty].getValueTypeClass()!=TypeClass_LONG), "SvtLocalisationOptions_Impl::SvtLocalisationOptions_Impl()\nWho has changed the value type of \"Office.Common\\View\\Localisation\\DialogScale\"?" );
                                                        seqValues[nProperty] >>= m_nDialogScale;
                                                    }
                                                    break;
        }
    }

    // Enable notification mechanism of ouer baseclass.
    // We need it to get information about changes outside these class on ouer used configuration keys!
    EnableNotification( seqNames );
}

//*****************************************************************************************************************
//  destructor
//*****************************************************************************************************************
SvtLocalisationOptions_Impl::~SvtLocalisationOptions_Impl()
{
    // We must save our current values .. if user forget it!
    if( IsModified() == sal_True )
    {
        Commit();
    }
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
void SvtLocalisationOptions_Impl::Notify( const Sequence< OUString >& seqPropertyNames )
{
    // Use given list of updated properties to get his values from configuration directly!
    Sequence< Any > seqValues = GetProperties( seqPropertyNames );
    // Safe impossible cases.
    // We need values from ALL notified configuration keys.
    DBG_ASSERT( !(seqPropertyNames.getLength()!=seqValues.getLength()), "SvtLocalisationOptions_Impl::Notify()\nI miss some values of configuration keys!\n" );
    // Step over list of property names and get right value from coreesponding value list to set it on internal members!
    sal_Int32 nCount = seqPropertyNames.getLength();
    for( sal_Int32 nProperty=0; nProperty<nCount; ++nProperty )
    {
        if( seqPropertyNames[nProperty] == PROPERTYNAME_AUTOMNEMONIC )
        {
            DBG_ASSERT(!(seqValues[nProperty].getValueTypeClass()!=TypeClass_BOOLEAN), "SvtLocalisationOptions_Impl::SvtLocalisationOptions_Impl()\nWho has changed the value type of \"Office.Common\\View\\Localisation\\AutoMnemonic\"?" );
            seqValues[nProperty] >>= m_bAutoMnemonic;
        }
        else
        if( seqPropertyNames[nProperty] == PROPERTYNAME_DIALOGSCALE )
        {
            DBG_ASSERT(!(seqValues[nProperty].getValueTypeClass()!=TypeClass_LONG), "SvtLocalisationOptions_Impl::SvtLocalisationOptions_Impl()\nWho has changed the value type of \"Office.Common\\View\\Localisation\\DialogScale\"?" );
            seqValues[nProperty] >>= m_nDialogScale;
        }
        #if OSL_DEBUG_LEVEL > 1
        else DBG_ASSERT( sal_False, "SvtLocalisationOptions_Impl::Notify()\nUnknown property detected ... I can't handle these!\n" );
        #endif
    }

    NotifyListeners(0);
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
void SvtLocalisationOptions_Impl::Commit()
{
    // Get names of supported properties, create a list for values and copy current values to it.
    Sequence< OUString >    seqNames    = GetPropertyNames  ();
    sal_Int32               nCount      = seqNames.getLength();
    Sequence< Any >         seqValues   ( nCount );
    for( sal_Int32 nProperty=0; nProperty<nCount; ++nProperty )
    {
        switch( nProperty )
        {
            case PROPERTYHANDLE_AUTOMNEMONIC    :   {
                                                        seqValues[nProperty] <<= m_bAutoMnemonic;
                                                    }
                                                    break;

            case PROPERTYHANDLE_DIALOGSCALE     :   {
                                                        seqValues[nProperty] <<= m_nDialogScale;
                                                    }
                                                    break;
        }
    }
    // Set properties in configuration.
    PutProperties( seqNames, seqValues );
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
sal_Bool SvtLocalisationOptions_Impl::IsAutoMnemonic() const
{
    return m_bAutoMnemonic;
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
sal_Int32 SvtLocalisationOptions_Impl::GetDialogScale() const
{
    return m_nDialogScale;
}

Sequence< OUString > SvtLocalisationOptions_Impl::GetPropertyNames()
{
    // Build static list of configuration key names.
    const OUString aProperties[] =
    {
        PROPERTYNAME_AUTOMNEMONIC   ,
        PROPERTYNAME_DIALOGSCALE    ,
    };
    // Initialize return sequence with these list ...
    Sequence< OUString > seqPropertyNames(aProperties, PROPERTYCOUNT);
    // ... and return it.
    return seqPropertyNames;
}

//*****************************************************************************************************************
//  initialize static member
//  DON'T DO IT IN YOUR HEADER!
//  see definition for further information
//*****************************************************************************************************************
SvtLocalisationOptions_Impl*    SvtLocalisationOptions::m_pDataContainer    = NULL  ;
sal_Int32                       SvtLocalisationOptions::m_nRefCount         = 0     ;

//*****************************************************************************************************************
//  constructor
//*****************************************************************************************************************
SvtLocalisationOptions::SvtLocalisationOptions()
{
    // Global access, must be guarded (multithreading!).
    MutexGuard aGuard( GetOwnStaticMutex() );
    // Increase ouer refcount ...
    ++m_nRefCount;
    // ... and initialize ouer data container only if it not already exist!
    if( m_pDataContainer == NULL )
    {
        m_pDataContainer = new SvtLocalisationOptions_Impl;

        ItemHolder1::holdConfigItem(E_LOCALISATIONOPTIONS);
    }
}

//*****************************************************************************************************************
//  destructor
//*****************************************************************************************************************
SvtLocalisationOptions::~SvtLocalisationOptions()
{
    // Global access, must be guarded (multithreading!)
    MutexGuard aGuard( GetOwnStaticMutex() );
    // Decrease ouer refcount.
    --m_nRefCount;
    // If last instance was deleted ...
    // we must destroy ouer static data container!
    if( m_nRefCount <= 0 )
    {
        delete m_pDataContainer;
        m_pDataContainer = NULL;
    }
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
sal_Bool SvtLocalisationOptions::IsAutoMnemonic() const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pDataContainer->IsAutoMnemonic();
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
sal_Int32 SvtLocalisationOptions::GetDialogScale() const
{
    MutexGuard aGuard( GetOwnStaticMutex() );
    return m_pDataContainer->GetDialogScale();
}

namespace
{
    class theLocalisationOptionsMutex : public rtl::Static<osl::Mutex, theLocalisationOptionsMutex>{};
}

//*****************************************************************************************************************
//  private method
//*****************************************************************************************************************
Mutex& SvtLocalisationOptions::GetOwnStaticMutex()
{
    return theLocalisationOptionsMutex::get();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

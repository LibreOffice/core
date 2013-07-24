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


#include <unotools/fontoptions.hxx>
#include <unotools/configmgr.hxx>
#include <unotools/configitem.hxx>
#include <tools/debug.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>

#include <rtl/instance.hxx>
#include "itemholder1.hxx"

using namespace ::utl                   ;
using namespace ::rtl                   ;
using namespace ::osl                   ;
using namespace ::com::sun::star::uno   ;

#define ROOTNODE_FONT                       OUString("Office.Common/Font")

#define PROPERTYNAME_REPLACEMENTTABLE       OUString("Substitution/Replacement")
#define PROPERTYNAME_FONTHISTORY            OUString("View/History")
#define PROPERTYNAME_FONTWYSIWYG            OUString("View/ShowFontBoxWYSIWYG")

#define PROPERTYHANDLE_REPLACEMENTTABLE     0
#define PROPERTYHANDLE_FONTHISTORY          1
#define PROPERTYHANDLE_FONTWYSIWYG          2

#define PROPERTYCOUNT                       3

class SvtFontOptions_Impl : public ConfigItem
{
    public:

         SvtFontOptions_Impl();
        ~SvtFontOptions_Impl();

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

        sal_Bool    IsFontHistoryEnabled        (                   ) const ;
        void        EnableFontHistory           ( sal_Bool bState   )       ;

        sal_Bool    IsFontWYSIWYGEnabled        (                   ) const ;
        void        EnableFontWYSIWYG           ( sal_Bool bState   )       ;

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

        static Sequence< OUString > impl_GetPropertyNames();

    private:

        sal_Bool        m_bReplacementTable     ;
        sal_Bool        m_bFontHistory          ;
        sal_Bool        m_bFontWYSIWYG          ;
};

//*****************************************************************************************************************
//  constructor
//*****************************************************************************************************************
SvtFontOptions_Impl::SvtFontOptions_Impl()
    // Init baseclasses first
    :   ConfigItem          ( ROOTNODE_FONT )
    // Init member then.
    ,   m_bReplacementTable ( sal_False     )
    ,   m_bFontHistory      ( sal_False     )
    ,   m_bFontWYSIWYG      ( sal_False     )
{
    // Use our static list of configuration keys to get his values.
    Sequence< OUString >    seqNames    = impl_GetPropertyNames (           );
    Sequence< Any >         seqValues   = GetProperties         ( seqNames  );

    // Safe impossible cases.
    // We need values from ALL configuration keys.
    // Follow assignment use order of values in relation to our list of key names!
    DBG_ASSERT( !(seqNames.getLength()!=seqValues.getLength()), "SvtFontOptions_Impl::SvtFontOptions_Impl()\nI miss some values of configuration keys!\n" );

    // Copy values from list in right order to ouer internal member.
    sal_Int32 nPropertyCount = seqValues.getLength();
    for( sal_Int32 nProperty=0; nProperty<nPropertyCount; ++nProperty )
    {
        // Safe impossible cases.
        // Check any for valid value.
        DBG_ASSERT( !(seqValues[nProperty].hasValue()==sal_False), "SvtFontOptions_Impl::SvtFontOptions_Impl()\nInvalid property value detected!\n" );
        switch( nProperty )
        {
            case PROPERTYHANDLE_REPLACEMENTTABLE    :   {
                                                            DBG_ASSERT(!(seqValues[nProperty].getValueTypeClass()!=TypeClass_BOOLEAN), "SvtFontOptions_Impl::SvtFontOptions_Impl()\nWho has changed the value type of \"Office.Common\\Font\\Substitution\\Replacement\"?" );
                                                            seqValues[nProperty] >>= m_bReplacementTable;
                                                        }
                                                        break;
            case PROPERTYHANDLE_FONTHISTORY         :   {
                                                            DBG_ASSERT(!(seqValues[nProperty].getValueTypeClass()!=TypeClass_BOOLEAN), "SvtFontOptions_Impl::SvtFontOptions_Impl()\nWho has changed the value type of \"Office.Common\\Font\\View\\History\"?" );
                                                            seqValues[nProperty] >>= m_bFontHistory;
                                                        }
                                                        break;
            case PROPERTYHANDLE_FONTWYSIWYG         :   {
                                                            DBG_ASSERT(!(seqValues[nProperty].getValueTypeClass()!=TypeClass_BOOLEAN), "SvtFontOptions_Impl::SvtFontOptions_Impl()\nWho has changed the value type of \"Office.Common\\Font\\View\\ShowFontBoxWYSIWYG\"?" );
                                                            seqValues[nProperty] >>= m_bFontWYSIWYG;
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
SvtFontOptions_Impl::~SvtFontOptions_Impl()
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
void SvtFontOptions_Impl::Notify( const Sequence< OUString >& seqPropertyNames )
{
    // Use given list of updated properties to get his values from configuration directly!
    Sequence< Any > seqValues = GetProperties( seqPropertyNames );
    // Safe impossible cases.
    // We need values from ALL notified configuration keys.
    DBG_ASSERT( !(seqPropertyNames.getLength()!=seqValues.getLength()), "SvtFontOptions_Impl::Notify()\nI miss some values of configuration keys!\n" );
    // Step over list of property names and get right value from coreesponding value list to set it on internal members!
    sal_Int32 nCount = seqPropertyNames.getLength();
    for( sal_Int32 nProperty=0; nProperty<nCount; ++nProperty )
    {
        if( seqPropertyNames[nProperty] == PROPERTYNAME_REPLACEMENTTABLE )
        {
            DBG_ASSERT(!(seqValues[nProperty].getValueTypeClass()!=TypeClass_BOOLEAN), "SvtFontOptions_Impl::Notify()\nWho has changed the value type of \"Office.Common\\Font\\Substitution\\Replacement\"?" );
            seqValues[nProperty] >>= m_bReplacementTable;
        }
        else
        if( seqPropertyNames[nProperty] == PROPERTYNAME_FONTHISTORY )
        {
            DBG_ASSERT(!(seqValues[nProperty].getValueTypeClass()!=TypeClass_BOOLEAN), "SvtFontOptions_Impl::Notify()\nWho has changed the value type of \"Office.Common\\Font\\View\\History\"?" );
            seqValues[nProperty] >>= m_bFontHistory;
        }
        else
        if( seqPropertyNames[nProperty] == PROPERTYNAME_FONTWYSIWYG )
        {
            DBG_ASSERT(!(seqValues[nProperty].getValueTypeClass()!=TypeClass_BOOLEAN), "SvtFontOptions_Impl::Notify()\nWho has changed the value type of \"Office.Common\\Font\\View\\ShowFontBoxWYSIWYG\"?" );
            seqValues[nProperty] >>= m_bFontWYSIWYG;
        }
        #if OSL_DEBUG_LEVEL > 1
        else DBG_ASSERT( sal_False, "SvtFontOptions_Impl::Notify()\nUnknown property detected ... I can't handle these!\n" );
        #endif
    }
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
void SvtFontOptions_Impl::Commit()
{
    // Get names of supported properties, create a list for values and copy current values to it.
    Sequence< OUString >    seqNames    = impl_GetPropertyNames();
    sal_Int32               nCount      = seqNames.getLength();
    Sequence< Any >         seqValues   ( nCount );
    for( sal_Int32 nProperty=0; nProperty<nCount; ++nProperty )
    {
        switch( nProperty )
        {
            case PROPERTYHANDLE_REPLACEMENTTABLE    :   {
                                                            seqValues[nProperty] <<= m_bReplacementTable;
                                                        }
                                                        break;
            case PROPERTYHANDLE_FONTHISTORY         :   {
                                                            seqValues[nProperty] <<= m_bFontHistory;
                                                        }
                                                        break;
            case PROPERTYHANDLE_FONTWYSIWYG         :   {
                                                            seqValues[nProperty] <<= m_bFontWYSIWYG;
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
sal_Bool SvtFontOptions_Impl::IsFontHistoryEnabled() const
{
    return m_bFontHistory;
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
void SvtFontOptions_Impl::EnableFontHistory( sal_Bool bState )
{
    m_bFontHistory = bState;
    SetModified();
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
sal_Bool SvtFontOptions_Impl::IsFontWYSIWYGEnabled() const
{
    return m_bFontWYSIWYG;
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
void SvtFontOptions_Impl::EnableFontWYSIWYG( sal_Bool bState )
{
    m_bFontWYSIWYG = bState;
    SetModified();
}

//*****************************************************************************************************************
//  private method
//*****************************************************************************************************************
Sequence< OUString > SvtFontOptions_Impl::impl_GetPropertyNames()
{
    // Build list of configuration key names.
    const OUString pProperties[] =
    {
        PROPERTYNAME_REPLACEMENTTABLE   ,
        PROPERTYNAME_FONTHISTORY        ,
        PROPERTYNAME_FONTWYSIWYG        ,
    };
    // Initialize return sequence with these list ...
    const Sequence< OUString > seqPropertyNames( pProperties, PROPERTYCOUNT );
    // ... and return it.
    return seqPropertyNames;
}

//*****************************************************************************************************************
//  initialize static member
//  DON'T DO IT IN YOUR HEADER!
//  see definition for further information
//*****************************************************************************************************************
SvtFontOptions_Impl*    SvtFontOptions::m_pDataContainer    = NULL  ;
sal_Int32               SvtFontOptions::m_nRefCount         = 0     ;

//*****************************************************************************************************************
//  constructor
//*****************************************************************************************************************
SvtFontOptions::SvtFontOptions()
{
    // Global access, must be guarded (multithreading!).
    MutexGuard aGuard( impl_GetOwnStaticMutex() );
    // Increase ouer refcount ...
    ++m_nRefCount;
    // ... and initialize ouer data container only if it not already exist!
    if( m_pDataContainer == NULL )
    {
        m_pDataContainer = new SvtFontOptions_Impl;

        ItemHolder1::holdConfigItem(E_FONTOPTIONS);
    }
}

//*****************************************************************************************************************
//  destructor
//*****************************************************************************************************************
SvtFontOptions::~SvtFontOptions()
{
    // Global access, must be guarded (multithreading!)
    MutexGuard aGuard( impl_GetOwnStaticMutex() );
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
sal_Bool SvtFontOptions::IsFontHistoryEnabled() const
{
    MutexGuard aGuard( impl_GetOwnStaticMutex() );
    return m_pDataContainer->IsFontHistoryEnabled();
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
void SvtFontOptions::EnableFontHistory( sal_Bool bState )
{
    MutexGuard aGuard( impl_GetOwnStaticMutex() );
    m_pDataContainer->EnableFontHistory( bState );
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
sal_Bool SvtFontOptions::IsFontWYSIWYGEnabled() const
{
    MutexGuard aGuard( impl_GetOwnStaticMutex() );
    return m_pDataContainer->IsFontWYSIWYGEnabled();
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
void SvtFontOptions::EnableFontWYSIWYG( sal_Bool bState )
{
    MutexGuard aGuard( impl_GetOwnStaticMutex() );
    m_pDataContainer->EnableFontWYSIWYG( bState );
}

namespace
{
    class theFontOptionsMutex : public rtl::Static<osl::Mutex, theFontOptionsMutex> {};
}

//*****************************************************************************************************************
//  private method
//*****************************************************************************************************************
Mutex& SvtFontOptions::impl_GetOwnStaticMutex()
{
    return theFontOptionsMutex::get();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

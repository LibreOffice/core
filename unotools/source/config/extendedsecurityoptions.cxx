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

#include <unotools/extendedsecurityoptions.hxx>
#include <unotools/configmgr.hxx>
#include <unotools/configitem.hxx>
#include <tools/debug.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <rtl/ustrbuf.hxx>

#include <unotools/pathoptions.hxx>

#include <boost/unordered_map.hpp>

#include "itemholder1.hxx"

using namespace ::utl                   ;
using namespace ::rtl                   ;
using namespace ::osl                   ;
using namespace ::com::sun::star::uno   ;

#define ROOTNODE_SECURITY               OUString("Office.Security")

#define SECURE_EXTENSIONS_SET           OUString("SecureExtensions")
#define EXTENSION_PROPNAME              OUString("/Extension")

#define PROPERTYNAME_HYPERLINKS_OPEN    OUString("Hyperlinks/Open")

#define PROPERTYHANDLE_HYPERLINKS_OPEN  0

#define PROPERTYCOUNT                   1

class ExtensionHashMap : public ::boost::unordered_map< OUString,
                                                 sal_Int32,
                                                 OUStringHash,
                                                 ::std::equal_to< OUString > >
{
    public:
        inline void free()
        {
            ExtensionHashMap().swap( *this );
        }
};

class SvtExtendedSecurityOptions_Impl : public ConfigItem
{
    public:
         SvtExtendedSecurityOptions_Impl();
        ~SvtExtendedSecurityOptions_Impl();

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

        SvtExtendedSecurityOptions::OpenHyperlinkMode   GetOpenHyperlinkMode();
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

        /*-****************************************************************************************************//**
            @short      Fills the hash map with all extensions known to be secure
            @descr      These methods fills the given hash map object with all extensions known to be secure.

            @seealso    -

            @param      aHashMap
                        A hash map to be filled with secure extension strings.
            @return     -

            @onerror    -
        *//*-*****************************************************************************************************/
        void FillExtensionHashMap( ExtensionHashMap& aHashMap );

        OUString                                        m_aSecureExtensionsSetName;
        OUString                                        m_aExtensionPropName;

        SvtExtendedSecurityOptions::OpenHyperlinkMode   m_eOpenHyperlinkMode;
        sal_Bool                                        m_bROOpenHyperlinkMode;
        ExtensionHashMap                                m_aExtensionHashMap;
};

//*****************************************************************************************************************
//  constructor
//*****************************************************************************************************************
SvtExtendedSecurityOptions_Impl::SvtExtendedSecurityOptions_Impl()
    // Init baseclasses first
    :   ConfigItem          ( ROOTNODE_SECURITY         ),
    m_aSecureExtensionsSetName( SECURE_EXTENSIONS_SET ),
    m_aExtensionPropName( EXTENSION_PROPNAME ),
    m_bROOpenHyperlinkMode(sal_False)
    // Init member then.
{
    // Fill the extension hash map with all secure extension strings
    FillExtensionHashMap( m_aExtensionHashMap );

    Sequence< OUString >    seqNames    = GetPropertyNames();
    Sequence< Any >         seqValues   = GetProperties( seqNames );
    Sequence< sal_Bool >    seqRO       = GetReadOnlyStates ( seqNames  );

    sal_Int32 nPropertyCount = seqValues.getLength();
    for( sal_Int32 nProperty=0; nProperty<nPropertyCount; ++nProperty )
    {
        // Safe impossible cases.
        // Check any for valid value.
        DBG_ASSERT( !(seqValues[nProperty].hasValue()==sal_False), "SvtExtendedSecurityOptions_Impl::SvtExtendedSecurityOptions_Impl()\nInvalid property value detected!\n" );
        switch( nProperty )
        {
            case PROPERTYHANDLE_HYPERLINKS_OPEN:
            {
                DBG_ASSERT( ( seqValues[nProperty].getValueTypeClass() == TypeClass_LONG ), "SvtExtendedSecurityOptions_Impl::SvtExtendedSecurityOptions_Impl()\nWho has changed the value type of 'Hyperlink/Open'?" );

                sal_Int32 nMode = SvtExtendedSecurityOptions::OPEN_WITHSECURITYCHECK;
                if ( seqValues[nProperty] >>= nMode )
                    m_eOpenHyperlinkMode = (SvtExtendedSecurityOptions::OpenHyperlinkMode)nMode;
                else {
                    OSL_FAIL("Wrong type for Open mode!");
                }
                m_bROOpenHyperlinkMode = seqRO[nProperty];
            }
            break;
        }
    }

    // Enable notification mechanism of our baseclass.
    // We need it to get information about changes outside these class on ouer used configuration keys!
    Sequence< OUString > seqNotifyNames( 1 );
    seqNotifyNames[0] = m_aSecureExtensionsSetName;
    EnableNotification( seqNotifyNames );
}

//*****************************************************************************************************************
//  destructor
//*****************************************************************************************************************
SvtExtendedSecurityOptions_Impl::~SvtExtendedSecurityOptions_Impl()
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
void SvtExtendedSecurityOptions_Impl::Notify( const Sequence< OUString >& )
{
    // Not implemented
}

//*****************************************************************************************************************
//  public method
//*****************************************************************************************************************
void SvtExtendedSecurityOptions_Impl::Commit()
{
    // Get names of supported properties, create a list for values and copy current values to it.
    Sequence< OUString >    seqNames    = GetPropertyNames  ();
    sal_Int32               nCount      = seqNames.getLength();
    Sequence< Any >         seqValues   ( nCount );
    for( sal_Int32 nProperty=0; nProperty<nCount; ++nProperty )
    {
        switch( nProperty )
        {
            case PROPERTYHANDLE_HYPERLINKS_OPEN:    {
                                                        seqValues[nProperty] <<= (sal_Int32)m_eOpenHyperlinkMode;
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
SvtExtendedSecurityOptions::OpenHyperlinkMode SvtExtendedSecurityOptions_Impl::GetOpenHyperlinkMode()
{
    return m_eOpenHyperlinkMode;
}

//*****************************************************************************************************************
//  private method
//*****************************************************************************************************************
void SvtExtendedSecurityOptions_Impl::FillExtensionHashMap( ExtensionHashMap& aHashMap )
{
    // Get sequence with secure extensions from configuration
    Sequence< OUString >    seqNodes = GetNodeNames( m_aSecureExtensionsSetName );

    OUString                aValue;
    Sequence< Any >         aValues;
    Sequence< OUString >    aPropSeq( 1 );
    for ( int i = 0; i < seqNodes.getLength(); i++ )
    {
        // Create access name for property
        OUStringBuffer aExtEntryProp( m_aSecureExtensionsSetName );
        aExtEntryProp.appendAscii( "/" );
        aExtEntryProp.append( seqNodes[i] );
        aExtEntryProp.append( m_aExtensionPropName );

        aPropSeq[0] = aExtEntryProp.makeStringAndClear();
        aValues = GetProperties( aPropSeq );
        if ( aValues.getLength() == 1 )
        {
            // Don't use value if sequence has not the correct length
            if ( aValues[0] >>= aValue )
                // Add extension into secure extensions hash map
                aHashMap.insert( ExtensionHashMap::value_type( aValue.toAsciiLowerCase(), 1 ) );
            else
            {
                SAL_WARN( "unotools.config", "SvtExtendedSecurityOptions_Impl::FillExtensionHashMap(): not string value?" );
            }
        }
    }
}

//*****************************************************************************************************************
//  private method (currently not used)
//*****************************************************************************************************************
Sequence< OUString > SvtExtendedSecurityOptions_Impl::GetPropertyNames()
{
    // Build list of configuration key names.
    const OUString pProperties[] =
    {
        PROPERTYNAME_HYPERLINKS_OPEN
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
SvtExtendedSecurityOptions_Impl*    SvtExtendedSecurityOptions::m_pDataContainer    = NULL  ;
sal_Int32                           SvtExtendedSecurityOptions::m_nRefCount         = 0     ;

//*****************************************************************************************************************
//  constructor
//*****************************************************************************************************************
SvtExtendedSecurityOptions::SvtExtendedSecurityOptions()
{
    // Global access, must be guarded (multithreading!).
    MutexGuard aGuard( GetInitMutex() );
    // Increase ouer refcount ...
    ++m_nRefCount;
    // ... and initialize ouer data container only if it not already exist!
    if( m_pDataContainer == NULL )
    {
       m_pDataContainer = new SvtExtendedSecurityOptions_Impl;

        ItemHolder1::holdConfigItem(E_EXTENDEDSECURITYOPTIONS);
    }
}

//*****************************************************************************************************************
//  destructor
//*****************************************************************************************************************
SvtExtendedSecurityOptions::~SvtExtendedSecurityOptions()
{
    // Global access, must be guarded (multithreading!)
    MutexGuard aGuard( GetInitMutex() );
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
SvtExtendedSecurityOptions::OpenHyperlinkMode SvtExtendedSecurityOptions::GetOpenHyperlinkMode()
{
    MutexGuard aGuard( GetInitMutex() );
    return m_pDataContainer->GetOpenHyperlinkMode();
}

namespace
{
    class theExtendedSecurityOptionsMutex : public rtl::Static<osl::Mutex, theExtendedSecurityOptionsMutex>{};
}

//*****************************************************************************************************************
//  private method
//*****************************************************************************************************************
Mutex& SvtExtendedSecurityOptions::GetInitMutex()
{
    return theExtendedSecurityOptionsMutex::get();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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

#include <unotools/extendedsecurityoptions.hxx>
#include <unotools/configitem.hxx>
#include <tools/debug.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <osl/diagnose.h>

#include "itemholder1.hxx"

#include <unordered_map>

using namespace ::utl;
using namespace ::osl;
using namespace ::com::sun::star::uno;

#define ROOTNODE_SECURITY               "Office.Security"

#define SECURE_EXTENSIONS_SET           OUString("SecureExtensions")

#define PROPERTYNAME_HYPERLINKS_OPEN    OUString("Hyperlinks/Open")

#define PROPERTYHANDLE_HYPERLINKS_OPEN  0

#define PROPERTYCOUNT                   1

typedef std::unordered_map<OUString, sal_Int32>
    ExtensionHashMap;

class SvtExtendedSecurityOptions_Impl : public ConfigItem
{
    public:
         SvtExtendedSecurityOptions_Impl();
        virtual ~SvtExtendedSecurityOptions_Impl() override;

        /*-****************************************************************************************************
            @short      called for notify of configmanager
            @descr      This method is called from the ConfigManager before the application ends or from the
                        PropertyChangeListener if the sub tree broadcasts changes. You must update your
                        internal values.

            @seealso    baseclass ConfigItem

            @param      "seqPropertyNames" is the list of properties which should be updated.
        *//*-*****************************************************************************************************/

        virtual void Notify( const Sequence< OUString >& seqPropertyNames ) override;

        SvtExtendedSecurityOptions::OpenHyperlinkMode   GetOpenHyperlinkMode() const { return m_eOpenHyperlinkMode;}

    private:
        virtual void ImplCommit() override;

        /*-****************************************************************************************************
            @short      return list of key names of our configuration management which represent our module tree
            @descr      This method returns a static const list of key names. We need it to get needed values from our
                        configuration management.
            @return     A list of needed configuration keys is returned.
        *//*-*****************************************************************************************************/

        static Sequence< OUString > GetPropertyNames();

        SvtExtendedSecurityOptions::OpenHyperlinkMode   m_eOpenHyperlinkMode;
};

//  constructor

SvtExtendedSecurityOptions_Impl::SvtExtendedSecurityOptions_Impl()
    // Init baseclasses first
    :   ConfigItem          ( ROOTNODE_SECURITY         )
    , m_eOpenHyperlinkMode(SvtExtendedSecurityOptions::OPEN_NEVER)
    // Init member then.
{
    Sequence< OUString >    seqNames    = GetPropertyNames();
    Sequence< Any >         seqValues   = GetProperties( seqNames );

    sal_Int32 nPropertyCount = seqValues.getLength();
    for( sal_Int32 nProperty=0; nProperty<nPropertyCount; ++nProperty )
    {
        // Safe impossible cases.
        // Check any for valid value.
        DBG_ASSERT( seqValues[nProperty].hasValue(), "SvtExtendedSecurityOptions_Impl::SvtExtendedSecurityOptions_Impl()\nInvalid property value detected!\n" );
        switch( nProperty )
        {
            case PROPERTYHANDLE_HYPERLINKS_OPEN:
            {
                DBG_ASSERT( ( seqValues[nProperty].getValueTypeClass() == TypeClass_LONG ), "SvtExtendedSecurityOptions_Impl::SvtExtendedSecurityOptions_Impl()\nWho has changed the value type of 'Hyperlink/Open'?" );

                sal_Int32 nMode = SvtExtendedSecurityOptions::OPEN_WITHSECURITYCHECK;
                if ( seqValues[nProperty] >>= nMode )
                    m_eOpenHyperlinkMode = static_cast<SvtExtendedSecurityOptions::OpenHyperlinkMode>(nMode);
                else {
                    OSL_FAIL("Wrong type for Open mode!");
                }
            }
            break;
        }
    }

    // Enable notification mechanism of our baseclass.
    // We need it to get information about changes outside these class on our used configuration keys!
    Sequence<OUString> seqNotifyNames { SECURE_EXTENSIONS_SET };
    EnableNotification( seqNotifyNames );
}

//  destructor

SvtExtendedSecurityOptions_Impl::~SvtExtendedSecurityOptions_Impl()
{
    assert(!IsModified()); // should have been committed
}

//  public method

void SvtExtendedSecurityOptions_Impl::Notify( const Sequence< OUString >& )
{
    // Not implemented
}

//  public method

void SvtExtendedSecurityOptions_Impl::ImplCommit()
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
                                                        seqValues[nProperty] <<= static_cast<sal_Int32>(m_eOpenHyperlinkMode);
                                                    }
                                                    break;
        }
    }

    // Set properties in configuration.
    PutProperties( seqNames, seqValues );
}

//  private method (currently not used)

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

namespace {

std::weak_ptr<SvtExtendedSecurityOptions_Impl> g_pExtendedSecurityOptions;

}

SvtExtendedSecurityOptions::SvtExtendedSecurityOptions()
{
    // Global access, must be guarded (multithreading!).
    MutexGuard aGuard( GetInitMutex() );

    m_pImpl = g_pExtendedSecurityOptions.lock();
    if( !m_pImpl )
    {
        m_pImpl = std::make_shared<SvtExtendedSecurityOptions_Impl>();
        g_pExtendedSecurityOptions = m_pImpl;
        ItemHolder1::holdConfigItem(EItem::ExtendedSecurityOptions);
    }
}

SvtExtendedSecurityOptions::~SvtExtendedSecurityOptions()
{
    // Global access, must be guarded (multithreading!)
    MutexGuard aGuard( GetInitMutex() );

    m_pImpl.reset();
}

//  public method

SvtExtendedSecurityOptions::OpenHyperlinkMode SvtExtendedSecurityOptions::GetOpenHyperlinkMode() const
{
    MutexGuard aGuard( GetInitMutex() );
    return m_pImpl->GetOpenHyperlinkMode();
}

namespace
{
    class theExtendedSecurityOptionsMutex : public rtl::Static<osl::Mutex, theExtendedSecurityOptionsMutex>{};
}

//  private method

Mutex& SvtExtendedSecurityOptions::GetInitMutex()
{
    return theExtendedSecurityOptionsMutex::get();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

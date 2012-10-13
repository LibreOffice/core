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

#include "connpooloptions.hxx"
#include "dbregisterednamesconfig.hxx"
#include "dbregistersettings.hxx"
#include "svx/svxids.hrc"
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/sdb/DatabaseContext.hpp>
#include <comphelper/componentcontext.hxx>
#include <comphelper/extract.hxx>
#include <comphelper/processfactory.hxx>
#include <svl/eitem.hxx>
#include <svl/itemset.hxx>
#include <unotools/pathoptions.hxx>
#include <unotools/confignode.hxx>
#include <tools/diagnose_ex.h>

//........................................................................
namespace svx
{
//........................................................................

    using namespace ::utl;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::sdb;
    using namespace ::com::sun::star::container;

    //====================================================================
    //= DbRegisteredNamesConfig
    //====================================================================
    //--------------------------------------------------------------------
    void DbRegisteredNamesConfig::GetOptions( SfxItemSet& _rFillItems )
    {
        DatabaseRegistrations aSettings;

        try
        {
            Reference<XComponentContext> xContext( ::comphelper::getProcessComponentContext() );
            Reference< XDatabaseContext > xRegistrations(
                DatabaseContext::create(xContext) );

            Sequence< ::rtl::OUString > aRegistrationNames( xRegistrations->getRegistrationNames() );
            const ::rtl::OUString* pRegistrationName = aRegistrationNames.getConstArray();
            const ::rtl::OUString* pRegistrationNamesEnd = pRegistrationName + aRegistrationNames.getLength();
            for ( ; pRegistrationName != pRegistrationNamesEnd; ++pRegistrationName )
            {
                ::rtl::OUString sLocation( xRegistrations->getDatabaseLocation( *pRegistrationName ) );
                aSettings[ *pRegistrationName ] =
                    DatabaseRegistration( sLocation, xRegistrations->isDatabaseRegistrationReadOnly( *pRegistrationName ) );
            }
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }

        _rFillItems.Put( DatabaseMapItem( SID_SB_DB_REGISTER, aSettings ) );
    }

    //--------------------------------------------------------------------
    void DbRegisteredNamesConfig::SetOptions(const SfxItemSet& _rSourceItems)
    {
        // the settings for the single drivers
        SFX_ITEMSET_GET( _rSourceItems, pRegistrations, DatabaseMapItem, SID_SB_DB_REGISTER, sal_True );
        if ( !pRegistrations )
            return;

        try
        {
            Reference< XDatabaseContext > xRegistrations(
                DatabaseContext::create(
                    comphelper::getProcessComponentContext()));

            const DatabaseRegistrations& rNewRegistrations = pRegistrations->getRegistrations();
            for (   DatabaseRegistrations::const_iterator reg = rNewRegistrations.begin();
                    reg != rNewRegistrations.end();
                    ++reg
                )
            {
                const ::rtl::OUString sName = reg->first;
                const ::rtl::OUString sLocation = reg->second.sLocation;

                if ( xRegistrations->hasRegisteredDatabase( sName ) )
                {
                    if ( !xRegistrations->isDatabaseRegistrationReadOnly( sName ) )
                        xRegistrations->changeDatabaseLocation( sName, sLocation );
                    else
                    {
                        OSL_ENSURE( xRegistrations->getDatabaseLocation( sName ) == sLocation,
                            "DbRegisteredNamesConfig::SetOptions: somebody changed a read-only registration. How unrespectful." );
                    }
                }
                else
                    xRegistrations->registerDatabaseLocation( sName, sLocation );
            }

            // delete unused entries
            Sequence< ::rtl::OUString > aRegistrationNames = xRegistrations->getRegistrationNames();
            const ::rtl::OUString* pRegistrationName = aRegistrationNames.getConstArray();
            const ::rtl::OUString* pRegistrationNamesEnd = pRegistrationName + aRegistrationNames.getLength();
            for ( ; pRegistrationName != pRegistrationNamesEnd; ++pRegistrationName )
            {
                if ( rNewRegistrations.find( *pRegistrationName ) == rNewRegistrations.end() )
                    xRegistrations->revokeDatabaseLocation( *pRegistrationName );
            }
        }
        catch( const Exception& )
        {
                        //DBG_UNHANDLED_EXCEPTION();
        }
    }

//........................................................................
}   // namespace svx
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

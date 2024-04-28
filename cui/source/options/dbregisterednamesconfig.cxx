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

#include "dbregisterednamesconfig.hxx"
#include "dbregistersettings.hxx"
#include <svx/databaseregistrationui.hxx>
#include <com/sun/star/sdb/DatabaseContext.hpp>
#include <comphelper/processfactory.hxx>
#include <svl/itemset.hxx>
#include <comphelper/diagnose_ex.hxx>


namespace svx
{


    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::sdb;

    void DbRegisteredNamesConfig::GetOptions( SfxItemSet& _rFillItems )
    {
        DatabaseRegistrations aSettings;

        try
        {
            Reference<XComponentContext> xContext( ::comphelper::getProcessComponentContext() );
            Reference< XDatabaseContext > xRegistrations(
                DatabaseContext::create(xContext) );

            for (auto& registrationName : xRegistrations->getRegistrationNames())
            {
                aSettings[registrationName] = DatabaseRegistration(
                    xRegistrations->getDatabaseLocation(registrationName),
                    xRegistrations->isDatabaseRegistrationReadOnly(registrationName));
            }
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION("cui.options");
        }

        _rFillItems.Put( DatabaseMapItem( SID_SB_DB_REGISTER, std::move(aSettings) ) );
    }


    void DbRegisteredNamesConfig::SetOptions(const SfxItemSet& _rSourceItems)
    {
        // the settings for the single drivers
        const DatabaseMapItem* pRegistrations = _rSourceItems.GetItem<DatabaseMapItem>(SID_SB_DB_REGISTER);
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
                const OUString sName = reg->first;
                const OUString sLocation = reg->second.sLocation;

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
            for (auto& registrationName : xRegistrations->getRegistrationNames())
            {
                if (rNewRegistrations.find(registrationName) == rNewRegistrations.end())
                    xRegistrations->revokeDatabaseLocation(registrationName);
            }
        }
        catch( const Exception& )
        {
                        //DBG_UNHANDLED_EXCEPTION();
        }
    }


}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

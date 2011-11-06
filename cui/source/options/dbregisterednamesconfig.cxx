/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_cui.hxx"
#include "connpooloptions.hxx"
#include "dbregisterednamesconfig.hxx"
#include "dbregistersettings.hxx"
#include "svx/svxids.hrc"
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/sdb/XDatabaseRegistrations.hpp>
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
            ::comphelper::ComponentContext aContext( ::comphelper::getProcessServiceFactory() );
            Reference< XDatabaseRegistrations > xRegistrations(
                aContext.createComponent( "com.sun.star.sdb.DatabaseContext" ), UNO_QUERY_THROW );

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
            ::comphelper::ComponentContext aContext( ::comphelper::getProcessServiceFactory() );
            Reference< XDatabaseRegistrations > xRegistrations(
                aContext.createComponent( "com.sun.star.sdb.DatabaseContext" ), UNO_QUERY_THROW );

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
            DBG_UNHANDLED_EXCEPTION();
        }
    }

//........................................................................
}   // namespace svx
//........................................................................

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

#include "fieldmappingimpl.hxx"
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#include <com/sun/star/ui/AddressBookSourceDialog.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/sdb/CommandType.hpp>
#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>
#include <vcl/weld.hxx>
#include <com/sun/star/util/AliasProgrammaticPair.hpp>
#include <strings.hrc>
#include <componentmodule.hxx>
#include <unotools/confignode.hxx>
#include <sal/macros.h>
#include <sal/log.hxx>


namespace abp
{


    using namespace ::utl;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::awt;
    using namespace ::com::sun::star::util;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::sdb;
    using namespace ::com::sun::star::ui;
    using namespace ::com::sun::star::ui::dialogs;


    const char16_t sDriverSettingsNodeName[] = u"/org.openoffice.Office.DataAccess/DriverSettings/com.sun.star.comp.sdbc.MozabDriver";
    const OUStringLiteral sAddressBookNodeName = u"/org.openoffice.Office.DataAccess/AddressBook";

    namespace fieldmapping
    {
        bool invokeDialog( const Reference< XComponentContext >& _rxORB, class weld::Window* _pParent,
            const Reference< XPropertySet >& _rxDataSource, AddressSettings& _rSettings )
        {
            _rSettings.aFieldMapping.clear();

            DBG_ASSERT( _rxORB.is(), "fieldmapping::invokeDialog: invalid service factory!" );
            DBG_ASSERT( _rxDataSource.is(), "fieldmapping::invokeDialog: invalid data source!" );
            if ( !_rxORB.is() || !_rxDataSource.is() )
                return false;

            try
            {

                // create an instance of the dialog service
                Reference< XWindow > xDialogParent = _pParent->GetXWindow();
                OUString sTitle(compmodule::ModuleRes(RID_STR_FIELDDIALOGTITLE));
                Reference< XExecutableDialog > xDialog = AddressBookSourceDialog::createWithDataSource(_rxORB,
                                                           // the parent window
                                                           xDialogParent,
                                                           _rxDataSource,
                                                           _rSettings.bRegisterDataSource ? _rSettings.sRegisteredDataSourceName : _rSettings.sDataSourceName,
                                                           // the table to use
                                                           _rSettings.sSelectedTable,
                                                           sTitle);

                // execute the dialog
                if ( xDialog->execute() )
                {
                    // retrieve the field mapping as set by he user
                    Reference< XPropertySet > xDialogProps( xDialog, UNO_QUERY );

                    Sequence< AliasProgrammaticPair > aMapping;
                    bool bSuccess =
                        xDialogProps->getPropertyValue("FieldMapping") >>= aMapping;
                    DBG_ASSERT( bSuccess, "fieldmapping::invokeDialog: invalid property type for FieldMapping!" );

                    // and copy it into the map
                    const AliasProgrammaticPair* pMapping = aMapping.getConstArray();
                    const AliasProgrammaticPair* pMappingEnd = pMapping + aMapping.getLength();
                    for (;pMapping != pMappingEnd; ++pMapping)
                        _rSettings.aFieldMapping[ pMapping->ProgrammaticName ] = pMapping->Alias;

                    return true;
                }

            }
            catch(const Exception&)
            {
                TOOLS_WARN_EXCEPTION("extensions.abpilot",
                                     "caught an exception while executing the dialog!");
            }
            return false;
        }


        void defaultMapping(  const Reference< XComponentContext >& _rxContext, MapString2String& _rFieldAssignment )
        {
            _rFieldAssignment.clear();

            try
            {
                // what we have:
                // a) For the address data source, we need a mapping from programmatic names (1) to real column names
                // b) The SDBC driver has a fixed set of columns, which, when returned, are named according to
                //    some configuration entries. E.g., the driver displays the field which it knows contains
                //    the first name as "First Name" - the latter string is stored in the config.
                //    For this, the driver uses programmatic names, too, but they differ from the programmatic names the
                //    template documents have.
                // So what we need first is a mapping from programmatic names (1) to programmatic names (2)
                const char* pMappingProgrammatics[] =
                {
                    "FirstName",            "FirstName",
                    "LastName",             "LastName",
                    "Street",               "HomeAddress",
                    "Zip",                  "HomeZipCode",
                    "City",                 "HomeCity",
                    "State",                "HomeState",
                    "Country",              "HomeCountry",
                    "PhonePriv",            "HomePhone",
                    "PhoneComp",            "WorkPhone",
                    "PhoneCell",            "CellularNumber",
                    "Pager",                "PagerNumber",
                    "Fax",                  "FaxNumber",
                    "EMail",                "PrimaryEmail",
                    "URL",                  "WebPage1",
                    "Note",                 "Notes",
                    "Altfield1",            "Custom1",
                    "Altfield2",            "Custom2",
                    "Altfield3",            "Custom3",
                    "Altfield4",            "Custom4",
                    "Title",                "JobTitle",
                    "Company",              "Company",
                    "Department",           "Department"
                };
                    // (this list is not complete: both lists of programmatic names are larger in real,
                    // but this list above is the intersection)


                // access the configuration information which the driver uses for determining its column names
                OUString sDriverAliasesNodeName(
                    OUString::Concat(sDriverSettingsNodeName)
                    + "/ColumnAliases");

                // create a config node for this
                OConfigurationTreeRoot aDriverFieldAliasing = OConfigurationTreeRoot::createWithComponentContext(
                    _rxContext, sDriverAliasesNodeName, -1, OConfigurationTreeRoot::CM_READONLY);

                // loop through all programmatic pairs
                DBG_ASSERT( 0 == SAL_N_ELEMENTS( pMappingProgrammatics ) % 2,
                    "fieldmapping::defaultMapping: invalid programmatic map!" );
                // number of pairs
                sal_Int32 const nIntersectedProgrammatics = SAL_N_ELEMENTS( pMappingProgrammatics ) / 2;

                const char** pProgrammatic = pMappingProgrammatics;
                OUString sAddressProgrammatic;
                OUString sDriverProgrammatic;
                OUString sDriverUI;
                for (   sal_Int32 i=0;
                        i < nIntersectedProgrammatics;
                        ++i
                    )
                {
                    sAddressProgrammatic = OUString::createFromAscii( *pProgrammatic++ );
                    sDriverProgrammatic = OUString::createFromAscii( *pProgrammatic++ );

                    if ( aDriverFieldAliasing.hasByName( sDriverProgrammatic ) )
                    {
                        aDriverFieldAliasing.getNodeValue( sDriverProgrammatic ) >>= sDriverUI;
                        if ( 0 == sDriverUI.getLength() )
                        {
                            OSL_FAIL( "fieldmapping::defaultMapping: invalid driver UI column name!");
                        }
                        else
                            _rFieldAssignment[ sAddressProgrammatic ] = sDriverUI;
                    }
                    else
                    {
                        OSL_FAIL( "fieldmapping::defaultMapping: invalid driver programmatic name!" );
                    }
                }
            }
            catch( const Exception& )
            {
                TOOLS_WARN_EXCEPTION("extensions.abpilot",
                                     "code is assumed to throw no exceptions!");
                    // the config nodes we're using herein should not do this...
            }
        }


        void writeTemplateAddressFieldMapping( const Reference< XComponentContext >& _rxContext, const MapString2String& _rFieldAssignment )
        {
            // want to have a non-const map for easier handling
            MapString2String aFieldAssignment( _rFieldAssignment );

            // access the configuration information which the driver uses for determining its column names

            // create a config node for this
            OConfigurationTreeRoot aAddressBookSettings = OConfigurationTreeRoot::createWithComponentContext(
                _rxContext, sAddressBookNodeName);

            OConfigurationNode aFields = aAddressBookSettings.openNode( OUString( "Fields" ) );

            // loop through all existent fields
            Sequence< OUString > aExistentFields = aFields.getNodeNames();
            const OUString* pExistentFields = aExistentFields.getConstArray();
            const OUString* pExistentFieldsEnd = pExistentFields + aExistentFields.getLength();

            const OUString sProgrammaticNodeName( "ProgrammaticFieldName" );
            const OUString sAssignedNodeName( "AssignedFieldName" );

            for ( ; pExistentFields != pExistentFieldsEnd; ++pExistentFields )
            {
                SAL_WARN_IF(
                    ((aFields.openNode(*pExistentFields)
                      .getNodeValue(sProgrammaticNodeName).get<OUString>())
                     != *pExistentFields),
                    "extensions.abpilot",
                    "fieldmapping::writeTemplateAddressFieldMapping: inconsistent config data!");
                    // there should be a redundancy in the config data... if this asserts, there isn't anymore!

                // do we have a new alias for the programmatic?
                MapString2String::iterator aPos = aFieldAssignment.find( *pExistentFields );
                if ( aFieldAssignment.end() != aPos )
                {   // yes
                    // -> set a new value
                    OConfigurationNode aExistentField = aFields.openNode( *pExistentFields );
                    aExistentField.setNodeValue( sAssignedNodeName, makeAny( aPos->second ) );
                    // and remove the mapping entry
                    aFieldAssignment.erase( *pExistentFields );
                }
                else
                {   // no
                    // -> remove it
                    aFields.removeNode( *pExistentFields );
                }
            }

            // now everything remaining in aFieldAssignment marks a mapping entry which was not present
            // in the config before
            for (auto const& elem : aFieldAssignment)
            {
                DBG_ASSERT( !aFields.hasByName( elem.first ),
                    "fieldmapping::writeTemplateAddressFieldMapping: inconsistence!" );
                    // in case the config node for the fields already has the node named <aNewMapping->first>,
                    // the entry should have been removed from aNewMapping (in the above loop)
                OConfigurationNode aNewField =  aFields.createNode( elem.first );
                aNewField.setNodeValue( sProgrammaticNodeName, makeAny( elem.first ) );
                aNewField.setNodeValue( sAssignedNodeName, makeAny( elem.second ) );
            }

            // commit the changes done
            aAddressBookSettings.commit();
        }


    }   // namespace fieldmapping


    namespace addressconfig
    {


        void writeTemplateAddressSource( const Reference< XComponentContext >& _rxContext,
            const OUString& _rDataSourceName, const OUString& _rTableName )
        {
            // access the configuration information which the driver uses for determining its column names

            // create a config node for this
            OConfigurationTreeRoot aAddressBookSettings = OConfigurationTreeRoot::createWithComponentContext(
                _rxContext, sAddressBookNodeName);

            aAddressBookSettings.setNodeValue( OUString( "DataSourceName" ), makeAny( _rDataSourceName ) );
            aAddressBookSettings.setNodeValue( OUString( "Command" ), makeAny( _rTableName ) );
            aAddressBookSettings.setNodeValue( OUString( "CommandType" ), makeAny( sal_Int16(CommandType::TABLE) ) );

            // commit the changes done
            aAddressBookSettings.commit();
        }


        void markPilotSuccess( const Reference< XComponentContext >& _rxContext )
        {
            // access the configuration information which the driver uses for determining its column names

            // create a config node for this
            OConfigurationTreeRoot aAddressBookSettings = OConfigurationTreeRoot::createWithComponentContext(
                _rxContext, sAddressBookNodeName);

            // set the flag
            aAddressBookSettings.setNodeValue( OUString( "AutoPilotCompleted" ), makeAny( true ) );

            // commit the changes done
            aAddressBookSettings.commit();
        }


    }   // namespace addressconfig


}   // namespace abp


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

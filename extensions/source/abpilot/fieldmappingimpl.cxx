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
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#include <com/sun/star/ui/AddressBookSourceDialog.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/sdb/CommandType.hpp>
#include <tools/debug.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/stdtext.hxx>
#include <com/sun/star/util/AliasProgrammaticPair.hpp>
#include "abpresid.hrc"
#include "componentmodule.hxx"
#include <unotools/confignode.hxx>
#include "sal/macros.h"

//.........................................................................
namespace abp
{
//.........................................................................

    using namespace ::utl;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::awt;
    using namespace ::com::sun::star::util;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::sdb;
    using namespace ::com::sun::star::ui;
    using namespace ::com::sun::star::ui::dialogs;

    
    static const OUString& lcl_getDriverSettingsNodeName()
    {
        static const OUString s_sDriverSettingsNodeName( "/org.openoffice.Office.DataAccess/DriverSettings/com.sun.star.comp.sdbc.MozabDriver" );
        return s_sDriverSettingsNodeName;
    }

    
    static const OUString& lcl_getAddressBookNodeName()
    {
        static const OUString s_sAddressBookNodeName( "/org.openoffice.Office.DataAccess/AddressBook" );
        return s_sAddressBookNodeName;
    }

    //.....................................................................
    namespace fieldmapping
    {
    //.....................................................................

        
        sal_Bool invokeDialog( const Reference< XComponentContext >& _rxORB, class Window* _pParent,
            const Reference< XPropertySet >& _rxDataSource, AddressSettings& _rSettings ) SAL_THROW ( ( ) )
        {
            _rSettings.aFieldMapping.clear();

            DBG_ASSERT( _rxORB.is(), "fieldmapping::invokeDialog: invalid service factory!" );
            DBG_ASSERT( _rxDataSource.is(), "fieldmapping::invokeDialog: invalid data source!" );
            if ( !_rxORB.is() || !_rxDataSource.is() )
                return sal_False;

            try
            {
                
                
                Reference< XWindow > xDialogParent = VCLUnoHelper::GetInterface( _pParent );
                OUString sTitle(ModuleRes(RID_STR_FIELDDIALOGTITLE).toString());
                Reference< XExecutableDialog > xDialog = AddressBookSourceDialog::createWithDataSource(_rxORB,
                                                           
                                                           xDialogParent,
                                                           _rxDataSource,
                                                           _rSettings.bRegisterDataSource ? _rSettings.sRegisteredDataSourceName : _rSettings.sDataSourceName,
                                                           
                                                           _rSettings.sSelectedTable,
                                                           sTitle);

                
                if ( xDialog->execute() )
                {
                    
                    Reference< XPropertySet > xDialogProps( xDialog, UNO_QUERY );

                    Sequence< AliasProgrammaticPair > aMapping;
#ifdef DBG_UTIL
                    sal_Bool bSuccess =
#endif
                    xDialogProps->getPropertyValue("FieldMapping") >>= aMapping;
                    DBG_ASSERT( bSuccess, "fieldmapping::invokeDialog: invalid property type for FieldMapping!" );

                    
                    const AliasProgrammaticPair* pMapping = aMapping.getConstArray();
                    const AliasProgrammaticPair* pMappingEnd = pMapping + aMapping.getLength();
                    for (;pMapping != pMappingEnd; ++pMapping)
                        _rSettings.aFieldMapping[ pMapping->ProgrammaticName ] = pMapping->Alias;

                    return sal_True;
                }

            }
            catch(const Exception&)
            {
                OSL_FAIL("fieldmapping::invokeDialog: caught an exception while executing the dialog!");
            }
            return sal_False;
        }

        
        void defaultMapping(  const Reference< XComponentContext >& _rxContext, MapString2String& _rFieldAssignment ) SAL_THROW ( ( ) )
        {
            _rFieldAssignment.clear();

            try
            {
                
                
                
                
                
                
                
                
                const sal_Char* pMappingProgrammatics[] =
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
                    
                    


                
                OUString sDriverAliasesNodeName = lcl_getDriverSettingsNodeName();
                sDriverAliasesNodeName += OUString( "/ColumnAliases" );

                
                OConfigurationTreeRoot aDriverFieldAliasing = OConfigurationTreeRoot::createWithComponentContext(
                    _rxContext, sDriverAliasesNodeName, -1, OConfigurationTreeRoot::CM_READONLY);

                
                DBG_ASSERT( 0 == SAL_N_ELEMENTS( pMappingProgrammatics ) % 2,
                    "fieldmapping::defaultMapping: invalid programmatic map!" );
                
                sal_Int32 nIntersectedProgrammatics = SAL_N_ELEMENTS( pMappingProgrammatics ) / 2;

                const sal_Char** pProgrammatic = pMappingProgrammatics;
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
                OSL_FAIL("fieldmapping::defaultMapping: code is assumed to throw no exceptions!");
                    
            }
        }

        
        void writeTemplateAddressFieldMapping( const Reference< XComponentContext >& _rxContext, const MapString2String& _rFieldAssignment ) SAL_THROW ( ( ) )
        {
            
            MapString2String aFieldAssignment( _rFieldAssignment );

            
            const OUString& sAddressBookNodeName = lcl_getAddressBookNodeName();

            
            OConfigurationTreeRoot aAddressBookSettings = OConfigurationTreeRoot::createWithComponentContext(
                _rxContext, sAddressBookNodeName, -1, OConfigurationTreeRoot::CM_UPDATABLE);

            OConfigurationNode aFields = aAddressBookSettings.openNode( OUString( "Fields" ) );

            
            Sequence< OUString > aExistentFields = aFields.getNodeNames();
            const OUString* pExistentFields = aExistentFields.getConstArray();
            const OUString* pExistentFieldsEnd = pExistentFields + aExistentFields.getLength();

            const OUString sProgrammaticNodeName( "ProgrammaticFieldName" );
            const OUString sAssignedNodeName( "AssignedFieldName" );

            for ( ; pExistentFields != pExistentFieldsEnd; ++pExistentFields )
            {
#ifdef DBG_UTIL
                OUString sRedundantProgrammaticName;
                aFields.openNode( *pExistentFields ).getNodeValue( sProgrammaticNodeName ) >>= sRedundantProgrammaticName;
#endif
                DBG_ASSERT( sRedundantProgrammaticName == *pExistentFields,
                    "fieldmapping::writeTemplateAddressFieldMapping: inconsistent config data!" );
                    

                
                MapString2String::iterator aPos = aFieldAssignment.find( *pExistentFields );
                if ( aFieldAssignment.end() != aPos )
                {   
                    
                    OConfigurationNode aExistentField = aFields.openNode( *pExistentFields );
                    aExistentField.setNodeValue( sAssignedNodeName, makeAny( aPos->second ) );
                    
                    aFieldAssignment.erase( *pExistentFields );
                }
                else
                {   
                    
                    aFields.removeNode( *pExistentFields );
                }
            }

            
            
            for (   MapString2String::const_iterator aNewMapping = aFieldAssignment.begin();
                    aNewMapping != aFieldAssignment.end();
                    ++aNewMapping
                )
            {
                DBG_ASSERT( !aFields.hasByName( aNewMapping->first ),
                    "fieldmapping::writeTemplateAddressFieldMapping: inconsistence!" );
                    
                    
                OConfigurationNode aNewField =  aFields.createNode( aNewMapping->first );
                aNewField.setNodeValue( sProgrammaticNodeName, makeAny( aNewMapping->first ) );
                aNewField.setNodeValue( sAssignedNodeName, makeAny( aNewMapping->second ) );
            }

            
            aAddressBookSettings.commit();
        }

    //.....................................................................
    }   
    //.....................................................................

    //.....................................................................
    namespace addressconfig
    {
    //.....................................................................

        
        void writeTemplateAddressSource( const Reference< XComponentContext >& _rxContext,
            const OUString& _rDataSourceName, const OUString& _rTableName ) SAL_THROW ( ( ) )
        {
            
            const OUString& sAddressBookNodeName = lcl_getAddressBookNodeName();

            
            OConfigurationTreeRoot aAddressBookSettings = OConfigurationTreeRoot::createWithComponentContext(
                _rxContext, sAddressBookNodeName, -1, OConfigurationTreeRoot::CM_UPDATABLE);

            aAddressBookSettings.setNodeValue( OUString( "DataSourceName" ), makeAny( _rDataSourceName ) );
            aAddressBookSettings.setNodeValue( OUString( "Command" ), makeAny( _rTableName ) );
            aAddressBookSettings.setNodeValue( OUString( "CommandType" ), makeAny( (sal_Int32)CommandType::TABLE ) );

            
            aAddressBookSettings.commit();
        }

        
        void markPilotSuccess( const Reference< XComponentContext >& _rxContext ) SAL_THROW ( ( ) )
        {
            
            const OUString& sAddressBookNodeName = lcl_getAddressBookNodeName();

            
            OConfigurationTreeRoot aAddressBookSettings = OConfigurationTreeRoot::createWithComponentContext(
                _rxContext, sAddressBookNodeName, -1, OConfigurationTreeRoot::CM_UPDATABLE);

            
            aAddressBookSettings.setNodeValue( OUString( "AutoPilotCompleted" ), makeAny( (sal_Bool)sal_True ) );

            
            aAddressBookSettings.commit();
        }

    //.....................................................................
    }   
    //.....................................................................

//.........................................................................
}   
//.........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

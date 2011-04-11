/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_extensions.hxx"
#include "fieldmappingimpl.hxx"
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/sdb/CommandType.hpp>
#include <tools/debug.hxx>
#include <toolkit/unohlp.hxx>
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
    using namespace ::com::sun::star::ui::dialogs;

    //---------------------------------------------------------------------
    static const ::rtl::OUString& lcl_getDriverSettingsNodeName()
    {
        static const ::rtl::OUString s_sDriverSettingsNodeName(RTL_CONSTASCII_USTRINGPARAM( "/org.openoffice.Office.DataAccess/DriverSettings/com.sun.star.comp.sdbc.MozabDriver" ));
        return s_sDriverSettingsNodeName;
    }

    //---------------------------------------------------------------------
    static const ::rtl::OUString& lcl_getAddressBookNodeName()
    {
        static const ::rtl::OUString s_sAddressBookNodeName(RTL_CONSTASCII_USTRINGPARAM( "/org.openoffice.Office.DataAccess/AddressBook" ));
        return s_sAddressBookNodeName;
    }

    //.....................................................................
    namespace fieldmapping
    {
    //.....................................................................

        //-----------------------------------------------------------------
        sal_Bool invokeDialog( const Reference< XMultiServiceFactory >& _rxORB, class Window* _pParent,
            const Reference< XPropertySet >& _rxDataSource, AddressSettings& _rSettings ) SAL_THROW ( ( ) )
        {
            _rSettings.aFieldMapping.clear();

            DBG_ASSERT( _rxORB.is(), "fieldmapping::invokeDialog: invalid service factory!" );
            DBG_ASSERT( _rxDataSource.is(), "fieldmapping::invokeDialog: invalid data source!" );
            if ( !_rxORB.is() || !_rxDataSource.is() )
                return sal_False;

            try
            {
                // ........................................................
                // the parameters for creating the dialog
                Sequence< Any > aArguments(5);
                Any* pArguments = aArguments.getArray();

                // the parent window
                Reference< XWindow > xDialogParent = VCLUnoHelper::GetInterface( _pParent );
                *pArguments++ <<= PropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "ParentWindow" )), -1, makeAny( xDialogParent ), PropertyState_DIRECT_VALUE);

                // the data source to use
                *pArguments++ <<= PropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "DataSource" )), -1, makeAny( _rxDataSource ), PropertyState_DIRECT_VALUE);
                *pArguments++ <<= PropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "DataSourceName" )), -1, makeAny( (sal_Bool)_rSettings.bRegisterDataSource ? _rSettings.sRegisteredDataSourceName : _rSettings.sDataSourceName ), PropertyState_DIRECT_VALUE);

                // the table to use
                *pArguments++ <<= PropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "Command" )), -1, makeAny( _rSettings.sSelectedTable ), PropertyState_DIRECT_VALUE);

                // the title
                ::rtl::OUString sTitle = String( ModuleRes( RID_STR_FIELDDIALOGTITLE ) );
                *pArguments++ <<= PropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "Title" )), -1, makeAny( sTitle ), PropertyState_DIRECT_VALUE);

                // ........................................................
                // create an instance of the dialog service
                static ::rtl::OUString s_sAdressBookFieldAssignmentServiceName(RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.ui.AddressBookSourceDialog" ));
                Reference< XExecutableDialog > xDialog(
                    _rxORB->createInstanceWithArguments( s_sAdressBookFieldAssignmentServiceName, aArguments ),
                    UNO_QUERY
                );
                if ( !xDialog.is( ) )
                {
                    ShowServiceNotAvailableError( _pParent, s_sAdressBookFieldAssignmentServiceName, sal_True );
                    return sal_False;
                }

                // execute the dialog
                if ( xDialog->execute() )
                {
                    // retrieve the field mapping as set by he user
                    Reference< XPropertySet > xDialogProps( xDialog, UNO_QUERY );

                    Sequence< AliasProgrammaticPair > aMapping;
#ifdef DBG_UTIL
                    sal_Bool bSuccess =
#endif
                    xDialogProps->getPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "FieldMapping" )) ) >>= aMapping;
                    DBG_ASSERT( bSuccess, "fieldmapping::invokeDialog: invalid property type for FieldMapping!" );

                    // and copy it into the map
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

        //-----------------------------------------------------------------
        void defaultMapping(  const Reference< XMultiServiceFactory >& _rxORB, MapString2String& _rFieldAssignment ) SAL_THROW ( ( ) )
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
                    // (this list is not complete: both lists of programmatic names are larger in real,
                    // but this list above is the intersection)


                // access the configuration information which the driver uses for determining it's column names
                ::rtl::OUString sDriverAliasesNodeName = lcl_getDriverSettingsNodeName();
                sDriverAliasesNodeName += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "/ColumnAliases" ));

                // create a config node for this
                OConfigurationTreeRoot aDriverFieldAliasing = OConfigurationTreeRoot::createWithServiceFactory(
                    _rxORB, sDriverAliasesNodeName, -1, OConfigurationTreeRoot::CM_READONLY);

                // loop through all programmatic pairs
                DBG_ASSERT( 0 == SAL_N_ELEMENTS( pMappingProgrammatics ) % 2,
                    "fieldmapping::defaultMapping: invalid programmatic map!" );
                // number of pairs
                sal_Int32 nIntersectedProgrammatics = SAL_N_ELEMENTS( pMappingProgrammatics ) / 2;

                const sal_Char** pProgrammatic = pMappingProgrammatics;
                ::rtl::OUString sAddressProgrammatic;
                ::rtl::OUString sDriverProgrammatic;
                ::rtl::OUString sDriverUI;
                for (   sal_Int32 i=0;
                        i < nIntersectedProgrammatics;
                        ++i
                    )
                {
                    sAddressProgrammatic = ::rtl::OUString::createFromAscii( *pProgrammatic++ );
                    sDriverProgrammatic = ::rtl::OUString::createFromAscii( *pProgrammatic++ );

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
                    // the config nodes we're using herein should not do this ....
            }
        }

        //-----------------------------------------------------------------
        void writeTemplateAddressFieldMapping( const Reference< XMultiServiceFactory >& _rxORB, const MapString2String& _rFieldAssignment ) SAL_THROW ( ( ) )
        {
            // want to have a non-const map for easier handling
            MapString2String aFieldAssignment( _rFieldAssignment );

            // access the configuration information which the driver uses for determining it's column names
            const ::rtl::OUString& sAddressBookNodeName = lcl_getAddressBookNodeName();

            // create a config node for this
            OConfigurationTreeRoot aAddressBookSettings = OConfigurationTreeRoot::createWithServiceFactory(
                _rxORB, sAddressBookNodeName, -1, OConfigurationTreeRoot::CM_UPDATABLE);

            OConfigurationNode aFields = aAddressBookSettings.openNode( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "Fields" )) );

            // loop through all existent fields
            Sequence< ::rtl::OUString > aExistentFields = aFields.getNodeNames();
            const ::rtl::OUString* pExistentFields = aExistentFields.getConstArray();
            const ::rtl::OUString* pExistentFieldsEnd = pExistentFields + aExistentFields.getLength();

            const ::rtl::OUString sProgrammaticNodeName(RTL_CONSTASCII_USTRINGPARAM( "ProgrammaticFieldName" ));
            const ::rtl::OUString sAssignedNodeName(RTL_CONSTASCII_USTRINGPARAM( "AssignedFieldName" ));

            for ( ; pExistentFields != pExistentFieldsEnd; ++pExistentFields )
            {
#ifdef DBG_UTIL
                ::rtl::OUString sRedundantProgrammaticName;
                aFields.openNode( *pExistentFields ).getNodeValue( sProgrammaticNodeName ) >>= sRedundantProgrammaticName;
#endif
                DBG_ASSERT( sRedundantProgrammaticName == *pExistentFields,
                    "fieldmapping::writeTemplateAddressFieldMapping: inconsistent config data!" );
                    // there should be a redundancy in the config data .... if this asserts, there isn't anymore!

                // do we have a new alias for the programmatic?
                MapString2StringIterator aPos = aFieldAssignment.find( *pExistentFields );
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
            for (   ConstMapString2StringIterator aNewMapping = aFieldAssignment.begin();
                    aNewMapping != aFieldAssignment.end();
                    ++aNewMapping
                )
            {
                DBG_ASSERT( !aFields.hasByName( aNewMapping->first ),
                    "fieldmapping::writeTemplateAddressFieldMapping: inconsistence!" );
                    // in case the config node for the fields already has the node named <aNewMapping->first>,
                    // the entry should have been removed from aNewMapping (in the above loop)
                OConfigurationNode aNewField =  aFields.createNode( aNewMapping->first );
                aNewField.setNodeValue( sProgrammaticNodeName, makeAny( aNewMapping->first ) );
                aNewField.setNodeValue( sAssignedNodeName, makeAny( aNewMapping->second ) );
            }

            // commit the changes done
            aAddressBookSettings.commit();
        }

    //.....................................................................
    }   // namespace fieldmapping
    //.....................................................................

    //.....................................................................
    namespace addressconfig
    {
    //.....................................................................

        //-----------------------------------------------------------------
        void writeTemplateAddressSource( const Reference< XMultiServiceFactory >& _rxORB,
            const ::rtl::OUString& _rDataSourceName, const ::rtl::OUString& _rTableName ) SAL_THROW ( ( ) )
        {
            // access the configuration information which the driver uses for determining it's column names
            const ::rtl::OUString& sAddressBookNodeName = lcl_getAddressBookNodeName();

            // create a config node for this
            OConfigurationTreeRoot aAddressBookSettings = OConfigurationTreeRoot::createWithServiceFactory(
                _rxORB, sAddressBookNodeName, -1, OConfigurationTreeRoot::CM_UPDATABLE);

            aAddressBookSettings.setNodeValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "DataSourceName" )), makeAny( _rDataSourceName ) );
            aAddressBookSettings.setNodeValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "Command" )), makeAny( _rTableName ) );
            aAddressBookSettings.setNodeValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "CommandType" )), makeAny( (sal_Int32)CommandType::TABLE ) );

            // commit the changes done
            aAddressBookSettings.commit();
        }

        //-----------------------------------------------------------------
        void markPilotSuccess( const Reference< XMultiServiceFactory >& _rxORB ) SAL_THROW ( ( ) )
        {
            // access the configuration information which the driver uses for determining it's column names
            const ::rtl::OUString& sAddressBookNodeName = lcl_getAddressBookNodeName();

            // create a config node for this
            OConfigurationTreeRoot aAddressBookSettings = OConfigurationTreeRoot::createWithServiceFactory(
                _rxORB, sAddressBookNodeName, -1, OConfigurationTreeRoot::CM_UPDATABLE);

            // set the flag
            aAddressBookSettings.setNodeValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "AutoPilotCompleted" )), makeAny( (sal_Bool)sal_True ) );

            // commit the changes done
            aAddressBookSettings.commit();
        }

    //.....................................................................
    }   // namespace addressconfig
    //.....................................................................

//.........................................................................
}   // namespace abp
//.........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

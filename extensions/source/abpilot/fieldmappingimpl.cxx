/*************************************************************************
 *
 *  $RCSfile: fieldmappingimpl.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-25 16:00:49 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef EXTENSIONS_ABP_FIELDMAPPINGIMPL_HXX
#include "fieldmappingimpl.hxx"
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_UI_DIALOGS_XEXECUTABLEDIALOG_HPP_
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XWINDOW_HPP_
#include <com/sun/star/awt/XWindow.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_COMMANDTYPE_HPP_
#include <com/sun/star/sdb/CommandType.hpp>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _TOOLKIT_HELPER_VCLUNOHELPER_HXX_
#include <toolkit/unohlp.hxx>
#endif
#ifndef _VCL_STDTEXT_HXX
#include <vcl/stdtext.hxx>
#endif
#ifndef _COM_SUN_STAR_UTIL_ALIASPROGRAMMATICPAIR_HPP_
#include <com/sun/star/util/AliasProgrammaticPair.hpp>
#endif
#ifndef EXTENSIONS_ABPRESID_HRC
#include "abpresid.hrc"
#endif
#ifndef _EXTENSIONS_COMPONENT_MODULE_HXX_
#include "componentmodule.hxx"
#endif
#ifndef _UNOTOOLS_CONFIGNODE_HXX_
#include <unotools/confignode.hxx>
#endif

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
        static const ::rtl::OUString s_sDriverSettingsNodeName =
            ::rtl::OUString::createFromAscii( "/org.openoffice.Office.DataAccess/DriverSettings/com.sun.star.comp.sdbc.MozabDriver" );
        return s_sDriverSettingsNodeName;
    }

    //---------------------------------------------------------------------
    static const ::rtl::OUString& lcl_getAddressBookNodeName()
    {
        static const ::rtl::OUString s_sAddressBookNodeName =
            ::rtl::OUString::createFromAscii( "/org.openoffice.Office.DataAccess/AddressBook" );
        return s_sAddressBookNodeName;
    }

    //.....................................................................
    namespace fieldmapping
    {
    //.....................................................................

        //-----------------------------------------------------------------
        sal_Bool invokeDialog( const Reference< XMultiServiceFactory >& _rxORB, class Window* _pParent,
            const ::rtl::OUString& _rDataSourceName, const ::rtl::OUString& _rTableName,
                MapString2String& _rFieldAssignment ) SAL_THROW ( ( ) )
        {
            _rFieldAssignment.clear();

            DBG_ASSERT( _rxORB.is(), "fieldmapping::invokeDialog: invalid service factory!" );
            try
            {
                // ........................................................
                // the parameters for creating the dialog
                Sequence< Any > aArguments(4);
                Any* pArguments = aArguments.getArray();

                // the parent window
                Reference< XWindow > xDialogParent = VCLUnoHelper::GetInterface( _pParent );
                *pArguments++ <<= PropertyValue(::rtl::OUString::createFromAscii( "ParentWindow" ), -1, makeAny( xDialogParent ), PropertyState_DIRECT_VALUE);

                // the data source to use
                *pArguments++ <<= PropertyValue(::rtl::OUString::createFromAscii( "DataSource" ), -1, makeAny( _rDataSourceName ), PropertyState_DIRECT_VALUE);

                // the table to use
                *pArguments++ <<= PropertyValue(::rtl::OUString::createFromAscii( "Command" ), -1, makeAny( _rTableName ), PropertyState_DIRECT_VALUE);

                // the title
                ::rtl::OUString sTitle = String( ModuleRes( RID_STR_FIELDDIALOGTITLE ) );
                *pArguments++ <<= PropertyValue(::rtl::OUString::createFromAscii( "Title" ), -1, makeAny( sTitle ), PropertyState_DIRECT_VALUE);

                // ........................................................
                // create an instance of the dialog service
                static ::rtl::OUString s_sAdressBookFieldAssignmentServiceName = ::rtl::OUString::createFromAscii( "com.sun.star.ui.AddressBookSourceDialog" );
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
                    xDialogProps->getPropertyValue( ::rtl::OUString::createFromAscii( "FieldMapping" ) ) >>= aMapping;
                    DBG_ASSERT( bSuccess, "fieldmapping::invokeDialog: invalid property type for FieldMapping!" );

                    // and copy it into the map
                    const AliasProgrammaticPair* pMapping = aMapping.getConstArray();
                    const AliasProgrammaticPair* pMappingEnd = pMapping + aMapping.getLength();
                    for (;pMapping != pMappingEnd; ++pMapping)
                        _rFieldAssignment[ pMapping->ProgrammaticName ] = pMapping->Alias;

                    return sal_True;
                }

            }
            catch(const Exception&)
            {
                DBG_ERROR("fieldmapping::invokeDialog: caught an exception while executing the dialog!");
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
                sDriverAliasesNodeName += ::rtl::OUString::createFromAscii( "/ColumnAliases" );

                // create a config node for this
                OConfigurationTreeRoot aDriverFieldAliasing = OConfigurationTreeRoot::createWithServiceFactory(
                    _rxORB, sDriverAliasesNodeName, -1, OConfigurationTreeRoot::CM_READONLY);

                // loop through all programmatic pairs
                DBG_ASSERT( 0 == ( sizeof( pMappingProgrammatics ) / sizeof( pMappingProgrammatics[ 0 ] ) ) % 2,
                    "fieldmapping::defaultMapping: invalid programmatic map!" );
                // number of pairs
                sal_Int32 nIntersectedProgrammatics = sizeof( pMappingProgrammatics ) / sizeof( pMappingProgrammatics[ 0 ] ) / 2;

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
                            DBG_ERROR( "fieldmapping::defaultMapping: invalid driver UI column name!");
                        }
                        else
                            _rFieldAssignment[ sAddressProgrammatic ] = sDriverUI;
                    }
                    else
                        DBG_ERROR( "fieldmapping::defaultMapping: invalid driver programmatic name!" );
                }
            }
            catch( const Exception& )
            {
                DBG_ERROR("fieldmapping::defaultMapping: code is assumed to throw no exceptions!");
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

            OConfigurationNode aFields = aAddressBookSettings.openNode( ::rtl::OUString::createFromAscii( "Fields" ) );

            // loop through all existent fields
            Sequence< ::rtl::OUString > aExistentFields = aFields.getNodeNames();
            const ::rtl::OUString* pExistentFields = aExistentFields.getConstArray();
            const ::rtl::OUString* pExistentFieldsEnd = pExistentFields + aExistentFields.getLength();

            const ::rtl::OUString sProgrammaticNodeName = ::rtl::OUString::createFromAscii( "ProgrammaticFieldName" );
            const ::rtl::OUString sAssignedNodeName = ::rtl::OUString::createFromAscii( "AssignedFieldName" );

            for (pExistentFields; pExistentFields != pExistentFieldsEnd; ++pExistentFields)
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

            aAddressBookSettings.setNodeValue( ::rtl::OUString::createFromAscii( "DataSourceName" ), makeAny( _rDataSourceName ) );
            aAddressBookSettings.setNodeValue( ::rtl::OUString::createFromAscii( "Command" ), makeAny( _rTableName ) );
            aAddressBookSettings.setNodeValue( ::rtl::OUString::createFromAscii( "CommandType" ), makeAny( (sal_Int32)CommandType::TABLE ) );

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
            aAddressBookSettings.setNodeValue( ::rtl::OUString::createFromAscii( "AutoPilotCompleted" ), makeAny( (sal_Bool)sal_True ) );

            // commit the changes done
            aAddressBookSettings.commit();
        }

    //.....................................................................
    }   // namespace addressconfig
    //.....................................................................

//.........................................................................
}   // namespace abp
//.........................................................................


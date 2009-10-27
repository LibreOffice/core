/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: DriverSettings.cxx,v $
 * $Revision: 1.17.18.1 $
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
#include "precompiled_dbaccess.hxx"

#include "DriverSettings.hxx"
#include "dsitems.hxx"
#include "datasourceui.hxx"
#include <connectivity/DriversConfig.hxx>

#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/beans/NamedValue.hpp>

using ::com::sun::star::uno::Sequence;
using ::com::sun::star::beans::NamedValue;

using namespace dbaui;
void ODriversSettings::getSupportedIndirectSettings( const ::rtl::OUString& _sURLPrefix,const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _xFactory, ::std::vector< sal_Int32>& _out_rDetailsIds )
{
    // for a number of settings, we do not need to use hard-coded here, but can ask a
    // central DataSourceUI instance.
    // TODO: isn't DataSourceUI obsolete, now that this is in the configuration?
    DataSourceMetaData aMeta(_sURLPrefix);
    DataSourceUI aDSUI( aMeta );
    const USHORT nGenericKnownSettings[] =
    {
         DSID_SQL92CHECK,
         DSID_APPEND_TABLE_ALIAS,
         DSID_AS_BEFORE_CORRNAME,
         DSID_ENABLEOUTERJOIN,
         DSID_IGNOREDRIVER_PRIV,
         DSID_PARAMETERNAMESUBST,
         DSID_SUPPRESSVERSIONCL,
         DSID_CATALOG,
         DSID_SCHEMA,
         DSID_INDEXAPPENDIX,
         DSID_CHECK_REQUIRED_FIELDS,
         DSID_AUTORETRIEVEENABLED,
         DSID_AUTOINCREMENTVALUE,
         DSID_AUTORETRIEVEVALUE,
         DSID_BOOLEANCOMPARISON,
         DSID_ESCAPE_DATETIME,
         0
    };
    for ( const USHORT* pGenericKnowSetting = nGenericKnownSettings; *pGenericKnowSetting; ++pGenericKnowSetting )
        if ( aDSUI.hasSetting( *pGenericKnowSetting ) )
            _out_rDetailsIds.push_back( *pGenericKnowSetting );

    // the rest is configuration-based
    ::connectivity::DriversConfig aDriverConfig(_xFactory);
    const ::comphelper::NamedValueCollection& aProperties = aDriverConfig.getProperties(_sURLPrefix);
#if OSL_DEBUG_LEVEL > 0
    {
        Sequence< NamedValue > aNamedValues;
        aProperties >>= aNamedValues;
        for (   const NamedValue* loop = aNamedValues.getConstArray();
                loop != aNamedValues.getConstArray() + aNamedValues.getLength();
                ++loop
            )
        {
            int dummy = 0;
            (void)dummy;
        }
    }
#endif
    typedef ::std::pair<USHORT, ::rtl::OUString> TProperties;
    TProperties aProps[] = { TProperties(DSID_SHOWDELETEDROWS,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ShowDeleted")))
                            ,TProperties(DSID_CHARSET,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CharSet")))
                            ,TProperties(DSID_FIELDDELIMITER,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("FieldDelimiter")))
                            ,TProperties(DSID_TEXTDELIMITER,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("StringDelimiter")))
                            ,TProperties(DSID_DECIMALDELIMITER,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DecimalDelimiter")))
                            ,TProperties(DSID_THOUSANDSDELIMITER,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ThousandDelimiter")))
                            ,TProperties(DSID_TEXTFILEEXTENSION,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Extension")))
                            ,TProperties(DSID_TEXTFILEHEADER,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("HeaderLine")))
                            ,TProperties(DSID_ADDITIONALOPTIONS,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SystemDriverSettings")))
                            ,TProperties(DSID_CONN_SHUTSERVICE,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ShutdownDatabase")))
                            ,TProperties(DSID_CONN_DATAINC,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DataCacheSizeIncrement")))
                            ,TProperties(DSID_CONN_CACHESIZE,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DataCacheSize")))
                            ,TProperties(DSID_CONN_CTRLUSER,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ControlUser")))
                            ,TProperties(DSID_CONN_CTRLPWD,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ControlPassword")))
                            ,TProperties(DSID_USECATALOG,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("UseCatalog")))
                            ,TProperties(DSID_CONN_SOCKET,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("LocalSocket")))
                            ,TProperties(DSID_NAMED_PIPE,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("NamedPipe")))
                            ,TProperties(DSID_JDBCDRIVERCLASS,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("JavaDriverClass")))
                            ,TProperties(DSID_CONN_LDAP_BASEDN,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("BaseDN")))
                            ,TProperties(DSID_CONN_LDAP_ROWCOUNT,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("MaxRowCount")))
                            ,TProperties(DSID_CONN_LDAP_USESSL,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("UseSSL")))
                            ,TProperties(DSID_IGNORECURRENCY,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("IgnoreCurrency")))
                            ,TProperties(0,::rtl::OUString())
    };
    // TODO: This mapping between IDs and property names already exists - in ODbDataSourceAdministrationHelper::ODbDataSourceAdministrationHelper.
    // We should not duplicate it here.
    for ( TProperties* pProps = aProps; pProps->first; ++pProps )
    {
        if ( aProperties.has(pProps->second) )
            _out_rDetailsIds.push_back(pProps->first);
    }
}

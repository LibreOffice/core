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


#include "DriverSettings.hxx"
#include "dsmeta.hxx"

#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/beans/NamedValue.hpp>

#include <connectivity/DriversConfig.hxx>

using ::com::sun::star::uno::Sequence;
using ::com::sun::star::beans::NamedValue;

using namespace dbaui;
void ODriversSettings::getSupportedIndirectSettings( const ::rtl::OUString& _sURLPrefix,const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _xContext, ::std::vector< sal_Int32>& _out_rDetailsIds )
{
    // for a number of settings, we do not need to use hard-coded here, but can ask a
    // central DataSourceUI instance.
    DataSourceMetaData aMeta( _sURLPrefix );
    const FeatureSet& rFeatures( aMeta.getFeatureSet() );
    for (   FeatureSet::const_iterator feature = rFeatures.begin();
            feature != rFeatures.end();
            ++feature
        )
    {
        _out_rDetailsIds.push_back( *feature );
    }

    // the rest is configuration-based
    // TODO: that's not really true: *everything* is configuration-based nowadays, even the FeatureSet obtained
    // from the DataSourceMetaData has been initialized from the configuration. So in fact, we could consolidate
    // the two blocks.
    // The best approach would be to extend the FeatureSet to contain *all* known data source features, not only
    // the ones from the "Advanced settings" UI.

    ::connectivity::DriversConfig aDriverConfig(_xContext);
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
    typedef ::std::pair<sal_uInt16, ::rtl::OUString> TProperties;
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
    // Another mapping (which is also duplicated in ODbDataSourceAdministrationHelper) exists in dsmeta.cxx. We should
    // consolidate those three places into one.
    // However, care has to be taken: We need to distinguish between "features" and "properties" of a data source (resp. driver).
    // That is, a driver can support a certain property, but not allow to change it in the UI, which means it would
    // not have the respective "feature".
    for ( TProperties* pProps = aProps; pProps->first; ++pProps )
    {
        if ( aProperties.has(pProps->second) )
            _out_rDetailsIds.push_back(pProps->first);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

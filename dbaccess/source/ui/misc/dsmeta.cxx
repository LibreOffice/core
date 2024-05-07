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

#include <dsmeta.hxx>
#include <connectivity/DriversConfig.hxx>
#include <dsntypes.hxx>
#include <comphelper/processfactory.hxx>

#include <map>
#include <utility>

namespace dbaui
{

    using namespace ::com::sun::star;

    namespace {

    struct FeatureSupport
    {
        // authentication mode of the data source
        AuthenticationMode      eAuthentication;

        FeatureSupport()
            :eAuthentication( AuthUserPwd )
        {
        }

        explicit FeatureSupport(AuthenticationMode Auth)
            :eAuthentication( Auth )
        {
        }
    };

    struct FeatureMapping
    {
        /// one of the items from dsitems.hxx
        ItemID      nItemID;
        OUString pAsciiFeatureName;
    };

    // global tables
    const FeatureMapping s_aMappings[] = {
        { DSID_AUTORETRIEVEENABLED,     u"GeneratedValues"_ustr },
        { DSID_AUTOINCREMENTVALUE,      u"GeneratedValues"_ustr },
        { DSID_AUTORETRIEVEVALUE,       u"GeneratedValues"_ustr },
        { DSID_SQL92CHECK,              u"UseSQL92NamingConstraints"_ustr },
        { DSID_APPEND_TABLE_ALIAS,      u"AppendTableAliasInSelect"_ustr },
        { DSID_AS_BEFORE_CORRNAME,      u"UseKeywordAsBeforeAlias"_ustr },
        { DSID_ENABLEOUTERJOIN,         u"UseBracketedOuterJoinSyntax"_ustr },
        { DSID_IGNOREDRIVER_PRIV,       u"IgnoreDriverPrivileges"_ustr },
        { DSID_PARAMETERNAMESUBST,      u"ParameterNameSubstitution"_ustr },
        { DSID_SUPPRESSVERSIONCL,       u"DisplayVersionColumns"_ustr },
        { DSID_CATALOG,                 u"UseCatalogInSelect"_ustr },
        { DSID_SCHEMA,                  u"UseSchemaInSelect"_ustr },
        { DSID_INDEXAPPENDIX,           u"UseIndexDirectionKeyword"_ustr },
        { DSID_DOSLINEENDS,             u"UseDOSLineEnds"_ustr },
        { DSID_BOOLEANCOMPARISON,       u"BooleanComparisonMode"_ustr },
        { DSID_CHECK_REQUIRED_FIELDS,   u"FormsCheckRequiredFields"_ustr },
        { DSID_IGNORECURRENCY,          u"IgnoreCurrency"_ustr },
        { DSID_ESCAPE_DATETIME,         u"EscapeDateTime"_ustr },
        { DSID_PRIMARY_KEY_SUPPORT,     u"PrimaryKeySupport"_ustr },
        { DSID_RESPECTRESULTSETTYPE,    u"RespectDriverResultSetType"_ustr },
        { DSID_MAX_ROW_SCAN,            u"MaxRowScan"_ustr },
    };
    }

    static const FeatureSet& lcl_getFeatureSet( const OUString& _rURL )
    {
        typedef std::map< OUString, FeatureSet > FeatureSets;
        static FeatureSets s_aFeatureSets = []()
        {
            FeatureSets tmp;
            ::connectivity::DriversConfig aDriverConfig( ::comphelper::getProcessComponentContext() );
            const uno::Sequence< OUString > aPatterns = aDriverConfig.getURLs();
            for ( auto const & pattern : aPatterns )
            {
                FeatureSet aCurrentSet;
                const ::comphelper::NamedValueCollection aCurrentFeatures( aDriverConfig.getFeatures( pattern ).getNamedValues() );

                for ( const FeatureMapping& rFeatureMapping : s_aMappings )
                {
                    if ( aCurrentFeatures.has( rFeatureMapping.pAsciiFeatureName ) )
                        aCurrentSet.put( rFeatureMapping.nItemID );
                }

                tmp[ pattern ] = aCurrentSet;
            }
            return tmp;
        }();

        OSL_ENSURE( s_aFeatureSets.find( _rURL ) != s_aFeatureSets.end(), "invalid URL/pattern!" );
        return s_aFeatureSets[ _rURL ];
    }

    static AuthenticationMode getAuthenticationMode( const OUString& _sURL )
    {
        static std::map< OUString, FeatureSupport > s_aSupport = []()
        {
            std::map< OUString, FeatureSupport > tmp;
            ::connectivity::DriversConfig aDriverConfig(::comphelper::getProcessComponentContext());
            for (auto& url : aDriverConfig.getURLs())
            {
                FeatureSupport aInit( AuthNone );
                const ::comphelper::NamedValueCollection& aMetaData = aDriverConfig.getMetaData(url);
                if ( aMetaData.has(u"Authentication"_ustr) )
                {
                    OUString sAuth;
                    aMetaData.get(u"Authentication"_ustr) >>= sAuth;
                    if ( sAuth == "UserPassword" )
                        aInit = FeatureSupport(AuthUserPwd);
                    else if ( sAuth == "Password" )
                        aInit = FeatureSupport(AuthPwd);
                }
                tmp.insert(std::make_pair(url, aInit));
            }
            return tmp;
        }();
        OSL_ENSURE(s_aSupport.find(_sURL) != s_aSupport.end(),"Illegal URL!");
        return s_aSupport[ _sURL ].eAuthentication;
    }

    // DataSourceMetaData
    DataSourceMetaData::DataSourceMetaData( const OUString& _sURL )
        :m_sURL(  _sURL )
    {
    }

    DataSourceMetaData::~DataSourceMetaData()
    {
    }

    const FeatureSet& DataSourceMetaData::getFeatureSet() const
    {
        return lcl_getFeatureSet( m_sURL );
    }

    AuthenticationMode  DataSourceMetaData::getAuthentication( const OUString& _sURL )
    {
        return getAuthenticationMode( _sURL );
    }

} // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

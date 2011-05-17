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

#include "dsmeta.hxx"
#include <connectivity/DriversConfig.hxx>
#include "dsntypes.hxx"
#include <comphelper/processfactory.hxx>
/** === begin UNO includes === **/
/** === end UNO includes === **/

#include <map>

//........................................................................
namespace dbaui
{
//........................................................................

    /** === begin UNO using === **/
    using namespace dbaccess;
    using namespace ::com::sun::star;
    /** === end UNO using === **/

    struct FeatureSupport
    {
        // authentication mode of the data source
        AuthenticationMode      eAuthentication;

        FeatureSupport()
            :eAuthentication( AuthUserPwd )
        {
        }

        FeatureSupport( AuthenticationMode _Auth )
            :eAuthentication( _Auth )
        {
        }
    };

    struct FeatureMapping
    {
        /// one of the items from dsitems.hxx
        ItemID          nItemID;
        const sal_Char* pAsciiFeatureName;
    };

    //====================================================================
    //= global tables
    //====================================================================
    //--------------------------------------------------------------------
    static const FeatureMapping* lcl_getFeatureMappings()
    {
        static const FeatureMapping s_aMappings[] = {
            { DSID_AUTORETRIEVEENABLED,     "GeneratedValues" },
            { DSID_AUTOINCREMENTVALUE,      "GeneratedValues" },
            { DSID_AUTORETRIEVEVALUE,       "GeneratedValues" },
            { DSID_SQL92CHECK,              "UseSQL92NamingConstraints" },
            { DSID_APPEND_TABLE_ALIAS,      "AppendTableAliasInSelect" },
            { DSID_AS_BEFORE_CORRNAME,      "UseKeywordAsBeforeAlias" },
            { DSID_ENABLEOUTERJOIN,         "UseBracketedOuterJoinSyntax" },
            { DSID_IGNOREDRIVER_PRIV,       "IgnoreDriverPrivileges" },
            { DSID_PARAMETERNAMESUBST,      "ParameterNameSubstitution" },
            { DSID_SUPPRESSVERSIONCL,       "DisplayVersionColumns" },
            { DSID_CATALOG,                 "UseCatalogInSelect" },
            { DSID_SCHEMA,                  "UseSchemaInSelect" },
            { DSID_INDEXAPPENDIX,           "UseIndexDirectionKeyword" },
            { DSID_DOSLINEENDS,             "UseDOSLineEnds" },
            { DSID_BOOLEANCOMPARISON,       "BooleanComparisonMode" },
            { DSID_CHECK_REQUIRED_FIELDS,   "FormsCheckRequiredFields" },
            { DSID_IGNORECURRENCY,          "IgnoreCurrency" },
            { DSID_ESCAPE_DATETIME,         "EscapeDateTime" },
            { DSID_PRIMARY_KEY_SUPPORT,     "PrimaryKeySupport" },
            { DSID_MAX_ROW_SCAN,            "MaxRowScan" },
            { 0, NULL }
        };
        return s_aMappings;
    }

    //--------------------------------------------------------------------
    static const FeatureSet& lcl_getFeatureSet( const ::rtl::OUString _rURL )
    {
        typedef ::std::map< ::rtl::OUString, FeatureSet, ::comphelper::UStringLess >    FeatureSets;
        static FeatureSets s_aFeatureSets;
        if ( s_aFeatureSets.empty() )
        {
            ::connectivity::DriversConfig aDriverConfig( ::comphelper::getProcessServiceFactory() );
            const uno::Sequence< ::rtl::OUString > aPatterns = aDriverConfig.getURLs();
            for (   const ::rtl::OUString* pattern = aPatterns.getConstArray();
                    pattern != aPatterns.getConstArray() + aPatterns.getLength();
                    ++pattern
                )
            {
                FeatureSet aCurrentSet;
                const ::comphelper::NamedValueCollection aCurrentFeatures( aDriverConfig.getFeatures( *pattern ).getNamedValues() );

                const FeatureMapping* pFeatureMapping = lcl_getFeatureMappings();
                while ( pFeatureMapping->pAsciiFeatureName )
                {
                    if ( aCurrentFeatures.has( pFeatureMapping->pAsciiFeatureName ) )
                        aCurrentSet.put( pFeatureMapping->nItemID );
                    ++pFeatureMapping;
                }

                s_aFeatureSets[ *pattern ] = aCurrentSet;
            }
        }

        OSL_ENSURE( s_aFeatureSets.find( _rURL ) != s_aFeatureSets.end(), "invalid URL/pattern!" );
        return s_aFeatureSets[ _rURL ];
    }

    //--------------------------------------------------------------------
    static AuthenticationMode getAuthenticationMode( const ::rtl::OUString& _sURL )
    {
        DECLARE_STL_USTRINGACCESS_MAP( FeatureSupport, Supported);
        static Supported s_aSupport;
        if ( s_aSupport.empty() )
        {
            ::connectivity::DriversConfig aDriverConfig(::comphelper::getProcessServiceFactory());
            const uno::Sequence< ::rtl::OUString > aURLs = aDriverConfig.getURLs();
            const ::rtl::OUString* pIter = aURLs.getConstArray();
            const ::rtl::OUString* pEnd = pIter + aURLs.getLength();
            for(;pIter != pEnd;++pIter)
            {
                FeatureSupport aInit( AuthNone );
                const ::comphelper::NamedValueCollection& aMetaData = aDriverConfig.getMetaData(*pIter);
                if ( aMetaData.has("Authentication") )
                {
                    ::rtl::OUString sAuth;
                    aMetaData.get("Authentication") >>= sAuth;
                    if ( sAuth.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("UserPassword")) )
                        aInit = AuthUserPwd;
                    else if ( sAuth.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("Password")) )
                        aInit = AuthPwd;
                }
                s_aSupport.insert(Supported::value_type(*pIter,aInit));
            }
        }
        OSL_ENSURE(s_aSupport.find(_sURL) != s_aSupport.end(),"Illegal URL!");
        return s_aSupport[ _sURL ].eAuthentication;
    }

    //====================================================================
    //= DataSourceMetaData_Impl
    //====================================================================
    class DataSourceMetaData_Impl
    {
    public:
        DataSourceMetaData_Impl( const ::rtl::OUString& _sURL );

        inline ::rtl::OUString getType() const { return m_sURL; }

    private:
        const ::rtl::OUString m_sURL;
    };

    //--------------------------------------------------------------------
    DataSourceMetaData_Impl::DataSourceMetaData_Impl( const ::rtl::OUString& _sURL )
        :m_sURL( _sURL )
    {
    }

    //====================================================================
    //= DataSourceMetaData
    //====================================================================
    //--------------------------------------------------------------------
    DataSourceMetaData::DataSourceMetaData( const ::rtl::OUString& _sURL )
        :m_pImpl( new DataSourceMetaData_Impl( _sURL ) )
    {
    }

    //--------------------------------------------------------------------
    DataSourceMetaData::~DataSourceMetaData()
    {
    }

    //--------------------------------------------------------------------
    const FeatureSet& DataSourceMetaData::getFeatureSet() const
    {
        return lcl_getFeatureSet( m_pImpl->getType() );
    }

    //--------------------------------------------------------------------
    AuthenticationMode  DataSourceMetaData::getAuthentication( const ::rtl::OUString& _sURL )
    {
        return getAuthenticationMode( _sURL );
    }

//........................................................................
} // namespace dbaui
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

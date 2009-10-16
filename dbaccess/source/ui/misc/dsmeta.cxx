/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: dsmeta.cxx,v $
 * $Revision: 1.8.8.2 $
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

    //====================================================================
    //= global tables
    //====================================================================
    //--------------------------------------------------------------------
    static const AdvancedSettingsSupport& getAdvancedSettingsSupport( const ::rtl::OUString& _sURL )
    {
        DECLARE_STL_USTRINGACCESS_MAP( AdvancedSettingsSupport, AdvancedSupport);
        static AdvancedSupport s_aSupport;
        if ( s_aSupport.empty() )
        {
            ::connectivity::DriversConfig aDriverConfig(::comphelper::getProcessServiceFactory());
            const uno::Sequence< ::rtl::OUString > aURLs = aDriverConfig.getURLs();
            const ::rtl::OUString* pIter = aURLs.getConstArray();
            const ::rtl::OUString* pEnd = pIter + aURLs.getLength();
            for(;pIter != pEnd;++pIter)
            {
                AdvancedSettingsSupport aInit;
                const uno::Sequence< beans::NamedValue> aProperties = aDriverConfig.getFeatures(*pIter).getNamedValues();
                const beans::NamedValue* pPropertiesIter = aProperties.getConstArray();
                const beans::NamedValue* pPropertiesEnd  = pPropertiesIter + aProperties.getLength();
                for (;pPropertiesIter != pPropertiesEnd ; ++pPropertiesIter)
                {
                    if ( pPropertiesIter->Name.equalsAscii("GeneratedValues") )
                    {
                        pPropertiesIter->Value >>= aInit.bGeneratedValues;
                    }
                    else if ( pPropertiesIter->Name.equalsAscii("UseSQL92NamingConstraints") )
                    {
                        pPropertiesIter->Value >>= aInit.bUseSQL92NamingConstraints;
                    }
                    else if ( pPropertiesIter->Name.equalsAscii("AppendTableAliasInSelect") )
                    {
                        pPropertiesIter->Value >>= aInit.bAppendTableAliasInSelect;
                    }
                    else if ( pPropertiesIter->Name.equalsAscii("UseKeywordAsBeforeAlias") )
                    {
                        pPropertiesIter->Value >>= aInit.bUseKeywordAsBeforeAlias;
                    }
                    else if ( pPropertiesIter->Name.equalsAscii("UseBracketedOuterJoinSyntax") )
                    {
                        pPropertiesIter->Value >>= aInit.bUseBracketedOuterJoinSyntax;
                    }
                    else if ( pPropertiesIter->Name.equalsAscii("IgnoreDriverPrivileges") )
                    {
                        pPropertiesIter->Value >>= aInit.bIgnoreDriverPrivileges;
                    }
                    else if ( pPropertiesIter->Name.equalsAscii("ParameterNameSubstitution") )
                    {
                        pPropertiesIter->Value >>= aInit.bParameterNameSubstitution;
                    }
                    else if ( pPropertiesIter->Name.equalsAscii("DisplayVersionColumns") )
                    {
                        pPropertiesIter->Value >>= aInit.bDisplayVersionColumns;
                    }
                    else if ( pPropertiesIter->Name.equalsAscii("UseCatalogInSelect") )
                    {
                        pPropertiesIter->Value >>= aInit.bUseCatalogInSelect;
                    }
                    else if ( pPropertiesIter->Name.equalsAscii("UseSchemaInSelect") )
                    {
                        pPropertiesIter->Value >>= aInit.bUseSchemaInSelect;
                    }
                    else if ( pPropertiesIter->Name.equalsAscii("UseIndexDirectionKeyword") )
                    {
                        pPropertiesIter->Value >>= aInit.bUseIndexDirectionKeyword;
                    }
                    else if ( pPropertiesIter->Name.equalsAscii("UseDOSLineEnds") )
                    {
                        pPropertiesIter->Value >>= aInit.bUseDOSLineEnds;
                    }
                    else if ( pPropertiesIter->Name.equalsAscii("BooleanComparisonMode") )
                    {
                        pPropertiesIter->Value >>= aInit.bBooleanComparisonMode;
                    }
                    else if ( pPropertiesIter->Name.equalsAscii("FormsCheckRequiredFields") )
                    {
                        pPropertiesIter->Value >>= aInit.bFormsCheckRequiredFields;
                    }
                    else if ( pPropertiesIter->Name.equalsAscii("IgnoreCurrency") )
                    {
                        pPropertiesIter->Value >>= aInit.bIgnoreCurrency;
                    }
                    else if ( pPropertiesIter->Name.equalsAscii("EscapeDateTime") )
                    {
                        pPropertiesIter->Value >>= aInit.bEscapeDateTime;
                    }
                    else if ( pPropertiesIter->Name.equalsAscii("PrimaryKeySupport") )
                    {
                        pPropertiesIter->Value >>= aInit.bPrimaryKeySupport;
                    }
                }
                s_aSupport.insert(AdvancedSupport::value_type(*pIter,aInit));
            }
        }
        OSL_ENSURE(s_aSupport.find(_sURL) != s_aSupport.end(),"Illegal URL!");
        return s_aSupport[ _sURL ];
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
                    if ( sAuth.equalsAscii("UserPassword") )
                        aInit = AuthUserPwd;
                    else if ( sAuth.equalsAscii("Password") )
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
    const AdvancedSettingsSupport& DataSourceMetaData::getAdvancedSettingsSupport() const
    {
        return ::dbaui::getAdvancedSettingsSupport( m_pImpl->getType() );
    }

    //--------------------------------------------------------------------
    AuthenticationMode DataSourceMetaData::getAuthentication() const
    {
        return getAuthenticationMode( m_pImpl->getType() );
    }

    //--------------------------------------------------------------------
    AuthenticationMode  DataSourceMetaData::getAuthentication( const ::rtl::OUString& _sURL )
    {
        return getAuthenticationMode( _sURL );
    }

//........................................................................
} // namespace dbaui
//........................................................................

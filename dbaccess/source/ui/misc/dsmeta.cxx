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

/** === begin UNO includes === **/
/** === end UNO includes === **/

#include <map>

//........................................................................
namespace dbaui
{
//........................................................................

    /** === begin UNO using === **/
    /** === end UNO using === **/

    struct InitAdvanced : public AdvancedSettingsSupport
    {
        // strange ctor, but makes instantiating this class more readable (see below)
        InitAdvanced( short _Generated, short _SQL, short _Append, short _As, short _Outer, short _Priv, short _Param,
                      short _Version, short _Catalog, short _Schema, short _Index, short _DOS, short _Required, short _Bool,short _IgnoreCur,short _AutoPKey, short _EscapeDT )
            :AdvancedSettingsSupport()
        {
            bGeneratedValues               = ( _Generated != 0 );
            bUseSQL92NamingConstraints     = ( _SQL       != 0 );
            bAppendTableAliasInSelect      = ( _Append    != 0 );
            bUseKeywordAsBeforeAlias       = ( _As        != 0 );
            bUseBracketedOuterJoinSyntax   = ( _Outer     != 0 );
            bIgnoreDriverPrivileges        = ( _Priv      != 0 );
            bParameterNameSubstitution     = ( _Param     != 0 );
            bDisplayVersionColumns         = ( _Version   != 0 );
            bUseCatalogInSelect            = ( _Catalog   != 0 );
            bUseSchemaInSelect             = ( _Schema    != 0 );
            bUseIndexDirectionKeyword      = ( _Index     != 0 );
            bUseDOSLineEnds                = ( _DOS       != 0 );
            bBooleanComparisonMode         = ( _Bool      != 0 );
            bFormsCheckRequiredFields      = ( _Required  != 0 );
            bIgnoreCurrency                = ( _IgnoreCur != 0 );
            bAutoIncrementIsPrimaryKey     = ( _AutoPKey  != 0 );
            bEscapeDateTime                = ( _EscapeDT  != 0 );
        }

        enum Special { All, AllButIgnoreCurrency, None };

        InitAdvanced( Special _eType )
            :AdvancedSettingsSupport()
        {
            bGeneratedValues               = ( _eType == All ) || ( _eType == AllButIgnoreCurrency );
            bUseSQL92NamingConstraints     = ( _eType == All ) || ( _eType == AllButIgnoreCurrency );
            bAppendTableAliasInSelect      = ( _eType == All ) || ( _eType == AllButIgnoreCurrency );
            bUseKeywordAsBeforeAlias       = ( _eType == All ) || ( _eType == AllButIgnoreCurrency );
            bUseBracketedOuterJoinSyntax   = ( _eType == All ) || ( _eType == AllButIgnoreCurrency );
            bIgnoreDriverPrivileges        = ( _eType == All ) || ( _eType == AllButIgnoreCurrency );
            bParameterNameSubstitution     = ( _eType == All ) || ( _eType == AllButIgnoreCurrency );
            bDisplayVersionColumns         = ( _eType == All ) || ( _eType == AllButIgnoreCurrency );
            bUseCatalogInSelect            = ( _eType == All ) || ( _eType == AllButIgnoreCurrency );
            bUseSchemaInSelect             = ( _eType == All ) || ( _eType == AllButIgnoreCurrency );
            bUseIndexDirectionKeyword      = ( _eType == All ) || ( _eType == AllButIgnoreCurrency );
            bUseDOSLineEnds                = ( _eType == All ) || ( _eType == AllButIgnoreCurrency );
            bBooleanComparisonMode         = ( _eType == All ) || ( _eType == AllButIgnoreCurrency );
            bFormsCheckRequiredFields      = ( _eType == All ) || ( _eType == AllButIgnoreCurrency );
            bIgnoreCurrency                = ( _eType == All );
            bAutoIncrementIsPrimaryKey     = false; // hsqldb special
            bEscapeDateTime                = ( _eType == All ) || ( _eType == AllButIgnoreCurrency );
        }
    };

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
    static const AdvancedSettingsSupport& getAdvancedSettingsSupport( ::dbaccess::DATASOURCE_TYPE _eType )
    {
        typedef ::std::map< ::dbaccess::DATASOURCE_TYPE, AdvancedSettingsSupport >    AdvancedSupport;

        static AdvancedSupport s_aSupport;
        if ( s_aSupport.empty() )
        {
            s_aSupport[  ::dbaccess::DST_MSACCESS            ] = InitAdvanced( 0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0 );
            s_aSupport[  ::dbaccess::DST_MYSQL_ODBC          ] = InitAdvanced( 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0 );
            s_aSupport[  ::dbaccess::DST_MYSQL_JDBC          ] = InitAdvanced( 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 1 );
            s_aSupport[  ::dbaccess::DST_MYSQL_NATIVE        ] = InitAdvanced( 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0 );
            s_aSupport[  ::dbaccess::DST_ORACLE_JDBC         ] = InitAdvanced( InitAdvanced::All );
            s_aSupport[  ::dbaccess::DST_ADABAS              ] = InitAdvanced( 0, 1, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0 );
            s_aSupport[  ::dbaccess::DST_CALC                ] = InitAdvanced( InitAdvanced::None );
            s_aSupport[  ::dbaccess::DST_DBASE               ] = InitAdvanced( 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0 );
            s_aSupport[  ::dbaccess::DST_FLAT                ] = InitAdvanced( 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 );
            s_aSupport[  ::dbaccess::DST_JDBC                ] = InitAdvanced( InitAdvanced::AllButIgnoreCurrency );
            s_aSupport[  ::dbaccess::DST_ODBC                ] = InitAdvanced( InitAdvanced::AllButIgnoreCurrency );
            s_aSupport[  ::dbaccess::DST_ADO                 ] = InitAdvanced( 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0 );
            s_aSupport[  ::dbaccess::DST_MOZILLA             ] = InitAdvanced( InitAdvanced::None );
            s_aSupport[  ::dbaccess::DST_THUNDERBIRD         ] = InitAdvanced( InitAdvanced::None );
            s_aSupport[  ::dbaccess::DST_LDAP                ] = InitAdvanced( InitAdvanced::None );
            s_aSupport[  ::dbaccess::DST_OUTLOOK             ] = InitAdvanced( InitAdvanced::None );
            s_aSupport[  ::dbaccess::DST_OUTLOOKEXP          ] = InitAdvanced( InitAdvanced::None );
            s_aSupport[  ::dbaccess::DST_EVOLUTION           ] = InitAdvanced( InitAdvanced::None );
            s_aSupport[  ::dbaccess::DST_EVOLUTION_GROUPWISE ] = InitAdvanced( InitAdvanced::None );
            s_aSupport[  ::dbaccess::DST_EVOLUTION_LDAP      ] = InitAdvanced( InitAdvanced::None );
            s_aSupport[  ::dbaccess::DST_KAB                 ] = InitAdvanced( InitAdvanced::None );
            s_aSupport[  ::dbaccess::DST_MACAB               ] = InitAdvanced( InitAdvanced::None );
            s_aSupport[  ::dbaccess::DST_MSACCESS_2007       ] = InitAdvanced( 0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0 );
            s_aSupport[  ::dbaccess::DST_EMBEDDED_HSQLDB     ] = InitAdvanced( 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 0 );
            s_aSupport[  ::dbaccess::DST_USERDEFINE1         ] = InitAdvanced( InitAdvanced::AllButIgnoreCurrency );
            s_aSupport[  ::dbaccess::DST_USERDEFINE2         ] = InitAdvanced( InitAdvanced::AllButIgnoreCurrency );
            s_aSupport[  ::dbaccess::DST_USERDEFINE3         ] = InitAdvanced( InitAdvanced::AllButIgnoreCurrency );
            s_aSupport[  ::dbaccess::DST_USERDEFINE4         ] = InitAdvanced( InitAdvanced::AllButIgnoreCurrency );
            s_aSupport[  ::dbaccess::DST_USERDEFINE5         ] = InitAdvanced( InitAdvanced::AllButIgnoreCurrency );
            s_aSupport[  ::dbaccess::DST_USERDEFINE6         ] = InitAdvanced( InitAdvanced::AllButIgnoreCurrency );
            s_aSupport[  ::dbaccess::DST_USERDEFINE7         ] = InitAdvanced( InitAdvanced::AllButIgnoreCurrency );
            s_aSupport[  ::dbaccess::DST_USERDEFINE8         ] = InitAdvanced( InitAdvanced::AllButIgnoreCurrency );
            s_aSupport[  ::dbaccess::DST_USERDEFINE9         ] = InitAdvanced( InitAdvanced::AllButIgnoreCurrency );
            s_aSupport[  ::dbaccess::DST_USERDEFINE10        ] = InitAdvanced( InitAdvanced::AllButIgnoreCurrency );
        }
        return s_aSupport[ _eType ];
    }

    //--------------------------------------------------------------------
    static AuthenticationMode getAuthenticationMode( ::dbaccess::DATASOURCE_TYPE _eType )
    {
        typedef ::std::map< ::dbaccess::DATASOURCE_TYPE, FeatureSupport >    Supported;

        static Supported s_aSupport;
        if ( s_aSupport.empty() )
        {
            s_aSupport[  ::dbaccess::DST_MSACCESS            ] = FeatureSupport( AuthNone    );
            s_aSupport[  ::dbaccess::DST_MYSQL_NATIVE        ] = FeatureSupport( AuthUserPwd );
            s_aSupport[  ::dbaccess::DST_MYSQL_ODBC          ] = FeatureSupport( AuthUserPwd );
            s_aSupport[  ::dbaccess::DST_MYSQL_JDBC          ] = FeatureSupport( AuthUserPwd );
            s_aSupport[  ::dbaccess::DST_ORACLE_JDBC         ] = FeatureSupport( AuthUserPwd );
            s_aSupport[  ::dbaccess::DST_ADABAS              ] = FeatureSupport( AuthUserPwd );
            s_aSupport[  ::dbaccess::DST_CALC                ] = FeatureSupport( AuthPwd     );
            s_aSupport[  ::dbaccess::DST_DBASE               ] = FeatureSupport( AuthNone    );
            s_aSupport[  ::dbaccess::DST_FLAT                ] = FeatureSupport( AuthNone    );
            s_aSupport[  ::dbaccess::DST_JDBC                ] = FeatureSupport( AuthUserPwd );
            s_aSupport[  ::dbaccess::DST_ODBC                ] = FeatureSupport( AuthUserPwd );
            s_aSupport[  ::dbaccess::DST_ADO                 ] = FeatureSupport( AuthUserPwd );
            s_aSupport[  ::dbaccess::DST_MOZILLA             ] = FeatureSupport( AuthNone    );
            s_aSupport[  ::dbaccess::DST_THUNDERBIRD         ] = FeatureSupport( AuthNone    );
            s_aSupport[  ::dbaccess::DST_LDAP                ] = FeatureSupport( AuthUserPwd );
            s_aSupport[  ::dbaccess::DST_OUTLOOK             ] = FeatureSupport( AuthNone    );
            s_aSupport[  ::dbaccess::DST_OUTLOOKEXP          ] = FeatureSupport( AuthNone    );
            s_aSupport[  ::dbaccess::DST_EVOLUTION           ] = FeatureSupport( AuthNone    );
            s_aSupport[  ::dbaccess::DST_EVOLUTION_GROUPWISE ] = FeatureSupport( AuthNone    );
            s_aSupport[  ::dbaccess::DST_EVOLUTION_LDAP      ] = FeatureSupport( AuthNone    );
            s_aSupport[  ::dbaccess::DST_KAB                 ] = FeatureSupport( AuthNone    );
            s_aSupport[  ::dbaccess::DST_MACAB               ] = FeatureSupport( AuthNone    );
            s_aSupport[  ::dbaccess::DST_MSACCESS_2007       ] = FeatureSupport( AuthNone    );
            s_aSupport[  ::dbaccess::DST_EMBEDDED_HSQLDB     ] = FeatureSupport( AuthNone    );
            s_aSupport[  ::dbaccess::DST_USERDEFINE1         ] = FeatureSupport( AuthUserPwd );
            s_aSupport[  ::dbaccess::DST_USERDEFINE2         ] = FeatureSupport( AuthUserPwd );
            s_aSupport[  ::dbaccess::DST_USERDEFINE3         ] = FeatureSupport( AuthUserPwd );
            s_aSupport[  ::dbaccess::DST_USERDEFINE4         ] = FeatureSupport( AuthUserPwd );
            s_aSupport[  ::dbaccess::DST_USERDEFINE5         ] = FeatureSupport( AuthUserPwd );
            s_aSupport[  ::dbaccess::DST_USERDEFINE6         ] = FeatureSupport( AuthUserPwd );
            s_aSupport[  ::dbaccess::DST_USERDEFINE7         ] = FeatureSupport( AuthUserPwd );
            s_aSupport[  ::dbaccess::DST_USERDEFINE8         ] = FeatureSupport( AuthUserPwd );
            s_aSupport[  ::dbaccess::DST_USERDEFINE9         ] = FeatureSupport( AuthUserPwd );
            s_aSupport[  ::dbaccess::DST_USERDEFINE10        ] = FeatureSupport( AuthUserPwd );
        }
        return s_aSupport[ _eType ].eAuthentication;
    }

    //====================================================================
    //= DataSourceMetaData_Impl
    //====================================================================
    class DataSourceMetaData_Impl
    {
    public:
        DataSourceMetaData_Impl( ::dbaccess::DATASOURCE_TYPE _eType );

        inline ::dbaccess::DATASOURCE_TYPE getType() const { return m_eType; }

    private:
        ::dbaccess::DATASOURCE_TYPE m_eType;
    };

    //--------------------------------------------------------------------
    DataSourceMetaData_Impl::DataSourceMetaData_Impl( ::dbaccess::DATASOURCE_TYPE _eType )
        :m_eType( _eType )
    {
    }

    //====================================================================
    //= DataSourceMetaData
    //====================================================================
    //--------------------------------------------------------------------
    DataSourceMetaData::DataSourceMetaData( ::dbaccess::DATASOURCE_TYPE _eType )
        :m_pImpl( new DataSourceMetaData_Impl( _eType ) )
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
    AuthenticationMode  DataSourceMetaData::getAuthentication( ::dbaccess::DATASOURCE_TYPE _eType )
    {
        return getAuthenticationMode( _eType );
    }

//........................................................................
} // namespace dbaui
//........................................................................

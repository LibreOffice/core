/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dsmeta.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-30 08:53:14 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
                      short _Version, short _Catalog, short _Schema, short _Index, short _DOS, short _Required, short _Bool,short _IgnoreCur )
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
        }

        enum Special { All, None };

        InitAdvanced( Special _eType )
            :AdvancedSettingsSupport()
        {
            bGeneratedValues               = ( _eType == All );
            bUseSQL92NamingConstraints     = ( _eType == All );
            bAppendTableAliasInSelect      = ( _eType == All );
            bUseKeywordAsBeforeAlias       = ( _eType == All );
            bUseBracketedOuterJoinSyntax   = ( _eType == All );
            bIgnoreDriverPrivileges        = ( _eType == All );
            bParameterNameSubstitution     = ( _eType == All );
            bDisplayVersionColumns         = ( _eType == All );
            bUseCatalogInSelect            = ( _eType == All );
            bUseSchemaInSelect             = ( _eType == All );
            bUseIndexDirectionKeyword      = ( _eType == All );
            bUseDOSLineEnds                = ( _eType == All );
            bBooleanComparisonMode         = ( _eType == All );
            bFormsCheckRequiredFields      = ( _eType == All );
            bIgnoreCurrency                = false; // Oracle special
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
    static const AdvancedSettingsSupport& getAdvancedSettingsSupport( DATASOURCE_TYPE _eType )
    {
        typedef ::std::map< DATASOURCE_TYPE, AdvancedSettingsSupport >    AdvancedSupport;

        static AdvancedSupport s_aSupport;
        if ( s_aSupport.empty() )
        {
            s_aSupport[ DST_MSACCESS            ] = InitAdvanced( 0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0 );
            s_aSupport[ DST_MYSQL_ODBC          ] = InitAdvanced( 0, 0, 1, 0, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0 );
            s_aSupport[ DST_MYSQL_JDBC          ] = InitAdvanced( 0, 0, 1, 0, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0 );
            s_aSupport[ DST_ORACLE_JDBC         ] = InitAdvanced( InitAdvanced::All );
            s_aSupport[ DST_ADABAS              ] = InitAdvanced( 0, 1, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 0 );
            s_aSupport[ DST_CALC                ] = InitAdvanced( InitAdvanced::None );
            s_aSupport[ DST_DBASE               ] = InitAdvanced( 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0 );
            s_aSupport[ DST_FLAT                ] = InitAdvanced( 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 );
            s_aSupport[ DST_JDBC                ] = InitAdvanced( InitAdvanced::All );
            s_aSupport[ DST_ODBC                ] = InitAdvanced( InitAdvanced::All );
            s_aSupport[ DST_ADO                 ] = InitAdvanced( 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 0 );
            s_aSupport[ DST_MOZILLA             ] = InitAdvanced( InitAdvanced::None );
            s_aSupport[ DST_THUNDERBIRD         ] = InitAdvanced( InitAdvanced::None );
            s_aSupport[ DST_LDAP                ] = InitAdvanced( InitAdvanced::None );
            s_aSupport[ DST_OUTLOOK             ] = InitAdvanced( InitAdvanced::None );
            s_aSupport[ DST_OUTLOOKEXP          ] = InitAdvanced( InitAdvanced::None );
            s_aSupport[ DST_EVOLUTION           ] = InitAdvanced( InitAdvanced::None );
            s_aSupport[ DST_EVOLUTION_GROUPWISE ] = InitAdvanced( InitAdvanced::None );
            s_aSupport[ DST_EVOLUTION_LDAP      ] = InitAdvanced( InitAdvanced::None );
            s_aSupport[ DST_KAB                 ] = InitAdvanced( InitAdvanced::None );
            s_aSupport[ DST_MSACCESS_2007       ] = InitAdvanced( 0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0 );
            s_aSupport[ DST_EMBEDDED_HSQLDB     ] = InitAdvanced( 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0 );
            s_aSupport[ DST_USERDEFINE1         ] = InitAdvanced( InitAdvanced::All );
            s_aSupport[ DST_USERDEFINE2         ] = InitAdvanced( InitAdvanced::All );
            s_aSupport[ DST_USERDEFINE3         ] = InitAdvanced( InitAdvanced::All );
            s_aSupport[ DST_USERDEFINE4         ] = InitAdvanced( InitAdvanced::All );
            s_aSupport[ DST_USERDEFINE5         ] = InitAdvanced( InitAdvanced::All );
            s_aSupport[ DST_USERDEFINE6         ] = InitAdvanced( InitAdvanced::All );
            s_aSupport[ DST_USERDEFINE7         ] = InitAdvanced( InitAdvanced::All );
            s_aSupport[ DST_USERDEFINE8         ] = InitAdvanced( InitAdvanced::All );
            s_aSupport[ DST_USERDEFINE9         ] = InitAdvanced( InitAdvanced::All );
            s_aSupport[ DST_USERDEFINE10        ] = InitAdvanced( InitAdvanced::All );

            s_aSupport[ DST_ORACLE_JDBC         ].bIgnoreCurrency = true;
        }
        return s_aSupport[ _eType ];
    }

    //--------------------------------------------------------------------
    static AuthenticationMode getAuthenticationMode( DATASOURCE_TYPE _eType )
    {
        typedef ::std::map< DATASOURCE_TYPE, FeatureSupport >    Supported;

        static Supported s_aSupport;
        if ( s_aSupport.empty() )
        {
            s_aSupport[ DST_MSACCESS            ] = FeatureSupport( AuthNone    );
            s_aSupport[ DST_MYSQL_ODBC          ] = FeatureSupport( AuthUserPwd );
            s_aSupport[ DST_MYSQL_JDBC          ] = FeatureSupport( AuthUserPwd );
            s_aSupport[ DST_ORACLE_JDBC         ] = FeatureSupport( AuthUserPwd );
            s_aSupport[ DST_ADABAS              ] = FeatureSupport( AuthUserPwd );
            s_aSupport[ DST_CALC                ] = FeatureSupport( AuthPwd     );
            s_aSupport[ DST_DBASE               ] = FeatureSupport( AuthNone    );
            s_aSupport[ DST_FLAT                ] = FeatureSupport( AuthNone    );
            s_aSupport[ DST_JDBC                ] = FeatureSupport( AuthUserPwd );
            s_aSupport[ DST_ODBC                ] = FeatureSupport( AuthUserPwd );
            s_aSupport[ DST_ADO                 ] = FeatureSupport( AuthUserPwd );
            s_aSupport[ DST_MOZILLA             ] = FeatureSupport( AuthNone    );
            s_aSupport[ DST_THUNDERBIRD         ] = FeatureSupport( AuthNone    );
            s_aSupport[ DST_LDAP                ] = FeatureSupport( AuthUserPwd );
            s_aSupport[ DST_OUTLOOK             ] = FeatureSupport( AuthNone    );
            s_aSupport[ DST_OUTLOOKEXP          ] = FeatureSupport( AuthNone    );
            s_aSupport[ DST_EVOLUTION           ] = FeatureSupport( AuthNone    );
            s_aSupport[ DST_EVOLUTION_GROUPWISE ] = FeatureSupport( AuthNone    );
            s_aSupport[ DST_EVOLUTION_LDAP      ] = FeatureSupport( AuthNone    );
            s_aSupport[ DST_KAB                 ] = FeatureSupport( AuthNone    );
            s_aSupport[ DST_MSACCESS_2007       ] = FeatureSupport( AuthNone    );
            s_aSupport[ DST_EMBEDDED_HSQLDB     ] = FeatureSupport( AuthNone    );
            s_aSupport[ DST_USERDEFINE1         ] = FeatureSupport( AuthUserPwd );
            s_aSupport[ DST_USERDEFINE2         ] = FeatureSupport( AuthUserPwd );
            s_aSupport[ DST_USERDEFINE3         ] = FeatureSupport( AuthUserPwd );
            s_aSupport[ DST_USERDEFINE4         ] = FeatureSupport( AuthUserPwd );
            s_aSupport[ DST_USERDEFINE5         ] = FeatureSupport( AuthUserPwd );
            s_aSupport[ DST_USERDEFINE6         ] = FeatureSupport( AuthUserPwd );
            s_aSupport[ DST_USERDEFINE7         ] = FeatureSupport( AuthUserPwd );
            s_aSupport[ DST_USERDEFINE8         ] = FeatureSupport( AuthUserPwd );
            s_aSupport[ DST_USERDEFINE9         ] = FeatureSupport( AuthUserPwd );
            s_aSupport[ DST_USERDEFINE10        ] = FeatureSupport( AuthUserPwd );
        }
        return s_aSupport[ _eType ].eAuthentication;
    }

    //====================================================================
    //= DataSourceMetaData_Impl
    //====================================================================
    class DataSourceMetaData_Impl
    {
    public:
        DataSourceMetaData_Impl( DATASOURCE_TYPE _eType );

        inline DATASOURCE_TYPE getType() const { return m_eType; }

    private:
        DATASOURCE_TYPE m_eType;
    };

    //--------------------------------------------------------------------
    DataSourceMetaData_Impl::DataSourceMetaData_Impl( DATASOURCE_TYPE _eType )
        :m_eType( _eType )
    {
    }

    //====================================================================
    //= DataSourceMetaData
    //====================================================================
    //--------------------------------------------------------------------
    DataSourceMetaData::DataSourceMetaData( DATASOURCE_TYPE _eType )
        :m_pImpl( new DataSourceMetaData_Impl( _eType ) )
    {
    }

    //--------------------------------------------------------------------
    DataSourceMetaData::~DataSourceMetaData()
    {
    }

    //--------------------------------------------------------------------
    DATASOURCE_TYPE DataSourceMetaData::getType() const
    {
        return m_pImpl->getType();
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
    AuthenticationMode  DataSourceMetaData::getAuthentication( DATASOURCE_TYPE _eType )
    {
        return getAuthenticationMode( _eType );
    }

//........................................................................
} // namespace dbaui
//........................................................................

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
#include "precompiled_dbaccess.hxx"

#include "dsntypes.hxx"
#include "dbamiscres.hrc"
#include <unotools/confignode.hxx>
#include <tools/debug.hxx>
#include <tools/wldcrd.hxx>
#include <osl/file.hxx>
#include "dbastrings.hrc"
#include "core_resource.hxx"
#include "core_resource.hrc"
#include <comphelper/documentconstants.hxx>

//.........................................................................
namespace dbaccess
{
//.........................................................................

    using namespace ::com::sun::star;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::lang;
    //using namespace ::com::sun::star::sdbc;

    namespace
    {
        void lcl_extractHostAndPort(const String& _sUrl,String& _sHostname,sal_Int32& _nPortNumber)
        {
            if ( _sUrl.GetTokenCount(':') >= 2 )
            {
                _sHostname      = _sUrl.GetToken(0,':');
                _nPortNumber    = _sUrl.GetToken(1,':').ToInt32();
            }
        }
    }
//=========================================================================
//= ODsnTypeCollection
//=========================================================================
DBG_NAME(ODsnTypeCollection)
//-------------------------------------------------------------------------
ODsnTypeCollection::ODsnTypeCollection(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _xFactory)
:m_aDriverConfig(_xFactory)
,m_xFactory(_xFactory)
#ifdef DBG_UTIL
,m_nLivingIterators(0)
#endif
{
    DBG_CTOR(ODsnTypeCollection,NULL);
    const uno::Sequence< ::rtl::OUString > aURLs = m_aDriverConfig.getURLs();
    const ::rtl::OUString* pIter = aURLs.getConstArray();
    const ::rtl::OUString* pEnd = pIter + aURLs.getLength();
    for(;pIter != pEnd;++pIter )
    {
        m_aDsnPrefixes.push_back(*pIter);
        m_aDsnTypesDisplayNames.push_back(m_aDriverConfig.getDriverTypeDisplayName(*pIter));
    }

    DBG_ASSERT(m_aDsnTypesDisplayNames.size() == m_aDsnPrefixes.size(),
        "ODsnTypeCollection::ODsnTypeCollection : invalid resources !");
}

//-------------------------------------------------------------------------
ODsnTypeCollection::~ODsnTypeCollection()
{
    DBG_DTOR(ODsnTypeCollection,NULL);
    DBG_ASSERT(0 == m_nLivingIterators, "ODsnTypeCollection::~ODsnTypeCollection : there are still living iterator objects!");
}
//-------------------------------------------------------------------------
String ODsnTypeCollection::getTypeDisplayName(const ::rtl::OUString& _sURL) const
{
    return m_aDriverConfig.getDriverTypeDisplayName(_sURL);
}
//-------------------------------------------------------------------------
String ODsnTypeCollection::cutPrefix(const ::rtl::OUString& _sURL) const
{
    String sURL( _sURL);
    String sRet;
    String sOldPattern;
    StringVector::const_iterator aIter = m_aDsnPrefixes.begin();
    StringVector::const_iterator aEnd = m_aDsnPrefixes.end();
    for(;aIter != aEnd;++aIter)
    {
        WildCard aWildCard(*aIter);
        if ( sOldPattern.Len() < aIter->Len() && aWildCard.Matches(_sURL) )
        {
            if ( aIter->Len() < sURL.Len() )
                sRet = sURL.Copy(sURL.Match(*aIter));
            else
                sRet = sURL.Copy(aIter->Match(sURL));
            sOldPattern = *aIter;
        }
    }

    return sRet;
}

//-------------------------------------------------------------------------
String ODsnTypeCollection::getPrefix(const ::rtl::OUString& _sURL) const
{
    String sURL( _sURL);
    String sRet;
    String sOldPattern;
    StringVector::const_iterator aIter = m_aDsnPrefixes.begin();
    StringVector::const_iterator aEnd = m_aDsnPrefixes.end();
    for(;aIter != aEnd;++aIter)
    {
        WildCard aWildCard(*aIter);
        if ( sOldPattern.Len() < aIter->Len() && aWildCard.Matches(sURL) )
        {
            if ( aIter->Len() < sURL.Len() )
                sRet = aIter->Copy(0,sURL.Match(*aIter));
            else
                sRet = sURL.Copy(0,aIter->Match(sURL));
            sRet.EraseTrailingChars('*');
            sOldPattern = *aIter;
        }
    }

    return sRet;
}

//-------------------------------------------------------------------------
bool ODsnTypeCollection::hasDriver( const sal_Char* _pAsciiPattern ) const
{
    String sPrefix( getPrefix( ::rtl::OUString::createFromAscii( _pAsciiPattern ) ) );
    return ( sPrefix.Len() > 0 );
}

// -----------------------------------------------------------------------------
bool ODsnTypeCollection::isConnectionUrlRequired(const ::rtl::OUString& _sURL) const
{
    String sURL( _sURL);
    String sRet;
    String sOldPattern;
    StringVector::const_iterator aIter = m_aDsnPrefixes.begin();
    StringVector::const_iterator aEnd = m_aDsnPrefixes.end();
    for(;aIter != aEnd;++aIter)
    {
        WildCard aWildCard(*aIter);
        if ( sOldPattern.Len() < aIter->Len() && aWildCard.Matches(sURL) )
        {
            sRet = *aIter;
            sOldPattern = *aIter;
        }
    } // for(;aIter != aEnd;++aIter)
    return sRet.GetChar(sRet.Len()-1) == '*';
}
// -----------------------------------------------------------------------------
String ODsnTypeCollection::getMediaType(const ::rtl::OUString& _sURL) const
{
    const ::comphelper::NamedValueCollection& aFeatures = m_aDriverConfig.getMetaData(_sURL);
    return aFeatures.getOrDefault("MediaType",::rtl::OUString());
}
// -----------------------------------------------------------------------------
String ODsnTypeCollection::getDatasourcePrefixFromMediaType(const ::rtl::OUString& _sMediaType,const ::rtl::OUString& _sExtension)
{
    String sURL, sFallbackURL;
    const uno::Sequence< ::rtl::OUString > aURLs = m_aDriverConfig.getURLs();
    const ::rtl::OUString* pIter = aURLs.getConstArray();
    const ::rtl::OUString* pEnd = pIter + aURLs.getLength();
    for(;pIter != pEnd;++pIter )
    {
        const ::comphelper::NamedValueCollection& aFeatures = m_aDriverConfig.getMetaData(*pIter);
        if ( aFeatures.getOrDefault("MediaType",::rtl::OUString()) == _sMediaType )
        {
            const ::rtl::OUString sFileExtension = aFeatures.getOrDefault("Extension",::rtl::OUString());
            if ( _sExtension == sFileExtension )
            {
                sURL = *pIter;
                break;
            }
            if ( !sFileExtension.getLength() && _sExtension.getLength() )
                sFallbackURL = *pIter;
        }
    } // for(;pIter != pEnd;++pIter )

    if ( !sURL.Len() && sFallbackURL.Len() )
        sURL = sFallbackURL;

    sURL.EraseTrailingChars('*');
    return sURL;
}
// -----------------------------------------------------------------------------
bool ODsnTypeCollection::isShowPropertiesEnabled( const ::rtl::OUString& _sURL ) const
{
    return !(    _sURL.matchIgnoreAsciiCaseAsciiL("sdbc:embedded:hsqldb",sizeof("sdbc:embedded:hsqldb")-1)
            ||  _sURL.matchIgnoreAsciiCaseAsciiL("sdbc:address:outlook",sizeof("sdbc:address:outlook")-1)
            ||  _sURL.matchIgnoreAsciiCaseAsciiL("sdbc:address:outlookexp",sizeof("sdbc:address:outlookexp")-1)
            ||  _sURL.matchIgnoreAsciiCaseAsciiL("sdbc:address:mozilla:",sizeof("sdbc:address:mozilla:")-1)
            ||  _sURL.matchIgnoreAsciiCaseAsciiL("sdbc:address:kab",sizeof("sdbc:address:kab")-1)
            ||  _sURL.matchIgnoreAsciiCaseAsciiL("sdbc:address:evolution:local",sizeof("sdbc:address:evolution:local")-1)
            ||  _sURL.matchIgnoreAsciiCaseAsciiL("sdbc:address:evolution:groupwise",sizeof("sdbc:address:evolution:groupwise")-1)
            ||  _sURL.matchIgnoreAsciiCaseAsciiL("sdbc:address:evolution:ldap",sizeof("sdbc:address:evolution:ldap")-1)
            ||  _sURL.matchIgnoreAsciiCaseAsciiL("sdbc:address:macab",sizeof("sdbc:address:macab")-1)  );
}
// -----------------------------------------------------------------------------
void ODsnTypeCollection::extractHostNamePort(const ::rtl::OUString& _rDsn,String& _sDatabaseName,String& _rsHostname,sal_Int32& _nPortNumber) const
{
    String sUrl = cutPrefix(_rDsn);
    if ( _rDsn.matchIgnoreAsciiCaseAsciiL("jdbc:oracle:thin:",sizeof("jdbc:oracle:thin:")-1) )
    {
        lcl_extractHostAndPort(sUrl,_rsHostname,_nPortNumber);
        if ( !_rsHostname.Len() && sUrl.GetTokenCount(':') == 2 )
        {
            _nPortNumber = -1;
            _rsHostname = sUrl.GetToken(0,':');
        }
        if ( _rsHostname.Len() )
            _rsHostname = _rsHostname.GetToken(_rsHostname.GetTokenCount('@') - 1,'@');
        _sDatabaseName = sUrl.GetToken(sUrl.GetTokenCount(':') - 1,':');
    } // if ( _rDsn.matchIgnoreAsciiCaseAsciiL("jdbc:oracle:thin:",sizeof("jdbc:oracle:thin:")-1) )
    else if ( _rDsn.matchIgnoreAsciiCaseAsciiL("sdbc:address:ldap:",sizeof("sdbc:address:ldap:")-1) )
    {
        lcl_extractHostAndPort(sUrl,_sDatabaseName,_nPortNumber);
    }
    else if ( _rDsn.matchIgnoreAsciiCaseAsciiL("sdbc:adabas:",sizeof("sdbc:adabas:")-1) )
    {
        if ( sUrl.GetTokenCount(':') == 2 )
            _rsHostname = sUrl.GetToken(0,':');
        _sDatabaseName = sUrl.GetToken(sUrl.GetTokenCount(':') - 1,':');
    }
    else if ( _rDsn.matchIgnoreAsciiCaseAsciiL("sdbc:mysql:mysqlc:",sizeof("sdbc:mysql:mysqlc:")-1) || _rDsn.matchIgnoreAsciiCaseAsciiL("sdbc:mysql:jdbc:",sizeof("sdbc:mysql:jdbc:")-1) )
    {
        lcl_extractHostAndPort(sUrl,_rsHostname,_nPortNumber);

        if ( _nPortNumber == -1 && !_rsHostname.Len() && sUrl.GetTokenCount('/') == 2 )
            _rsHostname = sUrl.GetToken(0,'/');
        _sDatabaseName = sUrl.GetToken(sUrl.GetTokenCount('/') - 1,'/');
    }
    else if ( _rDsn.matchIgnoreAsciiCaseAsciiL("sdbc:ado:access:Provider=Microsoft.ACE.OLEDB.12.0;DATA SOURCE=",sizeof("sdbc:ado:access:Provider=Microsoft.ACE.OLEDB.12.0;DATA SOURCE=")-1)
         || _rDsn.matchIgnoreAsciiCaseAsciiL("sdbc:ado:access:PROVIDER=Microsoft.Jet.OLEDB.4.0;DATA SOURCE=",sizeof("sdbc:ado:access:PROVIDER=Microsoft.Jet.OLEDB.4.0;DATA SOURCE=")-1))
    {
        ::rtl::OUString sNewFileName;
        if ( ::osl::FileBase::getFileURLFromSystemPath( sUrl, sNewFileName ) == ::osl::FileBase::E_None )
        {
            _sDatabaseName = sNewFileName;
        }
    }
}
// -----------------------------------------------------------------------------
String ODsnTypeCollection::getJavaDriverClass(const ::rtl::OUString& _sURL) const
{
    const ::comphelper::NamedValueCollection& aFeatures = m_aDriverConfig.getProperties(_sURL);
    return aFeatures.getOrDefault("JavaDriverClass",::rtl::OUString());
}
//-------------------------------------------------------------------------
sal_Bool ODsnTypeCollection::isFileSystemBased(const ::rtl::OUString& _sURL) const
{
    const ::comphelper::NamedValueCollection& aFeatures = m_aDriverConfig.getMetaData(_sURL);
    return aFeatures.getOrDefault("FileSystemBased",sal_False);
}
// -----------------------------------------------------------------------------
sal_Bool ODsnTypeCollection::supportsTableCreation(const ::rtl::OUString& _sURL) const
{
    const ::comphelper::NamedValueCollection& aFeatures = m_aDriverConfig.getMetaData(_sURL);
    return aFeatures.getOrDefault("SupportsTableCreation",sal_False);
}
// -----------------------------------------------------------------------------
sal_Bool ODsnTypeCollection::supportsColumnDescription(const ::rtl::OUString& _sURL) const
{
    const ::comphelper::NamedValueCollection& aFeatures = m_aDriverConfig.getMetaData(_sURL);
    return aFeatures.getOrDefault("SupportsColumnDescription",sal_False);
}
// -----------------------------------------------------------------------------
sal_Bool ODsnTypeCollection::supportsBrowsing(const ::rtl::OUString& _sURL) const
{
    const ::comphelper::NamedValueCollection& aFeatures = m_aDriverConfig.getMetaData(_sURL);
    return aFeatures.getOrDefault("SupportsBrowsing",sal_False);
}
// -----------------------------------------------------------------------------
bool ODsnTypeCollection::needsJVM(const String& _sURL) const
{
    const ::comphelper::NamedValueCollection& aFeatures = m_aDriverConfig.getMetaData(_sURL);
    return aFeatures.getOrDefault("UseJava",sal_False);
}
// -----------------------------------------------------------------------------
Sequence<PropertyValue> ODsnTypeCollection::getDefaultDBSettings( const ::rtl::OUString& _sURL ) const
{
    const ::comphelper::NamedValueCollection& aProperties = m_aDriverConfig.getProperties(_sURL);
    return aProperties.getPropertyValues();
}

//-------------------------------------------------------------------------
bool ODsnTypeCollection::isEmbeddedDatabase( const ::rtl::OUString& _sURL ) const
{
    const ::rtl::OUString sEmbeddedDatabaseURL = getEmbeddedDatabase();
    WildCard aWildCard(sEmbeddedDatabaseURL);
    return aWildCard.Matches(_sURL);
}
// -----------------------------------------------------------------------------
::rtl::OUString ODsnTypeCollection::getEmbeddedDatabase() const
{
    ::rtl::OUString sEmbeddedDatabaseURL;
    static const ::rtl::OUString s_sNodeName(RTL_CONSTASCII_USTRINGPARAM("org.openoffice.Office.DataAccess")); ///Installed
    const ::utl::OConfigurationTreeRoot aInstalled = ::utl::OConfigurationTreeRoot::createWithServiceFactory(m_xFactory, s_sNodeName, -1, ::utl::OConfigurationTreeRoot::CM_READONLY);
    if ( aInstalled.isValid() )
    {
        if ( aInstalled.hasByName("EmbeddedDatabases/DefaultEmbeddedDatabase/Value") )
        {
            static const ::rtl::OUString s_sValue(RTL_CONSTASCII_USTRINGPARAM("EmbeddedDatabases/DefaultEmbeddedDatabase/Value"));

            aInstalled.getNodeValue(s_sValue) >>= sEmbeddedDatabaseURL;
            if ( sEmbeddedDatabaseURL.getLength() )
                aInstalled.getNodeValue(s_sValue + ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/")) + sEmbeddedDatabaseURL + ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/URL"))) >>= sEmbeddedDatabaseURL;
        }
    } // if ( aInstalled.isValid() )
    if ( !sEmbeddedDatabaseURL.getLength() )
        sEmbeddedDatabaseURL = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("sdbc:embedded:hsqldb"));
    return sEmbeddedDatabaseURL;
}
//-------------------------------------------------------------------------
ODsnTypeCollection::TypeIterator ODsnTypeCollection::begin() const
{
    return TypeIterator(this, 0);
}

//-------------------------------------------------------------------------
ODsnTypeCollection::TypeIterator ODsnTypeCollection::end() const
{
    return TypeIterator(this, m_aDsnTypesDisplayNames.size());
}
//-------------------------------------------------------------------------
DATASOURCE_TYPE ODsnTypeCollection::determineType(const String& _rDsn) const
{
    String sDsn(_rDsn);
    sDsn.EraseTrailingChars('*');
    sal_uInt16 nSeparator = sDsn.Search((sal_Unicode)':');
    if (STRING_NOTFOUND == nSeparator)
    {
        // there should be at least one such separator
        DBG_ERROR("ODsnTypeCollection::implDetermineType : missing the colon !");
        return DST_UNKNOWN;
    }
    // find first :
    sal_uInt16 nOracleSeparator = sDsn.Search((sal_Unicode)':', nSeparator + 1);
    if ( nOracleSeparator != STRING_NOTFOUND )
    {
        nOracleSeparator = sDsn.Search((sal_Unicode)':', nOracleSeparator + 1);
        if (nOracleSeparator != STRING_NOTFOUND && sDsn.EqualsIgnoreCaseAscii("jdbc:oracle:thin", 0, nOracleSeparator))
            return DST_ORACLE_JDBC;
    }

    if (sDsn.EqualsIgnoreCaseAscii("jdbc", 0, nSeparator))
        return DST_JDBC;

    if (sDsn.EqualsIgnoreCaseAscii("sdbc:embedded:hsqldb", 0, sDsn.Len()))
        return DST_EMBEDDED_HSQLDB;

    // find second :
    nSeparator = sDsn.Search((sal_Unicode)':', nSeparator + 1);
    if (STRING_NOTFOUND == nSeparator)
    {
        // at the moment only jdbc is allowed to have just one separator
        DBG_ERROR("ODsnTypeCollection::implDetermineType : missing the second colon !");
        return DST_UNKNOWN;
    }

    if (sDsn.EqualsIgnoreCaseAscii("sdbc:ado:", 0, nSeparator))
    {
        nSeparator = sDsn.Search((sal_Unicode)':', nSeparator + 1);
        if (STRING_NOTFOUND != nSeparator && sDsn.EqualsIgnoreCaseAscii("sdbc:ado:access",0, nSeparator) )
        {
            nSeparator = sDsn.Search((sal_Unicode)';', nSeparator + 1);
            if (STRING_NOTFOUND != nSeparator && sDsn.EqualsIgnoreCaseAscii("sdbc:ado:access:Provider=Microsoft.ACE.OLEDB.12.0",0, nSeparator) )
                return DST_MSACCESS_2007;

            return DST_MSACCESS;
        }
        return DST_ADO;
    }

    struct KnownPrefix
    {
        const sal_Char*         pAsciiPrefix;
        const DATASOURCE_TYPE   eType;
        const bool              bMatchComplete;

        KnownPrefix()
            :pAsciiPrefix( NULL )
            ,eType( DST_UNKNOWN )
            ,bMatchComplete( false )
        {
        }
        KnownPrefix( const sal_Char* _p, const DATASOURCE_TYPE _t, const bool _m )
            :pAsciiPrefix( _p )
            ,eType ( _t )
            ,bMatchComplete( _m )
        {
        }
    };
    KnownPrefix aKnowPrefixes[] =
    {
        KnownPrefix( "sdbc:calc:",          DST_CALC,               false ),
        KnownPrefix( "sdbc:flat:",          DST_FLAT,               false ),
        KnownPrefix( "sdbc:adabas:",        DST_ADABAS,             false ),
        KnownPrefix( "sdbc:odbc:",          DST_ODBC,               false ),
        KnownPrefix( "sdbc:dbase:",         DST_DBASE,              false ),
        KnownPrefix( "sdbc:mysql:odbc:",    DST_MYSQL_ODBC,         false ),
        KnownPrefix( "sdbc:mysql:jdbc:",    DST_MYSQL_JDBC,         false ),
        KnownPrefix( "sdbc:mysql:mysqlc:",  DST_MYSQL_NATIVE,       false ),
        KnownPrefix( "sdbc:mysqlc:",        DST_MYSQL_NATIVE_DIRECT,false ),

        KnownPrefix( "sdbc:address:mozilla:",           DST_MOZILLA,            true ),
        KnownPrefix( "sdbc:address:thunderbird:",       DST_THUNDERBIRD,        true ),
        KnownPrefix( "sdbc:address:ldap:",              DST_LDAP,               true ),
        KnownPrefix( "sdbc:address:outlook",            DST_OUTLOOK,            true ),
        KnownPrefix( "sdbc:address:outlookexp",         DST_OUTLOOKEXP,         true ),
        KnownPrefix( "sdbc:address:evolution:ldap",     DST_EVOLUTION_LDAP,     true ),
        KnownPrefix( "sdbc:address:evolution:groupwise",DST_EVOLUTION_GROUPWISE,true ),
        KnownPrefix( "sdbc:address:evolution:local",    DST_EVOLUTION,          true ),
        KnownPrefix( "sdbc:address:kab",                DST_KAB,                true ),
        KnownPrefix( "sdbc:address:macab",              DST_MACAB,              true )
    };

    for ( size_t i=0; i < sizeof( aKnowPrefixes ) / sizeof( aKnowPrefixes[0] ); ++i )
    {
        sal_uInt16 nMatchLen = aKnowPrefixes[i].bMatchComplete ? sDsn.Len() : (sal_uInt16)rtl_str_getLength( aKnowPrefixes[i].pAsciiPrefix );
        if ( sDsn.EqualsIgnoreCaseAscii( aKnowPrefixes[i].pAsciiPrefix, 0, nMatchLen ) )
            return aKnowPrefixes[i].eType;
    }

    return DST_UNKNOWN;
}
// -----------------------------------------------------------------------------
void ODsnTypeCollection::fillPageIds(const ::rtl::OUString& _sURL,::std::vector<sal_Int16>& _rOutPathIds) const
{
    DATASOURCE_TYPE eType = determineType(_sURL);
    switch(eType)
    {
        case DST_ADO:
            _rOutPathIds.push_back(PAGE_DBSETUPWIZARD_ADO);
            break;
        case DST_DBASE:
            _rOutPathIds.push_back(PAGE_DBSETUPWIZARD_DBASE);
            break;
        case DST_FLAT:
            _rOutPathIds.push_back(PAGE_DBSETUPWIZARD_TEXT);
            break;
        case DST_CALC:
            _rOutPathIds.push_back(PAGE_DBSETUPWIZARD_SPREADSHEET);
            break;
        case DST_ODBC:
            _rOutPathIds.push_back(PAGE_DBSETUPWIZARD_ODBC);
            break;
        case DST_JDBC:
            _rOutPathIds.push_back(PAGE_DBSETUPWIZARD_JDBC);
            break;
        case DST_MYSQL_ODBC:
            _rOutPathIds.push_back(PAGE_DBSETUPWIZARD_MYSQL_INTRO);
            _rOutPathIds.push_back(PAGE_DBSETUPWIZARD_MYSQL_ODBC);
            break;
        case DST_MYSQL_JDBC:
            _rOutPathIds.push_back(PAGE_DBSETUPWIZARD_MYSQL_INTRO);
            _rOutPathIds.push_back(PAGE_DBSETUPWIZARD_MYSQL_JDBC);
            break;
        case DST_MYSQL_NATIVE:
            _rOutPathIds.push_back(PAGE_DBSETUPWIZARD_MYSQL_INTRO);
            _rOutPathIds.push_back(PAGE_DBSETUPWIZARD_MYSQL_NATIVE);
            break;
        case DST_ORACLE_JDBC:
            _rOutPathIds.push_back(PAGE_DBSETUPWIZARD_ORACLE);
            break;
        case DST_ADABAS:
            _rOutPathIds.push_back(PAGE_DBSETUPWIZARD_ADABAS);
            break;
        case DST_LDAP:
            _rOutPathIds.push_back(PAGE_DBSETUPWIZARD_LDAP);
            break;
        case DST_MSACCESS:
        case DST_MSACCESS_2007:
            _rOutPathIds.push_back(PAGE_DBSETUPWIZARD_MSACCESS);
            break;
        case DST_OUTLOOKEXP:
        case DST_OUTLOOK:
        case DST_MOZILLA:
        case DST_THUNDERBIRD:
        case DST_EVOLUTION:
        case DST_EVOLUTION_GROUPWISE:
        case DST_EVOLUTION_LDAP:
        case DST_KAB:
        case DST_MACAB:
        case DST_EMBEDDED_HSQLDB:
            break;
        default:
            _rOutPathIds.push_back(PAGE_DBSETUPWIZARD_USERDEFINED);
            break;
    }
}
// -----------------------------------------------------------------------------
::rtl::OUString ODsnTypeCollection::getType(const ::rtl::OUString& _sURL) const
{
    ::rtl::OUString sOldPattern;
    StringVector::const_iterator aIter = m_aDsnPrefixes.begin();
    StringVector::const_iterator aEnd = m_aDsnPrefixes.end();
    for(;aIter != aEnd;++aIter)
    {
        WildCard aWildCard(*aIter);
        if ( sOldPattern.getLength() < aIter->Len() && aWildCard.Matches(_sURL) )
        {
            sOldPattern = *aIter;
        }
    } // for(sal_Int32 i = 0;aIter != aEnd;++aIter,++i)
    return sOldPattern;
}
// -----------------------------------------------------------------------------
sal_Int32 ODsnTypeCollection::getIndexOf(const ::rtl::OUString& _sURL) const
{
    sal_Int32 nRet = -1;
    String sURL( _sURL);
    String sOldPattern;
    StringVector::const_iterator aIter = m_aDsnPrefixes.begin();
    StringVector::const_iterator aEnd = m_aDsnPrefixes.end();
    for(sal_Int32 i = 0;aIter != aEnd;++aIter,++i)
    {
        WildCard aWildCard(*aIter);
        if ( sOldPattern.Len() < aIter->Len() && aWildCard.Matches(sURL) )
        {
            nRet = i;
            sOldPattern = *aIter;
        }
    }

    return nRet;
}
// -----------------------------------------------------------------------------
sal_Int32 ODsnTypeCollection::size() const
{
    return m_aDsnPrefixes.size();
}
//=========================================================================
//= ODsnTypeCollection::TypeIterator
//=========================================================================
//-------------------------------------------------------------------------
ODsnTypeCollection::TypeIterator::TypeIterator(const ODsnTypeCollection* _pContainer, sal_Int32 _nInitialPos)
    :m_pContainer(_pContainer)
    ,m_nPosition(_nInitialPos)
{
    DBG_ASSERT(m_pContainer, "ODsnTypeCollection::TypeIterator::TypeIterator : invalid container!");
#ifdef DBG_UTIL
    ++const_cast<ODsnTypeCollection*>(m_pContainer)->m_nLivingIterators;
#endif
}

//-------------------------------------------------------------------------
ODsnTypeCollection::TypeIterator::TypeIterator(const TypeIterator& _rSource)
    :m_pContainer(_rSource.m_pContainer)
    ,m_nPosition(_rSource.m_nPosition)
{
#ifdef DBG_UTIL
    ++const_cast<ODsnTypeCollection*>(m_pContainer)->m_nLivingIterators;
#endif
}

//-------------------------------------------------------------------------
ODsnTypeCollection::TypeIterator::~TypeIterator()
{
#ifdef DBG_UTIL
    --const_cast<ODsnTypeCollection*>(m_pContainer)->m_nLivingIterators;
#endif
}

//-------------------------------------------------------------------------
String ODsnTypeCollection::TypeIterator::getDisplayName() const
{
    DBG_ASSERT(m_nPosition < (sal_Int32)m_pContainer->m_aDsnTypesDisplayNames.size(), "ODsnTypeCollection::TypeIterator::getDisplayName : invalid position!");
    return m_pContainer->m_aDsnTypesDisplayNames[m_nPosition];
}
// -----------------------------------------------------------------------------
::rtl::OUString ODsnTypeCollection::TypeIterator::getURLPrefix() const
{
    DBG_ASSERT(m_nPosition < (sal_Int32)m_pContainer->m_aDsnPrefixes.size(), "ODsnTypeCollection::TypeIterator::getDisplayName : invalid position!");
    return m_pContainer->m_aDsnPrefixes[m_nPosition];
}
//-------------------------------------------------------------------------
const ODsnTypeCollection::TypeIterator& ODsnTypeCollection::TypeIterator::operator++()
{
    DBG_ASSERT(m_nPosition < (sal_Int32)m_pContainer->m_aDsnTypesDisplayNames.size(), "ODsnTypeCollection::TypeIterator::operator++ : invalid position!");
    if (m_nPosition < (sal_Int32)m_pContainer->m_aDsnTypesDisplayNames.size())
        ++m_nPosition;
    return *this;
}

//-------------------------------------------------------------------------
const ODsnTypeCollection::TypeIterator& ODsnTypeCollection::TypeIterator::operator--()
{
    DBG_ASSERT(m_nPosition >= 0, "ODsnTypeCollection::TypeIterator::operator-- : invalid position!");
    if (m_nPosition >= 0)
        --m_nPosition;
    return *this;
}

//-------------------------------------------------------------------------
bool operator==(const ODsnTypeCollection::TypeIterator& lhs, const ODsnTypeCollection::TypeIterator& rhs)
{
    return (lhs.m_pContainer == rhs.m_pContainer) && (lhs.m_nPosition == rhs.m_nPosition);
}

//.........................................................................
}   // namespace dbaccess
//.........................................................................


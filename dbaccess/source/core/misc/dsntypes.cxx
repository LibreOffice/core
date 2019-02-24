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

#include <dsntypes.hxx>
#include <unotools/confignode.hxx>
#include <osl/diagnose.h>
#include <tools/wldcrd.hxx>
#include <osl/file.hxx>
#include <stringconstants.hxx>
#include <comphelper/documentconstants.hxx>
#include <comphelper/string.hxx>

namespace dbaccess
{

    using namespace ::com::sun::star;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::lang;

    namespace
    {
        void lcl_extractHostAndPort(const OUString& _sUrl,OUString& _sHostname,sal_Int32& _nPortNumber)
        {
            if ( comphelper::string::getTokenCount(_sUrl, ':') >= 2 )
            {
                sal_Int32 nPos {0};
                _sHostname   = _sUrl.getToken(0, ':', nPos);
                _nPortNumber = _sUrl.getToken(0, ':', nPos).toInt32();
            }
        }
    }
// ODsnTypeCollection
ODsnTypeCollection::ODsnTypeCollection(const css::uno::Reference< css::uno::XComponentContext >& _xContext)
:m_aDriverConfig(_xContext)
#if OSL_DEBUG_LEVEL > 0
,m_nLivingIterators(0)
#endif
{
    const uno::Sequence< OUString > aURLs = m_aDriverConfig.getURLs();
    const OUString* pIter = aURLs.getConstArray();
    const OUString* pEnd = pIter + aURLs.getLength();
    for(;pIter != pEnd;++pIter )
    {
        m_aDsnPrefixes.push_back(*pIter);
        m_aDsnTypesDisplayNames.push_back(m_aDriverConfig.getDriverTypeDisplayName(*pIter));
    }

    OSL_ENSURE(m_aDsnTypesDisplayNames.size() == m_aDsnPrefixes.size(),
        "ODsnTypeCollection::ODsnTypeCollection : invalid resources !");
}

ODsnTypeCollection::~ODsnTypeCollection()
{
#if OSL_DEBUG_LEVEL > 0
    OSL_ENSURE(0 == m_nLivingIterators, "ODsnTypeCollection::~ODsnTypeCollection : there are still living iterator objects!");
#endif
}

OUString ODsnTypeCollection::getTypeDisplayName(const OUString& _sURL) const
{
    return m_aDriverConfig.getDriverTypeDisplayName(_sURL);
}

OUString ODsnTypeCollection::cutPrefix(const OUString& _sURL) const
{
    OUString sRet;
    OUString sOldPattern;

    for (auto const& dsnPrefix : m_aDsnPrefixes)
    {
        WildCard aWildCard(dsnPrefix);
        if ( sOldPattern.getLength() < dsnPrefix.getLength() && aWildCard.Matches(_sURL) )
        {
            // This relies on the fact that all patterns are of the form
            //   foo*
            // that is, the very concept of "prefix" applies.
            OUString prefix(comphelper::string::stripEnd(dsnPrefix, '*'));
            OSL_ENSURE(prefix.getLength() <= _sURL.getLength(), "How can A match B when A shorter than B?");
            sRet = _sURL.copy(prefix.getLength());
            sOldPattern = dsnPrefix;
        }
    }

    return sRet;
}

OUString ODsnTypeCollection::getPrefix(const OUString& _sURL) const
{
    OUString sRet;
    OUString sOldPattern;
    for (auto const& dsnPrefix : m_aDsnPrefixes)
    {
        WildCard aWildCard(dsnPrefix);
        if ( sOldPattern.getLength() < dsnPrefix.getLength() && aWildCard.Matches(_sURL) )
        {
            // This relies on the fact that all patterns are of the form
            //   foo*
            // that is, the very concept of "prefix" applies.
            sRet = comphelper::string::stripEnd(dsnPrefix, '*');
            OSL_ENSURE(sRet.getLength() <= _sURL.getLength(), "How can A match B when A shorter than B?");
            sOldPattern = dsnPrefix;
        }
    }

    return sRet;
}

bool ODsnTypeCollection::hasDriver( const sal_Char* _pAsciiPattern ) const
{
    OUString sPrefix( getPrefix( OUString::createFromAscii( _pAsciiPattern ) ) );
    return !sPrefix.isEmpty();
}

bool ODsnTypeCollection::isConnectionUrlRequired(const OUString& _sURL) const
{
    OUString sRet;
    OUString sOldPattern;
    for (auto const& dsnPrefix : m_aDsnPrefixes)
    {
        WildCard aWildCard(dsnPrefix);
        if ( sOldPattern.getLength() < dsnPrefix.getLength() && aWildCard.Matches(_sURL) )
        {
            sRet = dsnPrefix;
            sOldPattern = dsnPrefix;
        }
    }
    return !sRet.isEmpty() && sRet[sRet.getLength()-1] == '*';
}

OUString ODsnTypeCollection::getMediaType(const OUString& _sURL) const
{
    const ::comphelper::NamedValueCollection& aFeatures = m_aDriverConfig.getMetaData(_sURL);
    return aFeatures.getOrDefault("MediaType",OUString());
}

OUString ODsnTypeCollection::getDatasourcePrefixFromMediaType(const OUString& _sMediaType,const OUString& _sExtension)
{
    OUString sURL, sFallbackURL;
    const uno::Sequence< OUString > aURLs = m_aDriverConfig.getURLs();
    const OUString* pIter = aURLs.getConstArray();
    const OUString* pEnd = pIter + aURLs.getLength();
    for(;pIter != pEnd;++pIter )
    {
        const ::comphelper::NamedValueCollection& aFeatures = m_aDriverConfig.getMetaData(*pIter);
        if ( aFeatures.getOrDefault("MediaType",OUString()) == _sMediaType )
        {
            const OUString sFileExtension = aFeatures.getOrDefault("Extension",OUString());
            if ( _sExtension == sFileExtension )
            {
                sURL = *pIter;
                break;
            }
            if ( sFileExtension.isEmpty() && !_sExtension.isEmpty() )
                sFallbackURL = *pIter;
        }
    }

    if ( sURL.isEmpty() && !sFallbackURL.isEmpty() )
        sURL = sFallbackURL;

    sURL = comphelper::string::stripEnd(sURL, '*');
    return sURL;
}

bool ODsnTypeCollection::isShowPropertiesEnabled( const OUString& _sURL )
{
    return !(   _sURL.startsWithIgnoreAsciiCase("sdbc:embedded:hsqldb")
            ||  _sURL.startsWithIgnoreAsciiCase("sdbc:embedded:firebird")
            ||  _sURL.startsWithIgnoreAsciiCase("sdbc:address:outlook")
            ||  _sURL.startsWithIgnoreAsciiCase("sdbc:address:outlookexp")
            ||  _sURL.startsWithIgnoreAsciiCase("sdbc:address:mozilla:")
            ||  _sURL.startsWithIgnoreAsciiCase("sdbc:address:kab")
            ||  _sURL.startsWithIgnoreAsciiCase("sdbc:address:evolution:local")
            ||  _sURL.startsWithIgnoreAsciiCase("sdbc:address:evolution:groupwise")
            ||  _sURL.startsWithIgnoreAsciiCase("sdbc:address:evolution:ldap")
            ||  _sURL.startsWithIgnoreAsciiCase("sdbc:address:macab")  );
}

void ODsnTypeCollection::extractHostNamePort(const OUString& _rDsn,OUString& _sDatabaseName,OUString& _rsHostname,sal_Int32& _nPortNumber) const
{
    OUString sUrl = cutPrefix(_rDsn);
    if ( _rDsn.startsWithIgnoreAsciiCase("jdbc:oracle:thin:") )
    {
        lcl_extractHostAndPort(sUrl,_rsHostname,_nPortNumber);
        const sal_Int32 nUrlTokens {comphelper::string::getTokenCount(sUrl, ':')};
        if ( _rsHostname.isEmpty() && nUrlTokens == 2 )
        {
            _nPortNumber = -1;
            _rsHostname = sUrl.getToken(0,':');
        }
        if ( !_rsHostname.isEmpty() )
            _rsHostname = _rsHostname.copy(_rsHostname.lastIndexOf('@')+1);
        _sDatabaseName = sUrl.copy(sUrl.lastIndexOf(':')+1);
    }
    else if ( _rDsn.startsWithIgnoreAsciiCase("sdbc:address:ldap:") )
    {
        lcl_extractHostAndPort(sUrl,_sDatabaseName,_nPortNumber);
    }
    else if ( _rDsn.startsWithIgnoreAsciiCase("sdbc:mysql:mysqlc:")
              || _rDsn.startsWithIgnoreAsciiCase("sdbc:mysql:jdbc:") )
    {
        lcl_extractHostAndPort(sUrl,_rsHostname,_nPortNumber);

        const sal_Int32 nUrlTokens {comphelper::string::getTokenCount(sUrl, '/')};
        if ( _nPortNumber == -1 && _rsHostname.isEmpty() && nUrlTokens == 2 )
            _rsHostname = sUrl.getToken(0,'/');
        _sDatabaseName = sUrl.copy(sUrl.lastIndexOf('/')+1);
    }
    else if ( _rDsn.startsWithIgnoreAsciiCase("sdbc:ado:access:Provider=Microsoft.ACE.OLEDB.12.0;DATA SOURCE=")
           || _rDsn.startsWithIgnoreAsciiCase("sdbc:ado:access:PROVIDER=Microsoft.Jet.OLEDB.4.0;DATA SOURCE=") )
    {
        OUString sNewFileName;
        if ( ::osl::FileBase::getFileURLFromSystemPath( sUrl, sNewFileName ) == ::osl::FileBase::E_None )
        {
            _sDatabaseName = sNewFileName;
        }
    }
}

OUString ODsnTypeCollection::getJavaDriverClass(const OUString& _sURL) const
{
    const ::comphelper::NamedValueCollection& aFeatures = m_aDriverConfig.getProperties(_sURL);
    return aFeatures.getOrDefault("JavaDriverClass",OUString());
}

bool ODsnTypeCollection::isFileSystemBased(const OUString& _sURL) const
{
    const ::comphelper::NamedValueCollection& aFeatures = m_aDriverConfig.getMetaData(_sURL);
    return aFeatures.getOrDefault("FileSystemBased",false);
}

bool ODsnTypeCollection::supportsTableCreation(const OUString& _sURL) const
{
    const ::comphelper::NamedValueCollection& aFeatures = m_aDriverConfig.getMetaData(_sURL);
    return aFeatures.getOrDefault("SupportsTableCreation",false);
}

bool ODsnTypeCollection::supportsColumnDescription(const OUString& _sURL) const
{
    const ::comphelper::NamedValueCollection& aFeatures = m_aDriverConfig.getMetaData(_sURL);
    return aFeatures.getOrDefault("SupportsColumnDescription",false);
}

bool ODsnTypeCollection::supportsBrowsing(const OUString& _sURL) const
{
    const ::comphelper::NamedValueCollection& aFeatures = m_aDriverConfig.getMetaData(_sURL);
    return aFeatures.getOrDefault("SupportsBrowsing",false);
}

bool ODsnTypeCollection::supportsDBCreation(const OUString& _sURL) const
{
    const ::comphelper::NamedValueCollection& aFeatures = m_aDriverConfig.getMetaData(_sURL);
    return aFeatures.getOrDefault("SupportsDBCreation",false);
}

Sequence<PropertyValue> ODsnTypeCollection::getDefaultDBSettings( const OUString& _sURL ) const
{
    const ::comphelper::NamedValueCollection& aProperties = m_aDriverConfig.getProperties(_sURL);
    return aProperties.getPropertyValues();
}

bool ODsnTypeCollection::isEmbeddedDatabase( const OUString& _sURL )
{
    return _sURL.startsWith( "sdbc:embedded:" );
}

OUString ODsnTypeCollection::getEmbeddedDatabase()
{
    return "sdbc:embedded:firebird";
}


DATASOURCE_TYPE ODsnTypeCollection::determineType(const OUString& _rDsn) const
{
    OUString sDsn(comphelper::string::stripEnd(_rDsn, '*'));
    sal_Int32 nSeparator = sDsn.indexOf(u':');
    if (-1 == nSeparator)
    {
        if (!sDsn.isEmpty())
        {
            // there should be at least one such separator
            OSL_FAIL("ODsnTypeCollection::implDetermineType : missing the colon !");
        }

        return DST_UNKNOWN;
    }

    // find first :
    if (sDsn.startsWithIgnoreAsciiCase("jdbc:oracle:thin:"))
        return DST_ORACLE_JDBC;

    if (sDsn.startsWithIgnoreAsciiCase("jdbc:"))
        return DST_JDBC;

    if (sDsn.equalsIgnoreAsciiCase("sdbc:embedded:hsqldb"))
        return DST_EMBEDDED_HSQLDB;

    if (sDsn.equalsIgnoreAsciiCase("sdbc:embedded:firebird"))
        return DST_EMBEDDED_FIREBIRD;

    // find second :
    nSeparator = sDsn.indexOf(u':', nSeparator + 1);
    if (-1 == nSeparator)
    {
        // at the moment only jdbc is allowed to have just one separator
        OSL_FAIL("ODsnTypeCollection::implDetermineType : missing the second colon !");
        return DST_UNKNOWN;
    }

    if (sDsn.startsWithIgnoreAsciiCase("sdbc:ado:"))
    {
        if (sDsn.startsWithIgnoreAsciiCase("sdbc:ado:access:"))
        {
            if (sDsn.startsWithIgnoreAsciiCase("sdbc:ado:access:Provider=Microsoft.ACE.OLEDB.12.0;"))
                return DST_MSACCESS_2007;
            else
                return DST_MSACCESS;
        }
        return DST_ADO;
    }

    struct KnownPrefix
    {
        const OUString          sPrefix;
        const DATASOURCE_TYPE   eType;
        const bool              bMatchComplete;

        KnownPrefix( const OUString &_s, const DATASOURCE_TYPE _t, const bool _m )
            :sPrefix( _s )
            ,eType ( _t )
            ,bMatchComplete( _m )
        {
        }

        bool match( const OUString &url) const
        {
            if(bMatchComplete)
            {
                return url.equalsIgnoreAsciiCase(sPrefix);
            }
            else
            {
                return url.startsWithIgnoreAsciiCase(sPrefix);
            }
        }
    };
    const KnownPrefix aKnowPrefixes[] =
    {
        KnownPrefix( "sdbc:calc:",          DST_CALC,               false ),
        KnownPrefix( "sdbc:writer:",        DST_WRITER,             false ),
        KnownPrefix( "sdbc:flat:",          DST_FLAT,               false ),
        KnownPrefix( "sdbc:odbc:",          DST_ODBC,               false ),
        KnownPrefix( "sdbc:dbase:",         DST_DBASE,              false ),
        KnownPrefix( "sdbc:firebird:",      DST_FIREBIRD,           false ),
        KnownPrefix( "sdbc:mysql:odbc:",    DST_MYSQL_ODBC,         false ),
        KnownPrefix( "sdbc:mysql:jdbc:",    DST_MYSQL_JDBC,         false ),
        KnownPrefix( "sdbc:mysql:mysqlc:",  DST_MYSQL_NATIVE,       false ),
        KnownPrefix( "sdbc:mysqlc:",        DST_MYSQL_NATIVE_DIRECT,false ),
        KnownPrefix( "sdbc:postgresql:",    DST_POSTGRES           ,false ),

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

    for (const auto & aKnowPrefixe : aKnowPrefixes)
    {
        if( aKnowPrefixe.match(sDsn) )
        {
            return aKnowPrefixe.eType;
        }
    }

    return DST_UNKNOWN;
}

void ODsnTypeCollection::fillPageIds(const OUString& _sURL,std::vector<sal_Int16>& _rOutPathIds) const
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
        case DST_WRITER:
            _rOutPathIds.push_back(PAGE_DBSETUPWIZARD_DOCUMENT_OR_SPREADSHEET);
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
        case DST_EMBEDDED_FIREBIRD:
            break;
        default:
            _rOutPathIds.push_back(PAGE_DBSETUPWIZARD_USERDEFINED);
            break;
    }
}

OUString ODsnTypeCollection::getType(const OUString& _sURL) const
{
    OUString sOldPattern;
    for (auto const& dsnPrefix : m_aDsnPrefixes)
    {
        WildCard aWildCard(dsnPrefix);
        if ( sOldPattern.getLength() < dsnPrefix.getLength() && aWildCard.Matches(_sURL) )
        {
            sOldPattern = dsnPrefix;
        }
    }
    return sOldPattern;
}

sal_Int32 ODsnTypeCollection::getIndexOf(const OUString& _sURL) const
{
    sal_Int32 nRet = -1;
    OUString sOldPattern;
    sal_Int32 i = 0;
    for (auto const& dsnPrefix : m_aDsnPrefixes)
    {
        WildCard aWildCard(dsnPrefix);
        if ( sOldPattern.getLength() < dsnPrefix.getLength() && aWildCard.Matches(_sURL) )
        {
            nRet = i;
            sOldPattern = dsnPrefix;
        }
        ++i;
    }

    return nRet;
}

sal_Int32 ODsnTypeCollection::size() const
{
    return m_aDsnPrefixes.size();
}

// ODsnTypeCollection::TypeIterator
ODsnTypeCollection::TypeIterator::TypeIterator(const ODsnTypeCollection* _pContainer, sal_Int32 _nInitialPos)
    :m_pContainer(_pContainer)
    ,m_nPosition(_nInitialPos)
{
    OSL_ENSURE(m_pContainer, "ODsnTypeCollection::TypeIterator::TypeIterator : invalid container!");
#if OSL_DEBUG_LEVEL > 0
    ++const_cast<ODsnTypeCollection*>(m_pContainer)->m_nLivingIterators;
#endif
}

ODsnTypeCollection::TypeIterator::TypeIterator(const TypeIterator& _rSource)
    :m_pContainer(_rSource.m_pContainer)
    ,m_nPosition(_rSource.m_nPosition)
{
#if OSL_DEBUG_LEVEL > 0
    ++const_cast<ODsnTypeCollection*>(m_pContainer)->m_nLivingIterators;
#endif
}

ODsnTypeCollection::TypeIterator::~TypeIterator()
{
#if OSL_DEBUG_LEVEL > 0
    --const_cast<ODsnTypeCollection*>(m_pContainer)->m_nLivingIterators;
#endif
}

OUString const & ODsnTypeCollection::TypeIterator::getDisplayName() const
{
    OSL_ENSURE(m_nPosition < static_cast<sal_Int32>(m_pContainer->m_aDsnTypesDisplayNames.size()), "ODsnTypeCollection::TypeIterator::getDisplayName : invalid position!");
    return m_pContainer->m_aDsnTypesDisplayNames[m_nPosition];
}

OUString const & ODsnTypeCollection::TypeIterator::getURLPrefix() const
{
    OSL_ENSURE(m_nPosition < static_cast<sal_Int32>(m_pContainer->m_aDsnPrefixes.size()), "ODsnTypeCollection::TypeIterator::getDisplayName : invalid position!");
    return m_pContainer->m_aDsnPrefixes[m_nPosition];
}

const ODsnTypeCollection::TypeIterator& ODsnTypeCollection::TypeIterator::operator++()
{
    OSL_ENSURE(m_nPosition < static_cast<sal_Int32>(m_pContainer->m_aDsnTypesDisplayNames.size()), "ODsnTypeCollection::TypeIterator::operator++ : invalid position!");
    if (m_nPosition < static_cast<sal_Int32>(m_pContainer->m_aDsnTypesDisplayNames.size()))
        ++m_nPosition;
    return *this;
}

bool operator==(const ODsnTypeCollection::TypeIterator& lhs, const ODsnTypeCollection::TypeIterator& rhs)
{
    return (lhs.m_pContainer == rhs.m_pContainer) && (lhs.m_nPosition == rhs.m_nPosition);
}

}   // namespace dbaccess

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

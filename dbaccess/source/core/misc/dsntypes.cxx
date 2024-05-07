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

#include <config_java.h>
#include <dsntypes.hxx>
#include <unotools/confignode.hxx>
#include <o3tl/safeint.hxx>
#include <o3tl/string_view.hxx>
#include <osl/diagnose.h>
#include <tools/wldcrd.hxx>
#include <osl/file.hxx>
#include <officecfg/Office/Common.hxx>
#include <comphelper/string.hxx>
#include <utility>

namespace dbaccess
{

    using namespace ::com::sun::star;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;

    namespace
    {
        void lcl_extractHostAndPort(std::u16string_view _sUrl, OUString& _sHostname, sal_Int32& _nPortNumber)
        {
            if ( comphelper::string::getTokenCount(_sUrl, ':') >= 2 )
            {
                sal_Int32 nPos {0};
                _sHostname   = o3tl::getToken(_sUrl, 0, ':', nPos);
                _nPortNumber = o3tl::toInt32(o3tl::getToken(_sUrl, 0, ':', nPos));
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
    for (auto& url : m_aDriverConfig.getURLs())
    {
        m_aDsnPrefixes.push_back(url);
        m_aDsnTypesDisplayNames.push_back(m_aDriverConfig.getDriverTypeDisplayName(url));
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

OUString ODsnTypeCollection::getTypeDisplayName(std::u16string_view _sURL) const
{
    return m_aDriverConfig.getDriverTypeDisplayName(_sURL);
}

OUString ODsnTypeCollection::cutPrefix(std::u16string_view _sURL) const
{
    OUString sRet;
    OUString sOldPattern;

    // on Windows or with gen rendering, the urls may begin with an ~
    std::u16string_view sCleanURL = comphelper::string::stripStart(_sURL, '~');

    for (auto const& dsnPrefix : m_aDsnPrefixes)
    {
        WildCard aWildCard(dsnPrefix);
        if ( sOldPattern.getLength() < dsnPrefix.getLength() && aWildCard.Matches(sCleanURL) )
        {
            // This relies on the fact that all patterns are of the form
            //   foo*
            // that is, the very concept of "prefix" applies.
            OUString prefix(comphelper::string::stripEnd(dsnPrefix, '*'));
            OSL_ENSURE(o3tl::make_unsigned(prefix.getLength()) <= sCleanURL.size(), "How can A match B when A shorter than B?");
            sRet = sCleanURL.substr(prefix.getLength());
            sOldPattern = dsnPrefix;
        }
    }

    return sRet;
}

OUString ODsnTypeCollection::getPrefix(std::u16string_view _sURL) const
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
            OSL_ENSURE(sRet.getLength() <= static_cast<sal_Int32>(_sURL.size()), "How can A match B when A shorter than B?");
            sOldPattern = dsnPrefix;
        }
    }

    return sRet;
}

bool ODsnTypeCollection::hasDriver( std::u16string_view _rAsciiPattern ) const
{
    OUString sPrefix( getPrefix( _rAsciiPattern ) );
    return !sPrefix.isEmpty();
}

bool ODsnTypeCollection::isConnectionUrlRequired(std::u16string_view _sURL) const
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

OUString ODsnTypeCollection::getMediaType(std::u16string_view _sURL) const
{
    const ::comphelper::NamedValueCollection& aFeatures = m_aDriverConfig.getMetaData(_sURL);
    return aFeatures.getOrDefault(u"MediaType"_ustr,OUString());
}

OUString ODsnTypeCollection::getDatasourcePrefixFromMediaType(std::u16string_view _sMediaType,std::u16string_view _sExtension)
{
    OUString sURL, sFallbackURL;
    for (auto& url : m_aDriverConfig.getURLs())
    {
        const ::comphelper::NamedValueCollection& aFeatures = m_aDriverConfig.getMetaData(url);
        if ( aFeatures.getOrDefault(u"MediaType"_ustr,OUString()) == _sMediaType )
        {
            const OUString sFileExtension = aFeatures.getOrDefault(u"Extension"_ustr,OUString());
            if ( _sExtension == sFileExtension )
            {
                sURL = url;
                break;
            }
            if ( sFileExtension.isEmpty() && !_sExtension.empty() )
                sFallbackURL = url;
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
    else if ( _rDsn.startsWithIgnoreAsciiCase("sdbc:ado:access:"))
    {
        OUString sNewFileName;
        if ( ::osl::FileBase::getFileURLFromSystemPath( sUrl, sNewFileName ) == ::osl::FileBase::E_None )
        {
            _sDatabaseName = sNewFileName;
        }
    }
}

OUString ODsnTypeCollection::getJavaDriverClass(std::u16string_view _sURL) const
{
    const ::comphelper::NamedValueCollection& aFeatures = m_aDriverConfig.getProperties(_sURL);
    return aFeatures.getOrDefault(u"JavaDriverClass"_ustr,OUString());
}

bool ODsnTypeCollection::isFileSystemBased(std::u16string_view _sURL) const
{
    const ::comphelper::NamedValueCollection& aFeatures = m_aDriverConfig.getMetaData(_sURL);
    return aFeatures.getOrDefault(u"FileSystemBased"_ustr,false);
}

bool ODsnTypeCollection::supportsTableCreation(std::u16string_view _sURL) const
{
    const ::comphelper::NamedValueCollection& aFeatures = m_aDriverConfig.getMetaData(_sURL);
    return aFeatures.getOrDefault(u"SupportsTableCreation"_ustr,false);
}

bool ODsnTypeCollection::supportsColumnDescription(std::u16string_view _sURL) const
{
    const ::comphelper::NamedValueCollection& aFeatures = m_aDriverConfig.getMetaData(_sURL);
    return aFeatures.getOrDefault(u"SupportsColumnDescription"_ustr,false);
}

bool ODsnTypeCollection::supportsBrowsing(std::u16string_view _sURL) const
{
    const ::comphelper::NamedValueCollection& aFeatures = m_aDriverConfig.getMetaData(_sURL);
    return aFeatures.getOrDefault(u"SupportsBrowsing"_ustr,false);
}

bool ODsnTypeCollection::supportsDBCreation(std::u16string_view _sURL) const
{
    const ::comphelper::NamedValueCollection& aFeatures = m_aDriverConfig.getMetaData(_sURL);
    return aFeatures.getOrDefault(u"SupportsDBCreation"_ustr,false);
}

Sequence<PropertyValue> ODsnTypeCollection::getDefaultDBSettings( std::u16string_view _sURL ) const
{
    const ::comphelper::NamedValueCollection& aProperties = m_aDriverConfig.getProperties(_sURL);
    return aProperties.getPropertyValues();
}

bool ODsnTypeCollection::isEmbeddedDatabase( std::u16string_view _sURL )
{
    return o3tl::starts_with( _sURL, u"sdbc:embedded:" );
}

OUString ODsnTypeCollection::getEmbeddedDatabase()
{
    if (!HAVE_FEATURE_JAVA || officecfg::Office::Common::Misc::ExperimentalMode::get())
        return u"sdbc:embedded:firebird"_ustr;
    else
        return u"sdbc:embedded:hsqldb"_ustr;
}


DATASOURCE_TYPE ODsnTypeCollection::determineType(std::u16string_view _rDsn) const
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

    if (sDsn.startsWithIgnoreAsciiCase("sdbc:embedded:"))
        return DST_EMBEDDED_UNKNOWN;

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
            return DST_MSACCESS;
        }
        return DST_ADO;
    }

    struct KnownPrefix
    {
        const OUString          sPrefix;
        const DATASOURCE_TYPE   eType;
        const bool              bMatchComplete;

        KnownPrefix( OUString _s, const DATASOURCE_TYPE _t, const bool _m )
            :sPrefix(std::move( _s ))
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
        KnownPrefix( u"sdbc:calc:"_ustr,          DST_CALC,               false ),
        KnownPrefix( u"sdbc:writer:"_ustr,        DST_WRITER,             false ),
        KnownPrefix( u"sdbc:flat:"_ustr,          DST_FLAT,               false ),
        KnownPrefix( u"sdbc:odbc:"_ustr,          DST_ODBC,               false ),
        KnownPrefix( u"sdbc:dbase:"_ustr,         DST_DBASE,              false ),
        KnownPrefix( u"sdbc:firebird:"_ustr,      DST_FIREBIRD,           false ),
        KnownPrefix( u"sdbc:mysql:odbc:"_ustr,    DST_MYSQL_ODBC,         false ),
        KnownPrefix( u"sdbc:mysql:jdbc:"_ustr,    DST_MYSQL_JDBC,         false ),
        KnownPrefix( u"sdbc:mysql:mysqlc:"_ustr,  DST_MYSQL_NATIVE,       false ),
        KnownPrefix( u"sdbc:mysqlc:"_ustr,        DST_MYSQL_NATIVE_DIRECT,false ),
        KnownPrefix( u"sdbc:postgresql:"_ustr,    DST_POSTGRES           ,false ),

        KnownPrefix( u"sdbc:address:mozilla:"_ustr,           DST_MOZILLA,            true ),
        KnownPrefix( u"sdbc:address:thunderbird:"_ustr,       DST_THUNDERBIRD,        true ),
        KnownPrefix( u"sdbc:address:ldap:"_ustr,              DST_LDAP,               true ),
        KnownPrefix( u"sdbc:address:outlook"_ustr,            DST_OUTLOOK,            true ),
        KnownPrefix( u"sdbc:address:outlookexp"_ustr,         DST_OUTLOOKEXP,         true ),
        KnownPrefix( u"sdbc:address:evolution:ldap"_ustr,     DST_EVOLUTION_LDAP,     true ),
        KnownPrefix( u"sdbc:address:evolution:groupwise"_ustr,DST_EVOLUTION_GROUPWISE,true ),
        KnownPrefix( u"sdbc:address:evolution:local"_ustr,    DST_EVOLUTION,          true ),
        KnownPrefix( u"sdbc:address:kab"_ustr,                DST_KAB,                true ),
        KnownPrefix( u"sdbc:address:macab"_ustr,              DST_MACAB,              true )
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

void ODsnTypeCollection::fillPageIds(std::u16string_view _sURL,std::vector<sal_Int16>& _rOutPathIds) const
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
        case DST_POSTGRES:
            _rOutPathIds.push_back(PAGE_DBSETUPWIZARD_POSTGRES);
            break;
        case DST_LDAP:
            _rOutPathIds.push_back(PAGE_DBSETUPWIZARD_LDAP);
            break;
        case DST_MSACCESS:
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
        case DST_EMBEDDED_UNKNOWN:
            break;
        default:
            _rOutPathIds.push_back(PAGE_DBSETUPWIZARD_USERDEFINED);
            break;
    }
}

OUString ODsnTypeCollection::getType(std::u16string_view _sURL) const
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

sal_Int32 ODsnTypeCollection::getIndexOf(std::u16string_view _sURL) const
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
    assert(m_pContainer && "ODsnTypeCollection::TypeIterator::TypeIterator : invalid container!");
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

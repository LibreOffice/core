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

#include <unotools/securityoptions.hxx>
#include <unotools/configmgr.hxx>
#include <unotools/configitem.hxx>
#include <unotools/ucbhelper.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/util/XChangesBatch.hpp>
#include <comphelper/propertyvalue.hxx>
#include <comphelper/sequence.hxx>
#include <tools/urlobj.hxx>

#include <unotools/pathoptions.hxx>
#include <officecfg/Office/Common.hxx>

//  namespaces

using namespace ::com::sun::star::uno;

constexpr OUString PROPERTYNAME_MACRO_TRUSTEDAUTHORS = u"TrustedAuthors"_ustr;
constexpr OUString PROPERTYNAME_TRUSTEDAUTHOR_SUBJECTNAME = u"SubjectName"_ustr;
constexpr OUString PROPERTYNAME_TRUSTEDAUTHOR_SERIALNUMBER = u"SerialNumber"_ustr;
constexpr OUString PROPERTYNAME_TRUSTEDAUTHOR_RAWDATA = u"RawData"_ustr;


namespace SvtSecurityOptions
{

bool IsReadOnly( EOption eOption )
{
    bool    bReadonly;
    switch(eOption)
    {
        case SvtSecurityOptions::EOption::SecureUrls :
            bReadonly = officecfg::Office::Common::Security::Scripting::SecureURL::isReadOnly();
            break;
        case SvtSecurityOptions::EOption::DocWarnSaveOrSend:
            bReadonly = officecfg::Office::Common::Security::Scripting::WarnSaveOrSendDoc::isReadOnly();
            break;
        case SvtSecurityOptions::EOption::DocWarnSigning:
            bReadonly = officecfg::Office::Common::Security::Scripting::WarnSignDoc::isReadOnly();
            break;
        case SvtSecurityOptions::EOption::DocWarnPrint:
            bReadonly = officecfg::Office::Common::Security::Scripting::WarnPrintDoc::isReadOnly();
            break;
        case SvtSecurityOptions::EOption::DocWarnCreatePdf:
            bReadonly = officecfg::Office::Common::Security::Scripting::WarnCreatePDF::isReadOnly();
            break;
        case SvtSecurityOptions::EOption::DocWarnRemovePersonalInfo:
            bReadonly = officecfg::Office::Common::Security::Scripting::RemovePersonalInfoOnSaving::isReadOnly();
            break;
        case SvtSecurityOptions::EOption::DocWarnKeepRedlineInfo:
            bReadonly = officecfg::Office::Common::Security::Scripting::KeepRedlineInfoOnSaving::isReadOnly();
            break;
        case SvtSecurityOptions::EOption::DocWarnKeepDocUserInfo:
            bReadonly = officecfg::Office::Common::Security::Scripting::KeepDocUserInfoOnSaving::isReadOnly();
            break;
        case SvtSecurityOptions::EOption::DocWarnKeepNoteAuthorDateInfo:
            bReadonly = officecfg::Office::Common::Security::Scripting::KeepNoteAuthorDateInfoOnSaving::isReadOnly();
            break;
        case SvtSecurityOptions::EOption::DocWarnKeepDocVersionInfo:
            bReadonly = officecfg::Office::Common::Security::Scripting::KeepDocVersionInfoOnSaving::isReadOnly();
            break;
        case SvtSecurityOptions::EOption::DocWarnRecommendPassword:
            bReadonly = officecfg::Office::Common::Security::Scripting::RecommendPasswordProtection::isReadOnly();
            break;
        case SvtSecurityOptions::EOption::MacroSecLevel:
            bReadonly = officecfg::Office::Common::Security::Scripting::MacroSecurityLevel::isReadOnly();
            break;
        case SvtSecurityOptions::EOption::MacroTrustedAuthors:
            // the officecfg does not expose isReadOnly for a ConfigurationSet, so we have to code this ourself
            bReadonly =
              comphelper::detail::ConfigurationWrapper::get().isReadOnly(u"/org.openoffice.Office.Common/Security/Scripting/TrustedAuthors"_ustr);
            break;
        case SvtSecurityOptions::EOption::CtrlClickHyperlink:
            bReadonly = officecfg::Office::Common::Security::Scripting::HyperlinksWithCtrlClick::isReadOnly();
            break;
        case SvtSecurityOptions::EOption::BlockUntrustedRefererLinks:
            bReadonly = officecfg::Office::Common::Security::Scripting::BlockUntrustedRefererLinks::isReadOnly();
            break;
        case SvtSecurityOptions::EOption::DisableActiveContent:
            bReadonly = officecfg::Office::Common::Security::Scripting::DisableActiveContent::isReadOnly() ||
                        officecfg::Office::Common::Security::Scripting::DisableOLEAutomation::isReadOnly();
            break;

        default:
            assert(false);
            bReadonly = true;
    }

    return bReadonly;
}

std::vector< OUString > GetSecureURLs()
{
    if (utl::ConfigManager::IsFuzzing())
        return {};
    std::vector<OUString> aRet = comphelper::sequenceToContainer<std::vector<OUString>>(
            officecfg::Office::Common::Security::Scripting::SecureURL::get());
    SvtPathOptions  aOpt;
    std::transform(aRet.begin(), aRet.end(), aRet.begin(),
        [&aOpt](const OUString& rUrl) -> OUString { return aOpt.SubstituteVariable( rUrl ); });
    return aRet;
}

void SetSecureURLs( std::vector< OUString >&& urlList )
{
//    DBG_ASSERT(!officecfg::SecureURL::isReadOnly(), "SvtSecurityOptions_Impl::SetSecureURLs()\nYou tried to write on a readonly value!\n");
//    if (officecfg::SecureURL::isReadOnly())
//        return;
    std::vector< OUString > lURLs( std::move(urlList) );
    SvtPathOptions aOpt;
    std::transform(lURLs.begin(), lURLs.end(), lURLs.begin(),
        [&aOpt](const OUString& rUrl) -> OUString { return aOpt.UseVariable( rUrl ); });
    std::shared_ptr<comphelper::ConfigurationChanges> xChanges = comphelper::ConfigurationChanges::create();
    officecfg::Office::Common::Security::Scripting::SecureURL::set(comphelper::containerToSequence(lURLs), xChanges);
    xChanges->commit();
}

bool isSecureMacroUri(
    OUString const & uri, OUString const & referer)
{
    switch (INetURLObject(uri).GetProtocol()) {
    case INetProtocol::Macro:
        if (uri.startsWithIgnoreAsciiCase("macro:///")) {
            // Denotes an App-BASIC macro (see SfxMacroLoader::loadMacro), which
            // is considered safe:
            return true;
        }
        [[fallthrough]];
    case INetProtocol::Slot:
        return referer.equalsIgnoreAsciiCase("private:user")
            || isTrustedLocationUri(referer);
    default:
        return true;
    }
}

bool isUntrustedReferer(OUString const & referer)
{
    return IsOptionSet(EOption::BlockUntrustedRefererLinks)
        && !(referer.isEmpty() || referer.startsWithIgnoreAsciiCase("private:")
             || isTrustedLocationUri(referer));
}

bool isTrustedLocationUri(OUString const & uri)
{
    for (const auto & url : GetSecureURLs())
    {
        if (utl::UCBContentHelper::IsSubPath(url, uri))
        {
            return true;
        }
    }
    return false;
}

bool isTrustedLocationUriForUpdatingLinks(OUString const & uri)
{
    return GetMacroSecurityLevel() == 0 || uri.isEmpty()
        || uri.startsWithIgnoreAsciiCase("private:")
        || isTrustedLocationUri(uri);
}

sal_Int32 GetMacroSecurityLevel()
{
    return utl::ConfigManager::IsFuzzing() ? 3 : officecfg::Office::Common::Security::Scripting::MacroSecurityLevel::get();
}

void SetMacroSecurityLevel( sal_Int32 _nLevel )
{
    if (utl::ConfigManager::IsFuzzing() || officecfg::Office::Common::Security::Scripting::MacroSecurityLevel::isReadOnly())
        return;

    if( _nLevel > 3 || _nLevel < 0 )
        _nLevel = 3;

    std::shared_ptr<comphelper::ConfigurationChanges> xChanges = comphelper::ConfigurationChanges::create();
    officecfg::Office::Common::Security::Scripting::MacroSecurityLevel::set(_nLevel, xChanges);
    xChanges->commit();
}

bool IsMacroDisabled()
{
    return utl::ConfigManager::IsFuzzing() || officecfg::Office::Common::Security::Scripting::DisableMacrosExecution::get();
}

std::vector< SvtSecurityOptions::Certificate > GetTrustedAuthors()
{
    Reference<css::container::XHierarchicalNameAccess> xHierarchyAccess = utl::ConfigManager::acquireTree(u"Office.Common/Security/Scripting");
    const Sequence< OUString > lAuthors = utl::ConfigItem::GetNodeNames( xHierarchyAccess, PROPERTYNAME_MACRO_TRUSTEDAUTHORS, utl::ConfigNameFormat::LocalPath );
    sal_Int32 c1 = lAuthors.getLength();
    if( !c1 )
        return {};

    sal_Int32               c2 = c1 * 3;                // 3 Properties inside Struct TrustedAuthor
    Sequence< OUString >    lAllAuthors( c2 );
    auto plAllAuthors = lAllAuthors.getArray();
    sal_Int32               i2 = 0;
    OUString                aSep( "/" );
    for( const auto& rAuthor : lAuthors )
    {
        plAllAuthors[ i2 ] = PROPERTYNAME_MACRO_TRUSTEDAUTHORS + aSep + rAuthor + aSep + PROPERTYNAME_TRUSTEDAUTHOR_SUBJECTNAME;
        ++i2;
        plAllAuthors[ i2 ] = PROPERTYNAME_MACRO_TRUSTEDAUTHORS + aSep + rAuthor + aSep + PROPERTYNAME_TRUSTEDAUTHOR_SERIALNUMBER;
        ++i2;
        plAllAuthors[ i2 ] = PROPERTYNAME_MACRO_TRUSTEDAUTHORS + aSep + rAuthor + aSep + PROPERTYNAME_TRUSTEDAUTHOR_RAWDATA;
        ++i2;
    }

    Sequence< Any >         lValues = utl::ConfigItem::GetProperties( xHierarchyAccess, lAllAuthors, /*bAllLocales*/false );
    if( lValues.getLength() != c2 )
        return {};

    std::vector< SvtSecurityOptions::Certificate > aTrustedAuthors;
    SvtSecurityOptions::Certificate aCert;
    i2 = 0;
    for( sal_Int32 i1 = 0; i1 < c1; ++i1 )
    {
        lValues[ i2 ] >>= aCert.SubjectName;
        ++i2;
        lValues[ i2 ] >>= aCert.SerialNumber;
        ++i2;
        lValues[ i2 ] >>= aCert.RawData;
        ++i2;
        // Filter out TrustedAuthor entries with empty RawData, which
        // would cause an unexpected std::bad_alloc in
        // SecurityEnvironment_NssImpl::createCertificateFromAscii and
        // have been observed in the wild (fdo#55019):
        if( !aCert.RawData.isEmpty() )
        {
            aTrustedAuthors.push_back( aCert );
        }
    }
    return aTrustedAuthors;
}

void SetTrustedAuthors( const std::vector< Certificate >& rAuthors )
{
//    DBG_ASSERT(!m_bROTrustedAuthors, "SvtSecurityOptions_Impl::SetTrustedAuthors()\nYou tried to write on a readonly value!\n");
//    if( m_bROTrustedAuthors )
//        return;

    Reference<css::container::XHierarchicalNameAccess> xHierarchyAccess = utl::ConfigManager::acquireTree(u"Office.Common/Security/Scripting");

    // first, clear existing entries
    {
        Reference<css::container::XNameContainer> xCont;
        xHierarchyAccess->getByHierarchicalName(PROPERTYNAME_MACRO_TRUSTEDAUTHORS) >>= xCont;
        const Sequence< OUString > aNames = xCont->getElementNames();
        Reference<css::util::XChangesBatch> xBatch(xHierarchyAccess, UNO_QUERY);
        for (const OUString& rName : aNames)
            xCont->removeByName(rName);
        xBatch->commitChanges();
    }

    sal_Int32   nCnt = rAuthors.size();
    for( sal_Int32 i = 0; i < nCnt; ++i )
    {
        OUString aPrefix(
            PROPERTYNAME_MACRO_TRUSTEDAUTHORS + "/a"
            + OUString::number(i) + "/");
        Sequence< css::beans::PropertyValue >    lPropertyValues{
            comphelper::makePropertyValue(aPrefix + PROPERTYNAME_TRUSTEDAUTHOR_SUBJECTNAME,
                                          rAuthors[i].SubjectName),
            comphelper::makePropertyValue(aPrefix + PROPERTYNAME_TRUSTEDAUTHOR_SERIALNUMBER,
                                          rAuthors[i].SerialNumber),
            comphelper::makePropertyValue(aPrefix + PROPERTYNAME_TRUSTEDAUTHOR_RAWDATA,
                                          rAuthors[i].RawData)
        };

        utl::ConfigItem::SetSetProperties( xHierarchyAccess, PROPERTYNAME_MACRO_TRUSTEDAUTHORS, lPropertyValues );
    }
}

bool IsOptionSet( EOption eOption )
{
    if (utl::ConfigManager::IsFuzzing())
        return false;
    bool    bSet = false;
    switch(eOption)
    {
        case SvtSecurityOptions::EOption::DocWarnSaveOrSend:
            bSet = officecfg::Office::Common::Security::Scripting::WarnSaveOrSendDoc::get();
            break;
        case SvtSecurityOptions::EOption::DocWarnSigning:
            bSet = officecfg::Office::Common::Security::Scripting::WarnSignDoc::get();
            break;
        case SvtSecurityOptions::EOption::DocWarnPrint:
            bSet = officecfg::Office::Common::Security::Scripting::WarnPrintDoc::get();
            break;
        case SvtSecurityOptions::EOption::DocWarnCreatePdf:
            bSet = officecfg::Office::Common::Security::Scripting::WarnCreatePDF::get();
            break;
        case SvtSecurityOptions::EOption::DocWarnRemovePersonalInfo:
            bSet = officecfg::Office::Common::Security::Scripting::RemovePersonalInfoOnSaving::get();
            break;
        case SvtSecurityOptions::EOption::DocWarnKeepRedlineInfo:
            bSet = officecfg::Office::Common::Security::Scripting::KeepRedlineInfoOnSaving::get();
            break;
        case SvtSecurityOptions::EOption::DocWarnKeepDocUserInfo:
            bSet = officecfg::Office::Common::Security::Scripting::KeepDocUserInfoOnSaving::get();
            break;
        case SvtSecurityOptions::EOption::DocWarnKeepNoteAuthorDateInfo:
            bSet = officecfg::Office::Common::Security::Scripting::KeepNoteAuthorDateInfoOnSaving::get();
            break;
        case SvtSecurityOptions::EOption::DocWarnKeepDocVersionInfo:
            bSet = officecfg::Office::Common::Security::Scripting::KeepDocVersionInfoOnSaving::get();
            break;
        case SvtSecurityOptions::EOption::DocWarnRecommendPassword:
            bSet = officecfg::Office::Common::Security::Scripting::RecommendPasswordProtection::get();
            break;
        case SvtSecurityOptions::EOption::CtrlClickHyperlink:
            bSet = officecfg::Office::Common::Security::Scripting::HyperlinksWithCtrlClick::get();
            break;
        case SvtSecurityOptions::EOption::BlockUntrustedRefererLinks:
            bSet = officecfg::Office::Common::Security::Scripting::BlockUntrustedRefererLinks::get();
            break;
        case SvtSecurityOptions::EOption::DisableActiveContent:
            bSet = officecfg::Office::Common::Security::Scripting::DisableActiveContent::get() &&
                   officecfg::Office::Common::Security::Scripting::DisableOLEAutomation::get();
            break;

        default:
            assert(false);
    }

    return bSet;
}

void SetOption( EOption eOption, bool bValue )
{
    std::shared_ptr<comphelper::ConfigurationChanges> xChanges = comphelper::ConfigurationChanges::create();
    switch(eOption)
    {
        case SvtSecurityOptions::EOption::DocWarnSaveOrSend:
             officecfg::Office::Common::Security::Scripting::WarnSaveOrSendDoc::set(bValue, xChanges);
            break;
        case SvtSecurityOptions::EOption::DocWarnSigning:
             officecfg::Office::Common::Security::Scripting::WarnSignDoc::set(bValue, xChanges);
            break;
        case SvtSecurityOptions::EOption::DocWarnPrint:
             officecfg::Office::Common::Security::Scripting::WarnPrintDoc::set(bValue, xChanges);
            break;
        case SvtSecurityOptions::EOption::DocWarnCreatePdf:
             officecfg::Office::Common::Security::Scripting::WarnCreatePDF::set(bValue, xChanges);
            break;
        case SvtSecurityOptions::EOption::DocWarnRemovePersonalInfo:
             officecfg::Office::Common::Security::Scripting::RemovePersonalInfoOnSaving::set(bValue, xChanges);
            break;
        case SvtSecurityOptions::EOption::DocWarnKeepRedlineInfo:
             officecfg::Office::Common::Security::Scripting::KeepRedlineInfoOnSaving::set(bValue, xChanges);
            break;
        case SvtSecurityOptions::EOption::DocWarnKeepDocUserInfo:
             officecfg::Office::Common::Security::Scripting::KeepDocUserInfoOnSaving::set(bValue, xChanges);
            break;
        case SvtSecurityOptions::EOption::DocWarnKeepNoteAuthorDateInfo:
             officecfg::Office::Common::Security::Scripting::KeepNoteAuthorDateInfoOnSaving::set(bValue, xChanges);
            break;
        case SvtSecurityOptions::EOption::DocWarnKeepDocVersionInfo:
             officecfg::Office::Common::Security::Scripting::KeepDocVersionInfoOnSaving::set(bValue, xChanges);
            break;
        case SvtSecurityOptions::EOption::DocWarnRecommendPassword:
             officecfg::Office::Common::Security::Scripting::RecommendPasswordProtection::set(bValue, xChanges);
            break;
        case SvtSecurityOptions::EOption::CtrlClickHyperlink:
             officecfg::Office::Common::Security::Scripting::HyperlinksWithCtrlClick::set(bValue, xChanges);
            break;
        case SvtSecurityOptions::EOption::BlockUntrustedRefererLinks:
             officecfg::Office::Common::Security::Scripting::BlockUntrustedRefererLinks::set(bValue, xChanges);
            break;
        case SvtSecurityOptions::EOption::DisableActiveContent:
            officecfg::Office::Common::Security::Scripting::DisableActiveContent::set(bValue, xChanges);
            officecfg::Office::Common::Security::Scripting::DisableOLEAutomation::set(bValue, xChanges);
            break;

        default:
            assert(false);
    }
    xChanges->commit();
}

} // namespace SvtSecurityOptions


// map personal info strings to 1, 2, ... to remove personal info
size_t SvtSecurityMapPersonalInfo::GetInfoID( const OUString sPersonalInfo )
{
    auto aIter = aInfoIDs.find( sPersonalInfo );
    if ( aIter == aInfoIDs.end() )
    {
        size_t nNewID = aInfoIDs.size() + 1;
        aInfoIDs[sPersonalInfo] = nNewID;
        return nNewID;
    }

    return aIter->second;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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

#include <sal/config.h>

#include <string_view>

#include <tools/urlobj.hxx>
#include <ucbhelper/content.hxx>
#include <tools/debug.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/util/thePathSettings.hpp>
#include <o3tl/typed_flags_set.hxx>

#include <linguistic/misc.hxx>

using namespace com::sun::star;

namespace {

/// Flags to be used with the multi-path related functions
/// @see GetDictionaryPaths
enum class DictionaryPathFlags
{
    NONE      = 0x00,
    INTERNAL  = 0x01,
    USER      = 0x02,
};

}

namespace o3tl
{
    template<> struct typed_flags<DictionaryPathFlags> : is_typed_flags<DictionaryPathFlags, 0x03> {};
}
#define PATH_FLAG_ALL       (DictionaryPathFlags::INTERNAL | DictionaryPathFlags::USER)

namespace linguistic
{


bool FileExists( const OUString &rMainURL )
{
    bool bExists = false;
    if (!rMainURL.isEmpty())
    {
        try
        {
            ::ucbhelper::Content aContent( rMainURL,
                    uno::Reference< css::ucb::XCommandEnvironment >(),
                    comphelper::getProcessComponentContext());
            bExists = aContent.isDocument();
        }
        catch (uno::Exception &)
        {
        }
    }
    return bExists;
}

static std::vector< OUString > GetMultiPaths_Impl(
    std::u16string_view rPathPrefix,
    DictionaryPathFlags nPathFlags )
{
    std::vector< OUString >     aRes;
    uno::Sequence< OUString >   aInternalPaths;
    uno::Sequence< OUString >   aUserPaths;
    OUString                    aWritablePath;

    bool bSuccess = true;
    uno::Reference< uno::XComponentContext >  xContext( comphelper::getProcessComponentContext() );
    try
    {
        OUString aInternal( OUString::Concat(rPathPrefix) + "_internal" );
        OUString aUser( OUString::Concat(rPathPrefix) + "_user" );
        OUString aWriteable( OUString::Concat(rPathPrefix) + "_writable" );

        uno::Reference< util::XPathSettings > xPathSettings =
            util::thePathSettings::get( xContext );
        xPathSettings->getPropertyValue( aInternal )  >>= aInternalPaths;
        xPathSettings->getPropertyValue( aUser )      >>= aUserPaths;
        xPathSettings->getPropertyValue( aWriteable ) >>= aWritablePath;
    }
    catch (uno::Exception &)
    {
        bSuccess = false;
    }
    if (bSuccess)
    {
        // build resulting sequence by adding the paths in the following order:
        // 1. writable path
        // 2. all user paths
        // 3. all internal paths
        sal_Int32 nMaxEntries = aInternalPaths.getLength() + aUserPaths.getLength();
        if (!aWritablePath.isEmpty())
            ++nMaxEntries;
        aRes.reserve( nMaxEntries );
        if (!aWritablePath.isEmpty())
            aRes.push_back(aWritablePath);

        auto lPathIsNotEmpty = [](const OUString& rPath) { return !rPath.isEmpty(); };

        if (nPathFlags & DictionaryPathFlags::USER)
            std::copy_if(std::cbegin(aUserPaths), std::cend(aUserPaths), std::back_inserter(aRes), lPathIsNotEmpty);

        if (nPathFlags & DictionaryPathFlags::INTERNAL)
            std::copy_if(std::cbegin(aInternalPaths), std::cend(aInternalPaths), std::back_inserter(aRes), lPathIsNotEmpty);
    }

    return aRes;
}

OUString GetDictionaryWriteablePath()
{
    std::vector< OUString > aPaths(
        GetMultiPaths_Impl( u"Dictionary", DictionaryPathFlags::NONE ) );
    DBG_ASSERT( aPaths.size() == 1, "Dictionary_writable path corrupted?" );
    OUString aRes;
    if (!aPaths.empty())
        aRes = aPaths[0];
    return aRes;
}

std::vector< OUString > GetDictionaryPaths()
{
    return GetMultiPaths_Impl( u"Dictionary", PATH_FLAG_ALL );
}

OUString  GetWritableDictionaryURL( const OUString &rDicName )
{
    // new user writable dictionaries should be created in the 'writable' path
    OUString aDirName( GetDictionaryWriteablePath() );

    // build URL to use for a new (persistent) dictionary
    INetURLObject aURLObj;
    aURLObj.SetSmartProtocol( INetProtocol::File );
    aURLObj.SetSmartURL( aDirName );
    DBG_ASSERT(!aURLObj.HasError(), "lng : invalid URL");
    aURLObj.Append( rDicName, INetURLObject::EncodeMechanism::All );
    DBG_ASSERT(!aURLObj.HasError(), "lng : invalid URL");

    // DecodeMechanism::NONE preserves the escape sequences that might be included in aDirName
    // depending on the characters used in the path string. (Needed when comparing
    // the dictionary URL with GetDictionaryWriteablePath in DicList::createDictionary.)
    return aURLObj.GetMainURL( INetURLObject::DecodeMechanism::NONE );
}

}   // namespace linguistic

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

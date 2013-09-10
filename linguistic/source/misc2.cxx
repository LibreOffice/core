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

#include <tools/string.hxx>
#include <tools/urlobj.hxx>
#include <ucbhelper/content.hxx>
#include <tools/debug.hxx>
#include <unotools/pathoptions.hxx>
#include <comphelper/processfactory.hxx>
#include <unotools/localfilehelper.hxx>
#include <unotools/localedatawrapper.hxx>
#include <unotools/ucbhelper.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XFastPropertySet.hpp>
#include <com/sun/star/beans/PropertyValues.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/util/PathSettings.hpp>

#include "linguistic/misc.hxx"

using namespace com::sun::star;

namespace linguistic
{


sal_Bool FileExists( const OUString &rMainURL )
{
    sal_Bool bExists = sal_False;
    if (!rMainURL.isEmpty())
    {
        try
        {
            ::ucbhelper::Content aContent( rMainURL,
                    uno::Reference< ::com::sun::star::ucb::XCommandEnvironment >(),
                    comphelper::getProcessComponentContext());
            bExists = aContent.isDocument();
        }
        catch (uno::Exception &)
        {
        }
    }
    return bExists;
}

static uno::Sequence< OUString > GetMultiPaths_Impl(
    const OUString &rPathPrefix,
    sal_Int16 nPathFlags )
{
    uno::Sequence< OUString >   aRes;
    uno::Sequence< OUString >   aInternalPaths;
    uno::Sequence< OUString >   aUserPaths;
    OUString                    aWritablePath;

    bool bSuccess = true;
    uno::Reference< uno::XComponentContext >  xContext( comphelper::getProcessComponentContext() );
    try
    {
        OUString aInternal( rPathPrefix + "_internal" );
        OUString aUser( rPathPrefix + "_user" );
        OUString aWriteable( rPathPrefix + "_writable" );

        uno::Reference< util::XPathSettings > xPathSettings =
            util::PathSettings::create( xContext );
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
        aRes.realloc( nMaxEntries );
        OUString *pRes = aRes.getArray();
        sal_Int32 nCount = 0;   // number of actually added entries
        if ((nPathFlags & PATH_FLAG_WRITABLE) && !aWritablePath.isEmpty())
            pRes[ nCount++ ] = aWritablePath;
        for (int i = 0;  i < 2;  ++i)
        {
            const uno::Sequence< OUString > &rPathSeq = i == 0 ? aUserPaths : aInternalPaths;
            const OUString *pPathSeq = rPathSeq.getConstArray();
            for (sal_Int32 k = 0;  k < rPathSeq.getLength();  ++k)
            {
                const bool bAddUser     = &rPathSeq == &aUserPaths     && (nPathFlags & PATH_FLAG_USER);
                const bool bAddInternal = &rPathSeq == &aInternalPaths && (nPathFlags & PATH_FLAG_INTERNAL);
                if ((bAddUser || bAddInternal) && !pPathSeq[k].isEmpty())
                    pRes[ nCount++ ] = pPathSeq[k];
            }
        }
        aRes.realloc( nCount );
    }

    return aRes;
}

OUString GetDictionaryWriteablePath()
{
    uno::Sequence< OUString > aPaths( GetMultiPaths_Impl( "Dictionary", PATH_FLAG_WRITABLE ) );
    DBG_ASSERT( aPaths.getLength() == 1, "Dictionary_writable path corrupted?" );
    OUString aRes;
    if (aPaths.getLength() > 0)
        aRes = aPaths[0];
    return aRes;
}

uno::Sequence< OUString > GetDictionaryPaths( sal_Int16 nPathFlags )
{
    return GetMultiPaths_Impl( "Dictionary", nPathFlags );
}

OUString  GetWritableDictionaryURL( const OUString &rDicName )
{
    // new user writable dictionaries should be created in the 'writable' path
    OUString aDirName( GetDictionaryWriteablePath() );

    // build URL to use for a new (persistent) dictionary
    INetURLObject aURLObj;
    aURLObj.SetSmartProtocol( INET_PROT_FILE );
    aURLObj.SetSmartURL( aDirName );
    DBG_ASSERT(!aURLObj.HasError(), "lng : invalid URL");
    aURLObj.Append( rDicName, INetURLObject::ENCODE_ALL );
    DBG_ASSERT(!aURLObj.HasError(), "lng : invalid URL");

    // NO_DECODE preserves the escape sequences that might be included in aDirName
    // depending on the characters used in the path string. (Needed when comparing
    // the dictionary URL with GetDictionaryWriteablePath in DicList::createDictionary.)
    return aURLObj.GetMainURL( INetURLObject::NO_DECODE );
}

}   // namespace linguistic

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_linguistic.hxx"
#include <tools/string.hxx>
#include <tools/fsys.hxx>
#include <tools/urlobj.hxx>
#include <ucbhelper/content.hxx>
#include <tools/debug.hxx>
#include <unotools/pathoptions.hxx>
#include <unotools/processfactory.hxx>
#include <unotools/localfilehelper.hxx>
#include <unotools/localedatawrapper.hxx>
#include <unotools/ucbhelper.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XFastPropertySet.hpp>

#include <com/sun/star/beans/PropertyValues.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Reference.h>

#include "misc.hxx"

using namespace com::sun::star;

namespace linguistic
{

///////////////////////////////////////////////////////////////////////////

BOOL FileExists( const String &rMainURL )
{
    BOOL bExists = FALSE;
    if (rMainURL.Len())
    {
        try
        {
            ::ucbhelper::Content aContent( rMainURL,
                    uno::Reference< ::com::sun::star::ucb::XCommandEnvironment >());
            bExists = aContent.isDocument();
        }
        catch (uno::Exception &)
        {
        }
    }
    return bExists;
}

///////////////////////////////////////////////////////////////////////////

rtl::OUString StripTrailingChars( rtl::OUString &rTxt, sal_Unicode cChar )
{
    sal_Int32 nTrailing = 0;
    sal_Int32 nTxtLen = rTxt.getLength();
    sal_Int32 nIdx = nTxtLen - 1;
    while (nIdx >= 0 && rTxt[ nIdx-- ] == cChar)
        ++nTrailing;

    rtl::OUString aRes( rTxt.copy( nTxtLen - nTrailing ) );
    rTxt = rTxt.copy( 0, nTxtLen - nTrailing );
    return aRes;
}

///////////////////////////////////////////////////////////////////////////

static uno::Sequence< rtl::OUString > GetMultiPaths_Impl(
    const rtl::OUString &rPathPrefix,
    sal_Int16 nPathFlags )
{
    uno::Sequence< rtl::OUString >   aRes;
    uno::Sequence< rtl::OUString >   aInternalPaths;
    uno::Sequence< rtl::OUString >   aUserPaths;
    rtl::OUString                    aWritablePath;

    bool bSuccess = true;
    uno::Reference< lang::XMultiServiceFactory >  xMgr( utl::getProcessServiceFactory() );
    if (xMgr.is())
    {
        try
        {
            String aInternal( rPathPrefix );
            String aUser( rPathPrefix );
            String aWriteable( rPathPrefix );
            aInternal .AppendAscii( "_internal" );
            aUser     .AppendAscii( "_user" );
            aWriteable.AppendAscii( "_writable" );

            uno::Reference< beans::XPropertySet > xPathSettings( xMgr->createInstance(
                    A2OU( "com.sun.star.util.PathSettings" ) ), uno::UNO_QUERY_THROW );
            xPathSettings->getPropertyValue( aInternal )  >>= aInternalPaths;
            xPathSettings->getPropertyValue( aUser )      >>= aUserPaths;
            xPathSettings->getPropertyValue( aWriteable ) >>= aWritablePath;
        }
        catch (uno::Exception &)
        {
            bSuccess = false;
        }
    }
    if (bSuccess)
    {
        // build resulting sequence by adding the pathes in the following order:
        // 1. writable path
        // 2. all user pathes
        // 3. all internal pathes
        sal_Int32 nMaxEntries = aInternalPaths.getLength() + aUserPaths.getLength();
        if (aWritablePath.getLength() > 0)
            ++nMaxEntries;
        aRes.realloc( nMaxEntries );
        rtl::OUString *pRes = aRes.getArray();
        sal_Int32 nCount = 0;   // number of actually added entries
        if ((nPathFlags & PATH_FLAG_WRITABLE) && aWritablePath.getLength() != 0)
            pRes[ nCount++ ] = aWritablePath;
        for (int i = 0;  i < 2;  ++i)
        {
            const uno::Sequence< rtl::OUString > &rPathSeq = i == 0 ? aUserPaths : aInternalPaths;
            const rtl::OUString *pPathSeq = rPathSeq.getConstArray();
            for (sal_Int32 k = 0;  k < rPathSeq.getLength();  ++k)
            {
                const bool bAddUser     = &rPathSeq == &aUserPaths     && (nPathFlags & PATH_FLAG_USER);
                const bool bAddInternal = &rPathSeq == &aInternalPaths && (nPathFlags & PATH_FLAG_INTERNAL);
                if ((bAddUser || bAddInternal) && pPathSeq[k].getLength() > 0)
                    pRes[ nCount++ ] = pPathSeq[k];
            }
        }
        aRes.realloc( nCount );
    }

    return aRes;
}

rtl::OUString GetDictionaryWriteablePath()
{
    uno::Sequence< rtl::OUString > aPaths( GetMultiPaths_Impl( A2OU("Dictionary"), PATH_FLAG_WRITABLE ) );
    DBG_ASSERT( aPaths.getLength() == 1, "Dictionary_writable path corrupted?" );
    String aRes;
    if (aPaths.getLength() > 0)
        aRes = aPaths[0];
    return aRes;
}

uno::Sequence< rtl::OUString > GetDictionaryPaths( sal_Int16 nPathFlags )
{
    return GetMultiPaths_Impl( A2OU("Dictionary"), nPathFlags );
}

uno::Sequence< rtl::OUString > GetLinguisticPaths( sal_Int16 nPathFlags )
{
    return GetMultiPaths_Impl( A2OU("Linguistic"), nPathFlags );
}

String  GetWritableDictionaryURL( const String &rDicName )
{
    // new user writable dictionaries should be created in the 'writable' path
    String aDirName( GetDictionaryWriteablePath() );

    // build URL to use for a new (persistent) dictionary
    INetURLObject aURLObj;
    aURLObj.SetSmartProtocol( INET_PROT_FILE );
    aURLObj.SetSmartURL( aDirName );
    DBG_ASSERT(!aURLObj.HasError(), "lng : invalid URL");
    aURLObj.Append( rDicName, INetURLObject::ENCODE_ALL );
    DBG_ASSERT(!aURLObj.HasError(), "lng : invalid URL");

    return aURLObj.GetMainURL( INetURLObject::DECODE_TO_IURI );
}


String SearchFileInPaths(
    const String &rFile,
    const uno::Sequence< rtl::OUString > &rPaths )
{
    //!! see also SvtPathOptions::SearchFile for the riginal code

    String aRes;

    // check in all paths...
    const sal_Int32 nPaths = rPaths.getLength();
    for (sal_Int32 k = 0;  k < nPaths;  ++k)
    {
        BOOL bIsURL = TRUE;
        INetURLObject aObj( rPaths[k] );
        if ( aObj.HasError() )
        {
            bIsURL = FALSE;
            String aURL;
            if ( utl::LocalFileHelper::ConvertPhysicalNameToURL( rPaths[k], aURL ) )
                aObj.SetURL( aURL );
        }

        xub_StrLen i, nCount = rFile.GetTokenCount( '/' );
        for ( i = 0; i < nCount; ++i )
            aObj.insertName( rFile.GetToken( i, '/' ) );
        bool bRet = ::utl::UCBContentHelper::Exists( aObj.GetMainURL( INetURLObject::NO_DECODE ) );

        if ( bRet )
        {
            if ( !bIsURL )
                ::utl::LocalFileHelper::ConvertURLToPhysicalName(
                                    aObj.GetMainURL( INetURLObject::NO_DECODE ), aRes );
            else
                aRes = aObj.GetMainURL( INetURLObject::NO_DECODE );
            break;
        }
    }

    return aRes;
}


}   // namespace linguistic


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

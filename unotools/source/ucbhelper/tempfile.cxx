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

#include <cassert>

#include <com/sun/star/ucb/UniversalContentBroker.hpp>
#include <comphelper/processfactory.hxx>
#include <unotools/tempfile.hxx>
#include <unotools/localfilehelper.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <rtl/ustring.hxx>
#include <rtl/instance.hxx>
#include <osl/detail/file.h>
#include <osl/file.hxx>
#include <tools/time.hxx>
#include <tools/debug.hxx>
#include <stdio.h>

#ifdef UNX
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#endif

using namespace osl;

namespace
{
    struct TempNameBase_Impl
        : public rtl::Static< OUString, TempNameBase_Impl > {};
}

namespace utl
{

OUString getParentName( const OUString& aFileName )
{
    sal_Int32 lastIndex = aFileName.lastIndexOf( '/' );
    OUString aParent = aFileName.copy( 0, lastIndex );

    if( aParent.endsWith(":") && aParent.getLength() == 6 )
        aParent += "/";

    if( aParent.equalsIgnoreAsciiCase( "file://" ) )
        aParent = "file:///";

    return aParent;
}

bool ensuredir( const OUString& rUnqPath )
{
    OUString aPath;
    if ( rUnqPath.isEmpty() )
        return false;

    // remove trailing slash
    if ( rUnqPath.endsWith("/") )
        aPath = rUnqPath.copy( 0, rUnqPath.getLength() - 1 );
    else
        aPath = rUnqPath;

    // HACK: create directory on a mount point with nobrowse option
    // returns ENOSYS in any case !!
    osl::Directory aDirectory( aPath );
    osl::FileBase::RC nError = aDirectory.open();
    aDirectory.close();
    if( nError == osl::File::E_None )
        return true;

    // try to create the directory
    nError = osl::Directory::create( aPath );
    bool  bSuccess = ( nError == osl::File::E_None || nError == osl::FileBase::E_EXIST );
    if( !bSuccess )
    {
        // perhaps parent(s) don't exist
        OUString aParentDir = getParentName( aPath );
        if ( aParentDir != aPath )
        {
            bSuccess = ensuredir( getParentName( aPath ) );

            // After parent directory structure exists try it one's more
            if ( bSuccess )
            {
                // Parent directory exists, retry creation of directory
                nError = osl::Directory::create( aPath );
                bSuccess =( nError == osl::File::E_None || nError == osl::FileBase::E_EXIST );
            }
        }
    }

    return bSuccess;
}

OUString ConstructTempDir_Impl( const OUString* pParent )
{
    OUString aName;
    if ( pParent && !pParent->isEmpty() )
    {
        com::sun::star::uno::Reference<
            com::sun::star::ucb::XUniversalContentBroker > pBroker(
                com::sun::star::ucb::UniversalContentBroker::create(
                    comphelper::getProcessComponentContext() ) );

        // test for valid filename
        OUString aRet;
        if ((osl::FileBase::getSystemPathFromFileURL(*pParent, aRet)
             == osl::FileBase::E_None)
            && (osl::FileBase::getFileURLFromSystemPath(aRet, aRet)
                == osl::FileBase::E_None))
        {
            ::osl::DirectoryItem aItem;
            sal_Int32 i = aRet.getLength();
            if ( aRet[i-1] == '/' )
                i--;

            if ( DirectoryItem::get( aRet.copy(0, i), aItem ) == FileBase::E_None )
                aName = aRet;
        }
    }

    if ( aName.isEmpty() )
    {
        OUString &rTempNameBase_Impl = TempNameBase_Impl::get();
        if (rTempNameBase_Impl.isEmpty())
        {
            OUString ustrTempDirURL;
            ::osl::FileBase::RC rc = ::osl::File::getTempDirURL(
                ustrTempDirURL );
            if (rc == ::osl::FileBase::E_None)
                rTempNameBase_Impl = ustrTempDirURL;
        }
        // if no parent or invalid parent : use default directory
        DBG_ASSERT( !rTempNameBase_Impl.isEmpty(), "No TempDir!" );
        aName = rTempNameBase_Impl;
        ensuredir( aName );
    }

    // Make sure that directory ends with a separator
    if( !aName.isEmpty() && !aName.endsWith("/") )
        aName += "/";

    return aName;
}

class Tokens {
public:
    virtual bool next(OUString *) = 0;

protected:
    virtual ~Tokens() {} // avoid warnings
};

class SequentialTokens: public Tokens {
public:
    explicit SequentialTokens(bool showZero): m_value(0), m_show(showZero) {}

    bool next(OUString * token) override {
        assert(token != 0);
        if (m_value == SAL_MAX_UINT32) {
            return false;
        }
        *token = m_show ? OUString::number(m_value) : OUString();
        ++m_value;
        m_show = true;
        return true;
    }

private:
    sal_uInt32 m_value;
    bool m_show;
};

class UniqueTokens: public Tokens {
public:
    UniqueTokens(): m_count(0) {}

    bool next(OUString * token) override {
        assert(token != 0);
        // Because of the shared globalValue, no single instance of UniqueTokens
        // is guaranteed to exhaustively test all 36^6 possible values, but stop
        // after that many attempts anyway:
        sal_uInt32 radix = 36;
        sal_uInt32 max = radix * radix * radix * radix * radix * radix;
            // 36^6 == 2'176'782'336 < SAL_MAX_UINT32 == 4'294'967'295
        if (m_count == max) {
            return false;
        }
        sal_uInt32 v;
        {
            osl::MutexGuard g(osl::Mutex::getGlobalMutex());
            globalValue
                = ((globalValue == SAL_MAX_UINT32
                    ? tools::Time::GetSystemTicks() : globalValue + 1)
                   % max);
            v = globalValue;
        }
        *token = OUString::number(v, radix);
        ++m_count;
        return true;
    }

private:
    static sal_uInt32 globalValue;

    sal_uInt32 m_count;
};

sal_uInt32 UniqueTokens::globalValue = SAL_MAX_UINT32;

OUString lcl_createName(
    const OUString& rLeadingChars, Tokens & tokens, const OUString* pExtension,
    const OUString* pParent, bool bDirectory, bool bKeep, bool bLock)
{
    OUString aName = ConstructTempDir_Impl( pParent ) + rLeadingChars;;
    OUString token;
    while (tokens.next(&token))
    {
        OUString aTmp( aName + token );
        if ( pExtension )
            aTmp += *pExtension;
        else
            aTmp += ".tmp";
        if ( bDirectory )
        {
            FileBase::RC err = Directory::create(
                aTmp,
                (osl_File_OpenFlag_Read | osl_File_OpenFlag_Write
                 | osl_File_OpenFlag_Private));
            if ( err == FileBase::E_None )
            {
                // !bKeep: only for creating a name, not a file or directory
                if ( bKeep || Directory::remove( aTmp ) == FileBase::E_None )
                    return aTmp;
                else
                    return OUString();
            }
            else if ( err != FileBase::E_EXIST )
                // if f.e. name contains invalid chars stop trying to create dirs
                return OUString();
        }
        else
        {
            DBG_ASSERT( bKeep, "Too expensive, use directory for creating name!" );
            File aFile( aTmp );
            FileBase::RC err = aFile.open(
                osl_File_OpenFlag_Create | osl_File_OpenFlag_Private
                | (bLock ? 0 : osl_File_OpenFlag_NoLock));
            if ( err == FileBase::E_None || (bLock && err == FileBase::E_NOLCK) )
            {
                aFile.close();
                return aTmp;
            }
            else if ( err != FileBase::E_EXIST )
            {
                // if f.e. name contains invalid chars stop trying to create dirs
                // but if there is a folder with such name proceed further

                DirectoryItem aTmpItem;
                FileStatus aTmpStatus( osl_FileStatus_Mask_Type );
                if ( DirectoryItem::get( aTmp, aTmpItem ) != FileBase::E_None
                  || aTmpItem.getFileStatus( aTmpStatus ) != FileBase::E_None
                  || aTmpStatus.getFileType() != FileStatus::Directory )
                    return OUString();
            }
        }
    }
    return OUString();
}

OUString CreateTempName_Impl( const OUString* pParent, bool bKeep, bool bDir = true )
{
    OUString aEyeCatcher = "lu";
#ifdef UNX
#ifdef DBG_UTIL
    const char* eye = getenv("LO_TESTNAME");
    if(eye)
    {
        aEyeCatcher = OUString(eye, strlen(eye), RTL_TEXTENCODING_ASCII_US);
    }
#else
    static const pid_t pid = getpid();
    static const OUString aPidString = OUString::number(pid);
    aEyeCatcher += aPidString;
#endif
#endif
    UniqueTokens t;
    return lcl_createName(aEyeCatcher, t, 0, pParent, bDir, bKeep, false);
}

OUString TempFile::CreateTempName()
{
    OUString aName(CreateTempName_Impl( 0, false ));

    // convert to file URL
    OUString aTmp;
    if ( !aName.isEmpty() )
        FileBase::getSystemPathFromFileURL( aName, aTmp );
    return aTmp;
}

TempFile::TempFile( const OUString* pParent, bool bDirectory )
    : pStream( 0 )
    , bIsDirectory( bDirectory )
    , bKillingFileEnabled( false )
{
    aName = CreateTempName_Impl( pParent, true, bDirectory );
}

TempFile::TempFile( const OUString& rLeadingChars, bool _bStartWithZero, const OUString* pExtension, const OUString* pParent, bool bDirectory)
    : pStream( 0 )
    , bIsDirectory( bDirectory )
    , bKillingFileEnabled( false )
{
    SequentialTokens t(_bStartWithZero);
    aName = lcl_createName(rLeadingChars, t, pExtension, pParent, bDirectory, true, true);
}

TempFile::~TempFile()
{
    delete pStream;
    if ( bKillingFileEnabled )
    {
        if ( bIsDirectory )
        {
            // at the moment no recursiv algorithm present
            Directory::remove( aName );
        }
        else
        {
            File::remove( aName );
        }
    }
}

bool TempFile::IsValid() const
{
    return !aName.isEmpty();
}

OUString TempFile::GetFileName() const
{
    OUString aTmp;
    FileBase::getSystemPathFromFileURL(aName, aTmp);
    return aTmp;
}

OUString TempFile::GetURL()
{
    assert(!aName.isEmpty() && "TempFile::GetURL failed: unit test is leaking temp files, add the ucpfile1 component!");
    return aName;
}

SvStream* TempFile::GetStream( StreamMode eMode )
{
    if (!pStream)
    {
        if (!aName.isEmpty())
            pStream = new SvFileStream(aName, eMode);
        else
            pStream = new SvMemoryStream(NULL, 0, eMode);
    }

    return pStream;
}

void TempFile::CloseStream()
{
    if ( pStream )
    {
        delete pStream;
        pStream = NULL;
    }
}

OUString TempFile::SetTempNameBaseDirectory( const OUString &rBaseName )
{
    if( rBaseName.isEmpty() )
        return OUString();

    OUString aUnqPath( rBaseName );

    // remove trailing slash
    if ( rBaseName.endsWith("/") )
        aUnqPath = rBaseName.copy( 0, rBaseName.getLength() - 1 );

    // try to create the directory
    bool bRet = false;
    osl::FileBase::RC err = osl::Directory::create( aUnqPath );
    if ( err != FileBase::E_None && err != FileBase::E_EXIST )
        // perhaps parent(s) don't exist
        bRet = ensuredir( aUnqPath );
    else
        bRet = true;

    // failure to create base directory means returning an empty string
    OUString aTmp;
    if ( bRet )
    {
        // append own internal directory
        OUString &rTempNameBase_Impl = TempNameBase_Impl::get();
        rTempNameBase_Impl = rBaseName + "/";

        TempFile aBase( NULL, true );
        if ( aBase.IsValid() )
            // use it in case of success
            rTempNameBase_Impl = aBase.aName;

        // return system path of used directory
        FileBase::getSystemPathFromFileURL( rTempNameBase_Impl, aTmp );
    }

    return aTmp;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

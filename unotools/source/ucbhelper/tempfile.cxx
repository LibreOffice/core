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
#include <utility>

#include <com/sun/star/io/BufferSizeExceededException.hpp>
#include <com/sun/star/io/NotConnectedException.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <unotools/tempfile.hxx>
#include <rtl/ustring.hxx>
#include <o3tl/safeint.hxx>
#include <o3tl/char16_t2wchar_t.hxx>
#include <osl/mutex.hxx>
#include <osl/detail/file.h>
#include <osl/file.hxx>
#include <tools/time.hxx>
#include <tools/debug.hxx>
#include <tools/Guid.hxx>
#include <comphelper/DirectoryHelper.hxx>

#ifdef UNX
#include <unistd.h>
#elif defined( _WIN32 )
#include <process.h>
#endif

namespace
{
OUString gTempNameBase_Impl;

OUString ensureTrailingSlash(const OUString& url)
{
    if (!url.isEmpty() && !url.endsWith("/"))
        return url + "/";
    return url;
}

OUString stripTrailingSlash(const OUString& url)
{
    if (url.endsWith("/"))
        return url.copy(0, url.getLength() - 1);
    return url;
}

bool okOrExists(osl::FileBase::RC ret)
{
    return ret == osl::FileBase::E_None || ret == osl::FileBase::E_EXIST;
}

const OUString& getTempNameBase_Impl()
{
    if (gTempNameBase_Impl.isEmpty())
    {
        OUString ustrTempDirURL;
        osl::FileBase::RC rc = osl::File::getTempDirURL(ustrTempDirURL);
        if (rc == osl::FileBase::E_None)
        {
            gTempNameBase_Impl = ensureTrailingSlash(ustrTempDirURL);
            osl::Directory::createPath(gTempNameBase_Impl);
        }
    }
    assert(gTempNameBase_Impl.isEmpty() || gTempNameBase_Impl.endsWith("/"));
    DBG_ASSERT(!gTempNameBase_Impl.isEmpty(), "No TempDir!");
    return gTempNameBase_Impl;
}

OUString ConstructTempDir_Impl( const OUString* pParent, bool bCreateParentDirs )
{
    OUString aName;

    // Ignore pParent on iOS. We don't want to create any temp files
    // in the same directory where the document being edited is.
#ifndef IOS
    if ( pParent && !pParent->isEmpty() )
    {
        // test for valid filename
        OUString aRet;
        if ((osl::FileBase::getSystemPathFromFileURL(*pParent, aRet)
             == osl::FileBase::E_None)
            && (osl::FileBase::getFileURLFromSystemPath(aRet, aRet)
                == osl::FileBase::E_None))
        {
            osl::DirectoryItem aItem;
            sal_Int32 i = aRet.getLength();
            if ( aRet[i-1] == '/' )
                i--;

            if ( osl::DirectoryItem::get( aRet.copy(0, i), aItem ) == osl::FileBase::E_None || bCreateParentDirs )
                aName = aRet;
        }
    }
#else
    (void) pParent;
    (void) bCreateParentDirs;
#endif

    if ( aName.isEmpty() )
    {
        // if no parent or invalid parent : use default directory
        aName = getTempNameBase_Impl();
        osl::Directory::createPath(aName); // tdf#159769: always make sure it exists
    }

    // Make sure that directory ends with a separator
    return ensureTrailingSlash(aName);
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
        assert(token != nullptr);
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
        assert(token != nullptr);
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

    class TempDirCreatedObserver : public osl::DirectoryCreationObserver
    {
    public:
        virtual void DirectoryCreated(const OUString& aDirectoryUrl) override
        {
            osl::File::setAttributes( aDirectoryUrl, osl_File_Attribute_OwnRead |
                osl_File_Attribute_OwnWrite | osl_File_Attribute_OwnExe );
        };
    };

OUString lcl_createName(
    std::u16string_view rLeadingChars, Tokens & tokens, std::u16string_view pExtension,
    const OUString* pParent, bool bDirectory, bool bKeep, bool bLock,
    bool bCreateParentDirs )
{
    OUString aName = ConstructTempDir_Impl( pParent, bCreateParentDirs );
    if ( bCreateParentDirs )
    {
        size_t nOffset = rLeadingChars.rfind(u"/");
        OUString aDirName;
        if (std::u16string_view::npos != nOffset)
            aDirName = aName + rLeadingChars.substr( 0, nOffset );
        else
            aDirName = aName;
        TempDirCreatedObserver observer;
        if (!okOrExists(osl::Directory::createPath(aDirName, &observer)))
            return OUString();
    }
    aName += rLeadingChars;

    OUString token;
    while (tokens.next(&token))
    {
        OUString aTmp( aName + token );
        if ( !pExtension.empty() )
            aTmp += pExtension;
        else
            aTmp += ".tmp";
        if ( bDirectory )
        {
            osl::FileBase::RC err = osl::Directory::create(
                aTmp,
                (osl_File_OpenFlag_Read | osl_File_OpenFlag_Write
                 | osl_File_OpenFlag_Private));
            if (err == osl::FileBase::E_None)
            {
                // !bKeep: only for creating a name, not a file or directory
                if (bKeep || osl::Directory::remove(aTmp) == osl::FileBase::E_None)
                    return aTmp;
                else
                    return OUString();
            }
            else if (err != osl::FileBase::E_EXIST)
                // if f.e. name contains invalid chars stop trying to create dirs
                return OUString();
        }
        else
        {
            DBG_ASSERT( bKeep, "Too expensive, use directory for creating name!" );
            osl::File aFile(aTmp);
            osl::FileBase::RC err = aFile.open(
                osl_File_OpenFlag_Create | osl_File_OpenFlag_Private
                | (bLock ? 0 : osl_File_OpenFlag_NoLock));
            if (err == osl::FileBase::E_None || (bLock && err == osl::FileBase::E_NOLCK))
            {
                aFile.close();
                return aTmp;
            }
            else if (err != osl::FileBase::E_EXIST)
            {
                // if f.e. name contains invalid chars stop trying to create dirs
                // but if there is a folder with such name proceed further

                osl::DirectoryItem aTmpItem;
                osl::FileStatus aTmpStatus(osl_FileStatus_Mask_Type);
                if (osl::DirectoryItem::get(aTmp, aTmpItem) != osl::FileBase::E_None
                  || aTmpItem.getFileStatus(aTmpStatus) != osl::FileBase::E_None
                  || aTmpStatus.getFileType() != osl::FileStatus::Directory)
                    return OUString();
            }
        }
    }
    return OUString();
}

OUString createEyeCatcher()
{
    OUString eyeCatcher = u"lu"_ustr;
#ifdef DBG_UTIL
#ifdef UNX
    if (const char* eye = getenv("LO_TESTNAME"))
        eyeCatcher = OUString(eye, strlen(eye), RTL_TEXTENCODING_ASCII_US);
#elif defined(_WIN32)
    if (const wchar_t* eye = _wgetenv(L"LO_TESTNAME"))
        eyeCatcher = OUString(o3tl::toU(eye));
#endif
#else
#ifdef UNX
    eyeCatcher += OUString::number(getpid());
#elif defined(_WIN32)
    eyeCatcher += OUString::number(_getpid());
#endif
#endif
    return eyeCatcher;
}

const OUString& getEyeCatcher()
{
    static const OUString sEyeCatcher = createEyeCatcher();
    return sEyeCatcher;
}

OUString CreateTempName_Impl( const OUString* pParent, bool bKeep, bool bDir = true )
{
    UniqueTokens t;
    return lcl_createName( getEyeCatcher(), t, u"", pParent, bDir, bKeep,
                           false, false);
}

OUString CreateTempNameFast()
{
    OUString aName = getTempNameBase_Impl() + getEyeCatcher();

    tools::Guid aGuid(tools::Guid::Generate);

    return aName + aGuid.getOUString() + ".tmp" ;
}
}

namespace utl
{

OUString CreateTempName()
{
    OUString aName(CreateTempName_Impl( nullptr, false ));

    // convert to file URL
    OUString aTmp;
    if ( !aName.isEmpty() )
        osl::FileBase::getSystemPathFromFileURL(aName, aTmp);
    return aTmp;
}

TempFileFast::TempFileFast( )
{
}

TempFileFast::TempFileFast(TempFileFast && other) noexcept :
    mxStream(std::move(other.mxStream))
{
}

TempFileFast::~TempFileFast()
{
    CloseStream();
}

SvStream* TempFileFast::GetStream( StreamMode eMode )
{
    if (!mxStream)
    {
        OUString aName = CreateTempNameFast();
#ifdef _WIN32
        mxStream.reset(new SvFileStream(aName, eMode | StreamMode::TEMPORARY | StreamMode::DELETE_ON_CLOSE));
#else
        mxStream.reset(new SvFileStream(aName, eMode | StreamMode::TEMPORARY));
#endif
    }
    return mxStream.get();
}

void TempFileFast::CloseStream()
{
    if (mxStream)
    {
#if !defined _WIN32
        OUString aName = mxStream->GetFileName();
#endif
        mxStream.reset();
#ifdef _WIN32
        // On Windows, the file is opened with FILE_FLAG_DELETE_ON_CLOSE, so it will delete as soon as the handle closes.
        // On other platforms, we need to explicitly delete it.
#else
        if (!aName.isEmpty() && (osl::FileBase::getFileURLFromSystemPath(aName, aName) == osl::FileBase::E_None))
            osl::File::remove(aName);
#endif
    }
}

OUString CreateTempURL( const OUString* pParent, bool bDirectory )
{
    return CreateTempName_Impl( pParent, true, bDirectory );
}

OUString CreateTempURL( std::u16string_view rLeadingChars, bool _bStartWithZero,
                    std::u16string_view pExtension, const OUString* pParent,
                    bool bCreateParentDirs )
{
    SequentialTokens t(_bStartWithZero);
    return lcl_createName( rLeadingChars, t, pExtension, pParent, false,
                            true, true, bCreateParentDirs );
}

TempFileNamed::TempFileNamed( const OUString* pParent, bool bDirectory )
    : bIsDirectory( bDirectory )
    , bKillingFileEnabled( false )
{
    aName = CreateTempName_Impl( pParent, true, bDirectory );
}

TempFileNamed::TempFileNamed( std::u16string_view rLeadingChars, bool _bStartWithZero,
                    std::u16string_view pExtension, const OUString* pParent,
                    bool bCreateParentDirs )
    : bIsDirectory( false )
    , bKillingFileEnabled( false )
{
    SequentialTokens t(_bStartWithZero);
    aName = lcl_createName( rLeadingChars, t, pExtension, pParent, false,
                            true, true, bCreateParentDirs );
}

TempFileNamed::TempFileNamed(TempFileNamed && other) noexcept :
    aName(std::move(other.aName)), pStream(std::move(other.pStream)), bIsDirectory(other.bIsDirectory),
    bKillingFileEnabled(other.bKillingFileEnabled)
{
    other.bKillingFileEnabled = false;
}

TempFileNamed::~TempFileNamed()
{
    if ( !bKillingFileEnabled )
        return;

    pStream.reset();
    if ( bIsDirectory )
    {
        comphelper::DirectoryHelper::deleteDirRecursively(aName);
    }
    else
    {
        osl::File::remove(aName);
    }
}

bool TempFileNamed::IsValid() const
{
    return !aName.isEmpty();
}

OUString TempFileNamed::GetFileName() const
{
    OUString aTmp;
    osl::FileBase::getSystemPathFromFileURL(aName, aTmp);
    return aTmp;
}

OUString const & TempFileNamed::GetURL() const
{
    // if you request the URL, then you presumably want to access this via UCB,
    // and UCB will want to open the file via a separate file handle, which means
    // we have to make this file data actually hit disk. We do this here (and not
    // elsewhere) to make the other (normal) paths fast. Flushing to disk
    // really slows temp files down.
    if (pStream)
        pStream->Flush();
    return aName;
}

SvStream* TempFileNamed::GetStream( StreamMode eMode )
{
    if (!pStream)
    {
        if (!aName.isEmpty())
            pStream.reset(new SvFileStream(aName, eMode | StreamMode::TEMPORARY));
        else
            pStream.reset(new SvMemoryStream);
    }

    return pStream.get();
}

void TempFileNamed::CloseStream()
{
    pStream.reset();
}

OUString SetTempNameBaseDirectory( const OUString &rBaseName )
{
    if( rBaseName.isEmpty() )
        return OUString();

    // remove trailing slash
    OUString aUnqPath(stripTrailingSlash(rBaseName));

    // try to create the directory
    bool bRet = okOrExists(osl::Directory::createPath(aUnqPath));

    // failure to create base directory means returning an empty string
    OUString aTmp;
    if ( bRet )
    {
        // append own internal directory
        gTempNameBase_Impl = ensureTrailingSlash(rBaseName);

        TempFileNamed aBase( {}, true );
        if ( aBase.IsValid() )
            // use it in case of success
            gTempNameBase_Impl = ensureTrailingSlash(aBase.GetURL());

        // return system path of used directory
        osl::FileBase::getSystemPathFromFileURL(gTempNameBase_Impl, aTmp);
    }

    return aTmp;
}

OUString GetTempNameBaseDirectory()
{
    return ConstructTempDir_Impl(nullptr, false);
}


TempFileFastService::TempFileFastService()
: mbInClosed( false )
, mbOutClosed( false )
{
    mpTempFile.emplace();
    mpStream = mpTempFile->GetStream(StreamMode::READWRITE);
}

TempFileFastService::~TempFileFastService ()
{
}

// XInputStream

sal_Int32 SAL_CALL TempFileFastService::readBytes( css::uno::Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead )
{
    std::unique_lock aGuard( maMutex );
    if ( mbInClosed )
        throw css::io::NotConnectedException ( OUString(), getXWeak() );

    checkConnected();
    if (nBytesToRead < 0)
        throw css::io::BufferSizeExceededException( OUString(), getXWeak());

    if (aData.getLength() < nBytesToRead)
        aData.realloc(nBytesToRead);

    sal_uInt32 nRead = mpStream->ReadBytes(static_cast<void*>(aData.getArray()), nBytesToRead);
    checkError();

    if (nRead < o3tl::make_unsigned(aData.getLength()))
        aData.realloc( nRead );

    return nRead;
}

sal_Int32 SAL_CALL TempFileFastService::readSomeBytes( css::uno::Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead )
{
    {
        std::unique_lock aGuard( maMutex );
        if ( mbInClosed )
            throw css::io::NotConnectedException ( OUString(), getXWeak() );

        checkConnected();
        checkError();

        if (nMaxBytesToRead < 0)
            throw css::io::BufferSizeExceededException( OUString(), getXWeak() );

        if (mpStream->eof())
        {
            aData.realloc(0);
            return 0;
        }
    }
    return readBytes(aData, nMaxBytesToRead);
}

void SAL_CALL TempFileFastService::skipBytes( sal_Int32 nBytesToSkip )
{
    std::unique_lock aGuard( maMutex );
    if ( mbInClosed )
        throw css::io::NotConnectedException ( OUString(), getXWeak() );

    checkConnected();
    checkError();
    mpStream->SeekRel(nBytesToSkip);
    checkError();
}

sal_Int32 SAL_CALL TempFileFastService::available()
{
    std::unique_lock aGuard( maMutex );
    if ( mbInClosed )
        throw css::io::NotConnectedException ( OUString(), getXWeak() );

    checkConnected();

    sal_Int64 nAvailable = mpStream->remainingSize();
    checkError();

    return std::min<sal_Int64>(SAL_MAX_INT32, nAvailable);
}

void SAL_CALL TempFileFastService::closeInput()
{
    std::unique_lock aGuard( maMutex );
    if ( mbInClosed )
        throw css::io::NotConnectedException ( OUString(), getXWeak() );

    mbInClosed = true;

    if ( mbOutClosed )
    {
        // stream will be deleted by TempFile implementation
        mpStream = nullptr;
        mpTempFile.reset();
    }
}

// XOutputStream

void SAL_CALL TempFileFastService::writeBytes( const css::uno::Sequence< sal_Int8 >& aData )
{
    std::unique_lock aGuard( maMutex );
    if ( mbOutClosed )
        throw css::io::NotConnectedException ( OUString(), getXWeak() );

    checkConnected();
    sal_uInt32 nWritten = mpStream->WriteBytes(aData.getConstArray(), aData.getLength());
    checkError();
    if  ( nWritten != static_cast<sal_uInt32>(aData.getLength()))
        throw css::io::BufferSizeExceededException( OUString(), getXWeak() );
}

void SAL_CALL TempFileFastService::flush()
{
    std::unique_lock aGuard( maMutex );
    if ( mbOutClosed )
        throw css::io::NotConnectedException ( OUString(), getXWeak() );

    checkConnected();
    mpStream->Flush();
    checkError();
}

void SAL_CALL TempFileFastService::closeOutput()
{
    std::unique_lock aGuard( maMutex );
    if ( mbOutClosed )
        throw css::io::NotConnectedException ( OUString(), getXWeak() );

    mbOutClosed = true;
    if (mpStream)
    {
        // so that if you then open the InputStream, you can read the content
        mpStream->FlushBuffer();
        mpStream->Seek(0);
    }

    if ( mbInClosed )
    {
        // stream will be deleted by TempFile implementation
        mpStream = nullptr;
        mpTempFile.reset();
    }
}

void TempFileFastService::checkError() const
{
    if (!mpStream || mpStream->SvStream::GetError () != ERRCODE_NONE )
        throw css::io::NotConnectedException ( OUString(), const_cast < TempFileFastService * > (this)->getXWeak() );
}

void TempFileFastService::checkConnected()
{
    if (!mpStream)
        throw css::io::NotConnectedException ( OUString(), getXWeak() );
}

// XSeekable

void SAL_CALL TempFileFastService::seek( sal_Int64 nLocation )
{
    std::unique_lock aGuard( maMutex );
    checkConnected();
    checkError();
    if ( nLocation < 0 )
        throw css::lang::IllegalArgumentException();

    sal_Int64 nNewLoc = mpStream->Seek(static_cast<sal_uInt32>(nLocation) );
    if ( nNewLoc != nLocation )
        throw css::lang::IllegalArgumentException();
    checkError();
}

sal_Int64 SAL_CALL TempFileFastService::getPosition()
{
    std::unique_lock aGuard( maMutex );
    checkConnected();

    sal_uInt64 nPos = mpStream->Tell();
    checkError();
    return static_cast<sal_Int64>(nPos);
}

sal_Int64 SAL_CALL TempFileFastService::getLength()
{
    std::unique_lock aGuard( maMutex );
    checkConnected();

    checkError();

    sal_Int64 nEndPos = mpStream->TellEnd();

    return nEndPos;
}

// XStream

css::uno::Reference< css::io::XInputStream > SAL_CALL TempFileFastService::getInputStream()
{
    return this;
}

css::uno::Reference< css::io::XOutputStream > SAL_CALL TempFileFastService::getOutputStream()
{
    return this;
}

// XTruncate

void SAL_CALL TempFileFastService::truncate()
{
    std::unique_lock aGuard( maMutex );
    checkConnected();
    // SetStreamSize() call does not change the position
    mpStream->Seek( 0 );
    mpStream->SetStreamSize( 0 );
    checkError();
}



}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

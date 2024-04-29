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
#pragma once

#include <unotools/unotoolsdllapi.h>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/io/XTruncate.hpp>
#include <cppuhelper/implbase.hxx>
#include <tools/stream.hxx>
#include <memory>
#include <mutex>
#include <optional>

namespace utl
{


/**
  This is the "fast" temp file. Different OSes have different ideas how this should work, so this
  class presents an interface that is fast across Windows and Unix (which differ in how they want
  temp files to work).
  The key point is that such a temporary file is only a readable/writeable stream, and does not have
  a filename, or a location in the filesystem hierarchy.
  If you need a name or a URL, you should use TempFileNamed, which is slower, but creates an actual
  file in the filesystem.
*/
class UNOTOOLS_DLLPUBLIC TempFileFast
{
    std::unique_ptr<SvFileStream> mxStream;

public:
                    TempFileFast();
                    TempFileFast(TempFileFast && other) noexcept;
                    ~TempFileFast();

                    /**
                    Returns a stream to the tempfiles data; the stream is owned by the tempfile object, so you have to keep this
                    alive as long as you want to use the stream.
                    */
    SvStream*       GetStream( StreamMode eMode );

                    /**
                    Close and destroy the owned stream object if any.
                    */
    void            CloseStream();

};

/**
Only create a "physical" file name for a temporary file that would be valid at that moment.
Should only be used for 3rd party code with a file name interface that wants to create the file by itself.
If you want to convert file name into a URL, always use class LocalFileHelper, but never use any
conversion functions of osl.
*/
UNOTOOLS_DLLPUBLIC OUString CreateTempName();

UNOTOOLS_DLLPUBLIC OUString CreateTempURL( const OUString* pParent=nullptr, bool bDirectory=false );

/**
Same as above; additionally the name starts with some given characters followed by a counter ( example:
rLeadingChars="abc" means "abc0","abc1" and so on, depending on existing files in the folder ).
The extension string may be f.e. ".txt" or "", if no extension string is given, ".tmp" is used
    @param  _bStartWithZero If set to false names will be generated like "abc","abc0","abc1"
    @param  bCreateParentDirs If rLeadingChars contains a slash, this will create the required
            parent directories.
*/
UNOTOOLS_DLLPUBLIC OUString CreateTempURL( std::u16string_view rLeadingChars, bool _bStartWithZero=true, std::u16string_view pExtension={},
                          const OUString* pParent=nullptr, bool bCreateParentDirs=false );

/**
The TempNameBaseDirectory is a subfolder in the folder that is passed as a "physical" file name in the
SetTempNameBaseDirectory method.
This subfolder will be used if a TempFile or TempName is created without a parent name or a parent name
that does not belong to the local file system.
The caller of the SetTempNameBase is responsible for deleting this folder and all temporary files in it.
The return value of both methods is the complete "physical" name of the tempname base folder.
It is not a URL because all URLs must be "UCB compatible", so there may be no suitable URL at all.
*/
UNOTOOLS_DLLPUBLIC OUString SetTempNameBaseDirectory( const OUString &rBaseName );

// Return the URL of the temp directory (the one set with SetTempNameBaseDirectory or the
// default tempfile folder):
UNOTOOLS_DLLPUBLIC OUString GetTempNameBaseDirectory();

/**
    The class TempFile gives access to temporary files in the local file system. Sometimes they are needed because a 3rd party
    code has a file name based interface, or some file access has to be done locally without transferring tons of bytes to or
    from a remote system.
    Creating a UCB content on a TempFile is only possible if a UCP for the local file system is present.
    TempFiles can always be accessed by SvFileStreams or Sot/SvStorages using the "physical" file name ( not the URL, because
    this may be a non-file URL, see below ), but if a UCB content can be created, it is also possible to take the URL and use
    the UCB helper classes for streams. For convenience use UcbStreamHelper.
    A Tempfile always has a "physical" file name ( a file name in the local computers host notation ) but it has a
    "UCB compatible" URL only if a UCP for the local file system exists. This URL may have its own URL scheme
    ( not necessarily "file://" ! ). The TempFile class methods take this into account, but other simple conversions like
    the osl functions do not.
    So it is a potential error to convert between the filename and the URL of a TempFile object using functions or methods
    outside this class.
*/
class UNOTOOLS_DLLPUBLIC TempFileNamed
{
    OUString    aName;
    std::unique_ptr<SvStream>
                pStream;
    bool        bIsDirectory;
    bool        bKillingFileEnabled;

public:
                    /**
                    Create a temporary file or directory, in the default tempfile folder or if possible in a given folder.
                    This given folder ( the "parent" parameter ( if not NULL ) ) must be a "UCB compatible" URL.
                    The temporary object is created in the local file system, even if there is no UCB that can access it.
                    If the given folder is part of the local file system, the TempFile is created in this folder.
                    */
                    TempFileNamed( const OUString* pParent=nullptr, bool bDirectory=false );

                    /**
                    Same as above; additionally the name starts with some given characters followed by a counter ( example:
                    rLeadingChars="abc" means "abc0","abc1" and so on, depending on existing files in the folder ).
                    The extension string may be f.e. ".txt" or "", if no extension string is given, ".tmp" is used
                        @param  _bStartWithZero If set to false names will be generated like "abc","abc0","abc1"
                        @param  bCreateParentDirs If rLeadingChars contains a slash, this will create the required
                                parent directories.
                    */
                    TempFileNamed( std::u16string_view rLeadingChars, bool _bStartWithZero=true, std::u16string_view pExtension={},
                              const OUString* pParent = nullptr, bool bCreateParentDirs=false );

                    TempFileNamed(TempFileNamed && other) noexcept;

                    /**
                    TempFile will be removed from disk in dtor if EnableKillingFile(true) was called before.
                    Temporary directories will be removed recursively in that case.
                    */
                    ~TempFileNamed();

                    /**
                    Returns sal_True if it has a valid file name.
                    */
    bool            IsValid() const;

                    /**
                    Returns the URL of the tempfile object.
                    If you want to have the system path file name, use the GetFileName() method of this object
                    */
    OUString const & GetURL() const;

                    /**
                    Returns the system path name of the tempfile in host notation
                    If you want to have the URL, use the GetURL() method of this object.
                    */
    OUString        GetFileName() const;

                    /**
                    Returns a stream to the tempfiles data; the stream is owned by the tempfile object, so you have to keep this
                    alive as long as you want to use the stream. If the TempFile object is destroyed, it also destroys the
                    stream object, the underlying file is only deleted if EnableKillingFile(true) has been called before!
                    */
    SvStream*       GetStream( StreamMode eMode );

                    /**
                    Let the TempFile object close and destroy the owned stream object if any.
                    */
    void            CloseStream();

                    /**
                    If enabled the file will be removed from disk when the dtor is called ( default is not enabled )
                    */
    void            EnableKillingFile( bool bEnable=true )
                    { bKillingFileEnabled = bEnable; }

};


typedef ::cppu::WeakImplHelper<
     css::io::XStream
    , css::io::XSeekable
    , css::io::XInputStream
    , css::io::XOutputStream
    , css::io::XTruncate> TempFileFastService_Base;
class UNOTOOLS_DLLPUBLIC TempFileFastService final : public TempFileFastService_Base
{
    std::optional<utl::TempFileFast> mpTempFile;
    std::mutex maMutex;
    SvStream* mpStream;
    bool mbInClosed;
    bool mbOutClosed;

    SAL_DLLPRIVATE void checkError () const;
    SAL_DLLPRIVATE void checkConnected ();

public:
    explicit TempFileFastService ();
    SAL_DLLPRIVATE virtual ~TempFileFastService () override;

    // XInputStream
    virtual ::sal_Int32 SAL_CALL readBytes( css::uno::Sequence< ::sal_Int8 >& aData, ::sal_Int32 nBytesToRead ) override;
    SAL_DLLPRIVATE virtual ::sal_Int32 SAL_CALL readSomeBytes( css::uno::Sequence< ::sal_Int8 >& aData, ::sal_Int32 nMaxBytesToRead ) override;
    SAL_DLLPRIVATE virtual void SAL_CALL skipBytes( ::sal_Int32 nBytesToSkip ) override;
    SAL_DLLPRIVATE virtual ::sal_Int32 SAL_CALL available(  ) override;
    SAL_DLLPRIVATE virtual void SAL_CALL closeInput(  ) override;
    // XOutputStream
    virtual void SAL_CALL writeBytes( const css::uno::Sequence< ::sal_Int8 >& aData ) override;
    SAL_DLLPRIVATE virtual void SAL_CALL flush(  ) override;
    virtual void SAL_CALL closeOutput(  ) override;
    // XSeekable
    virtual void SAL_CALL seek( sal_Int64 location ) override;
    virtual sal_Int64 SAL_CALL getPosition(  ) override;
    SAL_DLLPRIVATE virtual sal_Int64 SAL_CALL getLength(  ) override;
    // XStream
    virtual css::uno::Reference< css::io::XInputStream > SAL_CALL getInputStream(  ) override;
    virtual css::uno::Reference< css::io::XOutputStream > SAL_CALL getOutputStream(  ) override;
    // XTruncate
    SAL_DLLPRIVATE virtual void SAL_CALL truncate() override;

};


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

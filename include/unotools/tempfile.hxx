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
#include "unotools/unotoolsdllapi.h"

#ifndef _UNOTOOLS_TEMPFILE_HXX
#define _UNOTOOLS_TEMPFILE_HXX

#include <tools/stream.hxx>

namespace utl
{

struct TempFile_Impl;

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

class UNOTOOLS_DLLPUBLIC TempFile
{
    TempFile_Impl*  pImp;
    bool            bKillingFileEnabled;

protected:

public:
                    /**
                    Create a temporary file or directory, in the default tempfile folder or if possible in a given folder.
                    This given folder ( the "parent" parameter ( if not NULL ) ) must be a "UCB compatible" URL.
                    The temporary object is created in the local file system, even if there is no UCB that can access it.
                    If the given folder is part of the local file system, the TempFile is created in this folder.
                    */
                    TempFile( const OUString* pParent=NULL, bool bDirectory=false );

                    /**
                    Same as above; additionally the name starts with some given characters followed by a counter ( example:
                    rLeadingChars="abc" means "abc0","abc1" and so on, depending on existing files in the folder ).
                    The extension string may be f.e. ".txt" or "", if no extension string is given, ".tmp" is used
                    */
                    TempFile( const OUString& rLeadingChars, const OUString* pExtension=NULL, const OUString* pParent=NULL,
                                bool bDirectory=false);

                    /**
                    Same as above; additionally the name starts with some given characters followed by a counter ( example:
                    rLeadingChars="abc" means "abc0","abc1" and so on, depending on existing files in the folder ).
                    The extension string may be f.e. ".txt" or "", if no extension string is given, ".tmp" is used
                        @param  _bStartWithZero If set to false names will be generated like "abc","abc0","abc1"
                    */
                    TempFile( const OUString& rLeadingChars, bool _bStartWithZero, const OUString* pExtension=NULL, const OUString* pParent=NULL, bool bDirectory=false);

                    /**
                    TempFile will be removed from disk in dtor if EnableKillingTempFile was called before.
                    Temporary directories will be removed recursively in that case.
                    */
                    ~TempFile();

                    /**
                    Returns sal_True if it has a valid file name.
                    */
    bool            IsValid() const;

                    /**
                    Returns the "UCB compatible" URL of the tempfile object.
                    If you want to have the "physical" file name, use the GetFileName() method of this object, because these
                    method uses the UCB for the conversion, but never use any external conversion functions for URLs into
                    "physical" names.
                    If no UCP is available for the local file system, an empty URL is returned. In this case you can't access
                    the file as a UCB content !
                    */
    OUString        GetURL() const;

                    /**
                    Returns the "physical" name of the tempfile in host notation ( should only be used for 3rd party code
                    with file name interfaces ).
                    If you want to have the URL, use the GetURL() method of this object, but never use any external
                    conversion functions for "physical" names into URLs.
                    */
    OUString        GetFileName() const;

                    /**
                    Returns a stream to the tempfiles data; the stream is owned by the tempfile object, so you have to keep this
                    alive as long as you want to use the stream. If the TempFile object is destroyed, it also destroys the
                    stream object, the underlying file is only deleted if EnableKillingFile( sal_True ) has been called before!
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

    bool            IsKillingFileEnabled() const
                    { return bKillingFileEnabled; }

                    /**
                    Only create a "physical" file name for a temporary file that would be valid at that moment.
                    Should only be used for 3rd party code with a file name interface that wants to create the file by itself.
                    If you want to convert file name into a URL, always use class LocalFileHelper, but never use any
                    conversion functions of osl.
                    */
    static OUString CreateTempName( const OUString* pParent=NULL );

                    /**
                    The TempNameBaseDirectory is a subfolder in the folder that is passed as a "physical" file name in the
                    SetTempNameBaseDirectory method.
                    This subfolder will be used if a TempFile or TempName is created without a parent name or a parent name
                    that does not belong to the local file system.
                    The caller of the SetTempNameBase is responsible for deleting this folder and all temporary files in it.
                    The return value of both methods is the complete "physical" name of the tempname base folder.
                    It is not a URL because alle URLs must be "UCB compatible", so there may be no suitable URL at all.
                    */
    static OUString SetTempNameBaseDirectory( const OUString &rBaseName );
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

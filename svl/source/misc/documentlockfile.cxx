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


#include <com/sun/star/ucb/XSimpleFileAccess.hpp>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <com/sun/star/ucb/InsertCommandArgument.hpp>
#include <com/sun/star/ucb/NameClashException.hpp>
#include <com/sun/star/io/WrongFormatException.hpp>
#include <com/sun/star/io/TempFile.hpp>

#include <osl/time.h>
#include <osl/security.hxx>
#include <osl/socket.hxx>
#include <o3tl/enumrange.hxx>

#include <rtl/string.hxx>
#include <rtl/ustring.hxx>
#include <rtl/strbuf.hxx>
#include <rtl/ustrbuf.hxx>

#include <comphelper/processfactory.hxx>

#include <unotools/bootstrap.hxx>

#include <ucbhelper/content.hxx>

#include <unotools/useroptions.hxx>

#include <svl/documentlockfile.hxx>

using namespace ::com::sun::star;

namespace svt {

bool DocumentLockFile::m_bAllowInteraction = true;


DocumentLockFile::DocumentLockFile( const OUString& aOrigURL )
: LockFileCommon( aOrigURL, OUString( ".~lock."  ) )
{
}


DocumentLockFile::~DocumentLockFile()
{
}


void DocumentLockFile::WriteEntryToStream( const LockFileEntry& aEntry, const uno::Reference< io::XOutputStream >& xOutput )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    OUStringBuffer aBuffer;

    for ( LockFileComponent lft : o3tl::enumrange<LockFileComponent>() )
    {
        aBuffer.append( EscapeCharacters( aEntry[lft] ) );
        if ( lft < LockFileComponent::LAST )
            aBuffer.append( ',' );
        else
            aBuffer.append( ';' );
    }

    OString aStringData( OUStringToOString( aBuffer.makeStringAndClear(), RTL_TEXTENCODING_UTF8 ) );
    uno::Sequence< sal_Int8 > aData( reinterpret_cast<sal_Int8 const *>(aStringData.getStr()), aStringData.getLength() );
    xOutput->writeBytes( aData );
}


bool DocumentLockFile::CreateOwnLockFile()
{
    ::osl::MutexGuard aGuard( m_aMutex );

    try
    {
        uno::Reference< io::XStream > xTempFile(
            io::TempFile::create( comphelper::getProcessComponentContext() ),
            uno::UNO_QUERY_THROW );
        uno::Reference< io::XSeekable > xSeekable( xTempFile, uno::UNO_QUERY_THROW );

        uno::Reference< io::XInputStream > xInput = xTempFile->getInputStream();
        uno::Reference< io::XOutputStream > xOutput = xTempFile->getOutputStream();

        if ( !xInput.is() || !xOutput.is() )
            throw uno::RuntimeException();

        LockFileEntry aNewEntry = GenerateOwnEntry();
        WriteEntryToStream( aNewEntry, xOutput );
        xOutput->closeOutput();

        xSeekable->seek( 0 );

        uno::Reference < css::ucb::XCommandEnvironment > xEnv;
        ::ucbhelper::Content aTargetContent( m_aURL, xEnv, comphelper::getProcessComponentContext() );

        ucb::InsertCommandArgument aInsertArg;
        aInsertArg.Data = xInput;
        aInsertArg.ReplaceExisting = false;
        uno::Any aCmdArg;
        aCmdArg <<= aInsertArg;
        aTargetContent.executeCommand( "insert", aCmdArg );

        // try to let the file be hidden if possible
        try {
            aTargetContent.setPropertyValue("IsHidden", uno::makeAny( true ) );
        } catch( uno::Exception& ) {}
    }
    catch( ucb::NameClashException& )
    {
        return false;
    }

    return true;
}


LockFileEntry DocumentLockFile::GetLockData()
{
    ::osl::MutexGuard aGuard( m_aMutex );

    uno::Reference< io::XInputStream > xInput = OpenStream();
    if ( !xInput.is() )
        throw uno::RuntimeException();

    const sal_Int32 nBufLen = 32000;
    uno::Sequence< sal_Int8 > aBuffer( nBufLen );

    sal_Int32 nRead = 0;

    nRead = xInput->readBytes( aBuffer, nBufLen );
    xInput->closeInput();

    if ( nRead == nBufLen )
        throw io::WrongFormatException();

    sal_Int32 nCurPos = 0;
    return ParseEntry( aBuffer, nCurPos );
}


uno::Reference< io::XInputStream > DocumentLockFile::OpenStream()
{
    ::osl::MutexGuard aGuard( m_aMutex );

    uno::Reference < css::ucb::XCommandEnvironment > xEnv;
    ::ucbhelper::Content aSourceContent( m_aURL, xEnv, comphelper::getProcessComponentContext() );

    // the file can be opened readonly, no locking will be done
    return aSourceContent.openStream();
}


bool DocumentLockFile::OverwriteOwnLockFile()
{
    // allows to overwrite the lock file with the current data
    try
    {
        uno::Reference < css::ucb::XCommandEnvironment > xEnv;
        ::ucbhelper::Content aTargetContent( m_aURL, xEnv, comphelper::getProcessComponentContext() );

        LockFileEntry aNewEntry = GenerateOwnEntry();

        uno::Reference< io::XStream > xStream = aTargetContent.openWriteableStreamNoLock();
        uno::Reference< io::XOutputStream > xOutput = xStream->getOutputStream();
        uno::Reference< io::XTruncate > xTruncate( xOutput, uno::UNO_QUERY_THROW );

        xTruncate->truncate();
        WriteEntryToStream( aNewEntry, xOutput );
        xOutput->closeOutput();
    }
    catch( uno::Exception& )
    {
        return false;
    }

    return true;
}


void DocumentLockFile::RemoveFile()
{
    ::osl::MutexGuard aGuard( m_aMutex );

    // TODO/LATER: the removing is not atomic, is it possible in general to make it atomic?
    LockFileEntry aNewEntry = GenerateOwnEntry();
    LockFileEntry aFileData = GetLockData();

    if ( !aFileData[LockFileComponent::SYSUSERNAME].equals( aNewEntry[LockFileComponent::SYSUSERNAME] )
      || !aFileData[LockFileComponent::LOCALHOST].equals( aNewEntry[LockFileComponent::LOCALHOST] )
      || !aFileData[LockFileComponent::USERURL].equals( aNewEntry[LockFileComponent::USERURL] ) )
        throw io::IOException(); // not the owner, access denied

    uno::Reference < css::ucb::XCommandEnvironment > xEnv;
    ::ucbhelper::Content aCnt(m_aURL, xEnv, comphelper::getProcessComponentContext());
    aCnt.executeCommand("delete",
        uno::makeAny(true));
}

} // namespace svt

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

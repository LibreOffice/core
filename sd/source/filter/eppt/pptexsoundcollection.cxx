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

#include <memory>
#include "pptexsoundcollection.hxx"
#include "epptdef.hxx"
#include <tools/stream.hxx>
#include <tools/urlobj.hxx>
#include <ucbhelper/content.hxx>
#include <comphelper/processfactory.hxx>
#include <unotools/ucbstreamhelper.hxx>

namespace ppt
{

ExSoundEntry::ExSoundEntry(const OUString& rString)
    : nFileSize(0)
    , aSoundURL(rString)
{
    try
    {
        ::ucbhelper::Content aCnt( aSoundURL,
            css::uno::Reference< css::ucb::XCommandEnvironment >(),
            comphelper::getProcessComponentContext() );
        sal_Int64 nVal = 0;
        aCnt.getPropertyValue("Size") >>= nVal;
        nFileSize = static_cast<sal_uInt32>(nVal);
    }
    catch( css::uno::Exception& )
    {

    }
};

OUString ExSoundEntry::ImplGetName() const
{
    INetURLObject aTmp( aSoundURL );
    return aTmp.GetName();
}

OUString ExSoundEntry::ImplGetExtension() const
{
    INetURLObject aTmp( aSoundURL );
    OUString aExtension( aTmp.GetExtension() );
    if ( !aExtension.isEmpty() )
    {
        aExtension = "." + aExtension;
    }
    return aExtension;
}

bool ExSoundEntry::IsSameURL(const OUString& rURL) const
{
    return ( rURL == aSoundURL );
}

sal_uInt32 ExSoundEntry::GetSize( sal_uInt32 nId ) const
{
    OUString aName( ImplGetName() );
    OUString aExtension( ImplGetExtension() );

    sal_uInt32 nSize = 8;                           // SoundContainer Header
    if ( !aName.isEmpty() )                         // String Atom          ( instance 0 - name of sound )
        nSize += aName.getLength() * 2 + 8;
    if ( !aExtension.isEmpty() )                    // String Atom          ( instance 1 - extension of sound )
        nSize += aExtension.getLength() * 2 + 8;

    OUString aId( OUString::number(nId) );   // String Atom          ( instance 2 - reference id )
    nSize += 2 * aId.getLength() + 8;

    nSize += nFileSize + 8;                         // SoundData Atom

    return nSize;
}

void ExSoundEntry::Write( SvStream& rSt, sal_uInt32 nId ) const
{
    try
    {
        ::ucbhelper::Content aCnt( aSoundURL,
            css::uno::Reference< css::ucb::XCommandEnvironment >(),
            comphelper::getProcessComponentContext() );

        // create SoundContainer
        rSt.WriteUInt32( ( EPP_Sound << 16 ) | 0xf ).WriteUInt32( GetSize( nId ) - 8 );

        OUString aSoundName( ImplGetName() );
        sal_Int32 i, nSoundNameLen = aSoundName.getLength();
        if ( nSoundNameLen )
        {
            // name of sound ( instance 0 )
            rSt.WriteUInt32( EPP_CString << 16 ).WriteUInt32( nSoundNameLen * 2 );
            for ( i = 0; i < nSoundNameLen; ++i )
                rSt.WriteUInt16( aSoundName[i] );
        }
        OUString aExtension( ImplGetExtension() );
        sal_Int32 nExtensionLen = aExtension.getLength();
        if ( nExtensionLen )
        {
            // extension of sound ( instance 1 )
            rSt.WriteUInt32( ( EPP_CString << 16 ) | 16 ).WriteUInt32( nExtensionLen * 2 );
            for ( i = 0; i < nExtensionLen; ++i )
                rSt.WriteUInt16( aExtension[i] );
        }
        // id of sound ( instance 2 )
        OUString aId( OUString::number(nId ) );
        sal_Int32 nIdLen = aId.getLength();
        rSt.WriteUInt32( ( EPP_CString << 16 ) | 32 ).WriteUInt32( nIdLen * 2 );
        for ( i = 0; i < nIdLen; ++i )
            rSt.WriteUInt16( aId[i] );

        rSt.WriteUInt32( EPP_SoundData << 16 ).WriteUInt32( nFileSize );
        sal_uInt32 nBytesLeft = nFileSize;
        std::unique_ptr<SvStream> pSourceFile = ::utl::UcbStreamHelper::CreateStream( aSoundURL, StreamMode::READ );
        if ( pSourceFile )
        {
            std::unique_ptr<sal_uInt8[]> pBuf( new sal_uInt8[ 0x10000 ] );   // 64 kB  Buffer
            while ( nBytesLeft )
            {
                sal_uInt32 nToDo = std::min<sal_uInt32>( nBytesLeft, 0x10000 );
                pSourceFile->ReadBytes(pBuf.get(), nToDo);
                rSt.WriteBytes(pBuf.get(), nToDo);
                nBytesLeft -= nToDo;
            }
        }
    }
    catch( css::uno::Exception& )
    {

    }
}

sal_uInt32 ExSoundCollection::GetId(const OUString& rString)
{
    sal_uInt32 nSoundId = 0;
    if (!rString.isEmpty())
    {
        const sal_uInt32 nSoundCount = maEntries.size();

        auto iter = std::find_if(maEntries.begin(), maEntries.end(),
            [&rString](const ExSoundEntry& rEntry) { return rEntry.IsSameURL(rString); });
        nSoundId = static_cast<sal_uInt32>(std::distance(maEntries.begin(), iter));

        if ( nSoundId++ == nSoundCount )
        {
            ExSoundEntry aEntry( rString );
            if ( aEntry.GetFileSize() )
                maEntries.push_back(aEntry);
            else
            {
                nSoundId = 0;   // only insert sounds that are accessible
            }
        }
    }
    return nSoundId;
}

sal_uInt32 ExSoundCollection::GetSize() const
{
    sal_uInt32 nSize = 0;
    if (!maEntries.empty())
    {
        nSize += 8 + 12;    // size of SoundCollectionContainerHeader + SoundCollAtom
        sal_uInt32 i = 1;
        for ( const auto& rEntry : maEntries )
        {
            nSize += rEntry.GetSize(i);
            ++i;
        }
    }
    return nSize;
}

void ExSoundCollection::Write( SvStream& rSt ) const
{
    if (maEntries.empty())
        return;

    sal_uInt32 i = 1;
    sal_uInt32 nSoundCount = maEntries.size();

    // create SoundCollection Container
    rSt.WriteUInt16( 0xf ).WriteUInt16( EPP_SoundCollection ).WriteUInt32( GetSize() - 8 );

    // create SoundCollAtom ( reference to the next free SoundId );
    rSt.WriteUInt32( EPP_SoundCollAtom << 16 ).WriteUInt32( 4 ).WriteUInt32( nSoundCount );

    for ( const auto& rEntry : maEntries )
    {
        rEntry.Write(rSt,i);
        ++i;
    }
}

} // namespace ppt;

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

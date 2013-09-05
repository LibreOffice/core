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

#include <pptexsoundcollection.hxx>
#include "epptdef.hxx"
#include <tools/urlobj.hxx>
#include <ucbhelper/content.hxx>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/proptypehlp.hxx>
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
            ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XCommandEnvironment >(),
            comphelper::getProcessComponentContext() );
        sal_Int64 nVal = 0;
        ::cppu::convertPropertyValue( nVal, aCnt.getPropertyValue("Size") );
        nFileSize = (sal_uInt32)nVal;
    }
    catch( ::com::sun::star::uno::Exception& )
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

sal_Bool ExSoundEntry::IsSameURL(const OUString& rURL) const
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
            ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XCommandEnvironment >(),
            comphelper::getProcessComponentContext() );

        // create SoundContainer
        rSt << (sal_uInt32)( ( EPP_Sound << 16 ) | 0xf ) << (sal_uInt32)( GetSize( nId ) - 8 );

        OUString aSoundName( ImplGetName() );
        sal_Int32 i, nSoundNameLen = aSoundName.getLength();
        if ( nSoundNameLen )
        {
            // name of sound ( instance 0 )
            rSt << (sal_uInt32)( EPP_CString << 16 ) << (sal_uInt32)( nSoundNameLen * 2 );
            for ( i = 0; i < nSoundNameLen; ++i )
                rSt << aSoundName[i];
        }
        OUString aExtension( ImplGetExtension() );
        sal_Int32 nExtensionLen = aExtension.getLength();
        if ( nExtensionLen )
        {
            // extension of sound ( instance 1 )
            rSt << (sal_uInt32)( ( EPP_CString << 16 ) | 16 ) << (sal_uInt32)( nExtensionLen * 2 );
            for ( i = 0; i < nExtensionLen; ++i )
                rSt << aExtension[i];
        }
        // id of sound ( instance 2 )
        OUString aId( OUString::number(nId ) );
        sal_Int32 nIdLen = aId.getLength();
        rSt << (sal_uInt32)( ( EPP_CString << 16 ) | 32 ) << (sal_uInt32)( nIdLen * 2 );
        for ( i = 0; i < nIdLen; ++i )
            rSt << aId[i];

        rSt << (sal_uInt32)( EPP_SoundData << 16 ) << (sal_uInt32)( nFileSize );
        sal_uInt32 nBytesLeft = nFileSize;
        SvStream* pSourceFile = ::utl::UcbStreamHelper::CreateStream( aSoundURL, STREAM_READ );
        if ( pSourceFile )
        {
            sal_uInt8* pBuf = new sal_uInt8[ 0x10000 ];   // 64 kB  Buffer
            while ( nBytesLeft )
            {
                sal_uInt32 nToDo = ( nBytesLeft > 0x10000 ) ? 0x10000 : nBytesLeft;
                pSourceFile->Read( pBuf, nToDo );
                rSt.Write( pBuf, nToDo );
                nBytesLeft -= nToDo;
            }
            delete pSourceFile;
            delete[] pBuf;
        }
    }
    catch( ::com::sun::star::uno::Exception& )
    {

    }
}

sal_uInt32 ExSoundCollection::GetId(const OUString& rString)
{
    sal_uInt32 nSoundId = 0;
    if (!rString.isEmpty())
    {
        const sal_uInt32 nSoundCount = maEntries.size();
        boost::ptr_vector<ExSoundEntry>::const_iterator iter;

        for (iter = maEntries.begin(); iter != maEntries.end(); ++iter, ++nSoundId)
        {
            if (iter->IsSameURL(rString))
                break;
        }

        if ( nSoundId++ == nSoundCount )
        {
            ExSoundEntry* pEntry = new ExSoundEntry( rString );
            if ( pEntry->GetFileSize() )
                maEntries.push_back(pEntry);
            else
            {
                nSoundId = 0;   // only insert sounds that are accessible
                delete pEntry;
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
        boost::ptr_vector<ExSoundEntry>::const_iterator iter;
        sal_uInt32 i = 1;
        for ( iter = maEntries.begin(); iter != maEntries.end(); ++iter, ++i)
            nSize += iter->GetSize(i);
    }
    return nSize;
}

void ExSoundCollection::Write( SvStream& rSt ) const
{
    if (!maEntries.empty())
    {
        sal_uInt32 i = 1;
        sal_uInt32 nSoundCount = maEntries.size();

        // create SoundCollection Container
        rSt << (sal_uInt16)0xf << (sal_uInt16)EPP_SoundCollection << (sal_uInt32)( GetSize() - 8 );

        // create SoundCollAtom ( reference to the next free SoundId );
        rSt << (sal_uInt32)( EPP_SoundCollAtom << 16 ) << (sal_uInt32)4 << nSoundCount;

        boost::ptr_vector<ExSoundEntry>::const_iterator iter;
        for ( iter = maEntries.begin(); iter != maEntries.end(); ++iter, ++i)
            iter->Write(rSt,i);
    }
}


} // namespace ppt;

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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

#include <pptexsoundcollection.hxx>
#include "epptdef.hxx"
#include <tools/urlobj.hxx>
#include <ucbhelper/content.hxx>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/proptypehlp.hxx>
#include <unotools/ucbstreamhelper.hxx>

namespace ppt
{

ExSoundEntry::ExSoundEntry(const rtl::OUString& rString)
    : nFileSize(0)
    , aSoundURL(rString)
{
    try
    {
        ::ucbhelper::Content aCnt( aSoundURL,
            ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XCommandEnvironment >(),
            comphelper::getProcessComponentContext() );
        sal_Int64 nVal = 0;
        ::cppu::convertPropertyValue( nVal, aCnt.getPropertyValue( ::rtl::OUString( "Size" ) ) );
        nFileSize = (sal_uInt32)nVal;
    }
    catch( ::com::sun::star::uno::Exception& )
    {

    }
};

rtl::OUString ExSoundEntry::ImplGetName() const
{
    INetURLObject aTmp( aSoundURL );
    return aTmp.GetName();
}

rtl::OUString ExSoundEntry::ImplGetExtension() const
{
    INetURLObject aTmp( aSoundURL );
    String aExtension( aTmp.GetExtension() );
    if ( aExtension.Len() )
        aExtension.Insert( (sal_Unicode)'.', 0 );
    return aExtension;
}

sal_Bool ExSoundEntry::IsSameURL(const rtl::OUString& rURL) const
{
    return ( rURL == aSoundURL );
}

sal_uInt32 ExSoundEntry::GetSize( sal_uInt32 nId ) const
{
    rtl::OUString aName( ImplGetName() );
    rtl::OUString aExtension( ImplGetExtension() );

    sal_uInt32 nSize = 8;                           // SoundContainer Header
    if ( !aName.isEmpty() )                         // String Atom          ( instance 0 - name of sound )
        nSize += aName.getLength() * 2 + 8;
    if ( !aExtension.isEmpty() )                    // String Atom          ( instance 1 - extension of sound )
        nSize += aExtension.getLength() * 2 + 8;

    rtl::OUString aId( rtl::OUString::valueOf(static_cast<sal_Int32>(nId)) );   // String Atom          ( instance 2 - reference id )
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

        rtl::OUString aSoundName( ImplGetName() );
        sal_Int32 i, nSoundNameLen = aSoundName.getLength();
        if ( nSoundNameLen )
        {
            // name of sound ( instance 0 )
            rSt << (sal_uInt32)( EPP_CString << 16 ) << (sal_uInt32)( nSoundNameLen * 2 );
            for ( i = 0; i < nSoundNameLen; ++i )
                rSt << aSoundName[i];
        }
        rtl::OUString aExtension( ImplGetExtension() );
        sal_Int32 nExtensionLen = aExtension.getLength();
        if ( nExtensionLen )
        {
            // extension of sound ( instance 1 )
            rSt << (sal_uInt32)( ( EPP_CString << 16 ) | 16 ) << (sal_uInt32)( nExtensionLen * 2 );
            for ( i = 0; i < nExtensionLen; ++i )
                rSt << aExtension[i];
        }
        // id of sound ( instance 2 )
        rtl::OUString aId( rtl::OUString::valueOf(static_cast<sal_Int32>(nId) ) );
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

sal_uInt32 ExSoundCollection::GetId(const rtl::OUString& rString)
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

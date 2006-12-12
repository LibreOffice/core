/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: pptexsoundcollection.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-12 16:41:05 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#ifndef _SD_PPT_EXSOUNDCOLLECTION_HXX
#include <pptexsoundcollection.hxx>
#endif

#ifndef _EPPT_DEF_HXX
#include "epptdef.hxx"
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _UCBHELPER_CONTENT_HXX_
#include <ucbhelper/content.hxx>
#endif
#ifndef _UCBHELPER_CONTENTBROKER_HXX_
#include <ucbhelper/contentbroker.hxx>
#endif
#ifndef _CPPUHELPER_PROPTYPEHLP_HXX_
#include <cppuhelper/proptypehlp.hxx>
#endif
#ifndef _UNTOOLS_UCBSTREAMHELPER_HXX
#include <unotools/ucbstreamhelper.hxx>
#endif

namespace ppt
{

ExSoundEntry::ExSoundEntry( const String& rString )
:   nFileSize( 0 )
,   aSoundURL( rString )
{
    try
    {
        ::ucb::Content aCnt( aSoundURL,
            ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XCommandEnvironment >() );
        sal_Int64 nVal;
        ::cppu::convertPropertyValue( nVal, aCnt.getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Size" ) ) ) );
        nFileSize = (sal_uInt32)nVal;
    }
    catch( ::com::sun::star::uno::Exception& )
    {

    }
};

String ExSoundEntry::ImplGetName() const
{
    INetURLObject aTmp( aSoundURL );
    return aTmp.GetName();
}

String ExSoundEntry::ImplGetExtension() const
{
    INetURLObject aTmp( aSoundURL );
    String aExtension( aTmp.GetExtension() );
    if ( aExtension.Len() )
        aExtension.Insert( (sal_Unicode)'.', 0 );
    return aExtension;
}

sal_Bool ExSoundEntry::IsSameURL( const String& rURL ) const
{
    return ( rURL == aSoundURL );
}

sal_uInt32 ExSoundEntry::GetSize( sal_uInt32 nId ) const
{
    String aName( ImplGetName() );
    String aExtension( ImplGetExtension() );

    sal_uInt32 nSize = 8;                           // SoundContainer Header
    if ( aName.Len() )                              // String Atom          ( instance 0 - name of sound )
        nSize += aName.Len() * 2 + 8;
    if ( aExtension.Len() )                         // String Atom          ( instance 1 - extension of sound )
        nSize += aExtension.Len() * 2 + 8;

    String aId( String::CreateFromInt32( nId ) );   // String Atom          ( instance 2 - reference id )
    nSize += 2 * aId.Len() + 8;

    nSize += nFileSize + 8;                         // SoundData Atom

    return nSize;
}

void ExSoundEntry::Write( SvStream& rSt, sal_uInt32 nId )
{
    try
    {
        ::ucb::Content aCnt( aSoundURL,
            ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XCommandEnvironment >() );

        // create SoundContainer
        rSt << (sal_uInt32)( ( EPP_Sound << 16 ) | 0xf ) << (sal_uInt32)( GetSize( nId ) - 8 );

        String aSoundName( ImplGetName() );
        sal_uInt16 i, nSoundNameLen = aSoundName.Len();
        if ( nSoundNameLen )
        {
            // name of sound ( instance 0 )
            rSt << (sal_uInt32)( EPP_CString << 16 ) << (sal_uInt32)( nSoundNameLen * 2 );
            for ( i = 0; i < nSoundNameLen; i++ )
                rSt << aSoundName.GetChar( i );
        }
        String aExtension( ImplGetExtension() );
        sal_uInt32 nExtensionLen = aExtension.Len();
        if ( nExtensionLen )
        {
            // extension of sound ( instance 1 )
            rSt << (sal_uInt32)( ( EPP_CString << 16 ) | 16 ) << (sal_uInt32)( nExtensionLen * 2 );
            for ( i = 0; i < nExtensionLen; i++ )
                rSt << aExtension.GetChar( i );
        }
        // id of sound ( instance 2 )
        String aId( String::CreateFromInt32( nId ) );
        sal_uInt32 nIdLen = aId.Len();
        rSt << (sal_uInt32)( ( EPP_CString << 16 ) | 32 ) << (sal_uInt32)( nIdLen * 2 );
        for ( i = 0; i < nIdLen; i++ )
            rSt << aId.GetChar( i );

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

ExSoundCollection::~ExSoundCollection()
{
    for( void* pPtr = List::First(); pPtr; pPtr = List::Next() )
        delete (ExSoundEntry*)pPtr;
}

sal_uInt32 ExSoundCollection::GetId( const String& rString )
{
    sal_uInt32 nSoundId = 0;
    if( rString.Len() )
    {
        const sal_uInt32 nSoundCount = Count();

        for( ; nSoundId < nSoundCount; nSoundId++ )
            if( ImplGetByIndex( nSoundId )->IsSameURL( rString ) )
                break;
        if ( nSoundId++ == nSoundCount )
        {
            ExSoundEntry* pEntry = new ExSoundEntry( rString );
            if ( pEntry->GetFileSize() )
                List::Insert( pEntry, LIST_APPEND );
            else
            {
                nSoundId = 0;   // only insert sounds that are accessible
                delete pEntry;
            }
        }
    }
    return nSoundId;
}

const ExSoundEntry* ExSoundCollection::ImplGetByIndex( sal_uInt32 nIndex ) const
{
    return (ExSoundEntry*)List::GetObject( nIndex );
}

sal_uInt32 ExSoundCollection::GetSize() const
{
    sal_uInt32 nSize = 0;
    sal_uInt32 i, nSoundCount = Count();
    if ( nSoundCount )
    {
        nSize += 8 + 12;    // size of SoundCollectionContainerHeader + SoundCollAtom
        for ( i = 0; i < nSoundCount; i++ )
            nSize += ImplGetByIndex( i )->GetSize( i + 1 );
    }
    return nSize;
}

void ExSoundCollection::Write( SvStream& rSt )
{
    sal_uInt32 i, nSoundCount = Count();
    if ( nSoundCount )
    {
        // create SoundCollection Container
        rSt << (sal_uInt16)0xf << (sal_uInt16)EPP_SoundCollection << (sal_uInt32)( GetSize() - 8 );

        // create SoundCollAtom ( reference to the next free SoundId );
        rSt << (sal_uInt32)( EPP_SoundCollAtom << 16 ) << (sal_uInt32)4 << nSoundCount;

        for ( i = 0; i < nSoundCount; i++ )
            ((ExSoundEntry*)List::GetObject( i ))->Write( rSt, i + 1 );
    }
}


} // namespace ppt;


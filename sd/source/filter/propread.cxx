/*************************************************************************
 *
 *  $RCSfile: propread.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-01 14:11:38 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _PROPREAD_HXX_
#include <propread.hxx>
#endif
#ifndef _BIGINT_HXX
#include <tools/bigint.hxx>
#endif
#include "rtl/tencinfo.h"
#include "rtl/textenc.h"

// ------------------------------------------------------------------------

struct PropEntry
{
    sal_uInt32  mnId;
    sal_uInt32  mnSize;
    sal_uInt16  mnTextEnc;
    sal_uInt8*  mpBuf;

                        PropEntry( sal_uInt32 nId, const sal_uInt8* pBuf, sal_uInt32 nBufSize, sal_uInt16 nTextEnc );
                        PropEntry( const PropEntry& rProp );
                        ~PropEntry() { delete[] mpBuf; } ;

    const PropEntry&    operator=(const PropEntry& rPropEntry);
};

PropEntry::PropEntry( sal_uInt32 nId, const sal_uInt8* pBuf, sal_uInt32 nBufSize, sal_uInt16 nTextEnc ) :
    mnId        ( nId ),
    mnSize      ( nBufSize ),
    mnTextEnc   ( nTextEnc ),
    mpBuf       ( new sal_uInt8[ nBufSize ] )
{
    memcpy( (void*)mpBuf, (void*)pBuf, nBufSize );
};

PropEntry::PropEntry( const PropEntry& rProp ) :
    mnId        ( rProp.mnId ),
    mnSize      ( rProp.mnSize ),
    mpBuf       ( new sal_uInt8[ mnSize ] ),
    mnTextEnc   ( rProp.mnTextEnc )
{
    memcpy( (void*)mpBuf, (void*)rProp.mpBuf, mnSize );
};

const PropEntry& PropEntry::operator=(const PropEntry& rPropEntry)
{
    if ( this != &rPropEntry )
    {
        delete[] mpBuf;
        mnId = rPropEntry.mnId;
        mnSize = rPropEntry.mnSize;
        mnTextEnc = rPropEntry.mnTextEnc;
        mpBuf = new sal_uInt8[ mnSize ];
        memcpy( (void*)mpBuf, (void*)rPropEntry.mpBuf, mnSize );
    }
    return *this;
}

//  -----------------------------------------------------------------------

void PropItem::Clear()
{
    Seek( STREAM_SEEK_TO_BEGIN );
    delete[] SwitchBuffer();
}

//  -----------------------------------------------------------------------

BOOL PropItem::Read( String& rString, sal_uInt32 nStringType, sal_Bool bAlign )
{
    sal_uInt32  i, nSize, nType, nPos;
    sal_Bool    bRetValue = sal_False;

    nPos = Tell();

    if ( nStringType == VT_EMPTY )
        *this >> nType;
    else
        nType = nStringType & VT_TYPEMASK;

    *this >> nSize;

    switch( nType )
    {
        case VT_LPSTR :
        {
            if ( (sal_uInt16)nSize )
            {
                sal_Char* pString = new sal_Char[ (sal_uInt16)nSize ];
                if ( mnTextEnc == RTL_TEXTENCODING_UCS2 )
                {
                    nSize >>= 1;
                    if ( (sal_uInt16)nSize > 1 )
                    {
                        sal_Unicode* pWString = (sal_Unicode*)pString;
                        for ( i = 0; i < (sal_uInt16)nSize; i++ )
                            *this >> pWString[ i ];
                        rString = String( pWString, (sal_uInt16)nSize - 1 );
                    }
                    else
                        rString = String();
                    bRetValue = sal_True;
                }
                else
                {
                    SvMemoryStream::Read( pString, (sal_uInt16)nSize );
                    if ( pString[ (sal_uInt16)nSize - 1 ] == 0 )
                    {
                        if ( (sal_uInt16)nSize > 1 )
                            rString = String( ByteString( pString ), mnTextEnc );
                        else
                            rString = String();
                        bRetValue = sal_True;
                    }
                }
                delete[] pString;
            }
            if ( bAlign )
                SeekRel( ( 4 - ( nSize & 3 ) ) & 3 );       // dword align
        }
        break;

        case VT_LPWSTR :
        {
            if ( nSize )
            {
                sal_Unicode* pString = new sal_Unicode[ (sal_uInt16)nSize ];
                for ( i = 0; i < (sal_uInt16)nSize; i++ )
                    *this >> pString[ i ];
                if ( pString[ i - 1 ] == 0 )
                {
                    if ( (sal_uInt16)nSize > 1 )
                        rString = String( pString, (sal_uInt16)nSize - 1 );
                    else
                        rString = String();
                    bRetValue = sal_True;
                }
                delete[] pString;
            }
            if ( bAlign && ( nSize & 1 ) )
                SeekRel( 2 );                           // dword align
        }
        break;
    }
    if ( !bRetValue )
        Seek( nPos );
    return bRetValue;
}

//  -----------------------------------------------------------------------

PropItem& PropItem::operator=( PropItem& rPropItem )
{
    if ( this != &rPropItem )
    {
        Seek( STREAM_SEEK_TO_BEGIN );
        delete[] SwitchBuffer();

        mnTextEnc = rPropItem.mnTextEnc;
        sal_uInt32 nPos = rPropItem.Tell();
        rPropItem.Seek( STREAM_SEEK_TO_END );
        SvMemoryStream::Write( rPropItem.GetData(), rPropItem.Tell() );
        rPropItem.Seek( nPos );
    }
    return *this;
}

//  -----------------------------------------------------------------------

struct Dict
{
    sal_uInt32  mnId;
    String      aString;

            Dict( sal_uInt32 nId, String rString ) { mnId = nId; aString = rString; };
};

//  -----------------------------------------------------------------------

Dictionary::~Dictionary()
{
    for ( void* pPtr = First(); pPtr; pPtr = Next() )
        delete (Dict*)pPtr;
}

//  -----------------------------------------------------------------------

void Dictionary::AddProperty( sal_uInt32 nId, const String& rString )
{
    if ( rString.Len() )        // eindeutige namen bei properties
    {
        // pruefen, ob es die Propertybeschreibung in der Dictionary schon gibt
        for ( Dict* pDict = (Dict*)First(); pDict; pDict = (Dict*)Next() )
        {
            if ( pDict->mnId == nId )
            {
                pDict->aString = rString;
                return;
            }
        }
        Insert( new Dict( nId, rString ), LIST_APPEND );
    }
}

//  -----------------------------------------------------------------------

UINT32 Dictionary::GetProperty( const String& rString )
{
    for ( Dict* pDict = (Dict*)First(); pDict; pDict = (Dict*)Next() )
    {
        if ( pDict->aString == rString )
            return pDict->mnId;
    }
    return 0;
}

//  -----------------------------------------------------------------------

Dictionary& Dictionary::operator=( Dictionary& rDictionary )
{
    if ( this != &rDictionary )
    {
        for ( void* pPtr = First(); pPtr; pPtr = Next() )
            delete (Dict*)pPtr;

        for ( pPtr = rDictionary.First(); pPtr; pPtr = rDictionary.Next() )
            Insert( new Dict( ((Dict*)pPtr)->mnId, ((Dict*)pPtr)->aString ), LIST_APPEND );
    }
    return *this;
}

//  -----------------------------------------------------------------------

Section::Section( Section& rSection )
{
    mnTextEnc = rSection.mnTextEnc;
    for ( int i = 0; i < 16; i++ )
        aFMTID[ i ] = rSection.aFMTID[ i ];
    for ( PropEntry* pProp = (PropEntry*)rSection.First(); pProp; pProp = (PropEntry*)rSection.Next() )
        Insert( new PropEntry( *pProp ), LIST_APPEND );
}

//  -----------------------------------------------------------------------

Section::Section( const sal_uInt8* pFMTID )
{
    mnTextEnc = RTL_TEXTENCODING_MS_1252;
    for ( int i = 0; i < 16; i++ )
        aFMTID[ i ] = pFMTID[ i ];
}

//  -----------------------------------------------------------------------

sal_Bool Section::GetProperty( sal_uInt32 nId, PropItem& rPropItem )
{
    if ( nId )
    {
        for ( PropEntry* pProp = (PropEntry*)First(); pProp; pProp = (PropEntry*)Next() )
        {
            if ( pProp->mnId == nId )
                break;
        }
        if ( pProp )
        {
            rPropItem.Clear();
            rPropItem.SetTextEncoding( mnTextEnc );
            rPropItem.Write( pProp->mpBuf, pProp->mnSize );
            rPropItem.Seek( STREAM_SEEK_TO_BEGIN );
            return sal_True;
        }
    }
    return sal_False;
}

//  -----------------------------------------------------------------------

void Section::AddProperty( sal_uInt32 nId, const sal_uInt8* pBuf, sal_uInt32 nBufSize )
{
    // kleiner id check

    if ( !nId )
        return;
    if ( nId == 0xffffffff )
        nId = 0;

    // keine doppelten PropId's zulassen, sortieren
    for ( sal_uInt32 i = 0; i < Count(); i++ )
    {
        PropEntry* pPropEntry = (PropEntry*)GetObject( i );
        if ( pPropEntry->mnId == nId )
            delete (PropEntry*)Replace( new PropEntry( nId, pBuf, nBufSize, mnTextEnc ), i );
        else if ( pPropEntry->mnId > nId )
            Insert( new PropEntry( nId, pBuf, nBufSize, mnTextEnc ), i );
        else
            continue;
        return;
    }
    Insert( new PropEntry( nId, pBuf, nBufSize, mnTextEnc ), LIST_APPEND );
}

//  -----------------------------------------------------------------------

sal_Bool Section::GetDictionary( Dictionary& rDict )
{
    sal_Bool bRetValue = sal_False;

    Dictionary aDict;

    for ( PropEntry* pProp = (PropEntry*)First(); pProp; pProp = (PropEntry*)Next() )
    {
        if ( pProp->mnId == 0 )
            break;
    }
    if ( pProp )
    {
        sal_uInt32 nCount, nId, nSize, nPos;
        SvMemoryStream aStream( (sal_Int8*)pProp->mpBuf, pProp->mnSize, STREAM_READ );
        aStream.Seek( STREAM_SEEK_TO_BEGIN );
        aStream >> nCount;
        for ( sal_uInt32 i = 0; i < nCount; i++ )
        {
            aStream >> nId >> nSize;
            if ( (sal_uInt16)nSize )
            {
                String aString;
                nPos = aStream.Tell();
                sal_Char* pString = new sal_Char[ (sal_uInt16)nSize ];
                aStream.Read( pString, (sal_uInt16)nSize );
                if ( mnTextEnc == RTL_TEXTENCODING_UCS2 )
                {
                    nSize >>= 1;
                    aStream.Seek( nPos );
                    sal_Unicode* pWString = (sal_Unicode*)pString;
                    for ( i = 0; i < (sal_uInt16)nSize; i++ )
                        aStream >> pWString[ i ];
                    aString = String( pWString, (sal_uInt16)nSize - 1 );
                }
                else
                    aString = String( ByteString( pString, (sal_uInt16)nSize - 1 ), mnTextEnc );
                delete[] pString;
                if ( !aString.Len() )
                    break;
                aDict.AddProperty( nId, aString );
            }
            bRetValue = sal_True;
        }
    }
    rDict = aDict;
    return bRetValue;
}

//  -----------------------------------------------------------------------

Section::~Section()
{
    for ( PropEntry* pProp = (PropEntry*)First(); pProp; pProp = (PropEntry*)Next() )
        delete pProp;
}

//  -----------------------------------------------------------------------

void Section::Read( SvStorageStream *pStrm )
{
    sal_uInt32 i, nSecOfs, nSecSize, nPropCount, nPropId, nPropOfs, nPropType, nPropSize, nCurrent, nVectorCount, nTemp;
    nSecOfs = pStrm->Tell();
    mnTextEnc = RTL_TEXTENCODING_MS_1252;
    *pStrm >> nSecSize >> nPropCount;
    while( nPropCount-- && ( pStrm->GetError() == ERRCODE_NONE ) )
    {
        *pStrm >> nPropId >> nPropOfs;
        nCurrent = pStrm->Tell();
        pStrm->Seek( nPropOfs + nSecOfs );
        if ( nPropId )                  // dictionary wird nicht eingelesen
        {

            *pStrm >> nPropType;

            nPropSize = 4;

            if ( nPropType & VT_VECTOR )
            {
                *pStrm >> nVectorCount;
                nPropType &=~VT_VECTOR;
                nPropSize += 4;
            }
            else
                nVectorCount = 1;


            sal_Bool bVariant = ( nPropType == VT_VARIANT );

            for ( i = 0; nPropSize && ( i < nVectorCount ); i++ )
            {
                if ( bVariant )
                {
                    *pStrm >> nPropType;
                    nPropSize += 4;
                }
                switch( nPropType )
                {
                    case VT_UI1 :
                        nPropSize++;
                    break;

                    case VT_I2 :
                    case VT_UI2 :
                    case VT_BOOL :
                        nPropSize += 2;
                    break;

                    case VT_I4 :
                    case VT_R4 :
                    case VT_UI4 :
                    case VT_ERROR :
                        nPropSize += 4;
                    break;

                    case VT_I8 :
                    case VT_R8 :
                    case VT_CY :
                    case VT_UI8 :
                    case VT_DATE :
                    case VT_FILETIME :
                        nPropSize += 8;
                    break;

                    case VT_BSTR :
                        *pStrm >> nTemp;
                        nPropSize += ( nTemp + 4 );
                    break;

                    case VT_LPSTR :
                        *pStrm >> nTemp;
                        nPropSize += ( nTemp + 4 );
                    break;

                    case VT_LPWSTR :
                        *pStrm >> nTemp;
                        nPropSize += ( nTemp << 1 ) + 4;
                    break;

                    case VT_BLOB_OBJECT :
                    case VT_BLOB :
                    case VT_CF :
                        *pStrm >> nTemp;
                        nPropSize += ( nTemp + 4 );
                    break;

                    case VT_CLSID :
                    case VT_STREAM :
                    case VT_STORAGE :
                    case VT_STREAMED_OBJECT :
                    case VT_STORED_OBJECT :
                    case VT_VARIANT :
                    case VT_VECTOR :
                    default :
                        nPropSize = 0;
                }
                if ( nPropSize )
                {
                    if ( ( nVectorCount - i ) > 1 )
                        pStrm->Seek( nPropOfs + nSecOfs + nPropSize );
                }
                else
                    break;
            }
            if ( nPropSize )
            {
                pStrm->Seek( nPropOfs + nSecOfs );
                sal_uInt8* pBuf = new sal_uInt8[ nPropSize ];
                pStrm->Read( pBuf, nPropSize );
                AddProperty( nPropId, pBuf, nPropSize );
                delete[] pBuf;
            }
            if ( nPropId == 1 )
            {
                PropItem aPropItem;
                if ( GetProperty( 1, aPropItem ) )
                {
                    sal_uInt16 nCodePage;
                    aPropItem >> nPropType;
                    if ( nPropType == VT_I2 )
                        aPropItem >> nCodePage;
                    if ( nCodePage == 1200 )
                        mnTextEnc = RTL_TEXTENCODING_UCS2;
                    else
                    {
                        mnTextEnc = rtl_getTextEncodingFromWindowsCodePage( nCodePage );
                        if ( mnTextEnc == RTL_TEXTENCODING_DONTKNOW )
                            mnTextEnc = RTL_TEXTENCODING_MS_1252;
                    }
                }
            }
        }
        else
        {
            sal_uInt32 nDictCount, nSize;
            *pStrm >> nDictCount;
            for ( i = 0; i < nDictCount; i++ )
            {
                *pStrm >> nSize >> nSize;
                pStrm->SeekRel( nSize );
            }
            nSize = pStrm->Tell();
            pStrm->Seek( nPropOfs + nSecOfs );
            nSize -= pStrm->Tell();
            sal_uInt8* pBuf = new sal_uInt8[ nSize ];
            pStrm->Read( pBuf, nSize );
            AddProperty( 0xffffffff, pBuf, nSize );
            delete[] pBuf;
        }
        pStrm->Seek( nCurrent );
    }
    pStrm->Seek( nSecOfs + nSecSize );
}

//  -----------------------------------------------------------------------

Section& Section::operator=( Section& rSection )
{
    if ( this != &rSection )
    {
        memcpy( (void*)aFMTID, (void*)rSection.aFMTID, 16 );
        for ( PropEntry* pProp = (PropEntry*)First(); pProp; pProp = (PropEntry*)Next() )
            delete pProp;
        Clear();
        for ( pProp = (PropEntry*)rSection.First(); pProp; pProp = (PropEntry*)rSection.Next() )
            Insert( new PropEntry( *pProp ), LIST_APPEND );
    }
    return *this;
}

//  -----------------------------------------------------------------------

PropRead::PropRead( SvStorage& rStorage, const String& rName ) :
        mbStatus            ( sal_False ),
        mnByteOrder         ( 0xfffe ),
        mnFormat            ( 0 ),
        mnVersionLo         ( 4 ),
        mnVersionHi         ( 2 )
{
    if ( rStorage.IsStream( rName ) )
    {
        mpSvStream = rStorage.OpenStream( rName, STREAM_STD_READ );
        if ( mpSvStream )
        {
            mpSvStream->SetNumberFormatInt( NUMBERFORMAT_INT_LITTLEENDIAN );
            memset( mApplicationCLSID, 0, 16 );
            mbStatus = sal_True;
        }
    }
}

//  -----------------------------------------------------------------------

void PropRead::AddSection( Section& rSection )
{
    Insert( new Section( rSection ), LIST_APPEND );
}

//  -----------------------------------------------------------------------

const Section* PropRead::GetSection( const sal_uInt8* pFMTID )
{
    for ( Section* pSection = (Section*)First(); pSection; pSection = (Section*)Next() )
    {
        if ( memcmp( pSection->GetFMTID(), pFMTID, 16 ) == 0 )
            break;
    }
    return pSection;
}

//  -----------------------------------------------------------------------

PropRead::~PropRead()
{
    for ( Section* pSection = (Section*)First(); pSection; pSection = (Section*)Next() )
        delete pSection;
}

//  -----------------------------------------------------------------------

void PropRead::Read()
{
    for ( Section* pSection = (Section*)First(); pSection; pSection = (Section*)Next() )
        delete pSection;
    Clear();
    if ( mbStatus )
    {
        sal_uInt32  nSections;
        sal_uInt32  nSectionOfs;
        sal_uInt32  nCurrent;
        sal_uInt8*  pSectCLSID = new sal_uInt8[ 16 ];
        *mpSvStream >> mnByteOrder >> mnFormat >> mnVersionLo >> mnVersionHi;
        mpSvStream->Read( mApplicationCLSID, 16 );
        *mpSvStream >> nSections;
        for ( sal_uInt32 i = 0; i < nSections; i++ )
        {
            mpSvStream->Read( pSectCLSID, 16 );
            *mpSvStream >> nSectionOfs;
            nCurrent = mpSvStream->Tell();
            mpSvStream->Seek( nSectionOfs );
            Section aSection( pSectCLSID );
            aSection.Read( mpSvStream );
            AddSection( aSection );
            mpSvStream->Seek( nCurrent );
        }
        delete[] pSectCLSID;
    }
}

//  -----------------------------------------------------------------------

PropRead& PropRead::operator=( PropRead& rPropRead )
{
    if ( this != &rPropRead )
    {
        mbStatus = rPropRead.mbStatus;
        mpSvStream = rPropRead.mpSvStream;

        mnByteOrder = rPropRead.mnByteOrder;
        mnFormat = rPropRead.mnFormat;
        mnVersionLo = rPropRead.mnVersionLo;
        mnVersionHi = rPropRead.mnVersionHi;
        memcpy( mApplicationCLSID, rPropRead.mApplicationCLSID, 16 );

        for ( Section* pSection = (Section*)First(); pSection; pSection = (Section*)Next() )
            delete pSection;
        Clear();
        for ( pSection = (Section*)rPropRead.First(); pSection; pSection = (Section*)rPropRead.Next() )
            Insert( new Section( *pSection ), LIST_APPEND );
    }
    return *this;
}

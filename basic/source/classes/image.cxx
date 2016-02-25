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

#include <tools/stream.hxx>
#include <tools/tenccvt.hxx>
#include <osl/thread.h>
#include <basic/sbx.hxx>
#include "sb.hxx"
#include <string.h>
#include "image.hxx"
#include <codegen.hxx>
#include <memory>

SbiImage::SbiImage()
{
    pStringOff = nullptr;
    pStrings   = nullptr;
    pCode      = nullptr;
    pLegacyPCode = nullptr;
    nFlags     = SbiImageFlags::NONE;
    nStrings   = 0;
    nStringSize= 0;
    nCodeSize  = 0;
    nLegacyCodeSize  =
    nDimBase   = 0;
    bInit      =
    bError     = false;
    bFirstInit = true;
    eCharSet   = osl_getThreadTextEncoding();
    nStringIdx = 0;
    nStringOff = 0;
}

SbiImage::~SbiImage()
{
    Clear();
}

void SbiImage::Clear()
{
    delete[] pStringOff;
    delete[] pStrings;
    delete[] pCode;
    ReleaseLegacyBuffer();
    pStringOff = nullptr;
    pStrings   = nullptr;
    pCode      = nullptr;
    nFlags     = SbiImageFlags::NONE;
    nStrings   = 0;
    nStringSize= 0;
    nLegacyCodeSize  = 0;
    nCodeSize  = 0;
    eCharSet   = osl_getThreadTextEncoding();
    nDimBase   = 0;
    bError     = false;
}

/**************************************************************************
*
*    Service-Routines for Load/Store
*
**************************************************************************/

bool SbiGood( SvStream& r )
{
    return !r.IsEof() && r.GetError() == SVSTREAM_OK;
}

// Open Record
sal_uInt64 SbiOpenRecord( SvStream& r, FileOffset nSignature, sal_uInt16 nElem )
{
    sal_uInt64 nPos = r.Tell();
    r.WriteUInt16( static_cast<sal_uInt16>( nSignature ) )
        .WriteInt32( 0 ).WriteUInt16( nElem );
    return nPos;
}

// Close Record
void SbiCloseRecord( SvStream& r, sal_uInt64 nOff )
{
    sal_uInt64 nPos = r.Tell();
    r.Seek( nOff + 2 );
    r.WriteInt32(nPos - nOff - 8 );
    r.Seek( nPos );
}

/**************************************************************************
*
*    Load/Store
*
**************************************************************************/

bool SbiImage::Load( SvStream& r, sal_uInt32& nVersion )
{

    sal_uInt16 nSign, nCount;
    sal_uInt32 nLen, nOff;

    Clear();
    // Read Master-Record
    r.ReadUInt16( nSign ).ReadUInt32( nLen ).ReadUInt16( nCount );
    sal_uInt64 nLast = r.Tell() + nLen;
    sal_uInt32 nCharSet;               // System charset
    sal_uInt32 lDimBase;
    sal_uInt16 nReserved1;
    sal_uInt32 nReserved2;
    sal_uInt32 nReserved3;
    bool bBadVer = false;
    if( nSign == static_cast<sal_uInt16>( FileOffset::Module ) )
    {
        sal_uInt16 nTmpFlags;
        r.ReadUInt32( nVersion ).ReadUInt32( nCharSet ).ReadUInt32( lDimBase )
         .ReadUInt16( nTmpFlags ).ReadUInt16( nReserved1 ).ReadUInt32( nReserved2 ).ReadUInt32( nReserved3 );
        nFlags = static_cast<SbiImageFlags>(nTmpFlags);
        eCharSet = nCharSet;
        eCharSet = GetSOLoadTextEncoding( eCharSet );
        bBadVer  = ( nVersion > B_CURVERSION );
        nDimBase = (sal_uInt16) lDimBase;
    }

    bool bLegacy = ( nVersion < B_EXT_IMG_VERSION );

    sal_uInt64 nNext;
    while( ( nNext = r.Tell() ) < nLast )
    {

        r.ReadUInt16( nSign ).ReadUInt32( nLen ).ReadUInt16( nCount );
        nNext += nLen + 8;
        if( r.GetError() == SVSTREAM_OK )
        {
            switch( static_cast<FileOffset>( nSign ) )
            {
            case FileOffset::Name:
                aName = r.ReadUniOrByteString(eCharSet);
                break;
            case FileOffset::Comment:
                aComment = r.ReadUniOrByteString(eCharSet );
                break;
            case FileOffset::Source:
            {
                aOUSource = r.ReadUniOrByteString(eCharSet);
                break;
            }
            case FileOffset::ExtSource:
            {
                //assuming an empty string with just the lead 32bit/16bit len indicator
                const size_t nMinStringSize = (eCharSet == RTL_TEXTENCODING_UNICODE) ? 4 : 2;
                const sal_uInt64 nMaxStrings = r.remainingSize() / nMinStringSize;
                if (nCount > nMaxStrings)
                {
                    SAL_WARN("basic", "Parsing error: " << nMaxStrings <<
                             " max possible entries, but " << nCount << " claimed, truncating");
                    nCount = nMaxStrings;
                }
                for( sal_uInt16 j = 0; j < nCount; ++j)
                {
                    aOUSource += r.ReadUniOrByteString(eCharSet);
                }
                break;
            }
            case FileOffset::PCode:
                if( bBadVer ) break;
                pCode = new char[ nLen ];
                nCodeSize = nLen;
                r.Read( pCode, nCodeSize );
                if ( bLegacy )
                {
                    ReleaseLegacyBuffer(); // release any previously held buffer
                    nLegacyCodeSize = (sal_uInt16) nCodeSize;
                    pLegacyPCode = pCode;

                    PCodeBuffConvertor< sal_uInt16, sal_uInt32 > aLegacyToNew( reinterpret_cast<sal_uInt8*>(pLegacyPCode), nLegacyCodeSize );
                    aLegacyToNew.convert();
                    pCode = reinterpret_cast<char*>(aLegacyToNew.GetBuffer());
                    nCodeSize = aLegacyToNew.GetSize();
                    // we don't release the legacy buffer
                    // right now, thats because the module
                    // needs it to fix up the method
                    // nStart members. When that is done
                    // the module can release the buffer
                    // or it can wait until this routine
                    // is called again or when this class                       // destructs all of which will trigger
                    // release of the buffer.
                }
                break;
            case FileOffset::Publics:
            case FileOffset::PoolDir:
            case FileOffset::SymPool:
            case FileOffset::LineRanges:
                break;
            case FileOffset::StringPool:
            {
                if( bBadVer ) break;
                //assuming an empty string with just the lead 32bit len indicator
                const sal_uInt64 nMinStringSize = 4;
                const sal_uInt64 nMaxStrings = r.remainingSize() / nMinStringSize;
                if (nCount > nMaxStrings)
                {
                    SAL_WARN("basic", "Parsing error: " << nMaxStrings <<
                             " max possible entries, but " << nCount << " claimed, truncating");
                    nCount = nMaxStrings;
                }
                MakeStrings( nCount );
                short i;
                for( i = 0; i < nStrings && SbiGood( r ); i++ )
                {
                    r.ReadUInt32( nOff );
                    pStringOff[ i ] = (sal_uInt16) nOff;
                }
                r.ReadUInt32( nLen );
                if( SbiGood( r ) )
                {
                    delete [] pStrings;
                    pStrings = new sal_Unicode[ nLen ];
                    nStringSize = (sal_uInt16) nLen;

                    std::unique_ptr<char[]> pByteStrings(new char[ nLen ]);
                    r.Read( pByteStrings.get(), nStringSize );
                    for( short j = 0; j < nStrings; j++ )
                    {
                        sal_uInt16 nOff2 = (sal_uInt16) pStringOff[ j ];
                        OUString aStr( pByteStrings.get() + nOff2, strlen(pByteStrings.get() + nOff2), eCharSet );
                        memcpy( pStrings + nOff2, aStr.getStr(), (aStr.getLength() + 1) * sizeof( sal_Unicode ) );
                    }
                }
                break;
            }
            case FileOffset::UserTypes:
            {
                //assuming an empty string with just the lead 32bit/16bit len indicator
                const size_t nMinStringSize = (eCharSet == RTL_TEXTENCODING_UNICODE) ? 4 : 2;
                const sal_uInt64 nMinRecordSize = nMinStringSize + sizeof(sal_Int16);
                const sal_uInt64 nMaxRecords = r.remainingSize() / nMinRecordSize;
                if (nCount > nMaxRecords)
                {
                    SAL_WARN("basic", "Parsing error: " << nMaxRecords <<
                             " max possible entries, but " << nCount << " claimed, truncating");
                    nCount = nMaxRecords;
                }

                // User defined types
                for (sal_uInt16 i = 0; i < nCount; i++)
                {
                    OUString aTypeName = r.ReadUniOrByteString(eCharSet);

                    sal_uInt16 nTypeMembers;
                    r.ReadUInt16(nTypeMembers);

                    const sal_uInt64 nMaxTypeMembers = r.remainingSize() / 8;
                    if (nTypeMembers > nMaxTypeMembers)
                    {
                        SAL_WARN("basic", "Parsing error: " << nMaxTypeMembers <<
                                 " max possible entries, but " << nTypeMembers << " claimed, truncating");
                        nTypeMembers = nMaxTypeMembers;
                    }

                    SbxObject *pType = new SbxObject(aTypeName);
                    SbxArray *pTypeMembers = pType->GetProperties();

                    for (sal_uInt16 j = 0; j < nTypeMembers; j++)
                    {
                        OUString aMemberName = r.ReadUniOrByteString(eCharSet);

                        sal_Int16 aIntMemberType;
                        r.ReadInt16(aIntMemberType);
                        SbxDataType aMemberType = static_cast< SbxDataType > ( aIntMemberType );

                        SbxProperty *pTypeElem = new SbxProperty( aMemberName, aMemberType );

                        sal_uInt32 aIntFlag;
                        r.ReadUInt32(aIntFlag);
                        SbxFlagBits nElemFlags = static_cast< SbxFlagBits > ( aIntFlag );

                        pTypeElem->SetFlags(nElemFlags);

                        sal_Int16 hasObject;
                        r.ReadInt16(hasObject);

                        if (hasObject == 1)
                        {
                            if(aMemberType == SbxOBJECT)
                            {
                                // nested user defined types
                                // declared before use, so it is ok to reference it by name on load
                                OUString aNestedTypeName = r.ReadUniOrByteString(eCharSet);
                                SbxObject* pNestedTypeObj = static_cast< SbxObject* >( rTypes->Find( aNestedTypeName, SbxCLASS_OBJECT ) );
                                if (pNestedTypeObj)
                                {
                                    SbxObject* pCloneObj = cloneTypeObjectImpl( *pNestedTypeObj );
                                    pTypeElem->PutObject( pCloneObj );
                                }
                            }
                            else
                            {
                                // an array
                                SbxDimArray* pArray = new SbxDimArray();

                                sal_Int16 isFixedSize;
                                r.ReadInt16(isFixedSize);
                                if (isFixedSize == 1)
                                    pArray->setHasFixedSize( true );

                                sal_Int32 nDims;
                                r.ReadInt32(nDims);
                                for (sal_Int32 d = 0; d < nDims; d++)
                                {
                                    sal_Int32 lBound;
                                    sal_Int32 uBound;
                                    r.ReadInt32(lBound).ReadInt32(uBound);
                                    pArray->unoAddDim32(lBound, uBound);
                                }

                                pTypeElem->PutObject( pArray );
                            }
                        }

                        pTypeMembers->Insert( pTypeElem, pTypeMembers->Count() );

                    }

                    pType->Remove( "Name", SbxCLASS_DONTCARE );
                    pType->Remove( "Parent", SbxCLASS_DONTCARE );

                    AddType(pType);
                }
                break;
            }
            case FileOffset::ModEnd:
                goto done;
            default:
                break;
            }
        }
        else
        {
            break;
        }
        r.Seek( nNext );
    }
done:
    r.Seek( nLast );
    if( !SbiGood( r ) )
    {
        bError = true;
    }
    return !bError;
}

bool SbiImage::Save( SvStream& r, sal_uInt32 nVer )
{
    bool bLegacy = ( nVer < B_EXT_IMG_VERSION );

    // detect if old code exceeds legacy limits
    // if so, then disallow save
    if ( bLegacy && ExceedsLegacyLimits() )
    {
        SbiImage aEmptyImg;
        aEmptyImg.aName = aName;
        aEmptyImg.Save( r, B_LEGACYVERSION );
        return true;
    }
    // First of all the header
    sal_uInt64 nStart = SbiOpenRecord( r, FileOffset::Module, 1 );
    sal_uInt64 nPos;

    eCharSet = GetSOStoreTextEncoding( eCharSet );
    if ( bLegacy )
    {
        r.WriteInt32( B_LEGACYVERSION );
    }
    else
    {
        r.WriteInt32( B_CURVERSION );
    }
    r .WriteInt32( eCharSet )
      .WriteInt32( nDimBase )
      .WriteInt16( static_cast<sal_uInt16>(nFlags) )
      .WriteInt16( 0 )
      .WriteInt32( 0 )
      .WriteInt32( 0 );

    // Name?
    if( !aName.isEmpty() && SbiGood( r ) )
    {
        nPos = SbiOpenRecord( r, FileOffset::Name, 1 );
        r.WriteUniOrByteString( aName, eCharSet );
        SbiCloseRecord( r, nPos );
    }
    // Comment?
    if( !aComment.isEmpty() && SbiGood( r ) )
    {
        nPos = SbiOpenRecord( r, FileOffset::Comment, 1 );
        r.WriteUniOrByteString( aComment, eCharSet );
        SbiCloseRecord( r, nPos );
    }
    // Source?
    if( !aOUSource.isEmpty() && SbiGood( r ) )
    {
        nPos = SbiOpenRecord( r, FileOffset::Source, 1 );
        r.WriteUniOrByteString( aOUSource, eCharSet );
        SbiCloseRecord( r, nPos );
    }
    // Binary data?
    if( pCode && SbiGood( r ) )
    {
        nPos = SbiOpenRecord( r, FileOffset::PCode, 1 );
        if ( bLegacy )
        {
            ReleaseLegacyBuffer(); // release any previously held buffer
            PCodeBuffConvertor< sal_uInt32, sal_uInt16 > aNewToLegacy( reinterpret_cast<sal_uInt8*>(pCode), nCodeSize );
            aNewToLegacy.convert();
            pLegacyPCode = reinterpret_cast<char*>(aNewToLegacy.GetBuffer());
            nLegacyCodeSize = aNewToLegacy.GetSize();
            r.Write( pLegacyPCode, nLegacyCodeSize );
        }
        else
        {
            r.Write( pCode, nCodeSize );
        }
        SbiCloseRecord( r, nPos );
    }
    // String-Pool?
    if( nStrings )
    {
        nPos = SbiOpenRecord( r, FileOffset::StringPool, nStrings );
        // For every String:
        //  sal_uInt32 Offset of the Strings in the Stringblock
        short i;

        for( i = 0; i < nStrings && SbiGood( r ); i++ )
        {
            r.WriteUInt32( pStringOff[ i ] );
        }
        // Then the String-Block
        std::unique_ptr<char[]> pByteStrings(new char[ nStringSize ]);
        for( i = 0; i < nStrings; i++ )
        {
            sal_uInt16 nOff = (sal_uInt16) pStringOff[ i ];
            OString aStr(OUStringToOString(OUString(pStrings + nOff), eCharSet));
            memcpy( pByteStrings.get() + nOff, aStr.getStr(), (aStr.getLength() + 1) * sizeof( char ) );
        }
        r.WriteUInt32( nStringSize );
        r.Write( pByteStrings.get(), nStringSize );

        pByteStrings.reset();
        SbiCloseRecord( r, nPos );
    }
    // User defined types
    if (rTypes)
    {
        sal_uInt16 nTypes = rTypes->Count();
        if (nTypes > 0 )
        {
            nPos = SbiOpenRecord( r, FileOffset::UserTypes, nTypes );

            for (sal_uInt16 i = 0; i < nTypes; i++)
            {
                SbxObject* pType = static_cast< SbxObject* > ( rTypes->Get(i) );
                OUString aTypeName = pType->GetClassName();

                r.WriteUniOrByteString( aTypeName, eCharSet );

                SbxArray  *pTypeMembers = pType->GetProperties();
                sal_uInt16 nTypeMembers = pTypeMembers->Count();

                r.WriteInt16(nTypeMembers);

                for (sal_uInt16 j = 0; j < nTypeMembers; j++)
                {

                    SbxProperty* pTypeElem = static_cast< SbxProperty* > ( pTypeMembers->Get(j) );

                    OUString aElemName = pTypeElem->GetName();
                    r.WriteUniOrByteString( aElemName, eCharSet );

                    SbxDataType dataType =   pTypeElem->GetType();
                    r.WriteInt16(dataType);

                    SbxFlagBits nElemFlags = pTypeElem->GetFlags();
                    r.WriteUInt32(static_cast< sal_uInt32 > (nElemFlags) );

                    SbxBase* pElemObject = pTypeElem->GetObject();

                    if (pElemObject)
                    {
                        r.WriteInt16(1); // has elem Object

                        if( dataType == SbxOBJECT )
                        {
                            // nested user defined types
                            // declared before use, so it is ok to reference it by name on load
                            SbxObject* pNestedType = static_cast< SbxObject* > ( pElemObject );
                            r.WriteUniOrByteString( pNestedType->GetClassName(), eCharSet );
                        }
                        else
                        {
                            // an array
                            SbxDimArray* pArray = static_cast< SbxDimArray* > ( pElemObject );

                            bool bFixedSize = pArray->hasFixedSize();
                            if (bFixedSize)
                                r.WriteInt16(1);
                            else
                                r.WriteInt16(0);

                            sal_Int32 nDims = pArray->GetDims();
                            r.WriteInt32(nDims);

                            for (sal_Int32 d = 0; d < nDims; d++)
                            {
                                sal_Int32 lBound;
                                sal_Int32 uBound;
                                pArray->GetDim32(d, lBound, uBound);
                                r.WriteInt32(lBound).WriteInt32(uBound);
                            }
                        }
                    }
                    else
                        r.WriteInt16(0); // no elem Object

                }
            }
        SbiCloseRecord( r, nPos );
        }
    }
    // Set overall length
    SbiCloseRecord( r, nStart );
    if( !SbiGood( r ) )
    {
        bError = true;
    }
    return !bError;
}

/**************************************************************************
*
*    Routines called by the compiler
*
**************************************************************************/

void SbiImage::MakeStrings( short nSize )
{
    nStrings = 0;
    nStringIdx = 0;
    nStringOff = 0;
    nStringSize = 1024;
    pStrings = new sal_Unicode[ nStringSize ];
    pStringOff = new sal_uInt32[ nSize ];
    nStrings = nSize;
    memset( pStringOff, 0, nSize * sizeof( sal_uInt32 ) );
    memset( pStrings, 0, nStringSize * sizeof( sal_Unicode ) );
}

// Add a string to StringPool. The String buffer is dynamically
// growing in 1K-Steps
void SbiImage::AddString( const OUString& r )
{
    if( nStringIdx >= nStrings )
    {
        bError = true;
    }
    if( !bError )
    {
        sal_Int32  len = r.getLength() + 1;
        sal_uInt32 needed = nStringOff + len;
        if( needed > 0xFFFFFF00L )
        {
            bError = true;  // out of mem!
        }
        else if( needed > nStringSize )
        {
            sal_uInt32 nNewLen = needed + 1024;
            nNewLen &= 0xFFFFFC00;  // trim to 1K border
            sal_Unicode* p = new sal_Unicode[nNewLen];
            memcpy( p, pStrings, nStringSize * sizeof( sal_Unicode ) );
            delete[] pStrings;
            pStrings = p;
            nStringSize = sal::static_int_cast< sal_uInt16 >(nNewLen);
        }
        if( !bError )
        {
            pStringOff[ nStringIdx++ ] = nStringOff;
            memcpy( pStrings + nStringOff, r.getStr(), len * sizeof( sal_Unicode ) );
            nStringOff = nStringOff + len;
            // Last String? The update the size of the buffer
            if( nStringIdx >= nStrings )
            {
                nStringSize = nStringOff;
            }
        }
    }
}

// Add code block
// The block was fetched by the compiler from class SbBuffer and
// is already created with new. Additionally it contains all Integers
// in Big Endian format, so can be directly read/written.
void SbiImage::AddCode( char* p, sal_uInt32 s )
{
    pCode = p;
    nCodeSize = s;
}

// Add user type
void SbiImage::AddType(SbxObject* pObject)
{
    if( !rTypes.Is() )
    {
        rTypes = new SbxArray;
    }
    SbxObject *pCopyObject = new SbxObject(*pObject);
    rTypes->Insert (pCopyObject,rTypes->Count());
}

void SbiImage::AddEnum(SbxObject* pObject) // Register enum type
{
    if( !rEnums.Is() )
    {
        rEnums = new SbxArray;
    }
    rEnums->Insert( pObject, rEnums->Count() );
}


/**************************************************************************
*
*    Accessing the image
*
**************************************************************************/

// Note: IDs start with 1
OUString SbiImage::GetString( short nId ) const
{
    if( nId && nId <= nStrings )
    {
        sal_uInt32 nOff = pStringOff[ nId - 1 ];
        sal_Unicode* pStr = pStrings + nOff;

        // #i42467: Special treatment for vbNullChar
        if( *pStr == 0 )
        {
            sal_uInt32 nNextOff = (nId < nStrings) ? pStringOff[ nId ] : nStringOff;
            sal_uInt32 nLen = nNextOff - nOff - 1;
            if( nLen == 1 )
            {
                // Force length 1 and make char 0 afterwards
                OUString aNullCharStr( (sal_Unicode)0);
                return aNullCharStr;
            }
        }
        else
        {
            return OUString(pStr);
        }
    }
    return OUString();
}

const SbxObject* SbiImage::FindType (const OUString& aTypeName) const
{
    return rTypes.Is() ? static_cast<SbxObject*>(rTypes->Find(aTypeName,SbxCLASS_OBJECT)) : nullptr;
}

sal_uInt16 SbiImage::CalcLegacyOffset( sal_Int32 nOffset )
{
    return SbiCodeGen::calcLegacyOffSet( reinterpret_cast<sal_uInt8*>(pCode), nOffset ) ;
}

sal_uInt32 SbiImage::CalcNewOffset( sal_Int16 nOffset )
{
    return SbiCodeGen::calcNewOffSet( reinterpret_cast<sal_uInt8*>(pLegacyPCode), nOffset ) ;
}

void  SbiImage::ReleaseLegacyBuffer()
{
    delete[] pLegacyPCode;
    pLegacyPCode = nullptr;
    nLegacyCodeSize = 0;
}

bool SbiImage::ExceedsLegacyLimits()
{
    return ( nStringSize > 0xFF00L ) || ( CalcLegacyOffset( nCodeSize ) > 0xFF00L );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

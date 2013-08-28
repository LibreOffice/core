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


#include "oleprops.hxx"

#include <comphelper/types.hxx>
#include <tools/debug.hxx>
#include <tools/datetime.hxx>
#include <rtl/tencinfo.h>
#include <rtl/strbuf.hxx>

// ============================================================================


// ============================================================================

#define STREAM_BUFFER_SIZE 2048

// usings
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::makeAny;

using namespace ::com::sun::star;

#define TIMESTAMP_INVALID_DATETIME      ( DateTime ( Date ( 1, 1, 1601 ), Time ( 0, 0, 0 ) ) )  /// Invalid value for date and time to create invalid instance of TimeStamp.
/// Invalid value for date and time to create invalid instance of TimeStamp.
#define TIMESTAMP_INVALID_UTILDATETIME  (util::DateTime(0, 0, 0, 0, 1, 1, 1601, false))
/// Invalid value for date to create invalid instance of TimeStamp.
#define TIMESTAMP_INVALID_UTILDATE  (util::Date(1, 1, 1601))

static
bool operator==(const util::DateTime &i_rLeft, const util::DateTime &i_rRight)
{
    return i_rLeft.Year             == i_rRight.Year
        && i_rLeft.Month            == i_rRight.Month
        && i_rLeft.Day              == i_rRight.Day
        && i_rLeft.Hours            == i_rRight.Hours
        && i_rLeft.Minutes          == i_rRight.Minutes
        && i_rLeft.Seconds          == i_rRight.Seconds
        && i_rLeft.NanoSeconds      == i_rRight.NanoSeconds
        && i_rLeft.IsUTC            == i_rRight.IsUTC;
}

static
bool operator==(const util::Date &i_rLeft, const util::Date &i_rRight)
{
    return i_rLeft.Year             == i_rRight.Year
        && i_rLeft.Month            == i_rRight.Month
        && i_rLeft.Day              == i_rRight.Day;
}

// ============================================================================

/** Property representing a signed 32-bit integer value. */
class SfxOleInt32Property : public SfxOlePropertyBase
{
public:
    explicit            SfxOleInt32Property( sal_Int32 nPropId, sal_Int32 nValue = 0 );

    inline sal_Int32    GetValue() const { return mnValue; }
    inline void         SetValue( sal_Int32 nValue ) { mnValue = nValue; }

private:
    virtual void        ImplLoad( SvStream& rStrm );
    virtual void        ImplSave( SvStream& rStrm );

private:
    sal_Int32           mnValue;
};

// ============================================================================

/** Property representing a floating-point value. */
class SfxOleDoubleProperty : public SfxOlePropertyBase
{
public:
    explicit            SfxOleDoubleProperty( sal_Int32 nPropId, double fValue = 0.0 );

    inline double       GetValue() const { return mfValue; }
    inline void         SetValue( double fValue ) { mfValue = fValue; }

private:
    virtual void        ImplLoad( SvStream& rStrm );
    virtual void        ImplSave( SvStream& rStrm );

private:
    double              mfValue;
};

// ============================================================================

/** Property representing a boolean value. */
class SfxOleBoolProperty : public SfxOlePropertyBase
{
public:
    explicit            SfxOleBoolProperty( sal_Int32 nPropId, bool bValue = false );

    inline bool         GetValue() const { return mbValue; }
    inline void         SetValue( bool bValue ) { mbValue = bValue; }

private:
    virtual void        ImplLoad( SvStream& rStrm );
    virtual void        ImplSave( SvStream& rStrm );

private:
    bool                mbValue;
};

// ============================================================================

/** Base class for properties that contain a single string value. */
class SfxOleStringPropertyBase : public SfxOlePropertyBase, public SfxOleStringHelper
{
public:
    explicit            SfxOleStringPropertyBase(
                            sal_Int32 nPropId, sal_Int32 nPropType,
                            const SfxOleTextEncoding& rTextEnc );
    explicit            SfxOleStringPropertyBase(
                            sal_Int32 nPropId, sal_Int32 nPropType,
                            const SfxOleTextEncoding& rTextEnc, const String& rValue );
    explicit            SfxOleStringPropertyBase(
                            sal_Int32 nPropId, sal_Int32 nPropType,
                            rtl_TextEncoding eTextEnc );

    inline const String& GetValue() const { return maValue; }
    inline void         SetValue( const String& rValue ) { maValue = rValue; }

private:
    String              maValue;
};

// ============================================================================

/** Property representing a bytestring value. */
class SfxOleString8Property : public SfxOleStringPropertyBase
{
public:
    explicit            SfxOleString8Property(
                            sal_Int32 nPropId, const SfxOleTextEncoding& rTextEnc );
    explicit            SfxOleString8Property(
                            sal_Int32 nPropId, const SfxOleTextEncoding& rTextEnc,
                            const String& rValue );

private:
    virtual void        ImplLoad( SvStream& rStrm );
    virtual void        ImplSave( SvStream& rStrm );
};

// ============================================================================

/** Property representing a Unicode string value. */
class SfxOleString16Property : public SfxOleStringPropertyBase
{
public:
    explicit            SfxOleString16Property( sal_Int32 nPropId );

private:
    virtual void        ImplLoad( SvStream& rStrm );
    virtual void        ImplSave( SvStream& rStrm );
};

// ============================================================================

/** Property representing a filetime value as defined by the Windows API. */
class SfxOleFileTimeProperty : public SfxOlePropertyBase
{
public:
    explicit            SfxOleFileTimeProperty( sal_Int32 nPropId );
    /** @param rDateTime  Date and time as LOCAL time. */
    explicit            SfxOleFileTimeProperty( sal_Int32 nPropId, const util::DateTime& rDateTime );

    /** Returns the time value as LOCAL time. */
    inline const util::DateTime& GetValue() const { return maDateTime; }
    /** @param rDateTime  Date and time as LOCAL time. */
    inline void         SetValue( const util::DateTime& rDateTime ) { maDateTime = rDateTime; }

private:
    virtual void        ImplLoad( SvStream& rStrm );
    virtual void        ImplSave( SvStream& rStrm );

private:
    util::DateTime      maDateTime;
};

/** Property representing a filetime value as defined by the Windows API. */
class SfxOleDateProperty : public SfxOlePropertyBase
{
public:
    explicit            SfxOleDateProperty( sal_Int32 nPropId );

    /** Returns the date value as LOCAL time. */
    inline const util::Date& GetValue() const { return maDate; }
    /** @param rDate  Date as LOCAL time. */
    inline void         SetValue( const util::Date& rDate ) { maDate = rDate; }

private:
    virtual void        ImplLoad( SvStream& rStrm );
    virtual void        ImplSave( SvStream& rStrm );

private:
    util::Date      maDate;
};

// ============================================================================

/** Property representing a thumbnail picture.

    Currently, only saving this property is implemented.
 */
class SfxOleThumbnailProperty : public SfxOlePropertyBase
{
public:
    explicit            SfxOleThumbnailProperty( sal_Int32 nPropId,
                            const uno::Sequence<sal_uInt8> & i_rData);

    inline bool         IsValid() const { return mData.getLength() > 0; }

private:
    virtual void        ImplLoad( SvStream& rStrm );
    virtual void        ImplSave( SvStream& rStrm );

private:
    uno::Sequence<sal_uInt8>    mData;
};

// ============================================================================

/** Property representing a BLOB (which presumably stands for binary large
    object).

    Currently, only saving this property is implemented.
 */
class SfxOleBlobProperty : public SfxOlePropertyBase
{
public:
    explicit            SfxOleBlobProperty( sal_Int32 nPropId,
                            const uno::Sequence<sal_uInt8> & i_rData);
    inline bool         IsValid() const { return mData.getLength() > 0; }

private:
    virtual void        ImplLoad( SvStream& rStrm );
    virtual void        ImplSave( SvStream& rStrm );

private:
    uno::Sequence<sal_uInt8>    mData;
};

// ============================================================================

sal_uInt16 SfxOleTextEncoding::GetCodePage() const
{
    sal_uInt16 nCodePage = IsUnicode() ? CODEPAGE_UNICODE :
        static_cast< sal_uInt16 >( rtl_getWindowsCodePageFromTextEncoding( *mxTextEnc ) );
    return (nCodePage == CODEPAGE_UNKNOWN) ? CODEPAGE_UTF8 : nCodePage;
}

void SfxOleTextEncoding::SetCodePage( sal_uInt16 nCodePage )
{
    if( nCodePage == CODEPAGE_UNICODE )
        SetUnicode();
    else
    {
        rtl_TextEncoding eTextEnc = rtl_getTextEncodingFromWindowsCodePage( nCodePage );
        if( eTextEnc != RTL_TEXTENCODING_DONTKNOW )
            *mxTextEnc = eTextEnc;
    }
}

// ----------------------------------------------------------------------------

OUString SfxOleStringHelper::LoadString8( SvStream& rStrm ) const
{
    return IsUnicode() ? ImplLoadString16( rStrm ) : ImplLoadString8( rStrm );
}

void SfxOleStringHelper::SaveString8( SvStream& rStrm, const OUString& rValue ) const
{
    if( IsUnicode() )
        ImplSaveString16( rStrm, rValue );
    else
        ImplSaveString8( rStrm, rValue );
}

OUString SfxOleStringHelper::LoadString16( SvStream& rStrm ) const
{
    return ImplLoadString16( rStrm );
}

void SfxOleStringHelper::SaveString16( SvStream& rStrm, const OUString& rValue ) const
{
    ImplSaveString16( rStrm, rValue );
}

OUString SfxOleStringHelper::ImplLoadString8( SvStream& rStrm ) const
{
    OUString aValue;
    // read size field (signed 32-bit)
    sal_Int32 nSize(0);
    rStrm >> nSize;
    // size field includes trailing NUL character
    DBG_ASSERT( (0 < nSize) && (nSize <= 0xFFFF),
        OStringBuffer(RTL_CONSTASCII_STRINGPARAM(
            "SfxOleStringHelper::ImplLoadString8 - invalid string of len ")).
        append(nSize).getStr() );
    if( (0 < nSize) && (nSize <= 0xFFFF) )
    {
        // load character buffer
        ::std::vector< sal_Char > aBuffer( static_cast< size_t >( nSize + 1 ), 0 );
        rStrm.Read( &aBuffer.front(), static_cast< sal_Size >( nSize ) );
        // create string from encoded character array
        aValue = String( &aBuffer.front(), GetTextEncoding() );
    }
    return aValue;
}

OUString SfxOleStringHelper::ImplLoadString16( SvStream& rStrm ) const
{
    OUString aValue;
    // read size field (signed 32-bit), may be buffer size or character count
    sal_Int32 nSize(0);
    rStrm >> nSize;
    DBG_ASSERT( (0 < nSize) && (nSize <= 0xFFFF), "SfxOleStringHelper::ImplLoadString16 - invalid string" );
    // size field includes trailing NUL character
    if( (0 < nSize) && (nSize <= 0xFFFF) )
    {
        // load character buffer
        ::std::vector< sal_Unicode > aBuffer;
        aBuffer.reserve( static_cast< size_t >( nSize + 1 ) );
        sal_uInt16 cChar;
        for( sal_Int32 nIdx = 0; nIdx < nSize; ++nIdx )
        {
            rStrm >> cChar;
            aBuffer.push_back( static_cast< sal_Unicode >( cChar ) );
        }
        // stream is always padded to 32-bit boundary, skip 2 bytes on odd character count
        if( (nSize & 1) == 1 )
            rStrm.SeekRel( 2 );
        // create string from character array
        aBuffer.push_back( 0 );
        aValue = OUString( &aBuffer.front() );
    }
    return aValue;
}

void SfxOleStringHelper::ImplSaveString8( SvStream& rStrm, const OUString& rValue ) const
{
    // encode to byte string
    OString aEncoded(OUStringToOString(rValue, GetTextEncoding()));
    // write size field (including trailing NUL character)
    sal_Int32 nSize = aEncoded.getLength() + 1;
    rStrm << nSize;
    // write character array with trailing NUL character
    rStrm.Write(aEncoded.getStr(), aEncoded.getLength());
    rStrm << sal_uInt8( 0 );
}

void SfxOleStringHelper::ImplSaveString16( SvStream& rStrm, const OUString& rValue ) const
{
    // write size field (including trailing NUL character)
    sal_Int32 nSize = static_cast< sal_Int32 >( rValue.getLength() + 1 );
    rStrm << nSize;
    // write character array with trailing NUL character
    for( sal_Int32 nIdx = 0; nIdx < rValue.getLength(); ++nIdx )
        rStrm << static_cast< sal_uInt16 >( rValue[ nIdx ] );
    rStrm << sal_uInt16( 0 );
    // stream is always padded to 32-bit boundary, add 2 bytes on odd character count
    if( (nSize & 1) == 1 )
        rStrm << sal_uInt16( 0 );
}

// ----------------------------------------------------------------------------

SfxOleObjectBase::~SfxOleObjectBase()
{
}

ErrCode SfxOleObjectBase::Load( SvStream& rStrm )
{
    mnErrCode = ERRCODE_NONE;
    ImplLoad( rStrm );
    SetError( rStrm.GetErrorCode() );
    return GetError();
}

ErrCode SfxOleObjectBase::Save( SvStream& rStrm )
{
    mnErrCode = ERRCODE_NONE;
    ImplSave( rStrm );
    SetError( rStrm.GetErrorCode() );
    return GetError();
}

void SfxOleObjectBase::LoadObject( SvStream& rStrm, SfxOleObjectBase& rObj )
{
    SetError( rObj.Load( rStrm ) );
}

void SfxOleObjectBase::SaveObject( SvStream& rStrm, SfxOleObjectBase& rObj )
{
    SetError( rObj.Save( rStrm ) );
}

// ----------------------------------------------------------------------------

SfxOleCodePageProperty::SfxOleCodePageProperty() :
    SfxOlePropertyBase( PROPID_CODEPAGE, PROPTYPE_INT16 )
{
}

void SfxOleCodePageProperty::ImplLoad( SvStream& rStrm )
{
    // property type is signed int16, but we use always unsigned int16 for codepages
    sal_uInt16 nCodePage;
    rStrm >> nCodePage;
    SetCodePage( nCodePage );
}

void SfxOleCodePageProperty::ImplSave( SvStream& rStrm )
{
    // property type is signed int16, but we use always unsigned int16 for codepages
    rStrm << GetCodePage();
}

// ----------------------------------------------------------------------------

SfxOleInt32Property::SfxOleInt32Property( sal_Int32 nPropId, sal_Int32 nValue ) :
    SfxOlePropertyBase( nPropId, PROPTYPE_INT32 ),
    mnValue( nValue )
{
}

void SfxOleInt32Property::ImplLoad( SvStream& rStrm )
{
    rStrm >> mnValue;
}

void SfxOleInt32Property::ImplSave( SvStream& rStrm )
{
    rStrm << mnValue;
}

// ----------------------------------------------------------------------------

SfxOleDoubleProperty::SfxOleDoubleProperty( sal_Int32 nPropId, double fValue ) :
    SfxOlePropertyBase( nPropId, PROPTYPE_DOUBLE ),
    mfValue( fValue )
{
}

void SfxOleDoubleProperty::ImplLoad( SvStream& rStrm )
{
    rStrm >> mfValue;
}

void SfxOleDoubleProperty::ImplSave( SvStream& rStrm )
{
    rStrm << mfValue;
}

// ----------------------------------------------------------------------------

SfxOleBoolProperty::SfxOleBoolProperty( sal_Int32 nPropId, bool bValue ) :
    SfxOlePropertyBase( nPropId, PROPTYPE_BOOL ),
    mbValue( bValue )
{
}

void SfxOleBoolProperty::ImplLoad( SvStream& rStrm )
{
    sal_Int16 nValue(0);
    rStrm >> nValue;
    mbValue = nValue != 0;
}

void SfxOleBoolProperty::ImplSave( SvStream& rStrm )
{
    rStrm << static_cast< sal_Int16 >( mbValue ? -1 : 0 );
}

// ----------------------------------------------------------------------------

SfxOleStringPropertyBase::SfxOleStringPropertyBase(
        sal_Int32 nPropId, sal_Int32 nPropType, const SfxOleTextEncoding& rTextEnc ) :
    SfxOlePropertyBase( nPropId, nPropType ),
    SfxOleStringHelper( rTextEnc )
{
}

SfxOleStringPropertyBase::SfxOleStringPropertyBase(
        sal_Int32 nPropId, sal_Int32 nPropType, const SfxOleTextEncoding& rTextEnc, const String& rValue ) :
    SfxOlePropertyBase( nPropId, nPropType ),
    SfxOleStringHelper( rTextEnc ),
    maValue( rValue )
{
}

SfxOleStringPropertyBase::SfxOleStringPropertyBase(
        sal_Int32 nPropId, sal_Int32 nPropType, rtl_TextEncoding eTextEnc ) :
    SfxOlePropertyBase( nPropId, nPropType ),
    SfxOleStringHelper( eTextEnc )
{
}

// ----------------------------------------------------------------------------

SfxOleString8Property::SfxOleString8Property(
        sal_Int32 nPropId, const SfxOleTextEncoding& rTextEnc ) :
    SfxOleStringPropertyBase( nPropId, PROPTYPE_STRING8, rTextEnc )
{
}

SfxOleString8Property::SfxOleString8Property(
        sal_Int32 nPropId, const SfxOleTextEncoding& rTextEnc, const String& rValue ) :
    SfxOleStringPropertyBase( nPropId, PROPTYPE_STRING8, rTextEnc, rValue )
{
}

void SfxOleString8Property::ImplLoad( SvStream& rStrm )
{
    SetValue( LoadString8( rStrm ) );
}

void SfxOleString8Property::ImplSave( SvStream& rStrm )
{
    SaveString8( rStrm, GetValue() );
}

// ----------------------------------------------------------------------------

SfxOleString16Property::SfxOleString16Property( sal_Int32 nPropId ) :
    SfxOleStringPropertyBase( nPropId, PROPTYPE_STRING16, RTL_TEXTENCODING_UCS2 )
{
}

void SfxOleString16Property::ImplLoad( SvStream& rStrm )
{
    SetValue( LoadString16( rStrm ) );
}

void SfxOleString16Property::ImplSave( SvStream& rStrm )
{
    SaveString16( rStrm, GetValue() );
}

// ----------------------------------------------------------------------------

SfxOleFileTimeProperty::SfxOleFileTimeProperty( sal_Int32 nPropId ) :
    SfxOlePropertyBase( nPropId, PROPTYPE_FILETIME )
{
}

SfxOleFileTimeProperty::SfxOleFileTimeProperty( sal_Int32 nPropId, const util::DateTime& rDateTime ) :
    SfxOlePropertyBase( nPropId, PROPTYPE_FILETIME ),
    maDateTime( rDateTime )
{
}

void SfxOleFileTimeProperty::ImplLoad( SvStream& rStrm )
{
    sal_uInt32 nLower(0), nUpper(0);
    rStrm >> nLower >> nUpper;
    ::DateTime aDateTime = DateTime::CreateFromWin32FileDateTime( nLower, nUpper );
    // note: editing duration is stored as offset to TIMESTAMP_INVALID_DATETIME
    //       of course we should not convert the time zone of a duration!
    // heuristic to detect editing durations (which we assume to be < 1 year):
    // check only the year, not the entire date
    if ( aDateTime.GetYear() != TIMESTAMP_INVALID_DATETIME.GetYear() )
        aDateTime.ConvertToLocalTime();
    maDateTime.Year    = aDateTime.GetYear();
    maDateTime.Month   = aDateTime.GetMonth();
    maDateTime.Day     = aDateTime.GetDay();
    maDateTime.Hours   = aDateTime.GetHour();
    maDateTime.Minutes = aDateTime.GetMin();
    maDateTime.Seconds = aDateTime.GetSec();
    maDateTime.NanoSeconds = aDateTime.GetNanoSec();
    maDateTime.IsUTC   = false;
}

void SfxOleFileTimeProperty::ImplSave( SvStream& rStrm )
{
    DateTime aDateTimeUtc(
            Date(
                static_cast< sal_uInt16 >( maDateTime.Day ),
                static_cast< sal_uInt16 >( maDateTime.Month ),
                static_cast< sal_uInt16 >( maDateTime.Year ) ),
            Time(
                static_cast< sal_uIntPtr >( maDateTime.Hours ),
                static_cast< sal_uIntPtr >( maDateTime.Minutes ),
                static_cast< sal_uIntPtr >( maDateTime.Seconds ),
                static_cast< sal_uIntPtr >( maDateTime.NanoSeconds ) ) );
    // invalid time stamp is not converted to UTC
    // heuristic to detect editing durations (which we assume to be < 1 year):
    // check only the year, not the entire date
    if( aDateTimeUtc.IsValidAndGregorian()
        && aDateTimeUtc.GetYear() != TIMESTAMP_INVALID_DATETIME.GetYear() ) {
            aDateTimeUtc.ConvertToUTC();
    }
    sal_uInt32 nLower, nUpper;
    aDateTimeUtc.GetWin32FileDateTime( nLower, nUpper );
    rStrm << nLower << nUpper;
}

SfxOleDateProperty::SfxOleDateProperty( sal_Int32 nPropId ) :
    SfxOlePropertyBase( nPropId, PROPTYPE_DATE )
{
}

void SfxOleDateProperty::ImplLoad( SvStream& rStrm )
{
    double fValue(0.0);
    rStrm >> fValue;
    //stored as number of days (not seconds) since December 31, 1899
    ::Date aDate(31, 12, 1899);
    long nDays = fValue;
    aDate += nDays;
    maDate.Day = aDate.GetDay();
    maDate.Month = aDate.GetMonth();
    maDate.Year = aDate.GetYear();
}

void SfxOleDateProperty::ImplSave( SvStream& rStrm )
{
    long nDays = ::Date::DateToDays(maDate.Day, maDate.Month, maDate.Year);
    //number of days (not seconds) since December 31, 1899
    long nStartDays = ::Date::DateToDays(31, 12, 1899);
    double fValue = nDays-nStartDays;
    rStrm << fValue;
}

// ----------------------------------------------------------------------------

SfxOleThumbnailProperty::SfxOleThumbnailProperty(
        sal_Int32 nPropId, const uno::Sequence<sal_uInt8> & i_rData) :
    SfxOlePropertyBase( nPropId, PROPTYPE_CLIPFMT ),
    mData(i_rData)
{
}

void SfxOleThumbnailProperty::ImplLoad( SvStream& )
{
    SAL_WARN( "sfx.doc", "SfxOleThumbnailProperty::ImplLoad - not implemented" );
    SetError( SVSTREAM_INVALID_ACCESS );
}

void SfxOleThumbnailProperty::ImplSave( SvStream& rStrm )
{
    /*  Type        Contents
        -----------------------------------------------------------------------
        int32       size of following data
        int32       clipboard format tag (see below)
        byte[]      clipboard data (see below)

        Clipboard format tag:
            -1 = Windows clipboard format
            -2 = Macintosh clipboard format
            -3 = GUID that contains a format identifier (FMTID)
            >0 = custom clipboard format name plus data (see msdn site below)
            0  = no data

        References:
        http://msdn.microsoft.com/library/default.asp?url=/library/en-us/stg/stg/propvariant.asp
        http://jakarta.apache.org/poi/hpsf/thumbnails.html
        http://linux.com.hk/docs/poi/org/apache/poi/hpsf/Thumbnail.html
        http://sparks.discreet.com/knowledgebase/public/solutions/ExtractThumbnailImg.htm
     */
    if( IsValid() )
    {
        // clipboard size: clip_format_tag + data_format_tag + bitmap_len
        sal_Int32 nClipSize = static_cast< sal_Int32 >( 4 + 4 + mData.getLength() );
        rStrm << nClipSize << CLIPFMT_WIN << CLIPDATAFMT_DIB;
        rStrm.Write( mData.getConstArray(), mData.getLength() );
    }
    else
    {
        SAL_WARN( "sfx.doc", "SfxOleThumbnailProperty::ImplSave - invalid thumbnail property" );
        SetError( SVSTREAM_INVALID_ACCESS );
    }
}

// ----------------------------------------------------------------------------

SfxOleBlobProperty::SfxOleBlobProperty( sal_Int32 nPropId,
        const uno::Sequence<sal_uInt8> & i_rData) :
    SfxOlePropertyBase( nPropId, PROPTYPE_BLOB ),
    mData(i_rData)
{
}

void SfxOleBlobProperty::ImplLoad( SvStream& )
{
    SAL_WARN( "sfx.doc", "SfxOleBlobProperty::ImplLoad - not implemented" );
    SetError( SVSTREAM_INVALID_ACCESS );
}

void SfxOleBlobProperty::ImplSave( SvStream& rStrm )
{
    if (IsValid()) {
        rStrm.Write( mData.getConstArray(), mData.getLength() );
    } else {
        SAL_WARN( "sfx.doc", "SfxOleBlobProperty::ImplSave - invalid BLOB property" );
        SetError( SVSTREAM_INVALID_ACCESS );
    }
}

// ----------------------------------------------------------------------------

SfxOleDictionaryProperty::SfxOleDictionaryProperty( const SfxOleTextEncoding& rTextEnc ) :
    SfxOlePropertyBase( PROPID_DICTIONARY, 0 ),
    SfxOleStringHelper( rTextEnc )
{
}

OUString SfxOleDictionaryProperty::GetPropertyName( sal_Int32 nPropId ) const
{
    SfxOlePropNameMap::const_iterator aIt = maPropNameMap.find( nPropId );
    return (aIt == maPropNameMap.end()) ? OUString("") : aIt->second;
}

void SfxOleDictionaryProperty::SetPropertyName( sal_Int32 nPropId, const OUString& rPropName )
{
    maPropNameMap[ nPropId ] = rPropName;
    // dictionary property contains number of pairs in property type field
    SetPropType( static_cast< sal_Int32 >( maPropNameMap.size() ) );
}

void SfxOleDictionaryProperty::ImplLoad( SvStream& rStrm )
{
    // dictionary property contains number of pairs in property type field
    sal_Int32 nNameCount = GetPropType();
    // read property ID/name pairs
    maPropNameMap.clear();
    for( sal_Int32 nIdx = 0; (nIdx < nNameCount) && (rStrm.GetErrorCode() == SVSTREAM_OK) && !rStrm.IsEof(); ++nIdx )
    {
        sal_Int32 nPropId(0);
        rStrm >> nPropId;
        // name always stored as byte string
        maPropNameMap[ nPropId ] = LoadString8( rStrm );
    }
}

void SfxOleDictionaryProperty::ImplSave( SvStream& rStrm )
{
    // write property ID/name pairs
    for( SfxOlePropNameMap::const_iterator aIt = maPropNameMap.begin(), aEnd = maPropNameMap.end(); aIt != aEnd; ++aIt )
    {
        rStrm << aIt->first;
        // name always stored as byte string
        SaveString8( rStrm, aIt->second );
    }
}

// ----------------------------------------------------------------------------

SfxOleSection::SfxOleSection( bool bSupportsDict ) :
    maDictProp( maCodePageProp ),
    mnStartPos( 0 ),
    mbSupportsDict( bSupportsDict )
{
}

SfxOlePropertyRef SfxOleSection::GetProperty( sal_Int32 nPropId ) const
{
    SfxOlePropertyRef xProp;
    SfxOlePropMap::const_iterator aIt = maPropMap.find( nPropId );
    if( aIt != maPropMap.end() )
        xProp = aIt->second;
    return xProp;
}

bool SfxOleSection::GetInt32Value( sal_Int32& rnValue, sal_Int32 nPropId ) const
{
    SfxOlePropertyRef xProp = GetProperty( nPropId );
    const SfxOleInt32Property* pProp =
        dynamic_cast< const SfxOleInt32Property* >( xProp.get() );
    if( pProp )
        rnValue = pProp->GetValue();
    return pProp != 0;
}

bool SfxOleSection::GetDoubleValue( double& rfValue, sal_Int32 nPropId ) const
{
    SfxOlePropertyRef xProp = GetProperty( nPropId );
    const SfxOleDoubleProperty* pProp =
        dynamic_cast< const SfxOleDoubleProperty* >( xProp.get() );
    if( pProp )
        rfValue = pProp->GetValue();
    return pProp != 0;
}

bool SfxOleSection::GetBoolValue( bool& rbValue, sal_Int32 nPropId ) const
{
    SfxOlePropertyRef xProp = GetProperty( nPropId );
    const SfxOleBoolProperty* pProp =
        dynamic_cast< const SfxOleBoolProperty* >( xProp.get() );
    if( pProp )
        rbValue = pProp->GetValue();
    return pProp != 0;
}

bool SfxOleSection::GetStringValue( OUString& rValue, sal_Int32 nPropId ) const
{
    SfxOlePropertyRef xProp = GetProperty( nPropId );
    const SfxOleStringPropertyBase* pProp =
        dynamic_cast< const SfxOleStringPropertyBase* >( xProp.get() );
    if( pProp )
        rValue = pProp->GetValue();
    return pProp != 0;
}

bool SfxOleSection::GetFileTimeValue( util::DateTime& rValue, sal_Int32 nPropId ) const
{
    SfxOlePropertyRef xProp = GetProperty( nPropId );
    const SfxOleFileTimeProperty* pProp =
        dynamic_cast< const SfxOleFileTimeProperty* >( xProp.get() );
    if( pProp )
    {
        if ( pProp->GetValue() == TIMESTAMP_INVALID_UTILDATETIME )
            rValue = util::DateTime();
        else
            rValue = pProp->GetValue();
    }
    return pProp != 0;
}

bool SfxOleSection::GetDateValue( util::Date& rValue, sal_Int32 nPropId ) const
{
    SfxOlePropertyRef xProp = GetProperty( nPropId );
    const SfxOleDateProperty* pProp =
        dynamic_cast< const SfxOleDateProperty* >( xProp.get() );
    if( pProp )
    {
        if ( pProp->GetValue() == TIMESTAMP_INVALID_UTILDATE )
            rValue = util::Date();
        else
            rValue = pProp->GetValue();
    }
    return pProp != 0;
}

void SfxOleSection::SetProperty( SfxOlePropertyRef xProp )
{
    if( xProp.get() )
        maPropMap[ xProp->GetPropId() ] = xProp;
}

void SfxOleSection::SetInt32Value( sal_Int32 nPropId, sal_Int32 nValue )
{
    SetProperty( SfxOlePropertyRef( new SfxOleInt32Property( nPropId, nValue ) ) );
}

void SfxOleSection::SetDoubleValue( sal_Int32 nPropId, double fValue )
{
    SetProperty( SfxOlePropertyRef( new SfxOleDoubleProperty( nPropId, fValue ) ) );
}

void SfxOleSection::SetBoolValue( sal_Int32 nPropId, bool bValue )
{
    SetProperty( SfxOlePropertyRef( new SfxOleBoolProperty( nPropId, bValue ) ) );
}

bool SfxOleSection::SetStringValue( sal_Int32 nPropId, const OUString& rValue, bool bSkipEmpty )
{
    bool bInserted = !bSkipEmpty || !rValue.isEmpty();
    if( bInserted )
        SetProperty( SfxOlePropertyRef( new SfxOleString8Property( nPropId, maCodePageProp, rValue ) ) );
    return bInserted;
}

void SfxOleSection::SetFileTimeValue( sal_Int32 nPropId, const util::DateTime& rValue )
{
    if ( rValue.Year == 0 || rValue.Month == 0 || rValue.Day == 0 )
        SetProperty( SfxOlePropertyRef( new SfxOleFileTimeProperty( nPropId, TIMESTAMP_INVALID_UTILDATETIME ) ) );
    else
        SetProperty( SfxOlePropertyRef( new SfxOleFileTimeProperty( nPropId, rValue ) ) );
}

void SfxOleSection::SetDateValue( sal_Int32 nPropId, const util::Date& rValue )
{
    //Annoyingly MS2010 considers VT_DATE apparently as an invalid possibility, so here we use VT_FILETIME
    //instead :-(
    if ( rValue.Year == 0 || rValue.Month == 0 || rValue.Day == 0 )
        SetProperty( SfxOlePropertyRef( new SfxOleFileTimeProperty( nPropId, TIMESTAMP_INVALID_UTILDATETIME ) ) );
    else
    {
        const util::DateTime aValue(0, 0, 0, 0, rValue.Day, rValue.Month,
                rValue.Year, false );
        SetProperty( SfxOlePropertyRef( new SfxOleFileTimeProperty( nPropId, aValue ) ) );
    }
}

void SfxOleSection::SetThumbnailValue( sal_Int32 nPropId,
    const uno::Sequence<sal_uInt8> & i_rData)
{
    SfxOleThumbnailProperty* pThumbnail = new SfxOleThumbnailProperty( nPropId, i_rData );
    SfxOlePropertyRef xProp( pThumbnail );  // take ownership
    if( pThumbnail->IsValid() )
        SetProperty( xProp );
}

void SfxOleSection::SetBlobValue( sal_Int32 nPropId,
    const uno::Sequence<sal_uInt8> & i_rData)
{
    SfxOleBlobProperty* pBlob( new SfxOleBlobProperty( nPropId, i_rData ) );
    SfxOlePropertyRef xProp( pBlob );
    if( pBlob->IsValid() ) {
        SetProperty( xProp );
    }
}

Any SfxOleSection::GetAnyValue( sal_Int32 nPropId ) const
{
    Any aValue;
    sal_Int32 nInt32 = 0;
    double fDouble = 0.0;
    bool bBool = false;
    OUString aString;
    ::com::sun::star::util::DateTime aApiDateTime;
    ::com::sun::star::util::Date aApiDate;

    if( GetInt32Value( nInt32, nPropId ) )
        aValue <<= nInt32;
    else if( GetDoubleValue( fDouble, nPropId ) )
        aValue <<= fDouble;
    else if( GetBoolValue( bBool, nPropId ) )
        ::comphelper::setBOOL( aValue, bBool ? sal_True : sal_False );
    else if( GetStringValue( aString, nPropId ) )
        aValue <<= aString;
    else if( GetFileTimeValue( aApiDateTime, nPropId ) )
    {
        aValue <<= aApiDateTime;
    }
    else if( GetDateValue( aApiDate, nPropId ) )
    {
        aValue <<= aApiDate;
    }
    return aValue;
}

bool SfxOleSection::SetAnyValue( sal_Int32 nPropId, const Any& rValue )
{
    bool bInserted = true;
    sal_Int32 nInt32 = 0;
    double fDouble = 0.0;
    OUString aString;
    ::com::sun::star::util::DateTime aApiDateTime;
    ::com::sun::star::util::Date aApiDate;

    if( rValue.getValueType() == ::getBooleanCppuType() )
        SetBoolValue( nPropId, ::comphelper::getBOOL( rValue ) == sal_True );
    else if( rValue >>= nInt32 )
        SetInt32Value( nPropId, nInt32 );
    else if( rValue >>= fDouble )
        SetDoubleValue( nPropId, fDouble );
    else if( rValue >>= aString )
        bInserted = SetStringValue( nPropId, aString );
    else if( rValue >>= aApiDateTime )
        SetFileTimeValue( nPropId, aApiDateTime );
    else if( rValue >>= aApiDate )
        SetDateValue( nPropId, aApiDate );
    else
        bInserted = false;
    return bInserted;
}

OUString SfxOleSection::GetPropertyName( sal_Int32 nPropId ) const
{
    return maDictProp.GetPropertyName( nPropId );
}

void SfxOleSection::SetPropertyName( sal_Int32 nPropId, const OUString& rPropName )
{
    maDictProp.SetPropertyName( nPropId, rPropName );
}

void SfxOleSection::GetPropertyIds( ::std::vector< sal_Int32 >& rPropIds ) const
{
    rPropIds.clear();
    for( SfxOlePropMap::const_iterator aIt = maPropMap.begin(), aEnd = maPropMap.end(); aIt != aEnd; ++aIt )
        rPropIds.push_back( aIt->first );
}

sal_Int32 SfxOleSection::GetFreePropertyId() const
{
    return maPropMap.empty() ? PROPID_FIRSTCUSTOM : (maPropMap.rbegin()->first + 1);
}

void SfxOleSection::ImplLoad( SvStream& rStrm )
{
    // read section header
    mnStartPos = rStrm.Tell();
    sal_uInt32 nSize(0);
    sal_Int32 nPropCount(0);
    rStrm >> nSize >> nPropCount;

    // read property ID/position pairs
    typedef ::std::map< sal_Int32, sal_uInt32 > SfxOlePropPosMap;
    SfxOlePropPosMap aPropPosMap;
    for( sal_Int32 nPropIdx = 0; (nPropIdx < nPropCount) && (rStrm.GetErrorCode() == SVSTREAM_OK) && !rStrm.IsEof(); ++nPropIdx )
    {
        sal_Int32 nPropId(0);
        sal_uInt32 nPropPos(0);
        rStrm >> nPropId >> nPropPos;
        aPropPosMap[ nPropId ] = nPropPos;
    }

    // read codepage property
    SfxOlePropPosMap::iterator aCodePageIt = aPropPosMap.find( PROPID_CODEPAGE );
    if( (aCodePageIt != aPropPosMap.end()) && SeekToPropertyPos( rStrm, aCodePageIt->second ) )
    {
        // codepage property must be of type signed int-16
        sal_Int32 nPropType(0);
        rStrm >> nPropType;
        if( nPropType == PROPTYPE_INT16 )
            LoadObject( rStrm, maCodePageProp );
        // remove property position
        aPropPosMap.erase( aCodePageIt );
    }

    // read dictionary property
    SfxOlePropPosMap::iterator aDictIt = aPropPosMap.find( PROPID_DICTIONARY );
    if( (aDictIt != aPropPosMap.end()) && SeekToPropertyPos( rStrm, aDictIt->second ) )
    {
        // #i66214# #i66428# applications may write broken dictionary properties in wrong sections
        if( mbSupportsDict )
        {
            // dictionary property contains number of pairs in property type field
            sal_Int32 nNameCount(0);
            rStrm >> nNameCount;
            maDictProp.SetNameCount( nNameCount );
            LoadObject( rStrm, maDictProp );
        }
        // always remove position of dictionary property (do not try to read it again below)
        aPropPosMap.erase( aDictIt );
    }

    // read other properties
    maPropMap.clear();
    for( SfxOlePropPosMap::const_iterator aIt = aPropPosMap.begin(), aEnd = aPropPosMap.end(); aIt != aEnd; ++aIt )
        if( SeekToPropertyPos( rStrm, aIt->second ) )
            LoadProperty( rStrm, aIt->first );
}

void SfxOleSection::ImplSave( SvStream& rStrm )
{
    /*  Always export with UTF-8 encoding. All dependent properties (bytestring
        and dictionary) will be updated automatically. */
    maCodePageProp.SetTextEncoding( RTL_TEXTENCODING_UTF8 );

    // write section header
    mnStartPos = rStrm.Tell();
    sal_Int32 nPropCount = static_cast< sal_Int32 >( maPropMap.size() + 1 );
    if( maDictProp.HasPropertyNames() )
        ++nPropCount;
    rStrm << sal_uInt32( 0 ) << nPropCount;

    // write placeholders for property ID/position pairs
    sal_Size nPropPosPos = rStrm.Tell();
    rStrm.SeekRel( static_cast< sal_sSize >( 8 * nPropCount ) );

    // write dictionary property
    if( maDictProp.HasPropertyNames() )
        SaveProperty( rStrm, maDictProp, nPropPosPos );
    // write codepage property
    SaveProperty( rStrm, maCodePageProp, nPropPosPos );
    // write other properties
    for( SfxOlePropMap::const_iterator aIt = maPropMap.begin(), aEnd = maPropMap.end(); aIt != aEnd; ++aIt )
        SaveProperty( rStrm, *aIt->second, nPropPosPos );

    // write section size (first field in section header)
    rStrm.Seek( STREAM_SEEK_TO_END );
    sal_uInt32 nSectSize = static_cast< sal_uInt32 >( rStrm.Tell() - mnStartPos );
    rStrm.Seek( mnStartPos );
    rStrm << nSectSize;
}

bool SfxOleSection::SeekToPropertyPos( SvStream& rStrm, sal_uInt32 nPropPos ) const
{
    rStrm.Seek( static_cast< sal_Size >( mnStartPos + nPropPos ) );
    return rStrm.GetErrorCode() == SVSTREAM_OK;
}

void SfxOleSection::LoadProperty( SvStream& rStrm, sal_Int32 nPropId )
{
    // property data type
    sal_Int32 nPropType(0);
    rStrm >> nPropType;
    // create empty property object
    SfxOlePropertyRef xProp;
    switch( nPropType )
    {
        case PROPTYPE_INT32:
            xProp.reset( new SfxOleInt32Property( nPropId ) );
        break;
        case PROPTYPE_DOUBLE:
            xProp.reset( new SfxOleDoubleProperty( nPropId ) );
        break;
        case PROPTYPE_BOOL:
            xProp.reset( new SfxOleBoolProperty( nPropId ) );
        break;
        case PROPTYPE_STRING8:
            xProp.reset( new SfxOleString8Property( nPropId, maCodePageProp ) );
        break;
        case PROPTYPE_STRING16:
            xProp.reset( new SfxOleString16Property( nPropId ) );
        break;
        case PROPTYPE_FILETIME:
            xProp.reset( new SfxOleFileTimeProperty( nPropId ) );
        break;
        case PROPTYPE_DATE:
            xProp.reset( new SfxOleDateProperty( nPropId ) );
        break;
    }
    // load property contents
    if( xProp.get() )
    {
        SetError( xProp->Load( rStrm ) );
        maPropMap[ nPropId ] = xProp;
    }
}

void SfxOleSection::SaveProperty( SvStream& rStrm, SfxOlePropertyBase& rProp, sal_Size& rnPropPosPos )
{
    rStrm.Seek( STREAM_SEEK_TO_END );
    sal_uInt32 nPropPos = static_cast< sal_uInt32 >( rStrm.Tell() - mnStartPos );
    // property data type
    rStrm << rProp.GetPropType();
    // write property contents
    SaveObject( rStrm, rProp );
    // align to 32-bit
    while( (rStrm.Tell() & 3) != 0 )
        rStrm << sal_uInt8( 0 );
    // write property ID/position pair
    rStrm.Seek( rnPropPosPos );
    rStrm << rProp.GetPropId() << nPropPos;
    rnPropPosPos = rStrm.Tell();
}

// ----------------------------------------------------------------------------

ErrCode SfxOlePropertySet::LoadPropertySet( SotStorage* pStrg, const OUString& rStrmName )
{
    if( pStrg )
    {
        SotStorageStreamRef xStrm = pStrg->OpenSotStream( rStrmName, STREAM_STD_READ );
        if( xStrm.Is() && (xStrm->GetError() == SVSTREAM_OK) )
        {
            xStrm->SetBufferSize( STREAM_BUFFER_SIZE );
            Load( *xStrm );
        }
        else
            SetError( ERRCODE_IO_ACCESSDENIED );
    }
    else
        SetError( ERRCODE_IO_ACCESSDENIED );
    return GetError();
}

ErrCode SfxOlePropertySet::SavePropertySet( SotStorage* pStrg, const OUString& rStrmName )
{
    if( pStrg )
    {
        SotStorageStreamRef xStrm = pStrg->OpenSotStream( rStrmName, STREAM_TRUNC | STREAM_STD_WRITE );
        if( xStrm.Is() )
            Save( *xStrm );
        else
            SetError( ERRCODE_IO_ACCESSDENIED );
    }
    else
        SetError( ERRCODE_IO_ACCESSDENIED );
    return GetError();
}

SfxOleSectionRef SfxOlePropertySet::GetSection( SfxOleSectionType eSection ) const
{
    return GetSection( GetSectionGuid( eSection ) );
}

SfxOleSectionRef SfxOlePropertySet::GetSection( const SvGlobalName& rSectionGuid ) const
{
    SfxOleSectionRef xSection;
    SfxOleSectionMap::const_iterator aIt = maSectionMap.find( rSectionGuid );
    if( aIt != maSectionMap.end() )
        xSection = aIt->second;
    return xSection;
}

SfxOleSection& SfxOlePropertySet::AddSection( SfxOleSectionType eSection )
{
    return AddSection( GetSectionGuid( eSection ) );
}

SfxOleSection& SfxOlePropertySet::AddSection( const SvGlobalName& rSectionGuid )
{
    SfxOleSectionRef xSection = GetSection( rSectionGuid );
    if( !xSection )
    {
        // #i66214# #i66428# applications may write broken dictionary properties in wrong sections
        bool bSupportsDict = rSectionGuid == GetSectionGuid( SECTION_CUSTOM );
        xSection.reset( new SfxOleSection( bSupportsDict ) );
        maSectionMap[ rSectionGuid ] = xSection;
    }
    return *xSection;
}

void SfxOlePropertySet::ImplLoad( SvStream& rStrm )
{
    // read property set header
    sal_uInt16 nByteOrder;
    sal_uInt16 nVersion;
    sal_uInt16 nOsMinor;
    sal_uInt16 nOsType;
    SvGlobalName aGuid;
    sal_Int32 nSectCount(0);
    rStrm >> nByteOrder >> nVersion >> nOsMinor >> nOsType >> aGuid >> nSectCount;

    // read sections
    sal_Size nSectPosPos = rStrm.Tell();
    for( sal_Int32 nSectIdx = 0; (nSectIdx < nSectCount) && (rStrm.GetErrorCode() == SVSTREAM_OK) && !rStrm.IsEof(); ++nSectIdx )
    {
        // read section guid/position pair
        rStrm.Seek( nSectPosPos );
        SvGlobalName aSectGuid;
        sal_uInt32 nSectPos;
        rStrm >> aSectGuid >> nSectPos;
        nSectPosPos = rStrm.Tell();
        // read section
        rStrm.Seek( static_cast< sal_Size >( nSectPos ) );
        if( rStrm.GetErrorCode() == SVSTREAM_OK )
            LoadObject( rStrm, AddSection( aSectGuid ) );
    }
}

void SfxOlePropertySet::ImplSave( SvStream& rStrm )
{
    // write property set header
    SvGlobalName aGuid;
    sal_Int32 nSectCount = static_cast< sal_Int32 >( maSectionMap.size() );
    rStrm   << sal_uInt16( 0xFFFE )     // byte order
            << sal_uInt16( 0 )          // version
            << sal_uInt16( 1 )          // OS minor version
            << sal_uInt16( 2 )          // OS type always windows for text encoding
            << aGuid                    // unused guid
            << nSectCount;              // number of sections

    // write placeholders for section guid/position pairs
    sal_Size nSectPosPos = rStrm.Tell();
    rStrm.SeekRel( static_cast< sal_sSize >( 20 * nSectCount ) );

    // write sections
    for( SfxOleSectionMap::const_iterator aIt = maSectionMap.begin(), aEnd = maSectionMap.end(); aIt != aEnd; ++aIt )
    {
        SfxOleSection& rSection = *aIt->second;
        rStrm.Seek( STREAM_SEEK_TO_END );
        sal_uInt32 nSectPos = static_cast< sal_uInt32 >( rStrm.Tell() );
        // write the section
        SaveObject( rStrm, rSection );
        // write section guid/position pair
        rStrm.Seek( nSectPosPos );
        rStrm << aIt->first << nSectPos;
        nSectPosPos = rStrm.Tell();
    }
}

const SvGlobalName& SfxOlePropertySet::GetSectionGuid( SfxOleSectionType eSection )
{
    static const SvGlobalName saGlobalGuid(  0xF29F85E0, 0x4FF9, 0x1068, 0xAB, 0x91, 0x08, 0x00, 0x2B, 0x27, 0xB3, 0xD9 );
    static const SvGlobalName saBuiltInGuid( 0xD5CDD502, 0x2E9C, 0x101B, 0x93, 0x97, 0x08, 0x00, 0x2B, 0x2C, 0xF9, 0xAE );
    static const SvGlobalName saCustomGuid(  0xD5CDD505, 0x2E9C, 0x101B, 0x93, 0x97, 0x08, 0x00, 0x2B, 0x2C, 0xF9, 0xAE );
    static const SvGlobalName saEmptyGuid;
    switch( eSection )
    {
        case SECTION_GLOBAL:    return saGlobalGuid;
        case SECTION_BUILTIN:   return saBuiltInGuid;
        case SECTION_CUSTOM:    return saCustomGuid;
        default:    SAL_WARN( "sfx.doc", "SfxOlePropertySet::GetSectionGuid - unknown section type" );
    }
    return saEmptyGuid;
}

// ============================================================================

//} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

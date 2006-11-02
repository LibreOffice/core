/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: docinf.cxx,v $
 *
 *  $Revision: 1.44 $
 *
 *  last change: $Author: mav $ $Date: 2006-11-02 15:22:16 $
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
#include "precompiled_sfx2.hxx"

#include <map>
#include <boost/shared_ptr.hpp>

#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif

#ifndef _SFXECODE_HXX
#include <svtools/sfxecode.hxx>
#endif
#include <tools/urlobj.hxx>
#include <svtools/saveopt.hxx>
#include <tools/tenccvt.hxx>
#include <svtools/useroptions.hxx>
#include <sot/exchange.hxx>
#include <sot/storage.hxx>
#include <vcl/bitmapex.hxx>
#include "rtl/tencinfo.h"
#include <unotools/localfilehelper.hxx>

#include "docfilt.hxx"
#include "fcontnr.hxx"
#include "openflag.hxx"
#include "app.hxx"
#include "docinf.hxx"
#include "docfile.hxx"
#include "graphhelp.hxx"
#include "sfxtypes.hxx"
#include "appdata.hxx"
#include "doctempl.hxx"

// ============================================================================

namespace {

// ============================================================================

static const char __FAR_DATA pDocInfoSlot[] = "SfxDocumentInfo";
static const char __FAR_DATA pDocInfoHeader[] = "SfxDocumentInfo";

#define VERSION 11
#define STREAM_BUFFER_SIZE 2048

// stream names
#define STREAM_SUMMARYINFO      "\005SummaryInformation"
#define STREAM_DOCSUMMARYINFO   "\005DocumentSummaryInformation"

// property type IDs
const sal_Int32 PROPTYPE_INT16          = 2;
const sal_Int32 PROPTYPE_INT32          = 3;
const sal_Int32 PROPTYPE_FLOAT          = 4;
const sal_Int32 PROPTYPE_DOUBLE         = 5;
const sal_Int32 PROPTYPE_DATE           = 7;
const sal_Int32 PROPTYPE_STRING         = 8;
const sal_Int32 PROPTYPE_STATUS         = 10;
const sal_Int32 PROPTYPE_BOOL           = 11;
const sal_Int32 PROPTYPE_VARIANT        = 12;
const sal_Int32 PROPTYPE_INT8           = 16;
const sal_Int32 PROPTYPE_UINT8          = 17;
const sal_Int32 PROPTYPE_UINT16         = 18;
const sal_Int32 PROPTYPE_UINT32         = 19;
const sal_Int32 PROPTYPE_INT64          = 20;
const sal_Int32 PROPTYPE_UINT64         = 21;
const sal_Int32 PROPTYPE_STRING8        = 30;
const sal_Int32 PROPTYPE_STRING16       = 31;
const sal_Int32 PROPTYPE_FILETIME       = 64;
const sal_Int32 PROPTYPE_CLIPFMT        = 71;

// static property IDs
const sal_Int32 PROPID_DICTIONARY       = 0;
const sal_Int32 PROPID_CODEPAGE         = 1;
const sal_Int32 PROPID_FIRSTCUSTOM      = 2;

// property IDs for GlobalDocPropertySet
const sal_Int32 PROPID_TITLE            = 2;
const sal_Int32 PROPID_SUBJECT          = 3;
const sal_Int32 PROPID_AUTHOR           = 4;
const sal_Int32 PROPID_KEYWORDS         = 5;
const sal_Int32 PROPID_COMMENTS         = 6;
const sal_Int32 PROPID_TEMPLATE         = 7;
const sal_Int32 PROPID_LASTAUTHOR       = 8;
const sal_Int32 PROPID_REVNUMBER        = 9;
const sal_Int32 PROPID_EDITTIME         = 10;
const sal_Int32 PROPID_LASTPRINTED      = 11;
const sal_Int32 PROPID_CREATED          = 12;
const sal_Int32 PROPID_LASTSAVED        = 13;
const sal_Int32 PROPID_THUMBNAIL        = 17;

// predefined codepages
const sal_uInt16 CODEPAGE_UNKNOWN       = 0;
const sal_uInt16 CODEPAGE_UNICODE       = 1200;
const sal_uInt16 CODEPAGE_UTF8          = 65001;

// predefined clipboard format IDs
const sal_Int32 CLIPFMT_WIN             = -1;

// predefined clipboard data format IDs
const sal_Int32 CLIPDATAFMT_DIB         = 8;

// usings
using ::rtl::OUString;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::makeAny;

// ============================================================================
// ============================================================================

/** Helper for classes that need text encoding settings.

    Classes derived from this class will include functions to store and use
    text encoding settings and to convert Windows codepage constants.
 */
class SfxOleTextEncoding
{
public:
    inline explicit     SfxOleTextEncoding() :
                            mxTextEnc( new rtl_TextEncoding( gsl_getSystemTextEncoding() ) ) {}
    inline explicit     SfxOleTextEncoding( rtl_TextEncoding eTextEnc ) :
                            mxTextEnc( new rtl_TextEncoding( eTextEnc ) ) {}
    inline explicit     SfxOleTextEncoding( sal_Int16 nCodePage ) :
                            mxTextEnc( new rtl_TextEncoding ) { SetCodePage( nCodePage ); }

    /** Returns the current text encoding identifier. */
    inline rtl_TextEncoding GetTextEncoding() const { return *mxTextEnc; }
    /** Sets the passed text encoding. */
    inline void         SetTextEncoding( rtl_TextEncoding eTextEnc ) { *mxTextEnc = eTextEnc; }

    /** Returns true, if this object contains Unicode text encoding. */
    inline bool         IsUnicode() const { return GetTextEncoding() == RTL_TEXTENCODING_UCS2; }
    /** Sets Unicode text encoding to this object. */
    inline void         SetUnicode() { SetTextEncoding( RTL_TEXTENCODING_UCS2 ); }

    /** Converts the current settings to a Windows codepage identifier. */
    sal_uInt16          GetCodePage() const;
    /** Sets the current text encoding from a Windows codepage identifier. */
    void                SetCodePage( sal_uInt16 nCodePage );

private:
    typedef ::boost::shared_ptr< rtl_TextEncoding > TextEncRef;
    TextEncRef          mxTextEnc;
};

// ----------------------------------------------------------------------------

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

// ============================================================================

/** Helper for classes that need to load or save string values.

    Classes derived from this class contain functions to load and save string
    values with the text encoding passed in the constructor.
 */
class SfxOleStringHelper : public SfxOleTextEncoding
{
public:
    /** Creates a string helper object depending on an external text encoding. */
    inline explicit     SfxOleStringHelper( const SfxOleTextEncoding& rTextEnc ) :
                            SfxOleTextEncoding( rTextEnc ) {}
    /** Creates a string helper object with own text encoding. */
    inline explicit     SfxOleStringHelper( rtl_TextEncoding eTextEnc ) :
                            SfxOleTextEncoding( eTextEnc ) {}

    /** Loads a string from the passed stream with current encoding (maybe Unicode). */
    String              LoadString8( SvStream& rStrm ) const;
    /** Saves a string to the passed stream with current encoding (maybe Unicode). */
    void                SaveString8( SvStream& rStrm, const String& rValue ) const;

    /** Loads a Unicode string from the passed stream, ignores own encoding. */
    String              LoadString16( SvStream& rStrm ) const;
    /** Saves a Unicode string to the passed stream, ignores own encoding. */
    void                SaveString16( SvStream& rStrm, const String& rValue ) const;

private:
    String              ImplLoadString8( SvStream& rStrm ) const;
    String              ImplLoadString16( SvStream& rStrm ) const;
    void                ImplSaveString8( SvStream& rStrm, const String& rValue ) const;
    void                ImplSaveString16( SvStream& rStrm, const String& rValue ) const;
};

// ----------------------------------------------------------------------------

String SfxOleStringHelper::LoadString8( SvStream& rStrm ) const
{
    return IsUnicode() ? ImplLoadString16( rStrm ) : ImplLoadString8( rStrm );
}

void SfxOleStringHelper::SaveString8( SvStream& rStrm, const String& rValue ) const
{
    if( IsUnicode() )
        ImplSaveString16( rStrm, rValue );
    else
        ImplSaveString8( rStrm, rValue );
}

String SfxOleStringHelper::LoadString16( SvStream& rStrm ) const
{
    return ImplLoadString16( rStrm );
}

void SfxOleStringHelper::SaveString16( SvStream& rStrm, const String& rValue ) const
{
    ImplSaveString16( rStrm, rValue );
}

String SfxOleStringHelper::ImplLoadString8( SvStream& rStrm ) const
{
    String aValue;
    // read size field (signed 32-bit)
    sal_Int32 nSize;
    rStrm >> nSize;
    // size field includes trailing NUL character
    DBG_ASSERT( (0 < nSize) && (nSize <= 0xFFFF), "SfxOleStringHelper::ImplLoadString8 - invalid string" );
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

String SfxOleStringHelper::ImplLoadString16( SvStream& rStrm ) const
{
    String aValue;
    // read size field (signed 32-bit), may be buffer size or character count
    sal_Int32 nSize;
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
        aValue = String( &aBuffer.front() );
    }
    return aValue;
}

void SfxOleStringHelper::ImplSaveString8( SvStream& rStrm, const String& rValue ) const
{
    // encode to byte string
    ByteString aEncoded( rValue, GetTextEncoding() );
    // write size field (including trailing NUL character)
    sal_Int32 nSize = static_cast< sal_Int32 >( aEncoded.Len() + 1 );
    rStrm << nSize;
    // write character array with trailing NUL character
    rStrm.Write( aEncoded.GetBuffer(), aEncoded.Len() );
    rStrm << sal_uInt8( 0 );
}

void SfxOleStringHelper::ImplSaveString16( SvStream& rStrm, const String& rValue ) const
{
    // write size field (including trailing NUL character)
    sal_Int32 nSize = static_cast< sal_Int32 >( rValue.Len() + 1 );
    rStrm << nSize;
    // write character array with trailing NUL character
    for( xub_StrLen nIdx = 0; nIdx < rValue.Len(); ++nIdx )
        rStrm << static_cast< sal_uInt16 >( rValue.GetChar( nIdx ) );
    rStrm << sal_uInt16( 0 );
    // stream is always padded to 32-bit boundary, add 2 bytes on odd character count
    if( (nSize & 1) == 1 )
        rStrm << sal_uInt16( 0 );
}

// ============================================================================
// ============================================================================

/** Base class for all classes related to OLE property sets.

    Derived calsses have to implement the pure virtual functions ImplLoad() and
    ImplSave().
 */
class SfxOleObjectBase
{
public:
    inline explicit     SfxOleObjectBase() : mnErrCode( ERRCODE_NONE ) {}
    virtual             ~SfxOleObjectBase();

    /** Returns true, if an error code (other than ERRCODE_NONE) is set. */
    inline bool         HasError() const { return mnErrCode != ERRCODE_NONE; }
    /** Returns the current error code. */
    inline ErrCode      GetError() const { return mnErrCode; }

    /** Loads this object from the passed stream. Calls virtual ImplLoad(). */
    ErrCode             Load( SvStream& rStrm );
    /** Saves this object to the passed stream. Calls virtual ImplSave(). */
    ErrCode             Save( SvStream& rStrm );

protected:
    /** Sets the passed error code. Will be returned by Load() and Save() functions.
        Always the first error code is stored. Multiple calls have no effect. */
    inline void         SetError( ErrCode nErrCode ) { if( !HasError() ) mnErrCode = nErrCode; }
    /** Loads the passed object from the stream. Sets returned error code as own error. */
    void                LoadObject( SvStream& rStrm, SfxOleObjectBase& rObj );
    /** Saves the passed object to the stream. Sets returned error code as own error. */
    void                SaveObject( SvStream& rStrm, SfxOleObjectBase& rObj );

private:
    /** Derived classes implement loading the object from the passed steam. */
    virtual void        ImplLoad( SvStream& rStrm ) = 0;
    /** Derived classes implement saving the object to the passed steam. */
    virtual void        ImplSave( SvStream& rStrm ) = 0;

private:
    ErrCode             mnErrCode;      /// Current error code.
};

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

// ============================================================================
// ============================================================================

/** Base class for all OLE property objects. */
class SfxOlePropertyBase : public SfxOleObjectBase
{
public:
    inline explicit     SfxOlePropertyBase( sal_Int32 nPropId, sal_Int32 nPropType ) :
                            mnPropId( nPropId ), mnPropType( nPropType ) {}

    inline sal_Int32    GetPropId() const { return mnPropId; }
    inline sal_Int32    GetPropType() const { return mnPropType; }

protected:
    inline void         SetPropId( sal_Int32 nPropId ) { mnPropId = nPropId; }
    inline void         SetPropType( sal_Int32 nPropType ) { mnPropType = nPropType; }

private:
    sal_Int32           mnPropId;
    sal_Int32           mnPropType;
};

typedef ::boost::shared_ptr< SfxOlePropertyBase > SfxOlePropertyRef;

// ============================================================================

/** Property representing the codepage used to encode bytestrings in the entire property set. */
class SfxOleCodePageProperty : public SfxOlePropertyBase, public SfxOleTextEncoding
{
public:
    explicit            SfxOleCodePageProperty();

private:
    virtual void        ImplLoad( SvStream& rStrm );
    virtual void        ImplSave( SvStream& rStrm );
};

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

// ----------------------------------------------------------------------------

SfxOleBoolProperty::SfxOleBoolProperty( sal_Int32 nPropId, bool bValue ) :
    SfxOlePropertyBase( nPropId, PROPTYPE_BOOL ),
    mbValue( bValue )
{
}

void SfxOleBoolProperty::ImplLoad( SvStream& rStrm )
{
    sal_Int16 nValue;
    rStrm >> nValue;
    mbValue = nValue != 0;
}

void SfxOleBoolProperty::ImplSave( SvStream& rStrm )
{
    rStrm << static_cast< sal_Int16 >( mbValue ? -1 : 0 );
}

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
    explicit            SfxOleStringPropertyBase(
                            sal_Int32 nPropId, sal_Int32 nPropType,
                            rtl_TextEncoding eTextEnc, const String& rValue );

    inline const String& GetValue() const { return maValue; }
    inline void         SetValue( const String& rValue ) { maValue = rValue; }

private:
    String              maValue;
};

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

SfxOleStringPropertyBase::SfxOleStringPropertyBase(
        sal_Int32 nPropId, sal_Int32 nPropType, rtl_TextEncoding eTextEnc, const String& rValue ) :
    SfxOlePropertyBase( nPropId, nPropType ),
    SfxOleStringHelper( eTextEnc ),
    maValue( rValue )
{
}

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

// ============================================================================

/** Property representing a Unicode string value. */
class SfxOleString16Property : public SfxOleStringPropertyBase
{
public:
    explicit            SfxOleString16Property( sal_Int32 nPropId );
    explicit            SfxOleString16Property( sal_Int32 nPropId, const String& rValue );

private:
    virtual void        ImplLoad( SvStream& rStrm );
    virtual void        ImplSave( SvStream& rStrm );
};

// ----------------------------------------------------------------------------

SfxOleString16Property::SfxOleString16Property( sal_Int32 nPropId ) :
    SfxOleStringPropertyBase( nPropId, PROPTYPE_STRING16, RTL_TEXTENCODING_UCS2 )
{
}

SfxOleString16Property::SfxOleString16Property( sal_Int32 nPropId, const String& rValue ) :
    SfxOleStringPropertyBase( nPropId, PROPTYPE_STRING16, RTL_TEXTENCODING_UCS2, rValue )
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

// ============================================================================

/** Property representing a filetime value as defined by the Windows API. */
class SfxOleFileTimeProperty : public SfxOlePropertyBase
{
public:
    explicit            SfxOleFileTimeProperty( sal_Int32 nPropId );
    /** @param rDateTime  Date and time as LOCAL time. */
    explicit            SfxOleFileTimeProperty( sal_Int32 nPropId, const DateTime& rDateTime );

    /** Returns the time value as LOCAL time. */
    inline const DateTime& GetValue() const { return maDateTime; }
    /** @param rDateTime  Date and time as LOCAL time. */
    inline void         SetValue( const DateTime& rDateTime ) { maDateTime = rDateTime; }

private:
    virtual void        ImplLoad( SvStream& rStrm );
    virtual void        ImplSave( SvStream& rStrm );

private:
    DateTime            maDateTime;
};

// ----------------------------------------------------------------------------

SfxOleFileTimeProperty::SfxOleFileTimeProperty( sal_Int32 nPropId ) :
    SfxOlePropertyBase( nPropId, PROPTYPE_FILETIME )
{
}

SfxOleFileTimeProperty::SfxOleFileTimeProperty( sal_Int32 nPropId, const DateTime& rDateTime ) :
    SfxOlePropertyBase( nPropId, PROPTYPE_FILETIME ),
    maDateTime( rDateTime )
{
}

void SfxOleFileTimeProperty::ImplLoad( SvStream& rStrm )
{
    sal_uInt32 nLower, nUpper;
    rStrm >> nLower >> nUpper;
    maDateTime = DateTime::CreateFromWin32FileDateTime( nLower, nUpper );
    maDateTime.ConvertToLocalTime();
}

void SfxOleFileTimeProperty::ImplSave( SvStream& rStrm )
{
    DateTime aDateTimeUtc( maDateTime );
    // invalid time stamp is not converted to UTC
    if( aDateTimeUtc.IsValid() )
        aDateTimeUtc.ConvertToUTC();
    sal_uInt32 nLower, nUpper;
    aDateTimeUtc.GetWin32FileDateTime( nLower, nUpper );
    rStrm << nLower << nUpper;
}

// ============================================================================

/** Property representing a thumbnail picture.

    Currently, only saving this property is implemented.
 */
class SfxOleThumbnailProperty : public SfxOlePropertyBase
{
public:
    explicit            SfxOleThumbnailProperty( sal_Int32 nPropId, const GDIMetaFile& rMetaFile );

    inline bool         IsValid() const { return maBitmapData.GetSize() > 0; }

private:
    virtual void        ImplLoad( SvStream& rStrm );
    virtual void        ImplSave( SvStream& rStrm );

private:
    SvMemoryStream      maBitmapData;
};

// ----------------------------------------------------------------------------

SfxOleThumbnailProperty::SfxOleThumbnailProperty(
        sal_Int32 nPropId, const GDIMetaFile& rMetaFile ) :
    SfxOlePropertyBase( nPropId, PROPTYPE_CLIPFMT )
{
    BitmapEx aBitmap;
    // magic value 160 taken from GraphicHelper::getThumbnailFormatFromGDI_Impl()
    if( rMetaFile.CreateThumbnail( 160, aBitmap ) )
        aBitmap.GetBitmap().Write( maBitmapData, FALSE, FALSE );
}

void SfxOleThumbnailProperty::ImplLoad( SvStream& )
{
    DBG_ERRORFILE( "SfxOleThumbnailProperty::ImplLoad - not implemented" );
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
        // clibboard size: clip_format_tag + data_format_tag + bitmap_len
        sal_Int32 nClipSize = static_cast< sal_Int32 >( 4 + 4 + maBitmapData.GetSize() );
        rStrm << nClipSize << CLIPFMT_WIN << CLIPDATAFMT_DIB;
        rStrm.Write( maBitmapData.GetData(), maBitmapData.GetSize() );
    }
    else
    {
        DBG_ERRORFILE( "SfxOleThumbnailProperty::ImplSave - invalid thumbnail property" );
        SetError( SVSTREAM_INVALID_ACCESS );
    }
}

// ============================================================================
// ============================================================================

/** Property containing custom names for other properties in the property set. */
class SfxOleDictionaryProperty : public SfxOlePropertyBase, public SfxOleStringHelper
{
public:
    explicit            SfxOleDictionaryProperty( const SfxOleTextEncoding& rTextEnc );

    /** Returns true, if the property contains at least one custom property name. */
    inline bool         HasPropertyNames() const { return !maPropNameMap.empty(); }
    /** Prepares the property for loading. Does not affect contained names for its own. */
    inline void         SetNameCount( sal_Int32 nNameCount ) { SetPropType( nNameCount ); }

    /** Returns the custom name for the passed property ID, or an empty string, if name not found. */
    const String&       GetPropertyName( sal_Int32 nPropId ) const;
    /** Sets a custom name for the passed property ID. */
    void                SetPropertyName( sal_Int32 nPropId, const String& rPropName );

private:
    virtual void        ImplLoad( SvStream& rStrm );
    virtual void        ImplSave( SvStream& rStrm );

private:
    typedef ::std::map< sal_Int32, String > SfxOlePropNameMap;
    SfxOlePropNameMap   maPropNameMap;
};

// ----------------------------------------------------------------------------

SfxOleDictionaryProperty::SfxOleDictionaryProperty( const SfxOleTextEncoding& rTextEnc ) :
    SfxOlePropertyBase( PROPID_DICTIONARY, 0 ),
    SfxOleStringHelper( rTextEnc )
{
}

const String& SfxOleDictionaryProperty::GetPropertyName( sal_Int32 nPropId ) const
{
    SfxOlePropNameMap::const_iterator aIt = maPropNameMap.find( nPropId );
    return (aIt == maPropNameMap.end()) ? String::EmptyString() : aIt->second;
}

void SfxOleDictionaryProperty::SetPropertyName( sal_Int32 nPropId, const String& rPropName )
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
    for( sal_Int32 nIdx = 0; (nIdx < nNameCount) && (rStrm.GetErrorCode() == SVSTREAM_OK); ++nIdx )
    {
        sal_Int32 nPropId;
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

// ============================================================================
// ============================================================================

/** A section in a property set. Contains properties with unique identifiers. */
class SfxOleSection : public SfxOleObjectBase
{
private:
    typedef ::std::map< sal_Int32, SfxOlePropertyRef > SfxOlePropMap;

public:
    explicit            SfxOleSection( bool bSupportsDict );

    /** Returns the property with the passed ID, or an empty reference, if nothing found. */
    SfxOlePropertyRef   GetProperty( sal_Int32 nPropId ) const;
    /** Returns the value of a signed int32 property with the passed ID in rnValue.
        @return  true = Property found, rnValue is valid; false = Property not found. */
    bool                GetInt32Value( sal_Int32& rnValue, sal_Int32 nPropId ) const;
    /** Returns the value of a boolean property with the passed ID in rbValue.
        @return  true = Property found, rbValue is valid; false = Property not found. */
    bool                GetBoolValue( bool& rbValue, sal_Int32 nPropId ) const;
    /** Returns the value of a string property with the passed ID in rValue.
        @param nMaxLen  If specified, cuts the string returned in rValue to this length.
        @return  true = Property found, rValue is valid; false = Property not found. */
    bool                GetStringValue( String& rValue, sal_Int32 nPropId, xub_StrLen nMaxLen = STRING_LEN ) const;
    /** Returns the value of a time stamp property with the passed ID in rValue.
        @return  true = Property found, rValue is valid; false = Property not found. */
    bool                GetFileTimeValue( DateTime& rValue, sal_Int32 nPropId ) const;

    /** Adds the passed property to the property set. Drops an existing old property. */
    void                SetProperty( SfxOlePropertyRef xProp );
    /** Inserts a signed int32 property with the passed value. */
    void                SetInt32Value( sal_Int32 nPropId, sal_Int32 nValue );
    /** Inserts a boolean property with the passed value. */
    void                SetBoolValue( sal_Int32 nPropId, bool bValue );
    /** Inserts a string property with the passed value.
        @return  true = Property inserted; false = String was empty, property not inserted. */
    bool                SetStringValue( sal_Int32 nPropId, const String& rValue, bool bSkipEmpty = true );
    /** Inserts a time stamp property with the passed value. */
    void                SetFileTimeValue( sal_Int32 nPropId, const DateTime& rValue );
    /** Inserts a thumbnail property from the passed meta file. */
    void                SetThumbnailValue( sal_Int32 nPropId, const GDIMetaFile& rMetaFile );

    /** Returns the value of the property with the passed ID in a UNO any. */
    Any                 GetAnyValue( sal_Int32 nPropId ) const;
    /** Inserts a property created from the passed any.
        @return  true = Property converted and inserted; false = Property type not supported. */
    bool                SetAnyValue( sal_Int32 nPropId, const Any& rValue );

    /** Returns the custom name for the passed property ID, or an empty string, if name not found. */
    const String&       GetPropertyName( sal_Int32 nPropId ) const;
    /** Sets a custom name for the passed property ID. */
    void                SetPropertyName( sal_Int32 nPropId, const String& rPropName );

    /** Returns the identifiers of all existing properties in the passed vector. */
    void                GetPropertyIds( ::std::vector< sal_Int32 >& rPropIds ) const;
    /** Returns a property identifier not used in this section. */
    sal_Int32           GetFreePropertyId() const;

private:
    virtual void        ImplLoad( SvStream& rStrm );
    virtual void        ImplSave( SvStream& rStrm );

    bool                SeekToPropertyPos( SvStream& rStrm, sal_uInt32 nPropPos ) const;
    void                LoadProperty( SvStream& rStrm, sal_Int32 nPropId );
    void                SaveProperty( SvStream& rStrm, SfxOlePropertyBase& rProp, sal_Size& rnPropPosPos );

private:
    SfxOlePropMap       maPropMap;              /// All properties in this section, by identifier.
    SfxOleCodePageProperty maCodePageProp;      /// The codepage property.
    SfxOleDictionaryProperty maDictProp;        /// The dictionary property.
    sal_Size            mnStartPos;             /// Start stream position of the section.
    bool                mbSupportsDict;         /// true = section supports dictionary.
};

typedef ::boost::shared_ptr< SfxOleSection > SfxOleSectionRef;

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

bool SfxOleSection::GetBoolValue( bool& rbValue, sal_Int32 nPropId ) const
{
    SfxOlePropertyRef xProp = GetProperty( nPropId );
    const SfxOleBoolProperty* pProp =
        dynamic_cast< const SfxOleBoolProperty* >( xProp.get() );
    if( pProp )
        rbValue = pProp->GetValue();
    return pProp != 0;
}

bool SfxOleSection::GetStringValue( String& rValue, sal_Int32 nPropId, xub_StrLen nMaxLen ) const
{
    SfxOlePropertyRef xProp = GetProperty( nPropId );
    const SfxOleStringPropertyBase* pProp =
        dynamic_cast< const SfxOleStringPropertyBase* >( xProp.get() );
    if( pProp )
        rValue = pProp->GetValue().Copy( 0, nMaxLen );
    return pProp != 0;
}

bool SfxOleSection::GetFileTimeValue( DateTime& rValue, sal_Int32 nPropId ) const
{
    SfxOlePropertyRef xProp = GetProperty( nPropId );
    const SfxOleFileTimeProperty* pProp =
        dynamic_cast< const SfxOleFileTimeProperty* >( xProp.get() );
    if( pProp )
        rValue = pProp->GetValue();
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

void SfxOleSection::SetBoolValue( sal_Int32 nPropId, bool bValue )
{
    SetProperty( SfxOlePropertyRef( new SfxOleBoolProperty( nPropId, bValue ) ) );
}

bool SfxOleSection::SetStringValue( sal_Int32 nPropId, const String& rValue, bool bSkipEmpty )
{
    bool bInserted = !bSkipEmpty || (rValue.Len() > 0);
    if( bInserted )
        SetProperty( SfxOlePropertyRef( new SfxOleString8Property( nPropId, maCodePageProp, rValue ) ) );
    return bInserted;
}

void SfxOleSection::SetFileTimeValue( sal_Int32 nPropId, const DateTime& rValue )
{
    SetProperty( SfxOlePropertyRef( new SfxOleFileTimeProperty( nPropId, rValue ) ) );
}

void SfxOleSection::SetThumbnailValue( sal_Int32 nPropId, const GDIMetaFile& rMetaFile )
{
    SfxOleThumbnailProperty* pThumbnail = new SfxOleThumbnailProperty( nPropId, rMetaFile );
    SfxOlePropertyRef xProp( pThumbnail );  // take ownership
    if( pThumbnail->IsValid() )
        SetProperty( xProp );
}

Any SfxOleSection::GetAnyValue( sal_Int32 nPropId ) const
{
    Any aValue;
    sal_Int32 nInt32 = 0;
    bool bBool = false;
    String aString;
    DateTime aDateTime;

    if( GetInt32Value( nInt32, nPropId ) )
        aValue <<= nInt32;
    else if( GetBoolValue( bBool, nPropId ) )
        ::comphelper::setBOOL( aValue, bBool ? sal_True : sal_False );
    else if( GetStringValue( aString, nPropId ) )
        aValue <<= OUString( aString );
    else if( GetFileTimeValue( aDateTime, nPropId ) )
    {
        ::com::sun::star::util::DateTime aApiDateTime(
            static_cast< sal_uInt16 >( aDateTime.Get100Sec() ),
            static_cast< sal_uInt16 >( aDateTime.GetSec() ),
            static_cast< sal_uInt16 >( aDateTime.GetMin() ),
            static_cast< sal_uInt16 >( aDateTime.GetHour() ),
            static_cast< sal_uInt16 >( aDateTime.GetDay() ),
            static_cast< sal_uInt16 >( aDateTime.GetMonth() ),
            static_cast< sal_uInt16 >( aDateTime.GetYear() ) );
        aValue <<= aApiDateTime;
    }
    return aValue;
}

bool SfxOleSection::SetAnyValue( sal_Int32 nPropId, const Any& rValue )
{
    bool bInserted = true;
    sal_Int32 nInt32 = 0;
    OUString aString;
    ::com::sun::star::util::DateTime aApiDateTime;

    if( rValue >>= nInt32 )
        SetInt32Value( nPropId, nInt32 );
    else if( rValue.getValueType() == ::getBooleanCppuType() )
        SetBoolValue( nPropId, ::comphelper::getBOOL( rValue ) == sal_True );
    else if( rValue >>= aString )
        bInserted = SetStringValue( nPropId, aString );
    else if( rValue >>= aApiDateTime )
    {
        DateTime aDateTime(
            Date(
                static_cast< USHORT >( aApiDateTime.Day ),
                static_cast< USHORT >( aApiDateTime.Month ),
                static_cast< USHORT >( aApiDateTime.Year ) ),
            Time(
                static_cast< ULONG >( aApiDateTime.Hours ),
                static_cast< ULONG >( aApiDateTime.Minutes ),
                static_cast< ULONG >( aApiDateTime.Seconds ),
                static_cast< ULONG >( aApiDateTime.HundredthSeconds ) ) );
        SetFileTimeValue( nPropId, aDateTime );
    }
    else
        bInserted = false;
    return bInserted;
}

const String& SfxOleSection::GetPropertyName( sal_Int32 nPropId ) const
{
    return maDictProp.GetPropertyName( nPropId );
}

void SfxOleSection::SetPropertyName( sal_Int32 nPropId, const String& rPropName )
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
    sal_uInt32 nSize;
    sal_Int32 nPropCount;
    rStrm >> nSize >> nPropCount;

    // read property ID/position pairs
    typedef ::std::map< sal_Int32, sal_uInt32 > SfxOlePropPosMap;
    SfxOlePropPosMap aPropPosMap;
    for( sal_Int32 nPropIdx = 0; (nPropIdx < nPropCount) && (rStrm.GetErrorCode() == SVSTREAM_OK); ++nPropIdx )
    {
        sal_Int32 nPropId;
        sal_uInt32 nPropPos;
        rStrm >> nPropId >> nPropPos;
        aPropPosMap[ nPropId ] = nPropPos;
    }

    // read codepage property
    SfxOlePropPosMap::iterator aCodePageIt = aPropPosMap.find( PROPID_CODEPAGE );
    if( (aCodePageIt != aPropPosMap.end()) && SeekToPropertyPos( rStrm, aCodePageIt->second ) )
    {
        // codepage property must be of type signed int-16
        sal_Int32 nPropType;
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
            sal_Int32 nNameCount;
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
    sal_Int32 nPropType;
    rStrm >> nPropType;
    // create empty property object
    SfxOlePropertyRef xProp;
    switch( nPropType )
    {
        case PROPTYPE_INT32:
            xProp.reset( new SfxOleInt32Property( nPropId ) );
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

// ============================================================================
// ============================================================================

/** Enumerates different section types in OLE property sets. */
enum SfxOleSectionType
{
    SECTION_GLOBAL,         /// Globally defined properties.
    SECTION_BUILTIN,        /// Properties built into MS Office.
    SECTION_CUSTOM          /// Custom properties.
};

// ============================================================================

/** Represents a complete property set, may consist of several property sections. */
class SfxOlePropertySet : public SfxOleObjectBase
{
public:
    inline explicit     SfxOlePropertySet() {}

    /** Loads this object from the passed storage. */
    ErrCode             LoadPropertySet( SotStorage* pStrg, const String& rStrmName );
    /** Saves this object to the passed storage. */
    ErrCode             SavePropertySet( SotStorage* pStrg, const String& rStrmName );

    /** Returns the specified section, or an empty reference, if nothing found. */
    SfxOleSectionRef    GetSection( SfxOleSectionType eSection ) const;
    /** Returns the specified section, or an empty reference, if nothing found. */
    SfxOleSectionRef    GetSection( const SvGlobalName& rSectionGuid ) const;

    /** Creates and returns the specified section, or just returns it if it already exists. */
    SfxOleSection&      AddSection( SfxOleSectionType eSection );
    /** Creates and returns the specified section, or just returns it if it already exists. */
    SfxOleSection&      AddSection( const SvGlobalName& rSectionGuid );

private:
    virtual void        ImplLoad( SvStream& rStrm );
    virtual void        ImplSave( SvStream& rStrm );

    /** Returns the GUID for the specified section. */
    static const SvGlobalName& GetSectionGuid( SfxOleSectionType eSection );

private:
    typedef ::std::map< SvGlobalName, SfxOleSectionRef > SfxOleSectionMap;
    SfxOleSectionMap    maSectionMap;
};

// ----------------------------------------------------------------------------

ErrCode SfxOlePropertySet::LoadPropertySet( SotStorage* pStrg, const String& rStrmName )
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

ErrCode SfxOlePropertySet::SavePropertySet( SotStorage* pStrg, const String& rStrmName )
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
    sal_Int32 nSectCount;
    rStrm >> nByteOrder >> nVersion >> nOsMinor >> nOsType >> aGuid >> nSectCount;

    // read sections
    sal_Size nSectPosPos = rStrm.Tell();
    for( sal_Int32 nSectIdx = 0; (nSectIdx < nSectCount) && (rStrm.GetErrorCode() == SVSTREAM_OK); ++nSectIdx )
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
        default:    DBG_ERRORFILE( "SfxOlePropertySet::GetSectionGuid - unknown section type" );
    }
    return saEmptyGuid;
}

// ============================================================================

} // namespace

// ============================================================================

IMPL_PTRHINT(SfxDocumentInfoHint, SfxDocumentInfo)

// ============================================================================
// ============================================================================

class SfxDocumentInfo_Impl
{
public:
    String      aCopiesTo;
    String      aOriginal;
    String      aReferences;
    String      aRecipient;
    String      aReplyTo;
    String      aBlindCopies;
    String      aInReplyTo;
    String      aNewsgroups;
    String      aSpecialMimeType;
    USHORT      nPriority;
    BOOL        bUseUserData;
    // --> PB 2004-08-23 #i33095#
    sal_Bool    bLoadReadonly;
    GDIMetaFile aThumbnailMetaFile;
    TDynamicProps _lDynamicProps;

    SfxDocumentInfo_Impl() : nPriority( 0 ), bUseUserData( 1 ), bLoadReadonly( sal_False ) {}
};

//========================================================================


SvStream& PaddWithBlanks_Impl(SvStream &rStream, USHORT nCount)
{
    for ( USHORT n = nCount; n; --n )
        rStream << ' ';
    return rStream;
}

//-------------------------------------------------------------------------

inline SvStream& Skip(SvStream &rStream, USHORT nCount)
{
    rStream.SeekRel(nCount);
    return rStream;
}

//-------------------------------------------------------------------------

struct FileHeader
{
    String      aHeader;
    USHORT      nVersion;
    BOOL        bPasswd;

    FileHeader( const char* pHeader, USHORT nV, BOOL bPass ) :
        nVersion( nV ), bPasswd( bPass ) { aHeader = String::CreateFromAscii( pHeader ); }
    FileHeader( SvStream& rStream );

    void    Save( SvStream& rStream ) const;
};

//-------------------------------------------------------------------------


FileHeader::FileHeader( SvStream& rStream )
{
    BYTE b;
    long nVer = rStream.GetVersion();
    rStream.SetVersion( SOFFICE_FILEFORMAT_40 );
    rStream.ReadByteString( aHeader ); //(dv??)
    rStream >> nVersion >> b;
    rStream.SetVersion( nVer );
    bPasswd = (BOOL)b;
}

//-------------------------------------------------------------------------

void FileHeader::Save( SvStream& rStream ) const
{
    long nVer = rStream.GetVersion();
    rStream.SetVersion( SOFFICE_FILEFORMAT_40 );
    rStream.WriteByteString( aHeader );
    rStream.SetVersion( nVer );
    rStream << nVersion;
    rStream << (BYTE)bPasswd;
}

//-------------------------------------------------------------------------

void SfxDocUserKey::AdjustTitle_Impl()
{
    if ( aTitle.Len() > SFXDOCUSERKEY_LENMAX )
        aTitle.Erase( SFXDOCUSERKEY_LENMAX );
}

//-------------------------------------------------------------------------

BOOL SfxDocUserKey::Load(SvStream &rStream)
{
    rStream.ReadByteString( aTitle );
    Skip(rStream, SFXDOCUSERKEY_LENMAX - aTitle.Len());
    rStream.ReadByteString( aWord );
    Skip(rStream, SFXDOCUSERKEY_LENMAX - aWord.Len());
    return rStream.GetError() == SVSTREAM_OK;
}


//-------------------------------------------------------------------------

BOOL SfxDocUserKey::Save(SvStream &rStream) const
{
    DBG_ASSERT( aTitle.Len() <= SFXDOCUSERKEY_LENMAX, "length of user key title overflow" );
    DBG_ASSERT( aWord.Len() <= SFXDOCUSERKEY_LENMAX, "length of user key value overflow" );

    // save the title of the user key
    String aString = aTitle;
    aString.Erase( SFXDOCUSERKEY_LENMAX );
    rStream.WriteByteString( aString );
    PaddWithBlanks_Impl(rStream, SFXDOCUSERKEY_LENMAX - aString.Len());

    // save the value of the user key
    aString = aWord;
    aString.Erase( SFXDOCUSERKEY_LENMAX );
    rStream.WriteByteString( aString );
    PaddWithBlanks_Impl(rStream, SFXDOCUSERKEY_LENMAX - aString.Len());

    return rStream.GetError() == SVSTREAM_OK;
}

//-------------------------------------------------------------------------
SfxDocUserKey::SfxDocUserKey( const String& rTitle, const String& rWord ) :
        aTitle( rTitle ), aWord( rWord )
{
    //!AdjustTitle_Impl();
}
//------------------------------------------------------------------------
const SfxDocUserKey& SfxDocUserKey::operator=(const SfxDocUserKey &rCopy)
{
    aTitle = rCopy.aTitle;
    aWord = rCopy.aWord;
    //!AdjustTitle_Impl();
    return *this;
}
// SfxDocumentInfo -------------------------------------------------------

sal_Bool TestValidity_Impl( const String& rString, sal_Bool bURL )
{
    sal_Bool bRet = sal_True;
    xub_StrLen nLen = rString.Len();
    if ( nLen >= 1024 &&
         ( !bURL || INetURLObject::CompareProtocolScheme( rString ) == INET_PROT_NOT_VALID ) )
        // !bURL == the default target has not so many characters
        // bURL ==  the reload url must have a valid protocol
        bRet = sal_False;

    return bRet;
}

//------------------------------------------------------------------------

BOOL SfxDocumentInfo::LoadFromBinaryFormat( SvStream& rStream )
{
    long d, t;
    USHORT nUS;
    BYTE nByte;
    FileHeader aHeader(rStream);
    if( ! aHeader.aHeader.EqualsAscii( pDocInfoHeader ))
    {
        rStream.SetError(SVSTREAM_FILEFORMAT_ERROR);
        return FALSE;
    }
    Free();
    bPasswd = aHeader.bPasswd;
    rStream >> nUS;
    //eFileCharSet = (CharSet)nUS;
    eFileCharSet = GetSOLoadTextEncoding( nUS );

        // Einstellen an den Streams
    rStream.SetStreamCharSet(eFileCharSet);

    rStream >> nByte;
    bPortableGraphics = nByte? 1: 0;
    rStream >> nByte;
    bQueryTemplate = nByte? 1: 0;

    aCreated.Load(rStream);
    aChanged.Load(rStream);
    aPrinted.Load(rStream);

    rStream.ReadByteString( aTitle );
    Skip(rStream, SFXDOCINFO_TITLELENMAX - aTitle.Len());
    rStream.ReadByteString( aTheme );
    Skip(rStream, SFXDOCINFO_THEMELENMAX - aTheme.Len());
    rStream.ReadByteString( aComment );
    Skip(rStream, SFXDOCINFO_COMMENTLENMAX- aComment.Len());
    rStream.ReadByteString( aKeywords );
    Skip(rStream, SFXDOCINFO_KEYWORDLENMAX - aKeywords.Len());

    USHORT i;
    for(i = 0; i < MAXDOCUSERKEYS; ++i)
        aUserKeys[i].Load(rStream);

    rStream.ReadByteString( aTemplateName );
    rStream.ReadByteString( aTemplateFileName );
    rStream >> d >> t;
    aTemplateDate = DateTime(Date(d), Time(t));

    // wurde mal fuer MB in Panik eingebaut und dann doch nie benutzt :-)
    if ( rStream.GetVersion() <= SOFFICE_FILEFORMAT_40 )
    {
        USHORT nMailAddr;
        rStream >> nMailAddr;
        for( i = 0; i < nMailAddr; i++ )
        {
            String aDummyString;
            USHORT nDummyFlags;
            rStream.ReadByteString( aDummyString );
            rStream >> nDummyFlags;
        }
    }

    rStream >> lTime;
    if(aHeader.nVersion > 4)
        rStream >> nDocNo;
    else
        nDocNo = 1;
    rStream >> nUserDataSize;
    if(nUserDataSize) {
        pUserData = new char[nUserDataSize];
        rStream.Read(pUserData,nUserDataSize);
    }

    BOOL bOK = (rStream.GetError() == SVSTREAM_OK);
    nByte = 0;                          // wg.Kompatibilitaet;
    rStream >> nByte;                   // evtl. nicht in DocInfo enthalten
    bTemplateConfig = nByte ? 1 : 0;
    if( aHeader.nVersion > 5 )
    {
        rStream >> bReloadEnabled;
        rStream.ReadByteString( aReloadURL );
        rStream >> nReloadSecs;
        rStream.ReadByteString( aDefaultTarget );

        if ( !TestValidity_Impl( aReloadURL, sal_True ) )
        {
            // the reload url is invalid -> reset all reload attributes
            bReloadEnabled = FALSE;
            aReloadURL.Erase();
            nReloadSecs = 60;
            aDefaultTarget.Erase();
        }
        else if ( !TestValidity_Impl( aDefaultTarget, sal_False ) )
            // the default target is invalid -> reset it
            aDefaultTarget.Erase();
    }
    if ( aHeader.nVersion > 6 )
    {
        rStream >> nByte;
        bSaveGraphicsCompressed = nByte? 1: 0;
    }
    if ( aHeader.nVersion > 7 )
    {
        rStream >> nByte;
        bSaveOriginalGraphics = nByte? 1: 0;
    }
    if ( aHeader.nVersion > 8 )
    {
        rStream >> nByte;
        bSaveVersionOnClose = nByte? 1: 0;

        rStream.ReadByteString( pImp->aCopiesTo );
        rStream.ReadByteString( pImp->aOriginal );
        rStream.ReadByteString( pImp->aReferences );
        rStream.ReadByteString( pImp->aRecipient );
        rStream.ReadByteString( pImp->aReplyTo );
        rStream.ReadByteString( pImp->aBlindCopies );
        rStream.ReadByteString( pImp->aInReplyTo );
        rStream.ReadByteString( pImp->aNewsgroups );
        rStream >> pImp->nPriority;
    }
    if ( aHeader.nVersion > 9 )
    {
        rStream.ReadByteString( pImp->aSpecialMimeType );
    }
    if ( aHeader.nVersion > 10 )
    {
        rStream >> nByte;
        pImp->bUseUserData = nByte ? TRUE : FALSE;
    }
    return bOK;
}


//-------------------------------------------------------------------------

BOOL SfxDocumentInfo::SaveToBinaryFormat( SvStream& rStream ) const
{
    FileHeader aHeader(pDocInfoHeader, VERSION, bPasswd? 1: 0);
    aHeader.Save(rStream);
    CharSet eNewFileCharSet = GetSOStoreTextEncoding( eFileCharSet );
    rStream << (USHORT)eNewFileCharSet;
    rStream.SetStreamCharSet(eNewFileCharSet);
    rStream << (bPortableGraphics? (BYTE)1: (BYTE)0)
            << (bQueryTemplate? (BYTE)1: (BYTE)0);
    aCreated.Save(rStream);
    aChanged.Save(rStream);
    aPrinted.Save(rStream);

    DBG_ASSERT( aTitle.Len() <= SFXDOCINFO_TITLELENMAX , "length of title overflow" );
    DBG_ASSERT( aTheme.Len() <= SFXDOCINFO_THEMELENMAX , "length of theme overflow" );
    DBG_ASSERT( aComment.Len() <= SFXDOCINFO_COMMENTLENMAX , "length of description overflow" );
    DBG_ASSERT( aKeywords.Len() <= SFXDOCINFO_KEYWORDLENMAX , "length of keywords overflow" );

    // save the title
    String aString = aTitle;
    aString.Erase( SFXDOCINFO_TITLELENMAX );
    rStream.WriteByteString( aString );
    PaddWithBlanks_Impl(rStream, SFXDOCINFO_TITLELENMAX - aString.Len());
    // save the theme
    aString = aTheme;
    aString.Erase( SFXDOCINFO_THEMELENMAX );
    rStream.WriteByteString( aString );
    PaddWithBlanks_Impl(rStream, SFXDOCINFO_THEMELENMAX - aString.Len());
    // save the description
    aString = aComment;
    aString.Erase( SFXDOCINFO_COMMENTLENMAX );
    rStream.WriteByteString( aString );
    PaddWithBlanks_Impl(rStream, SFXDOCINFO_COMMENTLENMAX - aString.Len());
    // save the keywords
    aString = aKeywords;
    aString.Erase( SFXDOCINFO_KEYWORDLENMAX );
    rStream.WriteByteString( aString );
    PaddWithBlanks_Impl(rStream, SFXDOCINFO_KEYWORDLENMAX - aString.Len());

    for(USHORT i = 0; i < MAXDOCUSERKEYS; ++i)
        aUserKeys[i].Save(rStream);
    rStream.WriteByteString( aTemplateName );
    rStream.WriteByteString( aTemplateFileName );
    rStream << (long)aTemplateDate.GetDate()
            << (long)aTemplateDate.GetTime();

    // wurde mal fuer MB in Panik eingebaut und dann doch nie benutzt :-)
    if ( rStream.GetVersion() <= SOFFICE_FILEFORMAT_40 )
        rStream << (USHORT) 0;

    rStream << GetTime() << GetDocumentNumber();

    rStream << nUserDataSize;
    if(pUserData)
        rStream.Write(pUserData, nUserDataSize);
    rStream << (bTemplateConfig? (BYTE)1: (BYTE)0);
    if( aHeader.nVersion > 5 )
    {
        rStream << bReloadEnabled;
        rStream.WriteByteString( aReloadURL );
        rStream << nReloadSecs;
        rStream.WriteByteString( aDefaultTarget );
    }
    if ( aHeader.nVersion > 6 )
        rStream << (bSaveGraphicsCompressed? (BYTE)1: (BYTE)0);
    if ( aHeader.nVersion > 7 )
        rStream << (bSaveOriginalGraphics? (BYTE)1: (BYTE)0);
    if ( aHeader.nVersion > 8 )
    {
        rStream << (bSaveVersionOnClose? (BYTE)1: (BYTE)0);
        rStream.WriteByteString( pImp->aCopiesTo );
        rStream.WriteByteString( pImp->aOriginal );
        rStream.WriteByteString( pImp->aReferences );
        rStream.WriteByteString( pImp->aRecipient );
        rStream.WriteByteString( pImp->aReplyTo );
        rStream.WriteByteString( pImp->aBlindCopies );
        rStream.WriteByteString( pImp->aInReplyTo );
        rStream.WriteByteString( pImp->aNewsgroups );
        rStream << pImp->nPriority;
    }
    if ( aHeader.nVersion > 9 )
    {
        rStream.WriteByteString( pImp->aSpecialMimeType );
    }
    if ( aHeader.nVersion > 10 )
    {
        rStream << ( pImp->bUseUserData ? (BYTE)1: (BYTE)0 );
    }

    return rStream.GetError() == SVSTREAM_OK;
}

//-------------------------------------------------------------------------

sal_Bool SfxDocumentInfo::LoadFromBinaryFormat( SotStorage* pStorage )
{
    if(!pStorage->IsStream( String::CreateFromAscii( pDocInfoSlot )))
        return FALSE;

    if ( pStorage->GetVersion() >= SOFFICE_FILEFORMAT_60 )
    {
        DBG_ERROR("This method only supports binary file format, use service StandaloneDocumentInfo!");
        return FALSE;
    }

    SotStorageStreamRef rStr = pStorage->OpenSotStream( String::CreateFromAscii( pDocInfoSlot ),STREAM_STD_READ);
    if(!rStr.Is())
        return FALSE;
    rStr->SetVersion( pStorage->GetVersion() );
    rStr->SetBufferSize(STREAM_BUFFER_SIZE);
    BOOL bRet = LoadFromBinaryFormat(*rStr);
    if ( bRet )
    {
        String aStr = SotExchange::GetFormatMimeType( pStorage->GetFormat() );
        USHORT nPos = aStr.Search(';');
        if ( nPos != STRING_NOTFOUND )
            pImp->aSpecialMimeType = aStr.Copy( 0, nPos );
        else
            pImp->aSpecialMimeType = aStr;
    }

    return bRet;
}

//-------------------------------------------------------------------------

BOOL SfxDocumentInfo::SaveToBinaryFormat( SotStorage* pStorage ) const
{
    SotStorageStreamRef aStr = pStorage->OpenSotStream( String::CreateFromAscii( pDocInfoSlot ), STREAM_TRUNC | STREAM_STD_READWRITE);
    if(!aStr.Is())
        return FALSE;
    aStr->SetVersion( pStorage->GetVersion() );
    aStr->SetBufferSize(STREAM_BUFFER_SIZE);
    if(!SaveToBinaryFormat(*aStr))
        return FALSE;
    return SavePropertySet( pStorage );
}

//-------------------------------------------------------------------------

sal_uInt32 SfxDocumentInfo::LoadPropertySet( SotStorage* pStorage )
{
    // *** global properties from stream "005SummaryInformation" ***

    // load the property set
    SfxOlePropertySet aGlobSet;
    ErrCode nGlobError = aGlobSet.LoadPropertySet(
        pStorage, String( RTL_CONSTASCII_USTRINGPARAM( STREAM_SUMMARYINFO ) ) );

    // global section
    SfxOleSectionRef xGlobSect = aGlobSet.GetSection( SECTION_GLOBAL );
    if( xGlobSect.get() )
    {
        // set supported properties
        String aStrValue;
        DateTime aDateTime;

        if( xGlobSect->GetStringValue( aStrValue, PROPID_TITLE, SFXDOCINFO_TITLELENMAX ) )
            SetTitle( aStrValue );
        if( xGlobSect->GetStringValue( aStrValue, PROPID_SUBJECT, SFXDOCINFO_THEMELENMAX ) )
            SetTheme( aStrValue );
        if( xGlobSect->GetStringValue( aStrValue, PROPID_KEYWORDS, SFXDOCINFO_KEYWORDLENMAX ) )
            SetKeywords( aStrValue );
        if( xGlobSect->GetStringValue( aStrValue, PROPID_TEMPLATE ) )
            SetTemplateName( aStrValue );
        if( xGlobSect->GetStringValue( aStrValue, PROPID_COMMENTS, SFXDOCINFO_COMMENTLENMAX ) )
            SetComment( aStrValue );

        TimeStamp aCreatedStamp( TIMESTAMP_INVALID_DATETIME );
        if( xGlobSect->GetStringValue( aStrValue, PROPID_AUTHOR, TIMESTAMP_MAXLENGTH ) )
            aCreatedStamp.SetName( aStrValue );
        if( xGlobSect->GetFileTimeValue( aDateTime, PROPID_CREATED ) )
            aCreatedStamp.SetTime( aDateTime );
        SetCreated( aCreatedStamp );

        TimeStamp aLastStamp( TIMESTAMP_INVALID_DATETIME );
        if( xGlobSect->GetStringValue( aStrValue, PROPID_LASTAUTHOR, TIMESTAMP_MAXLENGTH ) )
            aLastStamp.SetName( aStrValue );
        if( xGlobSect->GetFileTimeValue( aDateTime, PROPID_LASTSAVED ) )
            aLastStamp.SetTime( aDateTime );
        SetChanged( aLastStamp );

        if( xGlobSect->GetFileTimeValue( aDateTime, PROPID_LASTPRINTED ) )
        {
            DateTime aUtcTime = aDateTime;
            aUtcTime.ConvertToUTC();
            if( aUtcTime == DateTime( Date( 1, 1, 1601 ) ) )
                aDateTime = TIMESTAMP_INVALID_DATETIME;
        }
        else
            aDateTime = DateTime();
        SetPrinted( SfxStamp( String(), aDateTime ) );
        if( xGlobSect->GetStringValue( aStrValue, PROPID_REVNUMBER ) )
            SetDocumentNumber( static_cast< USHORT >( aStrValue.ToInt32() ) );
        if( xGlobSect->GetFileTimeValue( aDateTime, PROPID_EDITTIME ) )
        {
            aDateTime.ConvertToUTC();
            SetTime( aDateTime.GetTime() );
        }
    }

    // *** custom properties from stream "005DocumentSummaryInformation" ***

    // load the property set
    SfxOlePropertySet aDocSet;
    ErrCode nDocError = aDocSet.LoadPropertySet(
        pStorage, String( RTL_CONSTASCII_USTRINGPARAM( STREAM_DOCSUMMARYINFO ) ) );

    // custom properties
    SfxOleSectionRef xCustomSect = aDocSet.GetSection( SECTION_CUSTOM );
    if( xCustomSect.get() )
    {
        ::std::vector< sal_Int32 > aPropIds;
        xCustomSect->GetPropertyIds( aPropIds );
        for( ::std::vector< sal_Int32 >::const_iterator aIt = aPropIds.begin(), aEnd = aPropIds.end(); aIt != aEnd; ++aIt )
        {
            OUString aPropName = xCustomSect->GetPropertyName( *aIt );
            Any aPropValue = xCustomSect->GetAnyValue( *aIt );
            if( (aPropName.getLength() > 0) && aPropValue.hasValue() )
                SetCustomProperty( aPropName, aPropValue );
        }
    }

    // return code
    return (nGlobError != ERRCODE_NONE) ? nGlobError : nDocError;
}

BOOL SfxDocumentInfo::SavePropertySet( SotStorage* pStorage ) const
{
    // *** global properties into stream "005SummaryInformation" ***

    SfxOlePropertySet aGlobSet;

    // set supported properties
    SfxOleSection& rGlobSect = aGlobSet.AddSection( SECTION_GLOBAL );
    rGlobSect.SetStringValue( PROPID_TITLE, GetTitle() );
    rGlobSect.SetStringValue( PROPID_SUBJECT, GetTheme() );
    rGlobSect.SetStringValue( PROPID_KEYWORDS, GetKeywords() );
    rGlobSect.SetStringValue( PROPID_TEMPLATE, GetTemplateName() );
    rGlobSect.SetStringValue( PROPID_COMMENTS, GetComment() );
    rGlobSect.SetStringValue( PROPID_AUTHOR, GetCreated().GetName() );
    rGlobSect.SetFileTimeValue( PROPID_CREATED, GetCreated().GetTime() );
    rGlobSect.SetStringValue( PROPID_LASTAUTHOR, GetChanged().GetName() );
    rGlobSect.SetFileTimeValue( PROPID_LASTSAVED, GetChanged().GetTime() );
    if( GetPrinted().GetTime() != GetCreated().GetTime() )
        rGlobSect.SetFileTimeValue( PROPID_LASTPRINTED, GetPrinted().GetTime() );
    DateTime aEditTime( Date( 1, 1, 1601 ), Time( GetTime() ) );
    aEditTime.ConvertToLocalTime();
    rGlobSect.SetFileTimeValue( PROPID_EDITTIME, aEditTime );
    rGlobSect.SetStringValue( PROPID_REVNUMBER, String::CreateFromInt32( GetDocumentNumber() ) );
    rGlobSect.SetThumbnailValue( PROPID_THUMBNAIL, GetThumbnailMetaFile() );

    // save the property set
    ErrCode nGlobError = aGlobSet.SavePropertySet(
        pStorage, String( RTL_CONSTASCII_USTRINGPARAM( STREAM_SUMMARYINFO ) ) );

    // *** custom properties into stream "005DocumentSummaryInformation" ***

    SfxOlePropertySet aDocSet;

    // set custom properties
    SfxOleSection& rCustomSect = aDocSet.AddSection( SECTION_CUSTOM );
    const TDynamicProps& rDynProps = GetDynamicProps_Impl();
    for( TDynamicProps::const_iterator aIt = rDynProps.begin(), aEnd = rDynProps.end(); aIt != aEnd; ++aIt )
    {
        sal_Int32 nPropId = rCustomSect.GetFreePropertyId();
        if( rCustomSect.SetAnyValue( nPropId, aIt->second.aValue ) )
            rCustomSect.SetPropertyName( nPropId, aIt->first );
    }

    // save the property set
    ErrCode nDocError = aDocSet.SavePropertySet(
        pStorage, String( RTL_CONSTASCII_USTRINGPARAM( STREAM_DOCSUMMARYINFO ) ) );

    // return code
    return (nGlobError == ERRCODE_NONE) && (nDocError == ERRCODE_NONE);
}

//-------------------------------------------------------------------------

const SfxDocUserKey& SfxDocumentInfo::GetUserKey( USHORT n ) const
{
    DBG_ASSERT( n < GetUserKeyCount(), "user key index overflow" );
    return aUserKeys[n];
}

//-------------------------------------------------------------------------

void SfxDocumentInfo::SetUserKey( const SfxDocUserKey& rKey, USHORT n )
{
    DBG_ASSERT( n < GetUserKeyCount(), "user key index overflow" );
    aUserKeys[n] = rKey;
}


//-------------------------------------------------------------------------

SfxDocumentInfo::SfxDocumentInfo( const SfxDocumentInfo& rInf ):
    nUserDataSize(0),
    pUserData(0)
{
    pImp = new SfxDocumentInfo_Impl;
    bReadOnly = rInf.bReadOnly;
    bReloadEnabled = FALSE;
    nReloadSecs = 60;
    *this = rInf;
}

BOOL SfxDocumentInfo::IsReloadEnabled() const
{
    return bReloadEnabled;
}

void SfxDocumentInfo::EnableReload( BOOL bEnable )
{
    bReloadEnabled = bEnable;
}

const String& SfxDocumentInfo::GetDefaultTarget() const
{
    return aDefaultTarget;
}

void SfxDocumentInfo::SetDefaultTarget( const String& rString )
{
    aDefaultTarget = rString;
}

const String& SfxDocumentInfo::GetReloadURL() const
{
    return aReloadURL;
}

void SfxDocumentInfo::SetReloadURL( const String& rString )
{
    aReloadURL = rString;
}

sal_uInt32 SfxDocumentInfo::GetReloadDelay() const
{
    return nReloadSecs;
}

void SfxDocumentInfo::SetReloadDelay( sal_uInt32 nSec )
{
    nReloadSecs = nSec;
}

//-------------------------------------------------------------------------
void SfxDocumentInfo::Clear()
{
    BOOL _bReadOnly           = bReadOnly;
    BOOL _bPortableGraphics   = bPortableGraphics;
    BOOL _bSaveGraphicsCompressed = bSaveGraphicsCompressed;
    BOOL _bSaveOriginalGraphics = bSaveOriginalGraphics;

    (*this) = SfxDocumentInfo();

    bReadOnly           = _bReadOnly;
    bPortableGraphics   = _bPortableGraphics;
    bSaveGraphicsCompressed = _bSaveGraphicsCompressed;
    bSaveOriginalGraphics = _bSaveOriginalGraphics;
}

void SfxDocumentInfo::ResetFromTemplate( const String& rTemplateName, const String& rFileName )
{
    bReadOnly = FALSE;
    bPasswd = FALSE;

    aCreated  = SfxStamp( TIMESTAMP_INVALID_DATETIME );
    aChanged  = SfxStamp( TIMESTAMP_INVALID_DATETIME );
    aPrinted  = SfxStamp( TIMESTAMP_INVALID_DATETIME );
    aTitle.Erase();

    aTemplateName.Erase();
    aTemplateFileName.Erase();
    aTemplateDate = DateTime();
    lTime = 0;
    nDocNo = 1;
    bTemplateConfig = sal_False;

    if( ::utl::LocalFileHelper::IsLocalFile( rFileName ) )
    {
        String aFoundName;
        if( SFX_APP()->Get_Impl()->GetDocumentTemplates()->GetFull( String(), rTemplateName, aFoundName ) )
        {
            INetURLObject aObj( rFileName );
            aTemplateFileName = aObj.GetMainURL(INetURLObject::DECODE_TO_IURI);
            aTemplateName = rTemplateName;
            bQueryTemplate = sal_True;
        }
    }
}

const SfxDocumentInfo& SfxDocumentInfo::operator=( const SfxDocumentInfo& rInf)
{
    if( this == &rInf ) return *this;

    bReadOnly           = rInf.bReadOnly;
    bPasswd             = rInf.bPasswd;
    bPortableGraphics   = rInf.bPortableGraphics;
    bSaveGraphicsCompressed = rInf.bSaveGraphicsCompressed;
    bSaveOriginalGraphics = rInf.bSaveOriginalGraphics;
    bQueryTemplate      = rInf.bQueryTemplate;
    bTemplateConfig     = rInf.bTemplateConfig;
    eFileCharSet        = rInf.eFileCharSet;

    aCreated  = rInf.aCreated;
    aChanged  = rInf.aChanged;
    aPrinted  = rInf.aPrinted;
    aTitle    = rInf.aTitle;
    aTheme    = rInf.aTheme;
    aComment  = rInf.aComment;
    aKeywords = rInf.aKeywords;

    for(USHORT i=0; i < MAXDOCUSERKEYS; ++i)
        aUserKeys[i] = rInf.aUserKeys[i];

    aTemplateName = rInf.aTemplateName;
    aTemplateFileName = rInf.aTemplateFileName;
    aTemplateDate = rInf.aTemplateDate;

    aDefaultTarget = rInf.GetDefaultTarget();
    aReloadURL = rInf.GetReloadURL();
    bReloadEnabled = rInf.IsReloadEnabled();
    nReloadSecs = rInf.GetReloadDelay();

    Free();

    nUserDataSize = rInf.nUserDataSize;
    if(nUserDataSize) {
        pUserData = new char[nUserDataSize];
        memcpy(pUserData, rInf.pUserData, nUserDataSize);
    }
    lTime = rInf.lTime;
    nDocNo = rInf.nDocNo;

    bSaveVersionOnClose     = rInf.bSaveVersionOnClose;
    pImp->aCopiesTo         = rInf.pImp->aCopiesTo;
    pImp->aOriginal         = rInf.pImp->aOriginal;
    pImp->aReferences       = rInf.pImp->aReferences;
    pImp->aRecipient        = rInf.pImp->aRecipient;
    pImp->aReplyTo          = rInf.pImp->aReplyTo;
    pImp->aBlindCopies      = rInf.pImp->aBlindCopies;
    pImp->aInReplyTo        = rInf.pImp->aInReplyTo;
    pImp->aNewsgroups       = rInf.pImp->aNewsgroups;
    pImp->aSpecialMimeType  = rInf.pImp->aSpecialMimeType;
    pImp->nPriority         = rInf.pImp->nPriority;
    pImp->bUseUserData      = rInf.pImp->bUseUserData;

    pImp->_lDynamicProps = rInf.pImp->_lDynamicProps;

    return *this;
}

//-------------------------------------------------------------------------

int SfxDocumentInfo::operator==( const SfxDocumentInfo& rCmp) const
{
    if(eFileCharSet != rCmp.eFileCharSet ||
        bPasswd != rCmp.bPasswd ||
        bPortableGraphics != rCmp.bPortableGraphics ||
        bSaveGraphicsCompressed != rCmp.bSaveGraphicsCompressed ||
        bSaveOriginalGraphics != rCmp.bSaveOriginalGraphics ||
        bQueryTemplate != rCmp.bQueryTemplate  ||
        bTemplateConfig != rCmp.bTemplateConfig  ||
        aCreated != rCmp.aCreated ||
        aChanged != rCmp.aChanged ||
        aPrinted != rCmp.aPrinted ||
        aTitle != rCmp.aTitle ||
        aTheme != rCmp.aTheme ||
        aComment != rCmp.aComment ||
        aKeywords != rCmp.aKeywords ||
        aTemplateName != rCmp.aTemplateName ||
        aTemplateDate != rCmp.aTemplateDate ||
       IsReloadEnabled() != rCmp.IsReloadEnabled() ||
       GetReloadURL() != rCmp.GetReloadURL() ||
       GetReloadDelay() != rCmp.GetReloadDelay() ||
       GetDefaultTarget() != rCmp.GetDefaultTarget())
        return FALSE;

    for(USHORT i = 0; i < MAXDOCUSERKEYS; ++i) {
        if(aUserKeys[i] != rCmp.aUserKeys[i])
            return FALSE;
    }
    if(nUserDataSize != rCmp.nUserDataSize)
        return FALSE;
    if(nUserDataSize)
        return 0 == memcmp(pUserData, rCmp.pUserData, nUserDataSize);

    if ( pImp->aCopiesTo        != rCmp.pImp->aCopiesTo      ||
         pImp->aOriginal        != rCmp.pImp->aOriginal      ||
         pImp->aReferences      != rCmp.pImp->aReferences    ||
         pImp->aRecipient       != rCmp.pImp->aRecipient     ||
         pImp->aReplyTo         != rCmp.pImp->aReplyTo       ||
         pImp->aBlindCopies     != rCmp.pImp->aBlindCopies   ||
         pImp->aInReplyTo       != rCmp.pImp->aInReplyTo     ||
         pImp->aNewsgroups      != rCmp.pImp->aNewsgroups    ||
         pImp->aSpecialMimeType != rCmp.pImp->aSpecialMimeType ||
         pImp->nPriority        != rCmp.pImp->nPriority      ||
         pImp->bUseUserData     != rCmp.pImp->bUseUserData   ||
         bSaveVersionOnClose    != rCmp.bSaveVersionOnClose )
        return FALSE;

    return TRUE;
}

//-------------------------------------------------------------------------

const SfxDocumentInfo &SfxDocumentInfo::CopyUserData(const SfxDocumentInfo &rSource)
{
    bQueryTemplate = rSource.bQueryTemplate;
    bTemplateConfig = rSource.bTemplateConfig;

    SetReloadDelay( rSource.GetReloadDelay() );
    SetReloadURL( rSource.GetReloadURL() );
    EnableReload( rSource.IsReloadEnabled() );
    SetDefaultTarget( rSource.GetDefaultTarget() );

    aTitle = rSource.aTitle;
    aTheme = rSource.aTheme;
    aComment = rSource.aComment;
    aKeywords = rSource.aKeywords;

    aTemplateName     = rSource.aTemplateName;
    aTemplateFileName = rSource.aTemplateFileName;
    aTemplateDate     = rSource.aTemplateDate;

    for(USHORT i = 0; i < MAXDOCUSERKEYS; ++i)
        aUserKeys[i] = rSource.aUserKeys[i];

    if(pUserData)
    {
        delete [](sal::static_int_cast<char*>( pUserData ));
        pUserData = 0;
    }
    nUserDataSize = rSource.nUserDataSize;
    if(nUserDataSize) {
        pUserData = new char[nUserDataSize];
        memcpy(pUserData, rSource.pUserData, nUserDataSize);
    }

    pImp->aCopiesTo         = rSource.pImp->aCopiesTo;
    pImp->aOriginal         = rSource.pImp->aOriginal;
    pImp->aReferences       = rSource.pImp->aReferences;
    pImp->aRecipient        = rSource.pImp->aRecipient;
    pImp->aReplyTo          = rSource.pImp->aReplyTo;
    pImp->aBlindCopies      = rSource.pImp->aBlindCopies;
    pImp->aInReplyTo        = rSource.pImp->aInReplyTo;
    pImp->aNewsgroups       = rSource.pImp->aNewsgroups;
    pImp->aSpecialMimeType  = rSource.pImp->aSpecialMimeType;
    pImp->nPriority         = rSource.pImp->nPriority;
    pImp->bUseUserData      = rSource.pImp->bUseUserData;

    return *this;
}

//-------------------------------------------------------------------------

void SfxDocumentInfo::Free()
{
    delete [](sal::static_int_cast<char*>( pUserData ));;
    pUserData = 0;
    nUserDataSize = 0;
}

//-------------------------------------------------------------------------

String SfxDocumentInfo::AdjustTextLen_Impl( const String& rText, USHORT /*nMax*/ )
{
    String aRet = rText;
/*! pb: dont cut any longer because the new file format has no length limit
    if ( aRet.Len() > nMax )
        aRet.Erase( nMax ); */
    return aRet;
}

//-------------------------------------------------------------------------

SfxDocumentInfo::SfxDocumentInfo() :
    eFileCharSet(gsl_getSystemTextEncoding()),
    bPasswd(FALSE),
    bQueryTemplate(FALSE),
    bTemplateConfig(FALSE),
    bSaveVersionOnClose( FALSE ),
    aChanged( TIMESTAMP_INVALID_DATETIME ),
    aPrinted( TIMESTAMP_INVALID_DATETIME ),
    nUserDataSize(0),
    nDocNo(1),
    pUserData(0),
    lTime(0)
{
    pImp = new SfxDocumentInfo_Impl;

    bReadOnly = FALSE;
    bReloadEnabled = FALSE;
    nReloadSecs = 60;
    SFX_APP();
    bPortableGraphics = TRUE;
    SvtSaveOptions aSaveOptions;
    bSaveGraphicsCompressed = aSaveOptions.GetSaveGraphicsMode() == SvtSaveOptions::SaveGraphicsCompressed;
    bSaveOriginalGraphics = aSaveOptions.GetSaveGraphicsMode() == SvtSaveOptions::SaveGraphicsOriginal;

    const String aInf( DEFINE_CONST_UNICODE( "Info " ) );
    for( USHORT i = 0; i < MAXDOCUSERKEYS; ++i )    {
        aUserKeys[i].aTitle = aInf;
        aUserKeys[i].aTitle += String::CreateFromInt32(i+1);
    }
}

//-------------------------------------------------------------------------

SfxDocumentInfo::~SfxDocumentInfo()
{
    Free();
    delete pImp;
}

String SfxDocumentInfo::GetCopiesTo() const
{
    return pImp->aCopiesTo;
}

void SfxDocumentInfo::SetCopiesTo( const String& rStr )
{
    pImp->aCopiesTo = rStr;
}

String SfxDocumentInfo::GetOriginal() const
{
    return pImp->aOriginal;
}

void SfxDocumentInfo::SetOriginal( const String& rStr )
{
    pImp->aOriginal = rStr;
}

String SfxDocumentInfo::GetReferences() const
{
    return pImp->aReferences;
}

void SfxDocumentInfo::SetReferences( const String& rStr )
{
    pImp->aReferences = rStr;
}

String SfxDocumentInfo::GetRecipient() const
{
    return pImp->aRecipient;
}

void SfxDocumentInfo::SetRecipient( const String& rStr )
{
    pImp->aRecipient = rStr;
}

String SfxDocumentInfo::GetReplyTo() const
{
    return pImp->aReplyTo;
}

void SfxDocumentInfo::SetReplyTo( const String& rStr )
{
    pImp->aReplyTo = rStr;
}

String SfxDocumentInfo::GetBlindCopies() const
{
    return pImp->aBlindCopies;
}

void SfxDocumentInfo::SetBlindCopies( const String& rStr )
{
    pImp->aBlindCopies = rStr;
}

String SfxDocumentInfo::GetInReplyTo() const
{
    return pImp->aInReplyTo;
}

void SfxDocumentInfo::SetInReplyTo( const String& rStr )
{
    pImp->aInReplyTo = rStr;
}

String SfxDocumentInfo::GetNewsgroups() const
{
    return pImp->aNewsgroups;
}

void SfxDocumentInfo::SetNewsgroups( const String& rStr )
{
    pImp->aNewsgroups = rStr;
}

String SfxDocumentInfo::GetSpecialMimeType() const
{
    return pImp->aSpecialMimeType;
}

void SfxDocumentInfo::SetSpecialMimeType( const String& rStr )
{
    pImp->aSpecialMimeType = rStr;
}

USHORT SfxDocumentInfo::GetPriority() const
{
    return pImp->nPriority;
}

void SfxDocumentInfo::SetPriority( USHORT nPrio )
{
    pImp->nPriority = nPrio;
}

BOOL SfxDocumentInfo::IsUseUserData() const
{
    return pImp->bUseUserData;
}

void SfxDocumentInfo::SetUseUserData( BOOL bNew )
{
    pImp->bUseUserData = bNew;
}

//-----------------------------------------------------------------------------

void SfxDocumentInfo::SetTitle( const String& rVal )
{
    aTitle = AdjustTextLen_Impl( rVal, SFXDOCINFO_TITLELENMAX );
}
//------------------------------------------------------------------------

void SfxDocumentInfo::SetTheme( const String& rVal )
{
    aTheme = AdjustTextLen_Impl( rVal, SFXDOCINFO_THEMELENMAX );
}
//------------------------------------------------------------------------

void SfxDocumentInfo::SetComment( const String& rVal )
{
    aComment = AdjustTextLen_Impl( rVal, SFXDOCINFO_COMMENTLENMAX );
}
//------------------------------------------------------------------------

void SfxDocumentInfo::SetKeywords( const String& rVal )
{
    aKeywords = AdjustTextLen_Impl( rVal, SFXDOCINFO_KEYWORDLENMAX );
}
//------------------------------------------------------------------------

const GDIMetaFile& SfxDocumentInfo::GetThumbnailMetaFile() const
{
    return pImp->aThumbnailMetaFile;
}

void SfxDocumentInfo::SetThumbnailMetaFile (const GDIMetaFile &aMetaFile)
{
    pImp->aThumbnailMetaFile = aMetaFile;
}

void SfxDocumentInfo::DeleteUserData( BOOL bUseAuthor )
{
    SfxStamp aCreatedStamp;
    if ( bUseAuthor )
        aCreatedStamp.SetName( SvtUserOptions().GetFullName() );
    SetCreated( aCreatedStamp );
    SfxStamp aInvalid( TIMESTAMP_INVALID_DATETIME );
    SetChanged( aInvalid );
    SetPrinted( aInvalid );
    SetTime( 0L );
    SetDocumentNumber( 1 );
}

void SfxDocumentInfo::DeleteUserDataCompletely()
{
    SfxStamp aInvalid( TIMESTAMP_INVALID_DATETIME );
    SetCreated( aInvalid );
    SetChanged( aInvalid );
    SetPrinted( aInvalid );
    SetTime( 0L );
    SetDocumentNumber( 1 );
}

// --> PB 2004-08-23 #i33095#
sal_Bool SfxDocumentInfo::IsLoadReadonly() const
{
    return pImp->bLoadReadonly;
}
void SfxDocumentInfo::SetLoadReadonly( sal_Bool _bReadonly )
{
    pImp->bLoadReadonly = _bReadonly;
}

TDynamicProps&  SfxDocumentInfo::GetDynamicProps_Impl()
{
    return pImp->_lDynamicProps;
}

const TDynamicProps&  SfxDocumentInfo::GetDynamicProps_Impl() const
{
    return pImp->_lDynamicProps;
}

BOOL SfxDocumentInfo::SetCustomProperty(const OUString& aPropertyName, const Any& aValue)
{
    if (pImp->_lDynamicProps.find(aPropertyName) != pImp->_lDynamicProps.end())
        return FALSE;

    SfxExtendedItemPropertyMap aProp;
    aProp.pName    = 0; // superflous -> held as hash key.
    aProp.nNameLen = 0;
    aProp.nFlags   = ::com::sun::star::beans::PropertyAttribute::REMOVEABLE;
    aProp.aValue   = aValue;
    aProp.nWID     = 0xFFFF;

    pImp->_lDynamicProps[aPropertyName] = aProp;
    return TRUE;
}

// <--


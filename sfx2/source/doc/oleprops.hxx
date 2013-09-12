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

#include <map>
#include <boost/shared_ptr.hpp>
#include <osl/thread.h>
#include <rtl/ustring.hxx>
#include <sot/storage.hxx>
#include <vcl/bitmapex.hxx>

#include <com/sun/star/util/DateTime.hpp>
#include <com/sun/star/util/Date.hpp>


// ============================================================================

//namespace {

// ============================================================================
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
const sal_Int32 PROPTYPE_BLOB           = 65;
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

// some Builtin properties
const sal_Int32 PROPID_CATEGORY         = 0x2;
const sal_Int32 PROPID_COMPANY          = 0xf;
const sal_Int32 PROPID_MANAGER          = 0xe;
// predefined codepages
const sal_uInt16 CODEPAGE_UNKNOWN       = 0;
const sal_uInt16 CODEPAGE_UNICODE       = 1200;
const sal_uInt16 CODEPAGE_UTF8          = 65001;

// predefined clipboard format IDs
const sal_Int32 CLIPFMT_WIN             = -1;

// predefined clipboard data format IDs
const sal_Int32 CLIPDATAFMT_DIB         = 8;

// ============================================================================

/** Helper for classes that need text encoding settings.

    Classes derived from this class will include functions to store and use
    text encoding settings and to convert Windows codepage constants.
 */
class SfxOleTextEncoding
{
public:
    inline explicit     SfxOleTextEncoding() :
                            mxTextEnc( new rtl_TextEncoding( osl_getThreadTextEncoding() ) ) {}
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
    OUString            LoadString8( SvStream& rStrm ) const;
    /** Saves a string to the passed stream with current encoding (maybe Unicode). */
    void                SaveString8( SvStream& rStrm, const OUString& rValue ) const;

    /** Loads a Unicode string from the passed stream, ignores own encoding. */
    OUString            LoadString16( SvStream& rStrm ) const;
    /** Saves a Unicode string to the passed stream, ignores own encoding. */
    void                SaveString16( SvStream& rStrm, const OUString& rValue ) const;

private:
    OUString            ImplLoadString8( SvStream& rStrm ) const;
    OUString            ImplLoadString16( SvStream& rStrm ) const;
    void                ImplSaveString8( SvStream& rStrm, const OUString& rValue ) const;
    void                ImplSaveString16( SvStream& rStrm, const OUString& rValue ) const;
};

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
    OUString            GetPropertyName( sal_Int32 nPropId ) const;
    /** Sets a custom name for the passed property ID. */
    void                SetPropertyName( sal_Int32 nPropId, const OUString& rPropName );

private:
    virtual void        ImplLoad( SvStream& rStrm );
    virtual void        ImplSave( SvStream& rStrm );

private:
    typedef ::std::map< sal_Int32, OUString > SfxOlePropNameMap;
    SfxOlePropNameMap   maPropNameMap;
};

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
    /** Returns the value of a floating-point property with the passed ID in rfValue.
        @return  true = Property found, rfValue is valid; false = Property not found. */
    bool                GetDoubleValue( double& rfValue, sal_Int32 nPropId ) const;
    /** Returns the value of a boolean property with the passed ID in rbValue.
        @return  true = Property found, rbValue is valid; false = Property not found. */
    bool                GetBoolValue( bool& rbValue, sal_Int32 nPropId ) const;
    /** Returns the value of a string property with the passed ID in rValue.
        @return  true = Property found, rValue is valid; false = Property not found. */
    bool                GetStringValue( OUString& rValue, sal_Int32 nPropId ) const;
    /** Returns the value of a time stamp property with the passed ID in rValue.
        @return  true = Property found, rValue is valid; false = Property not found. */
    bool                GetFileTimeValue( ::com::sun::star::util::DateTime& rValue, sal_Int32 nPropId ) const;
    /** Returns the value of a date property with the passed ID in rValue.
        @return  true = Property found, rValue is valid; false = Property not found. */
    bool                GetDateValue( ::com::sun::star::util::Date& rValue, sal_Int32 nPropId ) const;

    /** Adds the passed property to the property set. Drops an existing old property. */
    void                SetProperty( SfxOlePropertyRef xProp );
    /** Inserts a signed int32 property with the passed value. */
    void                SetInt32Value( sal_Int32 nPropId, sal_Int32 nValue );
    /** Inserts a floating-point property with the passed value. */
    void                SetDoubleValue( sal_Int32 nPropId, double fValue );
    /** Inserts a boolean property with the passed value. */
    void                SetBoolValue( sal_Int32 nPropId, bool bValue );
    /** Inserts a string property with the passed value.
        @return  true = Property inserted; false = String was empty, property not inserted. */
    bool                SetStringValue( sal_Int32 nPropId, const OUString& rValue, bool bSkipEmpty = true );
    /** Inserts a time stamp property with the passed value. */
    void                SetFileTimeValue( sal_Int32 nPropId, const ::com::sun::star::util::DateTime& rValue );
    /** Inserts a date property with the passed value. */
    void                SetDateValue( sal_Int32 nPropId, const ::com::sun::star::util::Date& rValue );
    /** Inserts a thumbnail property from the passed meta file. */
    void                SetThumbnailValue( sal_Int32 nPropId,
                            const ::com::sun::star::uno::Sequence<sal_uInt8> & i_rData);
    /** Inserts a BLOB property with the passed data. */
    void                SetBlobValue( sal_Int32 nPropId,
                            const ::com::sun::star::uno::Sequence<sal_uInt8> & i_rData);

    /** Returns the value of the property with the passed ID in a UNO any. */
    com::sun::star::uno::Any GetAnyValue( sal_Int32 nPropId ) const;
    /** Inserts a property created from the passed any.
        @return  true = Property converted and inserted; false = Property type not supported. */
    bool                SetAnyValue( sal_Int32 nPropId, const com::sun::star::uno::Any& rValue );

    /** Returns the custom name for the passed property ID, or an empty string, if name not found. */
    OUString            GetPropertyName( sal_Int32 nPropId ) const;
    /** Sets a custom name for the passed property ID. */
    void                SetPropertyName( sal_Int32 nPropId, const OUString& rPropName );

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
    ErrCode             LoadPropertySet( SotStorage* pStrg, const OUString& rStrmName );
    /** Saves this object to the passed storage. */
    ErrCode             SavePropertySet( SotStorage* pStrg, const OUString& rStrmName );

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


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

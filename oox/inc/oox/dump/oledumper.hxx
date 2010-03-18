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

#ifndef OOX_DUMP_OLEDUMPER_HXX
#define OOX_DUMP_OLEDUMPER_HXX

#include "oox/helper/storagebase.hxx"
#include "oox/dump/dumperbase.hxx"

#if OOX_INCLUDE_DUMPER

namespace com { namespace sun { namespace star {
    namespace io { class XInputStream; }
} } }

namespace oox {
namespace dump {

// ============================================================================
// ============================================================================

class StdFontObject : public InputObjectBase
{
public:
    explicit            StdFontObject( const InputObjectBase& rParent );

protected:
    virtual void        implDump();
};

// ============================================================================

class StdPicObject : public InputObjectBase
{
public:
    explicit            StdPicObject( const InputObjectBase& rParent );

protected:
    virtual void        implDump();
};

// ============================================================================

class StdHlinkObject : public InputObjectBase
{
public:
    explicit            StdHlinkObject( const InputObjectBase& rParent );

protected:
    virtual void        implDump();

private:
    ::rtl::OUString     dumpHyperlinkString( const String& rName, bool bUnicode );

    bool                dumpGuidAndMoniker();
    void                dumpUrlMoniker();
    void                dumpFileMoniker();
    void                dumpItemMoniker();
    void                dumpAntiMoniker();
    void                dumpCompositeMoniker();
};

// ============================================================================
// ============================================================================

class OlePropertyStreamObject : public BinaryStreamObject
{
public:
    explicit            OlePropertyStreamObject( const ObjectBase& rParent, const BinaryInputStreamRef& rxStrm, const ::rtl::OUString& rSysFileName );

protected:
    virtual void        implDump();

private:
    void                dumpSection( const ::rtl::OUString& rGuid, sal_uInt32 nStartPos );

    void                dumpProperty( sal_Int32 nPropId, sal_uInt32 nStartPos );
    void                dumpCodePageProperty( sal_uInt32 nStartPos );
    void                dumpDictionaryProperty( sal_uInt32 nStartPos );

    void                dumpPropertyContents( sal_Int32 nPropId );
    void                dumpPropertyValue( sal_Int32 nPropId, sal_Int32 nBaseType );

    sal_Int32           dumpPropertyType();
    void                dumpBlob( const String& rName );
    ::rtl::OUString     dumpString8( const String& rName );
    ::rtl::OUString     dumpCharArray8( const String& rName, sal_Int32 nLen );
    ::rtl::OUString     dumpString16( const String& rName );
    ::rtl::OUString     dumpCharArray16( const String& rName, sal_Int32 nLen );

    bool                startElement( sal_uInt32 nStartPos );
    void                writeSectionHeader( const ::rtl::OUString& rGuid, sal_uInt32 nStartPos );
    void                writePropertyHeader( sal_Int32 nPropId, sal_uInt32 nStartPos );

private:
    NameListRef         mxPropIds;
    rtl_TextEncoding    meTextEnc;
    bool                mbIsUnicode;
};

// ============================================================================

class OleStorageObject : public StorageObjectBase
{
public:
    explicit            OleStorageObject( const ObjectBase& rParent, const StorageRef& rxStrg, const ::rtl::OUString& rSysPath );

protected:
    inline explicit     OleStorageObject() {}

    using               StorageObjectBase::construct;
    void                construct( const ObjectBase& rParent, const StorageRef& rxStrg, const ::rtl::OUString& rSysPath );
    void                construct( const ObjectBase& rParent );

    virtual void        implDumpStream(
                            const BinaryInputStreamRef& rxStrm,
                            const ::rtl::OUString& rStrgPath,
                            const ::rtl::OUString& rStrmName,
                            const ::rtl::OUString& rSysFileName );
};

// ============================================================================
// ============================================================================

struct OcxFormSiteInfo
{
    ::rtl::OUString     maProgId;
    sal_Int32           mnId;
    sal_uInt32          mnLength;
    bool                mbInStream;

    inline explicit     OcxFormSiteInfo() : mnId( 0 ), mnLength( 0 ), mbInStream( false ) {}
};

typedef ::std::vector< OcxFormSiteInfo > OcxFormSiteInfoVector;

// ----------------------------------------------------------------------------

struct OcxFormSharedData
{
    OUStringVector      maClassInfoProgIds;
    OcxFormSiteInfoVector maSiteInfos;
};

// ============================================================================
// ============================================================================

class OcxPropertyObjectBase : public InputObjectBase
{
protected:
    inline explicit     OcxPropertyObjectBase() {}

    using               InputObjectBase::construct;
    void                construct(
                            const ObjectBase& rParent,
                            const BinaryInputStreamRef& rxStrm,
                            const ::rtl::OUString& rSysFileName,
                            const String& rPropNameList,
                            bool b64BitPropFlags = false );
    void                construct(
                            const OutputObjectBase& rParent,
                            const BinaryInputStreamRef& rxStrm,
                            const String& rPropNameList,
                            bool b64BitPropFlags = false );
    void                construct(
                            const InputObjectBase& rParent,
                            const String& rPropNameList,
                            bool b64BitPropFlags = false );

    virtual bool        implIsValid() const;
    virtual void        implDump();

    virtual void        implDumpShortProperties();
    virtual void        implDumpExtended();

    bool                ensureValid( bool bCondition = true );

    template< typename Type >
    void                alignInput();

    void                setAlignAnchor();
    bool                startNextProperty();
    ::rtl::OUString     getPropertyName() const;

    template< typename Type >
    Type                dumpDecProperty( Type nDefault, const NameListWrapper& rListWrp = NO_LIST );
    template< typename Type >
    Type                dumpHexProperty( Type nDefault, const NameListWrapper& rListWrp = NO_LIST );

    inline bool         dumpBoolProperty() { return startNextProperty(); }
    inline sal_Int32    dumpHmmProperty() { return dumpDecProperty< sal_Int32 >( 0, "CONV-HMM-TO-CM" ); }
    inline sal_uInt8    dumpMousePtrProperty() { return dumpDecProperty< sal_uInt8 >( 0, "OCX-MOUSEPTR" ); }
    template< typename Type >
    inline Type         dumpBorderStyleProperty( Type nDefault ) { return dumpDecProperty< Type >( nDefault, "OCX-BORDERSTYLE" ); }
    template< typename Type >
    inline Type         dumpSpecialEffectProperty( Type nDefault ) { return dumpDecProperty< Type >( nDefault, "OCX-SPECIALEFFECT" ); }
    inline sal_uInt32   dumpEnabledProperty() { return dumpDecProperty< sal_uInt32 >( 1, "OCX-ENABLED" ); }
    inline sal_Int32    dumpOrientationProperty() { return dumpDecProperty< sal_Int32 >( -1, "OCX-ORIENTATION" ); }
    inline sal_Int32    dumpDelayProperty() { return dumpDecProperty< sal_Int32 >( 50, "OCX-CONV-MS" ); }
    inline sal_uInt32   dumpImagePosProperty() { return dumpHexProperty< sal_uInt32 >( 0x00070001, "OCX-IMAGEPOS" ); }
    inline sal_uInt8    dumpImageSizeModeProperty() { return dumpDecProperty< sal_uInt8 >( 0, "OCX-IMAGESIZEMODE" ); }
    inline sal_uInt8    dumpImageAlignProperty() { return dumpDecProperty< sal_uInt8 >( 2, "OCX-IMAGEALIGN" ); }

    sal_uInt32          dumpFlagsProperty( sal_uInt32 nDefault, const sal_Char* pcNameList = "OCX-FLAGS" );
    sal_uInt32          dumpColorProperty( sal_uInt32 nDefault );
    sal_Unicode         dumpUnicodeProperty();
    void                dumpUnknownProperty();

    void                dumpPosProperty();
    void                dumpSizeProperty();
    void                dumpGuidProperty( ::rtl::OUString* pValue = 0 );
    void                dumpStringProperty( ::rtl::OUString* pValue = 0 );
    void                dumpStringArrayProperty();
    void                dumpStreamProperty();

    void                dumpEmbeddedFont();
    void                dumpToPosition( sal_Int64 nPos );

private:
    void                constructOcxPropObj( const String& rPropNameList, bool b64BitPropFlags );

    void                dumpVersion();
    ::rtl::OUString     dumpString( const String& rName, sal_uInt32 nSize, bool bArray );
    void                dumpShortProperties();
    void                dumpLargeProperties();

private:
    struct LargeProperty
    {
        enum LargePropertyType { PROPTYPE_POS, PROPTYPE_SIZE, PROPTYPE_GUID, PROPTYPE_STRING, PROPTYPE_STRINGARRAY };

        LargePropertyType   mePropType;
        ::rtl::OUString     maItemName;
        sal_uInt32          mnDataSize;
        ::rtl::OUString*    mpItemValue;
        inline explicit     LargeProperty( LargePropertyType ePropType, const String& rItemName, sal_uInt32 nDataSize, ::rtl::OUString* pItemValue = 0 ) :
                                mePropType( ePropType ), maItemName( rItemName ), mnDataSize( nDataSize ), mpItemValue( pItemValue ) {}
    };
    typedef ::std::vector< LargeProperty > LargePropertyVector;

    struct StreamProperty
    {
        ::rtl::OUString     maItemName;
        sal_uInt16          mnData;
        inline explicit     StreamProperty( const String& rItemName, sal_uInt16 nData ) :
                                maItemName( rItemName ), mnData( nData ) {}
    };
    typedef ::std::vector< StreamProperty > StreamPropertyVector;

    LargePropertyVector maLargeProps;
    StreamPropertyVector maStreamProps;
    NameListRef         mxPropNames;
    sal_Int64           mnPropertiesStart;
    sal_Int64           mnPropertiesEnd;
    sal_Int64           mnPropFlags;
    sal_Int64           mnCurrProp;
    bool                mb64BitPropFlags;
    bool                mbValid;
};

// ----------------------------------------------------------------------------

template< typename Type >
void OcxPropertyObjectBase::alignInput()
{
    in().skip( (sizeof( Type ) - ((in().tell() - mnPropertiesStart) % sizeof( Type ))) % sizeof( Type ) );
}

template< typename Type >
Type OcxPropertyObjectBase::dumpDecProperty( Type nDefault, const NameListWrapper& rListWrp )
{
    if( startNextProperty() )
    {
        alignInput< Type >();
        return dumpDec< Type >( getPropertyName(), rListWrp );
    }
    return nDefault;
}

template< typename Type >
Type OcxPropertyObjectBase::dumpHexProperty( Type nDefault, const NameListWrapper& rListWrp )
{
    if( startNextProperty() )
    {
        alignInput< Type >();
        return dumpHex< Type >( getPropertyName(), rListWrp );
    }
    return nDefault;
}

// ============================================================================

class OcxCFontNewObject : public OcxPropertyObjectBase
{
public:
    explicit            OcxCFontNewObject( const InputObjectBase& rParent );

protected:
    virtual void        implDumpShortProperties();
};

// ============================================================================

class OcxColumnInfoObject : public OcxPropertyObjectBase
{
public:
    explicit            OcxColumnInfoObject( const InputObjectBase& rParent );

protected:
    virtual void        implDumpShortProperties();
};

// ============================================================================

class OcxCommandButtonObject : public OcxPropertyObjectBase
{
public:
    explicit            OcxCommandButtonObject( const InputObjectBase& rParent );

protected:
    virtual void        implDumpShortProperties();
    virtual void        implDumpExtended();
};

// ============================================================================

class OcxMorphControlObject : public OcxPropertyObjectBase
{
public:
    explicit            OcxMorphControlObject( const InputObjectBase& rParent );

protected:
    virtual void        implDumpShortProperties();
    virtual void        implDumpExtended();

private:
    void                dumpColumnInfos();

private:
    sal_uInt16          mnColInfoCount;
    sal_uInt8           mnCtrlType;
};

// ============================================================================

class OcxLabelObject : public OcxPropertyObjectBase
{
public:
    explicit            OcxLabelObject( const InputObjectBase& rParent );

protected:
    virtual void        implDumpShortProperties();
    virtual void        implDumpExtended();
};

// ============================================================================

class OcxImageObject : public OcxPropertyObjectBase
{
public:
    explicit            OcxImageObject( const InputObjectBase& rParent );

protected:
    virtual void        implDumpShortProperties();
};

// ============================================================================

class OcxScrollBarObject : public OcxPropertyObjectBase
{
public:
    explicit            OcxScrollBarObject( const InputObjectBase& rParent );

protected:
    virtual void        implDumpShortProperties();
};

// ============================================================================

class OcxSpinButtonObject : public OcxPropertyObjectBase
{
public:
    explicit            OcxSpinButtonObject( const InputObjectBase& rParent );

protected:
    virtual void        implDumpShortProperties();
};

// ============================================================================

class OcxTabStripObject : public OcxPropertyObjectBase
{
public:
    explicit            OcxTabStripObject( const InputObjectBase& rParent );

protected:
    virtual void        implDumpShortProperties();
    virtual void        implDumpExtended();

private:
    sal_Int32           mnTabFlagCount;
};

// ============================================================================

class OcxControlObject : public InputObjectBase
{
public:
    explicit            OcxControlObject(
                            const InputObjectBase& rParent,
                            const ::rtl::OUString& rProgId,
                            sal_Int64 nLength );

protected:
    virtual void        implDump();

private:
    ::rtl::OUString     maProgId;
    sal_Int64           mnLength;
};

// ============================================================================

class OcxGuidControlObject : public InputObjectBase
{
public:
    explicit            OcxGuidControlObject(
                            const InputObjectBase& rParent,
                            sal_Int64 nLength );
    explicit            OcxGuidControlObject(
                            const OutputObjectBase& rParent,
                            const BinaryInputStreamRef& rxStrm );
    explicit            OcxGuidControlObject(
                            const ObjectBase& rParent,
                            const BinaryInputStreamRef& rxStrm,
                            const ::rtl::OUString& rSysFileName );

protected:
    virtual void        implDump();

private:
    sal_Int64           mnLength;
};

// ============================================================================

class OcxControlsStreamObject : public InputObjectBase
{
public:
    explicit            OcxControlsStreamObject(
                            const ObjectBase& rParent,
                            const BinaryInputStreamRef& rxStrm,
                            const ::rtl::OUString& rSysFileName,
                            OcxFormSharedData& rFormData );

protected:
    virtual void        implDump();

private:
    OcxFormSharedData&  mrFormData;
};

// ============================================================================
// ============================================================================

class OcxPageObject : public OcxPropertyObjectBase
{
public:
    explicit            OcxPageObject( const InputObjectBase& rParent );

protected:
    virtual void        implDumpShortProperties();
};

// ============================================================================

class OcxMultiPageObject : public OcxPropertyObjectBase
{
public:
    explicit            OcxMultiPageObject( const InputObjectBase& rParent );

protected:
    virtual void        implDumpShortProperties();
    virtual void        implDumpExtended();

private:
    sal_Int32           mnPageCount;
};

// ============================================================================

class OcxMultiPageStreamObject : public InputObjectBase
{
public:
    explicit            OcxMultiPageStreamObject(
                            const ObjectBase& rParent,
                            const BinaryInputStreamRef& rxStrm,
                            const ::rtl::OUString& rSysFileName,
                            OcxFormSharedData& rFormData );

protected:
    virtual void        implDump();

private:
    OcxFormSharedData&  mrFormData;
};

// ============================================================================
// ============================================================================

class OcxFormClassInfoObject : public OcxPropertyObjectBase
{
public:
    explicit            OcxFormClassInfoObject( const InputObjectBase& rParent, OcxFormSharedData& rFormData );

protected:
    virtual void        implDumpShortProperties();

private:
    OcxFormSharedData&  mrFormData;
};

// ============================================================================

class OcxFormSiteObject : public OcxPropertyObjectBase
{
public:
    explicit            OcxFormSiteObject( const InputObjectBase& rParent, OcxFormSharedData& rFormData );

protected:
    virtual void        implDumpShortProperties();

private:
    OcxFormSharedData&  mrFormData;
};

// ============================================================================

class OcxFormDesignExtObject : public OcxPropertyObjectBase
{
public:
    explicit            OcxFormDesignExtObject( const InputObjectBase& rParent );

protected:
    virtual void        implDumpShortProperties();
};

// ============================================================================

class OcxFormObject : public OcxPropertyObjectBase
{
public:
    explicit            OcxFormObject(
                            const ObjectBase& rParent,
                            const BinaryInputStreamRef& rxStrm,
                            const ::rtl::OUString& rSysFileName,
                            OcxFormSharedData& rFormData );

protected:
    virtual void        implDumpShortProperties();
    virtual void        implDumpExtended();

private:
    void                dumpClassInfos();
    void                dumpFormSites( sal_uInt32 nCount );
    void                dumpSiteData();
    void                dumpDesignExtender();

private:
    OcxFormSharedData&  mrFormData;
    sal_uInt32          mnFlags;
};

// ============================================================================

class OcxFormStorageObject : public OleStorageObject
{
public:
    explicit            OcxFormStorageObject(
                            const ObjectBase& rParent,
                            const StorageRef& rxStrg,
                            const ::rtl::OUString& rSysPath );

protected:
    virtual void        implDumpStream(
                            const BinaryInputStreamRef& rxStrm,
                            const ::rtl::OUString& rStrgPath,
                            const ::rtl::OUString& rStrmName,
                            const ::rtl::OUString& rSysFileName );

    virtual void        implDumpStorage(
                            const StorageRef& rxStrg,
                            const ::rtl::OUString& rStrgPath,
                            const ::rtl::OUString& rSysPath );

private:
    bool                isFormStorage( const ::rtl::OUString& rStrgPath ) const;

private:
    OcxFormSharedData   maFormData;
};

// ============================================================================
// ============================================================================

struct VbaSharedData
{
    typedef ::std::map< ::rtl::OUString, sal_Int32 > StreamOffsetMap;

    StreamOffsetMap     maStrmOffsets;
    rtl_TextEncoding    meTextEnc;

    explicit            VbaSharedData();

    bool                isModuleStream( const ::rtl::OUString& rStrmName ) const;
    sal_Int32           getStreamOffset( const ::rtl::OUString& rStrmName ) const;
};

// ============================================================================

class VbaDirStreamObject : public SequenceRecordObjectBase
{
public:
    explicit            VbaDirStreamObject(
                            const ObjectBase& rParent,
                            const BinaryInputStreamRef& rxStrm,
                            const ::rtl::OUString& rSysFileName,
                            VbaSharedData& rVbaData );

protected:
    virtual bool        implIsValid() const;
    virtual bool        implReadRecordHeader( BinaryInputStream& rBaseStrm, sal_Int64& ornRecId, sal_Int64& ornRecSize );
    virtual void        implDumpRecordBody();

private:
    ::rtl::OUString     dumpByteString( const String& rName = EMPTY_STRING );
    ::rtl::OUString     dumpUniString( const String& rName = EMPTY_STRING );

    ::rtl::OUString     dumpByteStringWithLength( const String& rName = EMPTY_STRING );

private:
    VbaSharedData&      mrVbaData;
    BinaryInputStreamRef mxInStrm;
    ::rtl::OUString     maCurrStream;
    sal_Int32           mnCurrOffset;
};

// ============================================================================

class VbaModuleStreamObject : public InputObjectBase
{
public:
    explicit            VbaModuleStreamObject(
                            const ObjectBase& rParent,
                            const BinaryInputStreamRef& rxStrm,
                            const ::rtl::OUString& rSysFileName,
                            VbaSharedData& rVbaData,
                            sal_Int32 nStrmOffset );

protected:
    virtual void        implDump();

private:
    VbaSharedData&      mrVbaData;
    sal_Int32           mnStrmOffset;
};

// ============================================================================

class VbaStorageObject : public OleStorageObject
{
public:
    explicit            VbaStorageObject(
                            const ObjectBase& rParent,
                            const StorageRef& rxStrg,
                            const ::rtl::OUString& rSysPath,
                            VbaSharedData& rVbaData );

protected:
    virtual void        implDumpStream(
                            const BinaryInputStreamRef& rxStrm,
                            const ::rtl::OUString& rStrgPath,
                            const ::rtl::OUString& rStrmName,
                            const ::rtl::OUString& rSysFileName );

private:
    VbaSharedData&      mrVbaData;
};

// ============================================================================

class VbaFormStorageObject : public OcxFormStorageObject
{
public:
    explicit            VbaFormStorageObject(
                            const ObjectBase& rParent,
                            const StorageRef& rxStrg,
                            const ::rtl::OUString& rSysPath,
                            VbaSharedData& rVbaData );

protected:
    virtual void        implDumpStream(
                            const BinaryInputStreamRef& rxStrm,
                            const ::rtl::OUString& rStrgPath,
                            const ::rtl::OUString& rStrmName,
                            const ::rtl::OUString& rSysFileName );

private:
    VbaSharedData&      mrVbaData;
};

// ============================================================================

class VbaProjectStorageObject : public OleStorageObject
{
public:
    explicit            VbaProjectStorageObject( const ObjectBase& rParent, const StorageRef& rxStrg, const ::rtl::OUString& rSysPath );

protected:
    virtual void        implDumpStream(
                            const BinaryInputStreamRef& rxStrm,
                            const ::rtl::OUString& rStrgPath,
                            const ::rtl::OUString& rStrmName,
                            const ::rtl::OUString& rSysFileName );

    virtual void        implDumpStorage(
                            const StorageRef& rxStrg,
                            const ::rtl::OUString& rStrgPath,
                            const ::rtl::OUString& rSysPath );

private:
    VbaSharedData       maVbaData;
};

// ============================================================================
// ============================================================================

} // namespace dump
} // namespace oox

#endif
#endif


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

class OleInputObjectBase : public InputObjectBase
{
protected:
    OleInputObjectBase() {}

    OUString     dumpAnsiString32( const String& rName );
    OUString     dumpUniString32( const String& rName );

    sal_Int32           dumpStdClipboardFormat( const String& rName = EMPTY_STRING );
    OUString     dumpAnsiString32OrStdClip( const String& rName );
    OUString     dumpUniString32OrStdClip( const String& rName );

    void                writeOleColorItem( const String& rName, sal_uInt32 nColor );
    sal_uInt32          dumpOleColor( const String& rName );
};

// ============================================================================
// ============================================================================

class StdFontObject : public OleInputObjectBase
{
public:
    explicit            StdFontObject( const InputObjectBase& rParent );

protected:
    virtual void        implDump();
};

// ============================================================================

class StdPicObject : public OleInputObjectBase
{
public:
    explicit            StdPicObject( const InputObjectBase& rParent );

protected:
    virtual void        implDump();
};

// ============================================================================
// ============================================================================

class OleStreamObject : public OleInputObjectBase
{
public:
    explicit            OleStreamObject( const ObjectBase& rParent, const BinaryInputStreamRef& rxStrm, const OUString& rSysFileName );
};

// ============================================================================

class OleCompObjObject : public OleStreamObject
{
public:
    explicit            OleCompObjObject( const ObjectBase& rParent, const BinaryInputStreamRef& rxStrm, const OUString& rSysFileName );

protected:
    virtual void        implDump();
};

// ============================================================================
// ============================================================================

class OlePropertyStreamObject : public InputObjectBase
{
public:
    explicit            OlePropertyStreamObject( const ObjectBase& rParent, const BinaryInputStreamRef& rxStrm, const OUString& rSysFileName );

protected:
    virtual void        implDump();

private:
    void                dumpSection( const OUString& rGuid, sal_uInt32 nStartPos );

    void                dumpProperty( sal_Int32 nPropId, sal_uInt32 nStartPos );
    void                dumpCodePageProperty( sal_uInt32 nStartPos );
    void                dumpDictionaryProperty( sal_uInt32 nStartPos );

    sal_uInt16          dumpPropertyContents( sal_Int32 nPropId );
    void                dumpPropertyValue( sal_Int32 nPropId, sal_uInt16 nBaseType );
    void                dumpPropertyVector( sal_Int32 nPropId, sal_uInt16 nBaseType );
    void                dumpPropertyArray( sal_Int32 nPropId, sal_uInt16 nBaseType );

    sal_uInt16          dumpPropertyType();
    void                dumpBlob( sal_Int32 nPropId, const String& rName );
    OUString     dumpString8( const String& rName );
    OUString     dumpCharArray8( const String& rName, sal_Int32 nLen );
    OUString     dumpString16( const String& rName );
    OUString     dumpCharArray16( const String& rName, sal_Int32 nLen );
    bool                dumpTypedProperty( const String& rName, sal_uInt16 nExpectedType );
    void                dumpHlinks( sal_Int32 nSize );

    bool                startElement( sal_uInt32 nStartPos );
    void                writeSectionHeader( const OUString& rGuid, sal_uInt32 nStartPos );
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
    explicit            OleStorageObject( const ObjectBase& rParent, const StorageRef& rxStrg, const OUString& rSysPath );

protected:
    OleStorageObject() {}

    using               StorageObjectBase::construct;
    void                construct( const ObjectBase& rParent, const StorageRef& rxStrg, const OUString& rSysPath );

    virtual void        implDumpStream(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& rxStrm,
                            const OUString& rStrgPath,
                            const OUString& rStrmName,
                            const OUString& rSysFileName );
};

// ============================================================================
// ============================================================================

class ComCtlObjectBase : public OleInputObjectBase
{
protected:
    explicit            ComCtlObjectBase(
                            const InputObjectBase& rParent,
                            sal_uInt32 nDataId5, sal_uInt32 nDataId6, sal_uInt16 nVersion,
                            bool bCommonPart, bool bComplexPart );

    virtual void        implDump();
    virtual void        implDumpProperties() = 0;
    virtual void        implDumpCommonExtra( sal_Int64 nEndPos );
    virtual void        implDumpCommonTrailing();

private:
    bool                dumpComCtlHeader( sal_uInt32 nExpId, sal_uInt16 nExpMajor = SAL_MAX_UINT16, sal_uInt16 nExpMinor = SAL_MAX_UINT16 );
    bool                dumpComCtlSize();
    bool                dumpComCtlData( sal_uInt32& ornCommonPartSize );
    bool                dumpComCtlCommon( sal_uInt32 nPartSize );
    bool                dumpComCtlComplex();

protected:
    sal_uInt32          mnDataId5;
    sal_uInt32          mnDataId6;
    sal_uInt16          mnVersion;
    bool                mbCommonPart;
    bool                mbComplexPart;
};

// ============================================================================

class ComCtlScrollBarObject : public ComCtlObjectBase
{
public:
    explicit            ComCtlScrollBarObject( const InputObjectBase& rParent, sal_uInt16 nVersion );

protected:
    virtual void        implDumpProperties();
};

// ============================================================================

class ComCtlProgressBarObject : public ComCtlObjectBase
{
public:
    explicit            ComCtlProgressBarObject( const InputObjectBase& rParent, sal_uInt16 nVersion );

protected:
    virtual void        implDumpProperties();
};

// ============================================================================

class ComCtlSliderObject : public ComCtlObjectBase
{
public:
    explicit            ComCtlSliderObject( const InputObjectBase& rParent, sal_uInt16 nVersion );

protected:
    virtual void        implDumpProperties();
};

// ============================================================================

class ComCtlUpDownObject : public ComCtlObjectBase
{
public:
    explicit            ComCtlUpDownObject( const InputObjectBase& rParent, sal_uInt16 nVersion );

protected:
    virtual void        implDumpProperties();
};

// ============================================================================

class ComCtlImageListObject : public ComCtlObjectBase
{
public:
    explicit            ComCtlImageListObject( const InputObjectBase& rParent, sal_uInt16 nVersion );

protected:
    virtual void        implDumpProperties();
    virtual void        implDumpCommonExtra( sal_Int64 nEndPos );
    virtual void        implDumpCommonTrailing();
};

// ============================================================================

class ComCtlTabStripObject : public ComCtlObjectBase
{
public:
    explicit            ComCtlTabStripObject( const InputObjectBase& rParent, sal_uInt16 nVersion );

protected:
    virtual void        implDumpProperties();
    virtual void        implDumpCommonExtra( sal_Int64 nEndPos );
};

// ============================================================================

class ComCtlTreeViewObject : public ComCtlObjectBase
{
public:
    explicit            ComCtlTreeViewObject( const InputObjectBase& rParent, sal_uInt16 nVersion );

protected:
    virtual void        implDumpProperties();
    virtual void        implDumpCommonExtra( sal_Int64 nEndPos );

private:
    sal_uInt32          mnStringFlags;
};

// ============================================================================

class ComCtlStatusBarObject : public ComCtlObjectBase
{
public:
    explicit            ComCtlStatusBarObject( const InputObjectBase& rParent, sal_uInt16 nVersion );

protected:
    virtual void        implDumpProperties();
    virtual void        implDumpCommonExtra( sal_Int64 nEndPos );
    virtual void        implDumpCommonTrailing();
};

// ============================================================================
// ============================================================================

class AxPropertyObjectBase : public OleInputObjectBase
{
protected:
    AxPropertyObjectBase() {}

    using               OleInputObjectBase::construct;
    void                construct(
                            const ObjectBase& rParent,
                            const BinaryInputStreamRef& rxStrm,
                            const OUString& rSysFileName,
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
    OUString     getPropertyName() const;

    template< typename Type >
    Type                dumpDecProperty( Type nDefault, const NameListWrapper& rListWrp = NO_LIST );
    template< typename Type >
    Type                dumpHexProperty( Type nDefault, const NameListWrapper& rListWrp = NO_LIST );

    bool         dumpBoolProperty() { return startNextProperty(); }
    sal_Int32    dumpHmmProperty() { return dumpDecProperty< sal_Int32 >( 0, "CONV-HMM-TO-CM" ); }
    sal_uInt8    dumpMousePtrProperty() { return dumpDecProperty< sal_uInt8 >( 0, "OLE-MOUSEPTR" ); }
    template< typename Type >
    Type         dumpBorderStyleProperty( Type nDefault ) { return dumpDecProperty< Type >( nDefault, "AX-BORDERSTYLE" ); }
    template< typename Type >
    Type         dumpSpecialEffectProperty( Type nDefault ) { return dumpDecProperty< Type >( nDefault, "AX-SPECIALEFFECT" ); }
    sal_uInt32   dumpEnabledProperty() { return dumpDecProperty< sal_uInt32 >( 1, "AX-ENABLED" ); }
    sal_Int32    dumpOrientationProperty() { return dumpDecProperty< sal_Int32 >( -1, "AX-ORIENTATION" ); }
    sal_Int32    dumpDelayProperty() { return dumpDecProperty< sal_Int32 >( 50, "AX-CONV-MS" ); }
    sal_uInt32   dumpImagePosProperty() { return dumpHexProperty< sal_uInt32 >( 0x00070001, "AX-IMAGEPOS" ); }
    sal_uInt8    dumpImageSizeModeProperty() { return dumpDecProperty< sal_uInt8 >( 0, "AX-IMAGESIZEMODE" ); }
    sal_uInt8    dumpImageAlignProperty() { return dumpDecProperty< sal_uInt8 >( 2, "AX-IMAGEALIGN" ); }

    sal_uInt32          dumpFlagsProperty( sal_uInt32 nDefault, const sal_Char* pcNameList = "AX-FLAGS" );
    sal_uInt32          dumpColorProperty( sal_uInt32 nDefault );
    sal_Unicode         dumpUnicodeProperty();
    void                dumpUnknownProperty();

    void                dumpPosProperty();
    void                dumpSizeProperty();
    void                dumpGuidProperty( OUString* pValue = 0 );
    void                dumpStringProperty( OUString* pValue = 0 );
    void                dumpStringArrayProperty();
    void                dumpStreamProperty();

    void                dumpEmbeddedFont();
    void                dumpToPosition( sal_Int64 nPos );

private:
    void                constructAxPropObj( const String& rPropNameList, bool b64BitPropFlags );

    void                dumpVersion();
    OUString            dumpString( const String& rName, sal_uInt32 nSize, bool bArray );
    void                dumpShortProperties();
    void                dumpLargeProperties();

private:
    struct LargeProperty
    {
        enum LargePropertyType { PROPTYPE_POS, PROPTYPE_SIZE, PROPTYPE_GUID, PROPTYPE_STRING, PROPTYPE_STRINGARRAY };

        LargePropertyType   mePropType;
        OUString     maItemName;
        sal_uInt32          mnDataSize;
        OUString*    mpItemValue;
        explicit     LargeProperty( LargePropertyType ePropType, const String& rItemName, sal_uInt32 nDataSize, OUString* pItemValue = 0 ) :
                                mePropType( ePropType ), maItemName( rItemName ), mnDataSize( nDataSize ), mpItemValue( pItemValue ) {}
    };
    typedef ::std::vector< LargeProperty > LargePropertyVector;

    struct StreamProperty
    {
        OUString     maItemName;
        sal_uInt16          mnData;
        explicit     StreamProperty( const String& rItemName, sal_uInt16 nData ) :
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
void AxPropertyObjectBase::alignInput()
{
    mxStrm->skip( (sizeof( Type ) - ((mxStrm->tell() - mnPropertiesStart) % sizeof( Type ))) % sizeof( Type ) );
}

template< typename Type >
Type AxPropertyObjectBase::dumpDecProperty( Type nDefault, const NameListWrapper& rListWrp )
{
    if( startNextProperty() )
    {
        alignInput< Type >();
        return dumpDec< Type >( getPropertyName(), rListWrp );
    }
    return nDefault;
}

template< typename Type >
Type AxPropertyObjectBase::dumpHexProperty( Type nDefault, const NameListWrapper& rListWrp )
{
    if( startNextProperty() )
    {
        alignInput< Type >();
        return dumpHex< Type >( getPropertyName(), rListWrp );
    }
    return nDefault;
}

// ============================================================================

class AxCFontNewObject : public AxPropertyObjectBase
{
public:
    explicit            AxCFontNewObject( const InputObjectBase& rParent );

protected:
    virtual void        implDumpShortProperties();
};

// ============================================================================

class AxColumnInfoObject : public AxPropertyObjectBase
{
public:
    explicit            AxColumnInfoObject( const InputObjectBase& rParent );

protected:
    virtual void        implDumpShortProperties();
};

// ============================================================================

class AxCommandButtonObject : public AxPropertyObjectBase
{
public:
    explicit            AxCommandButtonObject( const InputObjectBase& rParent );

protected:
    virtual void        implDumpShortProperties();
    virtual void        implDumpExtended();
};

// ============================================================================

class AxMorphControlObject : public AxPropertyObjectBase
{
public:
    explicit            AxMorphControlObject( const InputObjectBase& rParent );

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

class AxLabelObject : public AxPropertyObjectBase
{
public:
    explicit            AxLabelObject( const InputObjectBase& rParent );

protected:
    virtual void        implDumpShortProperties();
    virtual void        implDumpExtended();
};

// ============================================================================

class AxImageObject : public AxPropertyObjectBase
{
public:
    explicit            AxImageObject( const InputObjectBase& rParent );

protected:
    virtual void        implDumpShortProperties();
};

// ============================================================================

class AxScrollBarObject : public AxPropertyObjectBase
{
public:
    explicit            AxScrollBarObject( const InputObjectBase& rParent );

protected:
    virtual void        implDumpShortProperties();
};

// ============================================================================

class AxSpinButtonObject : public AxPropertyObjectBase
{
public:
    explicit            AxSpinButtonObject( const InputObjectBase& rParent );

protected:
    virtual void        implDumpShortProperties();
};

// ============================================================================

class AxTabStripObject : public AxPropertyObjectBase
{
public:
    explicit            AxTabStripObject( const InputObjectBase& rParent );

protected:
    virtual void        implDumpShortProperties();
    virtual void        implDumpExtended();

private:
    sal_Int32           mnTabFlagCount;
};

// ============================================================================
// ============================================================================

class FormControlStreamObject : public OleInputObjectBase
{
public:
    explicit            FormControlStreamObject(
                            const ObjectBase& rParent,
                            const BinaryInputStreamRef& rxStrm,
                            const OUString& rSysFileName,
                            const OUString* pProgId = 0 );
    explicit            FormControlStreamObject(
                            const OutputObjectBase& rParent,
                            const BinaryInputStreamRef& rxStrm,
                            const OUString* pProgId = 0 );

protected:
    virtual void        implDump();

private:
    void                constructFormCtrlStrmObj( const OUString* pProgId );

private:
    OUString     maProgId;
    bool                mbReadGuid;
};

// ============================================================================
// ============================================================================

struct VbaFormSiteInfo
{
    OUString     maProgId;
    sal_Int32           mnId;
    sal_uInt32          mnLength;
    bool                mbInStream;

    VbaFormSiteInfo() : mnId( 0 ), mnLength( 0 ), mbInStream( false ) {}
};

typedef ::std::vector< VbaFormSiteInfo > VbaFormSiteInfoVector;

// ============================================================================

struct VbaFormSharedData
{
    OUStringVector      maClassInfoProgIds;
    VbaFormSiteInfoVector maSiteInfos;
};

// ============================================================================

class VbaFormClassInfoObject : public AxPropertyObjectBase
{
public:
    explicit            VbaFormClassInfoObject( const InputObjectBase& rParent, VbaFormSharedData& rFormData );

protected:
    virtual void        implDumpShortProperties();

private:
    VbaFormSharedData&  mrFormData;
};

// ============================================================================

class VbaFormSiteObject : public AxPropertyObjectBase
{
public:
    explicit            VbaFormSiteObject( const InputObjectBase& rParent, VbaFormSharedData& rFormData );

protected:
    virtual void        implDumpShortProperties();

private:
    VbaFormSharedData&  mrFormData;
};

// ============================================================================

class VbaFormDesignExtObject : public AxPropertyObjectBase
{
public:
    explicit            VbaFormDesignExtObject( const InputObjectBase& rParent );

protected:
    virtual void        implDumpShortProperties();
};

// ============================================================================

class VbaFStreamObject : public AxPropertyObjectBase
{
public:
    explicit            VbaFStreamObject(
                            const ObjectBase& rParent,
                            const BinaryInputStreamRef& rxStrm,
                            const OUString& rSysFileName,
                            VbaFormSharedData& rFormData );

protected:
    virtual void        implDumpShortProperties();
    virtual void        implDumpExtended();

private:
    void                dumpClassInfos();
    void                dumpFormSites( sal_uInt32 nCount );
    void                dumpSiteData();
    void                dumpDesignExtender();

private:
    VbaFormSharedData&  mrFormData;
    sal_uInt32          mnFlags;
};

// ============================================================================

class VbaOStreamObject : public OleInputObjectBase
{
public:
    explicit            VbaOStreamObject(
                            const ObjectBase& rParent,
                            const BinaryInputStreamRef& rxStrm,
                            const OUString& rSysFileName,
                            VbaFormSharedData& rFormData );

protected:
    virtual void        implDump();

private:
    VbaFormSharedData&  mrFormData;
};

// ============================================================================

class VbaPageObject : public AxPropertyObjectBase
{
public:
    explicit            VbaPageObject( const InputObjectBase& rParent );

protected:
    virtual void        implDumpShortProperties();
};

// ============================================================================

class VbaMultiPageObject : public AxPropertyObjectBase
{
public:
    explicit            VbaMultiPageObject( const InputObjectBase& rParent );

protected:
    virtual void        implDumpShortProperties();
    virtual void        implDumpExtended();

private:
    sal_Int32           mnPageCount;
};

// ============================================================================

class VbaXStreamObject : public InputObjectBase
{
public:
    explicit            VbaXStreamObject(
                            const ObjectBase& rParent,
                            const BinaryInputStreamRef& rxStrm,
                            const OUString& rSysFileName,
                            VbaFormSharedData& rFormData );

protected:
    virtual void        implDump();

private:
    VbaFormSharedData&  mrFormData;
};

// ============================================================================

class VbaContainerStorageObject : public OleStorageObject
{
public:
    explicit            VbaContainerStorageObject(
                            const ObjectBase& rParent,
                            const StorageRef& rxStrg,
                            const OUString& rSysPath );

protected:
    virtual void        implDumpStream(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& rxStrm,
                            const OUString& rStrgPath,
                            const OUString& rStrmName,
                            const OUString& rSysFileName );

    virtual void        implDumpStorage(
                            const StorageRef& rxStrg,
                            const OUString& rStrgPath,
                            const OUString& rSysPath );

private:
    bool                isFormStorage( const OUString& rStrgPath ) const;

private:
    VbaFormSharedData   maFormData;
};

// ============================================================================
// ============================================================================

struct VbaSharedData
{
    typedef ::std::map< OUString, sal_Int32 > StreamOffsetMap;

    StreamOffsetMap     maStrmOffsets;
    rtl_TextEncoding    meTextEnc;

                        VbaSharedData();

    bool                isModuleStream( const OUString& rStrmName ) const;
    sal_Int32           getStreamOffset( const OUString& rStrmName ) const;
};

// ============================================================================

class VbaDirStreamObject : public SequenceRecordObjectBase
{
public:
    explicit            VbaDirStreamObject(
                            const ObjectBase& rParent,
                            const BinaryInputStreamRef& rxStrm,
                            const OUString& rSysFileName,
                            VbaSharedData& rVbaData );

protected:
    virtual bool        implIsValid() const;
    virtual bool        implReadRecordHeader( BinaryInputStream& rBaseStrm, sal_Int64& ornRecId, sal_Int64& ornRecSize );
    virtual void        implDumpRecordBody();

private:
    OUString     dumpByteString( const String& rName = EMPTY_STRING );
    OUString     dumpUniString( const String& rName = EMPTY_STRING );

    OUString     dumpByteStringWithLength( const String& rName = EMPTY_STRING );

private:
    VbaSharedData&      mrVbaData;
    BinaryInputStreamRef mxInStrm;
    OUString     maCurrStream;
    sal_Int32           mnCurrOffset;
};

// ============================================================================

class VbaModuleStreamObject : public InputObjectBase
{
public:
    explicit            VbaModuleStreamObject(
                            const ObjectBase& rParent,
                            const BinaryInputStreamRef& rxStrm,
                            const OUString& rSysFileName,
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
                            const OUString& rSysPath,
                            VbaSharedData& rVbaData );

protected:
    virtual void        implDumpStream(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& rxStrm,
                            const OUString& rStrgPath,
                            const OUString& rStrmName,
                            const OUString& rSysFileName );

private:
    VbaSharedData&      mrVbaData;
};

// ============================================================================

class VbaFormStorageObject : public VbaContainerStorageObject
{
public:
    explicit            VbaFormStorageObject(
                            const ObjectBase& rParent,
                            const StorageRef& rxStrg,
                            const OUString& rSysPath,
                            VbaSharedData& rVbaData );

protected:
    virtual void        implDumpStream(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& rxStrm,
                            const OUString& rStrgPath,
                            const OUString& rStrmName,
                            const OUString& rSysFileName );

private:
    VbaSharedData&      mrVbaData;
};

// ============================================================================

class VbaProjectStorageObject : public OleStorageObject
{
public:
    explicit            VbaProjectStorageObject( const ObjectBase& rParent, const StorageRef& rxStrg, const OUString& rSysPath );

protected:
    virtual void        implDumpStream(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >& rxStrm,
                            const OUString& rStrgPath,
                            const OUString& rStrmName,
                            const OUString& rSysFileName );

    virtual void        implDumpStorage(
                            const StorageRef& rxStrg,
                            const OUString& rStrgPath,
                            const OUString& rSysPath );

private:
    VbaSharedData       maVbaData;
};

// ============================================================================
// ============================================================================

class ActiveXStorageObject : public VbaContainerStorageObject
{
public:
    explicit            ActiveXStorageObject(
                            const ObjectBase& rParent,
                            const StorageRef& rxStrg,
                            const OUString& rSysPath );

protected:
    virtual void        implDumpBaseStream(
                            const BinaryInputStreamRef& rxStrm,
                            const OUString& rSysFileName );
};

// ============================================================================
// ============================================================================

} // namespace dump
} // namespace oox

#endif
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

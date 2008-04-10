/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: fdumperole.hxx,v $
 * $Revision: 1.6 $
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

#ifndef SC_FDUMPEROLE_HXX
#define SC_FDUMPEROLE_HXX

#include "fdumper.hxx"

#if SCF_INCL_DUMPER
#include <sot/storage.hxx>
#include <sot/storinfo.hxx>

namespace scf {
namespace dump {

// ============================================================================
// ============================================================================

class OleStorageObject : public ObjectBase
{
public:
    explicit            OleStorageObject( const OleStorageObject& rParentStrg, const String& rStrgName );
    explicit            OleStorageObject( const ObjectBase& rParent, SotStorageRef xRootStrg );
    explicit            OleStorageObject( const ObjectBase& rParent, SvStream& rRootStrm );
    explicit            OleStorageObject( const ObjectBase& rParent );
    virtual             ~OleStorageObject();

    inline SotStorageRef GetStorage() const { return mxStrg; }
    inline const String& GetStoragePath() const { return maPath; }
    inline const String& GetStorageName() const { return maName; }
    String              GetFullName() const;

    void                ExtractStorageToFileSystem();

protected:
    inline explicit     OleStorageObject() {}
    void                Construct( const ObjectBase& rParent, SotStorageRef xStrg, const String& rPath );
    void                Construct( const OleStorageObject& rParentStrg, const String& rStrgName );
    void                Construct( const ObjectBase& rParent, SvStream& rRootStrm );
    void                Construct( const ObjectBase& rParent );

    virtual bool        ImplIsValid() const;
    virtual void        ImplDumpHeader();
    virtual void        ImplDumpFooter();

    using               ObjectBase::Construct;

private:
    void                DumpStorageInfo( bool bExtended );

private:
    SotStorageRef       mxStrg;
    String              maPath;
    String              maName;
};

typedef ScfRef< OleStorageObject > OleStorageObjectRef;

// ============================================================================

class OleStorageIterator : public Base
{
public:
    explicit            OleStorageIterator( const OleStorageObject& rStrg );
    explicit            OleStorageIterator( SotStorageRef xStrg );
                        ~OleStorageIterator();

    ULONG               GetSize() const;

    OleStorageIterator& operator++();
    const SvStorageInfo* operator->() const;

protected:
    void                Construct( SotStorageRef xStrg );

private:
    virtual bool        ImplIsValid() const;

private:
    typedef ScfRef< SvStorageInfoList > SvStorageInfoListRef;
    SvStorageInfoListRef mxInfoList;
    ULONG               mnIndex;
};

// ============================================================================
// ============================================================================

class OleStreamObject : public StreamObjectBase
{
public:
    explicit            OleStreamObject( const OleStorageObject& rParentStrg, const String& rStrmName );
    virtual             ~OleStreamObject();

protected:
    inline explicit     OleStreamObject() {}
    void                Construct( const OleStorageObject& rParentStrg, const String& rStrmName );

    virtual bool        ImplIsValid() const;

    using               StreamObjectBase::Construct;

private:
    SotStorageStreamRef mxStrm;
};

typedef ScfRef< OleStreamObject > OleStreamObjectRef;

// ============================================================================

class OlePropertyStreamObject : public OleStreamObject
{
public:
    explicit            OlePropertyStreamObject( const OleStorageObject& rParentStrg, const String& rStrmName );

protected:
    inline explicit     OlePropertyStreamObject() {}
    void                Construct( const OleStorageObject& rParentStrg, const String& rStrmName );

    virtual void        ImplDumpBody();

    using               OleStreamObject::Construct;

private:
    void                DumpSection( const String& rGuid, sal_uInt32 nStartPos );

    void                DumpProperty( sal_Int32 nPropId, sal_uInt32 nStartPos );
    void                DumpCodePageProperty( sal_uInt32 nStartPos );
    void                DumpDictionaryProperty( sal_uInt32 nStartPos );

    void                DumpPropertyContents( sal_Int32 nPropId );
    void                DumpPropertyValue( sal_Int32 nPropId, sal_Int32 nBaseType );

    sal_Int32           DumpPropertyType();
    void                DumpBlob( const sal_Char* pcName );
    String              DumpString8( const sal_Char* pcName );
    String              DumpCharArray8( const sal_Char* pcName, sal_Int32 nCharCount );
    String              DumpString16( const sal_Char* pcName );
    String              DumpCharArray16( const sal_Char* pcName, sal_Int32 nCharCount );
    DateTime            DumpFileTime( const sal_Char* pcName );

    bool                StartElement( sal_uInt32 nStartPos );
    void                WriteSectionHeader( const String& rGuid, sal_uInt32 nStartPos );
    void                WritePropertyHeader( sal_Int32 nPropId, sal_uInt32 nStartPos );

private:
    NameListRef         mxPropIds;
    rtl_TextEncoding    meTextEnc;
    bool                mbIsUnicode;
};

// ============================================================================
// ============================================================================

} // namespace dump
} // namespace scf

#endif
#endif


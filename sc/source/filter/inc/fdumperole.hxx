/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fdumperole.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2006-07-10 13:52:09 $
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

#ifndef SC_FDUMPEROLE_HXX
#define SC_FDUMPEROLE_HXX

#ifndef SC_FDUMPER_HXX
#include "fdumper.hxx"
#endif

#if SCF_INCL_DUMPER

#ifndef _SOT_STORAGE_HXX
#include <sot/storage.hxx>
#endif

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

private:
    void                DumpStorageInfo( bool bExtended );

private:
    SotStorageRef       mxStrg;
    String              maPath;
    String              maName;
};

typedef ScfRef< OleStorageObject > OleStorageObjectRef;

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

private:
    SotStorageStreamRef mxStrm;
};

typedef ScfRef< OleStreamObject > OleStreamObjectRef;

// ============================================================================

class OlePropertyStreamObject : public OleStreamObject
{
public:
    explicit            OlePropertyStreamObject( const OleStorageObject& rParentStrg, const String& rStrmName );

    static void         InitializeConfig( Config& rCfg );

protected:
    inline explicit     OlePropertyStreamObject() {}
    void                Construct( const OleStorageObject& rParentStrg, const String& rStrmName );

    virtual void        ImplDumpBody();

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


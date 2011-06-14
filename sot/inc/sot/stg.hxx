/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef _STG_HXX
#define _STG_HXX

#include <com/sun/star/uno/Any.h>
#include <com/sun/star/uno/Reference.h>

#include <com/sun/star/io/XInputStream.hpp>

#include <com/sun/star/ucb/XCommandEnvironment.hpp>

#include <com/sun/star/embed/XStorage.hpp>


#include <tools/rtti.hxx>
#include <tools/stream.hxx>
#include <tools/globname.hxx>
#include "sot/sotdllapi.h"

#include <list>
class UNOStorageHolder;
typedef ::std::list< UNOStorageHolder* > UNOStorageHolderList;

class Storage;
class StorageStream;
class StgIo;
class StgDirEntry;
class StgStrm;
class SvGlobalName;
struct ClsId
{
    sal_Int32   n1;
    sal_Int16   n2, n3;
    sal_uInt8   n4, n5, n6, n7, n8, n9, n10, n11;
};

class SOT_DLLPUBLIC StorageBase : public SvRefBase
{
protected:
    sal_uLong           m_nError;                   // error code
    StreamMode      m_nMode;                    // open mode
    sal_Bool            m_bAutoCommit;
                    StorageBase();
    virtual         ~StorageBase();
public:
                    TYPEINFO();
    virtual const SvStream* GetSvStream() const = 0;
    virtual sal_Bool    Validate( sal_Bool=sal_False ) const = 0;
    virtual sal_Bool    ValidateMode( StreamMode ) const = 0;
    void            ResetError() const;
    void            SetError( sal_uLong ) const;
    sal_uLong           GetError() const;
    sal_Bool            Good() const          { return sal_Bool( m_nError == SVSTREAM_OK ); }
    StreamMode      GetMode() const  { return m_nMode;  }
    void            SetAutoCommit( sal_Bool bSet )
                    { m_bAutoCommit = bSet; }
};

class BaseStorageStream : public StorageBase
{
public:
                    TYPEINFO();
    virtual sal_uLong   Read( void * pData, sal_uLong nSize ) = 0;
    virtual sal_uLong   Write( const void* pData, sal_uLong nSize ) = 0;
    virtual sal_uLong   Seek( sal_uLong nPos ) = 0;
    virtual sal_uLong   Tell() = 0;
    virtual void    Flush() = 0;
    virtual sal_Bool    SetSize( sal_uLong nNewSize ) = 0;
    virtual sal_Bool    CopyTo( BaseStorageStream * pDestStm ) = 0;
    virtual sal_Bool    Commit() = 0;
    virtual sal_Bool    Revert() = 0;
    virtual sal_Bool    Equals( const BaseStorageStream& rStream ) const = 0;
};

class SvStorageInfoList;
class BaseStorage : public StorageBase
{
public:
                                TYPEINFO();
    virtual const String&       GetName() const = 0;
    virtual sal_Bool                IsRoot() const = 0;
    virtual void                SetClassId( const ClsId& ) = 0;
    virtual const ClsId&        GetClassId() const = 0;
    virtual void                SetDirty() = 0;
    virtual void                SetClass( const SvGlobalName & rClass,
                                    sal_uLong nOriginalClipFormat,
                                    const String & rUserTypeName ) = 0;
    virtual void                SetConvertClass( const SvGlobalName & rConvertClass,
                                           sal_uLong nOriginalClipFormat,
                                           const String & rUserTypeName ) = 0;
    virtual SvGlobalName        GetClassName() = 0;
    virtual sal_uLong               GetFormat() = 0;
    virtual String              GetUserName() = 0;
    virtual sal_Bool                ShouldConvert() = 0;
    virtual void                FillInfoList( SvStorageInfoList* ) const = 0;
    virtual sal_Bool                CopyTo( BaseStorage* pDestStg ) const = 0;
    virtual sal_Bool                Commit() = 0;
    virtual sal_Bool                Revert() = 0;
    virtual BaseStorageStream*  OpenStream( const String & rEleName,
                                  StreamMode = STREAM_STD_READWRITE,
                                  sal_Bool bDirect = sal_True, const ByteString* pKey=0 ) = 0;
    virtual BaseStorage*        OpenStorage( const String & rEleName,
                                   StreamMode = STREAM_STD_READWRITE,
                                   sal_Bool bDirect = sal_False ) = 0;
    virtual BaseStorage*        OpenUCBStorage( const String & rEleName,
                                   StreamMode = STREAM_STD_READWRITE,
                                   sal_Bool bDirect = sal_False ) = 0;
    virtual BaseStorage*        OpenOLEStorage( const String & rEleName,
                                   StreamMode = STREAM_STD_READWRITE,
                                   sal_Bool bDirect = sal_False ) = 0;
    virtual sal_Bool                IsStream( const String& rEleName ) const = 0;
    virtual sal_Bool                IsStorage( const String& rEleName ) const = 0;
    virtual sal_Bool                IsContained( const String& rEleName ) const = 0;
    virtual sal_Bool                Remove( const String & rEleName ) = 0;
    virtual sal_Bool                Rename( const String & rEleName, const String & rNewName ) = 0;
    virtual sal_Bool                CopyTo( const String & rEleName, BaseStorage * pDest, const String & rNewName ) = 0;
    virtual sal_Bool                MoveTo( const String & rEleName, BaseStorage * pDest, const String & rNewName ) = 0;
    virtual sal_Bool                ValidateFAT() = 0;
    virtual sal_Bool                Equals( const BaseStorage& rStream ) const = 0;
};

class OLEStorageBase
{
protected:
    StreamMode&     nStreamMode;              // open mode
    StgIo*          pIo;                      // I/O subsystem
    StgDirEntry*    pEntry;                   // the dir entry
                    OLEStorageBase( StgIo*, StgDirEntry*, StreamMode& );
                    ~OLEStorageBase();
    sal_Bool            Validate_Impl( sal_Bool=sal_False ) const;
    sal_Bool            ValidateMode_Impl( StreamMode, StgDirEntry* p = NULL ) const ;
    const SvStream* GetSvStream_Impl() const;
public:
};

class StorageStream : public BaseStorageStream, public OLEStorageBase
{
//friend class Storage;
    sal_uLong           nPos;                             // current position
protected:
                    ~StorageStream();
public:
                    TYPEINFO();
                    StorageStream( StgIo*, StgDirEntry*, StreamMode );
    virtual sal_uLong   Read( void * pData, sal_uLong nSize );
    virtual sal_uLong   Write( const void* pData, sal_uLong nSize );
    virtual sal_uLong   Seek( sal_uLong nPos );
    virtual sal_uLong   Tell() { return nPos; }
    virtual void    Flush();
    virtual sal_Bool    SetSize( sal_uLong nNewSize );
    virtual sal_Bool    CopyTo( BaseStorageStream * pDestStm );
    virtual sal_Bool    Commit();
    virtual sal_Bool    Revert();
    virtual sal_Bool    Validate( sal_Bool=sal_False ) const;
    virtual sal_Bool    ValidateMode( StreamMode ) const;
    sal_Bool            ValidateMode( StreamMode, StgDirEntry* p ) const;
    const SvStream* GetSvStream() const;
    virtual sal_Bool    Equals( const BaseStorageStream& rStream ) const;
};

class UCBStorageStream;

class SOT_DLLPUBLIC Storage : public BaseStorage, public OLEStorageBase
{
    String                      aName;
    sal_Bool                        bIsRoot;
    void                        Init( sal_Bool bCreate );
                                Storage( StgIo*, StgDirEntry*, StreamMode );
protected:
                                ~Storage();
public:
                                TYPEINFO();
                                Storage( const String &, StreamMode = STREAM_STD_READWRITE, sal_Bool bDirect = sal_True );
                                Storage( SvStream& rStrm, sal_Bool bDirect = sal_True );
                                Storage( UCBStorageStream& rStrm, sal_Bool bDirect = sal_True );

    static sal_Bool                 IsStorageFile( const String & rFileName );
    static sal_Bool                 IsStorageFile( SvStream* );

    virtual const String&       GetName() const;
    virtual sal_Bool                IsRoot() const { return bIsRoot; }
    virtual void                SetClassId( const ClsId& );
    virtual const ClsId&        GetClassId() const;
    virtual void                SetDirty();
    virtual void                SetClass( const SvGlobalName & rClass,
                                    sal_uLong nOriginalClipFormat,
                                    const String & rUserTypeName );
    virtual void                SetConvertClass( const SvGlobalName & rConvertClass,
                                           sal_uLong nOriginalClipFormat,
                                           const String & rUserTypeName );
    virtual SvGlobalName        GetClassName();
    virtual sal_uLong               GetFormat();
    virtual String              GetUserName();
    virtual sal_Bool                ShouldConvert();
    virtual void                FillInfoList( SvStorageInfoList* ) const;
    virtual sal_Bool                CopyTo( BaseStorage* pDestStg ) const;
    virtual sal_Bool                Commit();
    virtual sal_Bool                Revert();
    virtual BaseStorageStream*  OpenStream( const String & rEleName,
                                  StreamMode = STREAM_STD_READWRITE,
                                  sal_Bool bDirect = sal_True, const ByteString* pKey=0 );
    virtual BaseStorage*        OpenStorage( const String & rEleName,
                                       StreamMode = STREAM_STD_READWRITE,
                                       sal_Bool bDirect = sal_False );
    virtual BaseStorage*        OpenUCBStorage( const String & rEleName,
                                   StreamMode = STREAM_STD_READWRITE,
                                   sal_Bool bDirect = sal_False );
    virtual BaseStorage*        OpenOLEStorage( const String & rEleName,
                                   StreamMode = STREAM_STD_READWRITE,
                                   sal_Bool bDirect = sal_False );
    virtual sal_Bool                IsStream( const String& rEleName ) const;
    virtual sal_Bool                IsStorage( const String& rEleName ) const;
    virtual sal_Bool                IsContained( const String& rEleName ) const;
    virtual sal_Bool                Remove( const String & rEleName );
    virtual sal_Bool                Rename( const String & rEleName, const String & rNewName );
    virtual sal_Bool                CopyTo( const String & rEleName, BaseStorage * pDest, const String & rNewName );
    virtual sal_Bool                MoveTo( const String & rEleName, BaseStorage * pDest, const String & rNewName );
    virtual sal_Bool                ValidateFAT();
    virtual sal_Bool                Validate( sal_Bool=sal_False ) const;
    virtual sal_Bool                ValidateMode( StreamMode ) const;
    sal_Bool                        ValidateMode( StreamMode, StgDirEntry* p ) const;
    virtual const SvStream*     GetSvStream() const;
    virtual sal_Bool                Equals( const BaseStorage& rStream ) const;
};

class UCBStorageStream_Impl;
class UCBStorageStream : public BaseStorageStream
{
friend class UCBStorage;

    UCBStorageStream_Impl*
            pImp;
protected:
                                ~UCBStorageStream();
public:
                                TYPEINFO();
                                UCBStorageStream( const String& rName, StreamMode nMode, sal_Bool bDirect, const ByteString* pKey=0 );
                                UCBStorageStream( const String& rName, StreamMode nMode, sal_Bool bDirect, const ByteString* pKey, sal_Bool bRepair, ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XProgressHandler > xProgress );
                                UCBStorageStream( UCBStorageStream_Impl* );

    virtual sal_uLong               Read( void * pData, sal_uLong nSize );
    virtual sal_uLong               Write( const void* pData, sal_uLong nSize );
    virtual sal_uLong               Seek( sal_uLong nPos );
    virtual sal_uLong               Tell();
    virtual void                Flush();
    virtual sal_Bool                SetSize( sal_uLong nNewSize );
    virtual sal_Bool                CopyTo( BaseStorageStream * pDestStm );
    virtual sal_Bool                Commit();
    virtual sal_Bool                Revert();
    virtual sal_Bool                Validate( sal_Bool=sal_False ) const;
    virtual sal_Bool                ValidateMode( StreamMode ) const;
    const SvStream*             GetSvStream() const;
    virtual sal_Bool                Equals( const BaseStorageStream& rStream ) const;
    sal_Bool                        SetProperty( const String& rName, const ::com::sun::star::uno::Any& rValue );
    sal_Bool                        GetProperty( const String& rName, ::com::sun::star::uno::Any& rValue );

    SvStream*                   GetModifySvStream();

    ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > GetXInputStream() const;
};

namespace ucbhelper
{
    class Content;
}

class UCBStorage_Impl;
struct UCBStorageElement_Impl;
class SOT_DLLPUBLIC UCBStorage : public BaseStorage
{
    UCBStorage_Impl*            pImp;

protected:
                                ~UCBStorage();
public:
    static sal_Bool                 IsStorageFile( SvStream* );
    static sal_Bool                 IsStorageFile( const String& rName );
    static sal_Bool                 IsDiskSpannedFile( SvStream* );
    static String               GetLinkedFile( SvStream& );
    static String               CreateLinkFile( const String& rName );

                                UCBStorage( const ::ucbhelper::Content& rContent, const String& rName, StreamMode nMode, sal_Bool bDirect = sal_True, sal_Bool bIsRoot = sal_True );
                                UCBStorage( const String& rName,
                                            StreamMode nMode,
                                            sal_Bool bDirect = sal_True,
                                            sal_Bool bIsRoot = sal_True );

                                UCBStorage( const String& rName,
                                            StreamMode nMode,
                                            sal_Bool bDirect,
                                            sal_Bool bIsRoot,
                                            sal_Bool bIsRepair,
                                            ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XProgressHandler >
                                                xProgressHandler );

                                UCBStorage( UCBStorage_Impl* );
                                UCBStorage( SvStream& rStrm, sal_Bool bDirect = sal_True );

                                TYPEINFO();
    virtual const String&       GetName() const;
    virtual sal_Bool                IsRoot() const;
    virtual void                SetClassId( const ClsId& );
    virtual const ClsId&        GetClassId() const;
    virtual void                SetDirty();
    virtual void                SetClass( const SvGlobalName & rClass,
                                    sal_uLong nOriginalClipFormat,
                                    const String & rUserTypeName );
    virtual void                SetConvertClass( const SvGlobalName & rConvertClass,
                                           sal_uLong nOriginalClipFormat,
                                           const String & rUserTypeName );
    virtual SvGlobalName        GetClassName();
    virtual sal_uLong               GetFormat();
    virtual String              GetUserName();
    virtual sal_Bool                ShouldConvert();
    virtual void                FillInfoList( SvStorageInfoList* ) const;
    virtual sal_Bool                CopyTo( BaseStorage* pDestStg ) const;
    virtual sal_Bool                Commit();
    virtual sal_Bool                Revert();
    virtual BaseStorageStream*  OpenStream( const String & rEleName,
                                  StreamMode = STREAM_STD_READWRITE,
                                  sal_Bool bDirect = sal_True, const ByteString* pKey=0 );
    virtual BaseStorage*        OpenStorage( const String & rEleName,
                                       StreamMode = STREAM_STD_READWRITE,
                                       sal_Bool bDirect = sal_False );
    virtual BaseStorage*        OpenUCBStorage( const String & rEleName,
                                   StreamMode = STREAM_STD_READWRITE,
                                   sal_Bool bDirect = sal_False );
    virtual BaseStorage*        OpenOLEStorage( const String & rEleName,
                                   StreamMode = STREAM_STD_READWRITE,
                                   sal_Bool bDirect = sal_False );
    virtual sal_Bool                IsStream( const String& rEleName ) const;
    virtual sal_Bool                IsStorage( const String& rEleName ) const;
    virtual sal_Bool                IsContained( const String& rEleName ) const;
    virtual sal_Bool                Remove( const String & rEleName );
    virtual sal_Bool                Rename( const String & rEleName, const String & rNewName );
    virtual sal_Bool                CopyTo( const String & rEleName, BaseStorage * pDest, const String & rNewName );
    virtual sal_Bool                MoveTo( const String & rEleName, BaseStorage * pDest, const String & rNewName );
    virtual sal_Bool                ValidateFAT();
    virtual sal_Bool                Validate( sal_Bool=sal_False ) const;
    virtual sal_Bool                ValidateMode( StreamMode ) const;
    virtual const SvStream*     GetSvStream() const;
    virtual sal_Bool                Equals( const BaseStorage& rStream ) const;
    sal_Bool                        SetProperty( const String& rName, const ::com::sun::star::uno::Any& rValue );
    sal_Bool                        GetProperty( const String& rName, ::com::sun::star::uno::Any& rValue );
    sal_Bool                        GetProperty( const String& rEleName, const String& rName, ::com::sun::star::uno::Any& rValue );

    // HACK to avoid incompatible build, can be done since this feature is only for development
    // should be removed before release
    UNOStorageHolderList* GetUNOStorageHolderList();

//#if _SOLAR__PRIVATE
    UCBStorageElement_Impl*     FindElement_Impl( const String& rName ) const;
    sal_Bool                        CopyStorageElement_Impl( UCBStorageElement_Impl& rElement,
                                    BaseStorage* pDest, const String& rNew ) const;
    BaseStorage*                OpenStorage_Impl( const String & rEleName,
                                       StreamMode, sal_Bool bDirect, sal_Bool bForceUCBStorage );
//#endif

};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

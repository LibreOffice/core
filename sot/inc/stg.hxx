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
    INT32   n1;
    INT16   n2, n3;
    UINT8   n4, n5, n6, n7, n8, n9, n10, n11;
};

class SOT_DLLPUBLIC StorageBase : public SvRefBase
{
protected:
    ULONG           m_nError;                   // error code
    StreamMode      m_nMode;                    // open mode
    BOOL            m_bAutoCommit;
                    StorageBase();
    virtual         ~StorageBase();
public:
                    TYPEINFO();
    virtual const SvStream* GetSvStream() const = 0;
    virtual BOOL    Validate( BOOL=FALSE ) const = 0;
    virtual BOOL    ValidateMode( StreamMode ) const = 0;
    void            ResetError() const;
    void            SetError( ULONG ) const;
    ULONG           GetError() const;
    BOOL            Good() const          { return BOOL( m_nError == SVSTREAM_OK ); }
    StreamMode      GetMode() const  { return m_nMode;  }
    void            SetAutoCommit( BOOL bSet )
                    { m_bAutoCommit = bSet; }
};

class BaseStorageStream : public StorageBase
{
public:
                    TYPEINFO();
    virtual ULONG   Read( void * pData, ULONG nSize ) = 0;
    virtual ULONG   Write( const void* pData, ULONG nSize ) = 0;
    virtual ULONG   Seek( ULONG nPos ) = 0;
    virtual ULONG   Tell() = 0;
    virtual void    Flush() = 0;
    virtual BOOL    SetSize( ULONG nNewSize ) = 0;
    virtual BOOL    CopyTo( BaseStorageStream * pDestStm ) = 0;
    virtual BOOL    Commit() = 0;
    virtual BOOL    Revert() = 0;
    virtual BOOL    Equals( const BaseStorageStream& rStream ) const = 0;
};

class SvStorageInfoList;
class BaseStorage : public StorageBase
{
public:
                                TYPEINFO();
    virtual const String&       GetName() const = 0;
    virtual BOOL                IsRoot() const = 0;
    virtual void                SetClassId( const ClsId& ) = 0;
    virtual const ClsId&        GetClassId() const = 0;
    virtual void                SetDirty() = 0;
    virtual void                SetClass( const SvGlobalName & rClass,
                                    ULONG nOriginalClipFormat,
                                    const String & rUserTypeName ) = 0;
    virtual void                SetConvertClass( const SvGlobalName & rConvertClass,
                                           ULONG nOriginalClipFormat,
                                           const String & rUserTypeName ) = 0;
    virtual SvGlobalName        GetClassName() = 0;
    virtual ULONG               GetFormat() = 0;
    virtual String              GetUserName() = 0;
    virtual BOOL                ShouldConvert() = 0;
    virtual void                FillInfoList( SvStorageInfoList* ) const = 0;
    virtual BOOL                CopyTo( BaseStorage* pDestStg ) const = 0;
    virtual BOOL                Commit() = 0;
    virtual BOOL                Revert() = 0;
    virtual BaseStorageStream*  OpenStream( const String & rEleName,
                                  StreamMode = STREAM_STD_READWRITE,
                                  BOOL bDirect = TRUE, const ByteString* pKey=0 ) = 0;
    virtual BaseStorage*        OpenStorage( const String & rEleName,
                                   StreamMode = STREAM_STD_READWRITE,
                                   BOOL bDirect = FALSE ) = 0;
    virtual BaseStorage*        OpenUCBStorage( const String & rEleName,
                                   StreamMode = STREAM_STD_READWRITE,
                                   BOOL bDirect = FALSE ) = 0;
    virtual BaseStorage*        OpenOLEStorage( const String & rEleName,
                                   StreamMode = STREAM_STD_READWRITE,
                                   BOOL bDirect = FALSE ) = 0;
    virtual BOOL                IsStream( const String& rEleName ) const = 0;
    virtual BOOL                IsStorage( const String& rEleName ) const = 0;
    virtual BOOL                IsContained( const String& rEleName ) const = 0;
    virtual BOOL                Remove( const String & rEleName ) = 0;
    virtual BOOL                Rename( const String & rEleName, const String & rNewName ) = 0;
    virtual BOOL                CopyTo( const String & rEleName, BaseStorage * pDest, const String & rNewName ) = 0;
    virtual BOOL                MoveTo( const String & rEleName, BaseStorage * pDest, const String & rNewName ) = 0;
    virtual BOOL                ValidateFAT() = 0;
    virtual BOOL                Equals( const BaseStorage& rStream ) const = 0;
};

class OLEStorageBase
{
protected:
    StreamMode&     nStreamMode;              // open mode
    StgIo*          pIo;                      // I/O subsystem
    StgDirEntry*    pEntry;                   // the dir entry
                    OLEStorageBase( StgIo*, StgDirEntry*, StreamMode& );
                    ~OLEStorageBase();
    BOOL            Validate_Impl( BOOL=FALSE ) const;
    BOOL            ValidateMode_Impl( StreamMode, StgDirEntry* p = NULL ) const ;
    const SvStream* GetSvStream_Impl() const;
public:
};

class StorageStream : public BaseStorageStream, public OLEStorageBase
{
//friend class Storage;
    ULONG           nPos;                             // current position
protected:
                    ~StorageStream();
public:
                    TYPEINFO();
                    StorageStream( StgIo*, StgDirEntry*, StreamMode );
    virtual ULONG   Read( void * pData, ULONG nSize );
    virtual ULONG   Write( const void* pData, ULONG nSize );
    virtual ULONG   Seek( ULONG nPos );
    virtual ULONG   Tell() { return nPos; }
    virtual void    Flush();
    virtual BOOL    SetSize( ULONG nNewSize );
    virtual BOOL    CopyTo( BaseStorageStream * pDestStm );
    virtual BOOL    Commit();
    virtual BOOL    Revert();
    virtual BOOL    Validate( BOOL=FALSE ) const;
    virtual BOOL    ValidateMode( StreamMode ) const;
    BOOL            ValidateMode( StreamMode, StgDirEntry* p ) const;
    const SvStream* GetSvStream() const;
    virtual BOOL    Equals( const BaseStorageStream& rStream ) const;
};

class UCBStorageStream;

class SOT_DLLPUBLIC Storage : public BaseStorage, public OLEStorageBase
{
    String                      aName;
    BOOL                        bIsRoot;
    void                        Init( BOOL bCreate );
                                Storage( StgIo*, StgDirEntry*, StreamMode );
protected:
                                ~Storage();
public:
                                TYPEINFO();
                                Storage( const String &, StreamMode = STREAM_STD_READWRITE, BOOL bDirect = TRUE );
                                Storage( SvStream& rStrm, BOOL bDirect = TRUE );
                                Storage( UCBStorageStream& rStrm, BOOL bDirect = TRUE );

    static BOOL                 IsStorageFile( const String & rFileName );
    static BOOL                 IsStorageFile( SvStream* );

    virtual const String&       GetName() const;
    virtual BOOL                IsRoot() const { return bIsRoot; }
    virtual void                SetClassId( const ClsId& );
    virtual const ClsId&        GetClassId() const;
    virtual void                SetDirty();
    virtual void                SetClass( const SvGlobalName & rClass,
                                    ULONG nOriginalClipFormat,
                                    const String & rUserTypeName );
    virtual void                SetConvertClass( const SvGlobalName & rConvertClass,
                                           ULONG nOriginalClipFormat,
                                           const String & rUserTypeName );
    virtual SvGlobalName        GetClassName();
    virtual ULONG               GetFormat();
    virtual String              GetUserName();
    virtual BOOL                ShouldConvert();
    virtual void                FillInfoList( SvStorageInfoList* ) const;
    virtual BOOL                CopyTo( BaseStorage* pDestStg ) const;
    virtual BOOL                Commit();
    virtual BOOL                Revert();
    virtual BaseStorageStream*  OpenStream( const String & rEleName,
                                  StreamMode = STREAM_STD_READWRITE,
                                  BOOL bDirect = TRUE, const ByteString* pKey=0 );
    virtual BaseStorage*        OpenStorage( const String & rEleName,
                                       StreamMode = STREAM_STD_READWRITE,
                                       BOOL bDirect = FALSE );
    virtual BaseStorage*        OpenUCBStorage( const String & rEleName,
                                   StreamMode = STREAM_STD_READWRITE,
                                   BOOL bDirect = FALSE );
    virtual BaseStorage*        OpenOLEStorage( const String & rEleName,
                                   StreamMode = STREAM_STD_READWRITE,
                                   BOOL bDirect = FALSE );
    virtual BOOL                IsStream( const String& rEleName ) const;
    virtual BOOL                IsStorage( const String& rEleName ) const;
    virtual BOOL                IsContained( const String& rEleName ) const;
    virtual BOOL                Remove( const String & rEleName );
    virtual BOOL                Rename( const String & rEleName, const String & rNewName );
    virtual BOOL                CopyTo( const String & rEleName, BaseStorage * pDest, const String & rNewName );
    virtual BOOL                MoveTo( const String & rEleName, BaseStorage * pDest, const String & rNewName );
    virtual BOOL                ValidateFAT();
    virtual BOOL                Validate( BOOL=FALSE ) const;
    virtual BOOL                ValidateMode( StreamMode ) const;
    BOOL                        ValidateMode( StreamMode, StgDirEntry* p ) const;
    virtual const SvStream*     GetSvStream() const;
    virtual BOOL                Equals( const BaseStorage& rStream ) const;
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
                                UCBStorageStream( const String& rName, StreamMode nMode, BOOL bDirect, const ByteString* pKey=0 );
                                UCBStorageStream( const String& rName, StreamMode nMode, BOOL bDirect, const ByteString* pKey, BOOL bRepair, ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XProgressHandler > xProgress );
                                UCBStorageStream( UCBStorageStream_Impl* );

    virtual ULONG               Read( void * pData, ULONG nSize );
    virtual ULONG               Write( const void* pData, ULONG nSize );
    virtual ULONG               Seek( ULONG nPos );
    virtual ULONG               Tell();
    virtual void                Flush();
    virtual BOOL                SetSize( ULONG nNewSize );
    virtual BOOL                CopyTo( BaseStorageStream * pDestStm );
    virtual BOOL                Commit();
    virtual BOOL                Revert();
    virtual BOOL                Validate( BOOL=FALSE ) const;
    virtual BOOL                ValidateMode( StreamMode ) const;
    const SvStream*             GetSvStream() const;
    virtual BOOL                Equals( const BaseStorageStream& rStream ) const;
    BOOL                        SetProperty( const String& rName, const ::com::sun::star::uno::Any& rValue );
    BOOL                        GetProperty( const String& rName, ::com::sun::star::uno::Any& rValue );

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
    static BOOL                 IsStorageFile( SvStream* );
    static BOOL                 IsStorageFile( const String& rName );
    static BOOL                 IsDiskSpannedFile( SvStream* );
    static String               GetLinkedFile( SvStream& );
    static String               CreateLinkFile( const String& rName );

                                UCBStorage( const ::ucbhelper::Content& rContent, const String& rName, StreamMode nMode, BOOL bDirect = TRUE, BOOL bIsRoot = TRUE );
                                UCBStorage( const String& rName,
                                            StreamMode nMode,
                                            BOOL bDirect = TRUE,
                                            BOOL bIsRoot = TRUE );

                                UCBStorage( const String& rName,
                                            StreamMode nMode,
                                            BOOL bDirect,
                                            BOOL bIsRoot,
                                            BOOL bIsRepair,
                                            ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XProgressHandler >
                                                xProgressHandler );

                                UCBStorage( UCBStorage_Impl* );
                                UCBStorage( SvStream& rStrm, BOOL bDirect = TRUE );

                                TYPEINFO();
    virtual const String&       GetName() const;
    virtual BOOL                IsRoot() const;
    virtual void                SetClassId( const ClsId& );
    virtual const ClsId&        GetClassId() const;
    virtual void                SetDirty();
    virtual void                SetClass( const SvGlobalName & rClass,
                                    ULONG nOriginalClipFormat,
                                    const String & rUserTypeName );
    virtual void                SetConvertClass( const SvGlobalName & rConvertClass,
                                           ULONG nOriginalClipFormat,
                                           const String & rUserTypeName );
    virtual SvGlobalName        GetClassName();
    virtual ULONG               GetFormat();
    virtual String              GetUserName();
    virtual BOOL                ShouldConvert();
    virtual void                FillInfoList( SvStorageInfoList* ) const;
    virtual BOOL                CopyTo( BaseStorage* pDestStg ) const;
    virtual BOOL                Commit();
    virtual BOOL                Revert();
    virtual BaseStorageStream*  OpenStream( const String & rEleName,
                                  StreamMode = STREAM_STD_READWRITE,
                                  BOOL bDirect = TRUE, const ByteString* pKey=0 );
    virtual BaseStorage*        OpenStorage( const String & rEleName,
                                       StreamMode = STREAM_STD_READWRITE,
                                       BOOL bDirect = FALSE );
    virtual BaseStorage*        OpenUCBStorage( const String & rEleName,
                                   StreamMode = STREAM_STD_READWRITE,
                                   BOOL bDirect = FALSE );
    virtual BaseStorage*        OpenOLEStorage( const String & rEleName,
                                   StreamMode = STREAM_STD_READWRITE,
                                   BOOL bDirect = FALSE );
    virtual BOOL                IsStream( const String& rEleName ) const;
    virtual BOOL                IsStorage( const String& rEleName ) const;
    virtual BOOL                IsContained( const String& rEleName ) const;
    virtual BOOL                Remove( const String & rEleName );
    virtual BOOL                Rename( const String & rEleName, const String & rNewName );
    virtual BOOL                CopyTo( const String & rEleName, BaseStorage * pDest, const String & rNewName );
    virtual BOOL                MoveTo( const String & rEleName, BaseStorage * pDest, const String & rNewName );
    virtual BOOL                ValidateFAT();
    virtual BOOL                Validate( BOOL=FALSE ) const;
    virtual BOOL                ValidateMode( StreamMode ) const;
    virtual const SvStream*     GetSvStream() const;
    virtual BOOL                Equals( const BaseStorage& rStream ) const;
    BOOL                        SetProperty( const String& rName, const ::com::sun::star::uno::Any& rValue );
    BOOL                        GetProperty( const String& rName, ::com::sun::star::uno::Any& rValue );
    BOOL                        GetProperty( const String& rEleName, const String& rName, ::com::sun::star::uno::Any& rValue );

    // HACK to avoid incompatible build, can be done since this feature is only for development
    // should be removed before release
    UNOStorageHolderList* GetUNOStorageHolderList();

//#if _SOLAR__PRIVATE
    UCBStorageElement_Impl*     FindElement_Impl( const String& rName ) const;
    BOOL                        CopyStorageElement_Impl( UCBStorageElement_Impl& rElement,
                                    BaseStorage* pDest, const String& rNew ) const;
    BaseStorage*                OpenStorage_Impl( const String & rEleName,
                                       StreamMode, BOOL bDirect, BOOL bForceUCBStorage );
//#endif

};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

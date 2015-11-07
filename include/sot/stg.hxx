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

#ifndef INCLUDED_SOT_STG_HXX
#define INCLUDED_SOT_STG_HXX

#include <com/sun/star/uno/Any.h>
#include <com/sun/star/uno/Reference.h>

#include <com/sun/star/io/XInputStream.hpp>

#include <com/sun/star/ucb/XCommandEnvironment.hpp>

#include <com/sun/star/embed/XStorage.hpp>


#include <tools/solar.h>
#include <tools/stream.hxx>
#include <tools/globname.hxx>
#include <sot/storinfo.hxx>
#include <sot/sotdllapi.h>

class Storage;
class StorageStream;
class StgIo;
class StgDirEntry;
class StgStrm;
class SvGlobalName;
typedef struct SvGUID ClsId;

class SOT_DLLPUBLIC StorageBase : public SvRefBase
{
protected:
    mutable ErrCode m_nError;                   // error code
    StreamMode      m_nMode;                    // open mode
    bool            m_bAutoCommit;
                    StorageBase();
    virtual         ~StorageBase();
public:
    virtual bool    Validate( bool=false ) const = 0;
    virtual bool    ValidateMode( StreamMode ) const = 0;
    void            ResetError() const;
    void            SetError( ErrCode ) const;
    ErrCode         GetError() const;
    bool            Good() const          { return bool( m_nError == SVSTREAM_OK ); }
    StreamMode      GetMode() const  { return m_nMode;  }
    void            SetAutoCommit( bool bSet )
                    { m_bAutoCommit = bSet; }
};

class BaseStorageStream : public StorageBase
{
public:
    virtual sal_uLong   Read( void * pData, sal_uLong nSize ) = 0;
    virtual sal_uLong   Write( const void* pData, sal_uLong nSize ) = 0;
    virtual sal_uInt64  Seek( sal_uInt64 nPos ) = 0;
    virtual sal_uLong   Tell() = 0;
    virtual void        Flush() = 0;
    virtual bool        SetSize( sal_uLong nNewSize ) = 0;
    virtual sal_uLong   GetSize() const = 0;
    virtual bool        CopyTo( BaseStorageStream * pDestStm ) = 0;
    virtual bool        Commit() = 0;
    virtual bool        Equals( const BaseStorageStream& rStream ) const = 0;
};

enum class SotClipboardFormatId : sal_uLong;

class BaseStorage : public StorageBase
{
public:
    virtual const OUString&     GetName() const = 0;
    virtual bool                IsRoot() const = 0;
    virtual void                SetClassId( const ClsId& ) = 0;
    virtual const ClsId&        GetClassId() const = 0;
    virtual void                SetDirty() = 0;
    virtual void                SetClass( const SvGlobalName & rClass,
                                          SotClipboardFormatId nOriginalClipFormat,
                                          const OUString & rUserTypeName ) = 0;
    virtual SvGlobalName        GetClassName() = 0;
    virtual SotClipboardFormatId         GetFormat() = 0;
    virtual OUString            GetUserName() = 0;
    virtual void                FillInfoList( SvStorageInfoList* ) const = 0;
    virtual bool                CopyTo( BaseStorage* pDestStg ) const = 0;
    virtual bool                Commit() = 0;
    virtual bool                Revert() = 0;
    virtual BaseStorageStream*  OpenStream( const OUString & rEleName,
                                            StreamMode = STREAM_STD_READWRITE,
                                            bool bDirect = true, const OString* pKey=0 ) = 0;
    virtual BaseStorage*        OpenStorage( const OUString & rEleName,
                                             StreamMode = STREAM_STD_READWRITE,
                                             bool bDirect = false ) = 0;
    virtual BaseStorage*        OpenUCBStorage( const OUString & rEleName,
                                                StreamMode = STREAM_STD_READWRITE,
                                                bool bDirect = false ) = 0;
    virtual BaseStorage*        OpenOLEStorage( const OUString & rEleName,
                                                StreamMode = STREAM_STD_READWRITE,
                                                bool bDirect = false ) = 0;
    virtual bool                IsStream( const OUString& rEleName ) const = 0;
    virtual bool                IsStorage( const OUString& rEleName ) const = 0;
    virtual bool                IsContained( const OUString& rEleName ) const = 0;
    virtual bool                Remove( const OUString & rEleName ) = 0;
    virtual bool                CopyTo( const OUString & rEleName, BaseStorage * pDest, const OUString & rNewName ) = 0;
    virtual bool                ValidateFAT() = 0;
    virtual bool                Equals( const BaseStorage& rStream ) const = 0;
};

class OLEStorageBase
{
protected:
    StreamMode&     nStreamMode;              // open mode
    StgIo*          pIo;                      // I/O subsystem
    StgDirEntry*    pEntry;                   // the dir entry
                    OLEStorageBase( StgIo*, StgDirEntry*, StreamMode& );
                    ~OLEStorageBase();
    bool            Validate_Impl( bool=false ) const;
    static bool     ValidateMode_Impl( StreamMode, StgDirEntry* p = NULL );
};

class StorageStream : public BaseStorageStream, public OLEStorageBase
{
//friend class Storage;
    sal_uLong           nPos;                             // current position
protected:
                        virtual ~StorageStream();
public:
                        StorageStream( StgIo*, StgDirEntry*, StreamMode );
    virtual sal_uLong   Read( void * pData, sal_uLong nSize ) override;
    virtual sal_uLong   Write( const void* pData, sal_uLong nSize ) override;
    virtual sal_uInt64  Seek( sal_uInt64 nPos ) override;
    virtual sal_uLong   Tell() override { return nPos; }
    virtual void        Flush() override;
    virtual bool        SetSize( sal_uLong nNewSize ) override;
    virtual sal_uLong   GetSize() const override;
    virtual bool        CopyTo( BaseStorageStream * pDestStm ) override;
    virtual bool        Commit() override;
    virtual bool        Validate( bool=false ) const override;
    virtual bool        ValidateMode( StreamMode ) const override;
    virtual bool        Equals( const BaseStorageStream& rStream ) const override;
};

class UCBStorageStream;

class SOT_DLLPUBLIC Storage : public BaseStorage, public OLEStorageBase
{
    OUString                    aName;
    bool                        bIsRoot;
    void                        Init( bool bCreate );
                                Storage( StgIo*, StgDirEntry*, StreamMode );
protected:
                                virtual ~Storage();
public:
                                Storage( const OUString &, StreamMode = STREAM_STD_READWRITE, bool bDirect = true );
                                Storage( SvStream& rStrm, bool bDirect = true );
                                Storage( UCBStorageStream& rStrm, bool bDirect = true );

    static bool                 IsStorageFile( const OUString & rFileName );
    static bool                 IsStorageFile( SvStream* );

    virtual const OUString&     GetName() const override;
    virtual bool                IsRoot() const override { return bIsRoot; }
    virtual void                SetClassId( const ClsId& ) override;
    virtual const ClsId&        GetClassId() const override;
    virtual void                SetDirty() override;
    virtual void                SetClass( const SvGlobalName & rClass,
                                          SotClipboardFormatId nOriginalClipFormat,
                                          const OUString & rUserTypeName ) override;
    virtual SvGlobalName        GetClassName() override;
    virtual SotClipboardFormatId         GetFormat() override;
    virtual OUString            GetUserName() override;
    virtual void                FillInfoList( SvStorageInfoList* ) const override;
    virtual bool                CopyTo( BaseStorage* pDestStg ) const override;
    virtual bool                Commit() override;
    virtual bool                Revert() override;
    virtual BaseStorageStream*  OpenStream( const OUString & rEleName,
                                            StreamMode = STREAM_STD_READWRITE,
                                            bool bDirect = true, const OString* pKey=0 ) override;
    virtual BaseStorage*        OpenStorage( const OUString & rEleName,
                                             StreamMode = STREAM_STD_READWRITE,
                                             bool bDirect = false ) override;
    virtual BaseStorage*        OpenUCBStorage( const OUString & rEleName,
                                                StreamMode = STREAM_STD_READWRITE,
                                                bool bDirect = false ) override;
    virtual BaseStorage*        OpenOLEStorage( const OUString & rEleName,
                                                StreamMode = STREAM_STD_READWRITE,
                                                bool bDirect = false ) override;
    virtual bool                IsStream( const OUString& rEleName ) const override;
    virtual bool                IsStorage( const OUString& rEleName ) const override;
    virtual bool                IsContained( const OUString& rEleName ) const override;
    virtual bool                Remove( const OUString & rEleName ) override;
    virtual bool                CopyTo( const OUString & rEleName, BaseStorage * pDest, const OUString & rNewName ) override;
    virtual bool                ValidateFAT() override;
    virtual bool                Validate( bool=false ) const override;
    virtual bool                ValidateMode( StreamMode ) const override;
    bool                        ValidateMode( StreamMode, StgDirEntry* p ) const;
    virtual bool                Equals( const BaseStorage& rStream ) const override;
};

class UCBStorageStream_Impl;
class UCBStorageStream : public BaseStorageStream
{
friend class UCBStorage;

    UCBStorageStream_Impl*
            pImp;
protected:
                                virtual ~UCBStorageStream();
public:
                                UCBStorageStream( const OUString& rName, StreamMode nMode, bool bDirect, const OString* pKey, bool bRepair, css::uno::Reference< css::ucb::XProgressHandler > xProgress );
                                UCBStorageStream( UCBStorageStream_Impl* );

    virtual sal_uLong           Read( void * pData, sal_uLong nSize ) override;
    virtual sal_uLong           Write( const void* pData, sal_uLong nSize ) override;
    virtual sal_uInt64          Seek( sal_uInt64 nPos ) override;
    virtual sal_uLong           Tell() override;
    virtual void                Flush() override;
    virtual bool                SetSize( sal_uLong nNewSize ) override;
    virtual sal_uLong           GetSize() const override;
    virtual bool                CopyTo( BaseStorageStream * pDestStm ) override;
    virtual bool                Commit() override;
    virtual bool                Validate( bool=false ) const override;
    virtual bool                ValidateMode( StreamMode ) const override;
    virtual bool                Equals( const BaseStorageStream& rStream ) const override;
    bool                        SetProperty( const OUString& rName, const css::uno::Any& rValue );

    SvStream*                   GetModifySvStream();
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
                                virtual ~UCBStorage();
public:
    static bool                 IsStorageFile( SvStream* );
    static OUString             GetLinkedFile( SvStream& );

                                UCBStorage( const ::ucbhelper::Content& rContent, const OUString& rName, StreamMode nMode, bool bDirect = true, bool bIsRoot = true );
                                UCBStorage( const OUString& rName,
                                            StreamMode nMode,
                                            bool bDirect = true,
                                            bool bIsRoot = true );

                                UCBStorage( const OUString& rName,
                                            StreamMode nMode,
                                            bool bDirect,
                                            bool bIsRoot,
                                            bool bIsRepair,
                                            css::uno::Reference< css::ucb::XProgressHandler >
                                            xProgressHandler );

                                UCBStorage( UCBStorage_Impl* );
                                UCBStorage( SvStream& rStrm, bool bDirect = true );

    virtual const OUString&     GetName() const override;
    virtual bool                IsRoot() const override;
    virtual void                SetClassId( const ClsId& ) override;
    virtual const ClsId&        GetClassId() const override;
    virtual void                SetDirty() override;
    virtual void                SetClass( const SvGlobalName & rClass,
                                          SotClipboardFormatId nOriginalClipFormat,
                                          const OUString & rUserTypeName ) override;
    virtual SvGlobalName        GetClassName() override;
    virtual SotClipboardFormatId         GetFormat() override;
    virtual OUString            GetUserName() override;
    virtual void                FillInfoList( SvStorageInfoList* ) const override;
    virtual bool                CopyTo( BaseStorage* pDestStg ) const override;
    virtual bool                Commit() override;
    virtual bool                Revert() override;
    virtual BaseStorageStream*  OpenStream( const OUString & rEleName,
                                            StreamMode = STREAM_STD_READWRITE,
                                            bool bDirect = true, const OString* pKey=0 ) override;
    virtual BaseStorage*        OpenStorage( const OUString & rEleName,
                                             StreamMode = STREAM_STD_READWRITE,
                                             bool bDirect = false ) override;
    virtual BaseStorage*        OpenUCBStorage( const OUString & rEleName,
                                                StreamMode = STREAM_STD_READWRITE,
                                                bool bDirect = false ) override;
    virtual BaseStorage*        OpenOLEStorage( const OUString & rEleName,
                                                StreamMode = STREAM_STD_READWRITE,
                                                bool bDirect = false ) override;
    virtual bool                IsStream( const OUString& rEleName ) const override;
    virtual bool                IsStorage( const OUString& rEleName ) const override;
    virtual bool                IsContained( const OUString& rEleName ) const override;
    virtual bool                Remove( const OUString & rEleName ) override;
    virtual bool                CopyTo( const OUString & rEleName, BaseStorage * pDest, const OUString & rNewName ) override;
    virtual bool                ValidateFAT() override;
    virtual bool                Validate( bool=false ) const override;
    virtual bool                ValidateMode( StreamMode ) const override;
    virtual bool                Equals( const BaseStorage& rStream ) const override;

    UCBStorageElement_Impl*     FindElement_Impl( const OUString& rName ) const;
    bool                        CopyStorageElement_Impl( UCBStorageElement_Impl& rElement,
                                                         BaseStorage* pDest, const OUString& rNew ) const;
    BaseStorage*                OpenStorage_Impl( const OUString & rEleName,
                                                  StreamMode, bool bDirect, bool bForceUCBStorage );

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

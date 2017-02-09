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

#ifndef INCLUDED_SOT_STORAGE_HXX
#define INCLUDED_SOT_STORAGE_HXX

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/embed/XStorage.hpp>
#include <sot/object.hxx>
#include <tools/stream.hxx>
#include <tools/errcode.hxx>
#include <sot/storinfo.hxx>
#include <sot/sotdllapi.h>

enum class SotClipboardFormatId : sal_uLong;

class SotStorage;
class BaseStorageStream;
class SOT_DLLPUBLIC SotStorageStream : virtual public SotObject, public SvStream
{
friend class SotStorage;
    BaseStorageStream * pOwnStm; // pointer to the own stream
protected:
    virtual std::size_t GetData(void* pData, std::size_t nSize) override;
    virtual std::size_t PutData(const void* pData, std::size_t nSize) override;
    virtual sal_uInt64      SeekPos(sal_uInt64 nPos) override;
    virtual void        FlushData() override;
                        virtual ~SotStorageStream() override;
public:
                        SotStorageStream( const OUString &,
                                          StreamMode = StreamMode::STD_READWRITE );
                        SotStorageStream( BaseStorageStream *pStm );

    virtual void        ResetError() override;

    virtual void        SetSize( sal_uInt64 nNewSize ) override;
    sal_uInt32          GetSize() const;
    void                CopyTo( SotStorageStream * pDestStm );
    bool                Commit();
    bool                SetProperty( const OUString& rName, const css::uno::Any& rValue );
    virtual sal_uInt64 remainingSize() override;
};

class  BaseStorage;
class SOT_DLLPUBLIC SotStorage : virtual public SotObject
{
friend class SotStorageStream;

    BaseStorage *   m_pOwnStg;   // target storage
    SvStream *      m_pStorStm;  // only for SDSTORAGES
    ErrCode         m_nError;
    OUString        m_aName;     // name of the storage
    bool            m_bIsRoot;   // e.g.: File Storage
    bool            m_bDelStm;
    OString         m_aKey;      // aKey.Len != 0  -> encryption
    long            m_nVersion;

protected:
                        virtual ~SotStorage() override;
   void                 CreateStorage( bool bUCBStorage, StreamMode );
public:
                        SotStorage( const OUString &,
                                    StreamMode = StreamMode::STD_READWRITE );
                        SotStorage( bool bUCBStorage, const OUString &,
                                    StreamMode = StreamMode::STD_READWRITE );
                        SotStorage( BaseStorage * );
                        SotStorage( SvStream & rStm );
                        SotStorage( bool bUCBStorage, SvStream & rStm );
                        SotStorage( SvStream * pStm, bool bDelete );

    SvMemoryStream *    CreateMemoryStream();

    static bool         IsStorageFile( const OUString & rFileName );
    static bool         IsStorageFile( SvStream* pStream );

    const OUString&     GetName() const;

    bool                Validate();

    const OString&      GetKey() const { return m_aKey;}

    void                SetVersion( long nVers )
                        {
                            m_nVersion = nVers;
                        }
    long                GetVersion() const
                        {
                            return m_nVersion;
                        }

    ErrCode             GetError() const { return m_nError.IgnoreWarning(); }
    void                SetError( ErrCode nErrorCode )
                        {
                            if( m_nError == SVSTREAM_OK )
                                m_nError = nErrorCode;
                        }

    void                SignAsRoot( bool b ) { m_bIsRoot = b; }

                        // own data sector
    void                SetClass( const SvGlobalName & rClass,
                                  SotClipboardFormatId bOriginalClipFormat,
                                  const OUString & rUserTypeName );
    SvGlobalName        GetClassName(); // type of data in the storage
    SotClipboardFormatId GetFormat();
    OUString            GetUserName();

                        // list of all elements
    void                FillInfoList( SvStorageInfoList * ) const;
    bool                CopyTo( SotStorage * pDestStg );
    bool                Commit();

                        // create stream with connection to Storage,
                        // more or less a Parent-Child relationship
    SotStorageStream *  OpenSotStream( const OUString & rEleName,
                                       StreamMode = StreamMode::STD_READWRITE );
    SotStorage *        OpenSotStorage( const OUString & rEleName,
                                        StreamMode = StreamMode::STD_READWRITE,
                                        bool transacted = true );
                        // query whether Storage or Stream
    bool                IsStream( const OUString & rEleName ) const;
    bool                IsStorage( const OUString & rEleName ) const;
    bool                IsContained( const OUString & rEleName ) const;
                        // remove element
    bool                Remove( const OUString & rEleName );
    bool                CopyTo( const OUString & rEleName, SotStorage * pDest,
                                const OUString & rNewName );

    bool                IsOLEStorage() const;
    static bool         IsOLEStorage( const OUString & rFileName );
    static bool         IsOLEStorage( SvStream* pStream );

    static SotStorage*  OpenOLEStorage( const css::uno::Reference < css::embed::XStorage >& xStorage,
                                        const OUString& rEleName, StreamMode = StreamMode::STD_READWRITE );
    static SotClipboardFormatId  GetFormatID( const css::uno::Reference < css::embed::XStorage >& xStorage );
    static sal_Int32    GetVersion( const css::uno::Reference < css::embed::XStorage >& xStorage );
};

#endif // _SVSTOR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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

#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <sot/object.hxx>
#include <sot/factory.hxx>
#include <tools/stream.hxx>
#include <tools/errcode.hxx>
#include <sot/storinfo.hxx>
#include <sot/sotdllapi.h>

#define STORAGE_FAILIFTHERE     0x02
#define STORAGE_TRANSACTED      0x04
#define STORAGE_PRIORITY        0x08
#define STORAGE_DELETEONRELEASE 0x10
#define STORAGE_CONVERT         0x20
#define STORAGE_UNPACKED_MODE   0x40
#define STORAGE_DISKSPANNED_MODE   0x80
#define STORAGE_CREATE_UNPACKED 0x44
typedef short StorageMode;

class SvStorage;


/*************************************************************************
*************************************************************************/
class SotStorage;
class BaseStorageStream;
class SOT_DLLPUBLIC SotStorageStream : virtual public SotObject, public SvStream
{
friend class SotStorage;
    BaseStorageStream * pOwnStm;// Zeiger auf den eigenen Stream
protected:
    virtual sal_uLong       GetData( void* pData, sal_uLong nSize ) SAL_OVERRIDE;
    virtual sal_uLong       PutData( const void* pData, sal_uLong nSize ) SAL_OVERRIDE;
    virtual sal_uInt64      SeekPos(sal_uInt64 nPos) SAL_OVERRIDE;
    virtual void        FlushData() SAL_OVERRIDE;
                        virtual ~SotStorageStream();
public:
                        SotStorageStream( const OUString &,
                                          StreamMode = STREAM_STD_READWRITE,
                                          StorageMode = 0 );
                        SotStorageStream( BaseStorageStream *pStm );
                        SotStorageStream();

private:
    static SotFactory **       GetFactoryAdress()
                              { return &(SOTDATA()->pSotStorageStreamFactory); }
public:
    static void *             CreateInstance( SotObject ** = NULL );
    static SotFactory *        ClassFactory();
    virtual const SotFactory * GetSvFactory() const SAL_OVERRIDE;
    virtual void *            Cast( const SotFactory * ) SAL_OVERRIDE;

    virtual void        ResetError() SAL_OVERRIDE;

    virtual void        SetSize( sal_uInt64 nNewSize ) SAL_OVERRIDE;
    sal_uInt32              GetSize() const;
    bool                CopyTo( SotStorageStream * pDestStm );
    virtual bool        Commit();
    virtual bool        Revert();
    bool                SetProperty( const OUString& rName, const ::com::sun::star::uno::Any& rValue );
    virtual sal_uInt64 remainingSize() SAL_OVERRIDE;
};

#ifndef SOT_DECL_SOTSTORAGESTREAM_DEFINED
#define SOT_DECL_SOTSTORAGESTREAM_DEFINED
typedef tools::SvRef<SotStorageStream> SotStorageStreamRef;
#endif

namespace ucbhelper
{
    class Content;
}

class  BaseStorage;
class  UNOStorageHolder;
class SOT_DLLPUBLIC SotStorage : virtual public SotObject
{
friend class SotStorageStream;
friend class SvStorage;

    BaseStorage *   m_pOwnStg;   // Zielstorage
    SvStream *      m_pStorStm;  // nur fuer SDSTORAGES
    sal_uLong       m_nError;
    OUString        m_aName;     // Name des Storage
    bool            m_bIsRoot;   // z.B.: File-Storage
    bool            m_bDelStm;
    OString         m_aKey;      // aKey.Len != 0  -> Verschluesselung
    long            m_nVersion;

protected:
                        virtual ~SotStorage();
   void                 CreateStorage( bool bUCBStorage, StreamMode, StorageMode );
public:
                        SotStorage( const OUString &,
                                    StreamMode = STREAM_STD_READWRITE,
                                    StorageMode = 0 );
                        SotStorage( bool bUCBStorage, const OUString &,
                                    StreamMode = STREAM_STD_READWRITE,
                                    StorageMode = 0 );
                        SotStorage( BaseStorage * );
                        SotStorage( SvStream & rStm );
                        SotStorage( bool bUCBStorage, SvStream & rStm );
                        SotStorage( SvStream * pStm, bool bDelete );
                        SotStorage();

private:
    static SotFactory **       GetFactoryAdress()
                              { return &(SOTDATA()->pSotStorageFactory); }
public:
    static void *             CreateInstance( SotObject ** = NULL );
    static SotFactory *        ClassFactory();
    virtual const SotFactory * GetSvFactory() const SAL_OVERRIDE;
    virtual void *            Cast( const SotFactory * ) SAL_OVERRIDE;

    SvMemoryStream *    CreateMemoryStream();

    static bool         IsStorageFile( const OUString & rFileName );
    static bool         IsStorageFile( SvStream* pStream );

    virtual const OUString& GetName() const;

    virtual bool        Validate();

    const OString&      GetKey() const;

    void                SetVersion( long nVers )
                        {
                            m_nVersion = nVers;
                        }
    long                GetVersion() const
                        {
                            return m_nVersion;
                        }

    sal_uLong               GetErrorCode() const { return m_nError; }
    sal_uLong               GetError() const { return ERRCODE_TOERROR(m_nError); }
    void                SetError( sal_uLong nErrorCode )
                        {
                            if( m_nError == SVSTREAM_OK )
                                m_nError = nErrorCode;
                        }
    virtual void        ResetError();

    bool                IsRoot() const              { return m_bIsRoot; }
    void                SignAsRoot( bool b = true ) { m_bIsRoot = b; }
    void                SetDeleteStream( bool bDelete ) { m_bDelStm = bDelete; }

                        // eigener Datenbereich
    virtual void        SetClass( const SvGlobalName & rClass,
                                  sal_uLong bOriginalClipFormat,
                                  const OUString & rUserTypeName );
    virtual void        SetConvertClass( const SvGlobalName & rConvertClass,
                                         sal_uLong bOriginalClipFormat,
                                         const OUString & rUserTypeName );
    virtual SvGlobalName GetClassName();// Typ der Daten im Storage
    virtual sal_uLong   GetFormat();
    virtual OUString    GetUserName();
    virtual bool        ShouldConvert();

                        // Liste aller Elemente
    virtual void        FillInfoList( SvStorageInfoList * ) const;
    virtual bool        CopyTo( SotStorage * pDestStg );
    virtual bool        Commit();
    virtual bool        Revert();

                        /* Element Methoden     */
                        // Stream mit Verbindung zu Storage erzeugen,
                        // in etwa eine Parent-Child Beziehung
    SotStorageStream *  OpenSotStream( const OUString & rEleName,
                                       StreamMode = STREAM_STD_READWRITE,
                                       StorageMode = 0 );
    SotStorage *        OpenSotStorage( const OUString & rEleName,
                                        StreamMode = STREAM_STD_READWRITE,
                                        StorageMode = STORAGE_TRANSACTED );
                        // Abfrage auf Storage oder Stream
    virtual bool        IsStream( const OUString & rEleName ) const;
    virtual bool        IsStorage( const OUString & rEleName ) const;
    virtual bool        IsContained( const OUString & rEleName ) const;
                        // Element loeschen
    virtual bool        Remove( const OUString & rEleName );
                        // Elementnamen aendern
    virtual bool        Rename( const OUString & rEleName,
                                const OUString & rNewName );
    virtual bool        CopyTo( const OUString & rEleName, SotStorage * pDest,
                                const OUString & rNewName );
    virtual bool        MoveTo( const OUString & rEleName, SotStorage * pDest,
                                const OUString & rNewName );

    bool                IsOLEStorage() const;
    static bool         IsOLEStorage( const OUString & rFileName );
    static bool         IsOLEStorage( SvStream* pStream );

    static SotStorage*  OpenOLEStorage( const com::sun::star::uno::Reference < com::sun::star::embed::XStorage >& xStorage,
                                        const OUString& rEleName, StreamMode = STREAM_STD_READWRITE );
    static sal_Int32    GetFormatID( const com::sun::star::uno::Reference < com::sun::star::embed::XStorage >& xStorage );
    static sal_Int32    GetVersion( const com::sun::star::uno::Reference < com::sun::star::embed::XStorage >& xStorage );
};

#ifndef SOT_DECL_SOTSTORAGE_DEFINED
#define SOT_DECL_SOTSTORAGE_DEFINED
typedef tools::SvRef<SotStorage> SotStorageRef;
#endif

#define SvStorage SotStorage
#define SvStorageRef SotStorageRef
#define SvStorageStream SotStorageStream
#define SvStorageStreamRef SotStorageStreamRef

#endif // _SVSTOR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

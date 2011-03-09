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

#ifndef _SOT_STORAGE_HXX
#define _SOT_STORAGE_HXX

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.h>

#ifndef _COM_SUN_STAR_IO_XINPUTSTREAM_H_
#include <com/sun/star/io/XInputStream.hpp>
#endif
#include <com/sun/star/embed/XStorage.hpp>
#include <sot/object.hxx>
#include <sot/factory.hxx>
#ifndef _TOOLS_STREAM_HXX
#include <tools/stream.hxx>
#endif
#ifndef _TOOLS_ERRCODE_HXX
#include <tools/errcode.hxx>
#endif
#include "sot/sotdllapi.h"

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
namespace binfilter
{
    class SvStorage;
}

/*************************************************************************
*************************************************************************/
class SotStorage;
class BaseStorageStream;
class SOT_DLLPUBLIC SotStorageStream : virtual public SotObject, public SvStream
{
friend class SotStorage;
friend class ImpStream;
    BaseStorageStream * pOwnStm;// Zeiger auf den eigenen Stream
protected:
    virtual sal_uLong       GetData( void* pData, sal_uLong nSize );
    virtual sal_uLong       PutData( const void* pData, sal_uLong nSize );
    virtual sal_uLong       SeekPos( sal_uLong nPos );
    virtual void        FlushData();
                        ~SotStorageStream();
public:
                        SotStorageStream( const String &,
                                     StreamMode = STREAM_STD_READWRITE,
                                     StorageMode = 0 );
                        SotStorageStream( BaseStorageStream *pStm );
                        SotStorageStream();
                        SO2_DECL_BASIC_CLASS_DLL(SotStorageStream,SOTDATA())
                        SO2_DECL_INVARIANT()

    using SvStream::SyncSvStream;
    virtual void        SyncSvStream();
    void                SyncSysStream() { SvStream::SyncSysStream(); }

    virtual void        ResetError();

    virtual void        SetSize( sal_uLong nNewSize );
    sal_uInt32              GetSize() const;
    sal_Bool                CopyTo( SotStorageStream * pDestStm );
    virtual sal_Bool        Commit();
    virtual sal_Bool        Revert();
    sal_Bool                SetProperty( const String& rName, const ::com::sun::star::uno::Any& rValue );
    sal_Bool                GetProperty( const String& rName, ::com::sun::star::uno::Any& rValue );
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >
                        GetXInputStream() const;
};

#ifndef SOT_DECL_SOTSTORAGESTREAM_DEFINED
#define SOT_DECL_SOTSTORAGESTREAM_DEFINED
SO2_DECL_REF(SotStorageStream)
#endif
SO2_IMPL_REF(SotStorageStream)

namespace ucbhelper
{
    class Content;
}

class  SvStorageInfoList;
class  BaseStorage;
class  UNOStorageHolder;
class SOT_DLLPUBLIC SotStorage : virtual public SotObject
{
friend class SotStorageStream;
friend class SvStorage;
friend class ::binfilter::SvStorage;

    BaseStorage *   m_pTmpStg;   // Temp-Storage fuer Transacted, nur auf diesem schreiben!        ??? Useless ???
    BaseStorage *   m_pOwnStg;   // Zielstorage
    SvStream *  m_pStorStm;  // nur fuer SDSTORAGES
    sal_uLong       m_nError;
    String      m_aName;      // Name des Storage
    sal_Bool        m_bIsRoot:1,  // z.B.: File-Storage
                m_bDelStm:1;
    ByteString  m_aKey;           // aKey.Len != 0  -> Verschluesselung
    long        m_nVersion;

protected:
                        ~SotStorage();
   void                 CreateStorage( sal_Bool bUCBStorage, StreamMode, StorageMode );
public:
                        SotStorage( const String &,
                                   StreamMode = STREAM_STD_READWRITE,
                                   StorageMode = 0 );
                        SotStorage( sal_Bool bUCBStorage, const String &,
                                   StreamMode = STREAM_STD_READWRITE,
                                   StorageMode = 0 );
                        SotStorage( const ::ucbhelper::Content& rContent, const String &,
                                   StreamMode = STREAM_STD_READWRITE,
                                   StorageMode = 0 );
                        SotStorage( BaseStorage * );
                        SotStorage( SvStream & rStm );
                        SotStorage( sal_Bool bUCBStorage, SvStream & rStm );
                        SotStorage( SvStream * pStm, sal_Bool bDelete );
                        SotStorage();
                        SO2_DECL_BASIC_CLASS_DLL(SotStorage,SOTDATA())
                        SO2_DECL_INVARIANT()

    SvMemoryStream *    CreateMemoryStream();
    const SvStream *    GetSvStream();

    static sal_Bool         IsStorageFile( const String & rFileName );
    static sal_Bool         IsStorageFile( SvStream* pStream );

    virtual const String & GetName() const;

    virtual sal_Bool        Validate();

    void                SetKey( const ByteString& rKey );
    const ByteString &  GetKey() const { return m_aKey; }

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

    sal_Bool                IsRoot() const              { return m_bIsRoot; }
    void                SignAsRoot( sal_Bool b = sal_True ) { m_bIsRoot = b; }
    void                SetDeleteStream( sal_Bool bDelete ) { m_bDelStm = bDelete; }

                        // eigener Datenbereich
    virtual void        SetClass( const SvGlobalName & rClass,
                                  sal_uLong bOriginalClipFormat,
                                  const String & rUserTypeName );
    virtual void        SetConvertClass( const SvGlobalName & rConvertClass,
                                         sal_uLong bOriginalClipFormat,
                                         const String & rUserTypeName );
    virtual SvGlobalName GetClassName();// Typ der Daten im Storage
    virtual sal_uLong       GetFormat();
    virtual String      GetUserName();
    virtual sal_Bool        ShouldConvert();
    void                SetName( const String& rName );

                        // Liste aller Elemente
    virtual void        FillInfoList( SvStorageInfoList * ) const;
    virtual sal_Bool        CopyTo( SotStorage * pDestStg );
    virtual sal_Bool        Commit();
    virtual sal_Bool        Revert();

                        /* Element Methoden     */
                        // Stream mit Verbindung zu Storage erzeugen,
                        // in etwa eine Parent-Child Beziehung
    SotStorageStream *  OpenSotStream( const String & rEleName,
                                    StreamMode = STREAM_STD_READWRITE,
                                    StorageMode = 0 );
    SotStorageStream *  OpenEncryptedSotStream( const String & rEleName, const ByteString& rKey,
                                    StreamMode = STREAM_STD_READWRITE,
                                    StorageMode = 0 );
    SotStorage *        OpenSotStorage( const String & rEleName,
                                    StreamMode = STREAM_STD_READWRITE,
                                    StorageMode = STORAGE_TRANSACTED );
    SotStorage *        OpenUCBStorage( const String & rEleName,
                                    StreamMode = STREAM_STD_READWRITE,
                                    StorageMode = STORAGE_TRANSACTED );
    SotStorage *        OpenOLEStorage( const String & rEleName,
                                    StreamMode = STREAM_STD_READWRITE,
                                    StorageMode = STORAGE_TRANSACTED );
                        // Abfrage auf Storage oder Stream
    virtual sal_Bool        IsStream( const String & rEleName ) const;
    virtual sal_Bool        IsStorage( const String & rEleName ) const;
    virtual sal_Bool        IsContained( const String & rEleName ) const;
                        // Element loeschen
    virtual sal_Bool        Remove( const String & rEleName );
                        // Elementnamen aendern
    virtual sal_Bool        Rename( const String & rEleName,
                                const String & rNewName );
    virtual sal_Bool        CopyTo( const String & rEleName, SotStorage * pDest,
                                const String & rNewName );
    virtual sal_Bool        MoveTo( const String & rEleName, SotStorage * pDest,
                                const String & rNewName );

    SvStream*           GetTargetSvStream() const;
    sal_Bool                SetProperty( const String& rName, const ::com::sun::star::uno::Any& rValue );
    sal_Bool                GetProperty( const String& rName, ::com::sun::star::uno::Any& rValue );
    sal_Bool                GetProperty( const String& rEleName, const String& rName, ::com::sun::star::uno::Any& rValue );
    sal_Bool                IsOLEStorage() const;
    static sal_Bool         IsOLEStorage( const String & rFileName );
    static sal_Bool         IsOLEStorage( SvStream* pStream );

    // this is temporary HACK, _MUST_ be removed before release
    ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >
                        GetUNOAPIDuplicate( const String& rEleName, sal_Int32 nUNOStorageMode );
    void                RemoveUNOStorageHolder( UNOStorageHolder* pHolder );

    static SotStorage*  OpenOLEStorage( const com::sun::star::uno::Reference < com::sun::star::embed::XStorage >& xStorage,
                                    const String& rEleName, StreamMode = STREAM_STD_READWRITE );
    static sal_Int32    GetFormatID( const com::sun::star::uno::Reference < com::sun::star::embed::XStorage >& xStorage );
    static sal_Int32    GetVersion( const com::sun::star::uno::Reference < com::sun::star::embed::XStorage >& xStorage );
};

#ifndef SOT_DECL_SOTSTORAGE_DEFINED
#define SOT_DECL_SOTSTORAGE_DEFINED
SO2_DECL_REF(SotStorage)
#endif
SO2_IMPL_REF(SotStorage)

#define SvStorage SotStorage
#define SvStorageRef SotStorageRef
#define SvStorageStream SotStorageStream
#define SvStorageStreamRef SotStorageStreamRef

#endif // _SVSTOR_HXX

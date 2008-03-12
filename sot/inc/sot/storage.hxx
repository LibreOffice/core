/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: storage.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 13:21:37 $
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

#ifndef _SOT_STORAGE_HXX
#define _SOT_STORAGE_HXX

#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif

#ifndef _COM_SUN_STAR_IO_XINPUTSTREAM_H_
#include <com/sun/star/io/XInputStream.hpp>
#endif

#ifndef _COM_SUN_STAR_EMBED_XSTORAGE_HPP_
#include <com/sun/star/embed/XStorage.hpp>
#endif

#ifndef _SOT_OBJECT_HXX
#include <sot/object.hxx>
#endif
#ifndef _SOT_FACTORY_HXX
#include <sot/factory.hxx>
#endif
#ifndef _TOOLS_STREAM_HXX
#include <tools/stream.hxx>
#endif
#ifndef _TOOLS_ERRCODE_HXX
#include <tools/errcode.hxx>
#endif
#ifndef INCLUDED_SOTDLLAPI_H
#include "sot/sotdllapi.h"
#endif

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
    virtual ULONG       GetData( void* pData, ULONG nSize );
    virtual ULONG       PutData( const void* pData, ULONG nSize );
    virtual ULONG       SeekPos( ULONG nPos );
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

    virtual void        SetSize( ULONG nNewSize );
    UINT32              GetSize() const;
    BOOL                CopyTo( SotStorageStream * pDestStm );
    virtual BOOL        Commit();
    virtual BOOL        Revert();
    BOOL                SetProperty( const String& rName, const ::com::sun::star::uno::Any& rValue );
    BOOL                GetProperty( const String& rName, ::com::sun::star::uno::Any& rValue );
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
    ULONG       m_nError;
    String      m_aName;      // Name des Storage
    BOOL        m_bIsRoot:1,  // z.B.: File-Storage
                m_bDelStm:1;
    ByteString  m_aKey;           // aKey.Len != 0  -> Verschluesselung
    long        m_nVersion;

protected:
                        ~SotStorage();
   void                 CreateStorage( BOOL bUCBStorage, StreamMode, StorageMode );
public:
                        SotStorage( const String &,
                                   StreamMode = STREAM_STD_READWRITE,
                                   StorageMode = 0 );
                        SotStorage( BOOL bUCBStorage, const String &,
                                   StreamMode = STREAM_STD_READWRITE,
                                   StorageMode = 0 );
                        SotStorage( const ::ucbhelper::Content& rContent, const String &,
                                   StreamMode = STREAM_STD_READWRITE,
                                   StorageMode = 0 );
                        SotStorage( BaseStorage * );
                        SotStorage( SvStream & rStm );
                        SotStorage( BOOL bUCBStorage, SvStream & rStm );
                        SotStorage( SvStream * pStm, BOOL bDelete );
                        SotStorage();
                        SO2_DECL_BASIC_CLASS_DLL(SotStorage,SOTDATA())
                        SO2_DECL_INVARIANT()

    SvMemoryStream *    CreateMemoryStream();
    const SvStream *    GetSvStream();

    static BOOL         IsStorageFile( const String & rFileName );
    static BOOL         IsStorageFile( SvStream* pStream );

    virtual const String & GetName() const;

    virtual BOOL        Validate();

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

    ULONG               GetErrorCode() const { return m_nError; }
    ULONG               GetError() const { return ERRCODE_TOERROR(m_nError); }
    void                SetError( ULONG nErrorCode )
                        {
                            if( m_nError == SVSTREAM_OK )
                                m_nError = nErrorCode;
                        }
    virtual void        ResetError();

    BOOL                IsRoot() const              { return m_bIsRoot; }
    void                SignAsRoot( BOOL b = TRUE ) { m_bIsRoot = b; }
    void                SetDeleteStream( BOOL bDelete ) { m_bDelStm = bDelete; }

                        // eigener Datenbereich
    virtual void        SetClass( const SvGlobalName & rClass,
                                  ULONG bOriginalClipFormat,
                                  const String & rUserTypeName );
    virtual void        SetConvertClass( const SvGlobalName & rConvertClass,
                                         ULONG bOriginalClipFormat,
                                         const String & rUserTypeName );
    virtual SvGlobalName GetClassName();// Typ der Daten im Storage
    virtual ULONG       GetFormat();
    virtual String      GetUserName();
    virtual BOOL        ShouldConvert();
    void                SetName( const String& rName );

                        // Liste aller Elemente
    virtual void        FillInfoList( SvStorageInfoList * ) const;
    virtual BOOL        CopyTo( SotStorage * pDestStg );
    virtual BOOL        Commit();
    virtual BOOL        Revert();

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
    virtual BOOL        IsStream( const String & rEleName ) const;
    virtual BOOL        IsStorage( const String & rEleName ) const;
    virtual BOOL        IsContained( const String & rEleName ) const;
                        // Element loeschen
    virtual BOOL        Remove( const String & rEleName );
                        // Elementnamen aendern
    virtual BOOL        Rename( const String & rEleName,
                                const String & rNewName );
    virtual BOOL        CopyTo( const String & rEleName, SotStorage * pDest,
                                const String & rNewName );
    virtual BOOL        MoveTo( const String & rEleName, SotStorage * pDest,
                                const String & rNewName );

    SvStream*           GetTargetSvStream() const;
    BOOL                SetProperty( const String& rName, const ::com::sun::star::uno::Any& rValue );
    BOOL                GetProperty( const String& rName, ::com::sun::star::uno::Any& rValue );
    BOOL                GetProperty( const String& rEleName, const String& rName, ::com::sun::star::uno::Any& rValue );
    BOOL                IsOLEStorage() const;
    static BOOL         IsOLEStorage( const String & rFileName );
    static BOOL         IsOLEStorage( SvStream* pStream );

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

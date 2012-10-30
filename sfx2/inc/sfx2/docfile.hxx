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

#ifndef _SFXDOCFILE_HXX
#define _SFXDOCFILE_HXX

#include <com/sun/star/io/XSeekable.hpp>
#include "sal/config.h"
#include "sfx2/dllapi.h"
#include "sal/types.h"
#include <com/sun/star/util/RevisionTag.hpp>
#include <com/sun/star/util/DateTime.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/ucb/XContent.hpp>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <tools/link.hxx>
#include <tools/stream.hxx>
#include <tools/string.hxx>
#include <svl/lstner.hxx>

#include <cppuhelper/weak.hxx>
#include <ucbhelper/content.hxx>

#include <vector>

class SvKeyValueIterator;
class SfxObjectFactory;
class SfxFilter;
class SfxMedium_Impl;
class INetURLObject;
class SfxObjectShell;
class SfxFrame;
class Timer;
class SfxItemSet;
class DateTime;

#define XMULTISERVICEFACTORY        ::com::sun::star::lang::XMultiServiceFactory
#define XSERVICEINFO                ::com::sun::star::lang::XServiceInfo
#define OWEAKOBJECT                 ::cppu::OWeakObject
#define REFERENCE                   ::com::sun::star::uno::Reference
#define XINTERFACE                  ::com::sun::star::uno::XInterface
#define EXCEPTION                   ::com::sun::star::uno::Exception
#define RUNTIMEEXCEPTION            ::com::sun::star::uno::RuntimeException
#define ANY                         ::com::sun::star::uno::Any

class SFX2_DLLPUBLIC SfxMedium : public SvRefBase
{
    SfxMedium_Impl* pImp;

#if _SOLAR__PRIVATE
    SAL_DLLPRIVATE void SetIsRemote_Impl();
    SAL_DLLPRIVATE void CloseInStream_Impl();
    SAL_DLLPRIVATE sal_Bool CloseOutStream_Impl();
    SAL_DLLPRIVATE void CloseStreams_Impl();
    DECL_DLLPRIVATE_STATIC_LINK( SfxMedium, UCBHdl_Impl, sal_uInt32 * );

    SAL_DLLPRIVATE void SetEncryptionDataToStorage_Impl();
#endif

public:

                        SfxMedium();
                        SfxMedium( const String &rName,
                                   StreamMode nOpenMode,
                                   const SfxFilter *pFilter = 0,
                                   SfxItemSet *pSet = 0 );
                        //TODO: the next, non-defined overload is only there to
                        // detect uses of the above (String, StreamMode, etc.)
                        // overload from when it still had an additional third
                        // parameter sal_Bool bDirect, where now a leftover
                        // "false" or "sal_False" could be mistaken for a null
                        // pointer argument for the pFilter parameter; it can be
                        // removed once we are confident all old uses of the
                        // original overload have been adapted (in platform
                        // specific code etc.):
                        SfxMedium(String const &, StreamMode, void *)
                            /* = delete */;

                        SfxMedium( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage,
                                    const String& rBaseURL,
                                    const SfxItemSet* pSet=0 );
                        SfxMedium( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aArgs );

                        ~SfxMedium();

    void                UseInteractionHandler( sal_Bool );
    ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >
                        GetInteractionHandler();

    void setStreamToLoadFrom(
        const com::sun::star::uno::Reference<com::sun::star::io::XInputStream>& xInputStream,
        sal_Bool bIsReadOnly);

    void                SetLoadTargetFrame(SfxFrame* pFrame );
    SfxFrame*           GetLoadTargetFrame() const;

    void                SetFilter(const SfxFilter *pFlt, sal_Bool bResetOrig = sal_False);
    const SfxFilter* GetFilter() const;
    const SfxFilter *   GetOrigFilter( sal_Bool bNotCurrent = sal_False ) const;
    const rtl::OUString& GetOrigURL() const;

    SfxItemSet  *       GetItemSet() const;
    void                Close();
    void                CloseAndRelease();
    void                ReOpen();
    void                CompleteReOpen();
    const rtl::OUString& GetName() const;
    const INetURLObject& GetURLObject() const;

    void                CheckFileDate( const ::com::sun::star::util::DateTime& aInitDate );
    sal_Bool            DocNeedsFileDateCheck();
    ::com::sun::star::util::DateTime GetInitFileDate( sal_Bool bIgnoreOldValue );

    ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XContent > GetContent() const;
    const rtl::OUString& GetPhysicalName() const;
    sal_Bool            IsRemote();
    sal_Bool            IsOpen() const; // { return aStorage.Is() || pInStream; }
    void                DownLoad( const Link& aLink = Link());
    void                SetDoneLink( const Link& rLink );

    sal_uInt32          GetErrorCode() const;
    sal_uInt32          GetError() const
                        { return ERRCODE_TOERROR(GetErrorCode()); }
    sal_uInt32          GetLastStorageCreationState();

    void                SetError( sal_uInt32 nError, const ::rtl::OUString& aLogMessage );

    void                AddLog( const ::rtl::OUString& aMessage );

    void                CloseInStream();
    sal_Bool            CloseOutStream();

    void                CloseStorage();

    StreamMode          GetOpenMode() const;
    void                SetOpenMode( StreamMode nStorOpen, sal_Bool bDontClose = sal_False );

    SvStream*           GetInStream();
    SvStream*           GetOutStream();

    sal_Bool            Commit();
    sal_Bool            IsStorage();

    sal_Int8            ShowLockedDocumentDialog( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aData, sal_Bool bIsLoading, sal_Bool bOwnLock );
    bool                LockOrigFileOnDemand( sal_Bool bLoading, sal_Bool bNoUI );
    void                UnlockFile( sal_Bool bReleaseLockStream );

    ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage > GetStorage( sal_Bool bCreateTempIfNo = sal_True );
    ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage > GetOutputStorage();
    void                ResetError();
    sal_Bool            UsesCache() const;
    void                SetUsesCache( sal_Bool );
    sal_Bool            IsExpired() const;
    void                SetName( const String& rName, sal_Bool bSetOrigURL = sal_False );
    sal_Bool            IsAllowedForExternalBrowser() const;
    long                GetFileVersion() const;

    const com::sun::star::uno::Sequence < com::sun::star::util::RevisionTag >&
                        GetVersionList( bool _bNoReload = false );
    sal_Bool            IsReadOnly();

    // Whether the medium had originally been opened r/o, independent of later
    // changes via SetOpenMode; used to keep track of the "true" state of the
    // medium across toggles via SID_EDITDOC (which do change SetOpenMode):
    bool                IsOriginallyReadOnly() const;

    ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >  GetInputStream();

    void                CreateTempFile( sal_Bool bReplace = sal_True );
    void                CreateTempFileNoCopy();
    ::rtl::OUString     SwitchDocumentToTempFile();
    sal_Bool            SwitchDocumentToFile( const rtl::OUString& aURL );

    ::rtl::OUString     GetBaseURL( bool bForSaving=false );
    void                SetInCheckIn( bool bInCheckIn );
    bool                IsInCheckIn( );

#if _SOLAR__PRIVATE
    SAL_DLLPRIVATE sal_Bool HasStorage_Impl() const;

    SAL_DLLPRIVATE void StorageBackup_Impl();
    SAL_DLLPRIVATE ::rtl::OUString GetBackup_Impl();

    SAL_DLLPRIVATE ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage > GetZipStorageToSign_Impl( sal_Bool bReadOnly = sal_True );
    SAL_DLLPRIVATE void CloseZipStorage_Impl();

    // the storage that will be returned by the medium on GetStorage request
    SAL_DLLPRIVATE void SetStorage_Impl( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xNewStorage );

    SAL_DLLPRIVATE ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > GetInputStream_Impl();
    SAL_DLLPRIVATE void CloseAndReleaseStreams_Impl();
    SAL_DLLPRIVATE sal_uInt16 AddVersion_Impl( com::sun::star::util::RevisionTag& rVersion );
    SAL_DLLPRIVATE sal_Bool TransferVersionList_Impl( SfxMedium& rMedium );
    SAL_DLLPRIVATE sal_Bool SaveVersionList_Impl( sal_Bool bUseXML );
    SAL_DLLPRIVATE sal_Bool RemoveVersion_Impl( const ::rtl::OUString& rVersion );

    SAL_DLLPRIVATE void SetExpired_Impl( const DateTime& rDateTime );
    SAL_DLLPRIVATE SvKeyValueIterator* GetHeaderAttributes_Impl();

    // Diese Protokolle liefern MIME Typen
    SAL_DLLPRIVATE sal_Bool SupportsMIME_Impl() const;

    SAL_DLLPRIVATE void Init_Impl();
    SAL_DLLPRIVATE void ForceSynchronStream_Impl( sal_Bool bSynchron );

    SAL_DLLPRIVATE void GetLockingStream_Impl();
    SAL_DLLPRIVATE void GetMedium_Impl();
    SAL_DLLPRIVATE sal_Bool TryDirectTransfer( const ::rtl::OUString& aURL, SfxItemSet& aTargetSet );
    SAL_DLLPRIVATE void Transfer_Impl();
    SAL_DLLPRIVATE void CreateFileStream();
    SAL_DLLPRIVATE void SetUpdatePickList(sal_Bool);
    SAL_DLLPRIVATE sal_Bool IsUpdatePickList() const;

    SAL_DLLPRIVATE void SetLongName(const rtl::OUString &rName);
    SAL_DLLPRIVATE const rtl::OUString & GetLongName() const;
    SAL_DLLPRIVATE ErrCode CheckOpenMode_Impl( sal_Bool bSilent, sal_Bool bAllowRO = sal_True );
    SAL_DLLPRIVATE sal_Bool IsPreview_Impl();
    SAL_DLLPRIVATE void ClearBackup_Impl();
    SAL_DLLPRIVATE void Done_Impl( ErrCode );
    SAL_DLLPRIVATE void SetPhysicalName_Impl(const rtl::OUString& rName);
    SAL_DLLPRIVATE void CanDisposeStorage_Impl( sal_Bool bDisposeStorage );
    SAL_DLLPRIVATE sal_Bool WillDisposeStorageOnClose_Impl();

    SAL_DLLPRIVATE void DoBackup_Impl();
    SAL_DLLPRIVATE void DoInternalBackup_Impl( const ::ucbhelper::Content& aOriginalContent );
    SAL_DLLPRIVATE void DoInternalBackup_Impl( const ::ucbhelper::Content& aOriginalContent,
                                                const String& aPrefix,
                                                const String& aExtension,
                                                const String& aDestDir );

    SAL_DLLPRIVATE sal_Bool UseBackupToRestore_Impl( ::ucbhelper::Content& aOriginalContent,
                             const ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XCommandEnvironment >& xComEnv );

    SAL_DLLPRIVATE sal_Bool StorageCommit_Impl();

    SAL_DLLPRIVATE sal_Bool TransactedTransferForFS_Impl( const INetURLObject& aSource,
                             const INetURLObject& aDest,
                             const ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XCommandEnvironment >& xComEnv );

    SAL_DLLPRIVATE sal_Bool SignContents_Impl( sal_Bool bScriptingContent, const ::rtl::OUString& aODFVersion, sal_Bool bHasValidDocumentSignature );

    // the following two methods must be used and make sence only during saving currently
    // TODO/LATER: in future the signature state should be controlled by the medium not by the document
    //             in this case the methods will be used generally, and might need to be renamed
    SAL_DLLPRIVATE sal_uInt16 GetCachedSignatureState_Impl();
    SAL_DLLPRIVATE void       SetCachedSignatureState_Impl( sal_uInt16 nState );
#endif

    static com::sun::star::uno::Sequence < com::sun::star::util::RevisionTag > GetVersionList(
                    const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage );
    static ::rtl::OUString CreateTempCopyWithExt( const ::rtl::OUString& aURL );
    static sal_Bool CallApproveHandler( const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& xHandler, ::com::sun::star::uno::Any aRequest, sal_Bool bAllowAbort );

    static sal_Bool     SetWritableForUserOnly( const ::rtl::OUString& aURL );
    static sal_uInt32   CreatePasswordToModifyHash( const ::rtl::OUString& aPasswd, sal_Bool bWriter );
};

SV_DECL_IMPL_REF( SfxMedium )

typedef ::std::vector< SfxMedium* > SfxMediumList;

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

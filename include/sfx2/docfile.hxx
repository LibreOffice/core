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

#ifndef INCLUDED_SFX2_DOCFILE_HXX
#define INCLUDED_SFX2_DOCFILE_HXX

#include <com/sun/star/io/XSeekable.hpp>
#include <sal/config.h>
#include <sfx2/dllapi.h>
#include <sfx2/signaturestate.hxx>
#include <svl/lockfilecommon.hxx>
#include <sal/types.h>
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
#include <cppuhelper/weak.hxx>
#include <rtl/ustring.hxx>
#include <svl/lstner.hxx>
#include <tools/link.hxx>
#include <tools/stream.hxx>
#include <ucbhelper/content.hxx>

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

class SFX2_DLLPUBLIC SfxMedium : public SvRefBase
{
    std::unique_ptr< SfxMedium_Impl > pImpl;

    SAL_DLLPRIVATE void SetIsRemote_Impl();
    SAL_DLLPRIVATE void CloseInStream_Impl();
    SAL_DLLPRIVATE void CloseOutStream_Impl();
    SAL_DLLPRIVATE void CloseStreams_Impl();

    SAL_DLLPRIVATE void SetEncryptionDataToStorage_Impl();

public:

                        SfxMedium();
                        /**
                         * @param pSet Takes ownership
                         */
                        SfxMedium( const OUString &rName,
                                   StreamMode nOpenMode,
                                   std::shared_ptr<const SfxFilter> pFilter = nullptr,
                                   SfxItemSet *pSet = nullptr );
                        /**
                         * @param pSet Takes ownership
                         */
                        SfxMedium( const OUString &rName,
                                   const OUString &rReferer,
                                   StreamMode nOpenMode,
                                   std::shared_ptr<const SfxFilter> pFilter = nullptr,
                                   SfxItemSet *pSet = nullptr );

                        /**
                         * @param pSet does NOT take ownership
                         */
                        SfxMedium( const css::uno::Reference< css::embed::XStorage >& xStorage,
                                    const OUString& rBaseURL,
                                    const SfxItemSet* pSet=nullptr );
                        /**
                         * @param pSet does NOT take ownership
                         */
                        SfxMedium( const css::uno::Reference< css::embed::XStorage >& xStorage,
                                    const OUString& rBaseURL,
                                    const OUString& rTypeName,
                                    const SfxItemSet* pSet=nullptr );
                        SfxMedium( const css::uno::Sequence< css::beans::PropertyValue >& aArgs );

                        virtual ~SfxMedium() override;

    void                UseInteractionHandler( bool );
    css::uno::Reference< css::task::XInteractionHandler >
                        GetInteractionHandler( bool bGetAlways = false );

    void setStreamToLoadFrom(
        const css::uno::Reference<css::io::XInputStream>& xInputStream,
        bool bIsReadOnly);

    void                SetLoadTargetFrame(SfxFrame* pFrame );
    SfxFrame*           GetLoadTargetFrame() const;

    /**
     * Does not take ownership of pFlt but pFlt needs to be around as long as the SfxMedium instance.
     */
    void                SetFilter(const std::shared_ptr<const SfxFilter>& pFilter);
    const std::shared_ptr<const SfxFilter>& GetFilter() const;
    std::shared_ptr<const SfxFilter> const & GetOrigFilter() const;
    const OUString&     GetOrigURL() const;

    SfxItemSet  *       GetItemSet() const;
    void                Close();
    void                CloseAndRelease();
    void                ReOpen();
    void                CompleteReOpen();
    const OUString& GetName() const;
    const INetURLObject& GetURLObject() const;

    void                CheckFileDate( const css::util::DateTime& aInitDate );
    SAL_WARN_UNUSED_RESULT bool  DocNeedsFileDateCheck() const;
    css::util::DateTime GetInitFileDate( bool bIgnoreOldValue );

    css::uno::Reference< css::ucb::XContent > GetContent() const;
    const OUString& GetPhysicalName() const;
    SAL_WARN_UNUSED_RESULT bool IsRemote() const;
    SAL_WARN_UNUSED_RESULT bool IsOpen() const; // { return aStorage.Is() || pInStream; }
    void                Download( const Link<void*,void>& aLink = Link<void*,void>());
    void                SetDoneLink( const Link<void*,void>& rLink );

    sal_uInt32          GetErrorCode() const;
    sal_uInt32          GetError() const
                        { return IgnoreWarning(GetErrorCode()); }
    sal_uInt32          GetLastStorageCreationState();

    void                SetError( sal_uInt32 nError, const OUString& aLogMessage );

    void                AddLog( const OUString& aMessage );

    void                CloseInStream();
    bool                CloseOutStream();

    void                CloseStorage();

    StreamMode          GetOpenMode() const;
    void                SetOpenMode( StreamMode nStorOpen, bool bDontClose = false );

    SvStream*           GetInStream();
    SvStream*           GetOutStream();

    bool                Commit();
    bool                IsStorage();

    enum class ShowLockResult { NoLock, Succeeded,Try };
    ShowLockResult      ShowLockedDocumentDialog( const LockFileEntry& aData, bool bIsLoading, bool bOwnLock );
    void                LockOrigFileOnDemand( bool bLoading, bool bNoUI );
    void                DisableUnlockWebDAV( bool bDisableUnlockWebDAV = true );
    void                UnlockFile( bool bReleaseLockStream );

    css::uno::Reference< css::embed::XStorage > GetStorage( bool bCreateTempIfNo = true );
    css::uno::Reference< css::embed::XStorage > GetOutputStorage();
    void                ResetError();
    SAL_WARN_UNUSED_RESULT bool  IsExpired() const;
    void                SetName( const OUString& rName, bool bSetOrigURL = false );

    const css::uno::Sequence < css::util::RevisionTag >&
                        GetVersionList( bool _bNoReload = false );
    SAL_WARN_UNUSED_RESULT bool  IsReadOnly() const;

    // Whether the medium had originally been opened r/o (either because it is
    // "physically" r/o, or because it was requested to be opended r/o,
    // independent of later changes via SetOpenMode; used to keep track of the
    // "true" state of the medium across toggles via SID_EDITDOC (which do
    // change SetOpenMode):
    SAL_WARN_UNUSED_RESULT bool  IsOriginallyReadOnly() const;

    // Whether the medium had originally been requested to be opened r/o,
    // independent of later changes via SetOpenMode; used for SID_RELOAD:
    SAL_WARN_UNUSED_RESULT bool IsOriginallyLoadedReadOnly() const;

    css::uno::Reference< css::io::XInputStream >  GetInputStream();

    void                CreateTempFile( bool bReplace = true );
    void                CreateTempFileNoCopy();
    OUString            SwitchDocumentToTempFile();
    bool                SwitchDocumentToFile( const OUString& aURL );

    OUString            GetBaseURL( bool bForSaving=false );
    void                SetInCheckIn( bool bInCheckIn );
    bool                IsInCheckIn( );
    bool                IsSkipImages( );

    SAL_DLLPRIVATE bool HasStorage_Impl() const;

    SAL_DLLPRIVATE void StorageBackup_Impl();
    SAL_DLLPRIVATE OUString GetBackup_Impl();

    SAL_DLLPRIVATE css::uno::Reference< css::embed::XStorage > GetZipStorageToSign_Impl( bool bReadOnly = true );
    SAL_DLLPRIVATE void CloseZipStorage_Impl();

    // the storage that will be returned by the medium on GetStorage request
    SAL_DLLPRIVATE void SetStorage_Impl( const css::uno::Reference< css::embed::XStorage >& xNewStorage );

    SAL_DLLPRIVATE void CloseAndReleaseStreams_Impl();
    SAL_DLLPRIVATE void AddVersion_Impl( css::util::RevisionTag& rVersion );
    SAL_DLLPRIVATE bool TransferVersionList_Impl( SfxMedium& rMedium );
    SAL_DLLPRIVATE void SaveVersionList_Impl();
    SAL_DLLPRIVATE void RemoveVersion_Impl( const OUString& rVersion );

    SAL_DLLPRIVATE void SetExpired_Impl( const DateTime& rDateTime );
    SAL_DLLPRIVATE SvKeyValueIterator* GetHeaderAttributes_Impl();

    SAL_DLLPRIVATE void Init_Impl();

    SAL_DLLPRIVATE void GetLockingStream_Impl();
    SAL_DLLPRIVATE void GetMedium_Impl();
    SAL_DLLPRIVATE bool TryDirectTransfer( const OUString& aURL, SfxItemSet& aTargetSet );
    SAL_DLLPRIVATE void Transfer_Impl();
    SAL_DLLPRIVATE void CreateFileStream();
    SAL_DLLPRIVATE void SetUpdatePickList(bool);
    SAL_DLLPRIVATE bool IsUpdatePickList() const;

    SAL_DLLPRIVATE void SetLongName(const OUString &rName);
    SAL_DLLPRIVATE const OUString & GetLongName() const;
    SAL_DLLPRIVATE bool IsPreview_Impl();
    SAL_DLLPRIVATE void ClearBackup_Impl();
    SAL_DLLPRIVATE void SetPhysicalName_Impl(const OUString& rName);
    SAL_DLLPRIVATE void CanDisposeStorage_Impl( bool bDisposeStorage );
    SAL_DLLPRIVATE bool WillDisposeStorageOnClose_Impl();

    SAL_DLLPRIVATE void DoBackup_Impl();
    SAL_DLLPRIVATE void DoInternalBackup_Impl( const ::ucbhelper::Content& aOriginalContent );
    SAL_DLLPRIVATE void DoInternalBackup_Impl( const ::ucbhelper::Content& aOriginalContent,
                                                const OUString& aPrefix,
                                                const OUString& aExtension,
                                                const OUString& aDestDir );

    SAL_DLLPRIVATE bool UseBackupToRestore_Impl( ::ucbhelper::Content& aOriginalContent,
                             const css::uno::Reference< css::ucb::XCommandEnvironment >& xComEnv );

    SAL_DLLPRIVATE bool StorageCommit_Impl();

    SAL_DLLPRIVATE void TransactedTransferForFS_Impl( const INetURLObject& aSource,
                             const INetURLObject& aDest,
                             const css::uno::Reference< css::ucb::XCommandEnvironment >& xComEnv );

    SAL_DLLPRIVATE bool SignContents_Impl( bool bScriptingContent, const OUString& aODFVersion, bool bHasValidDocumentSignature );

    // the following two methods must be used and make sense only during saving currently
    // TODO/LATER: in future the signature state should be controlled by the medium not by the document
    //             in this case the methods will be used generally, and might need to be renamed
    SAL_DLLPRIVATE SignatureState GetCachedSignatureState_Impl();
    SAL_DLLPRIVATE void       SetCachedSignatureState_Impl( SignatureState nState );

    static css::uno::Sequence < css::util::RevisionTag > GetVersionList(
                    const css::uno::Reference< css::embed::XStorage >& xStorage );
    static OUString CreateTempCopyWithExt( const OUString& aURL );
    static bool CallApproveHandler(const css::uno::Reference< css::task::XInteractionHandler >& xHandler, const css::uno::Any& rRequest, bool bAllowAbort);

    static bool         SetWritableForUserOnly( const OUString& aURL );
    static sal_uInt32   CreatePasswordToModifyHash( const OUString& aPasswd, bool bWriter );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

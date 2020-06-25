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

#include <memory>
#include <sal/config.h>
#include <sfx2/dllapi.h>
#include <sfx2/signaturestate.hxx>
#include <svl/lockfilecommon.hxx>
#include <sal/types.h>
#include <rtl/ustring.hxx>
#include <svl/itemset.hxx>
#include <tools/link.hxx>
#include <tools/stream.hxx>

namespace com::sun::star::beans { struct PropertyValue; }
namespace com::sun::star::embed { class XStorage; }
namespace com::sun::star::graphic { class XGraphic; }
namespace com::sun::star::io { class XInputStream; }
namespace com::sun::star::security { class XCertificate; }
namespace com::sun::star::task { class XInteractionHandler; }
namespace com::sun::star::ucb { class XCommandEnvironment; }
namespace com::sun::star::ucb { class XContent; }
namespace com::sun::star::util { struct DateTime; }
namespace com::sun::star::util { struct RevisionTag; }
namespace com::sun::star::frame
{
class XModel;
}
namespace ucbhelper { class Content; }

class SvKeyValueIterator;
class SfxFilter;
class SfxMedium_Impl;
class INetURLObject;
class SfxFrame;
class DateTime;

namespace weld
{
    class Window;
}

class SFX2_DLLPUBLIC SfxMedium final : public SvRefBase
{
    std::unique_ptr< SfxMedium_Impl > pImpl;

    SAL_DLLPRIVATE void SetIsRemote_Impl();
    SAL_DLLPRIVATE void CloseInStream_Impl(bool bInDestruction = false);
    SAL_DLLPRIVATE void CloseOutStream_Impl();
    SAL_DLLPRIVATE void CloseStreams_Impl(bool bInDestruction = false);

    SAL_DLLPRIVATE void SetEncryptionDataToStorage_Impl();

public:

                        SfxMedium();
                        SfxMedium( const OUString &rName,
                                   StreamMode nOpenMode,
                                   std::shared_ptr<const SfxFilter> pFilter = nullptr,
                                   const std::shared_ptr<SfxItemSet>& pSet = nullptr );
                        SfxMedium( const OUString &rName,
                                   const OUString &rReferer,
                                   StreamMode nOpenMode,
                                   std::shared_ptr<const SfxFilter> pFilter = nullptr,
                                   const std::shared_ptr<SfxItemSet>& pSet = nullptr );
                        SfxMedium( const css::uno::Reference< css::embed::XStorage >& xStorage,
                                   const OUString& rBaseURL,
                                   const std::shared_ptr<SfxItemSet>& pSet = nullptr  );
                        SfxMedium( const css::uno::Reference< css::embed::XStorage >& xStorage,
                                   const OUString& rBaseURL,
                                   const OUString& rTypeName,
                                   const std::shared_ptr<SfxItemSet>& pSet = nullptr );
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
    const OUString&     GetOrigURL() const;

    SfxItemSet  *       GetItemSet() const;
    void SetArgs(const css::uno::Sequence<css::beans::PropertyValue>& rArgs);
    css::uno::Sequence<css::beans::PropertyValue> GetArgs() const;
    void                Close(bool bInDestruction = false);
    void                CloseAndRelease();
    void                ReOpen();
    void                CompleteReOpen();
    const OUString& GetName() const;
    const INetURLObject& GetURLObject() const;

    void                CheckFileDate( const css::util::DateTime& aInitDate );
    [[nodiscard]] bool  DocNeedsFileDateCheck() const;
    css::util::DateTime const & GetInitFileDate( bool bIgnoreOldValue );

    css::uno::Reference< css::ucb::XContent > GetContent() const;
    const OUString& GetPhysicalName() const;
    [[nodiscard]] bool IsRemote() const;
    [[nodiscard]] bool IsOpen() const; // { return aStorage.Is() || pInStream; }
    void                Download( const Link<void*,void>& aLink = Link<void*,void>());
    void                SetDoneLink( const Link<void*,void>& rLink );

    ErrCode             GetErrorCode() const;
    ErrCode             GetError() const
                        { return GetErrorCode().IgnoreWarning(); }
    ErrCode const &     GetLastStorageCreationState() const;

    void                SetError(ErrCode nError);

    void                CloseInStream();
    void                CloseOutStream();

    void                CloseStorage();

    StreamMode          GetOpenMode() const;
    void                SetOpenMode( StreamMode nStorOpen, bool bDontClose = false );

    SvStream*           GetInStream();
    SvStream*           GetOutStream();

    bool                Commit();
    bool                IsStorage();

    enum class          LockFileResult
    {
        Failed,
        FailedLockFile, // there was only lock file that prevented success - no syslock or IO error
        Succeeded,
    };
    LockFileResult LockOrigFileOnDemand(bool bLoading, bool bNoUI, bool bTryIgnoreLockFile = false,
                                        LockFileEntry* pLockData = nullptr);
    void                DisableUnlockWebDAV( bool bDisableUnlockWebDAV = true );
    void                UnlockFile( bool bReleaseLockStream );
    /// Lets Transfer_Impl() not fsync the output file.
    void DisableFileSync(bool bDisableFileSync);

    css::uno::Reference< css::embed::XStorage > GetStorage( bool bCreateTempFile = true );
    css::uno::Reference< css::embed::XStorage > GetOutputStorage();
    void                ResetError();
    [[nodiscard]] bool  IsExpired() const;
    void                SetName( const OUString& rName, bool bSetOrigURL = false );

    const css::uno::Sequence < css::util::RevisionTag >&
                        GetVersionList( bool _bNoReload = false );
    [[nodiscard]] bool  IsReadOnly() const;

    // Whether the medium had originally been opened r/o (either because it is
    // "physically" r/o, or because it was requested to be opened r/o,
    // independent of later changes via SetOpenMode; used to keep track of the
    // "true" state of the medium across toggles via SID_EDITDOC (which do
    // change SetOpenMode):
    [[nodiscard]] bool  IsOriginallyReadOnly() const;

    // Whether the medium had originally been requested to be opened r/o,
    // independent of later changes via SetOpenMode; used for SID_RELOAD:
    [[nodiscard]] bool IsOriginallyLoadedReadOnly() const;

    css::uno::Reference< css::io::XInputStream > const &  GetInputStream();

    void                CreateTempFile( bool bReplace = true );
    void                CreateTempFileNoCopy();
    OUString            SwitchDocumentToTempFile();
    bool                SwitchDocumentToFile( const OUString& aURL );

    OUString            GetBaseURL( bool bForSaving=false );
    void                SetInCheckIn( bool bInCheckIn );
    bool                IsInCheckIn( ) const;
    bool                IsSkipImages( ) const;

    SAL_DLLPRIVATE bool HasStorage_Impl() const;

    SAL_DLLPRIVATE void StorageBackup_Impl();
    SAL_DLLPRIVATE OUString const & GetBackup_Impl();

    SAL_DLLPRIVATE css::uno::Reference< css::embed::XStorage > const & GetZipStorageToSign_Impl( bool bReadOnly = true );
    SAL_DLLPRIVATE void CloseZipStorage_Impl();

    // the storage that will be returned by the medium on GetStorage request
    SAL_DLLPRIVATE void SetStorage_Impl( const css::uno::Reference< css::embed::XStorage >& xNewStorage );

    SAL_DLLPRIVATE void CloseAndReleaseStreams_Impl();
    SAL_DLLPRIVATE void AddVersion_Impl( css::util::RevisionTag& rVersion );
    SAL_DLLPRIVATE bool TransferVersionList_Impl( SfxMedium const & rMedium );
    SAL_DLLPRIVATE void SaveVersionList_Impl();
    SAL_DLLPRIVATE void RemoveVersion_Impl( const OUString& rVersion );

    SAL_DLLPRIVATE void SetExpired_Impl( const DateTime& rDateTime );
    SAL_DLLPRIVATE SvKeyValueIterator* GetHeaderAttributes_Impl();

    SAL_DLLPRIVATE void Init_Impl();

    SAL_DLLPRIVATE void GetLockingStream_Impl();
    SAL_DLLPRIVATE void GetMedium_Impl();
    SAL_DLLPRIVATE bool TryDirectTransfer( const OUString& aURL, SfxItemSet const & aTargetSet );
    SAL_DLLPRIVATE void Transfer_Impl();
    SAL_DLLPRIVATE void CreateFileStream();
    SAL_DLLPRIVATE void SetUpdatePickList(bool);
    SAL_DLLPRIVATE bool IsUpdatePickList() const;

    SAL_DLLPRIVATE void SetLongName(const OUString &rName);
    SAL_DLLPRIVATE const OUString & GetLongName() const;
    SAL_DLLPRIVATE bool IsPreview_Impl() const;
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

    SAL_DLLPRIVATE bool
    SignContents_Impl(weld::Window* pDialogParent,
                      bool bSignScriptingContent, bool bHasValidDocumentSignature,
                      const OUString& aSignatureLineId = OUString(),
                      const css::uno::Reference<css::security::XCertificate>& xCert
                      = css::uno::Reference<css::security::XCertificate>(),
                      const css::uno::Reference<css::graphic::XGraphic>& xValidGraphic
                      = css::uno::Reference<css::graphic::XGraphic>(),
                      const css::uno::Reference<css::graphic::XGraphic>& xInvalidGraphic
                      = css::uno::Reference<css::graphic::XGraphic>(),
                      const OUString& aComment = OUString());

    SAL_DLLPRIVATE bool SignDocumentContentUsingCertificate(
        const css::uno::Reference<css::frame::XModel>& xModel, bool bHasValidDocumentSignature,
        const css::uno::Reference<css::security::XCertificate>& xCertificate);

    // the following two methods must be used and make sense only during saving currently
    // TODO/LATER: in future the signature state should be controlled by the medium not by the document
    //             in this case the methods will be used generally, and might need to be renamed
    SAL_DLLPRIVATE SignatureState GetCachedSignatureState_Impl() const;
    SAL_DLLPRIVATE void       SetCachedSignatureState_Impl( SignatureState nState );

    void SetHasEmbeddedObjects(bool bHasEmbeddedObjects);

    static css::uno::Sequence < css::util::RevisionTag > GetVersionList(
                    const css::uno::Reference< css::embed::XStorage >& xStorage );
    static OUString CreateTempCopyWithExt( const OUString& aURL );
    static bool CallApproveHandler(const css::uno::Reference< css::task::XInteractionHandler >& xHandler, const css::uno::Any& rRequest, bool bAllowAbort);

    static bool         SetWritableForUserOnly( const OUString& aURL );
    static sal_uInt32   CreatePasswordToModifyHash( const OUString& aPasswd, bool bWriter );

private:
    enum class ShowLockResult { NoLock, Succeeded, Try };
    ShowLockResult ShowLockedDocumentDialog(const LockFileEntry& aData,
                                            bool bIsLoading, bool bOwnLock, bool bHandleSysLocked);
    enum class MessageDlg { LockFileIgnore, LockFileCorrupt };
    bool                ShowLockFileProblemDialog(MessageDlg nWhichDlg);

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

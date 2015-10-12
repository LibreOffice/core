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
#ifndef INCLUDED_SFX2_SOURCE_INC_OBJSHIMP_HXX
#define INCLUDED_SFX2_SOURCE_INC_OBJSHIMP_HXX

#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <rtl/ustring.hxx>
#include <rtl/ref.hxx>
#include <com/sun/star/logging/XSimpleLogRing.hpp>
#include <tools/datetime.hxx>

#include <unotools/securityoptions.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/docmacromode.hxx>
#include "bitset.hxx"

#include <appbaslib.hxx>

namespace svtools { class AsynchronLink; }

class SfxViewFrame;
struct MarkData_Impl
{
    OUString aMark;
    OUString aUserData;
    SfxViewFrame* pFrame;
};

class SfxBasicManagerHolder;

struct SfxObjectShell_Impl : public ::sfx2::IMacroDocumentAccess
{
    ::comphelper::EmbeddedObjectContainer* mpObjectContainer;
    SfxBasicManagerHolder aBasicManager;
    SfxObjectShell&     rDocShell;
    ::com::sun::star::uno::Reference< ::com::sun::star::script::XLibraryContainer >
                        xBasicLibraries;
    ::com::sun::star::uno::Reference< ::com::sun::star::script::XLibraryContainer >
                        xDialogLibraries;
    com::sun::star::uno::Sequence < OUString > xEventNames;
    ::sfx2::DocumentMacroMode
                        aMacroMode;
    SfxProgress*        pProgress;
    OUString            aTitle;
    OUString            aTempName;
    DateTime            nTime;
    sal_uInt16          nVisualDocumentNumber;
    SignatureState      nDocumentSignatureState;
    SignatureState      nScriptingSignatureState;
    bool            bInList:1,          // if reachable by First/Next
                        bClosing:1,         // sal_True while Close(), to prevent recurrences Notification
                        bIsSaving:1,
                        bPasswd:1,
                        bIsNamedVisible:1,
                        bIsTemplate:1,
                        bIsAbortingImport:1,  // Import operation should be canceled.
                        bImportDone : 1, // Import finished already? For auto reload of Docs.
                        bInPrepareClose : 1,
                        bPreparedForClose : 1,
                        bForbidReload : 1,
                        bBasicInitialized :1,
                        bIsPrintJobCancelable :1, // Stampit disable/enable cancel button for print jobs ... default = true = enable!
                        bOwnsStorage:1,
                        bNoBaseURL:1,
                        bInitialized:1,
                        bSignatureErrorIsShown:1,
                        bModelInitialized:1, // whether the related model is initialized
                        bPreserveVersions:1,
                        m_bMacroSignBroken:1, // whether the macro signature was explicitly broken
                        m_bNoBasicCapabilities:1,
                        m_bDocRecoverySupport:1,
                        bQueryLoadTemplate:1,
                        bLoadReadonly:1,
                        bUseUserData:1,
                        bSaveVersionOnClose:1,
                        m_bSharedXMLFlag:1, // whether the flag should be stored in xml file
                        m_bAllowShareControlFileClean:1, // whether the flag should be stored in xml file
                        m_bConfigOptionsChecked:1; // whether or not the user options are checked after the Options dialog is closed.

    IndexBitSet         aBitSet;
    sal_uInt32          lErr;
    sal_uInt16          nEventId;           // If Open/Create as to be sent
                                            // before Activate
    AutoReloadTimer_Impl *pReloadTimer;
    MarkData_Impl*      pMarkData;
    SfxLoadedFlags      nLoadedFlags;
    SfxLoadedFlags      nFlagsInProgress;
    bool                bModalMode;
    bool                bRunningMacro;
    bool                bReloadAvailable;
    sal_uInt16          nAutoLoadLocks;
    SfxModule*          pModule;
    SfxObjectShellFlags     eFlags;
    bool                bReadOnlyUI;
    tools::SvRef<SvRefBase>  xHeaderAttributes;
    ::rtl::Reference< SfxBaseModel >
                            pBaseModel;
    sal_uInt16          nStyleFilter;
    bool                bDisposing;

    bool                m_bEnableSetModified;
    bool                m_bIsModified;

    Rectangle           m_aVisArea;
    MapUnit             m_nMapUnit;

    bool                m_bCreateTempStor;
    ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage > m_xDocStorage;

    bool                m_bIsInit;

    OUString         m_aSharedFileURL;

    ::com::sun::star::uno::Reference< ::com::sun::star::logging::XSimpleLogRing > m_xLogRing;

    bool                m_bIncomplEncrWarnShown;

    // TODO/LATER: m_aModifyPasswordInfo should completely replace m_nModifyPasswordHash in future
    sal_uInt32              m_nModifyPasswordHash;
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > m_aModifyPasswordInfo;
    bool                m_bModifyPasswordEntered;

    SfxObjectShell_Impl( SfxObjectShell& _rDocShell );
    virtual ~SfxObjectShell_Impl();

    // IMacroDocumentAccess overridables
    virtual sal_Int16 getCurrentMacroExecMode() const override;
    virtual bool setCurrentMacroExecMode( sal_uInt16 nMacroMode ) override;
    virtual OUString getDocumentLocation() const override;
    virtual bool documentStorageHasMacros() const override;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::document::XEmbeddedScripts > getEmbeddedDocumentScripts() const override;
    virtual SignatureState getScriptingSignatureState() override;

    virtual bool hasTrustedScriptingSignature( bool bAllowUIToAddAuthor ) override;
    virtual void showBrokenSignatureWarning( const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& _rxInteraction ) const override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

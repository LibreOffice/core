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

#include <com/sun/star/uno/Sequence.hxx>
#include <rtl/ustring.hxx>
#include <rtl/ref.hxx>
#include <tools/datetime.hxx>
#include <tools/ref.hxx>

#include <sfx2/objsh.hxx>
#include <sfx2/docmacromode.hxx>
#include <sfx2/namedcolor.hxx>
#include <bitset.hxx>
#include <vcl/timer.hxx>

#include "appbaslib.hxx"

namespace svtools { class AsynchronLink; }

class SfxViewFrame;

class SfxBasicManagerHolder;

class AutoReloadTimer_Impl final : public Timer
{
    OUString          aUrl;
    SfxObjectShell*   pObjSh;

public:
    AutoReloadTimer_Impl( OUString aURL, sal_uInt32 nTime,
                          SfxObjectShell* pSh );
    virtual void Invoke() override;
};

struct SfxObjectShell_Impl final : public ::sfx2::IMacroDocumentAccess
{
    std::unique_ptr<::comphelper::EmbeddedObjectContainer> mxObjectContainer;
    SfxBasicManagerHolder aBasicManager;
    SfxObjectShell&     rDocShell;
    css::uno::Reference< css::script::XLibraryContainer >
                        xBasicLibraries;
    css::uno::Reference< css::script::XLibraryContainer >
                        xDialogLibraries;
    ::sfx2::DocumentMacroMode
                        aMacroMode;
    SfxProgress*        pProgress;
    OUString            aTitle;
    OUString            aTempName;
    DateTime            nTime;
    sal_uInt16          nVisualDocumentNumber;
    SignatureState      nDocumentSignatureState;
    SignatureState      nScriptingSignatureState;
    bool                bClosing:1,         // sal_True while Close(), to prevent recurrences Notification
                        bIsSaving:1,
                        bIsNamedVisible:1,
                        bIsAbortingImport:1,  // Import operation should be canceled.
                        bInPrepareClose : 1,
                        bPreparedForClose : 1,
                        bForbidReload : 1,
                        bBasicInitialized :1,
                        bIsPrintJobCancelable :1, // Stampit disable/enable cancel button for print jobs ... default = true = enable!
                        bOwnsStorage:1,
                        bInitialized:1,
                        bModelInitialized:1, // whether the related model is initialized
                        bPreserveVersions:1,
                        m_bMacroSignBroken:1, // whether the macro signature was explicitly broken
                        m_bNoBasicCapabilities:1,
                        m_bDocRecoverySupport:1,
                        bQueryLoadTemplate:1,
                        bLoadReadonly:1,
                        bUseUserData:1,
                        bUseThumbnailSave:1,
                        bSaveVersionOnClose:1,
                        m_bSharedXMLFlag:1, // whether the document should be edited in shared mode
                        m_bAllowShareControlFileClean:1, // whether the flag should be stored in xml file
                        m_bConfigOptionsChecked:1, // whether or not the user options are checked after the Options dialog is closed.
                        m_bMacroCallsSeenWhileLoading:1, // whether or not macro calls were seen when loading document.
                        m_bHadCheckedMacrosOnLoad:1; // if document contained macros (or calls) when loaded

    IndexBitSet         aBitSet;
    ErrCodeMsg          lErr;
    SfxEventHintId      nEventId;           // If Open/Create as to be sent
                                            // before Activate
    std::unique_ptr<AutoReloadTimer_Impl> pReloadTimer;
    SfxLoadedFlags      nLoadedFlags;
    SfxLoadedFlags      nFlagsInProgress;
    bool                bModalMode;
    bool                bRunningMacro;
    bool                bReadOnlyUI;
    tools::SvRef<SvKeyValueIterator>  xHeaderAttributes;
    ::rtl::Reference< SfxBaseModel >
                        pBaseModel;
    sal_uInt16          nStyleFilter;

    bool                m_bEnableSetModified;
    bool                m_bIsModified;

    tools::Rectangle           m_aVisArea;
    MapUnit             m_nMapUnit;

    bool                m_bCreateTempStor;
    css::uno::Reference< css::embed::XStorage > m_xDocStorage;

    bool                m_bIsInit;

    OUString            m_aSharedFileURL;

    bool                m_bIncomplEncrWarnShown;

    // TODO/LATER: m_aModifyPasswordInfo should completely replace m_nModifyPasswordHash in future
    sal_uInt32          m_nModifyPasswordHash;
    css::uno::Sequence< css::beans::PropertyValue > m_aModifyPasswordInfo;
    bool                m_bModifyPasswordEntered;
    /// If true, then this is not a real save, just the signatures change.
    bool m_bSavingForSigning;
    bool m_bAllowModifiedBackAfterSigning;

    /// Holds Infobars until View is fully loaded
    std::vector<InfobarData> m_aPendingInfobars;

    // Recent colors used by toolbar buttons
    std::unordered_map<sal_uInt16, NamedColor> m_aRecentColors;

    SfxObjectShell_Impl( SfxObjectShell& _rDocShell );
    virtual ~SfxObjectShell_Impl();

    // IMacroDocumentAccess overridables
    virtual sal_Int16 getCurrentMacroExecMode() const override;
    virtual void setCurrentMacroExecMode( sal_uInt16 nMacroMode ) override;
    virtual OUString getDocumentLocation() const override;
    virtual bool documentStorageHasMacros() const override;
    virtual bool macroCallsSeenWhileLoading() const override;
    virtual css::uno::Reference< css::document::XEmbeddedScripts > getEmbeddedDocumentScripts() const override;
    virtual SignatureState getScriptingSignatureState() override;

    virtual bool hasTrustedScriptingSignature(
        const css::uno::Reference<css::task::XInteractionHandler>& _rxInteraction) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

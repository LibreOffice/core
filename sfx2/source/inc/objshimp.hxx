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
#ifndef _SFX_OBJSHIMP_HXX
#define _SFX_OBJSHIMP_HXX

#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <rtl/ustring.hxx>
#include <rtl/ref.hxx>
#include <com/sun/star/logging/XSimpleLogRing.hpp>
#include <tools/datetime.hxx>

#include <unotools/securityoptions.hxx>
#include <sfx2/objsh.hxx>
#include "sfx2/docmacromode.hxx"
#include "bitset.hxx"

namespace svtools { class AsynchronLink; }

//====================================================================

DBG_NAMEEX(SfxObjectShell)

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
    SfxBasicManagerHolder*
                        pBasicManager;
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
    sal_Int16           nDocumentSignatureState;
    sal_Int16           nScriptingSignatureState;
    sal_Bool            bInList:1,          // if reachable by First/Next
                        bClosing:1,         // sal_True while Close(), to prevent recurrences Notification
                        bIsSaving:1,
                        bPasswd:1,
                        bIsTmp:1,
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
    sal_uInt32               lErr;
    sal_uInt16          nEventId;           // If Open/Create as to be sent
                                            // before Activate
    AutoReloadTimer_Impl *pReloadTimer;
    MarkData_Impl*      pMarkData;
    sal_uInt16              nLoadedFlags;
    sal_uInt16              nFlagsInProgress;
    sal_Bool                bModalMode;
    sal_Bool                bRunningMacro;
    sal_Bool                bReloadAvailable;
    sal_uInt16              nAutoLoadLocks;
    SfxModule*              pModule;
    SfxObjectShellFlags     eFlags;
    sal_Bool                bReadOnlyUI;
    SvRefBaseRef            xHeaderAttributes;
    ::rtl::Reference< SfxBaseModel >
                            pBaseModel;
    sal_uInt16              nStyleFilter;
    sal_Bool                bDisposing;

    sal_Bool                m_bEnableSetModified;
    sal_Bool                m_bIsModified;

    Rectangle               m_aVisArea;
    MapUnit                 m_nMapUnit;

    sal_Bool                m_bCreateTempStor;
    ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage > m_xDocStorage;

    sal_Bool                m_bIsInit;

    OUString         m_aSharedFileURL;

    ::com::sun::star::uno::Reference< ::com::sun::star::logging::XSimpleLogRing > m_xLogRing;

    sal_Bool                m_bIncomplEncrWarnShown;

    // TODO/LATER: m_aModifyPasswordInfo should completely replace m_nModifyPasswordHash in future
    sal_uInt32              m_nModifyPasswordHash;
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > m_aModifyPasswordInfo;
    sal_Bool                m_bModifyPasswordEntered;

    SfxObjectShell_Impl( SfxObjectShell& _rDocShell );
    virtual ~SfxObjectShell_Impl();

    // IMacroDocumentAccess overridables
    virtual sal_Int16 getCurrentMacroExecMode() const;
    virtual sal_Bool setCurrentMacroExecMode( sal_uInt16 nMacroMode );
    virtual OUString getDocumentLocation() const;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage > getZipStorageToSign();
    virtual sal_Bool documentStorageHasMacros() const;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::document::XEmbeddedScripts > getEmbeddedDocumentScripts() const;
    virtual sal_Int16 getScriptingSignatureState();

    virtual sal_Bool hasTrustedScriptingSignature( sal_Bool bAllowUIToAddAuthor );
    virtual void showBrokenSignatureWarning( const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& _rxInteraction ) const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

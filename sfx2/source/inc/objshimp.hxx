/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: objshimp.hxx,v $
 * $Revision: 1.46 $
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
#ifndef _SFX_OBJSHIMP_HXX
#define _SFX_OBJSHIMP_HXX

//#include <hash_map>

#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <rtl/ustring.hxx>
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
    String aMark;
    String aUserData;
    SfxViewFrame* pFrame;
};

class SfxFrame;
class SfxToolBoxConfig;
class SfxAcceleratorManager;
class SfxBasicManagerHolder;

struct SfxObjectShell_Impl : public ::sfx2::IMacroDocumentAccess
{
    ::comphelper::EmbeddedObjectContainer* mpObjectContainer;
    SfxAcceleratorManager*  pAccMgr;
    SfxConfigManager*   pCfgMgr;
    SfxBasicManagerHolder*
                        pBasicManager;
    SfxObjectShell&     rDocShell;
    ::com::sun::star::uno::Reference< ::com::sun::star::script::XLibraryContainer >
                        xBasicLibraries;
    ::com::sun::star::uno::Reference< ::com::sun::star::script::XLibraryContainer >
                        xDialogLibraries;
    com::sun::star::uno::Sequence < rtl::OUString > xEventNames;
    ::sfx2::DocumentMacroMode
                        aMacroMode;
    SfxProgress*        pProgress;
    String              aTitle;
    String              aTempName;
    DateTime            nTime;
    sal_uInt16          nVisualDocumentNumber;
    sal_Int16           nDocumentSignatureState;
    sal_Int16           nScriptingSignatureState;
    sal_Bool            bTemplateConfig:1,
                        bInList:1,          // ob per First/Next erreichbar
                        bClosing:1,         // sal_True w"aehrend Close(), um Benachrichtigungs-Rekursionen zu verhindern
                        bSetInPlaceObj:1,   // sal_True, falls bereits versucht wurde pInPlaceObject zu casten
                        bIsSaving:1,
                        bPasswd:1,
                        bIsTmp:1,
                        bIsNamedVisible:1,
                        bIsTemplate:1,
                        bIsAbortingImport:1,  // Importvorgang soll abgebrochen werden.
                        bImportDone : 1, //Import schon fertig? Fuer AutoReload von Docs.
                        bInPrepareClose : 1,
                        bPreparedForClose : 1,
                        bWaitingForPicklist : 1,// Muss noch in die Pickliste
                        bModuleSearched : 1,
                        bIsHelpObjSh : 1,
                        bForbidCaching : 1,
                        bForbidReload : 1,
                        bSupportsEventMacros: 1,
                        bLoadingWindows: 1,
                        bBasicInitialized :1,
                        //bHidden :1, // indicates a hidden view shell
                        bIsPrintJobCancelable :1, // Stampit disable/enable cancel button for print jobs ... default = true = enable!
                        bOwnsStorage:1,
                        bNoBaseURL:1,
                        bInitialized:1,
                        bSignatureErrorIsShown:1,
                        bModelInitialized:1, // whether the related model is initialized
                        bPreserveVersions:1,
                        m_bMacroSignBroken:1, // whether the macro signature was explicitly broken
                        m_bNoBasicCapabilities:1,
                        bQueryLoadTemplate:1,
                        bLoadReadonly:1,
                        bUseUserData:1,
                        bSaveVersionOnClose:1,
                        m_bSharedXMLFlag:1, // whether the flag should be stored in xml file
                        m_bAllowShareControlFileClean:1; // whether the flag should be stored in xml file

    String              aNewName;  // Der Name, unter dem das Doc gespeichert
                                   // werden soll
    IndexBitSet         aBitSet;
    sal_uInt32               lErr;
    sal_uInt16              nEventId;           // falls vor Activate noch ein
                                            // Open/Create gesendet werden mu/s
    sal_Bool                bDoNotTouchDocInfo;

    AutoReloadTimer_Impl *pReloadTimer;
    MarkData_Impl*      pMarkData;
    sal_uInt16              nLoadedFlags;
    sal_uInt16              nFlagsInProgress;
    String              aMark;
    Size                aViewSize;          // wird leider vom Writer beim
    sal_Bool                bInFrame;           // HTML-Import gebraucht
    sal_Bool                bModalMode;
    sal_Bool                bRunningMacro;
    sal_Bool                bReloadAvailable;
    sal_uInt16              nAutoLoadLocks;
    SfxModule*              pModule;
    SfxFrame*               pFrame;
    SfxToolBoxConfig*       pTbxConfig;
    SfxObjectShellFlags     eFlags;
    svtools::AsynchronLink* pCloser;
    String                  aBaseURL;
    sal_Bool                bReadOnlyUI;
    SvRefBaseRef            xHeaderAttributes;
    sal_Bool                bHiddenLockedByAPI;
    sal_Bool                bInCloseEvent;
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > xModel;
    sal_uInt16              nStyleFilter;
    sal_Bool                bDisposing;

    sal_Bool                m_bEnableSetModified;
    sal_Bool                m_bIsModified;

    Rectangle               m_aVisArea;
    MapUnit                 m_nMapUnit;

    sal_Bool                m_bCreateTempStor;
    ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage > m_xDocStorage;

    ::com::sun::star::uno::Reference<
        ::com::sun::star::util::XModifyListener > m_xDocInfoListener;

    sal_Bool                m_bIsInit;

    ::rtl::OUString         m_aSharedFileURL;

    ::com::sun::star::uno::Reference< ::com::sun::star::logging::XSimpleLogRing > m_xLogRing;

    sal_Bool                m_bIncomplEncrWarnShown;


    SfxObjectShell_Impl( SfxObjectShell& _rDocShell );
    virtual ~SfxObjectShell_Impl();

    static sal_Bool NeedsOfficeUpdateDialog();

    // IMacroDocumentAccess overridables
    virtual sal_Int16 getCurrentMacroExecMode() const;
    virtual sal_Bool setCurrentMacroExecMode( sal_uInt16 nMacroMode );
    virtual ::rtl::OUString getDocumentLocation() const;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage > getZipStorageToSign();
    virtual sal_Bool documentStorageHasMacros() const;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::document::XEmbeddedScripts > getEmbeddedDocumentScripts() const;
    virtual sal_Int16 getScriptingSignatureState();

    virtual sal_Bool hasTrustedScriptingSignature( sal_Bool bAllowUIToAddAuthor );
    virtual void showBrokenSignatureWarning( const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& _rxInteraction ) const;
};

#endif


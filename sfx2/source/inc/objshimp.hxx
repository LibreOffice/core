/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#ifndef _SFX_OBJSHIMP_HXX
#define _SFX_OBJSHIMP_HXX

//#include <hash_map>

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
    String aMark;
    String aUserData;
    SfxViewFrame* pFrame;
};

class SfxFrame;
class SfxToolBoxConfig;
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
    sal_Bool            bInList:1,          // ob per First/Next erreichbar
                        bClosing:1,         // sal_True w"aehrend Close(), um Benachrichtigungs-Rekursionen zu verhindern
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
                        m_bAllowShareControlFileClean:1; // whether the flag should be stored in xml file

    IndexBitSet         aBitSet;
    sal_uInt32               lErr;
    sal_uInt16              nEventId;           // falls vor Activate noch ein
                                            // Open/Create gesendet werden mu/s
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
    sal_Bool                bHiddenLockedByAPI;
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

    ::rtl::OUString         m_aSharedFileURL;

    ::com::sun::star::uno::Reference< ::com::sun::star::logging::XSimpleLogRing > m_xLogRing;

    sal_Bool                m_bIncomplEncrWarnShown;

    // TODO/LATER: m_aModifyPasswordInfo should completely replace m_nModifyPasswordHash in future
    sal_uInt32              m_nModifyPasswordHash;
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > m_aModifyPasswordInfo;
    sal_Bool                m_bModifyPasswordEntered;

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


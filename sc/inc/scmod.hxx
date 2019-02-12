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

#ifndef INCLUDED_SC_INC_SCMOD_HXX
#define INCLUDED_SC_INC_SCMOD_HXX

#include "scdllapi.h"
#include <vcl/timer.hxx>
#include <vcl/idle.hxx>
#include <svl/lstner.hxx>
#include <sfx2/module.hxx>
#include "global.hxx"
#include "shellids.hxx"
#include <unotools/options.hxx>
#include <com/sun/star/uno/Reference.h>

#include <vector>
#include <map>
#include <memory>

class KeyEvent;
class EditView;
class SfxErrorHandler;
class SvtAccessibilityOptions;
class SvtCTLOptions;
class SvtUserOptions;

namespace svtools { class ColorConfig; }
namespace ooo { namespace vba { class XSinkCaller; } }
namespace com { namespace sun { namespace star { namespace uno { class Any; } } } }
namespace com { namespace sun { namespace star { namespace uno { template <typename > class Sequence; } } } }

class ScRange;
class ScDocument;
class ScViewCfg;
class ScDocCfg;
class ScAppCfg;
class ScDefaultsCfg;
class ScFormulaCfg;
class ScInputCfg;
class ScPrintCfg;
class ScViewOptions;
class ScDocOptions;
class ScAppOptions;
class ScDefaultsOptions;
class ScFormulaOptions;
class ScInputOptions;
class ScPrintOptions;
class ScInputHandler;
class ScTabViewShell;
class ScMessagePool;
class EditFieldInfo;
class ScNavipiCfg;
class ScAddInCfg;
class ScTransferObj;
class ScDrawTransferObj;
class ScSelectionTransferObj;
class ScFormEditData;
class ScMarkData;
struct ScDragData;

class ScModule: public SfxModule, public SfxListener, public utl::ConfigurationListener
{
    Timer               m_aIdleTimer;
    Idle                m_aSpellIdle;
    std::unique_ptr<ScDragData> m_pDragData;
    ScSelectionTransferObj* m_pSelTransfer;
    ScMessagePool*      m_pMessagePool;
    // there is no global InputHandler anymore, each View has its own
    ScInputHandler*     m_pRefInputHandler;
    std::unique_ptr<ScViewCfg>        m_pViewCfg;
    std::unique_ptr<ScDocCfg>         m_pDocCfg;
    std::unique_ptr<ScAppCfg>         m_pAppCfg;
    std::unique_ptr<ScDefaultsCfg>    m_pDefaultsCfg;
    std::unique_ptr<ScFormulaCfg>     m_pFormulaCfg;
    std::unique_ptr<ScInputCfg>       m_pInputCfg;
    std::unique_ptr<ScPrintCfg>       m_pPrintCfg;
    std::unique_ptr<ScNavipiCfg>      m_pNavipiCfg;
    std::unique_ptr<ScAddInCfg>       m_pAddInCfg;
    std::unique_ptr<svtools::ColorConfig>    m_pColorConfig;
    std::unique_ptr<SvtAccessibilityOptions> m_pAccessOptions;
    std::unique_ptr<SvtCTLOptions>           m_pCTLOptions;
    std::unique_ptr<SvtUserOptions>          m_pUserOptions;
    std::unique_ptr<SfxErrorHandler>  m_pErrorHdl;
    std::unique_ptr<ScFormEditData>   m_pFormEditData;
    sal_uInt16          m_nCurRefDlgId;
    bool                m_bIsWaterCan:1;
    bool                m_bIsInEditCommand:1;
    bool                m_bIsInExecuteDrop:1;
    bool                m_bIsInSharedDocLoading:1;
    bool                m_bIsInSharedDocSaving:1;

    std::map<sal_uInt16, std::vector<VclPtr<vcl::Window> > > m_mapRefWindow;

    css::uno::Reference< ooo::vba::XSinkCaller > mxAutomationApplicationEventsCaller;

public:
                    SFX_DECL_INTERFACE(SCID_APP)

private:
    /// SfxInterface initializer.
    static void InitInterface_Impl();

public:
                        ScModule( SfxObjectFactory* pFact );
    virtual            ~ScModule() override;

    virtual void        Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;
    virtual void        ConfigurationChanged( utl::ConfigurationBroadcaster*, ConfigurationHints ) override;
    void                DeleteCfg();

                        // moved by the application

    DECL_LINK( IdleHandler, Timer*, void ); // Timer instead of idle
    DECL_LINK( SpellTimerHdl, Timer*, void );
    DECL_LINK( CalcFieldValueHdl, EditFieldInfo*, void );

    void                Execute( SfxRequest& rReq );
    void                GetState( SfxItemSet& rSet );
    static void         HideDisabledSlots( SfxItemSet& rSet );

    void                AnythingChanged();

    //  Drag & Drop:
    const ScDragData&   GetDragData() const { return *m_pDragData;}
    void                SetDragObject( ScTransferObj* pCellObj, ScDrawTransferObj* pDrawObj );
    void                ResetDragObject();
    void                SetDragLink(
        const OUString& rDoc, const OUString& rTab, const OUString& rArea );
    void                SetDragJump(
        ScDocument* pLocalDoc, const OUString& rTarget, const OUString& rText );

    static ScDocument*  GetClipDoc();       // called from document - should be removed later

    //  X selection:
    ScSelectionTransferObj* GetSelectionTransfer() const    { return m_pSelTransfer; }
    void                SetSelectionTransfer( ScSelectionTransferObj* pNew );

    void                SetWaterCan( bool bNew )    { m_bIsWaterCan = bNew; }
    bool                GetIsWaterCan() const       { return m_bIsWaterCan; }

    void                SetInEditCommand( bool bNew )   { m_bIsInEditCommand = bNew; }

    void                SetInExecuteDrop( bool bNew )   { m_bIsInExecuteDrop = bNew; }
    bool                IsInExecuteDrop() const         { return m_bIsInExecuteDrop; }

    // Options:
    const ScViewOptions&    GetViewOptions  ();
    SC_DLLPUBLIC const ScDocOptions&        GetDocOptions   ();
    SC_DLLPUBLIC const ScAppOptions&        GetAppOptions   ();
    SC_DLLPUBLIC const ScDefaultsOptions&   GetDefaultsOptions ();
    SC_DLLPUBLIC const ScFormulaOptions&    GetFormulaOptions ();
    SC_DLLPUBLIC const ScInputOptions&      GetInputOptions ();
    SC_DLLPUBLIC const ScPrintOptions&      GetPrintOptions ();
    void                SetViewOptions  ( const ScViewOptions& rOpt );
    SC_DLLPUBLIC void   SetDocOptions   ( const ScDocOptions& rOpt );
    SC_DLLPUBLIC void   SetAppOptions   ( const ScAppOptions& rOpt );
    void                SetDefaultsOptions ( const ScDefaultsOptions& rOpt );
    SC_DLLPUBLIC void   SetFormulaOptions ( const ScFormulaOptions& rOpt );
    SC_DLLPUBLIC void   SetInputOptions ( const ScInputOptions& rOpt );
    void                SetPrintOptions ( const ScPrintOptions& rOpt );
    void                InsertEntryToLRUList(sal_uInt16 nFIndex);

    static void         GetSpellSettings( LanguageType& rDefLang, LanguageType& rCjkLang, LanguageType& rCtlLang,
                                          bool& rAutoSpell );
    static void         SetAutoSpellProperty( bool bSet );
    static bool         HasThesaurusLanguage( LanguageType nLang );

    LanguageType        GetOptDigitLanguage();      // from CTL options

    ScNavipiCfg&        GetNavipiCfg();
    ScAddInCfg&         GetAddInCfg();
    svtools::ColorConfig&   GetColorConfig();
    SvtAccessibilityOptions& GetAccessOptions();
    SvtCTLOptions&      GetCTLOptions();
    SC_DLLPUBLIC SvtUserOptions& GetUserOptions();

    void                ModifyOptions( const SfxItemSet& rOptSet );

    // InputHandler:
    bool                IsEditMode();   // not for SC_INPUT_TYPE
    bool                IsInputMode();  // also for SC_INPUT_TYPE
    void                SetInputMode( ScInputMode eMode, const OUString* pInitText = nullptr );
    bool                InputKeyEvent( const KeyEvent& rKEvt, bool bStartEdit = false );
    SC_DLLPUBLIC void   InputEnterHandler( ScEnterMode nBlockMode = ScEnterMode::NORMAL );
    void                InputCancelHandler();
    void                InputSelection( const EditView* pView );
    void                InputChanged( const EditView* pView );
    ScInputHandler*     GetInputHdl( ScTabViewShell* pViewSh = nullptr, bool bUseRef = true );

    void                SetRefInputHdl( ScInputHandler* pNew );
    ScInputHandler*     GetRefInputHdl() { return m_pRefInputHandler;}

    void                ViewShellGone(const ScTabViewShell* pViewSh);
    void                ViewShellChanged(bool bStopEditing);
    // communication with function-autopilot
    void                InputGetSelection( sal_Int32& rStart, sal_Int32& rEnd );
    void                InputSetSelection( sal_Int32 nStart, sal_Int32 nEnd );
    void                InputReplaceSelection( const OUString& rStr );
    void                InputTurnOffWinEngine();
    OUString            InputGetFormulaStr();
    void                ActivateInputWindow( const OUString* pStr = nullptr,
                                                bool bMatrix = false );

    void                InitFormEditData();
    void                ClearFormEditData();
    ScFormEditData*     GetFormEditData()       { return m_pFormEditData.get(); }

    // input of reference:
    SC_DLLPUBLIC void   SetRefDialog( sal_uInt16 nId, bool bVis, SfxViewFrame* pViewFrm = nullptr );
    bool                IsModalMode(SfxObjectShell* pDocSh = nullptr);
    bool                IsFormulaMode();
    bool                IsRefDialogOpen();
    bool                IsTableLocked();
    void                SetReference( const ScRange& rRef, ScDocument* pDoc,
                                        const ScMarkData* pMarkData = nullptr );
    void                AddRefEntry();
    void                EndReference();
    sal_uInt16          GetCurRefDlgId() const                  { return m_nCurRefDlgId; }

    // virtual methods for the options dialog
    virtual std::unique_ptr<SfxItemSet> CreateItemSet( sal_uInt16 nId ) override;
    virtual void         ApplyItemSet( sal_uInt16 nId, const SfxItemSet& rSet ) override;
    virtual VclPtr<SfxTabPage> CreateTabPage( sal_uInt16 nId, TabPageParent pParent, const SfxItemSet& rSet ) override;
    virtual std::unique_ptr<SfxStyleFamilies> CreateStyleFamilies() override;

    void                SetInSharedDocLoading( bool bNew )  { m_bIsInSharedDocLoading = bNew; }
    bool                IsInSharedDocLoading() const        { return m_bIsInSharedDocLoading; }
    void                SetInSharedDocSaving( bool bNew )   { m_bIsInSharedDocSaving = bNew; }
    bool                IsInSharedDocSaving() const         { return m_bIsInSharedDocSaving; }

    SC_DLLPUBLIC void   RegisterRefWindow( sal_uInt16 nSlotId, vcl::Window *pWnd );
    SC_DLLPUBLIC void   UnregisterRefWindow( sal_uInt16 nSlotId, vcl::Window *pWnd );
    SC_DLLPUBLIC vcl::Window * Find1RefWindow( sal_uInt16 nSlotId, vcl::Window *pWndAncestor );

    SC_DLLPUBLIC void RegisterAutomationApplicationEventsCaller(css::uno::Reference< ooo::vba::XSinkCaller > const& xCaller);
    SC_DLLPUBLIC void CallAutomationApplicationEventSinks(const OUString& Method, css::uno::Sequence< css::uno::Any >& Arguments);
};

#define SC_MOD() ( static_cast<ScModule*>(SfxApplication::GetModule(SfxToolsModule::Calc)) )

void global_InitAppOptions();

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

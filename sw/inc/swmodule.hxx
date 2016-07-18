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
#ifndef INCLUDED_SW_INC_SWMODULE_HXX
#define INCLUDED_SW_INC_SWMODULE_HXX
#include <tools/fldunit.hxx>
#include <svl/lstner.hxx>
#include <unotools/options.hxx>
#include <sfx2/module.hxx>

#include "swdllapi.h"
#include "shellid.hxx"
#include <fldupde.hxx>
#include <com/sun/star/linguistic2/XLinguServiceEventListener.hpp>
#include <com/sun/star/linguistic2/XLanguageGuessing.hpp>
#include <editeng/svxenum.hxx>

class Color;
class SfxItemSet;
class SfxRequest;
class SfxErrorHandler;
class SwDBConfig;
class SwModuleOptions;
class SwMasterUsrPref;
class SwViewOption;
class SwView;
class SwWrtShell;
class SwPrintOptions;
class SwChapterNumRules;
class SwStdFontConfig;
class SwNavigationConfig;
class SwTransferable;
class SwToolbarConfigItem;
class SwAttrPool;
namespace svtools{ class ColorConfig;}
class SvtAccessibilityOptions;
class SvtCTLOptions;
class SvtUserOptions;

struct SwDBData;

enum class SvViewOpt {
    DestView,
    DestText,
    DestWeb,
    DestViewOnly   //ViewOptions are set only at View, not at the appl.
};

namespace com{ namespace sun{ namespace star{ namespace scanner{
    class XScannerManager2;
}}}}

class SW_DLLPUBLIC SwModule: public SfxModule, public SfxListener, public utl::ConfigurationListener
{
    OUString            m_sActAuthor;

    // ConfigItems
    SwModuleOptions*    m_pModuleConfig;
    SwMasterUsrPref*    m_pUsrPref;
    SwMasterUsrPref*    m_pWebUsrPref;
    SwPrintOptions*     m_pPrintOptions;
    SwPrintOptions*     m_pWebPrintOptions;
    SwChapterNumRules*  m_pChapterNumRules;
    SwStdFontConfig*    m_pStdFontConfig;
    SwNavigationConfig* m_pNavigationConfig;
    SwToolbarConfigItem*m_pToolbarConfig;     //For stacked toolbars. Which one was visible?
    SwToolbarConfigItem*m_pWebToolbarConfig;
    SwDBConfig*         m_pDBConfig;
    svtools::ColorConfig*   m_pColorConfig;
    SvtAccessibilityOptions* m_pAccessibilityOptions;
    SvtCTLOptions*      m_pCTLOptions;
    SvtUserOptions*     m_pUserOptions;

    SfxErrorHandler*    m_pErrorHandler;

    SwAttrPool          *m_pAttrPool;

    // Current view is held here in order to avoid one's being forced
    // to work via GetActiveView.
    // View is valid until destroyed in Activate or exchanged.
    SwView*             m_pView;

    // List of all Redline-authors.
    std::vector<OUString>* m_pAuthorNames;

    // DictionaryList listener to trigger spellchecking or hyphenation
    css::uno::Reference< css::linguistic2::XLinguServiceEventListener > m_xLinguServiceEventListener;
    css::uno::Reference< css::scanner::XScannerManager2 >    m_xScannerManager;
    css::uno::Reference< css::linguistic2::XLanguageGuessing >  m_xLanguageGuesser;

    bool                m_bAuthorInitialised : 1;
    bool                m_bEmbeddedLoadSave : 1;

    // Catch hint for DocInfo.
    virtual void        Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

    virtual void        ConfigurationChanged( utl::ConfigurationBroadcaster*, sal_uInt32 ) override;

protected:
    // Envelopes, labels.
    void                InsertEnv(SfxRequest&);
    void                InsertLab(SfxRequest&, bool bLabel);

public:
    // public Data - used for internal Clipboard / Drag & Drop / XSelection
    SwTransferable  *m_pDragDrop, *m_pXSelection;

    SFX_DECL_INTERFACE(SW_INTERFACE_MODULE)

private:
    /// SfxInterface initializer.
    static void InitInterface_Impl();

public:
    // This Ctor only for SW-Dll.
    SwModule( SfxObjectFactory* pFact,
                SfxObjectFactory* pWebFact,
                    SfxObjectFactory* pGlobalFact );

    virtual ~SwModule();

    // Set view for internal use only. It is public only for technical reasons.
    inline  void        SetView(SwView* pVw) { m_pView = pVw; }
    inline  SwView*     GetView() { return m_pView; }

    // Handler for slots.
    void                StateOther(SfxItemSet &);

    void                ExecOther(SfxRequest &);    // Fields, formula...

    // Modify user settings.
    const SwMasterUsrPref *GetUsrPref(bool bWeb) const;
    const SwViewOption* GetViewOption(bool bWeb);
    void                ApplyUsrPref(const SwViewOption &, SwView*,
                                     SvViewOpt nDest = SvViewOpt::DestView );
    void ApplyUserMetric( FieldUnit eMetric, bool bWeb );
    void ApplyRulerMetric( FieldUnit eMetric, bool bHorizontal, bool bWeb );
    void ApplyFieldUpdateFlags(SwFieldUpdateFlags eFieldFlags);
    void ApplyLinkMode(sal_Int32 nNewLinkMode);

    // Default page mode for text grid.
    void ApplyDefaultPageMode(bool bIsSquaredPageMode);

    void ApplyUserCharUnit(bool bApplyChar, bool bWeb);  // apply_char_unit

    // Create ConfigItems.
    SwModuleOptions*    GetModuleConfig()       { return m_pModuleConfig;}
    SwPrintOptions*     GetPrtOptions(bool bWeb);
    SwChapterNumRules*  GetChapterNumRules();
    SwStdFontConfig*    GetStdFontConfig()      { return m_pStdFontConfig; }
    SwNavigationConfig* GetNavigationConfig();
    SwToolbarConfigItem*GetToolbarConfig()      { return m_pToolbarConfig;    }
    SwToolbarConfigItem*GetWebToolbarConfig()   { return m_pWebToolbarConfig; }
    SwDBConfig*         GetDBConfig();
    svtools::ColorConfig&   GetColorConfig();
    SvtAccessibilityOptions&    GetAccessibilityOptions();
    SvtCTLOptions&      GetCTLOptions();
    SvtUserOptions&     GetUserOptions();

    // Iterate over views.
    static SwView*      GetFirstView();
    static SwView*      GetNextView(SwView*);

    bool IsEmbeddedLoadSave() const         { return m_bEmbeddedLoadSave; }
    void SetEmbeddedLoadSave( bool bFlag )  { m_bEmbeddedLoadSave = bFlag; }

    static void ShowDBObj( SwView& rView, const SwDBData& rData);

    // Table modi.
    bool            IsInsTableFormatNum(bool bHTML) const;
    bool            IsInsTableChangeNumFormat(bool bHTML) const;
    bool            IsInsTableAlignNum(bool bHTML) const;

    // Redlining.
    sal_uInt16          GetRedlineAuthor();
    OUString            GetRedlineAuthor(sal_uInt16 nPos);
    sal_uInt16          InsertRedlineAuthor(const OUString& rAuthor);
    void                SetRedlineAuthor(const OUString& rAuthor); // for unit tests

    void                GetInsertAuthorAttr(sal_uInt16 nAuthor, SfxItemSet &rSet);
    void                GetDeletedAuthorAttr(sal_uInt16 nAuthor, SfxItemSet &rSet);
    void                GetFormatAuthorAttr(sal_uInt16 nAuthor, SfxItemSet &rSet);

    sal_uInt16              GetRedlineMarkPos();
    const Color&            GetRedlineMarkColor();

    SvxCompareMode      GetCompareMode() const;
    bool            IsUseRsid() const;
    bool            IsIgnorePieces() const;
    sal_uInt16          GetPieceLen() const;

    // Return defined DocStat - WordDelimiter.
    OUString            GetDocStatWordDelim() const;

    // Pass metric of ModuleConfig (for HTML-export).
    FieldUnit GetMetric( bool bWeb ) const;

    // Pass update-statuses.
    sal_uInt16 GetLinkUpdMode( bool bWeb ) const;
    SwFieldUpdateFlags GetFieldUpdateFlags( bool bWeb ) const;

    // Virtual methods for options dialog.
    virtual SfxItemSet*  CreateItemSet( sal_uInt16 nId ) override;
    virtual void         ApplyItemSet( sal_uInt16 nId, const SfxItemSet& rSet ) override;
    virtual VclPtr<SfxTabPage> CreateTabPage( sal_uInt16 nId, vcl::Window* pParent, const SfxItemSet& rSet ) override;

    // Pool is created here and set at SfxShell.
    void    InitAttrPool();
    // Delete pool before it is too late.
    void    RemoveAttrPool();

    // Invalidates online spell-wrong-lists if necessary.
    static void  CheckSpellChanges( bool bOnlineSpelling,
                    bool bIsSpellWrongAgain, bool bIsSpellAllAgain, bool bSmartTags );

    inline const css::uno::Reference< css::linguistic2::XLinguServiceEventListener >&
            GetLngSvcEvtListener();
    void    CreateLngSvcEvtListener();

    css::uno::Reference< css::scanner::XScannerManager2 > const &
            GetScannerManager();

    css::uno::Reference< css::linguistic2::XLanguageGuessing > const &
            GetLanguageGuesser();
};

inline const css::uno::Reference< css::linguistic2::XLinguServiceEventListener >&
        SwModule::GetLngSvcEvtListener()
{
    return m_xLinguServiceEventListener;
}

//    Access to SwModule, the View and the shell.

#define SW_MOD() ( static_cast<SwModule*>(SfxApplication::GetModule(SfxToolsModule::Writer)))

SW_DLLPUBLIC SwView*    GetActiveView();
SW_DLLPUBLIC SwWrtShell* GetActiveWrtShell();

namespace sw
{
SW_DLLPUBLIC Color* GetActiveRetoucheColor();
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

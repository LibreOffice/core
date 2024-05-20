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

#include <sal/config.h>

#include <cstddef>

#include <o3tl/deleter.hxx>
#include <tools/fldunit.hxx>
#include <svl/lstner.hxx>
#include <unotools/options.hxx>
#include <sfx2/module.hxx>
#include <sfx2/app.hxx>

#include "swdllapi.h"
#include "shellid.hxx"
#include "fldupde.hxx"

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
class SvtCTLOptions;
class SvtUserOptions;
enum class SwCompareMode;
struct SwDBData;
namespace tools { class JsonWriter; }

enum class SvViewOpt {
    DestView,
    DestText,
    DestWeb,
    DestViewOnly   //ViewOptions are set only at View, not at the appl.
};

namespace com::sun::star::scanner { class XScannerManager2; }
namespace com::sun::star::linguistic2 { class XLanguageGuessing; }
namespace com::sun::star::linguistic2 { class XLinguServiceEventListener; }
namespace ooo::vba { class XSinkCaller; }

class SAL_DLLPUBLIC_RTTI SwModule final : public SfxModule, public SfxListener, public utl::ConfigurationListener
{
    OUString            m_sActAuthor;

    // ConfigItems
    std::unique_ptr<SwModuleOptions, o3tl::default_delete<SwModuleOptions>> m_pModuleConfig;
    std::unique_ptr<SwMasterUsrPref, o3tl::default_delete<SwMasterUsrPref>> m_pUsrPref;
    std::unique_ptr<SwMasterUsrPref, o3tl::default_delete<SwMasterUsrPref>> m_pWebUsrPref;
    std::unique_ptr<SwPrintOptions>      m_pPrintOptions;
    std::unique_ptr<SwPrintOptions>      m_pWebPrintOptions;
    std::unique_ptr<SwChapterNumRules>   m_pChapterNumRules;
    std::unique_ptr<SwStdFontConfig>     m_pStdFontConfig;
    std::unique_ptr<SwNavigationConfig>  m_pNavigationConfig;
    std::unique_ptr<SwToolbarConfigItem> m_pToolbarConfig;     //For stacked toolbars. Which one was visible?
    std::unique_ptr<SwToolbarConfigItem> m_pWebToolbarConfig;
    std::unique_ptr<SwDBConfig>          m_pDBConfig;
    std::unique_ptr<svtools::ColorConfig>   m_pColorConfig;
    std::unique_ptr<SvtCTLOptions>       m_pCTLOptions;
    std::unique_ptr<SvtUserOptions>      m_pUserOptions;

    std::unique_ptr<SfxErrorHandler> m_pErrorHandler;

    rtl::Reference<SwAttrPool> m_pAttrPool;

    // Current view is held here in order to avoid one's being forced
    // to work via GetActiveView.
    // View is valid until destroyed in Activate or exchanged.
    SwView*             m_pView;

    // List of all Redline-authors.
    std::vector<OUString> m_pAuthorNames;

    // DictionaryList listener to trigger spellchecking or hyphenation
    css::uno::Reference< css::linguistic2::XLinguServiceEventListener > m_xLinguServiceEventListener;
    css::uno::Reference< css::scanner::XScannerManager2 >    m_xScannerManager;
    css::uno::Reference< css::linguistic2::XLanguageGuessing >  m_xLanguageGuesser;

    bool                m_bAuthorInitialised : 1;
    bool                m_bEmbeddedLoadSave : 1;

    // Catch hint for DocInfo.
    virtual void        Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

    virtual void        ConfigurationChanged( utl::ConfigurationBroadcaster*, ConfigurationHints ) override;

    // Envelopes, labels.
    void                InsertEnv(SfxRequest&);
    void                InsertLab(SfxRequest&, bool bLabel);

    css::uno::Reference< ooo::vba::XSinkCaller > mxAutomationApplicationEventsCaller;

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

    virtual ~SwModule() override;

    // Set view for internal use only. It is public only for technical reasons.
    void        SetView(SwView* pVw) { m_pView = pVw; }
    SwView*     GetView() { return m_pView; }

    // Handler for slots.
    void                StateOther(SfxItemSet &);

    SW_DLLPUBLIC void ExecOther(SfxRequest &);    // Fields, formula...

    // Modify user settings.
    SW_DLLPUBLIC const SwMasterUsrPref *GetUsrPref(bool bWeb) const;
    const SwViewOption* GetViewOption(bool bWeb);
    SW_DLLPUBLIC void ApplyUsrPref(const SwViewOption &, SwView*,
                                     SvViewOpt nDest = SvViewOpt::DestView );
    void ApplyUserMetric( FieldUnit eMetric, bool bWeb );
    void ApplyRulerMetric( FieldUnit eMetric, bool bHorizontal, bool bWeb );
    SW_DLLPUBLIC void ApplyFieldUpdateFlags(SwFieldUpdateFlags eFieldFlags);
    SW_DLLPUBLIC void ApplyLinkMode(sal_Int32 nNewLinkMode);

    // Default page mode for text grid.
    SW_DLLPUBLIC void ApplyDefaultPageMode(bool bIsSquaredPageMode);

    void ApplyUserCharUnit(bool bApplyChar, bool bWeb);  // apply_char_unit

    // Create ConfigItems.
    SwModuleOptions*    GetModuleConfig()       { return m_pModuleConfig.get();}
    SwPrintOptions*     GetPrtOptions(bool bWeb);
    SW_DLLPUBLIC SwChapterNumRules*  GetChapterNumRules();
    SwStdFontConfig*    GetStdFontConfig()      { return m_pStdFontConfig.get(); }
    SwNavigationConfig* GetNavigationConfig();
    SwToolbarConfigItem*GetToolbarConfig()      { return m_pToolbarConfig.get();    }
    SwToolbarConfigItem*GetWebToolbarConfig()   { return m_pWebToolbarConfig.get(); }
    SW_DLLPUBLIC SwDBConfig* GetDBConfig();
    SW_DLLPUBLIC svtools::ColorConfig& GetColorConfig();
    SvtUserOptions&     GetUserOptions();

    // Iterate over views.
    SAL_RET_MAYBENULL static SwView*      GetFirstView();
    SAL_RET_MAYBENULL static SwView*      GetNextView(SwView const *);

    bool IsEmbeddedLoadSave() const         { return m_bEmbeddedLoadSave; }
    void SetEmbeddedLoadSave( bool bFlag )  { m_bEmbeddedLoadSave = bFlag; }

    static void ShowDBObj( SwView const & rView, const SwDBData& rData);

    // Table modi.
    bool            IsInsTableFormatNum(bool bHTML) const;
    bool            IsInsTableChangeNumFormat(bool bHTML) const;
    bool            IsInsTableAlignNum(bool bHTML) const;
    bool            IsSplitVerticalByDefault(bool bHTML) const;
    void            SetSplitVerticalByDefault(bool bHTML, bool value);

    // Redlining.
    std::size_t         GetRedlineAuthor();
    SW_DLLPUBLIC OUString const & GetRedlineAuthor(std::size_t nPos);
    SW_DLLPUBLIC void ClearRedlineAuthors();
    /// See SwXTextDocument::getTrackedChangeAuthors().
    void                GetRedlineAuthorInfo(tools::JsonWriter& rJsonWriter);
    std::size_t         InsertRedlineAuthor(const OUString& rAuthor);
    void                SetRedlineAuthor(const OUString& rAuthor); // for unit tests

    void                GetInsertAuthorAttr(std::size_t nAuthor, SfxItemSet &rSet);
    void                GetDeletedAuthorAttr(std::size_t nAuthor, SfxItemSet &rSet);
    void                GetFormatAuthorAttr(std::size_t nAuthor, SfxItemSet &rSet);

    sal_uInt16              GetRedlineMarkPos() const;
    const Color&            GetRedlineMarkColor() const;

    SwCompareMode      GetCompareMode() const;
    bool            IsUseRsid() const;
    bool            IsIgnorePieces() const;
    sal_uInt16          GetPieceLen() const;

    // Return defined DocStat - WordDelimiter.
    OUString const & GetDocStatWordDelim() const;

    // Pass metric of ModuleConfig (for HTML-export).
    FieldUnit GetMetric( bool bWeb ) const;

    // Pass update-statuses.
    sal_uInt16 GetLinkUpdMode() const;
    SwFieldUpdateFlags GetFieldUpdateFlags() const;

    // Virtual methods for options dialog.
    virtual std::optional<SfxItemSet> CreateItemSet( sal_uInt16 nId ) override;
    virtual void         ApplyItemSet( sal_uInt16 nId, const SfxItemSet& rSet ) override;
    virtual std::unique_ptr<SfxTabPage> CreateTabPage( sal_uInt16 nId, weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rSet ) override;
    SW_DLLPUBLIC virtual std::optional<SfxStyleFamilies> CreateStyleFamilies() override;

    // Pool is created here and set at SfxShell.
    void    InitAttrPool();
    // Delete pool before it is too late.
    void    RemoveAttrPool();

    // Invalidates online spell-wrong-lists if necessary.
    static void  CheckSpellChanges( bool bOnlineSpelling,
                    bool bIsSpellWrongAgain, bool bIsSpellAllAgain, bool bSmartTags );

    css::uno::Reference< css::scanner::XScannerManager2 > const &
            GetScannerManager();

    css::uno::Reference< css::linguistic2::XLanguageGuessing > const &
            GetLanguageGuesser();

    SW_DLLPUBLIC void RegisterAutomationApplicationEventsCaller(css::uno::Reference< ooo::vba::XSinkCaller > const& xCaller);
    void CallAutomationApplicationEventSinks(const OUString& Method, css::uno::Sequence< css::uno::Any >& Arguments);
};

//    Access to SwModule, the View and the shell.

#define SW_MOD() ( static_cast<SwModule*>(SfxApplication::GetModule(SfxToolsModule::Writer)))

SAL_RET_MAYBENULL SW_DLLPUBLIC SwView*    GetActiveView();
SAL_RET_MAYBENULL SW_DLLPUBLIC SwWrtShell* GetActiveWrtShell();

namespace sw
{
SW_DLLPUBLIC Color* GetActiveRetoucheColor();
}

extern bool g_bNoInterrupt;

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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
#ifndef SWMODULE_HXX
#define SWMODULE_HXX
#include <tools/fldunit.hxx>
#include <svl/lstner.hxx>
#include <unotools/options.hxx>
#include <sfx2/module.hxx>

#include <tools/shl.hxx>
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
#define VIEWOPT_DEST_VIEW       0
#define VIEWOPT_DEST_TEXT       1
#define VIEWOPT_DEST_WEB        2
#define VIEWOPT_DEST_VIEW_ONLY  3 //ViewOptions are set only at ::com::sun::star::sdbcx::View, not at the appl.

namespace com{ namespace sun{ namespace star{ namespace scanner{
    class XScannerManager2;
}}}}

class SW_DLLPUBLIC SwModule: public SfxModule, public SfxListener, public utl::ConfigurationListener
{
    OUString            sActAuthor;

    // ConfigItems
    SwModuleOptions*    pModuleConfig;
    SwMasterUsrPref*    pUsrPref;
    SwMasterUsrPref*    pWebUsrPref;
    SwPrintOptions*     pPrtOpt;
    SwPrintOptions*     pWebPrtOpt;
    SwChapterNumRules*  pChapterNumRules;
    SwStdFontConfig*    pStdFontConfig;
    SwNavigationConfig* pNavigationConfig;
    SwToolbarConfigItem*pToolbarConfig;     //For stacked toolbars. Which one was visible?
    SwToolbarConfigItem*pWebToolbarConfig;
    SwDBConfig*         pDBConfig;
    svtools::ColorConfig*   pColorConfig;
    SvtAccessibilityOptions* pAccessibilityOptions;
    SvtCTLOptions*      pCTLOptions;
    SvtUserOptions*     pUserOptions;

    SfxErrorHandler*    pErrorHdl;

    SwAttrPool          *pAttrPool;

    // Current view is held here in order to avoid one's being forced
    // to work via GetActiveView.
    // View is valid until destroyed in Activate or exchanged.
    SwView*             pView;

    // List of all Redline-authors.
    std::vector<OUString>* pAuthorNames;

    // DictionaryList listener to trigger spellchecking or hyphenation
    ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XLinguServiceEventListener > xLngSvcEvtListener;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::scanner::XScannerManager2 >    m_xScannerManager;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XLanguageGuessing >  m_xLanguageGuesser;

    sal_Bool                bAuthorInitialised : 1;
    sal_Bool                bEmbeddedLoadSave : 1;

    // Catch hint for DocInfo.
    virtual void        Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

    virtual void        ConfigurationChanged( utl::ConfigurationBroadcaster*, sal_uInt32 );

protected:
    // Envelopes, labels.
    void                InsertEnv(SfxRequest&);
    void                InsertLab(SfxRequest&, sal_Bool bLabel);

public:
    // public Data - used for internal Clipboard / Drag & Drop / XSelection
    SwTransferable  *pDragDrop, *pXSelection;

    TYPEINFO();
    SFX_DECL_INTERFACE(SW_INTERFACE_MODULE)

    // This Ctor only for SW-Dll.
    SwModule( SfxObjectFactory* pFact,
                SfxObjectFactory* pWebFact,
                    SfxObjectFactory* pGlobalFact );

    ~SwModule();

    // Set view for internal use only. It is public only for technical reasons.
    inline  void        SetView(SwView* pVw) { pView = pVw; }
    inline  SwView*     GetView() { return pView; }

    // Handler for slots.
    void                StateOther(SfxItemSet &);

    void                ExecOther(SfxRequest &);    // Fields, formula...

    // Modify user settings.
    const SwMasterUsrPref *GetUsrPref(sal_Bool bWeb) const;
    const SwViewOption* GetViewOption(sal_Bool bWeb);
    void                ApplyUsrPref(const SwViewOption &, SwView*,
                                     sal_uInt16 nDest = VIEWOPT_DEST_VIEW );
    void ApplyUserMetric( FieldUnit eMetric, sal_Bool bWeb );
    void ApplyRulerMetric( FieldUnit eMetric, sal_Bool bHorizontal, sal_Bool bWeb );
    void ApplyFldUpdateFlags(SwFldUpdateFlags eFldFlags);
    void ApplyLinkMode(sal_Int32 nNewLinkMode);

    // Default page mode for text grid.
    void ApplyDefaultPageMode(sal_Bool bIsSquaredPageMode);

    void ApplyUserCharUnit(sal_Bool bApplyChar, sal_Bool bWeb);  // apply_char_unit

    // Create ConfigItems.
    SwModuleOptions*    GetModuleConfig()       { return pModuleConfig;}
    SwPrintOptions*     GetPrtOptions(sal_Bool bWeb);
    SwChapterNumRules*  GetChapterNumRules();
    SwStdFontConfig*    GetStdFontConfig()      { return pStdFontConfig; }
    SwNavigationConfig* GetNavigationConfig();
    SwToolbarConfigItem*GetToolbarConfig()      { return pToolbarConfig;    }
    SwToolbarConfigItem*GetWebToolbarConfig()   { return pWebToolbarConfig; }
    SwDBConfig*         GetDBConfig();
    svtools::ColorConfig&   GetColorConfig();
    SvtAccessibilityOptions&    GetAccessibilityOptions();
    SvtCTLOptions&      GetCTLOptions();
    SvtUserOptions&     GetUserOptions();

    // Iterate over views.
    static SwView*      GetFirstView();
    static SwView*      GetNextView(SwView*);

    sal_Bool IsEmbeddedLoadSave() const         { return bEmbeddedLoadSave; }
    void SetEmbeddedLoadSave( sal_Bool bFlag )  { bEmbeddedLoadSave = bFlag; }

    void ShowDBObj( SwView& rView, const SwDBData& rData, sal_Bool bOnlyIfAvailable = sal_False);

    // Table modi.
    sal_Bool            IsInsTblFormatNum(sal_Bool bHTML) const;
    sal_Bool            IsInsTblChangeNumFormat(sal_Bool bHTML) const;
    sal_Bool            IsInsTblAlignNum(sal_Bool bHTML) const;

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
    sal_Bool            IsUseRsid() const;
    sal_Bool            IsIgnorePieces() const;
    sal_uInt16          GetPieceLen() const;

    // Return defined DocStat - WordDelimiter.
    OUString            GetDocStatWordDelim() const;

    // Pass metric of ModuleConfig (for HTML-export).
    sal_uInt16 GetMetric( sal_Bool bWeb ) const;

    // Pass update-statuses.
    sal_uInt16 GetLinkUpdMode( sal_Bool bWeb ) const;
    SwFldUpdateFlags GetFldUpdateFlags( sal_Bool bWeb ) const;

    // Virtual methods for options dialog.
    virtual SfxItemSet*  CreateItemSet( sal_uInt16 nId );
    virtual void         ApplyItemSet( sal_uInt16 nId, const SfxItemSet& rSet );
    virtual SfxTabPage*  CreateTabPage( sal_uInt16 nId, Window* pParent, const SfxItemSet& rSet );

    // Pool is created here and set at SfxShell.
    void    InitAttrPool();
    // Delete pool before it is too late.
    void    RemoveAttrPool();

    // Invalidates online spell-wrong-lists if necessary.
    void    CheckSpellChanges( bool bOnlineSpelling,
                    bool bIsSpellWrongAgain, bool bIsSpellAllAgain, bool bSmartTags );

    inline ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XLinguServiceEventListener >
            GetLngSvcEvtListener();
    inline void SetLngSvcEvtListener( ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XLinguServiceEventListener > & xLstnr);
    void    CreateLngSvcEvtListener();

    ::com::sun::star::uno::Reference<
        ::com::sun::star::scanner::XScannerManager2 >
            GetScannerManager();

    ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XLanguageGuessing >
            GetLanguageGuesser();
};


inline ::com::sun::star::uno::Reference<
    ::com::sun::star::linguistic2::XLinguServiceEventListener >
        SwModule::GetLngSvcEvtListener()
{
    return xLngSvcEvtListener;
}

inline void SwModule::SetLngSvcEvtListener(
    ::com::sun::star::uno::Reference<
       ::com::sun::star::linguistic2::XLinguServiceEventListener > & xLstnr)
{
    xLngSvcEvtListener = xLstnr;
}


//    Access to SwModule, the ::com::sun::star::sdbcx::View and the shell.

#define SW_MOD() ( *(SwModule**) GetAppData(SHL_WRITER))

SW_DLLPUBLIC SwView*    GetActiveView();
SW_DLLPUBLIC SwWrtShell* GetActiveWrtShell();

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

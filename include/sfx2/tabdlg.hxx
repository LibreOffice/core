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
#ifndef INCLUDED_SFX2_TABDLG_HXX
#define INCLUDED_SFX2_TABDLG_HXX

#include <memory>
#include <string_view>

#include <sal/config.h>
#include <sfx2/dllapi.h>
#include <sfx2/basedlgs.hxx>
#include <sal/types.h>
#include <vcl/bitmapex.hxx>
#include <vcl/builderpage.hxx>
#include <svl/itempool.hxx>
#include <svl/itemset.hxx>
#include <o3tl/typed_flags_set.hxx>

class SfxTabPage;

typedef std::unique_ptr<SfxTabPage> (*CreateTabPage)(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet *rAttrSet);
typedef const sal_uInt16*     (*GetTabPageRanges)(); // provides international Which-value
struct TabPageImpl;

struct TabDlg_Impl;

namespace com::sun::star::frame { class XFrame; }

#define RET_USER        100
#define RET_USER_CANCEL 101

class SFX2_DLLPUBLIC SfxTabDialogItem final : public SfxSetItem
{
public:
                            SfxTabDialogItem( sal_uInt16 nId, const SfxItemSet& rItemSet );
                            SfxTabDialogItem(const SfxTabDialogItem& rAttr, SfxItemPool* pItemPool);
    virtual SfxTabDialogItem* Clone(SfxItemPool* pToPool = nullptr) const override;
};

class SFX2_DLLPUBLIC SfxTabDialogController : public SfxOkDialogController
{
protected:
    std::unique_ptr<weld::Notebook> m_xTabCtrl;

    DECL_LINK(OkHdl, weld::Button&, void);
    DECL_LINK(ResetHdl, weld::Button&, void);
    DECL_LINK(BaseFmtHdl, weld::Button&, void);
    DECL_LINK(UserHdl, weld::Button&, void);
    DECL_LINK(CancelHdl, weld::Button&, void);
private:
    std::unique_ptr<weld::Button> m_xOKBtn;
    std::unique_ptr<weld::Button> m_xApplyBtn;
    std::unique_ptr<weld::Button> m_xUserBtn;
    std::unique_ptr<weld::Button> m_xCancelBtn;
    std::unique_ptr<weld::Button> m_xResetBtn;
    std::unique_ptr<weld::Button> m_xBaseFmtBtn;
    std::unique_ptr<weld::SizeGroup> m_xSizeGroup;

    std::unique_ptr<SfxItemSet>           m_pSet;
    std::unique_ptr<SfxItemSet>           m_pOutSet;
    std::unique_ptr<TabDlg_Impl>          m_pImpl;
    std::unique_ptr<sal_uInt16[]>         m_pRanges;
    OString             m_sAppPageId;
    bool                m_bStandardPushed;

    DECL_DLLPRIVATE_LINK(ActivatePageHdl, const OString&, void);
    DECL_DLLPRIVATE_LINK(DeactivatePageHdl, const OString&, bool);
    SAL_DLLPRIVATE void Start_Impl();
    SAL_DLLPRIVATE void CreatePages();
    SAL_DLLPRIVATE void setPreviewsToSamePlace();

protected:
    virtual short               Ok();
    virtual void                RefreshInputSet();
    virtual SfxItemSet*         CreateInputItemSet(const OString& rName);
    virtual void                PageCreated(const OString &rName, SfxTabPage &rPage);

    std::unique_ptr<SfxItemSet> m_xExampleSet;
    SfxItemSet*     GetInputSetImpl();
    SfxTabPage*     GetTabPage(std::string_view rPageId) const;

    /** prepare to leave the current page. Calls the DeactivatePage method of the current page, (if necessary),
        handles the item sets to copy.
        @return sal_True if it is allowed to leave the current page, sal_False otherwise
    */
    bool PrepareLeaveCurrentPage();

    /** save the position of the TabDialog and which tab page is the currently active one
     */
    void SavePosAndId();
public:
    SfxTabDialogController(weld::Widget* pParent, const OUString& rUIXMLDescription, const OString& rID,
                           const SfxItemSet * = nullptr, bool bEditFmt = false);
    virtual ~SfxTabDialogController() override;

    void                AddTabPage(const OString& rName,           // Name of the label for the existing page in the notebook .ui
                                   CreateTabPage pCreateFunc,      // != 0
                                   GetTabPageRanges pRangesFunc);  // can be 0

    void                AddTabPage(const OString& rName,           // Name of the label for the existing page in the notebook .ui
                                   sal_uInt16 nPageCreateId);      // Identifier of the Factory Method to create the page

    void                AddTabPage(const OString& rName,           // Name of the label for the new page to create
                                   const OUString& rLabel,         // UI Label for the new page to create
                                   CreateTabPage pCreateFunc);     // != 0

    void                AddTabPage(const OString& rName,           // Name of the label for the new page to create
                                   const OUString& rLabel,         // UI Label for the new page to create
                                   sal_uInt16 nPageCreateId);      // Identifier of the Factory Method to create the page

    void                RemoveTabPage( const OString& rName ); // Name of the label for the page in the notebook .ui

    void                SetCurPageId(const OString& rName);
    void                ShowPage(const OString& rName);  // SetCurPageId + call Activate on it
    OString             GetCurPageId() const;
    SfxTabPage*         GetCurTabPage() const { return GetTabPage(GetCurPageId()); }

    // may provide local slots converted by Map
    const sal_uInt16*   GetInputRanges( const SfxItemPool& );
    void                SetInputSet( const SfxItemSet* pInSet );
    const SfxItemSet*   GetOutputItemSet() const { return m_pOutSet.get(); }

    virtual weld::Button& GetOKButton() const override { return *m_xOKBtn; }
    weld::Button&       GetCancelButton() const { return *m_xCancelBtn; }
    weld::Button*       GetUserButton() const { return m_xUserBtn.get(); }
    weld::Button*       GetStandardButton() const { return m_xBaseFmtBtn.get(); }
    weld::Button*       GetApplyButton() const { return m_xApplyBtn.get(); }
    weld::Button*       GeResetButton() const { return m_xResetBtn.get(); }
    void                RemoveResetButton();
    void                RemoveStandardButton();

    virtual short       run() override;
    static bool runAsync(const std::shared_ptr<SfxTabDialogController>& rController,
                         const std::function<void(sal_Int32)>&);

    virtual const SfxItemSet* GetExampleSet() const override { return m_xExampleSet.get(); }

    void                SetApplyHandler(const Link<weld::Button&,void>& _rHdl);

    //calls Ok without closing dialog
    bool Apply();
    void Applied() { m_xExampleSet->Put(*GetInputSetImpl()); }

    //screenshotting
    std::vector<OString> getAllPageUIXMLDescriptions() const;
    bool selectPageByUIXMLDescription(const OString& rUIXMLDescription);
    BitmapEx createScreenshot() const;
    OString GetScreenshotId() const;
};

enum class DeactivateRC {
    KeepPage   = 0x00,      // Error handling; page does not change
    // 2. Fill an itemset for update
    // parent examples, this pointer can be NULL all the time!
    LeavePage  = 0x01,
    // Set, refresh and update other Page
    RefreshSet = 0x02
};
namespace o3tl {
    template<> struct typed_flags<DeactivateRC> : is_typed_flags<DeactivateRC, 0x03> {};
}

class SFX2_DLLPUBLIC SfxTabPage : public BuilderPage
{
friend class SfxTabDialog;
friend class SfxTabDialogController;

private:
    const SfxItemSet*   pSet;
    OUString            aUserString;
    bool                bHasExchangeSupport;
    std::unique_ptr< TabPageImpl >        pImpl;

protected:
    SfxTabPage(weld::Container* pPage, weld::DialogController* pController, const OUString& rUIXMLDescription, const OString& rID, const SfxItemSet *rAttrSet);

    sal_uInt16          GetWhich( sal_uInt16 nSlot, bool bDeep = true ) const
                            { return pSet->GetPool()->GetWhich( nSlot, bDeep ); }
    const SfxPoolItem*  GetOldItem( const SfxItemSet& rSet, sal_uInt16 nSlot, bool bDeep = true );
    template<class T> const T* GetOldItem( const SfxItemSet& rSet, TypedWhichId<T> nSlot, bool bDeep = true )
    {
        return static_cast<const T*>(GetOldItem(rSet, sal_uInt16(nSlot), bDeep));
    }

    SfxOkDialogController* GetDialogController() const;
public:
    void                SetDialogController(SfxOkDialogController* pDialog);
public:
    virtual             ~SfxTabPage() override;

    void set_visible(bool bVisible)
    {
        m_xContainer->set_visible(bVisible);
    }

    const SfxItemSet&   GetItemSet() const { return *pSet; }

    virtual bool        FillItemSet( SfxItemSet* );
    virtual void        Reset( const SfxItemSet* );
    // Allows to postpone some initialization to the first activation
    virtual bool        DeferResetToFirstActivation();

    bool                HasExchangeSupport() const
                            { return bHasExchangeSupport; }
    void                SetExchangeSupport()
                            { bHasExchangeSupport = true; }

    virtual void            ActivatePage( const SfxItemSet& );
    virtual DeactivateRC    DeactivatePage( SfxItemSet* pSet );
    void                    SetUserData(const OUString& rString)
                              { aUserString = rString; }
    const OUString&         GetUserData() const { return aUserString; }
    virtual void            FillUserData();
    virtual bool            IsReadOnly() const;
    virtual void PageCreated (const SfxAllItemSet& aSet);
    virtual void ChangesApplied();
    static const SfxPoolItem* GetItem( const SfxItemSet& rSet, sal_uInt16 nSlot, bool bDeep = true );
    template<class T> static const T* GetItem( const SfxItemSet& rSet, TypedWhichId<T> nSlot, bool bDeep = true )
    {
        return static_cast<const T*>(GetItem(rSet, sal_uInt16(nSlot), bDeep));
    }

    void SetFrame(const css::uno::Reference< css::frame::XFrame >& xFrame);
    css::uno::Reference< css::frame::XFrame > GetFrame() const;

    const SfxItemSet* GetDialogExampleSet() const;

    OString         GetHelpId() const;
    OString         GetConfigId() const { return GetHelpId(); }
    bool            IsVisible() const { return m_xContainer->get_visible(); }

    weld::Window*   GetFrameWeld() const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

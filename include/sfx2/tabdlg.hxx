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
#include <unordered_map>
#include <string_view>

#include <sal/config.h>
#include <sfx2/dllapi.h>
#include <sfx2/basedlgs.hxx>
#include <sal/types.h>
#include <vcl/bitmapex.hxx>
#include <vcl/builderpage.hxx>
#include <svl/itempool.hxx>
#include <svl/itemset.hxx>
#include <svl/setitem.hxx>
#include <o3tl/typed_flags_set.hxx>

class SfxTabPage;

typedef std::unique_ptr<SfxTabPage> (*CreateTabPage)(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet *rAttrSet);
typedef const WhichRangesContainer & (*GetTabPageRanges)(); // provides international Which-value
struct TabPageImpl;

struct TabDlg_Impl;

namespace com::sun::star::frame { class XFrame; }

#define RET_USER        100

class SFX2_DLLPUBLIC SfxTabDialogItem final : public SfxSetItem
{
public:
                            DECLARE_ITEM_TYPE_FUNCTION(SfxTabDialogItem)
                            SfxTabDialogItem( sal_uInt16 nId, const SfxItemSet& rItemSet );
                            SfxTabDialogItem(const SfxTabDialogItem& rAttr, SfxItemPool* pItemPool);
    virtual SfxTabDialogItem* Clone(SfxItemPool* pToPool = nullptr) const override;
};

class SFX2_DLLPUBLIC SfxTabDialogController : public SfxOkDialogController
{
protected:
    std::unique_ptr<weld::Notebook> m_xTabCtrl;

    DECL_LINK(OkHdl, weld::Button&, void);
    DECL_DLLPRIVATE_LINK(ResetHdl, weld::Button&, void);
    DECL_DLLPRIVATE_LINK(BaseFmtHdl, weld::Button&, void);
    DECL_DLLPRIVATE_LINK(UserHdl, weld::Button&, void);
    DECL_DLLPRIVATE_LINK(CancelHdl, weld::Button&, void);
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
    WhichRangesContainer                  m_pRanges;
    OUString             m_sAppPageId;
    bool                m_bStandardPushed;
    std::unique_ptr<SfxAllItemSet>       m_xItemSet;

    DECL_DLLPRIVATE_LINK(ActivatePageHdl, const OUString&, void);
    DECL_DLLPRIVATE_LINK(DeactivatePageHdl, const OUString&, bool);
    SAL_DLLPRIVATE void Start_Impl();
    SAL_DLLPRIVATE void CreatePages();
    SAL_DLLPRIVATE void setPreviewsToSamePlace();

protected:
    virtual short               Ok();
    virtual void                RefreshInputSet();
    virtual SfxItemSet*         CreateInputItemSet(const OUString& rName);
    virtual void                PageCreated(const OUString &rName, SfxTabPage &rPage);

    virtual void ActivatePage(const OUString& rPage);
    bool DeactivatePage(std::u16string_view aPage);

    std::unique_ptr<SfxItemSet> m_xExampleSet;
    SfxItemSet*     GetInputSetImpl();
    SfxTabPage*     GetTabPage(std::u16string_view rPageId) const;

    /** prepare to leave the current page. Calls the DeactivatePage method of the current page, (if necessary),
        handles the item sets to copy.
        @return sal_True if it is allowed to leave the current page, sal_False otherwise
    */
    bool PrepareLeaveCurrentPage();

    /** Called before user cancels the dialog.
        Calls DeactivatePage of all tab pages with IsCancelMode() set to true
    */
    void PrepareCancel();

    /** save the position of the TabDialog and which tab page is the currently active one
     */
    void SavePosAndId();
public:
    SfxTabDialogController(weld::Widget* pParent, const OUString& rUIXMLDescription, const OUString& rID,
                           const SfxItemSet * = nullptr, bool bEditFmt = false);
    virtual ~SfxTabDialogController() override;

    void                AddTabPage(const OUString& rName,          // Name of the label for the existing page in the notebook .ui
                                   CreateTabPage pCreateFunc,      // != 0
                                   GetTabPageRanges pRangesFunc);  // can be 0

    void                AddTabPage(const OUString& rName,          // Name of the label for the existing page in the notebook .ui
                                   sal_uInt16 nPageCreateId);      // Identifier of the Factory Method to create the page

    void                AddTabPage(const OUString& rName,          // Name of the label for the new page to create
                                   const OUString& rLabel,         // UI Label for the new page to create
                                   CreateTabPage pCreateFunc);     // != 0

    void                AddTabPage(const OUString& rName,          // Name of the label for the new page to create
                                   const OUString& rLabel,         // UI Label for the new page to create
                                   sal_uInt16 nPageCreateId);      // Identifier of the Factory Method to create the page

    void                RemoveTabPage( const OUString& rName ); // Name of the label for the page in the notebook .ui

    void                SetCurPageId(const OUString& rName);
    void                ShowPage(const OUString& rName);  // SetCurPageId + call Activate on it
    OUString             GetCurPageId() const;
    SfxTabPage*         GetCurTabPage() const { return GetTabPage(GetCurPageId()); }

    // may provide local slots converted by Map
    const WhichRangesContainer& GetInputRanges( const SfxItemPool& );
    void                SetInputSet( const SfxItemSet* pInSet );
    const SfxItemSet*   GetOutputItemSet() const { return m_pOutSet.get(); }
    const SfxItemSet*   GetInputItemSet() const { return m_pSet.get(); }

    virtual weld::Button& GetOKButton() const override { return *m_xOKBtn; }
    weld::Button&       GetCancelButton() const { return *m_xCancelBtn; }
    weld::Button*       GetUserButton() const { return m_xUserBtn.get(); }
    weld::Button*       GetStandardButton() const { return m_xBaseFmtBtn.get(); }
    weld::Button*       GetApplyButton() const { return m_xApplyBtn.get(); }
    weld::Button*       GetResetButton() const { return m_xResetBtn.get(); }
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
    std::vector<OUString> getAllPageUIXMLDescriptions() const;
    bool selectPageByUIXMLDescription(const OUString& rUIXMLDescription);
    BitmapEx createScreenshot() const;
    OUString GetScreenshotId() const;
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
    const SfxItemSet* mpSet;
    OUString maUserString;
    bool mbHasExchangeSupport;
    bool mbCancel;
    std::unordered_map<OUString, css::uno::Any> maAdditionalProperties;

    std::unique_ptr<TabPageImpl> mpImpl;

protected:
    SfxTabPage(weld::Container* pPage, weld::DialogController* pController, const OUString& rUIXMLDescription, const OUString& rID, const SfxItemSet *rAttrSet);

    sal_uInt16          GetWhich( sal_uInt16 nSlot, bool bDeep = true ) const
    {
        return mpSet->GetPool()->GetWhichIDFromSlotID(nSlot, bDeep);
    }
    template<class T>
    TypedWhichId<T> GetWhich( TypedWhichId<T> nSlot, bool bDeep = true ) const
    {
        return TypedWhichId<T>(GetWhich(sal_uInt16(nSlot), bDeep));
    }

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

    const SfxItemSet& GetItemSet() const
    {
        return *mpSet;
    }

    virtual bool        FillItemSet( SfxItemSet* );
    virtual void        Reset( const SfxItemSet* );
    // Allows to postpone some initialization to the first activation
    virtual bool        DeferResetToFirstActivation();

    bool HasExchangeSupport() const
    {
        return mbHasExchangeSupport;
    }

    void SetExchangeSupport()
    {
        mbHasExchangeSupport = true;
    }

    virtual void            ActivatePage( const SfxItemSet& );
    virtual DeactivateRC    DeactivatePage( SfxItemSet* pSet );
    void SetUserData(const OUString& rString)
    {
        maUserString = rString;
    }
    const OUString& GetUserData() const
    {
        return maUserString;
    }
    virtual void            FillUserData();
    virtual bool            IsReadOnly() const;
    // Whether the user has canceled the dialog. Allows to restore settings, etc.
    bool IsCancelMode() { return mbCancel; }
    void SetCancelMode(bool bCancel) { mbCancel = bCancel; }
    virtual void PageCreated (const SfxAllItemSet& aSet);
    virtual void ChangesApplied();
    static const SfxPoolItem* GetItem( const SfxItemSet& rSet, sal_uInt16 nSlot, bool bDeep = true );
    template<class T> static const T* GetItem( const SfxItemSet& rSet, TypedWhichId<T> nSlot, bool bDeep = true )
    {
        return static_cast<const T*>(GetItem(rSet, sal_uInt16(nSlot), bDeep));
    }

    virtual OUString GetAllStrings();
    void SetFrame(const css::uno::Reference< css::frame::XFrame >& xFrame);
    css::uno::Reference< css::frame::XFrame > GetFrame() const;

    const SfxItemSet* GetDialogExampleSet() const;

    OUString        GetHelpId() const;
    OUString        GetConfigId() const { return GetHelpId(); }
    bool            IsVisible() const { return m_xContainer->get_visible(); }

    weld::Window*   GetFrameWeld() const;

    std::unordered_map<OUString, css::uno::Any>& getAdditionalProperties()
    {
        return maAdditionalProperties;
    }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

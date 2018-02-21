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
#include <sal/config.h>
#include <sfx2/dllapi.h>
#include <sal/types.h>
#include <vcl/button.hxx>
#include <vcl/layout.hxx>
#include <vcl/tabctrl.hxx>
#include <vcl/tabdlg.hxx>
#include <vcl/tabpage.hxx>
#include <svl/itempool.hxx>
#include <svl/itemset.hxx>
#include <com/sun/star/frame/XFrame.hpp>
#include <o3tl/typed_flags_set.hxx>

class SfxPoolItem;
class SfxTabDialog;
class SfxViewFrame;
class SfxTabPage;
class SfxBindings;

typedef VclPtr<SfxTabPage> (*CreateTabPage)(vcl::Window *pParent, const SfxItemSet *rAttrSet);
typedef const sal_uInt16*     (*GetTabPageRanges)(); // provides international Which-value
struct TabPageImpl;

struct TabDlg_Impl;

#define RET_USER        100
#define RET_USER_CANCEL 101

class SFX2_DLLPUBLIC SfxTabDialogItem: public SfxSetItem
{
public:
                            SfxTabDialogItem( sal_uInt16 nId, const SfxItemSet& rItemSet );
                            SfxTabDialogItem(const SfxTabDialogItem& rAttr, SfxItemPool* pItemPool);
    virtual SfxPoolItem*    Clone(SfxItemPool* pToPool = nullptr) const override;
    virtual SfxPoolItem*    Create(SvStream& rStream, sal_uInt16 nVersion) const override;
};

class SFX2_DLLPUBLIC SfxTabDialog : public TabDialog
{
private:
friend class SfxTabPage;
friend class SfxTabDialogController;
friend class SfxTabDialogUIObject;

    VclPtr<VclBox>     m_pBox;
    VclPtr<TabControl> m_pTabCtrl;

    VclPtr<PushButton> m_pOKBtn;
    VclPtr<PushButton> m_pApplyBtn;
    VclPtr<PushButton> m_pUserBtn;
    VclPtr<CancelButton> m_pCancelBtn;
    VclPtr<HelpButton> m_pHelpBtn;
    VclPtr<PushButton> m_pResetBtn;
    VclPtr<PushButton> m_pBaseFmtBtn;

    bool m_bOwnsOKBtn;
    bool m_bOwnsCancelBtn;
    bool m_bOwnsHelpBtn;
    bool m_bOwnsResetBtn;
    bool m_bOwnsBaseFmtBtn;

    SfxItemSet*         m_pSet;
    SfxItemSet*         m_pOutSet;
    std::unique_ptr< TabDlg_Impl >        m_pImpl;
    sal_uInt16*         m_pRanges;
    sal_uInt16          m_nAppPageId;
    bool                m_bStandardPushed;

    DECL_DLLPRIVATE_LINK(ActivatePageHdl, TabControl*, void );
    DECL_DLLPRIVATE_LINK(DeactivatePageHdl, TabControl*, bool );
    DECL_DLLPRIVATE_LINK(OkHdl, Button*, void);
    DECL_DLLPRIVATE_LINK(ResetHdl, Button*, void);
    DECL_DLLPRIVATE_LINK(BaseFmtHdl, Button*, void);
    DECL_DLLPRIVATE_LINK(UserHdl, Button*, void);
    DECL_DLLPRIVATE_LINK(CancelHdl, Button*, void);
    SAL_DLLPRIVATE void Init_Impl(bool bFmtFlag);

protected:
    virtual short               Ok();
    // Is deleted in Sfx!
    virtual SfxItemSet*         CreateInputItemSet( sal_uInt16 nId );
    // Is not deleted in Sfx!
    virtual void                RefreshInputSet();
    virtual void                PageCreated( sal_uInt16 nId, SfxTabPage &rPage );

    VclPtr<VclButtonBox>   m_pActionArea;
    SfxItemSet*     m_pExampleSet;
    SfxItemSet*     GetInputSetImpl();
    SfxTabPage*     GetTabPage( sal_uInt16 nPageId ) const;

    /** prepare to leave the current page. Calls the DeactivatePage method of the current page, (if necessary),
        handles the item sets to copy.
        @return sal_True if it is allowed to leave the current page, sal_False otherwise
    */
    bool PrepareLeaveCurrentPage();

    /** save the position of the TabDialog and which tab page is the currently active one
     */
    void SavePosAndId();

public:
    SfxTabDialog(vcl::Window* pParent,
                 const OUString& rID, const OUString& rUIXMLDescription,
                 const SfxItemSet * = nullptr, bool bEditFmt = false);
    virtual ~SfxTabDialog() override;
    virtual void dispose() override;

    sal_uInt16          AddTabPage( const OString& rName,           // Name of the label for the page in the notebook .ui
                                    CreateTabPage pCreateFunc,      // != 0
                                    GetTabPageRanges pRangesFunc);  // can be 0

    sal_uInt16          AddTabPage ( const OString &rName,          // Name of the label for the page in the notebook .ui
                                     sal_uInt16 nPageCreateId );    // Identifier of the Factory Method to create the page

    void                AddTabPage( sal_uInt16 nId,
                                    const OUString &rRiderText,
                                    CreateTabPage pCreateFunc,      // != 0
                                    GetTabPageRanges pRangesFunc,   // can be 0
                                    sal_uInt16 nPos = TAB_APPEND);

    void                AddTabPage( sal_uInt16 nId,
                                    const OUString &rRiderText);

    void                RemoveTabPage( const OString& rName ); // Name of the label for the page in the notebook .ui
    void                RemoveTabPage( sal_uInt16 nId );

    void                SetCurPageId(sal_uInt16 nId)
    {
        m_nAppPageId = nId;
    }
    void                SetCurPageId(const OString& rName)
    {
        m_nAppPageId = m_pTabCtrl->GetPageId(rName);
    }
    sal_uInt16          GetCurPageId() const
    {
        return m_pTabCtrl->GetCurPageId();
    }

    SfxTabPage* GetCurTabPage() const
    {
        return GetTabPage(m_pTabCtrl->GetCurPageId());
    }

    virtual OString GetScreenshotId() const override;

    OUString            GetPageText( sal_uInt16 nPageId ) const
    {
        return m_pTabCtrl->GetPageText(nPageId);
    }

    void                ShowPage( sal_uInt16 nId );

    // may provide local slots converted by Map
    const sal_uInt16*       GetInputRanges( const SfxItemPool& );
    void                SetInputSet( const SfxItemSet* pInSet );
    const SfxItemSet*   GetOutputItemSet() const { return m_pOutSet; }

    const PushButton&   GetOKButton() const { return *m_pOKBtn; }
    PushButton&         GetOKButton() { return *m_pOKBtn; }
    const CancelButton& GetCancelButton() const { return *m_pCancelBtn; }
    CancelButton&       GetCancelButton() { return *m_pCancelBtn; }

    const PushButton*   GetUserButton() const { return m_pUserBtn; }
    PushButton*         GetUserButton() { return m_pUserBtn; }
    void                RemoveResetButton();
    void                RemoveStandardButton();

    short               Execute() override;
    void                StartExecuteModal( const Link<Dialog&,void>& rEndDialogHdl ) override;

    using               Dialog::StartExecuteAsync;
    bool                StartExecuteAsync( VclAbstractDialog::AsyncContext &rCtx ) override;
    void                Start();

    const SfxItemSet*   GetExampleSet() const { return m_pExampleSet; }
    SfxItemSet*         GetExampleSet() { return m_pExampleSet; }

    void                SetApplyHandler(const Link<Button*,void>& _rHdl);

    SAL_DLLPRIVATE void Start_Impl();

    //calls Ok without closing dialog
    bool Apply();

    virtual FactoryFunction GetUITestFactory() const override;
    // Screenshot interface
    virtual std::vector<OString> getAllPageUIXMLDescriptions() const override;
    virtual bool selectPageByUIXMLDescription(const OString& rUIXMLDescription) override;
};

namespace sfx { class ItemConnectionBase; }

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

class SFX2_DLLPUBLIC SfxTabPage: public TabPage
{
friend class SfxTabDialog;

private:
    const SfxItemSet*   pSet;
    OUString            aUserString;
    bool                bHasExchangeSupport;
    std::unique_ptr< TabPageImpl >        pImpl;

protected:
    SfxTabPage(vcl::Window *pParent, const OString& rID, const OUString& rUIXMLDescription, const SfxItemSet *rAttrSet);

    sal_uInt16          GetWhich( sal_uInt16 nSlot, bool bDeep = true ) const
                            { return pSet->GetPool()->GetWhich( nSlot, bDeep ); }
    const SfxPoolItem*  GetOldItem( const SfxItemSet& rSet, sal_uInt16 nSlot, bool bDeep = true );
    SfxTabDialog*       GetTabDialog() const;

    void                AddItemConnection( sfx::ItemConnectionBase* pConnection );

public:
    virtual             ~SfxTabPage() override;
    virtual void        dispose() override;

    const SfxItemSet&   GetItemSet() const { return *pSet; }

    virtual bool        FillItemSet( SfxItemSet* );
    virtual void        Reset( const SfxItemSet* );

    bool                HasExchangeSupport() const
                            { return bHasExchangeSupport; }
    void                SetExchangeSupport()
                            { bHasExchangeSupport = true; }

        using TabPage::ActivatePage;
        using TabPage::DeactivatePage;
    virtual void            ActivatePage( const SfxItemSet& );
    virtual DeactivateRC    DeactivatePage( SfxItemSet* pSet );
    void                    SetUserData(const OUString& rString)
                              { aUserString = rString; }
    const OUString&         GetUserData() { return aUserString; }
    virtual void            FillUserData();
    virtual bool            IsReadOnly() const;
    virtual void PageCreated (const SfxAllItemSet& aSet);
    virtual void ChangesApplied();
    static const SfxPoolItem* GetItem( const SfxItemSet& rSet, sal_uInt16 nSlot, bool bDeep = true );

    void SetFrame(const css::uno::Reference< css::frame::XFrame >& xFrame);
    css::uno::Reference< css::frame::XFrame > GetFrame();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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
#ifndef INCLUDED_SW_SOURCE_UI_DBUI_MMADDRESSBLOCKPAGE_HXX
#define INCLUDED_SW_SOURCE_UI_DBUI_MMADDRESSBLOCKPAGE_HXX

#include <editeng/weldeditview.hxx>
#include <svtools/wizardmachine.hxx>
#include <vcl/button.hxx>
#include <mailmergehelper.hxx>
#include <sfx2/basedlgs.hxx>
#include <vcl/edit.hxx>
#include <vcl/layout.hxx>
#include <svl/lstner.hxx>
#include <o3tl/typed_flags_set.hxx>

class SwMailMergeWizard;
class SwMailMergeConfigItem;

class SwMailMergeAddressBlockPage : public svt::OWizardPage
{
    VclPtr<PushButton>         m_pAddressListPB;
    VclPtr<FixedText>          m_pCurrentAddressFI;

    VclPtr<VclContainer>       m_pStep2;
    VclPtr<VclContainer>       m_pStep3;
    VclPtr<VclContainer>       m_pStep4;

    VclPtr<FixedText>          m_pSettingsFI;
    VclPtr<CheckBox>           m_pAddressCB;
    VclPtr<SwAddressPreview>   m_pSettingsWIN;
    VclPtr<PushButton>         m_pSettingsPB;
    VclPtr<CheckBox>           m_pHideEmptyParagraphsCB;

    VclPtr<PushButton>         m_pAssignPB;

    VclPtr<SwAddressPreview>   m_pPreviewWIN;
    VclPtr<FixedText>          m_pDocumentIndexFI;
    VclPtr<PushButton>         m_pPrevSetIB;
    VclPtr<PushButton>         m_pNextSetIB;

    OUString            m_sDocument;
    OUString            m_sCurrentAddress;
    OUString            m_sChangeAddress;

    VclPtr<SwMailMergeWizard>  m_pWizard;

    DECL_LINK(AddressListHdl_Impl, Button *, void);
    DECL_LINK(SettingsHdl_Impl, Button*, void);
    DECL_LINK(AssignHdl_Impl, Button*, void);
    DECL_LINK(AddressBlockHdl_Impl, Button*, void);
    DECL_LINK(InsertDataHdl_Impl, Button*, void);
    DECL_LINK(AddressBlockSelectHdl_Impl, LinkParamNone*, void);
    DECL_LINK(HideParagraphsHdl_Impl, Button*, void);

    void                EnableAddressBlock(bool bAll, bool bSelective);

    virtual void        ActivatePage() override;
    virtual bool        commitPage( ::svt::WizardTypes::CommitPageReason _eReason ) override;
    virtual bool        canAdvance() const override;

public:
    SwMailMergeAddressBlockPage(SwMailMergeWizard* _pParent);
    virtual ~SwMailMergeAddressBlockPage() override;
    virtual void dispose() override;
    SwMailMergeWizard* GetWizard() { return m_pWizard; }
};

class SwSelectAddressBlockDialog : public SfxDialogController
{
    css::uno::Sequence< OUString>    m_aAddressBlocks;
    SwMailMergeConfigItem& m_rConfig;

    std::unique_ptr<AddressPreview> m_xPreview;
    std::unique_ptr<weld::Button> m_xNewPB;
    std::unique_ptr<weld::Button> m_xCustomizePB;
    std::unique_ptr<weld::Button> m_xDeletePB;
    std::unique_ptr<weld::RadioButton> m_xNeverRB;
    std::unique_ptr<weld::RadioButton> m_xAlwaysRB;
    std::unique_ptr<weld::RadioButton> m_xDependentRB;
    std::unique_ptr<weld::Entry> m_xCountryED;
    std::unique_ptr<weld::CustomWeld> m_xPreviewWin;

    DECL_LINK(NewCustomizeHdl_Impl, weld::Button&, void);
    DECL_LINK(DeleteHdl_Impl, weld::Button&, void);
    DECL_LINK(IncludeHdl_Impl, weld::ToggleButton&, void);

public:
    SwSelectAddressBlockDialog(weld::Window* pParent, SwMailMergeConfigItem& rConfig);
    virtual ~SwSelectAddressBlockDialog() override;

    void         SetAddressBlocks(const css::uno::Sequence< OUString>& rBlocks,
                                sal_uInt16 nSelected);
    const css::uno::Sequence< OUString>&    GetAddressBlocks();

    void         SetSettings(bool bIsCountry, const OUString& sCountry);
    bool         IsIncludeCountry() const {return !m_xNeverRB->get_active();}
    OUString     GetCountry() const;
};

class SwCustomizeAddressBlockDialog;

enum class MoveItemFlags {
    NONE           = 0,
    Left           = 1,
    Right          = 2,
    Up             = 4,
    Down           = 8,
};
namespace o3tl {
    template<> struct typed_flags<MoveItemFlags> : is_typed_flags<MoveItemFlags, 0x0f> {};
}

class AddressMultiLineEdit;

class AddressMultiLineEdit : public WeldEditView
                           , public SfxListener
{
    Link<bool,void> m_aSelectionLink;
    Link<AddressMultiLineEdit&,void> m_aModifyLink;
    SwCustomizeAddressBlockDialog*  m_pParentDialog;

    css::uno::Reference<css::datatransfer::dnd::XDropTarget> m_xDropTarget;

    virtual void EditViewSelectionChange() const override;
    virtual css::uno::Reference<css::datatransfer::dnd::XDropTarget> GetDropTarget() const override;

    virtual bool KeyInput(const KeyEvent& rKEvt) override;
    virtual bool MouseButtonDown(const MouseEvent& rMEvt) override;

public:
    AddressMultiLineEdit(SwCustomizeAddressBlockDialog *pParent);
    virtual void SetDrawingArea(weld::DrawingArea* pDrawingArea) override;
    void EndDropTarget();
    bool SetCursorLogicPosition(const Point& rPosition);
    void UpdateFields();
    virtual ~AddressMultiLineEdit() override;

    void            SetSelectionChangedHdl( const Link<bool,void>& rLink ) { m_aSelectionLink = rLink; }
    void            SetModifyHdl( const Link<AddressMultiLineEdit&,void>& rLink ) { m_aModifyLink = rLink; }

    void            SetText( const OUString& rStr );
    OUString        GetText() const;
    OUString        GetAddress();

    void            InsertNewEntry( const OUString& rStr );
    void            InsertNewEntryAtPosition( const OUString& rStr, sal_uLong nPara, sal_uInt16 nIndex );
    void            RemoveCurrentEntry();

    void            MoveCurrentItem(MoveItemFlags nMove);
    MoveItemFlags   IsCurrentItemMoveable();
    bool            HasCurrentItem();
    OUString        GetCurrentItem();
    void            SelectCurrentItem();
};

class SwCustomizeAddressBlockDialog : public SfxDialogController
{
    friend class AddressMultiLineEdit;
public:
    enum DialogType
    {
        ADDRESSBLOCK_NEW,
        ADDRESSBLOCK_EDIT,
        GREETING_FEMALE,
        GREETING_MALE
    };
private:
    TextFilter              m_aTextFilter;

    std::vector<OUString>     m_aSalutations;
    std::vector<OUString>     m_aPunctuations;

    OUString                m_sCurrentSalutation;
    OUString                m_sCurrentPunctuation;
    OUString                m_sCurrentText;

    SwMailMergeConfigItem&  m_rConfigItem;
    DialogType const        m_eType;

    Idle m_aSelectionChangedIdle;

    std::unique_ptr<weld::Label> m_xAddressElementsFT;
    std::unique_ptr<weld::TreeView> m_xAddressElementsLB;
    std::unique_ptr<weld::Button> m_xInsertFieldIB;
    std::unique_ptr<weld::Button> m_xRemoveFieldIB;
    std::unique_ptr<weld::Label> m_xDragFT;
    std::unique_ptr<weld::Button> m_xUpIB;
    std::unique_ptr<weld::Button> m_xLeftIB;
    std::unique_ptr<weld::Button> m_xRightIB;
    std::unique_ptr<weld::Button> m_xDownIB;
    std::unique_ptr<weld::Label> m_xFieldFT;
    std::unique_ptr<weld::ComboBox> m_xFieldCB;
    std::unique_ptr<weld::Button> m_xOK;
    std::unique_ptr<AddressPreview> m_xPreview;
    std::unique_ptr<weld::CustomWeld> m_xPreviewWIN;
    std::unique_ptr<AddressMultiLineEdit> m_xDragED;
    std::unique_ptr<weld::CustomWeld> m_xDragWIN;

    DECL_LINK(OKHdl_Impl, weld::Button&, void);
    DECL_LINK(ListBoxSelectHdl_Impl, weld::TreeView&, void);
    DECL_LINK(EditModifyHdl_Impl, AddressMultiLineEdit&, void);
    DECL_LINK(ImageButtonHdl_Impl, weld::Button&, void);
    DECL_LINK(SelectionChangedHdl_Impl, bool, void);
    DECL_LINK(FieldChangeComboBoxHdl_Impl, weld::ComboBox&, void);
    DECL_LINK(TextFilterHdl, OUString&, bool);
    DECL_LINK(SelectionChangedIdleHdl, Timer*, void);

    sal_Int32       GetSelectedItem_Impl();
    void            UpdateImageButtons_Impl();

public:
    SwCustomizeAddressBlockDialog(weld::Widget* pParent, SwMailMergeConfigItem& rConfig, DialogType);
    virtual ~SwCustomizeAddressBlockDialog() override;

    bool SetCursorLogicPosition(const Point& rPosition);
    void UpdateFields();

    // for dragging from the TreeViews, return the active source
    virtual weld::TreeView* get_drag_source() const { return m_xAddressElementsLB->get_drag_source(); }
    bool            HasItem(sal_Int32 nUserData);

    void SetAddress(const OUString& rAddress);
    OUString GetAddress();
};

class SwAssignFieldsControl;
class SwAssignFieldsDialog : public SfxDialogController
{
    OUString const          m_sNone;
    OUString const          m_rPreviewString;

    SwMailMergeConfigItem&  m_rConfigItem;

    std::unique_ptr<AddressPreview> m_xPreview;
    std::unique_ptr<weld::Label> m_xMatchingFI;
    std::unique_ptr<weld::Label> m_xAddressTitle;
    std::unique_ptr<weld::Label> m_xMatchTitle;
    std::unique_ptr<weld::Label> m_xPreviewTitle;
    std::unique_ptr<weld::Label> m_xPreviewFI;
    std::unique_ptr<weld::Button> m_xOK;
    std::unique_ptr<weld::CustomWeld> m_xPreviewWin;
    std::unique_ptr<SwAssignFieldsControl> m_xFieldsControl;

    std::unique_ptr<weld::SizeGroup> m_xLabelGroup;
    std::unique_ptr<weld::SizeGroup> m_xComboGroup;
    std::unique_ptr<weld::SizeGroup> m_xPreviewGroup;

    css::uno::Sequence< OUString > CreateAssignments();
    DECL_LINK(OkHdl_Impl, weld::Button&, void);
    DECL_LINK(AssignmentModifyHdl_Impl, LinkParamNone*, void);

public:
    SwAssignFieldsDialog(weld::Window* pParent,
                SwMailMergeConfigItem& rConfigItem,
                const OUString& rPreview,
                bool bIsAddressBlock);

    void ConnectSizeGroups(int nLabelWidth, int nComboBoxWidth, int nPreviewWidth);

    virtual ~SwAssignFieldsDialog() override;
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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

#include <svtools/wizardmachine.hxx>
#include <vcl/button.hxx>
#include <mailmergehelper.hxx>
#include <sfx2/basedlgs.hxx>
#include <vcl/edit.hxx>
#include <vcl/layout.hxx>
#include <vcl/lstbox.hxx>
#include <svtools/svmedit.hxx>
#include <svtools/headbar.hxx>
#include <vcl/treelistbox.hxx>
#include <vcl/combobox.hxx>
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
class DDListBox : public SvTreeListBox
{
    VclPtr<SwCustomizeAddressBlockDialog>   m_pParentDialog;
public:
    DDListBox(vcl::Window* pParent, const WinBits nStyle);
    virtual ~DDListBox() override;
    virtual void dispose() override;

    void SetAddressDialog(SwCustomizeAddressBlockDialog *pParent);

    virtual void        StartDrag( sal_Int8 nAction, const Point& rPosPixel ) override;
};

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

class AddressMultiLineEdit : public VclMultiLineEdit, public SfxListener
{
    Link<AddressMultiLineEdit&,void>       m_aSelectionLink;
    VclPtr<SwCustomizeAddressBlockDialog>  m_pParentDialog;

    using VclMultiLineEdit::SetText;

protected:
    bool            PreNotify( NotifyEvent& rNEvt ) override;
public:
    AddressMultiLineEdit(vcl::Window* pParent, WinBits nWinStyle);
    virtual ~AddressMultiLineEdit() override;
    virtual void    dispose() override;

    void            SetAddressDialog(SwCustomizeAddressBlockDialog *pParent);

    virtual void    Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

    virtual Size    GetOptimalSize() const override;

    void            SetSelectionChangedHdl( const Link<AddressMultiLineEdit&,void>& rLink ) {m_aSelectionLink = rLink;}

    void            SetText( const OUString& rStr ) override;
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

class SwCustomizeAddressBlockDialog : public SfxModalDialog
{
    friend class DDListBox;
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
    VclPtr<FixedText>              m_pAddressElementsFT;
    VclPtr<DDListBox>              m_pAddressElementsLB;

    VclPtr<PushButton>             m_pInsertFieldIB;
    VclPtr<PushButton>             m_pRemoveFieldIB;

    VclPtr<FixedText>              m_pDragFT;
    VclPtr<AddressMultiLineEdit>   m_pDragED;
    VclPtr<PushButton>             m_pUpIB;
    VclPtr<PushButton>             m_pLeftIB;
    VclPtr<PushButton>             m_pRightIB;
    VclPtr<PushButton>             m_pDownIB;

    VclPtr<FixedText>              m_pFieldFT;
    VclPtr<ComboBox>               m_pFieldCB;
    TextFilter              m_aTextFilter;

    VclPtr<SwAddressPreview>       m_pPreviewWIN;

    VclPtr<OKButton>               m_pOK;

    std::vector<OUString>     m_aSalutations;
    std::vector<OUString>     m_aPunctuations;

    OUString                m_sCurrentSalutation;
    OUString                m_sCurrentPunctuation;
    OUString                m_sCurrentText;

    SwMailMergeConfigItem&  m_rConfigItem;
    DialogType const        m_eType;

    DECL_LINK(OKHdl_Impl, Button*, void);
    DECL_LINK(ListBoxSelectHdl_Impl, SvTreeListBox*, void);
    DECL_LINK(EditModifyHdl_Impl, Edit&, void);
    DECL_LINK(ImageButtonHdl_Impl, Button*, void);
    DECL_LINK(SelectionChangedHdl_Impl, AddressMultiLineEdit&, void);
    DECL_LINK(FieldChangeHdl_Impl, Edit&, void);
    DECL_LINK(FieldChangeComboBoxHdl_Impl, ComboBox&, void);

    bool            HasItem_Impl(sal_Int32 nUserData);
    sal_Int32       GetSelectedItem_Impl();
    void            UpdateImageButtons_Impl();

public:
    SwCustomizeAddressBlockDialog(vcl::Window* pParent, SwMailMergeConfigItem& rConfig, DialogType);
    virtual ~SwCustomizeAddressBlockDialog() override;
    virtual void dispose() override;

    void            SetAddress(const OUString& rAddress);
    OUString GetAddress();
};

class SwAssignFieldsControl;
class SwAssignFieldsDialog : public SfxModalDialog
{
    VclPtr<FixedText>               m_pMatchingFI;
    VclPtr<SwAssignFieldsControl>   m_pFieldsControl;

    VclPtr<FixedText>               m_pPreviewFI;
    VclPtr<SwAddressPreview>        m_pPreviewWIN;

    VclPtr<OKButton>                m_pOK;

    OUString const          m_sNone;
    OUString const          m_rPreviewString;

    SwMailMergeConfigItem&  m_rConfigItem;

    css::uno::Sequence< OUString > CreateAssignments();
    DECL_LINK(OkHdl_Impl, Button*, void);
    DECL_LINK(AssignmentModifyHdl_Impl, LinkParamNone*, void);

public:
    SwAssignFieldsDialog(vcl::Window* pParent,
                SwMailMergeConfigItem& rConfigItem,
                const OUString& rPreview,
                bool bIsAddressBlock);
    virtual ~SwAssignFieldsDialog() override;
    virtual void dispose() override;
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

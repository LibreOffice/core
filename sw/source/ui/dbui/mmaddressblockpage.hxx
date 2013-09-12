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
#ifndef _MAILMERGEADDRESSBLOCKPAGE_HXX
#define _MAILMERGEADDRESSBLOCKPAGE_HXX

#include <svtools/wizardmachine.hxx>
#include <vcl/button.hxx>
#include <svtools/stdctrl.hxx>
#include <mailmergehelper.hxx>
#include <sfx2/basedlgs.hxx>
#include <vcl/edit.hxx>
#include <vcl/lstbox.hxx>
#include <svtools/svmedit.hxx>
#include <svtools/headbar.hxx>
#include <svtools/treelistbox.hxx>
#include <vcl/combobox.hxx>
#include <svl/lstner.hxx>
class SwMailMergeWizard;
class SwMailMergeConfigItem;

class SwMailMergeAddressBlockPage : public svt::OWizardPage
{
    SwBoldFixedInfo     m_aHeaderFI;
    FixedInfo           m_aFirstFI;
    FixedInfo           m_aAddressListFI;
    PushButton          m_aAddressListPB;
    FixedInfo           m_aCurrentAddressFI;

    FixedLine           m_aFirstFL;

    FixedInfo           m_aSecondFI;
    FixedInfo           m_aSettingsFI;
    CheckBox            m_aAddressCB;
    SwAddressPreview    m_aSettingsWIN;
    PushButton          m_aSettingsPB;

    CheckBox            m_aHideEmptyParagraphsCB;

    FixedLine           m_aSecondFL;

    FixedInfo           m_aThirdFI;
    FixedInfo           m_aMatchFieldsFI;

    PushButton          m_aAssignPB;

    FixedLine           m_aThirdFL;

    FixedInfo           m_aFourthFI;
    FixedInfo           m_aPreviewFI;

    SwAddressPreview    m_aPreviewWIN;
    FixedInfo           m_aDocumentIndexFI;
    ImageButton         m_aPrevSetIB;
    ImageButton         m_aNextSetIB;

    String              m_sDocument;
    String              m_sCurrentAddress;
    String              m_sChangeAddress;

    SwMailMergeWizard*  m_pWizard;

    DECL_LINK(AddressListHdl_Impl, void *);
    DECL_LINK(SettingsHdl_Impl, PushButton*);
    DECL_LINK(AssignHdl_Impl, PushButton*);
    DECL_LINK(AddressBlockHdl_Impl, CheckBox*);
    DECL_LINK(InsertDataHdl_Impl, ImageButton*);
    DECL_LINK(AddressBlockSelectHdl_Impl, void *);
    DECL_LINK(HideParagraphsHdl_Impl, CheckBox*);

    void                EnableAddressBlock(sal_Bool bAll, sal_Bool bSelective);

    virtual void        ActivatePage();
    virtual sal_Bool    commitPage( ::svt::WizardTypes::CommitPageReason _eReason );
    virtual bool        canAdvance() const;

public:
    SwMailMergeAddressBlockPage( SwMailMergeWizard* _pParent);
    ~SwMailMergeAddressBlockPage();

    SwMailMergeWizard*  GetWizard() { return m_pWizard;}
};

class SwSelectAddressBlockDialog : public SfxModalDialog
{
    FixedText           m_aSelectFT;
    SwAddressPreview    m_aPreview;
    PushButton          m_aNewPB;
    PushButton          m_aCustomizePB;
    PushButton          m_aDeletePB;

    FixedInfo           m_aSettingsFI;
    RadioButton         m_aNeverRB;
    RadioButton         m_aAlwaysRB;
    RadioButton         m_aDependentRB;
    Edit                m_aCountryED;

    FixedLine           m_aSeparatorFL;

    OKButton            m_aOK;
    CancelButton        m_aCancel;
    HelpButton          m_aHelp;

    com::sun::star::uno::Sequence< OUString>    m_aAddressBlocks;
    SwMailMergeConfigItem& m_rConfig;

    DECL_LINK(NewCustomizeHdl_Impl, PushButton*);
    DECL_LINK(DeleteHdl_Impl, PushButton*);
    DECL_LINK(IncludeHdl_Impl, RadioButton*);

    using Window::SetSettings;

public:
    SwSelectAddressBlockDialog(Window* pParent, SwMailMergeConfigItem& rConfig);
    ~SwSelectAddressBlockDialog();

    void    SetAddressBlocks(const com::sun::star::uno::Sequence< OUString>& rBlocks,
                                sal_uInt16 nSelected);
    const com::sun::star::uno::Sequence< OUString>&    GetAddressBlocks();

    void                SetSettings(sal_Bool bIsCountry, OUString sCountry);
    sal_Bool            IsIncludeCountry() const {return !m_aNeverRB.IsChecked();}
    OUString     GetCountry() const;
};

class SwCustomizeAddressBlockDialog;
class DDListBox : public SvTreeListBox
{
    SwCustomizeAddressBlockDialog*   m_pParentDialog;
public:
    DDListBox(Window* pParent, const WinBits nStyle);

    void SetAddressDialog(SwCustomizeAddressBlockDialog *pParent);

    virtual void        StartDrag( sal_Int8 nAction, const Point& rPosPixel );
};

#define MOVE_ITEM_LEFT           1
#define MOVE_ITEM_RIGHT          2
#define MOVE_ITEM_UP             4
#define MOVE_ITEM_DOWN           8

class AddressMultiLineEdit : public VclMultiLineEdit, public SfxListener
{
    Link                            m_aSelectionLink;
    SwCustomizeAddressBlockDialog*  m_pParentDialog;

    using VclMultiLineEdit::Notify;

    using VclMultiLineEdit::SetText;

protected:
    long            PreNotify( NotifyEvent& rNEvt );
public:
    AddressMultiLineEdit(Window* pParent, WinBits nWinStyle = WB_LEFT | WB_BORDER);
    ~AddressMultiLineEdit();

    void            SetAddressDialog(SwCustomizeAddressBlockDialog *pParent);

    virtual void    Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

    virtual Size    GetOptimalSize() const;

    void            SetSelectionChangedHdl( const Link& rLink ) {m_aSelectionLink = rLink;}

    void            SetText( const OUString& rStr );
    String          GetAddress();

    void            InsertNewEntry( const String& rStr );
    void            InsertNewEntryAtPosition( const String& rStr, sal_uLong nPara, sal_uInt16 nIndex );
    void            RemoveCurrentEntry();

    void            MoveCurrentItem(sal_uInt16 nMove);
    sal_uInt16      IsCurrentItemMoveable();
    bool            HasCurrentItem();
    String          GetCurrentItem();
    void            SelectCurrentItem();
};

// Dialog is used to create custom address blocks as well as custom greeting lines
class SwRestrictedComboBox : public ComboBox
{
    String sForbiddenChars;

protected:
    virtual void KeyInput( const KeyEvent& );
    virtual void        Modify();
public:
    SwRestrictedComboBox(Window* pParent, WinBits nStyle = 0)
        : ComboBox( pParent, nStyle )
    {
    }

    void SetForbiddenChars(const String& rSet){sForbiddenChars = rSet;}

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
    FixedText*              m_pAddressElementsFT;
    DDListBox*              m_pAddressElementsLB;

    PushButton*             m_pInsertFieldIB;
    PushButton*             m_pRemoveFieldIB;

    FixedText*              m_pDragFT;
    AddressMultiLineEdit*   m_pDragED;
    PushButton*             m_pUpIB;
    PushButton*             m_pLeftIB;
    PushButton*             m_pRightIB;
    PushButton*             m_pDownIB;

    FixedText*              m_pFieldFT;
    SwRestrictedComboBox*   m_pFieldCB;

    SwAddressPreview*       m_pPreviewWIN;

    OKButton*               m_pOK;

    ::std::vector<String>   m_aSalutations;
    ::std::vector<String>   m_aPunctuations;

    String                  m_sCurrentSalutation;
    String                  m_sCurrentPunctuation;
    String                  m_sCurrentText;

    SwMailMergeConfigItem&  m_rConfigItem;
    DialogType              m_eType;

    DECL_LINK(OKHdl_Impl, void *);
    DECL_LINK(ListBoxSelectHdl_Impl, DDListBox*);
    DECL_LINK(EditModifyHdl_Impl, void *);
    DECL_LINK(ImageButtonHdl_Impl, ImageButton*);
    DECL_LINK(SelectionChangedHdl_Impl, AddressMultiLineEdit*);
    DECL_LINK(FieldChangeHdl_Impl, void *);

    bool            HasItem_Impl(sal_Int32 nUserData);
    sal_Int32       GetSelectedItem_Impl();
    void            UpdateImageButtons_Impl();

public:
    SwCustomizeAddressBlockDialog(Window* pParent, SwMailMergeConfigItem& rConfig, DialogType);
    ~SwCustomizeAddressBlockDialog();

    void            SetAddress(const OUString& rAddress);
    OUString GetAddress();
};

class SwAssignFieldsControl;
class SwAssignFieldsDialog : public SfxModalDialog
{
    FixedInfo               m_aMatchingFI;
    SwAssignFieldsControl*  m_pFieldsControl;

    FixedInfo               m_aPreviewFI;
    SwAddressPreview        m_aPreviewWIN;

    FixedLine               m_aSeparatorFL;

    OKButton                m_aOK;
    CancelButton            m_aCancel;
    HelpButton              m_aHelp;

    String                  m_sNone;
    OUString         m_rPreviewString;

    SwMailMergeConfigItem&  m_rConfigItem;

    ::com::sun::star::uno::Sequence< OUString > CreateAssignments();
    DECL_LINK(OkHdl_Impl, void *);
    DECL_LINK(AssignmentModifyHdl_Impl, void*);

public:
    SwAssignFieldsDialog(Window* pParent,
                SwMailMergeConfigItem& rConfigItem,
                const OUString& rPreview,
                bool bIsAddressBlock);
    ~SwAssignFieldsDialog();
};
#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

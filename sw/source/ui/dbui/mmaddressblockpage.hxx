/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: mmaddressblockpage.hxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: vg $ $Date: 2008-03-18 16:02:35 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _MAILMERGEADDRESSBLOCKPAGE_HXX
#define _MAILMERGEADDRESSBLOCKPAGE_HXX

#ifndef _SVTOOLS_WIZARDMACHINE_HXX_
#include <svtools/wizardmachine.hxx>
#endif
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#ifndef _STDCTRL_HXX
#include <svtools/stdctrl.hxx>
#endif
#ifndef _MAILMERGEHELPER_HXX
#include <mailmergehelper.hxx>
#endif
#ifndef _BASEDLGS_HXX
#include <sfx2/basedlgs.hxx>
#endif
#ifndef _SV_EDIT_HXX
#include <vcl/edit.hxx>
#endif
#ifndef _SV_LSTBOX_HXX
#include <vcl/lstbox.hxx>
#endif
#ifndef _SVEDIT_HXX
#include <svtools/svmedit.hxx>
#endif
#ifndef _HEADBAR_HXX
#include <svtools/headbar.hxx>
#endif
#ifndef _SVTREEBOX_HXX
#include <svtools/svtreebx.hxx>
#endif
#ifndef _SV_COMBOBOX_HXX
#include <vcl/combobox.hxx>
#endif
#ifndef _SFXLSTNER_HXX
#include <svtools/lstner.hxx>
#endif
class SwMailMergeWizard;
class SwMailMergeConfigItem;
/*-- 13.04.2004 15:50:37---------------------------------------------------

  -----------------------------------------------------------------------*/
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

    DECL_LINK(AddressListHdl_Impl, PushButton*);
    DECL_LINK(SettingsHdl_Impl, PushButton*);
    DECL_LINK(AssignHdl_Impl, PushButton*);
    DECL_LINK(AddressBlockHdl_Impl, CheckBox*);
    DECL_LINK(InsertDataHdl_Impl, ImageButton*);
    DECL_LINK(AddressBlockSelectHdl_Impl, SwAddressPreview*);
    DECL_LINK(HideParagraphsHdl_Impl, CheckBox*);

    void                EnableAddressBlock(sal_Bool bAll, sal_Bool bSelective);

    virtual void        ActivatePage();
    virtual sal_Bool    commitPage( CommitPageReason _eReason );
    virtual bool        canAdvance() const;

public:
    SwMailMergeAddressBlockPage( SwMailMergeWizard* _pParent);
    ~SwMailMergeAddressBlockPage();

    SwMailMergeWizard*  GetWizard() { return m_pWizard;}
};
/*-- 13.04.2004 15:45:31---------------------------------------------------

  -----------------------------------------------------------------------*/
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

    com::sun::star::uno::Sequence< ::rtl::OUString>    m_aAddressBlocks;
    SwMailMergeConfigItem& m_rConfig;

    DECL_LINK(NewCustomizeHdl_Impl, PushButton*);
    DECL_LINK(DeleteHdl_Impl, PushButton*);
    DECL_LINK(IncludeHdl_Impl, RadioButton*);

    using Window::SetSettings;

public:
    SwSelectAddressBlockDialog(Window* pParent, SwMailMergeConfigItem& rConfig);
    ~SwSelectAddressBlockDialog();

    void    SetAddressBlocks(const com::sun::star::uno::Sequence< ::rtl::OUString>& rBlocks,
                                sal_uInt16 nSelected);
    const com::sun::star::uno::Sequence< ::rtl::OUString>&    GetAddressBlocks();

    void                SetSettings(sal_Bool bIsCountry, ::rtl::OUString sCountry);
    sal_Bool            IsIncludeCountry() const {return !m_aNeverRB.IsChecked();}
    ::rtl::OUString     GetCountry() const;
};


/*-- 26.05.2004 11:16:25---------------------------------------------------

  -----------------------------------------------------------------------*/
class SwCustomizeAddressBlockDialog;
class DDListBox : public SvTreeListBox
{
    SwCustomizeAddressBlockDialog*   m_pParentDialog;
public:
    DDListBox(SwCustomizeAddressBlockDialog* pParent, const ResId rResId);
    ~DDListBox();

    virtual void        StartDrag( sal_Int8 nAction, const Point& rPosPixel );
};
/*-- 26.05.2004 11:16:25---------------------------------------------------

  -----------------------------------------------------------------------*/
#define MOVE_ITEM_LEFT           1
#define MOVE_ITEM_RIGHT          2
#define MOVE_ITEM_UP             4
#define MOVE_ITEM_DOWN           8

class AddressMultiLineEdit : public MultiLineEdit, public SfxListener
{
    Link                            m_aSelectionLink;
    SwCustomizeAddressBlockDialog*  m_pParentDialog;

    using Window::Notify;

    using MultiLineEdit::SetText;

protected:
    long            PreNotify( NotifyEvent& rNEvt );
public:
    AddressMultiLineEdit(SwCustomizeAddressBlockDialog* pParent, const ResId& rResId);
    ~AddressMultiLineEdit();

    virtual void    Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

    void            SetSelectionChangedHdl( const Link& rLink ) {m_aSelectionLink = rLink;}

    void            SetText( const String& rStr );
    String          GetAddress();

    void            InsertNewEntry( const String& rStr );
    void            InsertNewEntryAtPosition( const String& rStr, ULONG nPara, USHORT nIndex );
    void            RemoveCurrentEntry();

    void            MoveCurrentItem(sal_uInt16 nMove);
    sal_uInt16      IsCurrentItemMoveable();
    bool            HasCurrentItem();
    String          GetCurrentItem();
    void            SelectCurrentItem();
};
/*-- 13.04.2004 15:45:31---------------------------------------------------
    Dialog is used to create custom address blocks as well as
    custom greeting lines
  -----------------------------------------------------------------------*/
class SwRestrictedComboBox : public ComboBox
{
    String sForbiddenChars;

protected:
    virtual void KeyInput( const KeyEvent& );
    virtual void        Modify();
public:
    SwRestrictedComboBox(Window* pParent, const ResId& rResId):
        ComboBox( pParent, rResId ){}

    ~SwRestrictedComboBox();

    void            SetForbiddenChars(const String& rSet){sForbiddenChars = rSet;}

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
    FixedText       m_aAddressElementsFT;
    DDListBox       m_aAddressElementsLB;

    ImageButton     m_aInsertFieldIB;
    ImageButton     m_aRemoveFieldIB;

    FixedText               m_aDragFT;
    AddressMultiLineEdit    m_aDragED;
    ImageButton             m_aUpIB;
    ImageButton             m_aLeftIB;
    ImageButton             m_aRightIB;
    ImageButton             m_aDownIB;

    FixedText               m_aFieldFT;
    SwRestrictedComboBox    m_aFieldCB;

    FixedInfo               m_aPreviewFI;
    SwAddressPreview        m_aPreviewWIN;

    FixedLine               m_aSeparatorFL;

    OKButton                m_aOK;
    CancelButton            m_aCancel;
    HelpButton              m_aHelp;

    ::std::vector<String>   m_aSalutations;
    ::std::vector<String>   m_aPunctuations;

    String                  m_sCurrentSalutation;
    String                  m_sCurrentPunctuation;
    String                  m_sCurrentText;

    SwMailMergeConfigItem&  m_rConfigItem;
    DialogType              m_eType;

    DECL_LINK(OKHdl_Impl, OKButton*);
    DECL_LINK(ListBoxSelectHdl_Impl, DDListBox*);
    DECL_LINK(EditModifyHdl_Impl, AddressMultiLineEdit*);
    DECL_LINK(ImageButtonHdl_Impl, ImageButton*);
    DECL_LINK(SelectionChangedHdl_Impl, AddressMultiLineEdit*);
    DECL_LINK(FieldChangeHdl_Impl, ComboBox*);

    bool            HasItem_Impl(sal_Int32 nUserData);
    sal_Int32       GetSelectedItem_Impl();
    void            UpdateImageButtons_Impl();
    void            MoveFocus( Window* pMember, bool bNext );

public:
    SwCustomizeAddressBlockDialog(Window* pParent, SwMailMergeConfigItem& rConfig, DialogType);
    ~SwCustomizeAddressBlockDialog();

    void            SetAddress(const ::rtl::OUString& rAddress);
    ::rtl::OUString GetAddress();
};
/*-- 13.04.2004 17:41:11---------------------------------------------------

  -----------------------------------------------------------------------*/
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
    ::rtl::OUString         m_rPreviewString;

    SwMailMergeConfigItem&  m_rConfigItem;

    ::com::sun::star::uno::Sequence< ::rtl::OUString > CreateAssignments();
    DECL_LINK(OkHdl_Impl, PushButton*);
    DECL_LINK(AssignmentModifyHdl_Impl, void*);

public:
    SwAssignFieldsDialog(Window* pParent,
                SwMailMergeConfigItem& rConfigItem,
                const ::rtl::OUString& rPreview,
                bool bIsAddressBlock);
    ~SwAssignFieldsDialog();
};
#endif



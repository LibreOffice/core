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
#ifndef INCLUDED_CUI_SOURCE_INC_AUTOCDLG_HXX
#define INCLUDED_CUI_SOURCE_INC_AUTOCDLG_HXX

#include <sfx2/tabdlg.hxx>
#include <svtools/langtab.hxx>
#include <svtools/simptabl.hxx>
#include <vcl/svtabbx.hxx>
#include <vcl/treelistentry.hxx>
#include <svx/checklbx.hxx>
#include <svx/langbox.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <vcl/field.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/metric.hxx>

class CharClass;
class CollatorWrapper;
class SmartTagMgr;

namespace editeng { class SortedAutoCompleteStrings; }

// class OfaAutoCorrDlg --------------------------------------------------

class OfaAutoCorrDlg : public SfxTabDialog
{
    VclPtr<VclContainer> m_pLanguageBox;
    VclPtr<SvxLanguageBox>  m_pLanguageLB;

    sal_uInt16 m_nReplacePageId;
    sal_uInt16 m_nExceptionsPageId;

    DECL_LINK(SelectLanguageHdl, ListBox&, void);
public:

    OfaAutoCorrDlg(vcl::Window* pParent, const SfxItemSet *pSet);
    virtual ~OfaAutoCorrDlg() override;
    virtual void dispose() override;

    void EnableLanguage(bool bEnable);
};

// class OfaACorrCheckListBox ------------------------------------------

class OfaACorrCheckListBox : public SvSimpleTable
{
    using SvSimpleTable::SetTabs;
    using SvTreeListBox::GetCheckButtonState;
    using SvTreeListBox::SetCheckButtonState;

    protected:
        virtual void    SetTabs() override;
        virtual void    HBarClick() override;
        virtual void    KeyInput( const KeyEvent& rKEvt ) override;

    public:
        OfaACorrCheckListBox(SvSimpleTableContainer& rParent)
            : SvSimpleTable(rParent, WB_BORDER)
        {
        }

        void *GetUserData(sal_uLong nPos) { return GetEntry(nPos)->GetUserData(); }
        void SetUserData(sal_uLong nPos, void *pData ) { GetEntry(nPos)->SetUserData(pData); }
        sal_uLong GetSelectedEntryPos() { return GetModel()->GetAbsPos(FirstSelected()); }

        bool            IsChecked(sal_uLong nPos, sal_uInt16 nCol = 0);
        void            CheckEntryPos(sal_uLong nPos, sal_uInt16 nCol, bool bChecked);
        static SvButtonState GetCheckButtonState( SvTreeListEntry*, sal_uInt16 nCol );
        void            SetCheckButtonState( SvTreeListEntry*, sal_uInt16 nCol, SvButtonState );
};

// class OfaAutocorrOptionsPage ------------------------------------------


class OfaAutocorrOptionsPage : public SfxTabPage
{
    using TabPage::ActivatePage;

private:
    VclPtr<SvxCheckListBox> m_pCheckLB;

    OUString m_sInput;
    OUString m_sDoubleCaps;
    OUString m_sStartCap;
    OUString m_sBoldUnderline;
    OUString m_sURL;
    OUString m_sNoDblSpaces;
    OUString m_sDash;
    OUString m_sAccidentalCaps;

public:
    OfaAutocorrOptionsPage(vcl::Window* pParent, const SfxItemSet& rSet);
    virtual ~OfaAutocorrOptionsPage() override;
    virtual void dispose() override;

    static VclPtr<SfxTabPage>  Create( TabPageParent pParent,
                                const SfxItemSet* rAttrSet);

    virtual bool        FillItemSet( SfxItemSet* rSet ) override;
    virtual void        Reset( const SfxItemSet* rSet ) override;
    virtual void        ActivatePage( const SfxItemSet& ) override;

};

// class OfaSwAutoFmtOptionsPage ----------------------------------------------------

class OfaSwAutoFmtOptionsPage : public SfxTabPage
{
    friend class VclPtr<OfaSwAutoFmtOptionsPage>;
    using TabPage::ActivatePage;

    OUString        sDeleteEmptyPara;
    OUString        sUseReplaceTbl;
    OUString        sCapitalStartWord;
    OUString        sCapitalStartSentence;
    OUString        sUserStyle;
    OUString        sBullet;
    OUString        sBoldUnder;
    OUString        sNoDblSpaces;
    OUString        sCorrectCapsLock;
    OUString        sDetectURL;
    OUString        sDash;
    OUString        sRightMargin;
    OUString        sNum;
    OUString        sBorder;
    OUString        sTable;
    OUString        sReplaceTemplates;
    OUString        sDelSpaceAtSttEnd;
    OUString        sDelSpaceBetweenLines;

    OUString        sMargin;
    OUString        sBulletChar;
    OUString        sByInputBulletChar;

    vcl::Font        aBulletFont;
    vcl::Font        aByInputBulletFont;
    sal_uInt16       nPercent;

    std::unique_ptr<weld::TreeView> m_xCheckLB;
    std::unique_ptr<weld::Button> m_xEditPB;

    DECL_LINK(SelectHdl, weld::TreeView&, void);
    DECL_LINK(EditHdl, weld::Button&, void);
    DECL_LINK(DoubleClickEditHdl, weld::TreeView&, void);

    void CreateEntry(const OUString& rTxt, sal_uInt16 nCol);

    OfaSwAutoFmtOptionsPage(TabPageParent pParent, const SfxItemSet& rSet);
    virtual ~OfaSwAutoFmtOptionsPage() override;
    virtual void dispose() override;

public:
    static VclPtr<SfxTabPage>  Create( TabPageParent pParent,
                            const SfxItemSet* rAttrSet);
    virtual bool        FillItemSet( SfxItemSet* rSet ) override;
    virtual void        Reset( const SfxItemSet* rSet ) override;
    virtual void        ActivatePage( const SfxItemSet& ) override;
};

// class OfaAutocorrReplacePage ------------------------------------------

struct DoubleString
{
    OUString  sShort;
    OUString  sLong;
    void*   pUserData; ///< CheckBox -> form. Text Bool -> selection text
};

typedef std::vector<DoubleString> DoubleStringArray;

struct StringChangeList
{
    DoubleStringArray aNewEntries;
    DoubleStringArray aDeletedEntries;
};

typedef std::map<LanguageType, StringChangeList> StringChangeTable;

class OfaAutocorrReplacePage : public SfxTabPage
{
    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;

private:

    StringChangeTable aChangesTable;

    OUString        sModify;
    OUString        sNew;

    std::set<OUString>      aFormatText;
    std::map<LanguageType, DoubleStringArray>
                            aDoubleStringTable;
    std::unique_ptr<CollatorWrapper>  pCompareClass;
    std::unique_ptr<CharClass>        pCharClass;
    LanguageType            eLang;

    bool bHasSelectionText;
    bool bFirstSelect:1;
    bool bReplaceEditChanged:1;
    bool bSWriter:1;

    std::unique_ptr<weld::CheckButton> m_xTextOnlyCB;
    std::unique_ptr<weld::Entry> m_xShortED;
    std::unique_ptr<weld::Entry> m_xReplaceED;
    std::unique_ptr<weld::TreeView> m_xReplaceTLB;
    std::unique_ptr<weld::Button> m_xNewReplacePB;
    std::unique_ptr<weld::Button> m_xReplacePB;
    std::unique_ptr<weld::Button> m_xDeleteReplacePB;

    DECL_LINK(SelectHdl, weld::TreeView&, void);
    DECL_LINK(NewDelButtonHdl, weld::Button&, void);
    DECL_LINK(NewDelActionHdl, weld::Entry&, bool);
    DECL_LINK(EntrySizeAllocHdl, const Size&, void);
    DECL_LINK(ModifyHdl, weld::Entry&, void);
    bool NewDelHdl(const weld::Widget*);

    void RefillReplaceBox(  bool bFromReset,
                            LanguageType eOldLanguage,
                            LanguageType eNewLanguage);

public:
    OfaAutocorrReplacePage(TabPageParent pParent, const SfxItemSet& rSet);
    virtual ~OfaAutocorrReplacePage() override;
    virtual void dispose() override;

    static VclPtr<SfxTabPage>  Create( TabPageParent pParent, const SfxItemSet* rAttrSet);

    virtual bool        FillItemSet( SfxItemSet* rSet ) override;
    virtual void        Reset( const SfxItemSet* rSet ) override;
    virtual void        ActivatePage( const SfxItemSet& ) override;
    virtual DeactivateRC DeactivatePage( SfxItemSet* pSet ) override;

    void    SetLanguage(LanguageType eSet);
    void    DeleteEntry(const OUString& sShort, const OUString& sLong);
    void    NewEntry(const OUString& sShort, const OUString& sLong, bool bKeepSourceFormatting);
};

// class OfaAutocorrExceptPage ---------------------------------------------

struct StringsArrays
{
    std::vector<OUString> aAbbrevStrings;
    std::vector<OUString> aDoubleCapsStrings;

    StringsArrays() { }
};
typedef std::map<LanguageType, StringsArrays> StringsTable;

class OfaAutocorrExceptPage : public SfxTabPage
{
    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;

private:
    StringsTable    aStringsTable;
    std::unique_ptr<CollatorWrapper> pCompareClass;
    LanguageType    eLang;

    std::unique_ptr<weld::Entry> m_xAbbrevED;
    std::unique_ptr<weld::TreeView> m_xAbbrevLB;
    std::unique_ptr<weld::Button> m_xNewAbbrevPB;
    std::unique_ptr<weld::Button> m_xDelAbbrevPB;
    std::unique_ptr<weld::CheckButton> m_xAutoAbbrevCB;

    std::unique_ptr<weld::Entry> m_xDoubleCapsED;
    std::unique_ptr<weld::TreeView> m_xDoubleCapsLB;
    std::unique_ptr<weld::Button> m_xNewDoublePB;
    std::unique_ptr<weld::Button> m_xDelDoublePB;
    std::unique_ptr<weld::CheckButton> m_xAutoCapsCB;

    DECL_LINK(NewDelButtonHdl, weld::Button&, void);
    DECL_LINK(NewDelActionHdl, weld::Entry&, bool);
    DECL_LINK(SelectHdl, weld::TreeView&, void);
    DECL_LINK(ModifyHdl, weld::Entry&, void);
    void            NewDelHdl(const weld::Widget*);
                    /// Box filled with new language
    void            RefillReplaceBoxes(bool bFromReset,
                                        LanguageType eOldLanguage,
                                        LanguageType eNewLanguage);
public:
    OfaAutocorrExceptPage(TabPageParent pParent, const SfxItemSet& rSet);
    virtual ~OfaAutocorrExceptPage() override;
    virtual void        dispose() override;

    static VclPtr<SfxTabPage>  Create( TabPageParent pParent,
                                const SfxItemSet* rAttrSet);

    virtual bool        FillItemSet( SfxItemSet* rSet ) override;
    virtual void        Reset( const SfxItemSet* rSet ) override;
    virtual void        ActivatePage( const SfxItemSet& ) override;
    virtual DeactivateRC DeactivatePage( SfxItemSet* pSet ) override;
    void                SetLanguage(LanguageType eSet);

};

// class OfaQuoteTabPage -------------------------------------------------

class OfaQuoteTabPage : public SfxTabPage
{
    friend class VclPtr<OfaQuoteTabPage>;
    using TabPage::ActivatePage;

private:
    OUString        sNonBrkSpace;
    OUString        sOrdinal;

    sal_UCS4    cSglStartQuote;
    sal_UCS4    cSglEndQuote;

    sal_UCS4    cStartQuote;
    sal_UCS4    cEndQuote;

    std::unique_ptr<weld::CheckButton> m_xSingleTypoCB;
    std::unique_ptr<weld::Button> m_xSglStartQuotePB;
    std::unique_ptr<weld::Label> m_xSglStartExFT;
    std::unique_ptr<weld::Button> m_xSglEndQuotePB;
    std::unique_ptr<weld::Label> m_xSglEndExFT;
    std::unique_ptr<weld::Button> m_xSglStandardPB;
    std::unique_ptr<weld::CheckButton> m_xDoubleTypoCB;
    std::unique_ptr<weld::Button> m_xDblStartQuotePB;
    std::unique_ptr<weld::Label> m_xDblStartExFT;
    std::unique_ptr<weld::Button> m_xDblEndQuotePB;
    std::unique_ptr<weld::Label> m_xDblEndExFT;
    std::unique_ptr<weld::Button> m_xDblStandardPB;
    std::unique_ptr<weld::Label> m_xStandard;
    /// For anything but writer
    std::unique_ptr<weld::TreeView> m_xCheckLB;
    /// Just for writer
    std::unique_ptr<weld::TreeView> m_xSwCheckLB;


    DECL_LINK(QuoteHdl, weld::Button&, void);
    DECL_LINK(StdQuoteHdl, weld::Button&, void);

    OUString            ChangeStringExt_Impl( sal_UCS4 );

    static void CreateEntry(weld::TreeView& rLstBox, const OUString& rTxt,
                            sal_uInt16 nCol, sal_uInt16 nTextCol);

    OfaQuoteTabPage(TabPageParent pParent, const SfxItemSet& rSet);
public:
    virtual ~OfaQuoteTabPage() override;

    static VclPtr<SfxTabPage> Create(TabPageParent pParent,
                                     const SfxItemSet* rAttrSet);

    virtual bool        FillItemSet( SfxItemSet* rSet ) override;
    virtual void        Reset( const SfxItemSet* rSet ) override;
    virtual void        ActivatePage( const SfxItemSet& ) override;
};

// class OfaAutoCompleteTabPage ---------------------------------------------

class OfaAutoCompleteTabPage : public SfxTabPage
{
    friend class VclPtr<OfaAutoCompleteTabPage>;
private:
    using TabPage::ActivatePage;
    editeng::SortedAutoCompleteStrings* m_pAutoCompleteList;
    sal_uInt16      m_nAutoCmpltListCnt;

    std::unique_ptr<weld::CheckButton> m_xCBActiv; ///<Enable word completion
    std::unique_ptr<weld::CheckButton> m_xCBAppendSpace;///<Append space
    std::unique_ptr<weld::CheckButton> m_xCBAsTip; ///<Show as tip

    std::unique_ptr<weld::CheckButton> m_xCBCollect;///<Collect words
    std::unique_ptr<weld::CheckButton> m_xCBRemoveList;///<...save the list for later use...

    std::unique_ptr<weld::ComboBox> m_xDCBExpandKey;
    std::unique_ptr<weld::SpinButton> m_xNFMinWordlen;
    std::unique_ptr<weld::SpinButton> m_xNFMaxEntries;
    std::unique_ptr<weld::TreeView> m_xLBEntries;
    std::unique_ptr<weld::Button> m_xPBEntries;

    DECL_LINK(CheckHdl, weld::ToggleButton&, void);
    DECL_LINK(KeyReleaseHdl, const KeyEvent&, bool);

    OfaAutoCompleteTabPage(TabPageParent pParent, const SfxItemSet& rSet);
public:
    virtual ~OfaAutoCompleteTabPage() override;
    static VclPtr<SfxTabPage> Create(TabPageParent pParent,
                                     const SfxItemSet* rAttrSet);

    virtual bool        FillItemSet( SfxItemSet* rSet ) override;
    virtual void        Reset( const SfxItemSet* rSet ) override;
    virtual void        ActivatePage( const SfxItemSet& ) override;

    void CopyToClipboard() const;
    DECL_LINK(DeleteHdl, weld::Button&, void);
};

// class OfaSmartTagOptionsTabPage ---------------------------------------------

/** Smart tag options tab page

    This tab page is used to enable/disable smart tag types
*/
class OfaSmartTagOptionsTabPage : public SfxTabPage
{
    using TabPage::ActivatePage;

private:

    // controls
    VclPtr<CheckBox>               m_pMainCB;
    VclPtr<SvxCheckListBox>        m_pSmartTagTypesLB;
    VclPtr<PushButton>             m_pPropertiesPB;

    /** Inserts items into m_aSmartTagTypesLB

        Reads out the smart tag types supported by the SmartTagMgr and
        inserts the associated strings into the list box.
    */
    void FillListBox( const SmartTagMgr& rSmartTagMgr );

    /** Clears the m_aSmartTagTypesLB
    */
    void ClearListBox();

    /** Handler for the check box

        Enables/disables all controls in the tab page (except from the
        check box.
    */
    DECL_LINK(CheckHdl, CheckBox&, void);

    /** Handler for the push button

        Calls the displayPropertyPage function of the smart tag recognizer
        associated with the currently selected smart tag type.
    */
    DECL_LINK(ClickHdl, Button*, void);

    /** Handler for the list box

        Enables/disables the properties push button if selection in the
        smart tag types list box changes.
    */
    DECL_LINK(SelectHdl, SvTreeListBox*, void);

public:
    /// construction via Create()
    OfaSmartTagOptionsTabPage( vcl::Window* pParent, const SfxItemSet& rSet );
    virtual ~OfaSmartTagOptionsTabPage() override;
    virtual void dispose() override;

    static VclPtr<SfxTabPage>  Create( TabPageParent pParent, const SfxItemSet* rAttrSet);

    virtual bool        FillItemSet( SfxItemSet* rSet ) override;
    virtual void        Reset( const SfxItemSet* rSet ) override;
    virtual void        ActivatePage( const SfxItemSet& ) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

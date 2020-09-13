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
#pragma once

#include <sfx2/tabdlg.hxx>
#include <svx/langbox.hxx>
#include <vcl/metric.hxx>

#include <map>
#include <set>

class CharClass;
class CollatorWrapper;
class SmartTagMgr;

namespace editeng { class SortedAutoCompleteStrings; }

// class OfaAutoCorrDlg --------------------------------------------------

class OfaAutoCorrDlg : public SfxTabDialogController
{
    std::unique_ptr<weld::Widget> m_xLanguageBox;
    std::unique_ptr<SvxLanguageBox> m_xLanguageLB;

    DECL_LINK(SelectLanguageHdl, weld::ComboBox&, void);
public:

    OfaAutoCorrDlg(weld::Window* pParent, const SfxItemSet *pSet);
    virtual ~OfaAutoCorrDlg() override;

    void EnableLanguage(bool bEnable);
};

// class OfaAutocorrOptionsPage ------------------------------------------

class OfaAutocorrOptionsPage : public SfxTabPage
{
private:
    OUString m_sInput;
    OUString m_sDoubleCaps;
    OUString m_sStartCap;
    OUString m_sBoldUnderline;
    OUString m_sURL;
    OUString m_sNoDblSpaces;
    OUString m_sDash;
    OUString m_sAccidentalCaps;

    std::unique_ptr<weld::TreeView> m_xCheckLB;

    void InsertEntry(const OUString& rTxt);

public:
    OfaAutocorrOptionsPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rSet);
    virtual ~OfaAutocorrOptionsPage() override;

    static std::unique_ptr<SfxTabPage> Create( weld::Container* pPage, weld::DialogController* pController,
                                const SfxItemSet* rAttrSet);

    virtual bool        FillItemSet( SfxItemSet* rSet ) override;
    virtual void        Reset( const SfxItemSet* rSet ) override;
    virtual void        ActivatePage( const SfxItemSet& ) override;

};

// class OfaSwAutoFmtOptionsPage ----------------------------------------------------

class OfaSwAutoFmtOptionsPage : public SfxTabPage
{
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
    DECL_LINK(DoubleClickEditHdl, weld::TreeView&, bool);

    void CreateEntry(const OUString& rTxt, sal_uInt16 nCol);

public:
    OfaSwAutoFmtOptionsPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rSet);
    static std::unique_ptr<SfxTabPage> Create( weld::Container* pPage, weld::DialogController* pController,
                            const SfxItemSet* rAttrSet);
    virtual ~OfaSwAutoFmtOptionsPage() override;

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

    std::vector<int> m_aReplaceFixedWidths;
    std::unique_ptr<weld::CheckButton> m_xTextOnlyCB;
    std::unique_ptr<weld::Entry> m_xShortED;
    std::unique_ptr<weld::Entry> m_xReplaceED;
    std::unique_ptr<weld::TreeView> m_xReplaceTLB;
    std::unique_ptr<weld::Button> m_xNewReplacePB;
    std::unique_ptr<weld::Button> m_xReplacePB;
    std::unique_ptr<weld::Button> m_xDeleteReplacePB;
    std::unique_ptr<weld::Container> m_xButtonBox;

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
    OfaAutocorrReplacePage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rSet);
    virtual ~OfaAutocorrReplacePage() override;

    static std::unique_ptr<SfxTabPage> Create( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rAttrSet);

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
    OfaAutocorrExceptPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rSet);
    virtual ~OfaAutocorrExceptPage() override;

    static std::unique_ptr<SfxTabPage> Create( weld::Container* pPage, weld::DialogController* pController,
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
private:
    OUString        sNonBrkSpace;
    OUString        sOrdinal;
    OUString        sTransliterateRTL;
    OUString        sAngleQuotes;

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
    OUString m_sStandard;
    /// For anything but writer
    std::unique_ptr<weld::TreeView> m_xCheckLB;
    /// Just for writer
    std::unique_ptr<weld::TreeView> m_xSwCheckLB;


    DECL_LINK(QuoteHdl, weld::Button&, void);
    DECL_LINK(StdQuoteHdl, weld::Button&, void);

    OUString            ChangeStringExt_Impl( sal_UCS4 );

    static void CreateEntry(weld::TreeView& rLstBox, const OUString& rTxt,
                            sal_uInt16 nCol, sal_uInt16 nTextCol);

public:
    OfaQuoteTabPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rSet);
    static std::unique_ptr<SfxTabPage> Create(weld::Container* pPage, weld::DialogController* pController,
                                     const SfxItemSet* rAttrSet);
    virtual ~OfaQuoteTabPage() override;

    virtual bool        FillItemSet( SfxItemSet* rSet ) override;
    virtual void        Reset( const SfxItemSet* rSet ) override;
    virtual void        ActivatePage( const SfxItemSet& ) override;
};

// class OfaAutoCompleteTabPage ---------------------------------------------

class OfaAutoCompleteTabPage : public SfxTabPage
{
private:
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

public:
    OfaAutoCompleteTabPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rSet);
    static std::unique_ptr<SfxTabPage> Create(weld::Container* pPage, weld::DialogController* pController,
                                     const SfxItemSet* rAttrSet);
    virtual ~OfaAutoCompleteTabPage() override;

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
private:

    // controls
    std::unique_ptr<weld::CheckButton> m_xMainCB;
    std::unique_ptr<weld::TreeView> m_xSmartTagTypesLB;
    std::unique_ptr<weld::Button> m_xPropertiesPB;

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
    DECL_LINK(CheckHdl, weld::ToggleButton&, void);

    /** Handler for the push button

        Calls the displayPropertyPage function of the smart tag recognizer
        associated with the currently selected smart tag type.
    */
    DECL_LINK(ClickHdl, weld::Button&, void);

    /** Handler for the list box

        Enables/disables the properties push button if selection in the
        smart tag types list box changes.
    */
    DECL_LINK(SelectHdl, weld::TreeView&, void);

public:
    /// construction via Create()
    OfaSmartTagOptionsTabPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rSet);
    virtual ~OfaSmartTagOptionsTabPage() override;

    static std::unique_ptr<SfxTabPage> Create( weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rAttrSet);

    virtual bool        FillItemSet( SfxItemSet* rSet ) override;
    virtual void        Reset( const SfxItemSet* rSet ) override;
    virtual void        ActivatePage( const SfxItemSet& ) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

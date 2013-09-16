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
#ifndef _OFA_AUTOCDLG_HXX
#define _OFA_AUTOCDLG_HXX

#include <sfx2/tabdlg.hxx>
#include <svtools/langtab.hxx>
#include <svtools/simptabl.hxx>
#include <svtools/svtabbx.hxx>
#include <svtools/treelistentry.hxx>
#include <svx/checklbx.hxx>
#include <svx/langbox.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <vcl/field.hxx>
#include <vcl/group.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/metric.hxx>

class CharClass;
class CollatorWrapper;
class SmartTagMgr;

namespace editeng { class SortedAutoCompleteStrings; }

// class OfaAutoCorrDlg --------------------------------------------------

class OfaAutoCorrDlg : public SfxTabDialog
{
    VclContainer* m_pLanguageBox;
    SvxLanguageBox*  m_pLanguageLB;

    sal_uInt16 m_nReplacePageId;
    sal_uInt16 m_nExceptionsPageId;

    DECL_LINK(SelectLanguageHdl, ListBox*);
public:

    OfaAutoCorrDlg(Window* pParent, const SfxItemSet *pSet);

    void EnableLanguage(bool bEnable);
};

// class OfaACorrCheckListBox ------------------------------------------

class OfaACorrCheckListBox : public SvSimpleTable
{
    using SvSimpleTable::SetTabs;
    using SvTreeListBox::GetCheckButtonState;
    using SvTreeListBox::SetCheckButtonState;

    protected:
        virtual void    SetTabs();
        virtual void    HBarClick();
        virtual void    KeyInput( const KeyEvent& rKEvt );

    public:
        OfaACorrCheckListBox(SvSimpleTableContainer& rParent, WinBits nBits = WB_BORDER)
            : SvSimpleTable(rParent, nBits)
        {
        }

        inline void *GetUserData(sal_uLong nPos) { return GetEntry(nPos)->GetUserData(); }
        inline void SetUserData(sal_uLong nPos, void *pData ) { GetEntry(nPos)->SetUserData(pData); }
        inline sal_uLong GetSelectEntryPos() { return GetModel()->GetAbsPos(FirstSelected()); }

        sal_Bool            IsChecked(sal_uLong nPos, sal_uInt16 nCol = 0);
        void            CheckEntryPos(sal_uLong nPos, sal_uInt16 nCol, sal_Bool bChecked);
        SvButtonState   GetCheckButtonState( SvTreeListEntry*, sal_uInt16 nCol ) const;
        void            SetCheckButtonState( SvTreeListEntry*, sal_uInt16 nCol, SvButtonState );
};

// class OfaAutocorrOptionsPage ------------------------------------------


class OfaAutocorrOptionsPage : public SfxTabPage
{
    using TabPage::ActivatePage;

private:
    SvxCheckListBox *m_pCheckLB;

    OUString m_sInput;
    OUString m_sDoubleCaps;
    OUString m_sStartCap;
    OUString m_sBoldUnderline;
    OUString m_sURL;
    OUString m_sNoDblSpaces;
    OUString m_sDash;
    OUString m_sNonBrkSpace;
    OUString m_sFirst;
    OUString m_sAccidentalCaps;

public:
    OfaAutocorrOptionsPage(Window* pParent, const SfxItemSet& rSet);

    static SfxTabPage*  Create( Window* pParent,
                                const SfxItemSet& rAttrSet);

    virtual sal_Bool        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );
    virtual void        ActivatePage( const SfxItemSet& );

};

// class OfaSwAutoFmtOptionsPage ----------------------------------------------------

class OfaSwAutoFmtOptionsPage : public SfxTabPage
{
    using TabPage::ActivatePage;

    OfaACorrCheckListBox*   m_pCheckLB;
    PushButton*     m_pEditPB;

    String          sDeleteEmptyPara;
    String          sUseReplaceTbl;
    String          sCptlSttWord;
    String          sCptlSttSent;
    String          sUserStyle;
    String          sBullet;
    String          sByInputBullet;
    String          sBoldUnder;
    String          sNoDblSpaces;
    String          sCorrectCapsLock;
    String          sDetectURL;
    String          sDash;
    String          sNonBrkSpace;
    String          sOrdinal;
    String          sRightMargin;
    String          sNum;
    String          sBorder;
    String          sTable;
    String          sReplaceTemplates;
    String          sDelSpaceAtSttEnd;
    String          sDelSpaceBetweenLines;

    String          sMargin;
    String          sBulletChar;
    String          sByInputBulletChar;

    Font            aBulletFont;
    Font            aByInputBulletFont;
    sal_uInt16          nPercent;

    SvLBoxButtonData*   pCheckButtonData;

        DECL_LINK(SelectHdl, OfaACorrCheckListBox*);
        DECL_LINK(EditHdl, void *);
        SvTreeListEntry* CreateEntry(String& rTxt, sal_uInt16 nCol);


        OfaSwAutoFmtOptionsPage( Window* pParent,
                            const SfxItemSet& rSet );
        ~OfaSwAutoFmtOptionsPage();

    public:
        static SfxTabPage*  Create( Window* pParent,
                                const SfxItemSet& rAttrSet);
        virtual sal_Bool        FillItemSet( SfxItemSet& rSet );
        virtual void        Reset( const SfxItemSet& rSet );
        virtual void        ActivatePage( const SfxItemSet& );
};

// class AutoCorrEdit ----------------------------------------------------

class AutoCorrEdit : public Edit
{
    Link    aActionLink;
    sal_Bool    bSpaces;

    public:
                    AutoCorrEdit(Window* pParent, const ResId& rResId) :
                        Edit(pParent, rResId), bSpaces(sal_False){}
                    AutoCorrEdit(Window* pParent) :
                        Edit(pParent), bSpaces(sal_False){}

    void            SetActionHdl( const Link& rLink )
                                { aActionLink = rLink;}

    void            SetSpaces(sal_Bool bSet)
                                {bSpaces = bSet;}

    virtual void    KeyInput( const KeyEvent& rKEvent );
};

// class OfaAutocorrReplacePage ------------------------------------------


struct DoubleString
{
    String  sShort;
    String  sLong;
    void*   pUserData; ///< CheckBox -> form. Text Bool -> selection text
};

typedef std::vector<DoubleString> DoubleStringArray;
typedef std::map<LanguageType, DoubleStringArray> DoubleStringTable;

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

        CheckBox*       m_pTextOnlyCB;
        AutoCorrEdit*   m_pShortED;
        AutoCorrEdit*   m_pReplaceED;
        SvTabListBox*   m_pReplaceTLB;
        PushButton*     m_pNewReplacePB;
        PushButton*     m_pDeleteReplacePB;

        OUString        sModify;
        OUString        sNew;

        std::set<OUString> aFormatText;
        DoubleStringTable       aDoubleStringTable;
        CollatorWrapper*        pCompareClass;
        CharClass*              pCharClass;
        LanguageType            eLang;

        sal_Bool bHasSelectionText;
        sal_Bool bFirstSelect:1;
        sal_Bool bReplaceEditChanged:1;
        sal_Bool bSWriter:1;

        DECL_LINK(SelectHdl, SvTabListBox*);
        DECL_LINK(NewDelHdl, PushButton*);
        DECL_LINK(ModifyHdl, Edit*);

        void RefillReplaceBox(  sal_Bool bFromReset,
                                LanguageType eOldLanguage,
                                LanguageType eNewLanguage);

public:
                        OfaAutocorrReplacePage( Window* pParent, const SfxItemSet& rSet );
                        ~OfaAutocorrReplacePage();

    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rAttrSet);

    virtual sal_Bool    FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );
    virtual void        ActivatePage( const SfxItemSet& );
    virtual int         DeactivatePage( SfxItemSet* pSet = 0 );
    virtual void        Resize();

    void    SetLanguage(LanguageType eSet);
    void    DeleteEntry(String sShort, String sLong);
    void    NewEntry(String sShort, String sLong, bool bKeepSourceFormatting);
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
    AutoCorrEdit*   m_pAbbrevED;
    ListBox*        m_pAbbrevLB;
    PushButton*     m_pNewAbbrevPB;
    PushButton*     m_pDelAbbrevPB;
    CheckBox*       m_pAutoAbbrevCB;

    AutoCorrEdit*   m_pDoubleCapsED;
    ListBox*        m_pDoubleCapsLB;
    PushButton*     m_pNewDoublePB;
    PushButton*     m_pDelDoublePB;
    CheckBox*       m_pAutoCapsCB;

    StringsTable    aStringsTable;
    CollatorWrapper* pCompareClass;
    LanguageType    eLang;

    DECL_LINK(NewDelHdl, PushButton*);
    DECL_LINK(SelectHdl, ListBox*);
    DECL_LINK(ModifyHdl, Edit*);
                    /// Box filled with new language
    void            RefillReplaceBoxes(sal_Bool bFromReset,
                                        LanguageType eOldLanguage,
                                        LanguageType eNewLanguage);
public:
                        OfaAutocorrExceptPage( Window* pParent, const SfxItemSet& rSet );
                        ~OfaAutocorrExceptPage();

    static SfxTabPage*  Create( Window* pParent,
                                const SfxItemSet& rAttrSet);

    virtual sal_Bool        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );
    virtual void        ActivatePage( const SfxItemSet& );
    virtual int         DeactivatePage( SfxItemSet* pSet = 0 );
    void                SetLanguage(LanguageType eSet);

};

// class OfaQuoteTabPage -------------------------------------------------

class OfaQuoteTabPage : public SfxTabPage
{
    using TabPage::ActivatePage;

private:
    /// For anything but writer
    SvxCheckListBox* m_pCheckLB;

    /// Just for writer
    OfaACorrCheckListBox*   m_pSwCheckLB;

    String          sNonBrkSpace;
    String          sOrdinal;

    SvLBoxButtonData*   pCheckButtonData;

    CheckBox*   m_pSingleTypoCB;
    PushButton* m_pSglStartQuotePB;
    FixedText*  m_pSglStartExFT;
    PushButton* m_pSglEndQuotePB;
    FixedText*  m_pSglEndExFT;
    PushButton* m_pSglStandardPB;

    CheckBox*   m_pDoubleTypoCB;
    PushButton* m_pDblStartQuotePB;
    FixedText*  m_pDblStartExFT;
    PushButton* m_pDblEndQuotePB;
    FixedText*  m_pDblEndExFT;
    PushButton* m_pDblStandardPB;

    OUString    m_sStartQuoteDlg;
    OUString    m_sEndQuoteDlg;

    OUString    m_sStandard;


    sal_UCS4    cSglStartQuote;
    sal_UCS4    cSglEndQuote;

    sal_UCS4    cStartQuote;
    sal_UCS4    cEndQuote;

    DECL_LINK( QuoteHdl, PushButton* );
    DECL_LINK( StdQuoteHdl, PushButton* );

    String              ChangeStringExt_Impl( sal_UCS4 );

    SvTreeListEntry* CreateEntry(String& rTxt, sal_uInt16 nCol);

                        OfaQuoteTabPage( Window* pParent, const SfxItemSet& rSet );
public:
                        ~OfaQuoteTabPage();

    static SfxTabPage*  Create( Window* pParent,
                                const SfxItemSet& rAttrSet);

    virtual sal_Bool        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );
    virtual void        ActivatePage( const SfxItemSet& );
};

// class OfaAutoCompleteTabPage ---------------------------------------------

class OfaAutoCompleteTabPage : public SfxTabPage
{
    using TabPage::ActivatePage;

    class AutoCompleteMultiListBox : public MultiListBox
    {
        OfaAutoCompleteTabPage& rPage;
    public:
        AutoCompleteMultiListBox( OfaAutoCompleteTabPage& rPg,
                                    const ResId& rResId )
            : MultiListBox( &rPg, rResId ), rPage( rPg ) {}

        virtual long PreNotify( NotifyEvent& rNEvt );
    };

    CheckBox        aCBActiv; ///<Enable word completion
    CheckBox        aCBAppendSpace;///<Append space
    CheckBox        aCBAsTip; ///<Show as tip

    CheckBox        aCBCollect;///<Collect words
    CheckBox        aCBRemoveList;///<...save the list for later use...

    FixedText       aFTExpandKey;
    ListBox         aDCBExpandKey;
    FixedText       aFTMinWordlen;
    NumericField    aNFMinWordlen;
    FixedText       aFTMaxEntries;
    NumericField    aNFMaxEntries;
    AutoCompleteMultiListBox    aLBEntries;
    PushButton                  aPBEntries;
    editeng::SortedAutoCompleteStrings* m_pAutoCompleteList;
    sal_uInt16                      nAutoCmpltListCnt;

    DECL_LINK( CheckHdl, CheckBox* );

                        OfaAutoCompleteTabPage( Window* pParent,
                                                const SfxItemSet& rSet );
public:
                        virtual ~OfaAutoCompleteTabPage();

    static SfxTabPage*  Create( Window* pParent,
                                const SfxItemSet& rAttrSet);

    virtual sal_Bool        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );
    virtual void        ActivatePage( const SfxItemSet& );

    void CopyToClipboard() const;
    DECL_LINK(DeleteHdl, void *);
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
    CheckBox                m_aMainCB;
    SvxCheckListBox         m_aSmartTagTypesLB;
    PushButton              m_aPropertiesPB;
    FixedText               m_aTitleFT;

    /// construction via Create()
    OfaSmartTagOptionsTabPage( Window* pParent, const SfxItemSet& rSet );

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
    DECL_LINK(CheckHdl, void *);

    /** Handler for the push button

        Calls the displayPropertyPage function of the smart tag recognizer
        associated with the currently selected smart tag type.
    */
    DECL_LINK(ClickHdl, void *);

    /** Handler for the list box

        Enables/disables the properties push button if selection in the
        smart tag types list box changes.
    */
    DECL_LINK(SelectHdl, void *);

public:

    virtual ~OfaSmartTagOptionsTabPage();

    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rAttrSet);

    virtual sal_Bool        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );
    virtual void        ActivatePage( const SfxItemSet& );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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

#include <vcl/builder.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/field.hxx>
#include <vcl/keycodes.hxx>
#include <sot/exchange.hxx>
#include <svtools/transfer.hxx>
#include <unotools/syslocale.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/viewsh.hxx>
#include <unotools/charclass.hxx>
#include <unotools/collatorwrapper.hxx>
#include <com/sun/star/i18n/CollatorOptions.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>
#include <vcl/svapp.hxx>
#include <sfx2/module.hxx>
#include <sfx2/request.hxx>
#include <sfx2/sfxsids.hrc>
#include <svl/eitem.hxx>
#include <svl/languageoptions.hxx>
#include <svx/SmartTagMgr.hxx>
#include <com/sun/star/smarttags/XSmartTagRecognizer.hpp>
#include <com/sun/star/smarttags/XSmartTagAction.hpp>
#include <rtl/strbuf.hxx>

#include "autocdlg.hxx"
#include "autocdlg.hrc"
#include "helpid.hrc"
#include <editeng/acorrcfg.hxx>
#include <editeng/svxacorr.hxx>
#include "cuicharmap.hxx"
#include <editeng/unolingu.hxx>
#include <dialmgr.hxx>
#include <svx/svxids.hrc> // SID_OPEN_SMARTTAGSOPTIONS

static LanguageType eLastDialogLanguage = LANGUAGE_SYSTEM;

using namespace ::com::sun::star::util;
using namespace ::com::sun::star;
using namespace ::rtl;

OfaAutoCorrDlg::OfaAutoCorrDlg(Window* pParent, const SfxItemSet* _pSet )
    : SfxTabDialog(pParent, "AutoCorrectDialog", "cui/ui/autocorrectdialog.ui", _pSet)
{
    get(m_pLanguageBox, "langbox");
    get(m_pLanguageLB, "lang");

    sal_Bool bShowSWOptions = sal_False;
    sal_Bool bOpenSmartTagOptions = sal_False;

    if ( _pSet )
    {
        SFX_ITEMSET_ARG( _pSet, pItem, SfxBoolItem, SID_AUTO_CORRECT_DLG, sal_False );
        if ( pItem && pItem->GetValue() )
            bShowSWOptions = sal_True;

        SFX_ITEMSET_ARG( _pSet, pItem2, SfxBoolItem, SID_OPEN_SMARTTAGOPTIONS, sal_False );
        if ( pItem2 && pItem2->GetValue() )
            bOpenSmartTagOptions = sal_True;
    }

    AddTabPage("options", OfaAutocorrOptionsPage::Create, 0);
    AddTabPage("apply", OfaSwAutoFmtOptionsPage::Create, 0);
    AddTabPage("wordcompletion", OfaAutoCompleteTabPage::Create, 0);
    AddTabPage("smarttags", OfaSmartTagOptionsTabPage::Create, 0);

    if (!bShowSWOptions)
    {
        RemoveTabPage("apply");
        RemoveTabPage("wordcompletion");
        RemoveTabPage("smarttags");
    }
    else
    {
        // remove smart tag tab page if no extensions are installed
        SvxAutoCorrect* pAutoCorrect = SvxAutoCorrCfg::Get().GetAutoCorrect();
        SvxSwAutoFmtFlags *pOpt = &pAutoCorrect->GetSwFlags();
        if ( !pOpt || !pOpt->pSmartTagMgr || 0 == pOpt->pSmartTagMgr->NumberOfRecognizers() )
            RemoveTabPage("smarttags");

        RemoveTabPage("options");
    }

    m_nReplacePageId = AddTabPage("replace", OfaAutocorrReplacePage::Create, 0);
    m_nExceptionsPageId = AddTabPage("exceptions",  OfaAutocorrExceptPage::Create, 0);
    AddTabPage("localized", OfaQuoteTabPage::Create, 0);

    // initialize languages
    //! LANGUAGE_NONE is displayed as '[All]' and the LanguageType
    //! will be set to LANGUAGE_UNDETERMINED
    sal_Int16 nLangList = LANG_LIST_WESTERN;

    if( SvtLanguageOptions().IsCTLFontEnabled() )
        nLangList |= LANG_LIST_CTL;
    m_pLanguageLB->SetLanguageList( nLangList, sal_True, sal_True );
    m_pLanguageLB->SelectLanguage( LANGUAGE_NONE );
    sal_uInt16 nPos = m_pLanguageLB->GetSelectEntryPos();
    DBG_ASSERT( LISTBOX_ENTRY_NOTFOUND != nPos, "listbox entry missing" );
    m_pLanguageLB->SetEntryData( nPos, (void*)(long) LANGUAGE_UNDETERMINED );

    // Initializing doesn't work for static on linux - therefore here
    if( LANGUAGE_SYSTEM == eLastDialogLanguage )
        eLastDialogLanguage = Application::GetSettings().GetLanguageTag().getLanguageType();

    LanguageType nSelectLang = LANGUAGE_UNDETERMINED;
    nPos = m_pLanguageLB->GetEntryPos( (void*)(sal_IntPtr) eLastDialogLanguage );
    if (LISTBOX_ENTRY_NOTFOUND != nPos)
        nSelectLang = eLastDialogLanguage;
    m_pLanguageLB->SelectLanguage( nSelectLang );

    m_pLanguageLB->SetSelectHdl(LINK(this, OfaAutoCorrDlg, SelectLanguageHdl));

    if ( bOpenSmartTagOptions )
        SetCurPageId("smarttags");
}

void OfaAutoCorrDlg::EnableLanguage(bool bEnable)
{
    m_pLanguageBox->Enable(bEnable);
}

static sal_Bool lcl_FindEntry( ListBox& rLB, const String& rEntry,
                    CollatorWrapper& rCmpClass )
{
    sal_uInt16 nCount = rLB.GetEntryCount();
    sal_uInt16 nSelPos = rLB.GetSelectEntryPos();
    sal_uInt16 i;
    for(i = 0; i < nCount; i++)
    {
        if( 0 == rCmpClass.compareString(rEntry, rLB.GetEntry(i) ))
        {
            rLB.SelectEntryPos(i, sal_True);
            return sal_True;
        }
    }
    if(LISTBOX_ENTRY_NOTFOUND != nSelPos)
        rLB.SelectEntryPos(nSelPos, sal_False);
    return sal_False;
}

IMPL_LINK(OfaAutoCorrDlg, SelectLanguageHdl, ListBox*, pBox)
{
    sal_uInt16 nPos = pBox->GetSelectEntryPos();
    void* pVoid = pBox->GetEntryData(nPos);
    LanguageType eNewLang = (LanguageType)(sal_IntPtr)pVoid;
    // save old settings and fill anew
    if(eNewLang != eLastDialogLanguage)
    {
        sal_uInt16  nPageId = GetCurPageId();
        if (m_nReplacePageId == nPageId)
            ((OfaAutocorrReplacePage*)GetTabPage( nPageId ))->SetLanguage(eNewLang);
        else if (m_nExceptionsPageId == nPageId)
            ((OfaAutocorrExceptPage*)GetTabPage( nPageId ))->SetLanguage(eNewLang);
    }
    return 0;
}

OfaAutocorrOptionsPage::OfaAutocorrOptionsPage(Window* pParent, const SfxItemSet& rSet)
    : SfxTabPage(pParent, "AutocorrectOptionsPage", "cui/ui/acoroptionspage.ui", rSet)
    , m_sInput(CUI_RESSTR(RID_SVXSTR_USE_REPLACE))
    , m_sDoubleCaps(CUI_RESSTR(RID_SVXSTR_CPTL_STT_WORD))
    , m_sStartCap(CUI_RESSTR(RID_SVXSTR_CPTL_STT_SENT))
    , m_sBoldUnderline(CUI_RESSTR(RID_SVXSTR_BOLD_UNDER))
    , m_sURL(CUI_RESSTR(RID_SVXSTR_DETECT_URL))
    , m_sNoDblSpaces(CUI_RESSTR(RID_SVXSTR_NO_DBL_SPACES))
    , m_sDash(CUI_RESSTR(RID_SVXSTR_DASH))
    , m_sAccidentalCaps(CUI_RESSTR(RID_SVXSTR_CORRECT_ACCIDENTAL_CAPS_LOCK))
{
    get(m_pCheckLB, "checklist");
}

SfxTabPage* OfaAutocorrOptionsPage::Create( Window* pParent,
                                const SfxItemSet& rSet)
{
    return new OfaAutocorrOptionsPage(pParent, rSet);
}

sal_Bool OfaAutocorrOptionsPage::FillItemSet( SfxItemSet& )
{
    SvxAutoCorrect* pAutoCorrect = SvxAutoCorrCfg::Get().GetAutoCorrect();
    long nFlags = pAutoCorrect->GetFlags();

    sal_uInt16 nPos = 0;
    pAutoCorrect->SetAutoCorrFlag(Autocorrect,          m_pCheckLB->IsChecked(nPos++));
    pAutoCorrect->SetAutoCorrFlag(CptlSttWrd,           m_pCheckLB->IsChecked(nPos++));
    pAutoCorrect->SetAutoCorrFlag(CptlSttSntnc,         m_pCheckLB->IsChecked(nPos++));
    pAutoCorrect->SetAutoCorrFlag(ChgWeightUnderl,      m_pCheckLB->IsChecked(nPos++));
    pAutoCorrect->SetAutoCorrFlag(SetINetAttr,          m_pCheckLB->IsChecked(nPos++));
    pAutoCorrect->SetAutoCorrFlag(ChgToEnEmDash,        m_pCheckLB->IsChecked(nPos++));
    pAutoCorrect->SetAutoCorrFlag(IgnoreDoubleSpace,    m_pCheckLB->IsChecked(nPos++));
    pAutoCorrect->SetAutoCorrFlag(CorrectCapsLock,      m_pCheckLB->IsChecked(nPos++));

    sal_Bool bReturn = nFlags != pAutoCorrect->GetFlags();
    if(bReturn )
    {
        SvxAutoCorrCfg& rCfg = SvxAutoCorrCfg::Get();
        rCfg.SetModified();
        rCfg.Commit();
    }
    return bReturn;
}

void    OfaAutocorrOptionsPage::ActivatePage( const SfxItemSet& )
{
    ((OfaAutoCorrDlg*)GetTabDialog())->EnableLanguage(sal_False);
}

void OfaAutocorrOptionsPage::Reset( const SfxItemSet& )
{
    SvxAutoCorrect* pAutoCorrect = SvxAutoCorrCfg::Get().GetAutoCorrect();
    const long nFlags = pAutoCorrect->GetFlags();

    m_pCheckLB->SetUpdateMode(sal_False);
    m_pCheckLB->Clear();

    m_pCheckLB->InsertEntry(m_sInput);
    m_pCheckLB->InsertEntry(m_sDoubleCaps);
    m_pCheckLB->InsertEntry(m_sStartCap);
    m_pCheckLB->InsertEntry(m_sBoldUnderline);
    m_pCheckLB->InsertEntry(m_sURL);
    m_pCheckLB->InsertEntry(m_sDash);
    m_pCheckLB->InsertEntry(m_sNoDblSpaces);
    m_pCheckLB->InsertEntry(m_sAccidentalCaps);

    sal_uInt16 nPos = 0;
    m_pCheckLB->CheckEntryPos( nPos++, 0 != (nFlags & Autocorrect) );
    m_pCheckLB->CheckEntryPos( nPos++, 0 != (nFlags & CptlSttWrd) );
    m_pCheckLB->CheckEntryPos( nPos++, 0 != (nFlags & CptlSttSntnc) );
    m_pCheckLB->CheckEntryPos( nPos++, 0 != (nFlags & ChgWeightUnderl) );
    m_pCheckLB->CheckEntryPos( nPos++, 0 != (nFlags & SetINetAttr) );
    m_pCheckLB->CheckEntryPos( nPos++, 0 != (nFlags & ChgToEnEmDash) );
    m_pCheckLB->CheckEntryPos( nPos++, 0 != (nFlags & IgnoreDoubleSpace) );
    m_pCheckLB->CheckEntryPos( nPos++, 0 != (nFlags & CorrectCapsLock) );

    m_pCheckLB->SetUpdateMode(sal_True);
}

/*********************************************************************/
/*                                                                   */
/*  helping struct for dUserData of the Checklistbox                 */
/*                                                                   */
/*********************************************************************/

struct ImpUserData
{
    String  *pString;
    Font*   pFont;

    ImpUserData(String* pText, Font* pFnt)
        { pString = pText; pFont = pFnt;}
};


/*********************************************************************/
/*                                                                   */
/*  dialog for per cent settings                                     */
/*                                                                   */
/*********************************************************************/

class OfaAutoFmtPrcntSet : public ModalDialog
{
    MetricField* m_pPrcntMF;
public:
    OfaAutoFmtPrcntSet(Window* pParent)
        : ModalDialog(pParent, "PercentDialog","cui/ui/percentdialog.ui")
    {
        get(m_pPrcntMF, "margin");
    }
    MetricField& GetPrcntFld()
    {
        return *m_pPrcntMF;
    }
};

/*********************************************************************/
/*                                                                   */
/*  changed LBoxString                                               */
/*                                                                   */
/*********************************************************************/

class OfaImpBrwString : public SvLBoxString
{
public:

    OfaImpBrwString( SvTreeListEntry* pEntry, sal_uInt16 nFlags,
        const String& rStr ) : SvLBoxString(pEntry,nFlags,rStr){}

    virtual void Paint(
        const Point& rPos, SvTreeListBox& rDev, const SvViewDataEntry* pView,
        const SvTreeListEntry* pEntry);
};

void OfaImpBrwString::Paint(
    const Point& rPos, SvTreeListBox& rDev, const SvViewDataEntry* /*pView*/,
    const SvTreeListEntry* pEntry)
{
    rDev.DrawText( rPos, GetText() );
    if(pEntry->GetUserData())
    {
        ImpUserData* pUserData = (ImpUserData* )pEntry->GetUserData();
        Point aNewPos(rPos);
        aNewPos.X() += rDev.GetTextWidth(GetText());
        Font aOldFont( rDev.GetFont());
        Font aFont( aOldFont );
        if(pUserData->pFont)
        {
            aFont = *pUserData->pFont;
            aFont.SetColor(aOldFont.GetColor());
            aFont.SetSize(aOldFont.GetSize());
        }
        aFont.SetWeight( WEIGHT_BOLD );

        sal_Bool bFett = sal_True;
        sal_Int32 nPos = 0;
        do {
            String sTxt( pUserData->pString->GetToken( 0, 1, nPos ));

            if( bFett )
                rDev.SetFont( aFont );

            rDev.DrawText( aNewPos, sTxt );

            if( -1 != nPos )
                aNewPos.X() += rDev.GetTextWidth( sTxt );

            if( bFett )
                rDev.SetFont( aOldFont );

            bFett = !bFett;
        } while( -1 != nPos );
    }
}

/*********************************************************************/
/*                                                                   */
/*  use TabPage autoformat                                           */
/*                                                                   */
/*********************************************************************/

#define CBCOL_FIRST     0
#define CBCOL_SECOND    1
#define CBCOL_BOTH      2

enum OfaAutoFmtOptions
{
    USE_REPLACE_TABLE,
    CORR_UPPER,
    BEGIN_UPPER,
    BOLD_UNDERLINE,
    DETECT_URL,
    REPLACE_DASHES,
    DEL_SPACES_AT_STT_END,
    DEL_SPACES_BETWEEN_LINES,
    IGNORE_DBLSPACE,
    CORRECT_CAPS_LOCK,
    APPLY_NUMBERING,
    INSERT_BORDER,
    CREATE_TABLE,
    REPLACE_STYLES,
    DEL_EMPTY_NODE,
    REPLACE_USER_COLL,
    REPLACE_BULLETS,
    MERGE_SINGLE_LINE_PARA
};

OfaSwAutoFmtOptionsPage::OfaSwAutoFmtOptionsPage( Window* pParent,
                                const SfxItemSet& rSet )
    : SfxTabPage(pParent, "ApplyAutoFmtPage", "cui/ui/applyautofmtpage.ui", rSet)
    , sDeleteEmptyPara(CUI_RESSTR(RID_SVXSTR_DEL_EMPTY_PARA))
    , sUseReplaceTbl(CUI_RESSTR(RID_SVXSTR_USE_REPLACE))
    , sCptlSttWord(CUI_RESSTR(RID_SVXSTR_CPTL_STT_WORD))
    , sCptlSttSent(CUI_RESSTR(RID_SVXSTR_CPTL_STT_SENT))
    , sUserStyle(CUI_RESSTR(RID_SVXSTR_USER_STYLE))
    , sBullet(CUI_RESSTR(RID_SVXSTR_BULLET))
    , sBoldUnder(CUI_RESSTR(RID_SVXSTR_BOLD_UNDER))
    , sNoDblSpaces(CUI_RESSTR(RID_SVXSTR_NO_DBL_SPACES))
    , sCorrectCapsLock(CUI_RESSTR(RID_SVXSTR_CORRECT_ACCIDENTAL_CAPS_LOCK))
    , sDetectURL(CUI_RESSTR(RID_SVXSTR_DETECT_URL))
    , sDash(CUI_RESSTR(RID_SVXSTR_DASH))
    , sRightMargin(CUI_RESSTR(RID_SVXSTR_RIGHT_MARGIN))
    , sNum(CUI_RESSTR(RID_SVXSTR_NUM))
    , sBorder(CUI_RESSTR(RID_SVXSTR_BORDER))
    , sTable(CUI_RESSTR(RID_SVXSTR_CREATE_TABLE))
    , sReplaceTemplates(CUI_RESSTR(RID_SVXSTR_REPLACE_TEMPLATES))
    , sDelSpaceAtSttEnd(CUI_RESSTR(RID_SVXSTR_DEL_SPACES_AT_STT_END))
    , sDelSpaceBetweenLines(CUI_RESSTR(RID_SVXSTR_DEL_SPACES_BETWEEN_LINES))
    , nPercent(50)
    , pCheckButtonData(NULL)
{
    get(m_pEditPB, "edit");

    SvSimpleTableContainer* pCheckLBContainer(get<SvSimpleTableContainer>("list"));
    Size aControlSize(248 , 149);
    aControlSize = LogicToPixel(aControlSize, MAP_APPFONT);
    pCheckLBContainer->set_width_request(aControlSize.Width());
    pCheckLBContainer->set_height_request(aControlSize.Height());
    m_pCheckLB = new OfaACorrCheckListBox(*pCheckLBContainer);

    m_pCheckLB->SetStyle(m_pCheckLB->GetStyle()|WB_HSCROLL| WB_VSCROLL);

    m_pCheckLB->SetSelectHdl(LINK(this, OfaSwAutoFmtOptionsPage, SelectHdl));
    m_pCheckLB->SetDoubleClickHdl(LINK(this, OfaSwAutoFmtOptionsPage, EditHdl));

    static long aStaticTabs[]=
    {
        3, 0, 20, 40
    };

    m_pCheckLB->SvSimpleTable::SetTabs(aStaticTabs);
    OUStringBuffer sHeader(get<Window>("m")->GetText());
    sHeader.append('\t');
    sHeader.append(get<Window>("t")->GetText());
    sHeader.append('\t');
    m_pCheckLB->InsertHeaderEntry(sHeader.makeStringAndClear(), HEADERBAR_APPEND,
                        HIB_CENTER | HIB_VCENTER | HIB_FIXEDPOS | HIB_FIXED);

    m_pEditPB->SetClickHdl(LINK(this, OfaSwAutoFmtOptionsPage, EditHdl));
}

SvTreeListEntry* OfaSwAutoFmtOptionsPage::CreateEntry(String& rTxt, sal_uInt16 nCol)
{
    SvTreeListEntry* pEntry = new SvTreeListEntry;

    if ( !pCheckButtonData )
    {
        pCheckButtonData = new SvLBoxButtonData( m_pCheckLB );
        m_pCheckLB->SetCheckButtonData( pCheckButtonData );
    }

    pEntry->AddItem( new SvLBoxContextBmp( pEntry, 0, Image(), Image(), 0));

    String sEmpty;
    if (nCol == CBCOL_SECOND)
        pEntry->AddItem( new SvLBoxString( pEntry, 0, sEmpty) );
    else
        pEntry->AddItem( new SvLBoxButton( pEntry, SvLBoxButtonKind_enabledCheckbox, 0, pCheckButtonData ) );

    if (nCol == CBCOL_FIRST)
        pEntry->AddItem( new SvLBoxString( pEntry, 0, sEmpty) );
    else
        pEntry->AddItem( new SvLBoxButton( pEntry, SvLBoxButtonKind_enabledCheckbox, 0, pCheckButtonData ) );
    pEntry->AddItem( new OfaImpBrwString( pEntry, 0, rTxt ) );

    return pEntry;
}

OfaSwAutoFmtOptionsPage::~OfaSwAutoFmtOptionsPage()
{
    delete (ImpUserData*) m_pCheckLB->GetUserData( REPLACE_BULLETS );
    delete (ImpUserData*) m_pCheckLB->GetUserData( APPLY_NUMBERING );
    delete (ImpUserData*) m_pCheckLB->GetUserData( MERGE_SINGLE_LINE_PARA );
    delete pCheckButtonData;
    delete m_pCheckLB;
}

SfxTabPage* OfaSwAutoFmtOptionsPage::Create( Window* pParent,
                                const SfxItemSet& rAttrSet)
{
    return new OfaSwAutoFmtOptionsPage(pParent, rAttrSet);
}

sal_Bool OfaSwAutoFmtOptionsPage::FillItemSet( SfxItemSet&  )
{
    sal_Bool bModified = sal_False;
    SvxAutoCorrect* pAutoCorrect = SvxAutoCorrCfg::Get().GetAutoCorrect();
    SvxSwAutoFmtFlags *pOpt = &pAutoCorrect->GetSwFlags();
    long nFlags = pAutoCorrect->GetFlags();

    sal_Bool bCheck = m_pCheckLB->IsChecked(USE_REPLACE_TABLE, CBCOL_FIRST);
    bModified |= pOpt->bAutoCorrect != bCheck;
    pOpt->bAutoCorrect = bCheck;
    pAutoCorrect->SetAutoCorrFlag(Autocorrect,
                        m_pCheckLB->IsChecked(USE_REPLACE_TABLE, CBCOL_SECOND));

    bCheck = m_pCheckLB->IsChecked(CORR_UPPER, CBCOL_FIRST);
    bModified |= pOpt->bCptlSttWrd != bCheck;
    pOpt->bCptlSttWrd = bCheck;
    pAutoCorrect->SetAutoCorrFlag(CptlSttWrd,
                        m_pCheckLB->IsChecked(CORR_UPPER, CBCOL_SECOND));

    bCheck = m_pCheckLB->IsChecked(BEGIN_UPPER, CBCOL_FIRST);
    bModified |= pOpt->bCptlSttSntnc != bCheck;
    pOpt->bCptlSttSntnc = bCheck;
    pAutoCorrect->SetAutoCorrFlag(CptlSttSntnc,
                        m_pCheckLB->IsChecked(BEGIN_UPPER, CBCOL_SECOND));

    bCheck = m_pCheckLB->IsChecked(BOLD_UNDERLINE, CBCOL_FIRST);
    bModified |= pOpt->bChgWeightUnderl != bCheck;
    pOpt->bChgWeightUnderl = bCheck;
    pAutoCorrect->SetAutoCorrFlag(ChgWeightUnderl,
                        m_pCheckLB->IsChecked(BOLD_UNDERLINE, CBCOL_SECOND));

    pAutoCorrect->SetAutoCorrFlag(IgnoreDoubleSpace,
                        m_pCheckLB->IsChecked(IGNORE_DBLSPACE, CBCOL_SECOND));

    pAutoCorrect->SetAutoCorrFlag(CorrectCapsLock,
                        m_pCheckLB->IsChecked(CORRECT_CAPS_LOCK, CBCOL_SECOND));

    bCheck = m_pCheckLB->IsChecked(DETECT_URL, CBCOL_FIRST);
    bModified |= pOpt->bSetINetAttr != bCheck;
    pOpt->bSetINetAttr = bCheck;
    pAutoCorrect->SetAutoCorrFlag(SetINetAttr,
                        m_pCheckLB->IsChecked(DETECT_URL, CBCOL_SECOND));

    bCheck = m_pCheckLB->IsChecked(DEL_EMPTY_NODE, CBCOL_FIRST);
    bModified |= pOpt->bDelEmptyNode != bCheck;
    pOpt->bDelEmptyNode = bCheck;

    bCheck = m_pCheckLB->IsChecked(REPLACE_USER_COLL, CBCOL_FIRST);
    bModified |= pOpt->bChgUserColl != bCheck;
    pOpt->bChgUserColl = bCheck;

    bCheck = m_pCheckLB->IsChecked(REPLACE_BULLETS, CBCOL_FIRST);
    bModified |= pOpt->bChgEnumNum != bCheck;
    pOpt->bChgEnumNum = bCheck;
    bModified |= aBulletFont != pOpt->aBulletFont;
    pOpt->aBulletFont = aBulletFont;
    bModified |= !comphelper::string::equals(sBulletChar, pOpt->cBullet);
    pOpt->cBullet = sBulletChar.GetChar(0);

    bModified |= aByInputBulletFont != pOpt->aByInputBulletFont;
    bModified |= !comphelper::string::equals(sByInputBulletChar, pOpt->cByInputBullet);
    pOpt->aByInputBulletFont = aByInputBulletFont;
    pOpt->cByInputBullet = sByInputBulletChar.GetChar(0);

    bCheck = m_pCheckLB->IsChecked(MERGE_SINGLE_LINE_PARA, CBCOL_FIRST);
    bModified |= pOpt->bRightMargin != bCheck;
    pOpt->bRightMargin = bCheck;
    bModified |= nPercent != pOpt->nRightMargin;
    pOpt->nRightMargin = (sal_uInt8)nPercent;

    bCheck = m_pCheckLB->IsChecked(APPLY_NUMBERING, CBCOL_SECOND);
    bModified |= pOpt->bSetNumRule != bCheck;
    pOpt->bSetNumRule = bCheck;

    bCheck = m_pCheckLB->IsChecked(INSERT_BORDER, CBCOL_SECOND);
    bModified |= pOpt->bSetBorder != bCheck;
    pOpt->bSetBorder = bCheck;

    bCheck = m_pCheckLB->IsChecked(CREATE_TABLE, CBCOL_SECOND);
    bModified |= pOpt->bCreateTable != bCheck;
    pOpt->bCreateTable = bCheck;

    bCheck = m_pCheckLB->IsChecked(REPLACE_STYLES, CBCOL_SECOND);
    bModified |= pOpt->bReplaceStyles != bCheck;
    pOpt->bReplaceStyles = bCheck;

    bCheck = m_pCheckLB->IsChecked(REPLACE_DASHES, CBCOL_FIRST);
    bModified |= pOpt->bChgToEnEmDash != bCheck;
    pOpt->bChgToEnEmDash = bCheck;
    pAutoCorrect->SetAutoCorrFlag(ChgToEnEmDash,
                        m_pCheckLB->IsChecked(REPLACE_DASHES, CBCOL_SECOND));

    bCheck = m_pCheckLB->IsChecked(DEL_SPACES_AT_STT_END, CBCOL_FIRST);
    bModified |= pOpt->bAFmtDelSpacesAtSttEnd != bCheck;
    pOpt->bAFmtDelSpacesAtSttEnd = bCheck;
    bCheck = m_pCheckLB->IsChecked(DEL_SPACES_AT_STT_END, CBCOL_SECOND);
    bModified |= pOpt->bAFmtByInpDelSpacesAtSttEnd != bCheck;
    pOpt->bAFmtByInpDelSpacesAtSttEnd = bCheck;

    bCheck = m_pCheckLB->IsChecked(DEL_SPACES_BETWEEN_LINES, CBCOL_FIRST);
    bModified |= pOpt->bAFmtDelSpacesBetweenLines != bCheck;
    pOpt->bAFmtDelSpacesBetweenLines = bCheck;
    bCheck = m_pCheckLB->IsChecked(DEL_SPACES_BETWEEN_LINES, CBCOL_SECOND);
    bModified |= pOpt->bAFmtByInpDelSpacesBetweenLines != bCheck;
    pOpt->bAFmtByInpDelSpacesBetweenLines = bCheck;

    if(bModified || nFlags != pAutoCorrect->GetFlags())
    {
        SvxAutoCorrCfg& rCfg = SvxAutoCorrCfg::Get();
        rCfg.SetModified();
        rCfg.Commit();
    }

    return sal_True;
}

void    OfaSwAutoFmtOptionsPage::ActivatePage( const SfxItemSet& )
{
    ((OfaAutoCorrDlg*)GetTabDialog())->EnableLanguage(sal_False);
}

void OfaSwAutoFmtOptionsPage::Reset( const SfxItemSet& )
{
    SvxAutoCorrect* pAutoCorrect = SvxAutoCorrCfg::Get().GetAutoCorrect();
    SvxSwAutoFmtFlags *pOpt = &pAutoCorrect->GetSwFlags();
    const long nFlags = pAutoCorrect->GetFlags();

    m_pCheckLB->SetUpdateMode(sal_False);
    m_pCheckLB->Clear();

    // The following entries have to be inserted in the same order
    // as in the OfaAutoFmtOptions-enum!
    m_pCheckLB->GetModel()->Insert(CreateEntry(sUseReplaceTbl,     CBCOL_BOTH  ));
    m_pCheckLB->GetModel()->Insert(CreateEntry(sCptlSttWord,       CBCOL_BOTH  ));
    m_pCheckLB->GetModel()->Insert(CreateEntry(sCptlSttSent,       CBCOL_BOTH  ));
    m_pCheckLB->GetModel()->Insert(CreateEntry(sBoldUnder,         CBCOL_BOTH  ));
    m_pCheckLB->GetModel()->Insert(CreateEntry(sDetectURL,         CBCOL_BOTH  ));
    m_pCheckLB->GetModel()->Insert(CreateEntry(sDash,              CBCOL_BOTH  ));
    m_pCheckLB->GetModel()->Insert(CreateEntry(sDelSpaceAtSttEnd,  CBCOL_BOTH  ));
    m_pCheckLB->GetModel()->Insert(CreateEntry(sDelSpaceBetweenLines, CBCOL_BOTH  ));

    m_pCheckLB->GetModel()->Insert(CreateEntry(sNoDblSpaces,       CBCOL_SECOND));
    m_pCheckLB->GetModel()->Insert(CreateEntry(sCorrectCapsLock,   CBCOL_SECOND));
    m_pCheckLB->GetModel()->Insert(CreateEntry(sNum,               CBCOL_SECOND));
    m_pCheckLB->GetModel()->Insert(CreateEntry(sBorder,            CBCOL_SECOND));
    m_pCheckLB->GetModel()->Insert(CreateEntry(sTable,             CBCOL_SECOND));
    m_pCheckLB->GetModel()->Insert(CreateEntry(sReplaceTemplates,  CBCOL_SECOND));
    m_pCheckLB->GetModel()->Insert(CreateEntry(sDeleteEmptyPara,   CBCOL_FIRST ));
    m_pCheckLB->GetModel()->Insert(CreateEntry(sUserStyle,         CBCOL_FIRST ));
    m_pCheckLB->GetModel()->Insert(CreateEntry(sBullet,            CBCOL_FIRST ));
    m_pCheckLB->GetModel()->Insert(CreateEntry(sRightMargin,       CBCOL_FIRST ));

    m_pCheckLB->CheckEntryPos( USE_REPLACE_TABLE,  CBCOL_FIRST,    pOpt->bAutoCorrect );
    m_pCheckLB->CheckEntryPos( USE_REPLACE_TABLE,  CBCOL_SECOND,   0 != (nFlags & Autocorrect));
    m_pCheckLB->CheckEntryPos( CORR_UPPER,         CBCOL_FIRST,    pOpt->bCptlSttWrd );
    m_pCheckLB->CheckEntryPos( CORR_UPPER,         CBCOL_SECOND,   0 != (nFlags & CptlSttWrd) );
    m_pCheckLB->CheckEntryPos( BEGIN_UPPER,        CBCOL_FIRST,    pOpt->bCptlSttSntnc );
    m_pCheckLB->CheckEntryPos( BEGIN_UPPER,        CBCOL_SECOND,   0 != (nFlags & CptlSttSntnc) );
    m_pCheckLB->CheckEntryPos( BOLD_UNDERLINE,     CBCOL_FIRST,    pOpt->bChgWeightUnderl );
    m_pCheckLB->CheckEntryPos( BOLD_UNDERLINE,     CBCOL_SECOND,   0 != (nFlags & ChgWeightUnderl) );
    m_pCheckLB->CheckEntryPos( IGNORE_DBLSPACE,    CBCOL_SECOND,   0 != (nFlags & IgnoreDoubleSpace) );
    m_pCheckLB->CheckEntryPos( CORRECT_CAPS_LOCK,  CBCOL_SECOND,   0 != (nFlags & CorrectCapsLock) );
    m_pCheckLB->CheckEntryPos( DETECT_URL,         CBCOL_FIRST,    pOpt->bSetINetAttr );
    m_pCheckLB->CheckEntryPos( DETECT_URL,         CBCOL_SECOND,   0 != (nFlags & SetINetAttr) );
    m_pCheckLB->CheckEntryPos( REPLACE_DASHES,     CBCOL_FIRST,    pOpt->bChgToEnEmDash );
    m_pCheckLB->CheckEntryPos( REPLACE_DASHES,     CBCOL_SECOND,   0 != (nFlags & ChgToEnEmDash) );
    m_pCheckLB->CheckEntryPos( DEL_SPACES_AT_STT_END,      CBCOL_FIRST,    pOpt->bAFmtDelSpacesAtSttEnd );
    m_pCheckLB->CheckEntryPos( DEL_SPACES_AT_STT_END,      CBCOL_SECOND,   pOpt->bAFmtByInpDelSpacesAtSttEnd );
    m_pCheckLB->CheckEntryPos( DEL_SPACES_BETWEEN_LINES,   CBCOL_FIRST,    pOpt->bAFmtDelSpacesBetweenLines );
    m_pCheckLB->CheckEntryPos( DEL_SPACES_BETWEEN_LINES,   CBCOL_SECOND,   pOpt->bAFmtByInpDelSpacesBetweenLines );
    m_pCheckLB->CheckEntryPos( DEL_EMPTY_NODE,     CBCOL_FIRST,    pOpt->bDelEmptyNode );
    m_pCheckLB->CheckEntryPos( REPLACE_USER_COLL,  CBCOL_FIRST,    pOpt->bChgUserColl );
    m_pCheckLB->CheckEntryPos( REPLACE_BULLETS,    CBCOL_FIRST,    pOpt->bChgEnumNum );

    aBulletFont = pOpt->aBulletFont;
    sBulletChar = pOpt->cBullet;
    ImpUserData* pUserData = new ImpUserData(&sBulletChar, &aBulletFont);
    m_pCheckLB->SetUserData(  REPLACE_BULLETS, pUserData );

    nPercent = pOpt->nRightMargin;
    sMargin = " " + OUString::number( nPercent ) + "%";
    pUserData = new ImpUserData(&sMargin, 0);
    m_pCheckLB->SetUserData( MERGE_SINGLE_LINE_PARA, pUserData );

    m_pCheckLB->CheckEntryPos( APPLY_NUMBERING,    CBCOL_SECOND,   pOpt->bSetNumRule );

    aByInputBulletFont = pOpt->aByInputBulletFont;
    sByInputBulletChar = pOpt->cByInputBullet;
    ImpUserData* pUserData2 = new ImpUserData(&sByInputBulletChar, &aByInputBulletFont);
    m_pCheckLB->SetUserData( APPLY_NUMBERING , pUserData2 );

    m_pCheckLB->CheckEntryPos( MERGE_SINGLE_LINE_PARA, CBCOL_FIRST, pOpt->bRightMargin );
    m_pCheckLB->CheckEntryPos( INSERT_BORDER,      CBCOL_SECOND,   pOpt->bSetBorder );
    m_pCheckLB->CheckEntryPos( CREATE_TABLE,       CBCOL_SECOND,   pOpt->bCreateTable );
    m_pCheckLB->CheckEntryPos( REPLACE_STYLES,     CBCOL_SECOND,   pOpt->bReplaceStyles );

    m_pCheckLB->SetUpdateMode(sal_True);
}

IMPL_LINK(OfaSwAutoFmtOptionsPage, SelectHdl, OfaACorrCheckListBox*, pBox)
{
    m_pEditPB->Enable(0 != pBox->FirstSelected()->GetUserData());
    return 0;
}

IMPL_LINK_NOARG(OfaSwAutoFmtOptionsPage, EditHdl)
{
    sal_uLong nSelEntryPos = m_pCheckLB->GetSelectEntryPos();
    if( nSelEntryPos == REPLACE_BULLETS ||
        nSelEntryPos == APPLY_NUMBERING)
    {
        SvxCharacterMap *pMapDlg = new SvxCharacterMap(this);
        ImpUserData* pUserData = (ImpUserData*)m_pCheckLB->FirstSelected()->GetUserData();
        pMapDlg->SetCharFont(*pUserData->pFont);
        pMapDlg->SetChar( pUserData->pString->GetChar(0) );
        if(RET_OK == pMapDlg->Execute())
        {
            Font aFont(pMapDlg->GetCharFont());
            *pUserData->pFont = aFont;
            sal_UCS4 aChar = pMapDlg->GetChar();
            // using the UCS4 constructor
            OUString aOUStr( &aChar, 1 );
            *pUserData->pString = aOUStr;
        }
        delete pMapDlg;
    }
    else if( MERGE_SINGLE_LINE_PARA == nSelEntryPos )
    {
        // dialog for per cent settings
        OfaAutoFmtPrcntSet aDlg(this);
        aDlg.GetPrcntFld().SetValue(nPercent);
        if(RET_OK == aDlg.Execute())
        {
            nPercent = (sal_uInt16)aDlg.GetPrcntFld().GetValue();
            sMargin = " " + OUString::number( nPercent ) + "%";
        }
    }
    m_pCheckLB->Invalidate();
    return 0;
}

void OfaACorrCheckListBox::SetTabs()
{
    SvSimpleTable::SetTabs();
    sal_uInt16 nAdjust = SV_LBOXTAB_ADJUST_RIGHT|SV_LBOXTAB_ADJUST_LEFT|SV_LBOXTAB_ADJUST_CENTER|SV_LBOXTAB_ADJUST_NUMERIC|SV_LBOXTAB_FORCE;

    if( aTabs.size() > 1 )
    {
        SvLBoxTab* pTab = aTabs[1];
        pTab->nFlags &= ~nAdjust;
        pTab->nFlags |= SV_LBOXTAB_PUSHABLE|SV_LBOXTAB_ADJUST_CENTER|SV_LBOXTAB_FORCE;
    }
    if( aTabs.size() > 2 )
    {
        SvLBoxTab* pTab = aTabs[2];
        pTab->nFlags &= ~nAdjust;
        pTab->nFlags |= SV_LBOXTAB_PUSHABLE|SV_LBOXTAB_ADJUST_CENTER|SV_LBOXTAB_FORCE;
    }
}

void OfaACorrCheckListBox::CheckEntryPos(sal_uLong nPos, sal_uInt16 nCol, sal_Bool bChecked)
{
    if ( nPos < GetEntryCount() )
        SetCheckButtonState(
            GetEntry(nPos),
            nCol,
            bChecked ? SvButtonState( SV_BUTTON_CHECKED ) :
                                       SvButtonState( SV_BUTTON_UNCHECKED ) );
}

sal_Bool OfaACorrCheckListBox::IsChecked(sal_uLong nPos, sal_uInt16 nCol)
{
    return GetCheckButtonState( GetEntry(nPos), nCol ) == SV_BUTTON_CHECKED;
}

void OfaACorrCheckListBox::SetCheckButtonState( SvTreeListEntry* pEntry, sal_uInt16 nCol, SvButtonState eState)
{
    SvLBoxButton* pItem = (SvLBoxButton*)(pEntry->GetItem(nCol + 1));

    DBG_ASSERT(pItem,"SetCheckButton:Item not found");
    if (pItem->GetType() == SV_ITEM_ID_LBOXBUTTON)
    {
        switch( eState )
        {
            case SV_BUTTON_CHECKED:
                pItem->SetStateChecked();
                break;

            case SV_BUTTON_UNCHECKED:
                pItem->SetStateUnchecked();
                break;

            case SV_BUTTON_TRISTATE:
                pItem->SetStateTristate();
                break;
        }
        InvalidateEntry( pEntry );
    }
}

SvButtonState OfaACorrCheckListBox::GetCheckButtonState( SvTreeListEntry* pEntry, sal_uInt16 nCol ) const
{
    SvButtonState eState = SV_BUTTON_UNCHECKED;
    SvLBoxButton* pItem = (SvLBoxButton*)(pEntry->GetItem(nCol + 1));
    DBG_ASSERT(pItem,"GetChButnState:Item not found");

    if (pItem->GetType() == SV_ITEM_ID_LBOXBUTTON)
    {
        sal_uInt16 nButtonFlags = pItem->GetButtonFlags();
        eState = pCheckButtonData->ConvertToButtonState( nButtonFlags );
    }

    return eState;
}

void OfaACorrCheckListBox::HBarClick()
{
    // sorting is stopped by this overload
}

void    OfaACorrCheckListBox::KeyInput( const KeyEvent& rKEvt )
{
    if(!rKEvt.GetKeyCode().GetModifier() &&
        KEY_SPACE == rKEvt.GetKeyCode().GetCode())
    {
        sal_uLong nSelPos = GetSelectEntryPos();
        sal_uInt16 nCol = GetCurrentTabPos() - 1;
        if ( nCol < 2 )
        {
            CheckEntryPos( nSelPos, nCol, !IsChecked( nSelPos, nCol ) );
            CallImplEventListeners( VCLEVENT_CHECKBOX_TOGGLE, (void*)GetEntry( nSelPos ) );
        }
        else
        {
            sal_uInt16 nCheck = IsChecked(nSelPos, 1) ? 1 : 0;
            if(IsChecked(nSelPos, 0))
                nCheck += 2;
            nCheck--;
            nCheck &= 3;
            CheckEntryPos(nSelPos, 1, 0 != (nCheck & 1));
            CheckEntryPos(nSelPos, 0, 0 != (nCheck & 2));
        }
    }
    else
        SvSimpleTable::KeyInput(rKEvt);
}

OfaAutocorrReplacePage::OfaAutocorrReplacePage( Window* pParent,
                                                const SfxItemSet& rSet )
    : SfxTabPage(pParent, "AcorReplacePage", "cui/ui/acorreplacepage.ui", rSet)
    , eLang(eLastDialogLanguage)
    , bHasSelectionText(false)
    , bFirstSelect(true)
    , bReplaceEditChanged(false)
    , bSWriter(true)
{
    get(m_pTextOnlyCB, "textonly");
    get(m_pDeleteReplacePB, "delete");
    get(m_pNewReplacePB, "new");
    sNew = m_pNewReplacePB->GetText();
    sModify = get<PushButton>("replace")->GetText();
    get(m_pShortED, "origtext");
    get(m_pReplaceED, "newtext");
    get(m_pReplaceTLB, "tabview");
    m_pReplaceTLB->set_height_request(16 * GetTextHeight());

    SfxModule *pMod = *(SfxModule**)GetAppData(SHL_WRITER);
    bSWriter = pMod == SfxModule::GetActiveModule();

    LanguageTag aLanguageTag( eLastDialogLanguage );
    pCompareClass = new CollatorWrapper( comphelper::getProcessComponentContext() );
    pCompareClass->loadDefaultCollator( aLanguageTag.getLocale(), 0 );
    pCharClass = new CharClass( aLanguageTag );

    static long aTabs[] = { 2 /* Tab-Count */, 1, 61 };
    m_pReplaceTLB->SetTabs( &aTabs[0], MAP_APPFONT );

    m_pReplaceTLB->SetStyle( m_pReplaceTLB->GetStyle()|WB_HSCROLL|WB_CLIPCHILDREN );
    m_pReplaceTLB->SetSelectHdl( LINK(this, OfaAutocorrReplacePage, SelectHdl) );
    m_pNewReplacePB->SetClickHdl( LINK(this, OfaAutocorrReplacePage, NewDelHdl) );
    m_pDeleteReplacePB->SetClickHdl( LINK(this, OfaAutocorrReplacePage, NewDelHdl) );
    m_pShortED->SetModifyHdl( LINK(this, OfaAutocorrReplacePage, ModifyHdl) );
    m_pReplaceED->SetModifyHdl( LINK(this, OfaAutocorrReplacePage, ModifyHdl) );
    m_pShortED->SetActionHdl( LINK(this, OfaAutocorrReplacePage, NewDelHdl) );
    m_pReplaceED->SetActionHdl( LINK(this, OfaAutocorrReplacePage, NewDelHdl) );

    m_pReplaceED->SetSpaces(true);
    m_pShortED->SetSpaces(true);
}

void OfaAutocorrReplacePage::Resize()
{
    SfxTabPage::Resize();
    m_pReplaceTLB->SetTab(0, m_pShortED->GetPosPixel().X(), MAP_PIXEL);
    m_pReplaceTLB->SetTab(1, m_pReplaceED->GetPosPixel().X(), MAP_PIXEL);
}

OfaAutocorrReplacePage::~OfaAutocorrReplacePage()
{
    aDoubleStringTable.clear();
    aChangesTable.clear();

    delete pCompareClass;
    delete pCharClass;
}

SfxTabPage* OfaAutocorrReplacePage::Create( Window* pParent, const SfxItemSet& rSet)
{
    return new OfaAutocorrReplacePage(pParent, rSet);
}

void OfaAutocorrReplacePage::ActivatePage( const SfxItemSet& )
{
    if(eLang != eLastDialogLanguage)
        SetLanguage(eLastDialogLanguage);
    ((OfaAutoCorrDlg*)GetTabDialog())->EnableLanguage(sal_True);
}

int OfaAutocorrReplacePage::DeactivatePage( SfxItemSet*  )
{
    return LEAVE_PAGE;
}

sal_Bool OfaAutocorrReplacePage::FillItemSet( SfxItemSet& )
{
    SvxAutoCorrect* pAutoCorrect = SvxAutoCorrCfg::Get().GetAutoCorrect();

    for (StringChangeTable::reverse_iterator it = aChangesTable.rbegin(); it != aChangesTable.rend(); ++it)
    {
        LanguageType eCurrentLang = it->first;
        StringChangeList& rStringChangeList = it->second;
        std::vector<SvxAutocorrWord> aDeleteWords;
        std::vector<SvxAutocorrWord> aNewWords;

        for (sal_uInt32 i = 0; i < rStringChangeList.aDeletedEntries.size(); i++)
        {
            DoubleString& deleteEntry = rStringChangeList.aDeletedEntries[i];
            SvxAutocorrWord aDeleteWord( deleteEntry.sShort, deleteEntry.sLong );
            aDeleteWords.push_back( aDeleteWord );
        }

        for (sal_uInt32 i = 0; i < rStringChangeList.aNewEntries.size(); i++)
        {
            DoubleString& newEntry = rStringChangeList.aNewEntries[i];
            SvxAutocorrWord aNewWord( newEntry.sShort, newEntry.sLong );
            aNewWords.push_back( aNewWord );
        }
        pAutoCorrect->MakeCombinedChanges( aNewWords, aDeleteWords, eCurrentLang );
    }
    aChangesTable.clear();
    return sal_False;
}

void OfaAutocorrReplacePage::RefillReplaceBox(sal_Bool bFromReset,
                                        LanguageType eOldLanguage,
                                        LanguageType eNewLanguage)
{
    eLang = eNewLanguage;
    if(bFromReset)
    {
        aDoubleStringTable.clear();
        aChangesTable.clear();
    }
    else
    {
        DoubleStringArray* pArray;
        if(aDoubleStringTable.find(eOldLanguage) != aDoubleStringTable.end())
        {
            pArray = &aDoubleStringTable[eOldLanguage];
            pArray->clear();
        }
        else
        {
            pArray = &aDoubleStringTable[eOldLanguage]; // create new array
        }

        sal_uInt32 nListBoxCount = (sal_uInt32) m_pReplaceTLB->GetEntryCount();
        sal_uInt32 i;
        for(i = 0; i < nListBoxCount; i++)
        {
            pArray->push_back(DoubleString());
            DoubleString& rDouble = (*pArray)[pArray->size() - 1];
            SvTreeListEntry*  pEntry = m_pReplaceTLB->GetEntry( i );
            rDouble.sShort = m_pReplaceTLB->GetEntryText(pEntry, 0);
            rDouble.sLong = m_pReplaceTLB->GetEntryText(pEntry, 1);
            rDouble.pUserData = pEntry->GetUserData();
        }
    }

    m_pReplaceTLB->Clear();
    if( !bSWriter )
        aFormatText.clear();

    if( aDoubleStringTable.find(eLang) != aDoubleStringTable.end() )
    {
        DoubleStringArray& rArray = aDoubleStringTable[eNewLanguage];
        for( sal_uInt32 i = 0; i < rArray.size(); i++ )
        {
            DoubleString& rDouble = rArray[i];
            sal_Bool bTextOnly = 0 == rDouble.pUserData;
            // formatted text is only in Writer
            if(bSWriter || bTextOnly)
            {
                String sEntry(rDouble.sShort);
                sEntry += '\t';
                sEntry += rDouble.sLong;
                SvTreeListEntry* pEntry = m_pReplaceTLB->InsertEntry(sEntry);
                m_pTextOnlyCB->Check(bTextOnly);
                if(!bTextOnly)
                    pEntry->SetUserData(rDouble.pUserData); // that means: with format info or even with selection text
            }
            else
            {
                aFormatText.insert(rDouble.sShort);
            }
        }
    }
    else
    {
        SvxAutoCorrect* pAutoCorrect = SvxAutoCorrCfg::Get().GetAutoCorrect();
        SvxAutocorrWordList* pWordList = pAutoCorrect->LoadAutocorrWordList(eLang);
        m_pReplaceTLB->SetUpdateMode(sal_False);
        SvxAutocorrWordList::Content aContent = pWordList->getSortedContent();
        for( SvxAutocorrWordList::Content::const_iterator it = aContent.begin();
             it != aContent.end(); ++it )
        {
            SvxAutocorrWord* pWordPtr = *it;
            sal_Bool bTextOnly = pWordPtr->IsTextOnly();
            // formatted text is only in Writer
            if(bSWriter || bTextOnly)
            {
                String sEntry(pWordPtr->GetShort());
                sEntry += '\t';
                sEntry += pWordPtr->GetLong();
                SvTreeListEntry* pEntry = m_pReplaceTLB->InsertEntry(sEntry);
                m_pTextOnlyCB->Check(pWordPtr->IsTextOnly());
                if(!bTextOnly)
                    pEntry->SetUserData(m_pTextOnlyCB); // that means: with format info
            }
            else
            {
                aFormatText.insert(pWordPtr->GetShort());
            }
        }
        m_pNewReplacePB->Enable(sal_False);
        m_pDeleteReplacePB->Enable(sal_False);
        m_pReplaceTLB->SetUpdateMode(sal_True);
    }

    SfxViewShell* pViewShell = SfxViewShell::Current();
    if( pViewShell && pViewShell->HasSelection( sal_True ) )
    {
        bHasSelectionText = sal_True;
        const OUString sSelection( pViewShell->GetSelectionText() );
        m_pReplaceED->SetText( sSelection );
        m_pTextOnlyCB->Check( !bSWriter );
        m_pTextOnlyCB->Enable( bSWriter && !sSelection.isEmpty() );
    }
    else
    {
        m_pTextOnlyCB->Enable( sal_False );
    }
}

void OfaAutocorrReplacePage::Reset( const SfxItemSet& )
{
    RefillReplaceBox(sal_True, eLang, eLang);
    m_pShortED->GrabFocus();
}

void OfaAutocorrReplacePage::SetLanguage(LanguageType eSet)
{
    //save old settings an refill
    if(eSet != eLang)
    {
        RefillReplaceBox(sal_False, eLang, eSet);
        eLastDialogLanguage = eSet;
        delete pCompareClass;
        delete pCharClass;

        LanguageTag aLanguageTag( eLastDialogLanguage );
        pCompareClass = new CollatorWrapper( comphelper::getProcessComponentContext() );
        pCompareClass->loadDefaultCollator( aLanguageTag.getLocale(), 0 );
        pCharClass = new CharClass( aLanguageTag );
        ModifyHdl(m_pShortED);
    }
}

IMPL_LINK(OfaAutocorrReplacePage, SelectHdl, SvTabListBox*, pBox)
{
    if(!bFirstSelect || !bHasSelectionText)
    {
        SvTreeListEntry* pEntry = pBox->FirstSelected();
        OUString sTmpShort(pBox->GetEntryText(pEntry, 0));
        // if the text is set via ModifyHdl, the cursor is always at the beginning
        // of a word, although you're editing here
        sal_Bool bSameContent = 0 == pCompareClass->compareString( sTmpShort, m_pShortED->GetText() );
        Selection aSel = m_pShortED->GetSelection();
        if(m_pShortED->GetText() != sTmpShort)
        {
            m_pShortED->SetText(sTmpShort);
            // if it was only a different notation, the selection has to be set again
            if(bSameContent)
            {
                m_pShortED->SetSelection(aSel);
            }
        }
        m_pReplaceED->SetText( pBox->GetEntryText(pEntry, 1) );
        // with UserData there is a Formatinfo
        m_pTextOnlyCB->Check( pEntry->GetUserData() == 0);
    }
    else
    {
        bFirstSelect = sal_False;
    }

    m_pNewReplacePB->Enable(sal_False);
    m_pDeleteReplacePB->Enable();
    return 0;
};

void OfaAutocorrReplacePage::NewEntry(String sShort, String sLong)
{
    DoubleStringArray& rNewArray = aChangesTable[eLang].aNewEntries;
    for (sal_uInt32 i = 0; i < rNewArray.size(); i++)
    {
        if (rNewArray[i].sShort == sShort)
        {
            rNewArray.erase(rNewArray.begin() + i);
            break;
        }
    }

    DoubleStringArray& rDeletedArray = aChangesTable[eLang].aDeletedEntries;
    for (sal_uInt32 i = 0; i < rDeletedArray.size(); i++)
    {
        if (rDeletedArray[i].sShort == sShort)
        {
            rDeletedArray.erase(rDeletedArray.begin() + i);
            break;
        }
    }

    DoubleString aNewString = DoubleString();
    aNewString.sShort = sShort;
    aNewString.sLong = sLong;
    rNewArray.push_back(aNewString);
}

void OfaAutocorrReplacePage::DeleteEntry(String sShort, String sLong)
{
    DoubleStringArray& rNewArray = aChangesTable[eLang].aNewEntries;
    for (sal_uInt32 i = 0; i < rNewArray.size(); i++)
    {
        if (rNewArray[i].sShort == sShort)
        {
            rNewArray.erase(rNewArray.begin() + i);
            break;
        }
    }

    DoubleStringArray& rDeletedArray = aChangesTable[eLang].aDeletedEntries;
    for (sal_uInt32 i = 0; i < rDeletedArray.size(); i++)
    {
        if (rDeletedArray[i].sShort == sShort)
        {
            rDeletedArray.erase(rDeletedArray.begin() + i);
            break;
        }
    }

    DoubleString aDeletedString = DoubleString();
    aDeletedString.sShort = sShort;
    aDeletedString.sLong = sLong;
    rDeletedArray.push_back(aDeletedString);
}

IMPL_LINK(OfaAutocorrReplacePage, NewDelHdl, PushButton*, pBtn)
{
    SvTreeListEntry* pEntry = m_pReplaceTLB->FirstSelected();
    if( pBtn == m_pDeleteReplacePB )
    {
        DBG_ASSERT( pEntry, "no entry selected" );
        if( pEntry )
        {
            DeleteEntry(m_pReplaceTLB->GetEntryText(pEntry, 0), m_pReplaceTLB->GetEntryText(pEntry, 1));
            m_pReplaceTLB->GetModel()->Remove(pEntry);
            ModifyHdl(m_pShortED);
            return 0;
        }
    }
    if(pBtn == m_pNewReplacePB || m_pNewReplacePB->IsEnabled())
    {
        SvTreeListEntry* _pNewEntry = m_pReplaceTLB->FirstSelected();
        String sEntry(m_pShortED->GetText());
        if(sEntry.Len() && ( !m_pReplaceED->GetText().isEmpty() ||
                ( bHasSelectionText && bSWriter ) ))
        {
            NewEntry(m_pShortED->GetText(), m_pReplaceED->GetText());
            m_pReplaceTLB->SetUpdateMode(sal_False);
            sal_uInt32 nPos = UINT_MAX;
            sEntry += '\t';
            sEntry += m_pReplaceED->GetText();
            if(_pNewEntry)
            {
                nPos = (sal_uInt32) m_pReplaceTLB->GetModel()->GetAbsPos(_pNewEntry);
                m_pReplaceTLB->GetModel()->Remove(_pNewEntry);
            }
            else
            {
                sal_uInt32 j;
                for( j = 0; j < m_pReplaceTLB->GetEntryCount(); j++ )
                {
                    SvTreeListEntry* pReplaceEntry = m_pReplaceTLB->GetEntry(j);
                    if( 0 >=  pCompareClass->compareString(sEntry, m_pReplaceTLB->GetEntryText(pReplaceEntry, 0) ) )
                        break;
                }
                nPos = j;
            }
            SvTreeListEntry* pInsEntry = m_pReplaceTLB->InsertEntry(
                                        sEntry, static_cast< SvTreeListEntry * >(NULL), false,
                                        nPos == UINT_MAX ? LIST_APPEND : nPos);
            if( !bReplaceEditChanged && !m_pTextOnlyCB->IsChecked())
            {
                pInsEntry->SetUserData(&bHasSelectionText); // new formatted text
            }

            m_pReplaceTLB->MakeVisible( pInsEntry );
            m_pReplaceTLB->SetUpdateMode( sal_True );
            // if the request came from the ReplaceEdit, give focus to the ShortEdit
            if(m_pReplaceED->HasFocus())
            {
                m_pShortED->GrabFocus();
            }
        }
    }
    else
    {
        // this can only be an enter in one of the two edit fields
        // which means EndDialog() - has to be evaluated in KeyInput
        return 0;
    }
    ModifyHdl(m_pShortED);
    return 1;
}

IMPL_LINK(OfaAutocorrReplacePage, ModifyHdl, Edit*, pEdt)
{
    SvTreeListEntry* pFirstSel = m_pReplaceTLB->FirstSelected();
    sal_Bool bShort = pEdt == m_pShortED;
    const String rEntry = pEdt->GetText();
    const String rRepString = m_pReplaceED->GetText();
    String aWordStr( pCharClass->lowercase( rEntry ));

    if(bShort)
    {
        if(rEntry.Len())
        {
            sal_Bool bFound = sal_False;
            sal_Bool bTmpSelEntry=sal_False;

            for(sal_uInt32 i = 0; i < m_pReplaceTLB->GetEntryCount(); i++)
            {
                SvTreeListEntry*  pEntry = m_pReplaceTLB->GetEntry( i );
                String aTestStr=m_pReplaceTLB->GetEntryText(pEntry, 0);
                if( pCompareClass->compareString(rEntry, aTestStr ) == 0 )
                {
                    if( rRepString.Len() )
                    {
                        bFirstSelect = sal_True;
                    }
                    m_pReplaceTLB->SetCurEntry(pEntry);
                    pFirstSel = pEntry;
                    m_pNewReplacePB->SetText(sModify);
                    bFound = sal_True;
                    break;
                }
                else
                {
                    aTestStr = pCharClass->lowercase( aTestStr );
                    if( aTestStr.Search(aWordStr) == 0 && !bTmpSelEntry )
                    {
                        m_pReplaceTLB->MakeVisible( pEntry );
                        bTmpSelEntry = sal_True;
                    }
                }
            }
            if( !bFound )
            {
                m_pReplaceTLB->SelectAll( sal_False );
                pFirstSel = 0;
                m_pNewReplacePB->SetText( sNew );
                if( bReplaceEditChanged )
                    m_pTextOnlyCB->Enable(sal_False);
            }
            m_pDeleteReplacePB->Enable( bFound );
        }
        else if( m_pReplaceTLB->GetEntryCount() > 0 )
        {
            SvTreeListEntry*  pEntry = m_pReplaceTLB->GetEntry( 0 );
            m_pReplaceTLB->MakeVisible( pEntry );
        }

    }
    else if( !bShort )
    {
        bReplaceEditChanged = sal_True;
        if( pFirstSel )
        {
            m_pNewReplacePB->SetText( sModify );
        }
    }

    const String& rShortTxt = m_pShortED->GetText();
    sal_Bool bEnableNew = rShortTxt.Len() &&
                        ( rRepString.Len() ||
                                ( bHasSelectionText && bSWriter )) &&
                        ( !pFirstSel || rRepString !=
                                m_pReplaceTLB->GetEntryText( pFirstSel, 1 ) );
    if( bEnableNew )
    {
        for(std::set<OUString>::iterator i = aFormatText.begin(); i != aFormatText.end(); ++i)
        {
            if((*i).equals(rShortTxt))
            {
                bEnableNew = sal_False;
                break;
            }
        }
    }
    m_pNewReplacePB->Enable( bEnableNew );

    return 0;
}

static sal_Bool lcl_FindInArray(std::vector<OUString>& rStrings, const String& rString)
{
    for(std::vector<OUString>::iterator i = rStrings.begin(); i != rStrings.end(); ++i)
    {
        if((*i).equals(rString))
        {
            return sal_True;
        }
    }
    return sal_False;
}

OfaAutocorrExceptPage::OfaAutocorrExceptPage(Window* pParent, const SfxItemSet& rSet)
    : SfxTabPage(pParent, "AcorExceptPage", "cui/ui/acorexceptpage.ui", rSet)
    , eLang(eLastDialogLanguage)
{
    get(m_pAbbrevED, "abbrev");
    get(m_pAbbrevLB, "abbrevlist");
    m_pAbbrevLB->SetStyle(m_pAbbrevLB->GetStyle() | WB_SORT);
    m_pAbbrevLB->SetDropDownLineCount(7);
    get(m_pNewAbbrevPB, "newabbrev");
    get(m_pDelAbbrevPB, "delabbrev");
    get(m_pAutoAbbrevCB, "autoabbrev");

    get(m_pDoubleCapsED, "double");
    get(m_pDoubleCapsLB, "doublelist");
    m_pDoubleCapsLB->SetStyle(m_pDoubleCapsLB->GetStyle() | WB_SORT);
    m_pDoubleCapsLB->SetDropDownLineCount(7);
    get(m_pNewDoublePB, "newdouble");
    get(m_pDelDoublePB, "deldouble");
    get(m_pAutoCapsCB, "autodouble");

    ::com::sun::star::lang::Locale aLcl( LanguageTag::convertToLocale(eLastDialogLanguage ));
    pCompareClass = new CollatorWrapper( comphelper::getProcessComponentContext() );
    pCompareClass->loadDefaultCollator( aLcl, 0 );

    m_pNewAbbrevPB->SetClickHdl(LINK(this, OfaAutocorrExceptPage, NewDelHdl));
    m_pDelAbbrevPB->SetClickHdl(LINK(this, OfaAutocorrExceptPage, NewDelHdl));
    m_pNewDoublePB->SetClickHdl(LINK(this, OfaAutocorrExceptPage, NewDelHdl));
    m_pDelDoublePB->SetClickHdl(LINK(this, OfaAutocorrExceptPage, NewDelHdl));

    m_pAbbrevLB->SetSelectHdl(LINK(this, OfaAutocorrExceptPage, SelectHdl));
    m_pDoubleCapsLB->SetSelectHdl(LINK(this, OfaAutocorrExceptPage, SelectHdl));
    m_pAbbrevED->SetModifyHdl(LINK(this, OfaAutocorrExceptPage, ModifyHdl));
    m_pDoubleCapsED->SetModifyHdl(LINK(this, OfaAutocorrExceptPage, ModifyHdl));

    m_pAbbrevED->SetActionHdl(LINK(this, OfaAutocorrExceptPage, NewDelHdl));
    m_pDoubleCapsED->SetActionHdl(LINK(this, OfaAutocorrExceptPage, NewDelHdl));
}

OfaAutocorrExceptPage::~OfaAutocorrExceptPage()
{
    aStringsTable.clear();
    delete pCompareClass;
}

SfxTabPage* OfaAutocorrExceptPage::Create( Window* pParent,
                                const SfxItemSet& rSet)
{
    return new OfaAutocorrExceptPage(pParent, rSet);
}

void    OfaAutocorrExceptPage::ActivatePage( const SfxItemSet& )
{
    if(eLang != eLastDialogLanguage)
        SetLanguage(eLastDialogLanguage);
    ((OfaAutoCorrDlg*)GetTabDialog())->EnableLanguage(sal_True);
}

int     OfaAutocorrExceptPage::DeactivatePage( SfxItemSet* )
{
    return LEAVE_PAGE;
}

sal_Bool OfaAutocorrExceptPage::FillItemSet( SfxItemSet&  )
{
    SvxAutoCorrect* pAutoCorrect = SvxAutoCorrCfg::Get().GetAutoCorrect();
    for(StringsTable::reverse_iterator it1 = aStringsTable.rbegin(); it1 != aStringsTable.rend(); ++it1)
    {
        LanguageType eCurLang = it1->first;
        StringsArrays& rArrays = it1->second;
        if(eCurLang != eLang) // current language is treated later
        {
            SvStringsISortDtor* pWrdList = pAutoCorrect->LoadWrdSttExceptList(eCurLang);

            if(pWrdList)
            {
                sal_uInt16 nCount = pWrdList->size();
                sal_uInt16 i;
                for( i = nCount; i; )
                {
                    OUString aString = (*pWrdList)[ --i ];

                    if( !lcl_FindInArray(rArrays.aDoubleCapsStrings, aString))
                    {
                      pWrdList->erase(i);
                    }
                }

                for(std::vector<OUString>::iterator it = rArrays.aDoubleCapsStrings.begin(); it != rArrays.aDoubleCapsStrings.end(); ++it)
                {
                    pWrdList->insert(*it);
                }
                pAutoCorrect->SaveWrdSttExceptList(eCurLang);
            }

            SvStringsISortDtor* pCplList = pAutoCorrect->LoadCplSttExceptList(eCurLang);

            if(pCplList)
            {
                sal_uInt16 nCount = pCplList->size();
                sal_uInt16 i;
                for( i = nCount; i; )
                {
                    OUString aString = (*pCplList)[ --i ];
                    if( !lcl_FindInArray(rArrays.aAbbrevStrings, aString))
                    {
                        pCplList->erase(i);
                    }
                }

                for(std::vector<OUString>::iterator it = rArrays.aAbbrevStrings.begin(); it != rArrays.aAbbrevStrings.end(); ++it)
                {
                    pCplList->insert(*it);
                }

                pAutoCorrect->SaveCplSttExceptList(eCurLang);
            }
        }
    }
    aStringsTable.clear();

    SvStringsISortDtor* pWrdList = pAutoCorrect->LoadWrdSttExceptList(eLang);

    if(pWrdList)
    {
        sal_uInt16 nCount = pWrdList->size();
        sal_uInt16 i;
        for( i = nCount; i; )
        {
            OUString aString = (*pWrdList)[ --i ];
            if( USHRT_MAX == m_pDoubleCapsLB->GetEntryPos(aString) )
            {
                pWrdList->erase(i);
            }
        }
        nCount = m_pDoubleCapsLB->GetEntryCount();
        for( i = 0; i < nCount; ++i )
        {
            pWrdList->insert( m_pDoubleCapsLB->GetEntry( i ) );
        }
        pAutoCorrect->SaveWrdSttExceptList(eLang);
    }

    SvStringsISortDtor* pCplList = pAutoCorrect->LoadCplSttExceptList(eLang);

    if(pCplList)
    {
        sal_uInt16 nCount = pCplList->size();
        sal_uInt16 i;
        for( i = nCount; i; )
        {
            OUString aString = (*pCplList)[ --i ];
            if( USHRT_MAX == m_pAbbrevLB->GetEntryPos(aString) )
            {
                pCplList->erase(i);
            }
        }
        nCount = m_pAbbrevLB->GetEntryCount();
        for( i = 0; i < nCount; ++i )
        {
            pCplList->insert( m_pAbbrevLB->GetEntry( i ) );
        }
        pAutoCorrect->SaveCplSttExceptList(eLang);
    }
    if(m_pAutoAbbrevCB->IsChecked() != m_pAutoAbbrevCB->GetSavedValue())
        pAutoCorrect->SetAutoCorrFlag( SaveWordCplSttLst, m_pAutoAbbrevCB->IsChecked());
    if(m_pAutoCapsCB->IsChecked() != m_pAutoCapsCB->GetSavedValue())
        pAutoCorrect->SetAutoCorrFlag( SaveWordWrdSttLst, m_pAutoCapsCB->IsChecked());
    return sal_False;
}

void OfaAutocorrExceptPage::SetLanguage(LanguageType eSet)
{
    if(eLang != eSet)
    {
        // save old settings and fill anew
        RefillReplaceBoxes(sal_False, eLang, eSet);
        eLastDialogLanguage = eSet;
        delete pCompareClass;
        pCompareClass = new CollatorWrapper( comphelper::getProcessComponentContext() );
        pCompareClass->loadDefaultCollator( LanguageTag::convertToLocale( eLastDialogLanguage ), 0 );
        ModifyHdl(m_pAbbrevED);
        ModifyHdl(m_pDoubleCapsED);
    }
}

void OfaAutocorrExceptPage::RefillReplaceBoxes(sal_Bool bFromReset,
                                        LanguageType eOldLanguage,
                                        LanguageType eNewLanguage)
{
    eLang = eNewLanguage;
    if(bFromReset)
    {
        aStringsTable.clear();
    }
    else
    {
        StringsArrays* pArrays;
        if(aStringsTable.find(eOldLanguage) != aStringsTable.end())
        {
            pArrays = &aStringsTable[eOldLanguage];
            pArrays->aAbbrevStrings.clear();
            pArrays->aDoubleCapsStrings.clear();
        }
        else
        {
            pArrays = &aStringsTable[eOldLanguage]; // create new array
        }

        sal_uInt16 i;
        for(i = 0; i < m_pAbbrevLB->GetEntryCount(); i++)
            pArrays->aAbbrevStrings.push_back(OUString(m_pAbbrevLB->GetEntry(i)));

        for(i = 0; i < m_pDoubleCapsLB->GetEntryCount(); i++)
            pArrays->aDoubleCapsStrings.push_back(OUString(m_pDoubleCapsLB->GetEntry(i)));
    }
    m_pDoubleCapsLB->Clear();
    m_pAbbrevLB->Clear();
    String sTemp;
    m_pAbbrevED->SetText(sTemp);
    m_pDoubleCapsED->SetText(sTemp);

    if(aStringsTable.find(eLang) != aStringsTable.end())
    {
        StringsArrays& rArrays = aStringsTable[eLang];
        for(std::vector<OUString>::iterator i = rArrays.aAbbrevStrings.begin(); i != rArrays.aAbbrevStrings.end(); ++i)
            m_pAbbrevLB->InsertEntry(*i);

        for(std::vector<OUString>::iterator i = rArrays.aDoubleCapsStrings.begin(); i != rArrays.aDoubleCapsStrings.end(); ++i)
            m_pDoubleCapsLB->InsertEntry(*i);
    }
    else
    {
        SvxAutoCorrect* pAutoCorrect = SvxAutoCorrCfg::Get().GetAutoCorrect();
        const SvStringsISortDtor* pCplList = pAutoCorrect->GetCplSttExceptList(eLang);
        const SvStringsISortDtor* pWrdList = pAutoCorrect->GetWrdSttExceptList(eLang);
        sal_uInt16 i;
        for( i = 0; i < pCplList->size(); i++ )
        {
            m_pAbbrevLB->InsertEntry((*pCplList)[i]);
        }
        for( i = 0; i < pWrdList->size(); i++ )
        {
            m_pDoubleCapsLB->InsertEntry((*pWrdList)[i]);
        }
    }
}

void OfaAutocorrExceptPage::Reset( const SfxItemSet& )
{
    SvxAutoCorrect* pAutoCorrect = SvxAutoCorrCfg::Get().GetAutoCorrect();
    RefillReplaceBoxes(sal_True, eLang, eLang);
    m_pAutoAbbrevCB->  Check(  pAutoCorrect->IsAutoCorrFlag( SaveWordCplSttLst ));
    m_pAutoCapsCB->    Check(  pAutoCorrect->IsAutoCorrFlag( SaveWordWrdSttLst ));
    m_pAutoAbbrevCB->SaveValue();
    m_pAutoCapsCB->SaveValue();
}

IMPL_LINK(OfaAutocorrExceptPage, NewDelHdl, PushButton*, pBtn)
{
    if((pBtn == m_pNewAbbrevPB || pBtn == (PushButton*)m_pAbbrevED )
        && !m_pAbbrevED->GetText().isEmpty())
    {
        m_pAbbrevLB->InsertEntry(m_pAbbrevED->GetText());
        ModifyHdl(m_pAbbrevED);
    }
    else if(pBtn == m_pDelAbbrevPB)
    {
        m_pAbbrevLB->RemoveEntry(m_pAbbrevED->GetText());
        ModifyHdl(m_pAbbrevED);
    }
    else if((pBtn == m_pNewDoublePB || pBtn == (PushButton*)m_pDoubleCapsED )
            && !m_pDoubleCapsED->GetText().isEmpty())
    {
        m_pDoubleCapsLB->InsertEntry(m_pDoubleCapsED->GetText());
        ModifyHdl(m_pDoubleCapsED);
    }
    else if(pBtn == m_pDelDoublePB)
    {
        m_pDoubleCapsLB->RemoveEntry(m_pDoubleCapsED->GetText());
        ModifyHdl(m_pDoubleCapsED);
    }
    return 0;
}

IMPL_LINK(OfaAutocorrExceptPage, SelectHdl, ListBox*, pBox)
{
    if (pBox == m_pAbbrevLB)
    {
        m_pAbbrevED->SetText(pBox->GetSelectEntry());
        m_pNewAbbrevPB->Enable(sal_False);
        m_pDelAbbrevPB->Enable();
    }
    else
    {
        m_pDoubleCapsED->SetText(pBox->GetSelectEntry());
        m_pNewDoublePB->Enable(sal_False);
        m_pDelDoublePB->Enable();
    }
    return 0;
}

IMPL_LINK(OfaAutocorrExceptPage, ModifyHdl, Edit*, pEdt)
{
//  sal_Bool bSame = pEdt->GetText() == ->GetSelectEntry();
    const String& sEntry = pEdt->GetText();
    sal_Bool bEntryLen = 0!= sEntry.Len();
    if(pEdt == m_pAbbrevED)
    {
        sal_Bool bSame = lcl_FindEntry(*m_pAbbrevLB, sEntry, *pCompareClass);
        if(bSame && sEntry != m_pAbbrevLB->GetSelectEntry())
            pEdt->SetText(m_pAbbrevLB->GetSelectEntry());
        m_pNewAbbrevPB->Enable(!bSame && bEntryLen);
        m_pDelAbbrevPB->Enable(bSame && bEntryLen);
    }
    else
    {
        sal_Bool bSame = lcl_FindEntry(*m_pDoubleCapsLB, sEntry, *pCompareClass);
        if(bSame && sEntry != m_pDoubleCapsLB->GetSelectEntry())
            pEdt->SetText(m_pDoubleCapsLB->GetSelectEntry());
        m_pNewDoublePB->Enable(!bSame && bEntryLen);
        m_pDelDoublePB->Enable(bSame && bEntryLen);
    }
    return 0;
}

extern "C" SAL_DLLPUBLIC_EXPORT Window* SAL_CALL makeAutoCorrEdit(Window *pParent,
    VclBuilder::stringmap &)
{
    return new AutoCorrEdit(pParent);
}

void AutoCorrEdit::KeyInput( const KeyEvent& rKEvt )
{
    const KeyCode aKeyCode = rKEvt.GetKeyCode();
    const sal_uInt16 nModifier = aKeyCode.GetModifier();
    if( aKeyCode.GetCode() == KEY_RETURN )
    {
        // if there's nothing done on enter, call the
        // base class after all to close the dialog
        if(!nModifier && !aActionLink.Call(this))
                 Edit::KeyInput(rKEvt);
    }
    else if(bSpaces || aKeyCode.GetCode() != KEY_SPACE)
        Edit::KeyInput(rKEvt);
}

enum OfaQuoteOptions
{
    ADD_NONBRK_SPACE,
    REPLACE_1ST
};

SvTreeListEntry* OfaQuoteTabPage::CreateEntry(String& rTxt, sal_uInt16 nCol)
{
    SvTreeListEntry* pEntry = new SvTreeListEntry;

    if ( !pCheckButtonData )
    {
        pCheckButtonData = new SvLBoxButtonData(m_pSwCheckLB);
        m_pSwCheckLB->SetCheckButtonData(pCheckButtonData);
    }

    pEntry->AddItem( new SvLBoxContextBmp( pEntry, 0, Image(), Image(), 0));

    String sEmpty;
    if (nCol == CBCOL_SECOND)
        pEntry->AddItem( new SvLBoxString( pEntry, 0, sEmpty) );
    else
        pEntry->AddItem( new SvLBoxButton( pEntry, SvLBoxButtonKind_enabledCheckbox, 0, pCheckButtonData ) );

    if (nCol == CBCOL_FIRST)
        pEntry->AddItem( new SvLBoxString( pEntry, 0, sEmpty) );
    else
        pEntry->AddItem( new SvLBoxButton( pEntry, SvLBoxButtonKind_enabledCheckbox, 0, pCheckButtonData ) );

    pEntry->AddItem( new OfaImpBrwString( pEntry, 0, rTxt ) );

    return pEntry;
}

OfaQuoteTabPage::OfaQuoteTabPage(Window* pParent, const SfxItemSet& rSet)
    : SfxTabPage(pParent, "ApplyLocalizedPage", "cui/ui/applylocalizedpage.ui", rSet)
    , sNonBrkSpace(CUI_RESSTR(RID_SVXSTR_NON_BREAK_SPACE))
    , sOrdinal(CUI_RESSTR(RID_SVXSTR_ORDINAL))
    , pCheckButtonData(NULL)
{
    get(m_pCheckLB, "checklist");

    SvSimpleTableContainer *pListContainer = get<SvSimpleTableContainer>("list");
    Size aControlSize(252 , 85);
    aControlSize = LogicToPixel(aControlSize, MAP_APPFONT);
    pListContainer->set_width_request(aControlSize.Width());
    pListContainer->set_height_request(aControlSize.Height());
    m_pSwCheckLB = new OfaACorrCheckListBox(*pListContainer),

    get(m_pSingleTypoCB, "singlereplace");
    get(m_pSglStartQuotePB, "startsingle");
    get(m_pSglStartExFT, "singlestartex");
    get(m_pSglEndQuotePB, "endsingle");
    get(m_pSglEndExFT, "singleendex");
    get(m_pSglStandardPB, "defaultsingle");

    get(m_pDoubleTypoCB, "doublereplace");
    get(m_pDblStartQuotePB, "startdouble");
    get(m_pDblStartExFT, "doublestartex");
    get(m_pDblEndQuotePB, "enddouble");
    get(m_pDblEndExFT, "doubleendex");
    get(m_pDblStandardPB, "defaultdouble");

    using comphelper::string::strip;

    m_sStartQuoteDlg = strip(get<FixedText>("startquoteft")->GetText(), ':');
    m_sEndQuoteDlg = strip(get<FixedText>("endquoteft")->GetText(), ':');
    m_sStandard = get<FixedText>("singlestartex")->GetText();

    sal_Bool bShowSWOptions = sal_False;

    SFX_ITEMSET_ARG( &rSet, pItem, SfxBoolItem, SID_AUTO_CORRECT_DLG, sal_False );
    if ( pItem && pItem->GetValue() )
        bShowSWOptions = sal_True;

    if ( bShowSWOptions )
    {
        static long aStaticTabs[]=
        {
            3, 0, 20, 40
        };

        m_pSwCheckLB->SetStyle(m_pSwCheckLB->GetStyle() | WB_HSCROLL| WB_VSCROLL);

        m_pSwCheckLB->SvSimpleTable::SetTabs(aStaticTabs);
        String sHeader(get<Window>("m")->GetText());
        sHeader += '\t';
        sHeader += get<Window>("t")->GetText();
        sHeader += '\t';
        m_pSwCheckLB->InsertHeaderEntry( sHeader, HEADERBAR_APPEND,
                        HIB_CENTER | HIB_VCENTER | HIB_FIXEDPOS | HIB_FIXED);
        m_pCheckLB->Hide(true);
    }
    else
    {
        m_pSwCheckLB->HideTable();
    }

    m_pDblStartQuotePB->SetClickHdl(LINK(this,    OfaQuoteTabPage, QuoteHdl));
    m_pDblEndQuotePB->SetClickHdl(LINK(this,      OfaQuoteTabPage, QuoteHdl));
    m_pSglStartQuotePB->SetClickHdl(LINK(this, OfaQuoteTabPage, QuoteHdl));
    m_pSglEndQuotePB->SetClickHdl(LINK(this,   OfaQuoteTabPage, QuoteHdl));
    m_pDblStandardPB->SetClickHdl(LINK(this,   OfaQuoteTabPage, StdQuoteHdl));
    m_pSglStandardPB->SetClickHdl(LINK(this,   OfaQuoteTabPage, StdQuoteHdl));
}

OfaQuoteTabPage::~OfaQuoteTabPage()
{
    delete pCheckButtonData;
    delete m_pSwCheckLB;
}

SfxTabPage* OfaQuoteTabPage::Create( Window* pParent,
                                const SfxItemSet& rAttrSet)
{
    return new OfaQuoteTabPage(pParent, rAttrSet);
}

sal_Bool OfaQuoteTabPage::FillItemSet( SfxItemSet&  )
{
    SvxAutoCorrect* pAutoCorrect = SvxAutoCorrCfg::Get().GetAutoCorrect();

    long nFlags = pAutoCorrect->GetFlags();

    if (m_pCheckLB->IsVisible())
    {
        sal_uInt16 nPos = 0;
        pAutoCorrect->SetAutoCorrFlag(AddNonBrkSpace, m_pCheckLB->IsChecked(nPos++));
        pAutoCorrect->SetAutoCorrFlag(ChgOrdinalNumber, m_pCheckLB->IsChecked(nPos++));
    }

    sal_Bool bModified = sal_False;
    if (m_pSwCheckLB->IsVisible())
    {
        SvxSwAutoFmtFlags *pOpt = &pAutoCorrect->GetSwFlags();

        sal_Bool bCheck = m_pSwCheckLB->IsChecked(ADD_NONBRK_SPACE, CBCOL_FIRST);
        bModified |= pOpt->bAddNonBrkSpace != bCheck;
        pOpt->bAddNonBrkSpace = bCheck;
        pAutoCorrect->SetAutoCorrFlag(AddNonBrkSpace,
                            m_pSwCheckLB->IsChecked(ADD_NONBRK_SPACE, CBCOL_SECOND));

        bCheck = m_pSwCheckLB->IsChecked(REPLACE_1ST, CBCOL_FIRST);
        bModified |= pOpt->bChgOrdinalNumber != bCheck;
        pOpt->bChgOrdinalNumber = bCheck;
        pAutoCorrect->SetAutoCorrFlag(ChgOrdinalNumber,
                        m_pSwCheckLB->IsChecked(REPLACE_1ST, CBCOL_SECOND));
    }

    pAutoCorrect->SetAutoCorrFlag(ChgQuotes, m_pDoubleTypoCB->IsChecked());
    pAutoCorrect->SetAutoCorrFlag(ChgSglQuotes, m_pSingleTypoCB->IsChecked());
    sal_Bool bReturn = nFlags != pAutoCorrect->GetFlags();
    if(cStartQuote != pAutoCorrect->GetStartDoubleQuote())
    {
        bReturn = sal_True;
        sal_Unicode cUCS2 = static_cast<sal_Unicode>(cStartQuote); //TODO
        pAutoCorrect->SetStartDoubleQuote(cUCS2);
    }
    if(cEndQuote != pAutoCorrect->GetEndDoubleQuote())
    {
        bReturn = sal_True;
        sal_Unicode cUCS2 = static_cast<sal_Unicode>(cEndQuote); //TODO
        pAutoCorrect->SetEndDoubleQuote(cUCS2);
    }
    if(cSglStartQuote != pAutoCorrect->GetStartSingleQuote())
    {
        bReturn = sal_True;
        sal_Unicode cUCS2 = static_cast<sal_Unicode>(cSglStartQuote); //TODO
        pAutoCorrect->SetStartSingleQuote(cUCS2);
    }
    if(cSglEndQuote != pAutoCorrect->GetEndSingleQuote())
    {
        bReturn = sal_True;
        sal_Unicode cUCS2 = static_cast<sal_Unicode>(cSglEndQuote); //TODO
        pAutoCorrect->SetEndSingleQuote(cUCS2);
    }

    if( bModified || bReturn )
    {
        SvxAutoCorrCfg& rCfg = SvxAutoCorrCfg::Get();
        rCfg.SetModified();
        rCfg.Commit();
    }
    return bReturn;
}

void OfaQuoteTabPage::ActivatePage( const SfxItemSet& )
{
    ((OfaAutoCorrDlg*)GetTabDialog())->EnableLanguage(sal_False);
}

void OfaQuoteTabPage::Reset( const SfxItemSet& )
{
    SvxAutoCorrect* pAutoCorrect = SvxAutoCorrCfg::Get().GetAutoCorrect();
    const long nFlags = pAutoCorrect->GetFlags();

    // Initialize the Sw options
    if (m_pSwCheckLB->IsVisible())
    {
        SvxSwAutoFmtFlags *pOpt = &pAutoCorrect->GetSwFlags();

        m_pSwCheckLB->SetUpdateMode( sal_False );
        m_pSwCheckLB->Clear();

        m_pSwCheckLB->GetModel()->Insert(CreateEntry(sNonBrkSpace, CBCOL_BOTH ));
        m_pSwCheckLB->GetModel()->Insert(CreateEntry(sOrdinal, CBCOL_BOTH ));

        m_pSwCheckLB->CheckEntryPos( ADD_NONBRK_SPACE, CBCOL_FIRST,    pOpt->bAddNonBrkSpace );
        m_pSwCheckLB->CheckEntryPos( ADD_NONBRK_SPACE, CBCOL_SECOND,   0 != (nFlags & AddNonBrkSpace) );
        m_pSwCheckLB->CheckEntryPos( REPLACE_1ST, CBCOL_FIRST,    pOpt->bChgOrdinalNumber );
        m_pSwCheckLB->CheckEntryPos( REPLACE_1ST, CBCOL_SECOND,   0 != (nFlags & ChgOrdinalNumber) );

        m_pSwCheckLB->SetUpdateMode( sal_True );
    }

    // Initialize the non Sw options
    if (m_pCheckLB->IsVisible())
    {
        m_pCheckLB->SetUpdateMode(false);
        m_pCheckLB->Clear();

        m_pCheckLB->InsertEntry( sNonBrkSpace );
        m_pCheckLB->InsertEntry( sOrdinal );

        sal_uInt16 nPos = 0;
        m_pCheckLB->CheckEntryPos( nPos++, 0 != (nFlags & AddNonBrkSpace) );
        m_pCheckLB->CheckEntryPos( nPos++, 0 != (nFlags & ChgOrdinalNumber) );

        m_pCheckLB->SetUpdateMode(true);
    }

    // Initialize the quote stuffs
    m_pDoubleTypoCB->Check(0 != (nFlags & ChgQuotes));
    m_pSingleTypoCB->Check(0 != (nFlags & ChgSglQuotes));
    m_pDoubleTypoCB->SaveValue();
    m_pSingleTypoCB->SaveValue();

    cStartQuote = pAutoCorrect->GetStartDoubleQuote();
    cEndQuote = pAutoCorrect->GetEndDoubleQuote();
    cSglStartQuote = pAutoCorrect->GetStartSingleQuote();
    cSglEndQuote = pAutoCorrect->GetEndSingleQuote();

    m_pSglStartExFT->SetText(ChangeStringExt_Impl(cSglStartQuote));
    m_pSglEndExFT->SetText(ChangeStringExt_Impl(cSglEndQuote));
    m_pDblStartExFT->SetText(ChangeStringExt_Impl(cStartQuote));
    m_pDblEndExFT->SetText(ChangeStringExt_Impl(cEndQuote));
}

#define SGL_START       0
#define DBL_START       1
#define SGL_END         2
#define DBL_END         3


IMPL_LINK( OfaQuoteTabPage, QuoteHdl, PushButton*, pBtn )
{
    sal_uInt16 nMode = SGL_START;
    if (pBtn == m_pSglEndQuotePB)
        nMode = SGL_END;
    else if (pBtn == m_pDblStartQuotePB)
        nMode = DBL_START;
    else if (pBtn == m_pDblEndQuotePB)
        nMode = DBL_END;
    // start character selection dialog
    SvxCharacterMap* pMap = new SvxCharacterMap( this, sal_True );
    pMap->SetCharFont( OutputDevice::GetDefaultFont(DEFAULTFONT_LATIN_TEXT,
                        LANGUAGE_ENGLISH_US, DEFAULTFONT_FLAGS_ONLYONE, 0 ));
    pMap->SetText(nMode < SGL_END ? m_sStartQuoteDlg  : m_sEndQuoteDlg );
    sal_UCS4 cDlg;
    SvxAutoCorrect* pAutoCorrect = SvxAutoCorrCfg::Get().GetAutoCorrect();
    LanguageType eLang = Application::GetSettings().GetLanguageTag().getLanguageType();
    switch( nMode )
    {
        case SGL_START:
            cDlg = cSglStartQuote;
            if(cDlg == 0)
                cDlg = pAutoCorrect->GetQuote('\'', sal_True, eLang);
        break;
        case SGL_END:
            cDlg = cSglEndQuote;
            if(cDlg == 0)
                cDlg = pAutoCorrect->GetQuote('\'', sal_False, eLang);
        break;
        case DBL_START:
            cDlg = cStartQuote;
            if(cDlg == 0)
                cDlg = pAutoCorrect->GetQuote('\"', sal_True, eLang);
        break;
        case DBL_END:
            cDlg = cEndQuote;
            if(cDlg == 0)
                cDlg = pAutoCorrect->GetQuote('\"', sal_False, eLang);
        break;
        default:
            OSL_FAIL("svx::OfaQuoteTabPage::QuoteHdl(), how to initialize cDlg?" );
            cDlg = 0;
            break;

    }
    pMap->SetChar(  cDlg );
    pMap->DisableFontSelection();
    if(pMap->Execute() == RET_OK)
    {
        sal_UCS4 cNewChar = pMap->GetChar();
        switch( nMode )
        {
            case SGL_START:
                cSglStartQuote = cNewChar;
                m_pSglStartExFT->SetText(ChangeStringExt_Impl(cNewChar));
            break;
            case SGL_END:
                cSglEndQuote = cNewChar;
                m_pSglEndExFT->SetText(ChangeStringExt_Impl(cNewChar));
            break;
            case DBL_START:
                cStartQuote = cNewChar;
                m_pDblStartExFT->SetText(ChangeStringExt_Impl(cNewChar));
            break;
            case DBL_END:
                cEndQuote = cNewChar;
                m_pDblEndExFT->SetText(ChangeStringExt_Impl(cNewChar));
            break;
        }
    }
    delete pMap;

    return 0;
}

IMPL_LINK( OfaQuoteTabPage, StdQuoteHdl, PushButton*, pBtn )
{
    if (pBtn == m_pDblStandardPB)
    {
        cStartQuote = 0;
        m_pDblStartExFT->SetText(ChangeStringExt_Impl(0));
        cEndQuote = 0;
        m_pDblEndExFT->SetText(ChangeStringExt_Impl(0));

    }
    else
    {
        cSglStartQuote = 0;
        m_pSglStartExFT->SetText(ChangeStringExt_Impl(0));
        cSglEndQuote = 0;
        m_pSglEndExFT->SetText(ChangeStringExt_Impl(0));
    }
    return 0;
}

// --------------------------------------------------

String OfaQuoteTabPage::ChangeStringExt_Impl( sal_UCS4 cChar )
{
    if (!cChar)
        return m_sStandard;

    // convert codepoint value to unicode-hex string
    sal_UCS4 aStrCodes[32] = { 0, ' ', '(', 'U', '+', '0' };
    aStrCodes[0] = cChar;
    int nFullLen = 5;
    int nHexLen = 4;
    while( (cChar >> (4*nHexLen)) != 0 )
        ++nHexLen;
    for( int i = nHexLen; --i >= 0;)
    {
        sal_UCS4 cHexDigit = ((cChar >> (4*i)) & 0x0f) + '0';
        if( cHexDigit > '9' )
            cHexDigit += 'A' - ('9' + 1);
        aStrCodes[ nFullLen++ ] = cHexDigit;
    }
    aStrCodes[ nFullLen++ ] = ')';
    // using the new UCS4 constructor
    OUString aOUStr( aStrCodes, nFullLen );
    return aOUStr;
}

OfaAutoCompleteTabPage::OfaAutoCompleteTabPage( Window* pParent,
                                                const SfxItemSet& rSet )
    : SfxTabPage(pParent, CUI_RES( RID_OFAPAGE_AUTOCOMPLETE_OPTIONS ), rSet),
    aCBActiv        (this, CUI_RES(CB_ACTIV)),
    aCBAppendSpace  (this, CUI_RES(CB_APPEND_SPACE)),
    aCBAsTip        (this, CUI_RES(CB_AS_TIP)),
    aCBCollect      (this, CUI_RES(CB_COLLECT)),
    aCBRemoveList   (this, CUI_RES(CB_REMOVE_LIST)),
    aFTExpandKey    (this, CUI_RES(FT_EXPAND_KEY)),
    aDCBExpandKey   (this, CUI_RES(DCB_EXPAND_KEY)),
    aFTMinWordlen   (this, CUI_RES(FT_MIN_WORDLEN)),
    aNFMinWordlen   (this, CUI_RES(NF_MIN_WORDLEN)),
    aFTMaxEntries   (this, CUI_RES(FT_MAX_ENTRIES)),
    aNFMaxEntries   (this, CUI_RES(NF_MAX_ENTRIES)),
    aLBEntries      (*this, CUI_RES(LB_ENTRIES)),
    aPBEntries      (this, CUI_RES(PB_ENTRIES)),
    m_pAutoCompleteList( 0 ),
    nAutoCmpltListCnt( 0 )
{
    FreeResource();

    // the defined KEYs
    static const sal_uInt16 aKeyCodes[] = {
        KEY_END,
        KEY_RETURN,
        KEY_SPACE,
        KEY_RIGHT,
        KEY_TAB,
        0
    };

    for( const sal_uInt16* pKeys = aKeyCodes; *pKeys; ++pKeys )
    {
        KeyCode aKCode( *pKeys );
        sal_uInt16 nPos = aDCBExpandKey.InsertEntry( aKCode.GetName() );
        aDCBExpandKey.SetEntryData( nPos, (void*)(sal_uLong)*pKeys );
        if( KEY_RETURN == *pKeys )      // default to RETURN
            aDCBExpandKey.SelectEntryPos( nPos );
    }

    aPBEntries.SetClickHdl(LINK(this, OfaAutoCompleteTabPage, DeleteHdl));
    aCBActiv.SetToggleHdl(LINK(this, OfaAutoCompleteTabPage, CheckHdl));
    aCBCollect.SetToggleHdl(LINK(this, OfaAutoCompleteTabPage, CheckHdl));
    aLBEntries.SetAccessibleRelationLabeledBy(&aLBEntries);
}

OfaAutoCompleteTabPage::~OfaAutoCompleteTabPage()
{
}

SfxTabPage* OfaAutoCompleteTabPage::Create( Window* pParent,
                                            const SfxItemSet& rSet)
{
    return new OfaAutoCompleteTabPage( pParent, rSet );
}

sal_Bool OfaAutoCompleteTabPage::FillItemSet( SfxItemSet& )
{
    sal_Bool bModified = sal_False, bCheck;
    SvxAutoCorrect* pAutoCorrect = SvxAutoCorrCfg::Get().GetAutoCorrect();
    SvxSwAutoFmtFlags *pOpt = &pAutoCorrect->GetSwFlags();
    sal_uInt16 nVal;

    bCheck = aCBActiv.IsChecked();
    bModified |= pOpt->bAutoCompleteWords != bCheck;
    pOpt->bAutoCompleteWords = bCheck;
    bCheck = aCBCollect.IsChecked();
    bModified |= pOpt->bAutoCmpltCollectWords != bCheck;
    pOpt->bAutoCmpltCollectWords = bCheck;
    bCheck = !aCBRemoveList.IsChecked(); // inverted value!
    bModified |= pOpt->bAutoCmpltKeepList != bCheck;
    pOpt->bAutoCmpltKeepList = bCheck;
    bCheck = aCBAppendSpace.IsChecked();
    bModified |= pOpt->bAutoCmpltAppendBlanc != bCheck;
    pOpt->bAutoCmpltAppendBlanc = bCheck;
    bCheck = aCBAsTip.IsChecked();
    bModified |= pOpt->bAutoCmpltShowAsTip != bCheck;
    pOpt->bAutoCmpltShowAsTip = bCheck;

    nVal = (sal_uInt16)aNFMinWordlen.GetValue();
    bModified |= nVal != pOpt->nAutoCmpltWordLen;
    pOpt->nAutoCmpltWordLen = nVal;

    nVal = (sal_uInt16)aNFMaxEntries.GetValue();
    bModified |= nVal != pOpt->nAutoCmpltListLen;
    pOpt->nAutoCmpltListLen = nVal;

    nVal = aDCBExpandKey.GetSelectEntryPos();
    if( nVal < aDCBExpandKey.GetEntryCount() )
    {
        sal_uLong nKey = (sal_uLong)aDCBExpandKey.GetEntryData( nVal );
        bModified |= nKey != pOpt->nAutoCmpltExpandKey;
        pOpt->nAutoCmpltExpandKey = (sal_uInt16)nKey;
   }

    if (m_pAutoCompleteList && nAutoCmpltListCnt != aLBEntries.GetEntryCount())
    {
        bModified = sal_True;
        pOpt->m_pAutoCompleteList = m_pAutoCompleteList;
    }
    if( bModified )
    {
        SvxAutoCorrCfg& rCfg = SvxAutoCorrCfg::Get();
        rCfg.SetModified();
        rCfg.Commit();
    }
    return sal_True;
}

void OfaAutoCompleteTabPage::Reset( const SfxItemSet&  )
{
    SvxAutoCorrect* pAutoCorrect = SvxAutoCorrCfg::Get().GetAutoCorrect();
    SvxSwAutoFmtFlags *pOpt = &pAutoCorrect->GetSwFlags();

    aCBActiv.Check( 0 != pOpt->bAutoCompleteWords );
    aCBCollect.Check( 0 != pOpt->bAutoCmpltCollectWords );
    aCBRemoveList.Check( !pOpt->bAutoCmpltKeepList ); //inverted value!
    aCBAppendSpace.Check( 0 != pOpt->bAutoCmpltAppendBlanc );
    aCBAsTip.Check( 0 != pOpt->bAutoCmpltShowAsTip );

    aNFMinWordlen.SetValue( pOpt->nAutoCmpltWordLen );
    aNFMaxEntries.SetValue( pOpt->nAutoCmpltListLen );

    // select the specific KeyCode:
    {
        sal_uLong nKey = pOpt->nAutoCmpltExpandKey;
        for( sal_uInt16 n = 0, nCnt = aDCBExpandKey.GetEntryCount(); n < nCnt; ++n )
            if( nKey == (sal_uLong)aDCBExpandKey.GetEntryData( n ))
            {
                aDCBExpandKey.SelectEntryPos( n );
                break;
            }
    }

    if (pOpt->m_pAutoCompleteList && pOpt->m_pAutoCompleteList->size())
    {
        m_pAutoCompleteList = const_cast<editeng::SortedAutoCompleteStrings*>(
                pOpt->m_pAutoCompleteList);
        pOpt->m_pAutoCompleteList = 0;
        nAutoCmpltListCnt = m_pAutoCompleteList->size();
        for (size_t n = 0; n < nAutoCmpltListCnt; ++n)
        {
            const OUString* pStr =
                &(*m_pAutoCompleteList)[n]->GetAutoCompleteString();
            sal_uInt16 nPos = aLBEntries.InsertEntry( *pStr );
            aLBEntries.SetEntryData( nPos, (void*)pStr );
        }
    }
    else
    {
        aLBEntries.Disable();
        aPBEntries.Disable();
    }

    CheckHdl( &aCBActiv );
    CheckHdl( &aCBCollect );
}

void OfaAutoCompleteTabPage::ActivatePage( const SfxItemSet& )
{
    ((OfaAutoCorrDlg*)GetTabDialog())->EnableLanguage( sal_False );
}

IMPL_LINK_NOARG(OfaAutoCompleteTabPage, DeleteHdl)
{
    sal_uInt16 nSelCnt =
        (m_pAutoCompleteList) ? aLBEntries.GetSelectEntryCount() : 0;
    while( nSelCnt )
    {
        sal_uInt16 nPos = aLBEntries.GetSelectEntryPos( --nSelCnt );
        OUString* pStr = static_cast<OUString*>(aLBEntries.GetEntryData(nPos));
        aLBEntries.RemoveEntry( nPos );
        editeng::IAutoCompleteString hack(*pStr); // UGLY
        m_pAutoCompleteList->erase(&hack);
    }
    return 0;
}

IMPL_LINK( OfaAutoCompleteTabPage, CheckHdl, CheckBox*, pBox )
{
    sal_Bool bEnable = pBox->IsChecked();
    if( pBox == &aCBActiv )
    {
        aCBAppendSpace.Enable( bEnable );
        aCBAppendSpace.Enable( bEnable );
        aCBAsTip.Enable( bEnable );
        aDCBExpandKey.Enable( bEnable );
    }
    else if(&aCBCollect == pBox)
        aCBRemoveList.Enable( bEnable );
    return 0;
}

void OfaAutoCompleteTabPage::CopyToClipboard() const
{
    sal_uInt16 nSelCnt = aLBEntries.GetSelectEntryCount();
    if (m_pAutoCompleteList && nSelCnt)
    {
        TransferDataContainer* pCntnr = new TransferDataContainer;
        ::com::sun::star::uno::Reference<
            ::com::sun::star::datatransfer::XTransferable > xRef( pCntnr );

        OStringBuffer sData;
        const sal_Char aLineEnd[] =
#if defined(WNT)
                "\015\012";
#else
                "\012";
#endif

        rtl_TextEncoding nEncode = osl_getThreadTextEncoding();

        for( sal_uInt16 n = 0; n < nSelCnt; ++n )
        {
            sData.append(OUStringToOString(aLBEntries.GetSelectEntry(n),
                nEncode));
            sData.append(RTL_CONSTASCII_STRINGPARAM(aLineEnd));
        }
        pCntnr->CopyByteString( SOT_FORMAT_STRING, sData.makeStringAndClear() );
        pCntnr->CopyToClipboard( (Window*)this );
    }
}

long OfaAutoCompleteTabPage::AutoCompleteMultiListBox::PreNotify(
            NotifyEvent& rNEvt )
{
    long nHandled = MultiListBox::PreNotify( rNEvt );

    if( !nHandled && EVENT_KEYUP == rNEvt.GetType() )
    {
        const KeyCode& rKeyCode = rNEvt.GetKeyEvent()->GetKeyCode();
        switch( rKeyCode.GetModifier() | rKeyCode.GetCode() )
        {
        case KEY_DELETE:
            rPage.DeleteHdl( 0 );
            nHandled = 1;
            break;

        default:
            if( KEYFUNC_COPY == rKeyCode.GetFunction() )
            {
                rPage.CopyToClipboard();
                nHandled = 1;
            }
            break;
        }
    }
    return nHandled;
}

// class OfaSmartTagOptionsTabPage ---------------------------------------------

OfaSmartTagOptionsTabPage::OfaSmartTagOptionsTabPage( Window* pParent,
                                                      const SfxItemSet& rSet )
    : SfxTabPage(pParent, CUI_RES( RID_OFAPAGE_SMARTTAG_OPTIONS ), rSet),
    m_aMainCB( this, CUI_RES(CB_SMARTTAGS) ),
    m_aSmartTagTypesLB( this, CUI_RES(LB_SMARTTAGS) ),
    m_aPropertiesPB( this, CUI_RES(PB_SMARTTAGS) ),
    m_aTitleFT( this, CUI_RES(FT_SMARTTAGS) )
{
    FreeResource();

    // some options for the list box:
    m_aSmartTagTypesLB.SetStyle( m_aSmartTagTypesLB.GetStyle() | WB_HSCROLL | WB_HIDESELECTION );
    m_aSmartTagTypesLB.SetHighlightRange();

    // set the handlers:
    m_aMainCB.SetToggleHdl(LINK(this, OfaSmartTagOptionsTabPage, CheckHdl));
    m_aPropertiesPB.SetClickHdl(LINK(this, OfaSmartTagOptionsTabPage, ClickHdl));
    m_aSmartTagTypesLB.SetSelectHdl(LINK(this, OfaSmartTagOptionsTabPage, SelectHdl));
}

OfaSmartTagOptionsTabPage::~OfaSmartTagOptionsTabPage()
{

}

SfxTabPage* OfaSmartTagOptionsTabPage::Create( Window* pParent, const SfxItemSet& rSet)
{
    return new OfaSmartTagOptionsTabPage( pParent, rSet );
}

/** This struct is used to associate list box entries with smart tag data
*/
struct ImplSmartTagLBUserData
{
    OUString maSmartTagType;
    uno::Reference< smarttags::XSmartTagRecognizer > mxRec;
    sal_Int32 mnSmartTagIdx;

    ImplSmartTagLBUserData( const OUString& rSmartTagType,
                            uno::Reference< smarttags::XSmartTagRecognizer > xRec,
                            sal_Int32 nSmartTagIdx ) :
        maSmartTagType( rSmartTagType ),
        mxRec( xRec ),
        mnSmartTagIdx( nSmartTagIdx ) {}
};

/** Clears m_aSmartTagTypesLB
*/
void OfaSmartTagOptionsTabPage::ClearListBox()
{
    const sal_uLong nCount = m_aSmartTagTypesLB.GetEntryCount();
    for ( sal_uInt16 i = 0; i < nCount; ++i )
    {
        const SvTreeListEntry* pEntry = m_aSmartTagTypesLB.GetEntry(i);
        const ImplSmartTagLBUserData* pUserData = static_cast< ImplSmartTagLBUserData* >(pEntry->GetUserData());
        delete pUserData;
    }

    m_aSmartTagTypesLB.Clear();
}

/** Inserts items into m_aSmartTagTypesLB
*/
void OfaSmartTagOptionsTabPage::FillListBox( const SmartTagMgr& rSmartTagMgr )
{
    // first we have to clear the list box:
    ClearListBox();

    // fill list box:
    const sal_uInt32 nNumberOfRecognizers = rSmartTagMgr.NumberOfRecognizers();
    const lang::Locale aLocale( LanguageTag::convertToLocale( eLastDialogLanguage ) );

    for ( sal_uInt32 i = 0; i < nNumberOfRecognizers; ++i )
    {
        uno::Reference< smarttags::XSmartTagRecognizer > xRec = rSmartTagMgr.GetRecognizer(i);

        const OUString aName = xRec->getName( aLocale );
        const sal_Int32 nNumberOfSupportedSmartTags = xRec->getSmartTagCount();

        for ( sal_Int32 j = 0; j < nNumberOfSupportedSmartTags; ++j )
        {
            const OUString aSmartTagType = xRec->getSmartTagName(j);
            OUString aSmartTagCaption = rSmartTagMgr.GetSmartTagCaption( aSmartTagType, aLocale );

            if ( aSmartTagCaption.isEmpty() )
                aSmartTagCaption = aSmartTagType;

            const OUString aLBEntry = aSmartTagCaption +
                                           OUString(" (") +
                                           aName +
                                           OUString(")");

            SvTreeListEntry* pEntry = m_aSmartTagTypesLB.SvTreeListBox::InsertEntry( aLBEntry );
            if ( pEntry )
            {
                const bool bCheck = rSmartTagMgr.IsSmartTagTypeEnabled( aSmartTagType );
                m_aSmartTagTypesLB.SetCheckButtonState( pEntry, bCheck ? SV_BUTTON_CHECKED : SV_BUTTON_UNCHECKED );
                pEntry->SetUserData(static_cast<void*>(new ImplSmartTagLBUserData( aSmartTagType, xRec, j ) ) );
            }
        }
    }
}

/** Handler for the push button
*/
IMPL_LINK_NOARG(OfaSmartTagOptionsTabPage, ClickHdl)
{
    const sal_uInt16 nPos = m_aSmartTagTypesLB.GetSelectEntryPos();
    const SvTreeListEntry* pEntry = m_aSmartTagTypesLB.GetEntry(nPos);
    const ImplSmartTagLBUserData* pUserData = static_cast< ImplSmartTagLBUserData* >(pEntry->GetUserData());
    uno::Reference< smarttags::XSmartTagRecognizer > xRec = pUserData->mxRec;
    const sal_Int32 nSmartTagIdx = pUserData->mnSmartTagIdx;

     const lang::Locale aLocale( LanguageTag::convertToLocale( eLastDialogLanguage ) );
    if ( xRec->hasPropertyPage( nSmartTagIdx, aLocale ) )
        xRec->displayPropertyPage( nSmartTagIdx, aLocale );

    return 0;
}

/** Handler for the check box
*/
IMPL_LINK_NOARG(OfaSmartTagOptionsTabPage, CheckHdl)
{
    const sal_Bool bEnable = m_aMainCB.IsChecked();
    m_aSmartTagTypesLB.Enable( bEnable );
    m_aSmartTagTypesLB.Invalidate();
    m_aPropertiesPB.Enable( false );

    // if the controls are currently enabled, we still have to check
    // if the properties button should be disabled because the currently
    // seleted smart tag type does not have a properties dialog.
    // We do this by calling SelectHdl:
    if ( bEnable )
        SelectHdl( &m_aSmartTagTypesLB );

    return 0;
}

/** Handler for the list box
*/
IMPL_LINK_NOARG(OfaSmartTagOptionsTabPage, SelectHdl)
{
    if ( m_aSmartTagTypesLB.GetEntryCount() < 1 )
        return 0;

    const sal_uInt16 nPos = m_aSmartTagTypesLB.GetSelectEntryPos();
    const SvTreeListEntry* pEntry = m_aSmartTagTypesLB.GetEntry(nPos);
    const ImplSmartTagLBUserData* pUserData = static_cast< ImplSmartTagLBUserData* >(pEntry->GetUserData());
    uno::Reference< smarttags::XSmartTagRecognizer > xRec = pUserData->mxRec;
    const sal_Int32 nSmartTagIdx = pUserData->mnSmartTagIdx;

    const lang::Locale aLocale( LanguageTag::convertToLocale( eLastDialogLanguage ) );
    if ( xRec->hasPropertyPage( nSmartTagIdx, aLocale ) )
        m_aPropertiesPB.Enable( sal_True );
    else
        m_aPropertiesPB.Enable( sal_False );

    return 0;
}

/** Propagates the current settings to the smart tag manager.
*/
sal_Bool OfaSmartTagOptionsTabPage::FillItemSet( SfxItemSet& )
{
    SvxAutoCorrect* pAutoCorrect = SvxAutoCorrCfg::Get().GetAutoCorrect();
    SvxSwAutoFmtFlags *pOpt = &pAutoCorrect->GetSwFlags();
    SmartTagMgr* pSmartTagMgr = pOpt->pSmartTagMgr;

    // robust!
    if ( !pSmartTagMgr )
        return sal_False;

    sal_Bool bModifiedSmartTagTypes = sal_False;
    std::vector< OUString > aDisabledSmartTagTypes;

    const sal_uLong nCount = m_aSmartTagTypesLB.GetEntryCount();

    for ( sal_uInt16 i = 0; i < nCount; ++i )
    {
        const SvTreeListEntry* pEntry = m_aSmartTagTypesLB.GetEntry(i);
        const ImplSmartTagLBUserData* pUserData = static_cast< ImplSmartTagLBUserData* >(pEntry->GetUserData());
        const sal_Bool bChecked = m_aSmartTagTypesLB.IsChecked(i);
        const sal_Bool bIsCurrentlyEnabled = pSmartTagMgr->IsSmartTagTypeEnabled( pUserData->maSmartTagType );

        bModifiedSmartTagTypes = bModifiedSmartTagTypes || ( !bChecked != !bIsCurrentlyEnabled );

        if ( !bChecked )
            aDisabledSmartTagTypes.push_back( pUserData->maSmartTagType );

        delete pUserData;
    }

    const sal_Bool bModifiedRecognize = ( !m_aMainCB.IsChecked() != !pSmartTagMgr->IsLabelTextWithSmartTags() );
    if ( bModifiedSmartTagTypes || bModifiedRecognize )
    {
        bool bLabelTextWithSmartTags = m_aMainCB.IsChecked() ? true : false;
        pSmartTagMgr->WriteConfiguration( bModifiedRecognize     ? &bLabelTextWithSmartTags : 0,
                                          bModifiedSmartTagTypes ? &aDisabledSmartTagTypes : 0 );
    }

    return sal_True;
}

/** Sets the controls based on the current settings at SmartTagMgr.
*/
void OfaSmartTagOptionsTabPage::Reset( const SfxItemSet&  )
{
    SvxAutoCorrect* pAutoCorrect = SvxAutoCorrCfg::Get().GetAutoCorrect();
    SvxSwAutoFmtFlags *pOpt = &pAutoCorrect->GetSwFlags();
    const SmartTagMgr* pSmartTagMgr = pOpt->pSmartTagMgr;

    // robust, should not happen!
    if ( !pSmartTagMgr )
        return;

    FillListBox( *pSmartTagMgr );
    m_aSmartTagTypesLB.SelectEntryPos( 0 );
    m_aMainCB.Check( pSmartTagMgr->IsLabelTextWithSmartTags() );
    CheckHdl( &m_aMainCB );
}

void OfaSmartTagOptionsTabPage::ActivatePage( const SfxItemSet& )
{
    ((OfaAutoCorrDlg*)GetTabDialog())->EnableLanguage( sal_False );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

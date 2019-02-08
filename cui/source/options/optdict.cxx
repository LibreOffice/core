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

#include <editeng/unolingu.hxx>
#include <svx/dlgutil.hxx>
#include <svx/dialmgr.hxx>
#include <sfx2/sfxuno.hxx>
#include <svl/eitem.hxx>
#include <com/sun/star/frame/XStorable.hpp>
#include <comphelper/string.hxx>
#include <unotools/intlwrapper.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <vcl/settings.hxx>
#include <vcl/builderfactory.hxx>
#include <vcl/event.hxx>
#include <svx/dialogs.hrc>

#include <linguistic/misc.hxx>
#include <strings.hrc>
#include <optdict.hxx>
#include <dialmgr.hxx>
#include <svx/svxerr.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::linguistic2;
using namespace linguistic;

// static function -------------------------------------------------------

static OUString getNormDicEntry_Impl(const OUString &rText)
{
    OUString aTmp(comphelper::string::stripEnd(rText, '.'));
    // non-standard hyphenation
    if (aTmp.indexOf('[') > -1)
    {
        OUStringBuffer aTmp2 ( aTmp.getLength() );
        bool bSkip = false;
        for (sal_Int32 i = 0; i < aTmp.getLength(); i++)
        {
            sal_Unicode cTmp = aTmp[i];
            if (cTmp == '[')
                bSkip = true;
            else if (!bSkip)
                aTmp2.append( cTmp );
            else if (cTmp == ']')
                bSkip = false;
        }
        aTmp = aTmp2.makeStringAndClear();
    }
    return aTmp.replaceAll("=", "");
}

// Compare Dictionary Entry  result
enum CDE_RESULT { CDE_EQUAL, CDE_SIMILAR, CDE_DIFFERENT };

static CDE_RESULT cmpDicEntry_Impl( const OUString &rText1, const OUString &rText2 )
{
    CDE_RESULT eRes = CDE_DIFFERENT;

    if (rText1 == rText2)
        eRes = CDE_EQUAL;
    else
    {   // similar = equal up to trailing '.' and hyphenation positions
        // marked with '=' and '[' + alternative spelling pattern + ']'
        if (getNormDicEntry_Impl( rText1 ) == getNormDicEntry_Impl( rText2 ))
            eRes = CDE_SIMILAR;
    }

    return eRes;
}

// class SvxNewDictionaryDialog -------------------------------------------

SvxNewDictionaryDialog::SvxNewDictionaryDialog(weld::Window* pParent)
    : GenericDialogController(pParent, "cui/ui/optnewdictionarydialog.ui", "OptNewDictionaryDialog")
    , m_xNameEdit(m_xBuilder->weld_entry("nameedit"))
    , m_xLanguageLB(new LanguageBox(m_xBuilder->weld_combo_box("language")))
    , m_xExceptBtn(m_xBuilder->weld_check_button("except"))
    , m_xOKBtn(m_xBuilder->weld_button("ok"))
{
    // install handler
    m_xNameEdit->connect_changed(LINK(this, SvxNewDictionaryDialog, ModifyHdl_Impl));
    m_xOKBtn->connect_clicked(LINK(this, SvxNewDictionaryDialog, OKHdl_Impl));

    // display languages
    m_xLanguageLB->SetLanguageList(SvxLanguageListFlags::ALL, true, true);
    m_xLanguageLB->set_active(0);
}

IMPL_LINK_NOARG(SvxNewDictionaryDialog, OKHdl_Impl, weld::Button&, void)
{

  // add extension for personal dictionaries
    OUString sDict = comphelper::string::stripEnd(m_xNameEdit->get_text(), ' ') + ".dic";

    Reference< XSearchableDictionaryList >  xDicList( LinguMgr::GetDictionaryList() );

    Sequence< Reference< XDictionary >  > aDics;
    if (xDicList.is())
        aDics = xDicList->getDictionaries();
    const Reference< XDictionary >  *pDic = aDics.getConstArray();
    sal_Int32 nCount = aDics.getLength();

    bool bFound = false;
    sal_Int32 i;
    for (i = 0; !bFound && i < nCount; ++i )
        if ( sDict.equalsIgnoreAsciiCase( pDic[i]->getName()) )
            bFound = true;

    if ( bFound )
    {
        // duplicate names?
        std::unique_ptr<weld::MessageDialog> xInfoBox(Application::CreateMessageDialog(m_xDialog.get(),
                                                      VclMessageType::Info, VclButtonsType::Ok,
                                                      CuiResId(RID_SVXSTR_OPT_DOUBLE_DICTS)));
        xInfoBox->run();
        m_xNameEdit->grab_focus();
        return;
    }

    // create and add
    LanguageType nLang = m_xLanguageLB->get_active_id();
    try
    {
        // create new dictionary
        DictionaryType eType = m_xExceptBtn->get_active() ?
                DictionaryType_NEGATIVE : DictionaryType_POSITIVE;
        if (xDicList.is())
        {
            lang::Locale aLocale( LanguageTag::convertToLocale(nLang) );
            OUString aURL( linguistic::GetWritableDictionaryURL( sDict ) );
            m_xNewDic.set(xDicList->createDictionary(sDict, aLocale, eType, aURL) , UNO_QUERY);
            m_xNewDic->setActive(true);
        }
        DBG_ASSERT(m_xNewDic.is(), "NULL pointer");
    }
    catch(...)
    {
        m_xNewDic = nullptr;
        // error: couldn't create new dictionary
        SfxErrorContext aContext( ERRCTX_SVX_LINGU_DICTIONARY, OUString(),
            m_xDialog.get(), RID_SVXERRCTX, SvxResLocale() );
        ErrorHandler::HandleError( *new StringErrorInfo(
                ERRCODE_SVX_LINGU_DICT_NOTWRITEABLE, sDict ) );
        m_xDialog->response(RET_CANCEL);
    }

    if (xDicList.is() && m_xNewDic.is())
    {
        xDicList->addDictionary(Reference<XDictionary>(m_xNewDic, UNO_QUERY));

        // refresh list of dictionaries
        //! dictionaries may have been added/removed elsewhere too.
        aDics = xDicList->getDictionaries();
    }

    m_xDialog->response(RET_OK);
}

IMPL_LINK_NOARG(SvxNewDictionaryDialog, ModifyHdl_Impl, weld::Entry&, void)
{
    m_xOKBtn->set_sensitive(!m_xNameEdit->get_text().isEmpty());
}

// class SvxEditDictionaryDialog -------------------------------------------

SvxEditDictionaryDialog::SvxEditDictionaryDialog(weld::Window* pParent, const OUString& rName)
    : GenericDialogController(pParent, "cui/ui/editdictionarydialog.ui", "EditDictionaryDialog")
    , sModify(CuiResId(STR_MODIFY))
    , bFirstSelect(false)
    , bDoNothing(false)
    , bDicIsReadonly(false)
    , m_xAllDictsLB(m_xBuilder->weld_combo_box("book"))
    , m_xLangFT(m_xBuilder->weld_label("lang_label"))
    , m_xLangLB(new LanguageBox(m_xBuilder->weld_combo_box("lang")))
    , m_xWordED(m_xBuilder->weld_entry("word"))
    , m_xReplaceFT(m_xBuilder->weld_label("replace_label"))
    , m_xReplaceED(m_xBuilder->weld_entry("replace"))
    , m_xSingleColumnLB(m_xBuilder->weld_tree_view("words"))
    , m_xDoubleColumnLB(m_xBuilder->weld_tree_view("replaces"))
    , m_xNewReplacePB(m_xBuilder->weld_button("newreplace"))
    , m_xDeletePB(m_xBuilder->weld_button("delete"))
{
    sReplaceFT_Text = m_xReplaceFT->get_label();
    m_xSingleColumnLB->set_size_request(-1, m_xSingleColumnLB->get_height_rows(8));
    m_xDoubleColumnLB->set_size_request(-1, m_xDoubleColumnLB->get_height_rows(8));
    m_pWordsLB = m_xDoubleColumnLB.get();
    m_xSingleColumnLB->hide();

    //set to max of both sizes to avoid resizes
    sNew = m_xNewReplacePB->get_label();
    auto nNewWidth = m_xNewReplacePB->get_preferred_size().Width();
    m_xNewReplacePB->set_label(sModify);
    auto nReplaceWidth = m_xNewReplacePB->get_preferred_size().Width();
    m_xNewReplacePB->set_label(sNew);
    m_xNewReplacePB->set_size_request(std::max(nNewWidth, nReplaceWidth), -1);

    if (LinguMgr::GetDictionaryList().is())
        aDics = LinguMgr::GetDictionaryList()->getDictionaries();

    m_xSingleColumnLB->connect_changed(LINK(this, SvxEditDictionaryDialog, SelectHdl));
    m_xDoubleColumnLB->connect_changed(LINK(this, SvxEditDictionaryDialog, SelectHdl));

    std::vector<int> aWidths;
    aWidths.push_back(m_xDoubleColumnLB->get_approximate_digit_width() * 22);
    m_xDoubleColumnLB->set_column_fixed_widths(aWidths);

    // install handler
    m_xNewReplacePB->connect_clicked(
        LINK( this, SvxEditDictionaryDialog, NewDelButtonHdl));
    m_xDeletePB->connect_clicked(
        LINK( this, SvxEditDictionaryDialog, NewDelButtonHdl));

    m_xLangLB->connect_changed(
        LINK( this, SvxEditDictionaryDialog, SelectLangHdl_Impl ) );
    m_xAllDictsLB->connect_changed(
        LINK( this, SvxEditDictionaryDialog, SelectBookHdl_Impl ) );

    m_xWordED->connect_changed(LINK(this, SvxEditDictionaryDialog, ModifyHdl));
    m_xReplaceED->connect_changed(LINK(this, SvxEditDictionaryDialog, ModifyHdl));
    m_xWordED->connect_activate(LINK(this, SvxEditDictionaryDialog, NewDelActionHdl));
    m_xReplaceED->connect_activate(LINK(this, SvxEditDictionaryDialog, NewDelActionHdl));

    // fill listbox with all available WB's
    const Reference< XDictionary >  *pDic = aDics.getConstArray();
    sal_Int32 nCount = aDics.getLength();

    OUString aLookUpEntry;
    for ( sal_Int32 i = 0; i < nCount; ++i )
    {
        Reference< XDictionary >  xDic( pDic[i], UNO_QUERY );
        if (xDic.is())
        {
            bool bNegative = xDic->getDictionaryType() == DictionaryType_NEGATIVE;
            OUString aDicName( xDic->getName() );
            const OUString aTxt( ::GetDicInfoStr( aDicName,
                        LanguageTag( xDic->getLocale() ).getLanguageType(), bNegative ) );
            m_xAllDictsLB->append_text(aTxt);

            if (rName == aDicName)
                aLookUpEntry = aTxt;
        }
    }

    m_xLangLB->SetLanguageList( SvxLanguageListFlags::ALL, true, true );

    Link<OUString&,bool> aLink = LINK(this, SvxEditDictionaryDialog, InsertTextHdl);
    m_xReplaceED->connect_insert_text(aLink);
    m_xWordED->connect_insert_text(aLink);

    if ( nCount > 0 )
    {
        m_xAllDictsLB->set_active_text(aLookUpEntry);
        int nPos = m_xAllDictsLB->get_active();

        if (nPos == -1)
        {
            nPos = 0;
            m_xAllDictsLB->set_active(nPos);
        }
        Reference< XDictionary >  xDic;
        if (nPos != -1)
            xDic.set( aDics.getConstArray()[ nPos ], UNO_QUERY );
        if (xDic.is())
            SetLanguage_Impl( LanguageTag( xDic->getLocale() ).getLanguageType() );

        // check if dictionary is read-only
        SetDicReadonly_Impl(xDic);
        bool bEnable = !IsDicReadonly_Impl();
        m_xNewReplacePB->set_sensitive( false );
        m_xDeletePB->set_sensitive( false );
        m_xLangFT->set_sensitive( bEnable );
        m_xLangLB->set_sensitive( bEnable );
        ShowWords_Impl( nPos );
    }
    else
    {
        m_xNewReplacePB->set_sensitive(false);
        m_xDeletePB->set_sensitive(false);
    }

    m_xWordED->connect_size_allocate(LINK(this, SvxEditDictionaryDialog, EntrySizeAllocHdl));
    m_xReplaceED->connect_size_allocate(LINK(this, SvxEditDictionaryDialog, EntrySizeAllocHdl));
}

IMPL_LINK_NOARG(SvxEditDictionaryDialog, EntrySizeAllocHdl, const Size&, void)
{
    std::vector<int> aWidths;
    int x, y, width, height;
    if (m_xReplaceED->get_extents_relative_to(*m_pWordsLB, x, y, width, height))
    {
        aWidths.push_back(x);
        m_xDoubleColumnLB->set_column_fixed_widths(aWidths);
    }
}

SvxEditDictionaryDialog::~SvxEditDictionaryDialog()
{
}

void SvxEditDictionaryDialog::SetDicReadonly_Impl(
            Reference< XDictionary > const &xDic )
{
    // enable or disable new and delete button according to file attributes
    bDicIsReadonly = true;
    if (xDic.is())
    {
        Reference< frame::XStorable >  xStor( xDic, UNO_QUERY );
        if (   !xStor.is()              // non persistent dictionary
            || !xStor->hasLocation()    // not yet persistent
            || !xStor->isReadonly() )
        {
            bDicIsReadonly = false;
        }
    }
}

void SvxEditDictionaryDialog::SetLanguage_Impl(LanguageType nLanguage)
{
    // select language
    m_xLangLB->set_active_id(nLanguage);
}

int SvxEditDictionaryDialog::GetLBInsertPos(const OUString &rDicWord)
{
    IntlWrapper aIntlWrapper(SvtSysLocale().GetUILanguageTag());
    const CollatorWrapper* pCollator = aIntlWrapper.getCollator();
    int j;
    int nCount = m_pWordsLB->n_children();
    for (j = 0; j < nCount; ++j)
    {
        OUString aNormEntry( getNormDicEntry_Impl( rDicWord ) );
        sal_Int32 nCmpRes = pCollator->
            compareString( aNormEntry, getNormDicEntry_Impl( m_pWordsLB->get_text(j, 0) ) );
        if (nCmpRes < 0)
            break;
    }

    return j;
}

void SvxEditDictionaryDialog::RemoveDictEntry(int nEntry)
{
    int nLBPos = m_xAllDictsLB->get_active();
    if (nEntry != -1 && nLBPos != -1)
    {
        OUString sTmpShort(m_pWordsLB->get_text(nEntry, 0));

        Reference<XDictionary> xDic = aDics.getConstArray()[nLBPos];
        if (xDic->remove(sTmpShort))  // sal_True on success
        {
            m_pWordsLB->remove(nEntry);
            SelectHdl(*m_pWordsLB);
        }
    }
}

IMPL_LINK_NOARG(SvxEditDictionaryDialog, SelectBookHdl_Impl, weld::ComboBox&, void)
{
    int nPos = m_xAllDictsLB->get_active();
    if (nPos != -1)
    {
        m_xNewReplacePB->set_sensitive( false );
        m_xDeletePB->set_sensitive( false );
        // display dictionary
        ShowWords_Impl( nPos );
        // enable or disable new and delete button according to file attributes
        Reference< XDictionary >  xDic( aDics.getConstArray()[ nPos ], UNO_QUERY );
        if (xDic.is())
            SetLanguage_Impl( LanguageTag( xDic->getLocale() ).getLanguageType() );

        SetDicReadonly_Impl(xDic);
        bool bEnable = !IsDicReadonly_Impl();
        m_xLangFT->set_sensitive( bEnable );
        m_xLangLB->set_sensitive( bEnable );
    }
}

IMPL_LINK_NOARG(SvxEditDictionaryDialog, SelectLangHdl_Impl, weld::ComboBox&, void)
{
    int nDicPos = m_xAllDictsLB->get_active();
    LanguageType nLang = m_xLangLB->get_active_id();
    Reference< XDictionary >  xDic( aDics.getConstArray()[ nDicPos ], UNO_QUERY );
    LanguageType nOldLang = LanguageTag( xDic->getLocale() ).getLanguageType();

    if ( nLang != nOldLang )
    {
        std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(m_xDialog.get(),
                                                      VclMessageType::Question, VclButtonsType::YesNo,
                                                      CuiResId(RID_SVXSTR_CONFIRM_SET_LANGUAGE)));
        OUString sTxt(xBox->get_primary_text());
        sTxt = sTxt.replaceFirst("%1", m_xAllDictsLB->get_active_text());
        xBox->set_primary_text(sTxt);

        if (xBox->run() == RET_YES)
        {
            xDic->setLocale( LanguageTag::convertToLocale( nLang ) );
            bool bNegativ = xDic->getDictionaryType() == DictionaryType_NEGATIVE;

            const OUString sName(
                ::GetDicInfoStr( xDic->getName(),
                                 LanguageTag( xDic->getLocale() ).getLanguageType(),
                                 bNegativ ) );
            m_xAllDictsLB->remove(nDicPos);
            m_xAllDictsLB->insert_text(nDicPos, sName);
            m_xAllDictsLB->set_active(nDicPos);
        }
        else
            SetLanguage_Impl( nOldLang );
    }
}

void SvxEditDictionaryDialog::ShowWords_Impl( sal_uInt16 nId )
{
    Reference< XDictionary >  xDic = aDics.getConstArray()[ nId ];

    weld::WaitObject aWait(m_xDialog.get());

    m_xWordED->set_text(OUString());
    m_xReplaceED->set_text(OUString());

    bool bIsNegative = xDic->getDictionaryType() != DictionaryType_POSITIVE;
    bool bLangNone = LanguageTag(
            xDic->getLocale() ).getLanguageType() == LANGUAGE_NONE;

    // The label is "Replace By" only in negative dictionaries (forbidden
    // words), otherwise "Grammar By" in language-specific dictionaries
    // (where the optional second word is the sample word for
    // the Hunspell based affixation/compounding of the new dictionary word)
    if (bIsNegative)
    {
        m_xReplaceFT->set_label(sReplaceFT_Text);
    } else if (!bLangNone) {
        m_xReplaceFT->set_label(CuiResId(RID_SVXSTR_OPT_GRAMMAR_BY));
    }

    if(bIsNegative || !bLangNone)
    {
        // make controls for replacement text active
        if (!m_xReplaceFT->get_visible())
        {
            m_xReplaceFT->show();
            m_xReplaceED->show();
            m_xSingleColumnLB->hide();
            m_xDoubleColumnLB->show();
            m_pWordsLB = m_xDoubleColumnLB.get();
        }
    }
    else
    {
        // deactivate controls for replacement text
        if (m_xReplaceFT->get_visible())
        {
            m_xReplaceFT->hide();
            m_xReplaceED->hide();
            m_xDoubleColumnLB->hide();
            m_xSingleColumnLB->show();
            m_pWordsLB = m_xSingleColumnLB.get();
        }
    }

    m_pWordsLB->clear();

    Sequence< Reference< XDictionaryEntry >  > aEntries( xDic->getEntries() );
    const Reference< XDictionaryEntry >  *pEntry = aEntries.getConstArray();
    sal_Int32 nCount = aEntries.getLength();
    std::vector<OUString> aSortedDicEntries;
    aSortedDicEntries.reserve(nCount);
    for (sal_Int32 i = 0;  i < nCount;  i++)
    {
        OUString aStr = pEntry[i]->getDictionaryWord();
        if(!pEntry[i]->getReplacementText().isEmpty())
        {
            aStr += "\t" + pEntry[i]->getReplacementText();
        }
        aSortedDicEntries.push_back(aStr);
    }

    IntlWrapper aIntlWrapper(SvtSysLocale().GetUILanguageTag());
    const CollatorWrapper* pCollator = aIntlWrapper.getCollator();
    std::sort(aSortedDicEntries.begin(), aSortedDicEntries.end(),
        [&] (OUString const & lhs, OUString const & rhs)
        {
            sal_Int32 nCmpRes = pCollator->
                compareString( getNormDicEntry_Impl(lhs), getNormDicEntry_Impl( rhs ) );
            return nCmpRes < 0;
        });

    m_pWordsLB->freeze(); // speed up insert
    int nRow = 0;
    for (OUString const & rStr : aSortedDicEntries)
    {
        m_pWordsLB->append_text(rStr.getToken(0, '\t'));
        if (m_pWordsLB == m_xDoubleColumnLB.get())
        {
            OUString sReplace = rStr.getToken(1, '\t');
            m_pWordsLB->set_text(nRow, sReplace, 1);
            ++nRow;
        }
    }
    m_pWordsLB->thaw();

    if (m_pWordsLB->n_children())
    {
        m_pWordsLB->select(0);
        m_pWordsLB->set_cursor(0);
        SelectHdl(*m_pWordsLB);
    }
}

IMPL_LINK(SvxEditDictionaryDialog, SelectHdl, weld::TreeView&, rBox, void)
{
    if (bDoNothing)
        return;

    int nEntry = rBox.get_selected_index();

    if(!bFirstSelect)
    {
        if (nEntry != -1)
        {
            OUString sTmpShort(rBox.get_text(nEntry, 0));
            // without this the cursor is always at the beginning of a word, if the text
            // is set over the ModifyHdl, although you're editing there at the moment
            if (m_xWordED->get_text() != sTmpShort)
                m_xWordED->set_text(sTmpShort);
            if (&rBox == m_xDoubleColumnLB.get())
                m_xReplaceED->set_text(rBox.get_text(nEntry, 1));
        }
    }
    else
        bFirstSelect = false;

    // entries in the list box should exactly correspond to those from the
    // dictionary. Thus:
    m_xNewReplacePB->set_sensitive(false);
    m_xDeletePB->set_sensitive(nEntry != -1 && !IsDicReadonly_Impl());
}

IMPL_LINK(SvxEditDictionaryDialog, NewDelButtonHdl, weld::Button&, rBtn, void)
{
    NewDelHdl(&rBtn);
}

IMPL_LINK(SvxEditDictionaryDialog, NewDelActionHdl, weld::Entry&, rDictEdit, bool)
{
    return NewDelHdl(&rDictEdit);
}

bool SvxEditDictionaryDialog::NewDelHdl(const weld::Widget* pBtn)
{
    if (pBtn == m_xDeletePB.get())
    {
        OUString aStr;

        m_xWordED->set_text(aStr);
        m_xReplaceED->set_text(aStr);
        m_xDeletePB->set_sensitive(false);

        int nEntry = m_pWordsLB->get_selected_index();
        RemoveDictEntry(nEntry);    // remove entry from dic and list-box
    }
    if (pBtn == m_xNewReplacePB.get() || m_xNewReplacePB->get_sensitive())
    {
        int nEntry = m_pWordsLB->get_selected_index();
        OUString aNewWord(m_xWordED->get_text());
        OUString sEntry(aNewWord);
        OUString aReplaceStr(m_xReplaceED->get_text());

        DictionaryError nAddRes = DictionaryError::UNKNOWN;
        int nPos = m_xAllDictsLB->get_active();
        if (nPos != -1 && !aNewWord.isEmpty())
        {
            DBG_ASSERT(nPos < aDics.getLength(), "invalid dictionary index");
            Reference< XDictionary >  xDic( aDics.getConstArray()[ nPos ], UNO_QUERY );
            if (xDic.is())
            {
                // make changes in dic

                bool bIsNegEntry = xDic->getDictionaryType() == DictionaryType_NEGATIVE;

                OUString aRplcText;
                if(!aReplaceStr.isEmpty())
                    aRplcText = aReplaceStr;

                if (nEntry != -1) // entry selected in m_pWordsLB ie action = modify entry
                    xDic->remove(m_pWordsLB->get_text(nEntry, 0));
                // if remove has failed the following add should fail too
                // and thus a warning message should be triggered...

                nAddRes = linguistic::AddEntryToDic( xDic,
                            aNewWord, bIsNegEntry,
                            aRplcText, false );
             }
        }
        if (DictionaryError::NONE != nAddRes)
            SvxDicError(m_xDialog.get(), nAddRes);

        if (DictionaryError::NONE == nAddRes && !sEntry.isEmpty())
        {
            // insert new entry in list-box etc...
            m_pWordsLB->freeze();

            if (nEntry != -1) // entry selected in m_pWordsLB ie action = modify entry
            {
                m_pWordsLB->set_text(nEntry, sEntry);
                if (!aReplaceStr.isEmpty())
                    m_pWordsLB->set_text(nEntry, aReplaceStr, 1);
            }
            else
            {
                nEntry = GetLBInsertPos(aNewWord);
                m_pWordsLB->insert_text(nEntry, sEntry);
                if(!aReplaceStr.isEmpty())
                    m_pWordsLB->set_text(nEntry, aReplaceStr, 1);
            }

            m_pWordsLB->thaw();
            m_pWordsLB->scroll_to_row(nEntry);

            // if the request came from the ReplaceEdit, give focus to the ShortEdit
            if (m_xReplaceED->has_focus())
                m_xWordED->grab_focus();
        }
    }
    else
    {
        // this can only be an enter in one of the two edit fields
        // which means EndDialog() - has to be evaluated in KeyInput
        return false;
    }
    ModifyHdl(*m_xWordED);
    return true;
}

IMPL_LINK(SvxEditDictionaryDialog, ModifyHdl, weld::Entry&, rEdt, void)
{
    OUString rEntry = rEdt.get_text();

    sal_Int32 nWordLen = rEntry.getLength();
    const OUString& rRepString = m_xReplaceED->get_text();

    bool bEnableNewReplace  = false;
    bool bEnableDelete      = false;
    OUString aNewReplaceText  = sNew;

    if (&rEdt == m_xWordED.get())
    {
        if(nWordLen>0)
        {
            bool bFound = false;
            bool bTmpSelEntry=false;
            CDE_RESULT eCmpRes = CDE_DIFFERENT;

            for (int i = 0, nCount = m_pWordsLB->n_children(); i < nCount; ++i)
            {
                OUString aTestStr(m_pWordsLB->get_text(i, 0));
                eCmpRes = cmpDicEntry_Impl( rEntry, aTestStr );
                if(CDE_DIFFERENT != eCmpRes)
                {
                    if(!rRepString.isEmpty())
                        bFirstSelect = true;
                    bDoNothing=true;
                    m_pWordsLB->set_cursor(i);
                    bDoNothing=false;
                    m_xReplaceED->set_text(m_pWordsLB->get_text(i, 1));

                    if (CDE_SIMILAR == eCmpRes)
                    {
                        aNewReplaceText = sModify;
                        bEnableNewReplace = true;
                    }
                    bFound= true;
                    break;
                }
                else if(getNormDicEntry_Impl(aTestStr).indexOf(
                            getNormDicEntry_Impl( rEntry ) ) == 0
                        && !bTmpSelEntry)
                {
                    bDoNothing=true;
                    m_pWordsLB->scroll_to_row(i);
                    bDoNothing=false;
                    bTmpSelEntry=true;

                    aNewReplaceText = sNew;
                    bEnableNewReplace = true;
                }
            }

            if(!bFound)
            {
                m_pWordsLB->unselect_all();
                aNewReplaceText = sNew;
                bEnableNewReplace = true;
            }
            bEnableDelete = CDE_DIFFERENT != eCmpRes;
        }
        else if (m_pWordsLB->n_children() > 0)
        {
            bDoNothing=true;
            m_pWordsLB->scroll_to_row(0);
            bDoNothing=false;
        }
    }
    else if(&rEdt == m_xReplaceED.get())
    {
        OUString aReplaceText;
        OUString aWordText;
        int nFirstSel = m_pWordsLB->get_selected_index();
        if (nFirstSel != -1)  // a m_pWordsLB entry is selected
        {
            aWordText    = m_pWordsLB->get_text(nFirstSel, 0);
            aReplaceText = m_pWordsLB->get_text(nFirstSel, 1);

            aNewReplaceText = sModify;
            bEnableDelete = true;
        }
        bool bIsChange =
                CDE_EQUAL != cmpDicEntry_Impl(m_xWordED->get_text(), aWordText)
             || CDE_EQUAL != cmpDicEntry_Impl(m_xReplaceED->get_text(), aReplaceText);
        if (!m_xWordED->get_text().isEmpty()  &&  bIsChange)
            bEnableNewReplace = true;
    }

    m_xNewReplacePB->set_label(aNewReplaceText);
    m_xNewReplacePB->set_sensitive(bEnableNewReplace && !IsDicReadonly_Impl());
    m_xDeletePB->set_sensitive(bEnableDelete && !IsDicReadonly_Impl());
}

IMPL_STATIC_LINK(SvxEditDictionaryDialog, InsertTextHdl, OUString&, rText, bool)
{
    rText = rText.replaceAll(" ", "");
    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

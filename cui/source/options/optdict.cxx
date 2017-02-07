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
#include <sfx2/sfxuno.hxx>
#include <svl/eitem.hxx>
#include <com/sun/star/frame/XStorable.hpp>
#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>
#include <unotools/intlwrapper.hxx>
#include <vcl/svapp.hxx>
#include <vcl/layout.hxx>
#include <vcl/settings.hxx>
#include <vcl/builderfactory.hxx>
#include <svx/dialogs.hrc>

#include <linguistic/misc.hxx>
#include <cuires.hrc>
#include "optdict.hxx"
#include <dialmgr.hxx>
#include <svx/svxerr.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::linguistic2;
using namespace linguistic;

// static ----------------------------------------------------------------

static const short  NOACTDICT   = -1;

static long nStaticTabs[]=
{
    2,10,71,120
};

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

SvxNewDictionaryDialog::SvxNewDictionaryDialog( vcl::Window* pParent ) :
    ModalDialog( pParent, "OptNewDictionaryDialog" , "cui/ui/optnewdictionarydialog.ui" )
{
    get(pNameEdit,"nameedit");
    get(pLanguageLB,"language");
    get(pExceptBtn,"except");
    get(pOKBtn,"ok");
    // install handler
    pNameEdit->SetModifyHdl(
        LINK( this, SvxNewDictionaryDialog, ModifyHdl_Impl ) );
    pOKBtn->SetClickHdl( LINK( this, SvxNewDictionaryDialog, OKHdl_Impl ) );

    // display languages
    pLanguageLB->SetLanguageList( SvxLanguageListFlags::ALL, true, true );
    pLanguageLB->SelectEntryPos(0);
}

SvxNewDictionaryDialog::~SvxNewDictionaryDialog()
{
    disposeOnce();
}

void SvxNewDictionaryDialog::dispose()
{
    pNameEdit.clear();
    pLanguageLB.clear();
    pExceptBtn.clear();
    pOKBtn.clear();
    ModalDialog::dispose();
}


IMPL_LINK_NOARG(SvxNewDictionaryDialog, OKHdl_Impl, Button*, void)
{

  // add extension for personal dictionaries
    OUString sDict = comphelper::string::stripEnd(pNameEdit->GetText(), ' ') + ".dic";

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
        ScopedVclPtrInstance<MessageDialog>(this, CUI_RESSTR(RID_SVXSTR_OPT_DOUBLE_DICTS), VclMessageType::Info)->Execute();
        pNameEdit->GrabFocus();
        return;
    }

    // create and add
    sal_uInt16 nLang = pLanguageLB->GetSelectLanguage();
    try
    {
        // create new dictionary
        DictionaryType eType = pExceptBtn->IsChecked() ?
                DictionaryType_NEGATIVE : DictionaryType_POSITIVE;
        if (xDicList.is())
        {
            lang::Locale aLocale( LanguageTag::convertToLocale(nLang) );
            OUString aURL( linguistic::GetWritableDictionaryURL( sDict ) );
            xNewDic.set( xDicList->createDictionary( sDict, aLocale, eType, aURL ) , UNO_QUERY );
            xNewDic->setActive( true );
        }
        DBG_ASSERT(xNewDic.is(), "NULL pointer");
    }
    catch(...)
    {
        xNewDic = nullptr;

        // error: couldn't create new dictionary
        SfxErrorContext aContext( ERRCTX_SVX_LINGU_DICTIONARY, OUString(),
            this, RID_SVXERRCTX, &CUI_MGR() );
        ErrorHandler::HandleError( *new StringErrorInfo(
                ERRCODE_SVX_LINGU_DICT_NOTWRITEABLE, sDict ) );

        EndDialog();
    }

    if (xDicList.is() && xNewDic.is())
    {
        xDicList->addDictionary( Reference< XDictionary > ( xNewDic, UNO_QUERY ) );

        // refresh list of dictionaries
        //! dictionaries may have been added/removed elsewhere too.
        aDics = xDicList->getDictionaries();
    }


    EndDialog( RET_OK );
}


IMPL_LINK_NOARG(SvxNewDictionaryDialog, ModifyHdl_Impl, Edit&, void)
{
    if ( !pNameEdit->GetText().isEmpty() )
        pOKBtn->Enable();
    else
        pOKBtn->Disable();
}

// class SvxEditDictionaryDialog -------------------------------------------

VCL_BUILDER_FACTORY_ARGS(SvxDictEdit, WB_LEFT|WB_VCENTER|WB_BORDER|WB_3DLOOK)

SvxEditDictionaryDialog::SvxEditDictionaryDialog(
            vcl::Window* pParent,
            const OUString& rName ) :

    ModalDialog( pParent, "EditDictionaryDialog" ,"cui/ui/editdictionarydialog.ui" ),

    sModify         (CUI_RESSTR(STR_MODIFY)),
    nOld            ( NOACTDICT ),
    bFirstSelect    (true),
    bDoNothing      (false),
    bDicIsReadonly  (false)

{
    get(pAllDictsLB,"book");
    get(pLangFT,"lang_label");
    get(pLangLB,"lang");

    get(pWordED,"word");
    get(pReplaceFT,"replace_label");
    get(pReplaceED,"replace");
    get(pWordsLB,"words");
    pWordsLB->set_height_request(pWordsLB->GetTextHeight() * 8);
    get(pNewReplacePB,"newreplace");
    get(pDeletePB,"delete");

    sNew=pNewReplacePB->GetText();
    if (LinguMgr::GetDictionaryList().is())
        aDics = LinguMgr::GetDictionaryList()->getDictionaries();

    pWordsLB->SetSelectHdl(LINK(this, SvxEditDictionaryDialog, SelectHdl));
    pWordsLB->SetTabs(nStaticTabs);

    //! we use an algorithm of our own to insert elements sorted
    pWordsLB->SetStyle(pWordsLB->GetStyle()|/*WB_SORT|*/WB_HSCROLL|WB_CLIPCHILDREN);


    nWidth=pWordED->GetSizePixel().Width();
    // install handler
    pNewReplacePB->SetClickHdl(
        LINK( this, SvxEditDictionaryDialog, NewDelButtonHdl));
    pDeletePB->SetClickHdl(
        LINK( this, SvxEditDictionaryDialog, NewDelButtonHdl));

    pLangLB->SetSelectHdl(
        LINK( this, SvxEditDictionaryDialog, SelectLangHdl_Impl ) );
    pAllDictsLB->SetSelectHdl(
        LINK( this, SvxEditDictionaryDialog, SelectBookHdl_Impl ) );

    pWordED->SetModifyHdl(LINK(this, SvxEditDictionaryDialog, ModifyHdl));
    pReplaceED->SetModifyHdl(LINK(this, SvxEditDictionaryDialog, ModifyHdl));
    pWordED->SetActionHdl(LINK(this, SvxEditDictionaryDialog, NewDelActionHdl));
    pReplaceED->SetActionHdl(LINK(this, SvxEditDictionaryDialog, NewDelActionHdl));

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
            pAllDictsLB->InsertEntry( aTxt );

            if (rName == aDicName)
                aLookUpEntry = aTxt;
        }
    }

    pLangLB->SetLanguageList( SvxLanguageListFlags::ALL, true, true );

    pReplaceED->SetSpaces(true);
    pWordED->SetSpaces(true);

    if ( nCount > 0 )
    {
        pAllDictsLB->SelectEntry( aLookUpEntry );
        sal_Int32 nPos = pAllDictsLB->GetSelectEntryPos();

        if ( nPos == LISTBOX_ENTRY_NOTFOUND )
        {
            nPos = 0;
            pAllDictsLB->SelectEntryPos( nPos );
        }
        Reference< XDictionary >  xDic;
        if (nPos != LISTBOX_ENTRY_NOTFOUND)
            xDic.set( aDics.getConstArray()[ nPos ], UNO_QUERY );
        if (xDic.is())
            SetLanguage_Impl( LanguageTag( xDic->getLocale() ).getLanguageType() );

        // check if dictionary is read-only
        SetDicReadonly_Impl(xDic);
        bool bEnable = !IsDicReadonly_Impl();
        pNewReplacePB->Enable( false );
        pDeletePB->Enable( false );
        pLangFT->Enable( bEnable );
        pLangLB->Enable( bEnable );
        ShowWords_Impl( nPos );

    }
    else
    {
        pNewReplacePB->Disable();
        pDeletePB->Disable();
    }
}

SvxEditDictionaryDialog::~SvxEditDictionaryDialog()
{
    disposeOnce();
}

void SvxEditDictionaryDialog::dispose()
{
    pAllDictsLB.clear();
    pLangFT.clear();
    pLangLB.clear();
    pWordED.clear();
    pReplaceFT.clear();
    pReplaceED.clear();
    pWordsLB.clear();
    pNewReplacePB.clear();
    pDeletePB.clear();
    ModalDialog::dispose();
}


/*
void SvxEditDictionaryDialog::Paint( const Rectangle& rRect )
{
    ModalDialog::Paint(rRect );

    //Rectangle aRect(aEditDictsBox.GetPosPixel(),aEditDictsBox.GetSizePixel());

    sal_uInt16 nStyle=DrawButtonFlags::NoFill;
//    aDecoView.DrawButton( aRect, nStyle);
}
*/


void SvxEditDictionaryDialog::SetDicReadonly_Impl(
            Reference< XDictionary >  &xDic )
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


void SvxEditDictionaryDialog::SetLanguage_Impl( util::Language nLanguage )
{
    // select language
    pLangLB->SelectLanguage( nLanguage );
}

sal_uLong SvxEditDictionaryDialog::GetLBInsertPos(const OUString &rDicWord)
{
    sal_uLong nPos = TREELIST_ENTRY_NOTFOUND;

    IntlWrapper aIntlWrapper( Application::GetSettings().GetLanguageTag() );
    const CollatorWrapper* pCollator = aIntlWrapper.getCollator();
    sal_uLong j;
    for( j = 0; j < pWordsLB->GetEntryCount(); j++ )
    {
        SvTreeListEntry* pEntry = pWordsLB->GetEntry(j);
        DBG_ASSERT( pEntry, "NULL pointer");
        OUString aNormEntry( getNormDicEntry_Impl( rDicWord ) );
        sal_Int32 nCmpRes = pCollator->
            compareString( aNormEntry, getNormDicEntry_Impl( SvTabListBox::GetEntryText(pEntry, 0) ) );
        if (nCmpRes < 0)
            break;
    }
    if (j < pWordsLB->GetEntryCount())   // entry found?
        nPos = j;

    return nPos;
}

void SvxEditDictionaryDialog::RemoveDictEntry(SvTreeListEntry* pEntry)
{
    sal_Int32 nLBPos = pAllDictsLB->GetSelectEntryPos();

    if ( pEntry != nullptr && nLBPos != LISTBOX_ENTRY_NOTFOUND )
    {
        OUString sTmpShort(SvTabListBox::GetEntryText(pEntry, 0));

        Reference< XDictionary >  xDic = aDics.getConstArray()[ nLBPos ];
        if (xDic->remove( sTmpShort ))  // sal_True on success
        {
            pWordsLB->GetModel()->Remove(pEntry);
        }
    }
}


IMPL_LINK_NOARG(SvxEditDictionaryDialog, SelectBookHdl_Impl, ListBox&, void)
{
    sal_Int32 nPos = pAllDictsLB->GetSelectEntryPos();

    if ( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        pNewReplacePB->Enable( false );
        pDeletePB->Enable( false );
        // display dictionary
        ShowWords_Impl( nPos );
        // enable or disable new and delete button according to file attributes
        Reference< XDictionary >  xDic( aDics.getConstArray()[ nPos ], UNO_QUERY );
        if (xDic.is())
            SetLanguage_Impl( LanguageTag( xDic->getLocale() ).getLanguageType() );

        SetDicReadonly_Impl(xDic);
        bool bEnable = !IsDicReadonly_Impl();
        pLangFT->Enable( bEnable );
        pLangLB->Enable( bEnable );
    }
}


IMPL_LINK_NOARG(SvxEditDictionaryDialog, SelectLangHdl_Impl, ListBox&, void)
{
    sal_Int32 nDicPos = pAllDictsLB->GetSelectEntryPos();
    sal_Int32 nLang = pLangLB->GetSelectLanguage();
    Reference< XDictionary >  xDic( aDics.getConstArray()[ nDicPos ], UNO_QUERY );
    sal_Int16 nOldLang = LanguageTag( xDic->getLocale() ).getLanguageType();

    if ( nLang != nOldLang )
    {
        ScopedVclPtrInstance< MessageDialog > aBox(this, CUI_RES( RID_SVXSTR_CONFIRM_SET_LANGUAGE), VclMessageType::Question, VclButtonsType::YesNo);
        OUString sTxt(aBox->get_primary_text());
        sTxt = sTxt.replaceFirst( "%1", pAllDictsLB->GetSelectEntry() );
        aBox->set_primary_text(sTxt);

        if ( aBox->Execute() == RET_YES )
        {
            xDic->setLocale( LanguageTag::convertToLocale( nLang ) );
            bool bNegativ = xDic->getDictionaryType() == DictionaryType_NEGATIVE;

            const OUString sName(
                ::GetDicInfoStr( xDic->getName(),
                                 LanguageTag( xDic->getLocale() ).getLanguageType(),
                                 bNegativ ) );
            pAllDictsLB->RemoveEntry( nDicPos );
            pAllDictsLB->InsertEntry( sName, nDicPos );
            pAllDictsLB->SelectEntryPos( nDicPos );
        }
        else
            SetLanguage_Impl( nOldLang );
    }
}


void SvxEditDictionaryDialog::ShowWords_Impl( sal_uInt16 nId )
{
    Reference< XDictionary >  xDic = aDics.getConstArray()[ nId ];

    nOld = nId;
    EnterWait();

    OUString aStr;

    pWordED->SetText(aStr);
    pReplaceED->SetText(aStr);

    if(xDic->getDictionaryType() != DictionaryType_POSITIVE)
    {
        nStaticTabs[0]=2;

        // make controls for replacement text active
        if(!pReplaceFT->IsVisible())
        {
            Size aSize=pWordED->GetSizePixel();
            aSize.Width()=nWidth;
            pWordED->SetSizePixel(aSize);
            pReplaceFT->Show();
            pReplaceED->Show();
        }
    }
    else
    {
        nStaticTabs[0]=1;

        // deactivate controls for replacement text
        if(pReplaceFT->IsVisible())
        {
            Size aSize=pWordED->GetSizePixel();
            aSize.Width()=pWordsLB->GetSizePixel().Width();
            pWordED->SetSizePixel(aSize);
            pReplaceFT->Hide();
            pReplaceED->Hide();
        }

    }

    pWordsLB->SetTabs(nStaticTabs);
    pWordsLB->Clear();

    Sequence< Reference< XDictionaryEntry >  > aEntries( xDic->getEntries() );
    const Reference< XDictionaryEntry >  *pEntry = aEntries.getConstArray();
    sal_Int32 nCount = aEntries.getLength();

    for (sal_Int32 i = 0;  i < nCount;  i++)
    {
        aStr = pEntry[i]->getDictionaryWord();
        sal_uLong nPos = GetLBInsertPos( aStr );
        if(pEntry[i]->isNegative())
        {
            aStr += "\t" + pEntry[i]->getReplacementText();
        }
        pWordsLB->InsertEntry(aStr, nullptr, false, nPos == TREELIST_ENTRY_NOTFOUND ?  TREELIST_APPEND : nPos);
    }

    if (pWordsLB->GetEntryCount())
    {
        pWordED->SetText( pWordsLB->GetEntryText((sal_uLong)0, 0) );
        pReplaceED->SetText( pWordsLB->GetEntryText((sal_uLong)0, 1) );
    }

    LeaveWait();
}


IMPL_LINK(SvxEditDictionaryDialog, SelectHdl, SvTreeListBox*, pBox, void)
{
    if(!bDoNothing)
    {
        if(!bFirstSelect)
        {
            SvTreeListEntry* pEntry = pBox->FirstSelected();
            OUString sTmpShort(SvTabListBox::GetEntryText(pEntry, 0));
            // without this the cursor is always at the beginning of a word, if the text
            // is set over the ModifyHdl, although you're editing there at the moment
            if(pWordED->GetText() != sTmpShort)
                pWordED->SetText(sTmpShort);
            pReplaceED->SetText(SvTabListBox::GetEntryText(pEntry, 1));
        }
        else
            bFirstSelect = false;

        // entries in the list box should exactly correspond to those from the
        // dictionary. Thus:
        pNewReplacePB->Enable(false);
        pDeletePB->Enable( true && !IsDicReadonly_Impl() );
    }
};


IMPL_LINK(SvxEditDictionaryDialog, NewDelButtonHdl, Button*, pBtn, void)
{
    NewDelHdl(static_cast<PushButton*>(pBtn));
}

IMPL_LINK(SvxEditDictionaryDialog, NewDelActionHdl, SvxDictEdit&, rDictEdit, bool)
{
    return NewDelHdl(&rDictEdit);
}
bool SvxEditDictionaryDialog::NewDelHdl(void* pBtn)
{
    SvTreeListEntry* pEntry = pWordsLB->FirstSelected();

    if(pBtn == pDeletePB)
    {
        DBG_ASSERT(pEntry, "keine Eintrag selektiert");
        OUString aStr;

        pWordED->SetText(aStr);
        pReplaceED->SetText(aStr);
        pDeletePB->Disable();

        RemoveDictEntry(pEntry);    // remove entry from dic and list-box
    }
    if(pBtn == pNewReplacePB || pNewReplacePB->IsEnabled())
    {
        SvTreeListEntry* _pEntry = pWordsLB->FirstSelected();
        OUString aNewWord(pWordED->GetText());
        OUString sEntry(aNewWord);
        OUString aReplaceStr(pReplaceED->GetText());

        DictionaryError nAddRes = DictionaryError::UNKNOWN;
        sal_Int32 nPos = pAllDictsLB->GetSelectEntryPos();
        if ( nPos != LISTBOX_ENTRY_NOTFOUND && !aNewWord.isEmpty())
        {
            DBG_ASSERT(nPos < aDics.getLength(), "invalid dictionary index");
            Reference< XDictionary >  xDic( aDics.getConstArray()[ nPos ], UNO_QUERY );
            if (xDic.is())
            {
                // make changes in dic

                //! ...IsVisible should reflect whether the dictionary is a negativ
                //! or not (hopefully...)
                bool bIsNegEntry = pReplaceFT->IsVisible();
                OUString aRplcText;
                if(bIsNegEntry)
                    aRplcText = aReplaceStr;

                if (_pEntry) // entry selected in pWordsLB ie action = modify entry
                    xDic->remove( SvTabListBox::GetEntryText( _pEntry, 0 ) );
                // if remove has failed the following add should fail too
                // and thus a warning message should be triggered...

                nAddRes = linguistic::AddEntryToDic( xDic,
                            aNewWord, bIsNegEntry,
                            aRplcText, LanguageTag( xDic->getLocale() ).getLanguageType(), false );
             }
        }
        if (DictionaryError::NONE != nAddRes)
            SvxDicError( this, nAddRes );

        if(DictionaryError::NONE == nAddRes && !sEntry.isEmpty())
        {
            // insert new entry in list-box etc...

            pWordsLB->SetUpdateMode(false);
            sal_uLong _nPos = TREELIST_ENTRY_NOTFOUND;

            if(pReplaceFT->IsVisible())
            {
                sEntry += "\t" + aReplaceStr;
            }

            SvTreeListEntry* pNewEntry = nullptr;
            if(_pEntry) // entry selected in pWordsLB ie action = modify entry
            {
                pWordsLB->SetEntryText( sEntry, _pEntry );
                pNewEntry = _pEntry;
            }
            else
            {
                _nPos = GetLBInsertPos( aNewWord );
                SvTreeListEntry* pInsEntry = pWordsLB->InsertEntry(sEntry, nullptr, false,
                            _nPos == TREELIST_ENTRY_NOTFOUND ? TREELIST_APPEND : _nPos);
                pNewEntry = pInsEntry;
            }

            pWordsLB->MakeVisible( pNewEntry );
            pWordsLB->SetUpdateMode(true);
            // if the request came from the ReplaceEdit, give focus to the ShortEdit
            if(pReplaceED->HasFocus())
                pWordED->GrabFocus();
        }
    }
    else
    {
        // this can only be an enter in one of the two edit fields
        // which means EndDialog() - has to be evaluated in KeyInput
        return false;
    }
    ModifyHdl(*pWordED);
    return true;
}


IMPL_LINK(SvxEditDictionaryDialog, ModifyHdl, Edit&, rEdt, void)
{
    OUString rEntry = rEdt.GetText();

    sal_Int32 nWordLen = rEntry.getLength();
    const OUString& rRepString = pReplaceED->GetText();

    bool bEnableNewReplace  = false;
    bool bEnableDelete      = false;
    OUString aNewReplaceText  = sNew;

    if(&rEdt == pWordED)
    {
        if(nWordLen>0)
        {
            bool bFound = false;
            bool bTmpSelEntry=false;
            CDE_RESULT eCmpRes = CDE_DIFFERENT;

            for(sal_uLong i = 0; i < pWordsLB->GetEntryCount(); i++)
            {
                SvTreeListEntry*  pEntry = pWordsLB->GetEntry( i );
                OUString aTestStr( SvTabListBox::GetEntryText(pEntry, 0) );
                eCmpRes = cmpDicEntry_Impl( rEntry, aTestStr );
                if(CDE_DIFFERENT != eCmpRes)
                {
                    if(!rRepString.isEmpty())
                        bFirstSelect = true;
                    bDoNothing=true;
                    pWordsLB->SetCurEntry(pEntry);
                    bDoNothing=false;
                    pReplaceED->SetText(SvTabListBox::GetEntryText(pEntry, 1));

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
                    pWordsLB->MakeVisible(pEntry);
                    bDoNothing=false;
                    bTmpSelEntry=true;

                    aNewReplaceText = sNew;
                    bEnableNewReplace = true;
                }
            }

            if(!bFound)
            {
                pWordsLB->SelectAll(false);

                aNewReplaceText = sNew;
                bEnableNewReplace = true;
            }
            bEnableDelete = CDE_DIFFERENT != eCmpRes;
        }
        else if(pWordsLB->GetEntryCount()>0)
        {
            SvTreeListEntry*  pEntry = pWordsLB->GetEntry( 0 );
            bDoNothing=true;
            pWordsLB->MakeVisible(pEntry);
            bDoNothing=false;
        }
    }
    else if(&rEdt == pReplaceED)
    {
        OUString aReplaceText;
        OUString aWordText;
        SvTreeListEntry* pFirstSel = pWordsLB->FirstSelected();
        if (pFirstSel)  // a pWordsLB entry is selected
        {
            aWordText    = SvTabListBox::GetEntryText( pFirstSel, 0 );
            aReplaceText = SvTabListBox::GetEntryText( pFirstSel, 1 );

            aNewReplaceText = sModify;
            bEnableDelete = true;
        }
        bool bIsChange =
                CDE_EQUAL != cmpDicEntry_Impl(pWordED->GetText(), aWordText)
             || CDE_EQUAL != cmpDicEntry_Impl(pReplaceED->GetText(), aReplaceText);
        if (!pWordED->GetText().isEmpty()  &&  bIsChange)
            bEnableNewReplace = true;
    }

    pNewReplacePB->SetText( aNewReplaceText );
    pNewReplacePB->Enable( bEnableNewReplace && !IsDicReadonly_Impl() );
    pDeletePB->Enable( bEnableDelete     && !IsDicReadonly_Impl() );
}


//SvxDictEdit

void SvxDictEdit::KeyInput( const KeyEvent& rKEvt )
{
    const vcl::KeyCode aKeyCode = rKEvt.GetKeyCode();
    const sal_uInt16 nModifier = aKeyCode.GetModifier();
    if( aKeyCode.GetCode() == KEY_RETURN )
    {
        // if there's nothing done on enter, call the
        // base class after all to close the dialog
        if(!nModifier && !aActionLink.Call(*this))
                 Edit::KeyInput(rKEvt);
    }
    else if(bSpaces || aKeyCode.GetCode() != KEY_SPACE)
        Edit::KeyInput(rKEvt);
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

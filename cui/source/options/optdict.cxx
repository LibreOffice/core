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

#include <tools/shl.hxx>
#include <editeng/unolingu.hxx>
#include <svx/dlgutil.hxx>
#include <sfx2/sfxuno.hxx>
#include <svl/eitem.hxx>
#include <com/sun/star/frame/XStorable.hpp>
#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>
#include <unotools/intlwrapper.hxx>
#include <vcl/svapp.hxx>
#include <vcl/msgbox.hxx>
#include <svx/dialogs.hrc>

#include <linguistic/misc.hxx>
#include <cuires.hrc>
#include "optdict.hrc"
#include "optdict.hxx"
#include <dialmgr.hxx>
#include <svx/svxerr.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::linguistic2;

// static ----------------------------------------------------------------

static const short  NOACTDICT   = -1;

static long nStaticTabs[]=
{
    2,10,71,120
};

// static function -------------------------------------------------------

static String getNormDicEntry_Impl(const OUString &rText)
{
    OUString aTmp(comphelper::string::stripEnd(rText, '.'));
    return comphelper::string::remove(aTmp, '=');
}

// Compare Dictionary Entry  result
enum CDE_RESULT { CDE_EQUAL, CDE_SIMILAR, CDE_DIFFERENT };

static CDE_RESULT cmpDicEntry_Impl( const String &rText1, const String &rText2 )
{
    CDE_RESULT eRes = CDE_DIFFERENT;

    if (rText1 == rText2)
        eRes = CDE_EQUAL;
    else
    {   // similar = equal up to trailing '.' and hyphenation positions
        // marked with '='
        if (getNormDicEntry_Impl( rText1 ) == getNormDicEntry_Impl( rText2 ))
            eRes = CDE_SIMILAR;
    }

    return eRes;
}

// class SvxNewDictionaryDialog -------------------------------------------

SvxNewDictionaryDialog::SvxNewDictionaryDialog( Window* pParent,
        Reference< XSpellChecker1 >  &xSpl ) :

    ModalDialog( pParent, CUI_RES( RID_SFXDLG_NEWDICT ) ),

    aNewDictBox     ( this, CUI_RES( GB_NEWDICT ) ),
    aNameText       ( this, CUI_RES( FT_DICTNAME ) ),
    aNameEdit       ( this, CUI_RES( ED_DICTNAME ) ),
    aLanguageText   ( this, CUI_RES( FT_DICTLANG ) ),
    aLanguageLB     ( this, CUI_RES( LB_DICTLANG ) ),
    aExceptBtn      ( this, CUI_RES( BTN_EXCEPT ) ),
    aOKBtn          ( this, CUI_RES( BTN_NEWDICT_OK ) ),
    aCancelBtn      ( this, CUI_RES( BTN_NEWDICT_ESC ) ),
    aHelpBtn        ( this, CUI_RES( BTN_NEWDICT_HLP ) ),
    xSpell( xSpl )
{
    // install handler
    aNameEdit.SetModifyHdl(
        LINK( this, SvxNewDictionaryDialog, ModifyHdl_Impl ) );
    aOKBtn.SetClickHdl( LINK( this, SvxNewDictionaryDialog, OKHdl_Impl ) );

    // display languages
    aLanguageLB.SetLanguageList( LANG_LIST_ALL, sal_True, sal_True );
    aLanguageLB.SelectEntryPos(0);

    aNameText.SetAccessibleRelationMemberOf( &aNewDictBox );
    aNameEdit.SetAccessibleRelationMemberOf( &aNewDictBox );
    aLanguageText.SetAccessibleRelationMemberOf( &aNewDictBox );
    aLanguageLB.SetAccessibleRelationMemberOf( &aNewDictBox );

    FreeResource();
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(SvxNewDictionaryDialog, OKHdl_Impl)
{
    OUString sDict = comphelper::string::stripEnd(aNameEdit.GetText(), ' ');
    // add extension for personal dictionaries
    sDict += ".dic";

    Reference< XSearchableDictionaryList >  xDicList( SvxGetDictionaryList() );

    Sequence< Reference< XDictionary >  > aDics;
    if (xDicList.is())
        aDics = xDicList->getDictionaries();
    const Reference< XDictionary >  *pDic = aDics.getConstArray();
    sal_Int32 nCount = (sal_uInt16) aDics.getLength();

    sal_Bool bFound = sal_False;
    sal_uInt16 i;
    for (i = 0; !bFound && i < nCount; ++i )
        if ( sDict.equalsIgnoreAsciiCase( pDic[i]->getName()) )
            bFound = sal_True;

    if ( bFound )
    {
        // duplicate names?
        InfoBox( this, CUI_RESSTR( RID_SVXSTR_OPT_DOUBLE_DICTS ) ).Execute();
        aNameEdit.GrabFocus();
        return 0;
    }

    // create and add
    sal_uInt16 nLang = aLanguageLB.GetSelectLanguage();
    try
    {
        // create new dictionary
        DictionaryType eType = aExceptBtn.IsChecked() ?
                DictionaryType_NEGATIVE : DictionaryType_POSITIVE;
        if (xDicList.is())
        {
            lang::Locale aLocale( LanguageTag::convertToLocale(nLang) );
            String aURL( linguistic::GetWritableDictionaryURL( sDict ) );
            xNewDic = Reference< XDictionary > (
                    xDicList->createDictionary( sDict, aLocale, eType, aURL ) , UNO_QUERY );
            xNewDic->setActive( sal_True );
        }
        DBG_ASSERT(xNewDic.is(), "NULL pointer");
    }
    catch(...)
    {
        xNewDic = NULL;

        // error: couldn't create new dictionary
        SfxErrorContext aContext( ERRCTX_SVX_LINGU_DICTIONARY, String(),
            this, RID_SVXERRCTX, &CUI_MGR() );
        ErrorHandler::HandleError( *new StringErrorInfo(
                ERRCODE_SVX_LINGU_DICT_NOTWRITEABLE, sDict ) );

        EndDialog( RET_CANCEL );
    }

    if (xDicList.is() && xNewDic.is())
    {
        xDicList->addDictionary( Reference< XDictionary > ( xNewDic, UNO_QUERY ) );

        // refresh list of dictionaries
        //! dictionaries may have been added/removed elsewhere too.
        aDics = xDicList->getDictionaries();
    }
    pDic = aDics.getConstArray();
    nCount = (sal_uInt16) aDics.getLength();


    EndDialog( RET_OK );
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG_INLINE_START(SvxNewDictionaryDialog, ModifyHdl_Impl)
{
    if ( !aNameEdit.GetText().isEmpty() )
        aOKBtn.Enable();
    else
        aOKBtn.Disable();
    return 0;
}
IMPL_LINK_NOARG_INLINE_END(SvxNewDictionaryDialog, ModifyHdl_Impl)

//==========================================================================
//
// class SvxEditDictionaryDialog -------------------------------------------
//
//==========================================================================

SvxEditDictionaryDialog::SvxEditDictionaryDialog(
            Window* pParent,
            const String& rName,
            Reference< XSpellChecker1 >  &xSpl ) :

    ModalDialog( pParent, CUI_RES( RID_SFXDLG_EDITDICT ) ),

    aBookFT         ( this, CUI_RES( FT_BOOK ) ),
    aAllDictsLB     ( this, CUI_RES( LB_ALLDICTS ) ),
    aLangFT         ( this, CUI_RES( FT_DICTLANG ) ),
    aLangLB         ( this, CUI_RES( LB_DICTLANG ) ),

    aWordFT         ( this, CUI_RES( FT_WORD ) ),
    aWordED         ( this, CUI_RES( ED_WORD ) ),
    aReplaceFT      ( this, CUI_RES( FT_REPLACE ) ),
    aReplaceED      ( this, CUI_RES( ED_REPLACE ) ),
    aWordsLB        ( this, CUI_RES( TLB_REPLACE ) ),
    aNewReplacePB   ( this, CUI_RES( PB_NEW_REPLACE ) ),
    aDeletePB       ( this, CUI_RES( PB_DELETE_REPLACE ) ),
    aEditDictsBox   ( this, CUI_RES( GB_EDITDICTS ) ),
    aHelpBtn        ( this, CUI_RES( BTN_EDITHELP ) ),
    aCloseBtn       ( this, CUI_RES( BTN_EDITCLOSE ) ),
    sModify         (CUI_RES(STR_MODIFY)),
    sNew            (aNewReplacePB.GetText()),
    aDecoView       ( this),
    xSpell          ( xSpl ),
    nOld            ( NOACTDICT ),
    bFirstSelect    (sal_True),
    bDoNothing      (sal_False)

{
    if (SvxGetDictionaryList().is())
        aDics = SvxGetDictionaryList()->getDictionaries();

    aWordsLB.SetSelectHdl(LINK(this, SvxEditDictionaryDialog, SelectHdl));
    aWordsLB.SetTabs(nStaticTabs);

    //! we use an algorithm of our own to insert elements sorted
    aWordsLB.SetStyle(aWordsLB.GetStyle()|/*WB_SORT|*/WB_HSCROLL|WB_CLIPCHILDREN);


    nWidth=aWordED.GetSizePixel().Width();
    // install handler
    aNewReplacePB.SetClickHdl(
        LINK( this, SvxEditDictionaryDialog, NewDelHdl));
    aDeletePB.SetClickHdl(
        LINK( this, SvxEditDictionaryDialog, NewDelHdl));

    aLangLB.SetSelectHdl(
        LINK( this, SvxEditDictionaryDialog, SelectLangHdl_Impl ) );
    aAllDictsLB.SetSelectHdl(
        LINK( this, SvxEditDictionaryDialog, SelectBookHdl_Impl ) );

    aWordED.SetModifyHdl(LINK(this, SvxEditDictionaryDialog, ModifyHdl));
    aReplaceED.SetModifyHdl(LINK(this, SvxEditDictionaryDialog, ModifyHdl));
    aWordED.SetActionHdl(LINK(this, SvxEditDictionaryDialog, NewDelHdl));
    aReplaceED.SetActionHdl(LINK(this, SvxEditDictionaryDialog, NewDelHdl));

    // fill listbox with all available WB's
    const Reference< XDictionary >  *pDic = aDics.getConstArray();
    sal_Int32 nCount = aDics.getLength();

    String aLookUpEntry;
    for ( sal_Int32 i = 0; i < nCount; ++i )
    {
        Reference< XDictionary >  xDic( pDic[i], UNO_QUERY );
        if (xDic.is())
        {
            bool bNegative = xDic->getDictionaryType() == DictionaryType_NEGATIVE;
            String aDicName( xDic->getName() );
            const String aTxt( ::GetDicInfoStr( aDicName,
                        LanguageTag( xDic->getLocale() ).getLanguageType(), bNegative ) );
            aAllDictsLB.InsertEntry( aTxt );

            if (rName == aDicName)
                aLookUpEntry = aTxt;
        }
    }

    aLangLB.SetLanguageList( LANG_LIST_ALL, sal_True, sal_True );

    aReplaceED.SetSpaces(sal_True);
    aWordED.SetSpaces(sal_True);

    if ( nCount > 0 )
    {
        aAllDictsLB.SelectEntry( aLookUpEntry );
        sal_uInt16 nPos = aAllDictsLB.GetSelectEntryPos();

        if ( nPos == LISTBOX_ENTRY_NOTFOUND )
        {
            nPos = 0;
            aAllDictsLB.SelectEntryPos( nPos );
        }
        Reference< XDictionary >  xDic;
        if (nPos != LISTBOX_ENTRY_NOTFOUND)
            xDic = Reference< XDictionary > ( aDics.getConstArray()[ nPos ], UNO_QUERY );
        if (xDic.is())
            SetLanguage_Impl( LanguageTag( xDic->getLocale() ).getLanguageType() );

        // check if dictionary is read-only
        SetDicReadonly_Impl(xDic);
        sal_Bool bEnable = !IsDicReadonly_Impl();
        aNewReplacePB   .Enable( sal_False );
        aDeletePB       .Enable( sal_False );
        aLangFT.Enable( bEnable );
        aLangLB.Enable( bEnable );
        ShowWords_Impl( nPos );

    }
    else
    {
        aNewReplacePB.Disable();
        aDeletePB    .Disable();
    }
    FreeResource();
}

// -----------------------------------------------------------------------

SvxEditDictionaryDialog::~SvxEditDictionaryDialog()
{
}

// -----------------------------------------------------------------------

void SvxEditDictionaryDialog::Paint( const Rectangle& rRect )
{
    ModalDialog::Paint(rRect );

    Rectangle aRect(aEditDictsBox.GetPosPixel(),aEditDictsBox.GetSizePixel());

    sal_uInt16 nStyle=BUTTON_DRAW_NOFILL;
    aDecoView.DrawButton( aRect, nStyle);
}

// -----------------------------------------------------------------------

void SvxEditDictionaryDialog::SetDicReadonly_Impl(
            Reference< XDictionary >  &xDic )
{
    // enable or disable new and delete button according to file attributes
    bDicIsReadonly = sal_True;
    if (xDic.is())
    {
        Reference< frame::XStorable >  xStor( xDic, UNO_QUERY );
        if (   !xStor.is()              // non persistent dictionary
            || !xStor->hasLocation()    // not yet persistent
            || !xStor->isReadonly() )
        {
            bDicIsReadonly = sal_False;
        }
    }
}

// -----------------------------------------------------------------------

void SvxEditDictionaryDialog::SetLanguage_Impl( util::Language nLanguage )
{
    // select language
    aLangLB.SelectLanguage( nLanguage );
}

sal_uInt16 SvxEditDictionaryDialog::GetLBInsertPos(const String &rDicWord)
{
    sal_uInt16 nPos = USHRT_MAX;

    IntlWrapper aIntlWrapper( Application::GetSettings().GetLanguageTag() );
    const CollatorWrapper* pCollator = aIntlWrapper.getCollator();
    sal_uInt16 j;
    for( j = 0; j < aWordsLB.GetEntryCount(); j++ )
    {
        SvTreeListEntry* pEntry = aWordsLB.GetEntry(j);
        DBG_ASSERT( pEntry, "NULL pointer");
        String aNormEntry( getNormDicEntry_Impl( rDicWord ) );
        StringCompare eCmpRes = (StringCompare)pCollator->
            compareString( aNormEntry, getNormDicEntry_Impl( aWordsLB.GetEntryText(pEntry, 0) ) );
        if( COMPARE_LESS == eCmpRes )
            break;
    }
    if (j < aWordsLB.GetEntryCount())   // entry found?
        nPos = j;

    return nPos;
}

void SvxEditDictionaryDialog::RemoveDictEntry(SvTreeListEntry* pEntry)
{
    sal_uInt16 nLBPos = aAllDictsLB.GetSelectEntryPos();

    if ( pEntry != NULL && nLBPos != LISTBOX_ENTRY_NOTFOUND )
    {
        String sTmpShort(aWordsLB.GetEntryText(pEntry, 0));

        Reference< XDictionary >  xDic = aDics.getConstArray()[ nLBPos ];
        if (xDic->remove( sTmpShort ))  // sal_True on success
        {
            aWordsLB.GetModel()->Remove(pEntry);
        }
    }
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(SvxEditDictionaryDialog, SelectBookHdl_Impl)
{
    sal_uInt16 nPos = aAllDictsLB.GetSelectEntryPos();

    if ( nPos != LISTBOX_ENTRY_NOTFOUND )
    {
        aNewReplacePB.Enable( sal_False );
        aDeletePB    .Enable( sal_False );
        // display dictionary
        ShowWords_Impl( nPos );
        // enable or disable new and delete button according to file attributes
        Reference< XDictionary >  xDic( aDics.getConstArray()[ nPos ], UNO_QUERY );
        if (xDic.is())
            SetLanguage_Impl( LanguageTag( xDic->getLocale() ).getLanguageType() );

        SetDicReadonly_Impl(xDic);
        sal_Bool bEnable = !IsDicReadonly_Impl();
        aLangFT.Enable( bEnable );
        aLangLB.Enable( bEnable );
    }
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK_NOARG(SvxEditDictionaryDialog, SelectLangHdl_Impl)
{
    sal_uInt16 nDicPos = aAllDictsLB.GetSelectEntryPos();
    sal_uInt16 nLang = aLangLB.GetSelectLanguage();
    Reference< XDictionary >  xDic( aDics.getConstArray()[ nDicPos ], UNO_QUERY );
    sal_Int16 nOldLang = LanguageTag( xDic->getLocale() ).getLanguageType();

    if ( nLang != nOldLang )
    {
        QueryBox aBox( this, CUI_RES( RID_SFXQB_SET_LANGUAGE ) );
        String sTxt( aBox.GetMessText() );
        sTxt.SearchAndReplaceAscii( "%1", aAllDictsLB.GetSelectEntry() );
        aBox.SetMessText( sTxt );

        if ( aBox.Execute() == RET_YES )
        {
            xDic->setLocale( LanguageTag::convertToLocale( nLang ) );
            bool bNegativ = xDic->getDictionaryType() == DictionaryType_NEGATIVE;

            const String sName(
                ::GetDicInfoStr( xDic->getName(),
                                 LanguageTag( xDic->getLocale() ).getLanguageType(),
                                 bNegativ ) );
            aAllDictsLB.RemoveEntry( nDicPos );
            aAllDictsLB.InsertEntry( sName, nDicPos );
            aAllDictsLB.SelectEntryPos( nDicPos );
        }
        else
            SetLanguage_Impl( nOldLang );
    }
    return 1;
}

// -----------------------------------------------------------------------

void SvxEditDictionaryDialog::ShowWords_Impl( sal_uInt16 nId )
{
    Reference< XDictionary >  xDic = aDics.getConstArray()[ nId ];

    nOld = nId;
    EnterWait();

    String aStr;

    aWordED.SetText(aStr);
    aReplaceED.SetText(aStr);

    if(xDic->getDictionaryType() != DictionaryType_POSITIVE)
    {
        nStaticTabs[0]=2;

        // make controls for replacement text active
        if(!aReplaceFT.IsVisible())
        {
            Size aSize=aWordED.GetSizePixel();
            aSize.Width()=nWidth;
            aWordED.SetSizePixel(aSize);
            aReplaceFT.Show();
            aReplaceED.Show();
        }
    }
    else
    {
        nStaticTabs[0]=1;

        // deactivate controls for replacement text
        if(aReplaceFT.IsVisible())
        {
            Size aSize=aWordED.GetSizePixel();
            aSize.Width()=aWordsLB.GetSizePixel().Width();
            aWordED.SetSizePixel(aSize);
            aReplaceFT.Hide();
            aReplaceED.Hide();
        }

    }

    aWordsLB.SetTabs(nStaticTabs);
    aWordsLB.Clear();

    Sequence< Reference< XDictionaryEntry >  > aEntries( xDic->getEntries() );
    const Reference< XDictionaryEntry >  *pEntry = aEntries.getConstArray();
    sal_Int32 nCount = aEntries.getLength();

    for (sal_Int32 i = 0;  i < nCount;  i++)
    {
        aStr = String(pEntry[i]->getDictionaryWord());
        sal_uInt16 nPos = GetLBInsertPos( aStr );
        if(pEntry[i]->isNegative())
        {
            aStr += '\t';
            aStr += String(pEntry[i]->getReplacementText());
        }
        aWordsLB.InsertEntry(aStr, 0, sal_False, nPos == USHRT_MAX ?  LIST_APPEND : nPos);
    }

    if (aWordsLB.GetEntryCount())
    {
        aWordED   .SetText( aWordsLB.GetEntryText((sal_uLong)0, 0) );
        aReplaceED.SetText( aWordsLB.GetEntryText((sal_uLong)0, 1) );
    }

    LeaveWait();
}

// -----------------------------------------------------------------------

IMPL_LINK(SvxEditDictionaryDialog, SelectHdl, SvTabListBox*, pBox)
{
    if(!bDoNothing)
    {
        if(!bFirstSelect)
        {
            SvTreeListEntry* pEntry = pBox->FirstSelected();
            OUString sTmpShort(pBox->GetEntryText(pEntry, 0));
            // without this the curser is always at the beginning of a word, if the text
            // is set over the ModifyHdl, although you're editing there at the moment
            if(aWordED.GetText() != sTmpShort)
                aWordED.SetText(sTmpShort);
            aReplaceED.SetText(pBox->GetEntryText(pEntry, 1));
        }
        else
            bFirstSelect = sal_False;

        // entries in the list box should exactly correspond to those from the
        // dictionary. Thus:
        aNewReplacePB.Enable(sal_False);
        aDeletePB    .Enable( sal_True && !IsDicReadonly_Impl() );
    }
    return 0;
};

// -----------------------------------------------------------------------

IMPL_LINK(SvxEditDictionaryDialog, NewDelHdl, PushButton*, pBtn)
{
    SvTreeListEntry* pEntry = aWordsLB.FirstSelected();

    if(pBtn == &aDeletePB)
    {
        DBG_ASSERT(pEntry, "keine Eintrag selektiert");
        String aStr;

        aWordED.SetText(aStr);
        aReplaceED.SetText(aStr);
        aDeletePB.Disable();

        RemoveDictEntry(pEntry);    // remove entry from dic and list-box
    }
    if(pBtn == &aNewReplacePB || aNewReplacePB.IsEnabled())
    {
        SvTreeListEntry* _pEntry = aWordsLB.FirstSelected();
        OUString aNewWord(aWordED.GetText());
        OUString sEntry(aNewWord);
        OUString aReplaceStr(aReplaceED.GetText());

        sal_Int16 nAddRes = DIC_ERR_UNKNOWN;
        sal_uInt16 nPos = aAllDictsLB.GetSelectEntryPos();
        if ( nPos != LISTBOX_ENTRY_NOTFOUND && !aNewWord.isEmpty())
        {
            DBG_ASSERT(nPos < aDics.getLength(), "invalid dictionary index");
            Reference< XDictionary >  xDic( aDics.getConstArray()[ nPos ], UNO_QUERY );
            if (xDic.is())
            {
                // make changes in dic

                //! ...IsVisible should reflect whether the dictionary is a negativ
                //! or not (hopefully...)
                sal_Bool bIsNegEntry = aReplaceFT.IsVisible();
                OUString aRplcText;
                if(bIsNegEntry)
                    aRplcText = aReplaceStr;

                if (_pEntry) // entry selected in aWordsLB ie action = modify entry
                    xDic->remove( aWordsLB.GetEntryText( _pEntry, 0 ) );
                // if remove has failed the following add should fail too
                // and thus a warning message should be triggered...

                nAddRes = linguistic::AddEntryToDic( xDic,
                            aNewWord, bIsNegEntry,
                            aRplcText, LanguageTag( xDic->getLocale() ).getLanguageType(), sal_False );
             }
        }
        if (DIC_ERR_NONE != nAddRes)
            SvxDicError( this, nAddRes );

        if(DIC_ERR_NONE == nAddRes && !sEntry.isEmpty())
        {
            // insert new entry in list-box etc...

            aWordsLB.SetUpdateMode(sal_False);
            sal_uInt16 _nPos = USHRT_MAX;

            if(aReplaceFT.IsVisible())
            {
                sEntry += "\t";
                sEntry += aReplaceStr;
            }

            SvTreeListEntry* pNewEntry = NULL;
            if(_pEntry) // entry selected in aWordsLB ie action = modify entry
            {
                aWordsLB.SetEntryText( sEntry, _pEntry );
                pNewEntry = _pEntry;
            }
            else
            {
                _nPos = GetLBInsertPos( aNewWord );
                SvTreeListEntry* pInsEntry = aWordsLB.InsertEntry(sEntry, 0, sal_False,
                            _nPos == USHRT_MAX ? LIST_APPEND : (sal_uInt32)_nPos);
                pNewEntry = pInsEntry;
            }

            aWordsLB.MakeVisible( pNewEntry );
            aWordsLB.SetUpdateMode(sal_True);
            // if the request came from the ReplaceEdit, give focus to the ShortEdit
            if(aReplaceED.HasFocus())
                aWordED.GrabFocus();
        }
    }
    else
    {
        // this can only be an enter in one of the two edit fields
        // which means EndDialog() - has to be evaluated in KeyInput
        return 0;
    }
    ModifyHdl(&aWordED);
    return 1;
}

// -----------------------------------------------------------------------

IMPL_LINK(SvxEditDictionaryDialog, ModifyHdl, Edit*, pEdt)
{
    SvTreeListEntry* pFirstSel = aWordsLB.FirstSelected();
    String rEntry = pEdt->GetText();

    xub_StrLen nWordLen=rEntry.Len();
    const String& rRepString = aReplaceED.GetText();

    sal_Bool bEnableNewReplace  = sal_False;
    sal_Bool bEnableDelete      = sal_False;
    String aNewReplaceText  = sNew;

    if(pEdt == &aWordED)
    {
        if(nWordLen>0)
        {
            sal_Bool bFound = sal_False;
            sal_Bool bTmpSelEntry=sal_False;
            CDE_RESULT eCmpRes = CDE_DIFFERENT;

            for(sal_uInt16 i = 0; i < aWordsLB.GetEntryCount(); i++)
            {
                SvTreeListEntry*  pEntry = aWordsLB.GetEntry( i );
                String aTestStr( aWordsLB.GetEntryText(pEntry, 0) );
                eCmpRes = cmpDicEntry_Impl( rEntry, aTestStr );
                if(CDE_DIFFERENT != eCmpRes)
                {
                    if(rRepString.Len())
                        bFirstSelect = sal_True;
                    bDoNothing=sal_True;
                    aWordsLB.SetCurEntry(pEntry);
                    bDoNothing=sal_False;
                    pFirstSel = pEntry;
                    aReplaceED.SetText(aWordsLB.GetEntryText(pEntry, 1));

                    if (CDE_SIMILAR == eCmpRes)
                    {
                        aNewReplaceText = sModify;
                        bEnableNewReplace = sal_True;
                    }
                    bFound= sal_True;
                    break;
                }
                else if(getNormDicEntry_Impl(aTestStr).Search(
                            getNormDicEntry_Impl( rEntry ) ) == 0
                        && !bTmpSelEntry)
                {
                    bDoNothing=sal_True;
                    aWordsLB.MakeVisible(pEntry);
                    bDoNothing=sal_False;
                    bTmpSelEntry=sal_True;

                    aNewReplaceText = sNew;
                    bEnableNewReplace = sal_True;
                }
            }

            if(!bFound)
            {
                aWordsLB.SelectAll(sal_False);
                pFirstSel = 0;

                aNewReplaceText = sNew;
                bEnableNewReplace = sal_True;
            }
            bEnableDelete = CDE_DIFFERENT != eCmpRes;
        }
        else if(aWordsLB.GetEntryCount()>0)
        {
            SvTreeListEntry*  pEntry = aWordsLB.GetEntry( 0 );
            bDoNothing=sal_True;
            aWordsLB.MakeVisible(pEntry);
            bDoNothing=sal_False;
        }
    }
    else if(pEdt == &aReplaceED)
    {
        String aReplaceText;
        String aWordText;
        if (pFirstSel)  // a aWordsLB entry is selected
        {
            aWordText    = aWordsLB.GetEntryText( pFirstSel, 0 );
             aReplaceText = aWordsLB.GetEntryText( pFirstSel, 1 );

            aNewReplaceText = sModify;
            bEnableDelete = sal_True;
        }
        sal_Bool bIsChange =
                CDE_EQUAL != cmpDicEntry_Impl(aWordED.GetText(), aWordText)
             || CDE_EQUAL != cmpDicEntry_Impl(aReplaceED.GetText(), aReplaceText);
        if (!aWordED.GetText().isEmpty()  &&  bIsChange)
            bEnableNewReplace = sal_True;
    }

    aNewReplacePB.SetText( aNewReplaceText );
    aNewReplacePB.Enable( bEnableNewReplace && !IsDicReadonly_Impl() );
    aDeletePB    .Enable( bEnableDelete     && !IsDicReadonly_Impl() );

    return 0;
}

//=========================================================
//SvxDictEdit
//=========================================================
void SvxDictEdit::KeyInput( const KeyEvent& rKEvt )
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


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

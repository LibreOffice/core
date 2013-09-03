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

#include <com/sun/star/linguistic2/XAvailableLocales.hpp>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <linguistic/misc.hxx>
#include <rtl/ustring.hxx>
#include <unotools/localedatawrapper.hxx>
#include <tools/urlobj.hxx>
#include <svtools/langtab.hxx>
#include <tools/shl.hxx>
#include <i18nlangtag/mslangid.hxx>
#include <i18nlangtag/lang.h>
#include <editeng/scripttypeitem.hxx>
#include <editeng/unolingu.hxx>
#include <svx/langbox.hxx>
#include <svx/dialmgr.hxx>
#include <svx/dialogs.hrc>
#include <vcl/builder.hxx>

using namespace ::com::sun::star::util;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::linguistic2;
using namespace ::com::sun::star::uno;

// -----------------------------------------------------------------------

OUString GetDicInfoStr( const OUString& rName, const sal_uInt16 nLang, bool bNeg )
{
    INetURLObject aURLObj;
    aURLObj.SetSmartProtocol( INET_PROT_FILE );
    aURLObj.SetSmartURL( rName, INetURLObject::ENCODE_ALL );
    OUString aTmp( aURLObj.GetBase() );
    aTmp += " ";

    if ( bNeg )
    {
        aTmp += " (-) ";
    }

    if ( LANGUAGE_NONE == nLang )
        aTmp += SVX_RESSTR(RID_SVXSTR_LANGUAGE_ALL);
    else
    {
        aTmp += " '[' ";
        aTmp += SvtLanguageTable::GetLanguageString( (LanguageType)nLang );
        aTmp += " ']' ";
    }

    return aTmp;
}

//========================================================================
//  misc local helper functions
//========================================================================

static Sequence< sal_Int16 > lcl_LocaleSeqToLangSeq( Sequence< Locale > &rSeq )
{
    const Locale *pLocale = rSeq.getConstArray();
    sal_Int32 nCount = rSeq.getLength();

    Sequence< sal_Int16 >   aLangs( nCount );
    sal_Int16 *pLang = aLangs.getArray();
    for (sal_Int32 i = 0;  i < nCount;  ++i)
    {
        pLang[i] = LanguageTag::convertToLanguageType( pLocale[i] );

    }

    return aLangs;
}


static bool lcl_SeqHasLang( const Sequence< sal_Int16 > & rLangSeq, sal_Int16 nLang )
{
    sal_Int32 i = -1;
    sal_Int32 nLen = rLangSeq.getLength();
    if (nLen)
    {
        const sal_Int16 *pLang = rLangSeq.getConstArray();
        for (i = 0;  i < nLen;  ++i)
        {
            if (nLang == pLang[i])
                break;
        }
    }
    return i >= 0  &&  i < nLen;
}

//========================================================================
//  class SvxLanguageBox
//========================================================================

sal_uInt16 TypeToPos_Impl( LanguageType eType, const ListBox& rLb )
{
    sal_uInt16  nPos   = LISTBOX_ENTRY_NOTFOUND;
    sal_uInt16  nCount = rLb.GetEntryCount();

    for ( sal_uInt16 i=0; nPos == LISTBOX_ENTRY_NOTFOUND && i<nCount; i++ )
        if ( eType == LanguageType((sal_uIntPtr)rLb.GetEntryData(i)) )
            nPos = i;

    return nPos;
}

//------------------------------------------------------------------------
SvxLanguageBox::SvxLanguageBox( Window* pParent, const ResId& rResId, sal_Bool bCheck ) :
    ListBox( pParent, rResId ),
    m_pSpellUsedLang( NULL ),
    m_bWithCheckmark( bCheck )
{
    Init();
}

SvxLanguageBox::SvxLanguageBox( Window* pParent, WinBits nBits, sal_Bool bCheck )
    : ListBox( pParent, nBits )
    , m_pSpellUsedLang( NULL )
    , m_bWithCheckmark( bCheck )
{
    Init();
}

extern "C" SAL_DLLPUBLIC_EXPORT Window* SAL_CALL makeSvxLanguageBox(Window *pParent, VclBuilder::stringmap &)
{
    SvxLanguageBox *pListBox = new SvxLanguageBox(pParent, WB_LEFT|WB_DROPDOWN|WB_VCENTER|WB_3DLOOK|WB_TABSTOP);
    pListBox->EnableAutoSize(true);
    return pListBox;
}

//------------------------------------------------------------------------
void SvxLanguageBox::Init()
{
    m_pLangTable = new SvtLanguageTable;
    m_aNotCheckedImage = Image( SVX_RES( RID_SVXIMG_NOTCHECKED ) );
    m_aCheckedImage = Image( SVX_RES( RID_SVXIMG_CHECKED ) );
    m_aAllString            = String( SVX_RESSTR( RID_SVXSTR_LANGUAGE_ALL ) );
    m_nLangList             = LANG_LIST_EMPTY;
    m_bHasLangNone          = sal_False;
    m_bLangNoneIsLangAll    = sal_False;

    // display entries sorted
    SetStyle( GetStyle() | WB_SORT );

    if ( m_bWithCheckmark )
    {
        SvtLanguageTable aLangTable;
        sal_uInt32 nCount = aLangTable.GetEntryCount();
        for ( sal_uInt32 i = 0; i < nCount; i++ )
        {
            LanguageType nLangType = aLangTable.GetTypeAtIndex( i );

            bool bInsert = true;
            if ((LANGUAGE_DONTKNOW == nLangType)  ||
                (LANGUAGE_SYSTEM   == nLangType))
            {
                bInsert = false;
            }

            if ( bInsert )
                InsertLanguage( nLangType );
        }
        m_nLangList = LANG_LIST_ALL;
    }
}
//------------------------------------------------------------------------

SvxLanguageBox::~SvxLanguageBox()
{
    delete m_pSpellUsedLang;
    delete m_pLangTable;
}

//------------------------------------------------------------------------

sal_uInt16 SvxLanguageBox::ImplInsertImgEntry( const OUString& rEntry, sal_uInt16 nPos, bool bChecked )
{
    sal_uInt16 nRet = 0;
    if( !bChecked )
        nRet = InsertEntry( rEntry, m_aNotCheckedImage, nPos );
    else
        nRet = InsertEntry( rEntry, m_aCheckedImage, nPos );
    return nRet;
}

//------------------------------------------------------------------------

void SvxLanguageBox::SetLanguageList( sal_Int16 nLangList,
        sal_Bool bHasLangNone, sal_Bool bLangNoneIsLangAll, sal_Bool bCheckSpellAvail )
{
    Clear();

    m_nLangList             = nLangList;
    m_bHasLangNone          = bHasLangNone;
    m_bLangNoneIsLangAll    = bLangNoneIsLangAll;
    m_bWithCheckmark        = bCheckSpellAvail;

    if ( LANG_LIST_EMPTY != nLangList )
    {
        Sequence< sal_Int16 > aSpellAvailLang;
        Sequence< sal_Int16 > aHyphAvailLang;
        Sequence< sal_Int16 > aThesAvailLang;
        Sequence< sal_Int16 > aSpellUsedLang;
        Sequence< sal_Int16 > aHyphUsedLang;
        Sequence< sal_Int16 > aThesUsedLang;
        Reference< XAvailableLocales > xAvail( LinguMgr::GetLngSvcMgr(), UNO_QUERY );
        if (xAvail.is())
        {
            Sequence< Locale > aTmp;

            if (LANG_LIST_SPELL_AVAIL & nLangList)
            {
                aTmp = xAvail->getAvailableLocales( SN_SPELLCHECKER );
                aSpellAvailLang = lcl_LocaleSeqToLangSeq( aTmp );
            }
            if (LANG_LIST_HYPH_AVAIL  & nLangList)
            {
                aTmp = xAvail->getAvailableLocales( SN_HYPHENATOR );
                aHyphAvailLang = lcl_LocaleSeqToLangSeq( aTmp );
            }
            if (LANG_LIST_THES_AVAIL  & nLangList)
            {
                aTmp = xAvail->getAvailableLocales( SN_THESAURUS );
                aThesAvailLang = lcl_LocaleSeqToLangSeq( aTmp );
            }
        }
        if (LANG_LIST_SPELL_USED & nLangList)
        {
            Reference< XSpellChecker1 > xTmp1( SvxGetSpellChecker(), UNO_QUERY );
            if (xTmp1.is())
                aSpellUsedLang = xTmp1->getLanguages();
        }
        if (LANG_LIST_HYPH_USED  & nLangList)
        {
            Reference< XHyphenator > xTmp( SvxGetHyphenator() );
            if (xTmp.is()) {
                Sequence < Locale > aLocaleSequence( xTmp->getLocales() );
                aHyphUsedLang = lcl_LocaleSeqToLangSeq( aLocaleSequence );
            }
        }
        if (LANG_LIST_THES_USED  & nLangList)
        {
            Reference< XThesaurus > xTmp( SvxGetThesaurus() );
            if (xTmp.is()) {
                Sequence < Locale > aLocaleSequence( xTmp->getLocales() );
                aThesUsedLang = lcl_LocaleSeqToLangSeq( aLocaleSequence );
            }
        }

        SvtLanguageTable aLangTable;
        ::com::sun::star::uno::Sequence< sal_uInt16 > xKnown;
        const sal_uInt16* pKnown;
        sal_uInt32 nCount;
        if ( nLangList & LANG_LIST_ONLY_KNOWN )
        {
            xKnown = LocaleDataWrapper::getInstalledLanguageTypes();
            pKnown = xKnown.getConstArray();
            nCount = xKnown.getLength();
        }
        else
        {
            nCount = aLangTable.GetEntryCount();
            pKnown = NULL;
        }
        for ( sal_uInt32 i = 0; i < nCount; i++ )
        {
            LanguageType nLangType;
            if ( nLangList & LANG_LIST_ONLY_KNOWN )
                nLangType = pKnown[i];
            else
                nLangType = aLangTable.GetTypeAtIndex( i );
            if ( nLangType != LANGUAGE_DONTKNOW &&
                 nLangType != LANGUAGE_SYSTEM &&
                 nLangType != LANGUAGE_NONE &&
                 (MsLangId::getSubLanguage( nLangType) != 0 ||
                  (nLangList & LANG_LIST_ALSO_PRIMARY_ONLY)) &&
                 ((nLangList & LANG_LIST_ALL) != 0 ||
                  ((nLangList & LANG_LIST_WESTERN) != 0 &&
                   (SvtLanguageOptions::GetScriptTypeOfLanguage(nLangType) ==
                    SCRIPTTYPE_LATIN)) ||
                  ((nLangList & LANG_LIST_CTL) != 0 &&
                   (SvtLanguageOptions::GetScriptTypeOfLanguage(nLangType) ==
                    SCRIPTTYPE_COMPLEX)) ||
                  ((nLangList & LANG_LIST_CJK) != 0 &&
                   (SvtLanguageOptions::GetScriptTypeOfLanguage(nLangType) ==
                    SCRIPTTYPE_ASIAN)) ||
                  ((nLangList & LANG_LIST_FBD_CHARS) != 0 &&
                   MsLangId::hasForbiddenCharacters(nLangType)) ||
                  ((nLangList & LANG_LIST_SPELL_AVAIL) != 0 &&
                   lcl_SeqHasLang(aSpellAvailLang, nLangType)) ||
                  ((nLangList & LANG_LIST_HYPH_AVAIL) != 0 &&
                   lcl_SeqHasLang(aHyphAvailLang, nLangType)) ||
                  ((nLangList & LANG_LIST_THES_AVAIL) != 0 &&
                   lcl_SeqHasLang(aThesAvailLang, nLangType)) ||
                  ((nLangList & LANG_LIST_SPELL_USED) != 0 &&
                   lcl_SeqHasLang(aSpellUsedLang, nLangType)) ||
                  ((nLangList & LANG_LIST_HYPH_USED) != 0 &&
                   lcl_SeqHasLang(aHyphUsedLang, nLangType)) ||
                  ((nLangList & LANG_LIST_THES_USED) != 0 &&
                   lcl_SeqHasLang(aThesUsedLang, nLangType))) )
                InsertLanguage( nLangType );
        }

        if (bHasLangNone)
            InsertLanguage( LANGUAGE_NONE );
    }
}

//------------------------------------------------------------------------

sal_uInt16 SvxLanguageBox::InsertLanguage( const LanguageType nLangType, sal_uInt16 nPos )
{
    return ImplInsertLanguage( nLangType, nPos, ::com::sun::star::i18n::ScriptType::WEAK );
}

//------------------------------------------------------------------------

sal_uInt16 SvxLanguageBox::ImplInsertLanguage( const LanguageType nLangType, sal_uInt16 nPos, sal_Int16 nType )
{
    LanguageType nLang = MsLangId::getReplacementForObsoleteLanguage( nLangType);
    // For obsolete and to be replaced languages check whether an entry of the
    // replacement already exists and if so don't add an entry with identical
    // string as would be returned by SvtLanguageTable::GetString().
    if (nLang != nLangType)
    {
        sal_uInt16 nAt = TypeToPos_Impl( nLang, *this );
        if ( nAt != LISTBOX_ENTRY_NOTFOUND )
            return nAt;
    }

    String aStrEntry = m_pLangTable->GetString( nLang );
    if (LANGUAGE_NONE == nLang && m_bHasLangNone && m_bLangNoneIsLangAll)
        aStrEntry = m_aAllString;

    LanguageType nRealLang = nLang;
    if (nRealLang == LANGUAGE_SYSTEM)
    {
        nRealLang = MsLangId::resolveSystemLanguageByScriptType(nRealLang, nType);
        aStrEntry.AppendAscii(" - ");
        aStrEntry.Append(m_pLangTable->GetString( nRealLang ));
    } else if (nRealLang == LANGUAGE_USER_SYSTEM_CONFIG) {
        nRealLang = MsLangId::getSystemLanguage();
        aStrEntry.AppendAscii(" - ");
        aStrEntry.Append(m_pLangTable->GetString( nRealLang ));
    }

    aStrEntry = ApplyLreOrRleEmbedding( aStrEntry );

    sal_uInt16 nAt = 0;
    if ( m_bWithCheckmark )
    {
        bool bFound = false;

        if (!m_pSpellUsedLang)
        {
            Reference< XSpellChecker1 > xSpell( SvxGetSpellChecker(), UNO_QUERY );
            if ( xSpell.is() )
                m_pSpellUsedLang = new Sequence< sal_Int16 >( xSpell->getLanguages() );
        }
        bFound = m_pSpellUsedLang ?
            lcl_SeqHasLang( *m_pSpellUsedLang, nRealLang ) : false;

        nAt = ImplInsertImgEntry( aStrEntry, nPos, bFound );
    }
    else
        nAt = InsertEntry( aStrEntry, nPos );

    SetEntryData( nAt, (void*)(sal_uIntPtr)nLangType );
    return nAt;
}

//------------------------------------------------------------------------

sal_uInt16 SvxLanguageBox::InsertDefaultLanguage( sal_Int16 nType, sal_uInt16 nPos )
{
    return ImplInsertLanguage( LANGUAGE_SYSTEM, nPos, nType );
}

//------------------------------------------------------------------------

sal_uInt16 SvxLanguageBox::InsertSystemLanguage( sal_uInt16 nPos )
{
    return ImplInsertLanguage( LANGUAGE_USER_SYSTEM_CONFIG, nPos, ::com::sun::star::i18n::ScriptType::WEAK );
}

//------------------------------------------------------------------------

sal_uInt16 SvxLanguageBox::InsertLanguage( const LanguageType nLangType,
        sal_Bool bCheckEntry, sal_uInt16 nPos )
{
    LanguageType nLang = MsLangId::getReplacementForObsoleteLanguage( nLangType);
    // For obsolete and to be replaced languages check whether an entry of the
    // replacement already exists and if so don't add an entry with identical
    // string as would be returned by SvtLanguageTable::GetString().
    if (nLang != nLangType)
    {
        sal_uInt16 nAt = TypeToPos_Impl( nLang, *this );
        if ( nAt != LISTBOX_ENTRY_NOTFOUND )
            return nAt;
    }

    String aStrEntry = m_pLangTable->GetString( nLang );
    if (LANGUAGE_NONE == nLang && m_bHasLangNone && m_bLangNoneIsLangAll)
        aStrEntry = m_aAllString;

    sal_uInt16 nAt = ImplInsertImgEntry( aStrEntry, nPos, bCheckEntry );
    SetEntryData( nAt, (void*)(sal_uIntPtr)nLang );

    return nAt;
}

//------------------------------------------------------------------------

void SvxLanguageBox::RemoveLanguage( const LanguageType eLangType )
{
    sal_uInt16 nAt = TypeToPos_Impl( eLangType, *this );

    if ( nAt != LISTBOX_ENTRY_NOTFOUND )
        RemoveEntry( nAt );
}

//------------------------------------------------------------------------

LanguageType SvxLanguageBox::GetSelectLanguage() const
{
    sal_uInt16       nPos   = GetSelectEntryPos();

    if ( nPos != LISTBOX_ENTRY_NOTFOUND )
        return LanguageType( (sal_uIntPtr)GetEntryData(nPos) );
    else
        return LanguageType( LANGUAGE_DONTKNOW );
}

//------------------------------------------------------------------------

void SvxLanguageBox::SelectLanguage( const LanguageType eLangType, sal_Bool bSelect )
{
    // If the core uses a LangID of an imported MS document and wants to select
    // a language that is replaced, we need to select the replacement instead.
    LanguageType nLang = MsLangId::getReplacementForObsoleteLanguage( eLangType);

    sal_uInt16 nAt = TypeToPos_Impl( nLang, *this );

    if ( nAt != LISTBOX_ENTRY_NOTFOUND )
        SelectEntryPos( nAt, bSelect );
}

//------------------------------------------------------------------------

sal_Bool SvxLanguageBox::IsLanguageSelected( const LanguageType eLangType ) const
{
    // Same here, work on the replacement if applicable.
    LanguageType nLang = MsLangId::getReplacementForObsoleteLanguage( eLangType);

    sal_uInt16 nAt = TypeToPos_Impl( nLang, *this );

    if ( nAt != LISTBOX_ENTRY_NOTFOUND )
        return IsEntryPosSelected( nAt );
    else
        return sal_False;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

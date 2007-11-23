/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: langbox.cxx,v $
 *
 *  $Revision: 1.23 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-23 16:38:18 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

// include ---------------------------------------------------------------

#ifndef _COM_SUN_STAR_LINGUISTIC2_XLINGUSERVICEMANAGER_HDL_
#include <com/sun/star/linguistic2/XLinguServiceManager.hdl>
#endif
#ifndef _COM_SUN_STAR_LINGUISTIC2_XAVAILABLELOCALES_HPP_
#include <com/sun/star/linguistic2/XAvailableLocales.hpp>
#endif
#ifndef _LINGUISTIC_MISC_HXX_
#include <linguistic/misc.hxx>
#endif
#ifndef _RTL_USTRING_HXX_
#include<rtl/ustring.hxx>
#endif

#ifndef _UNOTOOLS_LOCALEDATAWRAPPER_HXX
#include <unotools/localedatawrapper.hxx>
#endif

#include <svtools/langtab.hxx>

#ifndef _SHL_HXX
#include <tools/shl.hxx>
#endif
#ifndef INCLUDED_I18NPOOL_MSLANGID_HXX
#include <i18npool/mslangid.hxx>
#endif
#ifndef INCLUDED_I18NPOOL_LANG_H
#include <i18npool/lang.h>
#endif
#ifndef _SVX_SCRIPTTYPEITEM_HXX
#include <svx/scripttypeitem.hxx>
#endif
#ifndef _UNO_LINGU_HXX
#include <unolingu.hxx>
#endif
#include <svx/langbox.hxx>
#include <svx/dialmgr.hxx>
#include <svx/dialogs.hrc>

using namespace ::rtl;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::linguistic2;
using namespace ::com::sun::star::uno;


//========================================================================
//  misc local helper functions
//========================================================================



static Sequence< INT16 > lcl_LocaleSeqToLangSeq( Sequence< Locale > &rSeq )
{
    const Locale *pLocale = rSeq.getConstArray();
    INT32 nCount = rSeq.getLength();

    Sequence< INT16 >   aLangs( nCount );
    INT16 *pLang = aLangs.getArray();
    for (INT32 i = 0;  i < nCount;  ++i)
    {
        pLang[i] = SvxLocaleToLanguage( pLocale[i] );

    }

    return aLangs;
}


static BOOL lcl_SeqHasLang( const Sequence< INT16 > & rLangSeq, INT16 nLang )
{
    INT32 i = -1;
    INT32 nLen = rLangSeq.getLength();
    if (nLen)
    {
        const INT16 *pLang = rLangSeq.getConstArray();
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

USHORT TypeToPos_Impl( LanguageType eType, const ListBox& rLb )
{
    USHORT  nPos   = LISTBOX_ENTRY_NOTFOUND;
    USHORT  nCount = rLb.GetEntryCount();

    for ( USHORT i=0; nPos == LISTBOX_ENTRY_NOTFOUND && i<nCount; i++ )
        if ( eType == LanguageType((ULONG)rLb.GetEntryData(i)) )
            nPos = i;

    return nPos;
}

//-----------------------------------------------------------------------
SvxLanguageBox::SvxLanguageBox( Window* pParent, WinBits nWinStyle, BOOL bCheck ) :
    ListBox( pParent, nWinStyle ),
    m_pSpellUsedLang( NULL ),
    m_bWithCheckmark( bCheck )
{
    Init();
}
//------------------------------------------------------------------------
SvxLanguageBox::SvxLanguageBox( Window* pParent, const ResId& rResId, BOOL bCheck ) :
    ListBox( pParent, rResId ),
    m_pSpellUsedLang( NULL ),
    m_bWithCheckmark( bCheck )
{
    Init();
}
//------------------------------------------------------------------------
void SvxLanguageBox::Init()
{
    m_pLangTable = new SvtLanguageTable;
    m_aNotCheckedImage = Image( SVX_RES( RID_SVXIMG_NOTCHECKED ) );
    m_aCheckedImage = Image( SVX_RES( RID_SVXIMG_CHECKED ) );
    m_aCheckedImageHC = Image( SVX_RES( RID_SVXIMG_CHECKED_H ) );
    m_aAllString            = String( SVX_RESSTR( RID_SVXSTR_LANGUAGE_ALL ) );
    m_nLangList             = LANG_LIST_EMPTY;
    m_bHasLangNone          = FALSE;
    m_bLangNoneIsLangAll    = FALSE;

    // display entries sorted
    SetStyle( GetStyle() | WB_SORT );

    if ( m_bWithCheckmark )
    {
        SvtLanguageTable aLangTable;
        sal_uInt32 nCount = aLangTable.GetEntryCount();
        for ( sal_uInt32 i = 0; i < nCount; i++ )
        {
            LanguageType nLangType = aLangTable.GetTypeAtIndex( i );

            BOOL bInsert = TRUE;
            if ((LANGUAGE_DONTKNOW == nLangType)  ||
                (LANGUAGE_SYSTEM   == nLangType)  ||
                (LANGUAGE_USER1 <= nLangType  &&  nLangType <= LANGUAGE_USER9))
            {
                bInsert = FALSE;
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

USHORT SvxLanguageBox::ImplInsertImgEntry( const String& rEntry, USHORT nPos, bool bChecked )
{
    USHORT nRet = 0;
    if( !bChecked )
        nRet = InsertEntry( rEntry, m_aNotCheckedImage, nPos );
    else if( GetSettings().GetStyleSettings().GetFaceColor().IsDark() )
        nRet = InsertEntry( rEntry, m_aCheckedImageHC, nPos );
    else
        nRet = InsertEntry( rEntry, m_aCheckedImage, nPos );
    return nRet;
}

//------------------------------------------------------------------------

void SvxLanguageBox::SetLanguageList( INT16 nLangList,
        BOOL bHasLangNone, BOOL bLangNoneIsLangAll, BOOL bCheckSpellAvail )
{
    Clear();

    m_nLangList             = nLangList;
    m_bHasLangNone          = bHasLangNone;
    m_bLangNoneIsLangAll    = bLangNoneIsLangAll;
    m_bWithCheckmark        = bCheckSpellAvail;

    if ( LANG_LIST_EMPTY != nLangList )
    {
        Sequence< INT16 > aSpellAvailLang;
        Sequence< INT16 > aHyphAvailLang;
        Sequence< INT16 > aThesAvailLang;
        Sequence< INT16 > aSpellUsedLang;
        Sequence< INT16 > aHyphUsedLang;
        Sequence< INT16 > aThesUsedLang;
        Reference< XAvailableLocales > xAvail( LinguMgr::GetLngSvcMgr(), UNO_QUERY );
        if (xAvail.is())
        {
            Sequence< Locale > aTmp;

            if (LANG_LIST_SPELL_AVAIL & nLangList)
            {
                aTmp = xAvail->getAvailableLocales( A2OU( SN_SPELLCHECKER ) );
                aSpellAvailLang = lcl_LocaleSeqToLangSeq( aTmp );
            }
            if (LANG_LIST_HYPH_AVAIL  & nLangList)
            {
                aTmp = xAvail->getAvailableLocales( A2OU( SN_HYPHENATOR ) );
                aHyphAvailLang = lcl_LocaleSeqToLangSeq( aTmp );
            }
            if (LANG_LIST_THES_AVAIL  & nLangList)
            {
                aTmp = xAvail->getAvailableLocales( A2OU( SN_THESAURUS ) );
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
                 (nLangType < LANGUAGE_USER1 || nLangType > LANGUAGE_USER9) &&
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

USHORT SvxLanguageBox::InsertLanguage( const LanguageType nLangType, USHORT nPos )
{
    String aStrEntry = m_pLangTable->GetString( nLangType );
    if (LANGUAGE_NONE == nLangType && m_bHasLangNone && m_bLangNoneIsLangAll)
        aStrEntry = m_aAllString;

    USHORT nAt = 0;
    if ( m_bWithCheckmark )
    {
        sal_Bool bFound = sal_False;

        if (!m_pSpellUsedLang)
        {
            Reference< XSpellChecker1 > xSpell( SvxGetSpellChecker(), UNO_QUERY );
            if ( xSpell.is() )
                m_pSpellUsedLang = new Sequence< INT16 >( xSpell->getLanguages() );
        }
        bFound = m_pSpellUsedLang ?
                    lcl_SeqHasLang( *m_pSpellUsedLang, nLangType ) : FALSE;

        nAt = ImplInsertImgEntry( aStrEntry, nPos, bFound );
    }
    else
        nAt = InsertEntry( aStrEntry, nPos );

    SetEntryData( nAt, (void*)(ULONG)nLangType );
    return nAt;
}

//------------------------------------------------------------------------

USHORT SvxLanguageBox::InsertLanguage( const LanguageType nLangType,
        BOOL bCheckEntry, USHORT nPos )
{
    String aStrEntry = m_pLangTable->GetString( nLangType );
    if (LANGUAGE_NONE == nLangType && m_bHasLangNone && m_bLangNoneIsLangAll)
        aStrEntry = m_aAllString;

    USHORT nAt = ImplInsertImgEntry( aStrEntry, nPos, bCheckEntry );
    SetEntryData( nAt, (void*)(ULONG)nLangType );

    return nAt;
}

//------------------------------------------------------------------------

void SvxLanguageBox::RemoveLanguage( const LanguageType eLangType )
{
    USHORT nAt = TypeToPos_Impl( eLangType, *this );

    if ( nAt != LISTBOX_ENTRY_NOTFOUND )
        RemoveEntry( nAt );
}

//------------------------------------------------------------------------

LanguageType SvxLanguageBox::GetSelectLanguage() const
{
    USHORT       nPos   = GetSelectEntryPos();

    if ( nPos != LISTBOX_ENTRY_NOTFOUND )
        return LanguageType( (ULONG)GetEntryData(nPos) );
    else
        return LanguageType( LANGUAGE_DONTKNOW );
}

//------------------------------------------------------------------------

void SvxLanguageBox::SelectLanguage( const LanguageType eLangType, BOOL bSelect )
{
    USHORT nAt = TypeToPos_Impl( eLangType, *this );

    if ( nAt != LISTBOX_ENTRY_NOTFOUND )
        SelectEntryPos( nAt, bSelect );
}

//------------------------------------------------------------------------

BOOL SvxLanguageBox::IsLanguageSelected( const LanguageType eLangType ) const
{
    USHORT nAt = TypeToPos_Impl( eLangType, *this );

    if ( nAt != LISTBOX_ENTRY_NOTFOUND )
        return IsEntryPosSelected( nAt );
    else
        return FALSE;
}


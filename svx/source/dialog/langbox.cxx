/*************************************************************************
 *
 *  $RCSfile: langbox.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: tl $ $Date: 2001-03-22 08:28:26 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

// include ---------------------------------------------------------------

#ifndef _COM_SUN_STAR_LINGUISTIC2_XLINGUSERVICEMANAGER_HDL_
#include <com/sun/star/linguistic2/XLinguServiceManager.hdl>
#endif
#ifndef _LINGUISTIC_MISC_HXX_
#include <linguistic/misc.hxx>
#endif
#ifndef _RTL_USTRING_HXX_
#include<rtl/ustring.hxx>
#endif

#ifndef _SHL_HXX
#include <tools/shl.hxx>
#endif
#ifndef _ISOLANG_HXX
#include <tools/isolang.hxx>
#endif
#pragma hdrstop

#ifndef _SVX_SCRIPTTYPEITEM_HXX
#include <scripttypeitem.hxx>
#endif
#ifndef _UNO_LINGU_HXX
#include <unolingu.hxx>
#endif
#ifndef _SVX_LANGTAB_HXX
#include <langtab.hxx>
#endif
#include "langbox.hxx"
#include "langtab.hxx"
#include "dialmgr.hxx"
#include "dialogs.hrc"
#include "unolingu.hxx"

using namespace ::rtl;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::linguistic2;
using namespace ::com::sun::star::uno;

#define A2OU(x)     OUString::createFromAscii( x )

//========================================================================
//  list of languages for forbidden chars
//========================================================================

static const LanguageType aForbiddenCharLang[] =
{
    LANGUAGE_CHINESE_TRADITIONAL,
    LANGUAGE_CHINESE_SIMPLIFIED,
    LANGUAGE_JAPANESE,
    LANGUAGE_KOREAN
};

static const int nForbiddenCharLang = sizeof( aForbiddenCharLang ) / sizeof( aForbiddenCharLang[0] );


static BOOL lcl_HasLanguage( const LanguageType *pLang, int nCount, LanguageType nLang )
{
    int i = -1;
    if (pLang && nCount > 0)
    {
        for (i = 0;  i < nCount; ++i )
        {
            if (pLang[i] == nLang)
                break;
        }
    }
    return i >= 0  &&  i < nCount;
}

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
/*!!! (pb) obsolete
SvxLanguageBox::SvxLanguageBox( Window* pParent, WinBits nWinStyle ) :

    ListBox( pParent, nWinStyle )

{
    m_pLangTable = new SvxLanguageTable;
    aNotCheckedImage = Image( SVX_RES( RID_SVXIMG_NOTCHECKED ) );
    aCheckedImage = Image( SVX_RES( RID_SVXIMG_CHECKED ) );
}
*/
//------------------------------------------------------------------------

SvxLanguageBox::SvxLanguageBox( Window* pParent, const ResId& rResId, BOOL bCheck ) :

    ListBox( pParent, rResId ),

    m_bWithCheckmark( bCheck )
{
    m_pLangTable = new SvxLanguageTable;
    m_aNotCheckedImage = Image( SVX_RES( RID_SVXIMG_NOTCHECKED ) );
    m_aCheckedImage = Image( SVX_RES( RID_SVXIMG_CHECKED ) );
    m_aAllString            = String( SVX_RESSTR( RID_SVXSTR_LANGUAGE_ALL ) );
    m_nLangList             = LANG_LIST_EMPTY;
    m_bHasLangNone          = FALSE;
    m_bLangNoneIsLangAll    = FALSE;

    // display entries sorted
    SetStyle( GetStyle() | WB_SORT );

    if ( m_bWithCheckmark )
    {
        SvxLanguageTable aLangTable;
        const USHORT nCount = aLangTable.GetEntryCount();
        for ( USHORT i = 0; i < nCount; i++ )
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

SvxLanguageBox::SvxLanguageBox( Window* pParent, const ResId& rResId ) :
    ListBox( pParent, rResId )
{
    m_pLangTable        = new SvxLanguageTable;
    m_aNotCheckedImage  = Image( SVX_RES( RID_SVXIMG_NOTCHECKED ) );
    m_aCheckedImage     = Image( SVX_RES( RID_SVXIMG_CHECKED ) );
    m_bWithCheckmark    = FALSE;
    m_aAllString            = String( SVX_RESSTR( RID_SVXSTR_LANGUAGE_ALL ) );
    m_nLangList             = LANG_LIST_EMPTY;
    m_bHasLangNone          = FALSE;
    m_bLangNoneIsLangAll    = FALSE;

    // display entries sorted
    SetStyle( GetStyle() | WB_SORT );
}

//------------------------------------------------------------------------

SvxLanguageBox::~SvxLanguageBox()
{
    delete m_pLangTable;
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
        if (LinguMgr::GetLngSvcMgr().is())
        {
            Sequence< Locale > aTmp;

            if (LANG_LIST_SPELL_AVAIL & nLangList)
            {
                aTmp = LinguMgr::GetLngSvcMgr()
                            ->getAvailableLocales( A2OU( SN_SPELLCHECKER ) );
                aSpellAvailLang = lcl_LocaleSeqToLangSeq( aTmp );
            }
            if (LANG_LIST_HYPH_AVAIL  & nLangList)
            {
                aTmp = LinguMgr::GetLngSvcMgr()
                            ->getAvailableLocales( A2OU( SN_HYPHENATOR ) );
                aHyphAvailLang = lcl_LocaleSeqToLangSeq( aTmp );
            }
            if (LANG_LIST_THES_AVAIL  & nLangList)
            {
                aTmp = LinguMgr::GetLngSvcMgr()
                            ->getAvailableLocales( A2OU( SN_THESAURUS ) );
                aThesAvailLang = lcl_LocaleSeqToLangSeq( aTmp );
            }
        }

        SvxLanguageTable aLangTable;
        const USHORT nCount = aLangTable.GetEntryCount();
        for ( USHORT i = 0; i < nCount; i++ )
        {
            LanguageType nLangType = aLangTable.GetTypeAtIndex( i );
            BOOL bInsert = FALSE;
            if ( nLangType != LANGUAGE_SYSTEM  &&
                 nLangType != LANGUAGE_NONE )
            {
                if (nLangList & LANG_LIST_ALL)
                    bInsert |= TRUE;
                if (nLangList & LANG_LIST_WESTERN)
                    bInsert |= SCRIPTTYPE_LATIN == GetScriptTypeOfLanguage( nLangType );
                if (nLangList & LANG_LIST_CTL)
                    bInsert |= SCRIPTTYPE_COMPLEX == GetScriptTypeOfLanguage( nLangType );
                if (nLangList & LANG_LIST_CJK)
                    bInsert |= SCRIPTTYPE_ASIAN == GetScriptTypeOfLanguage( nLangType );
                if (nLangList & LANG_LIST_FBD_CHARS)
                    bInsert |= lcl_HasLanguage( aForbiddenCharLang,
                                        nForbiddenCharLang, nLangType );
                if (nLangList & LANG_LIST_SPELL_AVAIL)
                    bInsert |= lcl_SeqHasLang( aSpellAvailLang, nLangType );
                if (nLangList & LANG_LIST_HYPH_AVAIL)
                    bInsert |= lcl_SeqHasLang( aHyphAvailLang, nLangType );
                if (nLangList & LANG_LIST_THES_AVAIL)
                    bInsert |= lcl_SeqHasLang( aThesAvailLang, nLangType );
            }

            if ((LANGUAGE_DONTKNOW == nLangType)  ||
                (LANGUAGE_USER1 <= nLangType  &&  nLangType <= LANGUAGE_USER9))
            {
                bInsert = FALSE;
            }

            if (bInsert)
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
        const USHORT nLanguageCount = SvxGetSelectableLanguages().getLength();
        const Language* pLangList = SvxGetSelectableLanguages().getConstArray();
        sal_Bool bFound = sal_False;
        for ( USHORT i = 0; i < nLanguageCount; ++i )
        {
            if ( nLangType == pLangList[i] )
            {
                bFound = sal_True;
                break;
            }
        }

        if ( !bFound )
            nAt = InsertEntry( aStrEntry, m_aNotCheckedImage, nPos );
        else
            nAt = InsertEntry( aStrEntry, m_aCheckedImage, nPos );
    }
    else
        nAt = InsertEntry( aStrEntry, nPos );

    SetEntryData( nAt, (void*)(ULONG)nLangType );
    return nPos;
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
    LanguageType eType  = LanguageType(LANGUAGE_DONTKNOW);
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


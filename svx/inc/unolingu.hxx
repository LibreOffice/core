/*************************************************************************
 *
 *  $RCSfile: unolingu.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:03 $
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

#ifndef _UNO_LINGU_HXX
#define _UNO_LINGU_HXX

#ifndef _LANG_HXX
#include <tools/lang.hxx>
#endif
#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef _VOS_REFERNCE_HXX_
#include <vos/refernce.hxx>
#endif

#ifndef _COM_SUN_STAR_UTIL_LANGUAGE_HPP_
#include <com/sun/star/util/Language.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_LOCALE_HPP_
#include <com/sun/star/lang/Locale.hpp>
#endif
#ifndef _COM_SUN_STAR_LINGUISTIC_XSPELLCHECKER1_HPP_
#include <com/sun/star/linguistic/XSpellChecker1.hpp>
#endif
#ifndef _COM_SUN_STAR_LINGUISTIC_XHYPHENATOR_HPP_
#include <com/sun/star/linguistic/XHyphenator.hpp>
#endif
#ifndef _COM_SUN_STAR_LINGUISTIC_XTHESAURUS_HPP_
#include <com/sun/star/linguistic/XThesaurus.hpp>
#endif
#ifndef _COM_SUN_STAR_LINGUISTIC_XDICTIONARYLIST_HPP_
#include <com/sun/star/linguistic/XDictionaryList.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_LINGUISTIC_XDICTIONARY1_HPP_
#include <com/sun/star/linguistic/XDictionary1.hpp>
#endif

class LinguMgrExitLstnr;

class Window;

// SvxAddEntryToDic return values
#define DIC_ERR_NONE        0
#define DIC_ERR_FULL        1
#define DIC_ERR_READONLY    2
#define DIC_ERR_UNKNOWN     3
#define DIC_ERR_NOT_EXISTS  4
//#define DIC_ERR_ENTRY_EXISTS  5
//#define DIC_ERR_ENTRY_NOTEXISTS   6

///////////////////////////////////////////////////////////////////////////

class LinguMgr
{
    friend class LinguMgrExitLstnr;

    //static ::VOS::ORefCount aRefCount;

    static ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic::XSpellChecker1 >  xSpell;
    static ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic::XHyphenator >     xHyph;
    static ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic::XThesaurus >      xThes;
    static ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic::XDictionaryList > xDicList;
    static ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet >         xProp;

    static ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic::XDictionary1 >    xIgnoreAll;
    static ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic::XDictionary1 >    xChangeAll;

    static LinguMgrExitLstnr                           *pExitLstnr;
    static sal_Bool                                     bExiting;

    static ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic::XSpellChecker1 >  GetSpell();
    static ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic::XHyphenator >     GetHyph();
    static ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic::XThesaurus >      GetThes();
    static ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic::XDictionaryList > GetDicList();
    static ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet >         GetProp();
    static ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic::XDictionary1 >    GetStandard();
    static ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic::XDictionary1 >    GetIgnoreAll();
    static ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic::XDictionary1 >    GetChangeAll();

    // disallow access to copy-constructor and assignment-operator
    LinguMgr(const LinguMgr &);
    LinguMgr & operator = (const LinguMgr &);

public:

    static ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic::XSpellChecker1 >  GetSpellChecker();
    static ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic::XHyphenator >     GetHyphenator();
    static ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic::XThesaurus >      GetThesaurus();
    static ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic::XDictionaryList > GetDictionaryList();
    static ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet >         GetLinguPropertySet();

    static ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic::XDictionary1 >    GetStandardDic();
    static ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic::XDictionary1 >    GetIgnoreAllList();
    static ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic::XDictionary1 >    GetChangeAllList();
};

///////////////////////////////////////////////////////////////////////////

#ifdef TL_NOTYET

namespace com { namespace sun { namespace star { namespace linguistic2 {
    class XHyphenatedWord;
}}}};


struct SvxAlternativeSpelling
{
    String      aReplacement;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XHyphenatedWord >    xHyphWord;
    INT16       nChangedPos,
                 nChangedLength;
    BOOL        bIsAltSpelling;

    inline SvxAlternativeSpelling();
};

inline SvxAlternativeSpelling::SvxAlternativeSpelling() :
    nChangedPos(-1), nChangedLength(-1), bIsAltSpelling(FALSE)
{
}


SvxAlternativeSpelling SvxGetAltSpelling(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::linguistic2::XHyphenatedWord > & rHyphWord );

#endif //TL_NOTYET

///////////////////////////////////////////////////////////////////////////

class SvxDicListChgClamp
{
private:
    ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic::XDictionaryList >     xDicList;

    // disallow access to copy-constructor and assignment-operator
    SvxDicListChgClamp(const SvxDicListChgClamp &);
    SvxDicListChgClamp & operator = (const SvxDicListChgClamp &);

public:
    SvxDicListChgClamp( ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic::XDictionaryList >  &rxDicList );
    ~SvxDicListChgClamp();
};

///////////////////////////////////////////////////////////////////////////

//TL:TODO: remove those functions or make them inline
::com::sun::star::uno::Reference<
    ::com::sun::star::linguistic::XSpellChecker1 >  SvxGetSpellChecker();
::com::sun::star::uno::Reference<
    ::com::sun::star::linguistic::XHyphenator >     SvxGetHyphenator();
::com::sun::star::uno::Reference<
    ::com::sun::star::linguistic::XThesaurus >      SvxGetThesaurus();
::com::sun::star::uno::Reference<
    ::com::sun::star::linguistic::XDictionaryList > SvxGetDictionaryList();
::com::sun::star::uno::Reference<
    ::com::sun::star::beans::XPropertySet >         SvxGetLinguPropertySet();
//TL:TODO: remove argument or provide SvxGetIgnoreAllList with the same one
::com::sun::star::uno::Reference<
    ::com::sun::star::linguistic::XDictionary1 >    SvxGetOrCreatePosDic(
            ::com::sun::star::uno::Reference<
                ::com::sun::star::linguistic::XDictionaryList >  xDicList );
::com::sun::star::uno::Reference<
    ::com::sun::star::linguistic::XDictionary1 >    SvxGetIgnoreAllList();
::com::sun::star::uno::Reference<
    ::com::sun::star::linguistic::XDictionary1 >    SvxGetChangeAllList();

///////////////////////////////////////////////////////////////////////////
// misc functions
//

LanguageType                        SvxLocaleToLanguage(
        const ::com::sun::star::lang::Locale& rLocale );
::com::sun::star::lang::Locale&     SvxLanguageToLocale(
        ::com::sun::star::lang::Locale& rLocale, LanguageType eLang );
::com::sun::star::lang::Locale      SvxCreateLocale( LanguageType eLang );

// Sequence of non XSpellChecker, XHyphenator or XThesaurus specific
// languages that are frequently used
const ::com::sun::star::uno::Sequence<
    ::com::sun::star::util::Language > &    SvxGetSelectableLanguages();

// returns position of language in sequence.
// If not found a negative value will be returned.
sal_Int32   SvxGetLanguagePos(
    const ::com::sun::star::uno::Sequence<
        ::com::sun::star::util::Language > &rSeq,
    ::com::sun::star::util::Language nLang);

String  SvxGetDictionaryURL(const String &rDicName,
                            sal_Bool bIsUserDic = sal_True);

//TL:TODO: soll mal den rictigen Rückgabetyp bekommen!
sal_Bool    SvxAddEntryToDic(
    ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic::XDictionary >  &rxDic,
    const ::rtl::OUString &rWord, sal_Bool bIsNeg,
    const ::rtl::OUString &rRplcTxt, sal_Int16 nRplcLang,
    sal_Bool bStripDot = sal_True );
short   SvxDicError( Window *pParent, sal_Int16 nError );

sal_Bool    SvxSaveDictionaries(
    const ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic::XDictionaryList >  &xDicList );

#endif




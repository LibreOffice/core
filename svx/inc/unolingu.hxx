/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: unolingu.hxx,v $
 * $Revision: 1.17 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _UNO_LINGU_HXX
#define _UNO_LINGU_HXX

#include <i18npool/lang.h>
#include <tools/string.hxx>
#include <vos/refernce.hxx>
#include <com/sun/star/util/Language.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/linguistic2/XLinguServiceManager.hpp>
#include <com/sun/star/linguistic2/XSpellChecker1.hpp>
#include <com/sun/star/linguistic2/XHyphenator.hpp>
#include <com/sun/star/linguistic2/XThesaurus.hpp>
#include <com/sun/star/linguistic2/XDictionaryList.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include "svx/svxdllapi.h"

class LinguMgrExitLstnr;

class Window;

///////////////////////////////////////////////////////////////////////////
// SvxLinguConfigUpdate
// class to update configuration items when (before!) the linguistic is used.
//
// This class is called by all the dummy implementations to update all of the
// configuration (list of used/available services) when the linguistic is
// accessed for the first time.

class SvxLinguConfigUpdate
{
    static INT32    nCurrentDataFilesChangedCheckValue;
    static INT16    nNeedUpdating;  // n == -1 => needs to be checked
                                    // n ==  0 => already updated, nothing to be done
                                    // n ==  1 => needs to be updated

    static INT32 CalcDataFilesChangedCheckValue();

public:

    static void UpdateAll( sal_Bool bForceCheck = sal_False );
    static BOOL IsNeedUpdateAll( sal_Bool bForceCheck = sal_False );
};

///////////////////////////////////////////////////////////////////////////

class SVX_DLLPUBLIC LinguMgr
{
    friend class LinguMgrExitLstnr;

    //static ::VOS::ORefCount aRefCount;

    static ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XLinguServiceManager > xLngSvcMgr;
    static ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XSpellChecker1 > xSpell;
    static ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XHyphenator >    xHyph;
    static ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XThesaurus >     xThes;
    static ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XDictionaryList > xDicList;
    static ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet >         xProp;

    static ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XDictionary >    xIgnoreAll;
    static ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XDictionary >    xChangeAll;

    static LinguMgrExitLstnr                           *pExitLstnr;
    static sal_Bool                                     bExiting;

    static ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XSpellChecker1 > GetSpell();
    static ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XHyphenator >    GetHyph();
    static ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XThesaurus >     GetThes();
    static ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XDictionaryList > GetDicList();
    static ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet >         GetProp();
    static ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XDictionary >    GetStandard();
    static ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XDictionary >    GetIgnoreAll();
    static ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XDictionary >    GetChangeAll();

    // disallow access to copy-constructor and assignment-operator
    LinguMgr(const LinguMgr &);
    LinguMgr & operator = (const LinguMgr &);

public:

    static ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XSpellChecker1 > GetSpellChecker();
    static ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XHyphenator >    GetHyphenator();
    static ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XThesaurus >     GetThesaurus();
    static ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XDictionaryList > GetDictionaryList();
    static ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet >         GetLinguPropertySet();

    static ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XLinguServiceManager > GetLngSvcMgr();

    static ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XDictionary >    GetStandardDic();
    static ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XDictionary >    GetIgnoreAllList();
    static ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XDictionary >    GetChangeAllList();

    // update all configuration entries
    static void UpdateAll();
};

///////////////////////////////////////////////////////////////////////////

namespace com { namespace sun { namespace star { namespace linguistic2 {
    class XHyphenatedWord;
}}}}


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


SVX_DLLPUBLIC SvxAlternativeSpelling SvxGetAltSpelling(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::linguistic2::XHyphenatedWord > & rHyphWord );


///////////////////////////////////////////////////////////////////////////

class SVX_DLLPUBLIC SvxDicListChgClamp
{
private:
    ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XDictionaryList >    xDicList;

    // disallow access to copy-constructor and assignment-operator
    SVX_DLLPRIVATE SvxDicListChgClamp(const SvxDicListChgClamp &);
    SVX_DLLPRIVATE SvxDicListChgClamp & operator = (const SvxDicListChgClamp &);

public:
    SvxDicListChgClamp( ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XDictionaryList >  &rxDicList );
    ~SvxDicListChgClamp();
};

///////////////////////////////////////////////////////////////////////////

//TL:TODO: remove those functions or make them inline
SVX_DLLPUBLIC ::com::sun::star::uno::Reference<
    ::com::sun::star::linguistic2::XSpellChecker1 > SvxGetSpellChecker();
::com::sun::star::uno::Reference<
    ::com::sun::star::linguistic2::XHyphenator >    SvxGetHyphenator();
::com::sun::star::uno::Reference<
    ::com::sun::star::linguistic2::XThesaurus >     SvxGetThesaurus();
SVX_DLLPUBLIC ::com::sun::star::uno::Reference<
    ::com::sun::star::linguistic2::XDictionaryList > SvxGetDictionaryList();
SVX_DLLPUBLIC ::com::sun::star::uno::Reference<
    ::com::sun::star::beans::XPropertySet >         SvxGetLinguPropertySet();
//TL:TODO: remove argument or provide SvxGetIgnoreAllList with the same one
SVX_DLLPUBLIC ::com::sun::star::uno::Reference<
    ::com::sun::star::linguistic2::XDictionary >    SvxGetOrCreatePosDic(
            ::com::sun::star::uno::Reference<
                ::com::sun::star::linguistic2::XDictionaryList >  xDicList );
SVX_DLLPUBLIC ::com::sun::star::uno::Reference<
    ::com::sun::star::linguistic2::XDictionary >    SvxGetIgnoreAllList();
SVX_DLLPUBLIC ::com::sun::star::uno::Reference<
    ::com::sun::star::linguistic2::XDictionary >    SvxGetChangeAllList();

///////////////////////////////////////////////////////////////////////////
// misc functions
//

SVX_DLLPUBLIC LanguageType                      SvxLocaleToLanguage(
        const ::com::sun::star::lang::Locale& rLocale );
SVX_DLLPUBLIC ::com::sun::star::lang::Locale&   SvxLanguageToLocale(
        ::com::sun::star::lang::Locale& rLocale, LanguageType eLang );
SVX_DLLPUBLIC ::com::sun::star::lang::Locale    SvxCreateLocale( LanguageType eLang );


SVX_DLLPUBLIC short SvxDicError( Window *pParent, sal_Int16 nError );


#endif




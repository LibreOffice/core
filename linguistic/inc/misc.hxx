/*************************************************************************
 *
 *  $RCSfile: misc.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2004-06-17 16:13:28 $
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

#ifndef _LINGUISTIC_MISC_HXX_
#define _LINGUISTIC_MISC_HXX_


#ifndef _COM_SUN_STAR_UNO_SEQUENCE_H_
#include <com/sun/star/uno/Sequence.h>
#endif
#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUES_HPP_
#include <com/sun/star/beans/PropertyValues.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XTERMINATELISTENER_HPP_
#include <com/sun/star/frame/XTerminateListener.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_LOCALE_HPP_
#include <com/sun/star/lang/Locale.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_LINGUISTIC2_XDICTIONARYENTRY_HPP_
#include <com/sun/star/linguistic2/XDictionaryEntry.hpp>
#endif
#ifndef _COM_SUN_STAR_LINGUISTIC2_XSEARCHABLEDICTIONARYLIST_HPP_
#include <com/sun/star/linguistic2/XSearchableDictionaryList.hpp>
#endif
#ifndef _COM_SUN_STAR_LINGUISTIC2_XHYPHENATEDWORD_HPP_
#include <com/sun/star/linguistic2/XHyphenatedWord.hpp>
#endif

#include <uno/lbnames.h>            // CPPU_CURRENT_LANGUAGE_BINDING_NAME macro, which specify the environment type
#include <cppuhelper/implbase1.hxx> // helper for implementations

#ifndef _ISOLANG_HXX
#include <tools/isolang.hxx>
#endif
#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef _TOOLS_INTN_HXX
#include <tools/intn.hxx>
#endif
#ifndef _UNOTOOLS_CHARCLASS_HXX
#include <unotools/charclass.hxx>
#endif
#ifndef _OSL_THREAD_H_
#include <osl/thread.h>
#endif
#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

namespace com { namespace sun { namespace star { namespace beans {
    class XPropertySet;
    class XFastPropertySet;
}}}};

namespace com { namespace sun { namespace star { namespace frame {
    class XDesktop;
}}}};

class LocaleDataWrapper;

///////////////////////////////////////////////////////////////////////////

#define SN_SPELLCHECKER             "com.sun.star.linguistic2.SpellChecker"
#define SN_HYPHENATOR               "com.sun.star.linguistic2.Hyphenator"
#define SN_THESAURUS                "com.sun.star.linguistic2.Thesaurus"
#define SN_LINGU_SERVCICE_MANAGER   "com.sun.star.linguistic2.LinguServiceManager"
#define SN_LINGU_PROPERTIES         "com.sun.star.linguistic2.LinguProperties"
#define SN_DICTIONARY_LIST          "com.sun.star.linguistic2.DictionaryList"
#define SN_OTHER_LINGU              "com.sun.star.linguistic2.OtherLingu"
#define SN_DESKTOP                  "com.sun.star.frame.Desktop"


namespace linguistic
{

///////////////////////////////////////////////////////////////////////////

::osl::Mutex &  GetLinguMutex();

LocaleDataWrapper & GetLocaleDataWrapper( INT16 nLang );

///////////////////////////////////////////////////////////////////////////

rtl_TextEncoding GetTextEncoding( INT16 nLanguage );

inline ::rtl::OUString BS2OU(const ByteString &rText, rtl_TextEncoding nEnc)
{
    return ::rtl::OUString( rText.GetBuffer(), rText.Len(), nEnc );
}

inline ByteString OU2BS(const ::rtl::OUString &rText, rtl_TextEncoding nEnc)
{
    return ByteString( rText.getStr(), nEnc );
}

///////////////////////////////////////////////////////////////////////////

sal_Int32 LevDistance( const rtl::OUString &rTxt1, const rtl::OUString &rTxt2 );

///////////////////////////////////////////////////////////////////////////

::com::sun::star::lang::Locale
    CreateLocale( LanguageType eLang );

LanguageType
     LocaleToLanguage( const ::com::sun::star::lang::Locale& rLocale );

::com::sun::star::lang::Locale&
    LanguageToLocale( ::com::sun::star::lang::Locale& rLocale, LanguageType eLang );

::com::sun::star::uno::Sequence< ::com::sun::star::lang::Locale >
    LangSeqToLocaleSeq( const ::com::sun::star::uno::Sequence< INT16 > &rLangSeq );

::com::sun::star::uno::Sequence< INT16 >
    LocaleSeqToLangSeq( ::com::sun::star::uno::Sequence<
        ::com::sun::star::lang::Locale > &rLocaleSeq );

///////////////////////////////////////////////////////////////////////////

// checks if file pointed to by rURL is readonly
// and may also check return if such a file exists or not
BOOL    IsReadOnly( const String &rURL, BOOL *pbExist = 0 );

///////////////////////////////////////////////////////////////////////////

INT32       GetPosInWordToCheck( const rtl::OUString &rTxt, INT32 nPos );

::com::sun::star::uno::Reference<
    ::com::sun::star::linguistic2::XHyphenatedWord >
            RebuildHyphensAndControlChars( const rtl::OUString &rOrigWord,
                ::com::sun::star::uno::Reference<
                    ::com::sun::star::linguistic2::XHyphenatedWord > &rxHyphWord );

///////////////////////////////////////////////////////////////////////////

BOOL        IsUpper( const String &rText, INT16 nLanguage );
BOOL        IsLower( const String &rText, INT16 nLanguage );
String      ToLower( const String &rText, INT16 nLanguage );
sal_Unicode ToLower( const sal_Unicode cChar, INT16 nLanguage );
sal_Unicode ToUpper( const sal_Unicode cChar, INT16 nLanguage );
BOOL        HasDigits( const String &rText );
BOOL        IsNumeric( const String &rText );

///////////////////////////////////////////////////////////////////////////

::com::sun::star::uno::Reference<
    ::com::sun::star::uno::XInterface >
        GetOneInstanceService( const char *pServiceName );

::com::sun::star::uno::Reference<
    ::com::sun::star::beans::XPropertySet >
        GetLinguProperties();

::com::sun::star::uno::Reference<
    ::com::sun::star::linguistic2::XSearchableDictionaryList >
        GetSearchableDictionaryList();

::com::sun::star::uno::Reference<
    ::com::sun::star::linguistic2::XDictionaryList >
        GetDictionaryList();

///////////////////////////////////////////////////////////////////////////

BOOL IsUseDicList( const ::com::sun::star::beans::PropertyValues &rProperties,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet > &rxPropSet );

BOOL IsIgnoreControlChars( const ::com::sun::star::beans::PropertyValues &rProperties,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet > &rxPropSet );

::com::sun::star::uno::Reference<
    ::com::sun::star::linguistic2::XDictionaryEntry >
        SearchDicList(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::linguistic2::XDictionaryList >& rDicList,
            const ::rtl::OUString& rWord, INT16 nLanguage,
            BOOL bSearchPosDics, BOOL bSearchSpellEntry );

///////////////////////////////////////////////////////////////////////////
//
// AppExitLstnr:
// virtual base class that calls it AtExit function when the application
// (ie the Desktop) is about to terminate
//

class AppExitListener :
    public cppu::WeakImplHelper1
    <
        ::com::sun::star::frame::XTerminateListener
    >
{
    ::com::sun::star::uno::Reference<
        ::com::sun::star::frame::XDesktop >     xDesktop;

public:
    AppExitListener();
    virtual ~AppExitListener();

    virtual void    AtExit() = 0;

    void            Activate();
    void            Deactivate();

    // XEventListener
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException);

    // XTerminateListener
    virtual void SAL_CALL queryTermination( const ::com::sun::star::lang::EventObject& aEvent ) throw(::com::sun::star::frame::TerminationVetoException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL notifyTermination( const ::com::sun::star::lang::EventObject& aEvent ) throw(::com::sun::star::uno::RuntimeException);
};

///////////////////////////////////////////////////////////////////////////

}   // namespace linguistic

#endif


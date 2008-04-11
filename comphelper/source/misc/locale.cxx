/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: locale.cxx,v $
 * $Revision: 1.8 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_comphelper.hxx"
#include <comphelper/locale.hxx>

//_______________________________________________
// includes
#include <rtl/ustrbuf.hxx>

//_______________________________________________
// namespace

namespace comphelper{

//-----------------------------------------------
const sal_Unicode Locale::SEPERATOR_LC       = (sal_Unicode)'-';
const sal_Unicode Locale::SEPERATOR_CV       = (sal_Unicode)'_';
const sal_Unicode Locale::SEPERATOR_CV_LINUX = (sal_Unicode)'.';

//-----------------------------------------------
const Locale Locale::X_DEFAULT()
{
    static Locale aLocale(
                    ::rtl::OUString::createFromAscii("x"),
                    ::rtl::OUString::createFromAscii("default"));
    return aLocale;
}

//-----------------------------------------------
const Locale Locale::EN()
{
    static Locale aLocale(
                    ::rtl::OUString::createFromAscii("en"),
                    ::rtl::OUString());
    return aLocale;
}

//-----------------------------------------------
const Locale Locale::EN_US()
{
    static Locale aLocale(
                    ::rtl::OUString::createFromAscii("en"),
                    ::rtl::OUString::createFromAscii("US"));
    return aLocale;
}

//-----------------------------------------------
const Locale Locale::DE_DE()
{
    static Locale aLocale(
                    ::rtl::OUString::createFromAscii("de"),
                    ::rtl::OUString::createFromAscii("DE"));
    return aLocale;
}

//-----------------------------------------------
const Locale Locale::DE_CH()
{
    static Locale aLocale(
                    ::rtl::OUString::createFromAscii("de"),
                    ::rtl::OUString::createFromAscii("CH"));
    return aLocale;
}

//-----------------------------------------------
const Locale Locale::DE_AT()
{
    static Locale aLocale(
                    ::rtl::OUString::createFromAscii("de"),
                    ::rtl::OUString::createFromAscii("AT"));
    return aLocale;
}

//-----------------------------------------------
const Locale Locale::AR()
{
    static Locale aLocale(
                    ::rtl::OUString::createFromAscii("ar"),
                    ::rtl::OUString());
    return aLocale;
}

//-----------------------------------------------
const Locale Locale::CA()
{
    static Locale aLocale(
                    ::rtl::OUString::createFromAscii("ca"),
                    ::rtl::OUString());
    return aLocale;
}

//-----------------------------------------------
const Locale Locale::CS()
{
    static Locale aLocale(
                    ::rtl::OUString::createFromAscii("cs"),
                    ::rtl::OUString());
    return aLocale;
}

//-----------------------------------------------
const Locale Locale::DA()
{
    static Locale aLocale(
                    ::rtl::OUString::createFromAscii("da"),
                    ::rtl::OUString());
    return aLocale;
}

//-----------------------------------------------
const Locale Locale::EL()
{
    static Locale aLocale(
                    ::rtl::OUString::createFromAscii("el"),
                    ::rtl::OUString());
    return aLocale;
}

//-----------------------------------------------
const Locale Locale::ES()
{
    static Locale aLocale(
                    ::rtl::OUString::createFromAscii("es"),
                    ::rtl::OUString());
    return aLocale;
}

//-----------------------------------------------
const Locale Locale::FI()
{
    static Locale aLocale(
                    ::rtl::OUString::createFromAscii("fi"),
                    ::rtl::OUString());
    return aLocale;
}

//-----------------------------------------------
const Locale Locale::FR()
{
    static Locale aLocale(
                    ::rtl::OUString::createFromAscii("fr"),
                    ::rtl::OUString());
    return aLocale;
}

//-----------------------------------------------
const Locale Locale::HE()
{
    static Locale aLocale(
                    ::rtl::OUString::createFromAscii("he"),
                    ::rtl::OUString());
    return aLocale;
}

//-----------------------------------------------
const Locale Locale::HI_IN()
{
    static Locale aLocale(
                    ::rtl::OUString::createFromAscii("hi"),
                    ::rtl::OUString::createFromAscii("IN"));
    return aLocale;
}

//-----------------------------------------------
const Locale Locale::HU()
{
    static Locale aLocale(
                    ::rtl::OUString::createFromAscii("hu"),
                    ::rtl::OUString());
    return aLocale;
}

//-----------------------------------------------
const Locale Locale::IT()
{
    static Locale aLocale(
                    ::rtl::OUString::createFromAscii("it"),
                    ::rtl::OUString());
    return aLocale;
}

//-----------------------------------------------
const Locale Locale::JA()
{
    static Locale aLocale(
                    ::rtl::OUString::createFromAscii("ja"),
                    ::rtl::OUString());
    return aLocale;
}

//-----------------------------------------------
const Locale Locale::KO()
{
    static Locale aLocale(
                    ::rtl::OUString::createFromAscii("ko"),
                    ::rtl::OUString());
    return aLocale;
}

//-----------------------------------------------
const Locale Locale::NL()
{
    static Locale aLocale(
                    ::rtl::OUString::createFromAscii("nl"),
                    ::rtl::OUString());
    return aLocale;
}

//-----------------------------------------------
const Locale Locale::PL()
{
    static Locale aLocale(
                    ::rtl::OUString::createFromAscii("pl"),
                    ::rtl::OUString());
    return aLocale;
}

//-----------------------------------------------
const Locale Locale::PT()
{
    static Locale aLocale(
                    ::rtl::OUString::createFromAscii("pt"),
                    ::rtl::OUString());
    return aLocale;
}

//-----------------------------------------------
const Locale Locale::PT_BR()
{
    static Locale aLocale(
                    ::rtl::OUString::createFromAscii("pt"),
                    ::rtl::OUString::createFromAscii("BR"));
    return aLocale;
}

//-----------------------------------------------
const Locale Locale::RU()
{
    static Locale aLocale(
                    ::rtl::OUString::createFromAscii("ru"),
                    ::rtl::OUString());
    return aLocale;
}

//-----------------------------------------------
const Locale Locale::SK()
{
    static Locale aLocale(
                    ::rtl::OUString::createFromAscii("sk"),
                    ::rtl::OUString());
    return aLocale;
}

//-----------------------------------------------
const Locale Locale::SL()
{
    static Locale aLocale(
                    ::rtl::OUString::createFromAscii("sl"),
                    ::rtl::OUString());
    return aLocale;
}

//-----------------------------------------------
const Locale Locale::SV()
{
    static Locale aLocale(
                    ::rtl::OUString::createFromAscii("sv"),
                    ::rtl::OUString());
    return aLocale;
}

//-----------------------------------------------
const Locale Locale::TH()
{
    static Locale aLocale(
                    ::rtl::OUString::createFromAscii("th"),
                    ::rtl::OUString());
    return aLocale;
}

//-----------------------------------------------
const Locale Locale::TR()
{
    static Locale aLocale(
                    ::rtl::OUString::createFromAscii("tr"),
                    ::rtl::OUString());
    return aLocale;
}

//-----------------------------------------------
const Locale Locale::X_COMMENT()
{
    static Locale aLocale(
                    ::rtl::OUString::createFromAscii("x"),
                    ::rtl::OUString::createFromAscii("comment"));
    return aLocale;
}

//-----------------------------------------------
const Locale Locale::X_TRANSLATE()
{
    static Locale aLocale(
                    ::rtl::OUString::createFromAscii("x"),
                    ::rtl::OUString::createFromAscii("translate"));
    return aLocale;
}

//-----------------------------------------------
const Locale Locale::X_NOTRANSLATE()
{
    static Locale aLocale(
                    ::rtl::OUString::createFromAscii("x"),
                    ::rtl::OUString::createFromAscii("notranslate"));
    return aLocale;
}

//-----------------------------------------------
const Locale Locale::ZH_CN()
{
    static Locale aLocale(
                    ::rtl::OUString::createFromAscii("zh"),
                    ::rtl::OUString::createFromAscii("CN"));
    return aLocale;
}

//-----------------------------------------------
const Locale Locale::ZH_TW()
{
    static Locale aLocale(
                    ::rtl::OUString::createFromAscii("zh"),
                    ::rtl::OUString::createFromAscii("TW"));
    return aLocale;
}

//-----------------------------------------------
Locale::Locale(const ::rtl::OUString& sISO)
    throw(Locale::MalFormedLocaleException)
{
    fromISO(sISO);
}

//-----------------------------------------------
Locale::Locale(const ::rtl::OUString& sLanguage,
               const ::rtl::OUString& sCountry ,
               const ::rtl::OUString& sVariant )
{
    // Use set methods to check values too!
    setLanguage(sLanguage);
    setCountry (sCountry );
    setVariant (sVariant );
}

//-----------------------------------------------
Locale::Locale()
{
    // Initialize instance ... otherwhise user will
    // may be get exceptions if he e.g. copy this instance ...
    (*this) = X_NOTRANSLATE();
}

//-----------------------------------------------
Locale::Locale(const Locale& aCopy)
{
    (*this) = aCopy; // recycle assign operator
}

//-----------------------------------------------
::rtl::OUString Locale::getLanguage() const
{
    return m_sLanguage;
}

//-----------------------------------------------
::rtl::OUString Locale::getCountry() const
{
    return m_sCountry;
}

//-----------------------------------------------
::rtl::OUString Locale::getVariant() const
{
    return m_sVariant;
}

//-----------------------------------------------
void Locale::setLanguage(const ::rtl::OUString& sLanguage)
{
    m_sLanguage = sLanguage;
}

//-----------------------------------------------
void Locale::setCountry(const ::rtl::OUString& sCountry)
{
    m_sCountry = sCountry;
}

//-----------------------------------------------
void Locale::setVariant(const ::rtl::OUString& sVariant)
{
    m_sVariant = sVariant;
}

//-----------------------------------------------
/* Attention: Use own interface methods to set the
   different parts of this locale. Because the
   check the incoming value and throw an exception
   automaticly ...
 */
void Locale::fromISO(const ::rtl::OUString& sISO)
    throw(Locale::MalFormedLocaleException)
{
    m_sLanguage = ::rtl::OUString();
    m_sCountry  = ::rtl::OUString();
    m_sVariant  = ::rtl::OUString();

    ::rtl::OUString sParser(sISO);
    sParser.trim();

    sal_Int32 nStart = 0;
    sal_Int32 nEnd   = 0;

    // extract language part
    nEnd = sParser.indexOf(SEPERATOR_LC, nStart);
    if (nEnd<0)
    {
        setLanguage(sParser);
        return;
    }
    setLanguage(sParser.copy(nStart, nEnd-nStart));
    nStart = nEnd+1;

    // extract country
    nEnd = sParser.indexOf(SEPERATOR_CV, nStart);
    if (nEnd<0)
        nEnd = sParser.indexOf(SEPERATOR_CV_LINUX, nStart);
    if (nEnd<0)
    {
        setCountry(sParser.copy(nStart, sParser.getLength()-nStart));
        return;
    }
    nStart = nEnd+1;

    // extract variant
    setVariant(sParser.copy(nStart, sParser.getLength()-nStart));
}

//-----------------------------------------------
::rtl::OUString Locale::toISO() const
{
    ::rtl::OUStringBuffer sISO(64);

    sISO.append(m_sLanguage);
    if (m_sCountry.getLength())
    {
        sISO.append(SEPERATOR_LC);
        sISO.append(m_sCountry);

        if (m_sVariant.getLength())
        {
            sISO.append(SEPERATOR_CV);
            sISO.append(m_sVariant);
        }
    }

    return sISO.makeStringAndClear();
}

//-----------------------------------------------
sal_Bool Locale::equals(const Locale& aComparable) const
{
    return (
            m_sLanguage.equals(aComparable.m_sLanguage) &&
            m_sCountry.equals (aComparable.m_sCountry ) &&
            m_sVariant.equals (aComparable.m_sVariant )
           );
}

//-----------------------------------------------
sal_Bool Locale::similar(const Locale& aComparable) const
{
    return (m_sLanguage.equals(aComparable.m_sLanguage));
}

//-----------------------------------------------
::std::vector< ::rtl::OUString >::const_iterator Locale::getFallback(const ::std::vector< ::rtl::OUString >& lISOList     ,
                                                                     const ::rtl::OUString&                  sReferenceISO)
    throw(Locale::MalFormedLocaleException)
{
    Locale aReference(sReferenceISO);

    // Note: The same language or "en"/"en-US" should be preferred as fallback.
    // On the other side some localized variables doesnt use localzation in real.
    // May be the use a "fix" value only ... marked as X-DEFAULT or X-NOTRANSLATE.
    // At least it can be discussed, if any language is a valid fallback ...
    // But in case some office functionality depends on that (that means real functionality instead
    // of pure UI descriptions) we should do anything, so it can work.

    ::std::vector< ::rtl::OUString >::const_iterator pSimilar      = lISOList.end();
    ::std::vector< ::rtl::OUString >::const_iterator pEN_US        = lISOList.end();
    ::std::vector< ::rtl::OUString >::const_iterator pEN           = lISOList.end();
    ::std::vector< ::rtl::OUString >::const_iterator pXDefault     = lISOList.end();
    ::std::vector< ::rtl::OUString >::const_iterator pXNoTranslate = lISOList.end();
    ::std::vector< ::rtl::OUString >::const_iterator pAny          = lISOList.end();

    ::std::vector< ::rtl::OUString >::const_iterator pIt;
    for (  pIt  = lISOList.begin();
           pIt != lISOList.end()  ;
         ++pIt                    )
    {
        Locale aCheck(*pIt);
        // found Locale, which match with 100% => return it
        if (aCheck.equals(aReference))
            return pIt;

        // found similar Locale => safe it as possible fallback
        if (
            (pSimilar == lISOList.end()) &&
            (aCheck.similar(aReference))
           )
        {
            pSimilar = pIt;
        }
        else
        // found en-US => safe it as fallback
        if (
            (pEN_US == lISOList.end()) &&
            (aCheck.equals(EN_US())  )
           )
        {
            pEN_US = pIt;
        }
        else
        // found en[-XX] => safe it as fallback
        if (
            (pEN == lISOList.end()  ) &&
            (aCheck.similar(EN_US()))
           )
        {
            pEN = pIt;
        }
        else
        // found an explicit default value(!) => safe it as fallback
        if (
            (pXDefault == lISOList.end()) &&
            (aCheck.equals(X_DEFAULT()) )
           )
        {
            pXDefault = pIt;
        }
        else
        // found an implicit default value(!) => safe it as fallback
        if (
            (pXNoTranslate == lISOList.end()) &&
            (aCheck.equals(X_NOTRANSLATE()) )
           )
        {
            pXNoTranslate = pIt;
        }
        else
        // safe the first locale, which isn't an explicit fallback
        // as "last possible fallback"
        if (pAny == lISOList.end())
            pAny = pIt;
    }

    if (pSimilar != lISOList.end())
        return pSimilar;

    if (pEN_US != lISOList.end())
        return pEN_US;

    if (pEN != lISOList.end())
        return pEN;

    if (pXDefault != lISOList.end())
        return pXDefault;

    if (pXNoTranslate != lISOList.end())
        return pXNoTranslate;

    if (pAny != lISOList.end())
        return pAny;

    return lISOList.end();
}

//-----------------------------------------------
sal_Bool Locale::getFallback(Locale& aLocale)
{
    // a)
    // this was our last fallback!
    // break any further calls to this method ...
    if (aLocale.equals(X_NOTRANSLATE()))
        return sal_False;

    // b)
    // switch from X_DEFAULT to X_NOTRANSLATE
    // next time we will go to a)
    if (aLocale.equals(X_DEFAULT()))
    {
        aLocale = X_NOTRANSLATE();
        return sal_True;
    }

    // c)
    // switch from EN to X_DEFAULT
    // next time we will go to b)
    if (aLocale.equals(EN()))
    {
        aLocale = X_DEFAULT();
        return sal_True;
    }

    // d) remove country from incoming locale
    //    e.g. "de-DE" => "de" or "en-US" => "en"!
    if (aLocale.getCountry().getLength())
    {
        aLocale.setCountry(::rtl::OUString());
        return sal_True;
    }

    // e) "en-US" possible?
    if (!aLocale.equals(EN_US()))
    {
        aLocale = EN_US();
        return sal_True;
    }

    // f) no more fallbacks
    return sal_False;
}

//-----------------------------------------------
void  Locale::operator=(const Locale& rCopy)
{
    // Take over these values without checking ...
    // They was already checked if the copy was constructed
    // and must be valid now!
    m_sLanguage = rCopy.m_sLanguage;
    m_sCountry  = rCopy.m_sCountry;
    m_sVariant  = rCopy.m_sVariant;
}

//-----------------------------------------------
sal_Bool Locale::operator==(const Locale& aComparable) const
{
    return equals(aComparable);
}

//-----------------------------------------------
sal_Bool Locale::operator!=(const Locale& aComparable) const
{
    return !equals(aComparable);
}

} // namespace comphelper


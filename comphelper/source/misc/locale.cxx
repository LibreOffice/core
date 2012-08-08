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

#include <comphelper/locale.hxx>

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
    return Locale(::rtl::OUString("x"), ::rtl::OUString("default"));
}

//-----------------------------------------------
const Locale Locale::EN_US()
{
    return Locale(::rtl::OUString("en"), ::rtl::OUString("US"));
}

//-----------------------------------------------
const Locale Locale::X_NOTRANSLATE()
{
    return Locale(::rtl::OUString("x"), ::rtl::OUString("notranslate"));
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
    if (!m_sCountry.isEmpty())
    {
        sISO.append(SEPERATOR_LC);
        sISO.append(m_sCountry);

        if (!m_sVariant.isEmpty())
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

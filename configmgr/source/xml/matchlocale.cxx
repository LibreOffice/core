/*************************************************************************
 *
 *  $RCSfile: matchlocale.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: jb $ $Date: 2001-03-16 17:41:38 $
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
#include "matchlocale.hxx"

#include <algorithm>
#include <iterator>

namespace configmgr
{
// -----------------------------------------------------------------------------
    namespace localehelper
    {
// -----------------------------------------------------------------------------
        using ::rtl::OUString;
        namespace uno       = ::com::sun::star::uno;
        namespace lang      = ::com::sun::star::lang;

#define ARRAYSIZE( arr ) (sizeof(arr) / sizeof 0[arr] )
// -----------------------------------------------------------------------------
    struct StaticLocale
    {
        typedef char const * String;
        String aLanguage;
        String aCountry;
    };

    StaticLocale::String c_sDefaultLanguage = "";   // exported !
    StaticLocale::String c_sAnyLanguage = "*";      // exported !

    StaticLocale::String c_sNoCountry = "";

    StaticLocale::String c_sLanguageEnglish = "en";
    StaticLocale::String c_sCountryUS = "US";

    StaticLocale c_aFallbackLocales[] =
    {
        { c_sDefaultLanguage,   c_sNoCountry }, // the default language
        { c_sLanguageEnglish,   c_sCountryUS }, // english [cannot make 'en' better than 'en-US' :-(]
        { c_sAnyLanguage,       c_sNoCountry }  // just take the first you find
    };
    const SequencePos c_nFallbackLocales = ARRAYSIZE(c_aFallbackLocales);

// -----------------------------------------------------------------------------
    static inline sal_Int32 countrySeparatorPos(OUString const& aLocaleName_)
    {
        sal_Int32 pos = aLocaleName_.indexOf('-');
        if (pos == 1) // allow for x-LL or i-LL
            pos = aLocaleName_.indexOf('-',pos+1);

        if (pos < 0)
            pos = aLocaleName_.indexOf('_');

        return pos;
    }
    // -------------------------------------------------------------------------
    static inline sal_Int32 countryLength(OUString const& aLocaleName_, sal_Int32 nCountryPos)
    {
        sal_Int32 pos1 = aLocaleName_.indexOf('.',nCountryPos);
        sal_Int32 pos2 = aLocaleName_.indexOf('_',nCountryPos);

        if (pos1 < 0) pos1 = aLocaleName_.getLength();

        if (pos2 < 0 || pos1 < pos2)
            return pos1 - nCountryPos;

        else
            return pos2 - nCountryPos;
    }
    // -------------------------------------------------------------------------
    static inline void splitLocaleString(OUString const& aLocaleName_, OUString& rLanguage_, OUString& rCountry_)
    {
        sal_Int32 nCountryPos = countrySeparatorPos(aLocaleName_);
        if (nCountryPos >= 0)
        {
            rLanguage_ = aLocaleName_.copy(0,nCountryPos).toLowerCase();

            ++nCountryPos; // advance past separator
            sal_Int32 nCountryLength = countryLength(aLocaleName_, nCountryPos);

            rCountry_  = aLocaleName_.copy(nCountryPos,nCountryLength).toUpperCase();
        }
        else
        {
            rLanguage_ = aLocaleName_.toLowerCase();
            rCountry_  = OUString();
        }
    }
// -----------------------------------------------------------------------------

// struct Locale { OUString aLanguage, aCountry };

// -----------------------------------------------------------------------------
// conversion helpers
Locale makeLocale(OUString const& sLocaleName_)
{
    Locale aResult;
    splitLocaleString(sLocaleName_, aResult.aLanguage, aResult.aCountry);
    return aResult;
}
Locale makeLocale(lang::Locale const& aUnoLocale_)
{
    Locale aResult;
    aResult.aLanguage = aUnoLocale_.Language.toLowerCase();
    aResult.aCountry  = aUnoLocale_.Country.toUpperCase();
    return aResult;
}
static
Locale makeLocale(StaticLocale const& aConstLocale_)
{
    Locale aResult;
    aResult.aLanguage = OUString::createFromAscii(aConstLocale_.aLanguage);
    aResult.aCountry  = OUString::createFromAscii(aConstLocale_.aCountry);
    return aResult;
}
// -----------------------------------------------------------------------------
template <class T>
inline
LocaleSequence makeLocaleSeq_impl(uno::Sequence< T > const& aLocales_)
{
    sal_Int32 const nLocaleCount = aLocales_.getLength();

    T const* pLocaleBegin = aLocales_.getConstArray();

    LocaleSequence aResult;
    aResult.reserve( nLocaleCount + c_nFallbackLocales ); // make room for fallback stuff as well

    Locale (* const xlate)(T const&) = &makeLocale;

    std::transform(pLocaleBegin, pLocaleBegin + nLocaleCount, std::back_inserter(aResult), xlate);

    return aResult;
}
// -----------------------------------------------------------------------------

void addFallbackLocales(LocaleSequence& aTargetList_)
{
    Locale (* const xlate)(StaticLocale const&) = &makeLocale;

    std::transform(c_aFallbackLocales, c_aFallbackLocales + c_nFallbackLocales, std::back_inserter(aTargetList_), xlate);
}
// -----------------------------------------------------------------------------

LocaleSequence makeLocaleSequence(uno::Sequence<OUString> const& sLocaleNames_)
{
    return makeLocaleSeq_impl(sLocaleNames_);
}
LocaleSequence makeLocaleSequence(uno::Sequence<lang::Locale> const& aUnoLocales_)
{
    return makeLocaleSeq_impl(aUnoLocales_);
}
// -----------------------------------------------------------------------------
bool designatesAllLocales(Locale const& aLocale_)
{
    return aLocale_.aLanguage.compareToAscii(c_sAnyLanguage) == 0;
}
bool designatesAllLocales(LocaleSequence const& aLocales_)
{
    return  aLocales_.size() <= 1 &&
            (aLocales_.size() == 0 || designatesAllLocales(aLocales_));
}
// -----------------------------------------------------------------------------

MatchQuality match(Locale const& aLocale_, Locale const& aTarget_)
{
    // check language
    if (!aLocale_.aLanguage.equals(aTarget_.aLanguage))
    {
        // can we accept any language
        if (aTarget_.aLanguage.compareToAscii(c_sAnyLanguage) == 0)
            return MATCH_LANGUAGE;

        return MISMATCH;
    }

    // check for exact match
    else if (aLocale_.aCountry.equals(aTarget_.aCountry))
        return MATCH_LOCALE;

    // check for plain language
    else if (aLocale_.aCountry.getLength() == 0)
        return MATCH_LANGUAGE_PLAIN;

    // so we are left with the wrong language
    else
        return MATCH_LANGUAGE;
}

// -----------------------------------------------------------------------------

/// check the given position and quality, if they are an improvement
bool MatchResult::improve(SequencePos nPos_, MatchQuality eQuality_)
{
    // is this a match at all ?
    if (eQuality_ == MISMATCH)
        return false;

    // is the position worse ?
    if (nPos_ > m_nPos )
        return false;

    // is this just a non-positive quality change ?
    if (nPos_ == m_nPos && eQuality_ <= m_eQuality)
        return false;

    // Improvement found
    m_nPos      = nPos_;
    m_eQuality  = eQuality_;

    return true;
}

// -----------------------------------------------------------------------------

MatchResult match(Locale const& aLocale_, LocaleSequence const& aTarget_)
{
    SequencePos const nEnd = aTarget_.size();

    for (SequencePos nPos = 0; nPos < nEnd; ++nPos)
    {
        if (MatchQuality eQuality = match(aLocale_, aTarget_[nPos]))
        {
            return MatchResult(nPos,eQuality);
        }
    }
    return MatchResult();
}
// -----------------------------------------------------------------------------

static
inline
SequencePos getSearchLimitPosition(MatchResult const& aPrevMatch_,LocaleSequence const& aTarget_)
{
    SequencePos nSize = aTarget_.size();

    if (aPrevMatch_.isMatch())
    {
        SequencePos nMatchPos = aPrevMatch_.position();

        OSL_ENSURE(nMatchPos < nSize,"localehelper::getSearchLimitPosition: ERROR - previous position is out-of-bounds");

        if (nMatchPos < nSize)
        {
            return nMatchPos + 1;
        }
    }
    return nSize;
}
// -----------------------------------------------------------------------------

bool improveMatch(MatchResult& rMatch_, Locale const& aLocale_, LocaleSequence const& aTarget_)
{
    SequencePos const nEnd = getSearchLimitPosition(rMatch_,aTarget_);

    for (SequencePos nPos = 0; nPos < nEnd; ++nPos)
    {
        if (rMatch_.improve(nPos, match(aLocale_, aTarget_[nPos])))
        {
            return true;
        }
    }
    return false;
}
// -----------------------------------------------------------------------------


// -----------------------------------------------------------------------------
// class FindBestLocale
// -----------------------------------------------------------------------------

inline
void FindBestLocale::implSetTarget(LocaleSequence const& aTarget_)
{
    m_aTarget = aTarget_;
    addFallbackLocales(m_aTarget);
}
// -----------------------------------------------------------------------------

FindBestLocale::FindBestLocale() // use only fallbacks !
{
    LocaleSequence aSeq;
    implSetTarget( aSeq );
}
// -----------------------------------------------------------------------------

FindBestLocale::FindBestLocale(Locale const& aTarget_)
{
    LocaleSequence aSeq(1,aTarget_);
    implSetTarget( aSeq );
}
// -----------------------------------------------------------------------------


FindBestLocale::FindBestLocale(LocaleSequence const& aTarget_)
{
    implSetTarget( aTarget_ );
}
// -----------------------------------------------------------------------------

Locale FindBestLocale::getBestMatch() const
{
    OSL_ENSURE(this->isMatch(), "FindBestLocale::getBestMatch(): ERROR - no match found");

    if (this->isMatch())
    {
        OSL_ENSURE(m_aResult.position() < m_aTarget.size(), "FindBestLocale::getBestMatch(): ERROR - invalid match position");
        return m_aTarget[m_aResult.position()];
    }
    else
    {
        return Locale();
    }
}
// -----------------------------------------------------------------------------

bool FindBestLocale::accept(Locale const& aLocale_)
{
    return improveMatch(m_aResult, aLocale_, m_aTarget);
}
// -----------------------------------------------------------------------------

void FindBestLocale::reset(Locale const& aTarget_)
{
    LocaleSequence aSeq(1,aTarget_);

    this->reset(aSeq);
}
// -----------------------------------------------------------------------------

void FindBestLocale::reset(LocaleSequence const& aTarget_)
{
    implSetTarget(aTarget_);
    m_aResult.reset();
}
// -----------------------------------------------------------------------------

void FindBestLocale::reset(bool bNeedLocale_)
{
    if (bNeedLocale_)
        m_aResult.reset();

    else // mark as best match already (no improvement possible)
        m_aResult = m_aResult.best();
}
// -----------------------------------------------------------------------------

    } // namespace locale helper
// -----------------------------------------------------------------------------

} // namespace



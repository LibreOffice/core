/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: matchlocale.cxx,v $
 * $Revision: 1.12 $
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
#include "precompiled_configmgr.hxx"
#include "matchlocale.hxx"

#include <rtl/ustrbuf.hxx>

#include <algorithm>
#include <iterator>

namespace configmgr
{
// -----------------------------------------------------------------------------
    namespace localehelper
    {
// -----------------------------------------------------------------------------
        namespace uno       = ::com::sun::star::uno;
        namespace lang      = ::com::sun::star::lang;

#define ARRAYSIZE( arr ) (sizeof(arr) / sizeof 0[arr] )
// -----------------------------------------------------------------------------
    struct StaticLocale
    {
        char const * aLanguage;
        char const * aCountry;
    };

    char const * const c_sAnyLanguage = "*";            // exported !
    char const * const c_sDefLanguage = "x-default";    // exported !

    char const * const c_sNoCountry = "";

    char const * const c_sLanguageEnglish = "en";
    char const * const c_sCountryUS = "US";

    StaticLocale const c_aFallbackLocales[] =
    {
        { c_sLanguageEnglish,   c_sCountryUS }, // english [cannot make 'en' better than 'en-US' :-(]
        { c_sAnyLanguage,       c_sNoCountry }  // just take the first you find
    };
    std::vector< com::sun::star::lang::Locale >::size_type const c_nFallbackLocales = ARRAYSIZE(c_aFallbackLocales);

// -----------------------------------------------------------------------------
    bool isAnyLanguage(rtl::OUString const & _sLanguage)
    {
        return !!_sLanguage.equalsAscii(c_sAnyLanguage);
    }

// -----------------------------------------------------------------------------
    bool isDefaultLanguage(rtl::OUString const & _sLanguage)
    {
        return !!_sLanguage.equalsAscii(c_sDefLanguage);
    }

// -----------------------------------------------------------------------------
    rtl::OUString getAnyLanguage()
    {
        return rtl::OUString::createFromAscii( c_sAnyLanguage );
    }

// -----------------------------------------------------------------------------
    rtl::OUString getDefaultLanguage()
    {
        return rtl::OUString::createFromAscii( c_sDefLanguage );
    }

// -----------------------------------------------------------------------------
    com::sun::star::lang::Locale getAnyLocale()
    {
        return com::sun::star::lang::Locale( getAnyLanguage(), rtl::OUString(), rtl::OUString() );
    }

// -----------------------------------------------------------------------------
    com::sun::star::lang::Locale getDefaultLocale()
    {
        return com::sun::star::lang::Locale( getDefaultLanguage(), rtl::OUString(), rtl::OUString() );
    }

// -----------------------------------------------------------------------------
    static inline sal_Int32 countrySeparatorPos(rtl::OUString const& aLocaleName_)
    {
        sal_Int32 pos = aLocaleName_.indexOf('-');
        if (pos == 1) // allow for x-LL or i-LL
            pos = aLocaleName_.indexOf('-',pos+1);

        if (pos < 0)
            pos = aLocaleName_.indexOf('_');

        return pos;
    }
    // -------------------------------------------------------------------------
    static inline sal_Int32 countryLength(rtl::OUString const& aLocaleName_, sal_Int32 nCountryPos)
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
    static inline void splitLocaleString(rtl::OUString const& aLocaleName_, rtl::OUString& rLanguage_, rtl::OUString& rCountry_)
    {
        sal_Int32 nCountryPos = countrySeparatorPos(aLocaleName_);
        if (nCountryPos >= 0)
        {
            rLanguage_ = aLocaleName_.copy(0,nCountryPos).toAsciiLowerCase();

            ++nCountryPos; // advance past separator
            sal_Int32 nCountryLength = countryLength(aLocaleName_, nCountryPos);

            rCountry_  = aLocaleName_.copy(nCountryPos,nCountryLength).toAsciiUpperCase();
        }
        else
        {
            rLanguage_ = aLocaleName_.toAsciiLowerCase();
            rCountry_  = rtl::OUString();
        }
    }
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// conversion helpers
com::sun::star::lang::Locale makeLocale(rtl::OUString const& sLocaleName_)
{
    com::sun::star::lang::Locale aResult;
    splitLocaleString(sLocaleName_, aResult.Language, aResult.Country);
    return aResult;
}
rtl::OUString makeIsoLocale(com::sun::star::lang::Locale const& aUnoLocale_)
{
    rtl::OUStringBuffer aResult(aUnoLocale_.Language.toAsciiLowerCase());
    if (aUnoLocale_.Country.getLength())
    {
        aResult.append( sal_Unicode('-') ).append(aUnoLocale_.Country.toAsciiUpperCase());
    }
    return aResult.makeStringAndClear();
}
static
com::sun::star::lang::Locale makeLocale(StaticLocale const& aConstLocale_)
{
    com::sun::star::lang::Locale aResult;
    aResult.Language = rtl::OUString::createFromAscii(aConstLocale_.aLanguage);
    aResult.Country  = rtl::OUString::createFromAscii(aConstLocale_.aCountry);
    return aResult;
}
// -----------------------------------------------------------------------------
template <class T>
inline
void addLocaleSeq_impl(T const* first, T const* last, std::vector< com::sun::star::lang::Locale >& rSeq)
{
    com::sun::star::lang::Locale (* const xlate)(T const&) = &makeLocale;

    std::transform(first, last, std::back_inserter(rSeq), xlate);
}
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
template <class T>
inline
std::vector< com::sun::star::lang::Locale > makeLocaleSeq_impl(uno::Sequence< T > const& aLocales_)
{
    sal_Int32 const nLocaleCount = aLocales_.getLength();

    T const* pLocaleBegin = aLocales_.getConstArray();

    std::vector< com::sun::star::lang::Locale > aResult;
    aResult.reserve( nLocaleCount + c_nFallbackLocales ); // make room for fallback stuff as well

    addLocaleSeq_impl(pLocaleBegin, pLocaleBegin + nLocaleCount, aResult);

    return aResult;
}
// -----------------------------------------------------------------------------

void addFallbackLocales(std::vector< com::sun::star::lang::Locale >& aTargetList_)
{
    addLocaleSeq_impl(c_aFallbackLocales, c_aFallbackLocales + c_nFallbackLocales, aTargetList_);
}
// -----------------------------------------------------------------------------

std::vector< com::sun::star::lang::Locale > makeLocaleSequence(uno::Sequence<rtl::OUString> const& sLocaleNames_)
{
    return makeLocaleSeq_impl(sLocaleNames_);
}
// -----------------------------------------------------------------------------

uno::Sequence<rtl::OUString> makeIsoSequence(std::vector< com::sun::star::lang::Locale > const& aLocales_)
{
    std::vector< com::sun::star::lang::Locale >::size_type const nLocaleCount = aLocales_.size();
    sal_Int32 const nSeqSize = sal_Int32(nLocaleCount);
    OSL_ASSERT( nSeqSize >= 0 && sal_uInt32(nSeqSize) == nLocaleCount );

    uno::Sequence<rtl::OUString> aResult(nSeqSize);
    std::transform(aLocales_.begin(), aLocales_.end(), aResult.getArray(), &makeIsoLocale);

    return aResult;
}
// -----------------------------------------------------------------------------
bool designatesAllLocales(com::sun::star::lang::Locale const& aLocale_)
{
    return aLocale_.Language.equalsAscii(c_sAnyLanguage);
}
bool designatesAllLocales(std::vector< com::sun::star::lang::Locale > const& aLocales_)
{
    return  aLocales_.size() <= 1 &&
            (aLocales_.size() == 0 || designatesAllLocales(aLocales_));
}
// -----------------------------------------------------------------------------

MatchQuality match(com::sun::star::lang::Locale const& aLocale_, com::sun::star::lang::Locale const& aTarget_)
{
    // check language
    if (!aLocale_.Language.equals(aTarget_.Language))
    {
        // can we accept any language
        if (aTarget_.Language.equalsAscii(c_sAnyLanguage))
            return MATCH_LANGUAGE;

        return MISMATCH;
    }

    // check for exact match
    else if (aLocale_.Country.equals(aTarget_.Country))
        return MATCH_LOCALE;

    // check for plain language
    else if (aLocale_.Country.getLength() == 0)
        return MATCH_LANGUAGE_PLAIN;

    // so we are left with the wrong country
    else
        return MATCH_LANGUAGE;
}

// -----------------------------------------------------------------------------

/// check the given position and quality, if they are an improvement
bool MatchResult::improve(std::vector< com::sun::star::lang::Locale >::size_type nPos_, MatchQuality eQuality_)
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

bool isMatch(com::sun::star::lang::Locale const& aLocale_, std::vector< com::sun::star::lang::Locale > const& aTarget_, MatchQuality eRequiredQuality_)
{
    std::vector< com::sun::star::lang::Locale >::size_type const nEnd = aTarget_.size();

    for (std::vector< com::sun::star::lang::Locale >::size_type nPos = 0; nPos < nEnd; ++nPos)
    {
        MatchQuality eQuality = match(aLocale_, aTarget_[nPos]);
        if (eQuality >= eRequiredQuality_)
        {
            return true;
        }
    }
    return false;
}
// -----------------------------------------------------------------------------

static
inline
std::vector< com::sun::star::lang::Locale >::size_type getSearchLimitPosition(MatchResult const& aPrevMatch_,std::vector< com::sun::star::lang::Locale > const& aTarget_)
{
    std::vector< com::sun::star::lang::Locale >::size_type nSize = aTarget_.size();

    if (aPrevMatch_.isMatch())
    {
        std::vector< com::sun::star::lang::Locale >::size_type nMatchPos = aPrevMatch_.position();

        OSL_ENSURE(nMatchPos < nSize,"localehelper::getSearchLimitPosition: ERROR - previous position is out-of-bounds");

        if (nMatchPos < nSize)
        {
            return nMatchPos + 1;
        }
    }
    return nSize;
}
// -----------------------------------------------------------------------------

bool improveMatch(MatchResult& rMatch_, com::sun::star::lang::Locale const& aLocale_, std::vector< com::sun::star::lang::Locale > const& aTarget_)
{
    std::vector< com::sun::star::lang::Locale >::size_type const nEnd = getSearchLimitPosition(rMatch_,aTarget_);

    for (std::vector< com::sun::star::lang::Locale >::size_type nPos = 0; nPos < nEnd; ++nPos)
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
void FindBestLocale::implSetTarget(std::vector< com::sun::star::lang::Locale > const& aTarget_)
{
    m_aTarget = aTarget_;
    addFallbackLocales(m_aTarget);
}
// -----------------------------------------------------------------------------

FindBestLocale::FindBestLocale(com::sun::star::lang::Locale const& aTarget_)
{
    std::vector< com::sun::star::lang::Locale > aSeq(1,aTarget_);
    implSetTarget( aSeq );
}
// -----------------------------------------------------------------------------

bool FindBestLocale::accept(com::sun::star::lang::Locale const& aLocale_)
{
    return improveMatch(m_aResult, aLocale_, m_aTarget);
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



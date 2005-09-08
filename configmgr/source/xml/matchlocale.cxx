/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: matchlocale.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 04:41:02 $
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

    StaticLocale::String const c_sAnyLanguage = "*";            // exported !
    StaticLocale::String const c_sDefLanguage = "x-default";    // exported !

    StaticLocale::String const c_sNoCountry = "";

    StaticLocale::String const c_sLanguageEnglish = "en";
    StaticLocale::String const c_sCountryUS = "US";

    StaticLocale const c_aFallbackLocales[] =
    {
        { c_sLanguageEnglish,   c_sCountryUS }, // english [cannot make 'en' better than 'en-US' :-(]
        { c_sAnyLanguage,       c_sNoCountry }  // just take the first you find
    };
    SequencePos const c_nFallbackLocales = ARRAYSIZE(c_aFallbackLocales);

// -----------------------------------------------------------------------------
    bool isAnyLanguage(OUString const & _sLanguage)
    {
        return !!_sLanguage.equalsAscii(c_sAnyLanguage);
    }

// -----------------------------------------------------------------------------
    bool isDefaultLanguage(OUString const & _sLanguage)
    {
        return !!_sLanguage.equalsAscii(c_sDefLanguage);
    }

// -----------------------------------------------------------------------------
    OUString getAnyLanguage()
    {
        return OUString::createFromAscii( c_sAnyLanguage );
    }

// -----------------------------------------------------------------------------
    OUString getDefaultLanguage()
    {
        return OUString::createFromAscii( c_sDefLanguage );
    }

// -----------------------------------------------------------------------------
    Locale getAnyLocale()
    {
        return Locale( getAnyLanguage(), OUString(), OUString() );
    }

// -----------------------------------------------------------------------------
    Locale getDefaultLocale()
    {
        return Locale( getDefaultLanguage(), OUString(), OUString() );
    }

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
            rLanguage_ = aLocaleName_.copy(0,nCountryPos).toAsciiLowerCase();

            ++nCountryPos; // advance past separator
            sal_Int32 nCountryLength = countryLength(aLocaleName_, nCountryPos);

            rCountry_  = aLocaleName_.copy(nCountryPos,nCountryLength).toAsciiUpperCase();
        }
        else
        {
            rLanguage_ = aLocaleName_.toAsciiLowerCase();
            rCountry_  = OUString();
        }
    }
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// conversion helpers
Locale makeLocale(OUString const& sLocaleName_)
{
    Locale aResult;
    splitLocaleString(sLocaleName_, aResult.Language, aResult.Country);
    return aResult;
}
Locale makeLocale(Locale const& aLocale_) // normalizes the locale
{
    return Locale(  aLocale_.Language.toAsciiLowerCase(),
                    aLocale_.Country .toAsciiUpperCase(),
                    OUString() );
}
OUString makeIsoLocale(Locale const& aUnoLocale_)
{
    rtl::OUStringBuffer aResult(aUnoLocale_.Language.toAsciiLowerCase());
    if (aUnoLocale_.Country.getLength())
    {
        aResult.append( sal_Unicode('-') ).append(aUnoLocale_.Country.toAsciiUpperCase());
    }
    return aResult.makeStringAndClear();
}
static
Locale makeLocale(StaticLocale const& aConstLocale_)
{
    Locale aResult;
    aResult.Language = OUString::createFromAscii(aConstLocale_.aLanguage);
    aResult.Country  = OUString::createFromAscii(aConstLocale_.aCountry);
    return aResult;
}
// -----------------------------------------------------------------------------
template <class T>
inline
void addLocaleSeq_impl(T const* first, T const* last, LocaleSequence& rSeq)
{
    typedef Locale (* const XlateFunc)(T const&);
    XlateFunc xlate = &makeLocale;

    std::transform(first, last, std::back_inserter(rSeq), xlate);
}
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
template <class T>
inline
LocaleSequence makeLocaleSeq_impl(uno::Sequence< T > const& aLocales_)
{
    sal_Int32 const nLocaleCount = aLocales_.getLength();

    T const* pLocaleBegin = aLocales_.getConstArray();

    LocaleSequence aResult;
    aResult.reserve( nLocaleCount + c_nFallbackLocales ); // make room for fallback stuff as well

    addLocaleSeq_impl(pLocaleBegin, pLocaleBegin + nLocaleCount, aResult);

    return aResult;
}
// -----------------------------------------------------------------------------

void addFallbackLocales(LocaleSequence& aTargetList_)
{
    addLocaleSeq_impl(c_aFallbackLocales, c_aFallbackLocales + c_nFallbackLocales, aTargetList_);
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

uno::Sequence<OUString> makeIsoSequence(LocaleSequence const& aLocales_)
{
    LocaleSequence::size_type const nLocaleCount = aLocales_.size();
    sal_Int32 const nSeqSize = sal_Int32(nLocaleCount);
    OSL_ASSERT( nSeqSize >= 0 && sal_uInt32(nSeqSize) == nLocaleCount );

    uno::Sequence<OUString> aResult(nSeqSize);
    std::transform(aLocales_.begin(), aLocales_.end(), aResult.getArray(), &makeIsoLocale);

    return aResult;
}
uno::Sequence<Locale>   makeUnoSequence(LocaleSequence const& aLocales_)
{
    LocaleSequence::size_type const nLocaleCount = aLocales_.size();
    sal_Int32 const nSeqSize = sal_Int32(nLocaleCount);
    OSL_ASSERT( nSeqSize >= 0 && sal_uInt32(nSeqSize) == nLocaleCount );

    uno::Sequence<Locale> aResult( &aLocales_.front(), nSeqSize );
    return aResult;
}
// -----------------------------------------------------------------------------
bool designatesAllLocales(Locale const& aLocale_)
{
    return aLocale_.Language.equalsAscii(c_sAnyLanguage);
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

bool isMatch(Locale const& aLocale_, LocaleSequence const& aTarget_, MatchQuality eRequiredQuality_)
{
    SequencePos const nEnd = aTarget_.size();

    for (SequencePos nPos = 0; nPos < nEnd; ++nPos)
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



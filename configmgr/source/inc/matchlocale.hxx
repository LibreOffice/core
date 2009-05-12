/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: matchlocale.hxx,v $
 * $Revision: 1.10 $
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

/* PLEASE DON'T DELETE ANY COMMENT LINES, ALSO IT'S UNNECESSARY. */

#ifndef CONFIGMGR_MATCHLOCALE_HXX
#define CONFIGMGR_MATCHLOCALE_HXX

#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/lang/Locale.hpp>

#include <rtl/ustring.hxx>
#include <vector>

namespace configmgr
{
// -----------------------------------------------------------------------------

    namespace localehelper
    {
    // -------------------------------------------------------------------------
        namespace uno       = ::com::sun::star::uno;
        namespace lang      = ::com::sun::star::lang;

    // -------------------------------------------------------------------------
        extern char const * const c_sAnyLanguage;
        extern char const * const c_sDefLanguage;

        extern bool isAnyLanguage(rtl::OUString const & _sLanguage);
        extern bool isDefaultLanguage(rtl::OUString const & _sLanguage);

        extern rtl::OUString getAnyLanguage();
        extern rtl::OUString getDefaultLanguage();

        extern com::sun::star::lang::Locale getAnyLocale();
        extern com::sun::star::lang::Locale getDefaultLocale();

    // -------------------------------------------------------------------------
        // the max value of std::vector< com::sun::star::lang::Locale >::size_type - marks out-of-range (Value == -1 == ~0)
        std::vector< com::sun::star::lang::Locale >::size_type const c_noPosition = std::vector< com::sun::star::lang::Locale >::size_type(0)-std::vector< com::sun::star::lang::Locale >::size_type(1);

        // conversion helpers
        com::sun::star::lang::Locale makeLocale(rtl::OUString const& sLocaleName_);
        rtl::OUString makeIsoLocale(com::sun::star::lang::Locale const& aUnoLocale_);

        std::vector< com::sun::star::lang::Locale > makeLocaleSequence(uno::Sequence<rtl::OUString> const& sLocaleNames_);
        uno::Sequence<rtl::OUString> makeIsoSequence(std::vector< com::sun::star::lang::Locale > const& aLocales_);

        inline
        bool equalLocale(com::sun::star::lang::Locale const & lhs, com::sun::star::lang::Locale const & rhs)
        { return lhs.Language == rhs.Language && lhs.Country == rhs.Country; }

        inline
        bool equalLanguage(com::sun::star::lang::Locale const & lhs, com::sun::star::lang::Locale const & rhs)
        { return lhs.Language == rhs.Language; }
    // -------------------------------------------------------------------------
        bool designatesAllLocales(com::sun::star::lang::Locale const& aLocale_);
        bool designatesAllLocales(std::vector< com::sun::star::lang::Locale > const& aLocales_);
    // -------------------------------------------------------------------------

        /// result of matching a locale against a target locale
        enum  MatchQuality
        {
            MISMATCH = 0,           /// match: locales do not match (must be zero!)
            MATCH_LANGUAGE,         /// match: languages match - country mismatch
            MATCH_LANGUAGE_PLAIN,   /// match: languages match - no country to match
            MATCH_LOCALE,           /// match: full match
            BEST_MATCH = MATCH_LOCALE
        };

        /// compare two locales for 'nearness'
        MatchQuality match(com::sun::star::lang::Locale const& aLocale_, com::sun::star::lang::Locale const& aTarget_);


    // -------------------------------------------------------------------------
        /// result of matching a Locale against a target sequence of locales
        class MatchResult
        {
            std::vector< com::sun::star::lang::Locale >::size_type      m_nPos;
            MatchQuality    m_eQuality;

        public:
            /// construct a default (no match) result
            MatchResult()
            { reset(); }

            /// construct a result from given parameters - use with care
            MatchResult(std::vector< com::sun::star::lang::Locale >::size_type nPos_, MatchQuality eQuality_)
            : m_nPos( nPos_ )
            , m_eQuality(eQuality_)
            {}

            /// construct an optimum result
            static MatchResult best() { return MatchResult(0,MATCH_LOCALE); }

            /// has there been a match
            bool isMatch() const { return m_eQuality != MISMATCH; }
            /// is this the best match possible ?
            bool isBest() const { return m_nPos == 0 && m_eQuality == MATCH_LOCALE; }

            /// retrieve the position that was matched
            std::vector< com::sun::star::lang::Locale >::size_type position() const { return m_nPos; }
            /// retrieve the quality of match
            MatchQuality quality() const { return m_eQuality; }

            /// assign the given position and quality, if they are an improvement
            bool improve(std::vector< com::sun::star::lang::Locale >::size_type nPos, MatchQuality eQuality_);

            /// reset to no match or best match state
            void reset()
            {
                m_nPos =  c_noPosition;
                m_eQuality = MISMATCH;
            }

            // ---------------------------------------------------------------------
            // comparing MatchResults
            friend bool operator ==(MatchResult const& lhs, MatchResult const& rhs)
            {
                return  lhs.m_nPos == rhs.m_nPos &&
                        lhs.m_eQuality == rhs.m_eQuality;
            }

            // ordering of MatchResults - greater is better
            friend bool operator < (MatchResult const& lhs, MatchResult const& rhs)
            {
                if (lhs.m_nPos > rhs.m_nPos) return true; // greater position is worse
                if (lhs.m_nPos < rhs.m_nPos) return false;

                return (lhs.m_eQuality < rhs.m_eQuality);  // least Quality is worse
            }

        };

        // ---------------------------------------------------------------------
        // derived relational operators
        inline bool operator !=(MatchResult const& lhs, MatchResult const& rhs)
        { return !(lhs == rhs); }
        inline bool operator > (MatchResult const& lhs, MatchResult const& rhs)
        { return rhs < lhs; }
        inline bool operator <= (MatchResult const& lhs, MatchResult const& rhs)
        { return !(rhs < lhs); }
        inline bool operator >=(MatchResult const& lhs, MatchResult const& rhs)
        { return !(lhs < rhs); }

        /// improve an existing match of a locale against a sequence of locales
        bool improveMatch(MatchResult& rMatch_, com::sun::star::lang::Locale const& aLocale_, std::vector< com::sun::star::lang::Locale > const& aTarget_);

        /// match a locale against a sequence of locales for a given quality level
        bool isMatch(com::sun::star::lang::Locale const& aLocales, std::vector< com::sun::star::lang::Locale > const& aTarget_, MatchQuality eRequiredQuality_);

    // -------------------------------------------------------------------------
        /// add defaults to a sequence of locales
        void addFallbackLocales(std::vector< com::sun::star::lang::Locale >& aTargetList_);

    // -------------------------------------------------------------------------
        class FindBestLocale
        {
        public:
            /// construct a MatchLocale with a single target locale
            FindBestLocale(com::sun::star::lang::Locale const& aTarget_);

            /// is there any match ?
            bool isMatch() const { return m_aResult.isMatch(); }

            /// is there an optimum match (so we are done) ?
            bool isBestMatch() const { return m_aResult.isBest(); }

            /// get the quality of the best match found
            MatchQuality getMatchQuality() const { return m_aResult.quality(); }

            /// check, if the given locale improves the quality. if it does, accept it
            bool accept(com::sun::star::lang::Locale const& aLocale_);

            /// reset the match result, indicating whether a match is needed at all
            void reset(bool bNeedLocale_ = true);

        private:
            void implSetTarget(std::vector< com::sun::star::lang::Locale > const& aTarget_);

            std::vector< com::sun::star::lang::Locale > m_aTarget;
            MatchResult     m_aResult;
        };

    } // namespace
// -----------------------------------------------------------------------------

} // namespace

#endif


/*************************************************************************
 *
 *  $RCSfile: matchlocale.hxx,v $
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

/* PLEASE DON'T DELETE ANY COMMENT LINES, ALSO IT'S UNNECESSARY. */

#ifndef CONFIGMGR_MATCHLOCALE_HXX
#define CONFIGMGR_MATCHLOCALE_HXX

#include <com/sun/star/uno/Sequence.hxx>

#ifndef _COM_SUN_STAR_LANG_LOCALE_HPP_
#include <com/sun/star/lang/Locale.hpp>
#endif

#include <rtl/ustring.hxx>
#include <vector>

namespace configmgr
{
// -----------------------------------------------------------------------------

    namespace localehelper
    {
    // -------------------------------------------------------------------------
        using ::rtl::OUString;
        namespace uno       = ::com::sun::star::uno;
        namespace lang      = ::com::sun::star::lang;

    // -------------------------------------------------------------------------
        extern char const * c_sDefaultLanguage;
        extern char const * c_sAnyLanguage;

    // -------------------------------------------------------------------------
    /// Type for storing a Locale. May in future be upgraded to a struct
        struct Locale { OUString aLanguage, aCountry; };

    /// Type for storing a list of Locales
        typedef std::vector< Locale > LocaleSequence;
        typedef LocaleSequence::size_type SequencePos;

        // the max value of SequencePos - marks out-of-range (Value == -1 == ~0)
        SequencePos const c_noPosition = SequencePos(0)-SequencePos(1);

        // conversion helpers
        Locale makeLocale(OUString const& sLocaleName_);
        Locale makeLocale(lang::Locale const& aUnoLocale_);

        LocaleSequence makeLocaleSequence(uno::Sequence<OUString> const& sLocaleNames_);
        LocaleSequence makeLocaleSequence(uno::Sequence<lang::Locale> const& aUnoLocales_);

    // -------------------------------------------------------------------------
        bool designatesAllLocales(Locale const& aLocale_);
        bool designatesAllLocales(LocaleSequence const& aLocales_);
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
        MatchQuality match(Locale const& aLocale_, Locale const& aTarget_);


    // -------------------------------------------------------------------------
        /// result of matching a Locale against a target sequence of locales
        class MatchResult
        {
            SequencePos     m_nPos;
            MatchQuality    m_eQuality;

        public:
            /// construct a default (no match) result
            MatchResult()
            { reset(); }

            /// construct a result from given parameters - use with care
            MatchResult(SequencePos nPos_, MatchQuality eQuality_)
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
            SequencePos position() const { return m_nPos; }
            /// retrieve the quality of match
            MatchQuality quality() const { return m_eQuality; }

            /// assign the given position and quality, if they are an improvement
            bool improve(SequencePos nPos, MatchQuality eQuality_);

            /// reset to no match or best match state
            bool reset()
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

        // ---------------------------------------------------------------------
        /// match a locale against a sequence of locales
        MatchResult match(Locale const& aLocale_, LocaleSequence const& aTarget_);

        /// improve an existing match of a locale against a sequence of locales
        bool improveMatch(MatchResult& rMatch_, Locale const& aLocale_, LocaleSequence const& aTarget_);

    // -------------------------------------------------------------------------
        /// add defaults to a sequence of locales
        void addFallbackLocales(LocaleSequence& aTargetList_);

    // -------------------------------------------------------------------------
        class FindBestLocale
        {
        public:
            /// construct a MatchLocale with no target locale
            FindBestLocale();
            /// construct a MatchLocale with a single target locale
            FindBestLocale(Locale const& aTarget_);
            /// construct a MatchLocale with a sequence of locales
            FindBestLocale(LocaleSequence const& aTarget_);

            /// is there any match ?
            bool isMatch() const { return m_aResult.isMatch(); }

            /// is there an optimum match (so we are done) ?
            bool isBestMatch() const { return m_aResult.isBest(); }

            /// get the best match found
            Locale getBestMatch() const;

            /// get the quality of the best match found
            MatchQuality getMatchQuality() const { return m_aResult.quality(); }

            /// check, if the given locale improves the quality. if it does, accept it
            bool accept(Locale const& aLocale_);

            /// change the target locale (and reset matching)
            void reset(Locale const& aTarget_);

            /// change the target locales (and reset matching)
            void reset(LocaleSequence const& aTarget_);

            /// reset the match result, indicating whether a match is needed at all
            void reset(bool bNeedLocale_ = true);

        private:
            void implSetTarget(LocaleSequence const& aTarget_);

            LocaleSequence  m_aTarget;
            MatchResult     m_aResult;
        };

    } // namespace
// -----------------------------------------------------------------------------

} // namespace

#endif


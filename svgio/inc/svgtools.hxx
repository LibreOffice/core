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

#pragma once

#include <basegfx/color/bcolor.hxx>
#include <basegfx/range/b2drange.hxx>
#include <basegfx/vector/b2ivector.hxx>
#include <rtl/ustrbuf.hxx>
#include "svgpaint.hxx"
#include "SvgNumber.hxx"

#include <string_view>
#include <vector>


namespace svgio::svgreader
    {
        // common non-token strings
        struct commonStrings
        {
            static constexpr OUString aStrUserSpaceOnUse = u"userSpaceOnUse"_ustr;
            static constexpr OUString aStrObjectBoundingBox = u"objectBoundingBox"_ustr;
            static constexpr OUString aStrNonzero = u"nonzero"_ustr;
            static constexpr OUString aStrEvenOdd = u"evenodd"_ustr;
        };

        enum class SvgUnits
        {
            userSpaceOnUse,
            objectBoundingBox
        };

        enum class SvgAlign
        {
            none,
            xMinYMin,
            xMidYMin,
            xMaxYMin,
            xMinYMid,
            xMidYMid, // default
            xMaxYMid,
            xMinYMax,
            xMidYMax,
            xMaxYMax
        };

        class SvgAspectRatio
        {
        private:
            SvgAlign    maSvgAlign;

            bool        mbMeetOrSlice : 1; // true = meet (default), false = slice
            bool        mbSet : 1;

        public:
            SvgAspectRatio()
            :   maSvgAlign(SvgAlign::xMidYMid),
                mbMeetOrSlice(true),
                mbSet(false)
            {
            }

            SvgAspectRatio(SvgAlign aSvgAlign, bool bMeetOrSlice)
            :   maSvgAlign(aSvgAlign),
                mbMeetOrSlice(bMeetOrSlice),
                mbSet(true)
            {
            }

            /// data read access
            SvgAlign getSvgAlign() const { return maSvgAlign; }
            bool isMeetOrSlice() const { return mbMeetOrSlice; }
            bool isSet() const { return mbSet; }

            /// tooling
            static basegfx::B2DHomMatrix createLinearMapping(const basegfx::B2DRange& rTarget, const basegfx::B2DRange& rSource);
            basegfx::B2DHomMatrix createMapping(const basegfx::B2DRange& rTarget, const basegfx::B2DRange& rSource) const;
        };

        void skip_char(std::u16string_view rCandidate, sal_Unicode aChar, sal_Int32& nPos, const sal_Int32 nLen);
        void skip_char(std::u16string_view rCandidate, sal_Unicode aCharA, sal_Unicode nCharB, sal_Int32& nPos, const sal_Int32 nLen);
        void copySign(std::u16string_view rCandidate, sal_Int32& nPos, OUStringBuffer& rTarget, const sal_Int32 nLen);
        void copyNumber(std::u16string_view rCandidate, sal_Int32& nPos, OUStringBuffer& rTarget, const sal_Int32 nLen);
        void copyHex(std::u16string_view rCandidate, sal_Int32& nPos, OUStringBuffer& rTarget, const sal_Int32 nLen);
        void copyString(std::u16string_view rCandidate, sal_Int32& nPos, OUStringBuffer& rTarget, const sal_Int32 nLen);
        void copyToLimiter(std::u16string_view rCandidate, sal_Unicode aLimiter, sal_Int32& nPos, OUStringBuffer& rTarget, const sal_Int32 nLen);
        bool readNumber(std::u16string_view rCandidate, sal_Int32& nPos, double& fNum, const sal_Int32 nLen);
        SvgUnit readUnit(std::u16string_view rCandidate, sal_Int32& nPos, const sal_Int32 nLen);
        bool readNumberAndUnit(std::u16string_view rCandidate, sal_Int32& nPos, SvgNumber& aNum, const sal_Int32 nLen);
        bool readAngle(std::u16string_view rCandidate, sal_Int32& nPos, double& fAngle, const sal_Int32 nLen);
        sal_Int32 read_hex(sal_Unicode aChar);
        bool match_colorKeyword(basegfx::BColor& rColor, const OUString& rName);
        bool read_color(const OUString& rCandidate, basegfx::BColor& rColor, SvgNumber& rOpacity);
        basegfx::B2DRange readViewBox(std::u16string_view rCandidate, InfoProvider const & rInfoProvider);
        std::vector<double> readFilterMatrix(std::u16string_view rCandidate, InfoProvider const & rInfoProvider);
        basegfx::B2DHomMatrix readTransform(std::u16string_view rCandidate, InfoProvider const & rInfoProvider);
        bool readSingleNumber(std::u16string_view rCandidate, SvgNumber& aNum);
        bool readLocalLink(std::u16string_view rCandidate, OUString& rURL);
        bool readLocalUrl(const OUString& rCandidate, OUString& rURL);
        bool readSvgPaint(const OUString& rCandidate, SvgPaint& rSvgPaint, OUString& rURL, SvgNumber& rOpacity);

        bool readSvgNumberVector(std::u16string_view rCandidate, SvgNumberVector& rSvgNumberVector);
        ::std::vector< double > solveSvgNumberVector(const SvgNumberVector& rInput, const InfoProvider& rInfoProvider);

        SvgAspectRatio readSvgAspectRatio(std::u16string_view rCandidate);

        typedef ::std::vector< OUString > SvgStringVector;
        bool readSvgStringVector(std::u16string_view rCandidate, SvgStringVector& rSvgStringVector, sal_Unicode nSeparator);

        void readImageLink(const OUString& rCandidate, OUString& rXLink, OUString& rUrl, OUString& rData);

        OUString consolidateContiguousSpace(const OUString& rCandidate);

        // #125325# removes block comment of the general form '/* ... */', returns
        // an adapted string or the original if no comments included
        OUString removeBlockComments(const OUString& rCandidate);

} // end of namespace svgio::svgreader

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

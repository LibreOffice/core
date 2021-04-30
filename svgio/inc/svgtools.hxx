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

#ifndef INCLUDED_SVGIO_INC_SVGTOOLS_HXX
#define INCLUDED_SVGIO_INC_SVGTOOLS_HXX

#include <basegfx/color/bcolor.hxx>
#include <basegfx/range/b2drange.hxx>
#include <basegfx/vector/b2ivector.hxx>
#include <rtl/ustrbuf.hxx>
#include "svgpaint.hxx"

#include <string_view>
#include <vector>

namespace svgio::svgreader
    {

// recommended value for this device dependent unit, see CSS2 section 4.3.2 Lengths
#define F_SVG_PIXEL_PER_INCH  96.0

        // common non-token strings
        struct commonStrings
        {
            static constexpr OUStringLiteral aStrUserSpaceOnUse = u"userSpaceOnUse";
            static constexpr OUStringLiteral aStrObjectBoundingBox = u"objectBoundingBox";
            static constexpr OUStringLiteral aStrNonzero = u"nonzero";
            static constexpr OUStringLiteral aStrEvenOdd = u"evenodd";
        };

        enum class SvgUnits
        {
            userSpaceOnUse,
            objectBoundingBox
        };

        enum class NumberType
        {
            xcoordinate,
            ycoordinate,
            length
        };

        class InfoProvider
        {
        public:
            virtual ~InfoProvider() {}
            virtual basegfx::B2DRange getCurrentViewPort() const = 0;
            /// return font size of node inherited from parents
            virtual double getCurrentFontSizeInherited() const = 0;
            /// return xheight of node inherited from parents
            virtual double getCurrentXHeightInherited() const = 0;
        };

        enum class SvgUnit
        {
            em = 0,    // relative to current font size
            ex,        // relative to current x-height

            px,        // 'user unit'
            pt,        // points, 1.25 px
            pc,        // 15.0 px
            cm,        // 35.43307 px
            mm,        // 3.543307 px
            in,        // 90 px

            percent,   // relative to range
            none       // for stroke-miterlimit, which has no unit
        };

        class SvgNumber
        {
        private:
            double      mfNumber;
            SvgUnit     meUnit;

            bool        mbSet : 1;

        public:
            SvgNumber()
            :   mfNumber(0.0),
                meUnit(SvgUnit::px),
                mbSet(false)
            {
            }

            SvgNumber(double fNum, SvgUnit aSvgUnit = SvgUnit::px, bool bSet = true)
            :   mfNumber(fNum),
                meUnit(aSvgUnit),
                mbSet(bSet)
            {
            }

            double getNumber() const
            {
                return mfNumber;
            }

            SvgUnit getUnit() const
            {
                return meUnit;
            }

            bool isSet() const
            {
                return mbSet;
            }

            bool isPositive() const;

            // Only usable in cases, when the unit is not SvgUnit::percent, otherwise use method solve
            double solveNonPercentage(const InfoProvider& rInfoProvider) const;

            double solve(const InfoProvider& rInfoProvider, NumberType aNumberType = NumberType::length) const;


        };

        typedef ::std::vector< SvgNumber > SvgNumberVector;

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
        bool readAngle(const OUString& rCandidate, sal_Int32& nPos, double& fAngle, const sal_Int32 nLen);
        sal_Int32 read_hex(sal_Unicode aChar);
        bool match_colorKeyword(basegfx::BColor& rColor, const OUString& rName, bool bCaseIndependent);
        bool read_color(const OUString& rCandidate, basegfx::BColor& rColor, bool bCaseIndependent, SvgNumber& rOpacity);
        basegfx::B2DRange readViewBox(const OUString& rCandidate, InfoProvider const & rInfoProvider);
        basegfx::B2DHomMatrix readTransform(const OUString& rCandidate, InfoProvider const & rInfoProvider);
        bool readSingleNumber(const OUString& rCandidate, SvgNumber& aNum);
        bool readLocalUrl(const OUString& rCandidate, OUString& rURL);
        bool readSvgPaint(const OUString& rCandidate, SvgPaint& rSvgPaint, OUString& rURL, bool bCaseIndependent, SvgNumber& rOpacity);

        bool readSvgNumberVector(const OUString& rCandidate, SvgNumberVector& rSvgNumberVector);
        ::std::vector< double > solveSvgNumberVector(const SvgNumberVector& rInput, const InfoProvider& rInfoProvider);

        SvgAspectRatio readSvgAspectRatio(const OUString& rCandidate);

        typedef ::std::vector< OUString > SvgStringVector;
        bool readSvgStringVector(const OUString& rCandidate, SvgStringVector& rSvgStringVector);

        void readImageLink(const OUString& rCandidate, OUString& rXLink, OUString& rUrl, OUString& rMimeType, OUString& rData);

        OUString convert(const OUString& rCandidate, sal_Unicode nPattern, sal_Unicode nNew, bool bRemove);
        OUString consolidateContiguousSpace(const OUString& rCandidate);
        OUString whiteSpaceHandlingDefault(const OUString& rCandidate);
        OUString whiteSpaceHandlingPreserve(const OUString& rCandidate);

        // #125325# removes block comment of the general form '/* ... */', returns
        // an adapted string or the original if no comments included
        OUString removeBlockComments(const OUString& rCandidate);

} // end of namespace svgio::svgreader

#endif // INCLUDED_SVGIO_INC_SVGTOOLS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

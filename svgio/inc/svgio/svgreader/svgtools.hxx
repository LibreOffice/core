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

#ifndef INCLUDED_SVGIO_SVGREADER_SVGTOOLS_HXX
#define INCLUDED_SVGIO_SVGREADER_SVGTOOLS_HXX

#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <basegfx/color/bcolor.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <svgio/svgreader/svgpaint.hxx>
#include <vector>

//////////////////////////////////////////////////////////////////////////////

namespace svgio
{
    namespace svgreader
    {
#ifdef DBG_UTIL
        // error helper
        void myAssert(const OUString& rMessage);
#endif

// recommended value for this devise dependend unit, see CSS2 section 4.3.2 Lenghts
#define F_SVG_PIXEL_PER_INCH  90.0

        // common non-token strings
        struct commonStrings
        {
            static const OUString aStrUserSpaceOnUse;
            static const OUString aStrObjectBoundingBox;
            static const OUString aStrNonzero;
            static const OUString aStrEvenOdd;
        };

        enum SvgUnits
        {
            userSpaceOnUse,
            objectBoundingBox
        };

        enum NumberType
        {
            xcoordinate,
            ycoordinate,
            length
        };

        class InfoProvider
        {
        public:
            virtual ~InfoProvider() {}
            virtual const basegfx::B2DRange getCurrentViewPort() const = 0;
            /// return font size of node inherited from parents
            virtual double getCurrentFontSizeInherited() const = 0;
            /// return xheight of node inherited from parents
            virtual double getCurrentXHeightInherited() const = 0;
        };

        enum SvgUnit
        {
            Unit_em = 0,    // relative to current font size
            Unit_ex,        // relative to current x-height

            Unit_px,        // 'user unit'
            Unit_pt,        // points, 1.25 px
            Unit_pc,        // 15.0 px
            Unit_cm,        // 35.43307 px
            Unit_mm,        // 3.543307 px
            Unit_in,        // 90 px

            Unit_percent    // relative to range
        };

        class SvgNumber
        {
        private:
            double      mfNumber;
            SvgUnit     meUnit;

            /// bitfield
            bool        mbSet : 1;

        public:
            SvgNumber()
            :   mfNumber(0.0),
                meUnit(Unit_px),
                mbSet(false)
            {
            }

            SvgNumber(double fNum, SvgUnit aSvgUnit = Unit_px, bool bSet = true)
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

            // Only usable in cases, when the unit is not Unit_percent, otherwise use method solve
            double solveNonPercentage(const InfoProvider& rInfoProvider) const;

            double solve(const InfoProvider& rInfoProvider, NumberType aNumberType = length) const;


        };

        typedef ::std::vector< SvgNumber > SvgNumberVector;

        enum SvgAlign
        {
            Align_none,
            Align_xMinYMin,
            Align_xMidYMin,
            Align_xMaxYMin,
            Align_xMinYMid,
            Align_xMidYMid, // default
            Align_xMaxYMid,
            Align_xMinYMax,
            Align_xMidYMax,
            Align_xMaxYMax
        };

        class SvgAspectRatio
        {
        private:
            SvgAlign    maSvgAlign;

            /// bitfield
            bool        mbDefer : 1; // default is false
            bool        mbMeetOrSlice : 1; // true = meet (default), false = slice
            bool        mbSet : 1;

        public:
            SvgAspectRatio()
            :   maSvgAlign(Align_xMidYMid),
                mbDefer(false),
                mbMeetOrSlice(true),
                mbSet(false)
            {
            }

            SvgAspectRatio(SvgAlign aSvgAlign, bool bDefer, bool bMeetOrSlice)
            :   maSvgAlign(aSvgAlign),
                mbDefer(bDefer),
                mbMeetOrSlice(bMeetOrSlice),
                mbSet(true)
            {
            }

            /// data read access
            SvgAlign getSvgAlign() const { return maSvgAlign; }
            bool isDefer() const { return mbDefer; }
            bool isMeetOrSlice() const { return mbMeetOrSlice; }
            bool isSet() const { return mbSet; }

            /// tooling
            static basegfx::B2DHomMatrix createLinearMapping(const basegfx::B2DRange& rTarget, const basegfx::B2DRange& rSource);
            basegfx::B2DHomMatrix createMapping(const basegfx::B2DRange& rTarget, const basegfx::B2DRange& rSource) const;
        };

        void skip_char(const OUString& rCandidate, const sal_Unicode& rChar, sal_Int32& nPos, const sal_Int32 nLen);
        void skip_char(const OUString& rCandidate, const sal_Unicode& rCharA, const sal_Unicode& rCharB, sal_Int32& nPos, const sal_Int32 nLen);
        void copySign(const OUString& rCandidate, sal_Int32& nPos, OUStringBuffer& rTarget, const sal_Int32 nLen);
        void copyNumber(const OUString& rCandidate, sal_Int32& nPos, OUStringBuffer& rTarget, const sal_Int32 nLen);
        void copyHex(const OUString& rCandidate, sal_Int32& nPos, OUStringBuffer& rTarget, const sal_Int32 nLen);
        void copyString(const OUString& rCandidate, sal_Int32& nPos, OUStringBuffer& rTarget, const sal_Int32 nLen);
        void copyToLimiter(const OUString& rCandidate, const sal_Unicode& rLimiter, sal_Int32& nPos, OUStringBuffer& rTarget, const sal_Int32 nLen);
        bool readNumber(const OUString& rCandidate, sal_Int32& nPos, double& fNum, const sal_Int32 nLen);
        SvgUnit readUnit(const OUString& rCandidate, sal_Int32& nPos, const sal_Int32 nLen);
        bool readNumberAndUnit(const OUString& rCandidate, sal_Int32& nPos, SvgNumber& aNum, const sal_Int32 nLen);
        bool readAngle(const OUString& rCandidate, sal_Int32& nPos, double& fAngle, const sal_Int32 nLen);
        sal_Int32 read_hex(const sal_Unicode& rChar);
        bool match_colorKeyword(basegfx::BColor& rColor, const OUString& rName);
        bool read_color(const OUString& rCandidate, basegfx::BColor& rColor);
        basegfx::B2DRange readViewBox(const OUString& rCandidate, InfoProvider& rInfoProvider);
        basegfx::B2DHomMatrix readTransform(const OUString& rCandidate, InfoProvider& rInfoProvider);
        bool readSingleNumber(const OUString& rCandidate, SvgNumber& aNum);
        bool readLocalUrl(const OUString& rCandidate, OUString& rURL);
        bool readSvgPaint(const OUString& rCandidate, SvgPaint& rSvgPaint, OUString& rURL);

        bool readSvgNumberVector(const OUString& rCandidate, SvgNumberVector& rSvgNumberVector);
        ::std::vector< double > solveSvgNumberVector(const SvgNumberVector& rInput, const InfoProvider& rInfoProvider, NumberType aNumberType = length);

        SvgAspectRatio readSvgAspectRatio(const OUString& rCandidate);

        typedef ::std::vector< OUString > SvgStringVector;
        bool readSvgStringVector(const OUString& rCandidate, SvgStringVector& rSvgStringVector);

        void readImageLink(const OUString& rCandidate, OUString& rXLink, OUString& rUrl, OUString& rMimeType, OUString& rData);

        OUString convert(const OUString& rCandidate, const sal_Unicode& rPattern, const sal_Unicode& rNew, bool bRemove);
        OUString consolidateContiguosSpace(const OUString& rCandidate);
        OUString whiteSpaceHandlingDefault(const OUString& rCandidate);
        OUString whiteSpaceHandlingPreserve(const OUString& rCandidate);

    } // end of namespace svgreader
} // end of namespace svgio

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_SVGIO_SVGREADER_SVGTOOLS_HXX

// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/

#ifndef INCLUDED_SVGIO_SVGREADER_SVGTOOLS_HXX
#define INCLUDED_SVGIO_SVGREADER_SVGTOOLS_HXX

#include <svgio/svgiodllapi.h>
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
        void myAssert(const rtl::OUString& rMessage);
#endif

// recommended value for this devise dependend unit, see CSS2 section 4.3.2 Lenghts
#define F_SVG_PIXEL_PER_INCH  90.0

        // common non-token strings
        struct commonStrings
        {
            static const rtl::OUString aStrUserSpaceOnUse;
            static const rtl::OUString aStrObjectBoundingBox;
            static const rtl::OUString aStrNonzero;
            static const rtl::OUString aStrEvenOdd;
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
            virtual const basegfx::B2DRange getCurrentViewPort() const = 0;
            virtual double getCurrentFontSize() const = 0;
            virtual double getCurrentXHeight() const = 0;
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

        void skip_char(const rtl::OUString& rCandidate, const sal_Unicode& rChar, sal_Int32& nPos, const sal_Int32 nLen);
        void skip_char(const rtl::OUString& rCandidate, const sal_Unicode& rCharA, const sal_Unicode& rCharB, sal_Int32& nPos, const sal_Int32 nLen);
        void copySign(const rtl::OUString& rCandidate, sal_Int32& nPos, rtl::OUStringBuffer& rTarget, const sal_Int32 nLen);
        void copyNumber(const rtl::OUString& rCandidate, sal_Int32& nPos, rtl::OUStringBuffer& rTarget, const sal_Int32 nLen);
        void copyHex(const rtl::OUString& rCandidate, sal_Int32& nPos, rtl::OUStringBuffer& rTarget, const sal_Int32 nLen);
        void copyString(const rtl::OUString& rCandidate, sal_Int32& nPos, rtl::OUStringBuffer& rTarget, const sal_Int32 nLen);
        void copyToLimiter(const rtl::OUString& rCandidate, const sal_Unicode& rLimiter, sal_Int32& nPos, rtl::OUStringBuffer& rTarget, const sal_Int32 nLen);
        bool readNumber(const rtl::OUString& rCandidate, sal_Int32& nPos, double& fNum, const sal_Int32 nLen);
        SvgUnit readUnit(const rtl::OUString& rCandidate, sal_Int32& nPos, const sal_Int32 nLen);
        bool readNumberAndUnit(const rtl::OUString& rCandidate, sal_Int32& nPos, SvgNumber& aNum, const sal_Int32 nLen);
        bool readAngle(const rtl::OUString& rCandidate, sal_Int32& nPos, double& fAngle, const sal_Int32 nLen);
        sal_Int32 read_hex(const sal_Unicode& rChar);
        bool match_colorKeyword(basegfx::BColor& rColor, const rtl::OUString& rName);
        bool read_color(const rtl::OUString& rCandidate, basegfx::BColor& rColor);
        basegfx::B2DRange readViewBox(const rtl::OUString& rCandidate, InfoProvider& rInfoProvider);
        basegfx::B2DHomMatrix readTransform(const rtl::OUString& rCandidate, InfoProvider& rInfoProvider);
        bool readSingleNumber(const rtl::OUString& rCandidate, SvgNumber& aNum);
        bool readLocalUrl(const rtl::OUString& rCandidate, rtl::OUString& rURL);
        bool readSvgPaint(const rtl::OUString& rCandidate, SvgPaint& rSvgPaint, rtl::OUString& rURL);

        bool readSvgNumberVector(const rtl::OUString& rCandidate, SvgNumberVector& rSvgNumberVector);
        ::std::vector< double > solveSvgNumberVector(const SvgNumberVector& rInput, const InfoProvider& rInfoProvider, NumberType aNumberType = length);

        SvgAspectRatio readSvgAspectRatio(const rtl::OUString& rCandidate);

        typedef ::std::vector< rtl::OUString > SvgStringVector;
        bool readSvgStringVector(const rtl::OUString& rCandidate, SvgStringVector& rSvgStringVector);

        void readImageLink(const rtl::OUString& rCandidate, rtl::OUString& rXLink, rtl::OUString& rUrl, rtl::OUString& rMimeType, rtl::OUString& rData);

        rtl::OUString convert(const rtl::OUString& rCandidate, const sal_Unicode& rPattern, const sal_Unicode& rNew, bool bRemove);
        rtl::OUString consolidateContiguosSpace(const rtl::OUString& rCandidate);
        rtl::OUString whiteSpaceHandlingDefault(const rtl::OUString& rCandidate);
        rtl::OUString whiteSpaceHandlingPreserve(const rtl::OUString& rCandidate);

    } // end of namespace svgreader
} // end of namespace svgio

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_SVGIO_SVGREADER_SVGTOOLS_HXX

// eof

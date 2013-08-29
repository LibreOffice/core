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

#ifndef SVX_BORDERLINE_HXX
#define SVX_BORDERLINE_HXX

#include <com/sun/star/table/BorderLineStyle.hpp>

#include <tools/color.hxx>
#include <svl/poolitem.hxx>
#include <editeng/editengdllapi.h>
#include <svtools/ctrlbox.hxx>

// Line defaults in twips (former Writer defaults):

#define DEF_LINE_WIDTH_0        1
#define DEF_LINE_WIDTH_1        20
#define DEF_LINE_WIDTH_2        50
#define DEF_LINE_WIDTH_3        80
#define DEF_LINE_WIDTH_4        100
#define DEF_LINE_WIDTH_5        10

// ============================================================================

namespace editeng {

    // values from ::com::sun::star::table::BorderLineStyle
    typedef sal_Int16 SvxBorderStyle;

    // convert border style between Word formats and LO
    SvxBorderStyle EDITENG_DLLPUBLIC ConvertBorderStyleFromWord(int);
    /// convert border width in twips between Word formats and LO
    double EDITENG_DLLPUBLIC ConvertBorderWidthToWord(SvxBorderStyle, double);
    double EDITENG_DLLPUBLIC ConvertBorderWidthFromWord(SvxBorderStyle,
            double, int);

    class EDITENG_DLLPUBLIC SvxBorderLine
    {
    protected:
        Color  aColor;

        long m_nWidth;
        bool m_bMirrorWidths;
        BorderWidthImpl m_aWidthImpl;
        long m_nMult;
        long m_nDiv;

        SvxBorderStyle   m_nStyle;
        sal_uInt16 nOutWidth;
        sal_uInt16 nInWidth;
        sal_uInt16 nDistance;

        bool             m_bUseLeftTop;
        Color            (*m_pColorOutFn)( Color );
        Color            (*m_pColorInFn)( Color );
        Color            (*m_pColorGapFn)( Color );

    public:
        SvxBorderLine( const Color *pCol = 0,
                long nWidth = 0, SvxBorderStyle nStyle =
                    ::com::sun::star::table::BorderLineStyle::SOLID,
                bool bUseLeftTop = false,
                Color (*pColorOutFn)( Color ) = &darkColor,
                Color (*pColorInFn)( Color ) = &darkColor,
                Color (*pColorGapFn)( Color ) = NULL );
        SvxBorderLine( const SvxBorderLine& r );

        SvxBorderLine& operator=( const SvxBorderLine& r );

        const Color&    GetColor() const { return aColor; }
        Color           GetColorOut( bool bLeftOrTop = true ) const;
        Color           GetColorIn( bool bLeftOrTop = true ) const;
        bool            HasGapColor() const { return m_pColorGapFn != NULL; }
        Color           GetColorGap() const;

        void            SetWidth( long nWidth = 0 ) { m_nWidth = nWidth; }
        /** Guess the style and width from the three lines widths values.

            When the value of nStyle is SvxBorderLine::DOUBLE, the style set will be guessed
            using the three values to match the best possible style among the following:
                - SvxBorderLine::DOUBLE
                - SvxBorderLine::THINTHICK_SMALLGAP
                - SvxBorderLine::THINTHICK_MEDIUMGAP
                - SvxBorderLine::THINTHICK_LARGEGAP
                - SvxBorderLine::THICKTHIN_SMALLGAP
                - SvxBorderLine::THICKTHIN_MEDIUMGAP
                - SvxBorderLine::THICKTHIN_LARGEGAP

            If no styles matches the width, then the width is set to 0.

            There is one known case that could fit several styles: \a nIn = \a nDist = 0.75 pt,
            \a nOut = 1.5 pt. This case fits SvxBorderLine::THINTHICK_SMALLGAP and
            SvxBorderLine::THINTHICK_MEDIUMGAP with a 1.5 pt width and
            SvxBorderLine::THINTHICK_LARGEGAP with a 0.75 pt width. The same case happens
            also for thick-thin styles.

            \param nStyle the border style used to guess the width.
            \param nIn the width of the inner line in 1th pt
            \param nOut the width of the outer line in 1th pt
            \param nDist the width of the gap between the lines in 1th pt
         */
        void            GuessLinesWidths( SvxBorderStyle nStyle, sal_uInt16 nOut, sal_uInt16 nIn = 0, sal_uInt16 nDist = 0 );

        // TODO Hacky method to mirror lines in only a few cases
        void            SetMirrorWidths( bool bMirror = true ) { m_bMirrorWidths = bMirror; }
        long            GetWidth( ) const { return m_nWidth; }
        sal_uInt16      GetOutWidth() const;
        sal_uInt16      GetInWidth() const;
        sal_uInt16      GetDistance() const;

        SvxBorderStyle  GetBorderLineStyle() const { return m_nStyle; }

        void            SetColor( const Color &rColor ) { aColor = rColor; }
        void            SetColorOutFn( Color (*pColorOutFn)( Color ) ) { m_pColorOutFn = pColorOutFn; }
        void            SetColorInFn( Color (*pColorInFn)( Color ) ) { m_pColorInFn = pColorInFn; }
        void            SetColorGapFn( Color (*pColorGapFn)( Color ) ) { m_pColorGapFn = pColorGapFn; }
        void            SetUseLeftTop( bool bUseLeftTop ) { m_bUseLeftTop = bUseLeftTop; }
        void            SetBorderLineStyle( SvxBorderStyle nNew );
        void            ScaleMetrics( long nMult, long nDiv );

        sal_Bool            operator==( const SvxBorderLine &rCmp ) const;

        OUString        GetValueString( SfxMapUnit eSrcUnit, SfxMapUnit eDestUnit,
                                        const IntlWrapper* pIntl,
                                        sal_Bool bMetricStr = sal_False ) const;

        bool            HasPriority( const SvxBorderLine& rOtherLine ) const;

        bool isEmpty() const {
            return m_aWidthImpl.IsEmpty()
                || m_nStyle == ::com::sun::star::table::BorderLineStyle::NONE
                || m_nWidth == 0;
        }
        bool isDouble() const { return m_aWidthImpl.IsDouble(); }
        sal_uInt16 GetScaledWidth() const { return GetOutWidth() + GetInWidth() + GetDistance(); }

        static Color darkColor( Color aMain );
        static Color lightColor( Color aMain );

        static Color threeDLightColor( Color aMain );
        static Color threeDMediumColor( Color aMain );
        static Color threeDDarkColor( Color aMain );

        static BorderWidthImpl getWidthImpl( SvxBorderStyle nStyle );
    };

EDITENG_DLLPUBLIC bool operator!=( const SvxBorderLine& rLeft, const SvxBorderLine& rRight );

} // namespace editeng

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

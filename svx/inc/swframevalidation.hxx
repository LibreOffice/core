/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#ifndef _SVXSWFRAMEVALIDATION_HXX
#define _SVXSWFRAMEVALIDATION_HXX
#include <sal/types.h>
#include <tools/gen.hxx>
#include <limits.h>

/* -----------------03.03.2004 16:31-----------------
    struct to determine min/max values for fly frame positioning in Writer
 --------------------------------------------------*/
struct SvxSwFrameValidation
{
    sal_Int16           nAnchorType; //com::sun::star::text::TextContentAnchorType
    sal_Int16           nHoriOrient; //com::sun::star::text::HoriOrientation
    sal_Int16           nVertOrient; //com::sun::star::text::VertOrientation
    sal_Int16           nHRelOrient; //com::sun::star::text::RelOrientation
    sal_Int16           nVRelOrient; //com::sun::star::text::RelOrientation
    bool                bAutoHeight;
    bool                bAutoWidth;
    bool                bMirror;
    bool                bFollowTextFlow;

    sal_Int32 nHPos;
    sal_Int32 nMaxHPos;
    sal_Int32 nMinHPos;

    sal_Int32 nVPos;
    sal_Int32 nMaxVPos;
    sal_Int32 nMinVPos;

    sal_Int32 nWidth;
    sal_Int32 nMinWidth;
    sal_Int32 nMaxWidth;

    sal_Int32 nHeight;
    sal_Int32 nMinHeight;
    sal_Int32 nMaxHeight;

    Size    aPercentSize;   // Size fuer 100%-Wert

    SvxSwFrameValidation() :
        bAutoHeight(false),
        bAutoWidth(false),
        bMirror(false),
        bFollowTextFlow( false ),
        nHPos(0),
        nMaxHPos(SAL_MAX_INT32),
        nMinHPos(0),
        nVPos(0),
        nMaxVPos(SAL_MAX_INT32),
        nMinVPos(0),
        nWidth( 283 * 4 ), //2.0 cm
        nMinWidth(0),
        nMaxWidth(SAL_MAX_INT32),
        nHeight( 283 ), //0.5 cm
        nMaxHeight(SAL_MAX_INT32)
    {
    }
};

#endif


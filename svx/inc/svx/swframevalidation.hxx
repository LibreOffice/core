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


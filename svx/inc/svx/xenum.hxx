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



#ifndef _XENUM_HXX
#define _XENUM_HXX

enum XLineStyle         { XLINE_NONE, XLINE_SOLID, XLINE_DASH };
enum XLineJoint
{
    XLINEJOINT_NONE,        // no rounding
    XLINEJOINT_MIDDLE,      // calc middle value between joints
    XLINEJOINT_BEVEL,       // join edges with line
    XLINEJOINT_MITER,       // extend till cut
    XLINEJOINT_ROUND        // create arc
};
enum XDashStyle         { XDASH_RECT, XDASH_ROUND, XDASH_RECTRELATIVE,
                          XDASH_ROUNDRELATIVE };
enum XFillStyle         { XFILL_NONE, XFILL_SOLID, XFILL_GRADIENT, XFILL_HATCH,
                          XFILL_BITMAP };
enum XGradientStyle     { XGRAD_LINEAR, XGRAD_AXIAL, XGRAD_RADIAL,
                          XGRAD_ELLIPTICAL, XGRAD_SQUARE, XGRAD_RECT };
enum XHatchStyle        { XHATCH_SINGLE, XHATCH_DOUBLE, XHATCH_TRIPLE };
enum XFormTextStyle     { XFT_ROTATE, XFT_UPRIGHT, XFT_SLANTX, XFT_SLANTY,
                          XFT_NONE };
enum XFormTextAdjust    { XFT_LEFT, XFT_RIGHT, XFT_AUTOSIZE, XFT_CENTER };
enum XFormTextShadow    { XFTSHADOW_NONE, XFTSHADOW_NORMAL, XFTSHADOW_SLANT};
enum XFormTextStdForm   { XFTFORM_NONE = 0, XFTFORM_TOPCIRC, XFTFORM_BOTCIRC,
                          XFTFORM_LFTCIRC, XFTFORM_RGTCIRC, XFTFORM_TOPARC,
                          XFTFORM_BOTARC, XFTFORM_LFTARC, XFTFORM_RGTARC,
                          XFTFORM_BUTTON1, XFTFORM_BUTTON2,
                          XFTFORM_BUTTON3, XFTFORM_BUTTON4};
enum XBitmapStyle       { XBITMAP_TILE, XBITMAP_STRETCH };
enum XBitmapType        { XBITMAP_IMPORT, XBITMAP_8X8, XBITMAP_NONE,
                          XBITMAP_16X16 };


#endif      // _XENUM_HXX


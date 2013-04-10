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

#endif      // _XENUM_HXX


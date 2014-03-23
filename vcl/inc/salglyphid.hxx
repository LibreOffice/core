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

#ifndef _SV_SALGLYPHID_HXX
#define _SV_SALGLYPHID_HXX

// TODO: sal_GlyphId should become a class...
typedef sal_uInt32 sal_GlyphId;

// Glyph Flags
#define GF_NONE     0x00000000
#define GF_FLAGMASK 0xFF800000
#define GF_IDXMASK  ~GF_FLAGMASK
#define GF_ISCHAR   0x00800000
#define GF_ROTL     0x01000000
// caution !!!
#define GF_VERT     0x02000000
// GF_VERT is only for windows implementation
// (win/source/gdi/salgdi3.cxx, win/source/gdi/winlayout.cxx)
// don't use this elsewhere !!!
#define GF_ROTR     0x03000000
#define GF_ROTMASK  0x03000000
#define GF_UNHINTED 0x04000000
#define GF_GSUB     0x08000000
#define GF_FONTMASK 0xF0000000
#define GF_FONTSHIFT 28

#define GF_DROPPED  0xFFFFFFFF

#endif // _SV_SALGLYPHID_HXX


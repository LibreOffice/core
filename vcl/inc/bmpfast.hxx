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



#ifndef _SV_BMPFAST_HXX
#define _SV_BMPFAST_HXX

class BitmapWriteAccess;
class BitmapReadAccess;
struct BitmapBuffer;
class BitmapColor;
class Size;
class Point;
struct SalTwoRect;

// the bmpfast functions have signatures with good compatibility to
// their canonic counterparts, which employ the GetPixel/SetPixel methods

bool ImplFastBitmapConversion( BitmapBuffer& rDst, const BitmapBuffer& rSrc,
        const SalTwoRect& rTwoRect );

bool ImplFastBitmapBlending( BitmapWriteAccess& rDst,
    const BitmapReadAccess& rSrc, const BitmapReadAccess& rMask,
    const SalTwoRect& rTwoRect );

bool ImplFastEraseBitmap( BitmapBuffer&, const BitmapColor& );

#endif // _SV_BMPFAST_HXX

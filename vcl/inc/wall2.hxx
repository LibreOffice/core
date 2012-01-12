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



#ifndef _SV_WALL2_HXX
#define _SV_WALL2_HXX

#include <vcl/wall.hxx>

class ImplWallpaper
{
    friend class Wallpaper;

private:
    Color           maColor;
    BitmapEx*       mpBitmap;
    Gradient*       mpGradient;
    Rectangle*      mpRect;
    WallpaperStyle  meStyle;
    sal_uLong           mnRefCount;
    BitmapEx*       mpCache;

    friend SvStream& operator>>( SvStream& rIStm, ImplWallpaper& rImplWallpaper );
    friend SvStream& operator<<( SvStream& rOStm, const ImplWallpaper& rImplWallpaper );

public:
                    ImplWallpaper();
                    ImplWallpaper( const ImplWallpaper& rImplWallpaper );
                    ~ImplWallpaper();

    void            ImplSetCachedBitmap( BitmapEx& rBmp );
    const BitmapEx* ImplGetCachedBitmap() { return mpCache; }
    void            ImplReleaseCachedBitmap();
};


#endif  // _SV_WALL2_HXX

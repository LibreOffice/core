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



#ifndef _SV_IMPANMVW_HXX
#define _SV_IMPANMVW_HXX

#include <vcl/animate.hxx>

// ----------------
// - ImplAnimView -
// ----------------

class Animation;
class OutputDevice;
class VirtualDevice;
struct AnimationBitmap;

class ImplAnimView
{
private:

    Animation*      mpParent;
    OutputDevice*   mpOut;
    long            mnExtraData;
    Point           maPt;
    Point           maDispPt;
    Point           maRestPt;
    Size            maSz;
    Size            maSzPix;
    Size            maDispSz;
    Size            maRestSz;
    MapMode         maMap;
    Region          maClip;
    VirtualDevice*  mpBackground;
    VirtualDevice*  mpRestore;
    sal_uLong           mnActPos;
    Disposal        meLastDisposal;
    sal_Bool            mbPause;
    sal_Bool            mbFirst;
    sal_Bool            mbMarked;
    sal_Bool            mbHMirr;
    sal_Bool            mbVMirr;

    void            ImplGetPosSize( const AnimationBitmap& rAnm, Point& rPosPix, Size& rSizePix );
    void            ImplDraw( sal_uLong nPos, VirtualDevice* pVDev );

public:

                    ImplAnimView( Animation* pParent, OutputDevice* pOut,
                                  const Point& rPt, const Size& rSz, sal_uLong nExtraData,
                                  OutputDevice* pFirstFrameOutDev = NULL );
                    ~ImplAnimView();

    sal_Bool            ImplMatches( OutputDevice* pOut, long nExtraData ) const;
    void            ImplDrawToPos( sal_uLong nPos );
    void            ImplDraw( sal_uLong nPos );
    void            ImplRepaint();
    AInfo*          ImplCreateAInfo() const;

    const Point&    ImplGetOutPos() const { return maPt; }

    const Size&     ImplGetOutSize() const { return maSz; }
    const Size&     ImplGetOutSizePix() const { return maSzPix; }

    void            ImplPause( sal_Bool bPause ) { mbPause = bPause; }
    sal_Bool            ImplIsPause() const { return mbPause; }

    void            ImplSetMarked( sal_Bool bMarked ) { mbMarked = bMarked; }
    sal_Bool            ImplIsMarked() const { return mbMarked; }
};

#endif

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



#include <vcl/floatwin.hxx>
#include <vcl/bitmap.hxx>
#include <vcl/image.hxx>

// -----------
// - Defines -
// -----------

#define WHEELMODE_NONE      0x00000000UL
#define WHEELMODE_VH        0x00000001UL
#define WHEELMODE_V         0x00000002UL
#define WHEELMODE_H         0x00000004UL
#define WHEELMODE_SCROLL_VH 0x00000008UL
#define WHEELMODE_SCROLL_V  0x00000010UL
#define WHEELMODE_SCROLL_H  0x00000020UL

// -------------------
// - ImplWheelWindow -
// -------------------

class Timer;

class ImplWheelWindow : public FloatingWindow
{
private:

    ImageList           maImgList;
    Bitmap              maWheelBmp;
    CommandScrollData   maCommandScrollData;
    Point               maLastMousePos;
    Point               maCenter;
    Timer*              mpTimer;
    sal_uLong               mnRepaintTime;
    sal_uLong               mnTimeout;
    sal_uLong               mnWheelMode;
    sal_uLong               mnMaxWidth;
    sal_uLong               mnActWidth;
    sal_uLong               mnActDist;
    long                mnActDeltaX;
    long                mnActDeltaY;

    void                ImplCreateImageList();
    void                ImplSetRegion( const Bitmap& rRegionBmp );
    using Window::ImplGetMousePointer;
    PointerStyle        ImplGetMousePointer( long nDistX, long nDistY );
    void                ImplDrawWheel();
    void                ImplRecalcScrollValues();

                        DECL_LINK( ImplScrollHdl, Timer* pTimer );

protected:

    virtual void        Paint( const Rectangle& rRect );
    virtual void        MouseMove( const MouseEvent& rMEvt );
    virtual void        MouseButtonUp( const MouseEvent& rMEvt );

public:

                        ImplWheelWindow( Window* pParent );
                        ~ImplWheelWindow();

    void                ImplStop();
    void                ImplSetWheelMode( sal_uLong nWheelMode );
    sal_uLong               ImplGetWheelMode() const { return mnWheelMode; }
};

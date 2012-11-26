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



#ifndef SD_FU_ZOOM_HXX
#define SD_FU_ZOOM_HXX

#ifndef _SV_POINTR_HXX //autogen
#include <vcl/pointr.hxx>
#endif
#include "fupoor.hxx"
#include <basegfx/range/b2drange.hxx>

namespace sd {

extern sal_uInt16 SidArrayZoom[];

class FuZoom
    : public FuPoor
{
public:
    static FunctionReference Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq );

    // Mouse- & Key-Events
    virtual bool MouseMove(const MouseEvent& rMEvt);
    virtual bool MouseButtonUp(const MouseEvent& rMEvt);
    virtual bool MouseButtonDown(const MouseEvent& rMEvt);

    virtual void Activate();        // Function aktivieren
    virtual void Deactivate();      // Function deaktivieren

protected:
    virtual ~FuZoom (void);

    basegfx::B2DPoint   maBeginPosPixel;
    basegfx::B2DPoint   maBeginPos;
    basegfx::B2DPoint   maEndPos;
    basegfx::B2DRange   maZoomRange;
    Pointer             maPtr;

    /// bitfield
    bool                mbVisible : 1;
    bool                mbStartDrag : 1;

private:
    FuZoom (
        ViewShell* pViewSh,
        ::sd::Window* pWin,
        ::sd::View* pView,
        SdDrawDocument* pDoc,
        SfxRequest& rReq);
};

} // end of namespace sd

#endif


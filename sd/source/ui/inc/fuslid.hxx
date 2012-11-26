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



#ifndef SD_FU_SLIDE_HXX
#define SD_FU_SLIDE_HXX

#include "fupoor.hxx"

class SdDrawDocument;

namespace sd {

class SlideView;
class SlideViewShell;
class Window;


/*************************************************************************
|*
|* Basisklasse der Funktionen des Diamodus
|*
\************************************************************************/

class FuSlide
    : public FuPoor
{
public:
    static FunctionReference Create( SlideViewShell* pViewSh, ::sd::Window* pWin, SlideView* pView, SdDrawDocument* pDoc, SfxRequest& rReq );

    virtual bool MouseMove(const MouseEvent& rMEvt);
    virtual bool MouseButtonUp(const MouseEvent& rMEvt);
    virtual bool MouseButtonDown(const MouseEvent& rMEvt);

    virtual void ScrollStart();
    virtual void ScrollEnd();

protected:
    FuSlide (
        SlideViewShell* pViewSh,
        ::sd::Window* pWin,
        SlideView* pView,
        SdDrawDocument* pDoc,
        SfxRequest& rReq);

    SlideViewShell* pSlViewShell;
    SlideView*    pSlView;
};

} // end of namespace sd

#endif

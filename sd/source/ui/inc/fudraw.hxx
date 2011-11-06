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



#ifndef SD_FU_DRAW_HXX
#define SD_FU_DRAW_HXX

#ifndef _SV_POINTR_HXX //autogen
#include <vcl/pointr.hxx>
#endif
#include "fupoor.hxx"

struct SdrViewEvent;
class SdrObject;

namespace sd {

/*************************************************************************
|*
|* Basisklasse fuer alle Drawmodul-spezifischen Funktionen
|*
\************************************************************************/

class FuDraw
    : public FuPoor
{
public:
    TYPEINFO();

    virtual sal_Bool KeyInput(const KeyEvent& rKEvt);
    virtual sal_Bool MouseMove(const MouseEvent& rMEvt);
    virtual sal_Bool MouseButtonUp(const MouseEvent& rMEvt);
    virtual sal_Bool MouseButtonDown(const MouseEvent& rMEvt);
    virtual sal_Bool RequestHelp(const HelpEvent& rHEvt);

    virtual void ScrollStart();
    virtual void ScrollEnd();

    virtual void Activate();
    virtual void Deactivate();

    virtual void ForcePointer(const MouseEvent* pMEvt = NULL);

    virtual void DoubleClick(const MouseEvent& rMEvt);

    sal_Bool    SetPointer(SdrObject* pObj, const Point& rPos);
    sal_Bool    SetHelpText(SdrObject* pObj, const Point& rPos, const SdrViewEvent& rVEvt);

    void    SetPermanent(sal_Bool bSet) { bPermanent = bSet; }

    /** is called when the currenct function should be aborted. <p>
        This is used when a function gets a KEY_ESCAPE but can also
        be called directly.

        @returns true if a active function was aborted
    */
    virtual bool cancel();

protected:
    FuDraw (ViewShell* pViewSh,
        ::sd::Window* pWin,
        ::sd::View* pView,
        SdDrawDocument* pDoc,
        SfxRequest& rReq);

    virtual ~FuDraw();

    Pointer aNewPointer;
    Pointer aOldPointer;
    sal_Bool    bMBDown;
    sal_Bool    bDragHelpLine;
    sal_uInt16  nHelpLine;
    sal_Bool    bPermanent;

};

} // end of namespace sd

#endif      // _SD_FUDRAW_HXX

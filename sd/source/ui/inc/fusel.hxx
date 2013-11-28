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



#ifndef SD_FU_SELECTION_HXX
#define SD_FU_SELECTION_HXX

#include "fudraw.hxx"

#include <com/sun/star/media/XPlayer.hpp>

class SdrHdl;
class SdrObject;
class Sound;


namespace sd {

class FuSelection
    : public FuDraw
{
public:
    TYPEINFO();

    static FunctionReference Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq );
    virtual void DoExecute( SfxRequest& rReq );

                                       // Mouse- & Key-Events
    virtual sal_Bool KeyInput(const KeyEvent& rKEvt);
    virtual sal_Bool MouseMove(const MouseEvent& rMEvt);
    virtual sal_Bool MouseButtonUp(const MouseEvent& rMEvt);
    virtual sal_Bool MouseButtonDown(const MouseEvent& rMEvt);

    virtual void Activate();           // Function aktivieren
    virtual void Deactivate();         // Function deaktivieren

    virtual void SelectionHasChanged();

    void    SetEditMode(sal_uInt16 nMode);
    sal_uInt16  GetEditMode() { return nEditMode; }

    sal_Bool    AnimateObj(SdrObject* pObj, const Point& rPos);

    /** is called when the currenct function should be aborted. <p>
        This is used when a function gets a KEY_ESCAPE but can also
        be called directly.

        @returns true if a active function was aborted
    */
    virtual bool cancel();

    //Solution: let mouse cursor move
    virtual void ForcePointer(const MouseEvent* pMEvt = NULL);
protected:
    FuSelection (ViewShell* pViewSh,
        ::sd::Window* pWin,
        ::sd::View* pView,
        SdDrawDocument* pDoc,
        SfxRequest& rReq);

    virtual ~FuSelection();

    sal_Bool            bTempRotation;
    sal_Bool            bSelectionChanged;
    sal_Bool            bHideAndAnimate;
    SdrHdl*         pHdl;
    sal_Bool            bSuppressChangesOfSelection;
    sal_Bool            bMirrorSide0;
    sal_uInt16          nEditMode;
        ::com::sun::star::uno::Reference< ::com::sun::star::media::XPlayer > mxPlayer;

private:
    /** This pointer stores a canidate for assigning a style in the water
        can mode between mouse button down and mouse button up.
    */
    SdrObject* pWaterCanCandidate;

    /** Find the object under the given test point without selecting it.
        @param rTestPoint
            The coordinates at which to search for a shape.
        @return
            The shape at the test point.  When there is no shape at this
            position then NULL is returned.
    */
    SdrObject* pickObject (const Point& rTestPoint);
  //Solution: Add Shift+UP/DOWN/LEFT/RIGHT key to move the position of insert point,
  //and SHIFT+ENTER key to decide the postion and draw the new insert point
    sal_Bool   bBeginInsertPoint;
    Point    oldPoint;
  //Solution: let mouse cursor move
  sal_Bool   bMovedToCenterPoint;
};

} // end of namespace sd

#endif      // _SD_FUSEL_HXX


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
    static FunctionReference Create( ViewShell* pViewSh, ::sd::Window* pWin, ::sd::View* pView, SdDrawDocument* pDoc, SfxRequest& rReq );
    virtual void DoExecute( SfxRequest& rReq );

                                       // Mouse- & Key-Events
    virtual bool KeyInput(const KeyEvent& rKEvt);
    virtual bool MouseMove(const MouseEvent& rMEvt);
    virtual bool MouseButtonUp(const MouseEvent& rMEvt);
    virtual bool MouseButtonDown(const MouseEvent& rMEvt);

    virtual void Activate();           // Function aktivieren
    virtual void Deactivate();         // Function deaktivieren

    virtual void SelectionHasChanged();

    void    SetEditMode(sal_uInt16 nMode);
    sal_uInt16  GetEditMode() { return nEditMode; }

    bool AnimateObj(SdrObject* pObj, const basegfx::B2DPoint& rPos);

    /** is called when the currenct function should be aborted. <p>
        This is used when a function gets a KEY_ESCAPE but can also
        be called directly.

        @returns true if a active function was aborted
    */
    virtual bool cancel();

//IAccessibility2 Implementation 2009-----
    //Solution: let mouse cursor move
    virtual void ForcePointer(const MouseEvent* pMEvt = NULL);
//-----IAccessibility2 Implementation 2009
protected:
    FuSelection (ViewShell* pViewSh,
        ::sd::Window* pWin,
        ::sd::View* pView,
        SdDrawDocument* pDoc,
        SfxRequest& rReq);

    virtual ~FuSelection();

    bool            bTempRotation;
    bool            bSelectionChanged;
    bool            bHideAndAnimate;
    SdrHdl*         pHdl;
    bool            bSuppressChangesOfSelection;
    bool            bMirrorSide0;
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
    SdrObject* pickObject (const basegfx::B2DPoint& rTestPoint);
//IAccessibility2 Implementation 2009-----
  //Solution: Add Shift+UP/DOWN/LEFT/RIGHT key to move the position of insert point,
  //and SHIFT+ENTER key to decide the postion and draw the new insert point
    Point           maOldPoint;

    /// bitfield
    bool            mbBeginInsertPoint : 1;
    bool            mbMovedToCenterPoint : 1;
//-----IAccessibility2 Implementation 2009
};

} // end of namespace sd

#endif      // _SD_FUSEL_HXX


/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fusel.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 05:39:10 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef SD_FU_SELECTION_HXX
#define SD_FU_SELECTION_HXX

#ifndef SD_FU_DRAW_HXX
#include "fudraw.hxx"
#endif

class SdrHdl;
class SdrObject;
class Sound;


namespace sd {

class FuSelection
    : public FuDraw
{
public:
    TYPEINFO();

    FuSelection (ViewShell* pViewSh,
        ::sd::Window* pWin,
        ::sd::View* pView,
        SdDrawDocument* pDoc,
        SfxRequest& rReq);

    virtual ~FuSelection();
                                       // Mouse- & Key-Events
    virtual BOOL KeyInput(const KeyEvent& rKEvt);
    virtual BOOL MouseMove(const MouseEvent& rMEvt);
    virtual BOOL MouseButtonUp(const MouseEvent& rMEvt);
    virtual BOOL MouseButtonDown(const MouseEvent& rMEvt);

    virtual void Activate();           // Function aktivieren
    virtual void Deactivate();         // Function deaktivieren

    virtual void SelectionHasChanged();

    void    SetEditMode(USHORT nMode);
    USHORT  GetEditMode() { return nEditMode; }

    BOOL    AnimateObj(SdrObject* pObj, const Point& rPos);

    /** is called when the currenct function should be aborted. <p>
        This is used when a function gets a KEY_ESCAPE but can also
        be called directly.

        @returns true if a active function was aborted
    */
    virtual bool cancel();

protected:
    BOOL            bTempRotation;
    BOOL            bSelectionChanged;
    BOOL            bHideAndAnimate;
    SdrHdl*         pHdl;
    BOOL            bSuppressChangesOfSelection;
    BOOL            bMirrorSide0;
    USHORT          nEditMode;
    Sound*          pSound;

                    DECL_LINK( SoundHasStoppedHdl, void* );
                    DECL_STATIC_LINK( FuSelection, StaticSoundHasStoppedHdl, Sound* );

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
};

} // end of namespace sd

#endif      // _SD_FUSEL_HXX


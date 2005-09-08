/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: futext.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 21:30:10 $
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

#ifndef SC_FUTEXT_HXX
#define SC_FUTEXT_HXX

#ifndef SC_FUCONSTR_HXX
#include "fuconstr.hxx"
#endif

class SdrObject;
class SdrTextObj;
class SdrOutliner;

/*************************************************************************
|*
|* Basisklasse fuer Textfunktionen
|*
\************************************************************************/
class FuText : public FuConstruct
{
protected:
//  USHORT          nOldObjectBar;
    SdrTextObj*     pTextObj;

public:
    FuText(ScTabViewShell* pViewSh, Window* pWin, SdrView* pView,
                   SdrModel* pDoc, SfxRequest& rReq);

    virtual ~FuText();

    virtual BOOL KeyInput(const KeyEvent& rKEvt);
    virtual BOOL MouseMove(const MouseEvent& rMEvt);
    virtual BOOL MouseButtonUp(const MouseEvent& rMEvt);
    virtual BOOL MouseButtonDown(const MouseEvent& rMEvt);

    virtual void Activate();           // Function aktivieren
    virtual void Deactivate();         // Function deaktivieren

    virtual void ForcePointer(const MouseEvent* pMEvt);

    virtual void SelectionHasChanged();

    void    SetInEditMode( SdrObject* pObj = NULL, const Point* pMousePixel = NULL,
                            BOOL bCursorToEnd = FALSE, const KeyEvent* pInitialKey = NULL );
    void    StopEditMode(BOOL bTextDirection = FALSE);
    void    StopDragMode(SdrObject* pObject);

    // #98185# Create default drawing objects via keyboard
    virtual SdrObject* CreateDefaultObject(const sal_uInt16 nID, const Rectangle& rRectangle);

private:
    SdrOutliner* MakeOutliner();
};



#endif


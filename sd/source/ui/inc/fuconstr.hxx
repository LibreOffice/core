/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fuconstr.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 05:31:41 $
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

#ifndef SD_FU_CONSTRUCT_HXX
#define SD_FU_CONSTRUCT_HXX

#ifndef SD_FU_DRAW_HXX
#include "fudraw.hxx"
#endif

class KeyEvent;
class SdrObject;
class SfxItemSet;

namespace sd {

/*************************************************************************
|*
|* Rechteck zeichnen
|*
\************************************************************************/

class FuConstruct
    : public FuDraw
{
public:
    static const int MIN_FREEHAND_DISTANCE = 10;

    TYPEINFO();

    FuConstruct (ViewShell* pViewSh,
        ::sd::Window* pWin,
        ::sd::View* pView,
        SdDrawDocument* pDoc,
        SfxRequest& rReq);
    virtual ~FuConstruct (void);

    // Mouse- & Key-Events
    virtual BOOL KeyInput(const KeyEvent& rKEvt);
    virtual BOOL MouseMove(const MouseEvent& rMEvt);
    virtual BOOL MouseButtonUp(const MouseEvent& rMEvt);
    virtual BOOL MouseButtonDown(const MouseEvent& rMEvt);

    virtual void Activate();           // Function aktivieren
    virtual void Deactivate();         // Function deaktivieren

    virtual void SelectionHasChanged() { bSelectionChanged = TRUE; }

    // SJ: setting stylesheet, the use of a filled or unfilled style
    // is determined by the member nSlotId :
    void SetStyleSheet(SfxItemSet& rAttr, SdrObject* pObj);

    // SJ: setting stylesheet, the use of a filled or unfilled style
    // is determinded by the parameters bUseFillStyle and bUseNoFillStyle :
    void SetStyleSheet( SfxItemSet& rAttr, SdrObject* pObj,
            const sal_Bool bUseFillStyle, const sal_Bool bUseNoFillStyle );

protected:
    bool bSelectionChanged;
};

} // end of namespace sd

#endif      // _SD_FUCONSTR_HXX

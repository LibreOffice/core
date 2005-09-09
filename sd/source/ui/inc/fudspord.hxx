/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fudspord.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 05:32:55 $
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

#ifndef SD_FU_DISPLAY_ORDER_HXX
#define SD_FU_DISPLAY_ORDER_HXX


#ifndef SD_FU_POOR_HXX
#include "fupoor.hxx"
#endif

#ifndef _VCL_POINTR_HXX
#include <vcl/pointr.hxx>
#endif

class SdrObject;
class SdrViewUserMarker;

namespace sd {

/*************************************************************************
|*
|* Funktion DisplayOrder
|*
\************************************************************************/

class FuDisplayOrder
    : public FuPoor
{
public:
    TYPEINFO();

    FuDisplayOrder (
        ViewShell* pViewSh,
        ::sd::Window* pWin,
        ::sd::View* pView,
        SdDrawDocument* pDoc,
        SfxRequest& rReq);
    virtual ~FuDisplayOrder (void);

    // Mouse- & Key-Events
    virtual BOOL MouseMove(const MouseEvent& rMEvt);
    virtual BOOL MouseButtonUp(const MouseEvent& rMEvt);
    virtual BOOL MouseButtonDown(const MouseEvent& rMEvt);

    virtual void Activate();        // Function aktivieren
    virtual void Deactivate();      // Function deaktivieren

protected:
     Pointer            aPtr;
     SdrObject*         pRefObj;
     SdrViewUserMarker* pUserMarker;
};

} // end of namespace sd

#endif      // _SD_FUDSPORD_HXX


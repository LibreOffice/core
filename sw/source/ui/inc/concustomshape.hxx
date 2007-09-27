/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: concustomshape.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 11:55:51 $
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

#ifndef _SW_CONCUSTOMSHAPE_HXX
#define _SW_CONCUSTOMSHAPE_HXX

#include <rtl/ustring.hxx>
#include "drawbase.hxx"

class SdrObject;
class SfxRequest;

/*************************************************************************
|*
|* Rechteck zeichnen
|*
\************************************************************************/

class ConstCustomShape : public SwDrawBase
{

    rtl::OUString aCustomShape;

    void SetAttributes( SdrObject* pObj );

 public:

    ConstCustomShape( SwWrtShell* pSh, SwEditWin* pWin, SwView* pView, SfxRequest& rReq );

                                       // Mouse- & Key-Events
    virtual BOOL MouseButtonUp(const MouseEvent& rMEvt);
    virtual BOOL MouseButtonDown(const MouseEvent& rMEvt);

    virtual void Activate(const USHORT nSlotId);    // Function aktivieren

    rtl::OUString GetShapeType() const;
    static rtl::OUString GetShapeTypeFromRequest( SfxRequest& rReq );

    virtual void CreateDefaultObject();

    // #i33136#
    virtual bool doConstructOrthogonal() const;
};



#endif      // _SW_CONRECT_HXX


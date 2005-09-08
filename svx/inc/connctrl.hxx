/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: connctrl.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 17:22:47 $
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
#ifndef _SVX_CONNCTRL_HXX
#define _SVX_CONNCTRL_HXX

// include ---------------------------------------------------------------

#ifndef _CTRL_HXX //autogen
#include <vcl/ctrl.hxx>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

class SfxItemSet;

class XOutputDevice;
class SdrEdgeObj;
class SdrView;
class SdrObjList;
/*************************************************************************
|*
|* SvxXConnectionPreview
|*
\************************************************************************/
class SVX_DLLPUBLIC SvxXConnectionPreview : public Control
{
 friend class SvxConnectionPage;

private:
    const SfxItemSet&   rAttrs;
    XOutputDevice*  pExtOutDev;
    SdrEdgeObj*         pEdgeObj;
    SdrObjList*         pObjList;
    const SdrView*      pView;

    SVX_DLLPRIVATE void SetStyles();
public:
            SvxXConnectionPreview( Window* pParent, const ResId& rResId,
                                const SfxItemSet& rInAttrs );
            ~SvxXConnectionPreview();

    virtual void Paint( const Rectangle& rRect );
    virtual void MouseButtonDown( const MouseEvent& rMEvt );

    void         SetAttributes( const SfxItemSet& rInAttrs );
    USHORT       GetLineDeltaAnz();

    void         Construct();
    void         SetView( const SdrView* pSdrView ) { pView = pSdrView; }

    virtual void DataChanged( const DataChangedEvent& rDCEvt );
};


#endif // _SVX_CONNCTRL_HXX


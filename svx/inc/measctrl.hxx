/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: measctrl.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 18:03:47 $
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
#ifndef _SVX_MEASCTRL_HXX
#define _SVX_MEASCTRL_HXX

// include ---------------------------------------------------------------

#ifndef _CTRL_HXX //autogen
#include <vcl/ctrl.hxx>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif


class SfxItemSet;

class XOutputDevice;
class SdrMeasureObj;
class SdrModel;

/*************************************************************************
|*
|* SvxXMeasurePreview
|*
\************************************************************************/
class SVX_DLLPUBLIC SvxXMeasurePreview : public Control
{
 friend class SvxMeasurePage;

private:
    const SfxItemSet&   rAttrs;
    XOutputDevice*  pExtOutDev;
    SdrMeasureObj*      pMeasureObj;
    SdrModel*           pModel;

public:
            SvxXMeasurePreview( Window* pParent, const ResId& rResId,
                                const SfxItemSet& rInAttrs );
            ~SvxXMeasurePreview();

    virtual void Paint( const Rectangle& rRect );
    virtual void MouseButtonDown( const MouseEvent& rMEvt );

    void         SetAttributes( const SfxItemSet& rInAttrs );

    virtual void DataChanged( const DataChangedEvent& rDCEvt );
};

#endif


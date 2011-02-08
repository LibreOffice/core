/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _SVX_CONNCTRL_HXX
#define _SVX_CONNCTRL_HXX

// include ---------------------------------------------------------------

#ifndef _CTRL_HXX //autogen
#include <vcl/ctrl.hxx>
#endif
#include "svx/svxdllapi.h"

class SfxItemSet;
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
    sal_uInt16       GetLineDeltaAnz();

    void         Construct();
    void         SetView( const SdrView* pSdrView ) { pView = pSdrView; }

    virtual void DataChanged( const DataChangedEvent& rDCEvt );
};


#endif // _SVX_CONNCTRL_HXX


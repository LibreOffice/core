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


#ifndef _BASCTL_DLGEDVIEW_HXX
#define _BASCTL_DLGEDVIEW_HXX

#include <svx/svdview.hxx>

class DlgEditor;

//============================================================================
// DlgEdView
//============================================================================

class DlgEdView : public SdrView
{
private:
    DlgEditor* pDlgEditor;

public:
    TYPEINFO();

    DlgEdView( SdrModel* pModel, OutputDevice* pOut, DlgEditor* pEditor );
    virtual ~DlgEdView();

    virtual void MarkListHasChanged();
    virtual void MakeVisible( const Rectangle& rRect, Window& rWin );

protected:
    // overloaded to handle HitTest for some objects special
    using SdrView::CheckSingleSdrObjectHit;
    virtual SdrObject* CheckSingleSdrObjectHit(const Point& rPnt, sal_uInt16 nTol, SdrObject* pObj, SdrPageView* pPV, sal_uLong nOptions, const SetOfByte* pMVisLay) const;
};

#endif //_BASCTL_DLGEDVIEW_HXX

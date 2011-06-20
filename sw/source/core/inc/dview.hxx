/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#ifndef _DVIEW_HXX
#define _DVIEW_HXX


#include <svx/fmview.hxx>

class OutputDevice;
class SwViewImp;
class SwFrm;
class SwFlyFrm;
class SwAnchoredObject;

class SwDrawView : public FmFormView
{
    //Fuer den Anker
    Point           aAnchorPoint;       //Ankerposition
    SwViewImp      &rImp;               //Die View gehoert immer zu einer Shell

    const SwFrm *CalcAnchor();

    /** determine maximal order number for a 'child' object of given 'parent' object

        The maximal order number will be determined on the current object
        order hierarchy. It's the order number of the 'child' object with the
        highest order number. The calculation can be influenced by parameter
        <_pExclChildObj> - this 'child' object won't be considered.

        @param <_rParentObj>
        input parameter - 'parent' object, for which the maximal order number
        for its 'childs' will be determined.

        @param <_pExclChildObj>
        optional input parameter - 'child' object, which will not be considered
        on the calculation of the maximal order number
    */
    sal_uInt32 _GetMaxChildOrdNum( const SwFlyFrm& _rParentObj,
                                   const SdrObject* _pExclChildObj = 0L ) const;

    /** method to move 'repeated' objects of the given moved object to the
        according level

        @param <_rMovedAnchoredObj>
        input parameter - moved object, for which the 'repeated' ones have also
        to be moved.

        @param <_rMovedChildsObjs>
        input parameter - data collection of moved 'child' objects - the 'repeated'
        ones of these 'childs' will also been moved.
    */
    void _MoveRepeatedObjs( const SwAnchoredObject& _rMovedAnchoredObj,
                            const std::vector<SdrObject*>& _rMovedChildObjs ) const;

protected:
    // add custom handles (used by other apps, e.g. AnchorPos)
    virtual void AddCustomHdl();

    // overloaded to allow extra handling when picking SwVirtFlyDrawObj's
    using FmFormView::CheckSingleSdrObjectHit;
    virtual SdrObject* CheckSingleSdrObjectHit(const Point& rPnt, sal_uInt16 nTol, SdrObject* pObj, SdrPageView* pPV, sal_uLong nOptions, const SetOfByte* pMVisLay) const;

public:
    SwDrawView( SwViewImp &rI, SdrModel *pMd, OutputDevice* pOutDev=NULL );

    //aus der Basisklasse
    virtual SdrObject*   GetMaxToTopObj(SdrObject* pObj) const;
    virtual SdrObject*   GetMaxToBtmObj(SdrObject* pObj) const;
    virtual void         MarkListHasChanged();

    // #i7672#
    // Overload to resue edit background color in active text edit view (OutlinerView)
    virtual void ModelHasChanged();

    virtual void         ObjOrderChanged( SdrObject* pObj, sal_uLong nOldPos,
                                            sal_uLong nNewPos );
    virtual sal_Bool TakeDragLimit(SdrDragMode eMode, Rectangle& rRect) const;
    virtual void MakeVisible( const Rectangle&, Window &rWin );
    virtual void CheckPossibilities();

    const SwViewImp &Imp() const { return rImp; }
          SwViewImp &Imp()       { return rImp; }

    //Anker und Xor fuer das Draggen.
    void ShowDragAnchor();

    virtual void DeleteMarked();

    inline void ValidateMarkList() { FlushComeBackTimer(); }

    // #i99665#
    sal_Bool IsAntiAliasing() const;

    // method to replace marked/selected <SwDrawVirtObj>
    // by its reference object for delete of selection and group selection
    static void ReplaceMarkedDrawVirtObjs( SdrMarkView& _rMarkView );
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

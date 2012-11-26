/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#ifndef _DVIEW_HXX
#define _DVIEW_HXX


#include <svx/fmview.hxx>

class OutputDevice;
class SwViewImp;
class SwFrm;
class SwFlyFrm;
class SwAnchoredObject;
class SdrUndoManager;

class SwDrawView : public FmFormView
{
    //Fuer den Anker
    Point           aAnchorPoint;       //Ankerposition
    SwViewImp      &rImp;               //Die View gehoert immer zu einer Shell

    const SwFrm *CalcAnchor();

    /** determine maximal order number for a 'child' object of given 'parent' object

        OD 2004-08-20 #110810#
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

        @author OD
    */
    sal_uInt32 _GetMaxChildOrdNum( const SwFlyFrm& _rParentObj,
                                   const SdrObject* _pExclChildObj = 0L ) const;

    /** method to move 'repeated' objects of the given moved object to the
        according level

        OD 2004-08-23 #110810#

        @param <_rMovedAnchoredObj>
        input parameter - moved object, for which the 'repeated' ones have also
        to be moved.

        @param <_rMovedChildsObjs>
        input parameter - data collection of moved 'child' objects - the 'repeated'
        ones of these 'childs' will also been moved.

        @author OD
    */
    void _MoveRepeatedObjs( const SwAnchoredObject& _rMovedAnchoredObj,
                            const SdrObjectVector& _rMovedChildObjs ) const;

protected:
    // add custom handles (used by other apps, e.g. AnchorPos)
    virtual void AddCustomHdl();

    // overloaded to allow extra handling when picking SwVirtFlyDrawObj's
    using FmFormView::CheckSingleSdrObjectHit;
    virtual SdrObject* CheckSingleSdrObjectHit(const basegfx::B2DPoint& rPnt, double fTol, SdrObject* pObj, sal_uInt32 nOptions, const SetOfByte* pMVisLay) const;

    // support enhanced text edit for draw objects
    virtual SdrUndoManager* getSdrUndoManagerForEnhancedTextEdit() const;

public:
    SwDrawView( SwViewImp &rI, FmFormModel& rModel, OutputDevice* pOutDev = NULL );

    //aus der Basisklasse
    virtual SdrObject*   GetMaxToTopObj(SdrObject* pObj) const;
    virtual SdrObject*   GetMaxToBtmObj(SdrObject* pObj) const;
    virtual void handleSelectionChange();

    // #i7672#
    // Overload to resue edit background color in active text edit view (OutlinerView)
    virtual void LazyReactOnObjectChanges();

    virtual void ObjOrderChanged( SdrObject* pObj, sal_uInt32 nOldPos, sal_uInt32 nNewPos );
    virtual bool TakeDragLimit(SdrDragMode eMode, basegfx::B2DRange& rRange) const;
    virtual void MakeVisibleAtView( const basegfx::B2DRange& rRange, Window &rWin );
    virtual void CheckPossibilities();

    const SwViewImp &Imp() const { return rImp; }
          SwViewImp &Imp()       { return rImp; }

    //Anker und Xor fuer das Draggen.
    void ShowDragAnchor();

    virtual void DeleteMarked();

    //JP 06.10.98: 2. Versuch
    inline void ValidateMarkList() { ForceLazyReactOnObjectChanges(); }

    // --> OD 2009-03-05 #i99665#
    sal_Bool IsAntiAliasing() const;
    // <--

    // OD 18.06.2003 #108784# - method to replace marked/selected <SwDrawVirtObj>
    // by its reference object for delete of selection and group selection
    static void ReplaceMarkedDrawVirtObjs( SdrMarkView& _rMarkView );
};


#endif


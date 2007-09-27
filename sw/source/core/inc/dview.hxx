/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dview.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 08:55:39 $
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
#ifndef _DVIEW_HXX
#define _DVIEW_HXX


#ifndef _SVX_FMVIEW_HXX //autogen
#include <svx/fmview.hxx>
#endif

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
                            const std::vector<SdrObject*>& _rMovedChildObjs ) const;

protected:
    // add custom handles (used by other apps, e.g. AnchorPos)
    virtual void AddCustomHdl();

public:
    SwDrawView( SwViewImp &rI, SdrModel *pMd, OutputDevice* pOutDev=NULL );

    //aus der Basisklasse
    virtual SdrObject*   GetMaxToTopObj(SdrObject* pObj) const;
    virtual SdrObject*   GetMaxToBtmObj(SdrObject* pObj) const;
    virtual void         MarkListHasChanged();

    // #i7672#
    // Overload to resue edit background color in active text edit view (OutlinerView)
    virtual void ModelHasChanged();

    virtual void         ObjOrderChanged( SdrObject* pObj, ULONG nOldPos,
                                            ULONG nNewPos );
    virtual BOOL TakeDragLimit(SdrDragMode eMode, Rectangle& rRect) const;
    virtual void MakeVisible( const Rectangle&, Window &rWin );
    virtual void CheckPossibilities();

    const SwViewImp &Imp() const { return rImp; }
          SwViewImp &Imp()       { return rImp; }

    //Anker und Xor fuer das Draggen.
    void ShowDragAnchor();

    virtual void DeleteMarked();

    //JP 06.10.98: 2. Versuch
    inline void ValidateMarkList() { FlushComeBackTimer(); }

    // OD 18.06.2003 #108784# - method to replace marked/selected <SwDrawVirtObj>
    // by its reference object for delete of selection and group selection
    static void ReplaceMarkedDrawVirtObjs( SdrMarkView& _rMarkView );
};


#endif


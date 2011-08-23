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


#ifndef __GLOBFUNC_HXX
#define __GLOBFUNC_HXX

#ifndef _E3D_LABEL3D_HXX //autogen
#include <bf_svx/label3d.hxx>
#endif

#include "schgroup.hxx"
#include "objid.hxx"
#include "datapoin.hxx"
#include "datarow.hxx"
#include "objadj.hxx"
#ifndef _SVX_CHRTITEM_HXX //autogen
#include <bf_svx/chrtitem.hxx>
#endif

#ifndef _COM_SUN_STAR_DRAWING_XSHAPE_HPP_
#include <com/sun/star/drawing/XShape.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_ 
#include <com/sun/star/uno/Reference.h>
#endif

#include <vector>
namespace binfilter {

//Item-Debugging
#ifdef DBG_UTIL
    class ChartModel;
    extern void Dbg_DebugItems(SfxItemSet& rSet,ChartModel* pModel,long num = 32);
    #define DBG_ITEMS(a,b) Dbg_DebugItems(a,b)
    #define DBG_ITEMSX(a,b,c) Dbg_DebugItems(a,b,c)
#else
    #define DBG_ITEMS(a,b)
    #define DBG_ITEMSX(a,b,c)
#endif

//hunderstel Grad (centi-degrees)in Rad
#define CDEG2RAD(fAngle) (  (double)(fAngle)*F_PI/18000.0 )

void AxisAttrOld2New(SfxItemSet &rDestSet,BOOL bClear,long nId);
void AxisAttrNew2Old(SfxItemSet &rDestSet,long nId,BOOL bClear);

Size GetRotatedTextSize(const Size& rSize,const long nDegrees);
long GetTextRotation(SfxItemSet &rAttr);
long GetTextRotation(SfxItemSet &rAttr,SvxChartTextOrient eOrient);

SdrObject *SetObjectAttr (SdrObject  *pObj,
                          UINT16     nID,
                          BOOL       bProtect,
                          BOOL       bResize,
                          SfxItemSet *pAttr);

SdrObjList *CreateGroup (SdrObjList &rObjList,
                         UINT16     nID,
                         ULONG      nIndex = CONTAINER_APPEND);

SchObjGroup *CreateSimpleGroup (UINT16 nID,
                                BOOL   bProtect,
                                BOOL   bResize);


void CubicSpline (XPolygon &pKnownPoints,
                  int      n,
                  int      splineSize,
                  XPolygon &pSplines);

void TVector (int    n,
              int    k,
              double *t);

double TLeft (double x,
              int    i,
              int    k,
              double *t);

double TRight (double x,
               int    i,
               int    k,
               double *t);

void BVector (double x,
              int    n,
              int    k,
              double *b,
              double *t);

void BSPoint (int      n,
              Point    &p1,
              Point    &p2,
              XPolygon &pKnownPoints,
              double   *b);

void approxMesh (int      splineSize,
                 XPolygon &pSplines,
                 XPolygon &pKnownPoints,
                 int      n,
                 int      k);

double SizeBounds (double,
                   double,
                   BOOL);

void IncValue(double& rValue,
              double  fInc,
              BOOL    bLogarithm);

void DecValue(double& rValue,
              double  fInc,
              BOOL    bLogarithm);


Color RGBColor(ColorData nColorName);

String StackString(const String& aString);



void AdjustRect (Rectangle          &rRect,
                 ChartAdjust        eAdjust);

Size AdjustRotatedRect (const Rectangle	&rRect,
                        ChartAdjust		eAdjust,
                        const Rectangle	&rOldRect);

void SetAdjust(ChartAdjust        &eAdjust,
               SvxChartTextOrient eOrient);

void SetTextPos(SdrTextObj  &rTextObj,
                const Point &rPos,SfxItemSet* pAttr);

void AdjustTextSize(SdrTextObj &rTextObj,
                    const Size &rTextSize);

Size GetOutputSize(SdrTextObj& rTextObj);

/** Creates an SchObjGroup containing SdrPathObjects, which are created from the
    XPolygons given in the vector rPolygons.  The items in rItems are applied to
    all of the created objects.

    Note: The ChartModel is needed for the CTOR of SchObjGroup.
 */
SdrObject * CreateErrorGroup(
    ChartModel * pModel,
    const ::std::vector< XPolygon > & rPolygons,
    const SfxItemSet & rItems );



void CreateChartGroup( SchObjGroup* &, SdrObjList* & );




/** Intersect two SfxItemSets in the following way: Modify rDestSet such that
    after this function completes there are only items left that are contained
    in both sets and have equal content (using the != operator of SfxPoolItem)

    Items that are set in either of the itemsets but are not equal are
    invalidated.  This is useful for dialogs.  For a template-like usage, you
    can remove the invalid items by calling ClearInvalidItems() at the result.

    Note: The Intersect method of SfxItemSet works differently for equal and
          non-equal which-ranges.  If the ranges are equal it does something
          like this method, otherwise it just removes items which are not
          contained in the which range of the other item set.

    @param rSourceSet is the item set which will be iterated over
    @param rDestSet   is the item set which is modified such that the condition
                      described above is maintained.
 */
void IntersectSets( const SfxItemSet &  rSource, SfxItemSet &  rDest );

Point SetPointOfRectangle (const Rectangle& rRect, ChartAdjust eAdjust);

void ItemsToFont(const SfxItemSet& rSet,Font& rFont);

BOOL ChIsPointInsideRectangle( const Point& rPoint, const Rectangle& rRect );


namespace	sch	{
        /**	@descr	Extract an item for a given which id from one of two item
                sets.  If the the second item set rOverride is valid and 
                defines the requested item then return that item, else extract
                the item from the base set.  This has the same effect as merging
                the override item set into the base set but with the advantage
                that no item set has to be copied or even modified.
            @param	nWhich	Specifies the requested item.
            @param	rBaseSet	The item set from which the item is taken if
                it the override item set is not valid or does not contain it.
            @param	rOverrideSet	The item set first search for the requested
                item.
            @param	bOverrideIsValid	If TRUE then rOverrideSet is search for the
                requested item prior to rBaseSet.  If it is FALSE then 
                rOverrideSet is ignored.
        */
        inline const SfxPoolItem *	GetItem	(USHORT nWhich, 
                                    const SfxItemSet & rBaseSet, 
                                    const SfxItemSet & rOverrideSet, 
                                    BOOL bOverrideIsValid)
        {
            const SfxPoolItem * pItem;
            
            if (	bOverrideIsValid 
                &&	rOverrideSet.GetItemState (
                        nWhich, TRUE, &pItem) == SFX_ITEM_SET)
                return pItem;
            else
                return rBaseSet.GetItem (nWhich);
        }
}

} //namespace binfilter
#endif



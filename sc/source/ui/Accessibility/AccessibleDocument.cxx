/*************************************************************************
 *
 *  $RCSfile: AccessibleDocument.cxx,v $
 *
 *  $Revision: 1.22 $
 *
 *  last change: $Author: sab $ $Date: 2002-04-19 18:21:43 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/


#ifndef _SC_ACCESSIBLEDOCUMENT_HXX
#include "AccessibleDocument.hxx"
#endif
#ifndef _SC_ACCESSIBLESPREADSHEET_HXX
#include "AccessibleSpreadsheet.hxx"
#endif
#ifndef SC_TABVWSH_HXX
#include "tabvwsh.hxx"
#endif
#ifndef SC_ACCESSIBILITYHINTS_HXX
#include "AccessibilityHints.hxx"
#endif
#ifndef SC_DOCUMENT_HXX
#include "document.hxx"
#endif
#ifndef SC_DRWLAYER_HXX
#include "drwlayer.hxx"
#endif
#ifndef SC_UNOGUARD_HXX
#include "unoguard.hxx"
#endif
#ifndef SC_SHAPEUNO_HXX
#include "shapeuno.hxx"
#endif

#ifndef _DRAFTS_COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLEEVENTID_HPP_
#include <drafts/com/sun/star/accessibility/AccessibleEventId.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLESTATETYPE_HPP_
#include <drafts/com/sun/star/accessibility/AccessibleStateType.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLERELATIONTYPE_HPP_
#include <drafts/com/sun/star/accessibility/AccessibleRelationType.hpp>
#endif
#ifndef _COM_SUN_STAR_VIEW_XSELECTIONSUPPLIER_HPP_
#include <com/sun/star/view/XSelectionSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XSHAPE_HPP_
#include <com/sun/star/drawing/XShape.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XSHAPES_HPP_
#include <com/sun/star/drawing/XShapes.hpp>
#endif

#ifndef _UTL_ACCESSIBLESTATESETHELPER_HXX
#include <unotools/accessiblestatesethelper.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _SV_GEN_HXX
#include <tools/gen.hxx>
#endif
#ifndef _SVDPAGE_HXX
#include <svx/svdpage.hxx>
#endif
#ifndef _SVDOBJ_HXX
#include <svx/svdobj.hxx>
#endif
#ifndef _SVX_ACCESSIBILITY_SHAPE_TYPE_HANDLER_HXX
#include <svx/ShapeTypeHandler.hxx>
#endif
#ifndef _SVX_ACCESSIBILITY_ACCESSIBLE_SHAPE_HXX
#include <svx/AccessibleShape.hxx>
#endif
#ifndef _SVX_ACCESSIBILITY_ACCESSIBLE_SHAPE_TREE_INFO_HXX
#include <svx/AccessibleShapeTreeInfo.hxx>
#endif
#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif
#ifndef _SFXVIEWFRM_HXX
#include <sfx2/viewfrm.hxx>
#endif
#ifndef _SVX_UNOSHGRP_HXX
#include <svx/unoshcol.hxx>
#endif
#ifndef _SVX_UNOSHAPE_HXX
#include <svx/unoshape.hxx>
#endif
#ifndef _SVX_ACCESSIBILITY_ACCESSIBLE_SHAPE_HXX
#include <svx/AccessibleShape.hxx>
#endif
#ifndef _UTL_ACCESSIBLERELATIONSETHELPER_HXX_
#include <unotools/accessiblerelationsethelper.hxx>
#endif

#include <list>

using namespace ::com::sun::star;
using namespace ::drafts::com::sun::star::accessibility;

    //=====  internal  ========================================================

struct ScAccessibleShapeData
{
    ScAccessibleShapeData() : pAccShape(NULL), pRelationCell(NULL), nVectorIndex(-1), bSelected(sal_False) {}
    ~ScAccessibleShapeData();
    mutable accessibility::AccessibleShape* pAccShape;
    mutable ScAddress*          pRelationCell; // if it is NULL this shape is anchored on the table
    com::sun::star::uno::Reference< com::sun::star::drawing::XShape > xShape;
    mutable sal_uInt32          nVectorIndex;
    mutable sal_Bool            bSelected;
};

ScAccessibleShapeData::~ScAccessibleShapeData()
{
    if (pAccShape)
        pAccShape->release();
}

struct ScShapeDataLess
{
    sal_Bool operator()(const ScAccessibleShapeData& rData1, const ScAccessibleShapeData& rData2) const
    {
      sal_Bool bResult(sal_False);
      if (rData1.xShape.is() && rData2.xShape.is())
          bResult = (rData1.xShape.get() < rData2.xShape.get());
      return bResult;
    }
};

struct DeselectShape
{
    void operator() (const ScAccessibleShapeData& rAccShapeData) const
    {
        rAccShapeData.bSelected = sal_False;
        if (rAccShapeData.pAccShape)
            rAccShapeData.pAccShape->ResetState(AccessibleStateType::SELECTED);
    }
};

struct SelectShape
{
    uno::Reference < drawing::XShapes > xShapes;
    SelectShape(uno::Reference<drawing::XShapes>& xTemp) : xShapes(xTemp) {}
    void operator() (const ScAccessibleShapeData& rAccShapeData) const
    {
        rAccShapeData.bSelected = sal_True;
        if (rAccShapeData.pAccShape)
            rAccShapeData.pAccShape->SetState(AccessibleStateType::SELECTED);
        if (xShapes.is())
            xShapes->add(rAccShapeData.xShape);
    }
};

class ScChildrenShapes : public SfxListener
{
public:
    ScChildrenShapes(ScAccessibleDocument* pAccessibleDocument, ScTabViewShell* pViewShell);
    ~ScChildrenShapes();

    ///=====  SfxListener  =====================================================

    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

    ///=====  Internal  ========================================================
    sal_Int32 GetCount() const;
    uno::Reference< XAccessible > Get(sal_Int32 nIndex) const;
    uno::Reference< XAccessible > GetAt(const awt::Point& rPoint) const;

    // gets the index of the shape starting on 0 (without the index of the table)
    // returns the selected shape
    sal_Bool IsSelected(sal_Int32 nIndex,
        com::sun::star::uno::Reference<com::sun::star::drawing::XShape>& rShape) const;

    sal_Bool SelectionChanged();

    void Select(sal_Int32 nIndex);
    void DeselectAll(); // deselect also the table
    void SelectAll();
    sal_Int32 GetSelectedCount() const;
    uno::Reference< XAccessible > GetSelected(sal_Int32 nSelectedChildIndex) const;
    void Deselect(sal_Int32 nChildIndex);

    SdrPage* GetDrawPage() const;
private:
    typedef std::set<ScAccessibleShapeData, ScShapeDataLess> SortedShapesList;
    typedef std::vector<SortedShapesList::iterator> ShapesItrs;

    mutable SortedShapesList maSortedShapes;
    mutable ShapesItrs maShapes;

    mutable accessibility::AccessibleShapeTreeInfo maShapeTreeInfo;
    mutable com::sun::star::uno::Reference<com::sun::star::view::XSelectionSupplier> xSelectionSupplier;
    mutable sal_uInt32 mnSdrObjCount;
    mutable sal_uInt32 mnShapesSelected;
    ScTabViewShell* mpViewShell;
    ScAccessibleDocument* mpAccessibleDocument;

    void FindSelectedShapesChanges(const com::sun::star::uno::Reference<com::sun::star::drawing::XShapes>& xShapes, sal_Bool bCommitChange) const;
    void FillSelectionSupplier() const;

    ScAddress* GetAnchor(const uno::Reference<drawing::XShape>& xShape) const;
    uno::Reference<XAccessibleRelationSet> GetRelationSet(const SortedShapesList::iterator& aItr) const;
    void CheckWhetherAnchorChanged(const uno::Reference<drawing::XShape>& xShape) const;
    void ScChildrenShapes::SetAnchor(const uno::Reference<drawing::XShape>& xShape, SortedShapesList::iterator& rItr) const;
    void AddShape(const uno::Reference<drawing::XShape>& xShape, sal_Bool bCommitChange) const;
    void RemoveShape(const uno::Reference<drawing::XShape>& xShape) const;

    SortedShapesList::iterator FindShape(const uno::Reference<drawing::XShape>& xShape) const;

    sal_Int8 Compare(const com::sun::star::uno::Reference<com::sun::star::drawing::XShape>& xShape1,
        const com::sun::star::uno::Reference<com::sun::star::drawing::XShape>& xShape2) const;
};

ScChildrenShapes::ScChildrenShapes(ScAccessibleDocument* pAccessibleDocument, ScTabViewShell* pViewShell)
    :
    mpAccessibleDocument(pAccessibleDocument),
    mpViewShell(pViewShell)
{
    FillSelectionSupplier();

    GetCount(); // fill list with filtered shapes (no internal shapes)

    if (mnShapesSelected)
    {
        //set flag on every selected shape
        if (!xSelectionSupplier.is())
            throw uno::RuntimeException();

        uno::Reference<drawing::XShapes> xShapes(xSelectionSupplier->getSelection(), uno::UNO_QUERY);
        if (xShapes.is())
            FindSelectedShapesChanges(xShapes, sal_False);
    }
    if (pViewShell)
    {
        SfxBroadcaster* pDrawBC = pViewShell->GetViewData()->GetDocument()->GetDrawBroadcaster();
        if (pDrawBC)
            StartListening(*pDrawBC);
    }
}

ScChildrenShapes::~ScChildrenShapes()
{
    if (mpViewShell)
    {
        SfxBroadcaster* pDrawBC = mpViewShell->GetViewData()->GetDocument()->GetDrawBroadcaster();
        if (pDrawBC)
            EndListening(*pDrawBC);
    }
}

void ScChildrenShapes::Notify(SfxBroadcaster& rBC, const SfxHint& rHint)
{
    if ( rHint.ISA( SdrHint ) )
    {
        const SdrHint* pSdrHint = PTR_CAST( SdrHint, &rHint );
        if (pSdrHint)
        {
            SdrObject* pObj = const_cast<SdrObject*>(pSdrHint->GetObject());
            if (pObj && (pObj->GetLayer() != SC_LAYER_INTERN))
            {
                switch (pSdrHint->GetKind())
                {
                    case HINT_OBJCHG :         // Objekt geaendert
                    {
                        uno::Reference<drawing::XShape> xShape (pObj->getUnoShape(), uno::UNO_QUERY);
                        if (xShape.is())
                            CheckWhetherAnchorChanged(xShape);
                    }
                    break;
                    case HINT_OBJINSERTED :    // Neues Zeichenobjekt eingefuegt
                    {
                        uno::Reference<drawing::XShape> xShape (pObj->getUnoShape(), uno::UNO_QUERY);
                        if (xShape.is())
                            AddShape(xShape, sal_True);
                    }
                    break;
                    case HINT_OBJREMOVED :     // Zeichenobjekt aus Liste entfernt
                    {
                        uno::Reference<drawing::XShape> xShape (pObj->getUnoShape(), uno::UNO_QUERY);
                        if (xShape.is())
                            RemoveShape(xShape);
                    }
                    break;
                    default :
                    {
                        // other events are not interesting
                    }
                    break;
                }
            }
        }
    }
}

sal_Int32 ScChildrenShapes::GetCount() const
{
    SdrPage* pDrawPage = GetDrawPage();
    if (pDrawPage && maShapes.empty())
    {
        mnSdrObjCount = pDrawPage->GetObjCount();
        maShapes.reserve(mnSdrObjCount);
        for (sal_uInt32 i = 0; i < mnSdrObjCount; ++i)
        {
            SdrObject* pObj = pDrawPage->GetObj(i);
            if (pObj && (pObj->GetLayer() != SC_LAYER_INTERN))
            {
                uno::Reference< drawing::XShape > xShape (pObj->getUnoShape(), uno::UNO_QUERY);
                AddShape(xShape, sal_False);
            }
        }
    }
    return maShapes.size();
}

uno::Reference< XAccessible > ScChildrenShapes::Get(sal_Int32 nIndex) const
{
    if (maShapes.empty())
        GetCount(); // fill list with filtered shapes (no internal shapes)

    if (static_cast<sal_uInt32>(nIndex) >= maShapes.size())
        throw lang::IndexOutOfBoundsException();

    if (!maShapes[nIndex]->pAccShape)
    {
        accessibility::ShapeTypeHandler& rShapeHandler = accessibility::ShapeTypeHandler::Instance();
        maShapes[nIndex]->pAccShape = rShapeHandler.CreateAccessibleObject(
            maShapes[nIndex]->xShape, mpAccessibleDocument, maShapeTreeInfo);
        if (maShapes[nIndex]->pAccShape)
        {
            maShapes[nIndex]->pAccShape->acquire();
            if (maShapes[nIndex]->bSelected)
                maShapes[nIndex]->pAccShape->SetState(AccessibleStateType::SELECTED);
        }
    }
    return maShapes[nIndex]->pAccShape;
}

uno::Reference< XAccessible > ScChildrenShapes::GetAt(const awt::Point& rPoint) const
{
    uno::Reference<XAccessible> xAccessible;
    DBG_ERRORFILE("not implemented");
    return xAccessible;
}

sal_Bool ScChildrenShapes::IsSelected(sal_Int32 nIndex,
                        uno::Reference<drawing::XShape>& rShape) const
{
    sal_Bool bResult (sal_False);
    if (maShapes.empty())
        GetCount(); // fill list with filtered shapes (no internal shapes)

    if (static_cast<sal_uInt32>(nIndex) >= maShapes.size())
        throw lang::IndexOutOfBoundsException();

    if (!xSelectionSupplier.is())
        throw uno::RuntimeException();

    bResult = maShapes[nIndex]->bSelected;
    rShape = maShapes[nIndex]->xShape;

#ifndef PRODUCT // test whether it is truly selected by a slower method
    uno::Reference< drawing::XShape > xReturnShape;
    sal_Bool bDebugResult(sal_False);
    uno::Reference<container::XIndexAccess> xIndexAccess;
    xSelectionSupplier->getSelection() >>= xIndexAccess;

    if (xIndexAccess.is())
    {
        sal_Int32 nCount(xIndexAccess->getCount());
        if (nCount)
        {
            uno::Reference< drawing::XShape > xShape;
            uno::Reference< drawing::XShape > xIndexShape = maShapes[nIndex]->xShape;
            sal_Int32 i(0);
            while (!bResult && (i < nCount))
            {
                xIndexAccess->getByIndex(i) >>= xShape;
                if (xShape.is() && (Compare(xIndexShape, xShape) == 0))
                {
                    bDebugResult = sal_True;
                    xReturnShape = xShape;
                }
                else
                    ++i;
            }
        }
    }
    DBG_ASSERT((bResult == bDebugResult) && (Compare(rShape, xReturnShape) == 0), "found the wrong shape or result")
#endif

    return bResult;
}

sal_Bool ScChildrenShapes::SelectionChanged()
{
    sal_Bool bResult(sal_False);
    if (!xSelectionSupplier.is())
        throw uno::RuntimeException();

    uno::Reference<drawing::XShapes> xShapes(xSelectionSupplier->getSelection(), uno::UNO_QUERY);
    if (xShapes.is())
    {
        sal_uInt32 nOldSelected(mnShapesSelected);
        mnShapesSelected = xShapes->getCount();
        if (nOldSelected != mnShapesSelected)
        {
            FindSelectedShapesChanges(xShapes, sal_True);
            bResult = sal_True;
        }
    }
    return bResult;
}

void ScChildrenShapes::Select(sal_Int32 nIndex)
{
    if (maShapes.empty())
        GetCount(); // fill list with filtered shapes (no internal shapes)

    if (static_cast<sal_uInt32>(nIndex) >= maShapes.size())
        throw lang::IndexOutOfBoundsException();

    if (!xSelectionSupplier.is())
        throw uno::RuntimeException();

    uno::Reference<drawing::XShape> xShape;
    if (!IsSelected(nIndex, xShape))
    {
        uno::Reference<drawing::XShapes> xShapes;
        xSelectionSupplier->getSelection() >>= xShapes;

        if (!xShapes.is())
            xShapes = new SvxShapeCollection();

        xShapes->add(maShapes[nIndex]->xShape);

        xSelectionSupplier->select(uno::makeAny(xShapes));

        maShapes[nIndex]->bSelected = sal_True;
        if (maShapes[nIndex]->pAccShape)
            maShapes[nIndex]->pAccShape->SetState(AccessibleStateType::SELECTED);
    }
}

void ScChildrenShapes::DeselectAll()
{
    if (!xSelectionSupplier.is())
        throw uno::RuntimeException();

    xSelectionSupplier->select(uno::Any()); //deselects all

    std::for_each(maSortedShapes.begin(), maSortedShapes.end(), DeselectShape());
}

void ScChildrenShapes::SelectAll()
{
    if (!xSelectionSupplier.is())
        throw uno::RuntimeException();

    if (maShapes.empty())
        GetCount(); // fill list with filtered shapes (no internal shapes)

    uno::Reference<drawing::XShapes> xShapes;
    xShapes = new SvxShapeCollection();

    std::for_each(maSortedShapes.begin(), maSortedShapes.end(), SelectShape(xShapes));

    xSelectionSupplier->select(uno::makeAny(xShapes));
}

sal_Int32 ScChildrenShapes::GetSelectedCount() const
{
    sal_Int32 nResult(0);
    if (!xSelectionSupplier.is())
        throw uno::RuntimeException();

    uno::Reference<container::XIndexAccess> xIndexAccess;
    xSelectionSupplier->getSelection() >>= xIndexAccess;

    if (xIndexAccess.is())
        nResult = xIndexAccess->getCount();

    return nResult;
}

uno::Reference< XAccessible > ScChildrenShapes::GetSelected(sal_Int32 nSelectedChildIndex) const
{
    uno::Reference< XAccessible > xAccessible;
    uno::Reference<container::XIndexAccess> xIndexAccess;
    xSelectionSupplier->getSelection() >>= xIndexAccess;

    if (xIndexAccess.is())
    {
        if ( nSelectedChildIndex >= xIndexAccess->getCount())
            throw lang::IndexOutOfBoundsException();

        uno::Reference<drawing::XShape> xShape;
        xIndexAccess->getByIndex(nSelectedChildIndex) >>= xShape;
        if (xShape.is())
        {
            if (maShapes.empty())
                GetCount(); // fill list with filtered shapes (no internal shapes)

            SortedShapesList::iterator aItr = FindShape(xShape);
            if (aItr != maSortedShapes.end())
                xAccessible = Get(aItr->nVectorIndex);
        }
    }
    return xAccessible;
}

void ScChildrenShapes::Deselect(sal_Int32 nChildIndex)
{
    uno::Reference<drawing::XShape> xShape;
    if (IsSelected(nChildIndex, xShape))
    {
        if (xShape.is())
        {
            uno::Reference<drawing::XShapes> xShapes;
            xSelectionSupplier->getSelection() >>= xShapes;
            if (xShapes.is())
                xShapes->remove(xShape);

            xSelectionSupplier->select(uno::makeAny(xShapes));

            maShapes[nChildIndex]->bSelected = sal_False;
            if (maShapes[nChildIndex]->pAccShape)
                maShapes[nChildIndex]->pAccShape->ResetState(AccessibleStateType::SELECTED);
        }
    }
}


SdrPage* ScChildrenShapes::GetDrawPage() const
{
    sal_uInt16 nTab(mpAccessibleDocument->getVisibleTable());
    SdrPage* pDrawPage = NULL;
    if (mpViewShell && mpViewShell->GetViewData())
    {
        ScDocument* pDoc = mpViewShell->GetViewData()->GetDocument();
        if (pDoc && pDoc->GetDrawLayer())
        {
            ScDrawLayer* pDrawLayer = pDoc->GetDrawLayer();
            if (pDrawLayer->HasObjects() && (pDrawLayer->GetPageCount() > nTab))
                pDrawPage = pDrawLayer->GetPage(nTab);
        }
    }
    return pDrawPage;
}

void ScChildrenShapes::FindSelectedShapesChanges(const uno::Reference<drawing::XShapes>& xShapes, sal_Bool bCommitChange) const
{
    uno::Reference<container::XEnumerationAccess> xEnumAcc(xShapes, uno::UNO_QUERY);
    if (xEnumAcc.is())
    {
        uno::Reference<container::XEnumeration> xEnum = xEnumAcc->createEnumeration();
        if (xEnum.is())
        {
            SortedShapesList aShapesList;
            while (xEnum->hasMoreElements())
            {
                uno::Reference< drawing::XShape > xShape;
                xEnum->nextElement() >>= xShape;
                if (xShape.is())
                {
                    ScAccessibleShapeData aShapeData;
                    aShapeData.xShape = xShape;
                    aShapesList.insert(aShapeData);
                }
            }

            SortedShapesList::iterator aSortedShapesEndItr = maSortedShapes.end();
            SortedShapesList::iterator aXShapesItr(aShapesList.begin());
            SortedShapesList::const_iterator aXShapesEndItr(aShapesList.end());
            SortedShapesList::iterator aDataItr(maSortedShapes.begin());
            SortedShapesList::const_iterator aDataEndItr(maSortedShapes.end());
            while((aDataItr != aDataEndItr))
            {
                sal_Int8 nComp(0);
                if (aXShapesItr == aXShapesEndItr)
                    nComp = -1; // simulate that the Shape is lower, so the selction state will be removed
                else
                    nComp = Compare(aDataItr->xShape, aXShapesItr->xShape);
                if (nComp == 0)
                {
                    if (!aDataItr->bSelected)
                    {
                        aDataItr->bSelected = sal_True;
                        if (aDataItr->pAccShape)
                            aDataItr->pAccShape->SetState(AccessibleStateType::SELECTED);
                    }
                    ++aDataItr;
                    ++aXShapesItr;
                }
                else if (nComp < 0)
                {
                    if (aDataItr->bSelected)
                    {
                        aDataItr->bSelected = sal_False;
                        if (aDataItr->pAccShape)
                            aDataItr->pAccShape->ResetState(AccessibleStateType::SELECTED);
                    }
                    ++aDataItr;
                }
                else
                {
                    DBG_ERRORFILE("here is a selected shape which is not in the childlist");
                    ++aXShapesItr;
                }
            }
        }
    }
}

void ScChildrenShapes::FillSelectionSupplier() const
{
    if (!xSelectionSupplier.is() && mpViewShell)
    {
        SfxViewFrame* pViewFrame = mpViewShell->GetViewFrame();
        if (pViewFrame)
        {
            SfxFrame* pFrame = pViewFrame->GetFrame();
            if (pFrame)
            {
                xSelectionSupplier = uno::Reference<view::XSelectionSupplier>(pFrame->GetController(), uno::UNO_QUERY);
                if (xSelectionSupplier.is())
                {
                    if (mpAccessibleDocument)
                        xSelectionSupplier->addSelectionChangeListener(mpAccessibleDocument);
                    uno::Reference<drawing::XShapes> xShapes (xSelectionSupplier->getSelection(), uno::UNO_QUERY);
                    if (xShapes.is())
                        mnShapesSelected = xShapes->getCount();
                }
            }
        }
    }
}

ScAddress* ScChildrenShapes::GetAnchor(const uno::Reference<drawing::XShape>& xShape) const
{
    ScAddress* pAddress = NULL;
    if (mpViewShell)
    {
        SvxShape* pShapeImp = SvxShape::getImplementation(xShape);
        uno::Reference<beans::XPropertySet> xShapeProp(xShape, uno::UNO_QUERY);
        if (pShapeImp && xShapeProp.is())
        {
            SdrObject *pSdrObj = pShapeImp->GetSdrObject();
            if (pSdrObj)
            {
                if (ScDrawLayer::GetAnchor(pSdrObj) == SCA_CELL)
                {
                    ScDocument* pDoc = mpViewShell->GetViewData()->GetDocument();
                    if (pDoc)
                    {
                        rtl::OUString sCaptionShape(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.CaptionShape"));
                        awt::Point aPoint(xShape->getPosition());
                        awt::Size aSize(xShape->getSize());
                        rtl::OUString sType(xShape->getShapeType());
                        Rectangle aRectangle(aPoint.X, aPoint.Y, aPoint.X + aSize.Width, aPoint.Y + aSize.Height);
                        if ( sType.equals(sCaptionShape) )
                        {
                            awt::Point aRelativeCaptionPoint;
                            rtl::OUString sCaptionPoint( RTL_CONSTASCII_USTRINGPARAM( "CaptionPoint" ));
                            xShapeProp->getPropertyValue( sCaptionPoint ) >>= aRelativeCaptionPoint;
                            Point aCoreRelativeCaptionPoint(aRelativeCaptionPoint.X, aRelativeCaptionPoint.Y);
                            Point aCoreAbsoluteCaptionPoint(aPoint.X, aPoint.Y);
                            aCoreAbsoluteCaptionPoint += aCoreRelativeCaptionPoint;
                            aRectangle.Union(Rectangle(aCoreAbsoluteCaptionPoint, aCoreAbsoluteCaptionPoint));
                        }
                        ScRange aRange = pDoc->GetRange(static_cast<USHORT>(mpAccessibleDocument->getVisibleTable()), aRectangle);
                        pAddress = new ScAddress(aRange.aStart);
                    }
                }
//              else
//                  do nothing, because it is always a NULL Pointer
            }
        }
    }

    return pAddress;
}

uno::Reference<XAccessibleRelationSet> ScChildrenShapes::GetRelationSet(const SortedShapesList::iterator& aItr) const
{
    utl::AccessibleRelationSetHelper* pRelationSet = new utl::AccessibleRelationSetHelper();

    if(pRelationSet && mpAccessibleDocument)
    {
        uno::Reference<XAccessible> xAccessible = mpAccessibleDocument->getAccessibleChild(0); // should be the current table
        if (aItr->pRelationCell && xAccessible.is())
        {
            uno::Reference<XAccessibleTable> xAccTable (xAccessible->getAccessibleContext(), uno::UNO_QUERY);
            if (xAccTable.is())
                xAccessible = xAccTable->getAccessibleCellAt(aItr->pRelationCell->Row(), aItr->pRelationCell->Col());
        }
        AccessibleRelation aRelation;
        aRelation.TargetSet.realloc(1);
        aRelation.TargetSet[0] = xAccessible;
        aRelation.RelationType = AccessibleRelationType::CONTROLLED_BY;
        pRelationSet->AddRelation(aRelation);
    }

    return pRelationSet;
}

void ScChildrenShapes::CheckWhetherAnchorChanged(const uno::Reference<drawing::XShape>& xShape) const
{
    SortedShapesList::iterator aItr = FindShape(xShape);
    SetAnchor(xShape, aItr);
}

void ScChildrenShapes::SetAnchor(const uno::Reference<drawing::XShape>& xShape, SortedShapesList::iterator& rItr) const
{
    if (rItr != maSortedShapes.end())
    {
        ScAddress* pAddress = GetAnchor(xShape);
        if ((pAddress && rItr->pRelationCell && (*pAddress != *(rItr->pRelationCell))) ||
            (!pAddress && rItr->pRelationCell) || (pAddress && !rItr->pRelationCell))
        {
            if (rItr->pRelationCell)
                delete rItr->pRelationCell;
            rItr->pRelationCell = pAddress;
            if (rItr->pAccShape)
                rItr->pAccShape->SetRelationSet(GetRelationSet(rItr));
        }
    }
}

void ScChildrenShapes::AddShape(const uno::Reference<drawing::XShape>& xShape, sal_Bool bCommitChange) const
{
    ScAccessibleShapeData aShape;
    aShape.xShape = xShape;
    std::pair<SortedShapesList::iterator, bool> aPair = maSortedShapes.insert(aShape);
    if (aPair.second)
    {
        SortedShapesList::iterator aItr = aPair.first;
        aItr->nVectorIndex = maShapes.size();
        maShapes.push_back(aItr);
        SetAnchor(xShape, aItr);

        if (!xSelectionSupplier.is())
            throw uno::RuntimeException();

        uno::Reference<container::XEnumerationAccess> xEnumAcc(xSelectionSupplier->getSelection(), uno::UNO_QUERY);
        if (xEnumAcc.is())
        {
            uno::Reference<container::XEnumeration> xEnum = xEnumAcc->createEnumeration();
            if (xEnum.is())
            {
                uno::Reference<drawing::XShape> xSelectedShape;
                sal_Bool bFound(sal_False);
                while (!bFound && xEnum->hasMoreElements())
                {
                    xEnum->nextElement() >>= xSelectedShape;
                    if (xShape.is() && (Compare(xShape, xSelectedShape) == 0))
                    {
                        aItr->bSelected = sal_True;
                        bFound = sal_True;
                    }
                }
            }
        }
        if (mpAccessibleDocument && bCommitChange)
        {
            AccessibleEventObject aEvent;
            aEvent.EventId = AccessibleEventId::ACCESSIBLE_CHILD_EVENT;
            aEvent.Source = uno::Reference< XAccessible >(mpAccessibleDocument);
            aEvent.NewValue <<= Get(aItr->nVectorIndex);

            mpAccessibleDocument->CommitChange(aEvent); // new child - event
        }
    }
    else
        DBG_ERRORFILE("shape is always in the list");
}

void ScChildrenShapes::RemoveShape(const uno::Reference<drawing::XShape>& xShape) const
{
    SortedShapesList::iterator aItr = FindShape(xShape);
    if (aItr != maSortedShapes.end())
    {
        if (mpAccessibleDocument)
        {
            uno::Reference<XAccessible> xOldAccessible (Get(aItr->nVectorIndex));

            maShapes.erase(maShapes.begin() + aItr->nVectorIndex);
            maSortedShapes.erase(aItr);

            AccessibleEventObject aEvent;
            aEvent.EventId = AccessibleEventId::ACCESSIBLE_CHILD_EVENT;
            aEvent.Source = uno::Reference< XAccessible >(mpAccessibleDocument);
            aEvent.OldValue <<= uno::makeAny(xOldAccessible);

            mpAccessibleDocument->CommitChange(aEvent); // child is gone - event
        }
        else
        {
            maShapes.erase(maShapes.begin() + aItr->nVectorIndex);
            maSortedShapes.erase(aItr);
        }
    }
    else
        DBG_ERRORFILE("shape was not in internal list");
}

ScChildrenShapes::SortedShapesList::iterator ScChildrenShapes::FindShape(const uno::Reference<drawing::XShape>& xShape) const
{
    ScAccessibleShapeData aShape;
    aShape.xShape = xShape;
    return maSortedShapes.find(aShape);
}

sal_Int8 ScChildrenShapes::Compare(const uno::Reference<drawing::XShape>& xShape1,
        const uno::Reference<drawing::XShape>& xShape2) const
{
    sal_Int8 nResult(0);
    if (xShape1.is() && xShape2.is())
    {
        if (xShape1.get() < xShape2.get())
            nResult = -1;
        else if (xShape1.get() > xShape2.get())
            nResult = 1;
    }
    else if (xShape1.is()) // a not give shape is lesser than a given shape
        nResult = 1;
    else if (xShape2.is())
        nResult = -1;

    return nResult;
}

// ============================================================================

ScAccessibleDocument::ScAccessibleDocument(
        const uno::Reference<XAccessible>& rxParent,
        ScTabViewShell* pViewShell,
        ScSplitPos eSplitPos)
    : ScAccessibleDocumentBase(rxParent),
    mpViewShell(pViewShell),
    meSplitPos(eSplitPos),
    mpAccessibleSpreadsheet(NULL),
    mpChildrenShapes(NULL),
    mbCompleteSheetSelected(sal_False)
{
    if (pViewShell)
        pViewShell->AddAccessibilityObject(*this);
}

void ScAccessibleDocument::Init()
{
    if(!mpChildrenShapes)
        mpChildrenShapes = new ScChildrenShapes(this, mpViewShell);
}

ScAccessibleDocument::~ScAccessibleDocument(void)
{
    if (!ScAccessibleContextBase::IsDefunc() && !rBHelper.bInDispose)
    {
        // increment refcount to prevent double call off dtor
        osl_incrementInterlockedCount( &m_refCount );
        dispose();
    }
}

void SAL_CALL ScAccessibleDocument::disposing()
{
    FreeAccessibleSpreadsheet();
    if (mpViewShell)
    {
        mpViewShell->RemoveAccessibilityObject(*this);
        mpViewShell = NULL;
    }
    if (mpChildrenShapes)
        DELETEZ(mpChildrenShapes);

    ScAccessibleDocumentBase::disposing();
}

void SAL_CALL ScAccessibleDocument::disposing( const lang::EventObject& Source )
        throw (uno::RuntimeException)
{
    disposing();
}

    //=====  SfxListener  =====================================================

void ScAccessibleDocument::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    if (rHint.ISA( ScAccGridViewChangeHint ) )
    {
        const ScAccGridViewChangeHint& rRef = (const ScAccGridViewChangeHint&)rHint;
        if ((rRef.GetOldGridWin() == meSplitPos) ||
            (rRef.GetNewGridWin() == meSplitPos))
        {
            awt::FocusEvent aFocusEvent;
            aFocusEvent.Temporary = sal_False;
            if (rRef.GetOldGridWin() == meSplitPos)
            {
                aFocusEvent.NextFocus = rRef.GetNewAccessible();
                CommitFocusLost(aFocusEvent);
            }
            else
            {
                aFocusEvent.NextFocus = rRef.GetOldAccessible();
                CommitFocusGained(aFocusEvent);
            }
        }
    }
    else if (rHint.ISA( SfxSimpleHint ))
    {
        const SfxSimpleHint& rRef = (const SfxSimpleHint&)rHint;
        // only notify if child exist, otherwise it is not necessary
        if ((rRef.GetId() == SC_HINT_ACC_TABLECHANGED) &&
            mpAccessibleSpreadsheet)
        {
            AccessibleEventObject aEvent;
            aEvent.EventId = AccessibleEventId::ACCESSIBLE_CHILD_EVENT;
            aEvent.Source = uno::Reference< XAccessible >(this);
            aEvent.OldValue <<= GetAccessibleSpreadsheet();

            CommitChange(aEvent); // child is gone - event

            aEvent.OldValue = uno::Any();
            FreeAccessibleSpreadsheet(); // free the spreadsheet after free the reference on this object
            aEvent.NewValue <<= GetAccessibleSpreadsheet();

            CommitChange(aEvent); // there is a new child - event
        }
    }

    ScAccessibleDocumentBase::Notify(rBC, rHint);
}

void SAL_CALL ScAccessibleDocument::selectionChanged( const lang::EventObject& aEvent )
        throw (uno::RuntimeException)
{
    sal_Bool bSelectionChanged(sal_False);
    if (mpAccessibleSpreadsheet)
    {
        sal_Bool bOldSelected(mbCompleteSheetSelected);
        mbCompleteSheetSelected = IsTableSelected();
        if (bOldSelected != mbCompleteSheetSelected)
        {
            mpAccessibleSpreadsheet->CompleteSelectionChanged(mbCompleteSheetSelected);
            bSelectionChanged = sal_True;
        }
    }

    if (mpChildrenShapes && mpChildrenShapes->SelectionChanged())
        bSelectionChanged = sal_True;

    if (bSelectionChanged)
    {
        AccessibleEventObject aEvent;
        aEvent.EventId = AccessibleEventId::ACCESSIBLE_SELECTION_EVENT;
        aEvent.Source = uno::Reference< XAccessible >(this);

        CommitChange(aEvent);
    }
}

    //=====  XInterface  =====================================================

uno::Any SAL_CALL ScAccessibleDocument::queryInterface( uno::Type const & rType )
    throw (uno::RuntimeException)
{
    uno::Any aAny (ScAccessibleDocumentImpl::queryInterface(rType));
    return aAny.hasValue() ? aAny : ScAccessibleContextBase::queryInterface(rType);
}

void SAL_CALL ScAccessibleDocument::acquire()
    throw ()
{
    ScAccessibleContextBase::acquire();
}

void SAL_CALL ScAccessibleDocument::release()
    throw ()
{
    ScAccessibleContextBase::release();
}

    //=====  XAccessibleComponent  ============================================

uno::Reference< XAccessible > SAL_CALL ScAccessibleDocument::getAccessibleAt(
        const awt::Point& rPoint )
        throw (uno::RuntimeException)
{
    ScUnoGuard aGuard;
    uno::Reference<XAccessible> xAccessible = NULL;
    if (mpChildrenShapes)
        xAccessible = mpChildrenShapes->GetAt(rPoint);
    if(!xAccessible.is())
        xAccessible = GetAccessibleSpreadsheet();
    return xAccessible;
}

void SAL_CALL ScAccessibleDocument::grabFocus(  )
        throw (uno::RuntimeException)
{
    ScUnoGuard aGuard;
    if (getAccessibleParent().is())
    {
        uno::Reference<XAccessibleComponent> xAccessibleComponent(getAccessibleParent()->getAccessibleContext(), uno::UNO_QUERY);
        if (xAccessibleComponent.is())
        {
            xAccessibleComponent->grabFocus();
            // grab only focus if it does not have the focus and it is not hidden
            if (mpViewShell && mpViewShell->GetViewData() &&
                (mpViewShell->GetViewData()->GetActivePart() != meSplitPos) &&
                mpViewShell->GetWindowByPos(meSplitPos)->IsVisible())
            {
                mpViewShell->ActivatePart(meSplitPos);
            }
        }
    }
}

    //=====  XAccessibleContext  ==============================================

    /// Return the number of currently visible children.
sal_Int32 SAL_CALL
    ScAccessibleDocument::getAccessibleChildCount(void)
    throw (uno::RuntimeException)
{
    ScUnoGuard aGuard;
    sal_Int32 nShapesCount(0);
    if (mpChildrenShapes)
        nShapesCount = mpChildrenShapes->GetCount();
    return nShapesCount + 1;
}

    /// Return the specified child or NULL if index is invalid.
uno::Reference<XAccessible> SAL_CALL
    ScAccessibleDocument::getAccessibleChild(sal_Int32 nIndex)
    throw (uno::RuntimeException,
        lang::IndexOutOfBoundsException)
{
    ScUnoGuard aGuard;
    uno::Reference<XAccessible> xAccessible;// = GetChild(nIndex);
    if (!xAccessible.is())
    {
        if (nIndex == 0)
            xAccessible = GetAccessibleSpreadsheet();
        else if(mpChildrenShapes)
            mpChildrenShapes->Get(nIndex - 1); // decrement childindex, because the shapes list starts at 0
    }
    return xAccessible;
}

    /// Return the set of current states.
uno::Reference<XAccessibleStateSet> SAL_CALL
    ScAccessibleDocument::getAccessibleStateSet(void)
    throw (uno::RuntimeException)
{
    ScUnoGuard aGuard;
    uno::Reference<XAccessibleStateSet> xParentStates;
    if (getAccessibleParent().is())
    {
        uno::Reference<XAccessibleContext> xParentContext = getAccessibleParent()->getAccessibleContext();
        xParentStates = xParentContext->getAccessibleStateSet();
    }
    utl::AccessibleStateSetHelper* pStateSet = new utl::AccessibleStateSetHelper();
    if (IsDefunc(xParentStates))
        pStateSet->AddState(AccessibleStateType::DEFUNC);
    if (IsEditable(xParentStates))
        pStateSet->AddState(AccessibleStateType::EDITABLE);
    pStateSet->AddState(AccessibleStateType::ENABLED);
    pStateSet->AddState(AccessibleStateType::OPAQUE);
    if (isShowing())
        pStateSet->AddState(AccessibleStateType::SHOWING);
    if (isVisible())
        pStateSet->AddState(AccessibleStateType::VISIBLE);
    return pStateSet;
}

    ///=====  XAccessibleSelection  ===========================================

void SAL_CALL
    ScAccessibleDocument::selectAccessibleChild( sal_Int32 nChildIndex )
        throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    ScUnoGuard aGuard;
    if (nChildIndex == 0)
    {
        if (mpViewShell)
            mpViewShell->SelectAll();
    }
    else if (nChildIndex > 0)
    {
        sal_Bool bWasTableSelected(IsTableSelected());
        --nChildIndex; // decrement childindex, because the shapes list starts at 0

        if (mpChildrenShapes)
            mpChildrenShapes->Select(nChildIndex);

        if (bWasTableSelected)
            mpViewShell->SelectAll();
    }
    else
        throw lang::IndexOutOfBoundsException();
}

sal_Bool SAL_CALL
    ScAccessibleDocument::isAccessibleChildSelected( sal_Int32 nChildIndex )
        throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    ScUnoGuard aGuard;
    sal_Bool bResult(sal_False);

    if (nChildIndex == 0)
        bResult = IsTableSelected();
    else if (nChildIndex > 0)
    {
        --nChildIndex; // decrement childindex, because the shapes list starts at 0
        uno::Reference<drawing::XShape> xShape;
        bResult = (mpChildrenShapes && mpChildrenShapes->IsSelected(nChildIndex, xShape));
    }
    else
        throw lang::IndexOutOfBoundsException();
    return bResult;
}

void SAL_CALL
    ScAccessibleDocument::clearAccessibleSelection(  )
        throw (uno::RuntimeException)
{
    ScUnoGuard aGuard;

    if (mpChildrenShapes)
        mpChildrenShapes->DeselectAll(); //deselects all (also the table)
}

void SAL_CALL
    ScAccessibleDocument::selectAllAccessible(  )
        throw (uno::RuntimeException)
{
    ScUnoGuard aGuard;

    if (mpChildrenShapes)
        mpChildrenShapes->SelectAll();

    // select table after shapes, because while selecting shapes the table will be deselected
    if (mpViewShell)
    {
        mpViewShell->SelectAll();
    }
}

sal_Int32 SAL_CALL
    ScAccessibleDocument::getSelectedAccessibleChildCount(  )
        throw (uno::RuntimeException)
{
    ScUnoGuard aGuard;
    sal_Int32 nCount(0);

    if (mpChildrenShapes)
        nCount = mpChildrenShapes->GetSelectedCount();

    if (IsTableSelected())
        ++nCount;

    return nCount;
}

uno::Reference<XAccessible > SAL_CALL
    ScAccessibleDocument::getSelectedAccessibleChild( sal_Int32 nSelectedChildIndex )
        throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    ScUnoGuard aGuard;
    uno::Reference<XAccessible> xAccessible;
    sal_Bool bTabMarked(IsTableSelected());

    if ((nSelectedChildIndex == 0) && bTabMarked)
        xAccessible = GetAccessibleSpreadsheet();
    else
    {
        if (bTabMarked)
            --nSelectedChildIndex;

        if (mpChildrenShapes)
            xAccessible = mpChildrenShapes->GetSelected(nSelectedChildIndex);
    }

    return xAccessible;
}

void SAL_CALL
    ScAccessibleDocument::deselectSelectedAccessibleChild( sal_Int32 nChildIndex )
        throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    ScUnoGuard aGuard;

    sal_Bool bTabMarked(IsTableSelected());

    if ((nChildIndex == 0) && bTabMarked)
        mpViewShell->Unmark();
    else if (nChildIndex >= 0)
    {
        if (bTabMarked)
            --nChildIndex;

        if (mpChildrenShapes)
            mpChildrenShapes->Deselect(nChildIndex);

        if (bTabMarked)
            mpViewShell->SelectAll();
    }
    else
        throw lang::IndexOutOfBoundsException();
}

    //=====  XServiceInfo  ====================================================

::rtl::OUString SAL_CALL
    ScAccessibleDocument::getImplementationName(void)
    throw (uno::RuntimeException)
{
    return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM ("ScAccessibleDocument"));
}

uno::Sequence< ::rtl::OUString> SAL_CALL
    ScAccessibleDocument::getSupportedServiceNames(void)
        throw (uno::RuntimeException)
{
    uno::Sequence< ::rtl::OUString > aSequence = ScAccessibleContextBase::getSupportedServiceNames();
    sal_Int32 nOldSize(aSequence.getLength());
    aSequence.realloc(nOldSize + 1);
    ::rtl::OUString* pNames = aSequence.getArray();

    pNames[nOldSize] = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("drafts.com.sun.star.AccessibleSpreadsheetDocumentView"));

    return aSequence;
}

//=====  XTypeProvider  =======================================================

uno::Sequence< uno::Type > SAL_CALL ScAccessibleDocument::getTypes()
        throw (uno::RuntimeException)
{
    return comphelper::concatSequences(ScAccessibleDocumentImpl::getTypes(), ScAccessibleContextBase::getTypes());
}

uno::Sequence<sal_Int8> SAL_CALL
    ScAccessibleDocument::getImplementationId(void)
    throw (uno::RuntimeException)
{
    ScUnoGuard aGuard;
    static uno::Sequence<sal_Int8> aId;
    if (aId.getLength() == 0)
    {
        aId.realloc (16);
        rtl_createUuid (reinterpret_cast<sal_uInt8 *>(aId.getArray()), 0, sal_True);
    }
    return aId;
}

    //=====  internal  ========================================================

::rtl::OUString SAL_CALL
    ScAccessibleDocument::createAccessibleDescription(void)
    throw (uno::RuntimeException)
{
    return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM ("This is a view of a Spreadsheet Document."));
}

::rtl::OUString SAL_CALL
    ScAccessibleDocument::createAccessibleName(void)
    throw (uno::RuntimeException)
{
    ScUnoGuard aGuard;
    rtl::OUString sName(RTL_CONSTASCII_USTRINGPARAM ("Spreadsheet Document View "));
    sal_Int32 nNumber(sal_Int32(meSplitPos) + 1);
    sName += rtl::OUString::valueOf(nNumber);
    return sName;
}

Rectangle ScAccessibleDocument::GetBoundingBoxOnScreen()
    throw (uno::RuntimeException)
{
    Rectangle aRect;
    if (mpViewShell)
    {
        Window* pWindow = mpViewShell->GetWindowByPos(meSplitPos);
        if (pWindow)
            aRect = pWindow->GetWindowExtentsRelative(NULL);
    }
    return aRect;
}

Rectangle ScAccessibleDocument::GetBoundingBox()
    throw (uno::RuntimeException)
{
    Rectangle aRect;
    if (mpViewShell)
    {
        Window* pWindow = mpViewShell->GetWindowByPos(meSplitPos);
        if (pWindow)
            aRect = pWindow->GetWindowExtentsRelative(pWindow->GetAccessibleParentWindow());
    }
    return aRect;
}

sal_uInt16 ScAccessibleDocument::getVisibleTable() const
{
    sal_uInt16 nVisibleTable(0);
    if (mpViewShell && mpViewShell->GetViewData())
        nVisibleTable = mpViewShell->GetViewData()->GetTabNo();
    return nVisibleTable;
}

uno::Reference < XAccessible >
    ScAccessibleDocument::GetAccessibleSpreadsheet()
{
    if (!mpAccessibleSpreadsheet && mpViewShell)
    {
        mpAccessibleSpreadsheet = new ScAccessibleSpreadsheet(this, mpViewShell, getVisibleTable(), meSplitPos);
        mpAccessibleSpreadsheet->acquire();
        mpAccessibleSpreadsheet->Init();
        mbCompleteSheetSelected = IsTableSelected();
    }
    return mpAccessibleSpreadsheet;
}

void ScAccessibleDocument::FreeAccessibleSpreadsheet()
{
    if (mpAccessibleSpreadsheet)
    {
        mpAccessibleSpreadsheet->dispose();
        mpAccessibleSpreadsheet->release();
        mpAccessibleSpreadsheet = NULL;
    }
}

sal_Bool ScAccessibleDocument::IsTableSelected() const
{
    sal_Bool bResult (sal_False);
    if(mpViewShell)
    {
        sal_uInt16 nTab(getVisibleTable());
        mpViewShell->GetViewData()->GetMarkData().MarkToMulti();
        if (mpViewShell->GetViewData()->GetMarkData().IsAllMarked(ScRange(ScAddress(0, 0, nTab),ScAddress(MAXCOL, MAXROW, nTab))))
            bResult = sal_True;
    }
    return bResult;
}

sal_Bool ScAccessibleDocument::IsDefunc(
    const uno::Reference<XAccessibleStateSet>& rxParentStates)
{
    return ScAccessibleContextBase::IsDefunc() || (mpViewShell == NULL) || !getAccessibleParent().is() ||
        (rxParentStates.is() && rxParentStates->contains(AccessibleStateType::DEFUNC));
}

sal_Bool ScAccessibleDocument::IsEditable(
    const uno::Reference<XAccessibleStateSet>& rxParentStates)
{
    // what is with document protection or readonly documents?
    return sal_True;
}

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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#include <svx/svdedtv.hxx>
#include <math.h>
#ifndef _MATH_H
#define _MATH_H
#endif
#include <tools/bigint.hxx>
#include <svl/itemiter.hxx>
#include <vcl/msgbox.hxx>
#include <svx/rectenum.hxx>
#include <svx/svxids.hrc>   // fuer SID_ATTR_TRANSFORM_...
#include <svx/svdattr.hxx>  // fuer Get/SetGeoAttr
#include <svx/svditer.hxx>
#include <svx/svditext.hxx>
#include <svx/svdtrans.hxx>
#include <svx/svdundo.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdlayer.hxx> // fuer MergeNotPersistAttr
#include <svx/svdattrx.hxx> // fuer MergeNotPersistAttr
#include <svx/svdetc.hxx>   // fuer SearchOutlinerItems
#include <svx/svdopath.hxx>  // fuer Crook
#include "svx/svdstr.hrc"   // Namen aus der Resource
#include "svx/svdglob.hxx"  // StringCache
#include <editeng/eeitem.hxx>
#include <svl/aeitem.hxx>
#include <svl/whiter.hxx>
#include <svx/sdr/contact/objectcontact.hxx>
#include <svx/sdr/contact/viewcontact.hxx>
#include <svx/e3dsceneupdater.hxx>
#include <svx/obj3d.hxx>
#include <svx/svdlegacy.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <svx/AffineMatrixItem.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <svx/xlnwtit.hxx>
#include <svx/xlnstwit.hxx>
#include <svx/xlnedwit.hxx>

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrEditView::SetMarkedObjSnapRange(const basegfx::B2DRange& rRange, bool bCopy)
{
    if(!rRange.isEmpty() && areSdrObjectsSelected())
    {
        SdrObjectVector aSelection(getSelectedSdrObjectVectorFromSdrMarkView());
        const basegfx::B2DRange& rCurrentRange(getMarkedObjectSnapRange());

        if(!rCurrentRange.equal(rRange))
        {
            String aStr;
            TakeMarkedDescriptionString(STR_EditPosSize, aStr);

            if(bCopy)
            {
                aStr += ImpGetResStr(STR_EditWithCopy);
            }

            const bool bUndo(IsUndoEnabled());

            if(bUndo)
            {
                BegUndo(aStr);
            }

            if(bCopy)
            {
                CopyMarkedObj();
                aSelection = getSelectedSdrObjectVectorFromSdrMarkView();
            }

            basegfx::B2DHomMatrix aTransformation(
                basegfx::tools::createScaleTranslateB2DHomMatrix(
                    rCurrentRange.getRange(),
                    rCurrentRange.getMinimum()));
            aTransformation.invert();
            aTransformation = basegfx::tools::createScaleTranslateB2DHomMatrix(
                rRange.getRange(),
                rRange.getMinimum()) * aTransformation;

            for(sal_uInt32 nm(0); nm < aSelection.size(); nm++)
            {
                SdrObject* pO = aSelection[nm];

                if(bUndo)
                {
                    AddUndo( getSdrModelFromSdrView().GetSdrUndoFactory().CreateUndoGeoObject(*pO));
                }

                sdr::legacy::transformSdrObject(*pO, aTransformation);
            }

            if(bUndo)
            {
                EndUndo();
            }
        }
    }
}

std::vector< SdrUndoAction* > SdrEditView::CreateConnectorUndo(SdrObject& rO)
{
    std::vector< SdrUndoAction* > vUndoActions;
    ::std::vector< SdrEdgeObj* > aConnectedEdges(rO.getAllConnectedSdrEdgeObj());

    if(!aConnectedEdges.empty())
    {
        for(sal_uInt32 a(0); a < aConnectedEdges.size(); a++)
        {
            vUndoActions.push_back(
                getSdrModelFromSdrView().GetSdrUndoFactory().CreateUndoGeoObject(*aConnectedEdges[a]));
        }
    }

    return vUndoActions;
}

void SdrEditView::AddUndoActions( std::vector< SdrUndoAction* >& rUndoActions )
{
    std::vector< SdrUndoAction* >::iterator aUndoActionIter( rUndoActions.begin() );

    while( aUndoActionIter != rUndoActions.end() )
    {
        AddUndo( *aUndoActionIter++ );
    }
}

void SdrEditView::MoveMarkedObj(const basegfx::B2DVector& rOffset, bool bCopy)
{
    if(areSdrObjectsSelected() && !rOffset.equalZero())
    {
        const bool bUndo(IsUndoEnabled());

        if(bUndo)
        {
            XubString aStr(ImpGetResStr(STR_EditMove));

            if(bCopy)
            {
                aStr += ImpGetResStr(STR_EditWithCopy);
            }

            // benoetigt eigene UndoGroup wegen Parameter
            BegUndo(aStr, getSelectionDescription(getSelectedSdrObjectVectorFromSdrMarkView()), SDRREPFUNC_OBJ_MOVE);
        }

        if(bCopy)
        {
            CopyMarkedObj();
        }

        const SdrObjectVector aSelection(getSelectedSdrObjectVectorFromSdrMarkView());
        basegfx::B2DHomMatrix aTransformation(basegfx::tools::createTranslateB2DHomMatrix(rOffset));

        for(sal_uInt32 nm(0); nm < aSelection.size(); nm++)
        {
            SdrObject* pO = aSelection[nm];

            if(bUndo)
            {
                std::vector< SdrUndoAction* > vConnectorUndoActions( CreateConnectorUndo( *pO ) );
                AddUndoActions( vConnectorUndoActions );
                AddUndo(getSdrModelFromSdrView().GetSdrUndoFactory().CreateUndoGeoObject(*pO));
            }

            sdr::legacy::transformSdrObject(*pO, aTransformation);
        }

        if(bUndo)
        {
            EndUndo();
        }
    }
}

void SdrEditView::ResizeMarkedObj(const basegfx::B2DPoint& rRefPoint, const basegfx::B2DTuple& rScale, bool bCopy)
{
    if(areSdrObjectsSelected() && !rScale.equalZero())
    {
        const bool bUndo(IsUndoEnabled());

        if(bUndo)
        {
            XubString aStr;

            TakeMarkedDescriptionString(STR_EditResize,aStr);

            if(bCopy)
            {
                aStr += ImpGetResStr(STR_EditWithCopy);
            }

            BegUndo(aStr);
        }

        if(bCopy)
        {
            CopyMarkedObj();
        }

        basegfx::B2DHomMatrix aTransformation;

        if(rRefPoint.equalZero())
        {
            aTransformation = basegfx::tools::createScaleB2DHomMatrix(rScale);
        }
        else
        {
            aTransformation.translate(-rRefPoint);
            aTransformation.scale(rScale);
            aTransformation.translate(rRefPoint);
        }

        const SdrObjectVector aSelection(getSelectedSdrObjectVectorFromSdrMarkView());

        for(sal_uInt32 nm(0); nm < aSelection.size(); nm++)
        {
            SdrObject* pO = aSelection[nm];

            if(bUndo)
            {
                std::vector< SdrUndoAction* > vConnectorUndoActions( CreateConnectorUndo( *pO ) );
                AddUndoActions( vConnectorUndoActions );
                AddUndo( getSdrModelFromSdrView().GetSdrUndoFactory().CreateUndoGeoObject(*pO));
            }

            // get transformation and correct to minimal scaling for zero-width/height objects
            basegfx::B2DHomMatrix aObjectMatrix(basegfx::tools::guaranteeMinimalScaling(pO->getSdrObjectTransformation()));

            // apply current transformation and set
            aObjectMatrix = aTransformation * aObjectMatrix;
            pO->setSdrObjectTransformation(aObjectMatrix);
        }

        if(bUndo)
        {
            EndUndo();
        }
    }
}

double SdrEditView::GetMarkedObjRotate() const
{
    SdrObject* pCandidate = getSelectedIfSingle();

    if(!pCandidate)
    {
        // for multiselection use the rotation angle of the 1st object
        const SdrObjectVector aSelection(getSelectedSdrObjectVectorFromSdrMarkView());

        if(aSelection.size())
        {
            pCandidate = aSelection[0];
        }
    }

    if(pCandidate)
    {
        return pCandidate->getSdrObjectRotate();
    }

    return 0.0;
}

void SdrEditView::RotateMarkedObj(const basegfx::B2DPoint& rRefPoint, double fAngle, bool bCopy)
{
    if(areSdrObjectsSelected() && (bCopy || !basegfx::fTools::equalZero(fAngle)))
    {
        const bool bUndo(IsUndoEnabled());

        if(bUndo)
        {
            XubString aStr;

            TakeMarkedDescriptionString(STR_EditRotate,aStr);

            if(bCopy)
            {
                aStr += ImpGetResStr(STR_EditWithCopy);
            }

            BegUndo(aStr);
        }

        if(bCopy)
        {
            CopyMarkedObj();
        }

        std::vector< E3DModifySceneSnapRectUpdater* > aUpdaters;
        basegfx::B2DHomMatrix aTransformation;

        if(rRefPoint.equalZero())
        {
            aTransformation = basegfx::tools::createRotateB2DHomMatrix(fAngle);
        }
        else
        {
            aTransformation.translate(-rRefPoint);
            aTransformation.rotate(fAngle);
            aTransformation.translate(rRefPoint);
        }

        const SdrObjectVector aSelection(getSelectedSdrObjectVectorFromSdrMarkView());

        for(sal_uInt32 nm(0); nm < aSelection.size(); nm++)
        {
            SdrObject* pO = aSelection[nm];

            if(bUndo)
            {
                // extra undo actions for changed connector which now may hold it's layouted path (SJ)
                std::vector< SdrUndoAction* > vConnectorUndoActions( CreateConnectorUndo( *pO ) );
                AddUndoActions( vConnectorUndoActions );
                AddUndo(getSdrModelFromSdrView().GetSdrUndoFactory().CreateUndoGeoObject(*pO));
            }

            // set up a scene updater if object is a 3d object
            if(dynamic_cast< E3dObject* >(pO))
            {
                aUpdaters.push_back(new E3DModifySceneSnapRectUpdater(pO));
            }

            sdr::legacy::transformSdrObject(*pO, aTransformation);
        }

        // fire scene updaters
        while(aUpdaters.size())
        {
            delete aUpdaters.back();
            aUpdaters.pop_back();
        }

        if(bUndo)
        {
            EndUndo();
        }
    }
}

void SdrEditView::MirrorMarkedObj(const basegfx::B2DPoint& rRef1, const basegfx::B2DPoint& rRef2, bool bCopy)
{
    if(areSdrObjectsSelected() && !rRef1.equal(rRef2))
    {
        const bool bUndo(IsUndoEnabled());
        const basegfx::B2DVector aMirrorEdge(rRef2 - rRef1);

        if(bUndo)
        {
            XubString aStr;

            if(basegfx::fTools::equalZero(aMirrorEdge.getX()))
            {
                TakeMarkedDescriptionString(STR_EditMirrorHori,aStr);
            }
            else if(basegfx::fTools::equalZero(aMirrorEdge.getY()))
            {
                TakeMarkedDescriptionString(STR_EditMirrorVert,aStr);
            }
            else if(basegfx::fTools::equal(fabs(aMirrorEdge.getX()), fabs(aMirrorEdge.getY())))
            {
                TakeMarkedDescriptionString(STR_EditMirrorDiag,aStr);
            }
            else
            {
                TakeMarkedDescriptionString(STR_EditMirrorFree,aStr);
            }

            if(bCopy)
            {
                aStr += ImpGetResStr(STR_EditWithCopy);
            }

            BegUndo(aStr);
        }

        if (bCopy)
        {
            CopyMarkedObj();
        }

        const double fAngleToXAxis(atan2(aMirrorEdge.getY(), aMirrorEdge.getX()));
        basegfx::B2DHomMatrix aTransformation;

        aTransformation.translate(-rRef1);
        aTransformation.rotate(-fAngleToXAxis);
        aTransformation.scale(1.0, -1.0);
        aTransformation.rotate(fAngleToXAxis);
        aTransformation.translate(rRef1);

        std::vector< E3DModifySceneSnapRectUpdater* > aUpdaters;
        const SdrObjectVector aSelection(getSelectedSdrObjectVectorFromSdrMarkView());

        for(sal_uInt32 nm(0); nm < aSelection.size(); nm++)
        {
            SdrObject* pO = aSelection[nm];

            if( bUndo )
            {
                // extra undo actions for changed connector which now may hold it's layouted path (SJ)
                std::vector< SdrUndoAction* > vConnectorUndoActions( CreateConnectorUndo( *pO ) );
                AddUndoActions( vConnectorUndoActions );

                AddUndo( getSdrModelFromSdrView().GetSdrUndoFactory().CreateUndoGeoObject(*pO));
            }

            // set up a scene updater if object is a 3d object
            if(dynamic_cast< E3dObject* >(pO))
            {
                aUpdaters.push_back(new E3DModifySceneSnapRectUpdater(pO));
            }

            sdr::legacy::transformSdrObject(*pO, aTransformation);
        }

        // fire scene updaters
        while(aUpdaters.size())
        {
            delete aUpdaters.back();
            aUpdaters.pop_back();
        }

        if(bUndo)
        {
            EndUndo();
        }
    }
}

void SdrEditView::MirrorMarkedObjHorizontal(bool bCopy)
{
    if(areSdrObjectsSelected())
    {
        const basegfx::B2DRange& rCurrentRange(getMarkedObjectSnapRange());

        MirrorMarkedObj(
            basegfx::B2DPoint(rCurrentRange.getCenterX(), rCurrentRange.getMinY()),
            basegfx::B2DPoint(rCurrentRange.getCenterX(), rCurrentRange.getMaxY()),
            bCopy);
    }
}

void SdrEditView::MirrorMarkedObjVertical(bool bCopy)
{
    if(areSdrObjectsSelected())
    {
        const basegfx::B2DRange& rCurrentRange(getMarkedObjectSnapRange());

        MirrorMarkedObj(
            basegfx::B2DPoint(rCurrentRange.getMinX(), rCurrentRange.getCenterY()),
            basegfx::B2DPoint(rCurrentRange.getMaxX(), rCurrentRange.getCenterY()),
            bCopy);
    }
}

double SdrEditView::GetMarkedObjShearX() const
{
    SdrObject* pCandidate = getSelectedIfSingle();

    if(!pCandidate)
    {
        // for multiselection use the shear value of the 1st object
        const SdrObjectVector aSelection(getSelectedSdrObjectVectorFromSdrMarkView());

        if(aSelection.size())
        {
            pCandidate = aSelection[0];
        }
    }

    if(pCandidate)
    {
        OSL_ENSURE(
            basegfx::fTools::less(atan(pCandidate->getSdrObjectShearX()), F_PI2 * (89.0/90.0)) &&
            basegfx::fTools::more(atan(pCandidate->getSdrObjectShearX()), -F_PI2 * (89.0/90.0)),
            "Shear angle is out of bounds (inside the one degree extrema corresponding to a +/- 90 degree range)");

        return pCandidate->getSdrObjectShearX();
    }

    return 0.0;
}

void SdrEditView::ShearMarkedObj(const basegfx::B2DPoint& rRefPoint, double fAngle, bool bVShear, bool bCopy)
{
    if(areSdrObjectsSelected() && !basegfx::fTools::equalZero(fAngle))
    {
        const bool bUndo(IsUndoEnabled());

        if(bUndo)
        {
            XubString aStr;

            TakeMarkedDescriptionString(STR_EditShear,aStr);

            if(bCopy)
            {
                aStr += ImpGetResStr(STR_EditWithCopy);
            }

            BegUndo(aStr);
        }

        if(bCopy)
        {
            CopyMarkedObj();
        }

        basegfx::B2DHomMatrix aTransformation;

        if(rRefPoint.equalZero())
        {
            if(bVShear)
            {
                aTransformation = basegfx::tools::createShearYB2DHomMatrix(tan(fAngle));
            }
            else
            {
                aTransformation = basegfx::tools::createShearXB2DHomMatrix(tan(fAngle));
            }
        }
        else
        {
            aTransformation.translate(-rRefPoint);

            if(bVShear)
            {
                aTransformation.shearY(tan(fAngle));
            }
            else
            {
                aTransformation.shearX(tan(fAngle));
            }

            aTransformation.translate(rRefPoint);
        }

        const SdrObjectVector aSelection(getSelectedSdrObjectVectorFromSdrMarkView());

        for(sal_uInt32 nm(0); nm < aSelection.size(); nm++)
        {
            SdrObject* pO = aSelection[nm];

            if(bUndo)
            {
                std::vector< SdrUndoAction* > vConnectorUndoActions( CreateConnectorUndo( *pO ) );
                AddUndoActions( vConnectorUndoActions );
                AddUndo( getSdrModelFromSdrView().GetSdrUndoFactory().CreateUndoGeoObject(*pO));
            }

            sdr::legacy::transformSdrObject(*pO, aTransformation);
        }

        if(bUndo)
        {
            EndUndo();
        }
    }
}

void SdrEditView::ImpCrookObj(SdrObject& rO, const basegfx::B2DPoint& rRef, const basegfx::B2DPoint& rRad,
    SdrCrookMode eMode, bool bVertical, bool bNoContortion, bool bRotate, const basegfx::B2DRange& rMarkRange)
{
    SdrPathObj* pPath = dynamic_cast< SdrPathObj* >(&rO);
    bool bDone(false);

    if(pPath && !bNoContortion)
    {
        basegfx::B2DPolyPolygon aPolyPolygon(pPath->getB2DPolyPolygonInObjectCoordinates());

        switch(eMode)
        {
            case SDRCROOK_ROTATE : CrookRotatePoly(aPolyPolygon, rRef, rRad, bVertical); break;
            case SDRCROOK_SLANT : CrookSlantPoly(aPolyPolygon, rRef, rRad, bVertical); break;
            case SDRCROOK_STRETCH: CrookStretchPoly(aPolyPolygon, rRef, rRad, bVertical, rMarkRange); break;
        }

        pPath->setB2DPolyPolygonInObjectCoordinates(aPolyPolygon);
        bDone = true;
    }

    if(!bDone && !pPath && rO.IsPolygonObject() && rO.GetObjectPointCount())
    {
        // FuerPolyObj's, aber NICHT fuer SdrPathObj's, z.B. fuer's Bemassungsobjekt
        const sal_uInt32 nPtAnz(rO.GetObjectPointCount());
        basegfx::B2DPolygon aPolygon;
        sal_uInt32 nPtNum;

        for(nPtNum = 0; nPtNum < nPtAnz; nPtNum++)
        {
            aPolygon.append(rO.GetObjectPoint(nPtNum));
        }

        switch (eMode)
        {
            case SDRCROOK_ROTATE : CrookRotatePoly(aPolygon, rRef, rRad, bVertical); break;
            case SDRCROOK_SLANT : CrookSlantPoly(aPolygon, rRef, rRad, bVertical); break;
            case SDRCROOK_STRETCH: CrookStretchPoly(aPolygon, rRef, rRad, bVertical, rMarkRange); break;
        }

        for(nPtNum = 0; nPtNum < nPtAnz; nPtNum++)
        {
            rO.SetObjectPoint(aPolygon.getB2DPoint(nPtNum), nPtNum);
        }

        bDone = true;
    }

    if(!bDone)
    {
        // Fuer alle anderen oder wenn bNoContortion
        const basegfx::B2DPoint aCtr0(sdr::legacy::GetSnapRange(rO).getCenter());
        basegfx::B2DPoint aCtr1(aCtr0);
        bool bRotOk(false);
        double nSin(0.0), nCos(1.0);
        double fAngle(0.0);

        if(!rRad.equalZero())
        {
            bRotOk = bRotate;

            switch (eMode)
            {
                case SDRCROOK_ROTATE : fAngle = CrookRotateXPoint(aCtr1, 0, 0, rRef, rRad, nSin, nCos, bVertical); bRotOk = bRotate; break;
                case SDRCROOK_SLANT : fAngle = CrookSlantXPoint(aCtr1, 0, 0, rRef, rRad, nSin, nCos, bVertical); break;
                case SDRCROOK_STRETCH: fAngle = CrookStretchXPoint(aCtr1, 0, 0, rRef, rRad, nSin, nCos, bVertical, rMarkRange); break;
            }
        }

        basegfx::B2DHomMatrix aObjectTransform;

        if(bRotOk)
        {
            aObjectTransform.translate(-aCtr0);
            aObjectTransform.rotate(fAngle);
            aObjectTransform.translate(aCtr0);
        }

        aObjectTransform.translate(aCtr1 - aCtr0);

        sdr::legacy::transformSdrObject(rO, aObjectTransform);
    }
}

void SdrEditView::CrookMarkedObj(const basegfx::B2DPoint& rRef, const basegfx::B2DPoint& rRad, SdrCrookMode eMode,
    bool bVertical, bool bNoContortion, bool bCopy)
{
    if(areSdrObjectsSelected())
    {
        SdrObjectVector aSelection(getSelectedSdrObjectVectorFromSdrMarkView());
        const basegfx::B2DRange& rMarkRange(getMarkedObjectSnapRange());
        const bool bUndo(IsUndoEnabled());
        const bool bRotate(bNoContortion && SDRCROOK_ROTATE == eMode && IsRotateAllowed(false));

        if(bUndo)
        {
            XubString aStr;

            TakeMarkedDescriptionString(bNoContortion ? STR_EditCrook : STR_EditCrookContortion, aStr);

            if(bCopy)
            {
                aStr += ImpGetResStr(STR_EditWithCopy);
            }

            BegUndo(aStr);
        }

        if(bCopy)
        {
            CopyMarkedObj();
            aSelection = getSelectedSdrObjectVectorFromSdrMarkView();
        }

        for(sal_uInt32 nm(0); nm < aSelection.size(); nm++)
        {
            SdrObject* pO = aSelection[nm];

            if(bUndo)
            {
                AddUndo(getSdrModelFromSdrView().GetSdrUndoFactory().CreateUndoGeoObject(*pO));
            }

            const SdrObjList* pOL = pO->getChildrenOfSdrObject();

            if(bNoContortion || !pOL)
            {
                ImpCrookObj(*pO, rRef, rRad, eMode, bVertical, bNoContortion, bRotate, rMarkRange);
            }
            else
            {
                SdrObjListIter aIter(*pOL, IM_DEEPNOGROUPS);

                while(aIter.IsMore())
                {
                    SdrObject* pO1 = aIter.Next();

                    ImpCrookObj(*pO1, rRef, rRad, eMode, bVertical, bNoContortion, bRotate, rMarkRange);
                }
            }
        }

        if(bUndo)
        {
            EndUndo();
        }
    }
}

void SdrEditView::ImpDistortObj(SdrObject& rO, const basegfx::B2DRange& rRefRange, const basegfx::B2DPolygon& rDistortedPolygon, bool bNoContortion)
{
    if(rDistortedPolygon.count() > 3)
    {
        SdrPathObj* pPath = dynamic_cast< SdrPathObj* >(&rO);

        if(!bNoContortion && pPath)
        {
            const basegfx::B2DPolyPolygon aPolyPolygon(
                basegfx::tools::distort(
                    pPath->getB2DPolyPolygonInObjectCoordinates(),
                    rRefRange,
                    rDistortedPolygon.getB2DPoint(0),   // TopLeft
                    rDistortedPolygon.getB2DPoint(1),   // rTopRight
                    rDistortedPolygon.getB2DPoint(3),   // rBottomLeft
                    rDistortedPolygon.getB2DPoint(2))); // rBottomRight

            pPath->setB2DPolyPolygonInObjectCoordinates(aPolyPolygon);
        }
        else if(rO.IsPolygonObject())
        {
            // z.B. fuer's Bemassungsobjekt
            const sal_uInt32 nPtAnz(rO.GetObjectPointCount());
            basegfx::B2DPolygon aPolygon;

            for(sal_uInt32 a(0); a < nPtAnz; a++)
            {
                aPolygon.append(rO.GetObjectPoint(a));
            }

            aPolygon = basegfx::tools::distort(
                aPolygon,
                rRefRange,
                rDistortedPolygon.getB2DPoint(0),  // TopLeft
                rDistortedPolygon.getB2DPoint(1),  // rTopRight
                rDistortedPolygon.getB2DPoint(3),  // rBottomLeft
                rDistortedPolygon.getB2DPoint(2)); // rBottomRight

            for(sal_uInt32 b(0); b < nPtAnz; b++)
            {
                rO.SetObjectPoint(aPolygon.getB2DPoint(b), b);
            }
        }
    }
    else
    {
        OSL_ENSURE(false, "SdrEditView::ImpDistortObj: DistortedPolygon needs to have four corner points (!)");
    }
}

void SdrEditView::DistortMarkedObj(const basegfx::B2DRange& rRefRange, const basegfx::B2DPolygon& rDistortedRect, bool bNoContortion, bool bCopy)
{
    if(areSdrObjectsSelected())
    {
        const bool bUndo(IsUndoEnabled());

        if(bUndo)
        {
            XubString aStr;

            TakeMarkedDescriptionString(STR_EditDistort, aStr);

            if(bCopy)
            {
                aStr += ImpGetResStr(STR_EditWithCopy);
            }

            BegUndo(aStr);
        }

        if(bCopy)
        {
            CopyMarkedObj();
        }

        const SdrObjectVector aSelection(getSelectedSdrObjectVectorFromSdrMarkView());

        for(sal_uInt32 nm(0); nm < aSelection.size(); nm++)
        {
            SdrObject* pO = aSelection[nm];

            if(bUndo)
            {
                AddUndo(getSdrModelFromSdrView().GetSdrUndoFactory().CreateUndoGeoObject(*pO));
            }

            const SdrObjList* pOL = pO->getChildrenOfSdrObject();

            if(bNoContortion || !pOL)
            {
                ImpDistortObj(*pO, rRefRange, rDistortedRect, bNoContortion);
            }
            else
            {
                SdrObjListIter aIter(*pOL, IM_DEEPNOGROUPS);

                while(aIter.IsMore())
                {
                    SdrObject* pO1 = aIter.Next();

                    ImpDistortObj(*pO1, rRefRange, rDistortedRect, bNoContortion);
                }
            }
        }

        if(bUndo)
        {
            EndUndo();
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void SdrEditView::SetNotPersistAttrToMarked(const SfxItemSet& rAttr, bool /*bReplaceAll*/)
{
    if(areSdrObjectsSelected())
    {
        const SdrObjectVector aSelection(getSelectedSdrObjectVectorFromSdrMarkView());
        const basegfx::B2DRange& rAllSnapRange(getMarkedObjectSnapRange());
        const SfxPoolItem *pPoolItem = 0;
        const bool bUndo(IsUndoEnabled());

        basegfx::B2DPoint aRef1(GetRef1());
        basegfx::B2DPoint aRef2(GetRef2());

        if (rAttr.GetItemState(SDRATTR_TRANSFORMREF1X,true,&pPoolItem)==SFX_ITEM_SET)
        {
            aRef1.setX(((const SdrMetricItem*)pPoolItem)->GetValue());
        }

        if (rAttr.GetItemState(SDRATTR_TRANSFORMREF1Y,true,&pPoolItem)==SFX_ITEM_SET)
        {
            aRef1.setY(((const SdrMetricItem*)pPoolItem)->GetValue());
        }

        if (rAttr.GetItemState(SDRATTR_TRANSFORMREF2X,true,&pPoolItem)==SFX_ITEM_SET)
        {
            aRef2.setX(((const SdrMetricItem*)pPoolItem)->GetValue());
        }

        if (rAttr.GetItemState(SDRATTR_TRANSFORMREF2Y,true,&pPoolItem)==SFX_ITEM_SET)
        {
            aRef2.setY(((const SdrMetricItem*)pPoolItem)->GetValue());
        }

        if(!aRef1.equal(GetRef1()))
        {
            SetRef1(aRef1);
        }

        if(!aRef2.equal(GetRef2()))
        {
            SetRef2(aRef2);
        }

        double fLeft(rAllSnapRange.getMinX());
        double fTop(rAllSnapRange.getMinY());
        double fWidth(rAllSnapRange.getWidth());
        double fHeight(rAllSnapRange.getHeight());

        if (rAttr.GetItemState(SDRATTR_ALLPOSITIONX,true,&pPoolItem)==SFX_ITEM_SET)
        {
            fLeft = ((const SdrMetricItem*)pPoolItem)->GetValue();
        }

        if (rAttr.GetItemState(SDRATTR_ALLPOSITIONY,true,&pPoolItem)==SFX_ITEM_SET)
        {
            fTop = ((const SdrMetricItem*)pPoolItem)->GetValue();
        }

        if (rAttr.GetItemState(SDRATTR_ALLSIZEWIDTH,true,&pPoolItem)==SFX_ITEM_SET)
        {
            fWidth = ((const SdrMetricItem*)pPoolItem)->GetValue();
        }

        if (rAttr.GetItemState(SDRATTR_ALLSIZEHEIGHT,true,&pPoolItem)==SFX_ITEM_SET)
        {
            fHeight = ((const SdrMetricItem*)pPoolItem)->GetValue();
        }

        const basegfx::B2DRange aNewRange(fLeft, fTop, fLeft + fWidth, fTop + fHeight);

        if(!aNewRange.equal(rAllSnapRange))
        {
            SetMarkedObjSnapRange(aNewRange);
        }

        basegfx::B2DTuple aScale(1.0, 1.0);

        if (rAttr.GetItemState(SDRATTR_RESIZEXALL,true,&pPoolItem)==SFX_ITEM_SET)
        {
            aScale.setX(((const SdrFractionItem*)pPoolItem)->GetValue());
        }

        if (rAttr.GetItemState(SDRATTR_RESIZEYALL,true,&pPoolItem)==SFX_ITEM_SET)
        {
            aScale.setY(((const SdrFractionItem*)pPoolItem)->GetValue());
        }

        if(!aScale.equal(basegfx::B2DTuple(1.0, 1.0)))
        {
            ResizeMarkedObj(rAllSnapRange.getMinimum(), aScale);
        }

        if (rAttr.GetItemState(SDRATTR_ROTATEALL,true,&pPoolItem)==SFX_ITEM_SET)
        {
            const sal_Int32 nAngle(((const SdrAngleItem*)pPoolItem)->GetValue());
            const double fNewAngle(sdr::legacy::convertRotateAngleLegacyToNew(nAngle));

            RotateMarkedObj(rAllSnapRange.getCenter(), fNewAngle);
        }

        if (rAttr.GetItemState(SDRATTR_HORZSHEARALL,true,&pPoolItem)==SFX_ITEM_SET)
        {
            const sal_Int32 nAngle(((const SdrAngleItem*)pPoolItem)->GetValue());
            const double fAngle(sdr::legacy::convertShearAngleXLegacyToNew(nAngle));

            ShearMarkedObj(rAllSnapRange.getCenter(), fAngle, false);
        }

        if (rAttr.GetItemState(SDRATTR_VERTSHEARALL,true,&pPoolItem)==SFX_ITEM_SET)
        {
            const sal_Int32 nAngle(((const SdrAngleItem*)pPoolItem)->GetValue());
            const double fAngle(sdr::legacy::convertShearAngleXLegacyToNew(nAngle));

            ShearMarkedObj(rAllSnapRange.getCenter(), fAngle, true);
        }

        for(sal_uInt32 nm(0); nm < aSelection.size(); nm++)
        {
            SdrObject* pObj = aSelection[nm];

            if(bUndo)
            {
                AddUndo(getSdrModelFromSdrView().GetSdrUndoFactory().CreateUndoGeoObject(*pObj));
            }

            pObj->ApplyNotPersistAttr(rAttr);
        }
    }
}

void SdrEditView::MergeNotPersistAttrFromMarked(SfxItemSet& rAttr) const
{
    if(areSdrObjectsSelected())
    {
        const SdrObjectVector aSelection(getSelectedSdrObjectVectorFromSdrMarkView());
        const Rectangle aAllSnapRect(sdr::legacy::GetAllObjSnapRect(aSelection));
        const sal_Int32 nAllSnapPosX(aAllSnapRect.Left());
        const sal_Int32 nAllSnapPosY(aAllSnapRect.Top());
        const sal_Int32 nAllSnapWdt(aAllSnapRect.GetWidth() - 1);
        const sal_Int32 nAllSnapHgt(aAllSnapRect.GetHeight() - 1);

        // koennte mal zu CheckPossibilities mit rein
        bool bMovProtect(false);
        bool bMovProtectDC(false);
        bool bSizProtect(false);
        bool bSizProtectDC(false);
        bool bPrintable(true);
        bool bPrintableDC(false);
        bool bVisible(true);
        bool bVisibleDC(false);
        SdrLayerID nLayerId(0);
        bool bLayerDC(false);
        XubString aObjName;
        bool bObjNameDC(false);
        bool bObjNameSet(false);

        sal_Int32 nSnapPosX(0);
        sal_Int32 nSnapPosY(0);
        sal_Int32 nSnapWdt(0);
        sal_Int32 nSnapHgt(0);
        sal_Int32 nLogicWdt(0);
        sal_Int32 nLogicHgt(0);
        sal_Int32 nRotAngle(0);
        sal_Int32 nShrAngle(0);
        bool bSnapPosXDC(false);
        bool bSnapPosYDC(false);
        bool bSnapWdtDC(false);
        bool bSnapHgtDC(false);
        bool bLogicWdtDC(false);
        bool bLogicWdtDiff(false);
        bool bLogicHgtDC(false);
        bool bLogicHgtDiff(false);
        bool bRotAngleDC(false);
        bool bShrAngleDC(false);
        Rectangle aSnapRect;
        Rectangle aLogicRect;

        for(sal_uInt32 nm(0); nm < aSelection.size(); nm++)
        {
            const SdrObject* pObj = aSelection[nm];

            if(!nm)
            {
                nLayerId = pObj->GetLayer();
                bMovProtect = pObj->IsMoveProtect();
                bSizProtect = pObj->IsResizeProtect();
                bPrintable  = pObj->IsPrintable();
                bVisible = pObj->IsVisible();
                const Rectangle aSnapRect2(sdr::legacy::GetSnapRect(*pObj));
                const Rectangle aLogicRect2(sdr::legacy::GetLogicRect(*pObj));
                nSnapPosX = aSnapRect2.Left();
                nSnapPosY = aSnapRect2.Top();
                nSnapWdt = aSnapRect2.GetWidth() - 1;
                nSnapHgt = aSnapRect2.GetHeight() - 1;
                nLogicWdt = aLogicRect2.GetWidth() - 1;
                nLogicHgt = aLogicRect2.GetHeight() - 1;
                bLogicWdtDiff = nLogicWdt != nSnapWdt;
                bLogicHgtDiff = nLogicHgt != nSnapHgt;
                nRotAngle = sdr::legacy::GetRotateAngle(*pObj);
                nShrAngle = sdr::legacy::GetShearAngleX(*pObj);
            }
            else
            {
                if(!bLayerDC && nLayerId != pObj->GetLayer())
                {
                    bLayerDC = true;
                }

                if(!bMovProtectDC && bMovProtect != pObj->IsMoveProtect())
                {
                    bMovProtectDC = true;
                }

                if(!bSizProtectDC && bSizProtect != pObj->IsResizeProtect())
                {
                    bSizProtectDC = true;
                }

                if(!bPrintableDC && bPrintable != pObj->IsPrintable())
                {
                    bPrintableDC = true;
                }

                if(!bVisibleDC && bVisible != pObj->IsVisible())
                {
                    bVisibleDC = true;
                }

                if(!bRotAngleDC && nRotAngle != sdr::legacy::GetRotateAngle(*pObj))
                {
                    bRotAngleDC = true;
                }

                if(!bShrAngleDC && nShrAngle != sdr::legacy::GetShearAngleX(*pObj))
                {
                    bShrAngleDC = true;
                }

                if(!bSnapWdtDC || !bSnapHgtDC || !bSnapPosXDC || !bSnapPosYDC || !bLogicWdtDiff || !bLogicHgtDiff)
                {
                    aSnapRect = sdr::legacy::GetSnapRect(*pObj);

                    if(nSnapPosX != aSnapRect.Left())
                    {
                        bSnapPosXDC = true;
                    }

                    if(nSnapPosY != aSnapRect.Top())
                    {
                        bSnapPosYDC = true;
                    }

                    if(nSnapWdt != aSnapRect.GetWidth() - 1)
                    {
                        bSnapWdtDC = true;
                    }

                    if(nSnapHgt != aSnapRect.GetHeight() - 1)
                    {
                        bSnapHgtDC = true;
                    }
                }

                if(!bLogicWdtDC || !bLogicHgtDC || !bLogicWdtDiff || !bLogicHgtDiff)
                {
                    aLogicRect = sdr::legacy::GetLogicRect(*pObj);

                    if(nLogicWdt != aLogicRect.GetWidth() - 1)
                    {
                        bLogicWdtDC = true;
                    }

                    if(nLogicHgt != aLogicRect.GetHeight() - 1)
                    {
                        bLogicHgtDC = true;
                    }

                    if(!bLogicWdtDiff && aSnapRect.GetWidth() != aLogicRect.GetWidth())
                    {
                        bLogicWdtDiff = true;
                    }

                    if(!bLogicHgtDiff && aSnapRect.GetHeight() != aLogicRect.GetHeight())
                    {
                        bLogicHgtDiff = true;
                    }
                }
            }

            if(!bObjNameDC)
            {
                if(!bObjNameSet)
                {
                    aObjName = pObj->GetName();
                }
                else
                {
                    if(aObjName != pObj->GetName())
                    {
                        bObjNameDC = true;
                    }
                }
            }
        }

        if(bSnapPosXDC || nAllSnapPosX != nSnapPosX)
        {
            rAttr.Put(SdrMetricItem(SDRATTR_ALLPOSITIONX, nAllSnapPosX));
        }

        if(bSnapPosYDC || nAllSnapPosY != nSnapPosY)
        {
            rAttr.Put(SdrMetricItem(SDRATTR_ALLPOSITIONY, nAllSnapPosY));
        }

        if(bSnapWdtDC || nAllSnapWdt != nSnapWdt)
        {
            rAttr.Put(SdrMetricItem(SDRATTR_ALLSIZEWIDTH, nAllSnapWdt));
        }

        if(bSnapHgtDC || nAllSnapHgt != nSnapHgt)
        {
            rAttr.Put(SdrMetricItem(SDRATTR_ALLSIZEHEIGHT, nAllSnapHgt));
        }

        // Items fuer reine Transformationen
        rAttr.Put(SdrMetricItem(SDRATTR_MOVEX, 0));
        rAttr.Put(SdrMetricItem(SDRATTR_MOVEY, 0));
        rAttr.Put(SdrFractionItem(SDRATTR_RESIZEXONE, Fraction(1,1)));
        rAttr.Put(SdrFractionItem(SDRATTR_RESIZEYONE, Fraction(1,1)));
        rAttr.Put(SdrAngleItem(SDRATTR_ROTATEONE, 0));
        rAttr.Put(SdrAngleItem(SDRATTR_HORZSHEARONE, 0));
        rAttr.Put(SdrAngleItem(SDRATTR_VERTSHEARONE, 0));

        if(aSelection.size() > 1)
        {
            rAttr.Put(SdrFractionItem(SDRATTR_RESIZEXALL, Fraction(1,1)));
            rAttr.Put(SdrFractionItem(SDRATTR_RESIZEYALL, Fraction(1,1)));
            rAttr.Put(SdrAngleItem(SDRATTR_ROTATEALL, 0));
            rAttr.Put(SdrAngleItem(SDRATTR_HORZSHEARALL, 0));
            rAttr.Put(SdrAngleItem(SDRATTR_VERTSHEARALL, 0));
        }

        if(SDRDRAG_ROTATE == GetDragMode() || SDRDRAG_MIRROR == GetDragMode())
        {
            rAttr.Put(SdrMetricItem(SDRATTR_TRANSFORMREF1X, basegfx::fround(GetRef1().getX())));
            rAttr.Put(SdrMetricItem(SDRATTR_TRANSFORMREF1Y, basegfx::fround(GetRef1().getY())));
        }

        if(SDRDRAG_MIRROR == GetDragMode())
        {
            rAttr.Put(SdrMetricItem(SDRATTR_TRANSFORMREF2X, basegfx::fround(GetRef2().getX())));
            rAttr.Put(SdrMetricItem(SDRATTR_TRANSFORMREF2Y, basegfx::fround(GetRef2().getY())));
        }
    }
}

SfxItemSet SdrEditView::GetAttrFromMarked(bool bOnlyHardAttr) const
{
    SfxItemSet aSet(getSdrModelFromSdrView().GetItemPool());

    if(areSdrObjectsSelected())
    {
        MergeAttrFromMarked(aSet, bOnlyHardAttr);
        //the EE_FEATURE items should not be set with SetAttrToMarked (see error message there)
        //so we do not set them here
        // #i32448#
        // Do not disable, but clear the items.
        aSet.ClearItem(EE_FEATURE_TAB);
        aSet.ClearItem(EE_FEATURE_LINEBR);
        aSet.ClearItem(EE_FEATURE_NOTCONV);
        aSet.ClearItem(EE_FEATURE_FIELD);
    }

    return aSet;
}

void SdrEditView::MergeAttrFromMarked(SfxItemSet& rAttr, bool bOnlyHardAttr) const
{
    if(areSdrObjectsSelected())
    {
        const SdrObjectVector aSelection(getSelectedSdrObjectVectorFromSdrMarkView());

        for(sal_uInt32 a(0); a < aSelection.size(); a++)
        {
            const SfxItemSet& rSet = aSelection[a]->GetMergedItemSet();
            SfxWhichIter aIter(rSet);
            sal_uInt16 nWhich(aIter.FirstWhich());

            while(nWhich)
            {
                if(!bOnlyHardAttr)
                {
                    if(SFX_ITEM_DONTCARE == rSet.GetItemState(nWhich, false))
                    {
                        rAttr.InvalidateItem(nWhich);
                    }
                    else
                    {
                        rAttr.MergeValue(rSet.Get(nWhich), true);
                    }
                }
                else if(SFX_ITEM_SET == rSet.GetItemState(nWhich, false))
                {
                    const SfxPoolItem& rItem = rSet.Get(nWhich);

                    rAttr.MergeValue(rItem, true);
                }

                nWhich = aIter.NextWhich();
            }
        }
    }
}

void SdrEditView::SetAttrToMarked(const SfxItemSet& rAttr, bool bReplaceAll)
{
    if (areSdrObjectsSelected())
    {
#ifdef DBG_UTIL
        {
            bool bHasEEFeatureItems=false;
            SfxItemIter aIter(rAttr);
            const SfxPoolItem* pItem=aIter.FirstItem();
            while (!bHasEEFeatureItems && pItem)
            {
                if (!IsInvalidItem(pItem)) {
                    sal_uInt16 nW=pItem->Which();
                    if (nW>=EE_FEATURE_START && nW<=EE_FEATURE_END) bHasEEFeatureItems=true;
                }
                pItem=aIter.NextItem();
            }
            if(bHasEEFeatureItems)
            {
                String aMessage;
                aMessage.AppendAscii("SdrEditView::SetAttrToMarked(): Das setzen von EE_FEATURE-Items an der SdrView macht keinen Sinn! Es fuehrt nur zu Overhead und nicht mehr lesbaren Dokumenten.");
                InfoBox(NULL, aMessage).Execute();
            }
        }
#endif

        // #103836# if the user thets character attributes to the complete shape,
        //          we want to remove all hard set character attributes with same
        //          which ids from the text. We do that later but here we remember
        //          all character attribute which id's that are set.
        std::vector<sal_uInt16> aCharWhichIds;

        {
            SfxItemIter aIter(rAttr);
            const SfxPoolItem* pItem = aIter.FirstItem();

            while(pItem)
            {
                if(!IsInvalidItem(pItem))
                {
                    const sal_uInt16 nWhich(pItem->Which());

                    if(nWhich >= EE_CHAR_START && nWhich <= EE_CHAR_END)
                    {
                        aCharWhichIds.push_back(nWhich);
                    }
                }

                pItem = aIter.NextItem();
            }
        }

        // Joe, 2.7.98: Damit Undo nach Format.Standard auch die Textattribute korrekt restauriert
        bool bHasEEItems(SearchOutlinerItems(rAttr, bReplaceAll));

        // AW 030100: save additional geom info when para or char attributes
        // are changed and the geom form of the text object might be changed
        bool bPossibleGeomChange(false);
        SfxWhichIter aIter(rAttr);
        sal_uInt16 nWhich(aIter.FirstWhich());

        while(!bPossibleGeomChange && nWhich)
        {
            const SfxItemState eState(rAttr.GetItemState(nWhich));

            if(SFX_ITEM_SET == eState)
            {
                if((nWhich >= SDRATTR_TEXT_MINFRAMEHEIGHT && nWhich <= SDRATTR_TEXT_CONTOURFRAME)
                    || nWhich == SDRATTR_3DOBJ_PERCENT_DIAGONAL
                    || nWhich == SDRATTR_3DOBJ_BACKSCALE
                    || nWhich == SDRATTR_3DOBJ_DEPTH
                    || nWhich == SDRATTR_3DOBJ_END_ANGLE
                    || nWhich == SDRATTR_3DSCENE_DISTANCE)
                {
                    bPossibleGeomChange = true;
                }
            }

            nWhich = aIter.NextWhich();
        }

        const bool bUndo(IsUndoEnabled());

        if(bUndo)
        {
            XubString aStr;

            TakeMarkedDescriptionString(STR_EditSetAttributes, aStr);
            BegUndo(aStr);
        }

        const SdrObjectVector aSelection(getSelectedSdrObjectVectorFromSdrMarkView());
        std::vector< E3DModifySceneSnapRectUpdater* > aUpdaters;

        // create ItemSet without SFX_ITEM_DONTCARE. Put()
        // uses it's second parameter (bInvalidAsDefault) to
        // remove all such items to set them to default.
        SfxItemSet aAttr(*rAttr.GetPool(), rAttr.GetRanges());
        aAttr.Put(rAttr, true);

        // #i38135#
        bool bResetAnimationTimer(false);

        // check if LineWidth is part of the change
        const bool bLineWidthChange(SFX_ITEM_SET == aAttr.GetItemState(XATTR_LINEWIDTH));
        sal_Int32 nNewLineWidth(0);
        sal_Int32 nOldLineWidth(0);

        if(bLineWidthChange)
        {
            nNewLineWidth = ((const XLineWidthItem&)aAttr.Get(XATTR_LINEWIDTH)).GetValue();
        }

        for(sal_uInt32 nm(0); nm < aSelection.size(); nm++)
        {
            SdrObject* pObj = aSelection[nm];

            if(bUndo)
            {
                const SdrEdgeObj* pSdrEdgeObj = dynamic_cast< const SdrEdgeObj* >(pObj);

                if(pSdrEdgeObj)
                {
                    bPossibleGeomChange = true;
                }
                else if(bUndo)
                {
                    std::vector< SdrUndoAction* > vConnectorUndoActions;

                    vConnectorUndoActions = CreateConnectorUndo(*pObj);
                    AddUndoActions( vConnectorUndoActions );
                }
            }

            // new geometry undo
            if(bPossibleGeomChange && bUndo)
            {
                // save position and size of obect, too
                AddUndo( getSdrModelFromSdrView().GetSdrUndoFactory().CreateUndoGeoObject(*pObj));
            }

            if( bUndo )
            {
                // #i8508#
                // If this is a text object also rescue the OutlinerParaObject since
                // applying attributes to the object may change text layout when
                // multiple portions exist with multiple formats. If a OutlinerParaObject
                // really exists and needs to be rescued is evaluated in the undo
                // implementation itself.
                const bool bRescueText = dynamic_cast< SdrTextObj* >(pObj) != 0;

                // add attribute undo
                AddUndo(getSdrModelFromSdrView().GetSdrUndoFactory().CreateUndoAttrObject(*pObj, false, bHasEEItems || bPossibleGeomChange || bRescueText));
            }

            // set up a scxene updater if object is a 3d object
            if(dynamic_cast< E3dObject* >(pObj))
            {
                aUpdaters.push_back(new E3DModifySceneSnapRectUpdater(pObj));
            }

            if(bLineWidthChange)
            {
                nOldLineWidth = ((const XLineWidthItem&)pObj->GetMergedItem(XATTR_LINEWIDTH)).GetValue();
            }

            // set attributes at object
            pObj->SetMergedItemSetAndBroadcast(aAttr, bReplaceAll);

            if(bLineWidthChange)
            {
                const SfxItemSet& rSet = pObj->GetMergedItemSet();

                if(nOldLineWidth != nNewLineWidth)
                {
                    if(SFX_ITEM_DONTCARE != rSet.GetItemState(XATTR_LINESTARTWIDTH))
                    {
                        const sal_Int32 nValAct(((const XLineStartWidthItem&)rSet.Get(XATTR_LINESTARTWIDTH)).GetValue());
                        const sal_Int32 nValNewStart(std::max((sal_Int32)0, nValAct + (((nNewLineWidth - nOldLineWidth) * 15) / 10)));

                        pObj->SetMergedItem(XLineStartWidthItem(nValNewStart));
                    }

                    if(SFX_ITEM_DONTCARE != rSet.GetItemState(XATTR_LINEENDWIDTH))
                    {
                        const sal_Int32 nValAct(((const XLineEndWidthItem&)rSet.Get(XATTR_LINEENDWIDTH)).GetValue());
                        const sal_Int32 nValNewEnd(std::max((sal_Int32)0, nValAct + (((nNewLineWidth - nOldLineWidth) * 15) / 10)));

                        pObj->SetMergedItem(XLineEndWidthItem(nValNewEnd));
                    }
                }
            }

            SdrTextObj* pTextObj = dynamic_cast< SdrTextObj* >(pObj);

            if(pTextObj)
            {
                if(aCharWhichIds.size())
                {
                    const SdrObjectChangeBroadcaster aSdrObjectChangeBroadcaster(*pTextObj, HINT_OBJCHG_ATTR);

                    pTextObj->RemoveOutlinerCharacterAttribs( aCharWhichIds );
                    pTextObj->SetChanged();
                }
            }

            // #i38495#
            if(!bResetAnimationTimer)
            {
                if(pObj->GetViewContact().isAnimatedInAnyViewObjectContact())
                {
                    bResetAnimationTimer = true;
                }
            }
        }

        // fire scene updaters
        while(aUpdaters.size())
        {
            delete aUpdaters.back();
            aUpdaters.pop_back();
        }

        // #i38135#
        if(bResetAnimationTimer)
        {
            SetAnimationTimer(0);
        }

        // besser vorher checken, was gemacht werden soll:
        // pObj->SetAttr() oder SetNotPersistAttr()
        // !!! fehlende Implementation !!!
        SetNotPersistAttrToMarked(rAttr, bReplaceAll);

        if(bUndo)
        {
            EndUndo();
        }
    }
}

SfxStyleSheet* SdrEditView::GetStyleSheetFromMarked() const
{
    SfxStyleSheet* pRet = 0;

    if(areSdrObjectsSelected())
    {
        const SdrObjectVector aSelection(getSelectedSdrObjectVectorFromSdrMarkView());

        for(sal_uInt32 nm(0); nm < aSelection.size(); nm++)
        {
            SfxStyleSheet* pSS = aSelection[nm]->GetStyleSheet();

            if(!nm)
            {
                pRet = pSS;
            }
            else if(pRet != pSS)
            {
                return 0; // different StyleSheets
            }
        }
    }

    return pRet;
}

void SdrEditView::SetStyleSheetToMarked(SfxStyleSheet* pStyleSheet, bool bDontRemoveHardAttr)
{
    if(areSdrObjectsSelected())
    {
        const SdrObjectVector aSelection(getSelectedSdrObjectVectorFromSdrMarkView());
        const bool bUndo(IsUndoEnabled());

        if(bUndo)
        {
            XubString aStr;

            if(pStyleSheet)
            {
                TakeMarkedDescriptionString(STR_EditSetStylesheet, aStr);
            }
            else
            {
                TakeMarkedDescriptionString(STR_EditDelStylesheet, aStr);
            }

            BegUndo(aStr);
        }

        for(sal_uInt32 nm(0); nm < aSelection.size(); nm++)
        {
            SdrObject* pMarkedSdrObject = aSelection[nm];

            if(bUndo)
            {
                AddUndo(getSdrModelFromSdrView().GetSdrUndoFactory().CreateUndoGeoObject(*pMarkedSdrObject));
                AddUndo(getSdrModelFromSdrView().GetSdrUndoFactory().CreateUndoAttrObject(*pMarkedSdrObject, true, true));
            }

            pMarkedSdrObject->SetStyleSheet(pStyleSheet, bDontRemoveHardAttr);
        }

        if(bUndo)
        {
            EndUndo();
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool SdrEditView::GetAttributes(SfxItemSet& rTargetSet, bool bOnlyHardAttr) const
{
    if(areSdrObjectsSelected())
    {
        rTargetSet.Put(GetAttrFromMarked(bOnlyHardAttr), false);

        return true;
    }
    else
    {
        return SdrMarkView::GetAttributes(rTargetSet, bOnlyHardAttr);
    }
}

bool SdrEditView::SetAttributes(const SfxItemSet& rSet, bool bReplaceAll)
{
    if(areSdrObjectsSelected())
    {
        SetAttrToMarked(rSet, bReplaceAll);

        return true;
    }
    else
    {
        return SdrMarkView::SetAttributes(rSet, bReplaceAll);
    }
}

SfxStyleSheet* SdrEditView::GetStyleSheet() const
{
    if(areSdrObjectsSelected())
    {
        return GetStyleSheetFromMarked();
    }
    else
    {
        return SdrMarkView::GetStyleSheet();
    }
}

bool SdrEditView::SetStyleSheet(SfxStyleSheet* pStyleSheet, bool bDontRemoveHardAttr)
{
    if(areSdrObjectsSelected())
    {
        SetStyleSheetToMarked(pStyleSheet, bDontRemoveHardAttr);

        return true;
    }
    else
    {
        return SdrMarkView::SetStyleSheet(pStyleSheet, bDontRemoveHardAttr);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

SfxItemSet SdrEditView::GetGeoAttrFromMarked() const
{
    SfxItemSet aRetSet(getSdrModelFromSdrView().GetItemPool(),   // SID_ATTR_TRANSFORM_... aus s:svxids.hrc
        SID_ATTR_TRANSFORM_POS_X, SID_ATTR_TRANSFORM_ANGLE,
        SID_ATTR_TRANSFORM_PROTECT_POS, SID_ATTR_TRANSFORM_AUTOHEIGHT,
        SDRATTR_ECKENRADIUS, SDRATTR_ECKENRADIUS,
        0, 0);

    if(areSdrObjectsSelected())
    {
        const SdrObjectVector aSelection(getSelectedSdrObjectVectorFromSdrMarkView());
        basegfx::B2DRange aRange(getMarkedObjectSnapRange());

        if(aRange.isEmpty())
        {
            return aRetSet;
        }

        const basegfx::B2DPoint aPageOrigin(GetSdrPageView()
            ? GetSdrPageView()->GetPageOrigin()
            : basegfx::B2DPoint(0.0, 0.0));

        aRange.transform(basegfx::tools::createTranslateB2DHomMatrix(-aPageOrigin));

        // translation
        aRetSet.Put(SfxInt32Item(SID_ATTR_TRANSFORM_POS_X, basegfx::fround(aRange.getMinX())));
        aRetSet.Put(SfxInt32Item(SID_ATTR_TRANSFORM_POS_Y, basegfx::fround(aRange.getMinY())));

        // scale
        double fResizeRefX(aRange.getMinX());
        double fResizeRefY(aRange.getMinY());

        if(SDRDRAG_ROTATE == GetDragMode())
        {
            // rotation axis also for scale
            fResizeRefX = GetRef1().getX();
            fResizeRefY = GetRef1().getY();
        }

        aRetSet.Put(SfxUInt32Item(SID_ATTR_TRANSFORM_WIDTH,basegfx::fround(aRange.getWidth())));
        aRetSet.Put(SfxUInt32Item(SID_ATTR_TRANSFORM_HEIGHT,basegfx::fround(aRange.getHeight())));
        aRetSet.Put(SfxInt32Item(SID_ATTR_TRANSFORM_RESIZE_REF_X,basegfx::fround(fResizeRefX)));
        aRetSet.Put(SfxInt32Item(SID_ATTR_TRANSFORM_RESIZE_REF_Y,basegfx::fround(fResizeRefY)));

        basegfx::B2DPoint aRotateAxe(GetRef1() - aPageOrigin);

        // rotation
        double fRotateRefX(aRange.getCenterX());
        double fRotateRefY(aRange.getCenterY());

        if(SDRDRAG_ROTATE == GetDragMode())
        {
            fRotateRefX = aRotateAxe.getX();
            fRotateRefY = aRotateAxe.getY();
        }

        // get rotation of selection. fAllRotation is in radians [0.0 .. F_2PI[
        const double fAllRotation(GetMarkedObjRotate());

        // convert to old notation used in the UI; it's orientation is mirrored and its
        // degrees * 100 in integer
        const sal_Int32 nOldAllRot(sdr::legacy::convertRotateAngleNewToLegacy(fAllRotation));

        aRetSet.Put(SfxInt32Item(SID_ATTR_TRANSFORM_ANGLE,nOldAllRot));
        aRetSet.Put(SfxInt32Item(SID_ATTR_TRANSFORM_ROT_X,basegfx::fround(fRotateRefX)));
        aRetSet.Put(SfxInt32Item(SID_ATTR_TRANSFORM_ROT_Y,basegfx::fround(fRotateRefY)));

        // shear
        double fShearRefX(aRange.getMinX());
        double fShearRefY(aRange.getMaxY());

        if(SDRDRAG_ROTATE == GetDragMode())
        {
            // rotation axis also for shear
            fShearRefX = aRotateAxe.getX();
            fShearRefY = aRotateAxe.getY();
        }

        // get shear of selection. GetMarkedObjShearX is the tan of the shear angle in radians
        // tan(]-F_PI2 .. F_PI2[), so apply atan to get the shear angle
        const double fAllShearAngleX(atan(GetMarkedObjShearX()));

        // convert to old notation used in the UI; it's orientation is mirrored and its
        // degrees * 100 in integer
        const sal_Int32 nOldAllShearAngleX(sdr::legacy::convertShearAngleXNewToLegacy(fAllShearAngleX));

        aRetSet.Put(SfxInt32Item(SID_ATTR_TRANSFORM_SHEAR,nOldAllShearAngleX));
        aRetSet.Put(SfxInt32Item(SID_ATTR_TRANSFORM_SHEAR_X,basegfx::fround(fShearRefX)));
        aRetSet.Put(SfxInt32Item(SID_ATTR_TRANSFORM_SHEAR_Y,basegfx::fround(fShearRefY)));

        // check single objects for translation/scale protection
        SdrObject* pObj = aSelection[0];
        bool bPosProt(pObj->IsMoveProtect());
        bool bSizProt(pObj->IsResizeProtect());
        bool bPosProtDontCare(false);
        bool bSizProtDontCare(false);

        for(sal_uInt32 i(1); i < aSelection.size() && (!bPosProtDontCare || !bSizProtDontCare); i++)
        {
            pObj = aSelection[i];

            if(bPosProt != pObj->IsMoveProtect())
            {
                bPosProtDontCare = true;
            }

            if(bSizProt != pObj->IsResizeProtect())
            {
                bSizProtDontCare = true;
            }
        }

        // use InvalidateItem to set to DONT_CARE if needed
        if(bPosProtDontCare)
        {
            aRetSet.InvalidateItem(SID_ATTR_TRANSFORM_PROTECT_POS);
        }
        else
        {
            aRetSet.Put(SfxBoolItem(SID_ATTR_TRANSFORM_PROTECT_POS,bPosProt));
        }

        if(bSizProtDontCare)
        {
            aRetSet.InvalidateItem(SID_ATTR_TRANSFORM_PROTECT_SIZE);
        }
        else
        {
            aRetSet.Put(SfxBoolItem(SID_ATTR_TRANSFORM_PROTECT_SIZE,bSizProt));
        }

        // get all attributes from marked SdrObjects
        SfxItemSet aMarkAttr(GetAttrFromMarked(false));

        SfxItemState eState(aMarkAttr.GetItemState(SDRATTR_TEXT_AUTOGROWWIDTH));
        bool bAutoGrow(((SdrOnOffItem&)(aMarkAttr.Get(SDRATTR_TEXT_AUTOGROWWIDTH))).GetValue());

        if(SFX_ITEM_DONTCARE == eState)
        {
            aRetSet.InvalidateItem(SID_ATTR_TRANSFORM_AUTOWIDTH);
        }
        else if(SFX_ITEM_SET == eState)
        {
            aRetSet.Put(SfxBoolItem(SID_ATTR_TRANSFORM_AUTOWIDTH, bAutoGrow));
        }

        eState = aMarkAttr.GetItemState(SDRATTR_TEXT_AUTOGROWHEIGHT);
        bAutoGrow = ((SdrOnOffItem&)(aMarkAttr.Get(SDRATTR_TEXT_AUTOGROWHEIGHT))).GetValue();

        if(SFX_ITEM_DONTCARE == eState)
        {
            aRetSet.InvalidateItem(SID_ATTR_TRANSFORM_AUTOHEIGHT);
        }
        else if(SFX_ITEM_SET == eState)
        {
            aRetSet.Put(SfxBoolItem(SID_ATTR_TRANSFORM_AUTOHEIGHT,bAutoGrow));
        }

        eState = aMarkAttr.GetItemState(SDRATTR_ECKENRADIUS);
        const sal_Int32 nRadius(((SdrMetricItem&)(aMarkAttr.Get(SDRATTR_ECKENRADIUS))).GetValue());

        if(SFX_ITEM_DONTCARE == eState)
        {
            aRetSet.InvalidateItem(SDRATTR_ECKENRADIUS);
        }
        else if(SFX_ITEM_SET == eState)
        {
            aRetSet.Put(SdrMetricItem(SDRATTR_ECKENRADIUS, nRadius));
        }

        const SdrObject* pSingle = getSelectedIfSingle();
        basegfx::B2DHomMatrix aTransformation;

        if(pSingle)
        {
            // single object, get homogen transformation
            aTransformation = pSingle->getSdrObjectTransformation();
        }
        else
        {
            // multiple objects, range is collected in aRange, but LogicToPagePos is already applied,
            // so get again
            const basegfx::B2DRange aAllRange(getMarkedObjectSnapRange());
            aTransformation = basegfx::tools::createScaleTranslateB2DHomMatrix(
                aAllRange.getMinimum(),
                aAllRange.getMaximum());
        }

        if(aTransformation.isIdentity())
        {
            aRetSet.InvalidateItem(SID_ATTR_TRANSFORM_MATRIX);
        }
        else
        {
            com::sun::star::geometry::AffineMatrix2D aAffineMatrix2D;
            basegfx::B2DPoint aPageOffset(0.0, 0.0);

            if(GetSdrPageView())
            {
                aPageOffset = GetSdrPageView()->GetPageOrigin();
            }

            aAffineMatrix2D.m00 = aTransformation.get(0, 0);
            aAffineMatrix2D.m01 = aTransformation.get(0, 1);
            aAffineMatrix2D.m02 = aTransformation.get(0, 2) - aPageOffset.getX();
            aAffineMatrix2D.m10 = aTransformation.get(1, 0);
            aAffineMatrix2D.m11 = aTransformation.get(1, 1);
            aAffineMatrix2D.m12 = aTransformation.get(1, 2) - aPageOffset.getY();

            aRetSet.Put(AffineMatrixItem(&aAffineMatrix2D));
        }
    }

    return aRetSet;
}

void SdrEditView::SetGeoAttrToMarked(const SfxItemSet& rAttr)
{
    if(!areSdrObjectsSelected())
    {
        return;
    }

    const SdrObjectVector aSelection(getSelectedSdrObjectVectorFromSdrMarkView());
    basegfx::B2DRange aRange(getMarkedObjectSnapRange());

    if(aRange.isEmpty())
    {
        return;
    }

    const basegfx::B2DPoint aPageOrigin(GetSdrPageView()
        ? GetSdrPageView()->GetPageOrigin()
        : basegfx::B2DPoint(0.0, 0.0));

    aRange.transform(basegfx::tools::createTranslateB2DHomMatrix(-aPageOrigin));

    const bool bModeIsRotate(SDRDRAG_ROTATE == GetDragMode());
    basegfx::B2DPoint aNewRotateCenter(0.0, 0.0);
    basegfx::B2DPoint aOldRotateCenter(aNewRotateCenter);

    if(bModeIsRotate)
    {
        const basegfx::B2DPoint aRotateAxe(GetRef1() - aPageOrigin);

        aNewRotateCenter = aOldRotateCenter = aRotateAxe;
    }

    SfxItemSet aSetAttr(getSdrModelFromSdrView().GetItemPool());
    const SfxPoolItem* pPoolItem = 0;

    // position change?
    const basegfx::B2DPoint aOldTranslate(aRange.getMinimum());
    basegfx::B2DPoint aNewTranslate(aOldTranslate);

    if (SFX_ITEM_SET==rAttr.GetItemState(SID_ATTR_TRANSFORM_POS_X,true,&pPoolItem))
    {
        aNewTranslate.setX(((const SfxInt32Item*)pPoolItem)->GetValue());
    }

    if (SFX_ITEM_SET==rAttr.GetItemState(SID_ATTR_TRANSFORM_POS_Y,true,&pPoolItem))
    {
        aNewTranslate.setY(((const SfxInt32Item*)pPoolItem)->GetValue());
    }

    // scale change?
    const basegfx::B2DVector aOldSize(aRange.getRange());
    basegfx::B2DVector aNewSize(aOldSize);
    bool bChgHgt(false);

    if (SFX_ITEM_SET==rAttr.GetItemState(SID_ATTR_TRANSFORM_WIDTH,true,&pPoolItem))
    {
        aNewSize.setX(((const SfxUInt32Item*)pPoolItem)->GetValue());
    }

    if (SFX_ITEM_SET==rAttr.GetItemState(SID_ATTR_TRANSFORM_HEIGHT,true,&pPoolItem))
    {
        aNewSize.setY(((const SfxUInt32Item*)pPoolItem)->GetValue());
        bChgHgt = true;
    }

    RECT_POINT eSizePoint(RP_MM);
    const bool bScaleChanged(!aNewSize.equal(aOldSize));

    if(bScaleChanged)
    {
        eSizePoint=(RECT_POINT)((const SfxAllEnumItem&)rAttr.Get(SID_ATTR_TRANSFORM_SIZE_POINT)).GetValue();
    }

    // rotation change?
    const double fOldRotateAngle(GetMarkedObjRotate()); // radians [0.0 .. F_2PI[
    double fNewRotateAngle(fOldRotateAngle);

    if (SFX_ITEM_SET==rAttr.GetItemState(SID_ATTR_TRANSFORM_ANGLE,true,&pPoolItem))
    {
        const sal_Int32 nAllRot(((const SfxInt32Item*)pPoolItem)->GetValue());

        // convert from old UI units (angle in degree * 100, wrong oriented)
        // to correctly orinented radians, same coordinate system as fOldRotateAngle
        fNewRotateAngle = sdr::legacy::convertRotateAngleLegacyToNew(nAllRot);
    }

    const bool bRotate(!basegfx::fTools::equal(fOldRotateAngle, fNewRotateAngle));

    // #86909# pos rot point x
    if(bRotate || SFX_ITEM_SET==rAttr.GetItemState(SID_ATTR_TRANSFORM_ROT_X, true ,&pPoolItem))
    {
        aNewRotateCenter.setX(((const SfxInt32Item&)rAttr.Get(SID_ATTR_TRANSFORM_ROT_X)).GetValue());
    }

    // #86909# pos rot point y
    if(bRotate || SFX_ITEM_SET==rAttr.GetItemState(SID_ATTR_TRANSFORM_ROT_Y, true ,&pPoolItem))
    {
        aNewRotateCenter.setY(((const SfxInt32Item&)rAttr.Get(SID_ATTR_TRANSFORM_ROT_Y)).GetValue());
    }

    // shear change?
    double fShearChange(0.0);
    basegfx::B2DPoint aShearOffset(0.0, 0.0);
    bool bDoShearY(false);
    bool bDoShearDelta(false);

    if(SFX_ITEM_SET == rAttr.GetItemState(SID_ATTR_TRANSFORM_SHEAR,true,&pPoolItem))
    {
        // get UI shear angle ]-9000 .. 9000[
        const sal_Int32 nAllShear(((const SfxInt32Item*)pPoolItem)->GetValue());

        // convert to correctly oriented shear angle in radians ]-F_PI2 .. F_PI2[
        // limit to partial values (corresponding to +-89 degree)
        // convert from shear angle to shear value used in the transformation to
        // get to the same coordinate system as fOldShearValue below will use
        const double fMaxShearRange(F_PI2 * (89.0/90.0));
        const double fNewShearValue(
            tan(
                basegfx::clamp(
                    sdr::legacy::convertShearAngleXLegacyToNew(nAllShear),
                    -fMaxShearRange,
                    fMaxShearRange)));

        // check if we have a ShearY
        bDoShearY = ((const SfxBoolItem&)rAttr.Get(SID_ATTR_TRANSFORM_SHEAR_VERTICAL)).GetValue();

        // get current shear value. It is the tan of the shear angle in radians
        // tan(]-F_PI .. F_PI[)
        double fOldShearValue(GetMarkedObjShearX());

        if(bDoShearY)
        {
            // Currently only ShearX is directly used at the SdrObject since the homogen
            // matrix only has six degrees of freedom and it has to be decided which one
            // to use. It can be shown mathematically that a ShearY about degree x is
            // the same as a 90 degree rotation, a ShearY(-x) and a -90 degree back-rotation.
            // Exactly this will be used below. It also shows that the ShearY is -ShearX, thus
            // the compare value can be detected
            fOldShearValue = -fOldShearValue;
        }

        if(!basegfx::fTools::equal(fNewShearValue, fOldShearValue))
        {
            // create shear diff and convert to shear angle, this is what ShearMarkedObj
            // expects
            fShearChange = atan(fNewShearValue - fOldShearValue);
            bDoShearDelta = true;
        }

        if(bDoShearDelta)
        {
            aShearOffset.setX(((const SfxInt32Item&)rAttr.Get(SID_ATTR_TRANSFORM_SHEAR_X)).GetValue());
            aShearOffset.setY(((const SfxInt32Item&)rAttr.Get(SID_ATTR_TRANSFORM_SHEAR_Y)).GetValue());
        }
    }

    bool bSetAttr(false);

    // AutoGrow
    if (SFX_ITEM_SET==rAttr.GetItemState(SID_ATTR_TRANSFORM_AUTOWIDTH,true,&pPoolItem))
    {
        bool bAutoGrow=((const SfxBoolItem*)pPoolItem)->GetValue();
        aSetAttr.Put(SdrOnOffItem(SDRATTR_TEXT_AUTOGROWWIDTH, bAutoGrow));
        bSetAttr = true;
    }

    if (SFX_ITEM_SET==rAttr.GetItemState(SID_ATTR_TRANSFORM_AUTOHEIGHT,true,&pPoolItem))
    {
        bool bAutoGrow=((const SfxBoolItem*)pPoolItem)->GetValue();
        aSetAttr.Put(SdrOnOffItem(SDRATTR_TEXT_AUTOGROWHEIGHT, bAutoGrow));
        bSetAttr = true;
    }

    // rounded edge changed?
    if (mbEdgeRadiusAllowed && SFX_ITEM_SET==rAttr.GetItemState(SDRATTR_ECKENRADIUS,true,&pPoolItem))
    {
        sal_Int32 nRadius=((SdrMetricItem*)pPoolItem)->GetValue();
        aSetAttr.Put(SdrMetricItem(SDRATTR_ECKENRADIUS, nRadius));
        bSetAttr = true;
    }

    ForcePossibilities();
    BegUndo(ImpGetResStr(STR_EditTransform), getSelectionDescription(aSelection));

    if(bSetAttr)
    {
        SetAttrToMarked(aSetAttr, false);
    }

    // change scale
    if(bScaleChanged && (mbResizeFreeAllowed || mbResizePropAllowed))
    {
        basegfx::B2DPoint aRefPoint(aRange.getMinimum());

        switch(eSizePoint)
        {
            default: break; // case RP_LT
            case RP_MT: aRefPoint = basegfx::B2DPoint(aRange.getCenterX(), aRange.getMinY()); break;
            case RP_RT: aRefPoint = basegfx::B2DPoint(aRange.getMaxX(), aRange.getMinY()); break;
            case RP_LM: aRefPoint = basegfx::B2DPoint(aRange.getMinX(), aRange.getCenterY()); break;
            case RP_MM: aRefPoint = aRange.getCenter(); break;
            case RP_RM: aRefPoint = basegfx::B2DPoint(aRange.getMaxX(), aRange.getCenterY()); break;
            case RP_LB: aRefPoint = basegfx::B2DPoint(aRange.getMinX(), aRange.getMaxY()); break;
            case RP_MB: aRefPoint = basegfx::B2DPoint(aRange.getCenterX(), aRange.getMaxY()); break;
            case RP_RB: aRefPoint = aRange.getMaximum(); break;
        }

        aRefPoint += aPageOrigin;

        const basegfx::B2DTuple aScale(
            aNewSize.getX() / (basegfx::fTools::equalZero(aOldSize.getX()) ? 1.0 : aOldSize.getX()),
            aNewSize.getY() / (basegfx::fTools::equalZero(aOldSize.getY()) ? 1.0 : aOldSize.getY()));

        ResizeMarkedObj(aRefPoint, aScale);
    }

    // change shear
    if(bDoShearDelta && mbShearAllowed)
    {
        basegfx::B2DPoint aRef(aShearOffset + aPageOrigin);

        if(bDoShearY)
        {
            // see explanation at setting bDoShearY above
            RotateMarkedObj(aRef, F_PI2);
            ShearMarkedObj(aRef, -fShearChange, true);
            RotateMarkedObj(aRef, -F_PI2);
        }
        else
        {
            ShearMarkedObj(aRef, fShearChange, false);
        }
    }

    // change rotation
    if(bRotate && (mbRotateFreeAllowed || mbRotate90Allowed))
    {
        const basegfx::B2DPoint aRef(aNewRotateCenter + aPageOrigin);

        RotateMarkedObj(aRef, fNewRotateAngle - fOldRotateAngle);
    }

    // set rotation point position
    if(bModeIsRotate && !aNewRotateCenter.equal(aOldRotateCenter))
    {
        const basegfx::B2DPoint aNewRef1(aNewRotateCenter + aPageOrigin);

        SetRef1(aNewRef1);
    }

    // change translation
    if(!aOldTranslate.equal(aNewTranslate) && mbMoveAllowedOnSelection)
    {
        MoveMarkedObj(aNewTranslate - aOldTranslate);
    }

    // protect position
    if(SFX_ITEM_SET == rAttr.GetItemState(SID_ATTR_TRANSFORM_PROTECT_POS, true, &pPoolItem))
    {
        const bool bProtPos(((const SfxBoolItem*)pPoolItem)->GetValue());
        bool bChanged(false);

        for(sal_uInt32 i(0); i < aSelection.size(); i++)
        {
            SdrObject* pObj = aSelection[i];

            if(pObj->IsMoveProtect() != bProtPos)
            {
                bChanged = true;
                pObj->SetMoveProtect(bProtPos);

                if(bProtPos)
                {
                    pObj->SetResizeProtect(true);
                }
            }
        }

        if(bChanged)
        {
            mbMoveProtect = bProtPos;

            if(bProtPos)
            {
                mbResizeProtect = true;
            }
        }
    }

    if(!mbMoveProtect)
    {
        // protect size
        if(SFX_ITEM_SET == rAttr.GetItemState(SID_ATTR_TRANSFORM_PROTECT_SIZE, true, &pPoolItem))
        {
            const bool bProtSize(((const SfxBoolItem*)pPoolItem)->GetValue());
            bool bChanged(false);

            for(sal_uInt32 i(0); i < aSelection.size(); i++)
            {
                SdrObject* pObj = aSelection[i];

                if(pObj->IsResizeProtect() != bProtSize)
                {
                    bChanged = true;
                    pObj->SetResizeProtect(bProtSize);
                }
            }

            if(bChanged)
            {
                mbResizeProtect = bProtSize;
            }
        }
    }

    EndUndo();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool SdrEditView::IsAlignPossible() const
{
    if(!areSdrObjectsSelected())
    {
        return false;         // Nix markiert!
    }
    else
    {
        ForcePossibilities();
        SdrObject* pSingleSelected = getSelectedIfSingle();

        if(pSingleSelected)
        {
            // einzelnes Obj an der Seite ausrichten
            return mbMoveAllowedOnSelection;
        }

        // ansonsten ist MarkCount>=2
        return mbOneOrMoreMovable;
    }
}

void SdrEditView::AlignMarkedObjects(SdrHorAlign eHor, SdrVertAlign eVert, bool bBoundRects)
{
    if(SDRHALIGN_NONE == eHor && SDRVALIGN_NONE == eVert)
    {
        return;
    }

    if(areSdrObjectsSelected())
    {
        const SdrObjectVector aSelection(getSelectedSdrObjectVectorFromSdrMarkView());
        const bool bUndo(IsUndoEnabled());

        if(bUndo)
        {
            String aStr(getSelectionDescription(aSelection));

            if(SDRHALIGN_NONE == eHor)
            {
                switch(eVert)
                {
                    case SDRVALIGN_TOP   : TakeMarkedDescriptionString(STR_EditAlignVTop   ,aStr); break;
                    case SDRVALIGN_BOTTOM: TakeMarkedDescriptionString(STR_EditAlignVBottom,aStr); break;
                    case SDRVALIGN_CENTER: TakeMarkedDescriptionString(STR_EditAlignVCenter,aStr); break;
                    default: break;
                }
            }
            else if(SDRVALIGN_NONE == eVert)
            {
                switch(eHor)
                {
                    case SDRHALIGN_LEFT  : TakeMarkedDescriptionString(STR_EditAlignHLeft  ,aStr); break;
                    case SDRHALIGN_RIGHT : TakeMarkedDescriptionString(STR_EditAlignHRight ,aStr); break;
                    case SDRHALIGN_CENTER: TakeMarkedDescriptionString(STR_EditAlignHCenter,aStr); break;
                    default: break;
                }
            }
            else if(SDRHALIGN_CENTER == eHor && SDRVALIGN_CENTER == eVert)
            {
                TakeMarkedDescriptionString(STR_EditAlignCenter, aStr);
            }
            else
            {
                TakeMarkedDescriptionString(STR_EditAlign, aStr);
            }

            BegUndo(aStr);
        }

        basegfx::B2DRange aBound;
        sal_uInt32 nm;
        bool bHasFixed(false);

        for(nm = 0; nm < aSelection.size(); nm++)
        {
            SdrObject* pObj = aSelection[nm];
            SdrObjTransformInfoRec aInfo;
            pObj->TakeObjInfo(aInfo);

            if(!aInfo.mbMoveAllowed || pObj->IsMoveProtect())
            {
                const basegfx::B2DRange aObjRange(bBoundRects
                    ? pObj->getObjectRange(getAsSdrView())
                    : sdr::legacy::GetSnapRange(*pObj));

                aBound.expand(aObjRange);
                bHasFixed = true;
            }
        }

        if(!bHasFixed)
        {
            if(1 == aSelection.size())
            {
                // einzelnes Obj an der Seite ausrichten
                const SdrObject* pObj = aSelection[0];
                const SdrPage* pPage = pObj->getSdrPageFromSdrObject();
                const Rectangle aCurrentSnapRect(sdr::legacy::GetSnapRect(*pObj));
                const SdrPageGridFrameList* pGFL = pPage->GetGridFrameList(*getAsSdrView(), &aCurrentSnapRect);
                const SdrPageGridFrame* pFrame = 0;

                if(pGFL && pGFL->GetCount())
                {
                    // Writer
                    pFrame = &((*pGFL)[0]);
                }

                if(pFrame)
                {
                    // Writer
                    aBound = pFrame->GetUserArea();
                }
                else
                {
                    aBound = pPage->GetInnerPageRange();
                }
            }
            else
            {
                if (bBoundRects)
                {
                    aBound = sdr::legacy::GetAllObjBoundRange(aSelection);
                }
                else
                {
                    aBound = getMarkedObjectSnapRange();
                }
            }
        }

        const basegfx::B2DPoint aCenter(aBound.getCenter());

        for(nm = 0; nm < aSelection.size(); nm++)
        {
            SdrObject* pObj = aSelection[nm];
            SdrObjTransformInfoRec aInfo;
            pObj->TakeObjInfo(aInfo);

            if(aInfo.mbMoveAllowed && !pObj->IsMoveProtect())
            {
                basegfx::B2DVector aMove(0.0, 0.0);
                const basegfx::B2DRange aObjRange(bBoundRects
                    ? pObj->getObjectRange(getAsSdrView())
                    : sdr::legacy::GetSnapRange(*pObj));

                switch(eVert)
                {
                    case SDRVALIGN_TOP : aMove.setY(aBound.getMinY() - aObjRange.getMinY()); break;
                    case SDRVALIGN_BOTTOM: aMove.setY(aBound.getMaxY() - aObjRange.getMaxY()); break;
                    case SDRVALIGN_CENTER: aMove.setY(aCenter.getY() - aObjRange.getCenter().getY()); break;
                    default: break;
                }

                switch(eHor)
                {
                    case SDRHALIGN_LEFT : aMove.setX(aBound.getMinX() - aObjRange.getMinX()); break;
                    case SDRHALIGN_RIGHT : aMove.setX(aBound.getMaxX() - aObjRange.getMaxX()); break;
                    case SDRHALIGN_CENTER: aMove.setX(aCenter.getX() - aObjRange.getCenter().getX()); break;
                    default: break;
                }

                if(!aMove.equalZero())
                {
                    // #104104# SdrEdgeObj needs an extra SdrUndoGeoObj since the
                    // connections may need to be saved
                    if(bUndo)
                    {
                        AddUndo(getSdrModelFromSdrView().GetSdrUndoFactory().CreateUndoGeoObject(*pObj));
                    }

                    sdr::legacy::transformSdrObject(*pObj, basegfx::tools::createTranslateB2DHomMatrix(aMove));
                }
            }
        }

        if(bUndo)
        {
            EndUndo();
        }
    }
}

//////////////////////////////////////////////////////////////////////////////
// eof

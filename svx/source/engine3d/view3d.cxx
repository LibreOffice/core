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

#include <vcl/wrkwin.hxx>
#include <svx/svdogrp.hxx>
#include <svx/svdopath.hxx>
#include <tools/shl.hxx>
#include <svx/svditer.hxx>
#include <svx/svdpool.hxx>
#include <svx/svdorect.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svxids.hrc>
#include <editeng/colritem.hxx>
#include <svx/xtable.hxx>
#include <svx/svdview.hxx>
#include <svx/dialogs.hrc>
#include <svx/dialmgr.hxx>
#include "svx/globl3d.hxx"
#include <svx/obj3d.hxx>
#include <svx/lathe3d.hxx>
#include <svx/sphere3d.hxx>
#include <svx/extrud3d.hxx>
#include <svx/cube3d.hxx>
#include "dragmt3d.hxx"
#include <svx/view3d.hxx>
#include <svx/svdundo.hxx>
#include <svx/xflclit.hxx>
#include <svx/xlnclit.hxx>
#include <svx/svdograf.hxx>
#include <svx/xbtmpit.hxx>
#include <svx/xflbmtit.hxx>
#include <basegfx/range/b2drange.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <svx/xlnwtit.hxx>
#include <svx/sdr/overlay/overlaypolypolygon.hxx>
#include <svx/sdr/overlay/overlaymanager.hxx>
#include <svx/sdrpaintwindow.hxx>
#include <svx/sdr/contact/viewcontactofe3dscene.hxx>
#include <drawinglayer/geometry/viewinformation3d.hxx>
#include <svx/sdrpagewindow.hxx>
#include <svx/sdr/contact/displayinfo.hxx>
#include <svx/sdr/contact/objectcontact.hxx>
#include <svx/sdr/contact/viewobjectcontact.hxx>
#include <drawinglayer/primitive2d/unifiedtransparenceprimitive2d.hxx>
#include <svx/sdr/overlay/overlayprimitive2dsequenceobject.hxx>
#include <drawinglayer/primitive2d/transformprimitive2d.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <basegfx/polygon/b2dpolypolygoncutter.hxx>
#include <svx/svdlegacy.hxx>
#include <editeng/eeitem.hxx>
#include <svx/scene3d.hxx>

#define ITEMVALUE(ItemSet,Id,Cast)  ((const Cast&)(ItemSet).Get(Id)).GetValue()

////////////////////////////////////////////////////////////////////////////////////////////////////
// Migrate Marking

class Impl3DMirrorConstructOverlay
{
    // The OverlayObjects
    ::sdr::overlay::OverlayObjectList               maObjects;

    // the view
    const E3dView&                                  mrView;

    // the selected objects
    SdrObjectVector                                 maSelection;

    // the unmirrored polygons
    basegfx::B2DPolyPolygon*                        mpPolygons;

    // the overlay geometry from selected objects
    drawinglayer::primitive2d::Primitive2DSequence  maFullOverlay;

public:
    Impl3DMirrorConstructOverlay(const E3dView& rView);
    ~Impl3DMirrorConstructOverlay();

    void SetMirrorAxis(const basegfx::B2DPoint& rMirrorAxisA, const basegfx::B2DPoint& rMirrorAxisB);
};

Impl3DMirrorConstructOverlay::Impl3DMirrorConstructOverlay(const E3dView& rView)
:   maObjects(),
    mrView(rView),
    maSelection(rView.getSelectedSdrObjectVectorFromSdrMarkView()),
    mpPolygons(0),
    maFullOverlay()
{
    if(mrView.IsSolidDragging())
    {
        if(rView.GetSdrPageView() && rView.GetSdrPageView()->PageWindowCount())
        {
            sdr::contact::ObjectContact& rOC = rView.GetSdrPageView()->GetPageWindow(0)->GetObjectContact();
            sdr::contact::DisplayInfo aDisplayInfo;

            // Do not use the last ViewPort set at the OC at the last ProcessDisplay()
            rOC.resetViewPort();

            for(sal_uInt32 a(0); a < maSelection.size(); a++)
            {
                SdrObject* pObject = maSelection[a];
                sdr::contact::ViewContact& rVC = pObject->GetViewContact();
                sdr::contact::ViewObjectContact& rVOC = rVC.GetViewObjectContact(rOC);

                const drawinglayer::primitive2d::Primitive2DSequence aNewSequence(rVOC.getPrimitive2DSequenceHierarchy(aDisplayInfo));
                drawinglayer::primitive2d::appendPrimitive2DSequenceToPrimitive2DSequence(maFullOverlay, aNewSequence);
            }
        }
    }
    else
    {
        mpPolygons = new basegfx::B2DPolyPolygon[maSelection.size()];

        for(sal_uInt32 a(0); a < maSelection.size(); a++)
        {
            SdrObject* pObject = maSelection[a];
            mpPolygons[maSelection.size() - (a + 1)] = pObject->TakeXorPoly();
        }
    }
}

Impl3DMirrorConstructOverlay::~Impl3DMirrorConstructOverlay()
{
    // The OverlayObjects are cleared using the destructor of OverlayObjectList.
    // That destructor calls clear() at the list which removes all objects from the
    // OverlayManager and deletes them.
    if(!mrView.IsSolidDragging())
    {
        delete[] mpPolygons;
    }
}

void Impl3DMirrorConstructOverlay::SetMirrorAxis(const basegfx::B2DPoint& rMirrorAxisA, const basegfx::B2DPoint& rMirrorAxisB)
{
    // get rid of old overlay objects
    maObjects.clear();

    // create new ones
    for(sal_uInt32 a(0); a < mrView.PaintWindowCount(); a++)
    {
        SdrPaintWindow* pCandidate = mrView.GetPaintWindow(a);
        ::sdr::overlay::OverlayManager* pTargetOverlay = pCandidate->GetOverlayManager();

        if(pTargetOverlay)
        {
            // buld transfoprmation: translate and rotate so that given edge is
            // on x axis, them mirror in y and translate back
            const basegfx::B2DVector aEdge(rMirrorAxisB - rMirrorAxisA);
            basegfx::B2DHomMatrix aMatrixTransform(basegfx::tools::createTranslateB2DHomMatrix(-rMirrorAxisA));
            aMatrixTransform.rotate(-atan2(aEdge.getY(), aEdge.getX()));
            aMatrixTransform.scale(1.0, -1.0);
            aMatrixTransform.rotate(atan2(aEdge.getY(), aEdge.getX()));
            aMatrixTransform.translate(rMirrorAxisA.getX(), rMirrorAxisA.getY());

            if(mrView.IsSolidDragging())
            {
                if(maFullOverlay.hasElements())
                {
                    drawinglayer::primitive2d::Primitive2DSequence aContent(maFullOverlay);

                    if(!aMatrixTransform.isIdentity())
                    {
                        // embed in transformation group
                        drawinglayer::primitive2d::Primitive2DReference aTransformPrimitive2D(new drawinglayer::primitive2d::TransformPrimitive2D(aMatrixTransform, aContent));
                        aContent = drawinglayer::primitive2d::Primitive2DSequence(&aTransformPrimitive2D, 1);
                    }

                    // if we have full overlay from selected objects, embed with 50% transparence, the
                    // transformation is added to the OverlayPrimitive2DSequenceObject
                    drawinglayer::primitive2d::Primitive2DReference aUnifiedTransparencePrimitive2D(new drawinglayer::primitive2d::UnifiedTransparencePrimitive2D(aContent, 0.5));
                    aContent = drawinglayer::primitive2d::Primitive2DSequence(&aUnifiedTransparencePrimitive2D, 1);

                    sdr::overlay::OverlayPrimitive2DSequenceObject* pNew = new sdr::overlay::OverlayPrimitive2DSequenceObject(aContent);

                    pTargetOverlay->add(*pNew);
                    maObjects.append(*pNew);
                }
            }
            else
            {
                for(sal_uInt32 b(0); b < maSelection.size(); b++)
                {
                    // apply to polygon
                    basegfx::B2DPolyPolygon aPolyPolygon(mpPolygons[b]);
                    aPolyPolygon.transform(aMatrixTransform);

                    ::sdr::overlay::OverlayPolyPolygonStriped* pNew = new ::sdr::overlay::OverlayPolyPolygonStriped(aPolyPolygon);
                    pTargetOverlay->add(*pNew);
                    maObjects.append(*pNew);
                }
            }
        }
    }
}

/*************************************************************************
|*
|* Konstruktor 1
|*
\************************************************************************/

E3dView::E3dView(SdrModel& rModel, OutputDevice* pOut)
:   SdrView(rModel, pOut)
{
    InitView ();
}

/*************************************************************************
|*
|* DrawMarkedObj ueberladen, da eventuell nur einzelne 3D-Objekte
|* gezeichnet werden sollen
|*
\************************************************************************/

void E3dView::DrawMarkedObj(OutputDevice& rOut) const
{
    // Existieren 3D-Objekte, deren Szenen nicht selektiert sind?
    bool bSpecialHandling = false;
    E3dScene *pScene = NULL;
    const SdrObjectVector aSelection(getSelectedSdrObjectVectorFromSdrMarkView());
    sal_Int32 nObjs(0);

    for(nObjs = 0; nObjs < aSelection.size(); nObjs++)
    {
        SdrObject *pObj = aSelection[nObjs];
        E3dCompoundObject* pE3dCompoundObject = dynamic_cast< E3dCompoundObject* >(pObj);
        E3dObject* pE3dObject = dynamic_cast< E3dObject* >(pObj);

        if(pE3dCompoundObject)
        {
            // zugehoerige Szene
            pScene = pE3dCompoundObject->GetScene();

            if(pScene && !IsObjMarked(*pScene))
            {
                bSpecialHandling = true;
            }
        }

        // Alle SelectionFlags zuruecksetzen
        if(pE3dObject)
        {
            pScene = pE3dObject->GetScene();

            if(pScene)
            {
                pScene->SetSelected(false);
            }
        }
    }

    if(bSpecialHandling)
    {
        // SelectionFlag bei allen zu 3D Objekten gehoerigen
        // Szenen und deren Objekten auf nicht selektiert setzen
        for(nObjs = 0; nObjs < aSelection.size(); nObjs++)
        {
            E3dCompoundObject* pE3dCompoundObject = dynamic_cast< E3dCompoundObject* >(aSelection[nObjs]);

            if(pE3dCompoundObject)
            {
                // zugehoerige Szene
                pScene = pE3dCompoundObject->GetScene();
                if(pScene)
                    pScene->SetSelected(false);
            }
        }

        // bei allen direkt selektierten Objekten auf selektiert setzen
        for(nObjs = 0; nObjs < aSelection.size(); nObjs++)
        {
            E3dObject* pE3dObject = dynamic_cast< E3dObject* >(aSelection[nObjs]);

            if(pE3dObject)
            {
                // Objekt markieren
                pE3dObject->SetSelected(true);
                pScene = pE3dObject->GetScene();
            }
        }

        if(pScene)
        {
            pScene->SetDrawOnlySelected(true);
            pScene->SingleObjectPainter(rOut); // #110094#-17
            pScene->SetDrawOnlySelected(false);
        }

        // SelectionFlag zuruecksetzen
        for(nObjs = 0; nObjs < aSelection.size(); nObjs++)
        {
            E3dCompoundObject* pE3dCompoundObject = dynamic_cast< E3dCompoundObject* >(aSelection[nObjs]);

            if(pE3dCompoundObject)
            {
                // zugehoerige Szene
                pScene = pE3dCompoundObject->GetScene();

                if(pScene)
                {
                    pScene->SetSelected(false);
                }
            }
        }
    }
    else
    {
        // call parent
        SdrExchangeView::DrawMarkedObj(rOut);
    }
}

/*************************************************************************
|*
|* Model holen ueberladen, da bei einzelnen 3D Objekten noch eine Szene
|* untergeschoben werden muss
|*
\************************************************************************/

SdrModel* E3dView::GetMarkedObjModel() const
{
    // Existieren 3D-Objekte, deren Szenen nicht selektiert sind?
    bool bSpecialHandling(false);
    const SdrObjectVector aSelection(getSelectedSdrObjectVectorFromSdrMarkView());
    sal_uInt32 nObjs(0);
    E3dScene *pScene = 0;

    for(nObjs = 0; nObjs < aSelection.size(); nObjs++)
    {
        const SdrObject* pObj = aSelection[nObjs];
        const E3dCompoundObject* pE3dCompoundObject = dynamic_cast< const E3dCompoundObject* >(pObj);
        const E3dObject* pE3dObject = dynamic_cast< const E3dObject* >(pObj);

        if(!bSpecialHandling && pE3dCompoundObject)
        {
            // if the object is selected, but it's scene not,
            // we need special handling
            pScene = pE3dCompoundObject->GetScene();

            if(pScene && !IsObjMarked(*pScene))
            {
                bSpecialHandling = true;
            }
        }

        if(pE3dObject)
        {
            // reset all selection flags at 3D objects
            pScene = pE3dObject->GetScene();

            if(pScene)
            {
                pScene->SetSelected(false);
            }
        }
    }

    if(!bSpecialHandling)
    {
        // call parent
        return SdrView::GetMarkedObjModel();
    }

    SdrModel* pNewModel = 0;
    basegfx::B2DRange aSelectedSnapRange;

    // set 3d selection flags at all directly selected objects
    // and collect SnapRect of selected objects
    for(nObjs = 0; nObjs < aSelection.size(); nObjs++)
    {
        E3dCompoundObject* pE3dCompoundObject = dynamic_cast< E3dCompoundObject* >(aSelection[nObjs]);

        if(pE3dCompoundObject)
        {
            // mark object, but not scenes
            pE3dCompoundObject->SetSelected(true);

            aSelectedSnapRange.expand(sdr::legacy::GetSnapRange(*pE3dCompoundObject));
        }
    }

    // create new mark list which contains all indirectly selected3d
    // scenes as selected objects
    const_cast< E3dView* >(this)->UnmarkAllObj();

    for(nObjs = 0; nObjs < aSelection.size(); nObjs++)
    {
        const E3dObject* pE3dObject = dynamic_cast< const E3dObject* >(aSelection[nObjs]);

        if(pE3dObject)
        {
            pScene = pE3dObject->GetScene();

            if(pScene && !IsObjMarked(*pScene) && GetSdrPageView())
            {
                const_cast< E3dView* >(this)->MarkObj(*pScene, false);
            }
        }
    }

    // call parent. This will copy all scenes and the selection flags at the 3d objectss. So
    // it will be possible to delete all non-selected 3d objects from the cloned 3d scenes
    pNewModel = SdrView::GetMarkedObjModel();

    if(pNewModel)
    {
        for(sal_uInt32 nPg(0); nPg < pNewModel->GetPageCount(); nPg++)
        {
            const SdrPage* pSrcPg=pNewModel->GetPage(nPg);
            const sal_uInt32 nObAnz(pSrcPg->GetObjCount());

            for(sal_uInt32 nOb(0); nOb < nObAnz; nOb++)
            {
                E3dScene* pE3dScene = dynamic_cast< E3dScene* >(pSrcPg->GetObj(nOb));

                if(pE3dScene)
                {
                    // delete all not intentionally cloned 3d objects
                    pE3dScene->removeAllNonSelectedObjects();

                    // reset select flags and set SnapRect of all selected objects
                    pE3dScene->SetSelected(false);
                    sdr::legacy::SetSnapRange(*pE3dScene, aSelectedSnapRange);
                }
            }
        }
    }

    // restore old selection
    const_cast< E3dView* >(this)->UnmarkAllObj();

    for(nObjs = 0; nObjs < aSelection.size(); nObjs++)
    {
        const_cast< E3dView* >(this)->MarkObj(const_cast< SdrObject& >(*aSelection[nObjs]), false);
    }

    // model zurueckgeben
    return pNewModel;
}

/*************************************************************************
|*
|* Bei Paste muss - falls in eine Scene eingefuegt wird - die
|* Objekte der Szene eingefuegt werden, die Szene selbst aber nicht
|*
\************************************************************************/

bool E3dView::Paste(const SdrModel& rMod, const basegfx::B2DPoint& rPos, SdrObjList* pLst, sal_uInt32 nOptions)
{
    bool bRetval = false;

    // Liste holen
    SdrObjList* pDstList = pLst;
    ImpGetPasteObjList(pDstList);

    if(!pDstList)
        return false;

    // Owner der Liste holen
    E3dScene* pDstScene = dynamic_cast< E3dScene* >(pDstList->getSdrObjectFromSdrObjList());

    if(pDstScene)
    {
        BegUndo(SVX_RESSTR(RID_SVX_3D_UNDO_EXCHANGE_PASTE));

        // Alle Objekte aus E3dScenes kopieren und direkt einfuegen
        for(sal_uInt32 nPg(0); nPg < rMod.GetPageCount(); nPg++)
        {
            const SdrPage* pSrcPg=rMod.GetPage(nPg);
            sal_uInt32 nObAnz(pSrcPg->GetObjCount());

            // Unterobjekte von Szenen einfuegen
            for(sal_uInt32 nOb(0); nOb < nObAnz; nOb++)
            {
                E3dScene* pSrcScene = dynamic_cast< E3dScene* >(pSrcPg->GetObj(nOb));

                if(pSrcScene)
                {
                    ImpCloneAll3DObjectsToDestScene(pSrcScene, pDstScene);
                }
            }
        }
        EndUndo();
    }
    else
    {
        // call parent
        bRetval = SdrView::Paste(rMod, rPos, pLst, nOptions);
    }

    // und Rueckgabewert liefern
    return bRetval;
}

// #83403# Service routine used from local Clone() and from SdrCreateView::EndCreateObj(...)
bool E3dView::ImpCloneAll3DObjectsToDestScene(E3dScene* pSrcScene, E3dScene* pDstScene)
{
    bool bRetval(false);

    if(pSrcScene && pDstScene)
    {
        const sdr::contact::ViewContactOfE3dScene& rVCSceneDst = static_cast< sdr::contact::ViewContactOfE3dScene& >(pDstScene->GetViewContact());
        const drawinglayer::geometry::ViewInformation3D aViewInfo3DDst(rVCSceneDst.getViewInformation3D());
        const sdr::contact::ViewContactOfE3dScene& rVCSceneSrc = static_cast< sdr::contact::ViewContactOfE3dScene& >(pSrcScene->GetViewContact());
        const drawinglayer::geometry::ViewInformation3D aViewInfo3DSrc(rVCSceneSrc.getViewInformation3D());

        for(sal_uInt32 i(0); i < pSrcScene->GetObjCount(); i++)
        {
            E3dCompoundObject* pCompoundObj = dynamic_cast< E3dCompoundObject* >(pSrcScene->GetObj(i));

            if(pCompoundObj)
            {
                // #116235#
                E3dCompoundObject* pNewCompoundObj = dynamic_cast< E3dCompoundObject* >(pCompoundObj->CloneSdrObject());

                if(pNewCompoundObj)
                {
                    // get dest scene's current range in 3D world coordinates
                    const basegfx::B3DHomMatrix aSceneToWorldTrans(pDstScene->GetFullTransform());
                    basegfx::B3DRange aSceneRange(pDstScene->GetBoundVolume());
                    aSceneRange.transform(aSceneToWorldTrans);

                    // get new object's implied object transformation
                    const basegfx::B3DHomMatrix aNewObjectTrans(pNewCompoundObj->GetB3DTransform());

                    // get new object's range in 3D world coordinates in dest scene
                    // as if it were already added
                    const basegfx::B3DHomMatrix aObjectToWorldTrans(aSceneToWorldTrans * aNewObjectTrans);
                    basegfx::B3DRange aObjectRange(pNewCompoundObj->GetBoundVolume());
                    aObjectRange.transform(aObjectToWorldTrans);

                    // get scale adaption
                    const basegfx::B3DVector aSceneScale(aSceneRange.getRange());
                    const basegfx::B3DVector aObjectScale(aObjectRange.getRange());
                    double fScale(1.0);

                    // if new object's size in X,Y or Z is bigger that 80% of dest scene, adapt scale
                    // to not change the scene by the inserted object
                    const double fSizeFactor(0.5);

                    if(aObjectScale.getX() * fScale > aSceneScale.getX() * fSizeFactor)
                    {
                        const double fObjSize(aObjectScale.getX() * fScale);
                        const double fFactor((aSceneScale.getX() * fSizeFactor) / (basegfx::fTools::equalZero(fObjSize) ? 1.0 : fObjSize));
                        fScale *= fFactor;
                    }

                    if(aObjectScale.getY() * fScale > aSceneScale.getY() * fSizeFactor)
                    {
                        const double fObjSize(aObjectScale.getY() * fScale);
                        const double fFactor((aSceneScale.getY() * fSizeFactor) / (basegfx::fTools::equalZero(fObjSize) ? 1.0 : fObjSize));
                        fScale *= fFactor;
                    }

                    if(aObjectScale.getZ() * fScale > aSceneScale.getZ() * fSizeFactor)
                    {
                        const double fObjSize(aObjectScale.getZ() * fScale);
                        const double fFactor((aSceneScale.getZ() * fSizeFactor) / (basegfx::fTools::equalZero(fObjSize) ? 1.0 : fObjSize));
                        fScale *= fFactor;
                    }

                    // get translation adaption
                    const basegfx::B3DPoint aSceneCenter(aSceneRange.getCenter());
                    const basegfx::B3DPoint aObjectCenter(aObjectRange.getCenter());

                    // build full modification transform. The object's transformation
                    // shall be modified, so start at object coordinates; transform to 3d world coor
                    basegfx::B3DHomMatrix aModifyingTransform(aObjectToWorldTrans);

                    // translate to absolute center in 3d world coor
                    aModifyingTransform.translate(-aObjectCenter);

                    // scale to dest size in 3d world coor
                    aModifyingTransform.scale(fScale, fScale, fScale);

                    // translate to dest scene center in 3d world coor
                    aModifyingTransform.translate(aSceneCenter);

                    // transform from 3d world to dest object coordinates
                    basegfx::B3DHomMatrix aWorldToObject(aObjectToWorldTrans);
                    aWorldToObject.invert();
                    aModifyingTransform = aWorldToObject * aModifyingTransform;

                    // correct implied object transform by applying changing one in object coor
                    pNewCompoundObj->SetB3DTransform(aModifyingTransform * aNewObjectTrans);

                    // fill and insert new object
                    pNewCompoundObj->SetLayer(pCompoundObj->GetLayer());
                    pNewCompoundObj->SetStyleSheet(pCompoundObj->GetStyleSheet(), true);
                    pDstScene->Insert3DObj(*pNewCompoundObj);
                    bRetval = true;

                    // Undo anlegen
                    if( getSdrModelFromSdrView().IsUndoEnabled() )
                    {
                        AddUndo(getSdrModelFromSdrView().GetSdrUndoFactory().CreateUndoNewObject(*pNewCompoundObj));
                    }
                }
            }
        }
    }

    return bRetval;
}

/*************************************************************************
|*
|* 3D-Konvertierung moeglich?
|*
\************************************************************************/

bool E3dView::IsConvertTo3DObjPossible() const
{
    bool bAny3D(false);
    bool bGroupSelected(false);
    bool bRetval(true);
    const SdrObjectVector aSelection(getSelectedSdrObjectVectorFromSdrMarkView());

    for(sal_uInt32 a(0); !bAny3D && a < aSelection.size(); a++)
    {
        SdrObject *pObj = aSelection[a];
            ImpIsConvertTo3DPossible(pObj, bAny3D, bGroupSelected);
    }

    bRetval = !bAny3D
        && (
           IsConvertToPolyObjPossible(false)
        || IsConvertToPathObjPossible(false)
        || IsImportMtfPossible());

    return bRetval;
}

void E3dView::ImpIsConvertTo3DPossible(SdrObject* pObj, bool& rAny3D,
    bool& rGroupSelected) const
{
    if(pObj)
    {
        if(dynamic_cast< E3dObject* >(pObj))
        {
            rAny3D = true;
        }
        else
        {
            if(pObj->getChildrenOfSdrObject())
            {
                SdrObjListIter aIter(*pObj->getChildrenOfSdrObject(), IM_DEEPNOGROUPS);
                while(aIter.IsMore())
                {
                    SdrObject* pNewObj = aIter.Next();
                    ImpIsConvertTo3DPossible(pNewObj, rAny3D, rGroupSelected);
                }
                rGroupSelected = true;
            }
        }
    }
}

/*************************************************************************
|*
|* 3D-Konvertierung zu Extrude ausfuehren
|*
\************************************************************************/

void E3dView::ImpChangeSomeAttributesFor3DConversion(SdrObject* pObj)
{
    SdrTextObj* pSdrTextObj = dynamic_cast< SdrTextObj* >(pObj);

    if(pSdrTextObj)
    {
        const SfxItemSet& rSet = pSdrTextObj->GetMergedItemSet();
        const SvxColorItem& rTextColorItem = (const SvxColorItem&)rSet.Get(EE_CHAR_COLOR);
        if(rTextColorItem.GetValue() == RGB_Color(COL_BLACK))
        {
            // Bei schwarzen Textobjekten wird die Farbe auf grau gesetzt
            if(pSdrTextObj->getSdrPageFromSdrObject())
            {
                // #84864# if black is only default attribute from
                // pattern set it hard so that it is used in undo.
                pSdrTextObj->SetMergedItem(SvxColorItem(RGB_Color(COL_BLACK), EE_CHAR_COLOR));

                // add undo now
                if( getSdrModelFromSdrView().IsUndoEnabled() )
                {
                    AddUndo(getSdrModelFromSdrView().GetSdrUndoFactory().CreateUndoAttrObject(*pSdrTextObj, false, false));
                }
            }

            pSdrTextObj->SetMergedItem(SvxColorItem(RGB_Color(COL_GRAY), EE_CHAR_COLOR));
        }
    }
}

void E3dView::ImpChangeSomeAttributesFor3DConversion2(SdrObject* pObj)
{
    SdrPathObj* pSdrPathObj = dynamic_cast< SdrPathObj* >(pObj);

    if(pSdrPathObj)
    {
        const SfxItemSet& rSet = pSdrPathObj->GetMergedItemSet();
        sal_Int32 nLineWidth = ((const XLineWidthItem&)(rSet.Get(XATTR_LINEWIDTH))).GetValue();
        XLineStyle eLineStyle = (XLineStyle)((const XLineStyleItem&)rSet.Get(XATTR_LINESTYLE)).GetValue();
        XFillStyle eFillStyle = ITEMVALUE(rSet, XATTR_FILLSTYLE, XFillStyleItem);

        if(pSdrPathObj->isClosed()
            && eLineStyle == XLINE_SOLID
            && !nLineWidth
            && eFillStyle != XFILL_NONE)
        {
            if(pSdrPathObj->getSdrPageFromSdrObject() && getSdrModelFromSdrView().IsUndoEnabled() )
            {
                AddUndo(getSdrModelFromSdrView().GetSdrUndoFactory().CreateUndoAttrObject(*pSdrPathObj, false, false));
            }

            pSdrPathObj->SetMergedItem(XLineStyleItem(XLINE_NONE));
            pSdrPathObj->SetMergedItem(XLineWidthItem(0L));
        }
    }
}

void E3dView::ImpCreateSingle3DObjectFlat(E3dScene* pScene, SdrObject* pObj, bool bExtrude, double fDepth, basegfx::B2DHomMatrix& rLatheMat)
{
    // Einzelnes PathObject, dieses umwanden
    SdrPathObj* pPath = dynamic_cast< SdrPathObj* >(pObj);

    if(pPath)
    {
        E3dDefaultAttributes aDefault = Get3DDefaultAttributes();
        if(bExtrude)
            aDefault.SetDefaultExtrudeCharacterMode(true);
        else
            aDefault.SetDefaultLatheCharacterMode(true);

        // ItemSet des Ursprungsobjektes holen
        SfxItemSet aSet(pObj->GetMergedItemSet());

        XFillStyle eFillStyle = ITEMVALUE(aSet, XATTR_FILLSTYLE, XFillStyleItem);

        // Linienstil ausschalten
        aSet.Put(XLineStyleItem(XLINE_NONE));

        // Feststellen, ob ein FILL_Attribut gesetzt ist.
        if(!pPath->isClosed() || eFillStyle == XFILL_NONE)
        {
            // Das SdrPathObj ist nicht gefuellt, lasse die
            // vordere und hintere Flaeche weg. Ausserdem ist
            // eine beidseitige Darstellung notwendig.
            aDefault.SetDefaultExtrudeCloseFront(false);
            aDefault.SetDefaultExtrudeCloseBack(false);

            aSet.Put(SfxBoolItem(SDRATTR_3DOBJ_DOUBLE_SIDED, true));

            // Fuellattribut setzen
            aSet.Put(XFillStyleItem(XFILL_SOLID));

            // Fuellfarbe muss auf Linienfarbe, da das Objekt vorher
            // nur eine Linie war
            Color aColorLine = ((const XLineColorItem&)(aSet.Get(XATTR_LINECOLOR))).GetColorValue();
            aSet.Put(XFillColorItem(String(), aColorLine));
        }

        // Neues Extrude-Objekt erzeugen
        E3dObject* p3DObj = NULL;

        if(bExtrude)
        {
            p3DObj = new E3dExtrudeObj(
                getSdrModelFromSdrView(),
                aDefault,
                pPath->getB2DPolyPolygonInObjectCoordinates(),
                fDepth);
        }
        else
        {
            basegfx::B2DPolyPolygon aPolyPoly2D(pPath->getB2DPolyPolygonInObjectCoordinates());
            aPolyPoly2D.transform(rLatheMat);
            p3DObj = new E3dLatheObj(
                getSdrModelFromSdrView(),
                aDefault,
                aPolyPoly2D);
        }

        // Attribute setzen
        if(p3DObj)
        {
            p3DObj->SetLayer(pObj->GetLayer());
            p3DObj->SetMergedItemSet(aSet);
            p3DObj->SetStyleSheet(pObj->GetStyleSheet(), true);

            // Neues 3D-Objekt einfuegen
            pScene->Insert3DObj(*p3DObj);
        }
    }
}

void E3dView::ImpCreate3DObject(E3dScene* pScene, SdrObject* pObj, bool bExtrude, double fDepth, basegfx::B2DHomMatrix& rLatheMat)
{
    if(pObj)
    {
        // change text color attribute for not so dark colors
        if(pObj->getChildrenOfSdrObject())
        {
            SdrObjListIter aIter(*pObj->getChildrenOfSdrObject(), IM_DEEPWITHGROUPS);
            while(aIter.IsMore())
            {
                SdrObject* pGroupMember = aIter.Next();
                ImpChangeSomeAttributesFor3DConversion(pGroupMember);
            }
        }
        else
            ImpChangeSomeAttributesFor3DConversion(pObj);

        // convert completely to path objects
        SdrObject* pNewObj1 = pObj->ConvertToPolyObj(false, false);

        if(pNewObj1)
        {
            // change text color attribute for not so dark colors
            if(pNewObj1->getChildrenOfSdrObject())
            {
                SdrObjListIter aIter(*pNewObj1->getChildrenOfSdrObject(), IM_DEEPWITHGROUPS);
                while(aIter.IsMore())
                {
                    SdrObject* pGroupMember = aIter.Next();
                    ImpChangeSomeAttributesFor3DConversion2(pGroupMember);
                }
            }
            else
                ImpChangeSomeAttributesFor3DConversion2(pNewObj1);

            // convert completely to path objects
            SdrObject* pNewObj2 = pObj->ConvertToContourObj(pNewObj1, true);

            if(pNewObj2)
            {
                // add all to flat scene
                if(pNewObj2->getChildrenOfSdrObject())
                {
                    SdrObjListIter aIter(*pNewObj2->getChildrenOfSdrObject(), IM_DEEPWITHGROUPS);
                    while(aIter.IsMore())
                    {
                        SdrObject* pGroupMember = aIter.Next();
                        ImpCreateSingle3DObjectFlat(pScene, pGroupMember, bExtrude, fDepth, rLatheMat);
                    }
                }
                else
                    ImpCreateSingle3DObjectFlat(pScene, pNewObj2, bExtrude, fDepth, rLatheMat);

                // delete zwi object
                if(pNewObj2 != pObj && pNewObj2 != pNewObj1 && pNewObj2)
                {
                    deleteSdrObjectSafeAndClearPointer( pNewObj2 );
                }
            }

            // delete zwi object
            if(pNewObj1 != pObj && pNewObj1)
            {
                deleteSdrObjectSafeAndClearPointer( pNewObj1 );
            }
        }
    }
}

/*************************************************************************
|*
|* 3D-Konvertierung zu Extrude steuern
|*
\************************************************************************/

void E3dView::ConvertMarkedObjTo3D(bool bExtrude, basegfx::B2DPoint aPnt1, basegfx::B2DPoint aPnt2)
{
    if(areSdrObjectsSelected())
    {
        // Undo anlegen
        if(bExtrude)
            BegUndo(SVX_RESSTR(RID_SVX_3D_UNDO_EXTRUDE));
        else
            BegUndo(SVX_RESSTR(RID_SVX_3D_UNDO_LATHE));

        // Neue Szene fuer zu erzeugende 3D-Objekte anlegen
        E3dScene* pScene = new E3dScene(
            getSdrModelFromSdrView(),
            Get3DDefaultAttributes());

        // Rechteck bestimmen und evtl. korrigieren
        basegfx::B2DRange aRange(getMarkedObjectSnapRange());

        if(aRange.getWidth() < 500.0 || aRange.getHeight() < 500.0)
        {
            basegfx::B2DRange aCentered(aRange.getCenter());

            aCentered.grow(500.0);
            aRange.expand(aCentered);
        }

        // Tiefe relativ zur Groesse der Selektion bestimmen
        double fDepth(0.0);
        double fRot3D(0.0);
        basegfx::B2DHomMatrix aLatheMat;

        if(bExtrude)
        {
            double fW = aRange.getWidth();
            double fH = aRange.getHeight();
            fDepth = sqrt(fW*fW + fH*fH) / 6.0;
        }

        const SdrObjectVector aSelection(getSelectedSdrObjectVectorFromSdrMarkView());

        if(!bExtrude)
        {
            // Transformation fuer Polygone Rotationskoerper erstellen
            if(aPnt1 != aPnt2)
            {
                // Rotation um Kontrollpunkt1 mit eigestelltem Winkel
                // fuer 3D Koordinaten
                basegfx::B2DPoint aDiff(aPnt1 - aPnt2);
                fRot3D = atan2(aDiff.getY(), aDiff.getX()) - F_PI2;

                if(basegfx::fTools::equalZero(fabs(fRot3D)))
                {
                    fRot3D = 0.0;
                }

                if(fRot3D != 0.0)
                {
                    aLatheMat = basegfx::tools::createRotateAroundPoint(aPnt2, -fRot3D) * aLatheMat;
                }
            }

            if(aPnt2.getX() != 0.0)
            {
                // Translation auf Y=0 - Achse
                aLatheMat.translate(-aPnt2.getX(), 0.0);
            }
            else
            {
                aLatheMat.translate(-aRange.getMinX(), 0.0);
            }

            // Inverse Matrix bilden, um die Zielausdehnung zu bestimmen
            basegfx::B2DHomMatrix aInvLatheMat(aLatheMat);
            aInvLatheMat.invert();

            // SnapRect Ausdehnung mittels Spiegelung an der Rotationsachse
            // erweitern
            for(sal_uInt32 a(0); a < aSelection.size(); a++)
            {
                const SdrObject* pObj = aSelection[a];
                const basegfx::B2DRange aSnapRange(sdr::legacy::GetSnapRange(*pObj));
                basegfx::B2DPoint aRot;
                Point aRotPnt;

                aRot = basegfx::B2DPoint(aSnapRange.getMinX(), -aSnapRange.getMinY());
                aRot *= aLatheMat;
                aRot.setX(-aRot.getX());
                aRot *= aInvLatheMat;
                aRange.expand(aRot);

                aRot = basegfx::B2DPoint(aSnapRange.getMinX(), -aSnapRange.getMaxY());
                aRot *= aLatheMat;
                aRot.setX(-aRot.getX());
                aRot *= aInvLatheMat;
                aRange.expand(aRot);

                aRot = basegfx::B2DPoint(aSnapRange.getMaxX(), -aSnapRange.getMinY());
                aRot *= aLatheMat;
                aRot.setX(-aRot.getX());
                aRot *= aInvLatheMat;
                aRange.expand(aRot);

                aRot = basegfx::B2DPoint(aSnapRange.getMaxX(), -aSnapRange.getMaxY());
                aRot *= aLatheMat;
                aRot.setX(-aRot.getX());
                aRot *= aInvLatheMat;
                aRange.expand(aRot);
            }
        }

        // Ueber die Selektion gehen und in 3D wandeln, komplett mit
        // Umwandeln in SdrPathObject, auch Schriften
        for(sal_uInt32 a(0); a < aSelection.size(); a++)
        {
            ImpCreate3DObject(pScene, aSelection[a], bExtrude, fDepth, aLatheMat);
        }

        if(pScene->GetObjCount())
        {
            // Alle angelegten Objekte Tiefenarrangieren
            if(bExtrude)
                DoDepthArrange(pScene, fDepth);

            // 3D-Objekte auf die Mitte des Gesamtrechtecks zentrieren
            basegfx::B3DPoint aCenter(pScene->GetBoundVolume().getCenter());
            basegfx::B3DHomMatrix aMatrix;

            aMatrix.translate(-aCenter);
            pScene->SetB3DTransform(aMatrix * pScene->GetB3DTransform()); // #112587#

            // Szene initialisieren
            sdr::legacy::SetSnapRange(*pScene, aRange);
            const basegfx::B3DRange aBoundVol(pScene->GetBoundVolume());
            InitScene(pScene, aRange.getWidth(), aRange.getHeight(), aBoundVol.getDepth());

            // Szene anstelle des ersten selektierten Objektes einfuegen
            // und alle alten Objekte weghauen
            SdrObject* pRepObj = aSelection[0];
            MarkObj(*pRepObj, true);
            ReplaceObjectAtView(*pRepObj, *pScene, false);
            DeleteMarked();
            MarkObj(*pScene);

            // Rotationskoerper um Rotationsachse drehen
            basegfx::B3DHomMatrix aRotate;

            if(!bExtrude && fRot3D != 0.0)
            {
                aRotate.rotate(0.0, 0.0, fRot3D);
            }

            // Default-Rotation setzen
            {
                double XRotateDefault = 20;
                aRotate.rotate(DEG2RAD(XRotateDefault), 0.0, 0.0);
            }

            if(!aRotate.isIdentity())
            {
                pScene->SetB3DTransform(aRotate * pScene->GetB3DTransform());
            }

            // SnapRects der Objekte ungueltig
            sdr::legacy::SetSnapRange(*pScene, aRange);
        }
        else
        {
            // Es wurden keine 3D Objekte erzeugt, schmeiss alles weg
            deleteSdrObjectSafeAndClearPointer(pScene);
        }

        // Undo abschliessen
        EndUndo();
    }
}

/*************************************************************************
|*
|* Alle enthaltenen Extrude-Objekte Tiefenarrangieren
|*
\************************************************************************/

struct E3dDepthNeighbour
{
    E3dDepthNeighbour*          mpNext;
    E3dExtrudeObj*              mpObj;
    basegfx::B2DPolyPolygon     maPreparedPolyPolygon;

    E3dDepthNeighbour()
    :   mpNext(0),
        mpObj(0),
        maPreparedPolyPolygon()
    {
    }
};

struct E3dDepthLayer
{
    E3dDepthLayer*              mpDown;
    E3dDepthNeighbour*          mpNext;

    E3dDepthLayer()
    :   mpDown(0),
        mpNext(0)
    {
    }

    ~E3dDepthLayer()
    {
        while(mpNext)
        {
            E3dDepthNeighbour* pSucc = mpNext->mpNext;
            delete mpNext;
            mpNext = pSucc;
        }
    }
};

void E3dView::DoDepthArrange(E3dScene* pScene, double fDepth)
{
    if(pScene && pScene->GetObjCount() > 1)
    {
        SdrObjList* pSubList = pScene->getChildrenOfSdrObject();
        SdrObjListIter aIter(*pSubList, IM_FLAT);
        E3dDepthLayer* pBaseLayer = NULL;
        E3dDepthLayer* pLayer = NULL;
        sal_Int32 nNumLayers = 0;

        while(aIter.IsMore())
        {
            E3dExtrudeObj* pExtrudeObj = dynamic_cast< E3dExtrudeObj* >(aIter.Next());

            if(pExtrudeObj)
            {
                const basegfx::B2DPolyPolygon aExtrudePoly(
                    basegfx::tools::prepareForPolygonOperation(pExtrudeObj->GetExtrudePolygon()));
                const SfxItemSet& rLocalSet = pExtrudeObj->GetMergedItemSet();
                const XFillStyle eLocalFillStyle = ITEMVALUE(rLocalSet, XATTR_FILLSTYLE, XFillStyleItem);
                const Color aLocalColor = ((const XFillColorItem&)(rLocalSet.Get(XATTR_FILLCOLOR))).GetColorValue();

                // sort in ExtrudeObj
                if(pLayer)
                {
                    // do we have overlap with an object of this layer?
                    bool bOverlap(false);
                    E3dDepthNeighbour* pAct = pLayer->mpNext;

                    while(!bOverlap && pAct)
                    {
                        // do pAct->mpObj and pExtrudeObj overlap? Check by
                        // using logical AND clipping
                        const basegfx::B2DPolyPolygon aAndPolyPolygon(
                            basegfx::tools::solvePolygonOperationAnd(
                                aExtrudePoly,
                                pAct->maPreparedPolyPolygon));

                        bOverlap = (0 != aAndPolyPolygon.count());

                        if(bOverlap)
                        {
                            // second ciriteria: is another fillstyle or color used?
                            const SfxItemSet& rCompareSet = pAct->mpObj->GetMergedItemSet();

                            XFillStyle eCompareFillStyle = ITEMVALUE(rCompareSet, XATTR_FILLSTYLE, XFillStyleItem);

                            if(eLocalFillStyle == eCompareFillStyle)
                            {
                                if(eLocalFillStyle == XFILL_SOLID)
                                {
                                    Color aCompareColor = ((const XFillColorItem&)(rCompareSet.Get(XATTR_FILLCOLOR))).GetColorValue();

                                    if(aCompareColor == aLocalColor)
                                    {
                                        bOverlap = false;
                                    }
                                }
                                else if(eLocalFillStyle == XFILL_NONE)
                                {
                                    bOverlap = false;
                                }
                            }
                        }

                        pAct = pAct->mpNext;
                    }

                    if(bOverlap)
                    {
                        // yes, start a new layer
                        pLayer->mpDown = new E3dDepthLayer;
                        pLayer = pLayer->mpDown;
                        nNumLayers++;
                        pLayer->mpNext = new E3dDepthNeighbour;
                        pLayer->mpNext->mpObj = pExtrudeObj;
                        pLayer->mpNext->maPreparedPolyPolygon = aExtrudePoly;
                    }
                    else
                    {
                        // no, add to current layer
                        E3dDepthNeighbour* pNewNext = new E3dDepthNeighbour;
                        pNewNext->mpObj = pExtrudeObj;
                        pNewNext->maPreparedPolyPolygon = aExtrudePoly;
                        pNewNext->mpNext = pLayer->mpNext;
                        pLayer->mpNext = pNewNext;
                    }
                }
                else
                {
                    // first layer ever
                    pBaseLayer = new E3dDepthLayer;
                    pLayer = pBaseLayer;
                    nNumLayers++;
                    pLayer->mpNext = new E3dDepthNeighbour;
                    pLayer->mpNext->mpObj = pExtrudeObj;
                    pLayer->mpNext->maPreparedPolyPolygon = aExtrudePoly;
                }
            }
        }

        // number of layers is done
        if(nNumLayers > 1)
        {
            // need to be arranged
            double fMinDepth = fDepth * 0.8;
            double fStep = (fDepth - fMinDepth) / (double)nNumLayers;
            pLayer = pBaseLayer;

            while(pLayer)
            {
                // move along layer
                E3dDepthNeighbour* pAct = pLayer->mpNext;

                while(pAct)
                {
                    // adapt extrude value
                    pAct->mpObj->SetMergedItem(SfxUInt32Item(SDRATTR_3DOBJ_DEPTH, sal_uInt32(fMinDepth + 0.5)));

                    // next
                    pAct = pAct->mpNext;
                }

                // next layer
                pLayer = pLayer->mpDown;
                fMinDepth += fStep;
            }
        }

        // cleanup
        while(pBaseLayer)
        {
            pLayer = pBaseLayer->mpDown;
            delete pBaseLayer;
            pBaseLayer = pLayer;
        }
    }
}

/*************************************************************************
|*
|* Drag beginnen, vorher ggf. Drag-Methode fuer 3D-Objekte erzeugen
|*
\************************************************************************/

bool E3dView::BegDragObj(const basegfx::B2DPoint& rPnt, const SdrHdl* pHdl, double fMinMovLogic, SdrDragMethod* pForcedMeth)
{
    if(Is3DRotationCreationActive() && areSdrObjectsSelected())
    {
        // bestimme alle selektierten Polygone und gebe die gespiegelte Hilfsfigur aus
        mpMirrorOverlay->SetMirrorAxis(GetRef1(), GetRef2());
    }
    else
    {
        bool bOwnActionNecessary;
        if (pHdl == NULL)
        {
           bOwnActionNecessary = true;
        }
        else if (pHdl->IsVertexHdl() || pHdl->IsCornerHdl())
        {
           bOwnActionNecessary = true;
        }
        else
        {
           bOwnActionNecessary = false;
        }

        if(bOwnActionNecessary && areSdrObjectsSelected())
        {
            E3dDragConstraint eConstraint = E3DDRAG_CONSTR_XYZ;
            bool bThereAreRootScenes = false;
            bool bThereAre3DObjects = false;
            const SdrObjectVector aSelection(getSelectedSdrObjectVectorFromSdrMarkView());

            for(sal_Int32 nObjs(0); nObjs < aSelection.size(); nObjs++)
            {
                SdrObject *pObj = aSelection[nObjs];
                E3dScene* pE3dScene = dynamic_cast< E3dScene* >(pObj);
                E3dObject* pE3dObject = dynamic_cast< E3dObject* >(pObj);

                if(pE3dScene && pE3dScene->GetScene() == pE3dScene)
                    bThereAreRootScenes = true;

                if(pE3dObject)
                    bThereAre3DObjects = true;
            }

            if( bThereAre3DObjects )
            {
                meDragHdl = (!pHdl ? HDL_MOVE : pHdl->GetKind());

                switch ( GetDragMode() )
                {
                    case SDRDRAG_ROTATE:
                    case SDRDRAG_SHEAR:
                    {
                        switch (GetDragHdlKind())
                        {
                            case HDL_LEFT:
                            case HDL_RIGHT:
                            {
                                eConstraint = E3DDRAG_CONSTR_X;
                            }
                            break;

                            case HDL_UPPER:
                            case HDL_LOWER:
                            {
                                eConstraint = E3DDRAG_CONSTR_Y;
                            }
                            break;

                            case HDL_UPLFT:
                            case HDL_UPRGT:
                            case HDL_LWLFT:
                            case HDL_LWRGT:
                            {
                                eConstraint = E3DDRAG_CONSTR_Z;
                            }
                            break;
                            default: break;
                        }

                        // die nicht erlaubten Rotationen ausmaskieren
                        eConstraint = E3dDragConstraint(eConstraint& eDragConstraint);
                        pForcedMeth = new E3dDragRotate(*this, aSelection, eConstraint, IsSolidDragging());
                    }
                    break;

                    case SDRDRAG_MOVE:
                    {
                        if(!bThereAreRootScenes)
                        {
                            pForcedMeth = new E3dDragMove(*this, aSelection, GetDragHdlKind(), eConstraint, IsSolidDragging());
                        }
                    }
                    break;

                    // spaeter mal
                    case SDRDRAG_MIRROR:
                    case SDRDRAG_CROOK:
                    case SDRDRAG_DISTORT:
                    case SDRDRAG_TRANSPARENCE:
                    case SDRDRAG_GRADIENT:
                    default:
                    {
                    }
                    break;
                }
            }
        }
    }

    return SdrView::BegDragObj(rPnt, pHdl, fMinMovLogic, pForcedMeth);
}

/*************************************************************************
|*
|* Pruefen, obj 3D-Szene markiert ist
|*
\************************************************************************/

bool E3dView::HasMarkedScene()
{
    return (GetMarkedScene() != NULL);
}

/*************************************************************************
|*
|* Pruefen, obj 3D-Szene markiert ist
|*
\************************************************************************/

E3dScene* E3dView::GetMarkedScene()
{
    const SdrObjectVector aSelection(getSelectedSdrObjectVectorFromSdrMarkView());

    for ( sal_uInt32 i(0); i < aSelection.size(); i++ )
    {
        E3dScene* pE3dScene = dynamic_cast< E3dScene* >(aSelection[i]);

        if (pE3dScene)
        {
            return pE3dScene;
        }
    }

    return 0;
}

/*************************************************************************
|*
|* aktuelles 3D-Zeichenobjekt setzen, dafuer Szene erzeugen
|*
\************************************************************************/

E3dScene* E3dView::SetCurrent3DObj(E3dObject* p3DObj)
{
    DBG_ASSERT(p3DObj != NULL, "Nana, wer steckt denn hier 'nen NULL-Zeiger rein?");

    // get transformed BoundVolume of the object
    basegfx::B3DRange aVolume(p3DObj->GetBoundVolume());
    aVolume.transform(p3DObj->GetB3DTransform());
    double fW(aVolume.getWidth());
    double fH(aVolume.getHeight());

    E3dScene* pScene = new E3dScene(
        getSdrModelFromSdrView(),
        Get3DDefaultAttributes());

    InitScene(pScene, fW, fH, aVolume.getMaxZ() + ((fW + fH) / 4.0));
    pScene->Insert3DObj(*p3DObj);
    sdr::legacy::SetSnapRange(*pScene, basegfx::B2DRange(0.0, 0.0, fW, fH));

    return pScene;
}

/*************************************************************************
|*
|* neu erzeugte Szene initialisieren
|*
\************************************************************************/

void E3dView::InitScene(E3dScene* pScene, double fW, double fH, double fCamZ)
{
    Camera3D aCam(pScene->GetCamera());

    aCam.SetAutoAdjustProjection(false);
    aCam.SetViewWindow(- fW / 2, - fH / 2, fW, fH);
    basegfx::B3DPoint aLookAt;

    double fDefaultCamPosZ = GetDefaultCamPosZ();
    basegfx::B3DPoint aCamPos(0.0, 0.0, fCamZ < fDefaultCamPosZ ? fDefaultCamPosZ : fCamZ);

    aCam.SetPosAndLookAt(aCamPos, aLookAt);
    aCam.SetFocalLength(GetDefaultCamFocal());
    aCam.SetDefaults(basegfx::B3DPoint(0.0, 0.0, fDefaultCamPosZ), aLookAt, GetDefaultCamFocal());
    pScene->SetCamera(aCam);
}

/*************************************************************************
|*
|* startsequenz fuer die erstellung eines 3D-Rotationskoerpers
|*
\************************************************************************/

void E3dView::Start3DCreation()
{
    if (areSdrObjectsSelected())
    {
        // bestimme die koordinaten fuer JOEs Mirrorachse
        // entgegen der normalen Achse wird diese an die linke Seite des Objektes
        // positioniert
        sal_Int32         nOutMin = 0;
        sal_Int32         nOutMax = 0;
        sal_Int32         nMinLen = 0;
        sal_Int32         nObjDst = 0;
        sal_Int32         nOutHgt = 0;
        OutputDevice* pOut    = GetFirstOutputDevice(); //GetWin(0);

        // erstmal Darstellungsgrenzen bestimmen
        if (pOut != NULL)
        {
            nMinLen = pOut->PixelToLogic(Size(0,50)).Height();
            nObjDst = pOut->PixelToLogic(Size(0,20)).Height();

            sal_Int32 nDst = pOut->PixelToLogic(Size(0,10)).Height();

            nOutMin =  -pOut->GetMapMode().GetOrigin().Y();
            nOutMax =  pOut->GetOutputSize().Height() - 1 + nOutMin;
            nOutMin += nDst;
            nOutMax -= nDst;

            if (nOutMax - nOutMin < nDst)
            {
                nOutMin += nOutMax + 1;
                nOutMin /= 2;
                nOutMin -= (nDst + 1) / 2;
                nOutMax  = nOutMin + nDst;
            }

            nOutHgt = nOutMax - nOutMin;

            sal_Int32 nTemp = nOutHgt / 4;
            if (nTemp > nMinLen) nMinLen = nTemp;
        }

        // und dann die Markierungen oben und unten an das Objekt heften
        basegfx::B2DRange aR;
        const SdrObjectVector aSelection(getSelectedSdrObjectVectorFromSdrMarkView());

        for(sal_uInt32 nMark(0); nMark < aSelection.size(); nMark++)
        {
            SdrObject* pMark = aSelection[nMark];
            basegfx::B2DPolyPolygon aXPP(pMark->TakeXorPoly());
            aR.expand(basegfx::tools::getRange(aXPP));
        }

        basegfx::B2DPoint aCenter(aR.getCenter());
        sal_Int32     nMarkHgt = basegfx::fround(aR.getHeight()) - 1;
        sal_Int32     nHgt     = nMarkHgt + nObjDst * 2;

        if (nHgt < nMinLen) nHgt = nMinLen;

        sal_Int32 nY1 = basegfx::fround(aCenter.getY()) - (nHgt + 1) / 2;
        sal_Int32 nY2 = nY1 + nHgt;

        if (pOut && (nMinLen > nOutHgt)) nMinLen = nOutHgt;
        if (pOut)
        {
            if (nY1 < nOutMin)
            {
                nY1 = nOutMin;
                if (nY2 < nY1 + nMinLen) nY2 = nY1 + nMinLen;
            }
            if (nY2 > nOutMax)
            {
                nY2 = nOutMax;
                if (nY1 > nY2 - nMinLen) nY1 = nY2 - nMinLen;
            }
        }

        SetRef1(basegfx::B2DPoint(aR.getMinX(), nY1));    // Initial Achse um 2/100mm nach links
        SetRef2(basegfx::B2DPoint(GetRef1().getX(), nY2));

        // force re-creation of SdrHdls TTTT needed?
        SetMarkHandles();

        // SpiegelPolygone SOFORT zeigen
        const SdrHdlList &aHdlList = GetHdlList();

        mpMirrorOverlay = new Impl3DMirrorConstructOverlay(*this);
        mpMirrorOverlay->SetMirrorAxis(aHdlList.GetHdlByKind(HDL_REF1)->getPosition(), aHdlList.GetHdlByKind(HDL_REF2)->getPosition());
    }
}

/*************************************************************************
|*
|* was passiert bei einer Mausbewegung, wenn das Objekt erstellt wird ?
|*
\************************************************************************/

void E3dView::MovAction(const basegfx::B2DPoint& rPnt)
{
    if(Is3DRotationCreationActive())
    {
        SdrHdl* pHdl = GetDragHdl();

        if (pHdl)
        {
            SdrHdlKind eHdlKind = pHdl->GetKind();

            // reagiere nur bei einer spiegelachse
            if ((eHdlKind == HDL_REF1) ||
                (eHdlKind == HDL_REF2) ||
                (eHdlKind == HDL_MIRX))
            {
                const SdrHdlList &aHdlList = GetHdlList ();

                // loesche das gespiegelte Polygon, spiegele das Original und zeichne es neu
                //ShowMirrored ();
                SdrView::MovAction (rPnt);
                mpMirrorOverlay->SetMirrorAxis(
                    aHdlList.GetHdlByKind(HDL_REF1)->getPosition(),
                    aHdlList.GetHdlByKind(HDL_REF2)->getPosition());
            }
        }
        else
        {
            SdrView::MovAction (rPnt);
        }
    }
    else
    {
        SdrView::MovAction (rPnt);
    }
}

/*************************************************************************
|*
|* Schluss. Objekt und evtl. Unterobjekte ueber ImpCreate3DLathe erstellen
|*          [FG] Mit dem Parameterwert true (SDefault: false) wird einfach ein
|*               Rotationskoerper erzeugt, ohne den Benutzer die Lage der
|*               Achse fetlegen zu lassen. Es reicht dieser Aufruf, falls
|*               ein Objekt selektiert ist. (keine Initialisierung noetig)
|*
\************************************************************************/

void E3dView::End3DCreation(bool bUseDefaultValuesForMirrorAxes)
{
    ResetCreationActive();

    if(areSdrObjectsSelected())
    {
        if(bUseDefaultValuesForMirrorAxes)
        {
            basegfx::B2DRange aRange(getMarkedObjectSnapRange());

            if(aRange.getWidth() < 500.0 || aRange.getHeight() < 500.0)
            {
                basegfx::B2DRange aCentered(aRange.getCenter());

                aCentered.grow(500.0);
                aRange.expand(aCentered);
            }

            const basegfx::B2DPoint aPnt1(aRange.getMinX(), -aRange.getMinY());
            const basegfx::B2DPoint aPnt2(aRange.getMinX(), -aRange.getMaxY());

            ConvertMarkedObjTo3D(false, aPnt1, aPnt2);
        }
        else
        {
            // Hilfsfigur ausschalten
            // bestimme aus den Handlepositionen und den Versatz der Punkte
            const SdrHdlList &aHdlList = GetHdlList();
            const basegfx::B2DPoint aMirrorRef1 = aHdlList.GetHdlByKind(HDL_REF1)->getPosition();
            const basegfx::B2DPoint aMirrorRef2 = aHdlList.GetHdlByKind(HDL_REF2)->getPosition();
            const basegfx::B2DPoint aPnt1(aMirrorRef1.getX(), -aMirrorRef1.getY());
            const basegfx::B2DPoint aPnt2(aMirrorRef2.getX(), -aMirrorRef2.getY());

            ConvertMarkedObjTo3D(false, aPnt1, aPnt2);
        }
    }
}

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

E3dView::~E3dView ()
{
}

/*************************************************************************
|*
|* beende das erzeugen und loesche die polygone
|*
\************************************************************************/

void E3dView::ResetCreationActive ()
{
    if(mpMirrorOverlay)
    {
        delete mpMirrorOverlay;
        mpMirrorOverlay = 0L;
    }
}

/*************************************************************************
|*
|* Klasse initialisieren
|*
\************************************************************************/

void E3dView::InitView ()
{
    eDragConstraint          = E3DDRAG_CONSTR_XYZ;
    fDefaultScaleX           =
    fDefaultScaleY           =
    fDefaultScaleZ           = 1.0;
    fDefaultRotateX          =
    fDefaultRotateY          =
    fDefaultRotateZ          = 0.0;
    fDefaultExtrusionDeepth  = 1000; // old: 2000;
    fDefaultLightIntensity   = 0.8; // old: 0.6;
    fDefaultAmbientIntensity = 0.4;
    nHDefaultSegments        = 12;
    nVDefaultSegments        = 12;
    aDefaultLightColor       = RGB_Color(COL_WHITE);
    aDefaultAmbientColor     = RGB_Color(COL_BLACK);
    bDoubleSided             = false;
    mpMirrorOverlay = 0L;
}

/*************************************************************************
|*
|* Koennen die selektierten Objekte aufgebrochen werden?
|*
\************************************************************************/

bool E3dView::IsBreak3DObjPossible() const
{
    const SdrObjectVector aSelection(getSelectedSdrObjectVectorFromSdrMarkView());

    if(!aSelection.size())
    {
        return false;
    }

    for(sal_uInt32 i(0); i < aSelection.size(); i++)
    {
        E3dObject* pObj = dynamic_cast< E3dObject* >(aSelection[i]);

        if (pObj )
        {
            if(!pObj->IsBreakObjPossible())
            {
                return false;
            }
        }
        else
        {
            return false;
        }
    }

    return true;
}

/*************************************************************************
|*
|* Selektierte Lathe-Objekte aufbrechen
|*
\************************************************************************/

void E3dView::Break3DObj()
{
    if(IsBreak3DObjPossible())
    {
        // ALLE selektierten Objekte werden gewandelt
        const SdrObjectVector aSelection(getSelectedSdrObjectVectorFromSdrMarkView());
        BegUndo(String(SVX_RESSTR(RID_SVX_3D_UNDO_BREAK_LATHE)));

        for(sal_uInt32 a(0); a < aSelection.size(); a++)
        {
            E3dObject* pObj = dynamic_cast< E3dObject* >(aSelection[a]);
            BreakSingle3DObj(pObj);
        }

        DeleteMarked();
        EndUndo();
    }
}

void E3dView::BreakSingle3DObj(E3dObject* pObj)
{
    E3dScene* pE3dScene = dynamic_cast< E3dScene* >(pObj);

    if(pE3dScene)
    {
        SdrObjList* pSubList = pE3dScene->getChildrenOfSdrObject();
        SdrObjListIter aIter(*pSubList, IM_FLAT);

        while(aIter.IsMore())
        {
            E3dObject* pSubObj = dynamic_cast< E3dObject* >(aIter.Next());

            if(pSubObj)
            {
            BreakSingle3DObj(pSubObj);
            }
        }
    }
    else
    {
        SdrAttrObj* pNewObj = pObj->GetBreakObj();

        if(pNewObj)
        {
            InsertObjectAtView(*pNewObj, SDRINSERT_DONTMARK);
            const SdrObjectChangeBroadcaster aSdrObjectChangeBroadcaster(*pNewObj);
            pNewObj->SetChanged();
        }
    }
}

/*************************************************************************
|*
|* Possibilities, hauptsaechlich gruppieren/ungruppieren
|*
\************************************************************************/
void E3dView::CheckPossibilities()
{
    // call parent
    SdrView::CheckPossibilities();

    // Weitere Flags bewerten
    if(mbGroupPossible || mbUnGroupPossible || mbGrpEnterPossible)
    {
        const SdrObjectVector aSelection(getSelectedSdrObjectVectorFromSdrMarkView());
        bool bCoumpound = false;
        bool b3DObject = false;

        for(sal_Int32 nObjs(0); (nObjs < aSelection.size()) && !bCoumpound; nObjs++)
        {
            SdrObject *pObj = aSelection[nObjs];

            if(dynamic_cast< E3dCompoundObject* >(pObj))
                bCoumpound = true;
            if(dynamic_cast< E3dObject* >(pObj))
                b3DObject = true;
        }

        // Bisher: Es sind ZWEI oder mehr beliebiger Objekte selektiert.
        // Nachsehen, ob CompoundObjects beteiligt sind. Falls ja,
        // das Gruppieren verbieten.
        if(mbGroupPossible && bCoumpound)
        {
            mbGroupPossible = false;
        }

        if(mbUnGroupPossible && b3DObject)
        {
            mbUnGroupPossible = false;
        }

        if(mbGrpEnterPossible && bCoumpound)
        {
            mbGrpEnterPossible = false;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// eof

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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#include "svx/svdstr.hrc"
#include "svx/svdglob.hxx"
#include <svx/svdview.hxx>
#include <svx/svdattr.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdmodel.hxx>
#include "svx/svditer.hxx"
#include "svx/globl3d.hxx"
#include <svx/camera3d.hxx>
#include <svx/scene3d.hxx>
#include <svx/polysc3d.hxx>
#include <svx/cube3d.hxx>
#include <svx/lathe3d.hxx>
#include <svx/sphere3d.hxx>
#include <svx/extrud3d.hxx>
#include <svx/obj3d.hxx>
#include <svx/xtable.hxx>
#include <svx/xflclit.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <svx/xlnclit.hxx>
#include <svl/metitem.hxx>
#include <svx/xtable.hxx>
#include <svx/xfillit.hxx>
#include <svx/xlnwtit.hxx>
#include <vcl/virdev.hxx>
#include <tools/poly.hxx>
#include <tools/b3dtrans.hxx>
#include <svx/svxids.hrc>
#include <editeng/colritem.hxx>
#include <svx/e3ditem.hxx>
#include <svx/xlntrit.hxx>
#include <svx/xfltrit.hxx>
#include <svx/svdpagv.hxx>
#include <vcl/gradient.hxx>
#include <vcl/metaact.hxx>
#include <svx/svx3ditems.hxx>
#include <svl/whiter.hxx>
#include <svtools/colorcfg.hxx>
#include <editeng/eeitem.hxx>
#include <svx/xgrscit.hxx>
#include "svdoimp.hxx"
#include <svx/sdr/properties/e3dproperties.hxx>
#include <svx/sdr/properties/e3dcompoundproperties.hxx>
#include <basegfx/polygon/b3dpolypolygontools.hxx>
#include <basegfx/point/b3dpoint.hxx>
#include <basegfx/vector/b3dvector.hxx>
#include <svx/xlndsit.hxx>
#include <basegfx/matrix/b3dhommatrix.hxx>
#include <basegfx/polygon/b3dpolygon.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/polygon/b3dpolygontools.hxx>
#include <svx/helperhittest3d.hxx>
#include <svx/sdr/contact/viewcontactofe3d.hxx>
#include <drawinglayer/geometry/viewinformation3d.hxx>
#include <com/sun/star/uno/Sequence.h>
#include <svx/sdr/contact/viewcontactofe3dscene.hxx>
#include <basegfx/polygon/b3dpolypolygontools.hxx>
#include <svx/e3dsceneupdater.hxx>

#define ITEMVALUE(ItemSet,Id,Cast)  ((const Cast&)(ItemSet).Get(Id)).GetValue()

//////////////////////////////////////////////////////////////////////////////

using namespace com::sun::star;

/*************************************************************************
|*
|* Liste fuer 3D-Objekte
|*
\************************************************************************/

TYPEINIT1(E3dObjList, SdrObjList);

E3dObjList::E3dObjList(SdrModel* pNewModel, SdrPage* pNewPage, E3dObjList* pNewUpList)
:   SdrObjList(pNewModel, pNewPage, pNewUpList)
{
}

E3dObjList::E3dObjList(const E3dObjList& rSrcList)
:   SdrObjList(rSrcList)
{
}

E3dObjList::~E3dObjList()
{
}

void E3dObjList::NbcInsertObject(SdrObject* pObj, sal_uIntPtr nPos, const SdrInsertReason* pReason)
{
    // Owner holen
    DBG_ASSERT(GetOwnerObj()->ISA(E3dObject), "AW: Einfuegen 3DObject in Parent != 3DObject");

    // Ist es ueberhaupt ein 3D-Objekt?
    if(pObj && pObj->ISA(E3dObject))
    {
        // Normales 3D Objekt, einfuegen mittels
        // call parent
        SdrObjList::NbcInsertObject(pObj, nPos, pReason);
    }
    else
    {
        // Kein 3D Objekt, fuege in Seite statt in Szene ein...
        GetOwnerObj()->GetPage()->InsertObject(pObj, nPos);
    }
}

void E3dObjList::InsertObject(SdrObject* pObj, sal_uIntPtr nPos, const SdrInsertReason* pReason)
{
    OSL_ENSURE(GetOwnerObj()->ISA(E3dObject), "Insert 3DObject in non-3D Parent");
    //E3DModifySceneSnapRectUpdater aUpdater(GetOwnerObj());

    // call parent
    SdrObjList::InsertObject(pObj, nPos, pReason);

    E3dScene* pScene = ((E3dObject*)GetOwnerObj())->GetScene();
    if(pScene)
    {
        pScene->Cleanup3DDepthMapper();
    }
}

SdrObject* E3dObjList::NbcRemoveObject(sal_uIntPtr nObjNum)
{
    DBG_ASSERT(GetOwnerObj()->ISA(E3dObject), "AW: Entfernen 3DObject aus Parent != 3DObject");
    //E3DModifySceneSnapRectUpdater aUpdater(GetOwnerObj());

    // call parent
    SdrObject* pRetval = SdrObjList::NbcRemoveObject(nObjNum);

    E3dScene* pScene = ((E3dObject*)GetOwnerObj())->GetScene();
    if(pScene)
    {
        pScene->Cleanup3DDepthMapper();
    }

    return pRetval;
}

SdrObject* E3dObjList::RemoveObject(sal_uIntPtr nObjNum)
{
    OSL_ENSURE(GetOwnerObj()->ISA(E3dObject), "3DObject is removed from non-3D Parent");
    //E3DModifySceneSnapRectUpdater aUpdater(GetOwnerObj());

    // call parent
    SdrObject* pRetval = SdrObjList::RemoveObject(nObjNum);

    E3dScene* pScene = ((E3dObject*)GetOwnerObj())->GetScene();
    if(pScene)
    {
        pScene->Cleanup3DDepthMapper();
    }

    return pRetval;
}

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

//////////////////////////////////////////////////////////////////////////////

sdr::properties::BaseProperties* E3dObject::CreateObjectSpecificProperties()
{
    return new sdr::properties::E3dProperties(*this);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

TYPEINIT1(E3dObject, SdrAttrObj);

E3dObject::E3dObject()
:   maSubList(),
    maLocalBoundVol(),
    maTransformation(),
    maFullTransform(),
    mbTfHasChanged(true),
    mbIsSelected(false)
{
    bIs3DObj = true;
    maSubList.SetOwnerObj(this);
    maSubList.SetListKind(SDROBJLIST_GROUPOBJ);
    bClosedObj = true;
}

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

E3dObject::~E3dObject()
{
}

/*************************************************************************
|*
|* Selektions-Flag setzen
|*
\************************************************************************/

void E3dObject::SetSelected(bool bNew)
{
    if((bool)mbIsSelected != bNew)
    {
        mbIsSelected = bNew;
    }

    for(sal_uInt32 a(0); a < maSubList.GetObjCount(); a++)
    {
        E3dObject* pCandidate = dynamic_cast< E3dObject* >(maSubList.GetObj(a));

        if(pCandidate)
        {
            pCandidate->SetSelected(bNew);
        }
    }
}

/*************************************************************************
|*
|* Aufbrechen, default-Implementierungen
|*
\************************************************************************/

sal_Bool E3dObject::IsBreakObjPossible()
{
    return sal_False;
}

SdrAttrObj* E3dObject::GetBreakObj()
{
    return 0L;
}

/*************************************************************************
|*
|* SetRectsDirty muss ueber die lokale SdrSubList gehen
|*
\************************************************************************/

void E3dObject::SetRectsDirty(sal_Bool bNotMyself)
{
    // call parent
    SdrAttrObj::SetRectsDirty(bNotMyself);

    for(sal_uInt32 a(0); a < maSubList.GetObjCount(); a++)
    {
        E3dObject* pCandidate = dynamic_cast< E3dObject* >(maSubList.GetObj(a));

        if(pCandidate)
        {
            pCandidate->SetRectsDirty(bNotMyself);
        }
    }
}

/*************************************************************************
|*
|* Inventor zurueckgeben
|*
\************************************************************************/

sal_uInt32 E3dObject::GetObjInventor() const
{
    return E3dInventor;
}

/*************************************************************************
|*
|* Identifier zurueckgeben
|*
\************************************************************************/

sal_uInt16 E3dObject::GetObjIdentifier() const
{
    return E3D_OBJECT_ID;
}

/*************************************************************************
|*
|* Faehigkeiten des Objektes feststellen
|*
\************************************************************************/

void E3dObject::TakeObjInfo(SdrObjTransformInfoRec& rInfo) const
{
    rInfo.bResizeFreeAllowed    = sal_True;
    rInfo.bResizePropAllowed    = sal_True;
    rInfo.bRotateFreeAllowed    = sal_True;
    rInfo.bRotate90Allowed      = sal_True;
    rInfo.bMirrorFreeAllowed    = sal_False;
    rInfo.bMirror45Allowed      = sal_False;
    rInfo.bMirror90Allowed      = sal_False;
    rInfo.bShearAllowed         = sal_False;
    rInfo.bEdgeRadiusAllowed    = sal_False;
    rInfo.bCanConvToPath        = sal_False;

    // no transparence for 3d objects
    rInfo.bTransparenceAllowed = sal_False;

    // gradient depends on fillstyle
    // BM *** check if SetItem is NULL ***
    XFillStyle eFillStyle = ((XFillStyleItem&)(GetMergedItem(XATTR_FILLSTYLE))).GetValue();
    rInfo.bGradientAllowed = (eFillStyle == XFILL_GRADIENT);

    // Umwandeln von 3D-Koerpern in Gruppe von Polygonen:
    //
    // Erst mal nicht moeglich, da die Erzeugung einer Gruppe von
    // 2D-Polygonen notwendig waere, die tiefensortiert werden muessten,
    // also bei Durchdringugnen auch gegeneinander geschnitten werden
    // muessten. Auch die Texturkoorinaten waeren ein ungeloestes
    // Problem.
    rInfo.bCanConvToPoly = sal_False;
    rInfo.bCanConvToContour = sal_False;
    rInfo.bCanConvToPathLineToArea = sal_False;
    rInfo.bCanConvToPolyLineToArea = sal_False;
}

/*************************************************************************
|*
|* Layer setzen
|*
\************************************************************************/

void E3dObject::NbcSetLayer(SdrLayerID nLayer)
{
    SdrAttrObj::NbcSetLayer(nLayer);

    for(sal_uInt32 a(0); a < maSubList.GetObjCount(); a++)
    {
        E3dObject* pCandidate = dynamic_cast< E3dObject* >(maSubList.GetObj(a));

        if(pCandidate)
        {
            pCandidate->NbcSetLayer(nLayer);
        }
    }
}

/*************************************************************************
|*
|* ObjList auch an SubList setzen
|*
\************************************************************************/

void E3dObject::SetObjList(SdrObjList* pNewObjList)
{
    SdrObject::SetObjList(pNewObjList);
    maSubList.SetUpList(pNewObjList);
}

/*************************************************************************
|*
|* Layer setzen
|*
\************************************************************************/

void E3dObject::SetPage(SdrPage* pNewPage)
{
    SdrAttrObj::SetPage(pNewPage);
    maSubList.SetPage(pNewPage);
}

/*************************************************************************
|*
|* Layer setzen
|*
\************************************************************************/

void E3dObject::SetModel(SdrModel* pNewModel)
{
    SdrAttrObj::SetModel(pNewModel);
    maSubList.SetModel(pNewModel);
}

/*************************************************************************
|*
|* resize object, used from old 2d interfaces, e.g. in Move/Scale dialog
|* (F4)
|*
\************************************************************************/
void E3dObject::NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact)
{
    // Bewegung in X,Y im Augkoordinatensystem
    E3dScene* pScene = GetScene();

    if(pScene)
    {
        // transform pos from 2D world to 3D eye
        const sdr::contact::ViewContactOfE3dScene& rVCScene = static_cast< sdr::contact::ViewContactOfE3dScene& >(pScene->GetViewContact());
        const drawinglayer::geometry::ViewInformation3D aViewInfo3D(rVCScene.getViewInformation3D());
        basegfx::B2DPoint aScaleCenter2D((double)rRef.X(), (double)rRef.Y());
        basegfx::B2DHomMatrix aInverseSceneTransform(rVCScene.getObjectTransformation());

        aInverseSceneTransform.invert();
        aScaleCenter2D = aInverseSceneTransform * aScaleCenter2D;

        basegfx::B3DPoint aScaleCenter3D(aScaleCenter2D.getX(), aScaleCenter2D.getY(), 0.5);
        basegfx::B3DHomMatrix aInverseViewToEye(aViewInfo3D.getDeviceToView() * aViewInfo3D.getProjection());

        aInverseViewToEye.invert();
        aScaleCenter3D = aInverseViewToEye * aScaleCenter3D;

        // scale-faktoren holen
        double fScaleX(xFact);
        double fScaleY(yFact);

        // build transform
        basegfx::B3DHomMatrix aInverseOrientation(aViewInfo3D.getOrientation());
        aInverseOrientation.invert();
        basegfx::B3DHomMatrix mFullTransform(GetFullTransform());
        basegfx::B3DHomMatrix mTrans(mFullTransform);

        mTrans *= aViewInfo3D.getOrientation();
        mTrans.translate(-aScaleCenter3D.getX(), -aScaleCenter3D.getY(), -aScaleCenter3D.getZ());
        mTrans.scale(fScaleX, fScaleY, 1.0);
        mTrans.translate(aScaleCenter3D.getX(), aScaleCenter3D.getY(), aScaleCenter3D.getZ());
        mTrans *= aInverseOrientation;
        mFullTransform.invert();
        mTrans *= mFullTransform;

        // anwenden
        basegfx::B3DHomMatrix mObjTrans(GetTransform());
        mObjTrans *= mTrans;

        E3DModifySceneSnapRectUpdater aUpdater(this);
        SetTransform(mObjTrans);
    }
}

/*************************************************************************
|*
|* Objekt verschieben in 2D, wird bei Cursortasten benoetigt
|*
\************************************************************************/
void E3dObject::NbcMove(const Size& rSize)
{
    // Bewegung in X,Y im Augkoordinatensystem
    E3dScene* pScene = GetScene();

    if(pScene)
    {
        // Abmessungen der Szene in 3D und 2D als Vergleich
        Rectangle aRect = pScene->GetSnapRect();

        // Transformation Weltkoordinaten bis eine VOR Objektkoordinaten holen
        basegfx::B3DHomMatrix mInvDispTransform;
        if(GetParentObj())
        {
            mInvDispTransform = GetParentObj()->GetFullTransform();
            mInvDispTransform.invert();
        }

        // BoundVolume from 3d world to 3d eye
        const sdr::contact::ViewContactOfE3dScene& rVCScene = static_cast< sdr::contact::ViewContactOfE3dScene& >(pScene->GetViewContact());
        const drawinglayer::geometry::ViewInformation3D aViewInfo3D(rVCScene.getViewInformation3D());
        basegfx::B3DRange aEyeVol(pScene->GetBoundVolume());
        aEyeVol.transform(aViewInfo3D.getOrientation());

        // build relative movement vector in eye coordinates
        basegfx::B3DPoint aMove(
            (double)rSize.Width() * aEyeVol.getWidth() / (double)aRect.GetWidth(),
            (double)-rSize.Height() * aEyeVol.getHeight() / (double)aRect.GetHeight(),
            0.0);
        basegfx::B3DPoint aPos(0.0, 0.0, 0.0);

        // movement vektor to local coordinates of objects' parent
        basegfx::B3DHomMatrix aInverseOrientation(aViewInfo3D.getOrientation());
        aInverseOrientation.invert();
        basegfx::B3DHomMatrix aCompleteTrans(mInvDispTransform * aInverseOrientation);

        aMove = aCompleteTrans * aMove;
        aPos = aCompleteTrans * aPos;

        // build transformation and apply
        basegfx::B3DHomMatrix aTranslate;
        aTranslate.translate(aMove.getX() - aPos.getX(), aMove.getY() - aPos.getY(), aMove.getZ() - aPos.getZ());

        E3DModifySceneSnapRectUpdater aUpdater(pScene);
        SetTransform(aTranslate * GetTransform());
    }
}

/*************************************************************************
|*
|* liefere die Sublist, aber nur dann, wenn darin Objekte enthalten sind !
|*
\************************************************************************/

SdrObjList* E3dObject::GetSubList() const
{
    return &(const_cast< E3dObjList& >(maSubList));
}

/*************************************************************************
|*
|* SnapRect berechnen
|*
\************************************************************************/

void E3dObject::RecalcSnapRect()
{
    maSnapRect = Rectangle();

    for(sal_uInt32 a(0); a < maSubList.GetObjCount(); a++)
    {
        E3dObject* pCandidate = dynamic_cast< E3dObject* >(maSubList.GetObj(a));

        if(pCandidate)
        {
            maSnapRect.Union(pCandidate->GetSnapRect());
        }
    }
}

/*************************************************************************
|*
|* Einfuegen eines 3D-Objekts an den Parent weitermelden, damit dieser
|* ggf. eine Sonderbehandlung fuer spezielle Objekte durchfuehren kann
|* (z.B. Light/Label in E3dScene)
|*
\************************************************************************/

void E3dObject::NewObjectInserted(const E3dObject* p3DObj)
{
    if(GetParentObj())
        GetParentObj()->NewObjectInserted(p3DObj);
}

/*************************************************************************
|*
|* Parent ueber Aenderung der Struktur (z.B. durch Transformation)
|* informieren; dabei wird das Objekt, in welchem die Aenderung
|* aufgetreten ist, uebergeben
|*
\************************************************************************/

void E3dObject::StructureChanged()
{
    if ( GetParentObj() )
    {
        GetParentObj()->InvalidateBoundVolume();
        GetParentObj()->StructureChanged();
    }
}

/*************************************************************************
|*
|* 3D-Objekt einfuegen
|*
\************************************************************************/

void E3dObject::Insert3DObj(E3dObject* p3DObj)
{
    DBG_ASSERT(p3DObj, "Insert3DObj mit NULL-Zeiger!");
    SdrPage* pPg = pPage;
    maSubList.InsertObject(p3DObj);
    pPage = pPg;
    InvalidateBoundVolume();
    NewObjectInserted(p3DObj);
    StructureChanged();
}

void E3dObject::Remove3DObj(E3dObject* p3DObj)
{
    DBG_ASSERT(p3DObj, "Remove3DObj mit NULL-Zeiger!");

    if(p3DObj->GetParentObj() == this)
    {
        SdrPage* pPg = pPage;
        maSubList.RemoveObject(p3DObj->GetOrdNum());
        pPage = pPg;

        InvalidateBoundVolume();
        StructureChanged();
    }
}

/*************************************************************************
|*
|* Parent holen
|*
\************************************************************************/

E3dObject* E3dObject::GetParentObj() const
{
    E3dObject* pRetval = NULL;

    if(GetObjList()
        && GetObjList()->GetOwnerObj()
        && GetObjList()->GetOwnerObj()->ISA(E3dObject))
        pRetval = ((E3dObject*)GetObjList()->GetOwnerObj());
    return pRetval;
}

/*************************************************************************
|*
|* Uebergeordnetes Szenenobjekt bestimmen
|*
\************************************************************************/

E3dScene* E3dObject::GetScene() const
{
    if(GetParentObj())
        return GetParentObj()->GetScene();
    return NULL;
}

/*************************************************************************
|*
|* umschliessendes Volumen inklusive aller Kindobjekte berechnen
|*
\************************************************************************/

basegfx::B3DRange E3dObject::RecalcBoundVolume() const
{
    basegfx::B3DRange aRetval;
    const sal_uInt32 nObjCnt(maSubList.GetObjCount());

    if(nObjCnt)
    {
        for(sal_uInt32 a(0); a < nObjCnt; a++)
        {
            const E3dObject* p3DObject = dynamic_cast< const E3dObject* >(maSubList.GetObj(a));

            if(p3DObject)
            {
                basegfx::B3DRange aLocalRange(p3DObject->GetBoundVolume());
                aLocalRange.transform(p3DObject->GetTransform());
                aRetval.expand(aLocalRange);
            }
        }
    }
    else
    {
        // single 3D object
        const sdr::contact::ViewContactOfE3d* pVCOfE3D = dynamic_cast< const sdr::contact::ViewContactOfE3d* >(&GetViewContact());

        if(pVCOfE3D)
        {
            // BoundVolume is without 3D object transformation, use correct sequence
            const drawinglayer::primitive3d::Primitive3DSequence xLocalSequence(pVCOfE3D->getVIP3DSWithoutObjectTransform());

            if(xLocalSequence.hasElements())
            {
                const uno::Sequence< beans::PropertyValue > aEmptyParameters;
                const drawinglayer::geometry::ViewInformation3D aLocalViewInformation3D(aEmptyParameters);

                aRetval = drawinglayer::primitive3d::getB3DRangeFromPrimitive3DSequence(
                    xLocalSequence, aLocalViewInformation3D);
            }
        }
    }

    return aRetval;
}

/*************************************************************************
|*
|* umschliessendes Volumen zurueckgeben und ggf. neu berechnen
|*
\************************************************************************/

const basegfx::B3DRange& E3dObject::GetBoundVolume() const
{
    if(maLocalBoundVol.isEmpty())
    {
        const_cast< E3dObject* >(this)->maLocalBoundVol = RecalcBoundVolume();
    }

    return maLocalBoundVol;
}

void E3dObject::InvalidateBoundVolume()
{
    maLocalBoundVol.reset();
}

/*************************************************************************
|*
|* Aederung des BoundVolumes an alle Kindobjekte weitergeben
|*
\************************************************************************/

void E3dObject::SetBoundVolInvalid()
{
    InvalidateBoundVolume();

    for(sal_uInt32 a(0); a < maSubList.GetObjCount(); a++)
    {
        E3dObject* pCandidate = dynamic_cast< E3dObject* >(maSubList.GetObj(a));

        if(pCandidate)
        {
            pCandidate->SetBoundVolInvalid();
        }
    }
}

/*************************************************************************
|*
|* Aederung der Transformation an alle Kindobjekte weitergeben
|*
\************************************************************************/

void E3dObject::SetTransformChanged()
{
    InvalidateBoundVolume();
    mbTfHasChanged = true;

    for(sal_uInt32 a(0); a < maSubList.GetObjCount(); a++)
    {
        E3dObject* pCandidate = dynamic_cast< E3dObject* >(maSubList.GetObj(a));

        if(pCandidate)
        {
            pCandidate->SetTransformChanged();
        }
    }
}

/*************************************************************************
|*
|* hierarchische Transformation ueber alle Parents bestimmen, in
|* maFullTransform ablegen und diese zurueckgeben
|*
\************************************************************************/

const basegfx::B3DHomMatrix& E3dObject::GetFullTransform() const
{
    if(mbTfHasChanged)
    {
        basegfx::B3DHomMatrix aNewFullTransformation(maTransformation);

        if ( GetParentObj() )
        {
            aNewFullTransformation = GetParentObj()->GetFullTransform() * aNewFullTransformation;
        }

        const_cast< E3dObject* >(this)->maFullTransform = aNewFullTransformation;
        const_cast< E3dObject* >(this)->mbTfHasChanged = false;
    }

    return maFullTransform;
}

/*************************************************************************
|*
|* Transformationsmatrix abfragen
|*
\************************************************************************/

const basegfx::B3DHomMatrix& E3dObject::GetTransform() const
{
    return maTransformation;
}

/*************************************************************************
|*
|* Transformationsmatrix setzen
|*
\************************************************************************/

void E3dObject::NbcSetTransform(const basegfx::B3DHomMatrix& rMatrix)
{
    if(maTransformation != rMatrix)
    {
        maTransformation = rMatrix;
        SetTransformChanged();
        StructureChanged();
    }
}

/*************************************************************************
|*
|* Transformationsmatrix setzen mit Repaint-Broadcast
|*
\************************************************************************/

void E3dObject::SetTransform(const basegfx::B3DHomMatrix& rMatrix)
{
    if(rMatrix != maTransformation)
    {
        // #110094#-14 SendRepaintBroadcast();
        NbcSetTransform(rMatrix);
        SetChanged();
        BroadcastObjectChange();
        if (pUserCall != NULL) pUserCall->Changed(*this, SDRUSERCALL_RESIZE, Rectangle());
    }
}

/*************************************************************************
|*
|* Linien fuer die Wireframe-Darstellung des Objekts dem uebergebenen
|* basegfx::B3DPolygon hinzufuegen
|*
\************************************************************************/

basegfx::B3DPolyPolygon E3dObject::CreateWireframe() const
{
    const basegfx::B3DRange aBoundVolume(GetBoundVolume());
    return basegfx::tools::createCubePolyPolygonFromB3DRange(aBoundVolume);
}

/*************************************************************************
|*
|* Get the name of the object (singular)
|*
\************************************************************************/

void E3dObject::TakeObjNameSingul(XubString& rName) const
{
    rName=ImpGetResStr(STR_ObjNameSingulObj3d);

    String aName( GetName() );
    if(aName.Len())
    {
        rName += sal_Unicode(' ');
        rName += sal_Unicode('\'');
        rName += aName;
        rName += sal_Unicode('\'');
    }
}

/*************************************************************************
|*
|* Get the name of the object (plural)
|*
\************************************************************************/

void E3dObject::TakeObjNamePlural(XubString& rName) const
{
    rName=ImpGetResStr(STR_ObjNamePluralObj3d);
}

/*************************************************************************
|*
|* Zuweisungsoperator
|*
\************************************************************************/

void E3dObject::operator=(const SdrObject& rObj)
{
    SdrObject::operator=(rObj);

    const E3dObject& r3DObj = (const E3dObject&) rObj;
    if (r3DObj.GetSubList())
    {
        maSubList.CopyObjects(*r3DObj.GetSubList());
    }

    // BoundVol kann uebernommen werden, da die Childs auch kopiert werden
    maLocalBoundVol  = r3DObj.maLocalBoundVol;
    maTransformation = r3DObj.maTransformation;

    // Da sich der Parent geaendert haben kann, Gesamttransformation beim
    // naechsten Mal auf jeden Fall neu bestimmen
    SetTransformChanged();

    // Selektionsstatus kopieren
    mbIsSelected = r3DObj.mbIsSelected;
}

/*************************************************************************
|*
|* erstelle neues GeoData-Objekt
|*
\************************************************************************/

SdrObjGeoData *E3dObject::NewGeoData() const
{
    // Theoretisch duerfen auch nur Szenen ihre GeoDatas erstellen und verwalten !!
    // AW: Dies stimmt nicht mehr, diese Stelle ist mit der neuen Engine OK!
    return new E3DObjGeoData;
}

/*************************************************************************
|*
|* uebergebe aktuelle werte an das GeoData-Objekt
|*
\************************************************************************/

void E3dObject::SaveGeoData(SdrObjGeoData& rGeo) const
{
    SdrAttrObj::SaveGeoData (rGeo);

    ((E3DObjGeoData &) rGeo).maLocalBoundVol  = maLocalBoundVol;
    ((E3DObjGeoData &) rGeo).maTransformation = maTransformation;
}

/*************************************************************************
|*
|* uebernehme werte aus dem GeoData-Objekt
|*
\************************************************************************/

void E3dObject::RestGeoData(const SdrObjGeoData& rGeo)
{
    maLocalBoundVol = ((E3DObjGeoData &) rGeo).maLocalBoundVol;
    E3DModifySceneSnapRectUpdater aUpdater(this);
    NbcSetTransform(((E3DObjGeoData &) rGeo).maTransformation);
    SdrAttrObj::RestGeoData (rGeo);
}

/*************************************************************************
|*
|* Rotation eines 3d-Koerpers
|*
\************************************************************************/
// 2D-rotation eines 3D-Koerpers, normalerweise macht das die Szene selbst
// Ist aber eine korrekte Implementierung, denn alles was passiert ist eine
// Rotation um die Achse die senkrecht auf dem Bildschirm steht und zwar
// unabhaengig davon, wie die Szene bisher gedreht worden ist.

void E3dObject::NbcRotate(const Point& rRef, long nWink, double sn, double cs)
{
    // Also derzeit sind die Klebepunkte relativ zum aOutRect der Szene definiert. Vor dem Drehen
    // werden die Klebepunkte relativ zur Seite definiert. Sie nehmen an der Drehung der Szene noch nicht Teil
    // dafuer gibt es den
    SetGlueReallyAbsolute(sal_True);

    // SendRepaintBroadcast();
    double fWinkelInRad = nWink/100 * F_PI180;

    basegfx::B3DHomMatrix aRotateZ;
    aRotateZ.rotate(0.0, 0.0, fWinkelInRad);
    NbcSetTransform(aRotateZ * GetTransform());

    SetRectsDirty();    // Veranlasst eine Neuberechnung aller BoundRects
    NbcRotateGluePoints(rRef,nWink,sn,cs);  // Rotiert die Klebepunkte (die haben noch Koordinaten relativ
                                            // zum Urpsung des Blattes
    SetGlueReallyAbsolute(sal_False);  // ab jetzt sind sie wieder relativ zum BoundRect (also dem aOutRect definiert)
}

/*************************************************************************/

//////////////////////////////////////////////////////////////////////////////

sdr::properties::BaseProperties* E3dCompoundObject::CreateObjectSpecificProperties()
{
    return new sdr::properties::E3dCompoundProperties(*this);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

TYPEINIT1(E3dCompoundObject, E3dObject);

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

E3dCompoundObject::E3dCompoundObject()
:   E3dObject(),
    aMaterialAmbientColor(),
    bCreateNormals(false),
    bCreateTexture(false)
{
    // Defaults setzen
    E3dDefaultAttributes aDefault;
    SetDefaultAttributes(aDefault);
}

E3dCompoundObject::E3dCompoundObject(E3dDefaultAttributes& rDefault)
:   E3dObject(),
    aMaterialAmbientColor(),
    bCreateNormals(false),
    bCreateTexture(false)
{
    // Defaults setzen
    SetDefaultAttributes(rDefault);
}

void E3dCompoundObject::SetDefaultAttributes(E3dDefaultAttributes& rDefault)
{
    // Defaults setzen
    aMaterialAmbientColor = rDefault.GetDefaultAmbientColor();

    bCreateNormals = rDefault.GetDefaultCreateNormals();
    bCreateTexture = rDefault.GetDefaultCreateTexture();
}

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

E3dCompoundObject::~E3dCompoundObject ()
{
}

/*************************************************************************
|*
|* Drag-Polygon zurueckgeben
|*
\************************************************************************/

basegfx::B2DPolyPolygon E3dCompoundObject::TakeXorPoly() const
{
    basegfx::B2DPolyPolygon aRetval;
    const uno::Sequence< beans::PropertyValue > aEmptyParameters;
    drawinglayer::geometry::ViewInformation3D aViewInfo3D(aEmptyParameters);
    E3dScene* pRootScene = fillViewInformation3DForCompoundObject(aViewInfo3D, *this);

    if(pRootScene)
    {
        const sdr::contact::ViewContactOfE3dScene& rVCScene = static_cast< sdr::contact::ViewContactOfE3dScene& >(pRootScene->GetViewContact());
        const basegfx::B3DPolyPolygon aCubePolyPolygon(CreateWireframe());
        aRetval = basegfx::tools::createB2DPolyPolygonFromB3DPolyPolygon(aCubePolyPolygon,
            aViewInfo3D.getObjectToView() * GetTransform());
        aRetval.transform(rVCScene.getObjectTransformation());
    }

    return aRetval;
}

/*************************************************************************
|*
|* Anzahl der Handles zurueckgeben
|*
\************************************************************************/

sal_uInt32 E3dCompoundObject::GetHdlCount() const
{
    // 8 Eckpunkte + 1 E3dVolumeMarker (= Wireframe-Darstellung)
    return 9L;
}

/*************************************************************************
|*
|* Handle-Liste fuellen
|*
\************************************************************************/

void E3dCompoundObject::AddToHdlList(SdrHdlList& rHdlList) const
{
    const uno::Sequence< beans::PropertyValue > aEmptyParameters;
    drawinglayer::geometry::ViewInformation3D aViewInfo3D(aEmptyParameters);
    E3dScene* pRootScene = fillViewInformation3DForCompoundObject(aViewInfo3D, *this);

    if(pRootScene)
    {
        const basegfx::B3DRange aBoundVolume(GetBoundVolume());

        if(!aBoundVolume.isEmpty())
        {
            const sdr::contact::ViewContactOfE3dScene& rVCScene = static_cast< sdr::contact::ViewContactOfE3dScene& >(pRootScene->GetViewContact());

            for(sal_uInt32 a(0); a < 8; a++)
            {
                basegfx::B3DPoint aPos3D;

                switch(a)
                {
                    case 0 : aPos3D.setX(aBoundVolume.getMinX()); aPos3D.setY(aBoundVolume.getMinY()); aPos3D.setZ(aBoundVolume.getMinZ()); break;
                    case 1 : aPos3D.setX(aBoundVolume.getMinX()); aPos3D.setY(aBoundVolume.getMinY()); aPos3D.setZ(aBoundVolume.getMaxZ()); break;
                    case 2 : aPos3D.setX(aBoundVolume.getMinX()); aPos3D.setY(aBoundVolume.getMaxY()); aPos3D.setZ(aBoundVolume.getMinZ()); break;
                    case 3 : aPos3D.setX(aBoundVolume.getMinX()); aPos3D.setY(aBoundVolume.getMaxY()); aPos3D.setZ(aBoundVolume.getMaxZ()); break;
                    case 4 : aPos3D.setX(aBoundVolume.getMaxX()); aPos3D.setY(aBoundVolume.getMinY()); aPos3D.setZ(aBoundVolume.getMinZ()); break;
                    case 5 : aPos3D.setX(aBoundVolume.getMaxX()); aPos3D.setY(aBoundVolume.getMinY()); aPos3D.setZ(aBoundVolume.getMaxZ()); break;
                    case 6 : aPos3D.setX(aBoundVolume.getMaxX()); aPos3D.setY(aBoundVolume.getMaxY()); aPos3D.setZ(aBoundVolume.getMinZ()); break;
                    case 7 : aPos3D.setX(aBoundVolume.getMaxX()); aPos3D.setY(aBoundVolume.getMaxY()); aPos3D.setZ(aBoundVolume.getMaxZ()); break;
                }

                // to 3d view coor
                aPos3D *= aViewInfo3D.getObjectToView() * GetTransform();

                // create 2d relative scene
                basegfx::B2DPoint aPos2D(aPos3D.getX(), aPos3D.getY());

                // to 2d world coor
                aPos2D *= rVCScene.getObjectTransformation();

                rHdlList.AddHdl(new SdrHdl(Point(basegfx::fround(aPos2D.getX()), basegfx::fround(aPos2D.getY())), HDL_BWGT));
            }
        }
    }

    const basegfx::B2DPolyPolygon aPolyPolygon(TakeXorPoly());

    if(aPolyPolygon.count())
    {
        E3dVolumeMarker* pVolMarker = new E3dVolumeMarker(aPolyPolygon);
        rHdlList.AddHdl(pVolMarker);
    }
}

/*************************************************************************
|*
|* Identifier zurueckgeben
|*
\************************************************************************/

sal_uInt16 E3dCompoundObject::GetObjIdentifier() const
{
    return E3D_COMPOUNDOBJ_ID;
}

/*************************************************************************
|*
|* SnapRect berechnen
|*
\************************************************************************/

void E3dCompoundObject::RecalcSnapRect()
{
    const uno::Sequence< beans::PropertyValue > aEmptyParameters;
    drawinglayer::geometry::ViewInformation3D aViewInfo3D(aEmptyParameters);
    E3dScene* pRootScene = fillViewInformation3DForCompoundObject(aViewInfo3D, *this);
    maSnapRect = Rectangle();

    if(pRootScene)
    {
        // get VC of 3D candidate
        const sdr::contact::ViewContactOfE3d* pVCOfE3D = dynamic_cast< const sdr::contact::ViewContactOfE3d* >(&GetViewContact());

        if(pVCOfE3D)
        {
            // get 3D primitive sequence
            const drawinglayer::primitive3d::Primitive3DSequence xLocalSequence(pVCOfE3D->getViewIndependentPrimitive3DSequence());

            if(xLocalSequence.hasElements())
            {
                // get BoundVolume
                basegfx::B3DRange aBoundVolume(drawinglayer::primitive3d::getB3DRangeFromPrimitive3DSequence(
                    xLocalSequence, aViewInfo3D));

                // transform bound volume to relative scene coordinates
                aBoundVolume.transform(aViewInfo3D.getObjectToView());

                // build 2d relative scene range
                basegfx::B2DRange aSnapRange(
                    aBoundVolume.getMinX(), aBoundVolume.getMinY(),
                    aBoundVolume.getMaxX(), aBoundVolume.getMaxY());

                // transform to 2D world coordiantes
                const sdr::contact::ViewContactOfE3dScene& rVCScene = static_cast< sdr::contact::ViewContactOfE3dScene& >(pRootScene->GetViewContact());
                aSnapRange.transform(rVCScene.getObjectTransformation());

                // snap to integer
                maSnapRect = Rectangle(
                    sal_Int32(floor(aSnapRange.getMinX())), sal_Int32(floor(aSnapRange.getMinY())),
                    sal_Int32(ceil(aSnapRange.getMaxX())), sal_Int32(ceil(aSnapRange.getMaxY())));
            }
        }
    }
}

/*************************************************************************
|*
|* Copy-Operator
|*
\************************************************************************/

void E3dCompoundObject::operator=(const SdrObject& rObj)
{
    // erstmal alle Childs kopieren
    E3dObject::operator=(rObj);

    // weitere Parameter kopieren
    const E3dCompoundObject& r3DObj = (const E3dCompoundObject&) rObj;

    bCreateNormals = r3DObj.bCreateNormals;
    bCreateTexture = r3DObj.bCreateTexture;
    aMaterialAmbientColor = r3DObj.aMaterialAmbientColor;
}

/*************************************************************************
|*
|* Parameter Geometrieerzeugung setzen
|*
\************************************************************************/

void E3dCompoundObject::SetCreateNormals(sal_Bool bNew)
{
    if(bCreateNormals != bNew)
    {
        bCreateNormals = bNew;
        ActionChanged();
    }
}

void E3dCompoundObject::SetCreateTexture(sal_Bool bNew)
{
    if(bCreateTexture != bNew)
    {
        bCreateTexture = bNew;
        ActionChanged();
    }
}

/*************************************************************************
|*
|* Material des Objektes
|*
\************************************************************************/

void E3dCompoundObject::SetMaterialAmbientColor(const Color& rColor)
{
    if(aMaterialAmbientColor != rColor)
    {
        aMaterialAmbientColor = rColor;
    }
}

/*************************************************************************
|*
|* convert given basegfx::B3DPolyPolygon to screen coor
|*
\************************************************************************/

basegfx::B2DPolyPolygon E3dCompoundObject::TransformToScreenCoor(const basegfx::B3DPolyPolygon& rCandidate)
{
    const uno::Sequence< beans::PropertyValue > aEmptyParameters;
    drawinglayer::geometry::ViewInformation3D aViewInfo3D(aEmptyParameters);
    E3dScene* pRootScene = fillViewInformation3DForCompoundObject(aViewInfo3D, *this);
    basegfx::B2DPolyPolygon aRetval;

    if(pRootScene)
    {
        aRetval = basegfx::tools::createB2DPolyPolygonFromB3DPolyPolygon(rCandidate,
            aViewInfo3D.getObjectToView() * GetTransform());
        const sdr::contact::ViewContactOfE3dScene& rVCScene = static_cast< sdr::contact::ViewContactOfE3dScene& >(pRootScene->GetViewContact());
        aRetval.transform(rVCScene.getObjectTransformation());
    }

    return aRetval;
}

sal_Bool E3dCompoundObject::IsAOrdNumRemapCandidate(E3dScene*& prScene) const
{
    if(GetObjList()
        && GetObjList()->GetOwnerObj()
        && GetObjList()->GetOwnerObj()->ISA(E3dScene))
    {
        prScene = (E3dScene*)GetObjList()->GetOwnerObj();
        return sal_True;
    }

    return sal_False;
}

//////////////////////////////////////////////////////////////////////////////
// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

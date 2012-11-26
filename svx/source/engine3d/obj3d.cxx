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

#include "svx/svdstr.hrc"
#include "svx/svdglob.hxx"
#include <svx/svdview.hxx>
#include <svx/svdattr.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svditer.hxx>
#include <svx/globl3d.hxx>
#include <svx/camera3d.hxx>
#include <svx/scene3d.hxx>
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
#include <basegfx/matrix/b2dhommatrixtools.hxx>

#define ITEMVALUE(ItemSet,Id,Cast)  ((const Cast&)(ItemSet).Get(Id)).GetValue()

//////////////////////////////////////////////////////////////////////////////

using namespace com::sun::star;

//////////////////////////////////////////////////////////////////////////////

sdr::properties::BaseProperties* E3dObject::CreateObjectSpecificProperties()
{
    return new sdr::properties::E3dProperties(*this);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

E3dObject::E3dObject(SdrModel& rSdrModel)
:   SdrAttrObj(rSdrModel),
    maLocalBoundVol(),
    maTransformation(),
    maFullTransform(),
    mbTfHasChanged(true),
    mbIsSelected(false)
{
}

bool E3dObject::IsClosedObj() const
{
    return true;
}

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

E3dObject::~E3dObject()
{
}

void E3dObject::copyDataFromSdrObject(const SdrObject& rSource)
{
    if(this != &rSource)
    {
        const E3dObject* pSource = dynamic_cast< const E3dObject* >(&rSource);

        if(pSource)
        {
            // call parent
            SdrAttrObj::copyDataFromSdrObject(rSource);

            // BoundVol can b
            maLocalBoundVol  = pSource->maLocalBoundVol;
            maTransformation = pSource->maTransformation;

            // Da sich der Parent geaendert haben kann, Gesamttransformation beim
            // naechsten Mal auf jeden Fall neu bestimmen
            SetTransformChanged();

            // Selektionsstatus kopieren
            mbIsSelected = pSource->mbIsSelected;
        }
        else
        {
            OSL_ENSURE(false, "copyDataFromSdrObject with ObjectType of Source different from Target (!)");
        }
    }
}

SdrObject* E3dObject::CloneSdrObject(SdrModel* pTargetModel) const
{
    E3dObject* pClone = new E3dObject(
        pTargetModel ? *pTargetModel : getSdrModelFromSdrObject());
    OSL_ENSURE(pClone, "CloneSdrObject error (!)");
    pClone->copyDataFromSdrObject(*this);

    return pClone;
}

/*************************************************************************
|*
|* Selektions-Flag setzen
|*
\************************************************************************/

bool E3dObject::IsE3dObject() const
{
    return true;
}

void E3dObject::SetSelected(bool bNew)
{
    if((bool)mbIsSelected != bNew)
    {
        mbIsSelected = bNew;
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
    OSL_ENSURE(false, "E3dObject is a helper class, it should never be incarnated (!)");
    return SdrAttrObj::GetObjIdentifier();
}

/*************************************************************************
|*
|* Faehigkeiten des Objektes feststellen
|*
\************************************************************************/

void E3dObject::TakeObjInfo(SdrObjTransformInfoRec& rInfo) const
{
    rInfo.mbResizeFreeAllowed = true;
    rInfo.mbResizePropAllowed = true;
    rInfo.mbRotateFreeAllowed = true;
    rInfo.mbRotate90Allowed = true;
    rInfo.mbMirrorFreeAllowed = false;
    rInfo.mbMirror45Allowed = false;
    rInfo.mbMirror90Allowed = false;
    rInfo.mbShearAllowed = false;
    rInfo.mbEdgeRadiusAllowed = false;
    rInfo.mbCanConvToPath = false;

    // no transparence for 3d objects
    rInfo.mbTransparenceAllowed = false;

    // gradient depends on fillstyle
    // BM *** check if SetItem is NULL ***
    XFillStyle eFillStyle = ((XFillStyleItem&)(GetMergedItem(XATTR_FILLSTYLE))).GetValue();
    rInfo.mbGradientAllowed = (eFillStyle == XFILL_GRADIENT);

    // Umwandeln von 3D-Koerpern in Gruppe von Polygonen:
    //
    // Erst mal nicht moeglich, da die Erzeugung einer Gruppe von
    // 2D-Polygonen notwendig waere, die tiefensortiert werden muessten,
    // also bei Durchdringugnen auch gegeneinander geschnitten werden
    // muessten. Auch die Texturkoorinaten waeren ein ungeloestes
    // Problem.
    rInfo.mbCanConvToPoly = false;
    rInfo.mbCanConvToContour = false;
    rInfo.mbCanConvToPathLineToArea = false;
    rInfo.mbCanConvToPolyLineToArea = false;
}

/*************************************************************************
|*
|* Einfuegen eines 3D-Objekts an den Parent weitermelden, damit dieser
|* ggf. eine Sonderbehandlung fuer spezielle Objekte durchfuehren kann
|* (z.B. Light/Label in E3dScene)
|*
\************************************************************************/

void E3dObject::NewObjectInserted(const E3dObject& r3DObj)
{
    if(GetParentObj())
        GetParentObj()->NewObjectInserted(r3DObj);
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
|* Parent holen
|*
\************************************************************************/

E3dObject* E3dObject::GetParentObj() const
{
    E3dObject* pRetval = NULL;

    if(getParentOfSdrObject())
    {
        pRetval = dynamic_cast< E3dObject* >(getParentOfSdrObject()->getSdrObjectFromSdrObjList());
    }

    return pRetval;
}

/*************************************************************************
|*
|* Uebergeordnetes Szenenobjekt bestimmen
|*
\************************************************************************/

E3dScene* E3dObject::GetScene() const
{
    E3dObject* pParent = GetParentObj();

    if(pParent)
    {
        return pParent->GetScene();
    }

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

const basegfx::B3DHomMatrix& E3dObject::GetB3DTransform() const
{
    return maTransformation;
}

/*************************************************************************
|*
|* Transformationsmatrix setzen
|*
\************************************************************************/

void E3dObject::SetB3DTransform(const basegfx::B3DHomMatrix& rMatrix)
{
    if(maTransformation != rMatrix)
    {
        const SdrObjectChangeBroadcaster aSdrObjectChangeBroadcaster(*this);

        maTransformation = rMatrix;
        SetTransformChanged();
        StructureChanged();
        SetChanged();
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
    SetB3DTransform(((E3DObjGeoData &) rGeo).maTransformation);
    SdrAttrObj::RestGeoData (rGeo);
}

//////////////////////////////////////////////////////////////////////////////

sdr::properties::BaseProperties* E3dCompoundObject::CreateObjectSpecificProperties()
{
    return new sdr::properties::E3dCompoundProperties(*this);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

E3dCompoundObject::E3dCompoundObject(SdrModel& rSdrModel, const E3dDefaultAttributes& rDefault)
:   E3dObject(rSdrModel),
    aMaterialAmbientColor(),
    bCreateNormals(false),
    bCreateTexture(false)
{
    // Defaults setzen
    SetDefaultAttributes(rDefault);
}

void E3dCompoundObject::SetDefaultAttributes(const E3dDefaultAttributes& rDefault)
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

void E3dCompoundObject::copyDataFromSdrObject(const SdrObject& rSource)
{
    if(this != &rSource)
    {
        const E3dCompoundObject* pSource = dynamic_cast< const E3dCompoundObject* >(&rSource);

        if(pSource)
        {
            // call parent
            E3dObject::copyDataFromSdrObject(rSource);

            // copy local data
            bCreateNormals = pSource->bCreateNormals;
            bCreateTexture = pSource->bCreateTexture;
            aMaterialAmbientColor = pSource->aMaterialAmbientColor;
        }
        else
        {
            OSL_ENSURE(false, "copyDataFromSdrObject with ObjectType of Source different from Target (!)");
        }
    }
}

void E3dCompoundObject::InvalidateBoundVolume()
{
    // call parent
    E3dObject::InvalidateBoundVolume();

    // reset 2D object transformation to flag that it has changed
    maSdrObjectTransformation.setB2DHomMatrix(basegfx::B2DHomMatrix());
}

SdrObject* E3dCompoundObject::CloneSdrObject(SdrModel* pTargetModel) const
{
    E3dCompoundObject* pClone = new E3dCompoundObject(
        pTargetModel ? *pTargetModel : getSdrModelFromSdrObject(),
        E3dDefaultAttributes());
    OSL_ENSURE(pClone, "CloneSdrObject error (!)");
    pClone->copyDataFromSdrObject(*this);

    return pClone;
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
            aViewInfo3D.getObjectToView() * GetB3DTransform());
        aRetval.transform(rVCScene.getObjectTransformation());
    }

    return aRetval;
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
                aPos3D *= aViewInfo3D.getObjectToView() * GetB3DTransform();

                // create 2d relative scene
                basegfx::B2DPoint aPos2D(aPos3D.getX(), aPos3D.getY());

                // to 2d world coor
                aPos2D *= rVCScene.getObjectTransformation();

                new SdrHdl(rHdlList, pRootScene, HDL_BWGT, aPos2D);
            }
        }
    }

    const basegfx::B2DPolyPolygon aPolyPolygon(TakeXorPoly());

    if(aPolyPolygon.count())
    {
        new E3dVolumeMarker(rHdlList, pRootScene, aPolyPolygon);
    }
}

/*************************************************************************
|*
|* Identifier zurueckgeben
|*
\************************************************************************/

sal_uInt16 E3dCompoundObject::GetObjIdentifier() const
{
    OSL_ENSURE(false, "E3dCompoundObject is a helper class, it should never be incarnated (!)");
    return SdrAttrObj::GetObjIdentifier();
}

/*************************************************************************
|*
|* SnapRect berechnen
|*
\************************************************************************/

const basegfx::B2DHomMatrix& E3dCompoundObject::getSdrObjectTransformation() const
{
    // Transformation is size (scale) and position (translation) of content
    // of all sub-objects combined. To not always create this, use isIdentity() as
    // hint for recalculation
    // TTTT: look for correct place to set to identity
    // TTTT: Added InvalidateBoundVolume to E3dCompoundObject to do the job, check
    if(maSdrObjectTransformation.getB2DHomMatrix().isIdentity())
    {
        const uno::Sequence< beans::PropertyValue > aEmptyParameters;
        drawinglayer::geometry::ViewInformation3D aViewInfo3D(aEmptyParameters);
        E3dScene* pRootScene = fillViewInformation3DForCompoundObject(aViewInfo3D, *this);

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

                        // build 2D relative scene range
                    basegfx::B2DRange aSnapRange(
                        aBoundVolume.getMinX(), aBoundVolume.getMinY(),
                        aBoundVolume.getMaxX(), aBoundVolume.getMaxY());

                    // transform to 2D world coordiantes
                    const sdr::contact::ViewContactOfE3dScene& rVCScene = static_cast< sdr::contact::ViewContactOfE3dScene& >(pRootScene->GetViewContact());
                    aSnapRange.transform(rVCScene.getObjectTransformation());

                        // compose local transformation
                        const_cast< E3dCompoundObject* >(this)->maSdrObjectTransformation.setB2DHomMatrix(
                            basegfx::tools::createScaleTranslateB2DHomMatrix(
                                aSnapRange.getRange(),
                                aSnapRange.getMinimum()));
                }
            }
        }
    }

    // call parent
    return E3dObject::getSdrObjectTransformation();
}

void E3dCompoundObject::setSdrObjectTransformation(const basegfx::B2DHomMatrix& rTransformation)
{
    // create a transformation to the 3D EyeCoordinateSystem and apply the 2D transformation there
    const E3dScene* pScene = GetScene();

    if(pScene && !rTransformation.isIdentity())
    {
        // create relative transformation from current state
        basegfx::B2DHomMatrix a2DSubTrans(getSdrObjectTransformation());
        a2DSubTrans.invert();
        a2DSubTrans = rTransformation * a2DSubTrans;

        // transform from 2D world to to 2D scene-local coordinates
        const sdr::contact::ViewContactOfE3dScene& rVCScene = static_cast< sdr::contact::ViewContactOfE3dScene& >(pScene->GetViewContact());
        basegfx::B2DHomMatrix aInverseSceneTransform(rVCScene.getObjectTransformation());
        aInverseSceneTransform.invert();
        a2DSubTrans = aInverseSceneTransform * a2DSubTrans;

        // extract 2D transform components and apply to 3D matrix for further transformation
        basegfx::B2DVector aScale;
        basegfx::B2DPoint aTranslate;
        double fRotate, fShearX;
        basegfx::B3DHomMatrix a3DSubTrans;
        a2DSubTrans.decompose(aScale, aTranslate, fRotate, fShearX);

        if(!aScale.equal(basegfx::B2DTuple(1.0, 1.0)))
        {
            a3DSubTrans.scale(aScale.getX(), aScale.getY(), 1.0);
        }

        if(!basegfx::fTools::equalZero(fShearX))
        {
            a3DSubTrans.shearXZ(fShearX, 0.0);
        }

        if(!basegfx::fTools::equalZero(fRotate))
        {
            a3DSubTrans.rotate(0.0, 0.0, fRotate);
        }

        if(!aTranslate.equal(basegfx::B2DTuple(0.0, 0.0)))
        {
            a3DSubTrans.translate(aTranslate.getX(), aTranslate.getY(), 0.0);
        }

        // transform from 2D scene-local to 3D eye coordinates
        const drawinglayer::geometry::ViewInformation3D aViewInfo3D(rVCScene.getViewInformation3D());
        basegfx::B3DHomMatrix aInverseViewToEye(aViewInfo3D.getDeviceToView() * aViewInfo3D.getProjection());
        aInverseViewToEye.invert();
        a3DSubTrans = aInverseViewToEye * a3DSubTrans;

        // build transform between object-local 3D and 3D eye coordinates
        basegfx::B3DHomMatrix aInverseOrientation(aViewInfo3D.getOrientation());
        aInverseOrientation.invert();
        basegfx::B3DHomMatrix mFullTransform(GetFullTransform());
        basegfx::B3DHomMatrix mTrans(mFullTransform);

        // build combined transformation
        mTrans *= aViewInfo3D.getOrientation();

        // apply adapted object-transformation
        mTrans = a3DSubTrans * mTrans;

        // transform to object.local coordinates
        mTrans *= aInverseOrientation;
        mFullTransform.invert();
        mTrans *= mFullTransform;

        // apply to 3D object
        basegfx::B3DHomMatrix mObjTrans(GetB3DTransform());
        mObjTrans *= mTrans;

        E3DModifySceneSnapRectUpdater aUpdater(this);
        SetB3DTransform(mObjTrans);
    }

    // reset local transformation
    maSdrObjectTransformation.setB2DHomMatrix(basegfx::B2DHomMatrix());
}

/*************************************************************************
|*
|* Parameter Geometrieerzeugung setzen
|*
\************************************************************************/

void E3dCompoundObject::SetCreateNormals(bool bNew)
{
    if(bCreateNormals != bNew)
    {
        bCreateNormals = bNew;
        ActionChanged();
    }
}

void E3dCompoundObject::SetCreateTexture(bool bNew)
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
            aViewInfo3D.getObjectToView() * GetB3DTransform());
        const sdr::contact::ViewContactOfE3dScene& rVCScene = static_cast< sdr::contact::ViewContactOfE3dScene& >(pRootScene->GetViewContact());
        aRetval.transform(rVCScene.getObjectTransformation());
    }

    return aRetval;
}

sal_Bool E3dCompoundObject::IsAOrdNumRemapCandidate(E3dScene*& prScene) const
{
    if(getParentOfSdrObject())
    {
        prScene = dynamic_cast< E3dScene* >(getParentOfSdrObject()->getSdrObjectFromSdrObjList());

        if(prScene)
        {
            return sal_True;
        }
    }

    return sal_False;
}

//////////////////////////////////////////////////////////////////////////////
// eof

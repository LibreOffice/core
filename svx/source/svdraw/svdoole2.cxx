/*************************************************************************
 *
 *  $RCSfile: svdoole2.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: jp $ $Date: 2001-03-08 21:15:38 $
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

#ifndef _COM_SUN_STAR_UTIL_XMODIFYBROADCASTER_HPP_
#include <com/sun/star/util/XModifyBroadcaster.hpp>
#endif

#include <comphelper/processfactory.hxx>

#ifdef SVX_LIGHT
#define SV_DECL_SDROLELINK_DEFINED
#define SdrOleLinkRef SdrOleLink*
#else
#ifndef _IPOBJ_HXX //autogen
#include <so3/ipobj.hxx>
#endif
#ifndef _SFX_INTERNO_HXX
#include <sfx2/interno.hxx>
#endif
#endif

#ifndef _SVDPAGV_HXX
#include <svdpagv.hxx>
#endif

#ifndef _GLOBNAME_HXX
#include <tools/globname.hxx>
#endif

#ifndef SVX_LIGHT
#ifndef _IPCLIENT_HXX //autogen
#include <so3/ipclient.hxx>
#endif
#ifndef _SV_JOBSET_HXX
#include <vcl/jobset.hxx>
#endif
#ifndef _SO_CLSIDS_HXX
#include <so3/clsids.hxx>
#endif
#endif

#include <sot/formats.hxx>

#ifndef _IPENV_HXX //autogen
#include <so3/ipenv.hxx>
#endif

#ifndef _PSEUDO_HXX
#include <so3/pseudo.hxx>
#endif
#ifndef _SVXLINKMGR_HXX //autogen
#include <linkmgr.hxx>
#endif

#include "svdoole2.hxx"
#include <svtools/solar.hrc>
#include "svdxout.hxx"
#ifndef SVTOOLS_URIHELPER_HXX
#include <svtools/urihelper.hxx>
#endif

#ifndef SVX_LIGHT
#include "svdpagv.hxx"
#endif

#include "svdmodel.hxx"
#include "svdio.hxx"
#include "svdglob.hxx"  // Stringcache
#include "svdstr.hrc"   // Objektname
#include "svdetc.hxx"
#include "svdview.hxx"
#include "unomlstr.hxx"

using namespace ::rtl;
using namespace ::com::sun::star;

////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef SVX_LIGHT

#ifndef _SOT_STORAGE_HXX
#include <sot/storage.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif

// warning: these things are copied from so3/source/inplace/plugin.cxx
SvGlobalName aPlgInName( 0x4caa7761, 0x6b8b, 0x11cf, 0x89,0xca,0x0,0x80,0x29,0xe4,0xb0,0xb1 );
#define PLUGIN_VERS 2

#else
#ifndef SO2_DECL_SVINPLACEOBJECT_DEFINED
#define SO2_DECL_SVINPLACEOBJECT_DEFINED
SO2_DECL_REF(SvInPlaceObject)
#endif
#endif // SVX_LIGHT


////////////////////////////////////////////////////////////////////////////////////////////////////

TYPEINIT1(SdrOle2Obj,SdrRectObj);

SdrOle2Obj::SdrOle2Obj(FASTBOOL bFrame_)
{
    Init();

    ppObjRef=new SvInPlaceObjectRef;
    bFrame=bFrame_;
}

SdrOle2Obj::SdrOle2Obj(const SvInPlaceObjectRef& rNewObjRef, FASTBOOL bFrame_)
{
    Init();
#ifndef SVX_LIGHT
    ppObjRef=new SvInPlaceObjectRef(rNewObjRef);
#endif // !SVX_LIGHT

    bFrame=bFrame_;

    SvInPlaceObjectRef& rIPRef = *ppObjRef;

#ifndef SVX_LIGHT
    if (rIPRef.Is() &&
        (rIPRef->GetMiscStatus() & SVOBJ_MISCSTATUS_NOTRESIZEABLE) )
    {
        SetResizeProtect(TRUE);
    }

#endif // !SVX_LIGHT
}

SdrOle2Obj::SdrOle2Obj(const SvInPlaceObjectRef& rNewObjRef, const XubString& rNewObjName, FASTBOOL bFrame_)
{
    Init();

#ifndef SVX_LIGHT
    ppObjRef=new SvInPlaceObjectRef(rNewObjRef);
#endif

    aName=rNewObjName;
    bFrame=bFrame_;

    SvInPlaceObjectRef& rIPRef = *ppObjRef;

#ifndef SVX_LIGHT
    if (rIPRef.Is() &&
        (rIPRef->GetMiscStatus() & SVOBJ_MISCSTATUS_NOTRESIZEABLE) )
    {
        SetResizeProtect(TRUE);
    }
#endif // !SVX_LIGHT
}

SdrOle2Obj::SdrOle2Obj(const SvInPlaceObjectRef& rNewObjRef, const XubString& rNewObjName, const Rectangle& rNewRect, FASTBOOL bFrame_):
    SdrRectObj(rNewRect)
{
    Init();

#ifndef SVX_LIGHT
    ppObjRef=new SvInPlaceObjectRef(rNewObjRef);
#endif

    aName=rNewObjName;
    bFrame=bFrame_;

    SvInPlaceObjectRef& rIPRef = *ppObjRef;

#ifndef SVX_LIGHT
    if (rIPRef.Is() &&
        (rIPRef->GetMiscStatus() & SVOBJ_MISCSTATUS_NOTRESIZEABLE) )
    {
        SetResizeProtect(TRUE);
    }
#endif
}

void SdrOle2Obj::Init()
{
    pModifyListener = NULL;
    pGraphic=NULL;
    pMetaFile=NULL;
}

SdrOle2Obj::~SdrOle2Obj()
{
#ifndef SVX_LIGHT
    // Aus Cache entfernen
    GetSdrGlobalData().GetOLEObjCache().RemoveObj(this);
#endif // !SVX_LIGHT

    ImpAbmeldung();

#ifndef SVX_LIGHT
    if (pModel!=NULL) {
        SvPersist* pPers=pModel->GetPersist();
        if (pPers!=NULL && ppObjRef->Is()) {
            pPers->Remove(*ppObjRef);
        }
    }
#endif

    delete ppObjRef;

    if(pGraphic!=NULL)
        delete pGraphic;

    if(pMetaFile!=NULL)
        delete pMetaFile;

#ifndef SVX_LIGHT
    if(pModifyListener)
    {
        pModifyListener->invalidate();
        pModifyListener->release();
        pModifyListener = NULL;
    }
#endif
}

void SdrOle2Obj::SetGraphic(const Graphic* pGrf)
{
    if ( pGraphic )
    {
        delete pGraphic;
        pGraphic = NULL;
    }

    if (pGrf!=NULL)
        pGraphic = new Graphic(*pGrf);

    if ( ppObjRef->Is() && pGrf )
        SendRepaintBroadcast();

    SetChanged();
}

FASTBOOL SdrOle2Obj::IsEmpty() const
{
    return !ppObjRef->Is();
}

void SdrOle2Obj::ImpAnmeldung()
{
#ifndef SVX_LIGHT
    if(pModel && aName.Len())
    {
        SvPersist* pPers=pModel->GetPersist();
        if (pPers!=NULL && ppObjRef->Is())
        {
            SvInfoObjectRef xIObj;
            SvInfoObject* pInfo = pPers->Find(aName);

            if (!pInfo)
            {
                xIObj = pInfo = new SvEmbeddedInfoObject(*ppObjRef,aName);
            }

            if ( !pPers->HasObject(aName) )
            {
                pPers->Move(pInfo, aName);
            }
            else
            {
                pInfo->SetDeleted(FALSE);
            }
        }

        // In Cache einfuegen
        GetSdrGlobalData().GetOLEObjCache().InsertObj(this);
    }

    if ( ppObjRef->Is() && (*ppObjRef)->GetMiscStatus() & SVOBJ_MISCSTATUS_RESIZEONPRINTERCHANGE )
    {
        if (pModel && pModel->GetRefDevice() &&
            pModel->GetRefDevice()->GetOutDevType() == OUTDEV_PRINTER)
        {
            // Kein RefDevice oder RefDevice kein Printer
            BOOL bModified = (*ppObjRef)->IsModified();
            Printer* pPrinter = (Printer*) pModel->GetRefDevice();
            (*ppObjRef)->OnDocumentPrinterChanged( pPrinter );
            (*ppObjRef)->SetModified( bModified );
        }
    }

    if( !IsEmpty() )
    {
        // register modify listener
        if( pModifyListener == NULL )
        {
            ((SdrOle2Obj*)this)->pModifyListener = new SvxUnoShapeModifyListener( (SdrOle2Obj*)this );
            pModifyListener->acquire();
        }

        uno::Reference< util::XModifyBroadcaster > xBC( getXModel(), uno::UNO_QUERY );
        if( xBC.is() && pModifyListener )
        {
            uno::Reference< util::XModifyListener > xListener( pModifyListener );
            xBC->addModifyListener( xListener );
        }
    }

#endif // SVX_LIGHT
}

void SdrOle2Obj::ImpAbmeldung()
{
#ifndef SVX_LIGHT
    if( !IsEmpty() && aName.Len() )
    {
        uno::Reference< util::XModifyBroadcaster > xBC( getXModel(), uno::UNO_QUERY );
        if( xBC.is() && pModifyListener )
        {
            uno::Reference< util::XModifyListener > xListener( pModifyListener );
            xBC->removeModifyListener( xListener );
        }
    }

    if (pModel && aName.Len() )
    {
        // Aus Cache entfernen
        GetSdrGlobalData().GetOLEObjCache().RemoveObj(this);

        if ( ppObjRef->Is() )
        {
            (*ppObjRef)->DoClose();

            SvPersist* pPers=pModel->GetPersist();

            if (pPers)
            {
                SvInfoObject* pInfo = pPers->Find(aName);

                if (pInfo)
                {
                    pInfo->SetDeleted(TRUE);
                }
            }
        }
    }
#endif // SVX_LIGHT
}

void SdrOle2Obj::SetModel(SdrModel* pNewModel)
{
    FASTBOOL bChg=pNewModel!=pModel;

    if (bChg) ImpAbmeldung();       // mit dem alten Namen

#ifndef SVX_LIGHT
    if (pModel && pNewModel)
    {
        SvPersist* pDestPers = pNewModel->GetPersist();
        SvPersist* pSrcPers  = pModel->GetPersist();

        if (pDestPers && pSrcPers)
        {
            if ( pDestPers != pSrcPers )
            {
                // Persists unterschiedlich (unterschiedliche Dokumente)
                // oder gleiches Dokument

                SvInfoObject* pInfo = pSrcPers->Find(aName);

                if (pInfo)
                {
                    USHORT i = 1;

                    // loop because of storage bug 46033
                    for( USHORT n = 0; n < 100; n++ )
                    {
                        aName=ImpGetResStr(bFrame ? STR_ObjFrameNamePrefix : STR_ObjOLE2NamePrefix);
                        aName += sal_Unicode(' ');
                        String aStr(aName);
                        aStr+=i;
                        while( pDestPers->Find( aStr ) )
                        {
                            i++;
                            aStr = aName;
                            aStr += String::CreateFromInt32(i);
                        }
                        aName = aStr;
                        const SvInPlaceObjectRef& aXRef = &pSrcPers->GetObject( aName );
                        if( pDestPers->Copy(aName, aName, pInfo, pSrcPers) )
                            break;

                        i++;
                    }

                    (*ppObjRef)=&(pDestPers->GetObject(aName));
                }
            }
        }
    }
#endif

    SdrRectObj::SetModel(pNewModel);
    if (bChg) ImpAnmeldung();
}

void SdrOle2Obj::SetPage(SdrPage* pNewPage)
{
    FASTBOOL bRemove=pNewPage==NULL && pPage!=NULL;
    FASTBOOL bInsert=pNewPage!=NULL && pPage==NULL;

    if (bRemove) ImpAbmeldung();

    SdrRectObj::SetPage(pNewPage);

    if (bInsert) ImpAnmeldung();
}

void SdrOle2Obj::SetObjRef(const SvInPlaceObjectRef& rNewObjRef)
{
    ImpAbmeldung();
    *ppObjRef=rNewObjRef;

    SvInPlaceObjectRef& rIPRef = *ppObjRef;

#ifndef SVX_LIGHT
    if (rIPRef.Is() &&
        (rIPRef->GetMiscStatus() & SVOBJ_MISCSTATUS_NOTRESIZEABLE) )
    {
        SetResizeProtect(TRUE);
    }
#endif // !SVX_LIGHT

    ImpAnmeldung();
    SetChanged();
    SendRepaintBroadcast();
}

FASTBOOL SdrOle2Obj::HasSetName() const
{
    return TRUE;
}

void SdrOle2Obj::SetName(const XubString& rStr)
{
    ImpAbmeldung();
    aName=rStr;
    ImpAnmeldung();
    SetChanged();
}

XubString SdrOle2Obj::GetName() const
{
    return aName;
}

void SdrOle2Obj::TakeObjInfo(SdrObjTransformInfoRec& rInfo) const
{
    rInfo.bRotateFreeAllowed=FALSE;
    rInfo.bRotate90Allowed  =FALSE;
    rInfo.bMirrorFreeAllowed=FALSE;
    rInfo.bMirror45Allowed  =FALSE;
    rInfo.bMirror90Allowed  =FALSE;
    rInfo.bTransparenceAllowed = FALSE;
    rInfo.bGradientAllowed = FALSE;
    rInfo.bShearAllowed     =FALSE;
    rInfo.bEdgeRadiusAllowed=FALSE;
    rInfo.bNoOrthoDesired   =FALSE;
    rInfo.bCanConvToPath    =FALSE;
    rInfo.bCanConvToPoly    =FALSE;
    rInfo.bCanConvToPathLineToArea=FALSE;
    rInfo.bCanConvToPolyLineToArea=FALSE;
    rInfo.bCanConvToContour = FALSE;
}

UINT16 SdrOle2Obj::GetObjIdentifier() const
{
    return bFrame ? UINT16(OBJ_FRAME) : UINT16(OBJ_OLE2);
}

FASTBOOL SdrOle2Obj::Paint(ExtOutputDevice& rOut, const SdrPaintInfoRec& rInfoRec) const
{
    FASTBOOL bOk=TRUE;

    if( !pGraphic )
        GetObjRef();    // try to load inplace object

    if (ppObjRef->Is())
    {
#ifndef SVX_LIGHT
        if( !bSizProt && (*ppObjRef)->GetMiscStatus() & SVOBJ_MISCSTATUS_NOTRESIZEABLE )
            ( (SdrOle2Obj*) this)->bSizProt = TRUE;

        const SvEditObjectProtocol& rProt=(*ppObjRef)->GetProtocol();

        OutputDevice* pOut=rOut.GetOutDev();

        if (!rProt.IsInPlaceActive() || (*ppObjRef)->GetIPClient()->GetEnv()->GetEditWin()!=pOut)
        {
            if ((rInfoRec.nPaintMode & SDRPAINTMODE_DRAFTGRAF) ==0)
            {
                if (((*ppObjRef)->GetMiscStatus() & SVOBJ_MISCSTATUS_ACTIVATEWHENVISIBLE))
                {
                    // PlugIn-Objekt connecten
                    if (rInfoRec.pPV!=NULL)
                    {
                        SdrOle2Obj* pOle2Obj = (SdrOle2Obj*) this;
                        SdrView* pSdrView = (SdrView*) &rInfoRec.pPV->GetView();
                        pSdrView->DoConnect(pOle2Obj);
                    }
                }
                pOut->Push( PUSH_CLIPREGION );
                pOut->IntersectClipRegion( aRect );
                (*ppObjRef)->DoDraw(pOut,aRect.TopLeft(),aRect.GetSize(),JobSetup());
                pOut->Pop();
            }
            else
            { // sonst SDRPAINTMODE_DRAFTGRAF
                Polygon aPoly(Rect2Poly(aRect,aGeo));
                pOut->SetLineColor(Color(COL_BLACK));
                pOut->DrawPolyLine(aPoly);
                pOut->DrawLine(aPoly[0],aPoly[2]);
                pOut->DrawLine(aPoly[1],aPoly[3]);
            }
        }
#else
        if (rInfoRec.pPV!=NULL)
        {
            SdrView* pSdrView = (SdrView*) &rInfoRec.pPV->GetView();
            pSdrView->DoConnect((SdrOle2Obj*) this);
        }
#endif // SVX_LIGHT

    }
    else if ( pGraphic )
    {
        OutputDevice* pOutDev=rOut.GetOutDev();
        if( IsEmptyPresObj() )
        {
            Size aSizePix(pOutDev->LogicToPixel(pGraphic->GetPrefSize(),pGraphic->GetPrefMapMode()));
            Size aSize(pOutDev->PixelToLogic(aSizePix));
            Point aPos(aRect.Center());
            aPos.X()-=aSize.Width() /2;
            aPos.Y()-=aSize.Height()/2;
            if (aPos.X() >= aRect.Left() && aPos.Y() >= aRect.Top())
                pGraphic->Draw(pOutDev,aPos);

            pOutDev->SetFillColor();
            pOutDev->SetLineColor( Color( COL_GRAY ) );
            pOutDev->DrawRect(aRect);
        }
        else
            pGraphic->Draw( pOutDev, aRect.TopLeft() );
    }

    if (HasText())
    {
        bOk=SdrTextObj::Paint(rOut,rInfoRec);
    }
    return bOk;
}

SdrObject* SdrOle2Obj::CheckHit(const Point& rPnt, USHORT nTol, const SetOfByte* pVisiLayer) const
{
    return ImpCheckHit(rPnt,nTol,pVisiLayer,TRUE,TRUE);
}

void SdrOle2Obj::TakeObjNameSingul(XubString& rName) const
{
    rName = ImpGetResStr(bFrame ? STR_ObjNameSingulFrame : STR_ObjNameSingulOLE2);

    if(aName.Len())
    {
        rName.AppendAscii(" '");
        rName += aName;
        rName += sal_Unicode('\'');
    }
}

void SdrOle2Obj::TakeObjNamePlural(XubString& rName) const
{
    rName=ImpGetResStr(bFrame ? STR_ObjNamePluralFrame : STR_ObjNamePluralOLE2);
}

void SdrOle2Obj::operator=(const SdrObject& rObj)
{
    FASTBOOL bModelOk=pModel!=NULL;
    if (bModelOk) ImpAbmeldung();
    SdrRectObj::operator=(rObj);
    aName    =((SdrOle2Obj&)rObj).aName;
    aProgName=((SdrOle2Obj&)rObj).aProgName;
    bFrame   =((SdrOle2Obj&)rObj).bFrame;

    if(((SdrOle2Obj&)rObj).pGraphic)
    {
        if(pGraphic)
            delete pGraphic;
        pGraphic = new Graphic(*((SdrOle2Obj&)rObj).pGraphic);
    }

    if (bModelOk) {
#ifndef SVX_LIGHT
        SvPersist* pDestPers=pModel->GetPersist();
        SdrModel* pSrcModel=((SdrOle2Obj&)rObj).GetModel();
        SvPersist* pSrcPers=pSrcModel!=NULL ? pSrcModel->GetPersist() : NULL;
        if (pDestPers!=NULL && pSrcPers!=NULL && pModel==pSrcModel) {
            // Kopie innerhalb eines Models
            // Eindeutigen Namen finden
            SvInfoObject* pInfo=pSrcPers->Find(aName);
            if (pInfo!=NULL)
            {
                USHORT i=1;

                // loop because of storage bug 46033
                for( USHORT n = 0; n < 100; n++ )
                {
                    aName=ImpGetResStr(bFrame ? STR_ObjFrameNamePrefix : STR_ObjOLE2NamePrefix);
                    aName += sal_Unicode(' ');
                    String aStr(aName);
                    aStr+=i;
                    while(pDestPers->Find(aStr)) {
                        i++;
                        aStr=aName;
                        aStr += String::CreateFromInt32(i);
                    }
                    aName=aStr;
                    const SvInPlaceObjectRef& aXRef = &pSrcPers->GetObject( aName );
                    if( pDestPers->Copy(aName,aName,pInfo,pSrcPers) )
                        break;

                    i++;
                }

                (*ppObjRef)=&(pDestPers->GetObject(aName));
            }
        }
#endif
        ImpAnmeldung();
    }
}

FASTBOOL SdrOle2Obj::HasSpecialDrag() const
{
    return FALSE;
}

void SdrOle2Obj::ImpSetVisAreaSize()
{
#ifndef SVX_LIGHT
    SvInPlaceObjectRef& rIPRef=*ppObjRef;

    if (rIPRef.Is())
    {
        if (rIPRef->GetMiscStatus() & SVOBJ_MISCSTATUS_SERVERRESIZE)
        {
            // Server resized selbst (StarChart)
            // Neue VisArea setzen
            Rectangle aVisArea = OutputDevice::LogicToLogic( aRect,
                                    pModel->GetScaleUnit(), rIPRef->GetMapUnit() );
            rIPRef->SetVisArea(aVisArea);

            // Wurde die VisArea akzeptiert?
            Rectangle aAcceptedVisArea(rIPRef->GetVisArea());

            if (aVisArea.GetSize() != aAcceptedVisArea.GetSize())
            {
                // VisArea wurde nicht akzeptiert -> korrigieren
                aRect.SetSize(OutputDevice::LogicToLogic( aAcceptedVisArea.GetSize(),
                        rIPRef->GetMapUnit(), pModel->GetScaleUnit()));
            }
        }
        else
        {
            /**********************************************************************
            * Nun wird nicht mehr die VisArea gesetzt, sondern es erfolgt eine
            * Skalierung
            **********************************************************************/
            SvEmbeddedClient* pClient = (*ppObjRef)->GetClient();

            if (pClient)
            {
                SvClientData* pData = pClient->GetClientData();
                Size aObjAreaSize = rIPRef->GetVisArea().GetSize();
                aObjAreaSize = OutputDevice::LogicToLogic( aObjAreaSize,
                                                           rIPRef->GetMapUnit(),
                                                           pModel->GetScaleUnit() );

                Size aSize = aRect.GetSize();
                Fraction aScaleWidth (aSize.Width(),  aObjAreaSize.Width() );
                Fraction aScaleHeight(aSize.Height(), aObjAreaSize.Height() );
                // Nun auf 10 Binaerstellen kuerzen
                Kuerzen(aScaleHeight, 10);
                Kuerzen(aScaleWidth,  10);

                pData->SetSizeScale(aScaleWidth, aScaleHeight);

                Rectangle aScaleRect(aRect.TopLeft(), aObjAreaSize);
                pData->SetObjArea(aScaleRect);
            }
        }
    }
#endif // SVX_LIGHT
}

void SdrOle2Obj::NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact)
{
    SdrRectObj::NbcResize(rRef,xFact,yFact);
    if (aGeo.nShearWink!=0 || aGeo.nDrehWink!=0) { // kleine Korrekturen
        if (aGeo.nDrehWink>=9000 && aGeo.nDrehWink<27000) {
            aRect.Move(aRect.Left()-aRect.Right(),aRect.Top()-aRect.Bottom());
        }
        aGeo.nDrehWink=0;
        aGeo.nShearWink=0;
        aGeo.nSin=0.0;
        aGeo.nCos=1.0;
        aGeo.nTan=0.0;
        SetRectsDirty();
    }
    ImpSetVisAreaSize();
}

void SdrOle2Obj::SetGeoData(const SdrObjGeoData& rGeo)
{
    SdrRectObj::SetGeoData(rGeo);
    ImpSetVisAreaSize();
}

void SdrOle2Obj::NbcSetSnapRect(const Rectangle& rRect)
{
    SdrRectObj::NbcSetSnapRect(rRect);
    ImpSetVisAreaSize();
}

void SdrOle2Obj::NbcSetLogicRect(const Rectangle& rRect)
{
    SdrRectObj::NbcSetLogicRect(rRect);
    ImpSetVisAreaSize();
}

FASTBOOL SdrOle2Obj::HasGDIMetaFile() const
{
    GetObjRef();    // try to load inplace object
    SvInPlaceObjectRef& rIPRef=*ppObjRef;
    BOOL bOK = FALSE;
    if ( rIPRef.Is() )
        bOK = TRUE;

    return bOK;
}

const GDIMetaFile* SdrOle2Obj::GetGDIMetaFile() const
{
#ifndef SVX_LIGHT
    if ( pMetaFile )
    {
        delete ((SdrOle2Obj*)this)->pMetaFile;
        ((SdrOle2Obj*)this)->pMetaFile = NULL;
    }

    GetObjRef();    // try to load inplace object
    SvInPlaceObjectRef& rIPRef=*ppObjRef;

    if (rIPRef.Is())
    {
        GDIMetaFile* pMtf=NULL;
        SvData aSvData(FORMAT_GDIMETAFILE);
        if (rIPRef->GetData(&aSvData))
        {
            if (aSvData.GetData(&pMtf,TRANSFER_REFERENCE))
            {
                // kopieren, weil *pMtf mit ~SvData zerstoert wird!
                ((SdrOle2Obj*)this)->pMetaFile = new GDIMetaFile(*pMtf);
            }
        }
    }
#endif // SVX_LIGHT
    return pMetaFile;
}

void SdrOle2Obj::WriteData(SvStream& rOut) const
{
    SdrRectObj::WriteData(rOut);
    SdrDownCompat aCompat(rOut,STREAM_WRITE); // Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
#ifdef DBG_UTIL
    aCompat.SetID("SdrOle2Obj");
#endif

    // UNICODE: rOut<<aName;
    rOut.WriteByteString(aName);

    // UNICODE: rOut<<aProgName;
    rOut.WriteByteString(aProgName);

    GetObjRef();
    BOOL bObjRefValid=ppObjRef->Is();
    rOut<<bObjRefValid;
    BOOL bPreview = FALSE;
    if( !IsEmptyPresObj() && pModel && pModel->IsSaveOLEPreview() )
        bPreview = TRUE;

    if( bPreview )
    {
        // set preview graphic (not for empty presentation objects)
        GetGDIMetaFile();
        if( pMetaFile )
        {
            Graphic* pGraph = new Graphic( *pMetaFile );
            ( (SdrOle2Obj*) this )->SetGraphic( pGraph );
        }
    }

    BOOL bHasGraphic=pGraphic!=NULL;
    rOut<<bHasGraphic;
    if (bHasGraphic)
    {
        SdrDownCompat aGrafCompat(rOut,STREAM_WRITE); // ab V11 eingepackt
#ifdef DBG_UTIL
        aGrafCompat.SetID("SdrOle2Obj(Graphic)");
#endif
        rOut<<*pGraphic;
    }

    if( bPreview )
        ( (SdrOle2Obj*) this )->SetGraphic( NULL );     // remove preview graphic
}

void SdrOle2Obj::ReadData(const SdrObjIOHeader& rHead, SvStream& rIn)
{
    rIn.SetError( 0 );

    if (rIn.GetError()!=0) return;
    SdrRectObj::ReadData(rHead,rIn);
    SdrDownCompat aCompat(rIn,STREAM_READ); // Fuer Abwaertskompatibilitaet (Lesen neuer Daten mit altem Code)
#ifdef DBG_UTIL
    aCompat.SetID("SdrOle2Obj");
#endif

    // UNICODE: rIn >> aName;
    rIn.ReadByteString(aName);

    // UNICODE: rIn >> aProgName;
    rIn.ReadByteString(aProgName);

    BOOL bObjRefValid;
    rIn>>bObjRefValid;

    BOOL bHasGraphic;
    rIn>>bHasGraphic;
    if (bHasGraphic) {
        if (pGraphic==NULL) pGraphic=new Graphic;
        if (rHead.GetVersion()>=11)
        { // ab V11 eingepackt
            SdrDownCompat aGrafCompat(rIn,STREAM_READ);
#ifdef DBG_UTIL
            aGrafCompat.SetID("SdrOle2Obj(Graphic)");
#endif
            rIn>>*pGraphic;
        }
        else
            rIn>>*pGraphic;
    }
}

void SdrOle2Obj::NbcMove(const Size& rSize)
{
    SdrRectObj::NbcMove(rSize);
    ImpSetVisAreaSize();
}

BOOL SdrOle2Obj::Unload()
{
    BOOL bUnloaded = FALSE;

    if( ppObjRef && ppObjRef->Is() )
    {
        //Nicht notwendig im Doc DTor (MM)
        ULONG nRefCount = (*ppObjRef)->GetRefCount();
        // prevent Unload if there are external references
        if( nRefCount > 2 )
            return FALSE;
        DBG_ASSERT( nRefCount == 2, "Wrong RefCount for unload" );
    }

#ifndef SVX_LIGHT

    if (pModel && ppObjRef && ppObjRef->Is() &&
        SVOBJ_MISCSTATUS_ALWAYSACTIVATE != (*ppObjRef)->GetMiscStatus() &&
        1 < (*ppObjRef)->GetRefCount()                                  &&
        !(*ppObjRef)->IsModified()                                      &&
        !(*ppObjRef)->GetProtocol().IsInPlaceActive() )
    {
        SvPersist* pPersist = pModel->GetPersist();

        if (pPersist)
        {
            SvPersist* pO = *ppObjRef;
            if( pO->IsModified() )
            {
                pO->DoSave();
                pO->DoSaveCompleted();
            }
            ppObjRef->Clear();
            if (pPersist->Unload(pO))
                bUnloaded = TRUE;
            else
                *ppObjRef = pO;
        }
    }

#else
    *ppObjRef = NULL;
#endif // !SVX_LIGHT

    return bUnloaded;
}

const SvInPlaceObjectRef& SdrOle2Obj::GetObjRef() const
{
#ifndef SVX_LIGHT
    if ( !ppObjRef->Is() && pModel && pModel->GetPersist() && !pModel->GetPersist()->IsHandsOff() )
    {
        // Objekt laden
        (*ppObjRef) = &( pModel->GetPersist()->GetObject( aName ) );

        if ( ppObjRef->Is() )
        {
            if( !IsEmptyPresObj() )
            {
                // #75637# remember modified status of model
                BOOL bWasChanged(pModel ? pModel->IsChanged() : FALSE);

                // perhaps preview not valid anymore
                // #75637# This line changes the modified state of the model
                ( (SdrOle2Obj*) this )->SetGraphic( NULL );

                // #75637# if status was not set before, force it back
                // to not set, so that SetGraphic(0L) above does not
                // set the modified state of the model.
                if(!bWasChanged && pModel && pModel->IsChanged())
                    pModel->SetChanged(FALSE);
            }

            if ( (*ppObjRef)->GetMiscStatus() & SVOBJ_MISCSTATUS_RESIZEONPRINTERCHANGE )
            {
                if (pModel && pModel->GetRefDevice() &&
                    pModel->GetRefDevice()->GetOutDevType() == OUTDEV_PRINTER)
                {
                    // Kein RefDevice oder RefDevice kein Printer
                    BOOL bModified = (*ppObjRef)->IsModified();
                    Printer* pPrinter = (Printer*) pModel->GetRefDevice();
                    (*ppObjRef)->OnDocumentPrinterChanged( pPrinter );
                    (*ppObjRef)->SetModified( bModified );
                }
            }

            // register modify listener on initial load
            if( pModifyListener == NULL )
            {
                ((SdrOle2Obj*)this)->pModifyListener = new SvxUnoShapeModifyListener( (SdrOle2Obj*)this );
                pModifyListener->acquire();

                uno::Reference< util::XModifyBroadcaster > xBC( getXModel(), uno::UNO_QUERY );
                if( xBC.is() && pModifyListener )
                {
                    uno::Reference< util::XModifyListener > xListener( pModifyListener );
                    xBC->addModifyListener( xListener );
                }
            }
        }
    }

    if (ppObjRef->Is())
    {
        // In Cache einfuegen
        GetSdrGlobalData().GetOLEObjCache().InsertObj((SdrOle2Obj*) this);
    }

#else

    if( !ppObjRef->Is() )
    {
        SotStorage* pStor = pModel->GetModelStorage();
        SotStorageRef xSt;
        if( pStor)
        {
            xSt = pStor->OpenSotStorage( aName );

            if( xSt.Is() )
            {
                *ppObjRef = SvInPlaceObject::Load( xSt );
                if( *ppObjRef && ppObjRef->Is() )
                    (*ppObjRef)->SetSizeAndPos( GetLogicRect() );
            }
        }
    }

#endif // !SVX_LIGHT
    return *ppObjRef;
}

uno::Reference< frame::XModel > SdrOle2Obj::getXModel() const
{
    uno::Reference< frame::XModel > xModel;

#ifndef SVX_LIGHT
    if( pModel )
    {
        SvInPlaceObjectRef xSvIPO( GetObjRef() );
        SfxInPlaceObjectRef xIPO( xSvIPO );
        if( xIPO.Is() )
        {
            SfxObjectShell* pShell = xIPO->GetObjectShell();

            if( pShell )
                xModel = pShell->GetModel();
        }
    }
#endif

    return xModel;
}

#ifdef SVX_LIGHT

#ifndef _COM_SUN_STAR_AWT_XTOPWINDOW_HPP_
#include <com/sun/star/awt/XTopWindow.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XWINDOW_HPP_
#include <com/sun/star/awt/XWindow.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_POSSIZE_HPP_
#include <com/sun/star/awt/PosSize.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XTOOLKIT_HPP_
#include <com/sun/star/awt/XToolkit.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XWINDOWPEER_HPP_
#include <com/sun/star/awt/XWindowPeer.hpp>
#endif
#ifndef _COM_SUN_STAR_PLUGIN_XPLUGINMANAGER_HPP_
#include <com/sun/star/plugin/XPluginManager.hpp>
#endif
#ifndef _COM_SUN_STAR_PLUGIN_PLUGINMODE_HPP_
#include <com/sun/star/plugin/PluginMode.hpp>
#endif

// here comes a dummy implementation for the SvInPlaceObject that is used
// to display plugins inside the StarOffice Player

SvInPlaceObject::SvInPlaceObject()
: pURL( NULL ),
  nPlugInMode( 0 )
{
}

SvInPlaceObject::~SvInPlaceObject()
{
}

// this method is called from the views DoConnect method
// Only the view knows the parent window for the plugin
void SvInPlaceObject::CreatePlugin( uno::Reference< awt::XToolkit > xToolkit,
                                    uno::Reference< awt::XWindowPeer > xPeer )
{
    if( xPlugin.is() )
        return;

    if( pURL )
    {
        ULONG nCount = aCmdList.Count();
        uno::Sequence< OUString > aCmds( nCount ), aArgs( nCount );
        OUString *pCmds = aCmds.getArray(), *pArgs = aArgs.getArray();
        for( ULONG i = 0; i < nCount; i++ )
        {
            SvCommand & rCmd = aCmdList.GetObject( i );
            pCmds[i] = rCmd.GetCommand();
            pArgs[i] = rCmd.GetArgument();
        }

        uno::Reference< lang::XMultiServiceFactory > xFactory( ::comphelper::getProcessServiceFactory() );
        if( xFactory.is() )
        {
            uno::Reference< plugin::XPluginManager > xPMgr( xFactory->createInstance( rtl::OUString::createFromAscii("com.sun.star.plugin.PluginManager")), uno::UNO_QUERY );

            INT16 nMode = nPlugInMode == PLUGIN_EMBEDED ? plugin::PluginMode::EMBED : plugin::PluginMode::FULL;

            if (xPMgr.is() )
                xPlugin = xPMgr->createPluginFromURL( xPMgr->createPluginContext(), nMode, aCmds, aArgs, xToolkit, xPeer, pURL->GetMainURL() );
        }
    }
}

void SvInPlaceObject::SetSizeAndPos( const Rectangle& rRect )
{
    uno::Reference< awt::XWindow > xWindow( xPlugin, uno::UNO_QUERY );

    if( xWindow.is() )
    {
        xWindow->setPosSize(rRect.getX(),rRect.getY(),rRect.getWidth(),rRect.getHeight(),awt::PosSize::POSSIZE);
        xWindow->setVisible(TRUE);
    }
}

SvInPlaceObject* SvInPlaceObject::Load( SotStorageRef xSt )
{
    SvInPlaceObject* pIPO = new SvInPlaceObject();

    String aFileName("plugin", gsl_getSystemTextEncoding());
    SotStorageStreamRef xStm = xSt->OpenSotStream( aFileName, STREAM_STD_READ );
    xStm->SetVersion( xStm->GetVersion() );
    xStm->SetBufferSize( 8192 );

    BYTE nVer = 0;
    *xStm >> nVer;
    if( nVer == 1 || nVer == PLUGIN_VERS )
    {
        *xStm >> pIPO->nPlugInMode;
        // Background gibt es nicht mehr
        if( pIPO->nPlugInMode == (USHORT)PLUGIN_BACKGROUND )
            pIPO->nPlugInMode = (USHORT)PLUGIN_EMBEDED;

        *xStm >> pIPO->aCmdList;
        DBG_ASSERT( !pIPO->pURL, "pURL exists in load" )
        BYTE bURLExist;
        *xStm >> bURLExist;
        if( bURLExist )
        {
            if( nVer == 1 )
            {
                // pIPO->pURL = new INetURLObject;
                // UNICODE: *xStm >> *pIPO->pURL;
                // old operator >> removed some versions ago, reconstructiong from older version
                ByteString aURL;

                xStm->ReadByteString( aURL );
                pIPO->pURL = new INetURLObject( aURL );

                // Ignore, not necessary
                BOOL bStrict;
                *xStm >> bStrict;
            }
            else
            {
                String aURL;
                // UNICODE: *xStm >> aURL;
                xStm->ReadByteString(aURL);
                pIPO->pURL = new INetURLObject( ::URIHelper::SmartRelToAbs( aURL, FALSE,
                                                                            INetURLObject::WAS_ENCODED,
                                                                            INetURLObject::DECODE_UNAMBIGUOUS ) );
            }
        }
        // UNICODE: *xStm >> pIPO->aMimeType;
        xStm->ReadByteString(pIPO->aMimeType);
    }
    else
        xStm->SetError( ERRCODE_IO_WRONGVERSION );

    if( xStm->GetError() != 0 )
    {
        delete pIPO;
        pIPO = NULL;
    }

    return pIPO;
}

// destroys the plugin for this inplace dummy
void SvInPlaceObject::DoDisconnect()
{
    if( xPlugin.is() )
    {
        uno::Reference< lang::XComponent > xComp( xPlugin, uno::UNO_QUERY );
        if( xComp.is() )
            xComp->dispose();
        xPlugin = NULL;
    }
}

#endif


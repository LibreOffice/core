/*************************************************************************
 *
 *  $RCSfile: base3d.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2000-11-14 10:32:39 $
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

#ifndef INCLUDED_SVTOOLS_OPTIONS3D_HXX
#include <svtools/options3d.hxx>
#endif

#ifndef _B3D_BASE3D_HXX
#include "base3d.hxx"
#endif

#ifndef _B3D_B3DENTITY_HXX
#include "b3dentty.hxx"
#endif

#ifndef _B3D_B3DTEX_HXX
#include "b3dtex.hxx"
#endif

#ifndef _B3D_B3DOPNGL_HXX
#include "b3dopngl.hxx"
#endif

#ifndef _B3D_B3DDEFLT_HXX
#include "b3ddeflt.hxx"
#endif

#ifndef _B3D_B3DPRINT_HXX
#include "b3dprint.hxx"
#endif

#ifndef _B3D_B3DGEOM_HXX
#include "b3dgeom.hxx"
#endif

#ifndef _B3D_B3DTRANS_HXX
#include "b3dtrans.hxx"
#endif

#ifndef _SHL_HXX
#include <tools/shl.hxx>
#endif

#ifndef _SV_POLY_HXX
#include <vcl/poly.hxx>
#endif


/*************************************************************************
|*
|* Konstruktor B3dGlobalData
|*
\************************************************************************/

B3dGlobalData::B3dGlobalData()
{
}

/*************************************************************************
|*
|* Destruktor B3dGlobalData
|*
\************************************************************************/

B3dGlobalData::~B3dGlobalData()
{
}

/*************************************************************************
|*
|* Konstruktor Base3D
|*
\************************************************************************/

Base3D::Base3D(OutputDevice* pOutDev)
:   OutDev3D(),
    pDevice(pOutDev),
    eObjectMode(Base3DPoints),
    aCurrentColor(Color(0xff, 0xff, 0xff)),
    aComplexPolygon(),
    eRenderModeFront(Base3DRenderFill),
    eRenderModeBack(Base3DRenderFill),
    eShadeModel(Base3DSmooth),
    eCullMode(Base3DCullNone),
    fPointSize(1.0),
    fLineWidth(1.0),
    pActiveTexture(NULL),
    pTransformationSet(NULL),
    pLightGroup(NULL),
    aMaterialFront(),
    aMaterialBack(),
    nDisplayQuality(127),
    bEdgeFlag(TRUE),
    bContextIsValid(TRUE),
    bPolyOffsetFill(FALSE),
    bPolyOffsetLine(FALSE),
    bPolyOffsetPoint(FALSE),
    bScissorRegionActive(FALSE),
    bDitherActive(TRUE)
{
    // Grundsaetzliche Materialeigenschaften setzen
    ResetMaterial(Base3DMaterialFrontAndBack);

    // Fuer OS/2 die FP-Exceptions abschalten
#if defined(OS2)
#define SC_FPEXCEPTIONS_ON()    _control87( MCW_EM, 0 )
#define SC_FPEXCEPTIONS_OFF()   _control87( MCW_EM, MCW_EM )
    SC_FPEXCEPTIONS_OFF();
#endif

    // Fuer WIN95/NT die FP-Exceptions abschalten
#if defined(WNT) || defined(WIN)
#define SC_FPEXCEPTIONS_ON()    _control87( _MCW_EM, 0 )
#define SC_FPEXCEPTIONS_OFF()   _control87( _MCW_EM, _MCW_EM )
    SC_FPEXCEPTIONS_OFF();
#endif
}

/*************************************************************************
|*
|* Destruktor Base3D
|*
\************************************************************************/

Base3D::~Base3D()
{
}

/*************************************************************************
|*
|* Erzeuge einen Base3D Kontext in Anhaengigkeit vom uebergebenen
|* OutputDevice und trage Ihn dort ein
|*
\************************************************************************/

Base3D* Base3D::Create(OutputDevice* pOutDev, BOOL bForcePrinter)
{
    Base3D* pRetval = NULL;
    if(pOutDev)
    {
        // Anforderungen feststellen
        BOOL bOwnDevice = FALSE;
        if(pOutDev->GetOutDevType() == OUTDEV_VIRDEV
            || pOutDev->GetOutDevType() == OUTDEV_PRINTER
            || pOutDev->GetConnectMetaFile() != NULL)
            bOwnDevice = TRUE;

        // Existiert schon ein 3D-Kontext, der auch an dieses
        // OutputDevice gebunden ist?
        if(pOutDev->Get3DContext()
            && ((Base3D*)(pOutDev->Get3DContext()))->GetOutputDevice() == pOutDev)
        {
            pRetval = (Base3D*)pOutDev->Get3DContext();
        }

        // Falls Ja, ist er den Anforderungen gewachsen?
        if(pRetval)
        {
            BOOL bForceNew(FALSE);

            if((!bForceNew) && (bOwnDevice)&&(pRetval->GetBase3DType() == BASE3D_TYPE_OPENGL))
            {
                bForceNew = TRUE;
            }

            if((!bForceNew) && (bForcePrinter)&&(pRetval->GetBase3DType() != BASE3D_TYPE_PRINTER))
            {
                bForceNew = TRUE;
            }

            if((!bForceNew) && (!bForcePrinter)&&(pRetval->GetBase3DType() == BASE3D_TYPE_PRINTER))
            {
                bForceNew = TRUE;
            }

            if(!bForceNew && !bOwnDevice)
            {
                // Versuchen, einen OpenGL Kontext zu bekommen? Teste das
                // globale Flag aus der .INI
                BOOL bUseOpenGL = SvtOptions3D().IsOpenGL();

                if((bUseOpenGL && pRetval->GetBase3DType() != BASE3D_TYPE_OPENGL)
                    || (!bUseOpenGL && pRetval->GetBase3DType() == BASE3D_TYPE_OPENGL))
                {
                    bForceNew = TRUE;
                    bOwnDevice = !bUseOpenGL;
                }
            }

            if(bForceNew)
            {
                pRetval->Destroy(pOutDev);
                pRetval = NULL;
            }
        }

        if(!pRetval)
        {
            // zerstoere altes Base3D, war nicht mehr an das
                // OutputDevice gebunden
            if(pOutDev->Get3DContext())
                pOutDev->Get3DContext()->Destroy(pOutDev);

            // erzeuge neues Base3D, je nach Anforderungen
            if(bForcePrinter)
            {
                pRetval = new Base3DPrinter(pOutDev);
            }
            else if(bOwnDevice)
            {
                pRetval = new Base3DDefault(pOutDev);
            }
            else
            {
                // Versuche OpenGL, fallback auf default
                // falls OpenGL nicht verfuegbar
                pRetval = CreateScreenRenderer(pOutDev);
            }

            // 3D-Kontext eintragen als Renderer im angegebenen OutputDevice
            if(pRetval)
                pOutDev->Set3DContext((Base3D*)pRetval);
        }
    }
    return pRetval;
}

/*************************************************************************
|*
|* Versuche, einen 3D Kontext zur Bildschirmdarstellung zu generieren.
|* Diese Funktion wird nur bei WNT realisiert. Sie muss entscheiden,
|* ob OpenGL-DLL's verfuegbar sind und entsprechend den OpenGL
|* Renderer oder einen Default Renderer erzeugen.
|*
\************************************************************************/

Base3D* Base3D::CreateScreenRenderer(OutputDevice* pOutDev)
{
    // OpenGL Kontext erzeugen
    Base3D* pRetval = NULL;

    // Versuchen, einen OpenGL Kontext zu bekommen? Teste das
    // globale Flag aus der .INI
    BOOL bUseOpenGL = SvtOptions3D().IsOpenGL();

    // Versuchen, einen OpenGL Kontext zu bekommen
    if(bUseOpenGL)
        pRetval = new Base3DOpenGL(pOutDev);

    if(!pRetval || !pRetval->IsContextValid())
    {
        if(pRetval)
        {
            delete pRetval;
            pRetval = NULL;
        }
    }

    // versuche alternativ, einen Default Renderer zu inkarnieren
    if(!pRetval)
        pRetval = new Base3DDefault(pOutDev);

    return pRetval;
}

/*************************************************************************
|*
|* Entferne den Kontext aus dem assoziierten OutputDevice und zerstoere
|* sich selbst
|*
|* Als Platzhalter fuer den Zeiger auf einen Kontext im OutputDevice
|* wird momentan pTheCurrentBase3DIncarnation benutzt
|*
\************************************************************************/

void Base3D::Destroy(OutputDevice *pOutDev)
{
    Base3D* pTmp;
    if((pTmp = (Base3D*)GetOutputDevice()->Get3DContext()) == this)
    {
        GetOutputDevice()->Set3DContext(NULL);
        delete pTmp;
    }
}

/*************************************************************************
|*
|* TransformationSet setzen
|*
\************************************************************************/

void Base3D::SetTransformationSet(B3dTransformationSet* pSet)
{
    // Aktuelles TransformationSet eintragen
    pTransformationSet = pSet;
}

/*************************************************************************
|*
|* Beleuchtung setzen
|*
\************************************************************************/

void Base3D::SetLightGroup(B3dLightGroup* pSet, BOOL bSetGlobal)
{
    // Aktuelle Beleuchtung eintragen
    pLightGroup = pSet;
}

/*************************************************************************
|*
|* Scissoring Region setzen
|*
\************************************************************************/

void Base3D::SetScissorRegionPixel(const Rectangle& rRect, BOOL bActivate)
{
    aScissorRectangle = rRect;
    ActivateScissorRegion(bActivate);
}

void Base3D::SetScissorRegion(const Rectangle& rRect, BOOL bActivate)
{
    SetScissorRegionPixel(pDevice->LogicToPixel(rRect), bActivate);
}

/*************************************************************************
|*
|* Scissoring aktivieren/deaktivieren
|*
\************************************************************************/

void Base3D::ActivateScissorRegion(BOOL bNew)
{
    if(bNew != bScissorRegionActive)
        bScissorRegionActive = bNew;
}

/*************************************************************************
|*
|* Dithering setzen
|*
\************************************************************************/

void Base3D::SetDither(BOOL bNew)
{
    bDitherActive = bNew;
}

/*************************************************************************
|*
|* Objektmodus holen
|*
\************************************************************************/

Base3DObjectMode Base3D::GetObjectMode()
{
    return eObjectMode;
}

/*************************************************************************
|*
|* Ein neues Primitiv vom Typ nMode starten
|*
\************************************************************************/

void Base3D::StartPrimitive(Base3DObjectMode eMode)
{
    eObjectMode = eMode;
    if(eObjectMode > Base3DPolygon)
    {
        switch(eObjectMode)
        {
            case Base3DComplexPolygonCut :
            {
                // Neues Polygon beginnen
                aComplexPolygon.SetTestForCut(TRUE);
                aComplexPolygon.StartPrimitive();
                break;
            }
            case Base3DComplexPolygon :
            {
                // Neues Polygon beginnen
                aComplexPolygon.SetTestForCut(FALSE);
                aComplexPolygon.StartPrimitive();
                break;
            }
        }
    }
    else
    {
        ImplStartPrimitive();
    }
}

/*************************************************************************
|*
|* Primitiv beenden
|*
\************************************************************************/

void Base3D::EndPrimitive()
{
    if(eObjectMode > Base3DPolygon)
    {
        switch(eObjectMode)
        {
            case Base3DComplexPolygon :
            case Base3DComplexPolygonCut :
            {
                // Selbst fuer Darstellung sorgen
                aComplexPolygon.EndPrimitive(this);
                break;
            }
        }
    }
    else
    {
        ImplEndPrimitive();
    }
}

/*************************************************************************
|*
|* Ein Objekt in Form einer B3dGeometry direkt ausgeben
|*
\************************************************************************/

void Base3D::DrawPolygonGeometry(B3dGeometry& rGeometry, BOOL bOutline)
{
    // Buckets der Geometrie holen
    B3dEntityBucket& rEntityBucket = rGeometry.GetEntityBucket();
    GeometryIndexValueBucket& rIndexBucket = rGeometry.GetIndexBucket();

    UINT32 nPolyCounter = 0;
    UINT32 nEntityCounter = 0;
    UINT32 nUpperBound;

    while(nPolyCounter < rIndexBucket.Count())
    {
        // Naechstes Primitiv
        nUpperBound = rIndexBucket[nPolyCounter].GetIndex();

        if(bOutline)
        {
            // Polygon als Outline ausgeben
            SetRenderMode(Base3DRenderLine);
            SetPolygonOffset(Base3DPolygonOffsetLine, TRUE);

            // ALLE Linien Zeichnen
            SetCullMode(Base3DCullNone);
        }
        else
        {
            // Polygone gefuellt ausgeben
            SetRenderMode(Base3DRenderFill);
            SetPolygonOffset(Base3DPolygonOffsetLine, FALSE);
        }

        if(rIndexBucket[nPolyCounter++].GetMode() == B3D_INDEX_MODE_LINE)
        {
            eObjectMode = Base3DLineStrip;
        }
        else
        {
            eObjectMode = Base3DPolygon;
        }

        // Neues Polygon beginnen
        ImplStartPrimitive();

        // Polygon ausgeben
        while(nEntityCounter < nUpperBound)
        {
            B3dEntity& rEntity = ImplGetFreeEntity();
            rEntity = rEntityBucket[nEntityCounter++];
            if(bOutline)
            {
                rEntity.SetNormalUsed(FALSE);
                rEntity.SetTexCoorUsed(FALSE);
                SetEdgeFlag(rEntity.IsEdgeVisible());
            }
            ImplPostAddVertex(rEntity);
        }

        // Primitiv abschliessen
        ImplEndPrimitive();
    }
}

/*************************************************************************
|*
|* Direkter Zugriff auf B3dMaterial fuer abgeleitete Klassen
|*
\************************************************************************/

B3dMaterial& Base3D::GetMaterialObject(Base3DMaterialMode eMode)
{
    if(eMode == Base3DMaterialFront)
        return aMaterialFront;
    return aMaterialBack;
}

/*************************************************************************
|*
|* geometrische Daten uebernehmen
|*
\************************************************************************/

void Base3D::AddVertex(Vector3D& rVertex)
{
    // Platz fuer neue Daten holen
    B3dEntity& rEntity = GetFreeEntity();
    rEntity.Reset();

    // geometrische Daten
    rEntity.Point() = Point4D(rVertex);
    rEntity.SetValid();

    // Nachbehandlung
    PostAddVertex(rEntity);
}

void Base3D::AddVertex(Vector3D& rVertex, Vector3D& rNormal)
{
    // Platz fuer neue Daten holen
    B3dEntity& rEntity = GetFreeEntity();
    rEntity.Reset();

    // geometrische Daten
    rEntity.Point() = Point4D(rVertex);
    rEntity.SetValid();

    // Normale
    if(GetLightGroup() && GetLightGroup()->IsLightingEnabled())
    {
        rEntity.Normal() = rNormal;
        rEntity.SetNormalUsed();
    }

    // Nachbehandlung
    PostAddVertex(rEntity);
}

void Base3D::AddVertex(Vector3D& rVertex, Vector3D& rNormal,
    Vector3D& rTexPos)
{
    // Platz fuer neue Daten holen
    B3dEntity& rEntity = GetFreeEntity();
    rEntity.Reset();

    // geometrische Daten
    rEntity.Point() = Point4D(rVertex);
    rEntity.SetValid();

    // Normale
    if(GetLightGroup() && GetLightGroup()->IsLightingEnabled())
    {
        rEntity.Normal() = rNormal;
        rEntity.SetNormalUsed();
    }

    // Texturdaten
    rEntity.TexCoor() = rTexPos;
    rEntity.SetTexCoorUsed();

    // Nachbehandlung
    PostAddVertex(rEntity);
}

void Base3D::AddVertex(B3dEntity& rEnt)
{
    // Platz fuer neue Daten holen
    B3dEntity& rEntity = GetFreeEntity();

    // Kopieren
    rEntity = rEnt;

    // Nachbehandlung
    PostAddVertex(rEntity);
}

void Base3D::PostAddVertex(B3dEntity& rEntity)
{
    // Flag fuer die Sichtbarkeit von Kanten kopieren
    rEntity.SetEdgeVisible(GetEdgeFlag());

    // aktuelle Farbe eintragen
    rEntity.Color().SetColor(GetColor().GetColor());

    if(eObjectMode > Base3DPolygon)
    {
        switch(eObjectMode)
        {
            case Base3DComplexPolygon :
            case Base3DComplexPolygonCut :
            {
                // Punkt ist nun ausgefuellt
                aComplexPolygon.PostAddVertex(rEntity);
                break;
            }
        }
    }
    else
    {
        ImplPostAddVertex(rEntity);
    }
}

/*************************************************************************
|*
|* Platz fuer neuen Punkt anfordern
|*
\************************************************************************/

B3dEntity& Base3D::GetFreeEntity()
{
    if(eObjectMode > Base3DPolygon)
    {
        switch(eObjectMode)
        {
            case Base3DComplexPolygon :
            case Base3DComplexPolygonCut :
            {
                // Im eigenen Buffer anlegen
                return aComplexPolygon.GetFreeEntity();
                break;
            }
        }
    }
    return ImplGetFreeEntity();
}

/*************************************************************************
|*
|* Farbe setzen
|*
\************************************************************************/

void Base3D::SetColor(Color aNew)
{
    // Farbe setzen
    if(GetOutputDevice()->GetDrawMode() & DRAWMODE_GRAYFILL)
    {
        // Graustufen
        UINT8 nLuminance = aNew.GetLuminance();
        aCurrentColor = Color(nLuminance, nLuminance, nLuminance);
    }
    else if(GetOutputDevice()->GetDrawMode() & DRAWMODE_WHITEFILL)
    {
        // Keine Ausgabe, hier Schwarz als Farbe setzen, wird
        // als Linienfarbe benutzt
        aCurrentColor = Color(COL_BLACK);
    }
    else
    {
        // Normale Farbausgabe
        aCurrentColor = aNew;
    }
}

/*************************************************************************
|*
|* Farbe liefern
|*
\************************************************************************/

Color Base3D::GetColor()
{
    return aCurrentColor;
}

/*************************************************************************
|*
|* Materialeigenschaften setzen
|*
\************************************************************************/

void Base3D::SetMaterial(Color aNew,
    Base3DMaterialValue eVal,
    Base3DMaterialMode eMode)
{
    Color aSource;
    if(GetOutputDevice()->GetDrawMode() & DRAWMODE_GRAYFILL)
    {
        // Graustufen
        UINT8 nLuminance = aNew.GetLuminance();
        aSource.SetRed(nLuminance);
        aSource.SetGreen(nLuminance);
        aSource.SetBlue(nLuminance);
        aSource.SetTransparency(aNew.GetTransparency());
    }
    else if(GetOutputDevice()->GetDrawMode() & DRAWMODE_WHITEFILL)
    {
        // Keine Ausgabe, hier Weiss als Farbe setzen
        aSource = Color(COL_WHITE);
    }
    else
    {
        // Normale Farbausgabe
        aSource = aNew;
    }

    if(eMode == Base3DMaterialFrontAndBack
        || eMode == Base3DMaterialFront)
    {
        aMaterialFront.SetMaterial(aSource, eVal);
    }
    if(eMode == Base3DMaterialFrontAndBack
        || eMode == Base3DMaterialBack)
    {
        aMaterialBack.SetMaterial(aSource, eVal);
    }
}

/*************************************************************************
|*
|* Materialeigenschaften abfragen
|*
\************************************************************************/

Color Base3D::GetMaterial(Base3DMaterialValue eVal,
    Base3DMaterialMode eMode)
{
    if(eMode == Base3DMaterialFrontAndBack
        || eMode == Base3DMaterialFront)
    {
        return aMaterialFront.GetMaterial(eVal);
    }
    return aMaterialBack.GetMaterial(eVal);
}

/*************************************************************************
|*
|* Materialeigenschaften setzen, exponent der specular-Eigenschaft
|*
\************************************************************************/

void Base3D::SetShininess(UINT16 nExponent,
    Base3DMaterialMode eMode)
{
    if(eMode == Base3DMaterialFrontAndBack
        || eMode == Base3DMaterialFront)
    {
        aMaterialFront.SetShininess(nExponent);
    }
    if(eMode == Base3DMaterialFrontAndBack
        || eMode == Base3DMaterialBack)
    {
        aMaterialBack.SetShininess(nExponent);
    }
}

/*************************************************************************
|*
|* Materialeigenschaften abfragen, exponent der specular-Eigenschaft
|*
\************************************************************************/

UINT16 Base3D::GetShininess(Base3DMaterialMode eMode)
{
    if(eMode == Base3DMaterialFrontAndBack
        || eMode == Base3DMaterialFront)
    {
        return aMaterialFront.GetShininess();
    }
    return aMaterialBack.GetShininess();
}

/*************************************************************************
|*
|* Materialeigenschaften auf Ausgangszustand
|*
\************************************************************************/

void Base3D::ResetMaterial(Base3DMaterialMode eMode)
{
    Color aColor(255, 51, 51, 51);
    SetMaterial(aColor, Base3DMaterialAmbient, eMode);
    aColor.SetColor(TRGB_COLORDATA(255, 204, 204, 204));
    SetMaterial(aColor, Base3DMaterialDiffuse, eMode);
    aColor.SetColor(TRGB_COLORDATA(255, 0, 0, 0));
    SetMaterial(aColor, Base3DMaterialSpecular, eMode);
    aColor.SetColor(TRGB_COLORDATA(255, 0, 0, 0));
    SetMaterial(aColor, Base3DMaterialEmission, eMode);
    SetShininess(0, eMode);
}

/*************************************************************************
|*
|* GlobalData holen
|*
\************************************************************************/

B3dGlobalData& Base3D::GetGlobalData()
{
    B3dGlobalData** ppGlobalData = (B3dGlobalData**)GetAppData(SHL_BASE3D);
    if(*ppGlobalData)
        return **ppGlobalData;

    // GlobalData anlegen
    *ppGlobalData = new B3dGlobalData;
    return **ppGlobalData;
}

/*************************************************************************
|*
|* TextureStore aus GlobalData holen
|*
\************************************************************************/

B3dTextureStore& Base3D::GetTextureStore()
{
    return GetGlobalData().GetTextureStore();
}

/*************************************************************************
|*
|* Textur mit den angegebenen Attributen als Grundlage anfordern.
|*
\************************************************************************/

B3dTexture* Base3D::ObtainTexture(TextureAttributes& rAtt)
{
    B3dTexture* pRetval = NULL;

    // Textur suchen und bei Treffer zurueckgeben
    B3dTextureStore& rTextureStore = GetTextureStore();
    for(UINT16 a=0;a<rTextureStore.Count();a++)
    {
        if(rTextureStore[a]->GetAttributes() == rAtt)
        {
            pRetval = rTextureStore[a];
            pRetval->Touch();
        }
        else
        {
            rTextureStore[a]->DecrementUsageCount();

            // Auf zu loeschende Texturen testen
            if(!rTextureStore[a]->GetUsageCount())
            {
                B3dTexture *pTex = rTextureStore[a];
                rTextureStore.Remove(a);
                DestroyTexture(pTex);
                a--;
            }
        }
    }

    // Textur zurueckgeben
    return pRetval;
}

/*************************************************************************
|*
|* Textur mit den angegebenen Attributen als Grundlage anfordern. Falls
|* eine solche Textur nicht existiert, erzeuge eine und gib diese zurueck
|*
\************************************************************************/

B3dTexture* Base3D::ObtainTexture(TextureAttributes& rAtt, Bitmap& rBitmap)
{
    B3dTexture* pRetval = ObtainTexture(rAtt);

    if(!pRetval)
    {
        // Existiert tatsaechlich nicht, generiere eine neue Textur
        B3dTextureStore& rTextureStore = GetTextureStore();

        pRetval = CreateTexture(rAtt, rBitmap);
        rTextureStore.Insert((const B3dTexture*&)pRetval, rTextureStore.Count());
    }

    // Textur zurueckgeben
    return pRetval;
}

/*************************************************************************
|*
|* Gezielt eine Textur freigeben
|*
\************************************************************************/

void Base3D::DeleteTexture(TextureAttributes& rAtt)
{
    B3dTexture* pTexture = NULL;

    // Textur suchen
    B3dTextureStore& rTextureStore = GetTextureStore();
    UINT16 a;
    for(a=0;a<rTextureStore.Count();a++)
    {
        if(rTextureStore[a]->GetAttributes() == rAtt)
        {
            pTexture = rTextureStore[a];
        }
    }

    if(pTexture)
    {
        if(pTexture == pActiveTexture)
            pActiveTexture = NULL;

        rTextureStore.Remove(a);
        DestroyTexture(pTexture);
    }
}

/*************************************************************************
|*
|* Alle Texturen freigeben
|*
\************************************************************************/

void Base3D::DeleteAllTextures()
{
    pActiveTexture = NULL;

    B3dTextureStore& rTextureStore = GetTextureStore();
    while(rTextureStore.Count())
    {
        B3dTexture *pTex = rTextureStore[0];
        rTextureStore.Remove(0);
        DestroyTexture(pTex);
    }
}

/*************************************************************************
|*
|* Ein Textur-Objekt inkarnieren
|*
\************************************************************************/

B3dTexture* Base3D::CreateTexture(TextureAttributes& rAtt, Bitmap& rBitmap)
{
    B3dTexture* pRetval = new B3dTexture(rAtt, rBitmap);
    DBG_ASSERT(pRetval,"AW: Kein Speicher fuer Textur bekommen!");
    return pRetval;
}

/*************************************************************************
|*
|* Normale Textur loeschen
|*
\************************************************************************/

void Base3D::DestroyTexture(B3dTexture* pTex)
{
    delete pTex;
}

/*************************************************************************
|*
|* Aktuell zu benutzende Textur setzen
|*
\************************************************************************/

void Base3D::SetActiveTexture(B3dTexture* pTex)
{
    if(pTex)
    {
        if(GetOutputDevice()->GetDrawMode() & DRAWMODE_GRAYFILL)
        {
            // Graustufen
            pTex->SetTextureKind(Base3DTextureIntensity);
        }
        else if(GetOutputDevice()->GetDrawMode() & DRAWMODE_WHITEFILL)
        {
            // Keine Ausgabe, keine Textur setzen
            pTex = NULL;
        }
    }

    // ... und setzen
    pActiveTexture = pTex;
}

/*************************************************************************
|*
|* Darstellungsqualitaet setzen
|*
\************************************************************************/

void Base3D::SetDisplayQuality(UINT8 nNew)
{
    nDisplayQuality = nNew;
}

/*************************************************************************
|*
|* Darstellungsqualitaet lesen
|*
\************************************************************************/

UINT8 Base3D::GetDisplayQuality()
{
    return nDisplayQuality;
}

/*************************************************************************
|*
|* PolygonOffset setzen
|*
\************************************************************************/

void Base3D::SetPolygonOffset(Base3DPolygonOffset eNew, BOOL bNew)
{
    switch(eNew)
    {
        case Base3DPolygonOffsetFill :
            bPolyOffsetFill = bNew;
            break;

        case Base3DPolygonOffsetLine :
            bPolyOffsetLine = bNew;
            break;

        case Base3DPolygonOffsetPoint :
            bPolyOffsetPoint = bNew;
            break;
    }
}

/*************************************************************************
|*
|* PolygonOffset lesen
|*
\************************************************************************/

BOOL Base3D::GetPolygonOffset(Base3DPolygonOffset eNew)
{
    if(eNew == Base3DPolygonOffsetLine)
        return bPolyOffsetLine;
    if(eNew == Base3DPolygonOffsetFill)
        return bPolyOffsetFill;
    return bPolyOffsetPoint;
}

/*************************************************************************
|*
|* EdgeFlag lesen
|*
\************************************************************************/

BOOL Base3D::GetEdgeFlag()
{
    return bEdgeFlag;
}

/*************************************************************************
|*
|* EdgeFlag schreiben
|*
\************************************************************************/

void Base3D::SetEdgeFlag(BOOL bNew)
{
    bEdgeFlag = bNew;
}

/*************************************************************************
|*
|* PointSize lesen
|*
\************************************************************************/

double Base3D::GetPointSize()
{
    return fPointSize;
}

/*************************************************************************
|*
|* PointSize schreiben
|*
\************************************************************************/

void Base3D::SetPointSize(double fNew)
{
    fPointSize = fNew;
}

/*************************************************************************
|*
|* LineWidth lesen
|*
\************************************************************************/

double Base3D::GetLineWidth()
{
    return fLineWidth;
}

/*************************************************************************
|*
|* LineWidth schreiben
|*
\************************************************************************/

void Base3D::SetLineWidth(double fNew)
{
    fLineWidth = fNew;
}

/*************************************************************************
|*
|* RenderMode setzen
|*
\************************************************************************/

void Base3D::SetRenderMode(Base3DRenderMode eNew,
    Base3DMaterialMode eMode)
{
    if(eMode == Base3DMaterialFrontAndBack
        || eMode == Base3DMaterialFront)
    {
        eRenderModeFront = eNew;
    }
    if(eMode == Base3DMaterialFrontAndBack
        || eMode == Base3DMaterialBack)
    {
        eRenderModeBack = eNew;
    }
}

/*************************************************************************
|*
|* RenderMode lieferen
|*
\************************************************************************/

Base3DRenderMode Base3D::GetRenderMode(Base3DMaterialMode eMode)
{
    if(eMode == Base3DMaterialFrontAndBack
        || eMode == Base3DMaterialFront)
    {
        return eRenderModeFront;
    }
    return eRenderModeBack;
}

/*************************************************************************
|*
|* ShadeModel setzen
|*
\************************************************************************/

void Base3D::SetShadeModel(Base3DShadeModel eNew)
{
    eShadeModel = eNew;
}

/*************************************************************************
|*
|* ShadeModel lieferen
|*
\************************************************************************/

Base3DShadeModel Base3D::GetShadeModel()
{
    return eShadeModel;
}

/*************************************************************************
|*
|* CullingMode setzen
|*
\************************************************************************/

void Base3D::SetCullMode(Base3DCullMode eNew)
{
    eCullMode = eNew;
}

/*************************************************************************
|*
|* CullingMode liefern
|*
\************************************************************************/

Base3DCullMode Base3D::GetCullMode()
{
    return eCullMode;
}

/*************************************************************************
|*
|* Texturenverwaltung
|*
\************************************************************************/

SV_IMPL_PTRARR(B3dTextureStore, B3dTexture*);


#ifdef DBG_UTIL
#include "b3dtest.cxx"
#endif


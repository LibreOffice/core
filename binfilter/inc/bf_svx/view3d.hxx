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

#ifndef _E3D_VIEW3D_HXX
#define _E3D_VIEW3D_HXX

#include <bf_svx/svdview.hxx>

#include <bf_svx/def3d.hxx>

#include <bf_svx/deflt3d.hxx>

#include <bf_goodies/vector3d.hxx>
namespace binfilter {

//************************************************************
//   Vorausdeklarationen
//************************************************************

class E3dObject;
class E3dScene;
class SceneList;

/*************************************************************************
|*
|* Ableitung von SdrView zur Bearbeitung von 3D-Objekten
|*
\************************************************************************/

class E3dView : public SdrView
{
protected:
    SdrObject**					pMarkedObjs;
    XPolyPolygon*				pMirrorPolygon;
    XPolyPolygon*				pMirroredPolygon;
    SdrPageView*				pMyPV;

    E3dDefaultAttributes		a3DDefaultAttr;
    MouseEvent					aMouseEvent;					// Die Parameter der letzten Events (Mouse, Keyboard)
    Vector3D					aDefaultTranslation;            // wird das Objekt erstmalig verschoben ?
    Vector3D					aDefaultLightPos;               // wo ist das gerichtete Licht ?
//-/	Vector3D					aDefaultCamPos;                 // nur die Z-Achse (aus dem Bildschirm heraus)
    Color						aDefaultLightColor;             // dito mit den Farben
    Color						aDefaultAmbientColor;

    double						fDefaultScaleX;                 // Verzerrungen
    double						fDefaultScaleY;
    double						fDefaultScaleZ;
    double						fDefaultRotateX;                // und Drehungen
    double						fDefaultRotateY;
    double						fDefaultRotateZ;
    double						fDefaultExtrusionDeepth;        // Extrusionstiefe
//-/	double						fDefaultCamFocal;               // Brennweite der Kamera
    double						fDefaultLightIntensity;         // Intensitaeten der beiden (notwendigen) Licht-
    double						fDefaultAmbientIntensity;       // quellen
    long						nHDefaultSegments;              // wieviele HSegmente braucht mein Lathe-Ojekt
    long						nVDefaultSegments;              // wieviele VSegmente braucht mein Lathe-Ojekt
    long						nPolyCnt;

    E3dDragConstraint			eDragConstraint;
    E3dDragDetail				eDragDetail;

    BOOL						b3dCreationActive;
    BOOL						bDoubleSided;

    void InitView();



public:
    TYPEINFO();

    E3dView(SdrModel* pModel, OutputDevice* pOut);
    virtual ~E3dView();

    // Alle markierten Objekte auf dem angegebenen OutputDevice ausgeben.

    // Zugriff auf die Default-Attribute
    E3dDefaultAttributes& Get3DDefaultAttributes() { return a3DDefaultAttr; }
    virtual	void CheckPossibilities();

    // Event setzen/rausruecken
    void SetMouseEvent(const MouseEvent& rNew) { aMouseEvent = rNew; }
    const MouseEvent& GetMouseEvent() { return aMouseEvent; }

    // Nachtraeglichhe Korrekturmoeglichkeit um alle Extrudes in einer
    // bestimmten Tiefensortierung anzulegen
    void DoDepthArrange(E3dScene* pScene, double fDepth);
    void Set3DDragConstraint(E3dDragConstraint eConstr) { eDragConstraint = eConstr; }
    E3dDragConstraint Get3DDragConstraint() { return eDragConstraint; }
    void Set3DDragDetail(E3dDragDetail eDetail)	{ eDragDetail = eDetail; }
    E3dDragDetail Get3DDragDetail() { return eDragDetail; }
    BOOL IsCreationActive() const { return b3dCreationActive; }

    const Vector3D &DefaultTranslation () const
    {
        return aDefaultTranslation;
    }

    Vector3D &DefaultTranslation ()
    {
        return aDefaultTranslation;
    }

    const Vector3D &DefaultLightPos () const
    {
        return aDefaultLightPos;
    }

    Vector3D &DefaultLightPos ()
    {
        return aDefaultLightPos;
    }

//-/	const Vector3D &DefaultCamPos () const
//-/	{
//-/		return aDefaultCamPos;
//-/	}

//-/	Vector3D &DefaultCamPos ()
//-/	{
//-/		return aDefaultCamPos;
//-/	}

    double &DefaultScaleX ()
    {
        return fDefaultScaleX;
    }

    double DefaultScaleX () const
    {
        return fDefaultScaleX;
    }

    double &DefaultScaleY ()
    {
        return fDefaultScaleY;
    }

    double DefaultScaleY () const
    {
        return fDefaultScaleY;
    }

    double &DefaultScaleZ ()
    {
        return fDefaultScaleZ;
    }

    double DefaultScaleZ () const
    {
        return fDefaultScaleZ;
    }

    double &DefaultRotateX ()
    {
        return fDefaultRotateX;
    }

    double DefaultRotateX () const
    {
        return fDefaultRotateX;
    }

    double &DefaultRotateY ()
    {
        return fDefaultRotateY;
    }

    double DefaultRotateY () const
    {
        return fDefaultRotateY;
    }

    double &DefaultRotateZ ()
    {
        return fDefaultRotateZ;
    }

    double DefaultRotateZ () const
    {
        return fDefaultRotateZ;
    }

    double &DefaultExtrusionDeepth ()
    {
        return fDefaultExtrusionDeepth;
    }

    double DefaultExtrusionDeepth () const
    {
        return fDefaultExtrusionDeepth;
    }

//-/	double &DefaultCamFocal ()
//-/	{
//-/		return fDefaultCamFocal;
//-/	}

//-/	double DefaultCamFocal () const
//-/	{
//-/		return fDefaultCamFocal;
//-/	}

    double &DefaultLightIntensity ()
    {
        return fDefaultLightIntensity;
    }

    double DefaultLightIntensity () const
    {
        return fDefaultLightIntensity;
    }

    double &DefaultAmbientIntensity ()
    {
        return fDefaultAmbientIntensity;
    }

    double DefaultAmbientIntensity () const
    {
        return fDefaultAmbientIntensity;
    }

    const Color &DefaultLightColor () const
    {
        return aDefaultLightColor;
    }

    Color DefaultLightColor ()
    {
        return aDefaultLightColor;
    }

    const Color &DefaultAmbientColor () const
    {
        return aDefaultAmbientColor;
    }

    Color DefaultAmbientColor ()
    {
        return aDefaultAmbientColor;
    }

    long GetHDefaultSegments() const { return nHDefaultSegments; }
    void SetHDefaultSegments(long nSegs) { nHDefaultSegments = nSegs; }

    long GetVDefaultSegments() const { return nVDefaultSegments; }
    void SetVDefaultSegments(long nSegs) { nVDefaultSegments = nSegs; }



    BOOL DoubleSided () const
    {
        return bDoubleSided;
    }

    BOOL &DoubleSided ()
    {
        return bDoubleSided;
    }

};

}//end of namespace binfilter
#endif			// _E3D_VIEW3D_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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

#ifndef _SVDHDL_HXX
#define _SVDHDL_HXX

#ifndef _GEN_HXX //autogen
#include <tools/gen.hxx>
#endif

#ifndef _POINTR_HXX //autogen
#include <vcl/pointr.hxx>
#endif

#ifndef _CONTNR_HXX //autogen
#include <tools/contnr.hxx>
#endif

#ifndef _SOLAR_HRC
#include <bf_svtools/solar.hrc>
#endif

//#ifndef _B2D_MPNT_HXX
//#include <bf_goodies/b2dmpnt.hxx>
//#endif

//#ifndef _B2D_MLINE_HXX
//#include <bf_goodies/b2dmline.hxx>
//#endif

//#ifndef _B2D_MBMP_HXX
//#include <bf_goodies/b2dmbmp.hxx>
//#endif

#include <bf_svx/xpoly.hxx>
#include <bf_svx/svdoedge.hxx>
class VirtualDevice;
class OutputDevice;
class Region;
class Window;
namespace binfilter {


class SdrHdlList;
class SdrObject;
class SdrPageView;
class SdrHdlBitmapSet;
class SdrMarkView;

////////////////////////////////////////////////////////////////////////////////////////////////////

// Jedes Objekt muss in der Lage seine Handles zu erzeugen. Diese werden dann
// bei einer Selektion abgeholt, bei der View angemeldet und sichtbar gemacht.
// Wird ein Handle von der Maus beruehrt (IsHit()), so wird von der View der
// entsprechende Mauszeiger vom Handle abgeholt und der App auf Anfrage zum
// reinschalten uebergeben.
// Handles wie z.B. der Rotationsmittelpunkt oder die Spiegelachse werden von
// der View generiert, wenn vom Controller der entsprechende Transformations-
// Modus selektiert wird.
// HDL_MOVE...HDL_LWRGT muessen im enum immer zusammen stehen bleiben!

enum SdrHdlKind 
{
    HDL_MOVE,		// Handle zum Verschieben des Objekts
    HDL_UPLFT,		// Oben links
    HDL_UPPER,		// Oben
    HDL_UPRGT,		// Oben rechts
    HDL_LEFT,		// Links
    HDL_RIGHT,		// Rechts
    HDL_LWLFT,		// Unten links
    HDL_LOWER,		// Unten
    HDL_LWRGT,		// Unten rechts
    HDL_POLY,		// Punktselektion an Polygon oder Bezierkurve
    HDL_BWGT,		// Gewicht an einer Bezierkurve
    HDL_CIRC,		// Winkel an Kreissegmenten, Eckenradius am Rect
    HDL_REF1,		// Referenzpunkt 1, z.B. Rotationsmitte
    HDL_REF2,		// Referenzpunkt 2, z.B. Endpunkt der Spiegelachse
    HDL_MIRX,		// Die Spiegelachse selbst
    HDL_GLUE,		// GluePoint
    HDL_ANCHOR,		// anchor symbol (SD, SW)
    HDL_TRNS,		// interactive transparence
    HDL_GRAD,		// interactive gradient
    HDL_COLR,		// interactive color
    HDL_USER,
    HDL_ANCHOR_TR	// #101688# Anchor handle with (0,0) at top right for SW
};

////////////////////////////////////////////////////////////////////////////////////////////////////

enum BitmapColorIndex
{
    LightGreen,
    Cyan,
    LightCyan,
    Red,
    LightRed
};

////////////////////////////////////////////////////////////////////////////////////////////////////

enum BitmapMarkerKind
{
    Rect_7x7,
    Rect_9x9,
    Rect_11x11,
    Rect_13x13,
    Circ_7x7,
    Circ_9x9,
    Circ_11x11,
    Elli_7x9,
    Elli_9x11,
    Elli_9x7,
    Elli_11x9,
    RectPlus_7x7,
    RectPlus_9x9,
    RectPlus_11x11,
    Crosshair,
    Glue,
    Anchor,

    // #98388# add AnchorPressed to be able to aninate anchor control, too.
    AnchorPressed,

    // #101688# AnchorTR for SW
    AnchorTR,
    AnchorPressedTR
};

////////////////////////////////////////////////////////////////////////////////////////////////////

class SdrHdl
{
    friend class				SdrMarkView; // fuer den Zugriff auf nObjHdlNum
public:
    virtual ~SdrHdl();

    SdrHdlKind GetKind() const { DBG_BF_ASSERT(0, "STRIP"); return HDL_MOVE; }//STRIP001 	SdrHdlKind GetKind() const { return eKind; }
    SdrObject* GetObj() const {DBG_BF_ASSERT(0, "STRIP"); return NULL;  }//STRIP001 	SdrObject* GetObj() const { return pObj;  }
};

////////////////////////////////////////////////////////////////////////////////////////////////////

#define	SDR_HANDLE_COLOR_SIZE_NORMAL			Size(13, 13)
#define	SDR_HANDLE_COLOR_SIZE_SELECTED			Size(17, 17)


////////////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////////////

// Spiegelachse

// Ein SdrHdlBezWgt hat Kenntnis von seinem "BasisHandle". Seine Draw-Methode
// zeichnet zusaetzlich eine Linie von seiner Position zur Position dieses
// BasisHandles.

////////////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

// #97016# II
class ImplHdlListData;

class SdrHdlList
{
protected:
    // #97016# II: use ImplHdlListData here to hold two data items
    //SdrMarkView*				pView;
    ImplHdlListData*			pImpl;

    Container					aList;
    USHORT						nHdlSize;

    unsigned					bRotateShear : 1;
    unsigned					bDistortShear : 1;
    unsigned					bMoveOutside : 1;      // Handles nach aussen ruecken (fuer TextEdit)
    unsigned					bFineHandles : 1;

private:
    SdrHdlList(const SdrHdlList&): aList(1024,64,64)  {}
    void operator=(const SdrHdlList&)                 {}
    BOOL operator==(const SdrHdlList&) const      { return FALSE; }
    BOOL operator!=(const SdrHdlList&) const      { return FALSE; }

public:
    SdrHdlList(SdrMarkView* pV);
    ~SdrHdlList();
    void Clear();

    // #97016# II
    SdrHdl* GetFocusHdl() const;

    // Access to View
    // #97016# II: moved implementation to cxx
    SdrMarkView* GetView() const;

    // Sortierung: 1.Level Erst Refpunkt-Handles, dann normale Handles, dann Glue, dann User, dann Plushandles
    //             2.Level PageView (Pointer)
    //             3.Level Position (x+y)
    void     Sort();
    ULONG    GetHdlCount() const                       { return aList.Count(); }
    SdrHdl*  GetHdl(ULONG nNum) const                  { return (SdrHdl*)(aList.GetObject(nNum)); }
    USHORT   GetHdlSize() const                        { return nHdlSize; }
    BOOL IsMoveOutside() const                     { return bMoveOutside; }
    void     SetRotateShear(BOOL bOn);
    BOOL IsRotateShear() const                     { return bRotateShear; }
    void     SetDistortShear(BOOL bOn);
    BOOL IsDistortShear() const                    { return bDistortShear; }
    void     SetFineHdl(BOOL bOn);
    BOOL IsFineHdl() const                        { return bFineHandles; }
};

}//end of namespace binfilter
#endif //_SVDHDL_HXX


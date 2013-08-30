/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef _SVDXCGV_HXX
#define _SVDXCGV_HXX

#include <svx/svdedxv.hxx>

#include <vcl/gdimtf.hxx>
#include "svx/svxdllapi.h"

class SVX_DLLPUBLIC SdrExchangeView: public SdrObjEditView
{
    friend class SdrPageView;

protected:

    void                ImpGetPasteObjList(Point& rPos, SdrObjList*& rpLst);
    void                ImpPasteObject(SdrObject* pObj, SdrObjList& rLst, const Point& rCenter, const Size& rSiz, const MapMode& rMap, sal_uInt32 nOptions);
    sal_Bool                ImpGetPasteLayer(const SdrObjList* pObjList, SdrLayerID& rLayer) const;

    // liefert True, wenn rPt geaendert wurde
    sal_Bool                ImpLimitToWorkArea(Point& rPt) const;

protected:
    // #i71538# make constructors of SdrView sub-components protected to avoid incomplete incarnations which may get casted to SdrView
    SdrExchangeView(SdrModel* pModel1, OutputDevice* pOut = 0L);

public:
    // Alle markierten Objekte auf dem angegebenen OutputDevice ausgeben.
    ::std::vector< SdrObject* > GetMarkedObjects() const;
    virtual void        DrawMarkedObj(OutputDevice& rOut) const;

    // Z.B. fuer's Clipboard, Drag&Drop, ...
    // Alle markierten Objekte in ein Metafile stecken. Z.Zt. noch etwas
    // buggee (Offset..., Fremdgrafikobjekte (SdrGrafObj), Virtuelle
    // Objektkopien (SdrVirtObj) mit Ankerpos<>(0,0)).
    GDIMetaFile GetMarkedObjMetaFile(bool bNoVDevIfOneMtfMarked = false) const;

    // Alle markierten Objekte auf eine Bitmap malen. Diese hat die Farbtiefe
    // und Aufloesung des Bildschirms.
    BitmapEx GetMarkedObjBitmapEx(bool bNoVDevIfOneBmpMarked = false) const;

    // Alle markierten Objekte in ein neues Model kopieren. Dieses neue Model
    // hat dann genau eine Page. Das Flag PageNotValid an diesem Model ist
    // gesetzt. Daran ist zu erkennen, dass nur die Objekte der Page Gueltikeit
    // haben, die Page sebst jedoch nicht (Seitengroesse, Raender). Das neue
    // Model wird auf dem Heap erzeugt und wird an den Aufrufer dieser Methode
    // uebergeben. Dieser hat es dann spaeter zu entsorgen.
    // Beim einfuegen der markierten Objekte in die eine Page des neuen Model
    // findet ein Merging der seitenlokalen Layer statt. Sollte kein Platz mehr
    // fuer weitere seitenlokale Layer sein, wird den entsprechenden Objekten
    // der Default-Layer zugewiesen (Layer 0, (dokumentglobaler Standardlayer).
    virtual SdrModel*   GetMarkedObjModel() const;

    Graphic         GetAllMarkedGraphic() const;

    /** Generate a Graphic for the given draw object in the given model

        @param pModel
        Must not be NULL. Denotes the draw model the object is a part
        of.

        @param pObj
        The object (can also be a group object) to retrieve a Graphic
        for. Must not be NULL.

        @return a graphical representation of the given object, as it
        appears on screen (e.g. with rotation, if any, applied).
     */
    static Graphic  GetObjGraphic( const SdrModel* pModel, const SdrObject* pObj );

    // Bei allen Paste-Methoden werden die neuen Draw-Objekte markiert.
    // Wird der Parameter bAddMark auf sal_True gesetzt, so werden die neuen
    // DrawObjekte zu einer bereits bestehenden Selektion "hinzumarkiert".
    // Dieser Fall ist fuer Drag&Drop mit mehreren Items gedacht.
    // Die Methoden mit Point-Parameter fuegen neue Objekte zentriert an
    // dieser Position ein, die anderen zentriert am 1.OutputDevice der View.
    // Ist der Parameter pPg gesetzt, werden die Objekte and dieser Seite
    // eingefuegt. Die Positionierung (rPos bzw. Zentrierung) bezieht sich
    // dann nichtmehr auf die View sondern auf die Page.
    // Hinweis: SdrObjList ist Basisklasse von SdrPage.
    // Die Methoden liefern sal_True, wenn die Objekte erfolgreich erzeugt und
    // eingefuegt wurden. Bei pLst=sal_False und kein TextEdit aktiv kann man
    // sich dann auch darauf verlassen, dass diese an der View markiert sind.
    // Andernfalls erfolgt die Markierung nur, wenn pLst z.Zt. auch an der
    // View angezeigt wird.
    // Gueltige Werte fuer nOptions sind SDRINSERT_DONTMARK und
    // SDRINSERT_ADDMARK (siehe svdedtv.hxx).
    virtual sal_Bool    Paste(const SdrModel& rMod, const Point& rPos, SdrObjList* pLst=NULL, sal_uInt32 nOptions=0);
    sal_Bool            Paste(const OUString& rStr, const Point& rPos, SdrObjList* pLst=NULL, sal_uInt32 nOptions=0);
    sal_Bool            Paste(SvStream& rInput, const OUString& rBaseURL, sal_uInt16 eFormat, const Point& rPos, SdrObjList* pLst=NULL, sal_uInt32 nOptions=0);

    sal_Bool            Cut( sal_uIntPtr nFormat = SDR_ANYFORMAT );

    sal_Bool            Yank( sal_uIntPtr nFormat = SDR_ANYFORMAT );

    sal_Bool            Paste( Window* pWin = NULL, sal_uIntPtr nFormat = SDR_ANYFORMAT );
    sal_Bool            PasteClipboard( OutputDevice* pOut = NULL, sal_uIntPtr nFormat = SDR_ANYFORMAT, sal_uInt32 nOptions = 0 );
};

#endif //_SVDXCGV_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

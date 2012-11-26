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



#ifndef _SVDXCGV_HXX
#define _SVDXCGV_HXX

#include <svx/svdedxv.hxx>
#include <vcl/gdimtf.hxx>
#include "svx/svxdllapi.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

/** Generate a Graphic for the given draw object in the given model

    @param pObj
    The object (can also be a group object) to retrieve a Graphic
    for. Must not be NULL.

    @return a graphical representation of the given object, as it
    appears on screen (e.g. with rotation, if any, applied).
    */
// TTTT: Candidate for tooling and replacement ?!?
Graphic SVX_DLLPUBLIC GetObjGraphic(const SdrObject& rObj);

////////////////////////////////////////////////////////////////////////////////////////////////////

class SVX_DLLPUBLIC SdrExchangeView: public SdrObjEditView
{
protected:
    void ImpGetPasteObjList(SdrObjList*& rpLst);
    void ImpPasteObject(SdrObject* pObj, SdrObjList& rLst, const basegfx::B2DPoint& rCenter, const basegfx::B2DVector& rSiz, const MapMode& rMap, sal_uInt32 nOptions);
    bool ImpGetPasteLayer(const SdrObjList* pObjList, SdrLayerID& rLayer) const;

    // liefert True, wenn rPt geaendert wurde
    basegfx::B2DPoint ImpLimitToWorkArea(const basegfx::B2DPoint& rPt) const;

    // #i71538# make constructors of SdrView sub-components protected to avoid incomplete incarnations which may get casted to SdrView
    SdrExchangeView(SdrModel& rModel1, OutputDevice* pOut = 0);

public:
    // Alle markierten Objekte auf dem angegebenen OutputDevice ausgeben.
    virtual void        DrawMarkedObj(OutputDevice& rOut) const;

    // Z.B. fuer's Clipboard, Drag&Drop, ...
    // Alle markierten Objekte in ein Metafile stecken. Z.Zt. noch etwas
    // buggee (Offset..., Fremdgrafikobjekte (SdrGrafObj)
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
    SdrModel*       GetAllMarkedModel() const { return GetMarkedObjModel(); }

    // Bestimmung des View-Mittelpunktes, z.B. zum Pasten
    basegfx::B2DPoint GetViewCenter(const OutputDevice* pOut = 0) const;

    // Bei allen Paste-Methoden werden die neuen Draw-Objekte markiert.
    // Wird der Parameter bAddMark auf true gesetzt, so werden die neuen
    // DrawObjekte zu einer bereits bestehenden Selektion "hinzumarkiert".
    // Dieser Fall ist fuer Drag&Drop mit mehreren Items gedacht.
    // Die Methoden mit Point-Parameter fuegen neue Objekte zentriert an
    // dieser Position ein, die anderen zentriert am 1.OutputDevice der View.
    // Ist der Parameter pPg gesetzt, werden die Objekte and dieser Seite
    // eingefuegt. Die Positionierung (rPos bzw. Zentrierung) bezieht sich
    // dann nichtmehr auf die View sondern auf die Page.
    // Hinweis: SdrObjList ist Basisklasse von SdrPage.
    // Die Methoden liefern true, wenn die Objekte erfolgreich erzeugt und
    // eingefuegt wurden. Bei pLst=false und kein TextEdit aktiv kann man
    // sich dann auch darauf verlassen, dass diese an der View markiert sind.
    // Andernfalls erfolgt die Markierung nur, wenn pLst z.Zt. auch an der
    // View angezeigt wird.
    // Gueltige Werte fuer nOptions sind SDRINSERT_DONTMARK und
    // SDRINSERT_ADDMARK (siehe svdedtv.hxx).

    basegfx::B2DPoint GetPastePos(SdrObjList* pLst, OutputDevice* pOut = 0) const;

    bool Paste(const GDIMetaFile& rMtf, const basegfx::B2DPoint& rPos, SdrObjList* pLst = 0, sal_uInt32 nOptions = 0);
    bool Paste(const Bitmap& rBmp, const basegfx::B2DPoint& rPos, SdrObjList* pLst = 0, sal_uInt32 nOptions = 0);
    virtual bool Paste(const SdrModel& rMod, const basegfx::B2DPoint& rPos, SdrObjList* pLst = 0, sal_uInt32 nOptions = 0);
    bool Paste(const String& rStr, const basegfx::B2DPoint& rPos, SdrObjList* pLst = 0, sal_uInt32 nOptions = 0);

    // der sal_uInt16 eFormat nimmt Werte des enum EETextFormat entgegen
    bool Paste(SvStream& rInput, const String& rBaseURL, sal_uInt16 eFormat, const basegfx::B2DPoint& rPos, SdrObjList* pLst = 0, sal_uInt32 nOptions = 0);

    // Feststellen, ob ein bestimmtes Format ueber Drag&Drop bzw. ueber's
    // Clipboard angenommen werden kann.
    bool IsExchangeFormatSupported(sal_uInt32 nFormat) const;

    bool Cut( sal_uInt32 nFormat = SDR_ANYFORMAT );
    void CutMarked( sal_uInt32 nFormat=SDR_ANYFORMAT );

    bool Yank( sal_uInt32 nFormat = SDR_ANYFORMAT );
    void YankMarked( sal_uInt32 nFormat=SDR_ANYFORMAT );

    bool Paste( Window* pWin = NULL, sal_uInt32 nFormat = SDR_ANYFORMAT );
    bool PasteClipboard( OutputDevice* pOut = NULL, sal_uInt32 nFormat = SDR_ANYFORMAT, sal_uInt32 nOptions = 0 );
};

#endif //_SVDXCGV_HXX

////////////////////////////////////////////////////////////////////////////////////////////////////
// eof

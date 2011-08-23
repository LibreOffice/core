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

#ifndef _SVDMRKV_HXX
#define _SVDMRKV_HXX

#include <bf_svx/svdmark.hxx>

#include <bf_svx/svdhdl.hxx>

#include <bf_svx/svdsnpv.hxx>

#include <bf_svx/svdtypes.hxx>
namespace binfilter {

//************************************************************
//   Defines 
//************************************************************

// folgendes ist noch nicht bzw. erst zum Teil implementiert:
#define SDRSEARCH_DEEP         0x0001 /* rekursiv in Gruppenobjekte hinein */
#define SDRSEARCH_ALSOONMASTER 0x0002 /* MasterPages werden auch durchsucht */
#define SDRSEARCH_WHOLEPAGE    0x0004 /* Nicht nur die ObjList der PageView */
#define SDRSEARCH_TESTMARKABLE 0x0008 /* nur markierbare Objekte/Punkte/Handles/... */
#define SDRSEARCH_TESTMACRO    0x0010 /* Nur Objekte mit Macro */
#define SDRSEARCH_TESTTEXTEDIT 0x0020 /* Nur TextEdit-faehige Objekte */
#define SDRSEARCH_WITHTEXT     0x0040 /* Nur Objekte mit Text */
#define SDRSEARCH_TESTTEXTAREA 0x0080 /* Den Textbereich von Objekten mit Text (TextEditHit) */
#define SDRSEARCH_BACKWARD     0x0100 /* Rueckwaertssuche */
#define SDRSEARCH_NEXT         0x0200 /* Suche startet hinter dem uebergebenen Objekt/Punkt/... */
#define SDRSEARCH_MARKED       0x0400 /* Nur markierte Objekte/Punkte/... */
#define SDRSEARCH_PASS2BOUND   0x0800 /* Wenn nix gefunden, dann neuer 2. Versuch mit BoundRectHit */
#define SDRSEARCH_PASS3NEAREST 0x1000 /* Wenn nix gefunden, dann neuer 3. Versuch mit NearestBoundRectHit */

#define SDRSEARCH_PICKMARKABLE (SDRSEARCH_TESTMARKABLE)
#define SDRSEARCH_PICKTEXTEDIT (SDRSEARCH_DEEP|SDRSEARCH_TESTMARKABLE|SDRSEARCH_TESTTEXTEDIT)
#define SDRSEARCH_PICKMACRO    (SDRSEARCH_DEEP|SDRSEARCH_ALSOONMASTER|SDRSEARCH_WHOLEPAGE|SDRSEARCH_TESTMACRO)

// SDRSEARCHPASS_... ist Rueckgabeparameterwert bei PickObj().
#define SDRSEARCHPASS_DIRECT       0x0000 /* Objekt durch direkten Hit getroffen     */
#define SDRSEARCHPASS_INACTIVELIST 0x0001 /* Obj auf der Page, nicht jedoch in der AktGroup (bei WHOLEPAGE) */
#define SDRSEARCHPASS_MASTERPAGE   0x0002 /* Objekt auf der MasterPage gefunden      */
#define SDRSEARCHPASS_BOUND        0x0010 /* Objekt erst beim Bound-Check gefunden   */
#define SDRSEARCHPASS_NEAREST      0x0020 /* Objekt erst beim Nearest-Check gefunden */

enum SdrHitKind {SDRHIT_NONE,      // Nicht getroffen, Schnaps gesoffen
                 SDRHIT_OBJECT,    // Treffer versenkt
                 SDRHIT_BOUNDRECT, // Treffer am BoundRect
                 SDRHIT_BOUNDTL,   // Treffer am BoundRect TopLeft
                 SDRHIT_BOUNDTC,   // Treffer am BoundRect TopCenter
                 SDRHIT_BOUNDTR,   // Treffer am BoundRect TopRight
                 SDRHIT_BOUNDCL,   // Treffer am BoundRect CenterLeft
                 SDRHIT_BOUNDCR,   // Treffer am BoundRect CenterRight
                 SDRHIT_BOUNDBL,   // Treffer am BoundRect BottomLeft
                 SDRHIT_BOUNDBC,   // Treffer am BoundRect BottomCenter
                 SDRHIT_BOUNDBR,/*,*/ // Treffer am BoundRect BottomRight
                 /*SDRHIT_REFPOINT*/ // Referenzpunkt (Rotationsachse,Spiegelachse) getroffen
                 // ab hier neu am 01-07-1996:
                 SDRHIT_HANDLE,          // Markierungshandle
                 SDRHIT_HELPLINE,        // Hilfslinie
                 SDRHIT_GLUEPOINT,       // Klebepunkt
                 SDRHIT_TEXTEDIT,        // Offene OutlinerView getroffen
                 SDRHIT_TEXTEDITOBJ,     // Objekt fuer BegTextEdit (Textbereich)
                 SDRHIT_URLFIELD,        // Feld im TextObj getroffen (wird gerade nicht editiert)
                 SDRHIT_MACRO,           // Objekt fuer BegMacroObj
                 SDRHIT_MARKEDOBJECT,    // Markiertes Objekt (z.B. zum Draggen)
                 SDRHIT_UNMARKEDOBJECT}; // nichtmarkiertes Objekt (z.B. zum markieren)

enum SdrViewEditMode {SDREDITMODE_EDIT,           // Auch bekannt aus Pfeil- oder Zeigermodus
                      SDREDITMODE_CREATE,         // Objekterzeugungswerkzeug
                      SDREDITMODE_GLUEPOINTEDIT}; // Klebepunkteditiermodus

#define IMPSDR_MARKOBJDESCRIPTION    0x0000
#define IMPSDR_POINTSDESCRIPTION     0x0001
#define IMPSDR_GLUEPOINTSDESCRIPTION 0x0002

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  @@   @@  @@@@  @@@@@  @@  @@  @@ @@ @@ @@@@@ @@   @@
//  @@@ @@@ @@  @@ @@  @@ @@  @@  @@ @@ @@ @@    @@   @@
//  @@@@@@@ @@  @@ @@  @@ @@ @@   @@ @@ @@ @@    @@ @ @@
//  @@@@@@@ @@@@@@ @@@@@  @@@@    @@@@@ @@ @@@@  @@@@@@@
//  @@ @ @@ @@  @@ @@  @@ @@ @@    @@@  @@ @@    @@@@@@@
//  @@   @@ @@  @@ @@  @@ @@  @@   @@@  @@ @@    @@@ @@@
//  @@   @@ @@  @@ @@  @@ @@  @@    @   @@ @@@@@ @@   @@
//
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

class SdrMarkView: public SdrSnapView 
{
    friend class				SdrPageView;

protected:
    SdrObject*					pMarkedObj;       // Wenn nur ein Objekt markiert ist.
    SdrPageView*				pMarkedPV;        // Wenn alle markierten Objekt auf derselben PageView liegen.

    Point						aRef1;            // Persistent - Drehmittelpunkt/Spiegelachse
    Point						aRef2;            // Persistent
    Point						aLastCrookCenter; // Persistent
    SdrHdlList					aHdl;
    SdrMarkList					aMark;            // Persistent (ni)
    SdrMarkList					aEdgesOfMarkedNodes;        // EdgeObjekte der markierten Objekte
    SdrMarkList					aMarkedEdgesOfMarkedNodes;  // Markierte EdgeObjekte der markierten Objekte

    // #i13033#
    // New mechanism to hold the transitive hull of all selected objects
    List						maAllMarkedObjects;

    Rectangle					aMarkedObjRect;
    Rectangle					aMarkedPointsRect;
    Rectangle					aMarkedGluePointsRect;

    USHORT						nFrameHandlesLimit;
    USHORT						nSpecialCnt;
    USHORT						nInsPointNum;      // Nummer des InsPoint
    ULONG						nMarkableObjCount;

    SdrDragMode					eDragMode;        // Persistent
    SdrViewEditMode				eEditMode;      // Persistent
    SdrViewEditMode				eEditMode0;     // Persistent

    unsigned					bHdlShown : 1;
    unsigned					bRefHdlShownOnly : 1; // Spiegelachse waerend Dragging (ni)
    unsigned					bDesignMode : 1;      // DesignMode fuer SdrUnoObj
    unsigned					bMarking : 1;         // Selektionsrahmen fuer Objekte
    unsigned					bMarkingPoints : 1;   // Selektionsrahmen fuer Punkte
    unsigned					bMarkingGluePoints : 1;// Selektionsrahmen fuer Klebepunkte
    unsigned					bUnmarking : 1;
    unsigned					bForceFrameHandles : 1; // Persistent - FrameDrag auch bei Einzelobjekten
    unsigned					bPlusHdlAlways : 1;   // Persistent
    unsigned					bMarkHdlWhenTextEdit : 1; // Persistent, default=FALSE
//    unsigned					bSolidHdlBackgroundInvalid : 1; // Hintergrund der Handles ungueltig?
    unsigned					bInsPolyPoint : 1;     // z.Zt InsPolyPointDragging
    unsigned					bEdgesOfMarkedNodesDirty : 1;
    unsigned					bMarkedObjRectDirty : 1;
    unsigned					bMrkPntDirty : 1;
    unsigned					bMarkedPointsRectsDirty : 1;
    unsigned					bMarkableObjCountDirty : 1;
    unsigned					bHdlHidden : 1;

private:
    void ImpClearVars();
    void UndirtyMrkPnt() const;

protected:
    virtual void SFX_NOTIFY(SfxBroadcaster& rBC, const TypeId& rBCType, const SfxHint& rHint, const TypeId& rHintType);
    virtual void ModelHasChanged(); // Wird von der PaintView gerufen


    virtual void SetMarkHandles();                                           // aHdl (Liste der Handles) fuellen
//    virtual void DrawMarkHdl(OutputDevice* pOut, BOOL bNoRefHdl) const;  // Togglen der Handles
    virtual void SetMarkRects();                                             // Rects an den PageViews
    virtual void CheckMarked();                                              // MarkList nach Del und Lock Layer durchsuchen...

    virtual void AddDragModeHdl(SdrDragMode eMode);

    // add custom handles (used by other apps, e.g. AnchorPos)
    virtual void AddCustomHdl();

    void ForceUndirtyMrkPnt() const                                       { if (bMrkPntDirty) UndirtyMrkPnt(); }

    void ImpShowMarkHdl(OutputDevice* pOut, const Region* pRegion, BOOL bNoRefHdl);
    SdrObject* ImpCheckObjHit(const Point& rPnt, USHORT nTol, SdrObject* pObj, SdrPageView* pPV, ULONG nOptions, const SetOfByte* pMVisLay) const;
    SdrObject* ImpCheckObjHit(const Point& rPnt, USHORT nTol, SdrObjList* pOL, SdrPageView* pPV, ULONG nOptions, const SetOfByte* pMVisLay, SdrObject*& rpRootObj) const;
    BOOL ImpIsObjHit(const Point& rPnt, USHORT nTol, SdrObject* pObj, SdrPageView* pPV, ULONG nOptions) const { return ImpCheckObjHit(rPnt,nTol,pObj,pPV,nOptions,NULL)!=NULL; }
    BOOL ImpIsFrameHandles() const;


    // Macht aus einer Winkelangabe in 1/100deg einen String inkl. Grad-Zeichen
    BOOL MarkGluePoints(const Rectangle* pRect, BOOL bUnmark);

    virtual void WriteRecords(SvStream& rOut) const;
    virtual BOOL ReadRecord(const SdrIOHeader& rViewHead, const SdrNamedSubRecord& rSubHead, SvStream& rIn);

    void SetMoveOutside(BOOL bOn);
    BOOL IsMoveOutside() const;

public:

    SdrMarkView(SdrModel* pModel1, OutputDevice* pOut=NULL);

    virtual void ToggleShownXor(OutputDevice* pOut, const Region* pRegion) const;
    virtual BOOL IsAction() const;
    virtual void BrkAction();

    virtual void ClearPageViews();
    virtual void HidePage(SdrPageView* pPV);

    virtual BOOL IsObjMarkable(SdrObject* pObj, SdrPageView* pPV) const;

    // Liefert TRUE wenn Objekte, Punkte oder Klebepunkte durch Rahmenaufziehen
    // selektiert werden (solange wie der Rahmen aufgezogen wird).
    BOOL IsMarking() const { return bMarking || bMarkingPoints || bMarkingGluePoints; }

    // Liefert TRUE wenn das Rahmenaufziehen nicht Objekte/Punkte/Klebepunkte
    // markieren sondern markierte deselektieren soll.
    BOOL IsUnmarking() const { return bUnmarking; }
    void SetUnmarking(BOOL bOn) { bUnmarking=bOn; }

    // Objekte durch Aufziehen eines Selektionsrahmens markieren
    void BrkMarkObj();
    BOOL IsMarkObj() const { return bMarking; }

    // DragModes: SDRDRAG_CREATE,SDRDRAG_MOVE,SDRDRAG_RESIZE,SDRDRAG_ROTATE,SDRDRAG_MIRROR,SDRDRAG_SHEAR,SDRDRAG_CROOK
    // Move==Resize
    // Das Interface wird hier evtl noch geaendert wg. Ortho-Drag
    SdrDragMode GetDragMode() const { return eDragMode; }
    BOOL ChkDragMode(SdrDragMode eMode) const;
    void SetFrameHandles(BOOL bOn);
    BOOL IsFrameHandles() const { return bForceFrameHandles; }

    // Limit, ab wann implizit auf FrameHandles umgeschaltet wird. default=50.
    void SetFrameHandlesLimit(USHORT nAnz) { nFrameHandlesLimit=nAnz; }
    USHORT GetFrameHandlesLimit() const { return nFrameHandlesLimit; }

    SdrViewEditMode GetEditMode() const { return eEditMode; }

    BOOL IsEditMode() const { return eEditMode==SDREDITMODE_EDIT; }
    BOOL IsCreateMode() const { return eEditMode==SDREDITMODE_CREATE; }
    BOOL IsGluePointEditMode() const { return eEditMode==SDREDITMODE_GLUEPOINTEDIT;; }

    void SetDesignMode(BOOL bOn=TRUE);
    BOOL IsDesignMode() const { return bDesignMode; }

    void SetFrameDragSingles(BOOL bOn=TRUE) { SetFrameHandles(bOn); }
    BOOL IsFrameDragSingles() const { return IsFrameHandles(); }

    // Feststellen, ob und wo ein Objekt bzw. ob ein Referenzpunkt
    // (Rotationsmittelpunkt,Spiegelachse) getroffen wird (SW special).
    BOOL HasMarkedObj() const { return aMark.GetMarkCount()!=0; }
    ULONG GetMarkedObjCount() const { return aMark.GetMarkCount(); }
    virtual void ShowMarkHdl(OutputDevice* pOut, BOOL bNoRefHdl=FALSE);
    virtual void HideMarkHdl(OutputDevice* pOut, BOOL bNoRefHdl=FALSE);
    BOOL IsMarkHdlShown() const { return bHdlShown; }

    // Mit MarkHdlHidden(TRUE) wird das Anzeigen der Handles
    // massiv unterdrueckt, auch bei ShowHarkHdl().
    // Show/HideMarkHdl() wird dann zwar immernoch von der Engine
    // gerufen, setzt auch den Status um (der mit IsMarkHdlShown()
    // abgefragt werden kann), Painted jedoch die Handles nicht.
    void SetMarkHdlHidden(BOOL bOn);
    BOOL IsMarkHdlHidden() const { return bHdlHidden; }

    // Pick: Unterstuetzte Optionen fuer nOptions sind SDRSEARCH_PASS2BOUND und SDRSEARCH_PASS3NEAREST

    // Sucht sich das Oberste der markierten Objekte (O1) und sucht von dort
    // aus in Richtung nach Unten dann das erste nichtmarkierte Objekt (O2).
    // Bei Erfolg wird die Markierung von O1 entfernt, an O2 gesetzt und TRUE
    // returniert. Mit dem Parameter bPrev=TRUE geht die Suche genau in die
    // andere Richtung.

    // Sucht sich das Oberste der markierten Objekte (O1) das von rPnt/nTol
    // getroffen wird und sucht von dort aus in Richtung nach Unten dann das
    // erste nichtmarkierte Objekt (O2). Bei Erfolg wird die Markierung von
    // O1 entfernt, an O2 gesetzt und TRUE returniert.
    // Mit dem Parameter bPrev=TRUE geht die Suche genau in die andere Richtung.

    // Alle Objekte innerhalb eines rechteckigen Bereichs markieren
    // Markiert werden nur Objekte, die vollstaendig eingeschlossen sind.
    void MarkObj(SdrObject* pObj, SdrPageView* pPV, BOOL bUnmark=FALSE, BOOL bImpNoSetMarkHdl=FALSE);
    void UnmarkAllObj(SdrPageView* pPV=NULL); // pPage=NULL => alle angezeigten Seiten

    // Diese Funktion kostet etwas Zeit, da die MarkList durchsucht werden muss.
    void UnMarkAll(SdrPageView* pPV=NULL) { UnmarkAllObj(pPV); }
    const SdrMarkList& GetMarkList() const { return aMark; }
    void SortMarkList() { aMark.ForceSort(); }

    // Die Groesse der Markierungs-Handles wird ueber die jeweilige Aufloesung
    // und die Groesse des Bereichs der markierten Objekte so angepasst, dass
    // sie sich bei einer Frame-Selektion moeglichst nicht ueberschneiden.
    // Dazu muessen die Handles ggf. verkleinert dargestellt werden. Mit der
    // MinMarkHdlSize kann man hierfuer eine Mindestgroesse angeben.
    // Defaultwert ist 3, Mindestwert 3 Pixel.
    BOOL IsSolidMarkHdl() const { return aHdl.IsFineHdl(); }
    void SetSolidMarkHdl(BOOL bOn);

    BOOL HasMarkedPoints() const;

    // Nicht alle Punkte lassen sich markieren:
    BOOL MarkPoint(SdrHdl& rHdl, BOOL bUnmark=FALSE);

    // Sucht sich den ersten markierten Punkt (P1) und sucht von dort
    // aus in den ersten nichtmarkierte Punkt (P2).
    // Bei Erfolg wird die Markierung von P1 entfernt, an P2 gesetzt und TRUE
    // returniert. Mit dem Parameter bPrev=TRUE geht die Suche genau in die
    // andere Richtung.

    // Sucht sich den ersten markierten Punkt (P1) das von rPnt
    // getroffen wird und sucht von dort aus den
    // ersten nichtmarkierten Punkt (P2). Bei Erfolg wird die Markierung von
    // P1 entfernt, an P2 gesetzt und TRUE returniert.
    // Mit dem Parameter bPrev=TRUE geht die Suche genau in die andere Richtung.

    // Die Nummer des passenden Handles raussuchen. Nicht gefunden
    // liefert CONTAINER_ENTRY_NOTFOUND.
    SdrHdl* GetHdl(ULONG nHdlNum)  const { return aHdl.GetHdl(nHdlNum); }
    const SdrHdlList& GetHdlList() const { return aHdl; }

    // Selektionsrahmen fuer Punktmarkierung aufziehen.
    // Wird nur gestartet, wenn HasMarkablePoints() TRUE liefert.
    void BrkMarkPoints();
    BOOL IsMarkPoints() const { return bMarkingPoints; }

    // Zusatzhandles dauerhaft sichtbar schalten
    void SetPlusHandlesAlwaysVisible(BOOL bOn);
    BOOL IsPlusHandlesAlwaysVisible() const { return bPlusHdlAlways; }

    // Handles sichrbar waerend TextEdit (in doppelter Groesse)?
    // Persistent, default=FALSE
    void SetMarkHdlWhenTextEdit(BOOL bOn) { bMarkHdlWhenTextEdit=bOn; }
    BOOL IsMarkHdlWhenTextEdit() const { return bMarkHdlWhenTextEdit; }

    BOOL HasMarkedGluePoints() const;

    // Hdl eines markierten GluePoints holen. Nicht markierte
    // GluePoints haben keine Handles

    // alle Punkte innerhalb dieses Rechtecks markieren (Viewkoordinaten)
    BOOL UnmarkAllGluePoints() { return MarkGluePoints(NULL,TRUE); }

    // Sucht sich den ersten markierten Klebepunkt (P1) und sucht von dort
    // aus in den ersten nichtmarkierte Klebepunkt (P2).
    // Bei Erfolg wird die Markierung von P1 entfernt, an P2 gesetzt und TRUE
    // returniert. Mit dem Parameter bPrev=TRUE geht die Suche genau in die
    // andere Richtung.

    // Sucht sich den ersten markierten Klebepunkt (P1) das von rPnt
    // getroffen wird und sucht von dort aus den
    // ersten nichtmarkierten Klebepunkt (P2). Bei Erfolg wird die Markierung
    // von P1 entfernt, an P2 gesetzt und TRUE returniert.
    // Mit dem Parameter bPrev=TRUE geht die Suche genau in die andere Richtung.

    // Selektionsrahmen fuer Klebepunktmarkierung aufziehen.
    // Wird nur gestartet, wenn HasMarkableGluePoints() TRUE liefert.
    // Der GlueEditMode TRUE wird nicht abgefragt.
    void BrkMarkGluePoints();
    BOOL IsMarkGluePoints() const { return bMarkingGluePoints; }

    // bRestraintPaint=FALSE bewirkt, dass die Handles nicht sofort wieder gemalt werden.
    // AdjustMarkHdl wird eh' nur gerufen, wenn was geaendert wurde; was idR ein Invalidate
    // zur Folge hat. Am Ende von des Redraw werden die Handles automatisch gezeichnet.
    // Der Zweck ist, unnoetiges Flackern zu vermeiden. -> Funkt noch nich, deshalb TRUE!
    void AdjustMarkHdl(BOOL bRestraintPaint=TRUE);

    const Rectangle& GetMarkedObjRect() const; // SnapRects der Objekte, ohne Strichstaerke

    // Wird immer dann gerufen, wenn sich die Liste der markierten Objekte
    // moeglicherweise geaendert hat. Wer ueberlaed muss unbedingt auch die
    // Methode der Basisklasse rufen!
    virtual void MarkListHasChanged();

    // Betreten (Editieren) einer evtl markierten Objektgruppe. Sind mehrere
    // Objektgruppen markiert, so wird die Oberste genommen. Anschliessend
    // liegen alle Memberobjekte der Gruppe im direkten Zugriff. Alle anderen
    // Objekte koennen waerendessen nicht bearbeitet werden (bis zum naechsten
    // LeaveGroup()). Bei einer seitenuebergreifenden Markierung wird jede Page
    // separat abgearbeitet. Die Methode liefert TRUE, wenn wenigstens eine
    // Gruppe betreten wurde.

    // Den Mittelpunkt des letzten Crook-Dragging abholen. Den kann man
    // bei einem anschliessenden Rotate sinnvoll als Drehmittelpunkt setzen.
    const Point& GetLastCrookCenter() const { return aLastCrookCenter; }

    // Wird automatisch von der DragView beim beenden eines Crook-Drag gesetzt.
    void SetLastCrookCenter(const Point& rPt) { aLastCrookCenter=rPt; }

    // Rotationsmittelpunkt bzw. Startpunkt der Spiegelachse
    const Point& GetRef1() const { return aRef1; }

    // Endpunkt der Spiegelachse
    const Point& GetRef2() const { return aRef1; }
    void SetRef2(const Point& rPt);

private:
    // #i13033#
    // Build transitive hull of complete selection in maAllMarkedObjects
    void ImplCollectCompleteSelection(SdrObject* pObj);
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// 
// - Hit-Toleranzen:
//   Die muessen natuerlich immer in logischen Koordinaten angegeben werden. Also
//   immer brav den gewuenschten Pixelwert mit PixelToLogic in Logischen umrechnen.
//   Angenommen ein Wert 100 (logisch)
//   - Bei einer waagerechten Haarlinie (Objekt mit Hoehe 0) waere dann +/-100,
//     also ein vertikaler Bereich von 200 logischen Einheiten sensitiv.
//   - Bei Polygonen wird ein Rect mit der Groesse (200,200) generiert und ein
//     Touch-Test zwischen Poly und diesem Rect durchgefuehrt.
//   - Eine Sonderbehandlung erfahren Objekte, die bei SdrObject::HasEdit()==TRUE
//     liefern (z.B. Textrahmen). Hierbei wird ein weiterer sensitiver Bereich mit
//     einer Breite von 2*Tol (in diesem Beispiel also 200 Einheiten) um das Objekt
//     herumgelegt. Waerend ein Hit direkt ins Objekt die Edit-Methode ruft,
//     ermoeglicht ein Hit in den umliegenden sensitiven Bereich ein Dragging.
// 
////////////////////////////////////////////////////////////////////////////////////////////////////

}//end of namespace binfilter
#endif //_SVDMRKV_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

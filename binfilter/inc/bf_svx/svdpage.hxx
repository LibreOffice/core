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

#ifndef _SVDPAGE_HXX
#define _SVDPAGE_HXX

#include <vcl/bitmap.hxx>

#include <vcl/print.hxx>
#include <vcl/gdimtf.hxx>

#include <cppuhelper/weakref.hxx>

#include <bf_svx/svdtypes.hxx>
#include <bf_svx/svdlayer.hxx>
class Color;
namespace binfilter {

class SfxItemPool;
class SfxStyleSheet;

// ------------------------------
// - intern benutzte Paint-Modi -
// ------------------------------

#if _SOLAR__PRIVATE

#define IMP_PAGEPAINT_NORMAL			0
#define IMP_PAGEPAINT_PREPARE_CACHE		1
#define IMP_PAGEPAINT_PAINT_CACHE		2
#define IMP_PAGEPAINT_PREPARE_BG_CACHE	3
#define IMP_PAGEPAINT_PAINT_BG_CACHE	4

#endif // __PRIVATE

class SdrObject;
class SdrPage;
class ExtOutputDevice;
class SdrPaintInfoRec;
class SdrIOHeader;
class SdrModel;
class SdrPageView;
class SdrLayerAdmin;
class SetOfByte;
class SvxUnoDrawPagesAccess;

struct SdrPaintProcRec {
    SdrObject* pObj;
    ExtOutputDevice& rOut;
    const SdrPaintInfoRec& rInfoRec;
    SdrPaintProcRec(SdrObject* pObj_, ExtOutputDevice& rOut_, const SdrPaintInfoRec& rInfoRec_):
        pObj(pObj_), rOut(rOut_), rInfoRec(rInfoRec_) {}
};

enum SdrInsertReasonKind {SDRREASON_UNKNOWN,    // unbekannt
                          SDRREASON_STREAMING,  // einlesen eines Doks
                          SDRREASON_UNDO,       // kommt aus dem Undo
                          SDRREASON_COPY,       // irgendjemand kopiert...
                          SDRREASON_VIEWCREATE, // vom Anwender interaktiv erzeugt
                          SDRREASON_VIEWCALL};  // Durch SdrView::Group(), ...

class SdrInsertReason {
    const SdrObject* pRefObj;
    SdrInsertReasonKind eReason;
public:
    SdrInsertReason(): pRefObj(NULL),eReason(SDRREASON_UNKNOWN) {}
    SdrInsertReason(SdrInsertReasonKind eR,const SdrObject* pO=NULL): pRefObj(pO),eReason(eR) {}
    void SetReferenceObject(const SdrObject* pO)  { pRefObj=pO; }
    const SdrObject* GetReferenceObject() const   { return pRefObj; }
    void SetReason(SdrInsertReasonKind eR)        { eReason=eR; }
    SdrInsertReasonKind GetReason() const         { return eReason; }
};

class SdrObjList {
protected:
friend class SdrObjListIter;
friend class SdrEditView;
    Container   aList;
    SdrObjList* pUpList;   // Vaterliste
    SdrModel*   pModel;    // Diese Liste gehoert zu diesem Model (Layer,ItemPool,Storage).
    SdrPage*    pPage;     // Page, in die Liste haengt. Kann auch this sein.
    SdrObject*  pOwnerObj; // OwnerObject, falls Liste eines GruppenObjekts.
    Rectangle   aOutRect;
    Rectangle   aSnapRect;
    SdrObjListKind eListKind;
    FASTBOOL    bObjOrdNumsDirty;
    FASTBOOL    bRectsDirty;
protected:
    virtual void RecalcRects();
public:
    TYPEINFO();
    SdrObjList(SdrModel* pNewModel, SdrPage* pNewPage, SdrObjList* pNewUpList=NULL);
    SdrObjList(const SdrObjList& rSrcList);
    virtual ~SdrObjList();
    // !!! Diese Methode nur fuer Leute, die ganz genau wissen was sie tun !!!
    Container& GetContainer()                           { return aList; }
    void SetObjOrdNumsDirty()                           { bObjOrdNumsDirty=TRUE; }
    // pModel, pPage, pUpList und pOwnerObj werden Zuweisungeoperator nicht veraendert!
    void operator=(const SdrObjList& rSrcList);
    void CopyObjects(const SdrObjList& rSrcList);
    // alles Aufraeumen (ohne Undo)
    void    Clear();
    SdrObjListKind GetListKind() const                  { return eListKind; }
    void           SetListKind(SdrObjListKind eNewKind) { eListKind=eNewKind; }
    SdrObjList*    GetUpList() const                    { return pUpList; }
    void           SetUpList(SdrObjList* pNewUpList)    { pUpList=pNewUpList; }
    SdrObject*     GetOwnerObj() const                  { return pOwnerObj; }
    void           SetOwnerObj(SdrObject* pNewOwner)    { pOwnerObj=pNewOwner; }
    virtual SdrPage* GetPage() const;
    virtual void     SetPage(SdrPage* pNewPage);
    virtual SdrModel* GetModel() const;
    virtual void      SetModel(SdrModel* pNewModel);
    // Neuberechnung der Objekt-Ordnungsnummern
    void     RecalcObjOrdNums();
    FASTBOOL IsObjOrdNumsDirty() const        { return bObjOrdNumsDirty; }
    virtual void NbcInsertObject(SdrObject* pObj, ULONG nPos=CONTAINER_APPEND
                                 , const SdrInsertReason* pReason=NULL
                                                                      );
    virtual void InsertObject(SdrObject* pObj, ULONG nPos=CONTAINER_APPEND
                              , const SdrInsertReason* pReason=NULL
                                                                     );
    // aus Liste entfernen ohne delete
    //virtual void RemoveObject(SdrObject* pObj);
    virtual SdrObject* NbcRemoveObject(ULONG nObjNum);
    virtual SdrObject* RemoveObject(ULONG nObjNum);
//    virtual SdrObject* RemoveObjectNum(ULONG nObjNum);
    // Vorhandenes Objekt durch ein anderes ersetzen.
    // Wie Remove&Insert jedoch performanter, da die Ordnungsnummern
    // nicht Dirty gesetzt werden muessen.
    virtual SdrObject* ReplaceObject(SdrObject* pNewObj, ULONG nObjNum);
    // Die Z-Order eines Objekts veraendern
    virtual SdrObject* NbcSetObjectOrdNum(ULONG nOldObjNum, ULONG nNewObjNum);
    virtual SdrObject* SetObjectOrdNum(ULONG nOldObjNum, ULONG nNewObjNum);

    virtual void SetRectsDirty();

    const Rectangle& GetAllObjSnapRect() const;
    const Rectangle& GetAllObjBoundRect() const;

    // Painten aller Objekte der Liste unter Berueksichtigung der sichtbaren
    // Layer (rInfoRec.aPaintLayer), etc.
    // Liefert TRUE wenn Paint korrekt beendet wurde. Wenn rInfoRec.nBrkEvent=0
    // ist sollte die Methode immer TRUE liefern.
    // -> Unterbrechbarer Redraw ist eh' noch nicht implementiert.
    FASTBOOL Paint(ExtOutputDevice& rXOut, const SdrPaintInfoRec& rInfoRec, FASTBOOL bRestoreColors=FALSE) const;
    FASTBOOL Paint(ExtOutputDevice& rXOut, const SdrPaintInfoRec& rInfoRec, FASTBOOL bRestoreColors, USHORT nImpMode) const;
    // HitTest auf alle Objekte der Liste

    // Alle Textobjekte neu formatieren, z.B. bei Druckerwechsel
    void ReformatAllTextObjects();

    /** #103122# reformats all edge objects that are connected to other objects */
    void ReformatAllEdgeObjects();

    // Die Vorlagenattribute der Zeichenobjekte in harte Attribute verwandeln.

    // Nichtpersistente Objekte aus der Liste entfernen. Dies sind einerseits
    // Objekte die bei SdrObject::IsNotPersistent() ein FALSE liefern. Wurde
    // zuvor SdrModel::SetStreamingSdrModel(TRUE) gerufen, so werden auch
    // OLE-Objekte entfernt.

    // Animations-Iterationscounter Resetten. Neustart der Animation
    // beim naechsten Paint. pPageView=NULL: Alle PageViews.
    void RestartAllAnimations(SdrPageView* pPageView) const;

    // Bestimmung der FuellFarbe an einer bestimmten Position.
    // FALSE=Kein Objekt mit FuellFarbe an dieser Position gefunden.
    // rVisLayers gibt die zu durchsuchenden Layer an.
    // bLayerSorted: TRUE=Es wird in der Reihenfolge der Layer gesucht (ni)
    // rCol: Hier wird die gefundene Farbe zurueckgegeben

    ULONG      GetObjCount() const           { return aList.Count(); }
    SdrObject* GetObj(ULONG nNum) const      { return (SdrObject*)aList.GetObject(nNum); }

    // Gelinkte Seite oder gelinktes Gruppenobjekt

    // Zaehlt alle Objekte inkl. Objekte in Objektgruppen, ...
    ULONG   CountAllObjects() const;

    // Alle aufgelagerten Teile (z.B. Grafiken) der Liste in den
    // Speicher laden.


    void    Save(SvStream& rOut) const;
    void    Load(SvStream& rIn, SdrPage& rPage);
    // AfterRead wird gerufen, nachdem das gesamte Model eingestreamt
    // worden ist. Damit ist ein PostProcessing moeglich um beispielsweise
    // Objektverknuepfungen ueber Surrogate herzustellen o.ae.
    // Anwendungsbeispiel hierfuer SdrEdgeObj (Objektverbinder)
    virtual void AfterRead();
    /** Makes the object list flat, i.e. the object list content are
        then tree leaves

        This method travels recursively over all group objects in this
        list, extracts the content, inserts it flat to the list and
        removes the group object afterwards.
     */
    virtual void FlattenGroups();
    /** Ungroup the object at the given index

        This method ungroups the content of the group object at the
        given index, i.e. the content is put flat into the object list
        (if the object at the given index is no group, this method is
        a no-op). If the group itself contains group objects, the
        operation is performed recursively, such that the content of
        the given object contains no groups afterwards.
     */
    virtual void UnGroupObj( ULONG nObjNum );
};

/*
Eine Sdraw-Seite enthaelt genau eine Objektliste sowie eine Beschreibung
der physikalischen Seitendimensionen (Groesse/Raender). Letzteres wird
lediglich zum Fangen von Objekten beim Draggen benoetigt.
An der Seite lassen sich (ueber SdrObjList) Objekte einfuegen und loeschen,
nach vorn und nach hinten stellen. Ausserdem kann die Ordnungszahl eines
Objektes abgefragt sowie direkt gesetzt werden.
*/

// fuer SdrPage::GetMasterPagePos()
#define SDRPAGE_NOTFOUND 0xFFFF

class SdrMasterPageDescriptor {
    USHORT    nPgNum;
    SetOfByte aVisLayers;
public:
    SdrMasterPageDescriptor(USHORT nPageNum=0): nPgNum(nPageNum) { aVisLayers.SetAll(); }
    FASTBOOL         operator==(const SdrMasterPageDescriptor& rCmp) const { return nPgNum==rCmp.nPgNum && aVisLayers==rCmp.aVisLayers; }
    FASTBOOL         operator!=(const SdrMasterPageDescriptor& rCmp) const { return !operator==(rCmp); }
    USHORT           GetPageNum() const                          { return nPgNum; }
    void             SetPageNum(USHORT nNum)                     { nPgNum=nNum; }
    const SetOfByte& GetVisibleLayers() const                    { return aVisLayers; }
    void             SetVisibleLayers(const SetOfByte& rVisLay)  { aVisLayers=rVisLay; }
//    SetOfByte&       VisibleLayers()                             { return aVisLayers; }
    friend SvStream& operator<<(SvStream& rOut, const SdrMasterPageDescriptor& rMDP);
    friend SvStream& operator>>(SvStream& rIn, SdrMasterPageDescriptor& rMDP);
};

class SdrMasterPageDescriptorList {
    Container aList;
protected:
    SdrMasterPageDescriptor* GetObject(USHORT i) const { return (SdrMasterPageDescriptor*)(aList.GetObject(i)); }
public:
    SdrMasterPageDescriptorList(): aList(1024,4,4) {}
    SdrMasterPageDescriptorList(const SdrMasterPageDescriptorList& rSrcList): aList(1024,4,4) { *this=rSrcList; }
    ~SdrMasterPageDescriptorList()                                         { Clear(); }
    void     Clear();
    FASTBOOL operator==(const SdrMasterPageDescriptorList& rCmpList) const;
    FASTBOOL operator!=(const SdrMasterPageDescriptorList& rCmpList) const   { return !operator==(rCmpList); }
    USHORT   GetCount() const                                                { return USHORT(aList.Count()); }
    void     Insert(USHORT nPgNum, USHORT nPos=0xFFFF)                       { aList.Insert(new SdrMasterPageDescriptor(nPgNum),nPos); }
    void     Insert(const SdrMasterPageDescriptor& rMPD, USHORT nPos=0xFFFF) { aList.Insert(new SdrMasterPageDescriptor(rMPD),nPos); }
    void     Remove(USHORT nPos)                                             { delete (SdrMasterPageDescriptor*)aList.Remove(nPos); }
    void     Move(USHORT nPos, USHORT nNewPos)                               { aList.Insert(aList.Remove(nPos),nNewPos); }
    SdrMasterPageDescriptor&       operator[](USHORT nPos)                   { return *GetObject(nPos); }
    const SdrMasterPageDescriptor& operator[](USHORT nPos) const             { return *GetObject(nPos); }
    friend SvStream& operator<<(SvStream& rOut, const SdrMasterPageDescriptorList& rMPDL);
    friend SvStream& operator>>(SvStream& rIn, SdrMasterPageDescriptorList& rMPDL);
};

// Fuer das Fangraster/Punkgitter im Writer
class SdrPageGridFrame {
    Rectangle aPaper;
    Rectangle aUserArea;
public:
    SdrPageGridFrame(const Rectangle& rPaper): aPaper(rPaper), aUserArea(rPaper) {}
    SdrPageGridFrame(const Rectangle& rPaper, const Rectangle& rUser): aPaper(rPaper), aUserArea(rUser) {}
    void             SetPaperRect(const Rectangle& rPaper) { aPaper=rPaper; }
    void             SetUserArea(const Rectangle& rUser)   { aUserArea=rUser; }
    const Rectangle& GetPaperRect() const                  { return aPaper; }
    const Rectangle& GetUserArea() const                   { return aUserArea; }
};

class SdrPageGridFrameList {
    Container aList;
private:
    SdrPageGridFrameList(const SdrPageGridFrameList& rSrcList): aList(1024,4,4) {}
    void           operator=(const SdrPageGridFrameList& rSrcList)              {}
protected:
    SdrPageGridFrame* GetObject(USHORT i) const { return (SdrPageGridFrame*)(aList.GetObject(i)); }
public:
    SdrPageGridFrameList(): aList(1024,4,4)                            {}
    ~SdrPageGridFrameList()                                            { Clear(); }
    void           Clear();
    USHORT         GetCount() const                                    { return USHORT(aList.Count()); }
    void           Insert(const SdrPageGridFrame& rGF, USHORT nPos=0xFFFF) { aList.Insert(new SdrPageGridFrame(rGF),nPos); }
    void           Delete(USHORT nPos)                                 { delete (SdrPageGridFrame*)aList.Remove(nPos); }
    void           Move(USHORT nPos, USHORT nNewPos)                   { aList.Insert(aList.Remove(nPos),nNewPos); }
    SdrPageGridFrame&       operator[](USHORT nPos)                    { return *GetObject(nPos); }
    const SdrPageGridFrame& operator[](USHORT nPos) const              { return *GetObject(nPos); }
};

////////////////////////////////////////////////////////////////////////////////////////////////////

class SdrPage: public SdrObjList
{
    // #i9076#
    friend class SdrModel;
    friend class SvxUnoDrawPagesAccess;

// this class uses its own UNO wrapper
// and thus has to set mxUnoPage
friend class ChXChartDocument;

    INT32 nWdt;     // Seitengroesse
    INT32 nHgt;     // Seitengroesse
    INT32 nBordLft; // Seitenrand links
    INT32 nBordUpp; // Seitenrand oben
    INT32 nBordRgt; // Seitenrand rechts
    INT32 nBordLwr; // Seitenrand unten

    // this is a weak reference to a possible living api wrapper for this page
    ::com::sun::star::uno::WeakReference< ::com::sun::star::uno::XInterface > mxUnoPage;

    // #108867# used by GetFillColor
    FASTBOOL ImplGetFillColor(const Point& rPnt, const SetOfByte& rVisLayers,
                              FASTBOOL bLayerSorted, Color& rCol, FASTBOOL bSkipBackgroundShape) const;

protected:
    //SdrModel& rModel;
    SdrLayerAdmin* 	pLayerAdmin;
    SdrObject*		pBackgroundObj;

    //UShortCont aMasters;
    SdrMasterPageDescriptorList aMasters; // Das sind meine MasterPages (Seitennummern)
    SetOfByte  aPrefVisiLayers;
    USHORT     nPageNum;
    FASTBOOL   bMaster;  // TRUE: Ich bin eine Stammseite
    FASTBOOL   bInserted;
    FASTBOOL   bObjectsNotPersistent;
    FASTBOOL   bSwappingLocked;

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > createUnoPage();

#if _SOLAR__PRIVATE
protected:
    void ImpMasterPageRemoved(USHORT nMasterPageNum);
    void ImpMasterPageMoved(USHORT nMasterPageNum, USHORT nNewMasterPageNum);
#endif // __PRIVATE
public:
    TYPEINFO();
    SdrPage(SdrModel& rNewModel, FASTBOOL bMasterPage=FALSE);
    // Copy-Ctor und Zuweisungeoperator sind nicht getestet!
    SdrPage(const SdrPage& rSrcPage);
    virtual ~SdrPage();
    // pModel, pPage, pUpList, pOwnerObj und bInserted werden Zuweisungeoperator nicht veraendert!
            FASTBOOL IsMasterPage() const       { return bMaster; }
            void SetInserted(FASTBOOL bJa=TRUE);
            FASTBOOL IsInserted() const         { return bInserted; }
    virtual void SetChanged();
            void   SetPageNum(USHORT nNum) { nPageNum=nNum; }; // wird vom Model gesetzt!
            USHORT GetPageNum() const;
    //virtual void InsertObject(SdrObject* pObj, ULONG nPos=CONTAINER_APPEND);
    //virtual void RemoveObject(SdrObject* pObj);
    //virtual SdrObject* RemoveObjectNum(ULONG nObjNum);
    virtual void SetSize(const Size& aSiz);
    virtual Size GetSize() const;
    virtual void SetOrientation(Orientation eOri);
    virtual INT32 GetWdt() const;
    virtual INT32 GetHgt() const;
    virtual void  SetBorder(INT32 nLft, INT32 nUpp, INT32 nRgt, INT32 Lwr);
    virtual void  SetLftBorder(INT32 nBorder);
    virtual void  SetUppBorder(INT32 nBorder);
    virtual void  SetRgtBorder(INT32 nBorder);
    virtual void  SetLwrBorder(INT32 nBorder);
    virtual INT32 GetLftBorder() const;
    virtual INT32 GetUppBorder() const;
    virtual INT32 GetRgtBorder() const;
    virtual INT32 GetLwrBorder() const;
    void SendRepaintBroadcast() const;

    // Wenn innerhalb einer Applikation der Seitenursprung nicht bei 0,0 liegt
    // muss diese Methode ueberladen werden und einen entsprechenden Offset
    // liefern. Der Offset gibt die Position der linken oberen Ecke des Blatt
    // Papiers im logischen Koordinatensystem an.
    // -> Massgeblich fuer Raster und Fang. (->Writer)

    // #i3694#
    // This GetOffset() method is not needed anymore, it even leads to errors.
    // virtual Point GetOffset() const;

    virtual void SetModel(SdrModel* pNewModel);

    // Eine einmal eingefuegte MasterPage wird anschliessend nur noch anhand
    // ihrer Posistion in der MasterPageliste der Seite identifiziert (ausser
    // GetMasterPagePos()). Masterpages die in am Anfang der Liste stehen,
    // werden auch zuerst gepainted.
    virtual void             InsertMasterPage(USHORT nPgNum, USHORT nPos=0xFFFF);
    virtual void             RemoveMasterPage(USHORT nPos);
    virtual void             MoveMasterPage(USHORT nPos, USHORT nNewPos);

    virtual void             InsertMasterPage(const SdrMasterPageDescriptor& rMPD, USHORT nPos=0xFFFF);

            SdrPage*         GetMasterPage(USHORT nPos) const;
            USHORT           GetMasterPageNum(USHORT nPos) const              { return aMasters[nPos].GetPageNum(); }
            const SetOfByte& GetMasterPageVisibleLayers(USHORT nPos) const    { return aMasters[nPos].GetVisibleLayers(); }
            USHORT           GetMasterPageCount() const                       { return aMasters.GetCount(); }
    SdrMasterPageDescriptor& GetMasterPageDescriptor(USHORT nPos)             { return aMasters[nPos]; }
    const SdrMasterPageDescriptor& GetMasterPageDescriptor(USHORT nPos) const { return aMasters[nPos]; }
    const SdrMasterPageDescriptorList& GetMasterPageDescriptorList() const    { return aMasters; }
    virtual void             SetMasterPageVisibleLayers(const SetOfByte& rVL, USHORT nPos);

    // Aenderungen an den Layern setzen nicht das Modified-Flag !
    const         SdrLayerAdmin& GetLayerAdmin() const                  { return *pLayerAdmin; }
                  SdrLayerAdmin& GetLayerAdmin()                        { return *pLayerAdmin; }


    virtual String GetLayoutName() const;

    // fuer's Raster im Writer, auch fuer AlignObjects wenn 1 Objekt markiert ist
    // wenn pRect!=NULL, dann die Seiten, die von diesem Rect intersected werden
    // ansonsten die sichtbaren Seiten.
    FASTBOOL IsObjectsNotPersistent() const          { return bObjectsNotPersistent; }
    void     SetObjectsNotPersistent(FASTBOOL b)     { bObjectsNotPersistent=b; }
    // Durch Setzen dieses Flags, kann das Auslagern (Swappen) von
    // Teilen der Page (z.B. Grafiken) unterbunden werden.
    // Es werden hierdurch jedoch nicht automatisch alle ausgelagerten
    // Teile nachgeladen, dies geschieht erst bei konkretem Bedarf oder
    // durch Aufruf von SwapInAll().
    // Fuer die MasterPage(s) der Page muss dies ggf. separat gemacht werden.
    FASTBOOL IsSwappingLocked() const                { return bSwappingLocked; }
    void     SetSwappingLocked(FASTBOOL bLock)       { bSwappingLocked=bLock; }

    SdrObject* GetBackgroundObj() const { return pBackgroundObj; }
    void 	   SetBackgroundObj( SdrObject* pObj );

    // Abgeleitete Klassen ueberlagern ggf. WriteData() und ReadData(). Diese
    // Methoden werden dann von den Stream-Operatoren gerufen.
    // Ausserdem muss man sich in diesem Fall von SdrModel ableiten und die
    // vMethode AllocPage() ueberladen, damit das Model beim reinstreamen die
    // richtige Page erzeugen kann.
    virtual void  WriteData(SvStream& rOut) const;
    virtual void  ReadData(const SdrIOHeader& rHead, SvStream& rIn);
    friend SvStream& operator<<(SvStream& rOut, const SdrPage& rPg);
    friend SvStream& operator>>(SvStream& rIn, SdrPage& rPg);

    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > getUnoPage();

    virtual SfxStyleSheet* GetTextStyleSheetForObject( SdrObject* pObj ) const;


    /** *deprecated* returns an averaged background color of this page */
    Color GetBackgroundColor() const;

    /** *deprecated* returns an averaged background color of this page */
    Color GetBackgroundColor( SdrPageView* pView ) const;
};

}//end of namespace binfilter
#endif //_SVDPAGE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

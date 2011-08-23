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

#ifndef _SVDMODEL_HXX
#define _SVDMODEL_HXX

#include <cppuhelper/weakref.hxx>

#include <bf_so3/svstor.hxx>

#include <tools/link.hxx>
#include <tools/contnr.hxx>
#include <vcl/mapmod.hxx>
#include <bf_svtools/brdcst.hxx>
#include <tools/string.hxx>
#include <tools/datetime.hxx>
#include <bf_svtools/hint.hxx>

#include <bf_svx/pageitem.hxx>

#include <vcl/field.hxx>

class OutputDevice; 

#include <bf_svx/svdtypes.hxx> // fuer enum RepeatFuncts

#include <vcl/field.hxx>

#include <rtl/ref.hxx>
class AutoTimer;
class SotStorage;
namespace binfilter {

class SvNumberFormatter;

class SvStorageRef;
class SvPersist;
class SfxItemPool;
class SfxItemSet;
class SfxRepeatTarget;
class SfxStyleSheet;
class SfxStyleSheetBasePool;
class SfxUndoAction;

#if defined(UNX) || defined(WIN) || defined(WNT)
#define DEGREE_CHAR ((sal_Unicode)176)   /* 0xB0 = Ansi */
#endif

#if defined(DOS) || defined(OS2)
#define DEGREE_CHAR ((sal_Unicode)248)   /* 0xF8 = IBM PC (Erw. ASCII) */
#endif

#ifndef DEGREE_CHAR
#error unbekannte Plattrorm
#endif

class SdrOutliner;
class SdrIOHeader;
class SdrLayerAdmin;
class SdrObjList;
class SdrObject;
class SdrPage;
class SdrPageView;
class SdrTextObj;
class SdrUndoAction;
class SdrUndoGroup;
class SvxLinkManager;
class XBitmapList;
class XBitmapTable;
class XColorTable;
class XDashList;
class XDashTable;
class XGradientList;
class XGradientTable;
class XHatchList;
class XHatchTable;
class XLineEndList;
class XLineEndTable;
class SvxForbiddenCharactersTable;
class SdrOutlinerCache;

////////////////////////////////////////////////////////////////////////////////////////////////////

#define SDR_SWAPGRAPHICSMODE_NONE		0x00000000
#define SDR_SWAPGRAPHICSMODE_TEMP		0x00000001
#define SDR_SWAPGRAPHICSMODE_DOC		0x00000002
#define SDR_SWAPGRAPHICSMODE_PURGE		0x00000100
#define SDR_SWAPGRAPHICSMODE_DEFAULT	(SDR_SWAPGRAPHICSMODE_TEMP|SDR_SWAPGRAPHICSMODE_DOC|SDR_SWAPGRAPHICSMODE_PURGE)

////////////////////////////////////////////////////////////////////////////////////////////////////

enum SdrHintKind 
{
                  HINT_UNKNOWN,         // Unbekannt
                  HINT_LAYERCHG,        // Layerdefinition geaendert
                  HINT_LAYERORDERCHG,   // Layerreihenfolge geaendert (Insert/Remove/ChangePos)
                  HINT_LAYERSETCHG,     // Layerset geaendert
                  HINT_LAYERSETORDERCHG,// Layersetreihenfolge geaendert (Insert/Remove/ChangePos)
                  HINT_PAGECHG,         // Page geaendert
                  HINT_PAGEORDERCHG,    // Reihenfolge der Seiten (Zeichenseiten oder Masterpages) geaendert (Insert/Remove/ChangePos)
                  HINT_OBJCHG,          // Objekt geaendert
                  HINT_OBJINSERTED,     // Neues Zeichenobjekt eingefuegt
                  HINT_OBJREMOVED,      // Zeichenobjekt aus Liste entfernt
                  HINT_MODELCLEARED,    // gesamtes Model geloescht (keine Pages mehr da). not impl.
                  HINT_OBJLISTCLEARED,  // ObjList geloescht (ohne Undo)
                  HINT_REFDEVICECHG,    // RefDevice geaendert
                  HINT_DEFAULTTABCHG,   // Default Tabulatorweite geaendert
                  HINT_DEFFONTHGTCHG,   // Default FontHeight geaendert
                  HINT_MODELSAVED,      // Dokument wurde gesichert
                  HINT_CONTROLINSERTED, // UnoControl wurde eingefuegt
                  HINT_CONTROLREMOVED,  // UnoControl wurde entfernt
                  HINT_SWITCHTOPAGE,    // #94278# UNDO/REDO at an object evtl. on another page
                  HINT_OBJLISTCLEAR,	// Is called before an SdrObjList will be cleared
                  HINT_BEGEDIT,			// Is called after the object has entered text edit mode
                  HINT_ENDEDIT			// Is called after the object has left text edit mode
};

class SdrHint: public SfxHint {
public:
    Rectangle         aRect;
    const SdrPage*    pPage;
    const SdrObject*  pObj;
    const SdrObjList* pObjList;
    FASTBOOL          bNeedRepaint;
    SdrHintKind       eHint;
    FASTBOOL          bAniRepaint;
public:
    TYPEINFO();
    SdrHint(): pPage(NULL),pObj(NULL),pObjList(NULL),bNeedRepaint(FALSE),eHint(HINT_UNKNOWN),bAniRepaint(FALSE) {}
    SdrHint(SdrHintKind eNewHint): pPage(NULL),pObj(NULL),pObjList(NULL),bNeedRepaint(FALSE),eHint(eNewHint),bAniRepaint(FALSE) {}
    SdrHint(const SdrPage& rNewPage);
    SdrHint(const SdrObject& rNewObj);
    SdrHint(const SdrObject& rNewObj, const Rectangle& rRect);
    void              SetPage(const SdrPage* pNewPage)     { pPage=pNewPage; }
    void              SetObjList(const SdrObjList* pNewOL) { pObjList=pNewOL; }
    void              SetObject(const SdrObject* pNewObj)  { pObj=pNewObj; }
    void              SetKind(SdrHintKind eNewKind)        { eHint=eNewKind; }
    void              SetRect(const Rectangle& rNewRect)   { aRect=rNewRect; }
    void              SetNeedRepaint(FASTBOOL bOn)         { bNeedRepaint=bOn; }
    void              SetAniRepaint(FASTBOOL bOn)          { bAniRepaint=bOn; }
    const SdrPage*    GetPage() const                      { return pPage; }
    const SdrObjList* GetObjList() const                   { return pObjList; }
    const SdrObject*  GetObject() const                    { return pObj;  }
    SdrHintKind       GetKind() const                      { return eHint; }
    const Rectangle&  GetRect() const                      { return aRect; }
    FASTBOOL          IsNeedRepaint() const                { return bNeedRepaint; }
    FASTBOOL          IsAniRepaint() const                 { return bAniRepaint; }
};

////////////////////////////////////////////////////////////////////////////////////////////////////

// Flag um nach dem Laden des Pools Aufzuraeumen (d.h. die RefCounts
// neu zu bestimmen und unbenutztes wegzuwerfen). FALSE == aktiv
#define LOADREFCOUNTS (FALSE)

class SdrModelInfo {
public:
    DateTime   aCreationDate;
    DateTime   aLastWriteDate;
    DateTime   aLastReadDate;
    DateTime   aLastPrintDate;
    rtl_TextEncoding eCreationCharSet;
    rtl_TextEncoding eLastWriteCharSet;
    rtl_TextEncoding eLastReadCharSet;
    UINT16     nCompressMode;
    UINT16     nNumberFormat;

    /* old SV-stuff, there is no possibility to determine
       this informations in another way
    GUIType    eCreationGUI;
    CPUType    eCreationCPU;
    SystemType eCreationSys;
    GUIType    eLastWriteGUI;
    CPUType    eLastWriteCPU;
    SystemType eLastWriteSys;
    GUIType    eLastReadGUI;
    CPUType    eLastReadCPU;
    SystemType eLastReadSys;
    */

public:
    // bei bInit=TRUE werden die Member mit den aktuellen
    // Informationen gesetzt, sonst gibts nur Defaults.
    SdrModelInfo(FASTBOOL bInit=FALSE);
    friend SvStream& operator<<(SvStream& rOut, const SdrModelInfo& rModInfo);
    friend SvStream& operator>>(SvStream& rIn, SdrModelInfo& rModInfo);
};

struct SdrDocumentStreamInfo
{
    FASTBOOL	    mbDeleteAfterUse;
    String		    maUserData;
    SvStorageRef*   mpStorageRef;
    BOOL		    mbDummy1 : 1;
};

class SdrModel: public SfxBroadcaster {
protected:
    SdrModelInfo   aInfo;
    DateTime       aReadDate;  // Datum des Einstreamens
    Container      aMaPag;     // StammSeiten (Masterpages)
    Container      aPages;
    Link           aUndoLink;  // Link fuer einen NotifyUndo-Handler
    Link           aIOProgressLink;
    String         aTablePath;
    String         aLoadedModelFN; // fuer Referenzen auf ein anderes Dok
    Size           aMaxObjSize; // z.B. fuer Autogrowing Text
    Fraction       aObjUnit;   // ?Beschreibung der Koordinateneinheiten
    MapUnit        eObjUnit;   // ?fuer ClipBoard, Drag&Drop, ...
    FieldUnit      eUIUnit;      // ?Masseinheit
    Fraction       aUIScale;     // ?Masstab (z.B. 1/1000)
    String         aUIUnitStr;   // ?fuer die UI        wird von ImpSetUIUnit() gesetzt
    Fraction       aUIUnitFact;  // ?(Statuszeile)      wird von ImpSetUIUnit() gesetzt
    int            nUIUnitKomma; // ?                   wird von ImpSetUIUnit() gesetzt
    FASTBOOL       bUIOnlyKomma; // ?                   wird von ImpSetUIUnit() gesetzt

    SdrLayerAdmin*  pLayerAdmin;
    SfxItemPool*    pItemPool;
    FASTBOOL        bMyPool;        // zum Aufraeumen von pMyPool ab 303a
    SvPersist*      pPersist;
    SdrOutliner*    pDrawOutliner;  // ein Outliner zur Textausgabe
    SdrOutliner*    pHitTestOutliner;// ein Outliner fuer den HitTest
    ULONG           nDefTextHgt;    // Default Texthoehe in logischen Einheiten
    OutputDevice*   pRefOutDev;     // ReferenzDevice fuer die EditEngine
    ULONG           nProgressAkt;   // fuer den
    ULONG           nProgressMax;   // ProgressBar-
    ULONG           nProgressOfs;   // -Handler
    SfxStyleSheetBasePool* pStyleSheetPool;
    SfxStyleSheet*  pDefaultStyleSheet;
    SvxLinkManager* pLinkManager;   // LinkManager
    SdrModel*       pLoadedModel;   // fuer Referenzen auf ein anderes Dok
    Container*      pUndoStack;
    Container*      pRedoStack;
    SdrUndoGroup*   pAktUndoGroup;  // Fuer mehrstufige
    USHORT          nUndoLevel;     // Undo-Klammerung
    USHORT          nProgressPercent; // fuer den ProgressBar-Handler
    USHORT          nLoadVersion;   // Versionsnummer der geladenen Datei
    FASTBOOL        bExtColorTable; // Keinen eigenen ColorTable
    FASTBOOL        bChanged;
    FASTBOOL        bInfoChanged;
    FASTBOOL        bPagNumsDirty;
    FASTBOOL        bMPgNumsDirty;
    FASTBOOL        bPageNotValid;  // TRUE=Doc ist nur ObjektTraeger. Page ist nicht gueltig.
    FASTBOOL        bSavePortable;  // Metafiles portabel speichern
    FASTBOOL        bNoBitmapCaching;   // Bitmaps fuer Screenoutput cachen
    FASTBOOL        bLoading;           // Model wird gerade eingestreamt
    FASTBOOL        bStreamingSdrModel; // Gestreamt wird ein SdrModel, ohne applikationsspeziefische Daten
    FASTBOOL        bReadOnly;
    FASTBOOL        bTransparentTextFrames;
    FASTBOOL        bSaveCompressed;
    FASTBOOL        bSwapGraphics;
    FASTBOOL        bPasteResize; // Objekte werden gerade resized wegen Paste mit anderem MapMode
    FASTBOOL		bSaveOLEPreview;      // save preview metafile of OLE objects
    UINT16          nStreamCompressMode;  // Komprimiert schreiben?
    UINT16          nStreamNumberFormat;
    UINT16          nDefaultTabulator;
    SdrPageView*    pAktPaintPV;
    UINT32          nMaxUndoCount;
    FASTBOOL        bSaveNative;
    BOOL            bStarDrawPreviewMode;
    UINT16          nStarDrawPreviewMasterPageNum;
    // Reserven fuer kompatible Erweiterungen
//-/	SfxItemPool*    pUndoItemPool;
    SotStorage*		pModelStorage;
    SvxForbiddenCharactersTable* mpForbiddenCharactersTable;
    ULONG			nSwapGraphicsMode;

    SdrOutlinerCache* mpOutlinerCache;
    UINT32          nReserveUInt3;
    UINT16          mnCharCompressType;
    UINT16          nReserveUInt5;
    UINT16          nReserveUInt6;
    UINT16          nReserveUInt7;
    FASTBOOL        mbModelLocked;
    FASTBOOL        mbKernAsianPunctuation;
    FASTBOOL        mbInDestruction;

    // Zeiger auf Paletten, Listen und Tabellen
    XColorTable*    pColorTable;
    XDashList*      pDashList;
    XLineEndList*   pLineEndList;
    XHatchList*     pHatchList;
    XGradientList*  pGradientList;
    XBitmapList*    pBitmapList;

    // New src638: NumberFormatter for drawing layer and
    // method for getting it. It is constructed on demand
    // and destroyed when destroying the SdrModel.
    SvNumberFormatter* mpNumberFormatter;
public:
protected:

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > createUnoModel();

private:
    // Nicht implementiert:
    SdrModel(const SdrModel& rSrcModel);
#if _SOLAR__PRIVATE
    // Zaehlt alle rauszustreamenden Objekte
    ULONG ImpCountAllSteamComponents() const;

    void ImpSetUIUnit();
    void ImpSetOutlinerDefaults( SdrOutliner* pOutliner, BOOL bInit = FALSE );
    void ImpReformatAllTextObjects();
    void ImpReformatAllEdgeObjects();	// #103122#
    void ImpCreateTables();
    void ImpCtor(SfxItemPool* pPool, SvPersist* pPers, FASTBOOL bUseExtColorTable,
        FASTBOOL bLoadRefCounts = TRUE);

#endif // __PRIVATE

    // this is a weak reference to a possible living api wrapper for this model
    ::com::sun::star::uno::WeakReference< ::com::sun::star::uno::XInterface > mxUnoModel;

public:
#if _SOLAR__PRIVATE
    FASTBOOL IsPasteResize() const        { return bPasteResize; }
    void     SetPasteResize(FASTBOOL bOn) { bPasteResize=bOn; }
#endif // __PRIVATE
    TYPEINFO();
    // Steckt man hier seinen eigenen Pool rein, so wird die Klasse auch
    // Aktionen an ihm vornehmen (Put(),Remove()). Bei Zerstoerung von
    // SdrModel wird dieser Pool ver delete geloescht!
    // Gibt man den Konstruktor stattdessen eine NULL mit, so macht sich
    // die Klasse einen eigenen Pool (SdrItemPool), den sie dann auch im
    // Destruktor zerstoert.
    // Bei Verwendung eines eigenen Pools ist darauf zu achten, dass dieser
    // von SdrItemPool abgeleitet ist, falls man von SdrAttrObj abgeleitete
    // Zeichenobjekte verwenden moechte. Setzt man degegen nur vom abstrakten
    // Basisobjekt SdrObject abgeleitete Objekte ein, so ist man frei in der
    // Wahl des Pools.
    SdrModel(SfxItemPool* pPool=NULL, SvPersist* pPers=NULL, INT32 bLoadRefCounts = LOADREFCOUNTS);
    SdrModel(const String& rPath, SfxItemPool* pPool=NULL, SvPersist* pPers=NULL, INT32 bLoadRefCounts = LOADREFCOUNTS);
    SdrModel(const String& rPath, SfxItemPool* pPool, SvPersist* pPers, FASTBOOL bUseExtColorTable, INT32 bLoadRefCounts = LOADREFCOUNTS);
    virtual ~SdrModel();
    void    Clear();
    // Intern genutzt:
    void    DoProgress(ULONG nVal);
    void    IncProgress()                       { DoProgress(nProgressAkt+1); }
    // DokumentInfo des Model erfragen
    const SdrModelInfo& GetInfo() const         { return aInfo; }
    SdrModelInfo&       GetInfo()               { return aInfo; }
    // Hier kann man erfragen, ob das Model gerade eingrstreamt wird
    FASTBOOL IsLoading() const                  { return bLoading; }
    // Um ein SdrModel im Office-weiten Format ins Clipboard zu stellen darf
    // der entsprechende Stream keine applikationsspeziefischen Daten
    // beinhalten. Ableitungen von SdrModel/SdrPage muessen dieses Flag beim
    // Streamen abfragen und entspsrechend reagieren.
    FASTBOOL IsStreamingSdrModel() const        { return bStreamingSdrModel; }
    void     SetStreamingSdrModel(FASTBOOL bOn) { bStreamingSdrModel=bOn; }
    // Muss z.B. ueberladen werden, um das Swappen/LoadOnDemand von Grafiken
    // zu ermoeglichen. Wird rbDeleteAfterUse auf TRUE gesetzt, so wird
    // die SvStream-Instanz vom Aufrufer nach Gebrauch destruiert.
    // Wenn diese Methode NULL liefert, wird zum Swappen eine temporaere
    // Datei angelegt.
    // Geliefert werden muss der Stream, aus dem das Model geladen wurde
    // bzw. in den es zuletzt gespeichert wurde.
    virtual SvStream* GetDocumentStream( SdrDocumentStreamInfo& rStreamInfo ) const;
    // Die Vorlagenattribute der Zeichenobjekte in harte Attribute verwandeln.
    // Nichtpersistente Objekte aus dem Model entfernen. Dies sind einerseits
    // Objekte die bei SdrObject::IsNotPersistent() ein FALSE liefert. Wurde
    // zuvor SetStreamingSdrModel(TRUE) gerufen, so werden auch OLE-Objekte
    // entfernt. Fuer #44515#.
    // Wer sich von SdrPage ableitet muss sich auch von SdrModel ableiten
    // und diese beiden VM AllocPage() und AllocModel() ueberladen...
    virtual SdrPage*  AllocPage(FASTBOOL bMasterPage);
    virtual SdrModel* AllocModel() const;
    // Auch diese Methode sollte dann ueberladen werden. Ein gelinktes
    // Gruppenobjekt verwendet diesen Aufruf z.B. um sich zu aktuallisieren.
    // Die Methode muss ein neues Model alloziieren und aus der uebergebenen
    // Datei laden. Bei Misserfolg kann die Methode NULL liefern.
    virtual void            DisposeLoadedModels();

    // Aenderungen an den Layern setzen das Modified-Flag und broadcasten am Model!
    const SdrLayerAdmin& GetLayerAdmin() const                  { return *pLayerAdmin; }
    SdrLayerAdmin&       GetLayerAdmin()                        { return *pLayerAdmin; }

    const SfxItemPool&   GetItemPool() const                    { return *pItemPool; }
    SfxItemPool&         GetItemPool()                          { return *pItemPool; }

//-/	const SfxItemPool&   GetUndoItemPool() const                    { return *pUndoItemPool; }
//-/	SfxItemPool&         GetUndoItemPool()                          { return *pUndoItemPool; }

    SdrOutliner&         GetDrawOutliner(const SdrTextObj* pObj=NULL) const;
    SdrOutliner&         GetHitTestOutliner() const { return *pHitTestOutliner; }
    // Die TextDefaults (Font,Hoehe,Farbe) in ein Set putten
    void         		 SetTextDefaults() const;
    static void    		 SetTextDefaults( SfxItemPool* pItemPool, ULONG nDefTextHgt );

    // ReferenzDevice fuer die EditEngine
    void                 SetRefDevice(OutputDevice* pDev);
    OutputDevice*        GetRefDevice() const                   { return pRefOutDev; }
    // Wenn ein neuer MapMode am RefDevice gesetzt wird o.ae.
    void                 RefDeviceChanged(); // noch nicht implementiert
    // Default-Schrifthoehe in logischen Einheiten
    void                 SetDefaultFontHeight(ULONG nVal);
    ULONG                GetDefaultFontHeight() const           { return nDefTextHgt; }
    // Default-Tabulatorweite fuer die EditEngine
    void                 SetDefaultTabulator(USHORT nVal);
    USHORT               GetDefaultTabulator() const            { return nDefaultTabulator; }

    // Der DefaultStyleSheet wird jedem Zeichenobjekt verbraten das in diesem
    // Model eingefuegt wird und kein StyleSheet gesetzt hat.
    SfxStyleSheet*       GetDefaultStyleSheet() const             { return pDefaultStyleSheet; }
    void                 SetDefaultStyleSheet(SfxStyleSheet* pDefSS) { pDefaultStyleSheet=pDefSS; }

    SvxLinkManager*      GetLinkManager()                         { return pLinkManager; }
    void                 SetLinkManager(SvxLinkManager* pLinkMgr) { pLinkManager = pLinkMgr; }

    SvPersist*           GetPersist() const                       { return pPersist; }
    void				 ClearPersist()                       { pPersist = 0; }
    void				 SetPersist( SvPersist *p )           { pPersist = p; }

    // Masseinheit fuer die Zeichenkoordinaten.
    // Default ist 1 logische Einheit = 1/100mm (Unit=MAP_100TH_MM, Fract=(1,1)).
    // Beispiele:
    //   MAP_POINT,    Fraction(72,1)    : 1 log Einh = 72 Point   = 1 Inch
    //   MAP_POINT,    Fraction(1,20)    : 1 log Einh = 1/20 Point = 1 Twip
    //   MAP_TWIP,     Fraction(1,1)     : 1 log Einh = 1 Twip
    //   MAP_100TH_MM, Fraction(1,10)    : 1 log Einh = 1/1000mm
    //   MAP_MM,       Fraction(1000,1)  : 1 log Einh = 1000mm     = 1m
    //   MAP_CM,       Fraction(100,1)   : 1 log Einh = 100cm      = 1m
    //   MAP_CM,       Fraction(100,1)   : 1 log Einh = 100cm      = 1m
    //   MAP_CM,       Fraction(100000,1): 1 log Einh = 100000cm   = 1km
    // (PS: Lichtjahre sind somit also nicht darstellbar).
    // Die Skalierungseinheit wird benoetigt, damit die Engine das Clipboard
    // mit den richtigen Groessen beliefern kann.
    MapUnit          GetScaleUnit() const                       { return eObjUnit; }
    void             SetScaleUnit(MapUnit eMap);
    const Fraction&  GetScaleFraction() const                   { return aObjUnit; }
    void             SetScaleFraction(const Fraction& rFrac);
    // Beides gleichzeitig setzen ist etwas performanter
    void             SetScaleUnit(MapUnit eMap, const Fraction& rFrac);

    // Maximale Groesse z.B. fuer Autogrowing-Texte
    const Size&      GetMaxObjSize() const                      { return aMaxObjSize; }
    void             SetMaxObjSize(const Size& rSiz)            { aMaxObjSize=rSiz; }

    // Damit die View! in der Statuszeile vernuenftige Zahlen anzeigen kann:
    // Default ist mm.
    void             SetUIUnit(FieldUnit eUnit);
    FieldUnit        GetUIUnit() const                          { return eUIUnit; }
    // Der Masstab der Zeichnung. Default 1/1.
    void             SetUIScale(const Fraction& rScale);
    const Fraction&  GetUIScale() const                         { return aUIScale; }
    // Beides gleichzeitig setzen ist etwas performanter
    void             SetUIUnit(FieldUnit eUnit, const Fraction& rScale);

    const Fraction&  GetUIUnitFact() const                      { return aUIUnitFact; }
    const String&    GetUIUnitStr() const                       { return aUIUnitStr; }
    int              GetUIUnitKomma() const                     { return nUIUnitKomma; }
    FASTBOOL         IsUIOnlyKomma() const                      { return bUIOnlyKomma; }

    void             TakeUnitStr(FieldUnit eUnit, String& rStr) const;
    void             TakeMetricStr(long nVal, String& rStr, FASTBOOL bNoUnitChars=FALSE, sal_Int32 nNumDigits = -1) const;

    // RecalcPageNums wird idR. nur von der Page gerufen.
    FASTBOOL         IsPagNumsDirty() const                     { return bPagNumsDirty; };
    FASTBOOL         IsMPgNumsDirty() const                     { return bMPgNumsDirty; };
    void             RecalcPageNums(FASTBOOL bMaster);
    // Nach dem Insert gehoert die Page dem SdrModel.
    virtual void     InsertPage(SdrPage* pPage, USHORT nPos=0xFFFF);
    virtual void     DeletePage(USHORT nPgNum);
    // Remove bedeutet Eigentumsuebereignung an den Aufrufer (Gegenteil von Insert)
    virtual SdrPage* RemovePage(USHORT nPgNum);
    const SdrPage*   GetPage(USHORT nPgNum) const               { return (SdrPage*)(aPages.GetObject(nPgNum)); }
    SdrPage*         GetPage(USHORT nPgNum)                     { return (SdrPage*)(aPages.GetObject(nPgNum)); }
    //USHORT           GetPageNum(const SdrPage* pPage) const;
    USHORT           GetPageCount() const                       { return USHORT(aPages.Count()); }
    // Masterpages
    virtual void     InsertMasterPage(SdrPage* pPage, USHORT nPos=0xFFFF);
    virtual void     DeleteMasterPage(USHORT nPgNum);
    // Remove bedeutet Eigentumsuebereignung an den Aufrufer (Gegenteil von Insert)
    virtual SdrPage* RemoveMasterPage(USHORT nPgNum);
    virtual void     MoveMasterPage(USHORT nPgNum, USHORT nNewPos);
    const SdrPage*   GetMasterPage(USHORT nPgNum) const         { return (SdrPage*)(aMaPag.GetObject(nPgNum)); }
    SdrPage*         GetMasterPage(USHORT nPgNum)               { return (SdrPage*)(aMaPag.GetObject(nPgNum)); }
    //USHORT           GetMasterPageNum(const SdrPage* pPage) const;
    USHORT           GetMasterPageCount() const                 { return USHORT(aMaPag.Count()); }

    // Modified-Flag. Wird automatisch gesetzt, wenn an den Pages oder
    // Zeichenobjekten was geaendert wird. Zuruecksetzen muss man es
    // jedoch selbst (z.B. bei Save() ...).
    FASTBOOL        IsChanged() const                           { return bChanged; }
    virtual void    SetChanged(FASTBOOL bFlg=TRUE);

    // PageNotValid bedeutet, dass das Model lediglich Objekte traegt die zwar
    // auf einer Page verankert sind, die Page aber nicht gueltig ist. Diese
    // Kennzeichnung wird fuers Clipboard/Drag&Drop benoetigt.
    FASTBOOL        IsPageNotValid() const                     { return bPageNotValid; }
    void            SetPageNotValid(FASTBOOL bJa=TRUE)         { bPageNotValid=bJa; }

    // Schaltet man dieses Flag auf TRUE, so werden Grafikobjekte
    // portabel gespeichert. Es findet dann beim Speichern ggf.
    // eine implizite Wandlung von Metafiles statt.
    // Default=FALSE. Flag ist nicht persistent.
    FASTBOOL        IsSavePortable() const                     { return bSavePortable; }
    void            SetSavePortable(FASTBOOL bJa=TRUE)         { bSavePortable=bJa; }

    // Schaltet man dieses Flag auf TRUE, so werden
    // Pixelobjekte (stark) komprimiert gespeichert.
    // Default=FALSE. Flag ist nicht persistent.
    FASTBOOL        IsSaveCompressed() const                   { return bSaveCompressed; }
    void            SetSaveCompressed(FASTBOOL bJa=TRUE)       { bSaveCompressed=bJa; }

    // Schaltet man dieses Flag auf TRUE, so werden
    // Grafikobjekte mit gesetztem Native-Link
    // native gespeichert.
    // Default=FALSE. Flag ist nicht persistent.
    FASTBOOL        IsSaveNative() const                       { return bSaveNative; }
    void            SetSaveNative(FASTBOOL bJa=TRUE)           { bSaveNative=bJa; }

    // Schaltet man dieses Flag auf TRUE, so werden die Grafiken
    // von Grafikobjekten:
    // - beim Laden eines Dokuments nicht sofort mitgeladen,
    //   sondern erst wenn sie gebraucht (z.B. angezeigt) werden.
    // - ggf. wieder aus dem Speicher geworfen, falls Sie gerade
    //   nicht benötigt werden.
    // Damit das funktioniert, muss die virtuelle Methode
    // GetDocumentStream() ueberladen werden.
    // Default=FALSE. Flag ist nicht persistent.
    FASTBOOL        IsSwapGraphics() const { return bSwapGraphics; }
    void            SetSwapGraphics(FASTBOOL bJa=TRUE);
    void			SetSwapGraphicsMode(ULONG nMode) { nSwapGraphicsMode = nMode; }
    ULONG			GetSwapGraphicsMode() const { return nSwapGraphicsMode; }

    FASTBOOL        IsSaveOLEPreview() const          { return bSaveOLEPreview; }
    void            SetSaveOLEPreview( FASTBOOL bSet) { bSaveOLEPreview = bSet; }

    // Damit die Bildschirmausgabe von Bitmaps (insbesondere bei gedrehten)
    // etwas schneller wird, werden sie gecachet. Diesen Cache kann man mit
    // diesem Flag ein-/ausschalten. Beim naechsten Paint wird an den Objekten
    // dann ggf. ein Image gemerkt bzw. freigegeben. Wandert ein Bitmapobjekt
    // in's Undo, so wird der Cache fuer dieses Objekt sofort ausgeschaltet
    // (Speicher sparen).
    // Default=Cache eingeschaltet. Flag ist nicht persistent.
    FASTBOOL        IsBitmapCaching() const                     { return !bNoBitmapCaching; }
    void            SetBitmapCaching(FASTBOOL bJa=TRUE)         { bNoBitmapCaching=!bJa; }

    // Defaultmaessig (FALSE) kann man Textrahmen ohne Fuellung durch
    // Mausklick selektieren. Nach Aktivierung dieses Flags trifft man sie
    // nur noch in dem Bereich, wo sich auch tatsaechlich Text befindet.
    FASTBOOL        IsPickThroughTransparentTextFrames() const  { return bTransparentTextFrames; }
    void            SetPickThroughTransparentTextFrames(FASTBOOL bOn) { bTransparentTextFrames=bOn; }

    // Die Seite, die gerade an einer View gepaintet wird.
    SdrPageView*    GetPaintingPageView() const                 { return pAktPaintPV; }
    void            SetPaintingPageView(SdrPageView* pPV)       { pAktPaintPV=pPV; }

    // Darf denn das Model ueberhaupt veraendert werden?
    // Wird nur von den Possibility-Methoden der View ausgewerdet.
    // Direkte Manipulationen am Model, ... berueksichtigen dieses Flag nicht.
    // Sollte ueberladen werden und entsprechend des ReadOnly-Status des Files
    // TRUE oder FALSE liefern (Methode wird oeffters gerufen, also ein Flag
    // verwenden!).
    virtual FASTBOOL IsReadOnly() const;
    virtual void     SetReadOnly(FASTBOOL bYes);

    // Vermischen zweier SdrModel. Zu beachten sei, dass rSourceModel nicht
    // const ist. Die Pages werden beim einfuegen nicht kopiert, sondern gemoved.
    // rSourceModel ist anschliessend u.U. weitgehend leer.
    // nFirstPageNum,nLastPageNum: Die aus rSourceModel zu uebernehmenden Seiten
    // nDestPos..................: Einfuegeposition
    // bMergeMasterPages.........: TRUE =benoetigte MasterPages werden aus
    //                                   rSourceModel ebenfalls uebernommen
    //                             FALSE=Die MasterPageDescriptoren der Seiten
    //                                   aus rSourceModel werden auf die
    //                                   vorhandenen MasterPages gemappt.
    // bUndo.....................: Fuer das Merging wird eine UndoAction generiert.
    //                             Undo ist nur fuer das ZielModel, nicht fuer
    //                             rSourceModel.
    // bTreadSourceAsConst.......: TRUE=Das SourceModel wird nicht veraendert,.
    //                             d.h die Seiten werden kopiert.

    // Ist wie Merge(SourceModel=DestModel,nFirst,nLast,nDest,FALSE,FALSE,bUndo,!bMoveNoCopy);

    // Mit BegUndo() / EndUndo() ist es moeglich beliebig viele UndoActions
    // beliebig tief zu klammern. Als Kommentar der
    // UndoAction wird der des ersten BegUndo(String) aller Klammerungen
    // verwendet. Der NotifyUndoActionHdl wird in diesem Fall erst beim letzten
    // EndUndo() gerufen. Bei einer leeren Klammerung wird keine UndoAction
    // generiert.
    // Alle direkten Aktionen am SdrModel erzeugen keine UndoActions, die
    // Aktionen an der SdrView dagegen generieren solche.
    void AddUndo(SdrUndoAction* pUndo);
    USHORT GetUndoBracketLevel() const                       { return nUndoLevel; }
    const SdrUndoGroup* GetAktUndoGroup() const              { return pAktUndoGroup; }
    // nur nach dem 1. BegUndo oder vor dem letzten EndUndo:

    // Das Undo-Managment findet nur statt, wenn kein NotifyUndoAction-Handler
    // gesetzt ist.
    // Default ist 16. Minimaler MaxUndoActionCount ist 1!
    void  SetMaxUndoActionCount(ULONG nAnz);
    ULONG GetMaxUndoActionCount() const { return nMaxUndoCount; }
    void  ClearUndoBuffer();
    // UndoAction(0) ist die aktuelle (also die zuletzt eingegangene)
    ULONG GetUndoActionCount() const                      { return pUndoStack!=NULL ? pUndoStack->Count() : 0; }
    const SfxUndoAction* GetUndoAction(ULONG nNum) const  { return (SfxUndoAction*)(pUndoStack!=NULL ? pUndoStack->GetObject(nNum) : NULL); }
    // RedoAction(0) ist die aktuelle (also die des letzten Undo)
    ULONG GetRedoActionCount() const                      { return pRedoStack!=NULL ? pRedoStack->Count() : 0; }
    const SfxUndoAction* GetRedoAction(ULONG nNum) const  { return (SfxUndoAction*)(pRedoStack!=NULL ? pRedoStack->GetObject(nNum) : NULL); }


    // Hier kann die Applikation einen Handler setzen, der die auflaufenden
    // UndoActions einsammelt. Der Handler hat folgendes Aussehen:
    //   void __EXPORT NotifyUndoActionHdl(SfxUndoAction* pUndoAction);
    // Beim Aufruf des Handlers findet eine Eigentumsuebereignung statt; die
    // UndoAction gehoert somit dem Handler, nicht mehr dem SdrModel.
    void        SetNotifyUndoActionHdl(const Link& rLink)    { aUndoLink=rLink; }
    const Link& GetNotifyUndoActionHdl() const               { return aUndoLink; }

    // Hier kann man einen Handler setzen der beim Streamen mehrfach gerufen
    // wird und ungefaehre Auskunft ueber den Fortschreitungszustand der
    // Funktion gibt. Der Handler muss folgendes Aussehen haben:
    //   void __EXPORT class::IOProgressHdl(const USHORT& nPercent);
    // Der erste Aufruf des Handlers erfolgt grundsaetzlich mit 0, der letzte
    // mit 100. Dazwischen erfolgen maximal 99 Aufrufe mit Werten 1...99.
    // Man kann also durchaus bei 0 den Progressbar Initiallisieren und bei
    // 100 wieder schliessen. Zu beachten sei, dass der Handler auch gerufen
    // wird, wenn die App Draw-Daten im officeweiten Draw-Exchange-Format
    // bereitstellt, denn dies geschieht durch streamen in einen MemoryStream.
    void        SetIOProgressHdl(const Link& rLink)          { aIOProgressLink=rLink; }
    const Link& GetIOProgressHdl() const                     { return aIOProgressLink; }

//    void AddViewListener(SfxListener& rListener)    { AddListener(rListener); }
//    void RemoveViewListener(SfxListener& rListener) { RemoveListener(rListener); }

    // Zugriffsmethoden fuer Paletten, Listen und Tabellen
    void            SetColorTable(XColorTable* pTable)       { pColorTable=pTable; }
    XColorTable*    GetColorTable() const                    { return pColorTable; }
    void            SetDashList(XDashList* pList)            { pDashList=pList; }
    XDashList*      GetDashList() const                      { return pDashList; }
    void            SetLineEndList(XLineEndList* pList)      { pLineEndList=pList; }
    XLineEndList*   GetLineEndList() const                   { return pLineEndList; }
    void            SetHatchList(XHatchList* pList)          { pHatchList=pList; }
    XHatchList*     GetHatchList() const                     { return pHatchList; }
    void            SetGradientList(XGradientList* pList)    { pGradientList=pList; }
    XGradientList*  GetGradientList() const                  { return pGradientList; }
    void            SetBitmapList(XBitmapList* pList)        { pBitmapList=pList; }
    XBitmapList*    GetBitmapList() const                    { return pBitmapList; }

    // Der StyleSheetPool wird der DrawingEngine nur bekanntgemacht.
    // Zu loeschen hat ihn schliesslich der, der ihn auch konstruiert hat.
    SfxStyleSheetBasePool* GetStyleSheetPool() const         { return pStyleSheetPool; }
    void SetStyleSheetPool(SfxStyleSheetBasePool* pPool)	{ pStyleSheetPool = pPool; }

    // Strings werden beim rausstreamen in den am Stream eingestellten
    // StreamCharSet konvertiert.
    // Abgeleitete Klassen mit eigenen persistenten Membern ueberladen
    // die virtuellen Methoden ReadData() und WriteData().
    friend SvStream& operator<<(SvStream& rOut, const SdrModel& rMod);
    // Wenn das Model im Stream in einem fremden CharSet vorliegt
    // wird beim einstreamen implizit auf den SystemCharSet konvertiert.
    // Abgeleitete Klassen mit eigenen persistenten Membern ueberladen
    // die virtuellen Methoden ReadData() und WriteData().
    friend SvStream& operator>>(SvStream& rIn, SdrModel& rMod);

    // Stream muss am Anfang des SdrModel stehen. Die FileVersion des im
    // Stream befindlichen Models muss >=11 sein, ansonsten wird die
    // ModelInfo nicht geschrieben.

    // Abgeleitete Klassen ueberladen lediglich die Methoden ReadData() und
    // WriteData(). Diese werden von den Streamoperatoren des Model gerufen.
    // Wichtig ist, dass die ueberladen Methoden der abgeleiteten Klasse
    // als erstes die Methode der Basisklasse rufen, da sonst der CharSet
    // am Stream nicht korrekt gesetzt wird, ...
    virtual void WriteData(SvStream& rOut) const;
    virtual void ReadData(const SdrIOHeader& rHead, SvStream& rIn);
    // AfterRead wird gerufen, nachdem das gesamte Model eingestreamt
    // worden ist. Damit ist ein PostProcessing moeglich um beispielsweise
    // Objektverknuepfungen ueber Surrogate herzustellen o.ae.
    // Anwendungsbeispiel hierfuer SdrEdgeObj (Objektverbinder)
    // Der Aufruf erfolgt innerhalb des Streamoperators des Models unmittelbar
    // nach ReadData.
    virtual void AfterRead();

    // Diese Methode fuert einen Konsistenzcheck auf die Struktur des Models
    // durch. Geprueft wird insbesondere die Verkettung von Verschachtelten
    // Gruppenobjekten, aber auch Stati wie bInserted sowie Model* und Page*
    // der Objects, SubLists und Pages. Bei korrekter Struktur liefert die
    // Methode TRUE, andernfalls FALSE.
    // Dieser Check steht nur zur Verfuegung, wenn die Engine mit DBG_UTIL
    // uebersetzt wurde. Andernfalls liefert die Methode immer TRUE. (ni)

    BOOL 	IsStarDrawPreviewMode() { return bStarDrawPreviewMode; }

    SotStorage*	GetModelStorage() const { return pModelStorage; }
    void		SetModelStorage( SotStorage* pStor ) { pModelStorage = pStor; }

    // pre- and postprocessing for objects for saving
    void PreSave();
    void PostSave();

    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > getUnoModel();
    void setUnoModel( ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > xModel ) { mxUnoModel = xModel; }

    // these functions are used by the api to disable repaints during a
    // set of api calls.
    BOOL isLocked() const { return mbModelLocked; }
    void setLock( BOOL bLock );

    void			SetForbiddenCharsTable( rtl::Reference<SvxForbiddenCharactersTable> xForbiddenChars );
    rtl::Reference<SvxForbiddenCharactersTable>	GetForbiddenCharsTable() const;

    void SetCharCompressType( UINT16 nType );
    UINT16 GetCharCompressType() const { return mnCharCompressType; }

    void SetKernAsianPunctuation( sal_Bool bEnabled );
    sal_Bool IsKernAsianPunctuation() const { return (sal_Bool)mbKernAsianPunctuation; }

    void ReformatAllTextObjects();


    SdrOutliner* createOutliner( USHORT nOutlinerMode );
    void disposeOutliner( SdrOutliner* pOutliner );

    sal_Bool IsWriter() const { return !bMyPool; }

    /** returns the numbering type that is used to format page fields in drawing shapes */
    virtual SvxNumType GetPageNumType() const;

    /** copies the items from the source set to the destination set. Both sets must have
        same ranges but can have different pools. If pNewModel is optional. If it is null,
        this model is used. */
    void MigrateItemSet( const SfxItemSet* pSourceSet, SfxItemSet* pDestSet, SdrModel* pNewModel );

    bool IsInDestruction() const;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

}//end of namespace binfilter
#endif //_SVDMODEL_HXX

/* /////////////////////////////////////////////////////////////////////////////////////////////////
            ?----------?
            | SdrModel  |
            ?-?-----??
               |      ?----------?
          ?---?----?           |
          |   ...    |            |
     ?---?--??---?--? ?----?-------?
     |SdrPage | |SdrPage |  |SdrLayerAdmin |
     ?--?---???-?-ÂÙ  ?--?------?-?
         |        |  |  |       |       ?------------------?
    ?---?---?          ?----?----?            ?------?------?
    |   ...   |           |    ...    |             |      ...      |
?--?--??--?--? ?---?---??---?---? ?----?-----??-----?----?
|SdrObj | |SdrObj |  |SdrLayer | |SdrLayer |  |SdrLayerSet | |SdrLayerSet |
?------??------? ?--------??--------? ?-----------??-----------?
Die Klasse SdrModel ist der Kopf des Datenmodells der StarView Drawing-Engine.

///////////////////////////////////////////////////////////////////////////////////////////////// */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

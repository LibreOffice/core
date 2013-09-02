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

#ifndef _SVDMODEL_HXX
#define _SVDMODEL_HXX

#include <com/sun/star/uno/Sequence.hxx>
#include <cppuhelper/weakref.hxx>
#include <sot/storage.hxx>
#include <tools/link.hxx>
#include <tools/weakbase.hxx>
#include <vcl/mapmod.hxx>
#include <svl/brdcst.hxx>
#include <tools/string.hxx>
#include <tools/datetime.hxx>
#include <svl/hint.hxx>

#include <svl/style.hxx>
#include <svx/xtable.hxx>
#include <svx/pageitem.hxx>
#include <vcl/field.hxx>

#include <boost/shared_ptr.hpp>

class OutputDevice;
#include <svx/svdtypes.hxx> // fuer enum RepeatFuncts
#include "svx/svxdllapi.h"

#include <rtl/ref.hxx>
#include <deque>

#if defined(UNX) || defined(WNT)
#define DEGREE_CHAR ((sal_Unicode)176)   /* 0xB0 = Ansi */
#endif

#ifndef DEGREE_CHAR
#error unbekannte Plattrorm
#endif

class SdrOutliner;
class SdrLayerAdmin;
class SdrObjList;
class SdrObject;
class SdrPage;
class SdrPageView;
class SdrTextObj;
class SdrUndoAction;
class SdrUndoGroup;
class AutoTimer;
class SfxItemPool;
class SfxItemSet;
class SfxRepeatTarget;
class SfxStyleSheet;
class SfxUndoAction;
class SfxUndoManager;
class XBitmapList;
class XColorList;
class XDashList;
class XGradientList;
class XHatchList;
class XLineEndList;
class SvxForbiddenCharactersTable;
class SvNumberFormatter;
class SotStorage;
class SdrOutlinerCache;
class SotStorageRef;
class SdrUndoFactory;
namespace comphelper
{
    class IEmbeddedHelper;
    class LifecycleProxy;
}
namespace sfx2
{
    class LinkManager;
}
////////////////////////////////////////////////////////////////////////////////////////////////////

#define SDR_SWAPGRAPHICSMODE_NONE       0x00000000
#define SDR_SWAPGRAPHICSMODE_TEMP       0x00000001
#define SDR_SWAPGRAPHICSMODE_DOC        0x00000002
#define SDR_SWAPGRAPHICSMODE_PURGE      0x00000100
#define SDR_SWAPGRAPHICSMODE_DEFAULT    (SDR_SWAPGRAPHICSMODE_TEMP|SDR_SWAPGRAPHICSMODE_DOC|SDR_SWAPGRAPHICSMODE_PURGE)

////////////////////////////////////////////////////////////////////////////////////////////////////

enum SdrHintKind
{
                  HINT_UNKNOWN,         // Unbekannt
                  HINT_LAYERCHG,        // Layerdefinition geaendert
                  HINT_LAYERORDERCHG,   // Layerreihenfolge geaendert (Insert/Remove/ChangePos)
                  HINT_PAGEORDERCHG,    // Reihenfolge der Seiten (Zeichenseiten oder Masterpages) geaendert (Insert/Remove/ChangePos)
                  HINT_OBJCHG,          // Objekt geaendert
                  HINT_OBJINSERTED,     // Neues Zeichenobjekt eingefuegt
                  HINT_OBJREMOVED,      // Zeichenobjekt aus Liste entfernt
                  HINT_MODELCLEARED,    // gesamtes Model geloescht (keine Pages mehr da). not impl.
                  HINT_REFDEVICECHG,    // RefDevice geaendert
                  HINT_DEFAULTTABCHG,   // Default Tabulatorweite geaendert
                  HINT_DEFFONTHGTCHG,   // Default FontHeight geaendert
                  HINT_MODELSAVED,      // Dokument wurde gesichert
                  HINT_SWITCHTOPAGE,    // #94278# UNDO/REDO at an object evtl. on another page
                  HINT_BEGEDIT,         // Is called after the object has entered text edit mode
                  HINT_ENDEDIT          // Is called after the object has left text edit mode
};

class SVX_DLLPUBLIC SdrHint: public SfxHint
{
public:
    Rectangle                               maRectangle;
    const SdrPage*                          mpPage;
    const SdrObject*                        mpObj;
    const SdrObjList*                       mpObjList;
    SdrHintKind                             meHint;

public:
    TYPEINFO();

    explicit SdrHint(SdrHintKind eNewHint);
    explicit SdrHint(const SdrObject& rNewObj);

    void SetPage(const SdrPage* pNewPage);
    void SetObject(const SdrObject* pNewObj);
    void SetKind(SdrHintKind eNewKind);

    const SdrPage* GetPage() const;
    const SdrObject* GetObject() const;
    SdrHintKind GetKind() const;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

// Flag um nach dem Laden des Pools Aufzuraeumen (d.h. die RefCounts
// neu zu bestimmen und unbenutztes wegzuwerfen). sal_False == aktiv
#define LOADREFCOUNTS (false)

struct SdrModelImpl;

class SVX_DLLPUBLIC SdrModel : public SfxBroadcaster, public tools::WeakBase< SdrModel >
{
protected:
    DateTime       aReadDate;  // Datum des Einstreamens
    std::vector<SdrPage*> maMaPag;     // StammSeiten (Masterpages)
    std::vector<SdrPage*> maPages;
    Link           aUndoLink;  // Link fuer einen NotifyUndo-Handler
    Link           aIOProgressLink;
    OUString       aTablePath;
    Size           aMaxObjSize; // z.B. fuer Autogrowing Text
    Fraction       aObjUnit;   // Beschreibung der Koordinateneinheiten fuer ClipBoard, Drag&Drop, ...
    MapUnit        eObjUnit;   // see above
    FieldUnit      eUIUnit;      // Masseinheit, Masstab (z.B. 1/1000) fuer die UI (Statuszeile) wird von ImpSetUIUnit() gesetzt
    Fraction       aUIScale;     // see above
    OUString       aUIUnitStr;   // see above
    Fraction       aUIUnitFact;  // see above
    int            nUIUnitKomma; // see above

    SdrLayerAdmin*  pLayerAdmin;
    SfxItemPool*    pItemPool;
    comphelper::IEmbeddedHelper*
                    m_pEmbeddedHelper; // helper for embedded objects to get rid of the SfxObjectShell
    SdrOutliner*    pDrawOutliner;  // ein Outliner zur Textausgabe
    SdrOutliner*    pHitTestOutliner;// ein Outliner fuer den HitTest
    sal_uIntPtr           nDefTextHgt;    // Default Texthoehe in logischen Einheiten
    OutputDevice*   pRefOutDev;     // ReferenzDevice fuer die EditEngine
    sal_uIntPtr           nProgressAkt;   // fuer den
    sal_uIntPtr           nProgressMax;   // ProgressBar-
    sal_uIntPtr           nProgressOfs;   // -Handler
    rtl::Reference< SfxStyleSheetBasePool > mxStyleSheetPool;
    SfxStyleSheet*  pDefaultStyleSheet;
    SfxStyleSheet* mpDefaultStyleSheetForSdrGrafObjAndSdrOle2Obj; // #i119287#
    sfx2::LinkManager* pLinkManager;   // LinkManager
    std::deque<SfxUndoAction*>* pUndoStack;
    std::deque<SfxUndoAction*>* pRedoStack;
    SdrUndoGroup*   pAktUndoGroup;  // Fuer mehrstufige
    sal_uInt16          nUndoLevel;     // Undo-Klammerung
    sal_uInt16          nProgressPercent; // fuer den ProgressBar-Handler
    sal_uInt16          nLoadVersion;   // Versionsnummer der geladenen Datei
    bool            bMyPool:1;        // zum Aufraeumen von pMyPool ab 303a
    bool            bUIOnlyKomma:1; // see eUIUnit
    bool            mbUndoEnabled:1;  // If false no undo is recorded or we are during the execution of an undo action
    bool            bExtColorTable:1; // Keinen eigenen ColorTable
    bool            mbChanged:1;
    bool            bInfoChanged:1;
    bool            bPagNumsDirty:1;
    bool            bMPgNumsDirty:1;
    bool            bPageNotValid:1;  // TRUE=Doc ist nur ObjektTraeger. Page ist nicht gueltig.
    bool            bSavePortable:1;  // Metafiles portabel speichern
    bool            bNoBitmapCaching:1;   // Bitmaps fuer Screenoutput cachen
    bool            bReadOnly:1;
    bool            bTransparentTextFrames:1;
    bool            bSaveCompressed:1;
    bool            bSwapGraphics:1;
    bool            bPasteResize:1; // Objekte werden gerade resized wegen Paste mit anderem MapMode
    bool            bSaveOLEPreview:1;      // save preview metafile of OLE objects
    bool            bSaveNative:1;
    bool            bStarDrawPreviewMode:1;
    bool            mbDisableTextEditUsesCommonUndoManager:1;
    sal_uInt16          nStreamCompressMode;  // Komprimiert schreiben?
    sal_uInt16          nStreamNumberFormat;
    sal_uInt16          nDefaultTabulator;
    sal_uInt32          nMaxUndoCount;


//////////////////////////////////////////////////////////////////////////////
// sdr::Comment interface
private:
    // the next unique comment ID, used for counting added comments. Initialized
    // to 0. UI shows one more due to the fact that 0 is a no-no for users.
    sal_uInt32                                          mnUniqueCommentID;

public:
    // create a new, unique comment ID
    sal_uInt32 GetNextUniqueCommentID();

    // get the author name
    OUString GetDocumentAuthorName() const;

    // for export
    sal_uInt32 GetUniqueCommentID() const { return mnUniqueCommentID; }

    // for import
    void SetUniqueCommentID(sal_uInt32 nNewID) { if(nNewID != mnUniqueCommentID) { mnUniqueCommentID = nNewID; } }

    sal_uInt16          nStarDrawPreviewMasterPageNum;
    SvxForbiddenCharactersTable* mpForbiddenCharactersTable;
    sal_uIntPtr         nSwapGraphicsMode;

    SdrOutlinerCache* mpOutlinerCache;
    SdrModelImpl*   mpImpl;
    sal_uInt16          mnCharCompressType;
    sal_uInt16          mnHandoutPageCount;
    sal_uInt16          nReserveUInt6;
    sal_uInt16          nReserveUInt7;
    bool            mbModelLocked;
    bool            mbKernAsianPunctuation;
    bool            mbAddExtLeading;
    bool            mbInDestruction;

    // Color, Dash, Line-End, Hatch, Gradient, Bitmap property lists ...
    XPropertyListRef maProperties[XPROPERTY_LIST_COUNT];

    // New src638: NumberFormatter for drawing layer and
    // method for getting it. It is constructed on demand
    // and destroyed when destroying the SdrModel.
    SvNumberFormatter* mpNumberFormatter;
public:
    sal_uInt16 getHandoutPageCount() const { return mnHandoutPageCount; }
    void setHandoutPageCount( sal_uInt16 nHandoutPageCount ) { mnHandoutPageCount = nHandoutPageCount; }

protected:

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > createUnoModel();

private:
    // Nicht implementiert:
    SVX_DLLPRIVATE SdrModel(const SdrModel& rSrcModel);
    SVX_DLLPRIVATE void operator=(const SdrModel& rSrcModel);
    SVX_DLLPRIVATE bool operator==(const SdrModel& rCmpModel) const;
    SVX_DLLPRIVATE void ImpPostUndoAction(SdrUndoAction* pUndo);
    SVX_DLLPRIVATE void ImpSetUIUnit();
    SVX_DLLPRIVATE void ImpSetOutlinerDefaults( SdrOutliner* pOutliner, sal_Bool bInit = sal_False );
    SVX_DLLPRIVATE void ImpReformatAllTextObjects();
    SVX_DLLPRIVATE void ImpReformatAllEdgeObjects();    // #103122#
    SVX_DLLPRIVATE void ImpCreateTables();
    SVX_DLLPRIVATE void ImpCtor(SfxItemPool* pPool, ::comphelper::IEmbeddedHelper* pPers, bool bUseExtColorTable,
        bool bLoadRefCounts = true);


    // this is a weak reference to a possible living api wrapper for this model
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > mxUnoModel;

public:
    bool     IsPasteResize() const        { return bPasteResize; }
    void     SetPasteResize(bool bOn) { bPasteResize=bOn; }
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
    explicit SdrModel(SfxItemPool* pPool=NULL, ::comphelper::IEmbeddedHelper* pPers=NULL, sal_Bool bLoadRefCounts = LOADREFCOUNTS);
    explicit SdrModel(const OUString& rPath, SfxItemPool* pPool=NULL, ::comphelper::IEmbeddedHelper* pPers=NULL, sal_Bool bLoadRefCounts = LOADREFCOUNTS);
    SdrModel(SfxItemPool* pPool, ::comphelper::IEmbeddedHelper* pPers, bool bUseExtColorTable, sal_Bool bLoadRefCounts = LOADREFCOUNTS);
    SdrModel(const OUString& rPath, SfxItemPool* pPool, ::comphelper::IEmbeddedHelper* pPers, bool bUseExtColorTable, sal_Bool bLoadRefCounts = LOADREFCOUNTS);
    virtual ~SdrModel();
    void ClearModel(sal_Bool bCalledFromDestructor);

    // Hier kann man erfragen, ob das Model gerade eingrstreamt wird
    bool IsLoading() const                  { return sal_False /*BFS01 bLoading */; }
    // Muss z.B. ueberladen werden, um das Swappen/LoadOnDemand von Grafiken
    // zu ermoeglichen. Wird rbDeleteAfterUse auf sal_True gesetzt, so wird
    // die SvStream-Instanz vom Aufrufer nach Gebrauch destruiert.
    // Wenn diese Methode NULL liefert, wird zum Swappen eine temporaere
    // Datei angelegt.
    // Geliefert werden muss der Stream, aus dem das Model geladen wurde
    // bzw. in den es zuletzt gespeichert wurde.
    virtual ::com::sun::star::uno::Reference<
                ::com::sun::star::embed::XStorage> GetDocumentStorage() const;
    ::com::sun::star::uno::Reference<
            ::com::sun::star::io::XInputStream >
        GetDocumentStream(OUString const& rURL,
                ::comphelper::LifecycleProxy & rProxy) const;
    // Die Vorlagenattribute der Zeichenobjekte in harte Attribute verwandeln.
    void BurnInStyleSheetAttributes();
    // Wer sich von SdrPage ableitet muss sich auch von SdrModel ableiten
    // und diese beiden VM AllocPage() und AllocModel() ueberladen...
    virtual SdrPage*  AllocPage(bool bMasterPage);
    virtual SdrModel* AllocModel() const;

    // Aenderungen an den Layern setzen das Modified-Flag und broadcasten am Model!
    const SdrLayerAdmin& GetLayerAdmin() const                  { return *pLayerAdmin; }
    SdrLayerAdmin&       GetLayerAdmin()                        { return *pLayerAdmin; }

    const SfxItemPool&   GetItemPool() const                    { return *pItemPool; }
    SfxItemPool&         GetItemPool()                          { return *pItemPool; }

    SdrOutliner&         GetDrawOutliner(const SdrTextObj* pObj=NULL) const;

    SdrOutliner&         GetHitTestOutliner() const { return *pHitTestOutliner; }
    const SdrTextObj*    GetFormattingTextObj() const;
    // Die TextDefaults (Font,Hoehe,Farbe) in ein Set putten
    void                 SetTextDefaults() const;
    static void          SetTextDefaults( SfxItemPool* pItemPool, sal_uIntPtr nDefTextHgt );

    // ReferenzDevice fuer die EditEngine
    void                 SetRefDevice(OutputDevice* pDev);
    OutputDevice*        GetRefDevice() const                   { return pRefOutDev; }
    // Wenn ein neuer MapMode am RefDevice gesetzt wird o.ae.
    void                 RefDeviceChanged(); // noch nicht implementiert
    // Default-Schrifthoehe in logischen Einheiten
    void                 SetDefaultFontHeight(sal_uIntPtr nVal);
    sal_uIntPtr                GetDefaultFontHeight() const           { return nDefTextHgt; }
    // Default-Tabulatorweite fuer die EditEngine
    void                 SetDefaultTabulator(sal_uInt16 nVal);
    sal_uInt16               GetDefaultTabulator() const            { return nDefaultTabulator; }

    // Der DefaultStyleSheet wird jedem Zeichenobjekt verbraten das in diesem
    // Model eingefuegt wird und kein StyleSheet gesetzt hat.
    SfxStyleSheet*       GetDefaultStyleSheet() const             { return pDefaultStyleSheet; }
    void                 SetDefaultStyleSheet(SfxStyleSheet* pDefSS) { pDefaultStyleSheet = pDefSS; }

    // #i119287# default StyleSheet for SdrGrafObj and SdrOle2Obj
    SfxStyleSheet* GetDefaultStyleSheetForSdrGrafObjAndSdrOle2Obj() const { return mpDefaultStyleSheetForSdrGrafObjAndSdrOle2Obj; }
    void SetDefaultStyleSheetForSdrGrafObjAndSdrOle2Obj(SfxStyleSheet* pDefSS) { mpDefaultStyleSheetForSdrGrafObjAndSdrOle2Obj = pDefSS; }

    sfx2::LinkManager*      GetLinkManager()                         { return pLinkManager; }
    void                 SetLinkManager( sfx2::LinkManager* pLinkMgr ) { pLinkManager = pLinkMgr; }

    ::comphelper::IEmbeddedHelper*     GetPersist() const               { return m_pEmbeddedHelper; }
    void                 ClearPersist()                                 { m_pEmbeddedHelper = 0; }
    void                 SetPersist( ::comphelper::IEmbeddedHelper *p ) { m_pEmbeddedHelper = p; }

    // Masseinheit fuer die Zeichenkoordinaten.
    // Default ist 1 logische Einheit = 1/100mm (Unit=MAP_100TH_MM, Fract=(1,1)).
    // Beispiele:
    //   MAP_POINT,    Fraction(72,1)    : 1 log Einh = 72 Point   = 1 Inch
    //   MAP_POINT,    Fraction(1,20)    : 1 log Einh = 1/20 Point = 1 Twip
    //   MAP_TWIP,     Fraction(1,1)     : 1 log Einh = 1 Twip
    //   MAP_100TH_MM, Fraction(1,10)    : 1 log Einh = 1/1000mm
    //   MAP_MM,       Fraction(1000,1)  : 1 log Einh = 1000mm     = 1m
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
    const OUString&  GetUIUnitStr() const                       { return aUIUnitStr; }
    int              GetUIUnitKomma() const                     { return nUIUnitKomma; }
    bool             IsUIOnlyKomma() const                      { return bUIOnlyKomma; }

    static void      TakeUnitStr(FieldUnit eUnit, OUString& rStr);
    void             TakeMetricStr(long nVal, OUString& rStr, bool bNoUnitChars = false, sal_Int32 nNumDigits = -1) const;
    void             TakeWinkStr(long nWink, OUString& rStr, bool bNoDegChar = false) const;
    void             TakePercentStr(const Fraction& rVal, OUString& rStr, bool bNoPercentChar = false) const;

    // RecalcPageNums wird idR. nur von der Page gerufen.
    bool         IsPagNumsDirty() const                     { return bPagNumsDirty; };
    bool         IsMPgNumsDirty() const                     { return bMPgNumsDirty; };
    void             RecalcPageNums(bool bMaster);
    // Nach dem Insert gehoert die Page dem SdrModel.
    virtual void     InsertPage(SdrPage* pPage, sal_uInt16 nPos=0xFFFF);
    virtual void     DeletePage(sal_uInt16 nPgNum);
    // Remove bedeutet Eigentumsuebereignung an den Aufrufer (Gegenteil von Insert)
    virtual SdrPage* RemovePage(sal_uInt16 nPgNum);
    virtual void     MovePage(sal_uInt16 nPgNum, sal_uInt16 nNewPos);
    const SdrPage* GetPage(sal_uInt16 nPgNum) const;
    SdrPage* GetPage(sal_uInt16 nPgNum);
    sal_uInt16 GetPageCount() const;
    // #109538#
    virtual void PageListChanged();

    // Masterpages
    virtual void     InsertMasterPage(SdrPage* pPage, sal_uInt16 nPos=0xFFFF);
    virtual void     DeleteMasterPage(sal_uInt16 nPgNum);
    // Remove bedeutet Eigentumsuebereignung an den Aufrufer (Gegenteil von Insert)
    virtual SdrPage* RemoveMasterPage(sal_uInt16 nPgNum);
    virtual void     MoveMasterPage(sal_uInt16 nPgNum, sal_uInt16 nNewPos);
    const SdrPage* GetMasterPage(sal_uInt16 nPgNum) const;
    SdrPage* GetMasterPage(sal_uInt16 nPgNum);
    sal_uInt16 GetMasterPageCount() const;
    // #109538#
    virtual void MasterPageListChanged();

    // Modified-Flag. Wird automatisch gesetzt, wenn an den Pages oder
    // Zeichenobjekten was geaendert wird. Zuruecksetzen muss man es
    // jedoch selbst (z.B. bei Save() ...).
    sal_Bool IsChanged() const { return mbChanged; }
    virtual void SetChanged(sal_Bool bFlg = sal_True);

    // PageNotValid bedeutet, dass das Model lediglich Objekte traegt die zwar
    // auf einer Page verankert sind, die Page aber nicht gueltig ist. Diese
    // Kennzeichnung wird fuers Clipboard/Drag&Drop benoetigt.
    bool            IsPageNotValid() const                     { return bPageNotValid; }
    void            SetPageNotValid(bool bJa = true)           { bPageNotValid=bJa; }

    // Schaltet man dieses Flag auf sal_True, so werden Grafikobjekte
    // portabel gespeichert. Es findet dann beim Speichern ggf.
    // eine implizite Wandlung von Metafiles statt.
    // Default=FALSE. Flag ist nicht persistent.
    bool            IsSavePortable() const                     { return bSavePortable; }
    void            SetSavePortable(bool bJa = true)           { bSavePortable=bJa; }

    // Schaltet man dieses Flag auf sal_True, so werden
    // Pixelobjekte (stark) komprimiert gespeichert.
    // Default=FALSE. Flag ist nicht persistent.
    bool            IsSaveCompressed() const                   { return bSaveCompressed; }
    void            SetSaveCompressed(bool bJa = true)         { bSaveCompressed=bJa; }

    // Schaltet man dieses Flag auf sal_True, so werden
    // Grafikobjekte mit gesetztem Native-Link
    // native gespeichert.
    // Default=FALSE. Flag ist nicht persistent.
    bool            IsSaveNative() const                       { return bSaveNative; }
    void            SetSaveNative(bool bJa = true)             { bSaveNative=bJa; }

    // Schaltet man dieses Flag auf sal_True, so werden die Grafiken
    // von Grafikobjekten:
    // - beim Laden eines Dokuments nicht sofort mitgeladen,
    //   sondern erst wenn sie gebraucht (z.B. angezeigt) werden.
    // - ggf. wieder aus dem Speicher geworfen, falls Sie gerade
    //   nicht benoetigt werden.
    // Damit das funktioniert, muss die virtuelle Methode
    // GetDocumentStream() ueberladen werden.
    // Default=FALSE. Flag ist nicht persistent.
    bool            IsSwapGraphics() const { return bSwapGraphics; }
    void            SetSwapGraphics(bool bJa = true);
    void            SetSwapGraphicsMode(sal_uIntPtr nMode) { nSwapGraphicsMode = nMode; }
    sal_uIntPtr         GetSwapGraphicsMode() const { return nSwapGraphicsMode; }

    bool            IsSaveOLEPreview() const          { return bSaveOLEPreview; }
    void            SetSaveOLEPreview( bool bSet) { bSaveOLEPreview = bSet; }

    // Damit die Bildschirmausgabe von Bitmaps (insbesondere bei gedrehten)
    // etwas schneller wird, werden sie gecachet. Diesen Cache kann man mit
    // diesem Flag ein-/ausschalten. Beim naechsten Paint wird an den Objekten
    // dann ggf. ein Image gemerkt bzw. freigegeben. Wandert ein Bitmapobjekt
    // in's Undo, so wird der Cache fuer dieses Objekt sofort ausgeschaltet
    // (Speicher sparen).
    // Default=Cache eingeschaltet. Flag ist nicht persistent.
    bool            IsBitmapCaching() const                     { return !bNoBitmapCaching; }
    void            SetBitmapCaching(bool bJa = true)           { bNoBitmapCaching=!bJa; }

    // Defaultmaessig (sal_False) kann man Textrahmen ohne Fuellung durch
    // Mausklick selektieren. Nach Aktivierung dieses Flags trifft man sie
    // nur noch in dem Bereich, wo sich auch tatsaechlich Text befindet.
    bool            IsPickThroughTransparentTextFrames() const  { return bTransparentTextFrames; }
    void            SetPickThroughTransparentTextFrames(bool bOn) { bTransparentTextFrames=bOn; }

    // Darf denn das Model ueberhaupt veraendert werden?
    // Wird nur von den Possibility-Methoden der View ausgewerdet.
    // Direkte Manipulationen am Model, ... berueksichtigen dieses Flag nicht.
    // Sollte ueberladen werden und entsprechend des ReadOnly-Status des Files
    // sal_True oder sal_False liefern (Methode wird oeffters gerufen, also ein Flag
    // verwenden!).
    virtual bool IsReadOnly() const;
    virtual void     SetReadOnly(bool bYes);

    // Vermischen zweier SdrModel. Zu beachten sei, dass rSourceModel nicht
    // const ist. Die Pages werden beim einfuegen nicht kopiert, sondern gemoved.
    // rSourceModel ist anschliessend u.U. weitgehend leer.
    // nFirstPageNum,nLastPageNum: Die aus rSourceModel zu uebernehmenden Seiten
    // nDestPos..................: Einfuegeposition
    // bMergeMasterPages.........: sal_True =benoetigte MasterPages werden aus
    //                                   rSourceModel ebenfalls uebernommen
    //                             sal_False=Die MasterPageDescriptoren der Seiten
    //                                   aus rSourceModel werden auf die
    //                                   vorhandenen MasterPages gemappt.
    // bUndo.....................: Fuer das Merging wird eine UndoAction generiert.
    //                             Undo ist nur fuer das ZielModel, nicht fuer
    //                             rSourceModel.
    // bTreadSourceAsConst.......: sal_True=Das SourceModel wird nicht veraendert,.
    //                             d.h die Seiten werden kopiert.
    virtual void Merge(SdrModel& rSourceModel,
               sal_uInt16 nFirstPageNum=0, sal_uInt16 nLastPageNum=0xFFFF,
               sal_uInt16 nDestPos=0xFFFF,
               bool bMergeMasterPages = false, bool bAllMasterPages = false,
               bool bUndo = true, bool bTreadSourceAsConst = false);

    // Ist wie Merge(SourceModel=DestModel,nFirst,nLast,nDest,sal_False,sal_False,bUndo,!bMoveNoCopy);
    void CopyPages(sal_uInt16 nFirstPageNum, sal_uInt16 nLastPageNum,
                   sal_uInt16 nDestPos,
                   bool bUndo = true, bool bMoveNoCopy = false);

    // Mit BegUndo() / EndUndo() ist es moeglich beliebig viele UndoActions
    // beliebig tief zu klammern. Als Kommentar der
    // UndoAction wird der des ersten BegUndo(String) aller Klammerungen
    // verwendet. Der NotifyUndoActionHdl wird in diesem Fall erst beim letzten
    // EndUndo() gerufen. Bei einer leeren Klammerung wird keine UndoAction
    // generiert.
    // Alle direkten Aktionen am SdrModel erzeugen keine UndoActions, die
    // Aktionen an der SdrView dagegen generieren solche.
    void BegUndo();                       // Undo-Klammerung auf
    void BegUndo(const OUString& rComment); // Undo-Klammerung auf
    void BegUndo(const OUString& rComment, const OUString& rObjDescr, SdrRepeatFunc eFunc=SDRREPFUNC_OBJ_NONE); // Undo-Klammerung auf
    void EndUndo();                       // Undo-Klammerung zu
    void AddUndo(SdrUndoAction* pUndo);
    sal_uInt16 GetUndoBracketLevel() const                       { return nUndoLevel; }
    const SdrUndoGroup* GetAktUndoGroup() const              { return pAktUndoGroup; }
    // nur nach dem 1. BegUndo oder vor dem letzten EndUndo:
    void SetUndoComment(const OUString& rComment);
    void SetUndoComment(const OUString& rComment, const OUString& rObjDescr);

    // Das Undo-Management findet nur statt, wenn kein NotifyUndoAction-Handler
    // gesetzt ist.
    // Default ist 16. Minimaler MaxUndoActionCount ist 1!
    void  SetMaxUndoActionCount(sal_uIntPtr nAnz);
    sal_uIntPtr GetMaxUndoActionCount() const { return nMaxUndoCount; }
    void  ClearUndoBuffer();

    bool HasUndoActions() const;
    bool HasRedoActions() const;
    bool Undo();
    bool Redo();
    bool Repeat(SfxRepeatTarget&);

    // Hier kann die Applikation einen Handler setzen, der die auflaufenden
    // UndoActions einsammelt. Der Handler hat folgendes Aussehen:
    //   void NotifyUndoActionHdl(SfxUndoAction* pUndoAction);
    // Beim Aufruf des Handlers findet eine Eigentumsuebereignung statt; die
    // UndoAction gehoert somit dem Handler, nicht mehr dem SdrModel.
    void        SetNotifyUndoActionHdl(const Link& rLink)    { aUndoLink=rLink; }
    const Link& GetNotifyUndoActionHdl() const               { return aUndoLink; }

    /** application can set it's own undo manager, BegUndo, EndUndo and AddUndoAction
        calls are routet to this interface if given */
    void SetSdrUndoManager( SfxUndoManager* pUndoManager );
    SfxUndoManager* GetSdrUndoManager() const;

    /** applications can set their own undo factory to overide creation of
        undo actions. The SdrModel will become owner of the given SdrUndoFactory
        and delete it upon its destruction. */
    void SetSdrUndoFactory( SdrUndoFactory* pUndoFactory );

    /** returns the models undo factory. This must be used to create
        undo actions for this model. */
    SdrUndoFactory& GetSdrUndoFactory() const;

    // Hier kann man einen Handler setzen der beim Streamen mehrfach gerufen
    // wird und ungefaehre Auskunft ueber den Fortschreitungszustand der
    // Funktion gibt. Der Handler muss folgendes Aussehen haben:
    //   void class::IOProgressHdl(const USHORT& nPercent);
    // Der erste Aufruf des Handlers erfolgt grundsaetzlich mit 0, der letzte
    // mit 100. Dazwischen erfolgen maximal 99 Aufrufe mit Werten 1...99.
    // Man kann also durchaus bei 0 den Progressbar Initiallisieren und bei
    // 100 wieder schliessen. Zu beachten sei, dass der Handler auch gerufen
    // wird, wenn die App Draw-Daten im officeweiten Draw-Exchange-Format
    // bereitstellt, denn dies geschieht durch streamen in einen MemoryStream.
    void        SetIOProgressHdl(const Link& rLink)          { aIOProgressLink=rLink; }
    const Link& GetIOProgressHdl() const                     { return aIOProgressLink; }

    // Accessor methods for Palettes, Lists and Tabeles
    // FIXME: this badly needs re-factoring ...
    XPropertyListRef GetPropertyList( XPropertyListType t ) const { return maProperties[ t ]; }
    void             SetPropertyList( XPropertyListRef p ) { maProperties[ p->Type() ] = p; }

    // friendlier helpers
    XDashListRef     GetDashList() const     { return GetPropertyList( XDASH_LIST )->AsDashList(); }
    XHatchListRef    GetHatchList() const    { return GetPropertyList( XHATCH_LIST )->AsHatchList(); }
    XColorListRef    GetColorList() const    { return GetPropertyList( XCOLOR_LIST )->AsColorList(); }
    XBitmapListRef   GetBitmapList() const   { return GetPropertyList( XBITMAP_LIST )->AsBitmapList(); }
    XLineEndListRef  GetLineEndList() const  { return GetPropertyList( XLINE_END_LIST )->AsLineEndList(); }
    XGradientListRef GetGradientList() const { return GetPropertyList( XGRADIENT_LIST )->AsGradientList(); }

    // Der StyleSheetPool wird der DrawingEngine nur bekanntgemacht.
    // Zu loeschen hat ihn schliesslich der, der ihn auch konstruiert hat.
    SfxStyleSheetBasePool* GetStyleSheetPool() const         { return mxStyleSheetPool.get(); }
    void SetStyleSheetPool(SfxStyleSheetBasePool* pPool)     { mxStyleSheetPool=pPool; }

    void    SetStarDrawPreviewMode(sal_Bool bPreview);
    sal_Bool    IsStarDrawPreviewMode() { return bStarDrawPreviewMode; }

    bool GetDisableTextEditUsesCommonUndoManager() const { return mbDisableTextEditUsesCommonUndoManager; }
    void SetDisableTextEditUsesCommonUndoManager(bool bNew) { mbDisableTextEditUsesCommonUndoManager = bNew; }

    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > getUnoModel();
    void setUnoModel( ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > xModel );

    // these functions are used by the api to disable repaints during a
    // set of api calls.
    bool isLocked() const { return mbModelLocked; }
    void setLock( bool bLock );

    void            SetForbiddenCharsTable( rtl::Reference<SvxForbiddenCharactersTable> xForbiddenChars );
    rtl::Reference<SvxForbiddenCharactersTable> GetForbiddenCharsTable() const;

    void SetCharCompressType( sal_uInt16 nType );
    sal_uInt16 GetCharCompressType() const { return mnCharCompressType; }

    void SetKernAsianPunctuation( sal_Bool bEnabled );
    sal_Bool IsKernAsianPunctuation() const { return (sal_Bool)mbKernAsianPunctuation; }

    void SetAddExtLeading( sal_Bool bEnabled );
    sal_Bool IsAddExtLeading() const { return (sal_Bool)mbAddExtLeading; }

    void ReformatAllTextObjects();

    SdrOutliner* createOutliner( sal_uInt16 nOutlinerMode );
    void disposeOutliner( SdrOutliner* pOutliner );

    sal_Bool IsWriter() const { return !bMyPool; }

    /** returns the numbering type that is used to format page fields in drawing shapes */
    virtual SvxNumType GetPageNumType() const;

    /** copies the items from the source set to the destination set. Both sets must have
        same ranges but can have different pools. If pNewModel is optional. If it is null,
        this model is used. */

    void MigrateItemSet( const SfxItemSet* pSourceSet, SfxItemSet* pDestSet, SdrModel* pNewModel );

    bool IsInDestruction() const;

    static const ::com::sun::star::uno::Sequence< sal_Int8 >& getUnoTunnelImplementationId();

    /** enables (true) or disables (false) recording of undo actions
        If undo actions are added while undo is disabled, they are deleted.
        Disabling undo does not clear the current undo buffer! */
    void EnableUndo( bool bEnable );

    /** returns true if undo is currently enabled
        This returns false if undo was disabled using EnableUndo( false ) and
        also during the runtime of the Undo() and Redo() methods. */
    bool IsUndoEnabled() const;
};

typedef tools::WeakReference< SdrModel > SdrModelWeakRef;

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //_SVDMODEL_HXX

/* /////////////////////////////////////////////////////////////////////////////////////////////////
            +-----------+
            | SdrModel  |
            +--+------+-+
               |      +-----------+
          +----+-----+            |
          |   ...    |            |
     +----+---+ +----+---+  +-----+--------+
     |SdrPage | |SdrPage |  |SdrLayerAdmin |
     +---+----+ +-+--+--++  +---+-------+--+
         |        |  |  |       |       +-------------------+
    +----+----+           +-----+-----+             +-------+-------+
    |   ...   |           |    ...    |             |      ...      |
+---+---+ +---+---+  +----+----+ +----+----+  +-----+------+ +------+-----+
|SdrObj | |SdrObj |  |SdrLayer | |SdrLayer |  |SdrLayerSet | |SdrLayerSet |
+-------+ +-------+  +---------+ +---------+  +------------+ +------------+
This class: SdrModel is the head of the data modells for the StarView Drawing Engine.

///////////////////////////////////////////////////////////////////////////////////////////////// */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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

#ifndef _SVDMODEL_HXX
#define _SVDMODEL_HXX

#include <com/sun/star/uno/Sequence.hxx>
#include <cppuhelper/weakref.hxx>
#include <sot/storage.hxx>
#include <tools/link.hxx>
#include <tools/contnr.hxx>
#include <tools/weakbase.hxx>
#include <vcl/mapmod.hxx>
#include <svl/brdcst.hxx>
#include <tools/string.hxx>
#include <tools/datetime.hxx>
#include <svl/hint.hxx>
#include <svl/style.hxx>
#include <svx/pageitem.hxx>
#include <vcl/field.hxx>
#include <boost/shared_ptr.hpp>
#include <svx/svdtypes.hxx> // fuer enum RepeatFuncts
#include <vcl/field.hxx>
#include "svx/svxdllapi.h"
#include <vos/ref.hxx>
#include <set>
#include <svx/xtable.hxx>

////////////////////////////////////////////////////////////////////////////////////////////////////
// predefines

#if defined(UNX) || defined(WNT)
#define DEGREE_CHAR ((sal_Unicode)176)   /* 0xB0 = Ansi */
#endif

#if defined(OS2)
#define DEGREE_CHAR ((sal_Unicode)248)   /* 0xF8 = IBM PC (Erw. ASCII) */
#endif

#ifndef DEGREE_CHAR
#error unbekannte Plattrorm
#endif

class OutputDevice;
class SdrOutliner;
class SdrLayerAdmin;
class SdrObject;
class SdrPage;
class SdrTextObj;
class SdrUndoAction;
class SdrUndoGroup;
class SfxItemPool;
class SfxItemSet;
class SfxRepeatTarget;
class SfxUndoAction;
class SfxUndoManager;
class SvxForbiddenCharactersTable;
class SvNumberFormatter;
class SdrOutlinerCache;
class SdrUndoFactory;
class SfxStyleSheet;
namespace comphelper { class IEmbeddedHelper; }
namespace sfx2 { class LinkManager; }
class SdrView;

//IAccessibility2 Implementation 2009-----
class ImageMap;
//-----IAccessibility2 Implementation 2009

namespace sfx2{
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
    HINT_LAYERCHG,        // layer definition changed
    HINT_LAYERORDERCHG,   // layer order changed
    HINT_PAGEORDERCHG,    // order of pages changed (master and/or normal)
    HINT_OBJCHG_MOVE,     // drawobject changed (translated)
    HINT_OBJCHG_RESIZE,   // drawobject changed (scaled)
    HINT_OBJCHG_ATTR,     // drawobject changed (attributes)
    HINT_OBJINSERTED,     // drawobject inserted
    HINT_OBJREMOVED,      // drawobject  removed
    HINT_MODELCLEARED,    // complete model cleared
    HINT_REFDEVICECHG,    // RefDevice changed
    HINT_DEFAULTTABCHG,   // Default Tab changed
    HINT_DEFFONTHGTCHG,   // Default FontHeight changed
    HINT_SWITCHTOPAGE,    // UNDO/REDO at an object evtl. on another page

    HINT_BEGEDIT,         // Is called after the object has entered text edit mode
    HINT_ENDEDIT,         // Is called after the object has left text edit mode

    HINT_SDROBJECTDYING,  // SdrObject is destructed
    HINT_SDRPAGEDYING     // SdrPage is destructed
};

////////////////////////////////////////////////////////////////////////////////////////////////////

class SVX_DLLPUBLIC SdrBaseHint : public SfxHint
{
private:
    SdrHintKind                             meSdrHint;
    const SdrPage*                          mpSdrPage;
    const SdrObject*                        mpSdrObject;

public:
    // constructor
    SdrBaseHint(
        SdrHintKind eSdrHintKind);
    SdrBaseHint(
        const SdrPage& rSdrPage,
        SdrHintKind eSdrHintKind = HINT_PAGEORDERCHG);
    SdrBaseHint(
        const SdrObject& rSdrObject,
        SdrHintKind eSdrHintKind = HINT_OBJCHG_RESIZE);

    // data read access
    SdrHintKind GetSdrHintKind() const { return meSdrHint; }
    const SdrPage* GetSdrHintPage() const { return mpSdrPage; }
    const SdrObject* GetSdrHintObject() const { return mpSdrObject; }
};

////////////////////////////////////////////////////////////////////////////////////////////////////

struct SdrDocumentStreamInfo
{
public:
    String          maUserData;
    com::sun::star::uno::Reference < com::sun::star::embed::XStorage > mxStorageRef;

    /// bitfield
    bool            mbDeleteAfterUse : 1;
    bool            mbDummy1 : 1;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

class SVX_DLLPUBLIC SdrModel
:   private boost::noncopyable,
    public SfxBroadcaster,
    public tools::WeakBase< SdrModel >
{
private:
    // this is a weak reference to a possible living api wrapper for this model
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > mxUnoModel;

    SVX_DLLPRIVATE bool operator==(const SdrModel& rCmpModel) const;
    SVX_DLLPRIVATE void ImpPostUndoAction(SdrUndoAction* pUndo);
    SVX_DLLPRIVATE void ImpSetUIUnit();
    SVX_DLLPRIVATE void ImpSetOutlinerDefaults( SdrOutliner* pOutliner, bool bInit = false );
    SVX_DLLPRIVATE void ImpReformatAllEdgeObjects();    // #103122#

    void EnsureValidPageNumbers(bool bMaster);

protected:
    // types for page and undo vectors
    typedef ::std::vector< SdrPage* > SdrPageContainerType;
    typedef ::std::vector< SfxUndoAction* > SfxUndoActionContainerType;

    // page lists
    SdrPageContainerType            maMasterPageVector;
    SdrPageContainerType            maPageVector;

    // Layers and ItemPool
    SdrLayerAdmin*                  mpModelLayerAdmin;
    SfxItemPool*                    mpItemPool;

    // undo
    Link                            maUndoLink;  // Link fuer einen NotifyUndo-Handler
    SfxUndoActionContainerType*     mpUndoStack;
    SfxUndoActionContainerType*     mpRedoStack;
    SdrUndoGroup*                   mpCurrentUndoGroup;  // Fuer mehrstufige
    sal_uInt32                      mnUndoLevel;     // Undo-Klammerung
    sal_uInt32                      mnMaxUndoCount;
    SfxUndoManager*                 mpUndoManager;
    SdrUndoFactory*                 mpUndoFactory;

    // object scales and UI units
    basegfx::B2DVector              maMaxObjectScale; // z.B. fuer Autogrowing Text
    Fraction                        maExchangeObjectScale;   // Beschreibung der Koordinateneinheiten fuer ClipBoard, Drag&Drop, ...
    MapUnit                         meExchangeObjectUnit;   // see above
    FieldUnit                       meUIUnit;      // Masseinheit, Masstab (z.B. 1/1000) fuer die UI (Statuszeile) wird von ImpSetUIUnit() gesetzt
    Fraction                        maUIScale;     // see above

    // values derived from meUIUnit and maUIScale, buffered
    String                          maUIUnitString;   // see above
    Fraction                        maUIUnitScale;  // see above
    sal_Int32                       mnUIUnitKomma; // see above

    // outliner
    SdrOutliner*                    mpDrawOutliner;  // ein Outliner zur Textausgabe
    SdrOutlinerCache*               mpOutlinerCache;

    // diverse data and helpers
    rtl::Reference< SfxStyleSheetBasePool > mxStyleSheetPool;
    SfxStyleSheet*                  mpDefaultStyleSheet;
    SfxStyleSheet*                  mpDefaultStyleSheetForSdrGrafObjAndSdrOle2Obj; // #119287#
    comphelper::IEmbeddedHelper*    m_pEmbeddedHelper; // helper for embedded objects to get rid of the SfxObjectShell
    OutputDevice*                   mpReferenceOutputDevice;     // ReferenzDevice fuer die EditEngine
    sfx2::LinkManager*              mpLinkManager;   // LinkManager
    SvxForbiddenCharactersTable*    mpForbiddenCharactersTable;
    SvNumberFormatter*              mpNumberFormatter;
    sal_uInt32                      mnDefaultFontHeight;    // Default Texthoehe in logischen Einheiten
    sal_uInt32                      mnSwapGraphicsMode;
    sal_uInt16                      mnDefaultTabulator;
    sal_uInt16                      mnCharCompressType;

    // lists for colors, dashes, lineends, hatches, gradients and bitmaps for this model
    String                  maTablePath;
    XColorListSharedPtr     maColorTable;
    XDashListSharedPtr      maDashList;
    XLineEndListSharedPtr   maLineEndList;
    XHatchListSharedPtr     maHatchList;
    XGradientListSharedPtr  maGradientList;
    XBitmapListSharedPtr    maBitmapList;

    sal_uInt32                      mnHandoutPageCount;

    /// bitfield
    bool                            mbDeletePool : 1;        // zum Aufraeumen von pMyPool ab 303a
    bool                            mbUndoEnabled : 1;  // If false no undo is recorded or we are during the execution of an undo action
    bool                            mbChanged : 1;
    bool                            mbReadOnly : 1;
    bool                            mbPickThroughTransparentTextFrames : 1;
    bool                            mbSwapGraphics : 1;
    bool                            mbStarDrawPreviewMode : 1;
    bool                            mbModelLocked : 1;
    bool                            mbKernAsianPunctuation : 1;
    bool                            mbAddExtLeading : 1;
    bool                            mbInDestruction : 1;
    bool                            mbDisableTextEditUsesCommonUndoManager : 1;

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > createUnoModel();

public:
    SdrModel(const String& rPath = String(), SfxItemPool* pPool = 0, ::comphelper::IEmbeddedHelper* pPers = 0);
    virtual ~SdrModel();
    void ClearModel(bool bCalledFromDestructor);

    sal_uInt32 getHandoutPageCount() const { return mnHandoutPageCount; }
    void setHandoutPageCount( sal_uInt32 nHandoutPageCount ) { mnHandoutPageCount = nHandoutPageCount; }

    const SvNumberFormatter& GetNumberFormatter() const;

    // Muss z.B. ueberladen werden, um das Swappen/LoadOnDemand von Grafiken
    // zu ermoeglichen. Wird rbDeleteAfterUse auf true gesetzt, so wird
    // die SvStream-Instanz vom Aufrufer nach Gebrauch destruiert.
    // Wenn diese Methode NULL liefert, wird zum Swappen eine temporaere
    // Datei angelegt.
    // Geliefert werden muss der Stream, aus dem das Model geladen wurde
    // bzw. in den es zuletzt gespeichert wurde.
    virtual SvStream* GetDocumentStream( SdrDocumentStreamInfo& rStreamInfo ) const;

    // Die Vorlagenattribute der Zeichenobjekte in harte Attribute verwandeln.
    void BurnInStyleSheetAttributes();

    // Wer sich von SdrPage ableitet muss sich auch von SdrModel ableiten
    // und diese beiden VM AllocPage() und AllocModel() ueberladen...
    virtual SdrPage* AllocPage(bool bMasterPage);
    virtual SdrModel* AllocModel() const;

    // Aenderungen an den Layern setzen das Modified-Flag und broadcasten am Model!
    const SdrLayerAdmin& GetModelLayerAdmin() const { return *mpModelLayerAdmin; }
    SdrLayerAdmin& GetModelLayerAdmin() { return *mpModelLayerAdmin; }

    const SfxItemPool& GetItemPool() const { return *mpItemPool; }
    SfxItemPool& GetItemPool() { return *mpItemPool; }

    SdrOutliner& GetDrawOutliner(const SdrTextObj* pObj = 0) const;

    /** returns a new created and non shared outliner.
        The outliner will not get updated when the SdrModel is changed.
    */
    boost::shared_ptr< SdrOutliner > CreateDrawOutliner(const SdrTextObj* pObj = 0);

    const SdrTextObj* GetFormattingTextObj() const;

    // Die TextDefaults (Font,Hoehe,Farbe) in ein Set putten
    static void SetTextDefaults(SfxItemPool* pItemPool, sal_uInt32 nDefaultFontHeight);

    // ReferenzDevice fuer die EditEngine
    void SetReferenceDevice(OutputDevice* pDev);
    OutputDevice* GetReferenceDevice() const { return mpReferenceOutputDevice; }

    // Default-Schrifthoehe in logischen Einheiten
    void SetDefaultFontHeight(sal_uInt32 nVal);
    sal_uInt32 GetDefaultFontHeight() const { return mnDefaultFontHeight; }

    // Default-Tabulatorweite fuer die EditEngine
    void SetDefaultTabulator(sal_uInt16 nVal);
    sal_uInt16 GetDefaultTabulator() const { return mnDefaultTabulator; }

    sfx2::LinkManager* GetLinkManager() { return mpLinkManager; }
    void SetLinkManager(sfx2::LinkManager* pLinkMgr) { if(mpLinkManager != pLinkMgr) mpLinkManager = pLinkMgr; }

    ::comphelper::IEmbeddedHelper* GetPersist() const { return m_pEmbeddedHelper; }
    void ClearPersist() { m_pEmbeddedHelper = 0; }
    void SetPersist( ::comphelper::IEmbeddedHelper *p ) { m_pEmbeddedHelper = p; }

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
    MapUnit GetExchangeObjectUnit() const { return meExchangeObjectUnit; }
    void SetExchangeObjectUnit(MapUnit eMap);
    const Fraction&  GetExchangeObjectScale() const { return maExchangeObjectScale; }
    void SetExchangeObjectScale(const Fraction& rFrac);

    // Maximale Groesse z.B. fuer Autogrowing-Texte
    const basegfx::B2DVector& GetMaxObjectScale() const { return maMaxObjectScale; }
    void SetMaxObjectScale(const basegfx::B2DVector& rSiz) { if(!maMaxObjectScale.equal(rSiz)) maMaxObjectScale = rSiz; }

    // Damit die View! in der Statuszeile vernuenftige Zahlen anzeigen kann:
    // Default ist mm.
    void SetUIUnit(FieldUnit eUnit);
    FieldUnit GetUIUnit() const { return meUIUnit; }

    // Der Masstab der Zeichnung. Default 1/1.
    void SetUIScale(const Fraction& rScale);
    const Fraction& GetUIScale() const { return maUIScale; }

    static void TakeUnitStr(FieldUnit eUnit, String& rStr);
    void TakeMetricStr(double fVal, String& rStr, bool bNoUnitChars = false, sal_Int32 nNumDigits = -1) const;
    void TakeWinkStr(long nWink, String& rStr, bool bNoDegChar = false) const;
    void TakePercentStr(const Fraction& rVal, String& rStr, bool bNoPercentChar = false) const;

    // Nach dem Insert gehoert die Page dem SdrModel.
    virtual void InsertPage(SdrPage* pPage, sal_uInt32 nPos = 0xffffffff);
    virtual void DeletePage(sal_uInt32 nPgNum);
    virtual SdrPage* RemovePage(sal_uInt32 nPgNum);
    virtual void MovePage(sal_uInt32 nPgNum, sal_uInt32 nNewPos);
    SdrPage* GetPage(sal_uInt32 nPgNum) const;
    sal_uInt32 GetPageCount() const { return maPageVector.size(); }

    // Masterpages
    virtual void InsertMasterPage(SdrPage* pPage, sal_uInt32 nPos = 0xffffffff);
    virtual void DeleteMasterPage(sal_uInt32 nPgNum);
    virtual SdrPage* RemoveMasterPage(sal_uInt32 nPgNum);
    virtual void MoveMasterPage(sal_uInt32 nPgNum, sal_uInt32 nNewPos);
    SdrPage* GetMasterPage(sal_uInt32 nPgNum) const;
    sal_uInt32 GetMasterPageCount() const { return maMasterPageVector.size(); }

    // Modified-Flag. Wird automatisch gesetzt, wenn an den Pages oder
    // Zeichenobjekten was geaendert wird. Zuruecksetzen muss man es
    // jedoch selbst (z.B. bei Save() ...).
    bool IsChanged() const { return mbChanged; }
    virtual void SetChanged(bool bFlg = true);

    // Schaltet man dieses Flag auf true, so werden die Grafiken
    // von Grafikobjekten:
    // - beim Laden eines Dokuments nicht sofort mitgeladen,
    //   sondern erst wenn sie gebraucht (z.B. angezeigt) werden.
    // - ggf. wieder aus dem Speicher geworfen, falls Sie gerade
    //   nicht benoetigt werden.
    // Damit das funktioniert, muss die virtuelle Methode
    // GetDocumentStream() ueberladen werden.
    // Default=false. Flag ist nicht persistent.
    bool IsSwapGraphics() const { return mbSwapGraphics; }
    void SetSwapGraphics(bool bSwap = true) { if(mbSwapGraphics != bSwap) mbSwapGraphics = bSwap; }
    void SetSwapGraphicsMode(sal_uInt32 nMode) { if(mnSwapGraphicsMode != nMode) mnSwapGraphicsMode = nMode; }
    sal_uInt32 GetSwapGraphicsMode() const { return mnSwapGraphicsMode; }

    // Defaultmaessig (false) kann man Textrahmen ohne Fuellung durch
    // Mausklick selektieren. Nach Aktivierung dieses Flags trifft man sie
    // nur noch in dem Bereich, wo sich auch tatsaechlich Text befindet.
    bool IsPickThroughTransparentTextFrames() const { return mbPickThroughTransparentTextFrames; }
    void SetPickThroughTransparentTextFrames(bool bOn) { if(mbPickThroughTransparentTextFrames != bOn) mbPickThroughTransparentTextFrames = bOn; }

    // is the model changeable at all?
    virtual bool IsReadOnly() const;
    virtual void SetReadOnly(bool bYes);

    // Vermischen zweier SdrModel. Zu beachten sei, dass rSourceModel nicht
    // const ist. Die Pages werden beim einfuegen nicht kopiert, sondern gemoved.
    // rSourceModel ist anschliessend u.U. weitgehend leer.
    // nFirstPageNum,nLastPageNum: Die aus rSourceModel zu uebernehmenden Seiten
    // nDestPos..................: Einfuegeposition
    // bMergeMasterPages.........: true =benoetigte MasterPages werden aus
    //                                   rSourceModel ebenfalls uebernommen
    //                             false=Die MasterPageDescriptoren der Seiten
    //                                   aus rSourceModel werden auf die
    //                                   vorhandenen MasterPages gemappt.
    // bUndo.....................: Fuer das Merging wird eine UndoAction generiert.
    //                             Undo ist nur fuer das ZielModel, nicht fuer
    //                             rSourceModel.
    // bTreadSourceAsConst.......: true=Das SourceModel wird nicht veraendert,.
    //                             d.h die Seiten werden kopiert.
    virtual void Merge(
        SdrModel& rSourceModel,
        sal_uInt32 nFirstPageNum = 0, sal_uInt32 nLastPageNum = 0xffffffff,
        sal_uInt32 nDestPos = 0xffffffff,
        bool bMergeMasterPages = false, bool bAllMasterPages = false,
        bool bUndo = true, bool bTreadSourceAsConst = false);

    // Ist wie Merge(SourceModel=DestModel,nFirst,nLast,nDest,false,false,bUndo,!bMoveNoCopy);
    void CopyPages(
        sal_uInt32 nFirstPageNum, sal_uInt32 nLastPageNum,
        sal_uInt32 nDestPos,
        bool bUndo = true, bool bMoveNoCopy = false);

    // Mit BegUndo() / EndUndo() ist es moeglich beliebig viele UndoActions
    // beliebig tief zu klammern. Als Kommentar der
    // UndoAction wird der des ersten BegUndo(String) aller Klammerungen
    // verwendet. Der NotifyUndoActionHdl wird in diesem Fall erst beim letzten
    // EndUndo() gerufen. Bei einer leeren Klammerung wird keine UndoAction
    // generiert.
    // Alle direkten Aktionen am SdrModel erzeugen keine UndoActions, die
    // Aktionen an der SdrView dagegen generieren solche.
    void BegUndo(); // Undo-Klammerung auf
    void BegUndo(const String& rComment); // Undo-Klammerung auf
    void BegUndo(const String& rComment, const String& rObjDescr, SdrRepeatFunc eFunc = SDRREPFUNC_OBJ_NONE); // Undo-Klammerung auf
    void BegUndo(SdrUndoGroup* pUndoGrp); // Undo-Klammerung auf
    void EndUndo(); // Undo-Klammerung zu
    void AddUndo(SdrUndoAction* pUndo);
    bool IsLastEndUndo() const { return (1 == mnUndoLevel); }

    // nur nach dem 1. BegUndo oder vor dem letzten EndUndo:
    void SetUndoComment(const String& rComment);
    void SetUndoComment(const String& rComment, const String& rObjDescr);

    // Das Undo-Managment findet nur statt, wenn kein NotifyUndoAction-Handler
    // gesetzt ist.
    // Default ist 16. Minimaler MaxUndoActionCount ist 1!
    void  SetMaxUndoActionCount(sal_uInt32 nAnz);
    sal_uInt32 GetMaxUndoActionCount() const { return mnMaxUndoCount; }
    void  ClearUndoBuffer();

    // UndoAction(0) ist die aktuelle (also die zuletzt eingegangene)
    sal_uInt32 GetUndoActionCount() const { return mpUndoStack ? mpUndoStack->size() : 0; }
    const SfxUndoAction* GetUndoAction(sal_uInt32 nNum) const;

    // RedoAction(0) ist die aktuelle (also die des letzten Undo)
    sal_uInt32 GetRedoActionCount() const { return mpRedoStack ? mpRedoStack->size() : 0; }
    const SfxUndoAction* GetRedoAction(sal_uInt32 nNum) const;

    bool Undo();
    bool Redo();
    bool Repeat(SfxRepeatTarget&);

    // Hier kann die Applikation einen Handler setzen, der die auflaufenden
    // UndoActions einsammelt. Der Handler hat folgendes Aussehen:
    //   void __EXPORT NotifyUndoActionHdl(SfxUndoAction* pUndoAction);
    // Beim Aufruf des Handlers findet eine Eigentumsuebereignung statt; die
    // UndoAction gehoert somit dem Handler, nicht mehr dem SdrModel.
    void SetNotifyUndoActionHdl(const Link& rLink) { maUndoLink = rLink; }
    const Link& GetNotifyUndoActionHdl() const { return maUndoLink; }

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

    // Zugriffsmethoden fuer Paletten, Listen und Tabellen
    void SetColorTableAtSdrModel(XColorListSharedPtr aTable);
    XColorListSharedPtr GetColorTableFromSdrModel() const;

    void SetDashListAtSdrModel(XDashListSharedPtr aList);
    XDashListSharedPtr GetDashListFromSdrModel() const;

    void SetLineEndListAtSdrModel(XLineEndListSharedPtr aList);
    XLineEndListSharedPtr GetLineEndListFromSdrModel() const;

    void SetHatchListAtSdrModel(XHatchListSharedPtr aList);
    XHatchListSharedPtr GetHatchListFromSdrModel() const;

    void SetGradientListAtSdrModel(XGradientListSharedPtr aList);
    XGradientListSharedPtr GetGradientListFromSdrModel() const;

    void SetBitmapListAtSdrModel(XBitmapListSharedPtr aList);
    XBitmapListSharedPtr GetBitmapListFromSdrModel() const;

    // Der StyleSheetPool wird der DrawingEngine nur bekanntgemacht.
    // Zu loeschen hat ihn schliesslich der, der ihn auch konstruiert hat.
    SfxStyleSheetBasePool* GetStyleSheetPool() const { return mxStyleSheetPool.get(); }
    void SetStyleSheetPool(SfxStyleSheetBasePool* pPool) { if(mxStyleSheetPool != pPool) mxStyleSheetPool = pPool; }

    // Der DefaultStyleSheet wird jedem Zeichenobjekt verbraten das in diesem
    // Model eingefuegt wird und kein StyleSheet gesetzt hat.
    SfxStyleSheet* GetDefaultStyleSheet() const { return mpDefaultStyleSheet; }
    void SetDefaultStyleSheet(SfxStyleSheet* pDefSS) { if(mpDefaultStyleSheet != pDefSS) mpDefaultStyleSheet = pDefSS; }

    // #119287# default StyleSheet for SdrGrafObj and SdrOle2Obj
    SfxStyleSheet* GetDefaultStyleSheetForSdrGrafObjAndSdrOle2Obj() const { return mpDefaultStyleSheetForSdrGrafObjAndSdrOle2Obj; }
    void SetDefaultStyleSheetForSdrGrafObjAndSdrOle2Obj(SfxStyleSheet* pDefSS) { mpDefaultStyleSheetForSdrGrafObjAndSdrOle2Obj = pDefSS; }

    void SetStarDrawPreviewMode(bool bPreview);
    bool IsStarDrawPreviewMode() { return mbStarDrawPreviewMode; }

    bool GetDisableTextEditUsesCommonUndoManager() const { return mbDisableTextEditUsesCommonUndoManager; }
    void SetDisableTextEditUsesCommonUndoManager(bool bNew) { mbDisableTextEditUsesCommonUndoManager = bNew; }

    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > getUnoModel();
    void setUnoModel( ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > xModel );

    // these functions are used by the api to disable repaints during a
    // set of api calls.
    bool isLocked() const { return mbModelLocked; }
    void setLock(bool bLock);

    void SetForbiddenCharsTable( vos::ORef<SvxForbiddenCharactersTable> xForbiddenChars );
    vos::ORef<SvxForbiddenCharactersTable>  GetForbiddenCharsTable() const;

    void SetCharCompressType( sal_uInt16 nType );
    sal_uInt16 GetCharCompressType() const { return mnCharCompressType; }

    void SetKernAsianPunctuation(bool bEnabled);
    bool IsKernAsianPunctuation() const { return mbKernAsianPunctuation; }

    void SetAddExtLeading(bool bEnabled);
    bool IsAddExtLeading() const { return mbAddExtLeading; }

    void ReformatAllTextObjects();

    SdrOutliner* createOutliner( sal_uInt16 nOutlinerMode );
    void disposeOutliner( SdrOutliner* pOutliner );

    /** returns the numbering type that is used to format page fields in drawing shapes */
    virtual SvxNumType GetPageNumType() const;

    bool IsInDestruction() const { return mbInDestruction; }

    static const ::com::sun::star::uno::Sequence< sal_Int8 >& getUnoTunnelImplementationId();

    //IAccessibility2 Implementation 2009-----
    virtual ImageMap* GetImageMapForObject(SdrObject*){return NULL;};
    virtual sal_Int32 GetHyperlinkCount(SdrObject*){return 0;}
    //-----IAccessibility2 Implementation 2009

    /** enables (true) or disables (false) recording of undo actions
        If undo actions are added while undo is disabled, they are deleted.
        Disabling undo does not clear the current undo buffer! */
    void EnableUndo( bool bEnable );

    /** returns true if undo is currently enabled
        This returns false if undo was disabled using EnableUndo( false ) and
        also during the runtime of the Undo() and Redo() methods. */
    bool IsUndoEnabled() const;

    // old hack, not easily to remove. Formally used mbMyPool, changed to virtual bool,
    // overloaded in SwDrawDocument
    virtual bool IsWriter() const;

    /// get all SdrViews registered at this SdrModel. Used for update purposes which
    /// could/should not be needed but be automated. For the moment, offer this to
    /// allow removal of SdrViewIter
    ::std::set< SdrView* > getSdrViews() const;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //_SVDMODEL_HXX

////////////////////////////////////////////////////////////////////////////////////////////////////
// eof

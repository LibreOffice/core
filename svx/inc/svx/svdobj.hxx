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

#ifndef _SVDOBJ_HXX
#define _SVDOBJ_HXX

#include <memory>
#include <cppuhelper/weakref.hxx>
#include <vcl/mapmod.hxx>
#include <tools/weakbase.hxx>
#include <svl/lstner.hxx>
#include <vcl/timer.hxx>
#include <svx/svdsob.hxx>
#include <svx/svdtypes.hxx>
#include <svx/xdash.hxx>
#include <svx/xpoly.hxx>
#include <svx/xenum.hxx>
#include <vcl/bitmapex.hxx>
#include "svx/svxdllapi.h"
#include "svx/shapeproperty.hxx"
#include <svx/svdmodel.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <svx/svdpool.hxx>
#include <boost/utility.hpp>
#include <svl/poolitem.hxx>
#include <svx/gluepoint.hxx>

//////////////////////////////////////////////////////////////////////////////
// predefines

class SfxBroadcaster;
class Pointer;
class AutoTimer;
class OutlinerParaObject;
class SdrOutliner;
class SdrDragStat;
class SdrHdl;
class SdrHdlList;
class SfxItemPool;
class SdrModel;
class SdrObjList;
class SdrObject;
class SdrPage;
class SdrView;
class SfxItemSet;
class SfxSetItem;
class SfxStyleSheet;
class SfxUndoAction;
class XFillAttrSetItem;
class XLineAttrSetItem;
class SfxItemPool;
class PolyPolygon;
class SfxPoolItem;
class SdrDragView;
namespace sdr { namespace properties { class BaseProperties; }}
namespace sdr { namespace contact { class ViewContact; }}
namespace svx { class PropertyChangeNotifier; }
class SvxShape;
class SdrObject;
namespace sdr { namespace gluepoint { class GluePointProvider; }}
class SdrEdgeObj;

//////////////////////////////////////////////////////////////////////////////
// defines

enum SdrObjKind
{
    OBJ_NONE = 0   ,  //            Abstraktes Objekt (SdrObject)
    OBJ_GRUP       ,  //            Objektgruppe
//  OBJ_LINE       ,  // OBJ_POLY   Strecke
    OBJ_RECT       ,  //            Rechteck ww. mit runden Ecken
    OBJ_CIRC       ,  //            Kreis, Ellipse
//  OBJ_SECT       ,  // OBJ_CIRC   Kreissektor
//  OBJ_CARC       ,  // OBJ_CIRC   Kreisbogen
//  OBJ_CCUT       ,  // OBJ_CIRC   Kreisabschnitt
    OBJ_POLY       ,  //            Polygon, PolyPolygon
//  OBJ_PLIN       ,  // OBJ_POLY   PolyLine
//  OBJ_PATHLINE   ,  // OBJ_POLY   Offene Bezierkurve
//  OBJ_PATHFILL   ,  // OBJ_POLY   Geschlossene Bezierkurve
//  OBJ_FREELINE   ,  // OBJ_POLY   Offene Freihandlinie
//  OBJ_FREEFILL   ,  // OBJ_POLY   Geschlossene Freihandlinie
    OBJ_TEXT       ,  //            Textobjekt
    OBJ_TITLETEXT  ,  //            Titeltext. Spezial-Textobjekt fuer StarDraw
    OBJ_OUTLINETEXT,  //            OutlineText. Spezial-Textobjekt fuer StarDraw
    OBJ_GRAF       ,  //            Fremdgrafik - (StarView Graphic)
    OBJ_OLE2       ,  //            OLE-Objekt
    OBJ_EDGE       ,  //            Verbindungsobjekt fuer Konnektoren
    OBJ_CAPTION    ,  //            Legendenobjekt
//  OBJ_PATHPOLY   ,  // Needed?    Polygon/PolyPolygon dargestellt durch SdrPathObj
//  OBJ_PATHPLIN   ,  // Needed?    Polyline dargestellt durch SdrPathObj
    OBJ_PAGE       ,  //            Objekt, das eine SdrPage darstellt
    OBJ_MEASURE    ,  //            Bemassungsobjekt
    OBJ_FRAME      ,  //            staendig aktives OLE (PlugIn-Frame oder sowas)
    OBJ_UNO        ,  //            Universal Network Object im SvDraw-Obj eingepackt
    OBJ_CUSTOMSHAPE,  //            CustomShape
    OBJ_MEDIA      ,  //            Media shape
    OBJ_TABLE      ,  //            Table
    OBJ_MAXI
};

//////////////////////////////////////////////////////////////////////////////
// broadcas helper class

class SVX_DLLPUBLIC SdrObjectChangeBroadcaster : public SdrBaseHint
{
public:
    SdrObjectChangeBroadcaster(
        const SdrObject& rSdrObject,
        SdrHintKind eSdrHintKind = HINT_OBJCHG_RESIZE);
    ~SdrObjectChangeBroadcaster();
};

//////////////////////////////////////////////////////////////////////////////

class SdrObjMacroHitRec
{
public:
    basegfx::B2DPoint           maPos;
    basegfx::B2DPoint           maDownPos;
    OutputDevice*               mpOut;
    const SdrView*              mpSdrView;
    double                      mfTol;

    /// bitfield
    bool                        mbDown;

public:
    SdrObjMacroHitRec()
    :   mpOut(0),
        mpSdrView(0),
        mfTol(0.0),
        mbDown(false)
    {
    }
};

//////////////////////////////////////////////////////////////////////////////

class SVX_DLLPUBLIC SdrObjUserData
{
protected:
    sal_uInt32                      mnInventor;
    sal_uInt16                      mnIdentifier;

private:
    SVX_DLLPRIVATE void operator=(const SdrObjUserData& rData);        // nicht implementiert
    SVX_DLLPRIVATE bool operator==(const SdrObjUserData& rData) const; // nicht implementiert
    SVX_DLLPRIVATE bool operator!=(const SdrObjUserData& rData) const; // nicht implementiert

public:
    SdrObjUserData(sal_uInt32 nInv, sal_uInt16 nId);
    SdrObjUserData(const SdrObjUserData& rData);
    virtual ~SdrObjUserData();

    virtual SdrObjUserData* Clone(SdrObject* pObj1) const = 0; // #i71039# NULL -> 0
    sal_uInt32  GetInventor() const { return mnInventor; }
    sal_uInt16  GetId() const { return mnIdentifier; }

    virtual bool HasMacro(const SdrObject* pObj) const;
    virtual SdrObject* CheckMacroHit (const SdrObjMacroHitRec& rRec, const SdrObject* pObj) const;
    virtual Pointer GetMacroPointer (const SdrObjMacroHitRec& rRec, const SdrObject* pObj) const;
    virtual void PaintMacro(OutputDevice& rOut, const SdrObjMacroHitRec& rRec, const SdrObject* pObj) const;
    virtual bool DoMacro(const SdrObjMacroHitRec& rRec, SdrObject* pObj);
    virtual XubString GetMacroPopupComment(const SdrObjMacroHitRec& rRec, const SdrObject* pObj) const;
};

//////////////////////////////////////////////////////////////////////////////

class SdrObjUserDataList
{
private:
    typedef ::std::vector< SdrObjUserData* > SdrObjUserDataContainerType;
    SdrObjUserDataContainerType maList;

public:
    SdrObjUserDataList();
    ~SdrObjUserDataList();

    void Clear();
    sal_uInt32 GetUserDataCount() const { return maList.size(); }
    SdrObjUserData* GetUserData(sal_uInt32 nNum) const;
    void InsertUserData(SdrObjUserData* pData, sal_uInt32 nPos = 0xffffffff);
    SdrObjUserData* RemoveUserData(sal_uInt32 nNum);
    void DeleteUserData(sal_uInt32 nNum) { delete RemoveUserData(nNum); }
};

//////////////////////////////////////////////////////////////////////////////

class SVX_DLLPUBLIC SdrObjGeoData
{
public:
    basegfx::B2DHomMatrix           maSdrObjectTransformation;
    basegfx::B2DPoint               maObjectAnchor;
    sdr::glue::GluePointProvider*   mpGluePointProvider;
    SdrLayerID                      mnLayerID;

    /// bitfield
    bool                            mbMoveProtect : 1;
    bool                            mbSizeProtect : 1;
    bool                            mbNoPrint : 1;
    bool                            mbVisible : 1;

public:
    SdrObjGeoData();
    virtual ~SdrObjGeoData();
};

//////////////////////////////////////////////////////////////////////////////

class SdrObjPlusData
{
public:
    SdrObjUserDataList*         mpUserDataList; // applikationsspeziefische Daten

    // object name, title and description
    String                      maObjName;
    String                      maObjTitle;
    String                      maObjDescription;

    // Name to be used by applications
    XubString                   maHTMLName;

    // on import of OLE object from MS documents the BLIP size might be retrieved,
    // in this case the following member is initialized as nonempty rectangle
    basegfx::B2DRange           maBLIPSizeRange;

    // anchor (only used for SW as extra-translation)
    basegfx::B2DPoint           maObjectAnchor;

public:
    SdrObjPlusData();
    virtual ~SdrObjPlusData();
    virtual SdrObjPlusData* Clone(SdrObject* pObj1) const;
};

//////////////////////////////////////////////////////////////////////////////

class SVX_DLLPUBLIC SdrObjTransformInfoRec
{
public:
    bool        mbSelectAllowed : 1;     // false=Obj kann nicht selektiert werden
    bool        mbMoveAllowed : 1;       // false=Obj kann nicht verschoben werden
    bool        mbResizeFreeAllowed : 1; // false=Obj kann nicht frei resized werden
    bool        mbResizePropAllowed : 1; // false=Obj kann nichtmal proportional resized werden
    bool        mbRotateFreeAllowed : 1; // false=Obj kann nicht frei gedreht werden
    bool        mbRotate90Allowed : 1;   // false=Obj kann nichtmal im 90deg Raster gedreht werden
    bool        mbMirrorFreeAllowed : 1; // false=Obj kann nicht frei gespiegelt werden
    bool        mbMirror45Allowed : 1;   // false=Obj kann nichtmal ueber Achse im 45deg Raster gespiegelt werden
    bool        mbMirror90Allowed : 1;   // false=Obj kann ebenfalls nicht ueber Achse im 90deg Raster gespiegelt werden
    bool        mbTransparenceAllowed : 1; // false=Obj does not have an interactive transparence control
    bool        mbGradientAllowed : 1; // false=Obj dooes not have an interactive gradient control
    bool        mbShearAllowed : 1;      // false=Obj kann nicht verzerrt werden
    bool        mbEdgeRadiusAllowed : 1;
    bool        mbNoOrthoDesired : 1;    // true bei Rect; ... false bei BMP,MTF;
    bool        mbNoContortion : 1;      // false=Kein verzerren (bei Crook) moeglich (nur true bei PathObj und Gruppierten PathObjs)
    bool        mbCanConvToPath : 1;     // false=Keine Konvertierung in PathObj moeglich
    bool        mbCanConvToPoly : 1;     // false=Keine Konvertierung in PolyObj moeglich
    bool        mbCanConvToContour : 1;     // false=no conversion down to whole contour possible
    bool        mbCanConvToPathLineToArea : 1; // false=Keine Konvertierung in PathObj moeglich mit Wandlung von LineToArea
    bool        mbCanConvToPolyLineToArea : 1; // false=Keine Konvertierung in PolyObj moeglich mit Wandlung von LineToArea

public:
    SdrObjTransformInfoRec();
};

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// SdrObject
//   SdrObjGroup
//   SdrAttrObj
//     E3dObject
//       E3dCompoundObject
//         E3dCubeObj
//         E3dExtrudeObj
//         E3dLatheObj
//         E3dPolygonObj
//         E3dSphereObj
//       E3dScene
//     SdrTextObj (eTextKind from here) bTextFrame=false (default)
//       SdrObjCustomShape bTextFrame = true, set in constructor
//         OCustomShape OK, derived
//       SdrEdgeObj bTextFrame=false, derived
//       SdrMeasureObj bTextFrame=false, derived
//       SdrPathObj (meKind from here) bTextFrame=false, derived
//       SdrRectObj
//         SdrCaptionObj
//         SdrCircObj (meCircleKind from here)
//         SdrGrafObj
//         SdrMediaObj
//         SdrOle2Obj
//           OOle2Obj
//         SdrUnoObj
//           DlgEdObj
//             DlgEdForm
//           OUnoObject
//           FmFormObj
//       SdrTableObj bTextFrame=false, derived
//   SdrPageObj
//     sd::slidesorter::view::PageObject
//   SwDrawVirtObj
//   SwFlyDrawObj
//   SwVirtFlyDrawObj
//

typedef ::std::vector< SdrObject* > SdrObjectVector;

//////////////////////////////////////////////////////////////////////////////
//
// checked usages of SfxListener. Used by:
// - SdrEdgeObj to react on changes of the objects selected to
// - SdrPageObj to react on changes of the visualized SdrPage
// - SdrTextObj to listen to changes of Text StyleSheets used in the
//   OutlinerParaObject
// - OBJ_OUTLINETEXT SdrTextObjs and PRESOBJ_OUTLINE to react on changes to all 10 layes
//   of stylesheets
//
// These usages could me moved to the involved SdrObject types. The specials for SdrTextObj
// should be moved to SdrText

class SVX_DLLPUBLIC SdrObject
:   private boost::noncopyable,
    public SfxListener,
    public SfxBroadcaster,
    public tools::WeakBase< SdrObject >
{
private:
    // the SdrModel this objects was created with, unchanged during SdrObject lifetime
    SdrModel&                                           mrSdrModelFromSdrObject;

    // on-demand members
    sdr::properties::BaseProperties*                    mpProperties;
    sdr::contact::ViewContact*                          mpViewContact;
    sdr::glue::GluePointProvider*                       mpGluePointProvider;

    virtual sdr::properties::BaseProperties* CreateObjectSpecificProperties();
    virtual sdr::contact::ViewContact* CreateObjectSpecificViewContact();
    virtual sdr::glue::GluePointProvider* CreateObjectSpecificGluePointProvider();

    /** only for internal use! */
    SvxShape* getSvxShape() const;

    /** do not use directly, always use getSvxShape() if you have to! */
    SvxShape*                                           mpSvxShape;
    ::com::sun::star::uno::WeakReference< ::com::sun::star::uno::XInterface > maWeakUnoShape;

    // only allow SetOrdNumAtSdrObjectFromSdrObjList to call SetOrdNum
    friend void SetOrdNumAtSdrObjectFromSdrObjList(SdrObject& rSdrObject, sal_uInt32 nOrdNum);
    SVX_DLLPRIVATE void SetOrdNum(sal_uInt32 nOrdNum);

    // only allow SetParentAtSdrObjectFromSdrObjList to call setParentOfSdrObject
    friend void SetParentAtSdrObjectFromSdrObjList(SdrObject& rSdrObject, SdrObjList* pNew);
    SVX_DLLPRIVATE void setParentOfSdrObject(SdrObjList* pNew);

    // only allow SetUnoShapeAtSdrObjectFromSvxShape to call impl_setUnoShape from outside SdrObject
    // SVX_DLLPUBLIC needed for SetUnoShapeAtSdrObjectFromSvxShape since it's used in reportdesigner
    friend void SVX_DLLPUBLIC SetUnoShapeAtSdrObjectFromSvxShape(SdrObject& rSdrObject, const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxUnoShape);
    SVX_DLLPRIVATE void impl_setUnoShape(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxUnoShape);

protected:
    // object transformation including scale, shearX, rotate and translate
    basegfx::tools::B2DHomMatrixBufferedOnDemandDecompose   maSdrObjectTransformation;

    // parent list this object is added to
    SdrObjList*                 mpParentOfSdrObject;
    SdrObjPlusData*             mpPlusData;    // Broadcaster, UserData, Konnektoren, ... (Das ist der Bitsack)

    sal_uInt32                  mnOrdNum;      // Rangnummer des Obj in der Liste
    SdrLayerID                  mnLayerID;

    // bitfield
    bool                        mbMoveProtect : 1;  // If true, the position is protected
    bool                        mbSizeProtect : 1;  // If true, the size is protected
    bool                        mbNoPrint : 1;      // If true, the object is not printed
    bool                        mbMarkProtect : 1;  // If true, object cannot be marked
    bool                        mbVisible : 1;      // If false, the object is not visible on screen (but maybe on printer, depending on bNoprint

    // Wenn bEmptyPresObj true ist, handelt es sich um ein
    // Praesentationsobjekt, dem noch kein Inhalt zugewiesen
    // wurde. Default ist das Flag auf false. Die Verwaltung
    // uebernimmt die Applikation. Im Zuweisungsoperator sowie
    // beim Clone wird das Flag nicht mitkopiert!
    // Das Flag ist persistent.
    bool                        mbEmptyPresObj : 1;     // Leeres Praesentationsobjekt (Draw)

    // true=Objekt ist als Objekt der MasterPage nicht sichtbar
    bool                        mbNotVisibleAsMaster : 1;

    // #i108739# extra bool flag(s) for applications
    bool                        mbUniversalApplicationFlag01 : 1;

    // Objekte werden gerade resized wegen Paste mit anderem MapMode
    bool                        mbPasteResize : 1;

    void ImpDeleteUserData();
    SdrObjUserData* ImpGetMacroUserData() const;

    // Fuer GetDragComment
    void TakeMarkedDescriptionString(sal_uInt16 nStrCacheID, String& rStr, sal_uInt16 nVal = 0) const;
    void ImpForcePlusData() { if (!mpPlusData) mpPlusData = NewPlusData(); }
    String GetWinkStr(sal_Int32 nWink, bool bNoDegChar = false) const;
    String GetMetrStr(sal_Int32 nVal, MapUnit eWantMap = MAP_MM, bool bNoUnitChars = false) const;

    // ueberladen, wenn man sich von SdrObjPlusData abgeleitet hat:
    virtual SdrObjPlusData* NewPlusData() const;

    // Diese 3 Methoden muss ein abgeleitetes Objekt ueberladen, wenn es eigene
    // geometrische Daten besitzt, die fuer den Undo-Fall gesichert werden
    // sollen. NewGeoData() erzeugt lediglich eine leere Instanz auf eine von
    // SdrObjGeoData abgeleitete Klasse.
    virtual SdrObjGeoData* NewGeoData() const;
    virtual void SaveGeoData(SdrObjGeoData& rGeo) const;
    virtual void RestGeoData(const SdrObjGeoData& rGeo);

    // internal versions for ItemSet accesses; these are without merge for group objects
    const SfxItemSet& GetObjectItemSet() const;
    void SetObjectItem(const SfxPoolItem& rItem);
    void ClearObjectItem(const sal_uInt16 nWhich = 0);
    void SetObjectItemSet(const SfxItemSet& rSet);
    const SfxPoolItem& GetObjectItem(const sal_uInt16 nWhich) const;

    // destructor is protected by purpose, use deleteSdrObjectSafeAndClearPointer(..)
    // or deleteSafe to delete SdrObjects
    virtual ~SdrObject();

    /// method to copy all data from given source
    virtual void copyDataFromSdrObject(const SdrObject& rSource);

public:
    bool IsPasteResize() const { return mbPasteResize; } // #51139# TTTT: Probably not needed anymore
    void SetPasteResize(bool bOn) { if(mbPasteResize != bOn) mbPasteResize = bOn; }

    /// create a copy, evtl. with a different target model (if given)
    virtual SdrObject* CloneSdrObject(SdrModel* pTargetModel = 0) const;

    // SdrModel/SdrPage access on SdrObject level
    virtual SdrPage* getSdrPageFromSdrObject() const;
    SdrModel& getSdrModelFromSdrObject() const;
    SdrObjList* getParentOfSdrObject() const { return mpParentOfSdrObject; }
    virtual SdrObjList* getChildrenOfSdrObject() const;

    // shortcut for getting the parent as SdrObj directly e.g. SdrObjGroup
    SdrObject* GetParentSdrObject() const;

    // react on model/page change
    virtual void handlePageChange(SdrPage* pOldPage, SdrPage* pNewPage);

    // check if SdrObject may be filled
    virtual bool IsClosedObj() const;

    SdrObject(SdrModel& rSdrModel, const basegfx::B2DHomMatrix& rTransform = basegfx::B2DHomMatrix());

    /** fres the SdrObject pointed to by the argument
        In case the object has an SvxShape, which has the ownership of the object, it
        is actually *not* deleted.
    */
    static void deleteSafe(SdrObject* pObject);

    // acces to on-demand members
    virtual sdr::properties::BaseProperties& GetProperties() const;
    sdr::contact::ViewContact& GetViewContact() const;
    sdr::glue::GluePointProvider& GetGluePointProvider() const;

    // DrawContact support: Methods for handling Object changes
    void ActionChanged() const;

    SfxItemPool& GetObjectItemPool() const;

    virtual sal_uInt32 GetObjInventor() const;
    virtual sal_uInt16 GetObjIdentifier() const;
    virtual void TakeObjInfo(SdrObjTransformInfoRec& rInfo) const;

    // Layer interface
    virtual SdrLayerID GetLayer() const;
    virtual void SetLayer(SdrLayerID nLayer);
    virtual void getMergedHierarchyLayerSet(SetOfByte& rSet) const;

    // #i68101#
    // An object may have a user-set Name (Get/SetName()), e.g SdrGrafObj, SdrObjGroup
    // or SdrOle2Obj.
    // It may also have a Title and a Description for accessibility purposes.
    void SetName(const String& rStr);
    String GetName() const;
    void SetTitle(const String& rStr);
    String GetTitle() const;
    void SetDescription(const String& rStr);
    String GetDescription() const;

    // support for HTMLName
    void SetHTMLName(const String& rStr);
    String GetHTMLName() const;

    // get objects current position in the object list which it belongs to.
    // If not member of an object list, null is returned.
    sal_uInt32 GetNavigationPosition() const;

    // access to bounding range of the object. If SdrView is given, the
    // view-dependent bound is returned, else the logical bounds. Take into
    // account that these may be different, e.g. with hairlines the
    // view-dependent bounds are evtl. bigger since the discrete view pixels cover
    // more space than the logical definition
    const basegfx::B2DRange& getObjectRange(const SdrView* pSdrView) const;

    // access to minimal geometry bounding rectangle (Snap range). This includes all
    // geometry as hairlines. Invisible parts are included, text is ignored
    const basegfx::B2DRange& getSnapRange() const;

    // Modified-Flag am Model setzen
    virtual void SetChanged();

    // Tooling for painting a single object to a OutputDevice. This will be needed as sal_Int32
    // as not all painting is changed to use DrawContact objects.
    bool SingleObjectPainter(OutputDevice& rOut) const;
    bool LineGeometryUsageIsNecessary() const;

    // TakeObjName...() ist fuer die Anzeige in der UI, z.B. "3 Rahmen selektiert".
    virtual void TakeObjNameSingul(String& rName) const;
    virtual void TakeObjNamePlural(String& rName) const;

    // Das Xor-Polygon wird von der View zu Draggen des Objektes benoetigt.
    // Alle Polygone innerhalb des PolyPolygon werden als PolyLine interpretiert.
    // Moechte man ein geschlossenes Polygon, so muss man es explizit schliessen.
    virtual basegfx::B2DPolyPolygon TakeXorPoly() const;

    // Ueber GetHdlCount gibt ein Objekt die Anzahl seiner Handles preis.
    // Im Normalfall werden dies 8 sein, bei einer Strecke 2. Bei Polygonobjekten
    // (Polygon,Spline,Bezier) kann die Handleanzahl wesentlich groesser werden.
    // Polygonobjekten wird ausserdem die Moeglichkeit eingeraeumt einen Punkt
    // eines selektierten Objekts zu selektieren. Das Handle dieses Punktes wird
    // dann durch einen Satz neuer Handles ausgetauscht (PlusHdl). Bei einem
    // Polygon wird das wohl ein einfacher Selektionshandle sein, bei einer
    // Bezierkurve dagegen koennen das schon bis zu 3 Handles werden (inkl Gewichte).
    // GetHdl() und GetPlusHdl() muessen Handleinstanzen mit new erzeugen!
    // Ein Objekt, das bei HasSpacialDrag() true liefert muss diese Methoden
    // zur Verfuegung stellen (inkl. FillHdlList).
    virtual sal_uInt32 GetPlusHdlCount(const SdrHdl& rHdl) const;
    virtual void GetPlusHdl(SdrHdlList& rHdlList, const SdrObject& rSdrObject, const SdrHdl& rHdl, sal_uInt32 nPlNum) const;
    virtual void AddToHdlList(SdrHdlList& rHdlList) const;

    // Die Standardtransformationen (Move,Resize,Rotate,Mirror,Shear) werden von der
    // View uebernommen (TakeXorPoly(),...).
    // Objektspeziefisches draggen wie z.B. Eckenradius bei Rechtecken,
    // Stuetzstellen bei Splines, Gewichte bei Bezierkurven und Pointer von
    // Labelobjekten muss vom Objekt selbst gehandled werden. Um das Model
    // Statusfrei zu halten werden die Statusdaten an der View gehalten und dem
    // Objekt dann uebergeben. EndDrag liefrt im Normalfall true fuer Erfolg.
    // false kann zurueckgegeben werden, wenn das Dragging das Objekt nicht
    // veraendert hat, wobei dir evtl. Tatsache das die Maus nicht bewegt wurde
    // bereits von der View abgefangen wird.
    virtual bool hasSpecialDrag() const;
    virtual bool beginSpecialDrag(SdrDragStat& rDrag) const;
    virtual bool applySpecialDrag(SdrDragStat& rDrag);
    virtual String getSpecialDragComment(const SdrDragStat& rDrag) const;
    virtual basegfx::B2DPolyPolygon getSpecialDragPoly(const SdrDragStat& rDrag) const;

    // FullDrag support. This is for standard interactions and for SdrObjOwn
    // support. If supportsFullDrag() returns true, getFullDragClone has to
    // return a cloned SdrObject (who's ownership it loses) at which modifications
    // like Move(), Scale(), etc or applySpecialDrag() will be executed. That
    // object will be visualized on overlay for full drag, but should not be
    // part of the model, thus not changing anything since it's only a temporary
    // helper object for interaction
    virtual bool supportsFullDrag() const;
    virtual SdrObject* getFullDragClone() const;

    // Jedes Objekt muss in der Lage sein sich selbst interaktiv zu erzeugen.
    // Beim MausDown wird zunaechst ein neues Objekt erzeugt und dann seine
    // BegCreate()-Methode gerufen. Bei jedem MausMode wird dann MovCreate
    // gerufen. BrkCreate() bedeutet, dass der User die interaktive Objekt-
    // erzeugung abgebrochen hat. EndCreate() wird gerufen beim MouseUp-Event.
    // Liefert EndCreate() ein true, so ist die Objekterzeugung abgeschlossen;
    // das Objekt wird in die entsprechende Liste eingefuegt. Andernfalls
    // (EndCreate()==false) gehe ich davon aus, dass weitere Punkte zur
    // Objekterzeugung notwendig sind (Polygon,...). Der Parameter eCmd
    // enthaelt die Anzahl der Mausklicks (so die App diese durchreicht).
    // BckCreate() -> Letztes EndCreate() rueckgaengig machen (z.B. letzten
    // Polygonpunkt wieder loeschen).
    // RetrunCode: true=Weiter gehts, false=Create dadurch abgebrochen.
    virtual bool BegCreate(SdrDragStat& rStat);
    virtual bool MovCreate(SdrDragStat& rStat); // true=Xor muss repainted werden
    virtual bool EndCreate(SdrDragStat& rStat, SdrCreateCmd eCmd);
    virtual bool BckCreate(SdrDragStat& rStat);
    virtual void BrkCreate(SdrDragStat& rStat);

    // Polygon das waehrend des Erzeugens aufgezogen wird
    virtual basegfx::B2DPolyPolygon TakeCreatePoly(const SdrDragStat& rDrag) const;

    // get/setSdrObjectTransformation
    virtual const basegfx::B2DHomMatrix& getSdrObjectTransformation() const;
    virtual void setSdrObjectTransformation(const basegfx::B2DHomMatrix& rTransformation);

    // convenience functions for access to atomic decomposed object transformation parts
    const basegfx::B2DVector& getSdrObjectScale() const { return maSdrObjectTransformation.getScale(); }
    const basegfx::B2DPoint& getSdrObjectTranslate() const { return maSdrObjectTransformation.getTranslate(); }
    double getSdrObjectRotate() const { return maSdrObjectTransformation.getRotate(); }
    double getSdrObjectShearX() const { return maSdrObjectTransformation.getShearX(); }
    bool isRotated() const { return maSdrObjectTransformation.isRotated(); }
    bool isSheared() const { return maSdrObjectTransformation.isSheared(); }
    bool isMirroredX() const { return maSdrObjectTransformation.isMirroredX(); }
    bool isMirroredY() const { return maSdrObjectTransformation.isMirroredY(); }
    bool isTranslatedX() const { return maSdrObjectTransformation.isTranslatedX(); }
    bool isTranslatedY() const { return maSdrObjectTransformation.isTranslatedY(); }
    bool isRotatedOrSheared() const { return maSdrObjectTransformation.isRotatedOrSheared(); }
    bool isMirrored() const { return maSdrObjectTransformation.isMirrored(); }
    bool isTranslated() const { return maSdrObjectTransformation.isTranslated(); }

    // #i108739# Anchor read/write access
    virtual void SetAnchorPos(const basegfx::B2DPoint& rPnt);
    basegfx::B2DPoint GetAnchorPos() const;

    // #i108739# extra bool flag for SC to get rid of the old Anchor-Hack
    bool getUniversalApplicationFlag01() const { return mbUniversalApplicationFlag01; }
    void setUniversalApplicationFlag01(bool bNew) { if(bNew != (bool)mbUniversalApplicationFlag01) mbUniversalApplicationFlag01 = bNew; }

    /** the defaul is to set the logic rect to the given rectangle rMaxRect. If the shape
        has an intrinsic aspect ratio it may set the logic rect so the aspect
        ratio is kept but still inside the rectangle rMaxRect.

        If bShrinkOnly is set to true, the size of the current logic rect will not
        be changed if it is smaller than the given rectangle rMaxRect. */
    virtual void AdjustToMaxRange( const basegfx::B2DRange& rMaxRange, bool bShrinkOnly = false );

    // Zum Fangen von/auf ausgezeichneten Punkten eines Obj (Polygonpunkte,
    // Kreismittelpunkt, ...)
    virtual sal_uInt32 GetSnapPointCount() const;
    virtual basegfx::B2DPoint GetSnapPoint(sal_uInt32 i) const;

    // Fuer Objekte, bei denen jeder einzelne Punkt verschoben werden kann,
    // z.B. Polygone, Polylines, Linien, ... . Bei diesen Objekten werden
    // Punkte selektiert (ggf. Mehrfachselektion), geloescht, eingefuegt,
    // als Mehrfachselektion verschoben und gedreht, ...
    // Nur solche Objekte koennen PlusHandles haben (z.B. die Gewichte an den
    // Bezierkurven.
    virtual bool IsPolygonObject() const;
    virtual sal_uInt32 GetObjectPointCount() const;
    virtual basegfx::B2DPoint GetObjectPoint(sal_uInt32 i) const;
    virtual void SetObjectPoint(const basegfx::B2DPoint& rPnt, sal_uInt32 i);

    // Alle geometrischen Daten holen fuer's Undo/Redo
    virtual SdrObjGeoData* GetGeoData() const;
    virtual void SetGeoData(const SdrObjGeoData& rGeo);

    // ItemSet access
    const SfxItemSet& GetMergedItemSet() const;
    void SetMergedItem(const SfxPoolItem& rItem);
    void ClearMergedItem(const sal_uInt16 nWhich = 0);
    void SetMergedItemSet(const SfxItemSet& rSet, bool bClearAllItems = false);
    const SfxPoolItem& GetMergedItem(const sal_uInt16 nWhich) const;

    // syntactical sugar for ItemSet accesses
    void SetMergedItemSetAndBroadcast(const SfxItemSet& rSet, bool bClearAllItems = false);

    // NotPersistAttr fuer Layer, ObjName, geometrische Transformationen, ...
    void TakeNotPersistAttr(SfxItemSet& rAttr, bool bMerge) const;
    void ApplyNotPersistAttr(const SfxItemSet& rAttr);

    // bDontRemoveHardAttr=false: alle in der Vorlage gesetzten Attribute werden am
    // Zeichenobjekt auf Default gesetzt; true: alle harten Attribute bleiben erhalten.
    virtual void SetStyleSheet(SfxStyleSheet* pNewStyleSheet, bool bDontRemoveHardAttr);
    SfxStyleSheet* GetStyleSheet() const;

    // TextEdit
    virtual bool HasTextEdit() const;

    // Return==true: TextEditMode gestartet
    virtual bool BegTextEdit(SdrOutliner& rOutl);
    virtual void EndTextEdit(SdrOutliner& rOutl);

    // Text wird im Format des Outliners gehalten
    // SetOutlinerParaObject: Eigentumsuebereignung von *pTextObject!
    virtual void SetOutlinerParaObject(OutlinerParaObject* pTextObject);
    virtual OutlinerParaObject* GetOutlinerParaObject() const;
    virtual void ReformatText();
    void BurnInStyleSheetAttributes();

    // Macrofaehigkeit, z.B. ein Rechteck als PushButton.
    virtual bool HasMacro() const;
    virtual SdrObject* CheckMacroHit (const SdrObjMacroHitRec& rRec) const;
    virtual Pointer GetMacroPointer (const SdrObjMacroHitRec& rRec) const;
    virtual void PaintMacro (OutputDevice& rOut, const SdrObjMacroHitRec& rRec) const;
    virtual bool DoMacro (const SdrObjMacroHitRec& rRec);
    virtual XubString GetMacroPopupComment(const SdrObjMacroHitRec& rRec) const;
    bool IsMacroHit(const SdrObjMacroHitRec& rRec) const { return CheckMacroHit(rRec)!=NULL; }

    /** sets the writing mode of the object's context

        Objects which itself do not support different writing modes will ignore this call.

        Objects which support different writing modes, but have an own, explicit writing mode set,
        will also ignore this call.

        Objects which support different writing modes, and whose own mode is set to css.text.WritingMode2.CONTEXT,
        will use the given writing mode to calculate their "effective writing mode".

        The behaviour of this method is undefined if you pass css.text.WritingMode2.CONTEXT.

        @param _nContextWritingMode
            the effective writing mode of the context of the object
    */
    virtual void    SetContextWritingMode( const sal_Int16 _nContextWritingMode );

    // Wenn ein Objekt in der Lage ist, sich in ein Polygon oder in eine
    // Bezierkurve (oder beides) zu verwandeln, dann sollten die folgenden
    // Methoden ueberladen werden.
    // Z.B. Ein RectObj mit Strichstaerke 10, SOLID_PEN in Polygon wandeln:
    // Im Modus bLineToArea=false soll ein PolyObj mit 4 Stuetzstellen,
    // Strichstaerke 10 und SOLiD_PEN erzeugt werden.
    // Im Modus bLineToArea=true dagegen soll das generierte Obj immer ein
    // LinienAttribut NULL_PEN haben, und die Linie (auch Linienmuster) selbst
    // durch Polygonflaechen emuliert werden, die dadurch anschliessend vom
    // Anwender manipuliert werden koennen. Das RectObj kann sich somit also
    // nur korrekt konvertieren, wenn es als Flaechenattribut NULL_BRUSH
    // besitzt. Es muesste in diesem Fall dann:
    // - SOLID_BRUSH mit der Farbe des gesetzten Pen setzen
    // - NULL_PEN setzen
    // - PolyPolygon mit 2 Polygonen zu je 4 Stuetzstellen generieren.
    // Der Returnwert ist jeweils ein SdrObject*, da als Rueckgabe zur Not
    // auch Gruppenobjekte erlaubt sind (z.B. fuer SdrTextObj).
    // Bei der Konvertierung von TextObj nach PathObj wird es wohl so sein,
    // dass beide Modi (bLineToArea=true/false) identisch sind.
    // Defaulted sind diese Methoden auf "Ich kann das nicht" (false/NULL).
    virtual SdrObject* DoConvertToPolygonObject(bool bBezier, bool bAddText) const;
    SdrObject* ConvertToPolyObj(bool bBezier, bool bLineToArea) const;

    // convert this path object to contour object; bForceLineDash converts even
    // when there is no filled new polygon created from line-to-polygon conversion,
    // specially used for XLINE_DASH and 3D conversion
    SdrObject* ConvertToContourObj(SdrObject* pRet, bool bForceLineDash = false) const;
    SdrObject* ImpConvertToContourObj(SdrObject* pRet, bool bForceLineDash = false) const;

    // true=Obj kann warsch. gefuellt werden; false=Obj kann warsch. Linienenden haben.
    // ungueltig, wenn es sich um ein GroupObj handelt.
//  virtual bool IsE3dObject() const;
//  virtual bool IsSdrUnoObj() const;
//  virtual bool IsSdrGrafObj() const;
    virtual bool DoesSupportTextIndentingOnLineWidthChange() const;

    // void SetObjectInserted(bool bNew);
    bool IsObjectInserted() const { return 0 != getParentOfSdrObject(); }

    void SetMoveProtect(bool bNew);
    bool IsMoveProtect() const { return mbMoveProtect; }

    void SetVisible(bool bNew);
    bool IsVisible() const { return mbVisible; }

    void SetResizeProtect(bool bNew);
    bool IsResizeProtect() const { return mbSizeProtect; }

    void SetPrintable(bool bNew);
    bool IsPrintable() const { return !mbNoPrint; }

    void SetMarkProtect(bool bNew) { if(mbMarkProtect != bNew) mbMarkProtect = bNew; }
    bool IsMarkProtect() const { return mbMarkProtect; }

    void SetEmptyPresObj(bool bNew) { if(mbEmptyPresObj != bNew) mbEmptyPresObj = bNew; }
    bool IsEmptyPresObj() const { return mbEmptyPresObj; }

    void SetNotVisibleAsMaster(bool bNew) { if(mbNotVisibleAsMaster != bNew) mbNotVisibleAsMaster = bNew; }
    bool IsNotVisibleAsMaster() const { return mbNotVisibleAsMaster; }

    // applikationsspeziefische Daten
    sal_uInt32 GetUserDataCount() const;
    SdrObjUserData* GetUserData(sal_uInt32 nNum) const;

    // Insert uebernimmt den auf dem Heap angelegten Record in den Besitz
    // des Zeichenobjekts
    void InsertUserData(SdrObjUserData* pData, sal_uInt32 nPos=0xffffffff);

    // Delete entfernt den Record aus der Liste und ruft
    // ein delete (FreeMem+Dtor).
    void DeleteUserData(sal_uInt32 nNum);

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // access to the UNO representation of the shape
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > getUnoShape();
    ::com::sun::star::uno::WeakReference< ::com::sun::star::uno::XInterface > getWeakUnoShape() const { return maWeakUnoShape; }

    static SdrObject* getSdrObjectFromXShape( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& xInt );

    /** retrieves the instance responsible for notifying changes in the properties of the shape associated with
        the SdrObject

        @precond
            There already exists an SvxShape instance associated with the SdrObject
        @throws ::com::sun::star::uno::RuntimeException
            if there does nt yet exists an SvxShape instance associated with the SdrObject.
    */
    ::svx::PropertyChangeNotifier& getShapePropertyChangeNotifier();

    /** notifies a change in the given property, to all applicable listeners registered at the associated SvxShape

        This method is equivalent to calling getShapePropertyChangeNotifier().notifyPropertyChange( _eProperty ),
        exception that it is allowed to be called when there does not yet exist an associated SvxShape - in which
        case the method will silently return without doing anything.
    */
    void notifyShapePropertyChange( const ::svx::ShapeProperty _eProperty ) const;

    // return if fill is != XFILL_NONE
    bool HasFillStyle() const;
    bool HasLineStyle() const;

    // on import of OLE object from MS documents the BLIP size might be retrieved,
    // the following methods are used to control it;
    // usually this data makes no sence after the import is finished, since the object
    // might be resized
    const basegfx::B2DRange GetBLIPSizeRange() const;
    void SetBLIPSizeRange( const basegfx::B2DRange& aRect );

    // get all SdrEdgeObjs connected to this object
    ::std::vector< SdrEdgeObj* > getAllConnectedSdrEdgeObj() const;

    // #121917#
    virtual bool HasText() const;
};

typedef tools::WeakReference< SdrObject > SdrObjectWeakRef;
#define deleteSdrObjectSafeAndClearPointer(Obj) { SdrObject::deleteSafe(Obj); Obj = 0; }
#define deleteSdrObjectSafe(Obj) { SdrObject::deleteSafe(Obj); }

//////////////////////////////////////////////////////////////////////////////

#endif //_SVDOBJ_HXX

//////////////////////////////////////////////////////////////////////////////
// eof

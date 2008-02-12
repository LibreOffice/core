/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: svdpage.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: vg $ $Date: 2008-02-12 16:35:31 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _SVDPAGE_HXX
#define _SVDPAGE_HXX

#ifndef _SV_BITMAP_HXX
#include <vcl/bitmap.hxx>
#endif

#ifndef _PRINT_HXX //autogen
#include <vcl/print.hxx>
#endif
#ifndef _GDIMTF_HXX //autogen
#include <vcl/gdimtf.hxx>
#endif

#ifndef _TOOLS_WEAKBASE_HXX_
#include <tools/weakbase.hxx>
#endif

#ifndef _CPPUHELPER_WEAKREF_HXX_
#include <cppuhelper/weakref.hxx>
#endif

#ifndef _SVDTYPES_HXX
#include <svx/svdtypes.hxx>
#endif
#ifndef _SVDLAYER_HXX
#include <svx/svdlayer.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////
// sdr::Comment interface
#ifndef _SDR_COMMENT_HXX
#include <svx/sdrcomment.hxx>
#endif

// #111111#
#include <vector>

#ifndef _SDR_PAGEUSER_HXX
#include <svx/sdrpageuser.hxx>
#endif

// StandardCheckVisisbilityRedirector
#ifndef _SDR_CONTACT_VIEWOBJECTCONTACTREDIRECTOR_HXX
#include <svx/sdr/contact/viewobjectcontactredirector.hxx>
#endif

#ifndef _SDR_MASTERPAGEDESCRIPTOR_HXX
#include <svx/sdrmasterpagedescriptor.hxx>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

#include <com/sun/star/container/XIndexAccess.hpp>
#include <svx/svdobj.hxx>
#include <boost/scoped_ptr.hpp>

// #110094#
namespace sdr
{
    namespace contact
    {
        class ViewContact;
    } // end of namespace contact
} // end of namespace sdr

// ------------------------------
// - intern benutzte Paint-Modi -
// ------------------------------

//#if 0 // _SOLAR__PRIVATE

#define IMP_PAGEPAINT_NORMAL            0
#define IMP_PAGEPAINT_PREPARE_CACHE     1
#define IMP_PAGEPAINT_PAINT_CACHE       2
#define IMP_PAGEPAINT_PREPARE_BG_CACHE  3
#define IMP_PAGEPAINT_PAINT_BG_CACHE    4

//#endif // __PRIVATE

class SdrPage;
class XOutputDevice;
class SdrPaintInfoRec;
class SdrModel;
class SfxItemPool;
class SdrPageView;
class SdrLayerAdmin;
class SetOfByte;
class Color;
class SfxStyleSheet;
class SvxUnoDrawPagesAccess;

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

class SVX_DLLPUBLIC SdrObjList {
private:
    typedef ::std::vector<SdrObject*> SdrObjectContainerType;
    SdrObjectContainerType maList;

protected:
friend class SdrObjListIter;
friend class SdrEditView;
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
//#if 0 // _SOLAR__PRIVATE
    FASTBOOL ImpGetFillColor(SdrObject* pObj, Color& rCol) const;
//#endif // __PRIVATE
public:
    TYPEINFO();
    SdrObjList(SdrModel* pNewModel, SdrPage* pNewPage, SdrObjList* pNewUpList=NULL);
    SdrObjList(const SdrObjList& rSrcList);
    virtual ~SdrObjList();
    // !!! Diese Methode nur fuer Leute, die ganz genau wissen was sie tun !!!

    // #110094# This should not be needed (!)
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
    virtual SdrObject* NbcRemoveObject(ULONG nObjNum);
    virtual SdrObject* RemoveObject(ULONG nObjNum);
    // Vorhandenes Objekt durch ein anderes ersetzen.
    // Wie Remove&Insert jedoch performanter, da die Ordnungsnummern
    // nicht Dirty gesetzt werden muessen.
    virtual SdrObject* NbcReplaceObject(SdrObject* pNewObj, ULONG nObjNum);
    virtual SdrObject* ReplaceObject(SdrObject* pNewObj, ULONG nObjNum);
    // Die Z-Order eines Objekts veraendern
    virtual SdrObject* NbcSetObjectOrdNum(ULONG nOldObjNum, ULONG nNewObjNum);
    virtual SdrObject* SetObjectOrdNum(ULONG nOldObjNum, ULONG nNewObjNum);

    virtual void SetRectsDirty();

    const Rectangle& GetAllObjSnapRect() const;
    const Rectangle& GetAllObjBoundRect() const;

    // HitTest auf alle Objekte der Liste
    SdrObject* CheckHit(const Point& rPnt, USHORT nTol, const SetOfByte* pVisiLayer, FASTBOOL bBackward=FALSE) const;
    SdrObject* CheckHit(const Point& rPnt, USHORT nTol, FASTBOOL bBackward=FALSE) const { return CheckHit(rPnt,nTol,NULL,bBackward); }
    FASTBOOL IsHit(const Point& rPnt, USHORT nTol, const SetOfByte* pVisiLayer) const { return CheckHit(rPnt,nTol,pVisiLayer)!=NULL; }
    FASTBOOL IsHit(const Point& rPnt, USHORT nTol) const                              { return CheckHit(rPnt,nTol)!=NULL; } // #i24906#

    // Alle Textobjekte neu formatieren, z.B. bei Druckerwechsel
    void NbcReformatAllTextObjects();
    void ReformatAllTextObjects();

    /** #103122# reformats all edge objects that are connected to other objects */
    void ReformatAllEdgeObjects();

    // Die Vorlagenattribute der Zeichenobjekte in harte Attribute verwandeln.
    void BurnInStyleSheetAttributes();

    // Bestimmung der FuellFarbe an einer bestimmten Position.
    // FALSE=Kein Objekt mit FuellFarbe an dieser Position gefunden.
    // rVisLayers gibt die zu durchsuchenden Layer an.
    // bLayerSorted: TRUE=Es wird in der Reihenfolge der Layer gesucht (ni)
    // rCol: Hier wird die gefundene Farbe zurueckgegeben
    FASTBOOL GetFillColor(const Point& rPnt, const SetOfByte& rVisLayers,
        /* FASTBOOL bLayerSorted, */ Color& rCol) const;

    ULONG      GetObjCount() const;
    SdrObject* GetObj(ULONG nNum) const;

    // Gelinkte Seite oder gelinktes Gruppenobjekt
    virtual FASTBOOL IsReadOnly() const;

    // Zaehlt alle Objekte inkl. Objekte in Objektgruppen, ...
    ULONG   CountAllObjects() const;

    // Alle aufgelagerten Teile (z.B. Grafiken) der Liste in den
    // Speicher laden.
    void    ForceSwapInObjects() const;
    void    ForceSwapOutObjects() const;

    void    SwapInAll() const { ForceSwapInObjects(); }
    void    SwapOutAll() const { ForceSwapOutObjects(); }

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

    /** Return whether there is an explicit, user defined, object navigation
        order.  When there is one this method returns <TRUE/> and the
        GetObjectForNavigationPosition() and
        SdrObject::GetNavigationPosition() methods will return values
        different from those returne by SdrObject::GetOrdNum() and
        GetObj().
    */
    bool HasObjectNavigationOrder (void) const;

    /** Set the navigation position of the given object to the specified
        value.  Note that this changes the navigation position for all
        objects on or following the old or new position.
    */
    void SetObjectNavigationPosition (
        SdrObject& rObject,
        const sal_uInt32 nNewNavigationPosition);

    /** Return the object for the given navigation position.  When there is
        a user defined navigation order, i.e. mpNavigationOrder is not NULL,
        then that is used to look up the object.  Otherwise the z-order is
        used by looking up the object in maList.
        @param nNavigationPosition
            Valid values include 0 and are smaller than the number of
            objects as returned by GetObjCount().
        @return
            The returned pointer is NULL for invalid positions.
    */
    SdrObject* GetObjectForNavigationPosition (const sal_uInt32 nNavigationPosition) const;

    /** Restore the navigation order to that defined by the z-order.
    */
    void ClearObjectNavigationOrder (void);

    /** Set the navigation position of all SdrObjects to their position in
        the mpNavigationOrder list.  This method returns immediately when no
        update is necessary.
        @return
            This method returns <TRUE/> when the navigation positions stored
            in SdrObjects are up to date.
            It returns <FALSE/> when teh navigation positions are not valid,
            for example because no explicit navigation order has been
            defined, i.e. HasObjectNavigationOrder() would return <FALSE/>.
    */
    bool RecalcNavigationPositions (void);

    /** Set the navigation order to the one defined by the given list of
        XShape objects.
        @param rxOrder
            When this is an empty reference then the navigation order is
            reset to the z-order. The preferred way to do this, however, is
            to call ClearObjectNavigationOrder().
            Otherwise this list is expected to contain all the shapes in the
            called SdrObjList.
    */
    void SetNavigationOrder (const ::com::sun::star::uno::Reference<
        ::com::sun::star::container::XIndexAccess>& rxOrder);

private:
    class WeakSdrObjectContainerType;
    /// This list, if it exists, defines the navigation order.  It it does
    /// not exist then maList defines the navigation order.
    ::boost::scoped_ptr<WeakSdrObjectContainerType> mpNavigationOrder;

    /// This flag is <TRUE/> when the mpNavigation list has been changed but
    /// the indices of the referenced SdrObjects still have their old values.
    bool mbIsNavigationOrderDirty;

    /** Insert an SdrObject into maList.  Do not modify the maList member
        directly.
        @param rObject
            The object to insert into the object list.
        @param nInsertPosition
            The given object is inserted before the object at this
            position.  Valid values include 0 (the object is inserted at the
            head of the list) and the number of objects in the list as
            returned by GetObjCount() (the object is inserted at the end of
            the list.)
    */
    void InsertObjectIntoContainer (
        SdrObject& rObject,
        const sal_uInt32 nInsertPosition);

    /** Replace an object in the object list.
        @param rObject
            The new object that replaces the one in the list at the
            specified position.
        @param nObjectPosition
            The object at this position in the object list is replaced by
            the given object.  Valid values include 0 and are smaller than
            the number of objects in the list.
    */
    void ReplaceObjectInContainer (
        SdrObject& rObject,
        const sal_uInt32 nObjectPosition);

    /** Remove an object from the object list.
        The object list has to contain at least one element.
        @param nObjectPosition
            The object at this position is removed from the object list.
            Valid values include 0 and are smaller than the number of
            objects in the list.
    */
    void RemoveObjectFromContainer (
        const sal_uInt32 nObjectPosition);
};

/*
Eine Sdraw-Seite enthaelt genau eine Objektliste sowie eine Beschreibung
der physikalischen Seitendimensionen (Groesse/Raender). Letzteres wird
lediglich zum Fangen von Objekten beim Draggen benoetigt.
An der Seite lassen sich (ueber SdrObjList) Objekte einfuegen und loeschen,
nach vorn und nach hinten stellen. Ausserdem kann die Ordnungszahl eines
Objektes abgefragt sowie direkt gesetzt werden.
*/

// Used for all methods which return a page number
#define SDRPAGE_NOTFOUND 0xFFFF

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

class SVX_DLLPUBLIC SdrPageGridFrameList {
    Container aList;
private:
    SVX_DLLPRIVATE SdrPageGridFrameList(const SdrPageGridFrameList& rSrcList);      // never implemented
    SVX_DLLPRIVATE void           operator=(const SdrPageGridFrameList& rSrcList);  // never implemented
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

class SVX_DLLPUBLIC SdrPage: public SdrObjList, public tools::WeakBase< SdrPage >
{
    ///////////////////////////////////////////////////////////////////////////////
    // start PageUser section
private:
    // #111111# PageUser section
    sdr::PageUserVector                                             maPageUsers;

public:
    void AddPageUser(sdr::PageUser& rNewUser);
    void RemovePageUser(sdr::PageUser& rOldUser);

    ///////////////////////////////////////////////////////////////////////////////
    // end PageUser section

    ///////////////////////////////////////////////////////////////////////////////
    // #110094# DrawContact section
private:
    sdr::contact::ViewContact*                                      mpViewContact;
    virtual sdr::contact::ViewContact* CreateObjectSpecificViewContact();
public:
    virtual sdr::contact::ViewContact& GetViewContact() const;

    // #110094# DrawContact support: Methods for handling Page changes
    void ActionChanged() const;

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
        /* FASTBOOL bLayerSorted, */ Color& rCol, FASTBOOL bSkipBackgroundShape) const;

protected:
    SdrLayerAdmin*  pLayerAdmin;
    SdrObject*      pBackgroundObj;

    // new MasterPageDescriptorVector
    ::sdr::MasterPageDescriptor*                    mpMasterPageDescriptor;

    SetOfByte  aPrefVisiLayers;
    USHORT     nPageNum;
    bool       bMaster;  // TRUE: Ich bin eine Stammseite
    FASTBOOL   bInserted;
    FASTBOOL   bObjectsNotPersistent;
    FASTBOOL   bSwappingLocked;

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > createUnoPage();

public:
    TYPEINFO();
    SdrPage(SdrModel& rNewModel, bool bMasterPage=false);
    // Copy-Ctor und Zuweisungeoperator sind nicht getestet!
    SdrPage(const SdrPage& rSrcPage);
    virtual ~SdrPage();
    // pModel, pPage, pUpList, pOwnerObj und bInserted werden Zuweisungeoperator nicht veraendert!
    virtual void operator=(const SdrPage& rSrcPage);
    virtual SdrPage* Clone() const;
    virtual SdrPage* Clone(SdrModel* pNewModel) const;
    bool IsMasterPage() const       { return bMaster; }
    void SetInserted(FASTBOOL bJa=TRUE);
    FASTBOOL IsInserted() const         { return bInserted; }
    virtual void SetChanged();

    // #i68775# React on PageNum changes (from Model in most cases)
    void SetPageNum(sal_uInt16 nNew);
    sal_uInt16 GetPageNum() const;

    virtual void SetSize(const Size& aSiz);
    virtual Size GetSize() const;
    virtual void SetOrientation(Orientation eOri);
    virtual Orientation GetOrientation() const;
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

    virtual void SetModel(SdrModel* pNewModel);

    // New MasterPage interface
    sal_Bool TRG_HasMasterPage() const { return (0L != mpMasterPageDescriptor); }
    void TRG_SetMasterPage(SdrPage& rNew);
    void TRG_ClearMasterPage();
    SdrPage& TRG_GetMasterPage() const;
    const SetOfByte& TRG_GetMasterPageVisibleLayers() const;
    void TRG_SetMasterPageVisibleLayers(const SetOfByte& rNew);
    sdr::contact::ViewContact& TRG_GetMasterPageDescriptorViewContact() const;

//#if 0 // _SOLAR__PRIVATE
protected:
    void TRG_ImpMasterPageRemoved(const SdrPage& rRemovedPage);
//#endif // __PRIVATE
public:

    // Aenderungen an den Layern setzen nicht das Modified-Flag !
    const         SdrLayerAdmin& GetLayerAdmin() const                  { return *pLayerAdmin; }
                  SdrLayerAdmin& GetLayerAdmin()                        { return *pLayerAdmin; }

    // Bestimmung der FuellFarbe an einer bestimmten Position.
    // FALSE=Kein Objekt mit FuellFarbe an dieser Position gefunden.
    // rVisLayers gibt die zu durchsuchenden Layer an.
    // bLayerSorted: TRUE=Es wird in der Reihenfolge der Layer gesucht (ni)
    // rCol: Hier wird die gefundene Farbe zurueckgegeben
    // Auch MasterPages werden durchsucht.
    FASTBOOL GetFillColor(const Point& rPnt, const SetOfByte& rVisLayers,
        /*FASTBOOL bLayerSorted,*/ Color& rCol) const;

    // GetBitmap und GetMetafile sind noch nicht implementiert.
    // Bitmap in Bildschirmaufloesung und -farbtiefe aus den Objekten der
    // Page erzeugen.
    Bitmap        GetBitmap(FASTBOOL bTrimBorders=TRUE) const               { return GetBitmap(aPrefVisiLayers,bTrimBorders); }
    Bitmap        GetBitmap(const SetOfByte& rVisibleLayers, FASTBOOL bTrimBorders=TRUE) const;
    // Metafile aus den Objekten der Page erzeugen
    GDIMetaFile   GetMetaFile(FASTBOOL bTrimBorders=TRUE)                   { return GetMetaFile(aPrefVisiLayers,bTrimBorders); }
    GDIMetaFile   GetMetaFile(const SetOfByte& rVisibleLayers, FASTBOOL bTrimBorders=TRUE);

    virtual String GetLayoutName() const;

    // fuer's Raster im Writer, auch fuer AlignObjects wenn 1 Objekt markiert ist
    // wenn pRect!=NULL, dann die Seiten, die von diesem Rect intersected werden
    // ansonsten die sichtbaren Seiten.
    virtual const SdrPageGridFrameList* GetGridFrameList(const SdrPageView* pPV, const Rectangle* pRect) const;
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
    void       SetBackgroundObj( SdrObject* pObj );

    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > getUnoPage();

    virtual SfxStyleSheet* GetTextStyleSheetForObject( SdrObject* pObj ) const;

    FASTBOOL HasTransparentObjects( BOOL bCheckForAlphaChannel = FALSE ) const;

    /** *deprecated* returns an averaged background color of this page */
    // #i75566# GetBackgroundColor -> GetPageBackgroundColor
    Color GetPageBackgroundColor() const;

    /** *deprecated* returns an averaged background color of this page */
    // #i75566# GetBackgroundColor -> GetPageBackgroundColor and bScreenDisplay hint value
    Color GetPageBackgroundColor( SdrPageView* pView, bool bScreenDisplay = true) const;

    /** this method returns true if the object from the ViewObjectContact should
        be visible on this page while rendering.
        bEdit selects if visibility test is for an editing view or a final render,
        like printing.
    */
    virtual bool checkVisibility(
        ::sdr::contact::ViewObjectContact& rOriginal,
        ::sdr::contact::DisplayInfo& rDisplayInfo,
        bool bEdit );

//////////////////////////////////////////////////////////////////////////////
// sdr::Comment interface
private:
    sdr::CommentVector                                  maComments;

public:
    sal_uInt32 GetCommentCount() const { return maComments.size(); }
    const sdr::Comment& GetCommentByIndex(sal_uInt32 nIndex);
    void AddComment(const sdr::Comment& rNew);
    void ReplaceCommentByIndex(sal_uInt32 nIndex, const sdr::Comment& rNew);

//////////////////////////////////////////////////////////////////////////////
};

typedef tools::WeakReference< SdrPage > SdrPageWeakRef;

//////////////////////////////////////////////////////////////////////////////
// use new redirector instead of pPaintProc

class SVX_DLLPUBLIC StandardCheckVisisbilityRedirector : public ::sdr::contact::ViewObjectContactRedirector
{
public:
    StandardCheckVisisbilityRedirector();
    virtual ~StandardCheckVisisbilityRedirector();

    // all default implementations just call the same methods at the original. To do something
    // different, overload the method and at least do what the method does.
    virtual void PaintObject(::sdr::contact::ViewObjectContact& rOriginal, ::sdr::contact::DisplayInfo& rDisplayInfo);
};

//////////////////////////////////////////////////////////////////////////////

#endif //_SVDPAGE_HXX


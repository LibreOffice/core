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
#include <tools/weakbase.hxx>
#include <cppuhelper/weakref.hxx>
#include <svx/svdtypes.hxx>
#include <svx/svdlayer.hxx>
#include <vector>
#include <svx/sdrpageuser.hxx>
#include <svx/sdr/contact/viewobjectcontactredirector.hxx>
#include <svx/sdrmasterpagedescriptor.hxx>
#include "svx/svxdllapi.h"
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <svx/svdobj.hxx>
#include <boost/scoped_ptr.hpp>

//////////////////////////////////////////////////////////////////////////////
// predefines

namespace reportdesign { class OSection; }
namespace sdr { namespace contact { class ViewContact; }}
class SdrPage;
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

//////////////////////////////////////////////////////////////////////////////
// class SdrObjList

class SVX_DLLPUBLIC SdrObjList
{
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
    bool    bObjOrdNumsDirty;
    bool    bRectsDirty;
protected:
    virtual void RecalcRects();

private:
    /// simple ActionChildInserted forwarder to have it on a central place
    void impChildInserted(SdrObject& rChild) const;
public:
    TYPEINFO();
    SdrObjList(SdrModel* pNewModel, SdrPage* pNewPage, SdrObjList* pNewUpList=NULL);
    SdrObjList(const SdrObjList& rSrcList);
    virtual ~SdrObjList();
    // !!! Diese Methode nur fuer Leute, die ganz genau wissen was sie tun !!!

    // #110094# This should not be needed (!)
    void SetObjOrdNumsDirty()                           { bObjOrdNumsDirty=sal_True; }
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
    bool IsObjOrdNumsDirty() const        { return bObjOrdNumsDirty; }
    virtual void NbcInsertObject(SdrObject* pObj, sal_uIntPtr nPos=0xFFFF
                                 , const SdrInsertReason* pReason=NULL
                                                                      );
    virtual void InsertObject(SdrObject* pObj, sal_uIntPtr nPos=0xFFFF
                              , const SdrInsertReason* pReason=NULL
                                                                     );
    // aus Liste entfernen ohne delete
    virtual SdrObject* NbcRemoveObject(sal_uIntPtr nObjNum);
    virtual SdrObject* RemoveObject(sal_uIntPtr nObjNum);
    // Vorhandenes Objekt durch ein anderes ersetzen.
    // Wie Remove&Insert jedoch performanter, da die Ordnungsnummern
    // nicht Dirty gesetzt werden muessen.
    virtual SdrObject* NbcReplaceObject(SdrObject* pNewObj, sal_uIntPtr nObjNum);
    virtual SdrObject* ReplaceObject(SdrObject* pNewObj, sal_uIntPtr nObjNum);
    // Die Z-Order eines Objekts veraendern
    virtual SdrObject* NbcSetObjectOrdNum(sal_uIntPtr nOldObjNum, sal_uIntPtr nNewObjNum);
    virtual SdrObject* SetObjectOrdNum(sal_uIntPtr nOldObjNum, sal_uIntPtr nNewObjNum);

    virtual void SetRectsDirty();

    const Rectangle& GetAllObjSnapRect() const;
    const Rectangle& GetAllObjBoundRect() const;

    // Alle Textobjekte neu formatieren, z.B. bei Druckerwechsel
    void NbcReformatAllTextObjects();
    void ReformatAllTextObjects();

    /** #103122# reformats all edge objects that are connected to other objects */
    void ReformatAllEdgeObjects();

    // Die Vorlagenattribute der Zeichenobjekte in harte Attribute verwandeln.
    void BurnInStyleSheetAttributes();

    sal_uIntPtr      GetObjCount() const;
    SdrObject* GetObj(sal_uIntPtr nNum) const;

    // Gelinkte Seite oder gelinktes Gruppenobjekt
    virtual bool IsReadOnly() const;

    // Zaehlt alle Objekte inkl. Objekte in Objektgruppen, ...
    sal_uIntPtr   CountAllObjects() const;

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
    virtual void UnGroupObj( sal_uIntPtr nObjNum );

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

//////////////////////////////////////////////////////////////////////////////
// class SdrPageGridFrame

// Fuer das Fangraster/Punkgitter im Writer
class SdrPageGridFrame
{
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
    std::vector<SdrPageGridFrame*> aList;
private:
    SVX_DLLPRIVATE SdrPageGridFrameList(const SdrPageGridFrameList& rSrcList);      // never implemented
    SVX_DLLPRIVATE void           operator=(const SdrPageGridFrameList& rSrcList);  // never implemented
protected:
    SdrPageGridFrame* GetObject(sal_uInt16 i) const { return aList[i]; }
public:
    SdrPageGridFrameList(): aList()                                    {}
    ~SdrPageGridFrameList()                                            { Clear(); }
    void           Clear();
    sal_uInt16         GetCount() const                                    { return sal_uInt16(aList.size()); }
    void           Insert(const SdrPageGridFrame& rGF) { aList.push_back(new SdrPageGridFrame(rGF)); }
    void           Insert(const SdrPageGridFrame& rGF, sal_uInt16 nPos)
    {
        if(nPos==0xFFFF)
            aList.push_back(new SdrPageGridFrame(rGF));
        else
            aList.insert(aList.begin()+nPos,new SdrPageGridFrame(rGF));
    }
    void           Delete(sal_uInt16 nPos)
    {
        SdrPageGridFrame* p = aList[nPos];
        aList.erase(aList.begin()+nPos);
        delete p;
    }
    void           Move(sal_uInt16 nPos, sal_uInt16 nNewPos)
    {
        SdrPageGridFrame* p = aList[nPos];
        aList.erase(aList.begin()+nPos);
        aList.insert(aList.begin()+nNewPos,p);
    }
    SdrPageGridFrame&       operator[](sal_uInt16 nPos)                    { return *GetObject(nPos); }
    const SdrPageGridFrame& operator[](sal_uInt16 nPos) const              { return *GetObject(nPos); }
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// class SdrPageProperties

class SVX_DLLPUBLIC SdrPageProperties : public SfxListener
{
private:
    // data
    SdrPage*                mpSdrPage;
    SfxStyleSheet*          mpStyleSheet;
    SfxItemSet*             mpProperties;

    // internal helpers
    void ImpRemoveStyleSheet();
    void ImpAddStyleSheet(SfxStyleSheet& rNewStyleSheet);

    // not implemented
    SdrPageProperties& operator=(const SdrPageProperties& rCandidate);

public:
    // construct/destruct
    SdrPageProperties(SdrPage& rSdrPage);
    virtual ~SdrPageProperties();

    // Notify(...) from baseclass SfxListener
    virtual void Notify(SfxBroadcaster& rBC, const SfxHint& rHint);

    // data read/write
    const SfxItemSet& GetItemSet() const;
    void PutItemSet(const SfxItemSet& rSet);
    void PutItem(const SfxPoolItem& rItem);
    void ClearItem(const sal_uInt16 nWhich = 0);

    // StyleSheet access
    void SetStyleSheet(SfxStyleSheet* pStyleSheet);
    SfxStyleSheet* GetStyleSheet() const;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// class SdrPage

class SVX_DLLPUBLIC SdrPage : public SdrObjList, public tools::WeakBase< SdrPage >
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
protected:
    virtual sdr::contact::ViewContact* CreateObjectSpecificViewContact();
public:
    sdr::contact::ViewContact& GetViewContact() const;

    // #110094# DrawContact support: Methods for handling Page changes
    void ActionChanged() const;

    // #i9076#
    friend class SdrModel;
    friend class SvxUnoDrawPagesAccess;

// this class uses its own UNO wrapper
// and thus has to set mxUnoPage (it also relies on mxUnoPage not being WeakRef)
friend class reportdesign::OSection;

    sal_Int32 nWdt;     // Seitengroesse
    sal_Int32 nHgt;     // Seitengroesse
    sal_Int32 nBordLft; // Seitenrand links
    sal_Int32 nBordUpp; // Seitenrand oben
    sal_Int32 nBordRgt; // Seitenrand rechts
    sal_Int32 nBordLwr; // Seitenrand unten

protected:
    SdrLayerAdmin*      pLayerAdmin;
private:
    SdrPageProperties*  mpSdrPageProperties;
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > mxUnoPage;

public:
    SdrPageProperties& getSdrPageProperties() { return *mpSdrPageProperties; }
    const SdrPageProperties& getSdrPageProperties() const { return *mpSdrPageProperties; }
    const SdrPageProperties* getCorrectSdrPageProperties() const;

protected:
    // new MasterPageDescriptorVector
    ::sdr::MasterPageDescriptor*                    mpMasterPageDescriptor;

    SetOfByte  aPrefVisiLayers;
    sal_uInt16     nPageNum;

    // bitfield
    unsigned            mbMaster : 1;               // flag if this is a MasterPage
    unsigned            mbInserted : 1;
    unsigned            mbObjectsNotPersistent : 1;
    unsigned            mbSwappingLocked : 1;

    // #i93597#
    unsigned            mbPageBorderOnlyLeftRight : 1;

    void SetUnoPage(::com::sun::star::uno::Reference<
                        ::com::sun::star::drawing::XDrawPage> const&);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > createUnoPage();

public:
    TYPEINFO();
    SdrPage(SdrModel& rNewModel, bool bMasterPage=false);
    // Copy-Ctor und Zuweisungeoperator sind nicht getestet!
    SdrPage(const SdrPage& rSrcPage);
    virtual ~SdrPage();
    // pModel, pPage, pUpList, pOwnerObj und mbInserted werden Zuweisungeoperator nicht veraendert!
    SdrPage& operator=(const SdrPage& rSrcPage);
    virtual SdrPage* Clone() const;
    virtual SdrPage* Clone(SdrModel* pNewModel) const;
    bool IsMasterPage() const       { return mbMaster; }
    void SetInserted(bool bNew = true);
    bool IsInserted() const         { return mbInserted; }
    virtual void SetChanged();

    // #i68775# React on PageNum changes (from Model in most cases)
    void SetPageNum(sal_uInt16 nNew);
    sal_uInt16 GetPageNum() const;

    // #i93597# Allow page border definition to not be the full rectangle but to
    // use only the left and right vertical edges (reportdesigner)
    void setPageBorderOnlyLeftRight(bool bNew) { mbPageBorderOnlyLeftRight = bNew; }
    bool getPageBorderOnlyLeftRight() const { return mbPageBorderOnlyLeftRight; }

    virtual void SetSize(const Size& aSiz);
    virtual Size GetSize() const;
    virtual void SetOrientation(Orientation eOri);
    virtual Orientation GetOrientation() const;
    virtual sal_Int32 GetWdt() const;
    virtual sal_Int32 GetHgt() const;
    virtual void  SetBorder(sal_Int32 nLft, sal_Int32 nUpp, sal_Int32 nRgt, sal_Int32 Lwr);
    virtual void  SetLftBorder(sal_Int32 nBorder);
    virtual void  SetUppBorder(sal_Int32 nBorder);
    virtual void  SetRgtBorder(sal_Int32 nBorder);
    virtual void  SetLwrBorder(sal_Int32 nBorder);
    virtual sal_Int32 GetLftBorder() const;
    virtual sal_Int32 GetUppBorder() const;
    virtual sal_Int32 GetRgtBorder() const;
    virtual sal_Int32 GetLwrBorder() const;

    virtual void SetModel(SdrModel* pNewModel);

    // New MasterPage interface
    sal_Bool TRG_HasMasterPage() const { return (0L != mpMasterPageDescriptor); }
    void TRG_SetMasterPage(SdrPage& rNew);
    void TRG_ClearMasterPage();
    SdrPage& TRG_GetMasterPage() const;
    const SetOfByte& TRG_GetMasterPageVisibleLayers() const;
    void TRG_SetMasterPageVisibleLayers(const SetOfByte& rNew);
    sdr::contact::ViewContact& TRG_GetMasterPageDescriptorViewContact() const;

protected:
    void TRG_ImpMasterPageRemoved(const SdrPage& rRemovedPage);
public:

    // Aenderungen an den Layern setzen nicht das Modified-Flag !
    const         SdrLayerAdmin& GetLayerAdmin() const                  { return *pLayerAdmin; }
                  SdrLayerAdmin& GetLayerAdmin()                        { return *pLayerAdmin; }

    virtual OUString GetLayoutName() const;

    // fuer's Raster im Writer, auch fuer AlignObjects wenn 1 Objekt markiert ist
    // wenn pRect!=NULL, dann die Seiten, die von diesem Rect intersected werden
    // ansonsten die sichtbaren Seiten.
    virtual const SdrPageGridFrameList* GetGridFrameList(const SdrPageView* pPV, const Rectangle* pRect) const;
    bool IsObjectsNotPersistent() const          { return mbObjectsNotPersistent; }
    void SetObjectsNotPersistent(bool b)     { mbObjectsNotPersistent = b; }
    // Durch Setzen dieses Flags, kann das Auslagern (Swappen) von
    // Teilen der Page (z.B. Grafiken) unterbunden werden.
    // Es werden hierdurch jedoch nicht automatisch alle ausgelagerten
    // Teile nachgeladen, dies geschieht erst bei konkretem Bedarf oder
    // durch Aufruf von SwapInAll().
    // Fuer die MasterPage(s) der Page muss dies ggf. separat gemacht werden.
    bool IsSwappingLocked() const { return mbSwappingLocked; }
    void SetSwappingLocked(bool bLock) { mbSwappingLocked = bLock; }

    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > getUnoPage();

    virtual SfxStyleSheet* GetTextStyleSheetForObject( SdrObject* pObj ) const;

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
        const sdr::contact::ViewObjectContact& rOriginal,
        const sdr::contact::DisplayInfo& rDisplayInfo,
        bool bEdit );
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
    virtual drawinglayer::primitive2d::Primitive2DSequence createRedirectedPrimitive2DSequence(
        const sdr::contact::ViewObjectContact& rOriginal,
        const sdr::contact::DisplayInfo& rDisplayInfo);
};

//////////////////////////////////////////////////////////////////////////////

#endif //_SVDPAGE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

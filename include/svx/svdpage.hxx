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

#ifndef INCLUDED_SVX_SVDPAGE_HXX
#define INCLUDED_SVX_SVDPAGE_HXX

#include <svl/stylesheetuser.hxx>
#include <vcl/bitmap.hxx>
#include <vcl/print.hxx>
#include <vcl/gdimtf.hxx>
#include <tools/weakbase.hxx>
#include <tools/contnr.hxx>
#include <cppuhelper/weakref.hxx>
#include <svx/svdtypes.hxx>
#include <svx/sdrpageuser.hxx>
#include <svx/sdr/contact/viewobjectcontactredirector.hxx>
#include <svx/sdrmasterpagedescriptor.hxx>
#include <svx/svxdllapi.h>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <svx/svdobj.hxx>
#include <memory>
#include <vector>


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

enum SdrInsertReasonKind {
    SDRREASON_UNKNOWN,
    SDRREASON_STREAMING,  /// importing document
    SDRREASON_UNDO,       /// from Undo
    SDRREASON_COPY,       /// something copied...
    SDRREASON_VIEWCREATE, /// created by User interactively
    SDRREASON_VIEWCALL    /// via SdrView::Group(), ...
};

class SdrInsertReason {
    SdrInsertReasonKind eReason;
public:
    SdrInsertReason(SdrInsertReasonKind eR): eReason(eR) {}

    SdrInsertReasonKind GetReason() const         { return eReason; }
};


// class SdrObjList

class SVX_DLLPUBLIC SdrObjList
{
    SdrObjList(const SdrObjList& rSrcList) = delete;
    SdrObjList &operator=(const SdrObjList& rSrcList) = delete;

private:
    ::std::vector<SdrObject*> maList;

protected:
friend class SdrObjListIter;
friend class SdrEditView;
    SdrObjList* pUpList;   /// parent list
    SdrModel*   pModel;    /// model to which the list belongs (Layer,ItemPool,Storage)
    SdrPage*    pPage;     /// Page containing the list, may be "this".
    SdrObject*  pOwnerObj; /// OwnerObject, if it's list of a Group object.
    Rectangle   aOutRect;
    Rectangle   aSnapRect;
    SdrObjListKind eListKind;
    bool        bObjOrdNumsDirty;
    bool        bRectsDirty;
protected:
    void RecalcRects();

    SdrObjList();
    void lateInit(const SdrObjList& rSrcList);

private:
    /// simple ActionChildInserted forwarder to have it on a central place
    static void impChildInserted(SdrObject& rChild);
public:
    SdrObjList(SdrModel* pNewModel, SdrPage* pNewPage, SdrObjList* pNewUpList=nullptr);
    virtual ~SdrObjList();

    virtual SdrObjList* Clone() const;

    void CopyObjects(const SdrObjList& rSrcList);
    /// clean up everything (without Undo)
    void    Clear();
    SdrObjListKind GetListKind() const                  { return eListKind; }
    void           SetListKind(SdrObjListKind eNewKind) { eListKind=eNewKind; }
    SdrObjList*    GetUpList() const                    { return pUpList; }
    void           SetUpList(SdrObjList* pNewUpList)    { pUpList=pNewUpList; }
    SdrObject*     GetOwnerObj() const                  { return pOwnerObj; }
    void           SetOwnerObj(SdrObject* pNewOwner)    { pOwnerObj=pNewOwner; }
    SdrPage*       GetPage() const;
    void           SetPage(SdrPage* pNewPage);
    SdrModel*      GetModel() const;
    virtual void   SetModel(SdrModel* pNewModel);
    /// recalculate order numbers / ZIndex
    void           RecalcObjOrdNums();
    bool           IsObjOrdNumsDirty() const        { return bObjOrdNumsDirty; }
    virtual void   NbcInsertObject(SdrObject* pObj, size_t nPos=SAL_MAX_SIZE,
                                   const SdrInsertReason* pReason=nullptr);
    virtual void   InsertObject(SdrObject* pObj, size_t nPos=SAL_MAX_SIZE,
                                const SdrInsertReason* pReason=nullptr);
    /// remove from list without delete
    virtual SdrObject* NbcRemoveObject(size_t nObjNum);
    virtual SdrObject* RemoveObject(size_t nObjNum);
    /// Replace existing object by different one.
    /// Same as Remove(old)+Insert(new) but faster because the order numbers
    /// do not have to be set dirty.
    virtual SdrObject* NbcReplaceObject(SdrObject* pNewObj, size_t nObjNum);
    virtual SdrObject* ReplaceObject(SdrObject* pNewObj, size_t nObjNum);
    /// Modify ZOrder of an SdrObject
    virtual SdrObject* SetObjectOrdNum(size_t nOldObjNum, size_t nNewObjNum);

    void SetRectsDirty();

    const Rectangle& GetAllObjSnapRect() const;
    const Rectangle& GetAllObjBoundRect() const;

    /// reformat all text objects, e.g. when changing printer
    void NbcReformatAllTextObjects();
    void ReformatAllTextObjects();

    /** #103122# reformats all edge objects that are connected to other objects */
    void ReformatAllEdgeObjects();

    /// convert attributes of the style to hard formatting
    void BurnInStyleSheetAttributes();

    size_t GetObjCount() const;
    SdrObject* GetObj(size_t nNum) const;

    /// linked page or linked group object
    virtual bool IsReadOnly() const;

    /// count all objects including objects in Groups
    size_t CountAllObjects() const;

    /** Makes the object list flat, i.e. the object list content are
        then tree leaves

        This method travels recursively over all group objects in this
        list, extracts the content, inserts it flat to the list and
        removes the group object afterwards.
     */
    void FlattenGroups();
    /** Ungroup the object at the given index

        This method ungroups the content of the group object at the
        given index, i.e. the content is put flat into the object list
        (if the object at the given index is no group, this method is
        a no-op). If the group itself contains group objects, the
        operation is performed recursively, such that the content of
        the given object contains no groups afterwards.
     */
    void UnGroupObj( size_t nObjNum );

    /** Return whether there is an explicit, user defined, object navigation
        order.  When there is one this method returns <TRUE/> and the
        GetObjectForNavigationPosition() and
        SdrObject::GetNavigationPosition() methods will return values
        different from those returne by SdrObject::GetOrdNum() and
        GetObj().
    */
    bool HasObjectNavigationOrder() const;

    /** Set the navigation position of the given object to the specified
        value.  Note that this changes the navigation position for all
        objects on or following the old or new position.
    */
    void SetObjectNavigationPosition (
        SdrObject& rObject,
        const sal_uInt32 nNewNavigationPosition);

    /** Return the object for the given navigation position.  When there is
        a user defined navigation order, i.e. mxNavigationOrder is not NULL,
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
    void ClearObjectNavigationOrder();

    /** Set the navigation position of all SdrObjects to their position in
        the mxNavigationOrder list.  This method returns immediately when no
        update is necessary.
        @return
            This method returns <TRUE/> when the navigation positions stored
            in SdrObjects are up to date.
            It returns <FALSE/> when the navigation positions are not valid,
            for example because no explicit navigation order has been
            defined, i.e. HasObjectNavigationOrder() would return <FALSE/>.
    */
    bool RecalcNavigationPositions();

    /** Set the navigation order to the one defined by the given list of
        XShape objects.
        @param rxOrder
            When this is an empty reference then the navigation order is
            reset to the z-order. The preferred way to do this, however, is
            to call ClearObjectNavigationOrder().
            Otherwise this list is expected to contain all the shapes in the
            called SdrObjList.
    */
    void SetNavigationOrder (const css::uno::Reference<
                             css::container::XIndexAccess>& rxOrder);

    virtual void dumpAsXml(struct _xmlTextWriter* pWriter) const;

private:
    class WeakSdrObjectContainerType;
    /// This list, if it exists, defines the navigation order. If it does
    /// not exist then maList defines the navigation order.
    std::unique_ptr<WeakSdrObjectContainerType> mxNavigationOrder;

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

// Used for all methods which return a page number
#define SDRPAGE_NOTFOUND 0xFFFF


// class SdrPageGridFrame

/// for the snap-to-grid in Writer
class SdrPageGridFrame
{
    Rectangle aPaper;
    Rectangle aUserArea;
public:
    SdrPageGridFrame(const Rectangle& rPaper, const Rectangle& rUser): aPaper(rPaper), aUserArea(rUser) {}
    const Rectangle& GetPaperRect() const                  { return aPaper; }
    const Rectangle& GetUserArea() const                   { return aUserArea; }
};

class SVX_DLLPUBLIC SdrPageGridFrameList {
    std::vector<SdrPageGridFrame*> aList;
private:
    SdrPageGridFrameList(const SdrPageGridFrameList& rSrcList) = delete;
    void           operator=(const SdrPageGridFrameList& rSrcList) = delete;
protected:
    SdrPageGridFrame* GetObject(sal_uInt16 i) const { return aList[i]; }
public:
    SdrPageGridFrameList(): aList()                                    {}
    ~SdrPageGridFrameList()                                            { Clear(); }
    void           Clear();
    sal_uInt16     GetCount() const                                    { return sal_uInt16(aList.size()); }
    void           Insert(const SdrPageGridFrame& rGF) { aList.push_back(new SdrPageGridFrame(rGF)); }
    SdrPageGridFrame&       operator[](sal_uInt16 nPos)                    { return *GetObject(nPos); }
    const SdrPageGridFrame& operator[](sal_uInt16 nPos) const              { return *GetObject(nPos); }
};


// class SdrPageProperties

class SVX_DLLPUBLIC SdrPageProperties : public SfxListener, public svl::StyleSheetUser
{
private:
    // data
    SdrPage*                mpSdrPage;
    SfxStyleSheet*          mpStyleSheet;
    SfxItemSet*             mpProperties;

    // internal helpers
    void ImpRemoveStyleSheet();
    void ImpAddStyleSheet(SfxStyleSheet& rNewStyleSheet);

    SdrPageProperties& operator=(const SdrPageProperties& rCandidate) = delete;

public:
    // construct/destruct
    SdrPageProperties(SdrPage& rSdrPage);
    virtual ~SdrPageProperties();

    // Notify(...) from baseclass SfxListener
    virtual void Notify(SfxBroadcaster& rBC, const SfxHint& rHint) override;

    virtual bool isUsedByModel() const override;

    // data read/write
    const SfxItemSet& GetItemSet() const { return *mpProperties;}
    void PutItemSet(const SfxItemSet& rSet);
    void PutItem(const SfxPoolItem& rItem);
    void ClearItem(const sal_uInt16 nWhich = 0);

    // StyleSheet access
    void SetStyleSheet(SfxStyleSheet* pStyleSheet);
    SfxStyleSheet* GetStyleSheet() const { return mpStyleSheet;}
};


/**
  A SdrPage contains exactly one SdrObjList and a description of the physical
  page dimensions (size / margins).  The latter is required to "catch" objects
  during drag-and-drop.
  The SdrPage allows (via SdrObjList) inserting and removing SdrObjects,
  as well as moving them into the foreground or background.
  Also it's possible to request and directly set the order number (ZOrder)
  of SdrObjects.
*/
class SVX_DLLPUBLIC SdrPage : public SdrObjList, public tools::WeakBase< SdrPage >
{
    // #i9076#
    friend class SdrModel;
    friend class SvxUnoDrawPagesAccess;

    // this class uses its own UNO wrapper
    // and thus has to set mxUnoPage (it also relies on mxUnoPage not being WeakRef)
    friend class reportdesign::OSection;

    SdrPage& operator=(const SdrPage& rSrcPage) = delete;

    // start PageUser section
private:
    // #111111# PageUser section
    sdr::PageUserVector                                             maPageUsers;

    std::unique_ptr<sdr::contact::ViewContact> mpViewContact;

public:
    void AddPageUser(sdr::PageUser& rNewUser);
    void RemovePageUser(sdr::PageUser& rOldUser);

protected:
    sdr::contact::ViewContact* CreateObjectSpecificViewContact();
public:
    const sdr::contact::ViewContact& GetViewContact() const;
    sdr::contact::ViewContact& GetViewContact();

    // #110094# DrawContact support: Methods for handling Page changes
    void ActionChanged();

private:
    sal_Int32 nWdt;     // Seitengroesse
    sal_Int32 nHgt;     // Seitengroesse
    sal_Int32 nBordLft; // Seitenrand links
    sal_Int32 nBordUpp; // Seitenrand oben
    sal_Int32 nBordRgt; // Seitenrand rechts
    sal_Int32 nBordLwr; // Seitenrand unten

    std::unique_ptr<SdrLayerAdmin> mpLayerAdmin;
    std::unique_ptr<SdrPageProperties> mpSdrPageProperties;
    css::uno::Reference< css::uno::XInterface > mxUnoPage;

public:
    SdrPageProperties& getSdrPageProperties();
    const SdrPageProperties& getSdrPageProperties() const;
    const SdrPageProperties* getCorrectSdrPageProperties() const;

protected:
    // new MasterPageDescriptorVector
    sdr::MasterPageDescriptor*                    mpMasterPageDescriptor;

    SetOfByte           aPrefVisiLayers;
    sal_uInt16          nPageNum;

    // bitfield
    bool                mbMaster : 1;               // flag if this is a MasterPage
    bool                mbInserted : 1;
    bool                mbObjectsNotPersistent : 1;

    // #i93597#
    bool                mbPageBorderOnlyLeftRight : 1;

    void                SetUnoPage(css::uno::Reference<
                                   css::drawing::XDrawPage> const&);
    virtual css::uno::Reference< css::uno::XInterface > createUnoPage();

    // Copying of pages is split into two parts: construction and copying of page objects,
    // because the copying might need access to fully initialized page. Clone() is responsible
    // to call lateInit() after copy-construction of a new object. Any initialization in derived
    // classes that needs access to the page objects must be deferred to lateInit. And it must
    // call lateInit() of its parent class.
    SdrPage(const SdrPage& rSrcPage);
    void lateInit(const SdrPage& rSrcPage, SdrModel* pNewModel = nullptr);

public:
    explicit SdrPage(SdrModel& rNewModel, bool bMasterPage=false);
    virtual ~SdrPage();
    virtual SdrPage* Clone() const override;
    virtual SdrPage* Clone(SdrModel* pNewModel) const;
    bool             IsMasterPage() const       { return mbMaster; }
    void             SetInserted(bool bNew = true);
    bool             IsInserted() const         { return mbInserted; }
    void             SetChanged();

    // #i68775# React on PageNum changes (from Model in most cases)
    void SetPageNum(sal_uInt16 nNew);
    sal_uInt16 GetPageNum() const;

    // #i93597# Allow page border definition to not be the full rectangle but to
    // use only the left and right vertical edges (reportdesigner)
    void setPageBorderOnlyLeftRight(bool bNew) { mbPageBorderOnlyLeftRight = bNew; }
    bool getPageBorderOnlyLeftRight() const { return mbPageBorderOnlyLeftRight; }

    virtual void SetSize(const Size& aSiz);
    Size GetSize() const;
    virtual void SetOrientation(Orientation eOri);
    virtual Orientation GetOrientation() const;
    sal_Int32 GetWdt() const;
    sal_Int32 GetHgt() const;
    virtual void  SetBorder(sal_Int32 nLft, sal_Int32 nUpp, sal_Int32 nRgt, sal_Int32 Lwr);
    virtual void  SetLftBorder(sal_Int32 nBorder);
    virtual void  SetUppBorder(sal_Int32 nBorder);
    virtual void  SetRgtBorder(sal_Int32 nBorder);
    virtual void  SetLwrBorder(sal_Int32 nBorder);
    sal_Int32 GetLftBorder() const;
    sal_Int32 GetUppBorder() const;
    sal_Int32 GetRgtBorder() const;
    sal_Int32 GetLwrBorder() const;

    virtual void SetModel(SdrModel* pNewModel) override;

    // New MasterPage interface
    bool TRG_HasMasterPage() const { return (nullptr != mpMasterPageDescriptor); }
    void TRG_SetMasterPage(SdrPage& rNew);
    void TRG_ClearMasterPage();
    SdrPage& TRG_GetMasterPage() const;
    const SetOfByte& TRG_GetMasterPageVisibleLayers() const;
    void TRG_SetMasterPageVisibleLayers(const SetOfByte& rNew);
    sdr::contact::ViewContact& TRG_GetMasterPageDescriptorViewContact() const;

protected:
    void TRG_ImpMasterPageRemoved(const SdrPage& rRemovedPage);
public:

    /// changing the layers does not set the modified-flag!
    const SdrLayerAdmin& GetLayerAdmin() const;
    SdrLayerAdmin& GetLayerAdmin();

    virtual OUString GetLayoutName() const;

    /// for snap-to-grid in Writer, also for AlignObjects if 1 object is marked
    /// if pRect != null, then the pages that are intersected by this Rect,
    /// otherwise the visible pages
    virtual const SdrPageGridFrameList* GetGridFrameList(const SdrPageView* pPV, const Rectangle* pRect) const;

    css::uno::Reference< css::uno::XInterface > const & getUnoPage();

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

private:
    void impl_setModelForLayerAdmin(SdrModel* pNewModel);
};

typedef tools::WeakReference< SdrPage > SdrPageWeakRef;


// use new redirector instead of pPaintProc

class SVX_DLLPUBLIC StandardCheckVisisbilityRedirector : public sdr::contact::ViewObjectContactRedirector
{
public:
    StandardCheckVisisbilityRedirector();
    virtual ~StandardCheckVisisbilityRedirector();

    // all default implementations just call the same methods at the original. To do something
    // different, override the method and at least do what the method does.
    virtual drawinglayer::primitive2d::Primitive2DContainer createRedirectedPrimitive2DSequence(
        const sdr::contact::ViewObjectContact& rOriginal,
        const sdr::contact::DisplayInfo& rDisplayInfo) override;
};


#endif // INCLUDED_SVX_SVDPAGE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

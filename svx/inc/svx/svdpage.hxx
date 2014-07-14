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



#ifndef _SVDPAGE_HXX
#define _SVDPAGE_HXX

#include <vcl/bitmap.hxx>
#include <vcl/print.hxx>
#include <vcl/gdimtf.hxx>
#include <tools/weakbase.hxx>
#include <cppuhelper/weakref.hxx>
#include <svx/svdtypes.hxx>
#include <svx/svdlayer.hxx>
#include <svx/sdrcomment.hxx>
#include <vector>
#include <svx/sdr/contact/viewobjectcontactredirector.hxx>
#include <svx/sdrmasterpagedescriptor.hxx>
#include "svx/svxdllapi.h"
#include <com/sun/star/container/XIndexAccess.hpp>
#include <svx/svdobj.hxx>
#include <boost/scoped_ptr.hpp>
#include <svl/brdcst.hxx>

//////////////////////////////////////////////////////////////////////////////
// predefines

namespace sdr { namespace contact { class ViewContact; }}
class SdrPage;
class SdrModel;
class SdrPageView;
class SdrLayerAdmin;
class SetOfByte;
class Color;
class SfxStyleSheet;

//////////////////////////////////////////////////////////////////////////////
// defines

#define SDRPAGE_NOTFOUND 0xffffffff

//////////////////////////////////////////////////////////////////////////////

class SVX_DLLPUBLIC SdrObjList
:   private boost::noncopyable
{
private:
    /// the regular SdrObjList
    SdrObjectVector     maList;

    /// the SdrObjList used when UserNavigationOrder is activated. Empty when
    /// unused
    SdrObjectVector     maUserNavigationOrder;

    /** Insert an SdrObject into maList

        @param rObject
            The object to insert into the object list.

        @param nInsertPosition
            The given object is inserted before the object at this
            position
    */
    void InsertObjectIntoContainer(SdrObject& rObject, const sal_uInt32 nInsertPosition);

    /** Remove an object from the object list.

        @param nObjectPosition
            The object at this position is removed from the object list.
            Valid values include 0 and are smaller than the number of
            objects in the list.
    */
    void RemoveObjectFromContainer(const sal_uInt32 nObjectPosition);

    // recalculate OrdNums
    void EnsureValidNavigationPositions(sal_uInt32 nFrom = 0);

protected:
    // protected constructor to make clear that this class should only
    // be used as base for derivations, not naked. See getSdrModelFromSdrObjList
    // which is pure virtual to force this, too
    SdrObjList();

    /// method to copy all data from given source. This includes
    /// cloning all SdrObjects
    void copyDataFromSdrObjList(const SdrObjList& rSource);

public:
    virtual ~SdrObjList();

    /// delete content and reset. This will destroy all SdrObjects
    void ClearSdrObjList();

    /// helper for unified access e.g. for std::legacy methods. This will
    /// return maUserNavigationOrder if used
    SdrObjectVector getSdrObjectVector() const;

    // SdrModel/SdrPage access on SdrObjList level
    virtual SdrPage* getSdrPageFromSdrObjList() const;
    virtual SdrObject* getSdrObjectFromSdrObjList() const;
    virtual SdrModel& getSdrModelFromSdrObjList() const = 0;

    // react on content change
    virtual void handleContentChange(const SfxHint& rHint);

    // insert SdrObject
    virtual void InsertObjectToSdrObjList(SdrObject& rObj, sal_uInt32 nPos = CONTAINER_APPEND);

    // remove SdrObject (without delete)
    virtual SdrObject* RemoveObjectFromSdrObjList(sal_uInt32 nObjNum);

    // replace SdrObject with another one
    virtual SdrObject* ReplaceObjectInSdrObjList(SdrObject& rNewObj, sal_uInt32 nObjNum);

    // change OrdNum (exchange two objects)
    virtual SdrObject* SetNavigationPosition(sal_uInt32 nOldObjNum, sal_uInt32 nNewObjNum);

    /// access methods, will use maUserNavigationOrder if used
    sal_uInt32 GetObjCount() const { return maList.size(); }
    SdrObject* GetObj(sal_uInt32 nNum) const;

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
    virtual void UnGroupObj( sal_uInt32 nObjNum );

    /** Return whether there is an explicit, user defined, object navigation
        order.  When there is one this method returns <true/> and the
        GetObjectForNavigationPosition() and
        SdrObject::GetNavigationPosition() methods will return values
        different from those returne by SdrObject::GetOrdNum() and
        GetObj().
    */
    bool HasUserNavigationOrder(void) const;

    /** Set the navigation position of the given object to the specified
        value.  Note that this changes the navigation position for all
        objects on or following the old or new position.
        This call creates a new UserNavigationOrder if not yet active
        and changes the position only there. The original positions keep
        unchanged and will be restored when the UserNavigationOrder
        is removed again
    */
    void SetUserNavigationPosition(SdrObject& rObject, const sal_uInt32 nNewNavigationPosition);

    /** Restore the navigation order to that defined by the z-order.
    */
    void ClearUserNavigationOrder(void);

    /** Set the navigation position of all SdrObjects to their position in
        the mpNavigationOrder list.  This method returns immediately when no
        update is necessary.
        @return
            This method returns <TRUE/> when the navigation positions stored
            in SdrObjects are up to date.
            It returns <FALSE/> when the navigation positions are not valid,
            for example because no explicit navigation order has been
            defined, i.e. HasObjectNavigationOrder() would return <FALSE/>.
    */
    bool RecalcNavigationPositions (void);

    /** Set the navigation order to the one defined by the given list of
        XShape objects.
        @param rxOrder
            When this is an empty reference then the navigation order is
            reset to the z-order. The preferred way to do this, however, is
            to call ClearUserNavigationOrder().
            Otherwise this list is expected to contain all the shapes in the
            called SdrObjList.
    */
    void SetUserNavigationOrder(const ::com::sun::star::uno::Reference<
        ::com::sun::star::container::XIndexAccess>& rxOrder);
};

//////////////////////////////////////////////////////////////////////////////

class SVX_DLLPUBLIC SdrPageGridFrame
{
private:
    basegfx::B2DRange   maPaper;
    basegfx::B2DRange   maUserArea;

public:
    SdrPageGridFrame(const basegfx::B2DRange& rPaper);
    SdrPageGridFrame(const basegfx::B2DRange& rPaper, const basegfx::B2DRange& rUser);

    void SetPaperRect(const basegfx::B2DRange& rPaper) { if(maPaper != rPaper) maPaper = rPaper; }
    void SetUserArea(const basegfx::B2DRange& rUser)   { if(maUserArea != rUser) maUserArea = rUser; }

    const basegfx::B2DRange& GetPaperRect() const { return maPaper; }
    const basegfx::B2DRange& GetUserArea() const { return maUserArea; }
};

class SVX_DLLPUBLIC SdrPageGridFrameList
{
private:
    typedef ::std::vector< SdrPageGridFrame* > SdrPageGridFrameContainerType;
    SdrPageGridFrameContainerType       maList;

    SVX_DLLPRIVATE SdrPageGridFrameList(const SdrPageGridFrameList& rSrcList);
    SVX_DLLPRIVATE void operator=(const SdrPageGridFrameList& rSrcList);

protected:
    SdrPageGridFrame* GetObject(sal_uInt32 i) const;

public:
    SdrPageGridFrameList();
    ~SdrPageGridFrameList();

    void           Clear();
    sal_uInt32 GetCount() const { return maList.size(); }

    void Insert(const SdrPageGridFrame& rGF, sal_uInt32 nPos = 0xffffffff);
    void Delete(sal_uInt32 nPos);

    SdrPageGridFrame& operator[](sal_uInt32 nPos) { return *GetObject(nPos); }
    const SdrPageGridFrame& operator[](sal_uInt32 nPos) const { return *GetObject(nPos); }
};

////////////////////////////////////////////////////////////////////////////////////////////////////

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
//
// SdrPage
//   DlgEdPage
//   OReportPage
//   FmFormPage
//     ScDrawPage
//     SdPage
//     SwDPage
//

class SVX_DLLPUBLIC SdrPage
:   public SdrObjList,
    public SfxBroadcaster,
    public tools::WeakBase< SdrPage >
{
private:
    SdrModel&                           mrSdrModelFromSdrPage;

    sdr::contact::ViewContact*                                      mpViewContact;
    SdrPageProperties*                  mpSdrPageProperties;
    sdr::CommentVector                  maComments;

    virtual sdr::contact::ViewContact* CreateObjectSpecificViewContact();

    // only allow SetPageNumberAtSdrPageFromSdrModel to call SetPageNumber
    friend void SetPageNumberAtSdrPageFromSdrModel(SdrPage& rPage, sal_uInt32 nPageNum);
    SVX_DLLPRIVATE void SetPageNumber(sal_uInt32 nNew);

    // only allow SetInsertedAtSdrPageFromSdrModel to call SetInserted
    friend void SetInsertedAtSdrPageFromSdrModel(SdrPage& rPage, bool bInserted);
    void SetInserted(bool bInserted);

    // current PageNumber, trhe index in the List in the model the page is added
    sal_uInt32                          mnPageNum;

protected:
    SdrLayerAdmin*                      mpPageLayerAdmin;
    ::sdr::MasterPageDescriptor*                    mpMasterPageDescriptor;

    // this is a weak reference to a possible living api wrapper for this page
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > mxUnoPage;

    basegfx::B2DVector                  maPageScale; // page width and height
    double                              mfLeftPageBorder; // Seitenrand links
    double                              mfTopPageBorder; // Seitenrand oben
    double                              mfRightPageBorder; // Seitenrand rechts
    double                              mfBottomPageBorder; // Seitenrand unten

    // bitfield
    bool                                mbMaster : 1;               // flag if this is a MasterPage

    // inserted state of the SdrPage
    bool                                mbInserted : 1;

    // #i93597#
    bool                                mbPageBorderOnlyLeftRight : 1;

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > createUnoPage();

    /// method to copy all data from given source
    virtual void copyDataFromSdrPage(const SdrPage& rSource);

public:
    /// create a copy, evtl. with a different target model (if given)
    virtual SdrPage* CloneSdrPage(SdrModel* pTargetModel = 0) const;

    // access to model of this page
    SdrModel& getSdrModelFromSdrPage() const { return mrSdrModelFromSdrPage; }
    virtual SdrModel& getSdrModelFromSdrObjList() const;

    SdrPage(SdrModel& rNewModel, bool bMasterPage=false);
    virtual ~SdrPage();

    sdr::contact::ViewContact& GetViewContact() const;

    void AddListener(SfxListener& rListener);
    void RemoveListener(SfxListener& rListener);
    void ActionChanged() const;

    // derived from SdrObjList, returns this
    virtual SdrPage* getSdrPageFromSdrObjList() const;

    SdrPageProperties& getSdrPageProperties() { return *mpSdrPageProperties; }
    const SdrPageProperties& getSdrPageProperties() const { return *mpSdrPageProperties; }
    const SdrPageProperties* getCorrectSdrPageProperties() const;

    bool IsMasterPage() const       { return mbMaster; }
    bool IsInserted() const { return mbInserted; }
    virtual void SetChanged();

    // #i68775# React on PageNum changes (from Model in most cases)
    sal_uInt32 GetPageNumber() const;

    // #i93597# Allow page border definition to not be the full rectangle but to
    // use only the left and right vertical edges (reportdesigner)
    void setPageBorderOnlyLeftRight(bool bNew) { mbPageBorderOnlyLeftRight = bNew; }
    bool getPageBorderOnlyLeftRight() const { return mbPageBorderOnlyLeftRight; }

    // helpers for page size definitions
    basegfx::B2DRange GetInnerPageRange() const;
    basegfx::B2DVector GetInnerPageScale() const;

    // page scale read/write access
    const basegfx::B2DVector& GetPageScale() const { return maPageScale; }
    virtual void SetPageScale(const basegfx::B2DVector& aNewScale);

    // overloaded in SD since an own orientation is used there
    virtual void SetOrientation(Orientation eOri);
    virtual Orientation GetOrientation() const;

    void SetPageBorder(double fLeft, double fTop, double fRight, double fBottom);
    void SetLeftPageBorder(double fBorder);
    void SetTopPageBorder(double fBorder);
    void SetRightPageBorder(double fBorder);
    void SetBottomPageBorder(double fBorder);

    double GetLeftPageBorder() const;
    double GetTopPageBorder() const;
    double GetRightPageBorder() const;
    double GetBottomPageBorder() const;

    // New MasterPage interface
    bool TRG_HasMasterPage() const { return (0L != mpMasterPageDescriptor); }
    void TRG_SetMasterPage(SdrPage& rNew);
    void TRG_ClearMasterPage();
    SdrPage& TRG_GetMasterPage() const;
    const SetOfByte& TRG_GetMasterPageVisibleLayers() const;
    void TRG_SetMasterPageVisibleLayers(const SetOfByte& rNew);
    sdr::contact::ViewContact& TRG_GetMasterPageDescriptorViewContact() const;
    void TRG_MasterPageRemoved(const SdrPage& rRemovedPage);

    // Aenderungen an den Layern setzen nicht das Modified-Flag !
    const SdrLayerAdmin& GetPageLayerAdmin() const { return *mpPageLayerAdmin; }
    SdrLayerAdmin& GetPageLayerAdmin() { return *mpPageLayerAdmin; }

    virtual String GetLayoutName() const;

    // fuer's Raster im Writer, auch fuer AlignObjects wenn 1 Objekt markiert ist
    // wenn pRect!=0, dann die Seiten, die von diesem Rect intersected werden
    // ansonsten die sichtbaren Seiten.
    virtual const SdrPageGridFrameList* GetGridFrameList(const SdrView& rSdrView, const Rectangle* pRect) const;

    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > getUnoPage();

    virtual SfxStyleSheet* GetTextStyleSheetForObject( SdrObject* pObj ) const;

    /** *deprecated* returns an averaged background color of this page */
    // #i75566# GetBackgroundColor -> GetPageBackgroundColor and bScreenDisplay hint value
    Color GetPageBackgroundColor(SdrPageView* pView = 0, bool bScreenDisplay = true) const;

    /** this method returns true if the object from the ViewObjectContact should
        be visible on this page while rendering.
        bEdit selects if visibility test is for an editing view or a final render,
        like printing.
    */
    virtual bool checkVisibility(
        const sdr::contact::ViewObjectContact& rOriginal,
        const sdr::contact::DisplayInfo& rDisplayInfo,
        bool bEdit );

    /** Check if page is the HandoutMasterPage (in SVX, no PK_HANDOUT available) */
    bool isHandoutMasterPage() const;

    // Comment API
    sal_uInt32 GetCommentCount() const { return maComments.size(); }
    const sdr::Comment& GetCommentByIndex(sal_uInt32 nIndex);
    void AddComment(const sdr::Comment& rNew);
    void ReplaceCommentByIndex(sal_uInt32 nIndex, const sdr::Comment& rNew);
};

typedef tools::WeakReference< SdrPage > SdrPageWeakRef;

//////////////////////////////////////////////////////////////////////////////

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

//////////////////////////////////////////////////////////////////////////////
// eof

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
#ifndef INCLUDED_SW_INC_DCONTACT_HXX
#define INCLUDED_SW_INC_DCONTACT_HXX

#include <svx/svdobj.hxx>
#include <svx/svdovirt.hxx>
#include "fmtanchr.hxx"
#include "frmfmt.hxx"
#include <list>
#include <vector>
#include <memory>

#include "calbck.hxx"
#include "anchoreddrawobject.hxx"

class SwFlyFrame;
class SwFrame;
class SwPageFrame;
class SwVirtFlyDrawObj;
class SwFlyDrawObj;
class SwRect;
class SwDrawContact;
struct SwPosition;
class SdrTextObj;
class SwContact;

/** The other way round: Search format for given object.
 If object is a SwVirtFlyDrawObj the format will be obtained from it.
 If not it is a simple DrawObject. It has a UserCall which
 is client of the format we are looking for.
 Implementation in dcontact.cxx. */
SW_DLLPUBLIC SwFrameFormat *FindFrameFormat( SdrObject *pObj );
inline const SwFrameFormat *FindFrameFormat( const SdrObject *pObj )
{   return ::FindFrameFormat( const_cast<SdrObject*>(pObj) ); }
bool HasWrap( const SdrObject* pObj );

void setContextWritingMode( SdrObject* pObj, SwFrame const * pAnchor );

/// @return BoundRect plus distance.
SwRect GetBoundRectOfAnchoredObj( const SdrObject* pObj );

/// @return UserCall of goup object (if applicable).
SwContact* GetUserCall( const SdrObject* );

/// @return TRUE if the SrdObject is a Marquee object.
bool IsMarqueeTextObj( const SdrObject& rObj );

/// Base class for the following contact objects (frame + draw objects).
class SAL_DLLPUBLIC_RTTI SwContact : public SdrObjUserCall, public SwClient
{
    /** boolean, indicating destruction of contact object
     important note: boolean has to be set at the beginning of each destructor
                     in the subclasses using method <SetInDTOR()>. */
    bool mbInDTOR;

    /** method to move object to visible/invisible layer

        Implementation for the public method <MoveObjToVisibleLayer(..)>
        and <MoveObjToInvisibleLayer(..)>
        If object is in invisible respectively visible layer, it's moved to
        the corresponding visible respectively invisible layers.
        For group object the members are individually moved to the corresponding
        layer, because <SdrObjGroup::GetLayer()> does return 0, if members
        aren't on the same layer as the group object, and
        <SdrObjGroup::SetLayer(..)|NbcSetLayer(..)> sets also the layer of
        the members.

        @param _bToVisible
        input parameter - boolean indicating, if object has to be moved to
        visible (== true) or invisible (== false) layer.

        @param _pDrawObj
        input parameter, which will be changed - drawing object, which will
        change its layer.
    */
    void MoveObjToLayer( const bool _bToVisible,
                          SdrObject* _pDrawObj );

protected:
    void SetInDTOR();
    virtual void SwClientNotify(const SwModify&, const SfxHint& rHint) override;

public:

    /// For reader. Only the connection is created.
    SwContact( SwFrameFormat *pToRegisterIn );
    virtual ~SwContact() override;

    virtual const SwAnchoredObject* GetAnchoredObj( const SdrObject* _pSdrObj ) const = 0;
    virtual SwAnchoredObject* GetAnchoredObj( SdrObject* _pSdrObj ) = 0;

    virtual const SdrObject *GetMaster() const = 0;
    virtual SdrObject *GetMaster() = 0;

          SwFrameFormat  *GetFormat() { return static_cast<SwFrameFormat*>(GetRegisteredIn()); }
    const SwFrameFormat  *GetFormat() const
        { return static_cast<const SwFrameFormat*>(GetRegisteredIn()); }

    bool IsInDTOR() const { return mbInDTOR;}

    /** method to move drawing object to corresponding visible layer

        uses method <MoveObjToLayer(..)>

        @param _pDrawObj
        drawing object, which will be moved to the visible layer
    */
    virtual void MoveObjToVisibleLayer( SdrObject* _pDrawObj );

    /** method to move drawing object to corresponding invisible layer

        uses method <MoveObjToLayer(..)>

        @param _pDrawObj
        drawing object, which will be moved to the visible layer
    */
    virtual void MoveObjToInvisibleLayer( SdrObject* _pDrawObj );

    /** some virtual helper methods for information
     about the object (Writer fly frame resp. drawing object) */
    const SwFormatAnchor& GetAnchorFormat() const
    {
        assert( GetFormat() );

        return GetFormat()->GetAnchor();
    }

    RndStdIds GetAnchorId() const { return GetAnchorFormat().GetAnchorId(); }
    bool      ObjAnchoredAtPage() const { return GetAnchorId() == RndStdIds::FLY_AT_PAGE; }
    bool      ObjAnchoredAtFly()  const { return GetAnchorId() == RndStdIds::FLY_AT_FLY; }
    bool      ObjAnchoredAtPara() const { return GetAnchorId() == RndStdIds::FLY_AT_PARA; }
    bool      ObjAnchoredAtChar() const { return GetAnchorId() == RndStdIds::FLY_AT_CHAR; }
    bool      ObjAnchoredAsChar() const { return GetAnchorId() == RndStdIds::FLY_AS_CHAR; }

    const SwPosition&  GetContentAnchor() const
    {
        assert( GetAnchorFormat().GetContentAnchor() );
        return *(GetAnchorFormat().GetContentAnchor());
    }

    /** get data collection of anchored objects, handled by with contact */
    virtual void GetAnchoredObjs( std::vector<SwAnchoredObject*>& _roAnchoredObjs ) const = 0;

    /** get minimum order number of anchored objects handled by with contact

    */
    sal_uInt32 GetMinOrdNum() const;

    /** get maximum order number of anchored objects handled by with contact

    */
    sal_uInt32 GetMaxOrdNum() const;
};

/** ContactObject for connection between frames (or their formats respectively)
 in SwClient and the drawobjects of Drawing (SdrObjUserCall). */

class SW_DLLPUBLIC SwFlyDrawContact final : public SwContact
{
private:
    typedef std::unique_ptr< SwFlyDrawObj, SdrObjectFreeOp > SwFlyDrawObjPtr;

    SwFlyDrawObjPtr mpMasterObj;
    void SwClientNotify(const SwModify&, const SfxHint& rHint) override;
    sal_uInt32 GetOrdNumForNewRef(const SwFlyFrame* pFly);

public:

    /// Creates DrawObject and registers it with the Model.
    SwFlyDrawContact(SwFlyFrameFormat* pToRegisterIn, SdrModel& rTargetModel);
    static SwVirtFlyDrawObj* CreateNewRef(SwFlyFrame* pFly, SwFlyFrameFormat* pFormat);
    virtual ~SwFlyDrawContact() override;

    virtual const SwAnchoredObject* GetAnchoredObj( const SdrObject* _pSdrObj ) const override;
    virtual SwAnchoredObject* GetAnchoredObj( SdrObject* _pSdrObj ) override;

    virtual const SdrObject* GetMaster() const override
            { return const_cast<SwFlyDrawContact*>(this)->GetMaster(); };
    virtual SdrObject* GetMaster() override;

    /** override methods to control Writer fly frames,
     which are linked, and to assure that all objects anchored at/inside the
     Writer fly frame are also made visible/invisible. */
    virtual void MoveObjToVisibleLayer( SdrObject* _pDrawObj ) override;
    virtual void MoveObjToInvisibleLayer( SdrObject* _pDrawObj ) override;

    /** get data collection of anchored objects handled by with contact */
    virtual void GetAnchoredObjs( std::vector<SwAnchoredObject*>& _roAnchoredObjs ) const override;
};

/** new class for re-direct methods calls at a 'virtual'
      drawing object to its referenced object.
    For example, if an SdrObject is anchored in a header, then later pages will have a SwDrawVirtObj
    in later headers. */
class SwDrawVirtObj final : public SdrVirtObj
{
    private:
        // data for connection to writer layout
        /** anchored drawing object instance for the
         'virtual' drawing object */
        SwAnchoredDrawObject maAnchoredDrawObj;

        /** writer-drawing contact object the 'virtual' drawing object is controlled by.
         This object is also the <UserCall> of the drawing object, if it's
         inserted into the drawing layer. */
        SwDrawContact&  mrDrawContact;

        /** AW: Need own sdr::contact::ViewContact since AnchorPos from parent is
         not used but something own (top left of new SnapRect minus top left
         of original SnapRect) */
        virtual std::unique_ptr<sdr::contact::ViewContact> CreateObjectSpecificViewContact() override;

        // protected destructor
        virtual ~SwDrawVirtObj() override;

    public:
        SwDrawVirtObj(
            SdrModel& rSdrModel,
            SdrObject& _rNewObj,
            SwDrawContact& _rDrawContact);
        // copy constructor
        SwDrawVirtObj(SdrModel& rSdrModel, SwDrawVirtObj const & rSource);

        /// access to offset
        virtual Point GetOffset() const override;

        virtual SwDrawVirtObj* CloneSdrObject(SdrModel& rTargetModel) const override;

        /// connection to writer layout
        const SwAnchoredObject& GetAnchoredObj() const { return maAnchoredDrawObj; }
        SwAnchoredObject& AnchoredObj() { return maAnchoredDrawObj; }
        const SwFrame* GetAnchorFrame() const;
        SwFrame* AnchorFrame();
        void RemoveFromWriterLayout();

        /// connection to drawing layer
        void AddToDrawingPage();
        void RemoveFromDrawingPage();

        /** is 'virtual' drawing object connected to writer layout and
        / to drawing layer. */
        bool IsConnected() const;

        virtual void NbcSetAnchorPos(const Point& rPnt) override;

        /// All overridden methods which need to use the offset
        virtual void RecalcBoundRect() override;
        virtual ::basegfx::B2DPolyPolygon TakeXorPoly() const override;
        virtual ::basegfx::B2DPolyPolygon TakeContour() const override;
        virtual void AddToHdlList(SdrHdlList& rHdlList) const override;
        virtual void NbcMove(const Size& rSiz) override;
        virtual void NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact) override;
        virtual void NbcRotate(const Point& rRef, Degree100 nAngle, double sn, double cs) override;
        virtual void NbcMirror(const Point& rRef1, const Point& rRef2) override;
        virtual void NbcShear(const Point& rRef, Degree100 nAngle, double tn, bool bVShear) override;
        virtual void Move(const Size& rSiz) override;
        virtual void Resize(const Point& rRef, const Fraction& xFact, const Fraction& yFact, bool bUnsetRelative = true) override;
        virtual void Rotate(const Point& rRef, Degree100 nAngle, double sn, double cs) override;
        virtual void Mirror(const Point& rRef1, const Point& rRef2) override;
        virtual void Shear(const Point& rRef, Degree100 nAngle, double tn, bool bVShear) override;
        virtual void RecalcSnapRect() override;
        virtual const tools::Rectangle& GetSnapRect() const override;
        virtual void SetSnapRect(const tools::Rectangle& rRect) override;
        virtual void NbcSetSnapRect(const tools::Rectangle& rRect) override;
        virtual const tools::Rectangle& GetLogicRect() const override;
        virtual void SetLogicRect(const tools::Rectangle& rRect) override;
        virtual void NbcSetLogicRect(const tools::Rectangle& rRect) override;
        virtual Point GetSnapPoint(sal_uInt32 i) const override;
        virtual Point GetPoint(sal_uInt32 i) const override;
        virtual void NbcSetPoint(const Point& rPnt, sal_uInt32 i) override;

        virtual bool HasTextEdit() const override;

        virtual SdrLayerID GetLayer() const override;
        virtual void NbcSetLayer(SdrLayerID nLayer) override;
        virtual void SetLayer(SdrLayerID nLayer) override;

        virtual void SetBoundRectDirty() override;
        virtual const tools::Rectangle& GetCurrentBoundRect() const override;
        virtual const tools::Rectangle& GetLastBoundRect() const override;
};

bool CheckControlLayer( const SdrObject *pObj );

/** ContactObject for connection of formats as representatives of draw objects
 in SwClient and the objects themselves in Drawing (SDrObjUserCall). */

typedef std::unique_ptr< SwDrawVirtObj, SdrObjectFreeOp > SwDrawVirtObjPtr;

class SAL_DLLPUBLIC_RTTI SwDrawContact final : public SwContact
{
    private:
        /** anchored drawing object instance for the
            'master' drawing object */
        SwAnchoredDrawObject maAnchoredDrawObj;

        /** container for 'virtual' drawing object supporting drawing objects in headers/footers. */
        std::vector< SwDrawVirtObjPtr > maDrawVirtObjs;

        /** boolean indicating set 'master' drawing
         object has been cleared. */
        bool mbMasterObjCleared : 1;

        /** internal flag to indicate that disconnect
         from layout is in progress */
        bool mbDisconnectInProgress : 1;

        /** Needed data for handling of nested <SdrObjUserCall> events in
         method <Changed_(..)> */
        bool mbUserCallActive : 1;
        /** event type, which is handled for <mpSdrObjHandledByCurrentUserCall>.
         Note: value only valid, if <mbUserCallActive> is true. */
        SdrUserCallType meEventTypeOfCurrentUserCall;

        friend class NestedUserCallHdl;


        void RemoveAllVirtObjs();

        void InvalidateObjs_( const bool _bUpdateSortedObjsList = false );

        SwDrawContact( const SwDrawContact& ) = delete;
        SwDrawContact& operator=( const SwDrawContact& ) = delete;

        virtual void SwClientNotify(const SwModify&, const SfxHint& rHint) override;

    public:

        SwDrawContact( SwFrameFormat *pToRegisterIn, SdrObject *pObj );
        virtual ~SwDrawContact() override;

        virtual const SwAnchoredObject* GetAnchoredObj( const SdrObject* _pSdrObj ) const override;
        virtual SwAnchoredObject* GetAnchoredObj( SdrObject* _pSdrObj ) override;

        virtual const SdrObject* GetMaster() const override
            { return const_cast<SwDrawContact*>(this)->GetMaster(); };
        virtual SdrObject* GetMaster() override;

        const SwFrame* GetAnchorFrame( const SdrObject* _pDrawObj = nullptr ) const;
        SwFrame* GetAnchorFrame( SdrObject const * _pDrawObj = nullptr );

        const SwPageFrame* GetPageFrame() const
        {
            return maAnchoredDrawObj.GetPageFrame();
        }
        SwPageFrame* GetPageFrame()
        {
            return maAnchoredDrawObj.GetPageFrame();
        }
        void ChkPage();
        SwPageFrame* FindPage( const SwRect &rRect );

        /** Inserts SdrObject in the arrays of the layout ((SwPageFrame and SwFrame).
         The anchor is determined according to the attribute SwFormatAnchor.
         If required the object gets unregistered with the old anchor. */
        void ConnectToLayout( const SwFormatAnchor *pAnch = nullptr );
        /** method to insert 'master' drawing object
         into drawing page */
        void InsertMasterIntoDrawPage();

        void DisconnectFromLayout( bool _bMoveMasterToInvisibleLayer = true );
        /** disconnect for a dedicated drawing object -
         could be 'master' or 'virtual'. */
        void DisconnectObjFromLayout( SdrObject* _pDrawObj );
        /** method to remove 'master' drawing object
         from drawing page.
         To be used by the undo for delete of object. Call it after method
         <DisconnectFromLayout( bool = true )> is already performed.
         Note: <DisconnectFromLayout( bool )> no longer removes the 'master'
         drawing object from drawing page. */
        void RemoveMasterFromDrawPage();

        /** get drawing object ('master' or 'virtual')
         by frame. */
        SdrObject* GetDrawObjectByAnchorFrame( const SwFrame& _rAnchorFrame );

        /// Virtual methods of SdrObjUserCall.
        virtual void Changed(const SdrObject& rObj, SdrUserCallType eType, const tools::Rectangle& rOldBoundRect) override;

        /** Used by Changed() and by UndoDraw.
         Notifies paragraphs that have to get out of the way. */
        void Changed_(const SdrObject& rObj, SdrUserCallType eType, const tools::Rectangle* pOldBoundRect);

        /// Moves all SW-connections to new Master)
        void ChangeMasterObject( SdrObject *pNewMaster );

        SwDrawVirtObj* AddVirtObj();

        void NotifyBackgrdOfAllVirtObjs( const tools::Rectangle* pOldBoundRect );

        /** get data collection of anchored objects, handled by with contact */

        static void GetTextObjectsFromFormat( std::list<SdrTextObj*>&, SwDoc& );
        virtual void GetAnchoredObjs( std::vector<SwAnchoredObject*>& _roAnchoredObjs ) const override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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
#include <swtypes.hxx>
#include <fmtanchr.hxx>
#include <frmfmt.hxx>
#include <list>

#include "calbck.hxx"
#include <anchoreddrawobject.hxx>

class SfxPoolItem;
class SwFrameFormat;
class SwFlyFrameFormat;
class SwFlyFrm;
class SwFrm;
class SwPageFrm;
class SwVirtFlyDrawObj;
class SwFormatAnchor;
class SwFlyDrawObj;
class SwRect;
class SwDrawContact;
struct SwPosition;
class SwIndex;
class SdrTextObj;

/** The other way round: Search format for given object.
 If object is a SwVirtFlyDrawObj the format will be obtained from it.
 If not it is a simple DrawObject. It has a UserCall which
 is client of the format we are looking for.
 Implementation in dcontact.cxx. */
SW_DLLPUBLIC SwFrameFormat *FindFrameFormat( SdrObject *pObj );
inline const SwFrameFormat *FindFrameFormat( const SdrObject *pObj )
{   return ::FindFrameFormat( const_cast<SdrObject*>(pObj) ); }
bool HasWrap( const SdrObject* pObj );

void setContextWritingMode( SdrObject* pObj, SwFrm* pAnchor );

/// @return BoundRect plus distance.
SwRect GetBoundRectOfAnchoredObj( const SdrObject* pObj );

/// @return UserCall of goup object (if applicable).
SwContact* GetUserCall( const SdrObject* );

/// @return TRUE if the SrdObject is a Marquee object.
bool IsMarqueeTextObj( const SdrObject& rObj );

/// Base class for the following contact objects (frame + draw objects).
class SwContact : public SdrObjUserCall, public SwClient
{
    /** boolean, indicating destruction of contact object
     important note: boolean has to be set at the beginning of each destructor
                     in the subclasses using method <SetInDTOR()>. */
    bool mbInDTOR;

    /** method to move object to visible/invisible layer

        Implementation for the public method <MoveObjToVisibleLayer(..)>
        and <MoveObjToInvisibleLayer(..)>
        If object is in invisble respectively visible layer, its moved to
        the corresponding visible respectively invisible layers.
        For group object the members are individually moved to the corresponding
        layer, because <SdrObjGroup::GetLayer()> does return 0, if members
        aren't on the same layer as the group object, and
        <SdrObjGroup::SetLayer(..)|NbcSetLayer(..)> sets also the layer of
        the members.

        @author OD

        @param _bToVisible
        input parameter - boolean indicating, if object has to be moved to
        visible (== true) or invisible (== false) layer.

        @param _pDrawObj
        input parameter, which will be changed - drawing object, which will
        change its layer.
    */
    void _MoveObjToLayer( const bool _bToVisible,
                          SdrObject* _pDrawObj );

protected:
    void SetInDTOR();

public:

    /// For reader. Only the connection is created.
    SwContact( SwFrameFormat *pToRegisterIn );
    virtual ~SwContact();

    virtual const SwAnchoredObject* GetAnchoredObj( const SdrObject* _pSdrObj ) const = 0;
    virtual SwAnchoredObject* GetAnchoredObj( SdrObject* _pSdrObj ) = 0;

    virtual const SdrObject *GetMaster() const = 0;
    virtual SdrObject *GetMaster() = 0;
    virtual void SetMaster( SdrObject* _pNewMaster ) = 0;

          SwFrameFormat  *GetFormat() { return static_cast<SwFrameFormat*>(GetRegisteredIn()); }
    const SwFrameFormat  *GetFormat() const
        { return static_cast<const SwFrameFormat*>(GetRegisteredIn()); }

    bool IsInDTOR() const { return mbInDTOR;}

    /** method to move drawing object to corresponding visible layer

        uses method <_MoveObjToLayer(..)>

        @author OD

        @param _pDrawObj
        drawing object, which will be moved to the visible layer
    */
    virtual void MoveObjToVisibleLayer( SdrObject* _pDrawObj );

    /** method to move drawing object to corresponding invisible layer

        uses method <_MoveObjToLayer(..)>

        @author OD

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
    bool      ObjAnchoredAtPage() const { return GetAnchorId() == FLY_AT_PAGE; }
    bool      ObjAnchoredAtFly()  const { return GetAnchorId() == FLY_AT_FLY; }
    bool      ObjAnchoredAtPara() const { return GetAnchorId() == FLY_AT_PARA; }
    bool      ObjAnchoredAtChar() const { return GetAnchorId() == FLY_AT_CHAR; }
    bool      ObjAnchoredAsChar() const { return GetAnchorId() == FLY_AS_CHAR; }

    const SwPosition&  GetContentAnchor() const
    {
        assert( GetAnchorFormat().GetContentAnchor() );
        return *(GetAnchorFormat().GetContentAnchor());
    }

    const SwIndex&     GetContentAnchorIndex() const;

    /** get data collection of anchored objects, handled by with contact

    */
    virtual void GetAnchoredObjs( std::list<SwAnchoredObject*>& _roAnchoredObjs ) const = 0;

    /** get minimum order number of anchored objects handled by with contact

    */
    sal_uInt32 GetMinOrdNum() const;

    /** get maximum order number of anchored objects handled by with contact

    */
    sal_uInt32 GetMaxOrdNum() const;
};

/** ContactObject for connection between frames (or their formats respectively)
 in SwClient and the drawobjects of Drawing (DsrObjUserCall). */

class SW_DLLPUBLIC SwFlyDrawContact : public SwContact
{
private:
    SwFlyDrawObj* mpMasterObj;

protected:
    virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew ) override;

public:

    /// Creates DrawObject and registers it with the Model.
    SwFlyDrawContact( SwFlyFrameFormat* pToRegisterIn, SdrModel* pMod );
    virtual ~SwFlyDrawContact();

    virtual const SwAnchoredObject* GetAnchoredObj( const SdrObject* _pSdrObj ) const override;
    virtual SwAnchoredObject* GetAnchoredObj( SdrObject* _pSdrObj ) override;

    virtual const SdrObject* GetMaster() const override;
    virtual SdrObject* GetMaster() override;
    virtual void SetMaster( SdrObject* _pNewMaster ) override;

    /** override methods to control Writer fly frames,
     which are linked, and to assure that all objects anchored at/inside the
     Writer fly frame are also made visible/invisible. */
    virtual void MoveObjToVisibleLayer( SdrObject* _pDrawObj ) override;
    virtual void MoveObjToInvisibleLayer( SdrObject* _pDrawObj ) override;

    /** get data collection of anchored objects handled by with contact
    */
    virtual void GetAnchoredObjs( std::list<SwAnchoredObject*>& _roAnchoredObjs ) const override;
};

/** new class for re-direct methods calls at a 'virtual'
      drawing object to its referenced object. */
class SwDrawVirtObj : public SdrVirtObj
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

   protected:
        /** AW: Need own sdr::contact::ViewContact since AnchorPos from parent is
         not used but something own (top left of new SnapRect minus top left
         of original SnapRect) */
        virtual sdr::contact::ViewContact* CreateObjectSpecificViewContact() override;

   public:

        SwDrawVirtObj( SdrObject&       _rNewObj,
                       SwDrawContact&   _rDrawContact );
        virtual ~SwDrawVirtObj();

        /// access to offset
        virtual const Point GetOffset() const override;

        virtual SwDrawVirtObj* Clone() const override;
        SwDrawVirtObj& operator= (const SwDrawVirtObj& rObj);

        /// connection to writer layout
        const SwAnchoredObject& GetAnchoredObj() const { return maAnchoredDrawObj; }
        SwAnchoredObject& AnchoredObj() { return maAnchoredDrawObj; }
        const SwFrm* GetAnchorFrm() const;
        SwFrm* AnchorFrm();
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
        virtual SdrHdl* GetHdl(sal_uInt32 nHdlNum) const override;
        virtual void NbcMove(const Size& rSiz) override;
        virtual void NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact) override;
        virtual void NbcRotate(const Point& rRef, long nAngle, double sn, double cs) override;
        virtual void NbcMirror(const Point& rRef1, const Point& rRef2) override;
        virtual void NbcShear(const Point& rRef, long nAngle, double tn, bool bVShear) override;
        virtual void Move(const Size& rSiz) override;
        virtual void Resize(const Point& rRef, const Fraction& xFact, const Fraction& yFact, bool bUnsetRelative = true) override;
        virtual void Rotate(const Point& rRef, long nAngle, double sn, double cs) override;
        virtual void Mirror(const Point& rRef1, const Point& rRef2) override;
        virtual void Shear(const Point& rRef, long nAngle, double tn, bool bVShear) override;
        virtual void RecalcSnapRect() override;
        virtual const Rectangle& GetSnapRect() const override;
        virtual void SetSnapRect(const Rectangle& rRect) override;
        virtual void NbcSetSnapRect(const Rectangle& rRect) override;
        virtual const Rectangle& GetLogicRect() const override;
        virtual void SetLogicRect(const Rectangle& rRect) override;
        virtual void NbcSetLogicRect(const Rectangle& rRect) override;
        virtual Point GetSnapPoint(sal_uInt32 i) const override;
        virtual Point GetPoint(sal_uInt32 i) const override;
        virtual void NbcSetPoint(const Point& rPnt, sal_uInt32 i) override;

        virtual bool HasTextEdit() const override;

        virtual SdrLayerID GetLayer() const override;
        virtual void NbcSetLayer(SdrLayerID nLayer) override;
        virtual void SetLayer(SdrLayerID nLayer) override;

        /// FullDrag support
        virtual bool supportsFullDrag() const override;
        virtual SdrObject* getFullDragClone() const override;

        virtual void SetBoundRectDirty() override;
        virtual const Rectangle& GetCurrentBoundRect() const override;
        virtual const Rectangle& GetLastBoundRect() const override;
};

bool CheckControlLayer( const SdrObject *pObj );

/** ContactObject for connection of formats as representatives of draw objects
 in SwClient and the objects themselves in Drawing (SDrObjUserCall). */
class NestedUserCallHdl;

class SwDrawContact : public SwContact
{
    private:
        /** anchored drawing object instance for the
            'master' drawing object */
        SwAnchoredDrawObject maAnchoredDrawObj;

        /** data structure for collecting 'virtual'
         drawing object supporting drawing objects in headers/footers. */
        std::list<SwDrawVirtObj*> maDrawVirtObjs;

        /** boolean indicating set 'master' drawing
         object has been cleared. */
        bool mbMasterObjCleared : 1;

        /** internal flag to indicate that disconnect
         from layout is in progress */
        bool mbDisconnectInProgress : 1;

        /** Needed data for handling of nested <SdrObjUserCall> events in
         method <_Changed(..)> */
        bool mbUserCallActive : 1;
        /** event type, which is handled for <mpSdrObjHandledByCurrentUserCall>.
         Note: value only valid, if <mbUserCallActive> is true. */
        SdrUserCallType meEventTypeOfCurrentUserCall;

        friend class NestedUserCallHdl;

        /** unary function used by <list> iterator to find a disconnected 'virtual'
         drawing object */
        struct UsedOrUnusedVirtObjPred
        {
            bool mbUsedPred;
            UsedOrUnusedVirtObjPred( bool _bUsed ) : mbUsedPred( _bUsed ) {};
            bool operator() ( const SwDrawVirtObj* _pDrawVirtObj )
            {
                if ( mbUsedPred )
                {
                    return _pDrawVirtObj->IsConnected();
                }
                else
                {
                    return !_pDrawVirtObj->IsConnected();
                }
            }
        };

        /** unary function used by <list> iterator to find a 'virtual' drawing
         object anchored at a given frame */
        struct VirtObjAnchoredAtFrmPred
        {
            const SwFrm* mpAnchorFrm;
            VirtObjAnchoredAtFrmPred( const SwFrm& _rAnchorFrm );
            bool operator() ( const SwDrawVirtObj* _pDrawVirtObj );
        };

        /// method for adding/removing 'virtual' drawing object.
        SwDrawVirtObj* CreateVirtObj();
        static void DestroyVirtObj( SwDrawVirtObj* pVirtObj );
        void RemoveAllVirtObjs();

        void _InvalidateObjs( const bool _bUpdateSortedObjsList = false );

        SwDrawContact( const SwDrawContact& ) = delete;
        SwDrawContact& operator=( const SwDrawContact& ) = delete;

    protected:
        /// virtuelle Methoden von SwClient
        virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew ) override;

    public:

        SwDrawContact( SwFrameFormat *pToRegisterIn, SdrObject *pObj );
        virtual ~SwDrawContact();

        virtual const SwAnchoredObject* GetAnchoredObj( const SdrObject* _pSdrObj ) const override;
        virtual SwAnchoredObject* GetAnchoredObj( SdrObject* _pSdrObj ) override;

        virtual const SdrObject* GetMaster() const override;
        virtual SdrObject* GetMaster() override;
        virtual void SetMaster( SdrObject* _pNewMaster ) override;

        const SwFrm* GetAnchorFrm( const SdrObject* _pDrawObj = 0L ) const;
        SwFrm* GetAnchorFrm( SdrObject* _pDrawObj = 0L );

        inline const SwPageFrm* GetPageFrm() const
        {
            return maAnchoredDrawObj.GetPageFrm();
        }
        inline SwPageFrm* GetPageFrm()
        {
            return maAnchoredDrawObj.GetPageFrm();
        }
        void SetPageFrm( SwPageFrm* _pNewPageFrm )
        {
            return maAnchoredDrawObj.SetPageFrm( _pNewPageFrm );
        }
        void ChkPage();
        SwPageFrm* FindPage( const SwRect &rRect );

        /** Inserts SdrObject in the arrays of the layout ((SwPageFrm and SwFrm).
         The anchor is determined according to the attribute SwFormatAnchor.
         If required the object gets unregistered with the old anchor. */
        void ConnectToLayout( const SwFormatAnchor *pAnch = 0 );
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
        SdrObject* GetDrawObjectByAnchorFrm( const SwFrm& _rAnchorFrm );

        /// Virtual methods of SdrObjUserCall.
        virtual void Changed(const SdrObject& rObj, SdrUserCallType eType, const Rectangle& rOldBoundRect) override;

        /** Used by Changed() and by UndoDraw.
         Notifies paragraphs that have to get out of the way. */
        void _Changed(const SdrObject& rObj, SdrUserCallType eType, const Rectangle* pOldBoundRect);

        /// Moves all SW-connections to new Master)
        void ChangeMasterObject( SdrObject *pNewMaster );

        SwDrawVirtObj* AddVirtObj();

        void NotifyBackgrdOfAllVirtObjs( const Rectangle* pOldBoundRect );

        /** get data collection of anchored objects, handled by with contact
        */

        static void GetTextObjectsFromFormat( std::list<SdrTextObj*>&, SwDoc* );
        virtual void GetAnchoredObjs( std::list<SwAnchoredObject*>& _roAnchoredObjs ) const override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

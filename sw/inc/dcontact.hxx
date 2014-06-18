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
class SwFrmFmt;
class SwFlyFrmFmt;
class SwFlyFrm;
class SwFrm;
class SwPageFrm;
class SwVirtFlyDrawObj;
class SwFmtAnchor;
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
SW_DLLPUBLIC SwFrmFmt *FindFrmFmt( SdrObject *pObj );
inline const SwFrmFmt *FindFrmFmt( const SdrObject *pObj )
{   return ::FindFrmFmt( (SdrObject*)pObj ); }
bool HasWrap( const SdrObject* pObj );

void setContextWritingMode( SdrObject* pObj, SwFrm* pAnchor );

/** When changes occur remove object from ContourCache.
 Implementation in TxtFly.cxx. */
void ClrContourCache( const SdrObject *pObj );

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
    TYPEINFO_OVERRIDE();

    /// For reader. Only the connection is created.
    SwContact( SwFrmFmt *pToRegisterIn );
    virtual ~SwContact();

    virtual const SwAnchoredObject* GetAnchoredObj( const SdrObject* _pSdrObj ) const = 0;
    virtual SwAnchoredObject* GetAnchoredObj( SdrObject* _pSdrObj ) = 0;

    virtual const SdrObject *GetMaster() const = 0;
    virtual SdrObject *GetMaster() = 0;
    virtual void SetMaster( SdrObject* _pNewMaster ) = 0;

          SwFrmFmt  *GetFmt(){ return (SwFrmFmt*)GetRegisteredIn(); }
    const SwFrmFmt  *GetFmt() const
        { return (const SwFrmFmt*)GetRegisteredIn(); }

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
    const SwFmtAnchor& GetAnchorFmt() const
    {
        assert( GetFmt() );

        return GetFmt()->GetAnchor();
    }

    RndStdIds GetAnchorId() const { return GetAnchorFmt().GetAnchorId(); }
    bool      ObjAnchoredAtPage() const { return GetAnchorId() == FLY_AT_PAGE; }
    bool      ObjAnchoredAtFly()  const { return GetAnchorId() == FLY_AT_FLY; }
    bool      ObjAnchoredAtPara() const { return GetAnchorId() == FLY_AT_PARA; }
    bool      ObjAnchoredAtChar() const { return GetAnchorId() == FLY_AT_CHAR; }
    bool      ObjAnchoredAsChar() const { return GetAnchorId() == FLY_AS_CHAR; }

    const SwPosition&  GetCntntAnchor() const
    {
        assert( GetAnchorFmt().GetCntntAnchor() );
        return *(GetAnchorFmt().GetCntntAnchor());
    }

    const SwIndex&     GetCntntAnchorIndex() const;

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
    virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew ) SAL_OVERRIDE;

public:
    TYPEINFO_OVERRIDE();

    /// Creates DrawObject and registers it with the Model.
    SwFlyDrawContact( SwFlyFrmFmt* pToRegisterIn, SdrModel* pMod );
    virtual ~SwFlyDrawContact();

    virtual const SwAnchoredObject* GetAnchoredObj( const SdrObject* _pSdrObj ) const SAL_OVERRIDE;
    virtual SwAnchoredObject* GetAnchoredObj( SdrObject* _pSdrObj ) SAL_OVERRIDE;

    virtual const SdrObject* GetMaster() const SAL_OVERRIDE;
    virtual SdrObject* GetMaster() SAL_OVERRIDE;
    virtual void SetMaster( SdrObject* _pNewMaster ) SAL_OVERRIDE;

    /** override methods to control Writer fly frames,
     which are linked, and to assure that all objects anchored at/inside the
     Writer fly frame are also made visible/invisible. */
    virtual void MoveObjToVisibleLayer( SdrObject* _pDrawObj ) SAL_OVERRIDE;
    virtual void MoveObjToInvisibleLayer( SdrObject* _pDrawObj ) SAL_OVERRIDE;

    /** get data collection of anchored objects handled by with contact
    */
    virtual void GetAnchoredObjs( std::list<SwAnchoredObject*>& _roAnchoredObjs ) const SAL_OVERRIDE;
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

        using SdrVirtObj::GetPlusHdl;

   protected:
        /** AW: Need own sdr::contact::ViewContact since AnchorPos from parent is
         not used but something own (top left of new SnapRect minus top left
         of original SnapRect) */
        virtual sdr::contact::ViewContact* CreateObjectSpecificViewContact() SAL_OVERRIDE;

   public:
        TYPEINFO_OVERRIDE();

        SwDrawVirtObj( SdrObject&       _rNewObj,
                       SwDrawContact&   _rDrawContact );
        virtual ~SwDrawVirtObj();

        /// access to offset
        virtual const Point GetOffset() const SAL_OVERRIDE;

        virtual SwDrawVirtObj* Clone() const SAL_OVERRIDE;
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

        virtual void NbcSetAnchorPos(const Point& rPnt) SAL_OVERRIDE;

        /// All overloaded methods which need to use the offset
        virtual void RecalcBoundRect() SAL_OVERRIDE;
        virtual ::basegfx::B2DPolyPolygon TakeXorPoly() const SAL_OVERRIDE;
        virtual ::basegfx::B2DPolyPolygon TakeContour() const SAL_OVERRIDE;
        virtual SdrHdl* GetHdl(sal_uInt32 nHdlNum) const SAL_OVERRIDE;
        virtual SdrHdl* GetPlusHdl(const SdrHdl& rHdl, sal_uInt16 nPlNum) const;
        virtual void NbcMove(const Size& rSiz) SAL_OVERRIDE;
        virtual void NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact) SAL_OVERRIDE;
        virtual void NbcRotate(const Point& rRef, long nWink, double sn, double cs) SAL_OVERRIDE;
        virtual void NbcMirror(const Point& rRef1, const Point& rRef2) SAL_OVERRIDE;
        virtual void NbcShear(const Point& rRef, long nWink, double tn, bool bVShear) SAL_OVERRIDE;
        virtual void Move(const Size& rSiz) SAL_OVERRIDE;
        virtual void Resize(const Point& rRef, const Fraction& xFact, const Fraction& yFact, bool bUnsetRelative = true) SAL_OVERRIDE;
        virtual void Rotate(const Point& rRef, long nWink, double sn, double cs) SAL_OVERRIDE;
        virtual void Mirror(const Point& rRef1, const Point& rRef2) SAL_OVERRIDE;
        virtual void Shear(const Point& rRef, long nWink, double tn, bool bVShear) SAL_OVERRIDE;
        virtual void RecalcSnapRect() SAL_OVERRIDE;
        virtual const Rectangle& GetSnapRect() const SAL_OVERRIDE;
        virtual void SetSnapRect(const Rectangle& rRect) SAL_OVERRIDE;
        virtual void NbcSetSnapRect(const Rectangle& rRect) SAL_OVERRIDE;
        virtual const Rectangle& GetLogicRect() const SAL_OVERRIDE;
        virtual void SetLogicRect(const Rectangle& rRect) SAL_OVERRIDE;
        virtual void NbcSetLogicRect(const Rectangle& rRect) SAL_OVERRIDE;
        virtual Point GetSnapPoint(sal_uInt32 i) const SAL_OVERRIDE;
        virtual Point GetPoint(sal_uInt32 i) const SAL_OVERRIDE;
        virtual void NbcSetPoint(const Point& rPnt, sal_uInt32 i) SAL_OVERRIDE;

        virtual bool HasTextEdit() const SAL_OVERRIDE;

        virtual SdrLayerID GetLayer() const SAL_OVERRIDE;
        virtual void NbcSetLayer(SdrLayerID nLayer) SAL_OVERRIDE;
        virtual void SetLayer(SdrLayerID nLayer) SAL_OVERRIDE;

        /// FullDrag support
        virtual bool supportsFullDrag() const SAL_OVERRIDE;
        virtual SdrObject* getFullDragClone() const SAL_OVERRIDE;

        virtual void SetBoundRectDirty() SAL_OVERRIDE;
        virtual const Rectangle& GetCurrentBoundRect() const SAL_OVERRIDE;
        virtual const Rectangle& GetLastBoundRect() const SAL_OVERRIDE;
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
         Note: value only valid, if <mbUserCallActive> is sal_True. */
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
        void DestroyVirtObj( SwDrawVirtObj* pVirtObj );
        void RemoveAllVirtObjs();

        void _InvalidateObjs( const bool _bUpdateSortedObjsList = false );

        /// no copy-constructor and no assignment operator
        SwDrawContact( const SwDrawContact& );
        SwDrawContact& operator=( const SwDrawContact& );

    protected:
        /// virtuelle Methoden von SwClient
        virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew ) SAL_OVERRIDE;

    public:
        TYPEINFO_OVERRIDE();

        SwDrawContact( SwFrmFmt *pToRegisterIn, SdrObject *pObj );
        virtual ~SwDrawContact();

        virtual const SwAnchoredObject* GetAnchoredObj( const SdrObject* _pSdrObj ) const SAL_OVERRIDE;
        virtual SwAnchoredObject* GetAnchoredObj( SdrObject* _pSdrObj ) SAL_OVERRIDE;

        virtual const SdrObject* GetMaster() const SAL_OVERRIDE;
        virtual SdrObject* GetMaster() SAL_OVERRIDE;
        virtual void SetMaster( SdrObject* _pNewMaster ) SAL_OVERRIDE;

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
         The anchor is determined according to the attribute SwFmtAnchor.
         If required the object gets unregistered with the old anchor. */
        void ConnectToLayout( const SwFmtAnchor *pAnch = 0 );
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
        virtual void Changed(const SdrObject& rObj, SdrUserCallType eType, const Rectangle& rOldBoundRect) SAL_OVERRIDE;

        /** Used by Changed() and by UndoDraw.
         Notifies paragraphs that have to get out of the way. */
        void _Changed(const SdrObject& rObj, SdrUserCallType eType, const Rectangle* pOldBoundRect);

        /// Moves all SW-connections to new Master)
        void ChangeMasterObject( SdrObject *pNewMaster );

        SwDrawVirtObj* AddVirtObj();

        void NotifyBackgrdOfAllVirtObjs( const Rectangle* pOldBoundRect );

        /** get data collection of anchored objects, handled by with contact
        */

        static void GetTextObjectsFromFmt( std::list<SdrTextObj*>&, SwDoc* );
        virtual void GetAnchoredObjs( std::list<SwAnchoredObject*>& _roAnchoredObjs ) const SAL_OVERRIDE;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

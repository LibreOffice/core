/*************************************************************************
 *
 *  $RCSfile: dcontact.hxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: kz $ $Date: 2004-08-02 13:56:23 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _DCONTACT_HXX
#define _DCONTACT_HXX

#ifndef _SVDOBJ_HXX //autogen
#include <svx/svdobj.hxx>
#endif
// OD 14.05.2003 #108784#
#ifndef _SVDOVIRT_HXX
#include <svx/svdovirt.hxx>
#endif
// OD 2004-01-16 #110582#
#ifndef _SWTYPES_HXX
#include <swtypes.hxx>
#endif

#ifndef _FMTANCHR_HXX //autogen
#include <fmtanchr.hxx>
#endif
#ifndef _FRMFMT_HXX
#include <frmfmt.hxx>
#endif

// OD 17.06.2003 #108784#
#include <list>

#include "calbck.hxx"

class SfxPoolItem;
class SwFrmFmt;
class SwFlyFrmFmt;
class SwDrawFrmFmt;
class SwFlyFrm;
class SwFrm;
class SwPageFrm;
class SwVirtFlyDrawObj;
class SwFmtAnchor;
class SwFlyDrawObj;
class SwRect;
// OD 17.06.2003 #108784# - forward declaration for class <SwDrawVirtObj>
class SwDrawContact;
// OD 2004-01-16 #110582#
struct SwPosition;
class SwNodeIndex;
class SwIndex;
// OD 2004-03-25 #i26791#
#ifndef _ANCHOREDDRAWOBJECT_HXX
#include <anchoreddrawobject.hxx>
#endif

//Der Umgekehrte Weg: Sucht das Format zum angegebenen Objekt.
//Wenn das Object ein SwVirtFlyDrawObj ist so wird das Format von
//selbigem besorgt.
//Anderfalls ist es eben ein einfaches Zeichenobjekt. Diese hat einen
//UserCall und der ist Client vom gesuchten Format.
//Implementierung in dcontact.cxx
SwFrmFmt *FindFrmFmt( SdrObject *pObj );
inline const SwFrmFmt *FindFrmFmt( const SdrObject *pObj )
{   return ::FindFrmFmt( (SdrObject*)pObj ); }
sal_Bool HasWrap( const SdrObject* pObj );

//Bei Aenderungen das Objekt aus dem ContourCache entfernen.
//Implementierung in TxtFly.Cxx
void ClrContourCache( const SdrObject *pObj );

// liefert BoundRect inklusive Abstand
SwRect GetBoundRect( const SdrObject* pObj );

//Liefert den UserCall ggf. vom Gruppenobjekt
// OD 2004-03-31 #i26791# - change return type
SwContact* GetUserCall( const SdrObject* );

// liefert TRUE falls das SrdObject ein Marquee-Object (Lauftext) ist
FASTBOOL IsMarqueeTextObj( const SdrObject& rObj );

//Basisklasse fuer die folgenden KontaktObjekte (Rahmen+Zeichenobjekte)
class SwContact : public SdrObjUserCall, public SwClient
{
    // OD 05.09.2003 #112039# - boolean, indicating destruction of contact object
    // important note: boolean has to be set at the beginning of each destructor
    //                 in the subclasses using method <SetInDTOR()>.
    bool mbInDTOR;

    /** method to move object to visible/invisible layer

        OD 21.08.2003 #i18447#
        Implementation for the public method <MoveObjToVisibleLayer(..)>
        and <MoveObjToInvisibleLayer(..)>
        If object is in invisble respectively visible layer, its moved to
        the corresponding visible respectively invisible layers.
        For group object the members are individually moved to the corresponding
        layer, because <SdrObjGroup::GetLayer()> does return 0, if members
        aren't on the same layer as the group object, and
        <SdrObjGroup::SetLayer(..)|NbcSetLayer(..)> sets also the layer of
        the members.
        OD 2004-01-15 #110582# - moved from subclass <SwDrawContact>

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
    // OD 05.09.2003 #112039# - accessor to set member <mbInDTOR>
    void SetInDTOR();

public:
    TYPEINFO();

    //Fuer den Reader, es wir nur die Verbindung hergestellt.
    SwContact( SwFrmFmt *pToRegisterIn );
    virtual ~SwContact();

    // OD 2004-03-29 #i26791#
    virtual const SwAnchoredObject* GetAnchoredObj( const SdrObject* _pSdrObj ) const = 0;
    virtual SwAnchoredObject* GetAnchoredObj( SdrObject* _pSdrObj ) = 0;

    // OD 13.05.2003 #108784# - made methods virtual and not inline
    // OD 2004-04-01 #i26791# - made methods pure virtual
    virtual const SdrObject *GetMaster() const = 0;
    virtual SdrObject *GetMaster() = 0;
    virtual void SetMaster( SdrObject* _pNewMaster ) = 0;

          SwFrmFmt  *GetFmt(){ return (SwFrmFmt*)GetRegisteredIn(); }
    const SwFrmFmt  *GetFmt() const
        { return (const SwFrmFmt*)GetRegisteredIn(); }

    // OD 05.09.2003 #112039# - accessor for member <mbInDTOR>
    const bool IsInDTOR() const;

    /** method to move drawing object to corresponding visible layer

        OD 21.08.2003 #i18447#
        uses method <_MoveObjToLayer(..)>
        OD 2004-01-15 #110582# - moved from subclass <SwDrawContact> and made virtual

        @author OD

        @param _pDrawObj
        drawing object, which will be moved to the visible layer
    */
    virtual void MoveObjToVisibleLayer( SdrObject* _pDrawObj );

    /** method to move drawing object to corresponding invisible layer

        OD 21.08.2003 #i18447#
        uses method <_MoveObjToLayer(..)>
        OD 2004-01-15 #110582# - moved from subclass <SwDrawContact> and made virtual.

        @author OD

        @param _pDrawObj
        drawing object, which will be moved to the visible layer
    */
    virtual void MoveObjToInvisibleLayer( SdrObject* _pDrawObj );

    // -------------------------------------------------------------------------
    // OD 2004-01-16 #110582# - some virtual helper methods for information
    // about the object (Writer fly frame resp. drawing object)
    const SwFmtAnchor& GetAnchorFmt() const
    {
        ASSERT( GetFmt(),
                "<SwContact::GetAnchorFmt()> - no frame format -> crash" );

        return GetFmt()->GetAnchor();
    }

    RndStdIds GetAnchorId() const { return GetAnchorFmt().GetAnchorId(); }
    bool      ObjAnchoredAtPage() const { return GetAnchorId() == FLY_PAGE; }
    bool      ObjAnchoredAtFly() const { return GetAnchorId() == FLY_AT_FLY; }
    bool      ObjAnchoredAtPara() const { return GetAnchorId() == FLY_AT_CNTNT; }
    bool      ObjAnchoredAtChar() const { return GetAnchorId() == FLY_AUTO_CNTNT; }
    bool      ObjAnchoredAsChar() const { return GetAnchorId() == FLY_IN_CNTNT; }

    const SwPosition&  GetCntntAnchor() const
    {
        ASSERT( GetAnchorFmt().GetCntntAnchor(),
                "<SwContact::GetCntntAnchor()> - no content anchor -> crash" );

        return *(GetAnchorFmt().GetCntntAnchor());
    }

    const SwNodeIndex& GetCntntAnchorNode() const;
    const SwIndex&     GetCntntAnchorIndex() const;

    // -------------------------------------------------------------------------
};

//KontactObjekt fuer die Verbindung zwischen Rahmen bzw. deren Formaten
//im StarWriter (SwClient) und den Zeichenobjekten des Drawing (SdrObjUserCall)

class SwFlyDrawContact : public SwContact
{
private:
    // OD 2004-04-01 #i26791#
    SwFlyDrawObj* mpMasterObj;

public:
    TYPEINFO();

    //Legt das DrawObjekt an und meldet es beim Model an.
    SwFlyDrawContact( SwFlyFrmFmt* pToRegisterIn, SdrModel* pMod );
    SwFlyDrawContact( SwFrmFmt* pToRegisterIn, SdrObject* pObj );
    virtual ~SwFlyDrawContact();

    // OD 2004-03-29 #i26791#
    virtual const SwAnchoredObject* GetAnchoredObj( const SdrObject* _pSdrObj ) const;
    virtual SwAnchoredObject* GetAnchoredObj( SdrObject* _pSdrObj );

    // OD 2004-04-01 #i26791#
    virtual const SdrObject* GetMaster() const;
    virtual SdrObject* GetMaster();
    virtual void SetMaster( SdrObject* _pNewMaster );

    SwVirtFlyDrawObj* CreateNewRef( SwFlyFrm* pFly );

    // virtuelle Methoden von SwClient
    virtual void Modify( SfxPoolItem *pOld, SfxPoolItem *pNew );

    // OD 2004-01-16 #110582# - override methods to control Writer fly frames,
    // which are linked, and to assure that all objects anchored at/inside the
    // Writer fly frame are also made visible/invisible.
    virtual void MoveObjToVisibleLayer( SdrObject* _pDrawObj );
    virtual void MoveObjToInvisibleLayer( SdrObject* _pDrawObj );
};


// OD 16.05.2003 #108784# - new class for re-direct methods calls at a 'virtual'
//      drawing object to its referenced object.
class SwDrawVirtObj : public SdrVirtObj
{
    private:
        // data for connection to writer layout
        // OD 2004-03-25 #i26791# - anchored drawing object instance for the
        // 'virtual' drawing object
        SwAnchoredDrawObject maAnchoredDrawObj;

        // writer-drawing contact object the 'virtual' drawing object is controlled by.
        // This object is also the <UserCall> of the drawing object, if it's
        // inserted into the drawing layer.
        SwDrawContact&  mrDrawContact;

    public:
        TYPEINFO();

        SwDrawVirtObj( SdrObject&       _rNewObj,
                       SwDrawContact&   _rDrawContact );
        SwDrawVirtObj( SdrObject&       _rNewObj,
                       const Point&     _rAnchorPos,
                       SwDrawContact&   _rDrawContact );
        virtual ~SwDrawVirtObj();

        // access to offset
        // OD 30.06.2003 #108784# - virtual!!!
        virtual const Point GetOffset() const;

        virtual SdrObject* Clone() const;
        virtual void operator=( const SdrObject& rObj );

        // connection to writer layout
        // OD 2004-03-29 #i26791#
        const SwAnchoredObject* GetAnchoredObj() const;
        SwAnchoredObject* AnchoredObj();
        void SetAnchorFrm( SwFrm* _pNewAnchorFrm );
        const SwFrm* GetAnchorFrm() const;
        SwFrm* AnchorFrm();
        void SetPageFrm( SwPageFrm* _pNewPageFrm );
        SwPageFrm* GetPageFrm();
        void RemoveFromWriterLayout();

        // connection to drawing layer
        void AddToDrawingPage();
        void RemoveFromDrawingPage();

        // is 'virtual' drawing object connected to writer layout and
        // to drawing layer.
        bool IsConnected() const;

        virtual void NbcSetAnchorPos(const Point& rPnt);

        // #108784#
        // All overloaded methods which need to use the offset
        virtual const Rectangle& GetCurrentBoundRect() const;
        virtual const Rectangle& GetLastBoundRect() const;
        virtual void RecalcBoundRect();
        virtual sal_Bool DoPaintObject(ExtOutputDevice& rOut, const SdrPaintInfoRec& rInfoRec) const;
        virtual SdrObject* CheckHit(const Point& rPnt, USHORT nTol, const SetOfByte* pVisiLayer) const;
        virtual void TakeXorPoly(XPolyPolygon& rPoly, FASTBOOL bDetail) const;
        virtual void TakeContour(XPolyPolygon& rPoly) const;
        virtual SdrHdl* GetHdl(USHORT nHdlNum) const;
        virtual SdrHdl* GetPlusHdl(const SdrHdl& rHdl, USHORT nPlNum) const;
        virtual void NbcMove(const Size& rSiz);
        virtual void NbcResize(const Point& rRef, const Fraction& xFact, const Fraction& yFact);
        virtual void NbcRotate(const Point& rRef, long nWink, double sn, double cs);
        virtual void NbcMirror(const Point& rRef1, const Point& rRef2);
        virtual void NbcShear(const Point& rRef, long nWink, double tn, FASTBOOL bVShear);
        virtual void Move(const Size& rSiz);
        virtual void Resize(const Point& rRef, const Fraction& xFact, const Fraction& yFact);
        virtual void Rotate(const Point& rRef, long nWink, double sn, double cs);
        virtual void Mirror(const Point& rRef1, const Point& rRef2);
        virtual void Shear(const Point& rRef, long nWink, double tn, FASTBOOL bVShear);
        virtual void RecalcSnapRect();
        virtual const Rectangle& GetSnapRect() const;
        virtual void SetSnapRect(const Rectangle& rRect);
        virtual void NbcSetSnapRect(const Rectangle& rRect);
        virtual const Rectangle& GetLogicRect() const;
        virtual void SetLogicRect(const Rectangle& rRect);
        virtual void NbcSetLogicRect(const Rectangle& rRect);
        virtual Point GetSnapPoint(USHORT i) const;
        virtual const Point& GetPoint(USHORT i) const;
        virtual void NbcSetPoint(const Point& rPnt, USHORT i);

        // #108784#
        virtual FASTBOOL HasTextEdit() const;

        // OD 17.06.2003 #108784# - overload 'layer' methods
        virtual SdrLayerID GetLayer() const;
        virtual void NbcSetLayer(SdrLayerID nLayer);
        virtual void SetLayer(SdrLayerID nLayer);
};

// OD 26.06.2003 #108784#
bool CheckControlLayer( const SdrObject *pObj );

//KontactObjekt fuer die Verbindung von Formaten als Repraesentanten der
//Zeichenobjekte im StarWriter (SwClient) und den Objekten selbst im Drawing
//(SdrObjUserCall).

class SwDrawContact : public SwContact
{
    private:
        // OD 2004-03-25 #i26791# - anchored drawing object instance for the
        // 'master' drawing object
        SwAnchoredDrawObject maAnchoredDrawObj;

        // OD 2004-04-01 #i26791# - boolean indicating set 'master' drawing
        // object has been cleared.
        bool mbMasterObjCleared;

        // OD 10.10.2003 #112299# - internal flag to indicate that disconnect
        // from layout is in progress
        bool mbDisconnectInProgress;

        // OD 16.05.2003 #108784# - data structure for collecting 'virtual'
        // drawing object supporting drawing objects in headers/footers.
        std::list<SwDrawVirtObj*> maDrawVirtObjs;

        // unary function used by <list> iterator to find a disconnected 'virtual'
        // drawing object
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

        // unary function used by <list> iterator to find a 'virtual' drawing
        // object anchored at a given frame
        struct VirtObjAnchoredAtFrmPred
        {
            const SwFrm* mpAnchorFrm;
            VirtObjAnchoredAtFrmPred( const SwFrm& _rAnchorFrm );
            bool operator() ( const SwDrawVirtObj* _pDrawVirtObj );
        };

        // OD 16.05.2003 #108784# - method for adding/removing 'virtual' drawing object.
        SwDrawVirtObj* CreateVirtObj();
        void DestroyVirtObj( SwDrawVirtObj* pVirtObj );
        void RemoveAllVirtObjs();

        // OD 2004-03-31 #i26791#
        void _InvalidateObjs( const bool _bUpdateSortedObjsList = false );

    public:
        TYPEINFO();

        SwDrawContact( SwFrmFmt *pToRegisterIn, SdrObject *pObj );
        virtual ~SwDrawContact();

        // OD 2004-03-29 #i26791#
        virtual const SwAnchoredObject* GetAnchoredObj( const SdrObject* _pSdrObj = 0L ) const;
        virtual SwAnchoredObject* GetAnchoredObj( SdrObject* _pSdrObj = 0L );

        // OD 2004-04-01 #i26791#
        virtual const SdrObject* GetMaster() const;
        virtual SdrObject* GetMaster();
        virtual void SetMaster( SdrObject* _pNewMaster );

        // OD 2004-03-29 #i26791#
        const SwFrm* GetAnchorFrm( const SdrObject* _pDrawObj = 0L ) const;
        SwFrm* GetAnchorFrm( SdrObject* _pDrawObj = 0L );
        void ChgAnchorFrm( SwFrm* _pNewAnchorFrm )
        {
            maAnchoredDrawObj.ChgAnchorFrm( _pNewAnchorFrm);
        }

        // --> OD 2004-06-30 #i28701# - page frame is now stored at member <maAnchoredDrawObj>
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
        // <--
        void ChkPage();
        SwPageFrm* FindPage( const SwRect &rRect );

        //Fuegt das SdrObject in die Arrays (SwPageFrm und SwFrm) des Layouts ein.
        //Der Anker wird Anhand des Attributes SwFmtAnchor bestimmt.
        //Das Objekt wird ggf. beim alten Anker abgemeldet.
        void ConnectToLayout( const SwFmtAnchor *pAnch = 0 );
        // OD 27.06.2003 #108784# - method to insert 'master' drawing object
        // into drawing page
        void InsertMasterIntoDrawPage();

        void DisconnectFromLayout( bool _bMoveMasterToInvisibleLayer = true );
        // OD 19.06.2003 #108784# - disconnect for a dedicated drawing object -
        // could be 'master' or 'virtual'.
        void DisconnectObjFromLayout( SdrObject* _pDrawObj );
        // OD 26.06.2003 #108784# - method to remove 'master' drawing object
        // from drawing page.
        // To be used by the undo for delete of object. Call it after method
        // <DisconnectFromLayout( bool = true )> is already performed.
        // Note: <DisconnectFromLayout( bool )> no longer removes the 'master'
        // drawing object from drawing page.
        void RemoveMasterFromDrawPage();

        // OD 19.06.2003 #108784# - get drawing object ('master' or 'virtual')
        // by frame.
        SdrObject* GetDrawObjectByAnchorFrm( const SwFrm& _rAnchorFrm );

        // virtuelle Methoden von SwClient
        virtual void Modify( SfxPoolItem *pOld, SfxPoolItem *pNew );

        // virtuelle Methoden von SdrObjUserCall
        virtual void Changed(const SdrObject& rObj, SdrUserCallType eType, const Rectangle& rOldBoundRect);

        // wird von Changed() und auch vom UndoDraw benutzt, uebernimmt
        // das Notifien von Absaetzen, die ausweichen muessen
        void _Changed(const SdrObject& rObj, SdrUserCallType eType, const Rectangle* pOldBoundRect);

        //Moved alle SW-Verbindungen zu dem neuen Master.
        void ChangeMasterObject( SdrObject *pNewMaster );

        // OD 19.06.2003 #108784#
        SwDrawVirtObj* AddVirtObj();

        // OD 20.06.2003 #108784#
        void NotifyBackgrdOfAllVirtObjs( const Rectangle* pOldBoundRect );
};

#endif

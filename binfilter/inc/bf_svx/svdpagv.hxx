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

#ifndef _SVDPAGV_HXX
#define _SVDPAGV_HXX

#include <tools/debug.hxx>

#include <com/sun/star/awt/XWindowListener.hpp>
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#include <com/sun/star/awt/XImageConsumer.hpp>
#include <com/sun/star/awt/XControl.hpp>
#include <com/sun/star/awt/XControlContainer.hpp>
#include <com/sun/star/util/XModeChangeListener.hpp>

#include <bf_svtools/lstner.hxx>
#include <vcl/svapp.hxx>
#include <bf_svx/svdlayer.hxx>
#include <bf_svx/svdhlpln.hxx>

#include <osl/mutex.hxx>
class Region;
class Link;
class OutputDevice;
class SdrObjListIter;
namespace binfilter {

////////////////////////////////////////////////////////////////////////////////////////////////////

class ExtOutputDevice;
class SdrPaintInfoRec;
class XPolyPolygon;
class SdrObjList;
class SdrObject;
class SdrModel;
class SdrPage;
class SdrUnoObj;
class SdrPageObj;
class SdrView;

////////////////////////////////////////////////////////////////////////////////////////////////////

class SdrPageViewWinList;
}//end of namespace binfilter
#if _SOLAR__PRIVATE

#include <cppuhelper/implbase4.hxx> 
namespace binfilter {
#define SDRUNOCONTROL_NOTFOUND USHRT_MAX

class SdrUnoControlList;

//	Listener an den UnoControls
class SdrUnoControlRec : public ::cppu::WeakImplHelper4<
                                    ::com::sun::star::awt::XWindowListener,
                                    ::com::sun::star::beans::XPropertyChangeListener,
                                    ::com::sun::star::awt::XImageConsumer,
                                    ::com::sun::star::util::XModeChangeListener
                                    >
{
    SdrUnoControlList*		pParent;
    SdrUnoObj*				pObj;
    bool                    bVisible : 1;
    bool                    bDisposed : 1;
    bool                    bIsListening : 1;
    sal_uInt16              mnPaintLevel;

    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl > xControl;

public:
    SdrUnoControlRec(SdrUnoControlList*	_pParent, SdrUnoObj* _pObj, ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl > _xControl) throw();
    ~SdrUnoControlRec() throw();

protected:
    // XEventListener
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException);

    // XWindowListener
    virtual void SAL_CALL windowResized( const ::com::sun::star::awt::WindowEvent& e ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL windowMoved( const ::com::sun::star::awt::WindowEvent& e ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL windowShown( const ::com::sun::star::lang::EventObject& e ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL windowHidden( const ::com::sun::star::lang::EventObject& e ) throw(::com::sun::star::uno::RuntimeException);

    // XPropertyChangeListener
    virtual void SAL_CALL propertyChange( const ::com::sun::star::beans::PropertyChangeEvent& evt ) throw(::com::sun::star::uno::RuntimeException);

    // XImageConsumer
    virtual void SAL_CALL init( sal_Int32 Width, sal_Int32 Height ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setColorModel( sal_Int16 BitCount, const ::com::sun::star::uno::Sequence< sal_Int32 >& RGBAPal, sal_Int32 RedMask, sal_Int32 GreenMask, sal_Int32 BlueMask, sal_Int32 AlphaMask ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPixelsByBytes( sal_Int32 nX, sal_Int32 nY, sal_Int32 nWidth, sal_Int32 nHeight, const ::com::sun::star::uno::Sequence< sal_Int8 >& aProducerData, sal_Int32 nOffset, sal_Int32 nScanSize ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPixelsByLongs( sal_Int32 nX, sal_Int32 nY, sal_Int32 nWidth, sal_Int32 nHeight, const ::com::sun::star::uno::Sequence< sal_Int32 >& aProducerData, sal_Int32 nOffset, sal_Int32 nScanSize ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL complete( sal_Int32 Status, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XImageProducer >& xProducer ) throw(::com::sun::star::uno::RuntimeException);

    // XModeChangeListener
    virtual void SAL_CALL modeChanged( const ::com::sun::star::util::ModeChangeEvent& _rSource ) throw (::com::sun::star::uno::RuntimeException);

public:
    // More functions
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl > GetControl() const {return xControl;}
    BOOL               IsVisible() const {return bVisible;}
    BOOL               IsListening() const {return bIsListening;}
    SdrUnoObj*         GetUnoObj() const { return pObj; }

    // Setzen eines neuen Controls
    void Clear(BOOL bDispose);

    struct AccessControl { friend class SdrUnoControlPaintGuard; private: AccessControl() { } };
    inline void EnterPaint( const AccessControl& )  { ++mnPaintLevel; }
    inline void LeavePaint( const AccessControl& )  { DBG_ASSERT( mnPaintLevel, "SdrUnoControlRec::LeavePaint: not locked!" ); --mnPaintLevel; }

    // in alive mode, the visibility of the control must be adjusted to the visibility of the layer
    void    adjustControlVisibility( bool _bForce );

private:
    // starts or stops listening for any relevant events on the control or it's model
    void    switchControlListening( bool _bStart );

};

// class for temporarily (during painting) locking some functionality of the SdrUnoControlRec
class SdrUnoControlPaintGuard
{
private:
    SdrUnoControlRec&  m_rRec;

public:
    inline SdrUnoControlPaintGuard( SdrUnoControlRec& _rRec )
        :m_rRec( _rRec )
    {
        m_rRec.EnterPaint( SdrUnoControlRec::AccessControl( ) );
    }
    inline ~SdrUnoControlPaintGuard( )
    {
        m_rRec.LeavePaint( SdrUnoControlRec::AccessControl( ) );
    }
};

// Klasse fuer den schnellen Zugriff auf Recs ueber das ControlModel
class SdrUnoControlAccess
{
public:
    SdrUnoControlRec* pControlRec;
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > xControlModel;

    SdrUnoControlAccess(::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > _xControlModel, SdrUnoControlRec* _pControlRec = NULL)
        :pControlRec( _pControlRec )
        ,xControlModel( _xControlModel )
    {};
    SdrUnoControlAccess()
    {
        pControlRec = NULL;
    };

    BOOL operator==( const SdrUnoControlAccess& rAcc ) const
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > xInt1( xControlModel, ::com::sun::star::uno::UNO_QUERY );
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > xInt2( rAcc.xControlModel, ::com::sun::star::uno::UNO_QUERY );
        return ((::com::sun::star::uno::XInterface*)xInt1.get()) == ((::com::sun::star::uno::XInterface*)xInt2.get());
    }
    BOOL operator<( const SdrUnoControlAccess& rAcc ) const
    {
        return xControlModel.get() < rAcc.xControlModel.get();
    }
};

typedef SdrUnoControlAccess* SdrUnoControlAccessPtr;
SV_DECL_PTRARR_SORT( SdrUnoControlAccessArr, SdrUnoControlAccessPtr, 16, 16 )

class SdrPageView;
class SdrUnoControlList
{
    friend class SdrUnoControlRec;

    Container aList;
    SdrUnoControlAccessArr aAccessArr;
    SdrPageView&            rPageView;

protected:
    SdrUnoControlRec* GetObject(USHORT i) const;

public:
    SdrUnoControlList( SdrPageView& _rPageView )
        :aList(1024,4,4)
        ,rPageView( _rPageView )
    {}
    ~SdrUnoControlList()                                              { Clear(TRUE); }
    void              Clear(BOOL bDispose);
    USHORT            GetCount() const                                { return (USHORT)aList.Count(); }
    void              Insert(SdrUnoControlRec* pRec);
    void              Delete(USHORT nPos, BOOL bDispose);
    SdrUnoControlRec&       operator[](USHORT nPos)                    { return *GetObject(nPos); }
    const SdrUnoControlRec& operator[](USHORT nPos) const              { return *GetObject(nPos); }
    USHORT            Find(::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > rUnoControlModel) const;
};

class SdrPageViewWinRec
{
friend class SdrPageView;
    SdrView&      		 rView;
    OutputDevice*        pOutDev;
    SdrUnoControlList    aControlList;
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlContainer > xControlContainer;

protected:
    void                 CreateControlContainer();

public:
    SdrPageViewWinRec(SdrPageView& rNewPageView, OutputDevice* pOutDev);
    ~SdrPageViewWinRec();

    BOOL                     MatchOutputDevice(OutputDevice* pOut) const;
    OutputDevice*            GetOutputDevice() const { return pOutDev; }
    const SdrUnoControlList& GetControlList() const { return aControlList; }
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlContainer >     GetControlContainerRef() const { return xControlContainer; }
};

#define SDRPAGEVIEWWIN_NOTFOUND 0xFFFF

class SdrPageViewWinList {
    Container aList;
private:
    SdrPageViewWinList(const SdrPageViewWinList& rSource): aList(1024,4,4) {}
    void operator=(const SdrPageViewWinList& rSource)                  {}
protected:
    SdrPageViewWinRec* GetObject(USHORT i) const { return (SdrPageViewWinRec*)(aList.GetObject(i)); }
public:
    SdrPageViewWinList(): aList(1024,4,4) {}
    ~SdrPageViewWinList()                                                    { Clear(); }
    void                     Clear();
    USHORT                   GetCount() const                                { return (USHORT)aList.Count(); }
    void                     Insert(SdrPageViewWinRec* pVWR, USHORT nPos=0xFFFF) { aList.Insert(pVWR,nPos); }
    void                     Delete(USHORT nPos)                             { delete (SdrPageViewWinRec*)aList.Remove(nPos); }
    SdrPageViewWinRec&       operator[](USHORT nPos)                         { return *GetObject(nPos); }
    const SdrPageViewWinRec& operator[](USHORT nPos) const                   { return *GetObject(nPos); }
    USHORT                   Find(OutputDevice* pW) const;
};
}//end of namespace binfilter
#endif // __PRIVATE

////////////////////////////////////////////////////////////////////////////////////////////////////
namespace binfilter {
class SdrPageView: public SfxListener {
friend class SdrPaintView;
//friend class SdrSnapView;
//friend class SdrMarkView;
//friend class SdrView;
protected:
//    Container     aRedraw;  // Ein Eintrag fuer jedes Win. Dieser Container
    SdrView&      rView;    // deckt sich mit der Fensterliste der SdrView.
    SdrPage*      pPage;
    Point         aPgOrg;   // Nullpunkt der Page
    Point         aOfs;     // Offset der Page im Win
    //Timer      aGetBack;

    Rectangle     aMarkBound; // wird
    Rectangle     aMarkSnap;  // von
    XPolyPolygon* pDragPoly0; // SdrView
    XPolyPolygon* pDragPoly;  //
    FASTBOOL      bHasMarked; // verwendet
    FASTBOOL      bVisible;   // Sichtbar?

    SetOfByte    aLayerVisi;   // Menge der sichtbaren Layer
    SetOfByte    aLayerLock;   // Menge der nicht editierbaren Layer
    SetOfByte    aLayerPrn;    // Menge der druckbaren Layer

    SdrObjList*  pAktList;     // Aktuelle Liste, in der Regel die Page.
    SdrObject*   pAktGroup;    // Aktuelle Gruppe. NULL=Keine.

    SdrHelpLineList aHelpLines; // Hilfslinien und -punkte

    // Reserve fuer kompatible Erweiterungen die sonst inkompatibel wuerden

    // #103834# Use one reserved slot (bReserveBool1) for the background color
    Color		  maBackgroundColor;

    // #103911# Use one reserved slot (bReserveBool2) for the document color
    Color		  maDocumentColor;

    long          nReserveLong1;
    long          nReserveLong2;
    SdrPageViewWinList* pWinList;
    SdrPageObj*   pPaintingPageObj;
    Point         aReservePoint1;
    Point         aReservePoint2;
    Rectangle     aReserveRect1;
    String        aReserveString1;

#if _SOLAR__PRIVATE
private:
    SdrPageViewWinRec* ImpMakePageViewWinRec(OutputDevice* pOutDev1);
    void        ImpInsertControl(const SdrUnoObj* pObj, SdrPageViewWinRec* pRec);
    void        ImpUnoInserted(const SdrObject* pObj);
protected:
#endif // __PRIVATE
protected:
    //USHORT      GetWinCount() const       { return USHORT(aRedraw.Count()); }
    //SdrPageWin* GetWin(USHORT nNum) const { return (SdrPageWin*)(aRedraw.GetObject(nNum)); }
    //void        AddWin()                  { aRedraw.Insert(new SdrPageWin,CONTAINER_APPEND); }
    //void        DelWin(USHORT nNum)       { delete aRedraw.Remove(nNum); }
    //void        ClearWinList();
    // Jeder ContainerEntry muss auf ein OutputDevice zeigen:
    //void        SetWinList(const Container& rWinList);

    void AddWin(OutputDevice* pOutDev1);
    void DelWin(OutputDevice* pOutDev1);

    FASTBOOL IsLayer(const String& rName, const SetOfByte& rBS) const;

    virtual void SFX_NOTIFY(SfxBroadcaster& rBC, const TypeId& rBCType, const SfxHint& rHint, const TypeId& rHintType);
    void Show();
    void Hide();
    // Nachsehen, ob AktGroup noch Inserted ist.
    void CheckAktGroup();
    // Wird von der PaintView gerufen, wenn Modelaenderungen abgeschlossen sind
    void ModelHasChanged();
public:
    TYPEINFO();
    SdrPageView(SdrPage* pPage1, const Point& rOffs, SdrView& rNewView);
    ~SdrPageView();
    SdrView& GetView()                                          { return rView; }
    const SdrView& GetView() const                              { return rView; }
    const SdrPageViewWinList& GetWinList() const                { return *pWinList; }

    FASTBOOL IsVisible() const                                  { return bVisible; }
    // Invalidiert den gesamten Bereich der Page
    void InvalidateAllWin();
    // rRect bezieht sich auf die Page
    void InvalidateAllWin(const Rectangle& rRect, FASTBOOL bPlus1Pix=FALSE);
    // rReg bezieht sich auf's OutDev, nicht auf die Page
    void InitRedraw(OutputDevice* pOut, const Region& rReg, USHORT nPaintMode=0, const Link* pPaintProc=NULL){DBG_BF_ASSERT(0, "STRIP");}//STRIP001 	void InitRedraw(OutputDevice* pOut, const Region& rReg, USHORT nPaintMode=0, const Link* pPaintProc=NULL);
    // rReg bezieht sich auf's OutDev, nicht auf die Page
    // Painten aller Objekte eines Layer der Page ohne MasterPage
    // pOut==NULL -> alle OutputDevices
    // Der Handler in pPaintProc hat folgendes aussehen:
    // void __EXPORT class::PaintProcHdl(const SdrPaintProcRec& rPaintProcRec);


    // MasterPage ueber VirtualDevice Painten

    const Point& GetOffset() const                              { return aOfs; }
    Rectangle GetPageRect() const;
    SdrPage* GetPage() const                                    { return pPage; }

    // Betretene Liste rausreichen
    SdrObjList* GetObjList() const                              { return pAktList; }
    // Betretene Gruppe rausreichen
    SdrObject* GetAktGroup() const								{ return pAktGroup; }
    // Betretene Gruppe und Liste setzen
    void SetAktGroupAndList(SdrObject* pNewGroup, SdrObjList* pNewList);

    FASTBOOL HasMarkedObj() const                               { return bHasMarked; }
    void     SetHasMarkedObj(FASTBOOL bOn)                      { bHasMarked=bOn; }

    const Rectangle& MarkBound() const                          { return aMarkBound; }
    const Rectangle& MarkSnap() const                           { return aMarkSnap; }
    Rectangle&       MarkBound()                                { return aMarkBound; }
    Rectangle&       MarkSnap()                                 { return aMarkSnap; }


    FASTBOOL IsLayerVisible(const String& rName) const              { return IsLayer(rName,aLayerVisi); }

    FASTBOOL IsLayerLocked(const String& rName) const               { return IsLayer(rName,aLayerLock); }

    FASTBOOL IsLayerPrintable(const String& rName) const            { return IsLayer(rName,aLayerPrn); }

    // PV stellt eine RefPage oder eine SubList eines RefObj dar oder Model ist ReadOnly

    // der Origin bezieht sich immer auf die obere linke Ecke der Page
    const Point& GetPageOrigin() const                          { return aPgOrg; }
    void LogicToPagePos(Point& rPnt) const                      { rPnt-=aPgOrg; }
    void LogicToPagePos(Rectangle& rRect) const                 { rRect.Move(-aPgOrg.X(),-aPgOrg.Y()); }
    void PagePosToLogic(Point& rPnt) const                      { rPnt+=aPgOrg; }
    void PagePosToLogic(Rectangle& rRect) const                 { rRect.Move(aPgOrg.X(),aPgOrg.Y()); }

    void SetVisibleLayers(const SetOfByte& rSet)                { aLayerVisi=rSet; InvalidateAllWin(); }
    const SetOfByte& GetVisibleLayers() const                   { return aLayerVisi; }
    void SetPrintableLayers(const SetOfByte& rSet)              { aLayerPrn=rSet; }
    const SetOfByte& GetPrintableLayers() const                 { return aLayerPrn;  }
    void SetLockedLayers(const SetOfByte& rSet)                 { aLayerLock=rSet; }
    const SetOfByte& GetLockedLayers() const                    { return aLayerLock; }

    const SdrHelpLineList& GetHelpLines() const                 { return aHelpLines; }
    void SetHelpLine(USHORT nNum, const SdrHelpLine& rNewHelpLine);
    void MoveHelpLine(USHORT nNum, USHORT nNewNum)              { aHelpLines.Move(nNum,nNewNum); }

    // Liefert TRUE, wenn Layer des Obj sichtbar und nicht gesperrt.
    // Beim Gruppenobjekt muss wenigstens ein Member sichtbar sein,
    // gesperrt sein darf keiner.
    FASTBOOL IsObjMarkable(SdrObject* pObj) const;

    // Betreten (Editieren) einer Objektgruppe. Anschliessend liegen alle
    // Memberobjekte der Gruppe im direkten Zugriff. Alle anderen Objekte
    // koennen waerendessen nicht bearbeitet werden (bis zum naechsten
    // LeaveGroup()). (wie MsDos chdir bla).
    // Verlassen einer betretenen Objektgruppe. (wie MsDos chdir ..)
    // Verlassen aller betretenen Objektgruppen. (wie MsDos chdir \)
    void LeaveAllGroup();
    // Name der aktuellen Objektgruppe
    // Die Namen aller z.Zt. betretenen Gruppen

    const XPolyPolygon& DragPoly0() const { return *pDragPoly0; }
    const XPolyPolygon& DragPoly () const { return *pDragPoly;  }
    XPolyPolygon& DragPoly0()             { return *pDragPoly0; }
    XPolyPolygon& DragPoly ()             { return *pDragPoly;  }

    void        SetPaintingPageObj( SdrPageObj* pObj ) { pPaintingPageObj = pObj; }
    SdrPageObj* GetPaintingPageObj() const { return pPaintingPageObj; }

    friend SvStream& operator<<(SvStream& rOut, const SdrPageView& rPageView);
    friend SvStream& operator>>(SvStream& rIn, SdrPageView& rPageView);

    // #103834# Set background color for svx at SdrPageViews
    void SetApplicationBackgroundColor(Color aBackgroundColor);

    // #103911# Set/Get document color for svx at SdrPageViews
    void SetApplicationDocumentColor(Color aDocumentColor);
    Color GetApplicationDocumentColor() const;
};

}//end of namespace binfilter
#endif //_SVDPAGV_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: svdpagv.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 16:26:48 $
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

#ifndef _SVDPAGV_HXX
#define _SVDPAGV_HXX

#ifndef _COM_SUN_STAR_AWT_XCONTROLCONTAINER_HPP_
#include <com/sun/star/awt/XControlContainer.hpp>
#endif

#ifndef _SFXLSTNER_HXX //autogen
#include <svtools/lstner.hxx>
#endif

#ifndef _SVDHLPLN_HXX
#include <svx/svdhlpln.hxx>
#endif

#ifndef _CPPUHELPER_IMPLBASE4_HXX_
#include <cppuhelper/implbase4.hxx>
#endif

#ifndef _SVDSOB_HXX
#include <svx/svdsob.hxx>
#endif

#ifndef _SVDTYPES_HXX
#include <svx/svdtypes.hxx>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

#include <cppuhelper/implbase3.hxx>
#include <vector>

#ifndef _BGFX_POLYGON_B2DPOLYPOLYGON_HXX
#include <basegfx/polygon/b2dpolypolygon.hxx>
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////

class Region;
class SdrObjList;
class SdrObject;
class SdrPage;
class SdrUnoObj;
class SdrPaintWindow;
class SdrView;
class SdrPageObj;
class B2dIAOManager;
class SdrPageView;

// #110094#
namespace sdr
{
    namespace contact
    {
        class ViewObjectContactRedirector;
        class DisplayInfo;
        class ViewObjectContactRedirector;
    } // end of namespace contact
} // end of namespace sdr

// typedefs for a list of SdrPageWindow
class SdrPageWindow;
typedef ::std::vector< SdrPageWindow* > SdrPageWindowVector;

////////////////////////////////////////////////////////////////////////////////////////////////////

class SVX_DLLPUBLIC SdrPageView : public SfxListener
{
    const ::sdr::contact::DisplayInfo*                              mpDisplayInfo;

protected:
    SdrView&                                                        mrView;
    SdrPage*                                                        mpPage;
    Point         aPgOrg;   // Nullpunkt der Page

    Rectangle     aMarkBound; // wird
    Rectangle     aMarkSnap;  // von
    basegfx::B2DPolyPolygon     maDragPoly0; // SdrView
    basegfx::B2DPolyPolygon     maDragPoly;  //
    sal_Bool                                                        mbHasMarked;
    sal_Bool                                                        mbVisible;

    SetOfByte    aLayerVisi;   // Menge der sichtbaren Layer
    SetOfByte    aLayerLock;   // Menge der nicht editierbaren Layer
    SetOfByte    aLayerPrn;    // Menge der druckbaren Layer

    SdrObjList*  pAktList;     // Aktuelle Liste, in der Regel die Page.
    SdrObject*   pAktGroup;    // Aktuelle Gruppe. NULL=Keine.

    SdrHelpLineList aHelpLines; // Hilfslinien und -punkte

    // #103911# Use one reserved slot (bReserveBool2) for the document color
    Color         maDocumentColor;

    // #103834# Use one reserved slot (bReserveBool1) for the background color
    Color         maBackgroundColor;

    SdrPageWindowVector                                         maPageWindows;

    // #i72752# member to remember with which SdrPageWindow the BeginDrawLayer
    // was done
    SdrPageWindow*                                              mpPreparedPageWindow;

    // interface to SdrPageWindow
protected:
    void ClearPageWindows();
    void AppendPageWindow(SdrPageWindow& rNew);
    SdrPageWindow* RemovePageWindow(sal_uInt32 nPos);
    SdrPageWindow* RemovePageWindow(SdrPageWindow& rOld);
public:
    sal_uInt32 PageWindowCount() const { return maPageWindows.size(); }
    SdrPageWindow* FindPageWindow( SdrPaintWindow& rPaintWindow ) const;
    SdrPageWindow* FindPageWindow( const OutputDevice& rOutDev ) const;
    SdrPageWindow* GetPageWindow(sal_uInt32 nIndex) const;

    void PaintOutlinerView(OutputDevice* pOut, const Rectangle& rRect) const;
private:
    SVX_DLLPRIVATE SdrPageWindow& CreateNewPageWindowEntry(SdrPaintWindow& rPaintWindow);

protected:
    void ImpInvalidateHelpLineArea(USHORT nNum) const;

protected:
    void SetLayer(const String& rName, SetOfByte& rBS, sal_Bool bJa);
    sal_Bool IsLayer(const String& rName, const SetOfByte& rBS) const;
    void SetAllLayers(SetOfByte& rB, sal_Bool bJa);

    virtual void SFX_NOTIFY(SfxBroadcaster& rBC, const TypeId& rBCType, const SfxHint& rHint, const TypeId& rHintType);

    // Nachsehen, ob AktGroup noch Inserted ist.
    void CheckAktGroup();

    void AdjHdl();

public:
    TYPEINFO();
    SdrPageView(SdrPage* pPage1, SdrView& rNewView);
    ~SdrPageView();

    // Wird von der PaintView gerufen, wenn Modelaenderungen abgeschlossen sind
    void ModelHasChanged();

    void Show();
    void Hide();

    void AddPaintWindowToPageView(SdrPaintWindow& rPaintWindow);
    void RemovePaintWindowFromPageView(SdrPaintWindow& rPaintWindow);

    SdrView& GetView() { return mrView; }
    const SdrView& GetView() const { return mrView; }

    /** looks up the control container belonging to given output device

        @return
            If the given output device belongs to one of the SdrPageViewWinRecs associated with this
            SdrPageView instance, the XControlContainer for this output device is returned, <NULL/>
            otherwise.
    */
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlContainer >
        GetControlContainer( const OutputDevice& _rDevice ) const;

    /** sets all elements in the view which support a design and a alive mode into the given mode
    */
    void    SetDesignMode( bool _bDesignMode ) const;

    sal_Bool IsVisible() const { return mbVisible; }

    // Invalidiert den gesamten Bereich der Page
    void InvalidateAllWin();

    // rRect bezieht sich auf die Page
    void InvalidateAllWin(const Rectangle& rRect, sal_Bool bPlus1Pix=FALSE);

    // rReg bezieht sich auf's OutDev, nicht auf die Page
    void CompleteRedraw(
        SdrPaintWindow& rPaintWindow, const Region& rReg, sal_uInt16 nPaintMode,
        ::sdr::contact::ViewObjectContactRedirector* pRedirector = 0L) const;
    void BeginDrawLayer(OutputDevice* pGivenTarget, const Region& rReg, sal_Bool bPrepareBuffered);
    void EndDrawLayer(OutputDevice* pGivenTarget);
    void DrawLayer(SdrLayerID nID, OutputDevice* pGivenTarget = 0L, sal_uInt16 nPaintMode = 0, ::sdr::contact::ViewObjectContactRedirector* pRedirector = 0L) const;
    void DrawPageViewGrid(OutputDevice& rOut, const Rectangle& rRect, Color aColor = Color( COL_BLACK ) );

    Rectangle GetPageRect() const;
    SdrPage* GetPage() const { return mpPage; }

    // Betretene Liste rausreichen
    SdrObjList* GetObjList() const { return pAktList; }

    // Betretene Gruppe rausreichen
    SdrObject* GetAktGroup() const { return pAktGroup; }

    // Betretene Gruppe und Liste setzen
    void SetAktGroupAndList(SdrObject* pNewGroup, SdrObjList* pNewList);

    sal_Bool HasMarkedObjPageView() const { return mbHasMarked; }
    void SetHasMarkedObj(sal_Bool bOn) { mbHasMarked = bOn; }

    const Rectangle& MarkBound() const { return aMarkBound; }
    const Rectangle& MarkSnap() const { return aMarkSnap; }
    Rectangle& MarkBound() { return aMarkBound; }
    Rectangle& MarkSnap() { return aMarkSnap; }

    void SetLayerVisible(const String& rName, sal_Bool bShow = sal_True) { SetLayer(rName, aLayerVisi, bShow); if(!bShow) AdjHdl(); InvalidateAllWin(); }
    sal_Bool IsLayerVisible(const String& rName) const { return IsLayer(rName, aLayerVisi); }
    void SetAllLayersVisible(sal_Bool bShow = sal_True) { SetAllLayers(aLayerVisi, bShow); if(!bShow) AdjHdl(); InvalidateAllWin(); }

    void SetLayerLocked(const String& rName, sal_Bool bLock = sal_True) { SetLayer(rName, aLayerLock, bLock); if(bLock) AdjHdl(); }
    sal_Bool IsLayerLocked(const String& rName) const { return IsLayer(rName,aLayerLock); }
    void SetAllLayersLocked(sal_Bool bLock = sal_True) { SetAllLayers(aLayerLock, bLock); if(bLock) AdjHdl(); }

    void SetLayerPrintable(const String& rName, sal_Bool bPrn = sal_True) { SetLayer(rName, aLayerPrn, bPrn); }
    sal_Bool IsLayerPrintable(const String& rName) const { return IsLayer(rName, aLayerPrn); }
    void SetAllLayersPrintable(sal_Bool bPrn = sal_True) { SetAllLayers(aLayerPrn, bPrn); }

    // PV stellt eine RefPage oder eine SubList eines RefObj dar oder Model ist ReadOnly
    sal_Bool IsReadOnly() const;

    // der Origin bezieht sich immer auf die obere linke Ecke der Page
    const Point& GetPageOrigin() const { return aPgOrg; }
    void SetPageOrigin(const Point& rOrg);

    void LogicToPagePos(Point& rPnt) const { rPnt-=aPgOrg; }
    void LogicToPagePos(Rectangle& rRect) const { rRect.Move(-aPgOrg.X(),-aPgOrg.Y()); }
    void PagePosToLogic(Point& rPnt) const { rPnt+=aPgOrg; }
    void PagePosToLogic(Rectangle& rRect) const { rRect.Move(aPgOrg.X(),aPgOrg.Y()); }

    void SetVisibleLayers(const SetOfByte& rSet) { aLayerVisi=rSet; InvalidateAllWin(); }
    const SetOfByte& GetVisibleLayers() const { return aLayerVisi; }
    void SetPrintableLayers(const SetOfByte& rSet) { aLayerPrn=rSet; }
    const SetOfByte& GetPrintableLayers() const { return aLayerPrn;  }
    void SetLockedLayers(const SetOfByte& rSet) { aLayerLock=rSet; }
    const SetOfByte& GetLockedLayers() const { return aLayerLock; }

    const SdrHelpLineList& GetHelpLines() const { return aHelpLines; }
    void SetHelpLines(const SdrHelpLineList& rHLL);
    //void SetHelpLinePos(USHORT nNum, const Point& rNewPos);
    void SetHelpLine(USHORT nNum, const SdrHelpLine& rNewHelpLine);
    void DeleteHelpLine(USHORT nNum);
    void InsertHelpLine(const SdrHelpLine& rHL, USHORT nNum=0xFFFF);
    void MoveHelpLine(USHORT nNum, USHORT nNewNum) { aHelpLines.Move(nNum,nNewNum); }

    // Liefert TRUE, wenn Layer des Obj sichtbar und nicht gesperrt.
    // Beim Gruppenobjekt muss wenigstens ein Member sichtbar sein,
    // gesperrt sein darf keiner.
    sal_Bool IsObjMarkable(SdrObject* pObj) const;

    // Betreten (Editieren) einer Objektgruppe. Anschliessend liegen alle
    // Memberobjekte der Gruppe im direkten Zugriff. Alle anderen Objekte
    // koennen waerendessen nicht bearbeitet werden (bis zum naechsten
    // LeaveGroup()). (wie MsDos chdir bla).
    sal_Bool EnterGroup(SdrObject* pObj);

    // Verlassen einer betretenen Objektgruppe. (wie MsDos chdir ..)
    void LeaveOneGroup();

    // Verlassen aller betretenen Objektgruppen. (wie MsDos chdir \)
    void LeaveAllGroup();

    // Feststellen, wie weit hinabgestiegen wurde (0=Root(Page))
    USHORT GetEnteredLevel() const;

    // Name der aktuellen Objektgruppe
    String GetActualGroupName() const;

    // Die Namen aller z.Zt. betretenen Gruppen
    String GetActualPathName(sal_Unicode cSep = sal_Unicode('|')) const;

    const basegfx::B2DPolyPolygon& getDragPoly0() const { return maDragPoly0; }
    const basegfx::B2DPolyPolygon& getDragPoly() const { return maDragPoly;  }
    void setDragPoly0(const basegfx::B2DPolyPolygon& rNew) { maDragPoly0 = rNew; }
    void setDragPoly(const basegfx::B2DPolyPolygon& rNew) { maDragPoly = rNew;  }

    // #103834# Set background color for svx at SdrPageViews
    void SetApplicationBackgroundColor(Color aBackgroundColor);

    // #109585#
    Color GetApplicationBackgroundColor() const;

    // #103911# Set/Get document color for svx at SdrPageViews
    void SetApplicationDocumentColor(Color aDocumentColor);
    Color GetApplicationDocumentColor() const;

    void SetCurrentPaintingDisplayInfo(const ::sdr::contact::DisplayInfo* pDisplayInfo)
    {
        if(pDisplayInfo != mpDisplayInfo)
        {
            mpDisplayInfo = pDisplayInfo;
        }
    }

    const ::sdr::contact::DisplayInfo* GetCurrentPaintingDisplayInfo() const
    {
        return mpDisplayInfo;
    }

    // find out if form controls are used by this PageView
    sal_Bool AreFormControlsUsed(SdrPaintWindow& rPaintWindow) const;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //_SVDPAGV_HXX

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



#ifndef _SVDPAGV_HXX
#define _SVDPAGV_HXX

#include <com/sun/star/awt/XControlContainer.hpp>
#include <svl/lstner.hxx>
#include <svx/svdhlpln.hxx>
#include <cppuhelper/implbase4.hxx>
#include <svx/svdsob.hxx>
#include <svx/svdtypes.hxx>
#include "svx/svxdllapi.h"
#include <cppuhelper/implbase3.hxx>
#include <vector>
#include <basegfx/polygon/b2dpolypolygon.hxx>

////////////////////////////////////////////////////////////////////////////////////////////////////
// predefines

class Region;
class SdrObjList;
class SdrObject;
class SdrPage;
class SdrPaintWindow;
class SdrView;
class SdrPageWindow;
class OutputDevice;
namespace sdr { namespace contact { class ViewObjectContactRedirector; }}

////////////////////////////////////////////////////////////////////////////////////////////////////

class SVX_DLLPUBLIC SdrPageView
{
private:
    SVX_DLLPRIVATE SdrPageWindow& CreateNewPageWindowEntry(SdrPaintWindow& rPaintWindow);

protected:
    SdrView&                                                        mrView;
    SdrPage&                    mrSdrPageOfSdrPageView;
    basegfx::B2DPoint           maPageOrigin;   // Nullpunkt der Page

    SetOfByte                   maVisibleLayerSet;   // Menge der sichtbaren Layer
    SetOfByte                   maLockedLayerSet;   // Menge der nicht editierbaren Layer
    SetOfByte                   maPrintableLayerSet;    // Menge der druckbaren Layer

    SdrObjList*                 mpCurrentList;     // Aktuelle Liste, in der Regel die Page.
    SdrObject*                  mpCurrentGroup;    // Aktuelle Gruppe. NULL=Keine.

    SdrHelpLineList             maHelpLines; // Hilfslinien und -punkte

    // #103911# Use one reserved slot (bReserveBool2) for the document color
    Color         maDocumentColor;

    // #103834# Use one reserved slot (bReserveBool1) for the background color
    Color         maBackgroundColor;

    typedef ::std::vector< SdrPageWindow* > SdrPageWindowVector;
    SdrPageWindowVector                                         maPageWindows;

    // #i72752# member to remember with which SdrPageWindow the BeginDrawLayer
    // was done
    SdrPageWindow*                                              mpPreparedPageWindow;

    /// bitfield
    bool                        mbVisible : 1;

    // interface to SdrPageWindow
    void ClearPageWindows();
    void AppendPageWindow(SdrPageWindow& rNew);
    SdrPageWindow* RemovePageWindow(sal_uInt32 nPos);
    SdrPageWindow* RemovePageWindow(SdrPageWindow& rOld);

    void ImpInvalidateHelpLineArea(sal_uInt16 nNum) const;

    void SetLayer(const String& rName, SetOfByte& rBS, bool bJa);
    bool IsLayer(const String& rName, const SetOfByte& rBS) const;
    void SetAllLayers(SetOfByte& rB, bool bJa);

    // Nachsehen, ob AktGroup noch Inserted ist.
    void CheckAktGroup();
    void AdjHdl();

public:
    SdrPageView(SdrPage& rSdrPageOfSdrPageView, SdrView& rNewView);
    ~SdrPageView();

    sal_uInt32 PageWindowCount() const { return maPageWindows.size(); }
    SdrPageWindow* FindPageWindow( SdrPaintWindow& rPaintWindow ) const;
    SdrPageWindow* FindPageWindow( const OutputDevice& rOutDev ) const;
    SdrPageWindow* GetPageWindow(sal_uInt32 nIndex) const;

    /** finds the page window whose PaintWindow belongs to the given output device

        In opposite to FindPageWindow, this method also cares possibly patched PaintWindow instances.
        That is, a SdrPageWindow might have an original, and a patched SdrPaintWindow instance - if
        this is the case, then the original SdrPaintWindow is examined before the patched one.
    */
    const SdrPageWindow* FindPatchedPageWindow( const OutputDevice& rOutDev ) const;

    // Wird von der PaintView gerufen, wenn Modelaenderungen abgeschlossen sind
    void LazyReactOnObjectChanges();

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
    void SetDesignMode( bool _bDesignMode );

    bool IsVisible() const { return mbVisible; }

    // Invalidiert den gesamten Bereich der Page
    void InvalidateAllWin();

    // rRect bezieht sich auf die Page
    void InvalidateAllWin(const basegfx::B2DRange& rRange, bool bPlus1Pix = false);

    // PrePaint call forwarded from app windows
    void PrePaint();

    // PostPaint call forwarded from app windows
    void PostPaint();

    // rReg bezieht sich auf's OutDev, nicht auf die Page
    void CompleteRedraw(SdrPaintWindow& rPaintWindow, const Region& rReg, sdr::contact::ViewObjectContactRedirector* pRedirector = 0) const;

    // write access to mpPreparedPageWindow
    void setPreparedPageWindow(SdrPageWindow* pKnownTarget);

    void DrawLayer(SdrLayerID nID, OutputDevice* pGivenTarget = 0, sdr::contact::ViewObjectContactRedirector* pRedirector = 0) const;
    void DrawPageViewGrid(OutputDevice& rOut, const basegfx::B2DRange& rRect, Color aColor = Color( COL_BLACK ) );

    basegfx::B2DRange GetPageRange() const;
    SdrPage& getSdrPageFromSdrPageView() const { return mrSdrPageOfSdrPageView; }

    // get/set entered List/Group
    SdrObjList* GetCurrentObjectList() const { return mpCurrentList; }
    SdrObject* GetCurrentGroup() const { return mpCurrentGroup; }
    void SetCurrentGroupAndObjectList(SdrObject* pNewGroup, SdrObjList* pNewList);

    void SetLayerVisible(const String& rName, bool bShow = true) { SetLayer(rName, maVisibleLayerSet, bShow); if(!bShow) AdjHdl(); InvalidateAllWin(); }
    bool IsLayerVisible(const String& rName) const { return IsLayer(rName, maVisibleLayerSet); }
    void SetAllLayersVisible(bool bShow = true) { SetAllLayers(maVisibleLayerSet, bShow); if(!bShow) AdjHdl(); InvalidateAllWin(); }

    void SetLayerLocked(const String& rName, bool bLock = true) { SetLayer(rName, maLockedLayerSet, bLock); if(bLock) AdjHdl(); }
    bool IsLayerLocked(const String& rName) const { return IsLayer(rName,maLockedLayerSet); }
    void SetAllLayersLocked(bool bLock = true) { SetAllLayers(maLockedLayerSet, bLock); if(bLock) AdjHdl(); }

    void SetLayerPrintable(const String& rName, bool bPrn = true) { SetLayer(rName, maPrintableLayerSet, bPrn); }
    bool IsLayerPrintable(const String& rName) const { return IsLayer(rName, maPrintableLayerSet); }
    void SetAllLayersPrintable(bool bPrn = true) { SetAllLayers(maPrintableLayerSet, bPrn); }

    // PV stellt eine RefPage oder eine SubList eines RefObj dar oder Model ist ReadOnly
    bool IsReadOnly() const;

    // der Origin bezieht sich immer auf die obere linke Ecke der Page
    const basegfx::B2DPoint& GetPageOrigin() const { return maPageOrigin; }
    void SetPageOrigin(const basegfx::B2DPoint& rOrg);

    void SetVisibleLayers(const SetOfByte& rSet) { maVisibleLayerSet = rSet; InvalidateAllWin(); }
    const SetOfByte& GetVisibleLayers() const { return maVisibleLayerSet; }
    void SetPrintableLayers(const SetOfByte& rSet) { maPrintableLayerSet = rSet; }
    const SetOfByte& GetPrintableLayers() const { return maPrintableLayerSet;  }
    void SetLockedLayers(const SetOfByte& rSet) { maLockedLayerSet = rSet; }
    const SetOfByte& GetLockedLayers() const { return maLockedLayerSet; }

    const SdrHelpLineList& GetHelpLines() const { return maHelpLines; }
    void SetHelpLines(const SdrHelpLineList& rHLL);
    //void SetHelpLinePos(sal_uInt16 nNum, const Point& rNewPos);
    void SetHelpLine(sal_uInt32 nNum, const SdrHelpLine& rNewHelpLine);
    void DeleteHelpLine(sal_uInt32 nNum);
    void InsertHelpLine(const SdrHelpLine& rHL);

    // Liefert sal_True, wenn Layer des Obj sichtbar und nicht gesperrt.
    // Beim Gruppenobjekt muss wenigstens ein Member sichtbar sein,
    // gesperrt sein darf keiner.
    bool IsObjMarkable(const SdrObject& rObj) const;

    // Betreten (Editieren) einer Objektgruppe. Anschliessend liegen alle
    // Memberobjekte der Gruppe im direkten Zugriff. Alle anderen Objekte
    // koennen waerendessen nicht bearbeitet werden (bis zum naechsten
    // LeaveGroup()). (wie MsDos chdir bla).
    bool EnterGroup(SdrObject* pObj);

    // Verlassen einer betretenen Objektgruppe. (wie MsDos chdir ..)
    void LeaveOneGroup();

    // Verlassen aller betretenen Objektgruppen. (wie MsDos chdir \)
    void LeaveAllGroup();

    // Feststellen, wie weit hinabgestiegen wurde (0=Root(Page))
    sal_uInt32 GetEnteredLevel() const;

    // Name der aktuellen Objektgruppe
    String GetActualGroupName() const;

    // Die Namen aller z.Zt. betretenen Gruppen
    String GetActualPathName(sal_Unicode cSep = sal_Unicode('|')) const;

    // #103834# Set background color for svx at SdrPageViews
    void SetApplicationBackgroundColor(Color aBackgroundColor);

    // #109585#
    Color GetApplicationBackgroundColor() const;

    // #103911# Set/Get document color for svx at SdrPageViews
    void SetApplicationDocumentColor(Color aDocumentColor);
    Color GetApplicationDocumentColor() const;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //_SVDPAGV_HXX

////////////////////////////////////////////////////////////////////////////////////////////////////
// eof

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

#ifndef INCLUDED_SVX_SVDPAGV_HXX
#define INCLUDED_SVX_SVDPAGV_HXX

#include <com/sun/star/awt/XControlContainer.hpp>
#include <cppuhelper/implbase4.hxx>
#include <rtl/ustring.hxx>
#include <svl/lstner.hxx>
#include <svx/svdhlpln.hxx>
#include <svx/svdsob.hxx>
#include <svx/svdtypes.hxx>
#include <svx/svxdllapi.h>

#include <cppuhelper/implbase3.hxx>
#include <vector>
#include <basegfx/polygon/b2dpolypolygon.hxx>



namespace vcl { class Region; }
class SdrObjList;
class SdrObject;
class SdrPage;
class SdrUnoObj;
class SdrPaintWindow;
class SdrView;
class SdrPageObj;
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



class SVX_DLLPUBLIC SdrPageView
{
private:
    SdrView& mrView;
    SdrPage* mpPage;
    Point         aPgOrg;   // Nullpunkt der Page

    Rectangle     aMarkBound; // wird
    Rectangle     aMarkSnap;  // von
    bool          mbHasMarked;
    bool          mbVisible;

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

    SdrPageWindowVector maPageWindows;

    // #i72752# member to remember with which SdrPageWindow the BeginDrawLayer
    // was done
    SdrPageWindow* mpPreparedPageWindow;

    // interface to SdrPageWindow
private:
    void ClearPageWindows();
    void AppendPageWindow(SdrPageWindow& rNew);
    SdrPageWindow* RemovePageWindow(SdrPageWindow& rOld);
public:
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

private:
    SVX_DLLPRIVATE SdrPageWindow& CreateNewPageWindowEntry(SdrPaintWindow& rPaintWindow);

    void ImpInvalidateHelpLineArea(sal_uInt16 nNum) const;

    void SetLayer(const OUString& rName, SetOfByte& rBS, bool bJa);
    bool IsLayer(const OUString& rName, const SetOfByte& rBS) const;

    // Nachsehen, ob AktGroup noch Inserted ist.
    void CheckAktGroup();

    void AdjHdl();

public:
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

    bool IsVisible() const { return mbVisible; }

    // Invalidiert den gesamten Bereich der Page
    void InvalidateAllWin();

    // PrePaint call forwarded from app windows
    void PrePaint();

    // rReg bezieht sich auf's OutDev, nicht auf die Page
    void CompleteRedraw( SdrPaintWindow& rPaintWindow, const vcl::Region& rReg, sdr::contact::ViewObjectContactRedirector* pRedirector = NULL );

    // write access to mpPreparedPageWindow
    void setPreparedPageWindow(SdrPageWindow* pKnownTarget);

    void DrawLayer(SdrLayerID nID, OutputDevice* pGivenTarget = 0, sdr::contact::ViewObjectContactRedirector* pRedirector = 0L,
                   const Rectangle& rRect =  Rectangle());
    void DrawPageViewGrid(OutputDevice& rOut, const Rectangle& rRect, Color aColor = Color( COL_BLACK ) );

    Rectangle GetPageRect() const;
    SdrPage* GetPage() const { return mpPage; }

    // Betretene Liste rausreichen
    SdrObjList* GetObjList() const { return pAktList; }

    // Betretene Gruppe rausreichen
    SdrObject* GetAktGroup() const { return pAktGroup; }

    // Betretene Gruppe und Liste setzen
    void SetAktGroupAndList(SdrObject* pNewGroup, SdrObjList* pNewList);

    bool HasMarkedObjPageView() const { return mbHasMarked; }
    void SetHasMarkedObj(bool bOn) { mbHasMarked = bOn; }

    const Rectangle& MarkBound() const { return aMarkBound; }
    const Rectangle& MarkSnap() const { return aMarkSnap; }
    Rectangle& MarkBound() { return aMarkBound; }
    Rectangle& MarkSnap() { return aMarkSnap; }

    void SetLayerVisible(const OUString& rName, bool bShow = true) { SetLayer(rName, aLayerVisi, bShow); if(!bShow) AdjHdl(); InvalidateAllWin(); }
    bool IsLayerVisible(const OUString& rName) const { return IsLayer(rName, aLayerVisi); }

    void SetLayerLocked(const OUString& rName, bool bLock = true) { SetLayer(rName, aLayerLock, bLock); if(bLock) AdjHdl(); }
    bool IsLayerLocked(const OUString& rName) const { return IsLayer(rName,aLayerLock); }

    void SetLayerPrintable(const OUString& rName, bool bPrn = true) { SetLayer(rName, aLayerPrn, bPrn); }
    bool IsLayerPrintable(const OUString& rName) const { return IsLayer(rName, aLayerPrn); }

    // PV stellt eine RefPage oder eine SubList eines RefObj dar oder Model ist ReadOnly
    bool IsReadOnly() const;

    // der Origin bezieht sich immer auf die obere linke Ecke der Page
    const Point& GetPageOrigin() const { return aPgOrg; }
    void SetPageOrigin(const Point& rOrg);

    void LogicToPagePos(Point& rPnt) const { rPnt-=aPgOrg; }
    void LogicToPagePos(Rectangle& rRect) const { rRect.Move(-aPgOrg.X(),-aPgOrg.Y()); }
    void PagePosToLogic(Point& rPnt) const { rPnt+=aPgOrg; }
    void PagePosToLogic(Rectangle& rRect) const { rRect.Move(aPgOrg.X(),aPgOrg.Y()); }

    void SetVisibleLayers(const SetOfByte& rSet) { aLayerVisi=rSet; }
    const SetOfByte& GetVisibleLayers() const { return aLayerVisi; }
    void SetPrintableLayers(const SetOfByte& rSet) { aLayerPrn=rSet; }
    const SetOfByte& GetPrintableLayers() const { return aLayerPrn;  }
    void SetLockedLayers(const SetOfByte& rSet) { aLayerLock=rSet; }
    const SetOfByte& GetLockedLayers() const { return aLayerLock; }

    const SdrHelpLineList& GetHelpLines() const { return aHelpLines; }
    void SetHelpLines(const SdrHelpLineList& rHLL);
    //void SetHelpLinePos(sal_uInt16 nNum, const Point& rNewPos);
    void SetHelpLine(sal_uInt16 nNum, const SdrHelpLine& rNewHelpLine);
    void DeleteHelpLine(sal_uInt16 nNum);
    void InsertHelpLine(const SdrHelpLine& rHL, sal_uInt16 nNum=0xFFFF);
    void MoveHelpLine(sal_uInt16 nNum, sal_uInt16 nNewNum) { aHelpLines.Move(nNum,nNewNum); }

    // Liefert sal_True, wenn Layer des Obj sichtbar und nicht gesperrt.
    // Beim Gruppenobjekt muss wenigstens ein Member sichtbar sein,
    // gesperrt sein darf keiner.
    bool IsObjMarkable(SdrObject* pObj) const;
    // hmm, selectable is surely the same as markable, now that I
    // see this as I look for a place to put it. TO-DO,
    // merge these
    bool IsObjSelectable(SdrObject *pObj) const;


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
    sal_uInt16 GetEnteredLevel() const;

    // #103834# Set background color for svx at SdrPageViews
    void SetApplicationBackgroundColor(Color aBackgroundColor);

    // #109585#
    Color GetApplicationBackgroundColor() const { return maBackgroundColor;}

    // #103911# Set/Get document color for svx at SdrPageViews
    void SetApplicationDocumentColor(Color aDocumentColor);
    Color GetApplicationDocumentColor() const { return maDocumentColor;}
};



#endif // INCLUDED_SVX_SVDPAGV_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

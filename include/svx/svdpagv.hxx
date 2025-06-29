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

#pragma once

#include <com/sun/star/awt/XControlContainer.hpp>
#include <rtl/ref.hxx>
#include <rtl/ustring.hxx>
#include <tools/color.hxx>
#include <svx/svdhlpln.hxx>
#include <svx/svdsob.hxx>
#include <svx/svdtypes.hxx>
#include <svx/svxdllapi.h>

#include <memory>
#include <vector>
#include <basegfx/range/b2irectangle.hxx>


namespace vcl { class Region; }
class SdrObjList;
class SdrObject;
class SdrPage;
class SdrPaintWindow;
class SdrView;
class UnoControlContainer;

namespace sdr::contact
{
    class ViewObjectContactRedirector;
}

// typedefs for a list of SdrPageWindow
class SdrPageWindow;

class SVXCORE_DLLPUBLIC SdrPageView
{
private:
    SdrView&     mrView;
    SdrPage*     mpPage;
    Point        maPageOrigin;   // The Page's point of origin

    tools::Rectangle    m_aMarkBound;
    tools::Rectangle    m_aMarkSnap;
    bool         mbHasMarked;
    bool         mbVisible;

    SdrLayerIDSet    m_aLayerVisi;   // Set of visible Layers
    SdrLayerIDSet    m_aLayerLock;   // Set of non-editable Layers
    SdrLayerIDSet    m_aLayerPrn;    // Set of printable Layers

    SdrObjList*  m_pCurrentList;     // Current List, usually the Page
    SdrObject*   m_pCurrentGroup;    // Current Group; nullptr means none

    SdrHelpLineList m_aHelpLines; // Helper lines and points

    // #103911# Use one reserved slot (bReserveBool2) for the document color
    Color         maDocumentColor;

    // #103834# Use one reserved slot (bReserveBool1) for the background color
    Color         maBackgroundColor;

    std::vector< std::unique_ptr<SdrPageWindow> > maPageWindows;

    // #i72752# member to remember with which SdrPageWindow the BeginDrawLayer
    // was done
    SdrPageWindow* mpPreparedPageWindow;

public:
    sal_uInt32 PageWindowCount() const { return maPageWindows.size(); }
    SdrPageWindow* FindPageWindow( const SdrPaintWindow& rPaintWindow ) const;
    SdrPageWindow* FindPageWindow( const OutputDevice& rOutDev ) const;
    SdrPageWindow* GetPageWindow(sal_uInt32 nIndex) const;

    /**
     * Finds the page window whose PaintWindow belongs to the given output device
     * In opposite to FindPageWindow, this method also cares possibly patched PaintWindow instances.
     * That is, a SdrPageWindow might have an original, and a patched SdrPaintWindow instance - if
     * this is the case, then the original SdrPaintWindow is examined before the patched one.
     */
    const SdrPageWindow* FindPatchedPageWindow( const OutputDevice& rOutDev ) const;

private:
    void ImpInvalidateHelpLineArea(sal_uInt16 nNum) const;

    // return true if changed, false if unchanged
    bool SetLayer(const OUString& rName, SdrLayerIDSet& rBS, bool bJa);
    bool IsLayer(const OUString& rName, const SdrLayerIDSet& rBS) const;

    /// Let's see if the current Group (pCurrentGroup) is still inserted
    void CheckCurrentGroup();

    void AdjHdl();

public:
    SdrPageView(SdrPage* pPage1, SdrView& rNewView);
    ~SdrPageView();

    SdrPageView& operator=( SdrPageView const & ) = delete; // MSVC2017 workaround
    SdrPageView( SdrPageView const & ) = delete; // MSVC2017 workaround


    /// Is called by PaintView, when modal changes have finished
    void ModelHasChanged();

    void Show();
    void Hide();

    void AddPaintWindowToPageView(SdrPaintWindow& rPaintWindow);
    void RemovePaintWindowFromPageView(SdrPaintWindow& rPaintWindow);

    SdrView& GetView() { return mrView; }
    const SdrView& GetView() const { return mrView; }

    /**
     * Looks up the control container belonging to given output device
     * @return
     *      If the given output device belongs to one of the SdrPageViewWinRecs associated with this
     *      SdrPageView instance, the XControlContainer for this output device is returned, <NULL/>
     *      otherwise.
     */
    rtl::Reference< UnoControlContainer >
        GetControlContainer( const OutputDevice& _rDevice ) const;

    /// Sets all elements in the view which support a design and an alive mode into the given mode
    void    SetDesignMode( bool _bDesignMode ) const;

    bool IsVisible() const { return mbVisible; }

    /// Invalidates the Page's whole area
    void InvalidateAllWin();

    /// PrePaint call forwarded from app windows
    void PrePaint();

    /// @param rReg refers to the OutDev and not to the Page
    void CompleteRedraw( SdrPaintWindow& rPaintWindow, const vcl::Region& rReg, sdr::contact::ViewObjectContactRedirector* pRedirector );

    /// Write access to mpPreparedPageWindow
    void setPreparedPageWindow(SdrPageWindow* pKnownTarget);

    void DrawLayer(SdrLayerID nID, OutputDevice* pGivenTarget, sdr::contact::ViewObjectContactRedirector* pRedirector = nullptr,
                   const tools::Rectangle& rRect = tools::Rectangle(),
                   basegfx::B2IRectangle const* pPageFrame = nullptr);
    void DrawPageViewGrid(OutputDevice& rOut, const tools::Rectangle& rRect, Color aColor = COL_BLACK );

    tools::Rectangle GetPageRect() const;
    SdrPage* GetPage() const { return mpPage; }

    /// Return current List
    SdrObjList* GetObjList() const { return m_pCurrentList; }

    /// Return current Group
    SdrObject* GetCurrentGroup() const { return m_pCurrentGroup; }

    /// Set current Group and List
    void SetCurrentGroupAndList(SdrObject* pNewGroup, SdrObjList* pNewList);

    bool HasMarkedObjPageView() const { return mbHasMarked; }
    void SetHasMarkedObj(bool bOn) { mbHasMarked = bOn; }

    const tools::Rectangle& MarkBound() const { return m_aMarkBound; }
    const tools::Rectangle& MarkSnap() const { return m_aMarkSnap; }
    tools::Rectangle& MarkBound() { return m_aMarkBound; }
    tools::Rectangle& MarkSnap() { return m_aMarkSnap; }

    bool SetLayerVisible(const OUString& rName, bool bShow) {
        const bool bChanged = SetLayer(rName, m_aLayerVisi, bShow);
        if (!bChanged)
            return false;
        if(!bShow) AdjHdl();
        InvalidateAllWin();
        return true;
    }
    bool IsLayerVisible(const OUString& rName) const { return IsLayer(rName, m_aLayerVisi); }

    void SetLayerLocked(const OUString& rName, bool bLock) { SetLayer(rName, m_aLayerLock, bLock); if(bLock) AdjHdl(); }
    bool IsLayerLocked(const OUString& rName) const { return IsLayer(rName,m_aLayerLock); }

    void SetLayerPrintable(const OUString& rName, bool bPrn) { SetLayer(rName, m_aLayerPrn, bPrn); }
    bool IsLayerPrintable(const OUString& rName) const { return IsLayer(rName, m_aLayerPrn); }

    /// PV represents a RefPage or a SubList of a RefObj, or the Model is ReadOnly
    bool IsReadOnly() const;

    /// The Origin always refers to the upper left corner of the Page
    const Point& GetPageOrigin() const { return maPageOrigin; }
    void SetPageOrigin(const Point& rOrg);

    void LogicToPagePos(Point& rPnt) const { rPnt-=maPageOrigin; }
    void LogicToPagePos(tools::Rectangle& rRect) const { rRect.Move(-maPageOrigin.X(),-maPageOrigin.Y()); }
    void PagePosToLogic(Point& rPnt) const { rPnt+=maPageOrigin; }

    void SetVisibleLayers(const SdrLayerIDSet& rSet) { m_aLayerVisi=rSet; }
    const SdrLayerIDSet& GetVisibleLayers() const { return m_aLayerVisi; }
    void SetPrintableLayers(const SdrLayerIDSet& rSet) { m_aLayerPrn=rSet; }
    const SdrLayerIDSet& GetPrintableLayers() const { return m_aLayerPrn;  }
    void SetLockedLayers(const SdrLayerIDSet& rSet) { m_aLayerLock=rSet; }
    const SdrLayerIDSet& GetLockedLayers() const { return m_aLayerLock; }

    const SdrHelpLineList& GetHelpLines() const { return m_aHelpLines; }
    void SetHelpLines(const SdrHelpLineList& rHLL);
    //void SetHelpLinePos(sal_uInt16 nNum, const Point& rNewPos);
    void SetHelpLine(sal_uInt16 nNum, const SdrHelpLine& rNewHelpLine);
    void DeleteHelpLine(sal_uInt16 nNum);
    void InsertHelpLine(const SdrHelpLine& rHL);

    /// At least one member must be visible for the Group object and
    /// it must not be locked
    /// @returns
    ///      true, if the object's layer is visible and not locked
    bool IsObjMarkable(SdrObject const * pObj) const;

    /// Entering (editing) an object group
    /// After that, we have direct access to all member objects of the group.
    /// All other objects are not editable in the meantime (until the next
    /// LeaveGroup())
    bool EnterGroup(SdrObject* pObj);

    /// Leave an object group we entered previously
    void LeaveOneGroup();

    /// Leave all object groups we entered previously
    void LeaveAllGroup();

    /// Determine, how deep we descended (0 = Root(Page))
    sal_uInt16 GetEnteredLevel() const;

    // #103834# Set background color for svx at SdrPageViews
    void SetApplicationBackgroundColor(Color aBackgroundColor);

    const Color& GetApplicationBackgroundColor() const { return maBackgroundColor;}

    // #103911# Set/Get document color for svx at SdrPageViews
    void SetApplicationDocumentColor(Color aDocumentColor);
    const Color& GetApplicationDocumentColor() const { return maDocumentColor;}

    void resetGridOffsetsOfAllPageWindows() const;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

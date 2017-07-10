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
#ifndef INCLUDED_SW_SOURCE_CORE_INC_DVIEW_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_DVIEW_HXX

#include <svx/fmview.hxx>

class OutputDevice;
class SwViewShellImp;
class SwFrame;
class SwFlyFrame;
class SwAnchoredObject;
class SdrUndoManager;

class SwDrawView : public FmFormView
{
    Point           m_aAnchorPoint;       // anchor position
    SwViewShellImp      &m_rImp;               // a view is always part of a shell

    const SwFrame *CalcAnchor();

    /** determine maximal order number for a 'child' object of given 'parent' object

        The maximal order number will be determined on the current object
        order hierarchy. It's the order number of the 'child' object with the
        highest order number. The calculation can be influenced by parameter
        <_pExclChildObj> - this 'child' object won't be considered.

        @param <_rParentObj>
        input parameter - 'parent' object, for which the maximal order number
        for its 'children' will be determined.

        @param <_pExclChildObj>
        optional input parameter - 'child' object, which will not be considered
        on the calculation of the maximal order number
    */
    static sal_uInt32 GetMaxChildOrdNum( const SwFlyFrame& _rParentObj,
                                   const SdrObject* _pExclChildObj = nullptr );

    /** method to move 'repeated' objects of the given moved object to the
        according level

        @param <_rMovedAnchoredObj>
        input parameter - moved object, for which the 'repeated' ones have also
        to be moved.

        @param <_rMovedChildrenObjs>
        input parameter - data collection of moved 'child' objects - the 'repeated'
        ones of these 'children' will also been moved.
    */
    void MoveRepeatedObjs( const SwAnchoredObject& _rMovedAnchoredObj,
                            const std::vector<SdrObject*>& _rMovedChildObjs ) const;

protected:
    // add custom handles (used by other apps, e.g. AnchorPos)
    virtual void AddCustomHdl() override;

    // override to allow extra handling when picking SwVirtFlyDrawObj's
    using FmFormView::CheckSingleSdrObjectHit;
    virtual SdrObject* CheckSingleSdrObjectHit(const Point& rPnt, sal_uInt16 nTol, SdrObject* pObj, SdrPageView* pPV, SdrSearchOptions nOptions, const SdrLayerIDSet* pMVisLay) const override;

    // support enhanced text edit for draw objects
    virtual SdrUndoManager* getSdrUndoManagerForEnhancedTextEdit() const override;

public:
    SwDrawView( SwViewShellImp &rI, SdrModel *pMd, OutputDevice* pOutDev );

    // from base class
    virtual SdrObject*   GetMaxToTopObj(SdrObject* pObj) const override;
    virtual SdrObject*   GetMaxToBtmObj(SdrObject* pObj) const override;
    virtual void         MarkListHasChanged() override;

    // #i7672#
    // Override to reuse edit background color in active text edit view (OutlinerView)
    virtual void ModelHasChanged() override;

    virtual void         ObjOrderChanged( SdrObject* pObj, sal_uLong nOldPos,
                                            sal_uLong nNewPos ) override;
    virtual bool TakeDragLimit(SdrDragMode eMode, tools::Rectangle& rRect) const override;
    virtual void MakeVisible( const tools::Rectangle&, vcl::Window &rWin ) override;
    virtual void CheckPossibilities() override;

    const SwViewShellImp &Imp() const { return m_rImp; }
          SwViewShellImp &Imp()       { return m_rImp; }

    // anchor and Xor for dragging
    void ShowDragAnchor();

    virtual void DeleteMarked() override;

    void ValidateMarkList() { FlushComeBackTimer(); }

    // #i99665#
    bool IsAntiAliasing() const;

    // method to replace marked/selected <SwDrawVirtObj>
    // by its reference object for delete of selection and group selection
    static void ReplaceMarkedDrawVirtObjs( SdrMarkView& _rMarkView );

    /// See SdrMarkView::GetSfxViewShell().
    SfxViewShell* GetSfxViewShell() const override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

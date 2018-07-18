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
#ifndef INCLUDED_SW_SOURCE_CORE_INC_VIEWIMP_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_VIEWIMP_HXX

#include <rtl/ustring.hxx>
#include <tools/color.hxx>
#include <svx/svdtypes.hxx>
#include <vcl/timer.hxx>
#include <swrect.hxx>
#include <swtypes.hxx>
#include <vector>
#include <memory>

class SwViewShell;
class SwFlyFrame;
class SwViewOption;
class SwRegionRects;
class SwFrame;
class SwLayAction;
class SwLayIdle;
class SwDrawView;
class SdrPageView;
class SwPageFrame;
class SwRegionRects;
class SwAccessibleMap;
class SdrObject;
class Fraction;
class SwPrintData;
class SwPagePreviewLayout;
struct PreviewPage;
class SwTextFrame;
// --> OD #i76669#
namespace sdr { namespace contact {
        class ViewObjectContactRedirector;
} }
// <--

class SwViewShellImp
{
    friend class SwViewShell;

    friend class SwLayAction;   // Lay- and IdleAction register and deregister
    friend class SwLayIdle;

    // for paint of page preview
    friend class SwPagePreviewLayout;

    SwViewShell *m_pShell;           // If someone passes an Imp, but needs a SwViewShell, we
                                // keep a backlink here

    std::unique_ptr<SwDrawView> m_pDrawView; // Our DrawView
    SdrPageView *m_pSdrPageView;  // Exactly one Page for our DrawView

    SwPageFrame     *m_pFirstVisiblePage; // Always points to the first visible Page
    std::unique_ptr<SwRegionRects> m_pRegion; // Collector of Paintrects from the LayAction

    SwLayAction   *m_pLayAction;      // Is set if an Action object exists
                                 // Is registered by the SwLayAction ctor and deregistered by the dtor
    SwLayIdle     *m_pIdleAct;     // The same as SwLayAction for SwLayIdle

    /// note: the map is *uniquely* owned here - the shared_ptr is only
    /// used so that SwAccessibleContext can check via weak_ptr that it's alive
    std::shared_ptr<SwAccessibleMap> m_pAccessibleMap;

    bool m_bFirstPageInvalid : 1; // Pointer to the first Page invalid?
    bool m_bResetHdlHiddenPaint : 1; // Ditto
    bool m_bSmoothUpdate : 1; // For SmoothScroll
    bool m_bStopSmooth : 1;

    sal_uInt16 m_nRestoreActions  ; // Count for the Action that need to be restored (UNO)
    SwRect m_aSmoothRect;

    std::unique_ptr<SwPagePreviewLayout> m_pPagePreviewLayout;

    void SetFirstVisPage(OutputDevice const * pRenderContext); // Recalculate the first visible Page

    void StartAction();         // Show handle and hide
    void EndAction();           // Called by SwViewShell::ImplXXXAction
    void LockPaint();           // Ditto; called by SwViewShell::ImplLockPaint
    void UnlockPaint();

private:

    SwAccessibleMap *CreateAccessibleMap();

    /** invalidate CONTENT_FLOWS_FROM/_TO relation for paragraphs

        #i27138#
        implementation for wrapper method
        <SwViewShell::InvalidateAccessibleParaFlowRelation(..)>

        @param _pFromTextFrame
        input parameter - paragraph frame, for which the relation CONTENT_FLOWS_FROM
        has to be invalidated.
        If NULL, no CONTENT_FLOWS_FROM relation has to be invalidated

        @param _pToTextFrame
        input parameter - paragraph frame, for which the relation CONTENT_FLOWS_TO
        has to be invalidated.
        If NULL, no CONTENT_FLOWS_TO relation has to be invalidated
    */
    void InvalidateAccessibleParaFlowRelation_( const SwTextFrame* _pFromTextFrame,
                                                const SwTextFrame* _pToTextFrame );

    /** invalidate text selection for paragraphs

        #i27301#
        implementation for wrapper method
        <SwViewShell::InvalidateAccessibleParaTextSelection(..)>
    */
    void InvalidateAccessibleParaTextSelection_();

    /** invalidate attributes for paragraphs and paragraph's characters

        #i88069#
        implementation for wrapper method
        <SwViewShell::InvalidateAccessibleParaAttrs(..)>
    */
    void InvalidateAccessibleParaAttrs_( const SwTextFrame& rTextFrame );

public:
    SwViewShellImp( SwViewShell * );
    ~SwViewShellImp();
    void Init( const SwViewOption * ); /// Only for SwViewShell::Init()

    const SwViewShell *GetShell() const { return m_pShell; }
          SwViewShell *GetShell()       { return m_pShell; }

    Color GetRetoucheColor() const;

    /// Management of the first visible Page
    const SwPageFrame *GetFirstVisPage(OutputDevice const * pRenderContext) const;
          SwPageFrame *GetFirstVisPage(OutputDevice const * pRenderContext);
    void SetFirstVisPageInvalid() { m_bFirstPageInvalid = true; }

    bool AddPaintRect( const SwRect &rRect );
    SwRegionRects *GetRegion()      { return m_pRegion.get(); }
    void DelRegion();

    /// New Interface for StarView Drawing
    bool  HasDrawView()             const { return nullptr != m_pDrawView; }
          SwDrawView* GetDrawView()       { return m_pDrawView.get(); }
    const SwDrawView* GetDrawView() const { return m_pDrawView.get(); }
          SdrPageView*GetPageView()       { return m_pSdrPageView; }
    const SdrPageView*GetPageView() const { return m_pSdrPageView; }
    void MakeDrawView();

    /**
     * @param _pPageBackgrdColor for setting this color as the background color
     * at the outliner of the draw view for painting layers "hell" and "heaven"
     *
     * @param _bIsPageRightToLeft for the horizontal text direction of the page
     * in order to set the default horizontal text direction at the outliner of
     * the draw view for painting layers "hell" and "heaven"
     */
    void   PaintLayer( const SdrLayerID _nLayerID,
                       SwPrintData const*const pPrintData,
                       SwPageFrame const& rPageFrame,
                       const SwRect& _rRect,
                       const Color* _pPageBackgrdColor,
                       const bool _bIsPageRightToLeft,
                       sdr::contact::ViewObjectContactRedirector* pRedirector );

    /**
     * Is passed to the DrawEngine as a Link and decides what is painted
     * or not and in what way
     */

    // Interface Drawing
    bool IsDragPossible( const Point &rPoint );
    void NotifySizeChg( const Size &rNewSz );

    /// SS for the Lay-/IdleAction and relatives
    bool  IsAction() const                   { return m_pLayAction  != nullptr; }
    bool  IsIdleAction() const               { return m_pIdleAct != nullptr; }
          SwLayAction &GetLayAction()        { return *m_pLayAction; }
    const SwLayAction &GetLayAction() const  { return *m_pLayAction; }

    /**
     * If an Action is running we ask it to check whether it's time
     * to enable the WaitCursor
     */
    void CheckWaitCursor();

    /// Asks the LayAction if present
    bool IsCalcLayoutProgress() const;

    /**
     * @returns true if a LayAction is running
     *
     * There we also set the Flag for ExpressionFields
     */
    bool IsUpdateExpFields();

    void    SetRestoreActions(sal_uInt16 nSet){m_nRestoreActions = nSet;}
    sal_uInt16  GetRestoreActions() const{return m_nRestoreActions;}

    void InitPagePreviewLayout();

    SwPagePreviewLayout* PagePreviewLayout()
    {
        return m_pPagePreviewLayout.get();
    }

    /// Is this view accessible?
    bool IsAccessible() const { return m_pAccessibleMap != nullptr; }

    inline SwAccessibleMap& GetAccessibleMap();

    /// Update (this) accessible view
    void UpdateAccessible();

    /// Remove a frame from the accessible view
    void DisposeAccessible( const SwFrame *pFrame, const SdrObject *pObj,
                            bool bRecursive, bool bCanSkipInvisible );
    inline void DisposeAccessibleFrame( const SwFrame *pFrame,
                               bool bRecursive = false );
    inline void DisposeAccessibleObj( const SdrObject *pObj, bool bCanSkipInvisible );

    /// Move a frame's position in the accessible view
    void MoveAccessible( const SwFrame *pFrame, const SdrObject *pObj,
                         const SwRect& rOldFrame );
    inline void MoveAccessibleFrame( const SwFrame *pFrame, const SwRect& rOldFrame );

    /// Add a frame in the accessible view
    inline void AddAccessibleFrame( const SwFrame *pFrame );

    inline void AddAccessibleObj( const SdrObject *pObj );

    /// Invalidate accessible frame's frame's content
    void InvalidateAccessibleFrameContent( const SwFrame *pFrame );

    /// Invalidate accessible frame's cursor position
    void InvalidateAccessibleCursorPosition( const SwFrame *pFrame );

    /// Invalidate editable state for all accessible frames
    void InvalidateAccessibleEditableState( bool bAllShells,
                                               const SwFrame *pFrame=nullptr );

    /// Invalidate frame's relation set (for chained frames)
    void InvalidateAccessibleRelationSet( const SwFlyFrame *pMaster,
                                          const SwFlyFrame *pFollow );

    /// update data for accessible preview
    /// change method signature due to new page preview functionality
    void UpdateAccessiblePreview( const std::vector<PreviewPage*>& _rPreviewPages,
                                  const Fraction&  _rScale,
                                  const SwPageFrame* _pSelectedPageFrame,
                                  const Size&      _rPreviewWinSize );

    void InvalidateAccessiblePreviewSelection( sal_uInt16 nSelPage );

    /// Fire all accessible events that have been collected so far
    void FireAccessibleEvents();
};

inline SwAccessibleMap& SwViewShellImp::GetAccessibleMap()
{
    if( !m_pAccessibleMap )
        CreateAccessibleMap();

    return *m_pAccessibleMap;
}

inline void SwViewShellImp::DisposeAccessibleFrame( const SwFrame *pFrame,
                               bool bRecursive )
{
    DisposeAccessible( pFrame, nullptr, bRecursive, true );
}

inline void SwViewShellImp::DisposeAccessibleObj( const SdrObject *pObj, bool bCanSkipInvisible )
{
    DisposeAccessible( nullptr, pObj, false, bCanSkipInvisible );
}

inline void SwViewShellImp::MoveAccessibleFrame( const SwFrame *pFrame,
                                          const SwRect& rOldFrame )
{
    MoveAccessible( pFrame, nullptr, rOldFrame );
}

inline void SwViewShellImp::AddAccessibleFrame( const SwFrame *pFrame )
{
    SwRect aEmptyRect;
    MoveAccessible( pFrame, nullptr, aEmptyRect );
}

inline void SwViewShellImp::AddAccessibleObj( const SdrObject *pObj )
{
    SwRect aEmptyRect;
    MoveAccessible( nullptr, pObj, aEmptyRect );
}
#endif // INCLUDED_SW_SOURCE_CORE_INC_VIEWIMP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

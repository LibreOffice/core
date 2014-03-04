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

class SwViewShell;
class SwFlyFrm;
class SwViewOption;
class SwRegionRects;
class SwFrm;
class SwLayAction;
class SwLayIdle;
class SwDrawView;
class SdrPageView;
class SwPageFrm;
class SwRegionRects;
class SwAccessibleMap;
class SdrObject;
class Fraction;
class SwPrintData;
class SwPagePreviewLayout;
struct PreviewPage;
class SwTxtFrm;
// --> OD #i76669#
namespace sdr { namespace contact {
        class ViewObjectContactRedirector;
} }
// <--

class SwViewImp
{
    friend class SwViewShell;

    friend class SwLayAction;   //Lay- und IdleAction tragen sich ein und aus.
    friend class SwLayIdle;

    // for paint of page preview
    friend class SwPagePreviewLayout;

    SwViewShell *pSh;             //Falls jemand einen Imp durchreicht und doch
                                //mal eine SwViewShell braucht hier die
                                //Rueckwaertsverkettung.

    SwDrawView  *pDrawView;     //Unsere DrawView
    SdrPageView *pSdrPageView;  //Genau eine Seite fuer unsere DrawView

    SwPageFrm     *pFirstVisPage;//Zeigt immer auf die erste sichtbare Seite.
    SwRegionRects *pRegion;      //Sammler fuer Paintrects aus der LayAction.

    SwLayAction   *pLayAct;      //Ist gesetzt wenn ein Action-Objekt existiert
                                 //Wird vom SwLayAction-CTor ein- und vom DTor
                                 //ausgetragen.
    SwLayIdle     *pIdleAct;     //Analog zur SwLayAction fuer SwLayIdle.

    SwAccessibleMap *pAccMap;       // Accessible Wrappers

    mutable const SdrObject * pSdrObjCached;
    mutable OUString sSdrObjCachedComment;

    bool bFirstPageInvalid : 1; //Pointer auf erste Seite ungueltig?
    bool bResetHdlHiddenPaint : 1;//  -- "" --
    bool bSmoothUpdate : 1; //Meber fuer SmoothScroll
    bool bStopSmooth : 1;

    sal_uInt16 nRestoreActions  ; //Die Anzahl der zu restaurierenden Actions (UNO)
    SwRect aSmoothRect;

    SwPagePreviewLayout* mpPgPreviewLayout;

    void SetFirstVisPage();     //Neue Ermittlung der ersten sichtbaren Seite

    void StartAction();         //Henkel Anzeigen und verstecken.
    void EndAction();           //gerufen von SwViewShell::ImplXXXAction
    void LockPaint();           //dito, gerufen von SwViewShell::ImplLockPaint
    void UnlockPaint();

private:

    SwAccessibleMap *CreateAccessibleMap();

    /** invalidate CONTENT_FLOWS_FROM/_TO relation for paragraphs

        #i27138#
        implementation for wrapper method
        <SwViewShell::InvalidateAccessibleParaFlowRelation(..)>

        @param _pFromTxtFrm
        input parameter - paragraph frame, for which the relation CONTENT_FLOWS_FROM
        has to be invalidated.
        If NULL, no CONTENT_FLOWS_FROM relation has to be invalidated

        @param _pToTxtFrm
        input parameter - paragraph frame, for which the relation CONTENT_FLOWS_TO
        has to be invalidated.
        If NULL, no CONTENT_FLOWS_TO relation has to be invalidated
    */
    void _InvalidateAccessibleParaFlowRelation( const SwTxtFrm* _pFromTxtFrm,
                                                const SwTxtFrm* _pToTxtFrm );

    /** invalidate text selection for paragraphs

        #i27301#
        implementation for wrapper method
        <SwViewShell::InvalidateAccessibleParaTextSelection(..)>
    */
    void _InvalidateAccessibleParaTextSelection();

    /** invalidate attributes for paragraphs and paragraph's characters

        #i88069#
        implementation for wrapper method
        <SwViewShell::InvalidateAccessibleParaAttrs(..)>
    */
    void _InvalidateAccessibleParaAttrs( const SwTxtFrm& rTxtFrm );

public:
    SwViewImp( SwViewShell * );
    ~SwViewImp();
    void Init( const SwViewOption * );          //nur fuer SwViewShell::Init()

    const SwViewShell *GetShell() const { return pSh; }
          SwViewShell *GetShell()       { return pSh; }

    Color GetRetoucheColor() const;

    //Verwaltung zur ersten sichtbaren Seite
    inline const SwPageFrm *GetFirstVisPage() const;
    inline       SwPageFrm *GetFirstVisPage();
    void SetFirstVisPageInvalid() { bFirstPageInvalid = true; }

    bool AddPaintRect( const SwRect &rRect );
    SwRegionRects *GetRegion()      { return pRegion; }
    void DelRegion();

    // neues Interface fuer StarView Drawing
    bool  HasDrawView()             const { return 0 != pDrawView; }
          SwDrawView* GetDrawView()       { return pDrawView; }
    const SwDrawView* GetDrawView() const { return pDrawView; }
          SdrPageView*GetPageView()       { return pSdrPageView; }
    const SdrPageView*GetPageView() const { return pSdrPageView; }
    void MakeDrawView();

    // add 3rd parameter <const Color* pPageBackgrdColor> for setting this
    // color as the background color at the outliner of the draw view
    // for painting layers <hell> and <heaven>

    // add 4th parameter for the horizontal text
    // direction of the page in order to set the default horizontal text
    // direction at the outliner of the draw view for painting layers <hell>
    // and <heaven>.

    // #i76669# - added parameter <pRedirector>
    void   PaintLayer( const SdrLayerID _nLayerID,
                       SwPrintData const*const pPrintData,
                       const SwRect& _rRect,
                       const Color* _pPageBackgrdColor = 0,
                       const bool _bIsPageRightToLeft = false,
                       sdr::contact::ViewObjectContactRedirector* pRedirector = 0 ) const;

    //wird als Link an die DrawEngine uebergeben, entscheidet was wie
    //gepaintet wird oder nicht.

    // Interface Drawing
    bool IsDragPossible( const Point &rPoint );
    void NotifySizeChg( const Size &rNewSz );

    //SS Fuer die Lay- bzw. IdleAction und verwandtes
    bool  IsAction() const                   { return pLayAct  != 0; }
    bool  IsIdleAction() const               { return pIdleAct != 0; }
          SwLayAction &GetLayAction()        { return *pLayAct; }
    const SwLayAction &GetLayAction() const  { return *pLayAct; }
          SwLayIdle   &GetIdleAction()       { return *pIdleAct;}
    const SwLayIdle   &GetIdleAction() const { return *pIdleAct;}

    //Wenn eine Aktion laueft wird diese gebeten zu pruefen ob es
    //an der zeit ist den WaitCrsr einzuschalten.
    void CheckWaitCrsr();
    bool IsCalcLayoutProgress() const;  //Fragt die LayAction wenn vorhanden.
    //true wenn eine LayAction laeuft, dort wird dann auch das Flag fuer
    //ExpressionFields gesetzt.
    bool IsUpdateExpFlds();

    void    SetRestoreActions(sal_uInt16 nSet){nRestoreActions = nSet;}
    sal_uInt16  GetRestoreActions() const{return nRestoreActions;}

    void InitPagePreviewLayout();

    inline SwPagePreviewLayout* PagePreviewLayout()
    {
        return mpPgPreviewLayout;
    }

    // Is this view accessible?
    bool IsAccessible() const { return pAccMap != 0; }

    inline SwAccessibleMap& GetAccessibleMap();

    // Update (this) accessible view
    void UpdateAccessible();

    // Remove a frame from the accessible view
    void DisposeAccessible( const SwFrm *pFrm, const SdrObject *pObj,
                            bool bRecursive );
    inline void DisposeAccessibleFrm( const SwFrm *pFrm,
                               bool bRecursive = false );
    inline void DisposeAccessibleObj( const SdrObject *pObj );

    // Move a frame's position in the accessible view
    void MoveAccessible( const SwFrm *pFrm, const SdrObject *pObj,
                         const SwRect& rOldFrm );
    inline void MoveAccessibleFrm( const SwFrm *pFrm, const SwRect& rOldFrm );

    // Add a frame in the accessible view
    inline void AddAccessibleFrm( const SwFrm *pFrm );

    inline void AddAccessibleObj( const SdrObject *pObj );

    // Invalidate accessible frame's frame's content
    void InvalidateAccessibleFrmContent( const SwFrm *pFrm );

    // Invalidate accessible frame's cursor position
    void InvalidateAccessibleCursorPosition( const SwFrm *pFrm );

    // Invalidate editable state for all accessible frames
    void InvalidateAccessibleEditableState( bool bAllShells = true,
                                               const SwFrm *pFrm=0 );

    // Invalidate frame's relation set (for chained frames)
    void InvalidateAccessibleRelationSet( const SwFlyFrm *pMaster,
                                          const SwFlyFrm *pFollow );

    // update data for accessible preview
    // change method signature due to new page preview functionality
    void UpdateAccessiblePreview( const std::vector<PreviewPage*>& _rPreviewPages,
                                  const Fraction&  _rScale,
                                  const SwPageFrm* _pSelectedPageFrm,
                                  const Size&      _rPreviewWinSize );

    void InvalidateAccessiblePreviewSelection( sal_uInt16 nSelPage );

    // Fire all accessible events that have been collected so far
    void FireAccessibleEvents();
};

inline SwPageFrm *SwViewImp::GetFirstVisPage()
{
    if ( bFirstPageInvalid )
        SetFirstVisPage();
    return pFirstVisPage;
}

inline const SwPageFrm *SwViewImp::GetFirstVisPage() const
{
    if ( bFirstPageInvalid )
        ((SwViewImp*)this)->SetFirstVisPage();
    return pFirstVisPage;
}

inline SwAccessibleMap& SwViewImp::GetAccessibleMap()
{
    if( !pAccMap )
        CreateAccessibleMap();

    return *pAccMap;
}

inline void SwViewImp::DisposeAccessibleFrm( const SwFrm *pFrm,
                               bool bRecursive )
{
    DisposeAccessible( pFrm, 0, bRecursive );
}

inline void SwViewImp::DisposeAccessibleObj( const SdrObject *pObj )
{
    DisposeAccessible( 0, pObj, false );
}

inline void SwViewImp::MoveAccessibleFrm( const SwFrm *pFrm,
                                          const SwRect& rOldFrm )
{
    MoveAccessible( pFrm, 0, rOldFrm );
}

inline void SwViewImp::AddAccessibleFrm( const SwFrm *pFrm )
{
    SwRect aEmptyRect;
    MoveAccessible( pFrm, 0, aEmptyRect );
}

inline void SwViewImp::AddAccessibleObj( const SdrObject *pObj )
{
    SwRect aEmptyRect;
    MoveAccessible( 0, pObj, aEmptyRect );
}
#endif // INCLUDED_SW_SOURCE_CORE_INC_VIEWIMP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

/*************************************************************************
 *
 *  $RCSfile: OutlineView.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-12 15:10:07 $
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

#ifndef SD_OUTLINE_VIEW_HXX
#define SD_OUTLINE_VIEW_HXX

#ifndef SD_VIEW_HXX
#include "View.hxx"
#endif


class SdPage;
class DrawDocShell;
class SdrPage;
class EditStatus;
class Paragraph;
class SdrTextObj;
class Outliner;
class SfxProgress;

namespace sd {

class OutlineViewShell;

static const int MAX_OUTLINERVIEWS = 4;

/*************************************************************************
|*
|* Ableitung von ::sd::View fuer den Outline-Modus
|*
\************************************************************************/

class OutlineView
    : public ::sd::View
{
public:
    OutlineView (DrawDocShell* pDocSh,
        ::Window* pWindow,
        OutlineViewShell* pOutlineViewSh);
    ~OutlineView (void);

    TYPEINFO();

    SdrTextObj*     GetTitleTextObject(SdrPage* pPage);
    SdrTextObj*     GetLayoutTextObject(SdrPage* pPage);

    virtual void AddWin (::sd::Window* pWin);
    virtual void DelWin (::sd::Window* pWin);

    OutlinerView*   GetViewByWindow (::Window* pWin) const;
    SdrOutliner*    GetOutliner() { return(pOutliner) ; }

    Paragraph*      GetPrevTitle(const Paragraph* pPara);
    Paragraph*      GetNextTitle(const Paragraph* pPara);
    SdPage*         GetActualPage();

    /** selects the paragraph for the given page at the outliner view*/
    void            SetActualPage( SdPage* pActual );

    virtual void Paint (const Rectangle& rRect, ::sd::Window* pWin);
    virtual void AdjustPosSizePixel(
        const Point &rPos,
        const Size &rSize,
        ::sd::Window* pWindow);

                    // Callbacks fuer LINKs
    DECL_LINK( ParagraphInsertedHdl, Outliner * );
    DECL_LINK( ParagraphRemovingHdl, Outliner * );
    DECL_LINK( DepthChangedHdl, Outliner * );
    DECL_LINK( StatusEventHdl, EditStatus * );
    DECL_LINK( BeginMovingHdl, Outliner * );
    DECL_LINK( EndMovingHdl, Outliner * );
    DECL_LINK( RemovingPagesHdl, OutlinerView * );
    DECL_LINK( IndentingPagesHdl, OutlinerView * );

    ULONG         GetPaperWidth() const { return 21000; }  // DIN A4 Breite

    BOOL          PrepareClose(BOOL bUI = TRUE);

    virtual BOOL    GetAttributes( SfxItemSet& rTargetSet, BOOL bOnlyHardAttr=FALSE ) const;
    virtual BOOL    SetAttributes(const SfxItemSet& rSet, BOOL bReplaceAll = FALSE);

//  virtual BOOL       HasMarkedObjUnused() const;
    void               FillOutliner();
    void               SetLinks();
    void               ResetLinks() const;

    SfxStyleSheet*     GetStyleSheet() const;

    void               SetSelectedPages();

    virtual sal_Int8 AcceptDrop (
        const AcceptDropEvent& rEvt,
        DropTargetHelper& rTargetHelper,
        ::sd::Window* pTargetWindow = NULL,
        USHORT nPage = SDRPAGE_NOTFOUND,
        USHORT nLayer = SDRPAGE_NOTFOUND);
    virtual sal_Int8 ExecuteDrop (
        const ExecuteDropEvent& rEvt,
        DropTargetHelper& rTargetHelper,
        ::sd::Window* pTargetWindow = NULL,
        USHORT nPage = SDRPAGE_NOTFOUND,
        USHORT nLayer = SDRPAGE_NOTFOUND);


    // #97766# Re-implement GetScriptType for this view to get correct results
    virtual sal_uInt16 GetScriptType() const;

private:
    OutlineViewShell* pOutlineViewShell;
    SdrOutliner*        pOutliner;
    OutlinerView*       pOutlinerView[MAX_OUTLINERVIEWS];

    Link                aOldParagraphInsertedHdl;
    Link                aOldParagraphRemovingHdl;
    Link                aOldDepthChangedHdl;
    Link                aOldStatusEventHdl;
    Link                aOldBeginMovingHdl;
    Link                aOldEndMovingHdl;
    Link                aOldRemovingPagesHdl;
    Link                aOldIndentingPagesHdl;

    List*               pOldParaOrder;
    List*               pSelectedParas;

    USHORT              nPagesToProcess;     // fuer die Fortschrittsanzeige
    USHORT              nPagesProcessed;

    BOOL                bFirstPaint;

    SfxProgress* mpProgress;

    /** stores the last used high contrast mode.
        this is changed in onUpdateStyleSettings()
    */
    bool mbHighContrastMode;

    /** stores the last used document color.
        this is changed in onUpdateStyleSettings()
    */
    Color maDocColor;

    /** updates the high contrast settings and document color if they changed.
        @param bForceUpdate forces the method to set all style settings
    */
    void onUpdateStyleSettings( bool bForceUpdate = false );

    /** this link is called from the vcl applicaten when the stylesettings
        change. Its only purpose is to call onUpdateStyleSettings() then.
    */
    DECL_LINK( AppEventListenerHdl, void * );
};

} // end of namespace sd

#endif

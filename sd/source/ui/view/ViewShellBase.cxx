/*************************************************************************
 *
 *  $RCSfile: ViewShellBase.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2004-07-06 14:45:24 $
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

#include "ViewShellBase.hxx"

#ifndef SD_DRAW_DOC_SHELL_HXX
#include "DrawDocShell.hxx"
#endif
#ifndef SD_VIEW_SHELL_HXX
#include "ViewShell.hxx"
#endif
#ifndef SD_SUB_SHELL_MANAGER_HXX
#include "SubShellManager.hxx"
#endif
#ifndef SD_OBJECT_BAR_MANAGER_HXX
#include "ObjectBarManager.hxx"
#endif
#ifndef SD_RESID_HXX
#include "sdresid.hxx"
#endif
#include "strings.hrc"
#ifndef _SFXREQUEST_HXX
#include <sfx2/request.hxx>
#endif
#include "app.hrc"
#ifndef _SFXENUMITEM_HXX
#include <svtools/eitem.hxx>
#endif
#ifndef _DRAWDOC_HXX
#include "drawdoc.hxx"
#endif
#ifndef _SFXAPP_HXX
#include <sfx2/app.hxx>
#endif
#include "glob.hrc"
#ifndef _SFXINTITEM_HXX
#include <svtools/intitem.hxx>
#endif
#ifndef _SFXDISPATCH_HXX
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SFXEVENT_HXX
#include <sfx2/event.hxx>
#endif
#include <svx/svxids.hrc>
#ifndef SD_DRAW_CONTROLLER_HXX
#include "DrawController.hxx"
#endif
#ifndef SD_DRAW_VIEW_SHELL_HXX
#include "DrawViewShell.hxx"
#endif
#ifndef SD_DRAW_VIEW_HXX
#include "drawview.hxx"
#endif
#ifndef _COM_SUN_STAR_FRAME_XFRAME_HPP_
#include <com/sun/star/frame/XFrame.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XWINDOW_HPP_
#include <com/sun/star/awt/XWindow.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XCONTROLLER_HPP_
#include <com/sun/star/frame/XController.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif

using namespace sd;
#define ViewShellBase
#include "sdslots.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace sd {

TYPEINIT1(ViewShellBase, SfxViewShell);

SFX_IMPL_VIEWFACTORY(ViewShellBase, SdResId(STR_DEFAULTVIEW))
{
    SFX_VIEW_REGISTRATION(DrawDocShell);
}

SFX_IMPL_INTERFACE(ViewShellBase, SfxViewShell, SdResId(STR_DRAWVIEWSHELL))
{
}

ViewShellBase::ViewShellBase (
    SfxViewFrame* pFrame,
    SfxViewShell* pOldShell,
    ViewShell::ShellType eDefaultSubShell)
    : SfxViewShell (pFrame,
        SFX_VIEW_MAXIMIZE_FIRST   |
        SFX_VIEW_OPTIMIZE_EACH    |
        SFX_VIEW_DISABLE_ACCELS   |
        SFX_VIEW_OBJECTSIZE_EMBEDDED |
        SFX_VIEW_CAN_PRINT           |
        SFX_VIEW_HAS_PRINTOPTIONS),
      mpDocShell (static_cast<DrawDocShell*>(
        GetViewFrame()->GetObjectShell())),
      mpDocument (mpDocShell->GetDoc()),
      mpController (NULL),
      maPrintManager (*this)
{
    Construct (eDefaultSubShell);
}




void ViewShellBase::Construct (
    ViewShell::ShellType eDefaultSubShell)
{
    // Now that this new object has (almost) finished its construction
    // we can pass it as argument to the SubShellManager constructor.
    mpSubShellManager = ::std::auto_ptr<SubShellManager>(
        new SubShellManager(*this));
    GetSubShellManager().SetMainSubShellType (eDefaultSubShell);

    StartListening(*GetViewFrame());
    StartListening(*GetDocShell());

    // Make sure that an instance of the controller exists.  We don't have
    // to call UpdateController() here because the registration at the frame
    // is called automatically.
    mpController = GetSubShellManager().GetMainSubShell()->GetController();
}




ViewShellBase::~ViewShellBase (void)
{
    EndListening(*GetViewFrame());
    EndListening(*GetDocShell());
    mpSubShellManager.reset();
}




SubShellManager& ViewShellBase::GetSubShellManager (void) const
{
    return *mpSubShellManager.get();
}




ObjectBarManager& ViewShellBase::GetObjectBarManager (void) const
{
    return mpSubShellManager->GetObjectBarManager();
}




ViewShell* ViewShellBase::GetMainViewShell (SfxViewFrame* pViewFrame)
{
    ViewShell* pViewShell = NULL;

    if (pViewFrame != NULL)
    {
        // Get the view shell for the frame and cast it to
        // sd::ViewShellBase.
        SfxViewShell* pSfxViewShell = pViewFrame->GetViewShell();
        if (pSfxViewShell->ISA(::sd::ViewShellBase))
        {
            ViewShellBase* pSdViewShellBase =
                PTR_CAST(::sd::ViewShellBase, pSfxViewShell);
            if (pSdViewShellBase != NULL)
            {
                // Use the main sub shell manager to obtain the
                // desired view shell.
                pViewShell =
                    pSdViewShellBase->GetSubShellManager().GetMainSubShell();
            }
        }
    }

    return pViewShell;
}




void ViewShellBase::ExecuteModeChange (SfxRequest& rRequest)
{
    bool bIsActive = true;
    ViewShell::ShellType eType = ViewShell::ST_NONE;

    switch (rRequest.GetSlot())
    {
        case SID_SWITCH_SHELL:
        {
            SFX_REQUEST_ARG(rRequest, pMode, SfxInt32Item,
                SID_SWITCH_SHELL, FALSE);
            eType = static_cast<ViewShell::ShellType>(
                pMode->GetValue());
            switch (eType)
            {
                case ViewShell::ST_IMPRESS:
                case ViewShell::ST_HANDOUT:
                case ViewShell::ST_NOTES:
                case ViewShell::ST_SLIDE:
                case ViewShell::ST_OUTLINE:
                {
                    // Turn off effects.
                    ViewShell* pShell = GetSubShellManager().GetMainSubShell();
                    /*af the current shell will be destroyed in a short time
                         so calling SetAnimationMode() should not be
                         necessary.

                    if (pShell != NULL)
                        if (pShell->ISA(DrawViewShell))
                            static_cast<DrawView*>(pShell->GetDrawView())
                                ->SetAnimationMode(FALSE);
                        */
                    // AutoLayouts have to be ready.
                    GetDocument()->StopWorkStartupDelay();
                    GetSubShellManager().SetMainSubShellType (eType);
                    //          Invalidate ();
                    rRequest.Ignore ();
                }
                break;

                case ViewShell::ST_NONE:
                case ViewShell::ST_PRESENTATION:
                    GetSubShellManager().SetMainSubShellType (eType);
                    rRequest.Done ();
                    break;

                case ViewShell::ST_DRAW:
                case ViewShell::ST_PREVIEW:
                    break;
            }
        }
    }
}




void ViewShellBase::GetMenuState (SfxItemSet& rSet)
{
}




void ViewShellBase::UpdateController (void)
{
    ::osl::MutexGuard aGuard (maMutex);
    ViewShell* pViewShell = GetSubShellManager().GetMainSubShell();
    DrawController* pController = pViewShell->GetController();

    if (pController != NULL)
        SetController (pController);
    Reference <frame::XController> xController (pController);
    Reference <awt::XWindow> xWindow(
        GetFrame()->GetFrame()->GetWindow().GetComponentInterface(),
        UNO_QUERY );
    Reference <frame::XFrame> xFrame (
        GetFrame()->GetFrame()->GetFrameInterface());
    SfxObjectShell* pObjectShellold = GetObjectShell();
    xFrame->setComponent (xWindow, xController);
    xController->attachFrame (xFrame);
    SfxObjectShell* pObjectShell = GetObjectShell();
    Reference <frame::XModel> xModel (pObjectShell->GetModel());
    if (xModel.is())
    {
        xController->attachModel (xModel);
        xModel->connectController (xController);
        xModel->setCurrentController (xController);
    }
}




DrawDocShell* ViewShellBase::GetDocShell (void) const
{
    return mpDocShell;
}



SdDrawDocument* ViewShellBase::GetDocument (void) const
{
    return mpDocument;
}




void ViewShellBase::SFX_NOTIFY(SfxBroadcaster& rBC,
    const TypeId& rBCType,
    const SfxHint& rHint,
    const TypeId& rHintType)
{
    SfxViewShell::SFX_NOTIFY(rBC, rBCType, rHint, rHintType);

    if (rHint.IsA(TYPE(SfxEventHint)))
    {
        if( ((SfxEventHint&)rHint).GetEventId() == SFX_EVENT_OPENDOC )
        {
            if( GetDocument() && GetDocument()->IsStartWithPresentation() )
            {
                if( GetViewFrame() )
                {
                    GetDocument()->SetStartWithPresentation( false );
                    GetViewFrame()->GetDispatcher()->Execute(
                        SID_PRESENTATION, SFX_CALLMODE_ASYNCHRON );
                }
            }
        }
    }
}




void ViewShellBase::InnerResizePixel(const Point &rPos, const Size &rSize)
{
    ViewShell* pMainViewShell = GetSubShellManager().GetMainSubShell();
    if (pMainViewShell != NULL)
        pMainViewShell->InnerResizePixel (rPos, rSize);
}




void ViewShellBase::OuterResizePixel(const Point &rPos, const Size &rSize)
{
    ViewShell* pMainViewShell = GetSubShellManager().GetMainSubShell();
    if (pMainViewShell != NULL)
        pMainViewShell->OuterResizePixel (rPos, rSize);
}




ErrCode ViewShellBase::DoVerb (long nVerb)
{
    ErrCode aResult = ERRCODE_NONE;

    ViewShell* pShell = GetSubShellManager().GetMainSubShell();
    if (pShell != NULL)
        aResult = pShell->DoVerb (nVerb);

    return aResult;
}




SfxPrinter* ViewShellBase::GetPrinter (BOOL bCreate)
{
    return maPrintManager.GetPrinter (bCreate);
}




USHORT ViewShellBase::SetPrinter (
    SfxPrinter* pNewPrinter,

    USHORT nDiffFlags)
{
    return maPrintManager.SetPrinter (pNewPrinter, nDiffFlags);
}




PrintDialog* ViewShellBase::CreatePrintDialog (::Window *pParent)
{
    return maPrintManager.CreatePrintDialog (pParent);
}




SfxTabPage*  ViewShellBase::CreatePrintOptionsPage(
    ::Window *pParent,
    const SfxItemSet &rOptions)
{
    return maPrintManager.CreatePrintOptionsPage (pParent, rOptions);
}




USHORT  ViewShellBase::Print(SfxProgress& rProgress, PrintDialog* pDlg)
{
    return maPrintManager.Print (rProgress, pDlg);
}




ErrCode ViewShellBase::DoPrint (
    SfxPrinter* pPrinter,
    PrintDialog* pPrintDialog,
    BOOL bSilent)
{
    return maPrintManager.DoPrint (pPrinter, pPrintDialog, bSilent);
}




USHORT ViewShellBase::SetPrinterOptDlg (
    SfxPrinter* pNewPrinter,
    USHORT nDiffFlags,
    BOOL bShowDialog)
{
   return maPrintManager.SetPrinterOptDlg (
       pNewPrinter, nDiffFlags, bShowDialog);
}




void ViewShellBase::PreparePrint (PrintDialog* pPrintDialog)
{
    SfxViewShell::PreparePrint (pPrintDialog);
    return maPrintManager.PreparePrint (pPrintDialog);
}




void ViewShellBase::WriteUserDataSequence (
    ::com::sun::star::uno::Sequence <
    ::com::sun::star::beans::PropertyValue >& rSequence,
    sal_Bool bBrowse)
{
    // Forward call to main sub shell.
    ViewShell* pShell = GetSubShellManager().GetMainSubShell();
    if (pShell != NULL)
        pShell->WriteUserDataSequence (rSequence, bBrowse);
}




void ViewShellBase::ReadUserDataSequence (
    const ::com::sun::star::uno::Sequence <
    ::com::sun::star::beans::PropertyValue >& rSequence,
    sal_Bool bBrowse)
{
    // Forward call to main sub shell.
    ViewShell* pShell = GetSubShellManager().GetMainSubShell();
    if (pShell != NULL)
        pShell->ReadUserDataSequence (rSequence, bBrowse);
}




void ViewShellBase::UIActivate (SvInPlaceObject *pIPObj)
{
    // Forward call to main sub shell.
    ViewShell* pShell = GetSubShellManager().GetMainSubShell();
    if (pShell != NULL)
        pShell->UIActivate (pIPObj);
}




void ViewShellBase::UIDeactivate (SvInPlaceObject *pIPObj)
{
    // Forward call to main sub shell.
    ViewShell* pShell = GetSubShellManager().GetMainSubShell();
    if (pShell != NULL)
        pShell->UIDeactivate (pIPObj);
}




void ViewShellBase::SetZoomFactor (
    const Fraction &rZoomX,
    const Fraction &rZoomY)
{
    SfxViewShell::SetZoomFactor (rZoomX, rZoomY);
    // Forward call to main sub shell.
    ViewShell* pShell = GetSubShellManager().GetMainSubShell();
    if (pShell != NULL)
        pShell->SetZoomFactor (rZoomX, rZoomY);
}




USHORT ViewShellBase::PrepareClose (BOOL bUI, BOOL bForBrowsing)
{
    USHORT nResult = SfxViewShell::PrepareClose (bUI, bForBrowsing);

    if (nResult == TRUE)
    {
        // Forward call to main sub shell.
        ViewShell* pShell = GetSubShellManager().GetMainSubShell();
        if (pShell != NULL)
            nResult = pShell->PrepareClose (bUI, bForBrowsing);
    }

    return nResult;
}




void ViewShellBase::WriteUserData (String& rString, BOOL bBrowse)
{
    SfxViewShell::WriteUserData (rString, bBrowse);

    // Forward call to main sub shell.
    ViewShell* pShell = GetSubShellManager().GetMainSubShell();
    if (pShell != NULL)
        pShell->WriteUserData (rString);
}




void ViewShellBase::ReadUserData (const String& rString, BOOL bBrowse)
{
    SfxViewShell::ReadUserData (rString, bBrowse);

    // Forward call to main sub shell.
    ViewShell* pShell = GetSubShellManager().GetMainSubShell();
    if (pShell != NULL)
        pShell->ReadUserData (rString);
}




SdrView* ViewShellBase::GetDrawView (void) const
{
    // Forward call to main sub shell.
    ViewShell* pShell = GetSubShellManager().GetMainSubShell();
    if (pShell != NULL)
        return pShell->GetDrawView ();
    else
        return SfxViewShell::GetDrawView();
}




void ViewShellBase::AdjustPosSizePixel (const Point &rOfs, const Size &rSize)
{
    SfxViewShell::AdjustPosSizePixel (rOfs, rSize);

    // Forward call to main sub shell.
    ViewShell* pShell = GetSubShellManager().GetMainSubShell();
    if (pShell != NULL)
        pShell->AdjustPosSizePixel (rOfs, rSize);
}



} // end of namespace sd

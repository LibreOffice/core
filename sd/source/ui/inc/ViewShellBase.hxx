/*************************************************************************
 *
 *  $RCSfile: ViewShellBase.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2004-08-04 08:55:19 $
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

#ifndef SD_VIEW_SHELL_BASE_HXX
#define SD_VIEW_SHELL_BASE_HXX

#include "ViewShell.hxx"

#ifndef SD_GLOB_HXX
#include "glob.hxx"
#endif
#ifndef _SFXVIEWSH_HXX
#include <sfx2/viewsh.hxx>
#endif
#ifndef _VIEWFAC_HXX
#include <sfx2/viewfac.hxx>
#endif
#ifndef SD_PRINT_MANAGER_HXX
#include "PrintManager.hxx"
#endif
#include <memory>

class SdDrawDocument;
class SfxRequest;


namespace sd {

class DrawDocShell;
class FormShellManager;
class PaneManager;
class ViewShell;
class ViewShellManager;

/** SfxViewShell descendant that the stacked Draw/Impress shells are
    based on.

    <p>The "base" part of the name does not mean that this is a base
    class of some class hierarchy.  It rather is the base of the
    stacked shells.</p>

    <p>This class starts as a new and relatively small class.  Over
    time as much code as possible should be moved from the stacked
    shells to this class.</p>
*/
class ViewShellBase
    : public SfxViewShell
{
public:
    TYPEINFO();
    SFX_DECL_VIEWFACTORY(ViewShellBase);
    SFX_DECL_INTERFACE(SD_IF_SDVIEWSHELLBASE);

    //    ViewShellBase (SfxViewFrame *pFrame, USHORT nFlags);
    /** This constructor is used by the view factory of the SFX macros.
        Note that LateInit() has to be called after the constructor
        terminates and before doing anything else.
    */
    ViewShellBase (
        SfxViewFrame *pFrame,
        SfxViewShell* pOldShell,
        ViewShell::ShellType eDefaultSubShell = ViewShell::ST_IMPRESS);

    virtual ~ViewShellBase (void);

    /** This method is part of the object construction.  It HAS to be called
        after the constructor has created a new object.
    */
    virtual void LateInit (void);

    ViewShellManager& GetViewShellManager (void) const;

    /** Return the main view shell stacked on the called ViewShellBase
        object.  This is usually the view shell displayed in the center
        pane.
    */
    ViewShell* GetMainViewShell (void) const;

    PaneManager& GetPaneManager (void);

    /** When given a view frame this static method returns the
        corresponding sd::ViewShellBase object.
        @return
            When the SfxViewShell of the given frame is not a
            ViewShellBase object then NULL is returned.
    */
    static ViewShellBase* GetViewShellBase (SfxViewFrame* pFrame);

    DrawDocShell* GetDocShell (void) const;
    SdDrawDocument* GetDocument (void) const;

    /** Callback function for retrieving item values related to menu entries.
    */
    void GetMenuState (SfxItemSet& rSet);

    /** Callback function for general slot calls.  At the moment these are
        slots for switching the pane docking windows on and off.
    */
    void Execute (SfxRequest& rRequest);

    /** Callback function for retrieving item values related to certain
        slots.  This is the companion of Execute() and handles the slots
        concerned with showing the pane docking windows.
    */
    void GetState (SfxItemSet& rSet);

    /** Make sure that mpMainController points to a controller that matches
        the current stacked view shell.  If that is not the case the current
        controller is replaced by a new one.  Otherwise this method returns
        without changing anything.
    */
    void UpdateController (void);

    SvBorder GetBorder (bool bOuterResize);
    virtual void InnerResizePixel (const Point& rOrigin, const Size& rSize);
    virtual void OuterResizePixel (const Point& rOrigin, const Size& rSize);

    /** This call is forwarded to the main sub-shell.
    */
    virtual ErrCode DoVerb (long nVerb);

    /// Forwarded to the print manager.
    virtual SfxPrinter* GetPrinter (BOOL bCreate = FALSE);

    /// Forwarded to the print manager.
    virtual USHORT SetPrinter (
        SfxPrinter* pNewPrinter,
        USHORT nDiffFlags = SFX_PRINTER_ALL);

    /// Forwarded to the print manager.
    virtual PrintDialog* CreatePrintDialog (::Window *pParent);

    /// Forwarded to the print manager.
    virtual SfxTabPage* CreatePrintOptionsPage (
        ::Window *pParent,
        const SfxItemSet &rOptions);

    /// Forwarded to the print manager.
    virtual USHORT Print (SfxProgress& rProgress, PrintDialog* pDialog);

    /// Forwarded to the print manager.
    virtual ErrCode DoPrint (
        SfxPrinter *pPrinter,
        PrintDialog *pPrintDialog,
        BOOL bSilent);

    /// Forwarded to the print manager.
    USHORT SetPrinterOptDlg (
        SfxPrinter* pNewPrinter,
        USHORT nDiffFlags = SFX_PRINTER_ALL,
        BOOL _bShowDialog = TRUE);

    virtual void PreparePrint (PrintDialog* pPrintDialog);

    /// Forward methods to main sub shell.
    virtual void WriteUserDataSequence (
        ::com::sun::star::uno::Sequence <
        ::com::sun::star::beans::PropertyValue >&,
        sal_Bool bBrowse = sal_False);
    virtual void ReadUserDataSequence (
        const ::com::sun::star::uno::Sequence <
        ::com::sun::star::beans::PropertyValue >&,
        sal_Bool bBrowse = sal_False);
    virtual void Activate (BOOL IsMDIActivate);
    virtual void Deactivate (BOOL IsMDIActivate);
    virtual void UIActivate (SvInPlaceObject *pIPObj);
    virtual void UIDeactivate (SvInPlaceObject *pIPObj);
    virtual void SetZoomFactor (
        const Fraction &rZoomX,
        const Fraction &rZoomY);
    virtual USHORT PrepareClose (BOOL bUI = TRUE, BOOL bForBrowsing = FALSE);
    virtual void WriteUserData (String&, BOOL bBrowse = FALSE);
    virtual void ReadUserData (const String&, BOOL bBrowse = FALSE);
    virtual SdrView* GetDrawView (void) const;
    virtual void AdjustPosSizePixel (const Point &rOfs, const Size &rSize);

    /** Arrange GUI elements of the pane which shows the given view shell.
        @return
            The returned border contains the controls placed by the method.
    */
    SvBorder ArrangeGUIElements (const Point& rOrigin, const Size& rSize);

    /** When <TRUE/> is given, then the mouse shape is set to hour glass (or
        whatever the busy shape looks like on the system.)
    */
    void SetBusyState (bool bBusy);

    /** Call this method when the controls of this view shell or the
        embedded sub shell need to be rearranged.  This is necessary
        e.g. when the border has been modified (UpdateBorder() calls this
        method).

        This method is like ResizePixel() with no arguments.
    */
    void Rearrange (void);

    /** Update the border that is set with SfxViewShell::SetBorderPixel().
        This is done by adding the border used by the ViewShellBase itself
        with the border used by the main view shell.
    */
    void UpdateBorder (void);

    /** With this method the UI controls can be turned on or off.  It is
        used by the FuSlideShow to hide the UI controls while showing a
        non-full-screen or in-window presentation in the center pane.
    */
    void ShowUIControls (bool bVisible);

protected:
    osl::Mutex maMutex;
    /** The view tab bar is the control for switching between different
        views in one pane.
    */
    ::std::auto_ptr<ViewTabBar> mpViewTabBar;
    virtual void SFX_NOTIFY(SfxBroadcaster& rBC,
        const TypeId& rBCType,
        const SfxHint& rHint,
        const TypeId& rHintType);

private:
    ::std::auto_ptr<ViewShellManager> mpViewShellManager;
    ::std::auto_ptr<PaneManager> mpPaneManager;
    DrawDocShell* mpDocShell;
    SdDrawDocument* mpDocument;

    /// The print manager is responsible for printing documents.
    PrintManager maPrintManager;

    ::std::auto_ptr<FormShellManager> mpFormShellManager;

    /** Common code of OuterResizePixel() and InnerResizePixel().
    */
    void ResizePixel (
        const Point& rOrigin,
        const Size& rSize,
        bool bOuterResize);
};


} // end of namespace sd

#endif

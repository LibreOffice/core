/*************************************************************************
 *
 *  $RCSfile: ViewShell.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2004-07-06 14:44:34 $
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

#ifndef SD_VIEW_SHELL_HXX
#define SD_VIEW_SHELL_HXX

#ifndef _IMAGEBTN_HXX //autogen
#include <vcl/imagebtn.hxx>
#endif
#ifndef _SPLIT_HXX //autogen
#include <vcl/split.hxx>
#endif
#ifndef _SV_FIELD_HXX //autogen
#include <vcl/field.hxx>
#endif
#ifndef _SFXVIEWSH_HXX //autogen
#include <sfx2/viewsh.hxx>
#endif
#ifndef _SV_FIELD_HXX //autogen
#include <vcl/field.hxx>
#endif
#ifndef _SV_PRNTYPES_HXX //autogen
#include <vcl/prntypes.hxx>
#endif
#ifndef _TRANSFER_HXX //autogen
#include <svtools/transfer.hxx>
#endif

#ifndef _SD_GLOB_HXX
#include "glob.hxx"
#endif
#include "pres.hxx"
#include "cfgids.hxx"

#include <memory>

class SdPage;
class SvxRuler;
class SdrOle2Obj;       // fuer die, die Teile von SVDRAW rausdefiniert haben
class ScrollBarBox;
class SdDrawDocument;
class ScrollBar;
class FmFormShell;
class SdOptionsPrintItem;
class MultiSelection;

extern String aEmptyStr;


namespace sd {

class Client;
class DrawDocShell;
class DrawController;
class FrameView;
class FuPoor;
class FuSearch;
class FuSlideShow;
class ObjectBarManager;
class View;
class ViewShellBase;
class Window;
class WindowUpdater;
class ZoomList;

#undef OUTPUT_DRAWMODE_COLOR
#undef OUTPUT_DRAWMODE_CONTRAST

/** Base class of the stacked shell hierarchy.

    <p>Despite its name this class is not a descendant of SfxViewShell
    but of SfxShell.  Its name expresses the fact that it acts like a
    view shell.  Beeing a stacked shell rather then being an actual view shell
    there can be several instances of this class that
    <ul>
    <li>all are based on the same view shell and thus show the same
    document and share common view functionality and</li>
    <li>are all visible at the same time and live in the same
    frame.</li>
    <ul></p>

    <p>This class replaces the former ViewShell class.</p>
*/
class ViewShell
    : public SfxShell
{
public:
    enum ShellType {
        ST_NONE,
        ST_DRAW, ST_IMPRESS, ST_NOTES, ST_HANDOUT, ST_OUTLINE,
        ST_SLIDE, ST_PREVIEW, ST_PRESENTATION };
    static const int MAX_HSPLIT_CNT = 2;
    static const int MAX_VSPLIT_CNT = 2;
    static const int MIN_SCROLLBAR_SIZE = 50;

    static const ULONG OUTPUT_DRAWMODE_COLOR = DRAWMODE_DEFAULT;
    static const ULONG OUTPUT_DRAWMODE_GRAYSCALE =
        DRAWMODE_GRAYLINE | DRAWMODE_GRAYFILL | DRAWMODE_BLACKTEXT
        | DRAWMODE_GRAYBITMAP | DRAWMODE_GRAYGRADIENT;
    static const int  OUTPUT_DRAWMODE_BLACKWHITE =
        DRAWMODE_BLACKLINE | DRAWMODE_BLACKTEXT | DRAWMODE_WHITEFILL
        | DRAWMODE_GRAYBITMAP | DRAWMODE_WHITEGRADIENT;
    static const int OUTPUT_DRAWMODE_CONTRAST = DRAWMODE_SETTINGSLINE
        | DRAWMODE_SETTINGSFILL | DRAWMODE_SETTINGSTEXT | DRAWMODE_SETTINGSGRADIENT;

    TYPEINFO();

    ViewShell (SfxViewFrame *pFrame, ViewShellBase& rViewShellBase,
        bool bAllowCenter = true);
    ViewShell (SfxViewFrame *pFrame, const ViewShell& rShell);
    virtual ~ViewShell (void);

    /** The Init method has to be called from the outside directly
        after a new object of this class has been created.  It can be
        used for that part of the initialisation that can be run only
        after the creation of the new object is finished.  This
        includes registration as listener at event broadcasters.
    */
    virtual void Init (void);

    void Cancel();
    void CancelSearching();

    virtual ::sd::View* GetView() const { return (pView); }
    virtual SdrView* GetDrawView() const { return ( (SdrView*) pView); } // fuer den Sfx
    DrawDocShell* GetDocSh (void) const;
    SdDrawDocument*  GetDoc (void) const;

    SfxViewFrame* GetViewFrame (void) const;

    // diese Funktionen werden gerufen, wenn Fenster gesplittet wird
    // bzw., wenn Splitting aufgehoben wird
    virtual void AddWindow(::sd::Window* pWin) {}
    virtual void RemoveWindow(::sd::Window* pWin) {}

    ::sd::Window* GetActiveWindow() { return pWindow; }
    void SetActiveWindow (::sd::Window* pWindow);

    const Rectangle& GetAllWindowRect();

    // Mouse- & Key-Events
    virtual void Paint(const Rectangle& rRect, ::sd::Window* pWin) {}
    virtual BOOL KeyInput(const KeyEvent& rKEvt, ::sd::Window* pWin);
    virtual void MouseMove(const MouseEvent& rMEvt, ::sd::Window* pWin);
    virtual void MouseButtonUp(const MouseEvent& rMEvt, ::sd::Window* pWin);
    virtual void MouseButtonDown(const MouseEvent& rMEvt, ::sd::Window* pWin);
    virtual void Command(const CommandEvent& rCEvt, ::sd::Window* pWin);
    virtual BOOL RequestHelp( const HelpEvent& rEvt, ::sd::Window* pWin );

    virtual void Draw(OutputDevice &rDev, const Region &rReg) {}

    /** Set the position and size of the area which contains the GUI
        elements like rulers, sliders, and buttons as well as the document
        view.  Both size and position are expected to be in pixel
        coordinates.  The positions and sizes of the mentioned GUI elements
        are updated as well.

        <p> This method is implemented by first setting copying the given
        values to internal variables and then calling the
        <type>ArrangeGUIElements</type> method which performs the actual
        work of sizeing and arranging the UI elements accordingly.</p>
        @param rPos
            The position of the enclosing window relative to the document
            window.  This is only interesting if a Draw/Impress document
            view is embedded as OLE object into another document view.  For
            normal documents this position is (0,0).
        @param rSize
            The new size in pixel.
    */
    virtual void AdjustPosSizePixel(const Point &rPos, const Size &rSize);

    /** Arrange and resize the GUI elements like rulers, sliders, and
        buttons as well as the actual document view according to the size of
        the enclosing window and current sizes of buttons, rulers, and
        sliders.
    */
    virtual void ArrangeGUIElements (void);
    virtual void SetUIUnit(FieldUnit eUnit);
    virtual void SetDefTabHRuler( UINT16 nDefTab );

    BOOL    HasRuler() { return bHasRuler; }
    void    SetRuler(BOOL bRuler);

    void    UpdateScrollBars();
    void    Scroll(long nX, long nY);
    void    ScrollLines(long nX, long nY);
    virtual void    SetZoom(long nZoom);
    virtual void    SetZoomRect(const Rectangle& rZoomRect);
    void    InitWindows(const Point& rViewOrigin, const Size& rViewSize,
                        const Point& rWinPos, BOOL bUpdate = FALSE);
    void    InvalidateWindows();
    void    UpdateWindows();
    virtual void    UpdatePreview( SdPage* pPage, BOOL bInit = FALSE );
    void    DrawMarkRect(const Rectangle& rRect) const;
    void    DrawFilledRect( const Rectangle& rRect, const Color& rLColor,
                            const Color& rFColor ) const;

    void    ExecReq( SfxRequest &rReq );

    ZoomList*     GetZoomList() { return pZoomList; }

    FrameView* GetFrameView (void);
    virtual void  ReadFrameViewData(FrameView* pView);
    virtual void  WriteFrameViewData();
    virtual void  WriteUserData(String& rString);
    virtual void  ReadUserData(const String& rString);

    virtual BOOL  ActivateObject(SdrOle2Obj* pObj, long nVerb);

    virtual SdPage* GetActualPage() = 0;
                    // kann auch NULL sein
    FuPoor* GetOldFunction() const    { return pFuOld; }
    FuPoor* GetActualFunction() const { return pFuActual; }

    FuSlideShow* GetSlideShow() const { return pFuSlideShow; }

    void    SetPageSizeAndBorder(PageKind ePageKind, const Size& rNewSize,
                            long nLeft, long nRight, long nUpper, long nLower,
                            BOOL bScaleAll, Orientation eOrient, USHORT nPaperBin,
                            BOOL bBackgroundFullSize );

    void    UpdateSlideChangeWindow();
    void    AssignFromSlideChangeWindow();

    void    SetStartShowWithDialog( BOOL bIn = TRUE ) { bStartShowWithDialog = bIn; }
    BOOL    IsStartShowWithDialog() const { return bStartShowWithDialog; }

    USHORT  GetPrintedHandoutPageNum() const { return nPrintedHandoutPageNum; }

    virtual USHORT PrepareClose( BOOL bUI = TRUE, BOOL bForBrowsing = FALSE );

    void GetMenuState(SfxItemSet& rSet);

    virtual sal_Int8 AcceptDrop( const AcceptDropEvent& rEvt, DropTargetHelper& rTargetHelper,
                                 ::sd::Window* pTargetWindow, USHORT nPage, USHORT nLayer );
    virtual sal_Int8 ExecuteDrop( const ExecuteDropEvent& rEvt, DropTargetHelper& rTargetHelper,
                                  ::sd::Window* pTargetWindow, USHORT nPage, USHORT nLayer );

    virtual void WriteUserDataSequence ( ::com::sun::star::uno::Sequence < ::com::sun::star::beans::PropertyValue >&, sal_Bool bBrowse = sal_False );
    virtual void ReadUserDataSequence ( const ::com::sun::star::uno::Sequence < ::com::sun::star::beans::PropertyValue >&, sal_Bool bBrowse = sal_False );

    /** this method is called when the visible area of the view from this viewshell is changed */
    virtual void VisAreaChanged(const Rectangle& rRect);

    /** Create an accessible object representing the specified window.
        Overload this method to provide view mode specific objects.  The
        default implementation returns an empty reference.
        @param pWindow
            Make the document displayed in this window accessible.
        @return
            This default implementation returns an empty reference.
    */
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible>
        CreateAccessibleDocumentView (::sd::Window* pWindow);

    /** makes the SdPreviewChildWindow visible or invisible
        @param bVisible if set to true, the preview is shown, if set to false, the preview is hidden
    */
    void SetPreview( bool bVisible );

    void SetWinViewPos(const Point& rWinPos, bool bUpdate);
    Point GetWinViewPos() const;
    Point GetViewOrigin() const;

    /** Return the window updater of this view shell.
        @return
            In rare circumstances the returned pointer may be <null/>,
            i.e. when no memory is available anymore.
    */
    ::sd::WindowUpdater* GetWindowUpdater (void) const;


    /** Return the current/main window of this view shell.
        This method exists for reasons of compatibility.
        May change later.
    */
    ::Window* GetWindow (void) const;

    ViewShellBase& GetViewShellBase (void) const;
    ObjectBarManager& GetObjectBarManager (void) const;

    /** Return an object that implements the necessary UNO interfaces to act
        as a controller for the ViewShellBase object.  The controller is
        created if it does not exist.  It is owned by the caller.
        @return
            Returns NULL when the controller does not yet exist and can not
            be created.
    */
    DrawController* GetController (void);

    /** Return the type of the shell.
    */
    virtual ShellType GetShellType (void) const;

    virtual void InnerResizePixel(const Point &rPos, const Size &rSize);
    virtual void OuterResizePixel(const Point &rPos, const Size &rSize);

    /** This function is called from the underlying ViewShellBase
        object to handle a verb execution request.
    */
    virtual ErrCode DoVerb (long nVerb);

    void PrintOutline(SfxPrinter& rPrinter, SfxProgress& rProgress,
                      const MultiSelection& rSelPages,
                      const String& rTimeDateStr, const Font& rTimeDateFont,
                      const SdOptionsPrintItem* pPrintOpts,
                      USHORT nPage, USHORT nPageMax,
                      USHORT nCopies, USHORT nProgressOffset, USHORT nTotal );
    void PrintHandout(SfxPrinter& rPrinter, SfxProgress& rProgress,
                      const MultiSelection& rSelPages,
                      const String& rTimeDateStr, const Font& rTimeDateFont,
                      const SdOptionsPrintItem* pPrintOpts,
                      USHORT nPage, USHORT nPageMax,
                      USHORT nCopies, USHORT nProgressOffset, USHORT nTotal );
    void PrintStdOrNotes(SfxPrinter& rPrinter, SfxProgress& rProgress,
                         const MultiSelection& rSelPages,
                         const String& rTimeDateStr, const Font& rTimeDateFont,
                         const SdOptionsPrintItem* pPrintOpts,
                         USHORT nPage, USHORT nPageMax,
                         USHORT nCopies, USHORT nProgressOffset, USHORT nTotal,
                         PageKind ePageKind, BOOL bPrintMarkedOnly);

protected:
    friend class ViewShellBase;

    // #96090# Support methods for centralized UNDO/REDO
    /** Return the undo manager of the currently active object bar.
        This is usually the one of the document.  Only the outline
        view has its own (the one of its Outliner object.)
    */
    void CreateBorder();
    SfxUndoManager* ImpGetUndoManager (void) const;
    void ImpGetUndoStrings(SfxItemSet &rSet) const;
    void ImpGetRedoStrings(SfxItemSet &rSet) const;
    void ImpSidUndo(BOOL bDrawViewShell, SfxRequest& rReq);
    void ImpSidRedo(BOOL bDrawViewShell, SfxRequest& rReq);

    // zweidimensionales Zeigerarray (X/Y) mit Fensterzeigern fuer alle
    // moeglichen Splitter-Aufteilungen
    ::sd::Window*   pWinArray[MAX_HSPLIT_CNT][MAX_VSPLIT_CNT];
    // Zeigerarrays fuer die ScrollBars
    ScrollBar*  pHScrlArray[MAX_HSPLIT_CNT];
    ScrollBar*  pVScrlArray[MAX_VSPLIT_CNT];
    // Zeigerarrays fuer die Lineale
    SvxRuler*   pHRulerArray[MAX_HSPLIT_CNT];
    SvxRuler*   pVRulerArray[MAX_VSPLIT_CNT];
    ScrollBarBox* pScrlBox;

    BOOL        bIsHSplit;
    BOOL        bIsVSplit;
    BOOL        bHasRuler;
    // aktives Fenster
    ::sd::Window* pWindow;
    ::sd::View* pView;
    FrameView*  pFrameView;

    FuPoor*      pFuActual;
    FuPoor*      pFuOld;
    FuSearch*    pFuSearch;
    FuSlideShow* pFuSlideShow;
    ZoomList*    pZoomList;

    Splitter    aHSplit;
    Splitter    aVSplit;

    ImageButton aDrawBtn;
    ImageButton aOutlineBtn;
    ImageButton aSlideBtn;
    ImageButton aNotesBtn;
    ImageButton aHandoutBtn;
    ImageButton aPresentationBtn;

    Point       aViewPos;
    Size        aViewSize;
    Size        aScrBarWH;

    BOOL        bCenterAllowed;           // wird an Fenster weitergegeben

    BOOL        bStartShowWithDialog;   // Praesentation wurde ueber Dialog gestartet
    USHORT      nPrintedHandoutPageNum; // Seitennummer der zu durckenden Handzettelseite

    // Bereich aller Fenster, wenn Splitter aktiv sind
    Rectangle   aAllWindowRect;

    /// The sub-controller.  Returned by GetSubController.  Owned by
    /// this base class once it has been created.
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>
        mxSubController;
    /// The controller.  Returned by GetController().
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>
        mxController;
    DrawController* mpController;

    /// The type of the shell.  Returned by GetShellType().
    ShellType meShellType;

    DECL_LINK( SplitHdl, Splitter * );
    DECL_LINK( HScrollHdl, ScrollBar * );
    DECL_LINK( VScrollHdl, ScrollBar * );
    DECL_LINK( ModeBtnHdl, Button * );

    // virt. Scroll-Handler, hier koennen sich abgeleitete Klassen einklinken
    virtual long VirtHScrollHdl(ScrollBar* pHScroll);
    virtual long VirtVScrollHdl(ScrollBar* pVScroll);

    // virtuelle Funktionen fuer Lineal-Handling
    virtual SvxRuler* CreateHRuler(::sd::Window* pWin, BOOL bIsFirst) { return NULL; }
    virtual SvxRuler* CreateVRuler(::sd::Window* pWin) { return NULL; }
    virtual void UpdateHRuler() {}
    virtual void UpdateVRuler() {}

    // Zeiger auf ein zusaetzliches Control im horizontalen ScrollBar
    // abgeleiteter Klassen (z.B. ein TabBar) zurueckgeben
    virtual long GetHCtrlWidth() { return 0; }

    virtual void Activate(BOOL IsMDIActivate);
    virtual void Deactivate(BOOL IsMDIActivate);

    virtual void UIActivate( SvInPlaceObject* );
    virtual void UIDeactivate( SvInPlaceObject* );

    virtual void SetZoomFactor( const Fraction &rZoomX,
                                const Fraction &rZoomY );


    //  virtual void PreparePrint(PrintDialog* pPrintDialog = 0);


    void PrintPage( SfxPrinter& rPrinter, ::sd::View* pPrintView,
                    SdPage* pPage, BOOL bPrintMarkedOnly );

private:
    void    CreateHSplitElems(long nSplitXPixel);
    void    CreateVSplitElems(long nSplitYPixel);

    /** Code common to all constructors.  It generally is a bad idea
        to call this function from outside a constructor.
    */
    void Construct (void);

    /** This window updater is used to keep all relevant windows up to date
        with reference to the digit langugage used to display digits in text
        shapes.
    */
    ::std::auto_ptr< ::sd::WindowUpdater> mpWindowUpdater;

    DECL_LINK(FrameWindowEventListener, VclSimpleEvent*);

};

} // end of namespace sd

#endif

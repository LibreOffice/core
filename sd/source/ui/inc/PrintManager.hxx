/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PrintManager.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 05:13:07 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef SD_PRINT_MANAGER_HXX
#define SD_PRINT_MANAGER_HXX

#include <tools/solar.h>
#include <tools/errcode.hxx>
#include <sfx2/viewsh.hxx>

class PrintDialog;
class SfxItemSet;
class SfxPrinter;
class SfxProgress;
class SfxTabPage;
class Window;

namespace sd {

class ViewShellBase;

/** Provide some functions to aid the view shell in printing a
    document.  The functions are about asking the user for specific
    information what to print and doing the high level printing.  The
    printing of the actual pages is done by the main sub-shell.
*/
class PrintManager
{
public:
    /* Create a new print manager for the given view shell.
        @param rViewShell
            The life time of that view shell has to exceed that of the
            print manager.
    */
    PrintManager (ViewShellBase& rViewShell);
    virtual ~PrintManager (void);

    /// Forwarded to the document shell.
    virtual SfxPrinter* GetPrinter (BOOL bCreate = FALSE);
    /// Forwarded to the main sub-shell.
    virtual USHORT SetPrinter (
        SfxPrinter* pNewPrinter,
        USHORT nDiffFlags = SFX_PRINTER_ALL);
    /// Forwarded to the main sub-shell.
    virtual PrintDialog* CreatePrintDialog (::Window *pParent);

    /** Create an options tab page for the curren document.
    */
    virtual SfxTabPage* CreatePrintOptionsPage (
        ::Window *pParent,
        const SfxItemSet &rOptions);

    /** Print the document.
        @param pDialog
            The dialog specifies what to print.
    */
    virtual USHORT Print (SfxProgress& rProgress, PrintDialog* pDialog);

    /** Show a dialog that allows the user to select the print range
        (among other things) before starting the actual printing.
    */
    virtual ErrCode DoPrint (
        SfxPrinter *pPrinter,
        PrintDialog *pPrintDialog,
        BOOL bSilent);

    /** When called with <TRUE/> then printing is restricted to the
        selected pages.  Otherwise all pages are printed.  Note that
        you have to set the page range with a call to
        SetPrintingPageRange().
    */
    void RestrictPrintingToSelection (bool bFlag);

    /** Set the range of the pages to print.  This is taken into
        account only after a previous call to
        RestrictPrintingToSelection() with <TRUE/> as argument.
    */
    void SetPrintingPageRange (const String& rsPageRange);

    USHORT SetPrinterOptDlg (
        SfxPrinter* pNewPrinter,
        USHORT nDiffFlags = SFX_PRINTER_ALL,
        BOOL _bShowDialog = TRUE);

    void PreparePrint (PrintDialog* pPrintDialog = 0);

private:
    ViewShellBase& mrViewShell;

    bool mbPrintDirectSelected;

    String msPageRange;

    /// Copy constructor not supported.
    PrintManager (const PrintManager&);

    /// Assignment operator not supported.
    PrintManager& operator= (const PrintManager&);

    /** Make sure that the pages that are to be printed do fit to the
        printer pages.  When the internal pages are larger then the printer
        pages and no method of resolution has yet been provided then a
        dialog is opened that asks for one.  These methods are
        a) scale internal page down to the printer page,
        b) crop internal page
        c) print internal page on several printer pages.
        @param pPrinter
            The printer to print the pages on that are specified by the
            msPageRange member.
        @param bSilent
            This flag, when set, tells the method not to wait for user
            input, i.e. show a dialog.
        @return
            The returned flag indicates whether the method was successfull (<TRUE/>)
            or whether the printing has to be aborted (<FALSE/>).  The later
            one is the case only when the dialog is shown and the user
            selects 'Cancel'.
    */
    bool FitPageToPrinterWithDialog (
        SfxPrinter *pPrinter,
        bool bSilent);
};

} // end of namespace sd

#endif

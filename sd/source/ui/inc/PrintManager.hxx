/*************************************************************************
 *
 *  $RCSfile: PrintManager.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2004-01-20 11:40:59 $
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
};

} // end of namespace sd

#endif

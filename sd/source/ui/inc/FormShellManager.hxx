/*************************************************************************
 *
 *  $RCSfile: FormShellManager.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 13:54:35 $
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

#ifndef SD_FORM_SHELL_MANAGER_HXX
#define SD_FORM_SHELL_MANAGER_HXX

#include <tools/link.hxx>

class VclWindowEvent;
class FmFormShell;

namespace sd {

class PaneManagerEvent;
class ViewShell;
class ViewShellBase;

/** This simple class is responsible for putting the form shell at the top
    or the bottom of the shell stack maintained by the ObjectBarManager.

    The form shell is moved to the top of the shell stack when it is
    activated, i.e. the FormControlActivated handler is called.

    It is moved to the bottom of the shell stack when the main window of the
    view shell displayed in the center pane is focused.
*/
class FormShellManager
{
public:
    FormShellManager (ViewShellBase& rBase);
    ~FormShellManager (void);

private:
    ViewShellBase& mrBase;

    enum StackPosition {SP_BELOW_VIEW_SHELL, SP_ABOVE_VIEW_SHELL, SP_UNKNOWN};

    void RegisterAtCenterPane (ViewShell* pShell);
    void UnregisterAtCenterPane (ViewShell* pShell);

    /** Remember whether the form shell is currently at the top of the shell
        stack or below the view shell.  Until one of the event handlers is
        called the stack position is unknown.
    */
    StackPosition meStackPosition;

    /** This call back is called by the application window (among others)
        when the window gets the focus.  In this case the form shell is
        moved to the bottom of the shell stack.
    */
    DECL_LINK(WindowEventHandler, VclWindowEvent*);

    /** This call back is called when the PaneManager switches shells in a
        pane.  When this happens in the center pane we unregister at the
        window of the old and register at the window of the new shell.
    */
    DECL_LINK(PaneManagerEventHandler, PaneManagerEvent*);

    /** This call back is called by the form shell when it gets the focus.
        In this case the form shell is moved to the top of the shell stack.
    */
    DECL_LINK(FormControlActivated, FmFormShell*);
};

} // end of namespace sd

#endif

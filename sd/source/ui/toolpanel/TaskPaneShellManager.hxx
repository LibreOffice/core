/*************************************************************************
 *
 *  $RCSfile: TaskPaneShellManager.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 14:37:11 $
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

#ifndef SD_TOOLPANEL_TASK_PANE_SHELL_MANAGER_HXX
#define SD_TOOLPANEL_TASK_PANE_SHELL_MANAGER_HXX

#include <list>
#include <vector>

class SfxShell;

namespace sd {
class ViewShell;
class ViewShellManager;
}

namespace sd { namespace toolpanel {

class TaskPaneShellManager
{
public:
    /** Create a shell manager that manages the stacked shells for the given
        view shell.  It works together with the given view shell manager.
    */
    TaskPaneShellManager (
        ViewShellManager& rViewShellManager,
        const ViewShell& rViewShell);

    void AddSubShell (SfxShell* pShell);
    void RemoveSubShell (SfxShell* pShell);

    /** Move the given sub-shell to the top of the local shell stack.
        Furthermore move the view shell whose sub-shells this class manages
        to the top of the global shell stack.
    */
    void MoveToTop (SfxShell* pShell);

    void GetLowerShellList (::std::vector<SfxShell*>& rShellList) const;
    void GetUpperShellList (::std::vector<SfxShell*>& rShellList) const;

private:
    /** The shell stack contains the shells from the task pane manager that
        will be stacked on the ViewShellBase object with the next calls to
        Get(Lower|Upper)ShellList().  The top element is front().
    */
    typedef ::std::list<SfxShell*> ShellStack;
    ShellStack maShellStack;
    ViewShellManager& mrViewShellManager;

    /// The view shell whose sub-shells this class manages.
    const ViewShell& mrViewShell;
};

} } // end of namespace ::sd::toolpanel

#endif

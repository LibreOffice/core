/*************************************************************************
 *
 *  $RCSfile: TaskPaneFocusManager.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 14:36:41 $
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

#ifndef SD_TOOLPANEL_FOCUS_MANAGER_HXX
#define SD_TOOLPANEL_FOCUS_MANAGER_HXX

#ifndef _LINK_HXX
#include <tools/link.hxx>
#endif

#include <hash_map>

class VclSimpleEvent;
class Window;

namespace sd { namespace toolpanel {

class WindowHash
{
public:
    size_t operator()(::Window* argument) const
    { return reinterpret_cast<unsigned long>(argument); }
};

/** Manage the focus in a window hierarchy.  The focus manager handles
    focus changes between different levels in the hierarchy,
    i.e. transferring the focus to a predecessor or succesor.

    <p>For this every window that wants its focus managed has to
    register or be registered and tell where to put the focus when
    going up or down.</p>
*/
class FocusManager
{
public:
    /** Return an instance of the focus manager.
    */
    static FocusManager& Instance (void);

    /** Register a bidirectional link between the two windows.
        @param pParent
            The window that is higher up in the window hierarchy.
        @param pChild
            The window that is lower down in a sub-tree of the window
            hierarchy that is rooted in pParent.
    */
    void RegisterLink (::Window* pParent, ::Window* pChild);

    /** Register a link that will be taken into account when moving up
        in the window hierarchy.
        @param pSource
            The window from which the focus will be transferred.
        @param pTarget
            The window to which the focus will be transferred.
    */
    void RegisterUpLink (::Window* pSource, ::Window* pTarget);

    /** Register a link that will be taken into account when moving down
        in the window hierarchy.
        @param pSource
            The window from which the focus will be transferred.
        @param pTarget
            The window to which the focus will be transferred.
    */
    void RegisterDownLink (::Window* pSource, ::Window* pTarget);

    /** Transfer the focus from the given window to one higher up in
        the hierarchy, i.e. nearer to the root window.
        @param pWindow
            Transfer the focus from this window to one of its
            predecessors.  For this to work the appropriate links has
            to have been registered previously.
        @return
            Returns <TRUE/> when the given window has been found
            amongst the registered windows and the focus transfer has
            been made successfully.
    */
    bool TransferFocusUp (::Window* pWindow);

    /** Transfer the focus from the given window to one lower down in
        the hierarchy, i.e. nearer one of the leafs.
        @param pWindow
            Transfer the focus from this window to one of its
            successors.  For this to work the appropriate links has to
            have been registered previously.
        @return
            Returns <TRUE/> when the given window has been found
            amongst the registered windows and the focus transfer has
            been made successfully.
    */
    bool TransferFocusDown (::Window* pWindow);

private:
    static FocusManager* spInstance;

    typedef ::std::hash_map< ::Window*, ::Window*, WindowHash> HashMap;
    HashMap maDownLinks;
    HashMap maUpLinks;

    FocusManager (void);
    ~FocusManager (void);

    /** Listen for key events and on KEY_RETURN go down and on
        KEY_ESCAPE go up.
    */
    DECL_LINK(WindowEventListener, VclSimpleEvent*);
};

} } // end of namespace ::sd::toolpanel

#endif

/*************************************************************************
 *
 *  $RCSfile: ControlContainerDescriptor.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 14:31:18 $
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

#ifndef SD_TOOLPANEL_CONTROL_CONTAINER_DESCRIPTOR_HXX
#define SD_TOOLPANEL_CONTROL_CONTAINER_DESCRIPTOR_HXX

#include "ILayoutableWindow.hxx"
#include "TitleBar.hxx"

#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef _SV_GEN_HXX
#include <tools/gen.hxx>
#endif
#ifndef SD_WINDOW_HXX
#include <vcl/window.hxx>
#endif
#include <memory>

class Window;

namespace sd { namespace toolpanel {

class ControlContainer;

/** Collection of information the describes entries of the tool
    panel.  A descriptor owns the control it is associated with.
*/
class ControlContainerDescriptor
    : public ::Window,
      public virtual ILayoutableWindow
{
public:
    /** Create a new descriptor for the given control.
        @param rContainer
            The container to inform about selection (caused by mouse
            clicks or keyboard.)
        @param pParent
            The parent window of the new descriptor.
        @param pControl
            The control that is shown when being in the expanded
            state.
        @param rTitle
            String that is shown as title in the title area above the
            control.
        @param eType
            Type of the title bar.  This specifies how the title bar
            will be formated.  For more information see TitleBar.

    */
    ControlContainerDescriptor (
        ControlContainer& rContainer,
        ::Window* pParent,
        ::std::auto_ptr<ILayoutableWindow> pControl,
        const String& rTitle,
        TitleBar::TitleBarType eType);

    virtual ~ControlContainerDescriptor (void);


    virtual Size GetPreferredSize (void);
    virtual int GetPreferredWidth (int nHeight);
    virtual int GetPreferredHeight (int nWidth);
    virtual bool IsResizable (void);
    virtual ::Window* GetWindow (void);

    virtual void Resize (void);
    virtual void GetFocus (void);
    virtual void LoseFocus (void);
    virtual void MouseButtonUp (const MouseEvent& rMouseEvent);
    virtual void KeyInput (const KeyEvent& rEvent);

    void Select (bool bExpansionState);

    //    const Rectangle& GetTitleBarBox (void) const;

    Window* GetControl (void) const;
    const String& GetTitle (void) const;

    void Expand (bool bExpanded = true);
    void Collapse (void);
    bool IsExpanded (void) const;

    /** Ownership of the given data remains with the caller.  The data
        is thus not destroyed when the destructor of this class is
        called.
    */
    void SetUserData (void* pUserData);
    void* GetUserData (void) const;

    bool IsVisible (void) const;
    void SetVisible (bool bVisible);

private:
    ControlContainer& mrContainer;
    ::std::auto_ptr<TitleBar> mpTitleBar;
    ::std::auto_ptr<ILayoutableWindow> mpControl;
    String msTitle;
    bool mbExpanded;
    bool mbVisible;
    void* mpUserData;
    bool mnVisible;

    /// Do not use! Assignment operator is not supported.
    const ControlContainerDescriptor& operator= (
        const ControlContainerDescriptor& aDescriptor);

    void UpdateStates (void);

    DECL_LINK(WindowEventListener, VclSimpleEvent*);
};

} } // end of namespace ::sd::toolpanel

#endif

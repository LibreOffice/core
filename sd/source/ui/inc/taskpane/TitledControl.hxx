/*************************************************************************
 *
 *  $RCSfile: TitledControl.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2005-07-14 10:15:00 $
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

#ifndef SD_TASKPANE_TITLED_CONTROL_HXX
#define SD_TASKPANE_TITLED_CONTROL_HXX

#include "taskpane/TaskPaneTreeNode.hxx"
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
class ControlFactory;

/** This wrapper adds a title bar to a control.  Both title bar and
    control are child windows.
*/
class TitledControl
    : public ::Window,
      public TreeNode
{
public:
    /** Create a new descriptor for the given control.
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
    TitledControl (
        TreeNode* pParent,
        ::std::auto_ptr<TreeNode> pControl,
        const String& rTitle,
        TitleBar::TitleBarType eType);

    TitledControl (
        TreeNode* pParent,
        ::std::auto_ptr<ControlFactory> pControlFactory,
        const String& rTitle,
        TitleBar::TitleBarType eType);

    virtual ~TitledControl (void);


    virtual Size GetPreferredSize (void);
    virtual sal_Int32 GetPreferredWidth (sal_Int32 nHeight);
    virtual sal_Int32 GetPreferredHeight (sal_Int32 nWidth);
    virtual bool IsResizable (void);
    virtual ::Window* GetWindow (void);

    virtual void Resize (void);
    virtual void GetFocus (void);
    virtual void LoseFocus (void);
    virtual void KeyInput (const KeyEvent& rEvent);

    //    void Select (bool bExpansionState);

    TitleBar* GetTitleBar (void);
    /** Return the control child.  When a control factory has been given and
        the control has not yet been created and the given flag is <TRUE/>
        then the control is created.
    */
    TreeNode* GetControl (bool bCreate=true);
    const TreeNode* GetConstControl (bool bCreate=true) const;

    const String& GetTitle (void) const;

    /** Expand the control without informing its container.  This
        method ususally is called by the container as a result of a
        higher level expand command.  You may want to use
        ExpandViaContainer() instead.
        @param bExpanded
            When <TRUE/> then the control is expanded, otherwise it is
            collapsed.
    */
    virtual bool Expand (bool bExpanded = true);

    /** Return whether the control is currently expanded (<TRUE/>) or
        not (<FALSE/>).
    */
    virtual bool IsExpanded (void) const;

    /** Returns the value of the control.
    */
    virtual bool IsExpandable (void) const;

    /** Ownership of the given data remains with the caller.  The data
        is thus not destroyed when the destructor of this class is
        called.
    */
    void SetUserData (void* pUserData);
    void* GetUserData (void) const;

    virtual bool IsShowing (void) const;
    virtual void Show (bool bVisible);

    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible > CreateAccessibleObject (
            const ::com::sun::star::uno::Reference<
            ::com::sun::star::accessibility::XAccessible>& rxParent);

private:
    String msTitle;
    bool mbVisible;
    void* mpUserData;
    ::std::auto_ptr<ControlFactory> mpControlFactory;

    /** Remember whether to toggle (true) the expansion state when the title
        bar is clicked on.  When set to false then the control is always
        expanded.
    */
    bool mbExpansionModeIsToggle;

    /// Do not use! Assignment operator is not supported.
    const TitledControl& operator= (
        const TitledControl& aDescriptor);

    void UpdateStates (void);

    DECL_LINK(WindowEventListener, VclSimpleEvent*);
};

} } // end of namespace ::sd::toolpanel

#endif

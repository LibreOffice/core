/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef SD_TASKPANE_TITLED_CONTROL_HXX
#define SD_TASKPANE_TITLED_CONTROL_HXX

#include "taskpane/TaskPaneTreeNode.hxx"
#include "taskpane/ControlContainer.hxx"
#include "TitleBar.hxx"
#include <com/sun/star/drawing/framework/XResourceId.hpp>
#include <tools/string.hxx>
#include <tools/gen.hxx>
#ifndef SD_WINDOW_HXX
#include <vcl/window.hxx>
#endif
#include <memory>
#include <boost/function.hpp>

class Window;

namespace sd { namespace toolpanel {

class ControlContainer;

/** This wrapper adds a title bar to a control.  Both title bar and
    control are child windows.
*/
class TitledControl
    : public ::Window,
      public TreeNode
{
public:
    typedef ::boost::function1<void, TitledControl&> ClickHandler;

    /** Create a new descriptor for the given control.
        @param pParent
            The parent window of the new descriptor.
        @param pControl
            The control that is shown when being in the expanded
            state.
        @param rTitle
            String that is shown as title in the title area above the
            control.
        @param rClickHandler
            The typical action of the click handler is to expand the control.
        @param eType
            Type of the title bar.  This specifies how the title bar
            will be formated.  For more information see TitleBar.

    */
    TitledControl (
        TreeNode* pParent,
        ::std::auto_ptr<TreeNode> pControl,
        const String& rTitle,
        const ClickHandler& rClickHandler,
        TitleBar::TitleBarType eType);

    virtual ~TitledControl (void);


    virtual Size GetPreferredSize (void);
    virtual sal_Int32 GetPreferredWidth (sal_Int32 nHeight);
    virtual sal_Int32 GetPreferredHeight (sal_Int32 nWidth);
    virtual bool IsResizable (void);
    virtual ::Window* GetWindow (void);

    virtual void Resize (void);
    virtual void GetFocus (void);
    virtual void KeyInput (const KeyEvent& rEvent);

    //    void Select (bool bExpansionState);

    TitleBar* GetTitleBar (void);
    /** Return the control child.  When a control factory has been given and
        the control has not yet been created and the given flag is <true/>
        then the control is created.
    */
    TreeNode* GetControl (void);
    const TreeNode* GetConstControl () const;

    const String& GetTitle (void) const;

    /** Expand the control without informing its container.  This
        method usually is called by the container as a result of a
        higher level expand command.  You may want to use
        ExpandViaContainer() instead.
        @param bExpanded
            When <true/> then the control is expanded, otherwise it is
            collapsed.
    */
    virtual bool Expand (bool bExpanded = true);

    /** Return whether the control is currently expanded (<true/>) or
        not (<false/>).
    */
    virtual bool IsExpanded (void) const;

    /** Returns the value of the control.
    */
    virtual bool IsExpandable (void) const;

    virtual void SetEnabledState(bool bFlag);

    virtual bool IsShowing (void) const;
    virtual void Show (bool bVisible);

    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible > CreateAccessibleObject (
            const ::com::sun::star::uno::Reference<
            ::com::sun::star::accessibility::XAccessible>& rxParent);

    using Window::GetWindow;
    using Window::Show;

private:
    String msTitle;
    bool mbVisible;
    void* mpUserData;
    ::std::auto_ptr<ClickHandler> mpClickHandler;

    /// Do not use! Assignment operator is not supported.
    const TitledControl& operator= (
        const TitledControl& aDescriptor);

    void UpdateStates (void);

    DECL_LINK(WindowEventListener, VclSimpleEvent*);
};




/** This standard implementation of the ClickHandler expands, or toggles the
    expansion state, of the control, whose title was clicked.
*/
class TitledControlStandardClickHandler
{
public:
    /** Create a new instance of this class.
        @param rControlContainer
            The container of which the TitledControl is part of.
        @param eExpansionState
            This specifies whether to always expand the titled control or to
            toggle its expansion state.
    */
    TitledControlStandardClickHandler (
        ControlContainer& rControlContainer,
        ControlContainer::ExpansionState eExpansionState);
    void operator () (TitledControl& rTitledControl);
private:
    ControlContainer& mrControlContainer;
    ControlContainer::ExpansionState meExpansionState;
};

} } // end of namespace ::sd::toolpanel

#endif

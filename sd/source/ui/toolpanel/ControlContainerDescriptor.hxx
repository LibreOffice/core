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



#ifndef SD_TASKPANE_CONTROL_CONTAINER_DESCRIPTOR_HXX
#define SD_TASKPANE_CONTROL_CONTAINER_DESCRIPTOR_HXX

#include "taskpane/ILayoutableWindow.hxx"
#include "taskpane/TitleBar.hxx"
#include <tools/string.hxx>
#include <tools/gen.hxx>
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

    using Window::GetWindow;
    using sd::toolpanel::ILayoutableWindow::GetPreferredWidth;
    using sd::toolpanel::ILayoutableWindow::GetPreferredHeight;

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

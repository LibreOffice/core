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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#include "TestPanel.hxx"
#include "taskpane/ScrollPanel.hxx"
#include "taskpane/TaskPaneControlFactory.hxx"

#include <vcl/lstbox.hxx>
#include <vcl/button.hxx>

namespace sd { namespace toolpanel {

#ifdef SHOW_TEST_PANEL
/** This factory class is used to create instances of TestPanel.  It can be
    extended so that its constructor stores arguments that later are passed
    to new TestPanel objects.
*/
class TestPanelFactory
    : public ControlFactory
{
protected:
    virtual TreeNode* InternalCreateControl( ::Window& i_rParent )
    {
        return new TestPanel (i_rParent);
    }
};


class Wrapper
    : public TreeNode
{
public:
    Wrapper (
        TreeNode* pParent,
        Size aPreferredSize,
        ::Window* pWrappedControl,
        bool bIsResizable)
        : TreeNode (pParent),
          maPreferredSize(aPreferredSize),
          mpWrappedControl(pWrappedControl),
          mbIsResizable(bIsResizable)
    {
        mpWrappedControl->Show();
    }
    virtual ~Wrapper (void)
    {
        delete mpWrappedControl;
    }

    virtual Size GetPreferredSize (void)
    {
        return maPreferredSize;
    }
    virtual sal_Int32 GetPreferredWidth (sal_Int32 )
    {
        return maPreferredSize.Width();
    }
    virtual sal_Int32 GetPreferredHeight (sal_Int32 )
    {
        return maPreferredSize.Height();
    }
    virtual ::Window* GetWindow (void)
    {
        return mpWrappedControl;
    }
    virtual bool IsResizable (void)
    {
        return mbIsResizable;
    }
    virtual bool IsExpandable (void) const
    {
        return false;
    }
    virtual bool IsExpanded (void) const
    {
        return true;
    }

private:
    Size maPreferredSize;
    ::Window* mpWrappedControl;
    bool mbIsResizable;
};


TestPanel::TestPanel (::Window& i_rParent)
    : SubToolPanel (i_rParent)
{
    // Create a scrollable panel with two list boxes.
    ScrollPanel* pScrollPanel = new ScrollPanel (this);

    ListBox* pBox = new ListBox (pScrollPanel->GetWindow());
    int i;
    for (i=1; i<=20; i++)
    {
        XubString aString (XubString::CreateFromAscii("Text "));
        aString.Append (XubString::CreateFromInt32(i));
        aString.Append (XubString::CreateFromAscii("/20"));
        pBox->InsertEntry (aString);
    }
    pScrollPanel->AddControl (
        ::std::auto_ptr<TreeNode>(new Wrapper (
            pScrollPanel, Size (200,300), pBox, true)),
        String::CreateFromAscii ("First ListBox"),
        "");

    pBox = new ListBox (pScrollPanel->GetWindow());
    for (i=1; i<=20; i++)
    {
        XubString aString (XubString::CreateFromAscii("More Text "));
        aString.Append (XubString::CreateFromInt32(i));
        aString.Append (XubString::CreateFromAscii("/20"));
        pBox->InsertEntry (aString);
    }
    pScrollPanel->AddControl (
        ::std::auto_ptr<TreeNode>(new Wrapper (
            pScrollPanel, Size (200,300), pBox, true)),
        String::CreateFromAscii ("Second ListBox"),
        "");

    AddControl (::std::auto_ptr<TreeNode>(pScrollPanel));

    // Add a fixed size button.
    Button* pButton = new OKButton (this);
    AddControl (
        ::std::auto_ptr<TreeNode>(new Wrapper (
            this, Size (100,30), pButton, false)),
        String::CreateFromAscii ("Button Area"),
        "");
}





TestPanel::~TestPanel (void)
{
}

std::auto_ptr<ControlFactory> TestPanel::CreateControlFactory (void)
{
    return std::auto_ptr<ControlFactory>(new TestPanelFactory());
}
#endif


} } // end of namespace ::sd::toolpanel

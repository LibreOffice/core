/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: TestPanel.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-12 18:44:19 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#include "TestPanel.hxx"
#include "taskpane/ScrollPanel.hxx"
#include "taskpane/TaskPaneControlFactory.hxx"

#include <vcl/lstbox.hxx>
#include <vcl/button.hxx>

namespace sd { namespace toolpanel {

/** This factory class is used to create instances of TestPanel.  It can be
    extended so that its constructor stores arguments that later are passed
    to new TestPanel objects.
*/
class TestPanelFactory
    : public ControlFactory
{
protected:
    virtual TreeNode* InternalCreateControl (TreeNode* pTreeNode)
    {
        return new TestPanel (pTreeNode);
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


TestPanel::TestPanel (TreeNode* pParent)
    : SubToolPanel (pParent)
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
        0);

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
        0);

    AddControl (::std::auto_ptr<TreeNode>(pScrollPanel));

    // Add a fixed size button.
    Button* pButton = new OKButton (this);
    AddControl (
        ::std::auto_ptr<TreeNode>(new Wrapper (
            this, Size (100,30), pButton, false)),
        String::CreateFromAscii ("Button Area"),
        0);
}





TestPanel::~TestPanel (void)
{
}




std::auto_ptr<ControlFactory> TestPanel::CreateControlFactory (void)
{
    return std::auto_ptr<ControlFactory>(new TestPanelFactory());
}


} } // end of namespace ::sd::toolpanel

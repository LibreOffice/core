/*************************************************************************
 *
 *  $RCSfile: TestPanel.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 14:38:46 $
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

#include "TestPanel.hxx"
#include "ScrollPanel.hxx"

#include <vcl/lstbox.hxx>
#include <vcl/button.hxx>

namespace sd { namespace toolpanel {

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
    virtual sal_Int32 GetPreferredWidth (sal_Int32 nHeigh)
    {
        return maPreferredSize.Width();
    }
    virtual sal_Int32 GetPreferredHeight (sal_Int32 nWidth)
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
        String::CreateFromAscii ("First ListBox"));

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
        String::CreateFromAscii ("Second ListBox"));

    AddControl (::std::auto_ptr<TreeNode>(pScrollPanel));

    // Add a fixed size button.
    Button* pButton = new OKButton (this);
    AddControl (
        ::std::auto_ptr<TreeNode>(new Wrapper (
            this, Size (100,30), pButton, false)),
        String::CreateFromAscii ("Button Area"));
}





TestPanel::~TestPanel (void)
{
}

} } // end of namespace ::sd::toolpanel

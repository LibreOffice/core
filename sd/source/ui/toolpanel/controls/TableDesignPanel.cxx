/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: TableDesignPanel.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 11:49:11 $
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

#include "TableDesignPanel.hxx"

#include "taskpane/TaskPaneControlFactory.hxx"

#include "strings.hrc"
#include "sdresid.hxx"

namespace sd
{

    class ViewShellBase;
    extern ::Window * createTableDesignPanel( ::Window* pParent, ViewShellBase& rBase );

namespace toolpanel { namespace controls {


TableDesignPanel::TableDesignPanel(TreeNode* pParent, ViewShellBase& rBase)
    : SubToolPanel (pParent)
{
    mpWrappedControl = createTableDesignPanel( pParent->GetWindow(), rBase );
    mpWrappedControl->Show();
}

TableDesignPanel::~TableDesignPanel()
{
    delete mpWrappedControl;
}

std::auto_ptr<ControlFactory> TableDesignPanel::CreateControlFactory (ViewShellBase& rBase)
{
    return std::auto_ptr<ControlFactory>(
        new ControlFactoryWithArgs1<TableDesignPanel,ViewShellBase>(rBase));
}

Size TableDesignPanel::GetPreferredSize()
{
    return maPreferredSize;
}
sal_Int32 TableDesignPanel::GetPreferredWidth(sal_Int32 )
{
    return maPreferredSize.Width();
}
sal_Int32 TableDesignPanel::GetPreferredHeight(sal_Int32 )
{
    return maPreferredSize.Height();
}
::Window* TableDesignPanel::GetWindow()
{
    return mpWrappedControl;
}
bool TableDesignPanel::IsResizable()
{
    return true;
}
bool TableDesignPanel::IsExpandable() const
{
    return true;
}

::com::sun::star::uno::Reference<
    ::com::sun::star::accessibility::XAccessible> TableDesignPanel::CreateAccessibleObject (
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible>& )
{
    if (GetWindow() != NULL)
        return GetWindow()->GetAccessible();
    else
        return NULL;
}

} } } // end of namespace ::sd::toolpanel::controls

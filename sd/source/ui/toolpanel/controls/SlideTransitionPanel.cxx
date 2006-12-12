/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SlideTransitionPanel.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-12 18:52:01 $
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
#include "SlideTransitionPanel.hxx"

#include "taskpane/TaskPaneControlFactory.hxx"

#include "strings.hrc"
#include "sdresid.hxx"

#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif

namespace sd
{

    class ViewShellBase;
    extern ::Window * createSlideTransitionPanel( ::Window* pParent, ViewShellBase& rBase );

namespace toolpanel { namespace controls {



SlideTransitionPanel::SlideTransitionPanel(TreeNode* pParent, ViewShellBase& rBase)
    : SubToolPanel (pParent),
      maPreferredSize( 100, 200 )
{
    mpWrappedControl = createSlideTransitionPanel( pParent->GetWindow(), rBase );
    mpWrappedControl->Show();
}

SlideTransitionPanel::~SlideTransitionPanel()
{
    delete mpWrappedControl;
}

std::auto_ptr<ControlFactory> SlideTransitionPanel::CreateControlFactory (ViewShellBase& rBase)
{
    return std::auto_ptr<ControlFactory>(
        new ControlFactoryWithArgs1<SlideTransitionPanel,ViewShellBase>(rBase));
}

Size SlideTransitionPanel::GetPreferredSize()
{
    return maPreferredSize;
}
sal_Int32 SlideTransitionPanel::GetPreferredWidth(sal_Int32 )
{
    return maPreferredSize.Width();
}
sal_Int32 SlideTransitionPanel::GetPreferredHeight(sal_Int32 )
{
    return maPreferredSize.Height();
}
::Window* SlideTransitionPanel::GetWindow()
{
    return mpWrappedControl;
}
bool SlideTransitionPanel::IsResizable()
{
    return true;
}
bool SlideTransitionPanel::IsExpandable() const
{
    return true;
}




::com::sun::star::uno::Reference<
    ::com::sun::star::accessibility::XAccessible> SlideTransitionPanel::CreateAccessibleObject (
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible>& )
{
    if (GetWindow() != NULL)
        return GetWindow()->GetAccessible();
    else
        return NULL;
}


} } } // end of namespace ::sd::toolpanel::controls

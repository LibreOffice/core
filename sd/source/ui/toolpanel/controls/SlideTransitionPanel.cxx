/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: SlideTransitionPanel.cxx,v $
 * $Revision: 1.8 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"
#include "SlideTransitionPanel.hxx"

#include "taskpane/TaskPaneControlFactory.hxx"

#include "strings.hrc"
#include "sdresid.hxx"
#include <com/sun/star/frame/XModel.hpp>

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

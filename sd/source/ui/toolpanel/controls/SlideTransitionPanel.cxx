/*************************************************************************
 *
 *  $RCSfile: SlideTransitionPanel.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2005-07-14 10:27:06 $
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
sal_Int32 SlideTransitionPanel::GetPreferredWidth(sal_Int32 nHeigh)
{
    return maPreferredSize.Width();
}
sal_Int32 SlideTransitionPanel::GetPreferredHeight(sal_Int32 nWidth)
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
        ::com::sun::star::accessibility::XAccessible>& rxParent)
{
    if (GetWindow() != NULL)
        return GetWindow()->GetAccessible();
    else
        return NULL;
}


} } } // end of namespace ::sd::toolpanel::controls

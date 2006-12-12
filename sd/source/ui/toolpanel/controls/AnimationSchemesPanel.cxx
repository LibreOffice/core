/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: AnimationSchemesPanel.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-12 18:45:56 $
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
#include "AnimationSchemesPanel.hxx"

#include "strings.hrc"
#include "sdresid.hxx"

#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif

namespace sd
{

    class ViewShellBase;
    extern ::Window * createAnimationSchemesPanel( ::Window* pParent, ViewShellBase& rBase );

namespace toolpanel { namespace controls {


AnimationSchemesPanel::AnimationSchemesPanel(TreeNode* pParent, ViewShellBase& rBase)
    : SubToolPanel (pParent),
      maPreferredSize( 100, 200 )
{
    mpWrappedControl = createAnimationSchemesPanel( pParent->GetWindow(), rBase );
    mpWrappedControl->Show();
}

AnimationSchemesPanel::~AnimationSchemesPanel()
{
    delete mpWrappedControl;
}

Size AnimationSchemesPanel::GetPreferredSize()
{
    return maPreferredSize;
}
sal_Int32 AnimationSchemesPanel::GetPreferredWidth(sal_Int32 )
{
    return maPreferredSize.Width();
}
sal_Int32 AnimationSchemesPanel::GetPreferredHeight(sal_Int32 )
{
    return maPreferredSize.Height();
}
::Window* AnimationSchemesPanel::GetWindow()
{
    return mpWrappedControl;
}
bool AnimationSchemesPanel::IsResizable()
{
    return true;
}
bool AnimationSchemesPanel::IsExpandable() const
{
    return true;
}


} } } // end of namespace ::sd::toolpanel::controls

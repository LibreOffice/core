/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: AnimationSchemesPanel.cxx,v $
 * $Revision: 1.6 $
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
#include "AnimationSchemesPanel.hxx"

#include "strings.hrc"
#include "sdresid.hxx"
#include <com/sun/star/frame/XModel.hpp>

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
// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"
#include "AnimationSchemesPanel.hxx"

#include "strings.hrc"
#include "sdresid.hxx"
#include <com/sun/star/frame/XModel.hpp>

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

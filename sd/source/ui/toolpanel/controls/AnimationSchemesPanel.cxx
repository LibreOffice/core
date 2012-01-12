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

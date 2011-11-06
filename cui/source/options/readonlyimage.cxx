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
#include "precompiled_cui.hxx"

#include <readonlyimage.hxx>
#include <vcl/help.hxx>
#include <dialmgr.hxx>
#include <cuires.hrc>

/*-- 26.02.2004 13:31:04---------------------------------------------------

  -----------------------------------------------------------------------*/
ReadOnlyImage::ReadOnlyImage(Window* pParent, const ResId rResId) :
            FixedImage(pParent, rResId)
{
    sal_Bool bHighContrast = pParent->GetSettings().GetStyleSettings().GetHighContrastMode();
    SetImage( Image(CUI_RES(bHighContrast ? RID_SVXBMP_LOCK_HC : RID_SVXBMP_LOCK )));
}

/*-- 26.02.2004 13:31:04---------------------------------------------------

  -----------------------------------------------------------------------*/
ReadOnlyImage::~ReadOnlyImage()
{
}
/*-- 26.02.2004 13:31:04---------------------------------------------------

  -----------------------------------------------------------------------*/
void ReadOnlyImage::RequestHelp( const HelpEvent& rHEvt )
{
    if( Help::IsBalloonHelpEnabled() || Help::IsQuickHelpEnabled() )
    {
        Rectangle   aLogicPix( LogicToPixel( Rectangle( Point(), GetOutputSize() ) ) );
        Rectangle   aScreenRect( OutputToScreenPixel( aLogicPix.TopLeft() ),
                                     OutputToScreenPixel( aLogicPix.BottomRight() ) );

        String aStr(ReadOnlyImage::GetHelpTip());
        if ( Help::IsBalloonHelpEnabled() )
            Help::ShowBalloon( this, rHEvt.GetMousePosPixel(), aScreenRect,
            aStr );
        else if ( Help::IsQuickHelpEnabled() )
            Help::ShowQuickHelp( this, aScreenRect, aStr );
    }
    else
        Window::RequestHelp( rHEvt );
}

/*-- 26.02.2004 14:20:21---------------------------------------------------

  -----------------------------------------------------------------------*/
const String& ReadOnlyImage::GetHelpTip()
{
     static String  aStr(CUI_RES(RID_SVXSTR_READONLY_CONFIG_TIP));
     return aStr;
}

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



#include "vclxbutton.hxx"

#include <layout/core/helper.hxx>
#include <com/sun/star/awt/ImagePosition.hpp>
#include <vcl/button.hxx>

namespace css = com::sun::star;

namespace layoutimpl
{

VCLXIconButton::VCLXIconButton( Window *p, rtl::OUString aDefaultLabel, char const *pGraphName )
    : VCLXButton()
{
    /* FIXME: before Window is set, setLabel, setProperty->setImage
     * are silent no-ops.  */
    p->SetComponentInterface( this );

    setLabel( aDefaultLabel );
    setProperty( rtl::OUString::createFromAscii( "Graphic" ),
                 css::uno::Any( layoutimpl::loadGraphic( pGraphName ) ) );
    setProperty( rtl::OUString::createFromAscii( "ImagePosition" ),
                 css::uno::Any( css::awt::ImagePosition::LeftCenter ) );
    setProperty( rtl::OUString::createFromAscii( "Align" ),
                 css::uno::Any( (sal_Int16) 1 /* magic - center */ ) );
}

// FIXME: l10n/i18n of Reset & Apply

VCLXOKButton::VCLXOKButton( Window *p )
    : VCLXIconButton( p, Button::GetStandardText( BUTTON_OK ),
                         "res/commandimagelist/sc_ok.png" )
{
}

VCLXCancelButton::VCLXCancelButton( Window *p )
    : VCLXIconButton( p, Button::GetStandardText( BUTTON_CANCEL ),
//    : VCLXIconButton( xButton, rtl::OUString::createFromAscii( "~Cancel " ),
                         "res/commandimagelist/sc_cancel.png" )
{
}

VCLXYesButton::VCLXYesButton( Window *p )
    : VCLXIconButton( p, Button::GetStandardText( BUTTON_YES ),
                  "res/commandimagelist/sc_yes.png" )
{
}

VCLXNoButton::VCLXNoButton( Window *p )
    : VCLXIconButton( p, Button::GetStandardText( BUTTON_NO ),
                  "res/commandimagelist/sc_no.png" )
{
}

VCLXRetryButton::VCLXRetryButton( Window *p )
    : VCLXIconButton( p, Button::GetStandardText( BUTTON_RETRY ),
                  "res/commandimagelist/sc_retry.png" )
{
}

VCLXIgnoreButton::VCLXIgnoreButton( Window *p )
    : VCLXIconButton( p, Button::GetStandardText( BUTTON_IGNORE ),
                  "res/commandimagelist/sc_ignore.png" )
{
}

VCLXResetButton::VCLXResetButton( Window *p )
    : VCLXIconButton( p, rtl::OUString::createFromAscii( "~Reset " ),
                  "res/commandimagelist/sc_reset.png" )
{
}

VCLXApplyButton::VCLXApplyButton( Window *p )
    : VCLXIconButton( p, rtl::OUString::createFromAscii( "Apply" ),
                  "res/commandimagelist/sc_apply.png" )
{
}

VCLXHelpButton::VCLXHelpButton( Window *p )
    : VCLXIconButton( p, Button::GetStandardText( BUTTON_HELP ),
                  "res/commandimagelist/sc_help.png" )
{
}

VCLXMoreButton::VCLXMoreButton( Window *p )
    : VCLXIconButton( p, Button::GetStandardText( BUTTON_MORE ),
//    : VCLXIconButton( p, rtl::OUString::createFromAscii( "More " ),
                  "res/commandimagelist/sc_more.png" )
{
}

VCLXAdvancedButton::VCLXAdvancedButton( Window *p )
//    : VCLXIconButton( p, Button::GetStandardText( BUTTON_ADVANCED ),
    : VCLXIconButton( p, rtl::OUString::createFromAscii( "Advanced " ),
                  "res/commandimagelist/sc_advanced.png" )
{
}

} // namespace layoutimpl

/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
    setProperty( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Graphic")),
                 css::uno::Any( layoutimpl::loadGraphic( pGraphName ) ) );
    setProperty( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ImagePosition")),
                 css::uno::Any( css::awt::ImagePosition::LeftCenter ) );
    setProperty( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Align")),
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
//    : VCLXIconButton( xButton, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("~Cancel ")),
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
    : VCLXIconButton( p, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("~Reset ")),
                  "res/commandimagelist/sc_reset.png" )
{
}

VCLXApplyButton::VCLXApplyButton( Window *p )
    : VCLXIconButton( p, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Apply")),
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
//    : VCLXIconButton( p, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("More ")),
                  "res/commandimagelist/sc_more.png" )
{
}

VCLXAdvancedButton::VCLXAdvancedButton( Window *p )
//    : VCLXIconButton( p, Button::GetStandardText( BUTTON_ADVANCED ),
    : VCLXIconButton( p, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Advanced ")),
                  "res/commandimagelist/sc_advanced.png" )
{
}

} // namespace layoutimpl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

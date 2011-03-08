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

#define _LAYOUT_PLUGIN_CXX

#include "plugin.hxx"

#include <com/sun/star/awt/XDialog2.hpp>
#include <dialcontrol.hxx>
#include <toolkit/awt/vclxwindow.hxx>

#include <layout/layout-pre.hxx>

using namespace com::sun::star;

#define LAYOUT_RES(x) #x

#undef ModalDialog
#define ModalDialog( parent, id ) Dialog( parent, "plugin.xml", id )

PluginDialog::PluginDialog( Window* pParent )
    : ModalDialog( pParent, LAYOUT_RES( RID_DLG_PLUGIN ) )
    , aHeaderImage( this, LAYOUT_RES( FI_HEADER ) )
    , aHeaderText( this, LAYOUT_RES( FT_HEADER ) )
    , aHeaderLine( this, LAYOUT_RES( FL_HEADER ) )
    , aPlugin( this, LAYOUT_RES( PL_DIAL ),
// FIXME: width=, height=, are not recognized as properties.
//               new svx::DialControl( LAYOUT_DIALOG_PARENT, Size( 0, 0 ), 0 ) )
               new svx::DialControl( LAYOUT_DIALOG_PARENT, Size( 80, 80 ), 0 ) )
      //, aDialControl( static_cast<svx::DialControl&> ( aPlugin.GetPlugin() ) )
    , aDialControl( static_cast<svx::DialControl&> ( *aPlugin.mpPlugin ) )
    , aOKBtn( this, LAYOUT_RES( BTN_OK ) )
    , aCancelBtn( this, LAYOUT_RES( BTN_CANCEL ) )
    , aHelpBtn( this, LAYOUT_RES( BTN_HELP ) )
{
    aDialControl.SetRotation( 425 );
}

PluginDialog::~PluginDialog()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

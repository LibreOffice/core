/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile$
 *
 *  $Revision$
 *
 *  last change: $Author$ $Date$
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

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

/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <svl/aeitem.hxx>

#include <svx/dialmgr.hxx>
#include <svx/dialogs.hrc>

#include "svx/tbxalign.hxx"
#include "svx/tbxdraw.hxx"
#include <tools/shl.hxx>
#include <sfx2/imagemgr.hxx>
#include <vcl/svapp.hxx>
#include <osl/mutex.hxx>

#include <sfx2/app.hxx>
#include <vcl/toolbox.hxx>

SFX_IMPL_TOOLBOX_CONTROL(SvxTbxCtlAlign, SfxAllEnumItem);

/*************************************************************************
|*
|* Klasse fuer SwToolbox
|*
\************************************************************************/

SvxTbxCtlAlign::SvxTbxCtlAlign( sal_uInt16 nSlotId, sal_uInt16 nId, ToolBox& rTbx ) :
    SfxToolBoxControl( nSlotId, nId, rTbx )
    ,   m_aSubTbName( "alignmentbar" )
    ,   m_aSubTbResName( "private:resource/toolbar/alignmentbar" )
{
    rTbx.SetItemBits( nId, TIB_DROPDOWN | rTbx.GetItemBits( nId ) );
    rTbx.Invalidate();

    m_aCommand = m_aCommandURL;
}

/*************************************************************************
|*
|* Wenn man ein PopupWindow erzeugen will
|*
\************************************************************************/

SfxPopupWindowType SvxTbxCtlAlign::GetPopupWindowType() const
{
    return(SFX_POPUPWINDOW_ONCLICK);
}

/*************************************************************************
|*
|* Hier wird das Fenster erzeugt
|* Lage der Toolbox mit GetToolBox() abfragbar
|* rItemRect sind die Screen-Koordinaten
|*
\************************************************************************/

SfxPopupWindow* SvxTbxCtlAlign::CreatePopupWindow()
{
    SolarMutexGuard aGuard;
    if ( GetSlotId() == SID_OBJECT_ALIGN )
        createAndPositionSubToolBar( m_aSubTbResName );
    return NULL;
}

//========================================================================
// XSubToolbarController
//========================================================================

::sal_Bool SAL_CALL SvxTbxCtlAlign::opensSubToolbar() throw (::com::sun::star::uno::RuntimeException)
{
    // We control a sub-toolbar therefor, we have to return true.
    return sal_True;
}

OUString SAL_CALL SvxTbxCtlAlign::getSubToolbarName() throw (::com::sun::star::uno::RuntimeException)
{
    // Provide the controlled sub-toolbar name, so we are notified whenever
    // this toolbar executes a function.
    SolarMutexGuard aGuard;
    return m_aSubTbName;
}

void SAL_CALL SvxTbxCtlAlign::functionSelected( const OUString& aCommand ) throw (::com::sun::star::uno::RuntimeException)
{
    // remember the new command
    m_aCommand = aCommand;

    // Our sub-toolbar wants to execute a function.
    // We have to change the image of our toolbar button to reflect the new function.
    updateImage();
}

void SAL_CALL SvxTbxCtlAlign::updateImage() throw (::com::sun::star::uno::RuntimeException)
{
    // We should update the button image of our parent (toolbar). Use the stored
    // command to set the correct current image.
    SolarMutexGuard aGuard;
    if ( !m_aCommand.isEmpty() )
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > xFrame( getFrameInterface());
        Image aImage = GetImage( xFrame, m_aCommand, hasBigImages() );
        if ( !!aImage )
            GetToolBox().SetItemImage( GetId(), aImage );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

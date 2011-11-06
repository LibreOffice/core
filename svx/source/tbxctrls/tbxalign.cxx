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
#include "precompiled_svx.hxx"
#include <svl/aeitem.hxx>

#include <svx/dialmgr.hxx>
#include <svx/dialogs.hrc>

#include "svx/tbxalign.hxx"
#include "svx/tbxdraw.hxx"
#include "tbxdraw.hrc"
#include <tools/shl.hxx>
#ifndef _SFX_IMAGEMGR_HXX
#include <sfx2/imagemgr.hxx>
#endif
#include <vcl/svapp.hxx>
#include <vos/mutex.hxx>

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
    ,   m_aSubTbName( RTL_CONSTASCII_USTRINGPARAM( "alignmentbar" ))
    ,   m_aSubTbResName( RTL_CONSTASCII_USTRINGPARAM( "private:resource/toolbar/alignmentbar" ))
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
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
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

::rtl::OUString SAL_CALL SvxTbxCtlAlign::getSubToolbarName() throw (::com::sun::star::uno::RuntimeException)
{
    // Provide the controlled sub-toolbar name, so we are notified whenever
    // this toolbar executes a function.
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    return m_aSubTbName;
}

void SAL_CALL SvxTbxCtlAlign::functionSelected( const ::rtl::OUString& aCommand ) throw (::com::sun::star::uno::RuntimeException)
{
    // Our sub-toolbar wants to executes a function. We have to change
    // the image of our toolbar button to reflect the new function.
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    if ( !m_bDisposed )
    {
        if ( aCommand.getLength() > 0 )
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > xFrame( getFrameInterface());
            Image aImage = GetImage( xFrame, aCommand, hasBigImages(), isHighContrast() );
            if ( !!aImage )
                GetToolBox().SetItemImage( GetId(), aImage );
        }
    }
}

void SAL_CALL SvxTbxCtlAlign::updateImage() throw (::com::sun::star::uno::RuntimeException)
{
    // We should update the button image of our parent (toolbar). Use the stored
    // command to set the correct current image.
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    if ( m_aCommand.getLength() > 0 )
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > xFrame( getFrameInterface());
        Image aImage = GetImage( xFrame, m_aCommand, hasBigImages(), isHighContrast() );
        if ( !!aImage )
            GetToolBox().SetItemImage( GetId(), aImage );
    }
}

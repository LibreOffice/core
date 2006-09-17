/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tbxalign.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 06:06:50 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#ifndef _AEITEM_HXX
#include <svtools/aeitem.hxx>
#endif

#include "dialmgr.hxx"
#include "dialogs.hrc"

#include "tbxalign.hxx"
#include "tbxdraw.hxx"
#include "tbxdraw.hrc"

#ifndef _SHL_HXX //autogen
#include <tools/shl.hxx>
#endif
#ifndef _SFX_IMAGEMGR_HXX
#include <sfx2/imagemgr.hxx>
#endif
#include <vcl/svapp.hxx>
#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif

#include <sfx2/app.hxx>
#include <vcl/toolbox.hxx>

SFX_IMPL_TOOLBOX_CONTROL(SvxTbxCtlAlign, SfxAllEnumItem);

/*************************************************************************
|*
|* Klasse fuer SwToolbox
|*
\************************************************************************/

SvxTbxCtlAlign::SvxTbxCtlAlign( USHORT nSlotId, USHORT nId, ToolBox& rTbx ) :
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

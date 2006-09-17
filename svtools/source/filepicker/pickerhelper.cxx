/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: pickerhelper.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 14:49:37 $
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
#include "precompiled_svtools.hxx"

#include "pickerhelper.hxx"

#ifndef _RTL_USTRING_HXX_
#include "rtl/ustring.hxx"
#endif

#ifndef _COM_SUN_STAR_UI_DIALOGS_XFILEPICKER_HPP_
#include "com/sun/star/ui/dialogs/XFilePicker.hpp"
#endif
#ifndef _COM_SUN_STAR_UI_DIALOGS_XFOLDERPICKER_HPP_
#include "com/sun/star/ui/dialogs/XFolderPicker.hpp"
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include "com/sun/star/beans/XPropertySet.hpp"
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSETINFO_HPP_
#include "com/sun/star/beans/XPropertySetInfo.hpp"
#endif

#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include "com/sun/star/uno/Any.hxx"
#endif

#ifndef _DEBUG_HXX
#include "tools/debug.hxx"
#endif

namespace css = com::sun::star;

using css::uno::Reference;
using rtl::OUString;

namespace svt
{
    void SetDialogHelpId(
        Reference < css::ui::dialogs::XFilePicker > _mxFileDlg, sal_Int32 _nHelpId )
    {
        try
        {
            // does the dialog haver a help URL property?
            Reference< css::beans::XPropertySet >       xDialogProps( _mxFileDlg, css::uno::UNO_QUERY );
            Reference< css::beans::XPropertySetInfo >   xInfo;
            if( xDialogProps.is() )
                xInfo = xDialogProps->getPropertySetInfo( );

            const OUString          sHelpURLPropertyName( RTL_CONSTASCII_USTRINGPARAM( "HelpURL" ) );

            if( xInfo.is() && xInfo->hasPropertyByName( sHelpURLPropertyName ) )
            {   // yep
                OUString                sId( RTL_CONSTASCII_USTRINGPARAM( "HID:" ) );
                sId += OUString::valueOf( _nHelpId );
                xDialogProps->setPropertyValue( sHelpURLPropertyName, css::uno::makeAny( sId ) );
            }
        }
        catch( const css::uno::Exception& )
        {
            DBG_ERROR( "svt::SetDialogHelpId(): caught an exception while setting the help id!" );
        }
    }

    void SetDialogHelpId(
        Reference< css::ui::dialogs::XFolderPicker > _mxFileDlg, sal_Int32 _nHelpId )
    {
        try
        {
            // does the dialog haver a help URL property?
            Reference< css::beans::XPropertySet >       xDialogProps( _mxFileDlg, css::uno::UNO_QUERY );
            Reference< css::beans::XPropertySetInfo >   xInfo;
            if( xDialogProps.is() )
                xInfo = xDialogProps->getPropertySetInfo( );

            const OUString          sHelpURLPropertyName( RTL_CONSTASCII_USTRINGPARAM( "HelpURL" ) );

            if( xInfo.is() && xInfo->hasPropertyByName( sHelpURLPropertyName ) )
            {   // yep
                OUString                sId( RTL_CONSTASCII_USTRINGPARAM( "HID:" ) );
                sId += OUString::valueOf( _nHelpId );
                xDialogProps->setPropertyValue( sHelpURLPropertyName, css::uno::makeAny( sId ) );
            }
        }
        catch( const css::uno::Exception& )
        {
            DBG_ERROR( "svt::SetDialogHelpId(): caught an exception while setting the help id!" );
        }
    }
}


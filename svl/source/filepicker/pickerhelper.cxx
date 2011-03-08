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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svl.hxx"

#include "pickerhelper.hxx"
#include "rtl/ustring.hxx"
#include "com/sun/star/ui/dialogs/XFilePicker.hpp"
#include "com/sun/star/ui/dialogs/XFolderPicker.hpp"
#include "com/sun/star/beans/XPropertySet.hpp"
#include "com/sun/star/beans/XPropertySetInfo.hpp"
#include "com/sun/star/uno/Any.hxx"
#include "tools/debug.hxx"

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
            OSL_FAIL( "svt::SetDialogHelpId(): caught an exception while setting the help id!" );
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
            OSL_FAIL( "svt::SetDialogHelpId(): caught an exception while setting the help id!" );
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

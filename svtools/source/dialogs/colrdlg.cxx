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
#include "precompiled_svtools.hxx"

#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/beans/XPropertyAccess.hpp>
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>

#include <comphelper/processfactory.hxx>

#include <toolkit/helper/vclunohelper.hxx>

#include <svtools/colrdlg.hxx>

using rtl::OUString;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::ui::dialogs;

// ---------------
// - ColorDialog -
// ---------------

SvColorDialog::SvColorDialog( Window* pWindow )
: mpParent( pWindow )
, meMode( svtools::ColorPickerMode_SELECT )
{
}

// -----------------------------------------------------------------------

void SvColorDialog::SetColor( const Color& rColor )
{
    maColor = rColor;
}

// -----------------------------------------------------------------------

const Color& SvColorDialog::GetColor() const
{
    return maColor;
}

// -----------------------------------------------------------------------

void SvColorDialog::SetMode( sal_Int16 eMode )
{
    meMode = eMode;
}

// -----------------------------------------------------------------------

short SvColorDialog::Execute()
{
    short ret = 0;
    try
    {
        const OUString sColor( RTL_CONSTASCII_USTRINGPARAM( "Color" ) );
        Reference< XMultiServiceFactory > xSMGR( ::comphelper::getProcessServiceFactory(), UNO_QUERY_THROW );

        Reference< com::sun::star::awt::XWindow > xParent( VCLUnoHelper::GetInterface( mpParent ) );

        Sequence< Any > args(1);
        args[0] = Any( xParent );

        Reference< XExecutableDialog > xDialog( xSMGR->createInstanceWithArguments(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.cui.ColorPicker")), args), UNO_QUERY_THROW );
        Reference< XPropertyAccess > xPropertyAccess( xDialog, UNO_QUERY_THROW );

        Sequence< PropertyValue > props( 2 );
        props[0].Name = sColor;
        props[0].Value <<= (sal_Int32) maColor.GetColor();
        props[1].Name = OUString( RTL_CONSTASCII_USTRINGPARAM( "Mode" ) );
        props[1].Value <<= (sal_Int16) meMode;

        xPropertyAccess->setPropertyValues( props );

        ret = xDialog->execute();

        if( ret )
        {
            props = xPropertyAccess->getPropertyValues();
            for( sal_Int32 n = 0; n < props.getLength(); n++ )
            {
                if( props[n].Name.equals( sColor ) )
                {
                    sal_Int32 nColor = 0;
                    if( props[n].Value >>= nColor )
                    {
                        maColor.SetColor( nColor );
                    }

                }
            }
        }
    }
    catch(Exception&)
    {
        OSL_ASSERT(false);
    }

    return ret;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

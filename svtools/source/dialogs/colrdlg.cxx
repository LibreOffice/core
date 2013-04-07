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


#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/beans/XPropertyAccess.hpp>
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#include <com/sun/star/cui/ColorPicker.hpp>

#include <comphelper/processfactory.hxx>

#include <toolkit/helper/vclunohelper.hxx>

#include <svtools/colrdlg.hxx>

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

SvColorDialog::~SvColorDialog()
{
}

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
        const OUString sColor( "Color" );
        Reference< XComponentContext > xContext( ::comphelper::getProcessComponentContext() );

        Reference< com::sun::star::awt::XWindow > xParent( VCLUnoHelper::GetInterface( mpParent ) );

        Reference< XExecutableDialog > xDialog = com::sun::star::cui::ColorPicker::createWithParent(xContext, xParent);
        Reference< XPropertyAccess > xPropertyAccess( xDialog, UNO_QUERY_THROW );

        Sequence< PropertyValue > props( 2 );
        props[0].Name = sColor;
        props[0].Value <<= (sal_Int32) maColor.GetColor();
        props[1].Name = OUString( "Mode" );
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

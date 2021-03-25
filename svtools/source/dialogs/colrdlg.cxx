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
#include <com/sun/star/beans/XPropertyAccess.hpp>
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#include <com/sun/star/cui/ColorPicker.hpp>

#include <comphelper/processfactory.hxx>

#include <svtools/colrdlg.hxx>
#include <vcl/weld.hxx>
#include <osl/diagnose.h>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::ui::dialogs;


SvColorDialog::SvColorDialog()
    : meMode(svtools::ColorPickerMode::Select)
{
}

SvColorDialog::~SvColorDialog()
{
}
void SvColorDialog::SetColor( const Color& rColor )
{
    maColor = rColor;
}

void SvColorDialog::SetMode( svtools::ColorPickerMode eMode )
{
    meMode = eMode;
}

short SvColorDialog::Execute(weld::Window* pParent)
{
    short ret = 0;
    try
    {
        static const OUStringLiteral sColor( u"Color" );
        Reference< XComponentContext > xContext( ::comphelper::getProcessComponentContext() );

        Reference<css::awt::XWindow> xParent;
        if (pParent)
            xParent = pParent->GetXWindow();

        Reference< XExecutableDialog > xDialog = css::cui::ColorPicker::createWithParent(xContext, xParent);
        Reference< XPropertyAccess > xPropertyAccess( xDialog, UNO_QUERY_THROW );

        Sequence< PropertyValue > props( 2 );
        props[0].Name = sColor;
        props[0].Value <<= maColor;
        props[1].Name = "Mode";
        props[1].Value <<= static_cast<sal_Int16>(meMode);

        xPropertyAccess->setPropertyValues( props );

        ret = xDialog->execute();

        if( ret )
        {
            props = xPropertyAccess->getPropertyValues();
            for( const auto& rProp : std::as_const(props) )
            {
                if( rProp.Name == sColor )
                {
                    rProp.Value >>= maColor;
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

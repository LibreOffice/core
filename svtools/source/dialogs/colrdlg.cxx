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
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#include <com/sun/star/cui/AsynchronousColorPicker.hpp>
#include <com/sun/star/cui/ColorPicker.hpp>

#include <comphelper/diagnose_ex.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/propertyvalue.hxx>

#include <svtools/colrdlg.hxx>
#include <svtools/dialogclosedlistener.hxx>
#include <vcl/weld.hxx>
#include <osl/diagnose.h>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::ui::dialogs;

constexpr OUString sColor = u"Color"_ustr;

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
        Reference< XComponentContext > xContext( ::comphelper::getProcessComponentContext() );

        Reference<css::awt::XWindow> xParent;
        if (pParent)
            xParent = pParent->GetXWindow();

        Reference< XExecutableDialog > xDialog = css::cui::ColorPicker::createWithParent(xContext, xParent);
        Reference< XPropertyAccess > xPropertyAccess( xDialog, UNO_QUERY_THROW );

        Sequence< PropertyValue > props{
            comphelper::makePropertyValue(sColor, maColor),
            comphelper::makePropertyValue(u"Mode"_ustr, static_cast<sal_Int16>(meMode))
        };

        xPropertyAccess->setPropertyValues( props );

        ret = xDialog->execute();

        if( ret )
        {
            props = xPropertyAccess->getPropertyValues();
            for (const auto& rProp : props)
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

void SvColorDialog::ExecuteAsync(weld::Window* pParent, const std::function<void(sal_Int32)>& func)
{
    m_aResultFunc = func;

    try
    {
        Reference< XComponentContext > xContext( ::comphelper::getProcessComponentContext() );

        Reference<css::awt::XWindow> xParent;
        if (pParent)
            xParent = pParent->GetXWindow();

        mxDialog = css::cui::AsynchronousColorPicker::createWithParent(xContext, xParent);
        Reference< XPropertyAccess > xPropertyAccess( mxDialog, UNO_QUERY_THROW );

        Sequence< PropertyValue > props{
            comphelper::makePropertyValue(sColor, maColor),
            comphelper::makePropertyValue(u"Mode"_ustr, static_cast<sal_Int16>(meMode))
        };

        xPropertyAccess->setPropertyValues( props );

        rtl::Reference< ::svt::DialogClosedListener > pListener = new ::svt::DialogClosedListener();
        pListener->SetDialogClosedLink( LINK( this, SvColorDialog, DialogClosedHdl ) );

        mxDialog->startExecuteModal( pListener );
    }
    catch(Exception&)
    {
        TOOLS_WARN_EXCEPTION("svtools.dialogs", "SvColorDialog::ExecuteAsync");
    }
}

IMPL_LINK( SvColorDialog, DialogClosedHdl, css::ui::dialogs::DialogClosedEvent*, pEvent, void )
{
    sal_Int32 nResult = 0;
    sal_Int16 nDialogRet = pEvent->DialogResult;
    if( nDialogRet == ExecutableDialogResults::OK )
    {
        nResult = RET_OK;

        Reference< XPropertyAccess > xPropertyAccess( mxDialog, UNO_QUERY_THROW );
        Sequence< PropertyValue > props = xPropertyAccess->getPropertyValues();

        for (const auto& rProp : props)
        {
            if( rProp.Name == sColor )
            {
                rProp.Value >>= maColor;
            }
        }
    }

    m_aResultFunc(nResult);
    mxDialog.clear();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

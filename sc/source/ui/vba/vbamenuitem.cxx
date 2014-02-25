/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "vbamenuitem.hxx"

using namespace com::sun::star;
using namespace ooo::vba;

ScVbaMenuItem::ScVbaMenuItem( const uno::Reference< ov::XHelperInterface > xParent, const uno::Reference< uno::XComponentContext > xContext, const uno::Reference< XCommandBarControl >& xCommandBarControl ) throw( uno::RuntimeException ) : MenuItem_BASE( xParent, xContext ), m_xCommandBarControl( xCommandBarControl )
{
}

OUString SAL_CALL
ScVbaMenuItem::getCaption() throw ( uno::RuntimeException, std::exception )
{
    return m_xCommandBarControl->getCaption();
}

void SAL_CALL
ScVbaMenuItem::setCaption( const OUString& _caption ) throw (uno::RuntimeException, std::exception)
{
    m_xCommandBarControl->setCaption( _caption );
}

OUString SAL_CALL
ScVbaMenuItem::getOnAction() throw ( uno::RuntimeException, std::exception )
{
    return m_xCommandBarControl->getOnAction();
}

void SAL_CALL
ScVbaMenuItem::setOnAction( const OUString& _onaction ) throw (uno::RuntimeException, std::exception)
{
    m_xCommandBarControl->setOnAction( _onaction );
}

void SAL_CALL
ScVbaMenuItem::Delete( ) throw (script::BasicErrorException, uno::RuntimeException, std::exception)
{
    m_xCommandBarControl->Delete();
}

OUString
ScVbaMenuItem::getServiceImplName()
{
    return OUString("ScVbaMenuItem");
}

uno::Sequence<OUString>
ScVbaMenuItem::getServiceNames()
{
    static uno::Sequence< OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = "ooo.vba.excel.MenuItem";
    }
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

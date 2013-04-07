/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Initial Developer of the Original Code is
 *       Novell Inc.
 * Portions created by the Initial Developer are Copyright (C) 2010 the
 * Initial Developer. All Rights Reserved.
 *
 * Contributor(s): Pei Feng Lin <pflin@novell.com>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */
#include "vbamenuitem.hxx"

using namespace com::sun::star;
using namespace ooo::vba;

ScVbaMenuItem::ScVbaMenuItem( const uno::Reference< ov::XHelperInterface > xParent, const uno::Reference< uno::XComponentContext > xContext, const uno::Reference< XCommandBarControl >& xCommandBarControl ) throw( uno::RuntimeException ) : MenuItem_BASE( xParent, xContext ), m_xCommandBarControl( xCommandBarControl )
{
}

OUString SAL_CALL
ScVbaMenuItem::getCaption() throw ( uno::RuntimeException )
{
    return m_xCommandBarControl->getCaption();
}

void SAL_CALL
ScVbaMenuItem::setCaption( const OUString& _caption ) throw (uno::RuntimeException)
{
    m_xCommandBarControl->setCaption( _caption );
}

OUString SAL_CALL
ScVbaMenuItem::getOnAction() throw ( uno::RuntimeException )
{
    return m_xCommandBarControl->getOnAction();
}

void SAL_CALL
ScVbaMenuItem::setOnAction( const OUString& _onaction ) throw (uno::RuntimeException)
{
    m_xCommandBarControl->setOnAction( _onaction );
}

void SAL_CALL
ScVbaMenuItem::Delete( ) throw (script::BasicErrorException, uno::RuntimeException)
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
        aServiceNames[ 0 ] = OUString("ooo.vba.excel.MenuItem" );
    }
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

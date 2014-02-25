/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "vbamenu.hxx"
#include "vbamenuitems.hxx"

using namespace com::sun::star;
using namespace ooo::vba;

ScVbaMenu::ScVbaMenu( const uno::Reference< ov::XHelperInterface > xParent, const uno::Reference< uno::XComponentContext > xContext, const uno::Reference< XCommandBarControl >& xCommandBarControl ) throw( uno::RuntimeException ) : Menu_BASE( xParent, xContext ), m_xCommandBarControl( xCommandBarControl )
{
}

OUString SAL_CALL
ScVbaMenu::getCaption() throw ( uno::RuntimeException, std::exception )
{
    return m_xCommandBarControl->getCaption();
}

void SAL_CALL
ScVbaMenu::setCaption( const OUString& _caption ) throw (uno::RuntimeException, std::exception)
{
    m_xCommandBarControl->setCaption( _caption );
}

void SAL_CALL
ScVbaMenu::Delete( ) throw (script::BasicErrorException, uno::RuntimeException, std::exception)
{
    m_xCommandBarControl->Delete();
}

uno::Any SAL_CALL
ScVbaMenu::MenuItems( const uno::Any& aIndex ) throw (script::BasicErrorException, uno::RuntimeException, std::exception)
{
    uno::Reference< XCommandBarControls > xCommandBarControls( m_xCommandBarControl->Controls( uno::Any() ), uno::UNO_QUERY_THROW );
    uno::Reference< excel::XMenuItems > xMenuItems( new ScVbaMenuItems( this, mxContext, xCommandBarControls ) );
    if( aIndex.hasValue() )
    {
        return xMenuItems->Item( aIndex, uno::Any() );
    }
    return uno::makeAny( xMenuItems );
}

OUString
ScVbaMenu::getServiceImplName()
{
    return OUString("ScVbaMenu");
}

uno::Sequence<OUString>
ScVbaMenu::getServiceNames()
{
    static uno::Sequence< OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = "ooo.vba.excel.Menu";
    }
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

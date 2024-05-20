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
#include <ooo/vba/XCommandBarControls.hpp>
#include <utility>

using namespace com::sun::star;
using namespace ooo::vba;

ScVbaMenu::ScVbaMenu( const uno::Reference< ov::XHelperInterface >& rParent, const uno::Reference< uno::XComponentContext >& rContext, uno::Reference< XCommandBarControl > xCommandBarControl ) : Menu_BASE( rParent, rContext ), m_xCommandBarControl(std::move( xCommandBarControl ))
{
}

OUString SAL_CALL
ScVbaMenu::getCaption()
{
    return m_xCommandBarControl->getCaption();
}

void SAL_CALL
ScVbaMenu::setCaption( const OUString& _caption )
{
    m_xCommandBarControl->setCaption( _caption );
}

void SAL_CALL
ScVbaMenu::Delete( )
{
    m_xCommandBarControl->Delete();
}

uno::Any SAL_CALL
ScVbaMenu::MenuItems( const uno::Any& aIndex )
{
    uno::Reference< XCommandBarControls > xCommandBarControls( m_xCommandBarControl->Controls( uno::Any() ), uno::UNO_QUERY_THROW );
    uno::Reference< excel::XMenuItems > xMenuItems( new ScVbaMenuItems( this, mxContext, xCommandBarControls ) );
    if( aIndex.hasValue() )
    {
        return xMenuItems->Item( aIndex, uno::Any() );
    }
    return uno::Any( xMenuItems );
}

OUString
ScVbaMenu::getServiceImplName()
{
    return u"ScVbaMenu"_ustr;
}

uno::Sequence<OUString>
ScVbaMenu::getServiceNames()
{
    static uno::Sequence< OUString > const aServiceNames
    {
        u"ooo.vba.excel.Menu"_ustr
    };
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

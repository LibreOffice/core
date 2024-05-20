/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "vbamenubar.hxx"
#include "vbamenus.hxx"
#include <ooo/vba/XCommandBarControls.hpp>
#include <utility>

using namespace com::sun::star;
using namespace ooo::vba;

ScVbaMenuBar::ScVbaMenuBar( const uno::Reference< ov::XHelperInterface >& rParent, const uno::Reference< uno::XComponentContext >& rContext, uno::Reference< XCommandBar > xCommandBar ) : MenuBar_BASE(rParent, rContext), m_xCommandBar(std::move(xCommandBar))
{
}

uno::Any SAL_CALL
ScVbaMenuBar::Menus( const uno::Any& aIndex )
{
    uno::Reference< XCommandBarControls > xCommandBarControls( m_xCommandBar->Controls( uno::Any() ), uno::UNO_QUERY_THROW );
    uno::Reference< excel::XMenus > xMenus( new ScVbaMenus( this, mxContext, xCommandBarControls ) );
    if( aIndex.hasValue() )
    {
        return xMenus->Item( aIndex, uno::Any() );
    }
    return uno::Any( xMenus );
}

OUString
ScVbaMenuBar::getServiceImplName()
{
    return u"ScVbaMenuBar"_ustr;
}

uno::Sequence<OUString>
ScVbaMenuBar::getServiceNames()
{
    static uno::Sequence< OUString > const aServiceNames
    {
        u"ooo.vba.excel.MenuBar"_ustr
    };
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

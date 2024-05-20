/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <utility>

#include "vbamenuitem.hxx"

using namespace com::sun::star;
using namespace ooo::vba;

ScVbaMenuItem::ScVbaMenuItem( const uno::Reference< ov::XHelperInterface >& rParent, const uno::Reference< uno::XComponentContext >& rContext, uno::Reference< XCommandBarControl > xCommandBarControl ) : MenuItem_BASE( rParent, rContext ), m_xCommandBarControl(std::move( xCommandBarControl ))
{
}

OUString SAL_CALL
ScVbaMenuItem::getCaption()
{
    return m_xCommandBarControl->getCaption();
}

void SAL_CALL
ScVbaMenuItem::setCaption( const OUString& _caption )
{
    m_xCommandBarControl->setCaption( _caption );
}

OUString SAL_CALL
ScVbaMenuItem::getOnAction()
{
    return m_xCommandBarControl->getOnAction();
}

void SAL_CALL
ScVbaMenuItem::setOnAction( const OUString& _onaction )
{
    m_xCommandBarControl->setOnAction( _onaction );
}

void SAL_CALL
ScVbaMenuItem::Delete( )
{
    m_xCommandBarControl->Delete();
}

OUString
ScVbaMenuItem::getServiceImplName()
{
    return u"ScVbaMenuItem"_ustr;
}

uno::Sequence<OUString>
ScVbaMenuItem::getServiceNames()
{
    static uno::Sequence< OUString > const aServiceNames
    {
        u"ooo.vba.excel.MenuItem"_ustr
    };
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

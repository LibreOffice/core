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

#include "vbafiledialog.hxx"
#include <com/sun/star/container/XIndexAccess.hpp>

using namespace ::com::sun::star;
using namespace ::ooo::vba;

ScVbaFileDialog::ScVbaFileDialog( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext)
    : ScVbaFileDialog_BASE( xParent, xContext)
    , m_sTitle("FileDialog")
    , m_xItems(new ScVbaFileDialogSelectedItems(this, xContext, uno::Reference<container::XIndexAccess>()))
{}

uno::Any
ScVbaFileDialog::getInitialFileName() { return uno::makeAny( m_sInitialFileName ); }

void ScVbaFileDialog::setInitialFileName( const css::uno::Any& rName )
{
    rName >>= m_sInitialFileName;
}

css::uno::Any ScVbaFileDialog::getTitle() { return uno::makeAny( m_sTitle ); }

void ScVbaFileDialog::setTitle( const css::uno::Any& rTitle )
{
    rTitle >>= m_sTitle;
}

uno::Reference< excel::XFileDialogSelectedItems > SAL_CALL ScVbaFileDialog::getSelectedItems()
{
    // TODO use InitialFileName when m_xItems is empty
    return m_xItems;
}

sal_Int32 ScVbaFileDialog::Show()
{
    // TODO show file picker

    // TODO change SelectedItems with the file picker
    return 0;
}

// XHelperInterface
OUString
ScVbaFileDialog::getServiceImplName()
{
    return OUString("ScVbaFileDialog");
}

uno::Sequence<OUString>
ScVbaFileDialog::getServiceNames()
{
    static uno::Sequence< OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = "ooo.vba.FileDialog";
    }
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

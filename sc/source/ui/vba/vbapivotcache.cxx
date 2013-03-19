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
#include "vbapivotcache.hxx"


using namespace ::com::sun::star;
using namespace ::ooo::vba;

ScVbaPivotCache::ScVbaPivotCache( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext,  const uno::Reference< sheet::XDataPilotTable >& xTable ) : PivotCacheImpl_BASE( xParent, xContext ), m_xTable( xTable )
{
}

void SAL_CALL
ScVbaPivotCache::Refresh() throw (css::uno::RuntimeException)
{
    m_xTable->refresh();
}

rtl::OUString
ScVbaPivotCache::getServiceImplName()
{
    return rtl::OUString("ScVbaPivotCache");
}

uno::Sequence< rtl::OUString >
ScVbaPivotCache::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString("ooo.vba.excel.PivotCache" );
    }
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

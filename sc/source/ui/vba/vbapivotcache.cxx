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
#include <utility>

#include "vbapivotcache.hxx"

using namespace ::com::sun::star;
using namespace ::ooo::vba;

ScVbaPivotCache::ScVbaPivotCache( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext,  uno::Reference< sheet::XDataPilotTable > xTable ) : PivotCacheImpl_BASE( xParent, xContext ), m_xTable(std::move( xTable ))
{
}

void SAL_CALL
ScVbaPivotCache::Refresh()
{
    m_xTable->refresh();
}

OUString
ScVbaPivotCache::getServiceImplName()
{
    return u"ScVbaPivotCache"_ustr;
}

uno::Sequence< OUString >
ScVbaPivotCache::getServiceNames()
{
    static uno::Sequence< OUString > const aServiceNames
    {
        u"ooo.vba.excel.PivotCache"_ustr
    };
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

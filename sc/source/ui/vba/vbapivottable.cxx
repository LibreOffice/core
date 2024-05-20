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

#include "vbapivottable.hxx"
#include "vbapivotcache.hxx"

using namespace ::com::sun::star;
using namespace ::ooo::vba;

ScVbaPivotTable::ScVbaPivotTable( const uno::Reference< uno::XComponentContext >& xContext, uno::Reference< sheet::XDataPilotTable > xTable ) : PivotTableImpl_BASE( uno::Reference< XHelperInterface >(),  xContext), m_xTable(std::move( xTable ))
{
}

uno::Reference< excel::XPivotCache >
ScVbaPivotTable::PivotCache()
{
    // #FIXME with a quick example failed to determine what the parent
    // should be, leaving as null at the moment
    return new ScVbaPivotCache( uno::Reference< XHelperInterface >(), mxContext, m_xTable );
}

OUString
ScVbaPivotTable::getServiceImplName()
{
    return u"ScVbaPivotTable"_ustr;
}

uno::Sequence< OUString >
ScVbaPivotTable::getServiceNames()
{
    static uno::Sequence< OUString > const aServiceNames
    {
        u"ooo.vba.excel.PivotTable"_ustr
    };
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

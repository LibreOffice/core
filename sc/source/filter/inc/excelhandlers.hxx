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

#ifndef INCLUDED_SC_SOURCE_FILTER_INC_EXCELHANDLERS_HXX
#define INCLUDED_SC_SOURCE_FILTER_INC_EXCELHANDLERS_HXX

#include <oox/core/fragmenthandler2.hxx>
#include <oox/helper/binaryinputstream.hxx>
#include "worksheethelper.hxx"

namespace oox {
namespace xls {

/** Context handler derived from the WorkbookHelper helper class.

    Used to import contexts in global workbook fragments.
 */
class WorkbookContextBase : public ::oox::core::ContextHandler2, public WorkbookHelper
{
public:
    template< typename ParentType >
    explicit     WorkbookContextBase( ParentType& rParent ) :
                            ::oox::core::ContextHandler2( rParent ), WorkbookHelper( rParent ) {}
};

/** Context handler derived from the WorksheetHelper helper class.

    Used to import contexts in sheet fragments.
 */
class WorksheetContextBase : public ::oox::core::ContextHandler2, public WorksheetHelper
{
public:
    template< typename ParentType >
    explicit     WorksheetContextBase( ParentType& rParent ) :
                            ::oox::core::ContextHandler2( rParent ), WorksheetHelper( rParent ) {}
};

/** Fragment handler derived from the WorkbookHelper helper class.

    Used to import global workbook fragments.
 */
class WorkbookFragmentBase : public ::oox::core::FragmentHandler2, public WorkbookHelper
{
public:
    explicit            WorkbookFragmentBase(
                            const WorkbookHelper& rHelper,
                            const OUString& rFragmentPath );
};

/** Fragment handler derived from the WorksheetHelper helper class.

    Used to import sheet fragments.
 */
class WorksheetFragmentBase : public ::oox::core::FragmentHandler2, public WorksheetHelper
{
public:
    explicit            WorksheetFragmentBase(
                            const WorksheetHelper& rHelper,
                            const OUString& rFragmentPath );
};


} // namespace xls
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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

#ifndef INCLUDED_OOX_DRAWINGML_TABLE_TABLESTYLELISTFRAGMENTHANDLER_HXX
#define INCLUDED_OOX_DRAWINGML_TABLE_TABLESTYLELISTFRAGMENTHANDLER_HXX

#include <drawingml/table/tablestylelist.hxx>
#include <oox/core/fragmenthandler2.hxx>

namespace oox {
namespace drawingml {
namespace table {

class TableStyleListFragmentHandler : public ::oox::core::FragmentHandler2
{
public:
    explicit            TableStyleListFragmentHandler(
                            ::oox::core::XmlFilterBase& rFilter,
                            const OUString& rFragmentPath,
                            TableStyleList& rTableStyleList );
    virtual             ~TableStyleListFragmentHandler();

    virtual ::oox::core::ContextHandlerRef onCreateContext( ::sal_Int32 Element, const AttributeList& rAttribs ) override;

private:

    TableStyleList&              mrTableStyleList;
};

} // namespace table
} // namespace drawingml
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

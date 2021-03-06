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
#pragma once

#include <xmloff/xmlimp.hxx>
#include <xmloff/maptype.hxx>

class PageHeaderFooterContext : public SvXMLImportContext
{
    ::std::vector< XMLPropertyState > &     rProperties;
    sal_Int32                               nStartIndex;
    sal_Int32                               nEndIndex;
    bool                                bHeader;
    const rtl::Reference < SvXMLImportPropertyMapper > rMap;

public:

    PageHeaderFooterContext( SvXMLImport& rImport,
                        ::std::vector< XMLPropertyState > & rProperties,
                        const rtl::Reference < SvXMLImportPropertyMapper > &rMap,
                        sal_Int32 nStartIndex, sal_Int32 nEndIndex,
                        const bool bHeader);

    virtual ~PageHeaderFooterContext() override;

    virtual css::uno::Reference< css::xml::sax::XFastContextHandler > SAL_CALL createFastChildContext(
        sal_Int32 nElement, const css::uno::Reference< css::xml::sax::XFastAttributeList >& AttrList ) override;

};


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

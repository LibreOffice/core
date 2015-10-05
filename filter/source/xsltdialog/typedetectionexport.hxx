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

#ifndef INCLUDED_FILTER_SOURCE_XSLTDIALOG_TYPEDETECTIONEXPORT_HXX
#define INCLUDED_FILTER_SOURCE_XSLTDIALOG_TYPEDETECTIONEXPORT_HXX

#include <com/sun/star/xml/sax/XWriter.hpp>
#include <com/sun/star/io/XOutputStream.hpp>

#include "xmlfilterjar.hxx"

class TypeDetectionExporter
{
public:
    explicit TypeDetectionExporter( css::uno::Reference< css::uno::XComponentContext >& mxContext );

    void doExport(css::uno::Reference < css::io::XOutputStream > xOS,  const XMLFilterVector& rFilters );

private:
    static void addProperty( css::uno::Reference< css::xml::sax::XWriter > xWriter, const OUString& rName, const OUString& rValue );
    static void addLocaleProperty( css::uno::Reference< css::xml::sax::XWriter > xWriter, const OUString& rName, const OUString& rValue );

    css::uno::Reference< css::uno::XComponentContext > mxContext;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

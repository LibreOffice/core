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

#include <dataprovider.hxx>
#include <datatransformation.hxx>
#include <datamapper.hxx>
#include <document.hxx>
#include <stringutil.hxx>

#include <com/sun/star/sheet/XExtensionDataProvider.hpp>
#include <com/sun/star/table/CellContentType.hpp>
#include <comphelper/string.hxx>
#include <tools/stream.hxx>
#include <vcl/svapp.hxx>
#include <docsh.hxx>
#include <orcus/csv_parser.hpp>
#include <utility>

namespace sc
{

class ExtensionDataProvider : public DataProvider
{
private:
    ScDocument* mpDocument;
    css::uno::Reference < css::sheet::XExtensionDataProvider > mxDataProvider;

public:
    ExtensionDataProvider(ScDocument* pDoc, sc::ExternalDataSource& rDataSource);
    virtual ~ExtensionDataProvider() override;

    virtual void Import() override;
    void ImportFinished();

    virtual const OUString& GetURL() const override;

    void Refresh();
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

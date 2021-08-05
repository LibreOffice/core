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

#include "ExtensionDataProvider.hxx"
#include <dataprovider.hxx>
#include <datatransformation.hxx>
#include <datamapper.hxx>
#include <document.hxx>
#include <stringutil.hxx>

#include <com/sun/star/sheet/XExtensionDataProvider.hpp>
#include <com/sun/star/table/XCell.hpp>
#include <com/sun/star/table/CellContentType.hpp>
#include <comphelper/string.hxx>
#include <tools/stream.hxx>
#include <vcl/svapp.hxx>
#include <docsh.hxx>
#include <orcus/csv_parser.hpp>
#include <utility>

namespace sc {

ExtensionDataProvider::ExtensionDataProvider(ScDocument* pDoc, sc::ExternalDataSource& rDataSource):
    DataProvider(rDataSource),
    mpDocument(pDoc)
{
}

ExtensionDataProvider::~ExtensionDataProvider()
{
}

void ExtensionDataProvider::Import()
{
    // already importing data
    if (mpDocument)
        return;

    /**
        Get Range of document.
        Returns the min Column , max Column , min Row & max Row
        in the given order.
    */
    css::uno::Sequence < int > aRange = mxDataProvider->getProviderRange();
    SCCOL nMinCol = aRange[0];
    SCCOL nMaxCol = aRange[1];
    SCROW nMinRow = aRange[2];
    SCROW nMaxRow = aRange[3];

    for (SCROW nRow = nMinRow; nRow <= nMaxRow; ++nRow)
    {
        for (SCCOL nCol = nMinCol; nCol <= nMaxCol; ++nCol)
            {
                // insert the data into the spreadsheet
                css::uno::Reference < css::table::XCell > aCellData = mxDataProvider->getCellByPosition(nCol , nRow);
                css::table::CellContentType aCellType = aCellData->getType();
                if (aCellType == css::table::CellContentType_VALUE)
                    mpDocument->SetValue(nCol, nRow, 0, aCellData->getValue());
                else if (aCellType == css::table::CellContentType_TEXT)
                    mpDocument->SetString(nCol, nRow, 0, aCellData->getFormula());
            }
    }
    ImportFinished();
}

void ExtensionDataProvider::ImportFinished()
{
    Refresh();
}

void ExtensionDataProvider::Refresh()
{
    ScDocShell* pDocShell = static_cast<ScDocShell*>(mpDocument->GetDocumentShell());
    if (pDocShell)
        pDocShell->SetDocumentModified();
}

const OUString& ExtensionDataProvider::GetURL() const
{
    return mrDataSource.getURL();
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

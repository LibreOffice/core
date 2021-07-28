/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <dataprovider.hxx>
#include <datatransformation.hxx>
#include <datamapper.hxx>
#include <document.hxx>
#include <stringutil.hxx>

#include <comphelper/string.hxx>
#include <tools/stream.hxx>
#include <vcl/svapp.hxx>
#include <docsh.hxx>
#include <orcus/csv_parser.hpp>
#include <utility>

ExtensionDataProvider::ExtensionDataProvider(ScDocument* pDoc, uno::Reference< sheet::XDataProvider > rDataProvider):
    mxDataProvider(rDataProvider),
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

    // Get Range of cells as String . For example A1:B3
    OUString aRange = mxDataProvider->GetRangeAsString();
    std::vector<String> aRangeString = comphelper::string::split(aRange , ':');
    SCCOL nMinCol = aRangeString[0][0] - 65;
    SCCOL nMaxCol = aRangeString[1][0] - 65;
    SCROW nMinRow = aRangeString[0][1] - 1;
    SCROW nMaxRow = aRangeString[1][1] - 1;
    // convert aRange to column and row size
    for (SCROW nRow = nMinRow; nRow <= nMaxRow; ++nRow)
    {
        for (SCCOL nCol = nMinCol; nCol <= nMaxCol; ++nCol)
            {
                std::pair <css::uno::Any, CellType> aData = mxDataProvider->GetCellData(nRow , nCol);
                if (aData.second == CELLTYPE_STRING)
                    mpDocument.SetString(nCol, nRow, 0, aData.first);
                // insert the data into the spreadsheet
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

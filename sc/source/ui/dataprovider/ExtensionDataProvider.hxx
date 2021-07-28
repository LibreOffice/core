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

namespace sc
{

class ExtensionDataProvider : public uno::Reference< sheet::XDataProvider >
{
private:
    ScDocument* mpDocument;
    uno::Reference< sheet::XDataProvider > mxDataProvider;

public:
    ExtensionDataProvider(ScDocument* pDoc, uno::Reference< sheet::XDataProvider > rDataProvider);
    virtual ~ExtensionDataProvider() override;

    void Import();

    void ImportFinished();
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

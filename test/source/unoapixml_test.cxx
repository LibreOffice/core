/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/unoapixml_test.hxx>

#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <comphelper/processfactory.hxx>
#include <comphelper/propertyvalue.hxx>

#include <sfx2/app.hxx>
#include <sfx2/objsh.hxx>
#include <utility>

using namespace css;

UnoApiXmlTest::UnoApiXmlTest(OUString path)
    : UnoApiTest(std::move(path))
{
}

xmlDocUniquePtr UnoApiXmlTest::parseExport(OUString const& rStreamName)
{
    std::unique_ptr<SvStream> const pStream(parseExportStream(maTempFile.GetURL(), rStreamName));
    xmlDocUniquePtr pXmlDoc = parseXmlStream(pStream.get());
    return pXmlDoc;
}

xmlDocUniquePtr UnoApiXmlTest::parseExportedFile()
{
    auto stream(SvFileStream(maTempFile.GetURL(), StreamMode::READ | StreamMode::TEMPORARY));
    return parseXmlStream(&stream);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

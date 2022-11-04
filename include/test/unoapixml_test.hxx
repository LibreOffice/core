/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_TEST_UNOAPIXML_TEST_HXX
#define INCLUDED_TEST_UNOAPIXML_TEST_HXX

#include <sal/config.h>

#include <test/unoapi_test.hxx>
#include <test/xmltesttools.hxx>

// basic uno api xml test class

class OOO_DLLPUBLIC_TEST UnoApiXmlTest : public UnoApiTest, public XmlTestTools
{
public:
    UnoApiXmlTest(OUString path);

    xmlDocUniquePtr parseExport(OUString const& rStreamName);
};

#endif // INCLUDED_TEST_UNOAPIXML_TEST_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

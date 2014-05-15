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

#include <test/bootstrapfixture.hxx>
#include <test/xmltesttools.hxx>
#include <test/mtfxmldump.hxx>
#include <vcl/wmf.hxx>
#include <vcl/metaact.hxx>

using namespace css;

class WmfTest : public test::BootstrapFixture, public XmlTestTools
{
public:
    WmfTest() :
        BootstrapFixture(true, false)
    {}

    void testNonPlaceableWmf();

    CPPUNIT_TEST_SUITE(WmfTest);
    CPPUNIT_TEST(testNonPlaceableWmf);
    CPPUNIT_TEST_SUITE_END();
};

void WmfTest::testNonPlaceableWmf()
{
    OUString aUrl = getURLFromSrc("/vcl/qa/cppunit/wmf/data/");

    SvFileStream aFileStream(aUrl + "visio_import_source.wmf", STREAM_READ);
    GDIMetaFile aGDIMetaFile;
    ReadWindowMetafile(aFileStream, aGDIMetaFile);

    boost::scoped_ptr<SvMemoryStream> aStream(new SvMemoryStream);

    MetafileXmlDump dumper(*aStream);
    dumper.filterAllActionTypes();
    dumper.filterActionType(META_POLYLINE_ACTION, false);
    dumper.dump(aGDIMetaFile);

    aStream->Seek(STREAM_SEEK_TO_BEGIN);

    xmlDocPtr pDoc = parseXmlStream(aStream.get());

    CPPUNIT_ASSERT (pDoc);

    assertXPath(pDoc, "/metafile/polyline[1]/point[1]", "x", "16798");
    assertXPath(pDoc, "/metafile/polyline[1]/point[1]", "y", "1003");

    assertXPath(pDoc, "/metafile/polyline[1]/point[2]", "x", "16798");
    assertXPath(pDoc, "/metafile/polyline[1]/point[2]", "y", "7507");

    assertXPath(pDoc, "/metafile/polyline[1]/point[3]", "x", "26090");
    assertXPath(pDoc, "/metafile/polyline[1]/point[3]", "y", "7507");

    assertXPath(pDoc, "/metafile/polyline[1]/point[4]", "x", "26090");
    assertXPath(pDoc, "/metafile/polyline[1]/point[4]", "y", "1003");

    assertXPath(pDoc, "/metafile/polyline[1]/point[5]", "x", "16798");
    assertXPath(pDoc, "/metafile/polyline[1]/point[5]", "y", "1003");
}

CPPUNIT_TEST_SUITE_REGISTRATION(WmfTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

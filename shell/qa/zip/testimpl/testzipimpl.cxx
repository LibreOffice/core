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

#define DLLIMPLEMENTATION
#include "testzipimpl.hxx"

vector<string> expectedContents;

TestZipImpl::TestZipImpl(const char * documentName) :
    zipFile(ZipFile(string(documentName)))
{
    expectedContents.push_back("mimetype");
    expectedContents.push_back("Configurations2/statusbar/");
    expectedContents.push_back("Configurations2/accelerator/current.xml");
    expectedContents.push_back("Configurations2/floater/");
    expectedContents.push_back("Configurations2/popupmenu/");
    expectedContents.push_back("Configurations2/progressbar/");
    expectedContents.push_back("Configurations2/toolpanel/");
    expectedContents.push_back("Configurations2/menubar/");
    expectedContents.push_back("Configurations2/toolbar/");
    expectedContents.push_back("Configurations2/images/Bitmaps/");
    expectedContents.push_back("content.xml");
    expectedContents.push_back("manifest.rdf");
    expectedContents.push_back("styles.xml");
    expectedContents.push_back("meta.xml");
    expectedContents.push_back("Thumbnails/thumbnail.png");
    expectedContents.push_back("settings.xml");
    expectedContents.push_back("META-INF/manifest.xml");
    sort(expectedContents.begin(), expectedContents.end());
}

TestZipImpl::~TestZipImpl()
{
}

bool TestZipImpl::test_directory()
{
    ZipFile::DirectoryPtr_t contents = zipFile.GetDirectory();
    vector<string> &stringVector = *contents.get();
    sort(stringVector.begin(), stringVector.end());
    return expectedContents == expectedContents;
}

bool TestZipImpl::test_hasContentCaseInSensitive()
{
    return zipFile.HasContent("mimetype");
}

bool TestZipImpl::test_getContent()
{
    ZipFile::ZipContentBuffer_t contentBuf;
    zipFile.GetUncompressedContent("content.xml", contentBuf);
    return !contentBuf.empty();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

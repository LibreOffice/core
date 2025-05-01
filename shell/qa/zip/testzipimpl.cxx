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

#include "testzipimpl.hxx"

TestZipImpl::TestZipImpl(StreamInterface* stream)
    : zipFile{ stream }
    , expectedContents{ "mimetype",
                        "Configurations2/statusbar/",
                        "Configurations2/accelerator/current.xml",
                        "Configurations2/floater/",
                        "Configurations2/popupmenu/",
                        "Configurations2/progressbar/",
                        "Configurations2/toolpanel/",
                        "Configurations2/menubar/",
                        "Configurations2/toolbar/",
                        "Configurations2/images/Bitmaps/",
                        "content.xml",
                        "manifest.rdf",
                        "styles.xml",
                        "meta.xml",
                        "Thumbnails/thumbnail.png",
                        "settings.xml",
                        "META-INF/manifest.xml" }
{
    sort(expectedContents.begin(), expectedContents.end());
}

TestZipImpl::~TestZipImpl() {}

bool TestZipImpl::test_directory()
{
    ZipFile::DirectoryPtr_t contents = zipFile.GetDirectory();
    std::vector<std::string>& stringVector = *contents;
    std::sort(stringVector.begin(), stringVector.end());
    return expectedContents == stringVector;
}

bool TestZipImpl::test_hasContentCaseInSensitive() { return zipFile.HasContent("mimetype"); }

bool TestZipImpl::test_getContent()
{
    ZipFile::ZipContentBuffer_t contentBuf;
    zipFile.GetUncompressedContent("content.xml", contentBuf);
    return !contentBuf.empty();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_shell.hxx"

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


//------------------------------------------------
bool TestZipImpl::test_directory()
{
        ZipFile::DirectoryPtr_t contents = zipFile.GetDirectory();
        vector<string> &stringVector = *contents.get();
        sort(stringVector.begin(), stringVector.end());
        return expectedContents == expectedContents;
}

//------------------------------------------------
bool TestZipImpl::test_hasContentCaseInSensitive()
{
        return zipFile.HasContent("mimetype");
}

//------------------------------------------------
bool TestZipImpl::test_getContent()
{
        ZipFile::ZipContentBuffer_t contentBuf;
        zipFile.GetUncompressedContent("content.xml", contentBuf);
        return !contentBuf.empty();
}


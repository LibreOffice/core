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

#ifndef INCLUDED_FILTER_SOURCE_PLACEWARE_ZIP_HXX
#define INCLUDED_FILTER_SOURCE_PLACEWARE_ZIP_HXX

#include <sal/types.h>
#include <rtl/string.hxx>
#include <osl/file.hxx>

#include <vector>

struct ZipEntry;

class PlacewareZipFile
{
public:
    explicit PlacewareZipFile( osl::File& rFile );
    ~PlacewareZipFile();

    bool addFile( osl::File& rFile, const OString& rName );
    bool close();

private:
    void writeShort( sal_Int16 s);
    void writeLong( sal_Int32 l );

    void copyAndCRC( ZipEntry *e, osl::File& rFile );
    void writeDummyLocalHeader(ZipEntry *e);
    void writeLocalHeader(ZipEntry *e);
    void writeCentralDir(ZipEntry *e);
    void writeEndCentralDir(sal_Int32 nCdOffset, sal_Int32 nCdSize);

private:
    bool isError() const { return osl::File::E_None != mnRC; }

    osl::File& mrFile;              /* file we're writing to */
    bool mbOpen;
    osl::File::RC mnRC;
    std::vector<ZipEntry*> maEntries;
};

#endif // INCLUDED_FILTER_SOURCE_PLACEWARE_ZIP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

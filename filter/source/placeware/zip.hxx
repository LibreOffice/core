/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include <sal/types.h>
#include <rtl/string.hxx>
#include <osl/file.hxx>

#include <vector>

struct ZipEntry;

class ZipFile
{
public:
    ZipFile( osl::File& rFile );
    ~ZipFile();

    bool addFile( osl::File& rFile, const rtl::OString& rName );
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

    osl::File& mrFile;				/* file we're writing to */
    bool mbOpen;
    osl::File::RC mnRC;
    std::vector<ZipEntry*> maEntries;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

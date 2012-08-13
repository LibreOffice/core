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

#include "sal/config.h"

#include <iosfwd>
#include <vector>

#include "export.hxx"

typedef std::vector< rtl::OString* > LngLineList;

#define LNG_OK              0x0000
#define LNG_COULD_NOT_OPEN  0x0001
#define SDF_OK              0x0002
#define SDF_COULD_NOT_OPEN  0x0003

//
// class LngParser
//

class LngParser
{
private:
    sal_uInt16 nError;
    LngLineList *pLines;
    rtl::OString sSource;
    sal_Bool bULF;
    std::vector<rtl::OString> aLanguages;

    bool isNextGroup(rtl::OString &sGroup_out, rtl::OString &sLine_in);
    void ReadLine(const rtl::OString &rLine_in,
        OStringHashMap &rText_inout);
    void WriteSDF(std::ofstream &aSDFStream, OStringHashMap &rText_inout,
        const rtl::OString &rPrj, const rtl::OString &rRoot,
        const rtl::OString &rActFileName, const rtl::OString &rID);
public:
    LngParser(const rtl::OString &rLngFile,
        sal_Bool bULFFormat);
    ~LngParser();

    sal_Bool CreateSDF(const rtl::OString &rSDFFile,
        const rtl::OString &rPrj,
        const rtl::OString &rRoot);
    sal_Bool Merge(const rtl::OString &rSDFFile,
        const rtl::OString &rDestinationFile);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

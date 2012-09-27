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

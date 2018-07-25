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

#ifndef INCLUDED_L10NTOOLS_INC_LNGMERGE_HXX
#define INCLUDED_L10NTOOLS_INC_LNGMERGE_HXX

#include <sal/config.h>

#include <iosfwd>
#include <vector>

#include "common.hxx"
#include "export.hxx"

#define LNG_OK              0x0000
#define LNG_COULD_NOT_OPEN  0x0001


/**
  Class for localization of *.ulf files

  Parse *.ulf files, extract translatable strings
  and merge translated strings.
*/
class LngParser
{
private:
    std::vector<OString> mvLines;
    OString sSource;
    std::vector<OString> aLanguages;

    static bool isNextGroup(OString &sGroup_out, const OString &sLine_in);
    static void ReadLine(const OString &rLine_in,
        OStringHashMap &rText_inout);
    static void WritePO(PoOfstream &aPOStream, OStringHashMap &rText_inout,
        const OString &rActFileName, const OString &rID);
public:
    LngParser(const OString &rLngFile);
    ~LngParser();

    void CreatePO( const OString &rPOFile );
    void Merge(const OString &rPOFile, const OString &rDestinationFile,
         const OString &rLanguage );
};

#endif // INCLUDED_L10NTOOLS_INC_LNGMERGE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

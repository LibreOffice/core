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

#ifndef SC_CELLFORM_HXX
#define SC_CELLFORM_HXX

#include <tools/solar.h>
#include "scdllapi.h"

class SvNumberFormatter;
class Color;
class ScDocument;
class ScAddress;
struct ScRefCellValue;

enum ScForceTextFmt {
    ftDontForce,            ///< numbers as numbers
    ftForce,                ///< numbers as text
    ftCheck                 ///< is the numberformat a textformat?
};

//------------------------------------------------------------------------

class SC_DLLPUBLIC ScCellFormat
{
public:

    static void GetString(
        ScRefCellValue& rCell, sal_uLong nFormat, OUString& rString,
        Color** ppColor, SvNumberFormatter& rFormatter, const ScDocument* pDoc, bool bNullVals = true,
        bool bFormula  = false, ScForceTextFmt eForceTextFmt = ftDontForce,
        bool bUseStarFormat = false );

    static OUString GetString(
        ScDocument& rDoc, const ScAddress& rPos, sal_uLong nFormat,
        Color** ppColor, SvNumberFormatter& rFormatter, bool bNullVals = true,
        bool bFormula  = false, ScForceTextFmt eForceTextFmt = ftDontForce, bool bUseStarFormat = false );

    static void GetInputString(
        ScRefCellValue& rCell, sal_uLong nFormat, OUString& rString, SvNumberFormatter& rFormatter,
        const ScDocument* pDoc );
};




#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

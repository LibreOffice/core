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

#ifndef SC_CELLFORM_HXX
#define SC_CELLFORM_HXX

#include <tools/solar.h>
#include "scdllapi.h"

class String;

class ScBaseCell;
class SvNumberFormatter;
class Color;

enum ScForceTextFmt {
    ftDontForce,            // numbers as numbers
    ftForce,                // numbers as text
    ftCheck                 // is the numberformat a textformat?
};

//------------------------------------------------------------------------

class SC_DLLPUBLIC ScCellFormat
{
public:
    static void     GetString( ScBaseCell* pCell, sal_uLong nFormat, String& rString,
                               Color** ppColor, SvNumberFormatter& rFormatter,
                               sal_Bool bNullVals = sal_True,
                               sal_Bool bFormula  = false,
                               ScForceTextFmt eForceTextFmt = ftDontForce );

    static void     GetInputString( ScBaseCell* pCell, sal_uLong nFormat, String& rString,
                                      SvNumberFormatter& rFormatter );
};




#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

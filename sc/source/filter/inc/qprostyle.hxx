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


#ifndef SC_QPROSTYLE_HXX
#define SC_QPROSTYLE_HXX

#include <sal/config.h>
#include "filter.hxx"
#include "document.hxx"
#include "cell.hxx"
#include <tools/string.hxx>

#include <tools/color.hxx>
#include "flttypes.hxx"
#include "ftools.hxx"

class ScQProStyle
{
    enum limits { maxsize = 256 };
    sal_uInt8  maAlign[ maxsize ];
    sal_uInt8  maFont[ maxsize ];
    sal_uInt16 maFontRecord[ maxsize ];
    sal_uInt16 maFontHeight[ maxsize ];
    String     maFontType[ maxsize ];

    public:
    ScQProStyle();
    void SetFormat( ScDocument *pDoc, sal_uInt8 nCol, sal_uInt16 nRow, SCTAB nTab, sal_uInt16 nStyle );
    void setFontRecord(sal_uInt16 nIndex, sal_uInt16 nData, sal_uInt16 nPtSize)
    {
        if (nIndex < maxsize)
        {
            maFontRecord[ nIndex ] = nData;
            maFontHeight[ nIndex ] = nPtSize;
        }
    }
    void setFontType( sal_uInt16 nIndex, String &aLabel )
        { if (nIndex < maxsize) maFontType[ nIndex ] = aLabel; }
    void setAlign( sal_uInt16 nIndex, sal_uInt8 nData )
        { if (nIndex < maxsize) maAlign[ nIndex ] = nData; }
    void setFont( sal_uInt16 nIndex, sal_uInt8 nData )
        { if (nIndex < maxsize) maFont[ nIndex ] = nData; }
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

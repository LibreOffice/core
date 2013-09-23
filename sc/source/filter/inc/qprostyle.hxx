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


#ifndef SC_QPROSTYLE_HXX
#define SC_QPROSTYLE_HXX

#include <rtl/ustring.hxx>
#include <sal/config.h>
#include "filter.hxx"

#include "flttypes.hxx"
#include "ftools.hxx"
#include "address.hxx"

class ScDocument;

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

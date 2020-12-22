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
#ifndef INCLUDED_EDITENG_ITEMTYPE_HXX
#define INCLUDED_EDITENG_ITEMTYPE_HXX

// forward ---------------------------------------------------------------
#include <rtl/ustring.hxx>
#include <tools/bigint.hxx>
#include <tools/mapunit.hxx>
#include <editeng/editengdllapi.h>

class Color;
class IntlWrapper;
// static and prototypes -------------------------------------------------

const sal_Unicode cpDelim[] = u", ";

EDITENG_DLLPUBLIC OUString GetMetricText( tools::Long nVal, MapUnit eSrcUnit, MapUnit eDestUnit, const IntlWrapper * pIntl );
OUString GetColorString( const Color& rCol );
EDITENG_DLLPUBLIC const char* GetMetricId(MapUnit eUnit);


inline tools::Long Scale( tools::Long nVal, tools::Long nMult, tools::Long nDiv )
{
    BigInt aVal( nVal );
    aVal *= nMult;
    aVal += nDiv/2;
    aVal /= nDiv;
    return aVal;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

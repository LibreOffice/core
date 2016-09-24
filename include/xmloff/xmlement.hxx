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

#ifndef INCLUDED_XMLOFF_XMLEMENT_HXX
#define INCLUDED_XMLOFF_XMLEMENT_HXX

#include <sal/types.h>
#include <xmloff/xmltoken.hxx>

/** Map an XMLTokenEnum to a sal_uInt16 value.
 * To be used with SvXMLUnitConverter::convertEnum(...)
 */
struct SvXMLEnumMapEntry
{
    ::xmloff::token::XMLTokenEnum   eToken;
    sal_uInt16                      nValue;
};

#define ENUM_STRING_MAP_ENTRY(name,tok) { name, sizeof(name)-1, tok }

#define ENUM_STRING_MAP_END()           { nullptr, 0, 0 }

/** Map a const sal_Char* (with length) to a sal_uInt16 value.
 * To be used with SvXMLUnitConverter::convertEnum(...)
 */
struct SvXMLEnumStringMapEntry
{
    const sal_Char *    pName;
    sal_Int32           nNameLength;
    sal_uInt16          nValue;
};

#endif // INCLUDED_XMLOFF_XMLEMENT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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

#include "opaquhdl.hxx"
#include <com/sun/star/uno/Any.hxx>
#include <rtl/ustrbuf.hxx>

#include <xmloff/xmltoken.hxx>
#include <xmloff/xmluconv.hxx>

using namespace ::com::sun::star::uno;
using namespace ::xmloff::token;

// class XMLOpaquePropHdl

XMLOpaquePropHdl::~XMLOpaquePropHdl()
{
    // nothing to do
}

bool XMLOpaquePropHdl::importXML( const OUString& rStrImpValue, Any& rValue, const SvXMLUnitConverter& ) const
{
    bool bRet = true;

    sal_Bool bValue = IsXMLToken( rStrImpValue, XML_OPAQUE_FOREGROUND );
    rValue <<= sal_Bool(bValue);

    return bRet;
}

bool XMLOpaquePropHdl::exportXML( OUString& rStrExpValue, const Any& rValue, const SvXMLUnitConverter& ) const
{
    bool bRet = false;
    sal_Bool bValue = sal_Bool();

    if (rValue >>= bValue)
    {
        if( bValue )
            rStrExpValue = GetXMLToken( XML_OPAQUE_FOREGROUND );
        else
            rStrExpValue = GetXMLToken( XML_OPAQUE_BACKGROUND );

        bRet = true;
    }

    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

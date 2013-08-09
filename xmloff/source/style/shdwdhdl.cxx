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

#include "shdwdhdl.hxx"
#include <com/sun/star/uno/Any.hxx>
#include <rtl/ustrbuf.hxx>

// --
#include <xmloff/xmltoken.hxx>

using namespace ::com::sun::star::uno;
using namespace ::xmloff::token;

//
// class XMLFmtSplitPropHdl
//

XMLShadowedPropHdl::~XMLShadowedPropHdl()
{
    // nothing to do
}

sal_Bool XMLShadowedPropHdl::importXML( const OUString& rStrImpValue, Any& rValue, const SvXMLUnitConverter& ) const
{
    sal_Bool bRet = sal_False;

    sal_Bool bValue = ! IsXMLToken( rStrImpValue, XML_NONE );
    rValue <<= sal_Bool(bValue);
    bRet = sal_True;

    return bRet;
}

sal_Bool XMLShadowedPropHdl::exportXML( OUString& rStrExpValue, const Any& rValue, const SvXMLUnitConverter& ) const
{
    sal_Bool bRet = sal_False;
    sal_Bool bValue = sal_Bool();

    if (rValue >>= bValue)
    {
        if( bValue )
        {
            rStrExpValue = "1pt 1pt";
        }
        else
        {
            rStrExpValue = GetXMLToken( XML_NONE );
        }

        bRet = sal_True;
    }

    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

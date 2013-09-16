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

#include "XMLTextOrientationHdl.hxx"
#include <xmloff/xmluconv.hxx>
#include <xmloff/xmltoken.hxx>

using namespace ::com::sun::star;
using namespace ::xmloff::token;

XMLTextOrientationHdl::~XMLTextOrientationHdl()
{
}

bool XMLTextOrientationHdl::importXML(
    const OUString& rStrImpValue,
    ::com::sun::star::uno::Any& rValue,
    const SvXMLUnitConverter& /*rUnitConverter*/ ) const
{
    bool bRetval( false );

    if( IsXMLToken( rStrImpValue, XML_LTR ))
    {
        rValue <<= static_cast< sal_Bool >( sal_False );
        bRetval = true;
    }
    else if( IsXMLToken( rStrImpValue, XML_TTB ))
    {
        rValue <<= static_cast< sal_Bool >( sal_True );
        bRetval = true;
    }

    return bRetval;
}

bool XMLTextOrientationHdl::exportXML(
    OUString& rStrExpValue,
    const ::com::sun::star::uno::Any& rValue,
    const SvXMLUnitConverter& /*rUnitConverter*/ ) const
{
    sal_Bool bVal (sal_False );
    bool bRetval( false );

    if( rValue >>= bVal )
    {
        if( bVal )
            rStrExpValue = GetXMLToken( XML_TTB );
        else
            rStrExpValue = GetXMLToken( XML_LTR );
        bRetval = true;
    }

    return bRetval;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

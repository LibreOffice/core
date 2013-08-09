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

#include <postuhdl.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmluconv.hxx>
#include <rtl/ustrbuf.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/awt/FontSlant.hpp>
#include <tools/fontenum.hxx>

using namespace ::com::sun::star;
using namespace ::xmloff::token;

SvXMLEnumMapEntry const aPostureGenericMapping[] =
{
    { XML_POSTURE_NORMAL,       ITALIC_NONE     },
    { XML_POSTURE_ITALIC,       ITALIC_NORMAL   },
    { XML_POSTURE_OBLIQUE,      ITALIC_OBLIQUE  },
    { XML_TOKEN_INVALID,        0               }
};

//
// class XMLPosturePropHdl
//

XMLPosturePropHdl::~XMLPosturePropHdl()
{
    // nothing to do
}

sal_Bool XMLPosturePropHdl::importXML( const OUString& rStrImpValue, uno::Any& rValue, const SvXMLUnitConverter& ) const
{
    sal_uInt16 ePosture;
    sal_Bool bRet = SvXMLUnitConverter::convertEnum( ePosture, rStrImpValue, aPostureGenericMapping );
    if( bRet )
        rValue <<= (awt::FontSlant)ePosture;

    return bRet;
}

sal_Bool XMLPosturePropHdl::exportXML( OUString& rStrExpValue, const uno::Any& rValue, const SvXMLUnitConverter& ) const
{
    awt::FontSlant eSlant;

    if( !( rValue >>= eSlant ) )
    {
        sal_Int32 nValue = 0;

        if( !( rValue >>= nValue ) )
            return sal_False;

        eSlant = (awt::FontSlant)nValue;
    }

    OUStringBuffer aOut;
    sal_Bool bRet = SvXMLUnitConverter::convertEnum( aOut, (sal_Int32)eSlant, aPostureGenericMapping );
    if( bRet )
        rStrExpValue = aOut.makeStringAndClear();

    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

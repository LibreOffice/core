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

#include <xmloff/NamedBoolPropertyHdl.hxx>
#include <xmloff/xmluconv.hxx>
#include <comphelper/extract.hxx>
#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>
#include <com/sun/star/uno/Any.hxx>

using namespace ::com::sun::star::uno;

//
// class XMLNamedBoolPropertyHdl
//

XMLNamedBoolPropertyHdl::~XMLNamedBoolPropertyHdl()
{
    // Nothing to do
}

bool XMLNamedBoolPropertyHdl::importXML( const OUString& rStrImpValue, Any& rValue, const SvXMLUnitConverter& ) const
{
    if( rStrImpValue == maTrueStr )
    {
        rValue = ::cppu::bool2any( sal_True );
        return true;
    }

    if( rStrImpValue == maFalseStr )
    {
        rValue = ::cppu::bool2any( sal_False );
        return true;
    }

    return false;
}

bool XMLNamedBoolPropertyHdl::exportXML( OUString& rStrExpValue, const Any& rValue, const SvXMLUnitConverter& ) const
{
    if( ::cppu::any2bool( rValue ) )
    {
        rStrExpValue = maTrueStr;
    }
    else
    {
        rStrExpValue = maFalseStr;
    }

    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

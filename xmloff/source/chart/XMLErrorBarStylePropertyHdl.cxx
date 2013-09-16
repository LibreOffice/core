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

#include "XMLErrorBarStylePropertyHdl.hxx"
#include <unotools/saveopt.hxx>

#include <com/sun/star/chart/ErrorBarStyle.hpp>
#include <com/sun/star/uno/Any.hxx>

using namespace com::sun::star;

XMLErrorBarStylePropertyHdl::XMLErrorBarStylePropertyHdl(  const SvXMLEnumMapEntry* pEnumMap, const ::com::sun::star::uno::Type & rType )
        : XMLEnumPropertyHdl( pEnumMap, rType )
{
}

XMLErrorBarStylePropertyHdl::~XMLErrorBarStylePropertyHdl()
{
}

bool XMLErrorBarStylePropertyHdl::exportXML( OUString& rStrExpValue,
                                              const uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const
{
    uno::Any aValue(rValue);
    const SvtSaveOptions::ODFDefaultVersion nCurrentVersion( SvtSaveOptions().GetODFDefaultVersion() );
    if( nCurrentVersion < SvtSaveOptions::ODFVER_012 )
    {
        sal_Int32 nValue = 0;
        if(rValue >>= nValue )
        {
            if( nValue == ::com::sun::star::chart::ErrorBarStyle::STANDARD_ERROR
                || nValue == ::com::sun::star::chart::ErrorBarStyle::FROM_DATA )
            {
                nValue = ::com::sun::star::chart::ErrorBarStyle::NONE;
                aValue = uno::makeAny(nValue);
            }
        }
    }

    return XMLEnumPropertyHdl::exportXML( rStrExpValue, aValue, rUnitConverter );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

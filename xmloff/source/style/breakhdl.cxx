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

#include "breakhdl.hxx"
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmluconv.hxx>
#include <xmloff/xmlement.hxx>
#include <rtl/ustrbuf.hxx>
#include <com/sun/star/style/BreakType.hpp>
#include <com/sun/star/uno/Any.hxx>

using namespace ::com::sun::star;
using namespace ::xmloff::token;

SvXMLEnumMapEntry<sal_uInt16> const pXML_BreakTypes[] =
{
    { XML_AUTO,         0 },
    { XML_COLUMN,       1 },
    { XML_PAGE,         2 },
    { XML_EVEN_PAGE,    2 },
    { XML_ODD_PAGE,     2 },
    { XML_TOKEN_INVALID, 0}
};


// class XMLFmtBreakBeforePropHdl


XMLFmtBreakBeforePropHdl::~XMLFmtBreakBeforePropHdl()
{
    // Nothing to do
}

bool XMLFmtBreakBeforePropHdl::importXML( const OUString& rStrImpValue, uno::Any& rValue, const SvXMLUnitConverter& ) const
{
    sal_uInt16 nEnum;
    bool bRet = SvXMLUnitConverter::convertEnum( nEnum, rStrImpValue, pXML_BreakTypes );
    if( bRet )
    {
        style::BreakType eBreak;
        switch ( nEnum )
        {
        case 0:
            eBreak = style::BreakType_NONE;
            break;
        case 1:
            eBreak = style::BreakType_COLUMN_BEFORE;
            break;
        default:
            eBreak = style::BreakType_PAGE_BEFORE;
            break;
        }
        rValue <<= eBreak;
    }

    return bRet;
}

bool XMLFmtBreakBeforePropHdl::exportXML( OUString& rStrExpValue, const uno::Any& rValue, const SvXMLUnitConverter& ) const
{
    style::BreakType eBreak;

    if( !( rValue >>= eBreak ) )
    {
        sal_Int32 nValue = 0;
        if( !( rValue >>= nValue ) )
            return false;

        eBreak = static_cast<style::BreakType>(nValue);
    }

    sal_uInt16 nEnum = 0;
    switch( eBreak )
    {
        case style::BreakType_COLUMN_BEFORE:
            nEnum = 1;
            break;
        case style::BreakType_PAGE_BEFORE:
            nEnum = 2;
            break;
        case style::BreakType_NONE:
            nEnum = 0;
            break;
        default:
            return false;
    }

    OUStringBuffer aOut;
    /* bool bOk = */ SvXMLUnitConverter::convertEnum( aOut, nEnum, pXML_BreakTypes );
    rStrExpValue = aOut.makeStringAndClear();

    return true;
}


// class XMLFmtBreakBeforePropHdl


XMLFmtBreakAfterPropHdl::~XMLFmtBreakAfterPropHdl()
{
    // Nothing to do
}

bool XMLFmtBreakAfterPropHdl::importXML( const OUString& rStrImpValue, uno::Any& rValue, const SvXMLUnitConverter& ) const
{
    sal_uInt16 nEnum;
    bool bRet = SvXMLUnitConverter::convertEnum( nEnum, rStrImpValue, pXML_BreakTypes );
    if( bRet )
    {
        style::BreakType eBreak;
        switch ( nEnum )
        {
        case 0:
            eBreak = style::BreakType_NONE;
            break;
        case 1:
            eBreak = style::BreakType_COLUMN_AFTER;
            break;
        default:
            eBreak = style::BreakType_PAGE_AFTER;
            break;
        }
        rValue <<= eBreak;
    }

    return bRet;
}

bool XMLFmtBreakAfterPropHdl::exportXML( OUString& rStrExpValue, const uno::Any& rValue, const SvXMLUnitConverter& ) const
{
    style::BreakType eBreak;

    if( !( rValue >>= eBreak ) )
    {
        sal_Int32 nValue = 0;
        if( !( rValue >>= nValue ) )
            return false;

        eBreak = static_cast<style::BreakType>(nValue);
    }

    sal_uInt16 nEnum = 0;
    switch( eBreak )
    {
        case style::BreakType_COLUMN_AFTER:
            nEnum = 1;
            break;
        case style::BreakType_PAGE_AFTER:
            nEnum = 2;
            break;
        case style::BreakType_NONE:
            nEnum = 0;
            break;
        default:
            return false;
    }

    OUStringBuffer aOut;
    /* bool bOk = */ SvXMLUnitConverter::convertEnum( aOut, nEnum, pXML_BreakTypes );
    rStrExpValue = aOut.makeStringAndClear();

    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

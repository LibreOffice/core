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

#pragma once

#include <xmloff/xmlprhdl.hxx>
#include <xmloff/xmltoken.hxx>


/**
    PropertyHandler for the XML-data-type: XML_TYPE_NUMBER
*/
class XMLNumberPropHdl final : public XMLPropertyHandler
{
    sal_Int8 nBytes;

public:
    explicit XMLNumberPropHdl( sal_Int8 nB ) : nBytes( nB ) {}
    virtual ~XMLNumberPropHdl() override;

    virtual bool importXML( const OUString& rStrImpValue, css::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const override;
    virtual bool exportXML( OUString& rStrExpValue, const css::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const override;
};

/**
    PropertyHandler for the XML-data-type: XML_TYPE_NUMBER_NONE
*/
class XMLNumberNonePropHdl final : public XMLPropertyHandler
{
    OUString sZeroStr;
    sal_Int8        nBytes;
public:
    explicit XMLNumberNonePropHdl( sal_Int8 nB = 4 );
    XMLNumberNonePropHdl( enum ::xmloff::token::XMLTokenEnum eZeroString, sal_Int8 nB );
    virtual ~XMLNumberNonePropHdl() override;

    virtual bool importXML( const OUString& rStrImpValue, css::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const override;
    virtual bool exportXML( OUString& rStrExpValue, const css::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const override;
};

/**
    PropertyHandler for the XML-data-type: XML_TYPE_MEASURE
*/
class XMLMeasurePropHdl final : public XMLPropertyHandler
{
    sal_Int8 nBytes;
public:
    explicit XMLMeasurePropHdl( sal_Int8 nB ) : nBytes( nB ) {}
    virtual ~XMLMeasurePropHdl() override;

    virtual bool importXML( const OUString& rStrImpValue, css::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const override;
    virtual bool exportXML( OUString& rStrExpValue, const css::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const override;
};

/**
    PropertyHandler for the XML-data-type: XML_TYPE_UNIT_MEASURE
*/
class XMLUnitMeasurePropHdl final : public XMLPropertyHandler
{
public:
    bool importXML( const OUString& rStrImpValue, css::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const override;
    bool exportXML( OUString& rStrExpValue, const css::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const override;
};

/**
    PropertyHandler for the XML-data-type: XML_TYPE_PERCENT
*/
class XMLPercentPropHdl final : public XMLPropertyHandler
{
    sal_Int8 nBytes;
public:
    explicit XMLPercentPropHdl( sal_Int8 nB ) : nBytes( nB ) {}
    virtual ~XMLPercentPropHdl() override;

    virtual bool importXML( const OUString& rStrImpValue, css::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const override;
    virtual bool exportXML( OUString& rStrExpValue, const css::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const override;
};

/**
    PropertyHandler for the XML-data-type: XML_TYPE_PERCENT
    that is mapped on a double from 0.0 to 1.0
*/
class XMLDoublePercentPropHdl final : public XMLPropertyHandler
{
    virtual bool importXML( const OUString& rStrImpValue, css::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const override;
    virtual bool exportXML( OUString& rStrExpValue, const css::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const override;
};

/// Maps between XML percentage and our 100th percent ints.
class XML100thPercentPropHdl final : public XMLPropertyHandler
{
    virtual bool importXML(const OUString& rStrImpValue, css::uno::Any& rValue,
                           const SvXMLUnitConverter& rUnitConverter) const override;
    virtual bool exportXML(OUString& rStrExpValue, const css::uno::Any& rValue,
                           const SvXMLUnitConverter& rUnitConverter) const override;
};

/**
    PropertyHandler for the XML-data-type: XML_TYPE_NEG_PERCENT
*/
class XMLNegPercentPropHdl final : public XMLPropertyHandler
{
    sal_Int8 nBytes;
public:
    explicit XMLNegPercentPropHdl( sal_Int8 nB ) : nBytes( nB ) {}
    virtual ~XMLNegPercentPropHdl() override;

    virtual bool importXML( const OUString& rStrImpValue, css::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const override;
    virtual bool exportXML( OUString& rStrExpValue, const css::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const override;
};

/**
    PropertyHandler for the XML-data-type: XML_TYPE_PERCENT
*/
class XMLMeasurePxPropHdl final : public XMLPropertyHandler
{
    sal_Int8 nBytes;
public:
    explicit XMLMeasurePxPropHdl( sal_Int8 nB ) : nBytes( nB ) {}
    virtual ~XMLMeasurePxPropHdl() override;

    virtual bool importXML( const OUString& rStrImpValue, css::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const override;
    virtual bool exportXML( OUString& rStrExpValue, const css::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const override;
};

/**
    PropertyHandler for the XML-data-type: XML_TYPE_BOOL
*/
class XMLBoolPropHdl : public XMLPropertyHandler
{
public:
    virtual ~XMLBoolPropHdl() override;

    virtual bool importXML( const OUString& rStrImpValue, css::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const override;
    virtual bool exportXML( OUString& rStrExpValue, const css::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const override;
};

class XMLBoolFalsePropHdl final : public XMLBoolPropHdl
{
public:
    virtual ~XMLBoolFalsePropHdl() override;

    virtual bool importXML( const OUString& rStrImpValue, css::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const override;
    virtual bool exportXML( OUString& rStrExpValue, const css::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const override;

};

/**
    PropertyHandler for the XML-data-type: XML_TYPE_COLOR
*/
class XMLColorPropHdl final : public XMLPropertyHandler
{
public:
    virtual ~XMLColorPropHdl() override;

    virtual bool importXML( const OUString& rStrImpValue, css::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const override;
    virtual bool exportXML( OUString& rStrExpValue, const css::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const override;
};

/**
    PropertyHandler for the XML-data-type: XML_TYPE_HEX
*/
class XMLHexPropHdl final : public XMLPropertyHandler
{
public:
    virtual ~XMLHexPropHdl() override;

    virtual bool importXML( const OUString& rStrImpValue, css::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const override;
    virtual bool exportXML( OUString& rStrExpValue, const css::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const override;
};

/**
    PropertyHandler for the XML-data-type: XML_TYPE_STRING
*/
class XMLStringPropHdl : public XMLPropertyHandler
{
public:
    virtual ~XMLStringPropHdl() override;

    virtual bool importXML( const OUString& rStrImpValue, css::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const override;
    virtual bool exportXML( OUString& rStrExpValue, const css::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const override;
};

/**
    PropertyHandler for the XML-data-type: XML_TYPE_STYLENAME
*/
class XMLStyleNamePropHdl final : public XMLStringPropHdl
{
public:
    virtual ~XMLStyleNamePropHdl() override;

    virtual bool exportXML( OUString& rStrExpValue, const css::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const override;
};


/**
    PropertyHandler for the XML-data-type: XML_TYPE_DOUBLE
*/
class XMLDoublePropHdl : public XMLPropertyHandler
{
public:
    virtual ~XMLDoublePropHdl() override;

    virtual bool importXML( const OUString& rStrImpValue, css::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const override;
    virtual bool exportXML( OUString& rStrExpValue, const css::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const override;
};

/**
    PropertyHandler for the XML-data-type: XML_TYPE_NBOOL
*/
class XMLNBoolPropHdl final : public XMLPropertyHandler
{
public:
    virtual ~XMLNBoolPropHdl() override;

    virtual bool importXML( const OUString& rStrImpValue, css::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const override;
    virtual bool exportXML( OUString& rStrExpValue, const css::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const override;
};

/**
    PropertyHandler for the XML-data-type: XML_TYPE_COLORTRANSPARENT
*/
class XMLColorTransparentPropHdl final : public XMLPropertyHandler
{
    const OUString sTransparent;

public:
    explicit XMLColorTransparentPropHdl( enum ::xmloff::token::XMLTokenEnum eTransparent = xmloff::token::XML_TOKEN_INVALID );
    virtual ~XMLColorTransparentPropHdl() override;

    virtual bool importXML( const OUString& rStrImpValue, css::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const override;
    virtual bool exportXML( OUString& rStrExpValue, const css::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const override;
};

/**
    PropertyHandler for the XML-data-type: XML_TYPE_ISTRANSPARENT
*/
class XMLIsTransparentPropHdl final : public XMLPropertyHandler
{
    const OUString sTransparent;
    bool bTransPropValue;

public:
    XMLIsTransparentPropHdl( enum ::xmloff::token::XMLTokenEnum eTransparent = xmloff::token::XML_TOKEN_INVALID,
                             bool bTransPropValue = true );
    virtual ~XMLIsTransparentPropHdl() override;

    virtual bool importXML( const OUString& rStrImpValue, css::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const override;
    virtual bool exportXML( OUString& rStrExpValue, const css::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const override;
};

/**
    PropertyHandler for the XML-data-type: XML_TYPE_COLORAUTO
*/
class XMLColorAutoPropHdl final : public XMLPropertyHandler
{
public:
    XMLColorAutoPropHdl();
    virtual ~XMLColorAutoPropHdl() override;

    virtual bool importXML( const OUString& rStrImpValue, css::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const override;
    virtual bool exportXML( OUString& rStrExpValue, const css::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const override;
};

/**
    PropertyHandler for the XML-data-type: XML_TYPE_COLORISAUTO
*/
class XMLIsAutoColorPropHdl final : public XMLPropertyHandler
{
public:
    XMLIsAutoColorPropHdl();
    virtual ~XMLIsAutoColorPropHdl() override;

    virtual bool importXML( const OUString& rStrImpValue, css::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const override;
    virtual bool exportXML( OUString& rStrExpValue, const css::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const override;
};


/**
    PropertyHandler for properties that cannot make use of importXML
    and exportXML methods, but can make use of the default comparison
*/
class XMLCompareOnlyPropHdl final : public XMLPropertyHandler
{
public:
    virtual ~XMLCompareOnlyPropHdl() override;

    virtual bool importXML( const OUString& rStrImpValue, css::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const override;
    virtual bool exportXML( OUString& rStrExpValue, const css::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const override;
};

/**
    PropertyHandler for the XML-data-type: XML_TYPE_NUMBER_NO_ZERO
    Reads/writes numeric properties, but fails for the value zero
    (i.e., a value 0 property will not be written)
*/
class XMLNumberWithoutZeroPropHdl final : public XMLPropertyHandler
{
    sal_Int8        nBytes;
public:
    explicit XMLNumberWithoutZeroPropHdl( sal_Int8 nB );
    virtual ~XMLNumberWithoutZeroPropHdl() override;

    virtual bool importXML( const OUString& rStrImpValue, css::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const override;
    virtual bool exportXML( OUString& rStrExpValue, const css::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

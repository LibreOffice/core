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

#ifndef _XMLOFF_PROPERTYHANDLER_BASICTYPES_HXX
#define _XMLOFF_PROPERTYHANDLER_BASICTYPES_HXX

#include <xmloff/xmlprhdl.hxx>
#include <rtl/ustrbuf.hxx>
#include <xmloff/xmltoken.hxx>


/**
    PropertyHandler for the XML-data-type: XML_TYPE_NUMBER
*/
class XMLNumberPropHdl : public XMLPropertyHandler
{
    sal_Int8 nBytes;

public:
    XMLNumberPropHdl( sal_Int8 nB=4 ) : nBytes( nB ) {}
    virtual ~XMLNumberPropHdl();

    virtual bool importXML( const OUString& rStrImpValue, ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
    virtual bool exportXML( OUString& rStrExpValue, const ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
};

/**
    PropertyHandler for the XML-data-type: XML_TYPE_NUMBER_NONE
*/
class XMLNumberNonePropHdl : public XMLPropertyHandler
{
    OUString sZeroStr;
    sal_Int8        nBytes;
public:
    XMLNumberNonePropHdl( sal_Int8 nB = 4 );
    XMLNumberNonePropHdl( enum ::xmloff::token::XMLTokenEnum eZeroString, sal_Int8 nB = 4 );
    virtual ~XMLNumberNonePropHdl();

    virtual bool importXML( const OUString& rStrImpValue, ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
    virtual bool exportXML( OUString& rStrExpValue, const ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
};

/**
    PropertyHandler for the XML-data-type: XML_TYPE_MEASURE
*/
class XMLMeasurePropHdl : public XMLPropertyHandler
{
    sal_Int8 nBytes;
public:
    XMLMeasurePropHdl( sal_Int8 nB=4 ) : nBytes( nB ) {}
    virtual ~XMLMeasurePropHdl();

    virtual bool importXML( const OUString& rStrImpValue, ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
    virtual bool exportXML( OUString& rStrExpValue, const ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
};

/**
    PropertyHandler for the XML-data-type: XML_TYPE_PERCENT
*/
class XMLPercentPropHdl : public XMLPropertyHandler
{
    sal_Int8 nBytes;
public:
    XMLPercentPropHdl( sal_Int8 nB=4 ) : nBytes( nB ) {}
    virtual ~XMLPercentPropHdl();

    virtual bool importXML( const OUString& rStrImpValue, ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
    virtual bool exportXML( OUString& rStrExpValue, const ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
};

/**
    PropertyHandler for the XML-data-type: XML_TYPE_PERCENT
    that is mapped on a double from 0.0 to 1.0
*/
class XMLDoublePercentPropHdl : public XMLPropertyHandler
{
    virtual bool importXML( const OUString& rStrImpValue, ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
    virtual bool exportXML( OUString& rStrExpValue, const ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
};

/**
    PropertyHandler for the XML-data-type: XML_TYPE_NEG_PERCENT
*/
class XMLNegPercentPropHdl : public XMLPropertyHandler
{
    sal_Int8 nBytes;
public:
    XMLNegPercentPropHdl( sal_Int8 nB=4 ) : nBytes( nB ) {}
    virtual ~XMLNegPercentPropHdl();

    virtual bool importXML( const OUString& rStrImpValue, ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
    virtual bool exportXML( OUString& rStrExpValue, const ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
};

/**
    PropertyHandler for the XML-data-type: XML_TYPE_PERCENT
*/
class XMLMeasurePxPropHdl : public XMLPropertyHandler
{
    sal_Int8 nBytes;
public:
    XMLMeasurePxPropHdl( sal_Int8 nB=4 ) : nBytes( nB ) {}
    virtual ~XMLMeasurePxPropHdl();

    virtual bool importXML( const OUString& rStrImpValue, ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
    virtual bool exportXML( OUString& rStrExpValue, const ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
};

/**
    PropertyHandler for the XML-data-type: XML_TYPE_BOOL
*/
class XMLBoolPropHdl : public XMLPropertyHandler
{
public:
    virtual ~XMLBoolPropHdl();

    virtual bool importXML( const OUString& rStrImpValue, ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
    virtual bool exportXML( OUString& rStrExpValue, const ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
};

class XMLBoolFalsePropHdl : public XMLBoolPropHdl
{
public:
    virtual ~XMLBoolFalsePropHdl();

    virtual bool importXML( const OUString& rStrImpValue, ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
    virtual bool exportXML( OUString& rStrExpValue, const ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;

};

/**
    PropertyHandler for the XML-data-type: XML_TYPE_COLOR
*/
class XMLColorPropHdl : public XMLPropertyHandler
{
public:
    virtual ~XMLColorPropHdl();

    virtual bool importXML( const OUString& rStrImpValue, ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
    virtual bool exportXML( OUString& rStrExpValue, const ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
};

/**
    PropertyHandler for the XML-data-type: XML_TYPE_HEX
*/
class XMLHexPropHdl : public XMLPropertyHandler
{
public:
    virtual ~XMLHexPropHdl();

    virtual bool importXML( const OUString& rStrImpValue, ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
    virtual bool exportXML( OUString& rStrExpValue, const ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
};

/**
    PropertyHandler for the XML-data-type: XML_TYPE_STRING
*/
class XMLStringPropHdl : public XMLPropertyHandler
{
public:
    virtual ~XMLStringPropHdl();

    virtual bool importXML( const OUString& rStrImpValue, ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
    virtual bool exportXML( OUString& rStrExpValue, const ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
};

/**
    PropertyHandler for the XML-data-type: XML_TYPE_STYLENAME
*/
class XMLStyleNamePropHdl : public XMLStringPropHdl
{
public:
    virtual ~XMLStyleNamePropHdl();

    virtual bool exportXML( OUString& rStrExpValue, const ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
};


/**
    PropertyHandler for the XML-data-type: XML_TYPE_DOUBLE
*/
class XMLDoublePropHdl : public XMLPropertyHandler
{
public:
    virtual ~XMLDoublePropHdl();

    virtual bool importXML( const OUString& rStrImpValue, ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
    virtual bool exportXML( OUString& rStrExpValue, const ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
};

/**
    PropertyHandler for the XML-data-type: XML_TYPE_NBOOL
*/
class XMLNBoolPropHdl : public XMLPropertyHandler
{
public:
    virtual ~XMLNBoolPropHdl();

    virtual bool importXML( const OUString& rStrImpValue, ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
    virtual bool exportXML( OUString& rStrExpValue, const ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
};

/**
    PropertyHandler for the XML-data-type: XML_TYPE_COLORTRANSPARENT
*/
class XMLColorTransparentPropHdl : public XMLPropertyHandler
{
    const OUString sTransparent;

public:
    XMLColorTransparentPropHdl( enum ::xmloff::token::XMLTokenEnum eTransparent = xmloff::token::XML_TOKEN_INVALID );
    virtual ~XMLColorTransparentPropHdl();

    virtual bool importXML( const OUString& rStrImpValue, ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
    virtual bool exportXML( OUString& rStrExpValue, const ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
};

/**
    PropertyHandler for the XML-data-type: XML_TYPE_ISTRANSPARENT
*/
class XMLIsTransparentPropHdl : public XMLPropertyHandler
{
    const OUString sTransparent;
    bool bTransPropValue;

public:
    XMLIsTransparentPropHdl( enum ::xmloff::token::XMLTokenEnum eTransparent = xmloff::token::XML_TOKEN_INVALID,
                             bool bTransPropValue = true );
    virtual ~XMLIsTransparentPropHdl();

    virtual bool importXML( const OUString& rStrImpValue, ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
    virtual bool exportXML( OUString& rStrExpValue, const ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
};

/**
    PropertyHandler for the XML-data-type: XML_TYPE_COLORAUTO
*/
class XMLColorAutoPropHdl : public XMLPropertyHandler
{
public:
    XMLColorAutoPropHdl();
    virtual ~XMLColorAutoPropHdl();

    virtual bool importXML( const OUString& rStrImpValue, ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
    virtual bool exportXML( OUString& rStrExpValue, const ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
};

/**
    PropertyHandler for the XML-data-type: XML_TYPE_COLORISAUTO
*/
class XMLIsAutoColorPropHdl : public XMLPropertyHandler
{
public:
    XMLIsAutoColorPropHdl();
    virtual ~XMLIsAutoColorPropHdl();

    virtual bool importXML( const OUString& rStrImpValue, ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
    virtual bool exportXML( OUString& rStrExpValue, const ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
};


/**
    PropertyHandler for properties that cannot make use of importXML
    and exportXML methods, but can make use of the default comparison
*/
class XMLCompareOnlyPropHdl : public XMLPropertyHandler
{
public:
    virtual ~XMLCompareOnlyPropHdl();

    virtual bool importXML( const OUString& rStrImpValue, ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
    virtual bool exportXML( OUString& rStrExpValue, const ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
};

/**
    PropertyHandler for the XML-data-type: XML_TYPE_NUMBER_NO_ZERO
    Reads/writes numeric properties, but fails for the value zero
    (i.e., a value 0 property will not be written)
*/
class XMLNumberWithoutZeroPropHdl : public XMLPropertyHandler
{
    sal_Int8        nBytes;
public:
    XMLNumberWithoutZeroPropHdl( sal_Int8 nB = 4 );
    virtual ~XMLNumberWithoutZeroPropHdl();

    virtual bool importXML( const OUString& rStrImpValue, ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
    virtual bool exportXML( OUString& rStrExpValue, const ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
};

/**
    PropertyHandler for the XML-data-type: XML_TYPE_NUMBER16_AUTO
    Reads/writes numeric properties with special handling for the value zero
    (i.e., a value 0 property will be written as "auto")
*/
class XMLNumberWithAutoInsteadZeroPropHdl : public XMLNumberWithoutZeroPropHdl
{
public:
    virtual ~XMLNumberWithAutoInsteadZeroPropHdl();

    virtual bool importXML( const OUString& rStrImpValue, ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
    virtual bool exportXML( OUString& rStrExpValue, const ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
};

#endif      // _XMLOFF_PROPERTYHANDLER_BASICTYPES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

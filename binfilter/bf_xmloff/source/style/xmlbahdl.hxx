/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _XMLOFF_PROPERTYHANDLER_BASICTYPES_HXX
#define _XMLOFF_PROPERTYHANDLER_BASICTYPES_HXX

#include <xmlprhdl.hxx>

#include <rtl/ustrbuf.hxx>

#include "xmltoken.hxx"
namespace binfilter {


/**
    PropertyHandler for the XML-data-type: XML_TYPE_NUMBER
*/
class XMLNumberPropHdl : public XMLPropertyHandler
{
    sal_Int8 nBytes;

public:
    XMLNumberPropHdl( sal_Int8 nB=4 ) : nBytes( nB ) {}
    virtual ~XMLNumberPropHdl();

    virtual sal_Bool importXML( const ::rtl::OUString& rStrImpValue, ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
    virtual sal_Bool exportXML( ::rtl::OUString& rStrExpValue, const ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
};

/**
    PropertyHandler for the XML-data-type: XML_TYPE_NUMBER_NONE
*/
class XMLNumberNonePropHdl : public XMLPropertyHandler
{
    ::rtl::OUString	sZeroStr;
    sal_Int8		nBytes;
public:
    XMLNumberNonePropHdl( sal_Int8 nB = 4 );
    XMLNumberNonePropHdl( enum ::binfilter::xmloff::token::XMLTokenEnum eZeroString, sal_Int8 nB = 4 );
    virtual ~XMLNumberNonePropHdl();

    virtual sal_Bool importXML( const ::rtl::OUString& rStrImpValue, ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
    virtual sal_Bool exportXML( ::rtl::OUString& rStrExpValue, const ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
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

    virtual sal_Bool importXML( const ::rtl::OUString& rStrImpValue, ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
    virtual sal_Bool exportXML( ::rtl::OUString& rStrExpValue, const ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
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

    virtual sal_Bool importXML( const ::rtl::OUString& rStrImpValue, ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
    virtual sal_Bool exportXML( ::rtl::OUString& rStrExpValue, const ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
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

    virtual sal_Bool importXML( const ::rtl::OUString& rStrImpValue, ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
    virtual sal_Bool exportXML( ::rtl::OUString& rStrExpValue, const ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
};

/**
    PropertyHandler for the XML-data-type: XML_TYPE_BOOL
*/
class XMLBoolPropHdl : public XMLPropertyHandler
{
public:
    virtual ~XMLBoolPropHdl();

    virtual sal_Bool importXML( const ::rtl::OUString& rStrImpValue, ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
    virtual sal_Bool exportXML( ::rtl::OUString& rStrExpValue, const ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
};

/**
    PropertyHandler for the XML-data-type: XML_TYPE_COLOR
*/
class XMLColorPropHdl : public XMLPropertyHandler
{
public:
    virtual ~XMLColorPropHdl();

    virtual sal_Bool importXML( const ::rtl::OUString& rStrImpValue, ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
    virtual sal_Bool exportXML( ::rtl::OUString& rStrExpValue, const ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
};

/**
    PropertyHandler for the XML-data-type: XML_TYPE_STRING
*/
class XMLStringPropHdl : public XMLPropertyHandler
{
public:
    virtual ~XMLStringPropHdl();

    virtual sal_Bool importXML( const ::rtl::OUString& rStrImpValue, ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
    virtual sal_Bool exportXML( ::rtl::OUString& rStrExpValue, const ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
};

/**
    PropertyHandler for the XML-data-type: XML_TYPE_DOUBLE
*/
class XMLDoublePropHdl : public XMLPropertyHandler
{
public:
    virtual ~XMLDoublePropHdl();

    virtual sal_Bool importXML( const ::rtl::OUString& rStrImpValue, ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
    virtual sal_Bool exportXML( ::rtl::OUString& rStrExpValue, const ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
};

/**
    PropertyHandler for the XML-data-type: XML_TYPE_NBOOL
*/
class XMLNBoolPropHdl : public XMLPropertyHandler
{
public:
    virtual ~XMLNBoolPropHdl();

    virtual sal_Bool importXML( const ::rtl::OUString& rStrImpValue, ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
    virtual sal_Bool exportXML( ::rtl::OUString& rStrExpValue, const ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
};

/**
    PropertyHandler for the XML-data-type: XML_TYPE_COLORTRANSPARENT
*/
class XMLColorTransparentPropHdl : public XMLPropertyHandler
{
    const ::rtl::OUString sTransparent;

public:
    XMLColorTransparentPropHdl( enum ::binfilter::xmloff::token::XMLTokenEnum eTransparent = xmloff::token::XML_TOKEN_INVALID );
    virtual ~XMLColorTransparentPropHdl();

    virtual sal_Bool importXML( const ::rtl::OUString& rStrImpValue, ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
    virtual sal_Bool exportXML( ::rtl::OUString& rStrExpValue, const ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
};

/**
    PropertyHandler for the XML-data-type: XML_TYPE_ISTRANSPARENT
*/
class XMLIsTransparentPropHdl : public XMLPropertyHandler
{
    const ::rtl::OUString sTransparent;
    sal_Bool bTransPropValue;

public:
    XMLIsTransparentPropHdl( enum ::binfilter::xmloff::token::XMLTokenEnum eTransparent = xmloff::token::XML_TOKEN_INVALID,
                             sal_Bool bTransPropValue = sal_True );
    virtual ~XMLIsTransparentPropHdl();

    virtual sal_Bool importXML( const ::rtl::OUString& rStrImpValue, ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
    virtual sal_Bool exportXML( ::rtl::OUString& rStrExpValue, const ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
};

/**
    PropertyHandler for the XML-data-type: XML_TYPE_COLORAUTO
*/
class XMLColorAutoPropHdl : public XMLPropertyHandler
{
public:
    XMLColorAutoPropHdl();
    virtual ~XMLColorAutoPropHdl();

    virtual sal_Bool importXML( const ::rtl::OUString& rStrImpValue, ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
    virtual sal_Bool exportXML( ::rtl::OUString& rStrExpValue, const ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
};

/**
    PropertyHandler for the XML-data-type: XML_TYPE_COLORISAUTO
*/
class XMLIsAutoColorPropHdl : public XMLPropertyHandler
{
public:
    XMLIsAutoColorPropHdl();
    virtual ~XMLIsAutoColorPropHdl();

    virtual sal_Bool importXML( const ::rtl::OUString& rStrImpValue, ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
    virtual sal_Bool exportXML( ::rtl::OUString& rStrExpValue, const ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
};


/**
    PropertyHandler for properties that cannot make use of importXML
    and exportXML methods, but can make use of the default comparison
*/
class XMLCompareOnlyPropHdl : public XMLPropertyHandler
{
public:
    virtual ~XMLCompareOnlyPropHdl();

    virtual sal_Bool importXML( const ::rtl::OUString& rStrImpValue, ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
    virtual sal_Bool exportXML( ::rtl::OUString& rStrExpValue, const ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
};


}//end of namespace binfilter
#endif		// _XMLOFF_PROPERTYHANDLER_BASICTYPES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xmlbahdl.hxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: hr $ $Date: 2007-08-03 12:54:56 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _XMLOFF_PROPERTYHANDLER_BASICTYPES_HXX
#define _XMLOFF_PROPERTYHANDLER_BASICTYPES_HXX

#ifndef _XMLOFF_PROPERTYHANDLERBASE_HXX
#include <xmloff/xmlprhdl.hxx>
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#ifndef _XMLOFF_XMLTOKEN_HXX
#include <xmloff/xmltoken.hxx>
#endif


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
    ::rtl::OUString sZeroStr;
    sal_Int8        nBytes;
public:
    XMLNumberNonePropHdl( sal_Int8 nB = 4 );
    XMLNumberNonePropHdl( enum ::xmloff::token::XMLTokenEnum eZeroString, sal_Int8 nB = 4 );
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
    that is mapped on a double from 0.0 to 1.0
*/
class XMLDoublePercentPropHdl : public XMLPropertyHandler
{
    virtual sal_Bool importXML( const ::rtl::OUString& rStrImpValue, ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
    virtual sal_Bool exportXML( ::rtl::OUString& rStrExpValue, const ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
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
    PropertyHandler for the XML-data-type: XML_TYPE_STYLENAME
*/
class XMLStyleNamePropHdl : public XMLStringPropHdl
{
public:
    virtual ~XMLStyleNamePropHdl();

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
    XMLColorTransparentPropHdl( enum ::xmloff::token::XMLTokenEnum eTransparent = xmloff::token::XML_TOKEN_INVALID );
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
    XMLIsTransparentPropHdl( enum ::xmloff::token::XMLTokenEnum eTransparent = xmloff::token::XML_TOKEN_INVALID,
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

    virtual sal_Bool importXML( const ::rtl::OUString& rStrImpValue, ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
    virtual sal_Bool exportXML( ::rtl::OUString& rStrExpValue, const ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
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

    virtual sal_Bool importXML( const ::rtl::OUString& rStrImpValue, ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
    virtual sal_Bool exportXML( ::rtl::OUString& rStrExpValue, const ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
};

#endif      // _XMLOFF_PROPERTYHANDLER_BASICTYPES_HXX

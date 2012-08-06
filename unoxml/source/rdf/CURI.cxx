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

#include "CNodes.hxx"

#include <cppuhelper/implbase3.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/rdf/XURI.hpp>
#include <com/sun/star/rdf/URIs.hpp>

#include <com/sun/star/lang/IllegalArgumentException.hpp>


/// anonymous implementation namespace
namespace {

namespace css = ::com::sun::star;

class CURI:
    public ::cppu::WeakImplHelper3<
        css::lang::XServiceInfo,
        css::lang::XInitialization,
        css::rdf::XURI>
{
public:
    explicit CURI(css::uno::Reference< css::uno::XComponentContext > const & context);
    virtual ~CURI() {}

    // ::com::sun::star::lang::XServiceInfo:
    virtual ::rtl::OUString SAL_CALL getImplementationName() throw (css::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL supportsService(const ::rtl::OUString & ServiceName) throw (css::uno::RuntimeException);
    virtual css::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw (css::uno::RuntimeException);

    // ::com::sun::star::lang::XInitialization:
    virtual void SAL_CALL initialize(const css::uno::Sequence< ::com::sun::star::uno::Any > & aArguments) throw (css::uno::RuntimeException, css::uno::Exception);

    // ::com::sun::star::rdf::XNode:
    virtual ::rtl::OUString SAL_CALL getStringValue() throw (css::uno::RuntimeException);

    // ::com::sun::star::rdf::XURI:
    virtual ::rtl::OUString SAL_CALL getLocalName() throw (css::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getNamespace() throw (css::uno::RuntimeException);

private:
    CURI(const CURI &); // not defined
    CURI& operator=(const CURI &); // not defined

    /// handle css.rdf.URIs
    void SAL_CALL initFromConstant(const sal_Int16 i_Constant);

    css::uno::Reference< css::uno::XComponentContext > m_xContext;

    ::rtl::OUString m_Namespace;
    ::rtl::OUString m_LocalName;
};

CURI::CURI(css::uno::Reference< css::uno::XComponentContext > const & context) :
    m_xContext(context), m_Namespace(), m_LocalName()
{}

// com.sun.star.uno.XServiceInfo:
::rtl::OUString SAL_CALL CURI::getImplementationName() throw (css::uno::RuntimeException)
{
    return comp_CURI::_getImplementationName();
}

::sal_Bool SAL_CALL CURI::supportsService(::rtl::OUString const & serviceName) throw (css::uno::RuntimeException)
{
    css::uno::Sequence< ::rtl::OUString > serviceNames = comp_CURI::_getSupportedServiceNames();
    for (::sal_Int32 i = 0; i < serviceNames.getLength(); ++i) {
        if (serviceNames[i] == serviceName)
            return sal_True;
    }
    return sal_False;
}

css::uno::Sequence< ::rtl::OUString > SAL_CALL CURI::getSupportedServiceNames() throw (css::uno::RuntimeException)
{
    return comp_CURI::_getSupportedServiceNames();
}

const char s_nsXS       [] = "http://www.w3.org/2001/XMLSchema#";
const char s_nsXSD      [] = "http://www.w3.org/2001/XMLSchema-datatypes#";
const char s_nsRDF      [] = "http://www.w3.org/1999/02/22-rdf-syntax-ns#";
const char s_nsRDFs     [] = "http://www.w3.org/2000/01/rdf-schema#";
const char s_nsOWL      [] = "http://www.w3.org/2002/07/owl#";
const char s_nsPkg      [] =
    "http://docs.oasis-open.org/ns/office/1.2/meta/pkg#";
const char s_nsODF      [] =
    "http://docs.oasis-open.org/ns/office/1.2/meta/odf#";

void SAL_CALL CURI::initFromConstant(const sal_Int16 i_Constant)
{
    const char *ns(0);
    const char *ln(0);
    switch (i_Constant)
    {
        case css::rdf::URIs::XSD_NCNAME:
            ns = s_nsXSD;
            ln = "NCName";
            break;

        case css::rdf::URIs::XSD_STRING:
            ns = s_nsXSD;
            ln = "string";
            break;

        case css::rdf::URIs::XSD_NORMALIZEDSTRING:
            ns = s_nsXSD;
            ln = "normalizedString";
            break;

        case css::rdf::URIs::XSD_BOOLEAN:
            ns = s_nsXSD;
            ln = "boolean";
            break;

        case css::rdf::URIs::XSD_DECIMAL:
            ns = s_nsXSD;
            ln = "decimal";
            break;

        case css::rdf::URIs::XSD_FLOAT:
            ns = s_nsXSD;
            ln = "float";
            break;

        case css::rdf::URIs::XSD_DOUBLE:
            ns = s_nsXSD;
            ln = "double";
            break;

        case css::rdf::URIs::XSD_INTEGER:
            ns = s_nsXSD;
            ln = "integer";
            break;

        case css::rdf::URIs::XSD_NONNEGATIVEINTEGER:
            ns = s_nsXSD;
            ln = "nonNegativeInteger";
            break;

        case css::rdf::URIs::XSD_POSITIVEINTEGER:
            ns = s_nsXSD;
            ln = "positiveInteger";
            break;

        case css::rdf::URIs::XSD_NONPOSITIVEINTEGER:
            ns = s_nsXSD;
            ln = "nonPositiveInteger";
            break;

        case css::rdf::URIs::XSD_NEGATIVEINTEGER:
            ns = s_nsXSD;
            ln = "negativeInteger";
            break;

        case css::rdf::URIs::XSD_LONG:
            ns = s_nsXSD;
            ln = "long";
            break;

        case css::rdf::URIs::XSD_INT:
            ns = s_nsXSD;
            ln = "int";
            break;

        case css::rdf::URIs::XSD_SHORT:
            ns = s_nsXSD;
            ln = "short";
            break;

        case css::rdf::URIs::XSD_BYTE:
            ns = s_nsXSD;
            ln = "byte";
            break;

        case css::rdf::URIs::XSD_UNSIGNEDLONG:
            ns = s_nsXSD;
            ln = "unsignedLong";
            break;

        case css::rdf::URIs::XSD_UNSIGNEDINT:
            ns = s_nsXSD;
            ln = "unsignedInt";
            break;

        case css::rdf::URIs::XSD_UNSIGNEDSHORT:
            ns = s_nsXSD;
            ln = "unsignedShort";
            break;

        case css::rdf::URIs::XSD_UNSIGNEDBYTE:
            ns = s_nsXSD;
            ln = "unsignedByte";
            break;

        case css::rdf::URIs::XSD_HEXBINARY:
            ns = s_nsXSD;
            ln = "hexBinary";
            break;

        case css::rdf::URIs::XSD_BASE64BINARY:
            ns = s_nsXSD;
            ln = "base64Binary";
            break;

        case css::rdf::URIs::XSD_DATETIME:
            ns = s_nsXSD;
            ln = "dateTime";
            break;

        case css::rdf::URIs::XSD_TIME:
            ns = s_nsXSD;
            ln = "time";
            break;

        case css::rdf::URIs::XSD_DATE:
            ns = s_nsXSD;
            ln = "date";
            break;

        case css::rdf::URIs::XSD_GYEARMONTH:
            ns = s_nsXSD;
            ln = "gYearMonth";
            break;

        case css::rdf::URIs::XSD_GYEAR:
            ns = s_nsXSD;
            ln = "gYear";
            break;

        case css::rdf::URIs::XSD_GMONTHDAY:
            ns = s_nsXSD;
            ln = "gMonthDay";
            break;

        case css::rdf::URIs::XSD_GDAY:
            ns = s_nsXSD;
            ln = "gDay";
            break;

        case css::rdf::URIs::XSD_GMONTH:
            ns = s_nsXSD;
            ln = "gMonth";
            break;

        case css::rdf::URIs::XSD_ANYURI:
            ns = s_nsXSD;
            ln = "anyURI";
            break;

        case css::rdf::URIs::XSD_TOKEN:
            ns = s_nsXSD;
            ln = "token";
            break;

        case css::rdf::URIs::XSD_LANGUAGE:
            ns = s_nsXSD;
            ln = "language";
            break;

        case css::rdf::URIs::XSD_NMTOKEN:
            ns = s_nsXSD;
            ln = "NMTOKEN";
            break;

        case css::rdf::URIs::XSD_NAME:
            ns = s_nsXSD;
            ln = "Name";
            break;

        case css::rdf::URIs::XSD_DURATION:
            ns = s_nsXSD;
            ln = "duration";
            break;

        case css::rdf::URIs::XSD_QNAME:
            ns = s_nsXSD;
            ln = "QName";
            break;

        case css::rdf::URIs::XSD_NOTATION:
            ns = s_nsXSD;
            ln = "NOTATION";
            break;

        case css::rdf::URIs::XSD_NMTOKENS:
            ns = s_nsXSD;
            ln = "NMTOKENS";
            break;

        case css::rdf::URIs::XSD_ID:
            ns = s_nsXSD;
            ln = "ID";
            break;

        case css::rdf::URIs::XSD_IDREF:
            ns = s_nsXSD;
            ln = "IDREF";
            break;

        case css::rdf::URIs::XSD_IDREFS:
            ns = s_nsXSD;
            ln = "IDREFS";
            break;

        case css::rdf::URIs::XSD_ENTITY:
            ns = s_nsXSD;
            ln = "ENTITY";
            break;

        case css::rdf::URIs::XSD_ENTITIES:
            ns = s_nsXSD;
            ln = "ENTITIES";
            break;

        case css::rdf::URIs::RDF_TYPE:
            ns = s_nsRDF;
            ln = "type";
            break;

        case css::rdf::URIs::RDF_SUBJECT:
            ns = s_nsRDF;
            ln = "subject";
            break;

        case css::rdf::URIs::RDF_PREDICATE:
            ns = s_nsRDF;
            ln = "predicate";
            break;

        case css::rdf::URIs::RDF_OBJECT:
            ns = s_nsRDF;
            ln = "object";
            break;

        case css::rdf::URIs::RDF_PROPERTY:
            ns = s_nsRDF;
            ln = "Property";
            break;

        case css::rdf::URIs::RDF_STATEMENT:
            ns = s_nsRDF;
            ln = "Statement";
            break;

        case css::rdf::URIs::RDF_VALUE:
            ns = s_nsRDF;
            ln = "value";
            break;

        case css::rdf::URIs::RDF_FIRST:
            ns = s_nsRDF;
            ln = "first";
            break;

        case css::rdf::URIs::RDF_REST:
            ns = s_nsRDF;
            ln = "rest";
            break;

        case css::rdf::URIs::RDF_NIL:
            ns = s_nsRDF;
            ln = "nil";
            break;

        case css::rdf::URIs::RDF_XMLLITERAL:
            ns = s_nsRDF;
            ln = "XMLLiteral";
            break;

        case css::rdf::URIs::RDF_ALT:
            ns = s_nsRDF;
            ln = "Alt";
            break;

        case css::rdf::URIs::RDF_BAG:
            ns = s_nsRDF;
            ln = "Bag";
            break;

        case css::rdf::URIs::RDF_LIST:
            ns = s_nsRDF;
            ln = "List";
            break;

        case css::rdf::URIs::RDF_SEQ:
            ns = s_nsRDF;
            ln = "Seq";
            break;

        case css::rdf::URIs::RDF_1:
            ns = s_nsRDF;
            ln = "_1";
            break;

        case css::rdf::URIs::RDFS_COMMENT:
            ns = s_nsRDFs;
            ln = "comment";
            break;

        case css::rdf::URIs::RDFS_LABEL:
            ns = s_nsRDFs;
            ln = "label";
            break;

        case css::rdf::URIs::RDFS_DOMAIN:
            ns = s_nsRDFs;
            ln = "domain";
            break;

        case css::rdf::URIs::RDFS_RANGE:
            ns = s_nsRDFs;
            ln = "range";
            break;

        case css::rdf::URIs::RDFS_SUBCLASSOF:
            ns = s_nsRDFs;
            ln = "subClassOf";
            break;

        case css::rdf::URIs::RDFS_LITERAL:
            ns = s_nsRDFs;
            ln = "Literal";
            break;

        case css::rdf::URIs::OWL_CLASS:
            ns = s_nsOWL;
            ln = "Class";
            break;

        case css::rdf::URIs::OWL_OBJECTPROPERTY:
            ns = s_nsOWL;
            ln = "ObjectProperty";
            break;

        case css::rdf::URIs::OWL_DATATYPEPROPERTY:
            ns = s_nsOWL;
            ln = "DatatypeProperty";
            break;

        case css::rdf::URIs::OWL_FUNCTIONALPROPERTY:
            ns = s_nsOWL;
            ln = "FunctionalProperty";
            break;

        case css::rdf::URIs::OWL_THING:
            ns = s_nsOWL;
            ln = "Thing";
            break;

        case css::rdf::URIs::OWL_NOTHING:
            ns = s_nsOWL;
            ln = "Nothing";
            break;

        case css::rdf::URIs::OWL_INDIVIDUAL:
            ns = s_nsOWL;
            ln = "Individual";
            break;

        case css::rdf::URIs::OWL_EQUIVALENTCLASS:
            ns = s_nsOWL;
            ln = "equivalentClass";
            break;

        case css::rdf::URIs::OWL_EQUIVALENTPROPERTY:
            ns = s_nsOWL;
            ln = "equivalentProperty";
            break;

        case css::rdf::URIs::OWL_SAMEAS:
            ns = s_nsOWL;
            ln = "sameAs";
            break;

        case css::rdf::URIs::OWL_DIFFERENTFROM:
            ns = s_nsOWL;
            ln = "differentFrom";
            break;

        case css::rdf::URIs::OWL_ALLDIFFERENT:
            ns = s_nsOWL;
            ln = "AllDifferent";
            break;

        case css::rdf::URIs::OWL_DISTINCTMEMBERS:
            ns = s_nsOWL;
            ln = "distinctMembers";
            break;

        case css::rdf::URIs::OWL_INVERSEOF:
            ns = s_nsOWL;
            ln = "inverseOf";
            break;

        case css::rdf::URIs::OWL_TRANSITIVEPROPERTY:
            ns = s_nsOWL;
            ln = "TransitiveProperty";
            break;

        case css::rdf::URIs::OWL_SYMMETRICPROPERTY:
            ns = s_nsOWL;
            ln = "SymmetricProperty";
            break;

        case css::rdf::URIs::OWL_INVERSEFUNCTIONALPROPERTY:
            ns = s_nsOWL;
            ln = "InverseFunctionalProperty";
            break;

        case css::rdf::URIs::OWL_RESTRICTION:
            ns = s_nsOWL;
            ln = "Restriction";
            break;

        case css::rdf::URIs::OWL_ONPROPERTY:
            ns = s_nsOWL;
            ln = "onProperty";
            break;

        case css::rdf::URIs::OWL_ALLVALUESFROM:
            ns = s_nsOWL;
            ln = "allValuesFrom";
            break;

        case css::rdf::URIs::OWL_SOMEVALUESFROM:
            ns = s_nsOWL;
            ln = "someValuesFrom";
            break;

        case css::rdf::URIs::OWL_MINCARDINALITY:
            ns = s_nsOWL;
            ln = "minCardinality";
            break;

        case css::rdf::URIs::OWL_MAXCARDINALITY:
            ns = s_nsOWL;
            ln = "maxCardinality";
            break;

        case css::rdf::URIs::OWL_CARDINALITY:
            ns = s_nsOWL;
            ln = "cardinality";
            break;

        case css::rdf::URIs::OWL_ONTOLOGY:
            ns = s_nsOWL;
            ln = "Ontology";
            break;

        case css::rdf::URIs::OWL_IMPORTS:
            ns = s_nsOWL;
            ln = "imports";
            break;

        case css::rdf::URIs::OWL_VERSIONINFO:
            ns = s_nsOWL;
            ln = "versionInfo";
            break;

        case css::rdf::URIs::OWL_PRIORVERSION:
            ns = s_nsOWL;
            ln = "priorVersion";
            break;

        case css::rdf::URIs::OWL_BACKWARDCOMPATIBLEWITH:
            ns = s_nsOWL;
            ln = "backwardCompatibleWith";
            break;

        case css::rdf::URIs::OWL_INCOMPATIBLEWITH:
            ns = s_nsOWL;
            ln = "incompatibleWith";
            break;

        case css::rdf::URIs::OWL_DEPRECATEDCLASS:
            ns = s_nsOWL;
            ln = "DeprecatedClass";
            break;

        case css::rdf::URIs::OWL_DEPRECATEDPROPERTY:
            ns = s_nsOWL;
            ln = "DeprecatedProperty";
            break;

        case css::rdf::URIs::OWL_ANNOTATIONPROPERTY:
            ns = s_nsOWL;
            ln = "AnnotationProperty";
            break;

        case css::rdf::URIs::OWL_ONTOLOGYPROPERTY:
            ns = s_nsOWL;
            ln = "OntologyProperty";
            break;

        case css::rdf::URIs::OWL_ONEOF:
            ns = s_nsOWL;
            ln = "oneOf";
            break;

        case css::rdf::URIs::OWL_DATARANGE:
            ns = s_nsOWL;
            ln = "dataRange";
            break;

        case css::rdf::URIs::OWL_DISJOINTWITH:
            ns = s_nsOWL;
            ln = "disjointWith";
            break;

        case css::rdf::URIs::OWL_UNIONOF:
            ns = s_nsOWL;
            ln = "unionOf";
            break;

        case css::rdf::URIs::OWL_COMPLEMENTOF:
            ns = s_nsOWL;
            ln = "complementOf";
            break;

        case css::rdf::URIs::OWL_INTERSECTIONOF:
            ns = s_nsOWL;
            ln = "intersectionOf";
            break;

        case css::rdf::URIs::OWL_HASVALUE:
            ns = s_nsOWL;
            ln = "hasValue";
            break;


        case css::rdf::URIs::PKG_HASPART:
            ns = s_nsPkg;
            ln = "hasPart";
            break;

        case css::rdf::URIs::PKG_MIMETYPE:
            ns = s_nsPkg;
            ln = "mimeType";
            break;

        case css::rdf::URIs::PKG_PACKAGE:
            ns = s_nsPkg;
            ln = "Package";
            break;

        case css::rdf::URIs::PKG_ELEMENT:
            ns = s_nsPkg;
            ln = "Element";
            break;

        case css::rdf::URIs::PKG_FILE:
            ns = s_nsPkg;
            ln = "File";
            break;

        case css::rdf::URIs::PKG_METADATAFILE:
            ns = s_nsPkg;
            ln = "MetadataFile";
            break;

        case css::rdf::URIs::PKG_DOCUMENT:
            ns = s_nsPkg;
            ln = "Document";
            break;

        case css::rdf::URIs::ODF_PREFIX:
            ns = s_nsODF;
            ln = "prefix";
            break;

        case css::rdf::URIs::ODF_SUFFIX:
            ns = s_nsODF;
            ln = "suffix";
            break;

        case css::rdf::URIs::ODF_ELEMENT:
            ns = s_nsODF;
            ln = "Element";
            break;

        case css::rdf::URIs::ODF_CONTENTFILE:
            ns = s_nsODF;
            ln = "ContentFile";
            break;

        case css::rdf::URIs::ODF_STYLESFILE:
            ns = s_nsODF;
            ln = "StylesFile";
            break;

        default:
            throw css::lang::IllegalArgumentException(
                ::rtl::OUString("CURI::initialize: "
                    "invalid URIs constant argument"), *this, 0);
    }
    m_Namespace = ::rtl::OUString::createFromAscii(ns).intern();
    m_LocalName = ::rtl::OUString::createFromAscii(ln).intern();
    return;
}

// ::com::sun::star::lang::XInitialization:
void SAL_CALL CURI::initialize(const css::uno::Sequence< ::com::sun::star::uno::Any > & aArguments) throw (css::uno::RuntimeException, css::uno::Exception)
{
    sal_Int32 len = aArguments.getLength();
    if ((len < 1) || (len > 2)) {
        throw css::lang::IllegalArgumentException(
            ::rtl::OUString("CURI::initialize: "
                "must give 1 or 2 argument(s)"), *this, 2);
    }

    sal_Int16 arg(0);
    ::rtl::OUString arg0;
    ::rtl::OUString arg1;
    if ((aArguments[0] >>= arg)) {
        // integer argument: constant from rdf::URIs
        if (len != 1) {
            throw css::lang::IllegalArgumentException(
                ::rtl::OUString("CURI::initialize: "
                    "must give 1 int argument"), *this, 1);
        }
        initFromConstant(arg);
        return;
    }
    if (!(aArguments[0] >>= arg0)) {
        throw css::lang::IllegalArgumentException(
            ::rtl::OUString("CURI::initialize: "
                "argument must be string or short"), *this, 0);
    }
    if (len > 1) {
        if (!(aArguments[1] >>= arg1)) {
            throw css::lang::IllegalArgumentException(
                ::rtl::OUString("CURI::initialize: "
                    "argument must be string"), *this, 1);
        }
        // just append the parameters and then split them again; seems simplest
        arg0 = arg0 + arg1;
        arg1 = ::rtl::OUString();
    }

    // split parameter
    sal_Int32 idx;
    if (    ((idx = arg0.indexOf    ('#')) >= 0)
        ||  ((idx = arg0.lastIndexOf('/')) >= 0)
        ||  ((idx = arg0.lastIndexOf(':')) >= 0))
    {

        if (idx < arg0.getLength() - 1) {
            arg1 = arg0.copy(idx+1);
            arg0 = arg0.copy(0, idx+1);
        }
    } else {
        throw css::lang::IllegalArgumentException(
            ::rtl::OUString("CURI::initialize: "
                "argument not splittable: no separator [#/:]"), *this, 0);
    }

    //FIXME: what is legal?
    if (!arg0.isEmpty()) {
        m_Namespace = arg0;
    } else {
        throw css::lang::IllegalArgumentException(
            ::rtl::OUString("CURI::initialize: "
                "argument is not valid namespace"), *this, 0);
    }
    //FIXME: what is legal?
    if (true) {
        m_LocalName = arg1;
    } else {
        throw css::lang::IllegalArgumentException(
            ::rtl::OUString("CURI::initialize: "
                "argument is not valid local name"), *this, 1);
    }
}

// ::com::sun::star::rdf::XNode:
::rtl::OUString SAL_CALL CURI::getStringValue() throw (css::uno::RuntimeException)
{
    return m_Namespace + m_LocalName;
}

// ::com::sun::star::rdf::XURI:
::rtl::OUString SAL_CALL CURI::getNamespace() throw (css::uno::RuntimeException)
{
    return m_Namespace;
}

::rtl::OUString SAL_CALL CURI::getLocalName() throw (css::uno::RuntimeException)
{
    return m_LocalName;
}

} // closing anonymous implementation namespace



// component helper namespace
namespace comp_CURI {

::rtl::OUString SAL_CALL _getImplementationName() {
    return ::rtl::OUString(
        "CURI");
}

css::uno::Sequence< ::rtl::OUString > SAL_CALL _getSupportedServiceNames()
{
    css::uno::Sequence< ::rtl::OUString > s(1);
    s[0] = ::rtl::OUString( "com.sun.star.rdf.URI");
    return s;
}

css::uno::Reference< css::uno::XInterface > SAL_CALL _create(
    const css::uno::Reference< css::uno::XComponentContext > & context)
        SAL_THROW((css::uno::Exception))
{
    return static_cast< ::cppu::OWeakObject * >(new CURI(context));
}

} // closing component helper namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

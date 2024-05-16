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

#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/rdf/XURI.hpp>
#include <com/sun/star/rdf/URIs.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <com/sun/star/lang/IllegalArgumentException.hpp>


/// anonymous implementation namespace
namespace {

class CURI:
    public ::cppu::WeakImplHelper<
        css::lang::XServiceInfo,
        css::lang::XInitialization,
        css::rdf::XURI>
{
public:
    explicit CURI();

    // css::lang::XServiceInfo:
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(const OUString & ServiceName) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    // css::lang::XInitialization:
    virtual void SAL_CALL initialize(const css::uno::Sequence< css::uno::Any > & aArguments) override;

    // css::rdf::XNode:
    virtual OUString SAL_CALL getStringValue() override;

    // css::rdf::XURI:
    virtual OUString SAL_CALL getLocalName() override;
    virtual OUString SAL_CALL getNamespace() override;

private:
    CURI(CURI const&) = delete;
    CURI& operator=(CURI const&) = delete;

    /// handle css.rdf.URIs
    void initFromConstant(const sal_Int16 i_Constant);

    OUString m_Namespace;
    OUString m_LocalName;
};

CURI::CURI()
{}

// com.sun.star.uno.XServiceInfo:
OUString SAL_CALL CURI::getImplementationName()
{
    return u"CURI"_ustr;
}

sal_Bool SAL_CALL CURI::supportsService(OUString const & serviceName)
{
    return cppu::supportsService(this, serviceName);
}

css::uno::Sequence< OUString > SAL_CALL CURI::getSupportedServiceNames()
{
    return { u"com.sun.star.rdf.URI"_ustr };
}

constexpr OUString s_nsXSD = u"http://www.w3.org/2001/XMLSchema-datatypes#"_ustr;
constexpr OUString s_nsRDF = u"http://www.w3.org/1999/02/22-rdf-syntax-ns#"_ustr;
constexpr OUString s_nsRDFs = u"http://www.w3.org/2000/01/rdf-schema#"_ustr;
constexpr OUString s_nsOWL = u"http://www.w3.org/2002/07/owl#"_ustr;
constexpr OUString s_nsPkg = u"http://docs.oasis-open.org/ns/office/1.2/meta/pkg#"_ustr;
constexpr OUString s_nsODF = u"http://docs.oasis-open.org/ns/office/1.2/meta/odf#"_ustr;
constexpr OUString s_nsLO_EXT =
    u"urn:org:documentfoundation:names:experimental:office:xmlns:loext:1.0odf#"_ustr;

void CURI::initFromConstant(const sal_Int16 i_Constant)
{
    OUString ns;
    OUString ln;
    switch (i_Constant)
    {
        case css::rdf::URIs::XSD_NCNAME:
            ns = s_nsXSD;
            ln = u"NCName"_ustr;
            break;

        case css::rdf::URIs::XSD_STRING:
            ns = s_nsXSD;
            ln = u"string"_ustr;
            break;

        case css::rdf::URIs::XSD_NORMALIZEDSTRING:
            ns = s_nsXSD;
            ln = u"normalizedString"_ustr;
            break;

        case css::rdf::URIs::XSD_BOOLEAN:
            ns = s_nsXSD;
            ln = u"boolean"_ustr;
            break;

        case css::rdf::URIs::XSD_DECIMAL:
            ns = s_nsXSD;
            ln = u"decimal"_ustr;
            break;

        case css::rdf::URIs::XSD_FLOAT:
            ns = s_nsXSD;
            ln = u"float"_ustr;
            break;

        case css::rdf::URIs::XSD_DOUBLE:
            ns = s_nsXSD;
            ln = u"double"_ustr;
            break;

        case css::rdf::URIs::XSD_INTEGER:
            ns = s_nsXSD;
            ln = u"integer"_ustr;
            break;

        case css::rdf::URIs::XSD_NONNEGATIVEINTEGER:
            ns = s_nsXSD;
            ln = u"nonNegativeInteger"_ustr;
            break;

        case css::rdf::URIs::XSD_POSITIVEINTEGER:
            ns = s_nsXSD;
            ln = u"positiveInteger"_ustr;
            break;

        case css::rdf::URIs::XSD_NONPOSITIVEINTEGER:
            ns = s_nsXSD;
            ln = u"nonPositiveInteger"_ustr;
            break;

        case css::rdf::URIs::XSD_NEGATIVEINTEGER:
            ns = s_nsXSD;
            ln = u"negativeInteger"_ustr;
            break;

        case css::rdf::URIs::XSD_LONG:
            ns = s_nsXSD;
            ln = u"long"_ustr;
            break;

        case css::rdf::URIs::XSD_INT:
            ns = s_nsXSD;
            ln = u"int"_ustr;
            break;

        case css::rdf::URIs::XSD_SHORT:
            ns = s_nsXSD;
            ln = u"short"_ustr;
            break;

        case css::rdf::URIs::XSD_BYTE:
            ns = s_nsXSD;
            ln = u"byte"_ustr;
            break;

        case css::rdf::URIs::XSD_UNSIGNEDLONG:
            ns = s_nsXSD;
            ln = u"unsignedLong"_ustr;
            break;

        case css::rdf::URIs::XSD_UNSIGNEDINT:
            ns = s_nsXSD;
            ln = u"unsignedInt"_ustr;
            break;

        case css::rdf::URIs::XSD_UNSIGNEDSHORT:
            ns = s_nsXSD;
            ln = u"unsignedShort"_ustr;
            break;

        case css::rdf::URIs::XSD_UNSIGNEDBYTE:
            ns = s_nsXSD;
            ln = u"unsignedByte"_ustr;
            break;

        case css::rdf::URIs::XSD_HEXBINARY:
            ns = s_nsXSD;
            ln = u"hexBinary"_ustr;
            break;

        case css::rdf::URIs::XSD_BASE64BINARY:
            ns = s_nsXSD;
            ln = u"base64Binary"_ustr;
            break;

        case css::rdf::URIs::XSD_DATETIME:
            ns = s_nsXSD;
            ln = u"dateTime"_ustr;
            break;

        case css::rdf::URIs::XSD_TIME:
            ns = s_nsXSD;
            ln = u"time"_ustr;
            break;

        case css::rdf::URIs::XSD_DATE:
            ns = s_nsXSD;
            ln = u"date"_ustr;
            break;

        case css::rdf::URIs::XSD_GYEARMONTH:
            ns = s_nsXSD;
            ln = u"gYearMonth"_ustr;
            break;

        case css::rdf::URIs::XSD_GYEAR:
            ns = s_nsXSD;
            ln = u"gYear"_ustr;
            break;

        case css::rdf::URIs::XSD_GMONTHDAY:
            ns = s_nsXSD;
            ln = u"gMonthDay"_ustr;
            break;

        case css::rdf::URIs::XSD_GDAY:
            ns = s_nsXSD;
            ln = u"gDay"_ustr;
            break;

        case css::rdf::URIs::XSD_GMONTH:
            ns = s_nsXSD;
            ln = u"gMonth"_ustr;
            break;

        case css::rdf::URIs::XSD_ANYURI:
            ns = s_nsXSD;
            ln = u"anyURI"_ustr;
            break;

        case css::rdf::URIs::XSD_TOKEN:
            ns = s_nsXSD;
            ln = u"token"_ustr;
            break;

        case css::rdf::URIs::XSD_LANGUAGE:
            ns = s_nsXSD;
            ln = u"language"_ustr;
            break;

        case css::rdf::URIs::XSD_NMTOKEN:
            ns = s_nsXSD;
            ln = u"NMTOKEN"_ustr;
            break;

        case css::rdf::URIs::XSD_NAME:
            ns = s_nsXSD;
            ln = u"Name"_ustr;
            break;

        case css::rdf::URIs::XSD_DURATION:
            ns = s_nsXSD;
            ln = u"duration"_ustr;
            break;

        case css::rdf::URIs::XSD_QNAME:
            ns = s_nsXSD;
            ln = u"QName"_ustr;
            break;

        case css::rdf::URIs::XSD_NOTATION:
            ns = s_nsXSD;
            ln = u"NOTATION"_ustr;
            break;

        case css::rdf::URIs::XSD_NMTOKENS:
            ns = s_nsXSD;
            ln = u"NMTOKENS"_ustr;
            break;

        case css::rdf::URIs::XSD_ID:
            ns = s_nsXSD;
            ln = u"ID"_ustr;
            break;

        case css::rdf::URIs::XSD_IDREF:
            ns = s_nsXSD;
            ln = u"IDREF"_ustr;
            break;

        case css::rdf::URIs::XSD_IDREFS:
            ns = s_nsXSD;
            ln = u"IDREFS"_ustr;
            break;

        case css::rdf::URIs::XSD_ENTITY:
            ns = s_nsXSD;
            ln = u"ENTITY"_ustr;
            break;

        case css::rdf::URIs::XSD_ENTITIES:
            ns = s_nsXSD;
            ln = u"ENTITIES"_ustr;
            break;

        case css::rdf::URIs::RDF_TYPE:
            ns = s_nsRDF;
            ln = u"type"_ustr;
            break;

        case css::rdf::URIs::RDF_SUBJECT:
            ns = s_nsRDF;
            ln = u"subject"_ustr;
            break;

        case css::rdf::URIs::RDF_PREDICATE:
            ns = s_nsRDF;
            ln = u"predicate"_ustr;
            break;

        case css::rdf::URIs::RDF_OBJECT:
            ns = s_nsRDF;
            ln = u"object"_ustr;
            break;

        case css::rdf::URIs::RDF_PROPERTY:
            ns = s_nsRDF;
            ln = u"Property"_ustr;
            break;

        case css::rdf::URIs::RDF_STATEMENT:
            ns = s_nsRDF;
            ln = u"Statement"_ustr;
            break;

        case css::rdf::URIs::RDF_VALUE:
            ns = s_nsRDF;
            ln = u"value"_ustr;
            break;

        case css::rdf::URIs::RDF_FIRST:
            ns = s_nsRDF;
            ln = u"first"_ustr;
            break;

        case css::rdf::URIs::RDF_REST:
            ns = s_nsRDF;
            ln = u"rest"_ustr;
            break;

        case css::rdf::URIs::RDF_NIL:
            ns = s_nsRDF;
            ln = u"nil"_ustr;
            break;

        case css::rdf::URIs::RDF_XMLLITERAL:
            ns = s_nsRDF;
            ln = u"XMLLiteral"_ustr;
            break;

        case css::rdf::URIs::RDF_ALT:
            ns = s_nsRDF;
            ln = u"Alt"_ustr;
            break;

        case css::rdf::URIs::RDF_BAG:
            ns = s_nsRDF;
            ln = u"Bag"_ustr;
            break;

        case css::rdf::URIs::RDF_LIST:
            ns = s_nsRDF;
            ln = u"List"_ustr;
            break;

        case css::rdf::URIs::RDF_SEQ:
            ns = s_nsRDF;
            ln = u"Seq"_ustr;
            break;

        case css::rdf::URIs::RDF_1:
            ns = s_nsRDF;
            ln = u"_1"_ustr;
            break;

        case css::rdf::URIs::RDFS_COMMENT:
            ns = s_nsRDFs;
            ln = u"comment"_ustr;
            break;

        case css::rdf::URIs::RDFS_LABEL:
            ns = s_nsRDFs;
            ln = u"label"_ustr;
            break;

        case css::rdf::URIs::RDFS_DOMAIN:
            ns = s_nsRDFs;
            ln = u"domain"_ustr;
            break;

        case css::rdf::URIs::RDFS_RANGE:
            ns = s_nsRDFs;
            ln = u"range"_ustr;
            break;

        case css::rdf::URIs::RDFS_SUBCLASSOF:
            ns = s_nsRDFs;
            ln = u"subClassOf"_ustr;
            break;

        case css::rdf::URIs::RDFS_LITERAL:
            ns = s_nsRDFs;
            ln = u"Literal"_ustr;
            break;

        case css::rdf::URIs::OWL_CLASS:
            ns = s_nsOWL;
            ln = u"Class"_ustr;
            break;

        case css::rdf::URIs::OWL_OBJECTPROPERTY:
            ns = s_nsOWL;
            ln = u"ObjectProperty"_ustr;
            break;

        case css::rdf::URIs::OWL_DATATYPEPROPERTY:
            ns = s_nsOWL;
            ln = u"DatatypeProperty"_ustr;
            break;

        case css::rdf::URIs::OWL_FUNCTIONALPROPERTY:
            ns = s_nsOWL;
            ln = u"FunctionalProperty"_ustr;
            break;

        case css::rdf::URIs::OWL_THING:
            ns = s_nsOWL;
            ln = u"Thing"_ustr;
            break;

        case css::rdf::URIs::OWL_NOTHING:
            ns = s_nsOWL;
            ln = u"Nothing"_ustr;
            break;

        case css::rdf::URIs::OWL_INDIVIDUAL:
            ns = s_nsOWL;
            ln = u"Individual"_ustr;
            break;

        case css::rdf::URIs::OWL_EQUIVALENTCLASS:
            ns = s_nsOWL;
            ln = u"equivalentClass"_ustr;
            break;

        case css::rdf::URIs::OWL_EQUIVALENTPROPERTY:
            ns = s_nsOWL;
            ln = u"equivalentProperty"_ustr;
            break;

        case css::rdf::URIs::OWL_SAMEAS:
            ns = s_nsOWL;
            ln = u"sameAs"_ustr;
            break;

        case css::rdf::URIs::OWL_DIFFERENTFROM:
            ns = s_nsOWL;
            ln = u"differentFrom"_ustr;
            break;

        case css::rdf::URIs::OWL_ALLDIFFERENT:
            ns = s_nsOWL;
            ln = u"AllDifferent"_ustr;
            break;

        case css::rdf::URIs::OWL_DISTINCTMEMBERS:
            ns = s_nsOWL;
            ln = u"distinctMembers"_ustr;
            break;

        case css::rdf::URIs::OWL_INVERSEOF:
            ns = s_nsOWL;
            ln = u"inverseOf"_ustr;
            break;

        case css::rdf::URIs::OWL_TRANSITIVEPROPERTY:
            ns = s_nsOWL;
            ln = u"TransitiveProperty"_ustr;
            break;

        case css::rdf::URIs::OWL_SYMMETRICPROPERTY:
            ns = s_nsOWL;
            ln = u"SymmetricProperty"_ustr;
            break;

        case css::rdf::URIs::OWL_INVERSEFUNCTIONALPROPERTY:
            ns = s_nsOWL;
            ln = u"InverseFunctionalProperty"_ustr;
            break;

        case css::rdf::URIs::OWL_RESTRICTION:
            ns = s_nsOWL;
            ln = u"Restriction"_ustr;
            break;

        case css::rdf::URIs::OWL_ONPROPERTY:
            ns = s_nsOWL;
            ln = u"onProperty"_ustr;
            break;

        case css::rdf::URIs::OWL_ALLVALUESFROM:
            ns = s_nsOWL;
            ln = u"allValuesFrom"_ustr;
            break;

        case css::rdf::URIs::OWL_SOMEVALUESFROM:
            ns = s_nsOWL;
            ln = u"someValuesFrom"_ustr;
            break;

        case css::rdf::URIs::OWL_MINCARDINALITY:
            ns = s_nsOWL;
            ln = u"minCardinality"_ustr;
            break;

        case css::rdf::URIs::OWL_MAXCARDINALITY:
            ns = s_nsOWL;
            ln = u"maxCardinality"_ustr;
            break;

        case css::rdf::URIs::OWL_CARDINALITY:
            ns = s_nsOWL;
            ln = u"cardinality"_ustr;
            break;

        case css::rdf::URIs::OWL_ONTOLOGY:
            ns = s_nsOWL;
            ln = u"Ontology"_ustr;
            break;

        case css::rdf::URIs::OWL_IMPORTS:
            ns = s_nsOWL;
            ln = u"imports"_ustr;
            break;

        case css::rdf::URIs::OWL_VERSIONINFO:
            ns = s_nsOWL;
            ln = u"versionInfo"_ustr;
            break;

        case css::rdf::URIs::OWL_PRIORVERSION:
            ns = s_nsOWL;
            ln = u"priorVersion"_ustr;
            break;

        case css::rdf::URIs::OWL_BACKWARDCOMPATIBLEWITH:
            ns = s_nsOWL;
            ln = u"backwardCompatibleWith"_ustr;
            break;

        case css::rdf::URIs::OWL_INCOMPATIBLEWITH:
            ns = s_nsOWL;
            ln = u"incompatibleWith"_ustr;
            break;

        case css::rdf::URIs::OWL_DEPRECATEDCLASS:
            ns = s_nsOWL;
            ln = u"DeprecatedClass"_ustr;
            break;

        case css::rdf::URIs::OWL_DEPRECATEDPROPERTY:
            ns = s_nsOWL;
            ln = u"DeprecatedProperty"_ustr;
            break;

        case css::rdf::URIs::OWL_ANNOTATIONPROPERTY:
            ns = s_nsOWL;
            ln = u"AnnotationProperty"_ustr;
            break;

        case css::rdf::URIs::OWL_ONTOLOGYPROPERTY:
            ns = s_nsOWL;
            ln = u"OntologyProperty"_ustr;
            break;

        case css::rdf::URIs::OWL_ONEOF:
            ns = s_nsOWL;
            ln = u"oneOf"_ustr;
            break;

        case css::rdf::URIs::OWL_DATARANGE:
            ns = s_nsOWL;
            ln = u"dataRange"_ustr;
            break;

        case css::rdf::URIs::OWL_DISJOINTWITH:
            ns = s_nsOWL;
            ln = u"disjointWith"_ustr;
            break;

        case css::rdf::URIs::OWL_UNIONOF:
            ns = s_nsOWL;
            ln = u"unionOf"_ustr;
            break;

        case css::rdf::URIs::OWL_COMPLEMENTOF:
            ns = s_nsOWL;
            ln = u"complementOf"_ustr;
            break;

        case css::rdf::URIs::OWL_INTERSECTIONOF:
            ns = s_nsOWL;
            ln = u"intersectionOf"_ustr;
            break;

        case css::rdf::URIs::OWL_HASVALUE:
            ns = s_nsOWL;
            ln = u"hasValue"_ustr;
            break;


        case css::rdf::URIs::PKG_HASPART:
            ns = s_nsPkg;
            ln = u"hasPart"_ustr;
            break;

        case css::rdf::URIs::PKG_MIMETYPE:
            ns = s_nsPkg;
            ln = u"mimeType"_ustr;
            break;

        case css::rdf::URIs::PKG_PACKAGE:
            ns = s_nsPkg;
            ln = u"Package"_ustr;
            break;

        case css::rdf::URIs::PKG_ELEMENT:
            ns = s_nsPkg;
            ln = u"Element"_ustr;
            break;

        case css::rdf::URIs::PKG_FILE:
            ns = s_nsPkg;
            ln = u"File"_ustr;
            break;

        case css::rdf::URIs::PKG_METADATAFILE:
            ns = s_nsPkg;
            ln = u"MetadataFile"_ustr;
            break;

        case css::rdf::URIs::PKG_DOCUMENT:
            ns = s_nsPkg;
            ln = u"Document"_ustr;
            break;

        case css::rdf::URIs::ODF_PREFIX:
            ns = s_nsODF;
            ln = u"prefix"_ustr;
            break;

        case css::rdf::URIs::ODF_SUFFIX:
            ns = s_nsODF;
            ln = u"suffix"_ustr;
            break;

        case css::rdf::URIs::ODF_ELEMENT:
            ns = s_nsODF;
            ln = u"Element"_ustr;
            break;

        case css::rdf::URIs::ODF_CONTENTFILE:
            ns = s_nsODF;
            ln = u"ContentFile"_ustr;
            break;

        case css::rdf::URIs::ODF_STYLESFILE:
            ns = s_nsODF;
            ln = u"StylesFile"_ustr;
            break;

        case css::rdf::URIs::LO_EXT_SHADING:
            ns = s_nsLO_EXT;
            ln = u"shading"_ustr;
            break;

        default:
            throw css::lang::IllegalArgumentException(
                u"CURI::initialize: invalid URIs constant argument"_ustr, *this, 0);
    }
    m_Namespace = ns;
    m_LocalName = ln;
}

// css::lang::XInitialization:
void SAL_CALL CURI::initialize(const css::uno::Sequence< css::uno::Any > & aArguments)
{
    sal_Int32 len = aArguments.getLength();
    if ((len < 1) || (len > 2)) {
        throw css::lang::IllegalArgumentException(
            u"CURI::initialize: must give 1 or 2 argument(s)"_ustr, *this, 2);
    }

    sal_Int16 arg(0);
    OUString arg0;
    OUString arg1;
    if (aArguments[0] >>= arg) {
        // integer argument: constant from rdf::URIs
        if (len != 1) {
            throw css::lang::IllegalArgumentException(
                u"CURI::initialize: must give 1 int argument"_ustr, *this, 1);
        }
        initFromConstant(arg);
        return;
    }
    if (!(aArguments[0] >>= arg0)) {
        throw css::lang::IllegalArgumentException(
            u"CURI::initialize: argument must be string or short"_ustr, *this, 0);
    }
    if (len > 1) {
        if (!(aArguments[1] >>= arg1)) {
            throw css::lang::IllegalArgumentException(
                u"CURI::initialize: argument must be string"_ustr, *this, 1);
        }
        // just append the parameters and then split them again; seems simplest
        arg0 = arg0 + arg1;
        arg1.clear();
    }

    // split parameter
    sal_Int32 idx = arg0.indexOf('#');
    if (idx < 0)
        idx = arg0.lastIndexOf('/');
    if (idx < 0)
        idx = arg0.lastIndexOf(':');
    if (idx < 0)
    {
        throw css::lang::IllegalArgumentException(
            u"CURI::initialize: argument not splittable: no separator [#/:]"_ustr, *this, 0);
    }
    if (idx < arg0.getLength() - 1) {
        arg1 = arg0.copy(idx+1);
        arg0 = arg0.copy(0, idx+1);
    }

    //FIXME: what is legal?
    if (arg0.isEmpty()) {
        throw css::lang::IllegalArgumentException(
            u"CURI::initialize: argument is not valid namespace"_ustr, *this, 0);
    }
    m_Namespace = arg0;

    //FIXME: what is legal?
    if ((false)) {
        throw css::lang::IllegalArgumentException(
            u"CURI::initialize: argument is not valid local name"_ustr, *this, 1);
    }
    m_LocalName = arg1;
}

// css::rdf::XNode:
OUString SAL_CALL CURI::getStringValue()
{
    return m_Namespace + m_LocalName;
}

// css::rdf::XURI:
OUString SAL_CALL CURI::getNamespace()
{
    return m_Namespace;
}

OUString SAL_CALL CURI::getLocalName()
{
    return m_LocalName;
}

} // closing anonymous implementation namespace


extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
unoxml_CURI_get_implementation(
    css::uno::XComponentContext* , css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new CURI());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

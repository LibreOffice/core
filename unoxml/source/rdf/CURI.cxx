/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: CURI.cxx,v $
 * $Revision: 1.2 $
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
    "http://docs.oasis-open.org/opendocument/meta/package/common#";
const char s_nsODF      [] =
    "http://docs.oasis-open.org/opendocument/meta/package/odf#";

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

        case css::rdf::URIs::PKG_HASPART:
            ns = s_nsPkg;
            ln = "hasPart";
            break;

        case css::rdf::URIs::PKG_IDREF:
            ns = s_nsPkg;
            ln = "idref";
            break;

        case css::rdf::URIs::PKG_PATH:
            ns = s_nsPkg;
            ln = "path";
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

        case css::rdf::URIs::ODF_METADATAFILE:
            ns = s_nsODF;
            ln = "MetadataFile";
            break;

        default:
            throw css::lang::IllegalArgumentException(
                ::rtl::OUString::createFromAscii("CURI::initialize: "
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
            ::rtl::OUString::createFromAscii("CURI::initialize: "
                "must give 1 or 2 argument(s)"), *this, 2);
    }

    sal_Int16 arg(0);
    ::rtl::OUString arg0;
    ::rtl::OUString arg1;
    if ((aArguments[0] >>= arg)) {
        // integer argument: constant from rdf::URIs
        if (len != 1) {
            throw css::lang::IllegalArgumentException(
                ::rtl::OUString::createFromAscii("CURI::initialize: "
                    "must give 1 int argument"), *this, 1);
        }
        initFromConstant(arg);
        return;
    }
    if (!(aArguments[0] >>= arg0)) {
        throw css::lang::IllegalArgumentException(
            ::rtl::OUString::createFromAscii("CURI::initialize: "
                "argument must be string or short"), *this, 0);
    }
    if (len > 1) {
        if (!(aArguments[1] >>= arg1)) {
            throw css::lang::IllegalArgumentException(
                ::rtl::OUString::createFromAscii("CURI::initialize: "
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
            ::rtl::OUString::createFromAscii("CURI::initialize: "
                "argument not splittable: no separator [#/:]"), *this, 0);
    }

    //FIXME: what is legal?
    if (arg0.getLength() > 0) {
        m_Namespace = arg0;
    } else {
        throw css::lang::IllegalArgumentException(
            ::rtl::OUString::createFromAscii("CURI::initialize: "
                "argument is not valid namespace"), *this, 0);
    }
    //FIXME: what is legal?
    if (true) {
        m_LocalName = arg1;
    } else {
        throw css::lang::IllegalArgumentException(
            ::rtl::OUString::createFromAscii("CURI::initialize: "
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
    return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
        "CURI"));
}

css::uno::Sequence< ::rtl::OUString > SAL_CALL _getSupportedServiceNames()
{
    css::uno::Sequence< ::rtl::OUString > s(1);
    s[0] = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(
        "com.sun.star.rdf.URI"));
    return s;
}

css::uno::Reference< css::uno::XInterface > SAL_CALL _create(
    const css::uno::Reference< css::uno::XComponentContext > & context)
        SAL_THROW((css::uno::Exception))
{
    return static_cast< ::cppu::OWeakObject * >(new CURI(context));
}

} // closing component helper namespace


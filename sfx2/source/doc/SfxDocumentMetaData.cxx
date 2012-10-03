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


#include "sal/config.h"
#include "cppuhelper/factory.hxx"
#include "cppuhelper/implementationentry.hxx"
#include "cppuhelper/compbase6.hxx"
#include "com/sun/star/lang/XServiceInfo.hpp"
#include "com/sun/star/document/XDocumentProperties.hpp"
#include "com/sun/star/lang/XInitialization.hpp"
#include "com/sun/star/util/XCloneable.hpp"
#include "com/sun/star/util/XModifiable.hpp"
#include "com/sun/star/xml/sax/XSAXSerializable.hpp"

#include "com/sun/star/lang/WrappedTargetRuntimeException.hpp"
#include "com/sun/star/lang/EventObject.hpp"
#include "com/sun/star/beans/XPropertySet.hpp"
#include "com/sun/star/beans/XPropertySetInfo.hpp"
#include "com/sun/star/beans/PropertyAttribute.hpp"
#include "com/sun/star/task/ErrorCodeIOException.hpp"
#include "com/sun/star/embed/XStorage.hpp"
#include "com/sun/star/embed/XTransactedObject.hpp"
#include "com/sun/star/embed/ElementModes.hpp"
#include "com/sun/star/io/XActiveDataControl.hpp"
#include "com/sun/star/io/XActiveDataSource.hpp"
#include "com/sun/star/io/XStream.hpp"
#include "com/sun/star/document/XImporter.hpp"
#include "com/sun/star/document/XExporter.hpp"
#include "com/sun/star/document/XFilter.hpp"
#include "com/sun/star/xml/sax/Parser.hpp"
#include "com/sun/star/xml/dom/XDocument.hpp"
#include "com/sun/star/xml/dom/XElement.hpp"
#include "com/sun/star/xml/dom/DocumentBuilder.hpp"
#include "com/sun/star/xml/dom/XSAXDocumentBuilder.hpp"
#include "com/sun/star/xml/dom/NodeType.hpp"
#include "com/sun/star/xml/xpath/XXPathAPI.hpp"
#include "com/sun/star/util/Date.hpp"
#include "com/sun/star/util/Time.hpp"
#include "com/sun/star/util/Duration.hpp"

#include "SfxDocumentMetaData.hxx"
#include "rtl/ustrbuf.hxx"
#include "tools/debug.hxx"
#include "tools/datetime.hxx"
#include "osl/mutex.hxx"
#include "cppuhelper/basemutex.hxx"
#include "cppuhelper/interfacecontainer.hxx"
#include "comphelper/storagehelper.hxx"
#include "comphelper/mediadescriptor.hxx"
#include "comphelper/sequenceasvector.hxx"
#include "comphelper/stlunosequence.hxx"
#include "sot/storage.hxx"
#include "sfx2/docfile.hxx"
#include "sax/tools/converter.hxx"

#include <utility>
#include <vector>
#include <map>
#include <cstring>
#include <limits>


#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/document/XCompatWriterDocProperties.hpp>

/**
 * This file contains the implementation of the service
 * com.sun.star.document.DocumentProperties.
 * This service enables access to the meta-data stored in documents.
 * Currently, this service only handles documents in ODF format.
 *
 * The implementation uses an XML DOM to store the properties.
 * This approach was taken because it allows for preserving arbitrary XML data
 * in loaded documents, which will be stored unmodified when saving the
 * document again.
 *
 * Upon access, some properties are directly read from and updated in the DOM.
 * Exception: it seems impossible to get notified upon addition of a property
 * to a com.sun.star.beans.PropertyBag, which is used for storing user-defined
 * properties; because of this, user-defined properties are updated in the
 * XML DOM only when storing the document.
 * Exception 2: when setting certain properties which correspond to attributes
 * in the XML DOM, we want to remove the corresponding XML element. Detecting
 * this condition can get messy, so we store all such properties as members,
 * and update the DOM tree only when storing the document (in
 * <method>updateUserDefinedAndAttributes</method>).
 *
 * @author mst
 */

/// anonymous implementation namespace
namespace {

namespace css = ::com::sun::star;


/// a list of attribute-lists, where attribute means name and content
typedef std::vector<std::vector<std::pair<const char*, ::rtl::OUString> > >
        AttrVector;

typedef ::cppu::WeakComponentImplHelper6<
            css::lang::XServiceInfo,
            css::document::XDocumentProperties,
            css::lang::XInitialization,
            css::util::XCloneable,
            css::util::XModifiable,
            css::xml::sax::XSAXSerializable>
    SfxDocumentMetaData_Base;

class SfxDocumentMetaData:
    private ::cppu::BaseMutex,
    public SfxDocumentMetaData_Base
{
public:
    explicit SfxDocumentMetaData(
        css::uno::Reference< css::uno::XComponentContext > const & context);

    // ::com::sun::star::lang::XServiceInfo:
    virtual ::rtl::OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL supportsService(
        const ::rtl::OUString & ServiceName) throw (css::uno::RuntimeException);
    virtual css::uno::Sequence< ::rtl::OUString > SAL_CALL
        getSupportedServiceNames() throw (css::uno::RuntimeException);

    // ::com::sun::star::lang::XComponent:
    virtual void SAL_CALL dispose() throw (css::uno::RuntimeException);

    // ::com::sun::star::document::XDocumentProperties:
    virtual ::rtl::OUString SAL_CALL getAuthor()
        throw (css::uno::RuntimeException);
    virtual void SAL_CALL setAuthor(const ::rtl::OUString & the_value)
        throw (css::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getGenerator()
        throw (css::uno::RuntimeException);
    virtual void SAL_CALL setGenerator(const ::rtl::OUString & the_value)
        throw (css::uno::RuntimeException);
    virtual css::util::DateTime SAL_CALL getCreationDate()
        throw (css::uno::RuntimeException);
    virtual void SAL_CALL setCreationDate(const css::util::DateTime & the_value)
        throw (css::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getTitle()
        throw (css::uno::RuntimeException);
    virtual void SAL_CALL setTitle(const ::rtl::OUString & the_value)
        throw (css::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getSubject()
        throw (css::uno::RuntimeException);
    virtual void SAL_CALL setSubject(const ::rtl::OUString & the_value)
        throw (css::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getDescription()
        throw (css::uno::RuntimeException);
    virtual void SAL_CALL setDescription(const ::rtl::OUString & the_value)
        throw (css::uno::RuntimeException);
    virtual css::uno::Sequence< ::rtl::OUString > SAL_CALL getKeywords()
        throw (css::uno::RuntimeException);
    virtual void SAL_CALL setKeywords(
        const css::uno::Sequence< ::rtl::OUString > & the_value)
        throw (css::uno::RuntimeException);
    virtual css::lang::Locale SAL_CALL getLanguage()
        throw (css::uno::RuntimeException);
    virtual void SAL_CALL setLanguage(const css::lang::Locale & the_value)
        throw (css::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getModifiedBy()
        throw (css::uno::RuntimeException);
    virtual void SAL_CALL setModifiedBy(const ::rtl::OUString & the_value)
        throw (css::uno::RuntimeException);
    virtual css::util::DateTime SAL_CALL getModificationDate()
        throw (css::uno::RuntimeException);
    virtual void SAL_CALL setModificationDate(
            const css::util::DateTime & the_value)
        throw (css::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getPrintedBy()
        throw (css::uno::RuntimeException);
    virtual void SAL_CALL setPrintedBy(const ::rtl::OUString & the_value)
        throw (css::uno::RuntimeException);
    virtual css::util::DateTime SAL_CALL getPrintDate()
        throw (css::uno::RuntimeException);
    virtual void SAL_CALL setPrintDate(const css::util::DateTime & the_value)
        throw (css::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getTemplateName()
        throw (css::uno::RuntimeException);
    virtual void SAL_CALL setTemplateName(const ::rtl::OUString & the_value)
        throw (css::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getTemplateURL()
        throw (css::uno::RuntimeException);
    virtual void SAL_CALL setTemplateURL(const ::rtl::OUString & the_value)
        throw (css::uno::RuntimeException);
    virtual css::util::DateTime SAL_CALL getTemplateDate()
        throw (css::uno::RuntimeException);
    virtual void SAL_CALL setTemplateDate(const css::util::DateTime & the_value)
        throw (css::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getAutoloadURL()
        throw (css::uno::RuntimeException);
    virtual void SAL_CALL setAutoloadURL(const ::rtl::OUString & the_value)
        throw (css::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL getAutoloadSecs()
        throw (css::uno::RuntimeException);
    virtual void SAL_CALL setAutoloadSecs(::sal_Int32 the_value)
        throw (css::uno::RuntimeException, css::lang::IllegalArgumentException);
    virtual ::rtl::OUString SAL_CALL getDefaultTarget()
        throw (css::uno::RuntimeException);
    virtual void SAL_CALL setDefaultTarget(const ::rtl::OUString & the_value)
        throw (css::uno::RuntimeException);
    virtual css::uno::Sequence< css::beans::NamedValue > SAL_CALL
        getDocumentStatistics() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setDocumentStatistics(
        const css::uno::Sequence< css::beans::NamedValue > & the_value)
        throw (css::uno::RuntimeException);
    virtual ::sal_Int16 SAL_CALL getEditingCycles()
        throw (css::uno::RuntimeException);
    virtual void SAL_CALL setEditingCycles(::sal_Int16 the_value)
        throw (css::uno::RuntimeException, css::lang::IllegalArgumentException);
    virtual ::sal_Int32 SAL_CALL getEditingDuration()
        throw (css::uno::RuntimeException);
    virtual void SAL_CALL setEditingDuration(::sal_Int32 the_value)
        throw (css::uno::RuntimeException, css::lang::IllegalArgumentException);
    virtual void SAL_CALL resetUserData(const ::rtl::OUString & the_value)
        throw (css::uno::RuntimeException);
    virtual css::uno::Reference< css::beans::XPropertyContainer > SAL_CALL
        getUserDefinedProperties() throw (css::uno::RuntimeException);
    virtual void SAL_CALL loadFromStorage(
        const css::uno::Reference< css::embed::XStorage > & Storage,
        const css::uno::Sequence< css::beans::PropertyValue > & Medium)
        throw (css::uno::RuntimeException, css::lang::IllegalArgumentException,
               css::io::WrongFormatException,
               css::lang::WrappedTargetException, css::io::IOException);
    virtual void SAL_CALL loadFromMedium(const ::rtl::OUString & URL,
        const css::uno::Sequence< css::beans::PropertyValue > & Medium)
        throw (css::uno::RuntimeException,
               css::io::WrongFormatException,
               css::lang::WrappedTargetException, css::io::IOException);
    virtual void SAL_CALL storeToStorage(
        const css::uno::Reference< css::embed::XStorage > & Storage,
        const css::uno::Sequence< css::beans::PropertyValue > & Medium)
        throw (css::uno::RuntimeException, css::lang::IllegalArgumentException,
               css::lang::WrappedTargetException, css::io::IOException);
    virtual void SAL_CALL storeToMedium(const ::rtl::OUString & URL,
        const css::uno::Sequence< css::beans::PropertyValue > & Medium)
        throw (css::uno::RuntimeException,
               css::lang::WrappedTargetException, css::io::IOException);

    // ::com::sun::star::lang::XInitialization:
    virtual void SAL_CALL initialize(
        const css::uno::Sequence< css::uno::Any > & aArguments)
        throw (css::uno::RuntimeException, css::uno::Exception);

    // ::com::sun::star::util::XCloneable:
    virtual css::uno::Reference<css::util::XCloneable> SAL_CALL createClone()
        throw (css::uno::RuntimeException);

    // ::com::sun::star::util::XModifiable:
    virtual ::sal_Bool SAL_CALL isModified(  )
        throw (css::uno::RuntimeException);
    virtual void SAL_CALL setModified( ::sal_Bool bModified )
        throw (css::beans::PropertyVetoException, css::uno::RuntimeException);

    // ::com::sun::star::util::XModifyBroadcaster:
    virtual void SAL_CALL addModifyListener(
        const css::uno::Reference< css::util::XModifyListener > & xListener)
        throw (css::uno::RuntimeException);
    virtual void SAL_CALL removeModifyListener(
        const css::uno::Reference< css::util::XModifyListener > & xListener)
        throw (css::uno::RuntimeException);

    // ::com::sun::star::xml::sax::XSAXSerializable
    virtual void SAL_CALL serialize(
        const css::uno::Reference<css::xml::sax::XDocumentHandler>& i_xHandler,
        const css::uno::Sequence< css::beans::StringPair >& i_rNamespaces)
        throw (css::uno::RuntimeException, css::xml::sax::SAXException);

protected:
    SfxDocumentMetaData(SfxDocumentMetaData &); // not defined
    SfxDocumentMetaData& operator =(SfxDocumentMetaData &); // not defined

    virtual ~SfxDocumentMetaData() {}
    virtual SfxDocumentMetaData* createMe( css::uno::Reference< css::uno::XComponentContext > const & context ) { return new SfxDocumentMetaData( context ); };
    const css::uno::Reference< css::uno::XComponentContext > m_xContext;

    /// for notification
    ::cppu::OInterfaceContainerHelper m_NotifyListeners;
    /// flag: false means not initialized yet, or disposed
    bool m_isInitialized;
    /// flag
    bool m_isModified;
    /// meta-data DOM tree
    css::uno::Reference< css::xml::dom::XDocument > m_xDoc;
    /// meta-data super node in the meta-data DOM tree
    css::uno::Reference< css::xml::dom::XNode> m_xParent;
    /// standard meta data (single occurrence)
    std::map< ::rtl::OUString, css::uno::Reference<css::xml::dom::XNode> >
        m_meta;
    /// standard meta data (multiple occurrences)
    std::map< ::rtl::OUString,
        std::vector<css::uno::Reference<css::xml::dom::XNode> > > m_metaList;
    /// user-defined meta data (meta:user-defined) @ATTENTION may be null!
    css::uno::Reference<css::beans::XPropertyContainer> m_xUserDefined;
    // now for some meta-data attributes; these are not updated directly in the
    // DOM because updates (detecting "empty" elements) would be quite messy
    ::rtl::OUString m_TemplateName;
    ::rtl::OUString m_TemplateURL;
    css::util::DateTime m_TemplateDate;
    ::rtl::OUString m_AutoloadURL;
    sal_Int32 m_AutoloadSecs;
    ::rtl::OUString m_DefaultTarget;

    /// check if we are initialized properly
    void SAL_CALL checkInit() const;
    /// initialize state from given DOM tree
    void SAL_CALL init(css::uno::Reference<css::xml::dom::XDocument> i_xDom);
    /// update element in DOM tree
    void SAL_CALL updateElement(const char *i_name,
        std::vector<std::pair<const char *, ::rtl::OUString> >* i_pAttrs = 0);
    /// update user-defined meta data and attributes in DOM tree
    void SAL_CALL updateUserDefinedAndAttributes();
    /// create empty DOM tree (XDocument)
    css::uno::Reference<css::xml::dom::XDocument> SAL_CALL createDOM() const;
    /// extract base URL (necessary for converting relative links)
    css::uno::Reference<css::beans::XPropertySet> SAL_CALL getURLProperties(
        const css::uno::Sequence<css::beans::PropertyValue> & i_rMedium) const;
    /// get text of standard meta data element
    ::rtl::OUString SAL_CALL getMetaText(const char* i_name) const;
    /// set text of standard meta data element iff not equal to existing text
    bool SAL_CALL setMetaText(const char* i_name,
        const ::rtl::OUString & i_rValue);
    /// set text of standard meta data element iff not equal to existing text
    void SAL_CALL setMetaTextAndNotify(const char* i_name,
        const ::rtl::OUString & i_rValue);
    /// get text of standard meta data element's attribute
    ::rtl::OUString SAL_CALL getMetaAttr(const char* i_name,
        const char* i_attr) const;
    /// get text of a list of standard meta data elements (multiple occ.)
    css::uno::Sequence< ::rtl::OUString > SAL_CALL getMetaList(
        const char* i_name) const;
    /// set text of a list of standard meta data elements (multiple occ.)
    bool SAL_CALL setMetaList(const char* i_name,
        const css::uno::Sequence< ::rtl::OUString > & i_rValue,
        AttrVector const* = 0);
    void createUserDefined();
};

typedef ::cppu::ImplInheritanceHelper1< SfxDocumentMetaData, css::document::XCompatWriterDocProperties > CompatWriterDocPropsImpl_BASE;

class CompatWriterDocPropsImpl : public CompatWriterDocPropsImpl_BASE
{
    rtl::OUString msManager;
    rtl::OUString msCategory;
    rtl::OUString msCompany;
protected:
    virtual SfxDocumentMetaData* createMe( css::uno::Reference< css::uno::XComponentContext > const & context ) { return new CompatWriterDocPropsImpl( context ); };
public:
    CompatWriterDocPropsImpl( css::uno::Reference< css::uno::XComponentContext > const & context) : CompatWriterDocPropsImpl_BASE( context ) {}
// XCompatWriterDocPropsImpl
    virtual ::rtl::OUString SAL_CALL getManager() throw (::com::sun::star::uno::RuntimeException) { return msManager; }
    virtual void SAL_CALL setManager( const ::rtl::OUString& _manager ) throw (::com::sun::star::uno::RuntimeException) { msManager = _manager; }
    virtual ::rtl::OUString SAL_CALL getCategory() throw (::com::sun::star::uno::RuntimeException){ return msCategory; }
    virtual void SAL_CALL setCategory( const ::rtl::OUString& _category ) throw (::com::sun::star::uno::RuntimeException){ msCategory = _category; }
    virtual ::rtl::OUString SAL_CALL getCompany() throw (::com::sun::star::uno::RuntimeException){ return msCompany; }
    virtual void SAL_CALL setCompany( const ::rtl::OUString& _company ) throw (::com::sun::star::uno::RuntimeException){ msCompany = _company; }

// XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException)
    {
        return comp_CompatWriterDocProps::_getImplementationName();
    }

    virtual ::sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException)
    {
        css::uno::Sequence< rtl::OUString > sServiceNames= getSupportedServiceNames();
        sal_Int32 nLen = sServiceNames.getLength();
        rtl::OUString* pIt = sServiceNames.getArray();
        rtl::OUString* pEnd = ( pIt + nLen );
        sal_Bool bRes = sal_False;
        for ( ; pIt != pEnd; ++pIt )
        {
            if ( pIt->equals( ServiceName ) )
            {
                bRes = sal_True;
                break;
            }
        }
        return bRes;
    }

    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw (::com::sun::star::uno::RuntimeException)
    {
        return comp_CompatWriterDocProps::_getSupportedServiceNames();
    }
};

bool operator== (const css::util::DateTime &i_rLeft,
                 const css::util::DateTime &i_rRight)
{
    return i_rLeft.Year             == i_rRight.Year
        && i_rLeft.Month            == i_rRight.Month
        && i_rLeft.Day              == i_rRight.Day
        && i_rLeft.Hours            == i_rRight.Hours
        && i_rLeft.Minutes          == i_rRight.Minutes
        && i_rLeft.Seconds          == i_rRight.Seconds
        && i_rLeft.HundredthSeconds == i_rRight.HundredthSeconds;
}

// NB: keep these two arrays in sync!
const char* s_stdStatAttrs[] = {
    "meta:page-count",
    "meta:table-count",
    "meta:draw-count",
    "meta:image-count",
    "meta:object-count",
    "meta:ole-object-count",
    "meta:paragraph-count",
    "meta:word-count",
    "meta:character-count",
    "meta:row-count",
    "meta:frame-count",
    "meta:sentence-count",
    "meta:syllable-count",
    "meta:non-whitespace-character-count",
    "meta:cell-count",
    0
};

// NB: keep these two arrays in sync!
const char* s_stdStats[] = {
    "PageCount",
    "TableCount",
    "DrawCount",
    "ImageCount",
    "ObjectCount",
    "OLEObjectCount",
    "ParagraphCount",
    "WordCount",
    "CharacterCount",
    "RowCount",
    "FrameCount",
    "SentenceCount",
    "SyllableCount",
    "NonWhitespaceCharacterCount",
    "CellCount",
    0
};

const char* s_stdMeta[] = {
    "meta:generator",           // string
    "dc:title",                 // string
    "dc:description",           // string
    "dc:subject",               // string
    "meta:initial-creator",     // string
    "dc:creator",               // string
    "meta:printed-by",          // string
    "meta:creation-date",       // dateTime
    "dc:date",                  // dateTime
    "meta:print-date",          // dateTime
    "meta:template",            // XLink
    "meta:auto-reload",         // ...
    "meta:hyperlink-behaviour", // ...
    "dc:language",              // language
    "meta:editing-cycles",      // nonNegativeInteger
    "meta:editing-duration",    // duration
    "meta:document-statistic",  // ... // note: statistic is singular, no s!
    0
};

const char* s_stdMetaList[] = {
    "meta:keyword",             // string*
    "meta:user-defined",        // ...*
    0
};

const char* s_nsXLink   = "http://www.w3.org/1999/xlink";
const char* s_nsDC      = "http://purl.org/dc/elements/1.1/";
const char* s_nsODF     = "urn:oasis:names:tc:opendocument:xmlns:office:1.0";
const char* s_nsODFMeta = "urn:oasis:names:tc:opendocument:xmlns:meta:1.0";
// const char* s_nsOOo     = "http://openoffice.org/2004/office"; // not used (yet?)

static const char s_meta    [] = "meta.xml";

bool isValidDate(const css::util::Date & i_rDate)
{
    return i_rDate.Month > 0;
}

bool isValidDateTime(const css::util::DateTime & i_rDateTime)
{
    return i_rDateTime.Month > 0;
}

std::pair< ::rtl::OUString, ::rtl::OUString > SAL_CALL
getQualifier(const char* i_name) {
    ::rtl::OUString nm = ::rtl::OUString::createFromAscii(i_name);
    sal_Int32 ix = nm.indexOf(static_cast<sal_Unicode> (':'));
    if (ix == -1) {
        return std::make_pair(::rtl::OUString(), nm);
    } else {
        return std::make_pair(nm.copy(0,ix), nm.copy(ix+1));
    }
}

// get namespace for standard qualified names
// NB: only call this with statically known strings!
::rtl::OUString SAL_CALL getNameSpace(const char* i_qname) throw ()
{
    DBG_ASSERT(i_qname, "SfxDocumentMetaData: getNameSpace: argument is null");
    const char * ns = "";
    ::rtl::OUString n = getQualifier(i_qname).first;
    if ( n == "xlink" ) ns = s_nsXLink;
    if ( n == "dc" ) ns = s_nsDC;
    if ( n == "office" ) ns = s_nsODF;
    if ( n == "meta" ) ns = s_nsODFMeta;
    DBG_ASSERT(*ns, "SfxDocumentMetaData: unknown namespace prefix");
    return ::rtl::OUString::createFromAscii(ns);
}

bool SAL_CALL
textToDateOrDateTime(css::util::Date & io_rd, css::util::DateTime & io_rdt,
        bool & o_rIsDateTime, ::rtl::OUString i_text) throw ()
{
    if (::sax::Converter::convertDateOrDateTime(
                io_rd, io_rdt, o_rIsDateTime, i_text)) {
        return true;
    } else {
        DBG_WARNING1("SfxDocumentMetaData: invalid date: %s",
            OUStringToOString(i_text, RTL_TEXTENCODING_UTF8).getStr());
        return false;
    }
}

// convert string to date/time
bool SAL_CALL
textToDateTime(css::util::DateTime & io_rdt, ::rtl::OUString i_text) throw ()
{
    if (::sax::Converter::convertDateTime(io_rdt, i_text)) {
        return true;
    } else {
        DBG_WARNING1("SfxDocumentMetaData: invalid date: %s",
            OUStringToOString(i_text, RTL_TEXTENCODING_UTF8).getStr());
        return false;
    }
}

// convert string to date/time with default return value
css::util::DateTime SAL_CALL
textToDateTimeDefault(::rtl::OUString i_text) throw ()
{
    css::util::DateTime dt;
    static_cast<void> (textToDateTime(dt, i_text));
    // on conversion error: return default value (unchanged)
    return dt;
}

// convert date to string
::rtl::OUString SAL_CALL
dateToText(css::util::Date const& i_rd) throw ()
{
    if (isValidDate(i_rd)) {
        ::rtl::OUStringBuffer buf;
        ::sax::Converter::convertDate(buf, i_rd);
        return buf.makeStringAndClear();
    } else {
        return ::rtl::OUString();
    }
}


// convert date/time to string
::rtl::OUString SAL_CALL
dateTimeToText(css::util::DateTime const& i_rdt) throw ()
{
    if (isValidDateTime(i_rdt)) {
        ::rtl::OUStringBuffer buf;
        ::sax::Converter::convertDateTime(buf, i_rdt, true);
        return buf.makeStringAndClear();
    } else {
        return ::rtl::OUString();
    }
}

// convert string to duration
bool
textToDuration(css::util::Duration& io_rDur, ::rtl::OUString const& i_rText)
throw ()
{
    if (::sax::Converter::convertDuration(io_rDur, i_rText)) {
        return true;
    } else {
        DBG_WARNING1("SfxDocumentMetaData: invalid duration: %s",
            OUStringToOString(i_rText, RTL_TEXTENCODING_UTF8).getStr());
        return false;
    }
}

sal_Int32 textToDuration(::rtl::OUString const& i_rText) throw ()
{
    css::util::Duration d;
    if (textToDuration(d, i_rText)) {
        // #i107372#: approximate years/months
        const sal_Int32 days( (d.Years * 365) + (d.Months * 30) + d.Days );
        return  (days * (24*3600))
                + (d.Hours * 3600) + (d.Minutes * 60) + d.Seconds;
    } else {
        return 0; // default
    }
}

// convert duration to string
::rtl::OUString durationToText(css::util::Duration const& i_rDur) throw ()
{
    ::rtl::OUStringBuffer buf;
    ::sax::Converter::convertDuration(buf, i_rDur);
    return buf.makeStringAndClear();
}

// convert duration to string
::rtl::OUString SAL_CALL durationToText(sal_Int32 i_value) throw ()
{
    css::util::Duration ud;
    ud.Days    = static_cast<sal_Int16>(i_value / (24 * 3600));
    ud.Hours   = static_cast<sal_Int16>((i_value % (24 * 3600)) / 3600);
    ud.Minutes = static_cast<sal_Int16>((i_value % 3600) / 60);
    ud.Seconds = static_cast<sal_Int16>(i_value % 60);
    ud.MilliSeconds = 0;
    return durationToText(ud);
}

// extract base URL (necessary for converting relative links)
css::uno::Reference< css::beans::XPropertySet > SAL_CALL
SfxDocumentMetaData::getURLProperties(
    const css::uno::Sequence< css::beans::PropertyValue > & i_rMedium) const
{
    css::uno::Reference<css::lang::XMultiComponentFactory> xMsf (
        m_xContext->getServiceManager());
    css::uno::Reference< css::beans::XPropertyContainer> xPropArg(
        xMsf->createInstanceWithContext(::rtl::OUString(
                "com.sun.star.beans.PropertyBag"), m_xContext),
        css::uno::UNO_QUERY_THROW);
    try {
        ::rtl::OUString dburl =
            ::rtl::OUString("DocumentBaseURL");
        ::rtl::OUString hdn =
            ::rtl::OUString("HierarchicalDocumentName");
        for (sal_Int32 i = 0; i < i_rMedium.getLength(); ++i) {
            if (i_rMedium[i].Name.equals(dburl)) {
                xPropArg->addProperty(
                    ::rtl::OUString("BaseURI"),
                    css::beans::PropertyAttribute::MAYBEVOID,
                    i_rMedium[i].Value);
            } else if (i_rMedium[i].Name.equals(hdn)) {
                xPropArg->addProperty(
                    ::rtl::OUString("StreamRelPath"),
                    css::beans::PropertyAttribute::MAYBEVOID,
                    i_rMedium[i].Value);
            }
        }
        xPropArg->addProperty(::rtl::OUString("StreamName"),
                css::beans::PropertyAttribute::MAYBEVOID,
                css::uno::makeAny(::rtl::OUString(s_meta)));
    } catch (const css::uno::Exception &) {
        // ignore
    }
    return css::uno::Reference< css::beans::XPropertySet>(xPropArg,
                css::uno::UNO_QUERY_THROW);
}

// return the text of the (hopefully unique, i.e., normalize first!) text
// node _below_ the given node
::rtl::OUString SAL_CALL
getNodeText(css::uno::Reference<css::xml::dom::XNode> i_xNode)
        throw (css::uno::RuntimeException)
{
    if (!i_xNode.is()) throw css::uno::RuntimeException(
        ::rtl::OUString(
                "SfxDocumentMetaData::getNodeText: argument is null"), i_xNode);
    for (css::uno::Reference<css::xml::dom::XNode> c = i_xNode->getFirstChild();
            c.is();
            c = c->getNextSibling()) {
        if (c->getNodeType() == css::xml::dom::NodeType_TEXT_NODE) {
            try {
                return c->getNodeValue();
            } catch (const css::xml::dom::DOMException &) { // too big?
                return ::rtl::OUString();
            }
        }
    }
    return ::rtl::OUString();
}

::rtl::OUString SAL_CALL
SfxDocumentMetaData::getMetaText(const char* i_name) const
//        throw (css::uno::RuntimeException)
{
    checkInit();

    const ::rtl::OUString name( ::rtl::OUString::createFromAscii(i_name) );
    DBG_ASSERT(m_meta.find(name) != m_meta.end(),
        "SfxDocumentMetaData::getMetaText: not found");
    css::uno::Reference<css::xml::dom::XNode> xNode = m_meta.find(name)->second;
    return (xNode.is()) ? getNodeText(xNode) : ::rtl::OUString();
}

bool SAL_CALL
SfxDocumentMetaData::setMetaText(const char* i_name,
        const ::rtl::OUString & i_rValue)
    // throw (css::uno::RuntimeException)
{
    checkInit();

    const ::rtl::OUString name( ::rtl::OUString::createFromAscii(i_name) );
    DBG_ASSERT(m_meta.find(name) != m_meta.end(),
        "SfxDocumentMetaData::setMetaText: not found");
    css::uno::Reference<css::xml::dom::XNode> xNode = m_meta.find(name)->second;

    try {
        if (i_rValue.isEmpty()) {
            if (xNode.is()) { // delete
                m_xParent->removeChild(xNode);
                xNode.clear();
                m_meta[name] = xNode;
                return true;
            } else {
                return false;
            }
        } else {
            if (xNode.is()) { // update
                for (css::uno::Reference<css::xml::dom::XNode> c =
                            xNode->getFirstChild();
                        c.is();
                        c = c->getNextSibling()) {
                    if (c->getNodeType() == css::xml::dom::NodeType_TEXT_NODE) {
                        if (!c->getNodeValue().equals(i_rValue)) {
                            c->setNodeValue(i_rValue);
                            return true;
                        } else {
                            return false;
                        }
                    }
                }
            } else { // insert
                xNode.set(m_xDoc->createElementNS(getNameSpace(i_name), name),
                            css::uno::UNO_QUERY_THROW);
                m_xParent->appendChild(xNode);
                m_meta[name] = xNode;
            }
            css::uno::Reference<css::xml::dom::XNode> xTextNode(
                m_xDoc->createTextNode(i_rValue), css::uno::UNO_QUERY_THROW);
            xNode->appendChild(xTextNode);
            return true;
        }
    } catch (const css::xml::dom::DOMException & e) {
        css::uno::Any a(e);
        throw css::lang::WrappedTargetRuntimeException(
                ::rtl::OUString(
                        "SfxDocumentMetaData::setMetaText: DOM exception"),
                css::uno::Reference<css::uno::XInterface>(*this), a);
    }
}

void SAL_CALL
SfxDocumentMetaData::setMetaTextAndNotify(const char* i_name,
        const ::rtl::OUString & i_rValue)
    // throw (css::uno::RuntimeException)
{
    ::osl::ClearableMutexGuard g(m_aMutex);
    if (setMetaText(i_name, i_rValue)) {
        g.clear();
        setModified(true);
    }
}

::rtl::OUString SAL_CALL
SfxDocumentMetaData::getMetaAttr(const char* i_name, const char* i_attr) const
//        throw (css::uno::RuntimeException)
{
    ::rtl::OUString name = ::rtl::OUString::createFromAscii(i_name);
    DBG_ASSERT(m_meta.find(name) != m_meta.end(),
        "SfxDocumentMetaData::getMetaAttr: not found");
    css::uno::Reference<css::xml::dom::XNode> xNode = m_meta.find(name)->second;
    if (xNode.is()) {
        css::uno::Reference<css::xml::dom::XElement> xElem(xNode,
            css::uno::UNO_QUERY_THROW);
        return xElem->getAttributeNS(getNameSpace(i_attr),
                    getQualifier(i_attr).second);
    } else {
        return ::rtl::OUString();
    }
}

css::uno::Sequence< ::rtl::OUString> SAL_CALL
SfxDocumentMetaData::getMetaList(const char* i_name) const
//        throw (css::uno::RuntimeException)
{
    checkInit();
    ::rtl::OUString name = ::rtl::OUString::createFromAscii(i_name);
    DBG_ASSERT(m_metaList.find(name) != m_metaList.end(),
        "SfxDocumentMetaData::getMetaList: not found");
    std::vector<css::uno::Reference<css::xml::dom::XNode> > const & vec =
        m_metaList.find(name)->second;
    css::uno::Sequence< ::rtl::OUString> ret(vec.size());
    for (size_t i = 0; i < vec.size(); ++i) {
        ret[i] = getNodeText(vec.at(i));
    }
    return ret;
}

bool SAL_CALL
SfxDocumentMetaData::setMetaList(const char* i_name,
        const css::uno::Sequence< ::rtl::OUString> & i_rValue,
        AttrVector const* i_pAttrs)
    // throw (css::uno::RuntimeException)
{
    checkInit();
    DBG_ASSERT((i_pAttrs == 0) ||
               (static_cast<size_t>(i_rValue.getLength()) == i_pAttrs->size()),
        "SfxDocumentMetaData::setMetaList: invalid args");

    try {
        ::rtl::OUString name = ::rtl::OUString::createFromAscii(i_name);
        DBG_ASSERT(m_metaList.find(name) != m_metaList.end(),
            "SfxDocumentMetaData::setMetaList: not found");
        std::vector<css::uno::Reference<css::xml::dom::XNode> > & vec =
            m_metaList[name];

        // if nothing changed, do nothing
        // alas, this does not check for permutations, or attributes...
        if ((0 == i_pAttrs)) {
            if (static_cast<size_t>(i_rValue.getLength()) == vec.size()) {
                bool isEqual(true);
                for (sal_Int32 i = 0; i < i_rValue.getLength(); ++i) {
                    css::uno::Reference<css::xml::dom::XNode> xNode(vec.at(i));
                    if (xNode.is()) {
                        ::rtl::OUString val = getNodeText(xNode);
                        if (!val.equals(i_rValue[i])) {
                            isEqual = false;
                            break;
                        }
                    }
                }
                if (isEqual) return false;
            }
        }

        // remove old meta data nodes
        {
            std::vector<css::uno::Reference<css::xml::dom::XNode> >
                ::reverse_iterator it(vec.rbegin());
            try {
                for ( ;it != vec.rend(); ++it)
                {
                    m_xParent->removeChild(*it);
                }
            }
            catch (...)
            {
                // Clean up already removed nodes
                vec.erase(it.base(), vec.end());
                throw;
            }
            vec.clear();
        }

        // insert new meta data nodes into DOM tree
        for (sal_Int32 i = 0; i < i_rValue.getLength(); ++i) {
            css::uno::Reference<css::xml::dom::XElement> xElem(
                m_xDoc->createElementNS(getNameSpace(i_name), name),
                css::uno::UNO_QUERY_THROW);
            css::uno::Reference<css::xml::dom::XNode> xNode(xElem,
                css::uno::UNO_QUERY_THROW);
            css::uno::Reference<css::xml::dom::XNode> xTextNode(
                m_xDoc->createTextNode(i_rValue[i]), css::uno::UNO_QUERY_THROW);
            // set attributes
            if (i_pAttrs != 0) {
                for (std::vector<std::pair<const char*, ::rtl::OUString> >
                                ::const_iterator it = (*i_pAttrs)[i].begin();
                        it != (*i_pAttrs)[i].end(); ++it) {
                    xElem->setAttributeNS(getNameSpace(it->first),
                        ::rtl::OUString::createFromAscii(it->first),
                        it->second);
                }
            }
            xNode->appendChild(xTextNode);
            m_xParent->appendChild(xNode);
            vec.push_back(xNode);
        }

        return true;
    } catch (const css::xml::dom::DOMException & e) {
        css::uno::Any a(e);
        throw css::lang::WrappedTargetRuntimeException(
                ::rtl::OUString(
                        "SfxDocumentMetaData::setMetaList: DOM exception"),
                css::uno::Reference<css::uno::XInterface>(*this), a);
    }
}

// convert property list to string list and attribute list
std::pair<css::uno::Sequence< ::rtl::OUString>, AttrVector> SAL_CALL
propsToStrings(css::uno::Reference<css::beans::XPropertySet> const & i_xPropSet)
{
    ::comphelper::SequenceAsVector< ::rtl::OUString > values;
    AttrVector attrs;

    css::uno::Reference<css::beans::XPropertySetInfo> xSetInfo
        = i_xPropSet->getPropertySetInfo();
    css::uno::Sequence<css::beans::Property> props = xSetInfo->getProperties();

    for (sal_Int32 i = 0; i < props.getLength(); ++i) {
        if (props[i].Attributes & css::beans::PropertyAttribute::TRANSIENT) {
            continue;
        }
        const ::rtl::OUString name = props[i].Name;
        css::uno::Any any;
        try {
            any = i_xPropSet->getPropertyValue(name);
        } catch (const css::uno::Exception &) {
            // ignore
        }
        const css::uno::Type & type = any.getValueType();
        std::vector<std::pair<const char*, ::rtl::OUString> > as;
        as.push_back(std::make_pair(static_cast<const char*>("meta:name"),
                                        name));
        const char* vt = "meta:value-type";

        // convert according to type
        if (type == ::cppu::UnoType<bool>::get()) {
            bool b = false;
            any >>= b;
            ::rtl::OUStringBuffer buf;
            ::sax::Converter::convertBool(buf, b);
            values.push_back(buf.makeStringAndClear());
            as.push_back(std::make_pair(vt,
                ::rtl::OUString("boolean")));
        } else if (type == ::cppu::UnoType< ::rtl::OUString>::get()) {
            ::rtl::OUString s;
            any >>= s;
            values.push_back(s);
// #i90847# OOo 2.x does stupid things if value-type="string";
// fortunately string is default anyway, so we can just omit it
// #i107502#: however, OOo 2.x only reads 4 user-defined without @value-type
// => best backward compatibility: first 4 without @value-type, rest with
            if (4 <= i)
            {
                as.push_back(std::make_pair(vt,
                    ::rtl::OUString("string")));
            }
        } else if (type == ::cppu::UnoType<css::util::DateTime>::get()) {
            css::util::DateTime dt;
            any >>= dt;
            values.push_back(dateTimeToText(dt));
            as.push_back(std::make_pair(vt,
                ::rtl::OUString("date")));
        } else if (type == ::cppu::UnoType<css::util::Date>::get()) {
            css::util::Date d;
            any >>= d;
            values.push_back(dateToText(d));
            as.push_back(std::make_pair(vt,
                ::rtl::OUString("date")));
        } else if (type == ::cppu::UnoType<css::util::Time>::get()) {
            // #i97029#: replaced by Duration
            // Time is supported for backward compatibility with OOo 3.x, x<=2
            css::util::Time ut;
            any >>= ut;
            css::util::Duration ud;
            ud.Hours   = ut.Hours;
            ud.Minutes = ut.Minutes;
            ud.Seconds = ut.Seconds;
            ud.MilliSeconds = 10 * ut.HundredthSeconds;
            values.push_back(durationToText(ud));
            as.push_back(std::make_pair(vt,
                ::rtl::OUString("time")));
        } else if (type == ::cppu::UnoType<css::util::Duration>::get()) {
            css::util::Duration ud;
            any >>= ud;
            values.push_back(durationToText(ud));
            as.push_back(std::make_pair(vt,
                ::rtl::OUString("time")));
        } else if (::cppu::UnoType<double>::get().isAssignableFrom(type)) {
            // support not just double, but anything that can be converted
            double d = 0;
            any >>= d;
            ::rtl::OUStringBuffer buf;
            ::sax::Converter::convertDouble(buf, d);
            values.push_back(buf.makeStringAndClear());
            as.push_back(std::make_pair(vt,
                ::rtl::OUString("float")));
        } else {
            DBG_WARNING1("SfxDocumentMetaData: unsupported property type: %s",
                OUStringToOString(any.getValueTypeName(),
                    RTL_TEXTENCODING_UTF8).getStr());
            continue;
        }
        attrs.push_back(as);
    }

    return std::make_pair(values.getAsConstList(), attrs);
}

// remove the given element from the DOM, and iff i_pAttrs != 0 insert new one
void SAL_CALL
SfxDocumentMetaData::updateElement(const char *i_name,
        std::vector<std::pair<const char *, ::rtl::OUString> >* i_pAttrs)
{
    ::rtl::OUString name = ::rtl::OUString::createFromAscii(i_name);
    try {
        // remove old element
        css::uno::Reference<css::xml::dom::XNode> xNode =
            m_meta.find(name)->second;
        if (xNode.is()) {
            m_xParent->removeChild(xNode);
            xNode.clear();
        }
        // add new element
        if (0 != i_pAttrs) {
            css::uno::Reference<css::xml::dom::XElement> xElem(
                m_xDoc->createElementNS(getNameSpace(i_name), name),
                    css::uno::UNO_QUERY_THROW);
            xNode.set(xElem, css::uno::UNO_QUERY_THROW);
            // set attributes
            for (std::vector<std::pair<const char *, ::rtl::OUString> >
                    ::const_iterator it = i_pAttrs->begin();
                    it != i_pAttrs->end(); ++it) {
                xElem->setAttributeNS(getNameSpace(it->first),
                    ::rtl::OUString::createFromAscii(it->first), it->second);
            }
            m_xParent->appendChild(xNode);
        }
        m_meta[name] = xNode;
    } catch (const css::xml::dom::DOMException & e) {
        css::uno::Any a(e);
        throw css::lang::WrappedTargetRuntimeException(
                ::rtl::OUString(
                    "SfxDocumentMetaData::updateElement: DOM exception"),
                css::uno::Reference<css::uno::XInterface>(*this), a);
    }
}

// update user-defined meta data in DOM tree
void SAL_CALL SfxDocumentMetaData::updateUserDefinedAndAttributes()
{
    createUserDefined();
    const css::uno::Reference<css::beans::XPropertySet> xPSet(m_xUserDefined,
            css::uno::UNO_QUERY_THROW);
    const std::pair<css::uno::Sequence< ::rtl::OUString>, AttrVector>
        udStringsAttrs( propsToStrings(xPSet) );
    (void) setMetaList("meta:user-defined", udStringsAttrs.first,
            &udStringsAttrs.second);

    // update elements with attributes
    std::vector<std::pair<const char *, ::rtl::OUString> > attributes;
    if (!m_TemplateName.isEmpty() || !m_TemplateURL.isEmpty()
            || isValidDateTime(m_TemplateDate)) {
        attributes.push_back(std::make_pair(
                static_cast<const char*>("xlink:type"),
                ::rtl::OUString("simple")));
        attributes.push_back(std::make_pair(
                static_cast<const char*>("xlink:actuate"),
                ::rtl::OUString("onRequest")));
        attributes.push_back(std::make_pair(
                static_cast<const char*>("xlink:title"), m_TemplateName));
        attributes.push_back(std::make_pair(
                static_cast<const char*>("xlink:href" ), m_TemplateURL ));
        if (isValidDateTime(m_TemplateDate)) {
            attributes.push_back(std::make_pair(
                static_cast<const char*>("meta:date"  ),
                dateTimeToText(m_TemplateDate)));
        }
        updateElement("meta:template", &attributes);
    } else {
        updateElement("meta:template");
    }
    attributes.clear();

    if (!m_AutoloadURL.isEmpty() || (0 != m_AutoloadSecs)) {
        attributes.push_back(std::make_pair(
                static_cast<const char*>("xlink:href" ), m_AutoloadURL ));
        attributes.push_back(std::make_pair(
                static_cast<const char*>("meta:delay" ),
                durationToText(m_AutoloadSecs)));
        updateElement("meta:auto-reload", &attributes);
    } else {
        updateElement("meta:auto-reload");
    }
    attributes.clear();

    if (!m_DefaultTarget.isEmpty()) {
        attributes.push_back(std::make_pair(
                static_cast<const char*>("office:target-frame-name"),
                m_DefaultTarget));
        // xlink:show: _blank -> new, any other value -> replace
        const sal_Char* show = m_DefaultTarget == "_blank" ? "new" : "replace";
        attributes.push_back(std::make_pair(
                static_cast<const char*>("xlink:show"),
                ::rtl::OUString::createFromAscii(show)));
        updateElement("meta:hyperlink-behaviour", &attributes);
    } else {
        updateElement("meta:hyperlink-behaviour");
    }
    attributes.clear();
}

// create empty DOM tree (XDocument)
css::uno::Reference<css::xml::dom::XDocument> SAL_CALL
SfxDocumentMetaData::createDOM() const // throw (css::uno::RuntimeException)
{
    css::uno::Reference<css::lang::XMultiComponentFactory> xMsf ( m_xContext->getServiceManager());
    css::uno::Reference<css::xml::dom::XDocumentBuilder> xBuilder( css::xml::dom::DocumentBuilder::create(m_xContext) );
    css::uno::Reference<css::xml::dom::XDocument> xDoc = xBuilder->newDocument();
    if (!xDoc.is()) throw css::uno::RuntimeException(
        ::rtl::OUString("SfxDocumentMetaData::createDOM: "
                "cannot create new document"),
                *const_cast<SfxDocumentMetaData*>(this));
    return xDoc;
}

void SAL_CALL
SfxDocumentMetaData::checkInit() const // throw (css::uno::RuntimeException)
{
    if (!m_isInitialized) {
        throw css::uno::RuntimeException(::rtl::OUString(
                "SfxDocumentMetaData::checkInit: not initialized"),
                *const_cast<SfxDocumentMetaData*>(this));
    }
    DBG_ASSERT((m_xDoc.is() && m_xParent.is() ),
                "SfxDocumentMetaData::checkInit: reference is null");
}

// initialize state from DOM tree
void SAL_CALL SfxDocumentMetaData::init(
        css::uno::Reference<css::xml::dom::XDocument> i_xDoc)
{
    if (!i_xDoc.is()) throw css::uno::RuntimeException(
        ::rtl::OUString(
                "SfxDocumentMetaData::init: no DOM tree given"), *this);

    css::uno::Reference<css::lang::XMultiComponentFactory> xMsf (
        m_xContext->getServiceManager());
    css::uno::Reference<css::xml::xpath::XXPathAPI> xPath(
        xMsf->createInstanceWithContext(::rtl::OUString(
                "com.sun.star.xml.xpath.XPathAPI"), m_xContext),
        css::uno::UNO_QUERY_THROW );
    if (!xPath.is()) throw css::uno::RuntimeException(
        ::rtl::OUString("SfxDocumentMetaData::init:"
                " cannot create XPathAPI service"), *this);

    m_isInitialized = false;
    m_xDoc = i_xDoc;

    // select nodes for standard meta data stuff
    xPath->registerNS(::rtl::OUString("xlink"),
        ::rtl::OUString::createFromAscii(s_nsXLink));
    xPath->registerNS(::rtl::OUString("dc"),
        ::rtl::OUString::createFromAscii(s_nsDC));
    xPath->registerNS(::rtl::OUString("office"),
        ::rtl::OUString::createFromAscii(s_nsODF));
    xPath->registerNS(::rtl::OUString("meta"),
        ::rtl::OUString::createFromAscii(s_nsODFMeta));
    // NB: we do not handle the single-XML-file ODF variant, which would
    //     have the root element office:document.
    //     The root of such documents must be converted in the importer!
    ::rtl::OUString prefix(
        "/child::office:document-meta/child::office:meta");
    css::uno::Reference<css::xml::dom::XNode> xDocNode(
        m_xDoc, css::uno::UNO_QUERY_THROW);
    m_xParent.clear();
    try {
        m_xParent = xPath->selectSingleNode(xDocNode, prefix);
    } catch (const com::sun::star::uno::Exception &) {
    }

    if (!m_xParent.is()) {
        // all this create/append stuff may throw DOMException
        try {
            css::uno::Reference<css::xml::dom::XElement> xRElem;
            css::uno::Reference<css::xml::dom::XNode> xNode(
                i_xDoc->getFirstChild());
            while (xNode.is()) {
                if (css::xml::dom::NodeType_ELEMENT_NODE ==xNode->getNodeType())
                {
                    if ( xNode->getNamespaceURI().equalsAscii(s_nsODF) && xNode->getLocalName() == "document-meta" )
                    {
                        xRElem.set(xNode, css::uno::UNO_QUERY_THROW);
                        break;
                    }
                    else
                    {
                        OSL_TRACE("SfxDocumentMetaData::init(): "
                                "deleting unexpected root element: %s",
                            ::rtl::OUStringToOString(xNode->getLocalName(),
                                RTL_TEXTENCODING_UTF8).getStr());
                        i_xDoc->removeChild(xNode);
                        xNode = i_xDoc->getFirstChild(); // start over
                    }
                } else {
                    xNode = xNode->getNextSibling();
                }
            }
            if (!xRElem.is()) {
                xRElem = i_xDoc->createElementNS(
                    ::rtl::OUString::createFromAscii(s_nsODF),
                    ::rtl::OUString("office:document-meta"));
                css::uno::Reference<css::xml::dom::XNode> xRNode(xRElem,
                    css::uno::UNO_QUERY_THROW);
                i_xDoc->appendChild(xRNode);
            }
            xRElem->setAttributeNS(::rtl::OUString::createFromAscii(s_nsODF),
                        ::rtl::OUString("office:version"),
                        ::rtl::OUString("1.0"));
            // does not exist, otherwise m_xParent would not be null
            css::uno::Reference<css::xml::dom::XNode> xParent (
                i_xDoc->createElementNS(
                    ::rtl::OUString::createFromAscii(s_nsODF),
                    ::rtl::OUString("office:meta")),
            css::uno::UNO_QUERY_THROW);
            xRElem->appendChild(xParent);
            m_xParent = xParent;
        } catch (const css::xml::dom::DOMException & e) {
            css::uno::Any a(e);
            throw css::lang::WrappedTargetRuntimeException(
                    ::rtl::OUString(
                            "SfxDocumentMetaData::init: DOM exception"),
                    css::uno::Reference<css::uno::XInterface>(*this), a);
        }
    }


    // select nodes for elements of which we only handle one occurrence
    for (const char **pName = s_stdMeta; *pName != 0; ++pName) {
        ::rtl::OUString name = ::rtl::OUString::createFromAscii(*pName);
        // NB: If a document contains more than one occurrence of a
        // meta-data element, we arbitrarily pick one of them here.
        // We do not remove the others, i.e., when we write the
        // document, it will contain the duplicates unchanged.
        // The ODF spec says that handling multiple occurrences is
        // application-specific.
        css::uno::Reference<css::xml::dom::XNode> xNode =
            xPath->selectSingleNode(m_xParent,
                ::rtl::OUString("child::") + name);
        // Do not create an empty element if it is missing;
        // for certain elements, such as dateTime, this would be invalid
        m_meta[name] = xNode;
    }

    // select nodes for elements of which we handle all occurrences
    for (const char **pName = s_stdMetaList; *pName != 0; ++pName) {
        ::rtl::OUString name = ::rtl::OUString::createFromAscii(*pName);
        css::uno::Reference<css::xml::dom::XNodeList> nodes =
            xPath->selectNodeList(m_xParent,
                ::rtl::OUString("child::") + name);
        std::vector<css::uno::Reference<css::xml::dom::XNode> > v;
        for (sal_Int32 i = 0; i < nodes->getLength(); ++i) {
            v.push_back(nodes->item(i));
        }
        m_metaList[name] = v;
    }

    // initialize members corresponding to attributes from DOM nodes
    m_TemplateName  = getMetaAttr("meta:template", "xlink:title");
    m_TemplateURL   = getMetaAttr("meta:template", "xlink:href");
    m_TemplateDate  =
        textToDateTimeDefault(getMetaAttr("meta:template", "meta:date"));
    m_AutoloadURL   = getMetaAttr("meta:auto-reload", "xlink:href");
    m_AutoloadSecs  =
        textToDuration(getMetaAttr("meta:auto-reload", "meta:delay"));
    m_DefaultTarget =
        getMetaAttr("meta:hyperlink-behaviour", "office:target-frame-name");


    std::vector<css::uno::Reference<css::xml::dom::XNode> > & vec =
        m_metaList[::rtl::OUString("meta:user-defined")];
    m_xUserDefined.clear(); // #i105826#: reset (may be re-initialization)
    if ( !vec.empty() )
    {
        createUserDefined();
    }

    // user-defined meta data: initialize PropertySet from DOM nodes
    for (std::vector<css::uno::Reference<css::xml::dom::XNode> >::iterator
            it = vec.begin(); it != vec.end(); ++it) {
        css::uno::Reference<css::xml::dom::XElement> xElem(*it,
            css::uno::UNO_QUERY_THROW);
        css::uno::Any any;
        ::rtl::OUString name = xElem->getAttributeNS(
                ::rtl::OUString::createFromAscii(s_nsODFMeta),
                ::rtl::OUString("name"));
        ::rtl::OUString type = xElem->getAttributeNS(
                ::rtl::OUString::createFromAscii(s_nsODFMeta),
                ::rtl::OUString("value-type"));
        ::rtl::OUString text = getNodeText(*it);
        if ( type == "float" ) {
            double d;
            if (::sax::Converter::convertDouble(d, text)) {
                any <<= d;
            } else {
                DBG_WARNING1("SfxDocumentMetaData: invalid float: %s",
                    OUStringToOString(text, RTL_TEXTENCODING_UTF8).getStr());
                continue;
            }
        } else if ( type == "date" ) {
            bool isDateTime;
            css::util::Date d;
            css::util::DateTime dt;
            if (textToDateOrDateTime(d, dt, isDateTime, text)) {
                if (isDateTime) {
                    any <<= dt;
                } else {
                    any <<= d;
                }
            } else {
                DBG_WARNING1("SfxDocumentMetaData: invalid date: %s",
                    OUStringToOString(text, RTL_TEXTENCODING_UTF8).getStr());
                continue;
            }
        } else if ( type == "time" ) {
            css::util::Duration ud;
            if (textToDuration(ud, text)) {
                any <<= ud;
            } else {
                DBG_WARNING1("SfxDocumentMetaData: invalid time: %s",
                    OUStringToOString(text, RTL_TEXTENCODING_UTF8).getStr());
                continue;
            }
        } else if ( type == "boolean" ) {
            bool b;
            if (::sax::Converter::convertBool(b, text)) {
                any <<= b;
            } else {
                DBG_WARNING1("SfxDocumentMetaData: invalid boolean: %s",
                    OUStringToOString(text, RTL_TEXTENCODING_UTF8).getStr());
                continue;
            }
        } else if ( type == "string" || true) { // default
            any <<= text;
        }
        try {
            m_xUserDefined->addProperty(name,
                css::beans::PropertyAttribute::REMOVEABLE, any);
        } catch (const css::beans::PropertyExistException &) {
            DBG_WARNING1("SfxDocumentMetaData: duplicate: %s",
                    OUStringToOString(name, RTL_TEXTENCODING_UTF8).getStr());
            // ignore; duplicate
        } catch (const css::beans::IllegalTypeException &) {
            OSL_TRACE("SfxDocumentMetaData: illegal type: %s",
                    OUStringToOString(name, RTL_TEXTENCODING_UTF8).getStr());
        } catch (const css::lang::IllegalArgumentException &) {
            OSL_TRACE("SfxDocumentMetaData: illegal arg: %s",
                    OUStringToOString(name, RTL_TEXTENCODING_UTF8).getStr());
        }
    }

    m_isModified = false;
    m_isInitialized = true;
}



SfxDocumentMetaData::SfxDocumentMetaData(
        css::uno::Reference< css::uno::XComponentContext > const & context)
    : BaseMutex()
    , SfxDocumentMetaData_Base(m_aMutex)
    , m_xContext(context)
    , m_NotifyListeners(m_aMutex)
    , m_isInitialized(false)
    , m_isModified(false)
    , m_AutoloadSecs(0)
{
    DBG_ASSERT(context.is(), "SfxDocumentMetaData: context is null");
    DBG_ASSERT(context->getServiceManager().is(),
        "SfxDocumentMetaData: context has no service manager");
    init(createDOM());
}

// com.sun.star.uno.XServiceInfo:
::rtl::OUString SAL_CALL
SfxDocumentMetaData::getImplementationName() throw (css::uno::RuntimeException)
{
    return comp_SfxDocumentMetaData::_getImplementationName();
}

::sal_Bool SAL_CALL
SfxDocumentMetaData::supportsService(::rtl::OUString const & serviceName)
        throw (css::uno::RuntimeException)
{
    css::uno::Sequence< ::rtl::OUString > serviceNames =
        comp_SfxDocumentMetaData::_getSupportedServiceNames();
    for (::sal_Int32 i = 0; i < serviceNames.getLength(); ++i) {
        if (serviceNames[i] == serviceName)
            return sal_True;
    }
    return sal_False;
}

css::uno::Sequence< ::rtl::OUString > SAL_CALL
SfxDocumentMetaData::getSupportedServiceNames()
        throw (css::uno::RuntimeException)
{
    return comp_SfxDocumentMetaData::_getSupportedServiceNames();
}


// ::com::sun::star::lang::XComponent:
void SAL_CALL SfxDocumentMetaData::dispose() throw (css::uno::RuntimeException)
{
    ::osl::MutexGuard g(m_aMutex);
    if (!m_isInitialized) {
        return;
    }
    WeakComponentImplHelperBase::dispose(); // superclass
    m_NotifyListeners.disposeAndClear(css::lang::EventObject(
            static_cast< ::cppu::OWeakObject* >(this)));
    m_isInitialized = false;
    m_meta.clear();
    m_metaList.clear();
    m_xParent.clear();
    m_xDoc.clear();
    m_xUserDefined.clear();
}


// ::com::sun::star::document::XDocumentProperties:
::rtl::OUString SAL_CALL
SfxDocumentMetaData::getAuthor() throw (css::uno::RuntimeException)
{
    ::osl::MutexGuard g(m_aMutex);
    return getMetaText("meta:initial-creator");
}

void SAL_CALL SfxDocumentMetaData::setAuthor(const ::rtl::OUString & the_value)
        throw (css::uno::RuntimeException)
{
    setMetaTextAndNotify("meta:initial-creator", the_value);
}


::rtl::OUString SAL_CALL
SfxDocumentMetaData::getGenerator() throw (css::uno::RuntimeException)
{
    ::osl::MutexGuard g(m_aMutex);
    return getMetaText("meta:generator");
}

void SAL_CALL
SfxDocumentMetaData::setGenerator(const ::rtl::OUString & the_value)
        throw (css::uno::RuntimeException)
{
    setMetaTextAndNotify("meta:generator", the_value);
}

css::util::DateTime SAL_CALL
SfxDocumentMetaData::getCreationDate() throw (css::uno::RuntimeException)
{
    ::osl::MutexGuard g(m_aMutex);
    return textToDateTimeDefault(getMetaText("meta:creation-date"));
}

void SAL_CALL
SfxDocumentMetaData::setCreationDate(const css::util::DateTime & the_value)
        throw (css::uno::RuntimeException)
{
    setMetaTextAndNotify("meta:creation-date", dateTimeToText(the_value));
}

::rtl::OUString SAL_CALL
SfxDocumentMetaData::getTitle() throw (css::uno::RuntimeException)
{
    ::osl::MutexGuard g(m_aMutex);
    return getMetaText("dc:title");
}

void SAL_CALL SfxDocumentMetaData::setTitle(const ::rtl::OUString & the_value)
        throw (css::uno::RuntimeException)
{
    setMetaTextAndNotify("dc:title", the_value);
}

::rtl::OUString SAL_CALL
SfxDocumentMetaData::getSubject() throw (css::uno::RuntimeException)
{
    ::osl::MutexGuard g(m_aMutex);
    return getMetaText("dc:subject");
}

void SAL_CALL
SfxDocumentMetaData::setSubject(const ::rtl::OUString & the_value)
        throw (css::uno::RuntimeException)
{
    setMetaTextAndNotify("dc:subject", the_value);
}

::rtl::OUString SAL_CALL
SfxDocumentMetaData::getDescription() throw (css::uno::RuntimeException)
{
    ::osl::MutexGuard g(m_aMutex);
    return getMetaText("dc:description");
}

void SAL_CALL
SfxDocumentMetaData::setDescription(const ::rtl::OUString & the_value)
        throw (css::uno::RuntimeException)
{
    setMetaTextAndNotify("dc:description", the_value);
}

css::uno::Sequence< ::rtl::OUString >
SAL_CALL SfxDocumentMetaData::getKeywords() throw (css::uno::RuntimeException)
{
    ::osl::MutexGuard g(m_aMutex);
    return getMetaList("meta:keyword");
}

void SAL_CALL
SfxDocumentMetaData::setKeywords(
        const css::uno::Sequence< ::rtl::OUString > & the_value)
        throw (css::uno::RuntimeException)
{
    ::osl::ClearableMutexGuard g(m_aMutex);
    if (setMetaList("meta:keyword", the_value)) {
        g.clear();
        setModified(true);
    }
}

css::lang::Locale SAL_CALL
        SfxDocumentMetaData::getLanguage() throw (css::uno::RuntimeException)
{
    ::osl::MutexGuard g(m_aMutex);
    css::lang::Locale loc;
    ::rtl::OUString text = getMetaText("dc:language");
    sal_Int32 ix = text.indexOf(static_cast<sal_Unicode> ('-'));
    if (ix == -1) {
        loc.Language = text;
    } else {
        loc.Language = text.copy(0, ix);
        loc.Country = text.copy(ix+1);
    }
    return loc;
}

void SAL_CALL
SfxDocumentMetaData::setLanguage(const css::lang::Locale & the_value)
        throw (css::uno::RuntimeException)
{
    ::rtl::OUString text = the_value.Language;
    if (!the_value.Country.isEmpty()) {
        text += ::rtl::OUString("-").concat(the_value.Country);
    }
    setMetaTextAndNotify("dc:language", text);
}

::rtl::OUString SAL_CALL
SfxDocumentMetaData::getModifiedBy() throw (css::uno::RuntimeException)
{
    ::osl::MutexGuard g(m_aMutex);
    return getMetaText("dc:creator");
}

void SAL_CALL
SfxDocumentMetaData::setModifiedBy(const ::rtl::OUString & the_value)
        throw (css::uno::RuntimeException)
{
    setMetaTextAndNotify("dc:creator", the_value);
}

css::util::DateTime SAL_CALL
SfxDocumentMetaData::getModificationDate() throw (css::uno::RuntimeException)
{
    ::osl::MutexGuard g(m_aMutex);
    return textToDateTimeDefault(getMetaText("dc:date"));
}

void SAL_CALL
SfxDocumentMetaData::setModificationDate(const css::util::DateTime & the_value)
        throw (css::uno::RuntimeException)
{
    setMetaTextAndNotify("dc:date", dateTimeToText(the_value));
}

::rtl::OUString SAL_CALL
SfxDocumentMetaData::getPrintedBy() throw (css::uno::RuntimeException)
{
    ::osl::MutexGuard g(m_aMutex);
    return getMetaText("meta:printed-by");
}

void SAL_CALL
SfxDocumentMetaData::setPrintedBy(const ::rtl::OUString & the_value)
        throw (css::uno::RuntimeException)
{
    setMetaTextAndNotify("meta:printed-by", the_value);
}

css::util::DateTime SAL_CALL
SfxDocumentMetaData::getPrintDate() throw (css::uno::RuntimeException)
{
    ::osl::MutexGuard g(m_aMutex);
    return textToDateTimeDefault(getMetaText("meta:print-date"));
}

void SAL_CALL
SfxDocumentMetaData::setPrintDate(const css::util::DateTime & the_value)
        throw (css::uno::RuntimeException)
{
    setMetaTextAndNotify("meta:print-date", dateTimeToText(the_value));
}

::rtl::OUString SAL_CALL
SfxDocumentMetaData::getTemplateName() throw (css::uno::RuntimeException)
{
    ::osl::MutexGuard g(m_aMutex);
    checkInit();
    return m_TemplateName;
}

void SAL_CALL
SfxDocumentMetaData::setTemplateName(const ::rtl::OUString & the_value)
        throw (css::uno::RuntimeException)
{
    ::osl::ClearableMutexGuard g(m_aMutex);
    checkInit();
    if (m_TemplateName != the_value) {
        m_TemplateName = the_value;
        g.clear();
        setModified(true);
    }
}

::rtl::OUString SAL_CALL
SfxDocumentMetaData::getTemplateURL() throw (css::uno::RuntimeException)
{
    ::osl::MutexGuard g(m_aMutex);
    checkInit();
    return m_TemplateURL;
}

void SAL_CALL
SfxDocumentMetaData::setTemplateURL(const ::rtl::OUString & the_value)
        throw (css::uno::RuntimeException)
{
    ::osl::ClearableMutexGuard g(m_aMutex);
    checkInit();
    if (m_TemplateURL != the_value) {
        m_TemplateURL = the_value;
        g.clear();
        setModified(true);
    }
}

css::util::DateTime SAL_CALL
SfxDocumentMetaData::getTemplateDate() throw (css::uno::RuntimeException)
{
    ::osl::MutexGuard g(m_aMutex);
    checkInit();
    return m_TemplateDate;
}

void SAL_CALL
SfxDocumentMetaData::setTemplateDate(const css::util::DateTime & the_value)
        throw (css::uno::RuntimeException)
{
    ::osl::ClearableMutexGuard g(m_aMutex);
    checkInit();
    if (!(m_TemplateDate == the_value)) {
        m_TemplateDate = the_value;
        g.clear();
        setModified(true);
    }
}

::rtl::OUString SAL_CALL
SfxDocumentMetaData::getAutoloadURL() throw (css::uno::RuntimeException)
{
    ::osl::MutexGuard g(m_aMutex);
    checkInit();
    return m_AutoloadURL;
}

void SAL_CALL
SfxDocumentMetaData::setAutoloadURL(const ::rtl::OUString & the_value)
        throw (css::uno::RuntimeException)
{
    ::osl::ClearableMutexGuard g(m_aMutex);
    checkInit();
    if (m_AutoloadURL != the_value) {
        m_AutoloadURL = the_value;
        g.clear();
        setModified(true);
    }
}

::sal_Int32 SAL_CALL
SfxDocumentMetaData::getAutoloadSecs() throw (css::uno::RuntimeException)
{
    ::osl::MutexGuard g(m_aMutex);
    checkInit();
    return m_AutoloadSecs;
}

void SAL_CALL
SfxDocumentMetaData::setAutoloadSecs(::sal_Int32 the_value)
        throw (css::uno::RuntimeException, css::lang::IllegalArgumentException)
{
    if (the_value < 0) throw css::lang::IllegalArgumentException(
        ::rtl::OUString(
            "SfxDocumentMetaData::setAutoloadSecs: argument is negative"),
            *this, 0);
    ::osl::ClearableMutexGuard g(m_aMutex);
    checkInit();
    if (m_AutoloadSecs != the_value) {
        m_AutoloadSecs = the_value;
        g.clear();
        setModified(true);
    }
}

::rtl::OUString SAL_CALL
SfxDocumentMetaData::getDefaultTarget() throw (css::uno::RuntimeException)
{
    ::osl::MutexGuard g(m_aMutex);
    checkInit();
    return m_DefaultTarget;
}

void SAL_CALL
SfxDocumentMetaData::setDefaultTarget(const ::rtl::OUString & the_value)
        throw (css::uno::RuntimeException)
{
    ::osl::ClearableMutexGuard g(m_aMutex);
    checkInit();
    if (m_DefaultTarget != the_value) {
        m_DefaultTarget = the_value;
        g.clear();
        setModified(true);
    }
}

css::uno::Sequence< css::beans::NamedValue > SAL_CALL
SfxDocumentMetaData::getDocumentStatistics() throw (css::uno::RuntimeException)
{
    ::osl::MutexGuard g(m_aMutex);
    checkInit();
    ::comphelper::SequenceAsVector<css::beans::NamedValue> stats;
    for (size_t i = 0; s_stdStats[i] != 0; ++i) {
        const char * aName = s_stdStatAttrs[i];
        ::rtl::OUString text = getMetaAttr("meta:document-statistic", aName);
        if (text.isEmpty()) continue;
        css::beans::NamedValue stat;
        stat.Name = ::rtl::OUString::createFromAscii(s_stdStats[i]);
        sal_Int32 val;
        css::uno::Any any;
        if (!::sax::Converter::convertNumber(val, text, 0,
                std::numeric_limits<sal_Int32>::max()) || (val < 0)) {
            val = 0;
            DBG_WARNING1("SfxDocumentMetaData: invalid number: %s",
                OUStringToOString(text, RTL_TEXTENCODING_UTF8).getStr());
        }
        any <<= val;
        stat.Value = any;
        stats.push_back(stat);
    }

    return stats.getAsConstList();
}

void SAL_CALL
SfxDocumentMetaData::setDocumentStatistics(
        const css::uno::Sequence< css::beans::NamedValue > & the_value)
        throw (css::uno::RuntimeException)
{
    ::osl::ClearableMutexGuard g(m_aMutex);
    checkInit();
    std::vector<std::pair<const char *, ::rtl::OUString> > attributes;
    for (sal_Int32 i = 0; i < the_value.getLength(); ++i) {
        const ::rtl::OUString name = the_value[i].Name;
        // inefficently search for matching attribute
        for (size_t j = 0; s_stdStats[j] != 0; ++j) {
            if (name.equalsAscii(s_stdStats[j])) {
                const css::uno::Any any = the_value[i].Value;
                sal_Int32 val = 0;
                if (any >>= val) {
                    ::rtl::OUStringBuffer buf;
                    ::sax::Converter::convertNumber(buf, val);
                    attributes.push_back(std::make_pair(s_stdStatAttrs[j],
                                buf.makeStringAndClear()));
                } else {
                    DBG_WARNING1("SfxDocumentMetaData: invalid statistic: %s",
                        OUStringToOString(name, RTL_TEXTENCODING_UTF8)
                            .getStr());
                }
                break;
            }
        }
    }
    updateElement("meta:document-statistic", &attributes);
    g.clear();
    setModified(true);
}

::sal_Int16 SAL_CALL
SfxDocumentMetaData::getEditingCycles() throw (css::uno::RuntimeException)
{
    ::osl::MutexGuard g(m_aMutex);
    ::rtl::OUString text = getMetaText("meta:editing-cycles");
    sal_Int32 ret;
    if (::sax::Converter::convertNumber(ret, text,
            0, std::numeric_limits<sal_Int16>::max())) {
        return static_cast<sal_Int16>(ret);
    } else {
        return 0;
    }
}

void SAL_CALL
SfxDocumentMetaData::setEditingCycles(::sal_Int16 the_value)
        throw (css::uno::RuntimeException, css::lang::IllegalArgumentException)
{
    if (the_value < 0) throw css::lang::IllegalArgumentException(
        ::rtl::OUString(
                "SfxDocumentMetaData::setEditingCycles: argument is negative"),
                *this, 0);
    ::rtl::OUStringBuffer buf;
    ::sax::Converter::convertNumber(buf, the_value);
    setMetaTextAndNotify("meta:editing-cycles", buf.makeStringAndClear());
}

::sal_Int32 SAL_CALL
SfxDocumentMetaData::getEditingDuration() throw (css::uno::RuntimeException)
{
    ::osl::MutexGuard g(m_aMutex);
    return textToDuration(getMetaText("meta:editing-duration"));
}

void SAL_CALL
SfxDocumentMetaData::setEditingDuration(::sal_Int32 the_value)
        throw (css::uno::RuntimeException, css::lang::IllegalArgumentException)
{
    if (the_value < 0) throw css::lang::IllegalArgumentException(
        ::rtl::OUString(
            "SfxDocumentMetaData::setEditingDuration: argument is negative"),
            *this, 0);
    setMetaTextAndNotify("meta:editing-duration", durationToText(the_value));
}

void SAL_CALL
SfxDocumentMetaData::resetUserData(const ::rtl::OUString & the_value)
    throw (css::uno::RuntimeException)
{
    ::osl::ClearableMutexGuard g(m_aMutex);

    bool bModified( false );
    bModified |= setMetaText("meta:initial-creator", the_value);
    ::DateTime now( ::DateTime::SYSTEM );
    css::util::DateTime uDT(now.Get100Sec(), now.GetSec(), now.GetMin(),
        now.GetHour(), now.GetDay(), now.GetMonth(), now.GetYear());
    bModified |= setMetaText("meta:creation-date", dateTimeToText(uDT));
    bModified |= setMetaText("dc:creator", ::rtl::OUString());
    bModified |= setMetaText("meta:printed-by", ::rtl::OUString());
    bModified |= setMetaText("dc:date", dateTimeToText(css::util::DateTime()));
    bModified |= setMetaText("meta:print-date",
        dateTimeToText(css::util::DateTime()));
    bModified |= setMetaText("meta:editing-duration", durationToText(0));
    bModified |= setMetaText("meta:editing-cycles",
        ::rtl::OUString("1"));

    if (bModified) {
        g.clear();
        setModified(true);
    }
}


css::uno::Reference< css::beans::XPropertyContainer > SAL_CALL
SfxDocumentMetaData::getUserDefinedProperties()
        throw (css::uno::RuntimeException)
{
    ::osl::MutexGuard g(m_aMutex);
    checkInit();
    createUserDefined();
    return m_xUserDefined;
}


void SAL_CALL
SfxDocumentMetaData::loadFromStorage(
        const css::uno::Reference< css::embed::XStorage > & xStorage,
        const css::uno::Sequence< css::beans::PropertyValue > & Medium)
    throw (css::uno::RuntimeException, css::lang::IllegalArgumentException,
           css::io::WrongFormatException,
           css::lang::WrappedTargetException, css::io::IOException)
{
    if (!xStorage.is()) throw css::lang::IllegalArgumentException(
        ::rtl::OUString("SfxDocumentMetaData::loadFromStorage:"
                " argument is null"), *this, 0);
    ::osl::MutexGuard g(m_aMutex);

    // open meta data file
    css::uno::Reference<css::io::XStream> xStream(
        xStorage->openStreamElement(
            ::rtl::OUString(s_meta),
            css::embed::ElementModes::READ) );
    if (!xStream.is()) throw css::uno::RuntimeException();
    css::uno::Reference<css::io::XInputStream> xInStream =
        xStream->getInputStream();
    if (!xInStream.is()) throw css::uno::RuntimeException();

    // create DOM parser service
    css::uno::Reference<css::lang::XMultiComponentFactory> xMsf (
        m_xContext->getServiceManager());
    css::uno::Reference<css::xml::sax::XParser> xParser = css::xml::sax::Parser::create(m_xContext);
    css::xml::sax::InputSource input;
    input.aInputStream = xInStream;

    sal_uInt64 version = SotStorage::GetVersion( xStorage );
    // Oasis is also the default (0)
    sal_Bool bOasis = ( version > SOFFICE_FILEFORMAT_60 || version == 0 );
    const sal_Char *pServiceName = bOasis
        ? "com.sun.star.document.XMLOasisMetaImporter"
        : "com.sun.star.document.XMLMetaImporter";

    // set base URL
    css::uno::Reference<css::beans::XPropertySet> xPropArg =
        getURLProperties(Medium);
    try {
        xPropArg->getPropertyValue(::rtl::OUString("BaseURI"))
            >>= input.sSystemId;
        input.sSystemId += ::rtl::OUString("/").concat(
                ::rtl::OUString(s_meta));
    } catch (const css::uno::Exception &) {
        input.sSystemId = ::rtl::OUString(s_meta);
    }
    css::uno::Sequence< css::uno::Any > args(1);
    args[0] <<= xPropArg;

    css::uno::Reference<css::xml::sax::XDocumentHandler> xDocHandler (
        xMsf->createInstanceWithArgumentsAndContext(
            ::rtl::OUString::createFromAscii(pServiceName), args, m_xContext),
        css::uno::UNO_QUERY_THROW);
    if (!xDocHandler.is()) throw css::uno::RuntimeException(
        ::rtl::OUString("SfxDocumentMetaData::loadFromStorage:"
                " cannot create XMLOasisMetaImporter service"), *this);
    css::uno::Reference<css::document::XImporter> xImp (xDocHandler,
        css::uno::UNO_QUERY_THROW);
    xImp->setTargetDocument(css::uno::Reference<css::lang::XComponent>(this));
    xParser->setDocumentHandler(xDocHandler);
    try {
        xParser->parseStream(input);
    } catch (const css::xml::sax::SAXException &) {
        throw css::io::WrongFormatException(::rtl::OUString(
                "SfxDocumentMetaData::loadFromStorage:"
                " XML parsing exception"), *this);
    }
    // NB: the implementation of XMLOasisMetaImporter calls initialize
    checkInit();
}

void SAL_CALL
SfxDocumentMetaData::storeToStorage(
        const css::uno::Reference< css::embed::XStorage > & xStorage,
        const css::uno::Sequence< css::beans::PropertyValue > & Medium)
    throw (css::uno::RuntimeException, css::lang::IllegalArgumentException,
           css::lang::WrappedTargetException, css::io::IOException)
{
    if (!xStorage.is()) throw css::lang::IllegalArgumentException(
        ::rtl::OUString("SfxDocumentMetaData::storeToStorage:"
                " argument is null"), *this, 0);
    ::osl::MutexGuard g(m_aMutex);
    checkInit();

    // update user-defined meta data in DOM tree
//    updateUserDefinedAndAttributes(); // this will be done in serialize!

    // write into storage
    css::uno::Reference<css::io::XStream> xStream =
        xStorage->openStreamElement(::rtl::OUString(s_meta),
            css::embed::ElementModes::WRITE
            | css::embed::ElementModes::TRUNCATE);
    if (!xStream.is()) throw css::uno::RuntimeException();
    css::uno::Reference< css::beans::XPropertySet > xStreamProps(xStream,
        css::uno::UNO_QUERY_THROW);
    xStreamProps->setPropertyValue(
        ::rtl::OUString("MediaType"),
        css::uno::makeAny(::rtl::OUString("text/xml")));
    xStreamProps->setPropertyValue(
        ::rtl::OUString("Compressed"),
        css::uno::makeAny(static_cast<sal_Bool> (sal_False)));
    xStreamProps->setPropertyValue(
        ::rtl::OUString("UseCommonStoragePasswordEncryption"),
        css::uno::makeAny(static_cast<sal_Bool> (sal_False)));
    css::uno::Reference<css::io::XOutputStream> xOutStream =
        xStream->getOutputStream();
    if (!xOutStream.is()) throw css::uno::RuntimeException();
    css::uno::Reference<css::lang::XMultiComponentFactory> xMsf (
        m_xContext->getServiceManager());
    css::uno::Reference<css::io::XActiveDataSource> xSaxWriter(
        xMsf->createInstanceWithContext(::rtl::OUString(
                "com.sun.star.xml.sax.Writer"), m_xContext),
        css::uno::UNO_QUERY_THROW);
    xSaxWriter->setOutputStream(xOutStream);
    css::uno::Reference<css::xml::sax::XDocumentHandler> xDocHandler (
        xSaxWriter, css::uno::UNO_QUERY_THROW);

    const sal_uInt64 version = SotStorage::GetVersion( xStorage );
    // Oasis is also the default (0)
    const sal_Bool bOasis = ( version > SOFFICE_FILEFORMAT_60 || version == 0 );
    const sal_Char *pServiceName = bOasis
        ? "com.sun.star.document.XMLOasisMetaExporter"
        : "com.sun.star.document.XMLMetaExporter";

    // set base URL
    css::uno::Reference<css::beans::XPropertySet> xPropArg =
        getURLProperties(Medium);
    css::uno::Sequence< css::uno::Any > args(2);
    args[0] <<= xDocHandler;
    args[1] <<= xPropArg;

    css::uno::Reference<css::document::XExporter> xExp(
        xMsf->createInstanceWithArgumentsAndContext(
            ::rtl::OUString::createFromAscii(pServiceName), args, m_xContext),
        css::uno::UNO_QUERY_THROW);
    xExp->setSourceDocument(css::uno::Reference<css::lang::XComponent>(this));
    css::uno::Reference<css::document::XFilter> xFilter(xExp,
        css::uno::UNO_QUERY_THROW);
    if (xFilter->filter(css::uno::Sequence< css::beans::PropertyValue >())) {
        css::uno::Reference<css::embed::XTransactedObject> xTransaction(
            xStorage, css::uno::UNO_QUERY);
        if (xTransaction.is()) {
            xTransaction->commit();
        }
    } else {
        throw css::io::IOException(::rtl::OUString(
                "SfxDocumentMetaData::storeToStorage: cannot filter"), *this);
    }
}

void SAL_CALL
SfxDocumentMetaData::loadFromMedium(const ::rtl::OUString & URL,
        const css::uno::Sequence< css::beans::PropertyValue > & Medium)
    throw (css::uno::RuntimeException, css::io::WrongFormatException,
           css::lang::WrappedTargetException, css::io::IOException)
{
    css::uno::Reference<css::io::XInputStream> xIn;
    ::comphelper::MediaDescriptor md(Medium);
    // if we have an URL parameter, it replaces the one in the media descriptor
    if (!URL.isEmpty()) {
        md[ ::comphelper::MediaDescriptor::PROP_URL() ] <<= URL;
    }
    if (sal_True == md.addInputStream()) {
        md[ ::comphelper::MediaDescriptor::PROP_INPUTSTREAM() ] >>= xIn;
    }
    css::uno::Reference<css::embed::XStorage> xStorage;
    css::uno::Reference<css::lang::XMultiServiceFactory> xMsf (
        m_xContext->getServiceManager(), css::uno::UNO_QUERY_THROW);
    try {
        if (xIn.is()) {
            xStorage = ::comphelper::OStorageHelper::GetStorageFromInputStream(
                            xIn, xMsf);
        } else { // fallback to url parameter
            xStorage = ::comphelper::OStorageHelper::GetStorageFromURL(
                            URL, css::embed::ElementModes::READ, xMsf);
        }
    } catch (const css::uno::RuntimeException &) {
        throw;
    } catch (const css::io::IOException &) {
        throw;
    } catch (const css::uno::Exception & e) {
        throw css::lang::WrappedTargetException(
                ::rtl::OUString(
                    "SfxDocumentMetaData::loadFromMedium: exception"),
                css::uno::Reference<css::uno::XInterface>(*this),
                css::uno::makeAny(e));
    }
    if (!xStorage.is()) {
        throw css::uno::RuntimeException(::rtl::OUString(
                "SfxDocumentMetaData::loadFromMedium: cannot get Storage"),
                *this);
    }
    loadFromStorage(xStorage, md.getAsConstPropertyValueList());
}

void SAL_CALL
SfxDocumentMetaData::storeToMedium(const ::rtl::OUString & URL,
        const css::uno::Sequence< css::beans::PropertyValue > & Medium)
    throw (css::uno::RuntimeException,
           css::lang::WrappedTargetException, css::io::IOException)
{
    ::comphelper::MediaDescriptor md(Medium);
    if (!URL.isEmpty()) {
        md[ ::comphelper::MediaDescriptor::PROP_URL() ] <<= URL;
    }
    SfxMedium aMedium(md.getAsConstPropertyValueList());
    css::uno::Reference<css::embed::XStorage> xStorage
        = aMedium.GetOutputStorage();


    if (!xStorage.is()) {
        throw css::uno::RuntimeException(::rtl::OUString(
                "SfxDocumentMetaData::storeToMedium: cannot get Storage"),
                *this);
    }
    // set MIME type of the storage
    ::comphelper::MediaDescriptor::const_iterator iter
        = md.find(::comphelper::MediaDescriptor::PROP_MEDIATYPE());
    if (iter != md.end()) {
        css::uno::Reference< css::beans::XPropertySet > xProps(xStorage,
            css::uno::UNO_QUERY_THROW);
        xProps->setPropertyValue(
            ::comphelper::MediaDescriptor::PROP_MEDIATYPE(),
            iter->second);
    }
    storeToStorage(xStorage, md.getAsConstPropertyValueList());


    const sal_Bool bOk = aMedium.Commit();
    aMedium.Close();
    if ( !bOk ) {
        sal_uInt32 nError = aMedium.GetError();
        if ( nError == ERRCODE_NONE ) {
            nError = ERRCODE_IO_GENERAL;
        }

        throw css::task::ErrorCodeIOException( ::rtl::OUString(),
                css::uno::Reference< css::uno::XInterface >(), nError);

    }
}

// ::com::sun::star::lang::XInitialization:
void SAL_CALL
SfxDocumentMetaData::initialize(
        const css::uno::Sequence< ::com::sun::star::uno::Any > & aArguments)
    throw (css::uno::RuntimeException, css::uno::Exception)
{
    // possible arguments:
    // - no argument: default initialization (empty DOM)
    // - 1 argument, XDocument: initialize with given DOM and empty base URL
    // NB: links in document must be absolute

    ::osl::MutexGuard g(m_aMutex);
    css::uno::Reference<css::xml::dom::XDocument> xDoc;

    for (sal_Int32 i = 0; i < aArguments.getLength(); ++i) {
        const css::uno::Any any = aArguments[i];
        if (any >>= xDoc) {
            if (!xDoc.is()) {
                throw css::lang::IllegalArgumentException(
                    ::rtl::OUString("SfxDocumentMetaData::"
                        "initialize: argument is null"),
                    *this, static_cast<sal_Int16>(i));
            }
        } else {
            throw css::lang::IllegalArgumentException(
                ::rtl::OUString("SfxDocumentMetaData::"
                    "initialize: argument must be XDocument"),
                *this, static_cast<sal_Int16>(i));
        }
    }

    if (!xDoc.is()) {
        // For a new document, we create a new DOM tree here.
        xDoc = createDOM();
    }

    init(xDoc);
}

// ::com::sun::star::util::XCloneable:
css::uno::Reference<css::util::XCloneable> SAL_CALL
SfxDocumentMetaData::createClone()
    throw (css::uno::RuntimeException)
{
    ::osl::MutexGuard g(m_aMutex);
    checkInit();

    SfxDocumentMetaData *pNew = createMe(m_xContext);

    // NB: do not copy the modification listeners, only DOM
    css::uno::Reference<css::xml::dom::XDocument> xDoc = createDOM();
    try {
        updateUserDefinedAndAttributes();
        // deep copy of root node
        css::uno::Reference<css::xml::dom::XNode> xRoot(
            m_xDoc->getDocumentElement(), css::uno::UNO_QUERY_THROW);
        css::uno::Reference<css::xml::dom::XNode> xRootNew(
            xDoc->importNode(xRoot, true));
        xDoc->appendChild(xRootNew);
        pNew->init(xDoc);
    } catch (const css::uno::RuntimeException &) {
        throw;
    } catch (const css::uno::Exception & e) {
        css::uno::Any a(e);
        throw css::lang::WrappedTargetRuntimeException(
                ::rtl::OUString(
                    "SfxDocumentMetaData::createClone: exception"),
                css::uno::Reference<css::uno::XInterface>(*this), a);
    }
    return css::uno::Reference<css::util::XCloneable> (pNew);
}

// ::com::sun::star::util::XModifiable:
::sal_Bool SAL_CALL SfxDocumentMetaData::isModified(  )
        throw (css::uno::RuntimeException)
{
    ::osl::MutexGuard g(m_aMutex);
    checkInit();
    css::uno::Reference<css::util::XModifiable> xMB(m_xUserDefined,
        css::uno::UNO_QUERY);
    return m_isModified || (xMB.is() ? xMB->isModified() : sal_False);
}

void SAL_CALL SfxDocumentMetaData::setModified( ::sal_Bool bModified )
        throw (css::beans::PropertyVetoException, css::uno::RuntimeException)
{
    css::uno::Reference<css::util::XModifiable> xMB;
    { // do not lock mutex while notifying (#i93514#) to prevent deadlock
        ::osl::MutexGuard g(m_aMutex);
        checkInit();
        m_isModified = bModified;
        if ( !bModified && m_xUserDefined.is() )
        {
            xMB.set(m_xUserDefined, css::uno::UNO_QUERY);
            DBG_ASSERT(xMB.is(),
                "SfxDocumentMetaData::setModified: PropertyBag not Modifiable?");
        }
    }
    if (bModified) {
        try {
            css::uno::Reference<css::uno::XInterface> xThis(*this);
            css::lang::EventObject event(xThis);
            m_NotifyListeners.notifyEach(&css::util::XModifyListener::modified,
                event);
        } catch (const css::uno::RuntimeException &) {
            throw;
        } catch (const css::uno::Exception & e) {
            // ignore
            DBG_WARNING1("SfxDocumentMetaData::setModified: exception:\n%s",
                OUStringToOString(e.Message, RTL_TEXTENCODING_UTF8).getStr());
            (void) e;
        }
    } else {
        if (xMB.is()) {
            xMB->setModified(false);
        }
    }
}

// ::com::sun::star::util::XModifyBroadcaster:
void SAL_CALL SfxDocumentMetaData::addModifyListener(
        const css::uno::Reference< css::util::XModifyListener > & xListener)
        throw (css::uno::RuntimeException)
{
    ::osl::MutexGuard g(m_aMutex);
    checkInit();
    m_NotifyListeners.addInterface(xListener);
    css::uno::Reference<css::util::XModifyBroadcaster> xMB(m_xUserDefined,
        css::uno::UNO_QUERY);
    if (xMB.is()) {
        xMB->addModifyListener(xListener);
    }
}

void SAL_CALL SfxDocumentMetaData::removeModifyListener(
        const css::uno::Reference< css::util::XModifyListener > & xListener)
        throw (css::uno::RuntimeException)
{
    ::osl::MutexGuard g(m_aMutex);
    checkInit();
    m_NotifyListeners.removeInterface(xListener);
    css::uno::Reference<css::util::XModifyBroadcaster> xMB(m_xUserDefined,
        css::uno::UNO_QUERY);
    if (xMB.is()) {
        xMB->removeModifyListener(xListener);
    }
}

// ::com::sun::star::xml::sax::XSAXSerializable
void SAL_CALL SfxDocumentMetaData::serialize(
    const css::uno::Reference<css::xml::sax::XDocumentHandler>& i_xHandler,
    const css::uno::Sequence< css::beans::StringPair >& i_rNamespaces)
    throw (css::uno::RuntimeException, css::xml::sax::SAXException)
{
    ::osl::MutexGuard g(m_aMutex);
    checkInit();
    updateUserDefinedAndAttributes();
    css::uno::Reference<css::xml::sax::XSAXSerializable> xSAXable(m_xDoc,
        css::uno::UNO_QUERY_THROW);
    xSAXable->serialize(i_xHandler, i_rNamespaces);
}

void SfxDocumentMetaData::createUserDefined()
{
    // user-defined meta data: create PropertyBag which only accepts property
    // values of allowed types
    if ( !m_xUserDefined.is() )
    {
        css::uno::Sequence<css::uno::Type> types(11);
        types[0] = ::cppu::UnoType<bool>::get();
        types[1] = ::cppu::UnoType< ::rtl::OUString>::get();
        types[2] = ::cppu::UnoType<css::util::DateTime>::get();
        types[3] = ::cppu::UnoType<css::util::Date>::get();
        types[4] = ::cppu::UnoType<css::util::Duration>::get();
        types[5] = ::cppu::UnoType<float>::get();
        types[6] = ::cppu::UnoType<double>::get();
        types[7] = ::cppu::UnoType<sal_Int16>::get();
        types[8] = ::cppu::UnoType<sal_Int32>::get();
        types[9] = ::cppu::UnoType<sal_Int64>::get();
        // Time is supported for backward compatibility with OOo 3.x, x<=2
        types[10] = ::cppu::UnoType<css::util::Time>::get();
        css::uno::Sequence<css::uno::Any> args(2);
        args[0] <<= css::beans::NamedValue(
            ::rtl::OUString("AllowedTypes"),
            css::uno::makeAny(types));
        // #i94175#:  ODF allows empty user-defined property names!
        args[1] <<= css::beans::NamedValue( ::rtl::OUString(
                        "AllowEmptyPropertyName"),
            css::uno::makeAny(sal_True));

        const css::uno::Reference<css::lang::XMultiComponentFactory> xMsf(
                m_xContext->getServiceManager());
        m_xUserDefined.set(
            xMsf->createInstanceWithContext(
                ::rtl::OUString(
                    "com.sun.star.beans.PropertyBag"), m_xContext),
            css::uno::UNO_QUERY_THROW);
        const css::uno::Reference<css::lang::XInitialization> xInit(
            m_xUserDefined, css::uno::UNO_QUERY);
        if (xInit.is()) {
            xInit->initialize(args);
        }

        const css::uno::Reference<css::util::XModifyBroadcaster> xMB(
            m_xUserDefined, css::uno::UNO_QUERY);
        if (xMB.is())
        {
            const css::uno::Sequence<css::uno::Reference<css::uno::XInterface> >
                listeners(m_NotifyListeners.getElements());
            for (css::uno::Reference< css::uno::XInterface > const * iter =
                                ::comphelper::stl_begin(listeners);
                        iter != ::comphelper::stl_end(listeners); ++iter) {
                xMB->addModifyListener(
                    css::uno::Reference< css::util::XModifyListener >(*iter,
                        css::uno::UNO_QUERY));
            }
        }
    }
}

} // closing anonymous implementation namespace


// component helper namespace
namespace comp_CompatWriterDocProps {

    ::rtl::OUString SAL_CALL _getImplementationName() {
        return ::rtl::OUString(
            "CompatWriterDocPropsImpl");
}

   css::uno::Sequence< ::rtl::OUString > SAL_CALL _getSupportedServiceNames()
   {
        css::uno::Sequence< rtl::OUString > aServiceNames(1);
        aServiceNames[ 0 ] = rtl::OUString( "com.sun.star.writer.DocumentProperties"  );
        return aServiceNames;
   }
    css::uno::Reference< css::uno::XInterface > SAL_CALL _create(
        const css::uno::Reference< css::uno::XComponentContext > & context)
            SAL_THROW((css::uno::Exception))
    {
        return static_cast< ::cppu::OWeakObject * >
                    (new CompatWriterDocPropsImpl(context));
    }

}
namespace comp_SfxDocumentMetaData {

::rtl::OUString SAL_CALL _getImplementationName() {
    return ::rtl::OUString(
        "SfxDocumentMetaData");
}

css::uno::Sequence< ::rtl::OUString > SAL_CALL _getSupportedServiceNames()
{
    css::uno::Sequence< ::rtl::OUString > s(1);
    s[0] = ::rtl::OUString(
        "com.sun.star.document.DocumentProperties");
    return s;
}

css::uno::Reference< css::uno::XInterface > SAL_CALL _create(
    const css::uno::Reference< css::uno::XComponentContext > & context)
        SAL_THROW((css::uno::Exception))
{
    return static_cast< ::cppu::OWeakObject * >
                (new SfxDocumentMetaData(context));
}

} // closing component helper namespace

static ::cppu::ImplementationEntry const entries[] = {
    { &comp_SfxDocumentMetaData::_create,
      &comp_SfxDocumentMetaData::_getImplementationName,
      &comp_SfxDocumentMetaData::_getSupportedServiceNames,
      &::cppu::createSingleComponentFactory, 0, 0 },
    { 0, 0, 0, 0, 0, 0 }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

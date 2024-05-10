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

#include <sal/config.h>
#include <sal/log.hxx>

#include <comphelper/compbase.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/document/XDocumentProperties.hpp>
#include <com/sun/star/document/XDocumentProperties2.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/util/XCloneable.hpp>
#include <com/sun/star/util/XModifiable.hpp>
#include <com/sun/star/xml/sax/SAXException.hpp>
#include <com/sun/star/xml/sax/XSAXSerializable.hpp>

#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>
#include <com/sun/star/lang/EventObject.hpp>
#include <com/sun/star/beans/IllegalTypeException.hpp>
#include <com/sun/star/beans/PropertyExistException.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/task/ErrorCodeIOException.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/embed/XTransactedObject.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/io/WrongFormatException.hpp>
#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/document/XImporter.hpp>
#include <com/sun/star/document/XExporter.hpp>
#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/xml/sax/Writer.hpp>
#include <com/sun/star/xml/sax/Parser.hpp>
#include <com/sun/star/xml/sax/XFastParser.hpp>
#include <com/sun/star/xml/dom/DOMException.hpp>
#include <com/sun/star/xml/dom/XDocument.hpp>
#include <com/sun/star/xml/dom/XElement.hpp>
#include <com/sun/star/xml/dom/DocumentBuilder.hpp>
#include <com/sun/star/xml/dom/NodeType.hpp>
#include <com/sun/star/xml/xpath/XPathAPI.hpp>
#include <com/sun/star/util/Date.hpp>
#include <com/sun/star/util/Time.hpp>
#include <com/sun/star/util/DateWithTimezone.hpp>
#include <com/sun/star/util/DateTimeWithTimezone.hpp>
#include <com/sun/star/util/Duration.hpp>

#include <rtl/ustrbuf.hxx>
#include <tools/datetime.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <osl/mutex.hxx>
#include <comphelper/fileformat.h>
#include <comphelper/interfacecontainer3.hxx>
#include <comphelper/storagehelper.hxx>
#include <unotools/mediadescriptor.hxx>
#include <comphelper/sequence.hxx>
#include <sot/storage.hxx>
#include <sfx2/docfile.hxx>
#include <sax/tools/converter.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <optional>

#include <algorithm>
#include <utility>
#include <vector>
#include <map>
#include <cstring>
#include <limits>


#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <com/sun/star/document/XCompatWriterDocProperties.hpp>
#include <com/sun/star/beans/PropertyBag.hpp>

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
 */

/// anonymous implementation namespace
namespace {

/// a list of attribute-lists, where attribute means name and content
typedef std::vector<std::vector<std::pair<OUString, OUString> > >
        AttrVector;

typedef ::comphelper::WeakComponentImplHelper<
            css::lang::XServiceInfo,
            css::document::XDocumentProperties2,
            css::lang::XInitialization,
            css::util::XCloneable,
            css::util::XModifiable,
            css::xml::sax::XSAXSerializable>
    SfxDocumentMetaData_Base;

class SfxDocumentMetaData:
    public SfxDocumentMetaData_Base
{
public:
    explicit SfxDocumentMetaData(
        css::uno::Reference< css::uno::XComponentContext > const & context);
    SfxDocumentMetaData(const SfxDocumentMetaData&) = delete;
    SfxDocumentMetaData& operator=(const SfxDocumentMetaData&) = delete;

    // css::lang::XServiceInfo:
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService(
        const OUString & ServiceName) override;
    virtual css::uno::Sequence< OUString > SAL_CALL
        getSupportedServiceNames() override;

    // css::lang::XComponent:
    virtual void disposing(std::unique_lock<std::mutex>& rGuard) override;

    // css::document::XDocumentProperties:
    virtual OUString SAL_CALL getAuthor() override;
    virtual void SAL_CALL setAuthor(const OUString & the_value) override;
    virtual OUString SAL_CALL getGenerator() override;
    virtual void SAL_CALL setGenerator(const OUString & the_value) override;
    virtual css::util::DateTime SAL_CALL getCreationDate() override;
    virtual void SAL_CALL setCreationDate(const css::util::DateTime & the_value) override;
    virtual OUString SAL_CALL getTitle() override;
    virtual void SAL_CALL setTitle(const OUString & the_value) override;
    virtual OUString SAL_CALL getSubject() override;
    virtual void SAL_CALL setSubject(const OUString & the_value) override;
    virtual OUString SAL_CALL getDescription() override;
    virtual void SAL_CALL setDescription(const OUString & the_value) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getKeywords() override;
    virtual void SAL_CALL setKeywords(
        const css::uno::Sequence< OUString > & the_value) override;
    virtual css::lang::Locale SAL_CALL getLanguage() override;
    virtual void SAL_CALL setLanguage(const css::lang::Locale & the_value) override;
    virtual OUString SAL_CALL getModifiedBy() override;
    virtual void SAL_CALL setModifiedBy(const OUString & the_value) override;
    virtual css::util::DateTime SAL_CALL getModificationDate() override;
    virtual void SAL_CALL setModificationDate(
            const css::util::DateTime & the_value) override;
    virtual OUString SAL_CALL getPrintedBy() override;
    virtual void SAL_CALL setPrintedBy(const OUString & the_value) override;
    virtual css::util::DateTime SAL_CALL getPrintDate() override;
    virtual void SAL_CALL setPrintDate(const css::util::DateTime & the_value) override;
    virtual OUString SAL_CALL getTemplateName() override;
    virtual void SAL_CALL setTemplateName(const OUString & the_value) override;
    virtual OUString SAL_CALL getTemplateURL() override;
    virtual void SAL_CALL setTemplateURL(const OUString & the_value) override;
    virtual css::util::DateTime SAL_CALL getTemplateDate() override;
    virtual void SAL_CALL setTemplateDate(const css::util::DateTime & the_value) override;
    virtual OUString SAL_CALL getAutoloadURL() override;
    virtual void SAL_CALL setAutoloadURL(const OUString & the_value) override;
    virtual ::sal_Int32 SAL_CALL getAutoloadSecs() override;
    virtual void SAL_CALL setAutoloadSecs(::sal_Int32 the_value) override;
    virtual OUString SAL_CALL getDefaultTarget() override;
    virtual void SAL_CALL setDefaultTarget(const OUString & the_value) override;
    virtual css::uno::Sequence< css::beans::NamedValue > SAL_CALL
        getDocumentStatistics() override;
    virtual void SAL_CALL setDocumentStatistics(
        const css::uno::Sequence< css::beans::NamedValue > & the_value) override;
    virtual ::sal_Int16 SAL_CALL getEditingCycles() override;
    virtual void SAL_CALL setEditingCycles(::sal_Int16 the_value) override;
    virtual ::sal_Int32 SAL_CALL getEditingDuration() override;
    virtual void SAL_CALL setEditingDuration(::sal_Int32 the_value) override;
    virtual void SAL_CALL resetUserData(const OUString & the_value) override;
    virtual css::uno::Reference< css::beans::XPropertyContainer > SAL_CALL
        getUserDefinedProperties() override;
    virtual void SAL_CALL loadFromStorage(
        const css::uno::Reference< css::embed::XStorage > & Storage,
        const css::uno::Sequence< css::beans::PropertyValue > & Medium) override;
    virtual void SAL_CALL loadFromMedium(const OUString & URL,
        const css::uno::Sequence< css::beans::PropertyValue > & Medium) override;
    virtual void SAL_CALL storeToStorage(
        const css::uno::Reference< css::embed::XStorage > & Storage,
        const css::uno::Sequence< css::beans::PropertyValue > & Medium) override;
    virtual void SAL_CALL storeToMedium(const OUString & URL,
        const css::uno::Sequence< css::beans::PropertyValue > & Medium) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getContributor() override;
    virtual void SAL_CALL setContributor(const css::uno::Sequence< OUString >& the_value) override;
    virtual OUString SAL_CALL getCoverage() override;
    virtual void SAL_CALL setCoverage(const OUString & the_value) override;
    virtual OUString SAL_CALL getIdentifier() override;
    virtual void SAL_CALL setIdentifier(const OUString & the_value) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getPublisher() override;
    virtual void SAL_CALL setPublisher(const css::uno::Sequence< OUString > & the_value) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getRelation() override;
    virtual void SAL_CALL setRelation(const css::uno::Sequence< OUString > & the_value) override;
    virtual OUString SAL_CALL getRights() override;
    virtual void SAL_CALL setRights(const OUString & the_value) override;
    virtual OUString SAL_CALL getSource() override;
    virtual void SAL_CALL setSource(const OUString& the_value) override;
    virtual OUString SAL_CALL getType() override;
    virtual void SAL_CALL setType(const OUString& the_value) override;


    // css::lang::XInitialization:
    virtual void SAL_CALL initialize(
        const css::uno::Sequence< css::uno::Any > & aArguments) override;

    // css::util::XCloneable:
    virtual css::uno::Reference<css::util::XCloneable> SAL_CALL createClone() override;

    // css::util::XModifiable:
    virtual sal_Bool SAL_CALL isModified(  ) override;
    virtual void SAL_CALL setModified( sal_Bool bModified ) override;

    // css::util::XModifyBroadcaster:
    virtual void SAL_CALL addModifyListener(
        const css::uno::Reference< css::util::XModifyListener > & xListener) override;
    virtual void SAL_CALL removeModifyListener(
        const css::uno::Reference< css::util::XModifyListener > & xListener) override;

    // css::xml::sax::XSAXSerializable
    virtual void SAL_CALL serialize(
        const css::uno::Reference<css::xml::sax::XDocumentHandler>& i_xHandler,
        const css::uno::Sequence< css::beans::StringPair >& i_rNamespaces) override;

protected:
    virtual ~SfxDocumentMetaData() override {}
    virtual rtl::Reference<SfxDocumentMetaData> createMe( css::uno::Reference< css::uno::XComponentContext > const & context ) { return new SfxDocumentMetaData( context ); };
    const css::uno::Reference< css::uno::XComponentContext > m_xContext;

    /// for notification
    ::comphelper::OInterfaceContainerHelper4<css::util::XModifyListener> m_NotifyListeners;
    /// flag: false means not initialized yet, or disposed
    bool m_isInitialized;
    /// flag
    bool m_isModified;
    /// meta-data DOM tree
    css::uno::Reference< css::xml::dom::XDocument > m_xDoc;
    /// meta-data super node in the meta-data DOM tree
    css::uno::Reference< css::xml::dom::XNode> m_xParent;
    /// standard meta data (single occurrence)
    std::map< OUString, css::uno::Reference<css::xml::dom::XNode> >
        m_meta;
    /// standard meta data (multiple occurrences)
    std::map< OUString,
        std::vector<css::uno::Reference<css::xml::dom::XNode> > > m_metaList;
    /// user-defined meta data (meta:user-defined) @ATTENTION may be null!
    css::uno::Reference<css::beans::XPropertyContainer> m_xUserDefined;
    // now for some meta-data attributes; these are not updated directly in the
    // DOM because updates (detecting "empty" elements) would be quite messy
    OUString m_TemplateName;
    OUString m_TemplateURL;
    css::util::DateTime m_TemplateDate;
    OUString m_AutoloadURL;
    sal_Int32 m_AutoloadSecs;
    OUString m_DefaultTarget;

    /// check if we are initialized properly
    void checkInit(std::unique_lock<std::mutex>& rGuard) const;
    /// initialize state from given DOM tree
    void init(std::unique_lock<std::mutex>& rGuard, const css::uno::Reference<css::xml::dom::XDocument>& i_xDom);
    /// update element in DOM tree
    void updateElement(std::unique_lock<std::mutex>& rGuard,
        const OUString & i_name,
        std::vector<std::pair<OUString, OUString> >* i_pAttrs = nullptr);
    /// update user-defined meta data and attributes in DOM tree
    void updateUserDefinedAndAttributes(std::unique_lock<std::mutex>& rGuard);
    /// create empty DOM tree (XDocument)
    css::uno::Reference<css::xml::dom::XDocument> createDOM() const;
    /// extract base URL (necessary for converting relative links)
    css::uno::Reference<css::beans::XPropertySet> getURLProperties(
        std::unique_lock<std::mutex>& rGuard,
        const css::uno::Sequence<css::beans::PropertyValue> & i_rMedium) const;
    /// get text of standard meta data element
    OUString getMetaText(std::unique_lock<std::mutex>& rGuard, const char* i_name) const;
    /// set text of standard meta data element iff not equal to existing text
    bool setMetaText(std::unique_lock<std::mutex>& g, const OUString& i_name,
        const OUString & i_rValue);
    /// set text of standard meta data element iff not equal to existing text
    void setMetaTextAndNotify(const OUString& i_name,
        const OUString & i_rValue);
    /// get text of standard meta data element's attribute
    OUString getMetaAttr(std::unique_lock<std::mutex>& rGuard,
        const OUString& i_name,
        const OUString& i_attr) const;
    /// get text of a list of standard meta data elements (multiple occ.)
    css::uno::Sequence< OUString > getMetaList(
        std::unique_lock<std::mutex>& rGuard,
        const char* i_name) const;
    /// set text of a list of standard meta data elements (multiple occ.)
    bool setMetaList(std::unique_lock<std::mutex>& rGuard, const OUString& i_name,
        const css::uno::Sequence< OUString > & i_rValue,
        AttrVector const*);
    void createUserDefined(std::unique_lock<std::mutex>& rGuard);
};

typedef ::cppu::ImplInheritanceHelper< SfxDocumentMetaData, css::document::XCompatWriterDocProperties > CompatWriterDocPropsImpl_BASE;

class CompatWriterDocPropsImpl : public CompatWriterDocPropsImpl_BASE
{
    OUString msManager;
    OUString msCategory;
    OUString msCompany;
protected:
    virtual rtl::Reference<SfxDocumentMetaData> createMe( css::uno::Reference< css::uno::XComponentContext > const & context ) override { return new CompatWriterDocPropsImpl( context ); };
public:
    explicit CompatWriterDocPropsImpl( css::uno::Reference< css::uno::XComponentContext > const & context) : CompatWriterDocPropsImpl_BASE( context ) {}

// XCompatWriterDocPropsImpl
    virtual OUString SAL_CALL getManager() override { return msManager; }
    virtual void SAL_CALL setManager( const OUString& _manager ) override { msManager = _manager; }
    virtual OUString SAL_CALL getCategory() override { return msCategory; }
    virtual void SAL_CALL setCategory( const OUString& _category ) override { msCategory = _category; }
    virtual OUString SAL_CALL getCompany() override { return msCompany; }
    virtual void SAL_CALL setCompany( const OUString& _company ) override { msCompany = _company; }

// XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) override
    {
        return u"CompatWriterDocPropsImpl"_ustr;
    }

    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override
    {
        return cppu::supportsService(this, ServiceName);
    }

    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) override
    {
        css::uno::Sequence<OUString> aServiceNames { u"com.sun.star.writer.DocumentProperties"_ustr };
        return aServiceNames;
    }
};

constexpr OUString sMetaPageCount = u"meta:page-count"_ustr;
constexpr OUString sMetaTableCount = u"meta:table-count"_ustr;
constexpr OUString sMetaDrawCount = u"meta:draw-count"_ustr;
constexpr OUString sMetaImageCount = u"meta:image-count"_ustr;
constexpr OUString sMetaObjectCount = u"meta:object-count"_ustr;
constexpr OUString sMetaOleObjectCount = u"meta:ole-object-count"_ustr;
constexpr OUString sMetaParagraphCount = u"meta:paragraph-count"_ustr;
constexpr OUString sMetaWordCount = u"meta:word-count"_ustr;
constexpr OUString sMetaCharacterCount = u"meta:character-count"_ustr;
constexpr OUString sMetaRowCount = u"meta:row-count"_ustr;
constexpr OUString sMetaFrameCount = u"meta:frame-count"_ustr;
constexpr OUString sMetaSentenceCount = u"meta:sentence-count"_ustr;
constexpr OUString sMetaSyllableCount = u"meta:syllable-count"_ustr;
constexpr OUString sMetaNonWhitespaceCharacterCount = u"meta:non-whitespace-character-count"_ustr;
constexpr OUString sMetaCellCount = u"meta:cell-count"_ustr;

// NB: keep these two arrays in sync!
constexpr OUString s_stdStatAttrs[] = {
    sMetaPageCount,
    sMetaTableCount,
    sMetaDrawCount,
    sMetaImageCount,
    sMetaObjectCount,
    sMetaOleObjectCount,
    sMetaParagraphCount,
    sMetaWordCount,
    sMetaCharacterCount,
    sMetaRowCount,
    sMetaFrameCount,
    sMetaSentenceCount,
    sMetaSyllableCount,
    sMetaNonWhitespaceCharacterCount,
    sMetaCellCount
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
    nullptr
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
    "meta:auto-reload",
    "meta:hyperlink-behaviour",
    "dc:language",              // language
    "meta:editing-cycles",      // nonNegativeInteger
    "meta:editing-duration",    // duration
    "meta:document-statistic",  // ... // note: statistic is singular, no s!
    "dc:coverage",
    "dc:identifier",
    "dc:rights",
    "dc:source",
    "dc:type",
    nullptr
};

constexpr OUString sMetaKeyword = u"meta:keyword"_ustr;
constexpr OUString sMetaUserDefined = u"meta:user-defined"_ustr;
constexpr OUString sDCContributor = u"dc:contributor"_ustr;
constexpr OUString sDCPublisher = u"dc:publisher"_ustr;
constexpr OUString sDCRelation = u"dc:relation"_ustr;
constexpr OUString s_stdMetaList[] {
    sMetaKeyword,             // string*
    sMetaUserDefined,        // ...*
    sDCContributor, // string*
    sDCPublisher, // string*
    sDCRelation, // string*
};

constexpr OUStringLiteral s_nsXLink = u"http://www.w3.org/1999/xlink";
constexpr OUString s_nsDC = u"http://purl.org/dc/elements/1.1/"_ustr;
constexpr OUString s_nsODF = u"urn:oasis:names:tc:opendocument:xmlns:office:1.0"_ustr;
constexpr OUString s_nsODFMeta = u"urn:oasis:names:tc:opendocument:xmlns:meta:1.0"_ustr;
// constexpr OUStringLiteral s_nsOOo = "http://openoffice.org/2004/office"; // not used (yet?)

constexpr OUString s_meta = u"meta.xml"_ustr;

bool isValidDate(const css::util::Date & i_rDate)
{
    return i_rDate.Month > 0;
}

bool isValidDateTime(const css::util::DateTime & i_rDateTime)
{
    return i_rDateTime.Month > 0;
}

std::pair< OUString, OUString >
getQualifier(const OUString& nm) {
    sal_Int32 ix = nm.indexOf(u':');
    if (ix == -1) {
        return std::make_pair(OUString(), nm);
    } else {
        return std::make_pair(nm.copy(0,ix), nm.copy(ix+1));
    }
}

// get namespace for standard qualified names
// NB: only call this with statically known strings!
OUString getNameSpace(const OUString& i_qname) noexcept
{
    OUString ns;
    OUString n = getQualifier(i_qname).first;
    if ( n == "xlink" ) ns = s_nsXLink;
    if ( n == "dc" ) ns = s_nsDC;
    if ( n == "office" ) ns = s_nsODF;
    if ( n == "meta" ) ns = s_nsODFMeta;
    assert(!ns.isEmpty());
    return ns;
}

bool
textToDateOrDateTime(css::util::Date & io_rd, css::util::DateTime & io_rdt,
        bool & o_rIsDateTime, std::optional<sal_Int16> & o_rTimeZone,
        const OUString& i_text) noexcept
{
    if (::sax::Converter::parseDateOrDateTime(
                &io_rd, io_rdt, o_rIsDateTime, &o_rTimeZone, i_text)) {
        return true;
    } else {
        SAL_WARN_IF(!i_text.isEmpty(), "sfx.doc", "Invalid date: " << i_text);
        return false;
    }
}

// convert string to date/time
bool
textToDateTime(css::util::DateTime & io_rdt, const OUString& i_text) noexcept
{
    if (::sax::Converter::parseDateTime(io_rdt, i_text)) {
        return true;
    } else {
        SAL_WARN_IF(!i_text.isEmpty(), "sfx.doc", "Invalid date: " << i_text);
        return false;
    }
}

// convert string to date/time with default return value
css::util::DateTime
textToDateTimeDefault(const OUString& i_text) noexcept
{
    css::util::DateTime dt;
    static_cast<void> (textToDateTime(dt, i_text));
    // on conversion error: return default value (unchanged)
    return dt;
}

// convert date to string
OUString
dateToText(css::util::Date const& i_rd,
           sal_Int16 const*const pTimeZone) noexcept
{
    if (isValidDate(i_rd)) {
        OUStringBuffer buf;
        ::sax::Converter::convertDate(buf, i_rd, pTimeZone);
        return buf.makeStringAndClear();
    } else {
        return OUString();
    }
}


// convert date/time to string
OUString
dateTimeToText(css::util::DateTime const& i_rdt,
               sal_Int16 const*const pTimeZone = nullptr) noexcept
{
    if (isValidDateTime(i_rdt)) {
        OUStringBuffer buf(32);
        ::sax::Converter::convertDateTime(buf, i_rdt, pTimeZone, true);
        return buf.makeStringAndClear();
    } else {
        return OUString();
    }
}

// convert string to duration
bool
textToDuration(css::util::Duration& io_rDur, OUString const& i_rText)
noexcept
{
    if (::sax::Converter::convertDuration(io_rDur, i_rText)) {
        return true;
    } else {
        SAL_WARN_IF(!i_rText.isEmpty(), "sfx.doc", "Invalid duration: " << i_rText);
        return false;
    }
}

sal_Int32 textToDuration(OUString const& i_rText) noexcept
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
OUString durationToText(css::util::Duration const& i_rDur) noexcept
{
    OUStringBuffer buf;
    ::sax::Converter::convertDuration(buf, i_rDur);
    return buf.makeStringAndClear();
}

// convert duration to string
OUString durationToText(sal_Int32 i_value) noexcept
{
    css::util::Duration ud;
    ud.Days    = static_cast<sal_Int16>(i_value / (24 * 3600));
    ud.Hours   = static_cast<sal_Int16>((i_value % (24 * 3600)) / 3600);
    ud.Minutes = static_cast<sal_Int16>((i_value % 3600) / 60);
    ud.Seconds = static_cast<sal_Int16>(i_value % 60);
    ud.NanoSeconds = 0;
    return durationToText(ud);
}

// extract base URL (necessary for converting relative links)
css::uno::Reference< css::beans::XPropertySet >
SfxDocumentMetaData::getURLProperties(
    std::unique_lock<std::mutex>& /*rGuard*/,
    const css::uno::Sequence< css::beans::PropertyValue > & i_rMedium) const
{
    css::uno::Reference< css::beans::XPropertyBag> xPropArg = css::beans::PropertyBag::createDefault( m_xContext );
    try {
        css::uno::Any baseUri;
        for (const auto& rProp : i_rMedium) {
            if (rProp.Name == "DocumentBaseURL") {
                baseUri = rProp.Value;
            } else if (rProp.Name == "URL") {
                if (!baseUri.hasValue()) {
                    baseUri = rProp.Value;
                }
            } else if (rProp.Name == "HierarchicalDocumentName") {
                xPropArg->addProperty(
                    u"StreamRelPath"_ustr,
                    css::beans::PropertyAttribute::MAYBEVOID,
                    rProp.Value);
            }
        }
        if (baseUri.hasValue()) {
            xPropArg->addProperty(
                u"BaseURI"_ustr, css::beans::PropertyAttribute::MAYBEVOID,
                baseUri);
        }
        xPropArg->addProperty(u"StreamName"_ustr,
                css::beans::PropertyAttribute::MAYBEVOID,
                css::uno::Any(s_meta));
    } catch (const css::uno::Exception &) {
        // ignore
    }
    return css::uno::Reference< css::beans::XPropertySet>(xPropArg,
                css::uno::UNO_QUERY_THROW);
}

// return the text of the (hopefully unique, i.e., normalize first!) text
// node _below_ the given node
/// @throws css::uno::RuntimeException
OUString
getNodeText(const css::uno::Reference<css::xml::dom::XNode>& i_xNode)
{
    if (!i_xNode.is())
        throw css::uno::RuntimeException(u"SfxDocumentMetaData::getNodeText: argument is null"_ustr, i_xNode);
    for (css::uno::Reference<css::xml::dom::XNode> c = i_xNode->getFirstChild();
            c.is();
            c = c->getNextSibling()) {
        if (c->getNodeType() == css::xml::dom::NodeType_TEXT_NODE) {
            try {
                return c->getNodeValue();
            } catch (const css::xml::dom::DOMException &) { // too big?
                return OUString();
            }
        }
    }
    return OUString();
}

OUString
SfxDocumentMetaData::getMetaText(std::unique_lock<std::mutex>& rGuard, const char* i_name) const
//        throw (css::uno::RuntimeException)
{
    checkInit(rGuard);

    const OUString name( OUString::createFromAscii(i_name) );
    assert(m_meta.find(name) != m_meta.end());
    css::uno::Reference<css::xml::dom::XNode> xNode = m_meta.find(name)->second;
    return (xNode.is()) ? getNodeText(xNode) : OUString();
}

bool
SfxDocumentMetaData::setMetaText(std::unique_lock<std::mutex>& rGuard, const OUString& name,
        const OUString & i_rValue)
    // throw (css::uno::RuntimeException)
{
    checkInit(rGuard);

    assert(m_meta.find(name) != m_meta.end());
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
                        if (c->getNodeValue() != i_rValue) {
                            c->setNodeValue(i_rValue);
                            return true;
                        } else {
                            return false;
                        }
                    }
                }
            } else { // insert
                xNode.set(m_xDoc->createElementNS(getNameSpace(name), name),
                            css::uno::UNO_QUERY_THROW);
                m_xParent->appendChild(xNode);
                m_meta[name] = xNode;
            }
            css::uno::Reference<css::xml::dom::XNode> xTextNode(
                m_xDoc->createTextNode(i_rValue), css::uno::UNO_QUERY_THROW);
            xNode->appendChild(xTextNode);
            return true;
        }
    } catch (const css::xml::dom::DOMException &) {
        css::uno::Any anyEx = cppu::getCaughtException();
        throw css::lang::WrappedTargetRuntimeException(
                u"SfxDocumentMetaData::setMetaText: DOM exception"_ustr,
                css::uno::Reference<css::uno::XInterface>(*this), anyEx);
    }
}

void
SfxDocumentMetaData::setMetaTextAndNotify(const OUString & i_name,
        const OUString & i_rValue)
    // throw (css::uno::RuntimeException)
{
    std::unique_lock g(m_aMutex);
    if (setMetaText(g, i_name, i_rValue)) {
        g.unlock();
        setModified(true);
    }
}

OUString
SfxDocumentMetaData::getMetaAttr(std::unique_lock<std::mutex>& /*rGuard*/, const OUString& name, const OUString& i_attr) const
//        throw (css::uno::RuntimeException)
{
    assert(m_meta.find(name) != m_meta.end());
    css::uno::Reference<css::xml::dom::XNode> xNode = m_meta.find(name)->second;
    if (xNode.is()) {
        css::uno::Reference<css::xml::dom::XElement> xElem(xNode,
            css::uno::UNO_QUERY_THROW);
        return xElem->getAttributeNS(getNameSpace(i_attr),
                    getQualifier(i_attr).second);
    } else {
        return OUString();
    }
}

css::uno::Sequence< OUString>
SfxDocumentMetaData::getMetaList(std::unique_lock<std::mutex>& rGuard, const char* i_name) const
//        throw (css::uno::RuntimeException)
{
    checkInit(rGuard);
    OUString name = OUString::createFromAscii(i_name);
    assert(m_metaList.find(name) != m_metaList.end());
    std::vector<css::uno::Reference<css::xml::dom::XNode> > const & vec =
        m_metaList.find(name)->second;
    css::uno::Sequence< OUString> ret(vec.size());
    std::transform(vec.begin(), vec.end(), ret.getArray(),
                   [](const auto& node) { return getNodeText(node); });
    return ret;
}

bool
SfxDocumentMetaData::setMetaList(std::unique_lock<std::mutex>& rGuard, const OUString& name,
        const css::uno::Sequence<OUString> & i_rValue,
        AttrVector const* i_pAttrs)
    // throw (css::uno::RuntimeException)
{
    checkInit(rGuard);
    assert((i_pAttrs == nullptr) ||
           (static_cast<size_t>(i_rValue.getLength()) == i_pAttrs->size()));

    try {
        assert(m_metaList.find(name) != m_metaList.end());
        std::vector<css::uno::Reference<css::xml::dom::XNode> > & vec =
            m_metaList[name];

        // if nothing changed, do nothing
        // alas, this does not check for permutations, or attributes...
        if (nullptr == i_pAttrs) {
            if (static_cast<size_t>(i_rValue.getLength()) == vec.size()) {
                bool isEqual(true);
                for (sal_Int32 i = 0; i < i_rValue.getLength(); ++i) {
                    css::uno::Reference<css::xml::dom::XNode> xNode(vec.at(i));
                    if (xNode.is()) {
                        OUString val = getNodeText(xNode);
                        if (val != i_rValue[i]) {
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
                m_xDoc->createElementNS(getNameSpace(name), name),
                css::uno::UNO_SET_THROW);
            css::uno::Reference<css::xml::dom::XNode> xNode(xElem,
                css::uno::UNO_QUERY_THROW);
            css::uno::Reference<css::xml::dom::XNode> xTextNode(
                m_xDoc->createTextNode(i_rValue[i]), css::uno::UNO_QUERY_THROW);
            // set attributes
            if (i_pAttrs != nullptr) {
                for (auto const& elem : (*i_pAttrs)[i])
                {
                    xElem->setAttributeNS(getNameSpace(elem.first),
                        elem.first, elem.second);
                }
            }
            xNode->appendChild(xTextNode);
            m_xParent->appendChild(xNode);
            vec.push_back(xNode);
        }

        return true;
    } catch (const css::xml::dom::DOMException &) {
        css::uno::Any anyEx = cppu::getCaughtException();
        throw css::lang::WrappedTargetRuntimeException(
                u"SfxDocumentMetaData::setMetaList: DOM exception"_ustr,
                css::uno::Reference<css::uno::XInterface>(*this), anyEx);
    }
}

// convert property list to string list and attribute list
std::pair<css::uno::Sequence< OUString>, AttrVector>
propsToStrings(css::uno::Reference<css::beans::XPropertySet> const & i_xPropSet)
{
    ::std::vector< OUString > values;
    AttrVector attrs;

    css::uno::Reference<css::beans::XPropertySetInfo> xSetInfo
        = i_xPropSet->getPropertySetInfo();
    css::uno::Sequence<css::beans::Property> props = xSetInfo->getProperties();

    for (sal_Int32 i = 0; i < props.getLength(); ++i) {
        if (props[i].Attributes & css::beans::PropertyAttribute::TRANSIENT) {
            continue;
        }
        const OUString name = props[i].Name;
        css::uno::Any any;
        try {
            any = i_xPropSet->getPropertyValue(name);
        } catch (const css::uno::Exception &) {
            // ignore
        }
        const css::uno::Type & type = any.getValueType();
        std::vector<std::pair<OUString, OUString> > as;
        as.emplace_back("meta:name", name);
        static constexpr OUString vt = u"meta:value-type"_ustr;

        // convert according to type
        if (type == ::cppu::UnoType<bool>::get()) {
            bool b = false;
            any >>= b;
            OUStringBuffer buf;
            ::sax::Converter::convertBool(buf, b);
            values.push_back(buf.makeStringAndClear());
            as.emplace_back(vt, u"boolean"_ustr);
        } else if (type == ::cppu::UnoType< OUString>::get()) {
            OUString s;
            any >>= s;
            values.push_back(s);
// #i90847# OOo 2.x does stupid things if value-type="string";
// fortunately string is default anyway, so we can just omit it
// #i107502#: however, OOo 2.x only reads 4 user-defined without @value-type
// => best backward compatibility: first 4 without @value-type, rest with
            if (4 <= i)
            {
                as.emplace_back(vt, u"string"_ustr);
            }
        } else if (type == ::cppu::UnoType<css::util::DateTime>::get()) {
            css::util::DateTime dt;
            any >>= dt;
            values.push_back(dateTimeToText(dt));
            as.emplace_back(vt, u"date"_ustr);
        } else if (type == ::cppu::UnoType<css::util::Date>::get()) {
            css::util::Date d;
            any >>= d;
            values.push_back(dateToText(d, nullptr));
            as.emplace_back(vt,u"date"_ustr);
        } else if (type == ::cppu::UnoType<css::util::DateTimeWithTimezone>::get()) {
            css::util::DateTimeWithTimezone dttz;
            any >>= dttz;
            values.push_back(dateTimeToText(dttz.DateTimeInTZ, &dttz.Timezone));
            as.emplace_back(vt, u"date"_ustr);
        } else if (type == ::cppu::UnoType<css::util::DateWithTimezone>::get()) {
            css::util::DateWithTimezone dtz;
            any >>= dtz;
            values.push_back(dateToText(dtz.DateInTZ, &dtz.Timezone));
            as.emplace_back(vt, u"date"_ustr);
        } else if (type == ::cppu::UnoType<css::util::Time>::get()) {
            // #i97029#: replaced by Duration
            // Time is supported for backward compatibility with OOo 3.x, x<=2
            css::util::Time ut;
            any >>= ut;
            css::util::Duration ud;
            ud.Hours   = ut.Hours;
            ud.Minutes = ut.Minutes;
            ud.Seconds = ut.Seconds;
            ud.NanoSeconds = ut.NanoSeconds;
            values.push_back(durationToText(ud));
            as.emplace_back(vt, u"time"_ustr);
        } else if (type == ::cppu::UnoType<css::util::Duration>::get()) {
            css::util::Duration ud;
            any >>= ud;
            values.push_back(durationToText(ud));
            as.emplace_back(vt, u"time"_ustr);
        } else if (::cppu::UnoType<double>::get().isAssignableFrom(type)) {
            // support not just double, but anything that can be converted
            double d = 0;
            any >>= d;
            OUStringBuffer buf;
            ::sax::Converter::convertDouble(buf, d);
            values.push_back(buf.makeStringAndClear());
            as.emplace_back(vt, u"float"_ustr);
        } else {
            SAL_WARN("sfx.doc", "Unsupported property type: " << any.getValueTypeName() );
            continue;
        }
        attrs.push_back(as);
    }

    return std::make_pair(comphelper::containerToSequence(values), attrs);
}

// remove the given element from the DOM, and iff i_pAttrs != 0 insert new one
void
SfxDocumentMetaData::updateElement(std::unique_lock<std::mutex>& /*rGuard*/, const OUString& name,
        std::vector<std::pair<OUString, OUString> >* i_pAttrs)
{
    try {
        // remove old element
        css::uno::Reference<css::xml::dom::XNode> xNode =
            m_meta.find(name)->second;
        if (xNode.is()) {
            m_xParent->removeChild(xNode);
            xNode.clear();
        }
        // add new element
        if (nullptr != i_pAttrs) {
            css::uno::Reference<css::xml::dom::XElement> xElem(
                m_xDoc->createElementNS(getNameSpace(name), name),
                    css::uno::UNO_SET_THROW);
            xNode.set(xElem, css::uno::UNO_QUERY_THROW);
            // set attributes
            for (auto const& elem : *i_pAttrs)
            {
                xElem->setAttributeNS(getNameSpace(elem.first),
                    elem.first, elem.second);
            }
            m_xParent->appendChild(xNode);
        }
        m_meta[name] = xNode;
    } catch (const css::xml::dom::DOMException &) {
        css::uno::Any anyEx = cppu::getCaughtException();
        throw css::lang::WrappedTargetRuntimeException(
                u"SfxDocumentMetaData::updateElement: DOM exception"_ustr,
                css::uno::Reference<css::uno::XInterface>(*this), anyEx);
    }
}

// update user-defined meta data in DOM tree
void SfxDocumentMetaData::updateUserDefinedAndAttributes(std::unique_lock<std::mutex>& g)
{
    createUserDefined(g);
    const css::uno::Reference<css::beans::XPropertySet> xPSet(m_xUserDefined,
            css::uno::UNO_QUERY_THROW);
    const std::pair<css::uno::Sequence< OUString>, AttrVector>
        udStringsAttrs( propsToStrings(xPSet) );
    (void) setMetaList(g, u"meta:user-defined"_ustr, udStringsAttrs.first,
            &udStringsAttrs.second);

    // update elements with attributes
    std::vector<std::pair<OUString, OUString> > attributes;
    if (!m_TemplateName.isEmpty() || !m_TemplateURL.isEmpty()
            || isValidDateTime(m_TemplateDate)) {
        attributes.emplace_back("xlink:type", u"simple"_ustr);
        attributes.emplace_back("xlink:actuate", u"onRequest"_ustr);
        attributes.emplace_back("xlink:title", m_TemplateName);
        attributes.emplace_back("xlink:href", m_TemplateURL );
        if (isValidDateTime(m_TemplateDate)) {
            attributes.emplace_back(
                "meta:date", dateTimeToText(m_TemplateDate));
        }
        updateElement(g, u"meta:template"_ustr, &attributes);
    } else {
        updateElement(g, u"meta:template"_ustr);
    }
    attributes.clear();

    if (!m_AutoloadURL.isEmpty() || (0 != m_AutoloadSecs)) {
        attributes.emplace_back("xlink:href", m_AutoloadURL );
        attributes.emplace_back("meta:delay",
                durationToText(m_AutoloadSecs));
        updateElement(g, u"meta:auto-reload"_ustr, &attributes);
    } else {
        updateElement(g, u"meta:auto-reload"_ustr);
    }
    attributes.clear();

    if (!m_DefaultTarget.isEmpty()) {
        attributes.emplace_back(
                "office:target-frame-name",
                m_DefaultTarget);
        // xlink:show: _blank -> new, any other value -> replace
        const char* show = m_DefaultTarget == "_blank" ? "new" : "replace";
        attributes.emplace_back(
                "xlink:show",
                OUString::createFromAscii(show));
        updateElement(g, u"meta:hyperlink-behaviour"_ustr, &attributes);
    } else {
        updateElement(g, u"meta:hyperlink-behaviour"_ustr);
    }
    attributes.clear();
}

// create empty DOM tree (XDocument)
css::uno::Reference<css::xml::dom::XDocument>
SfxDocumentMetaData::createDOM() const // throw (css::uno::RuntimeException)
{
    css::uno::Reference<css::xml::dom::XDocumentBuilder> xBuilder( css::xml::dom::DocumentBuilder::create(m_xContext) );
    css::uno::Reference<css::xml::dom::XDocument> xDoc = xBuilder->newDocument();
    if (!xDoc.is())
        throw css::uno::RuntimeException(
                u"SfxDocumentMetaData::createDOM: cannot create new document"_ustr,
                *const_cast<SfxDocumentMetaData*>(this));
    return xDoc;
}

void
SfxDocumentMetaData::checkInit(std::unique_lock<std::mutex>& /*rGuard*/) const // throw (css::uno::RuntimeException)
{
    if (!m_isInitialized) {
        throw css::uno::RuntimeException(
                u"SfxDocumentMetaData::checkInit: not initialized"_ustr,
                *const_cast<SfxDocumentMetaData*>(this));
    }
    assert(m_xDoc.is() && m_xParent.is());
}

void extractTagAndNamespaceUri(std::u16string_view aChildNodeName,
                std::u16string_view& rTagName, std::u16string_view& rNamespaceURI)
{
    size_t idx = aChildNodeName.find(':');
    assert(idx != std::u16string_view::npos);
    std::u16string_view aPrefix = aChildNodeName.substr(0, idx);
    rTagName = aChildNodeName.substr(idx + 1);
    if (aPrefix == u"dc")
        rNamespaceURI = s_nsDC;
    else if (aPrefix == u"meta")
        rNamespaceURI = s_nsODFMeta;
    else if (aPrefix == u"office")
        rNamespaceURI = s_nsODF;
    else
        assert(false);
}


css::uno::Reference<css::xml::dom::XElement> getChildNodeByName(
                const css::uno::Reference<css::xml::dom::XNode>& xNode,
                std::u16string_view aChildNodeName)
{
    css::uno::Reference< css::xml::dom::XNodeList > xList = xNode->getChildNodes();
    if (!xList)
        return nullptr;
    std::u16string_view aTagName, aNamespaceURI;
    extractTagAndNamespaceUri(aChildNodeName, aTagName, aNamespaceURI);

    const sal_Int32 nLength(xList->getLength());
    for (sal_Int32 a(0); a < nLength; a++)
    {
        const css::uno::Reference< css::xml::dom::XElement > xChild(xList->item(a), css::uno::UNO_QUERY);
        if (xChild && xChild->getNodeName() == aTagName && aNamespaceURI == xChild->getNamespaceURI())
            return xChild;
    }
    return nullptr;
}


std::vector<css::uno::Reference<css::xml::dom::XNode> > getChildNodeListByName(
                const css::uno::Reference<css::xml::dom::XNode>& xNode,
                std::u16string_view aChildNodeName)
{
    css::uno::Reference< css::xml::dom::XNodeList > xList = xNode->getChildNodes();
    if (!xList)
        return {};
    std::u16string_view aTagName, aNamespaceURI;
    extractTagAndNamespaceUri(aChildNodeName, aTagName, aNamespaceURI);
    std::vector<css::uno::Reference<css::xml::dom::XNode>> aList;
    const sal_Int32 nLength(xList->getLength());
    for (sal_Int32 a(0); a < nLength; a++)
    {
        const css::uno::Reference< css::xml::dom::XElement > xChild(xList->item(a), css::uno::UNO_QUERY);
        if (xChild && xChild->getNodeName() == aTagName && aNamespaceURI == xChild->getNamespaceURI())
            aList.push_back(xChild);
    }
    return aList;
}

// initialize state from DOM tree
void SfxDocumentMetaData::init(
        std::unique_lock<std::mutex>& g,
        const css::uno::Reference<css::xml::dom::XDocument>& i_xDoc)
{
    if (!i_xDoc.is())
        throw css::uno::RuntimeException(u"SfxDocumentMetaData::init: no DOM tree given"_ustr, *this);

    m_isInitialized = false;
    m_xDoc = i_xDoc;

    // select nodes for standard meta data stuff
    // NB: we do not handle the single-XML-file ODF variant, which would
    //     have the root element office:document.
    //     The root of such documents must be converted in the importer!
    css::uno::Reference<css::xml::dom::XNode> xDocNode(
        m_xDoc, css::uno::UNO_QUERY_THROW);
    m_xParent.clear();
    try {
        css::uno::Reference<css::xml::dom::XNode> xChild = getChildNodeByName(xDocNode, u"office:document-meta");
        if (xChild)
            m_xParent = getChildNodeByName(xChild, u"office:meta");
    } catch (const css::uno::Exception &) {
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
                    if ( xNode->getNamespaceURI() == s_nsODF && xNode->getLocalName() == "document-meta" )
                    {
                        xRElem.set(xNode, css::uno::UNO_QUERY_THROW);
                        break;
                    }
                    else
                    {
                        SAL_INFO("sfx.doc", "SfxDocumentMetaData::init(): "
                                "deleting unexpected root element: "
                                << xNode->getLocalName());
                        i_xDoc->removeChild(xNode);
                        xNode = i_xDoc->getFirstChild(); // start over
                    }
                } else {
                    xNode = xNode->getNextSibling();
                }
            }
            if (!xRElem.is()) {
                static constexpr OUStringLiteral sOfficeDocumentMeta = u"office:document-meta";
                xRElem = i_xDoc->createElementNS(
                    s_nsODF, sOfficeDocumentMeta);
                css::uno::Reference<css::xml::dom::XNode> xRNode(xRElem,
                    css::uno::UNO_QUERY_THROW);
                i_xDoc->appendChild(xRNode);
            }
            static constexpr OUStringLiteral sOfficeVersion = u"office:version";
            xRElem->setAttributeNS(s_nsODF, sOfficeVersion, u"1.0"_ustr);
            // does not exist, otherwise m_xParent would not be null
            static constexpr OUStringLiteral sOfficeMeta = u"office:meta";
            css::uno::Reference<css::xml::dom::XNode> xParent (
                i_xDoc->createElementNS(s_nsODF, sOfficeMeta),
                css::uno::UNO_QUERY_THROW);
            xRElem->appendChild(xParent);
            m_xParent = xParent;
        } catch (const css::xml::dom::DOMException &) {
            css::uno::Any anyEx = cppu::getCaughtException();
            throw css::lang::WrappedTargetRuntimeException(
                    u"SfxDocumentMetaData::init: DOM exception"_ustr,
                    css::uno::Reference<css::uno::XInterface>(*this), anyEx);
        }
    }


    // select nodes for elements of which we only handle one occurrence
    for (const char **pName = s_stdMeta; *pName != nullptr; ++pName) {
        OUString name = OUString::createFromAscii(*pName);
        // NB: If a document contains more than one occurrence of a
        // meta-data element, we arbitrarily pick one of them here.
        // We do not remove the others, i.e., when we write the
        // document, it will contain the duplicates unchanged.
        // The ODF spec says that handling multiple occurrences is
        // application-specific.
        css::uno::Reference<css::xml::dom::XNode> xNode =
                getChildNodeByName(m_xParent, name);
        // Do not create an empty element if it is missing;
        // for certain elements, such as dateTime, this would be invalid
        m_meta[name] = xNode;
    }

    // select nodes for elements of which we handle all occurrences
    for (const auto & name : s_stdMetaList) {
        std::vector<css::uno::Reference<css::xml::dom::XNode> > nodes =
            getChildNodeListByName(m_xParent, name);
        m_metaList[name] = nodes;
    }

    // initialize members corresponding to attributes from DOM nodes
    static constexpr OUString sMetaTemplate = u"meta:template"_ustr;
    static constexpr OUString sMetaAutoReload = u"meta:auto-reload"_ustr;
    static constexpr OUStringLiteral sMetaHyperlinkBehaviour = u"meta:hyperlink-behaviour";
    m_TemplateName  = getMetaAttr(g, sMetaTemplate, u"xlink:title"_ustr);
    m_TemplateURL   = getMetaAttr(g, sMetaTemplate, u"xlink:href"_ustr);
    m_TemplateDate  =
        textToDateTimeDefault(getMetaAttr(g, sMetaTemplate, u"meta:date"_ustr));
    m_AutoloadURL   = getMetaAttr(g, sMetaAutoReload, u"xlink:href"_ustr);
    m_AutoloadSecs  =
        textToDuration(getMetaAttr(g, sMetaAutoReload, u"meta:delay"_ustr));
    m_DefaultTarget =
        getMetaAttr(g, sMetaHyperlinkBehaviour, u"office:target-frame-name"_ustr);


    std::vector<css::uno::Reference<css::xml::dom::XNode> > & vec =
        m_metaList[u"meta:user-defined"_ustr];
    m_xUserDefined.clear(); // #i105826#: reset (may be re-initialization)
    if ( !vec.empty() )
    {
        createUserDefined(g);
    }

    // user-defined meta data: initialize PropertySet from DOM nodes
    for (auto const& elem : vec)
    {
        css::uno::Reference<css::xml::dom::XElement> xElem(elem,
            css::uno::UNO_QUERY_THROW);
        css::uno::Any any;
        OUString name = xElem->getAttributeNS(s_nsODFMeta, u"name"_ustr);
        OUString type = xElem->getAttributeNS(s_nsODFMeta, u"value-type"_ustr);
        OUString text = getNodeText(elem);
        if ( type == "float" ) {
            double d;
            if (::sax::Converter::convertDouble(d, text)) {
                any <<= d;
            } else {
                SAL_WARN("sfx.doc", "Invalid float: " << text);
                continue;
            }
        } else if ( type == "date" ) {
            bool isDateTime;
            css::util::Date d;
            css::util::DateTime dt;
            std::optional<sal_Int16> nTimeZone;
            if (textToDateOrDateTime(d, dt, isDateTime, nTimeZone, text)) {
                if (isDateTime) {
                    if (nTimeZone) {
                        any <<= css::util::DateTimeWithTimezone(dt,
                                    *nTimeZone);
                    } else {
                        any <<= dt;
                    }
                } else {
                    if (nTimeZone) {
                        any <<= css::util::DateWithTimezone(d, *nTimeZone);
                    } else {
                        any <<= d;
                    }
                }
            } else {
                SAL_WARN("sfx.doc", "Invalid date: " << text);
                continue;
            }
        } else if ( type == "time" ) {
            css::util::Duration ud;
            if (textToDuration(ud, text)) {
                any <<= ud;
            } else {
                SAL_WARN("sfx.doc", "Invalid time: " << text);
                continue;
            }
        } else if ( type == "boolean" ) {
            bool b;
            if (::sax::Converter::convertBool(b, text)) {
                any <<= b;
            } else {
                SAL_WARN("sfx.doc", "Invalid boolean: " << text);
                continue;
            }
        } else { // default
            any <<= text;
        }
        try {
            m_xUserDefined->addProperty(name,
                css::beans::PropertyAttribute::REMOVABLE, any);
        } catch (const css::beans::PropertyExistException &) {
            SAL_WARN("sfx.doc", "Duplicate: " << name);
            // ignore; duplicate
        } catch (const css::beans::IllegalTypeException &) {
            SAL_INFO("sfx.doc", "SfxDocumentMetaData: illegal type: " << name);
        } catch (const css::lang::IllegalArgumentException &) {
            SAL_INFO("sfx.doc", "SfxDocumentMetaData: illegal arg: " << name);
        }
    }

    m_isModified = false;
    m_isInitialized = true;
}


SfxDocumentMetaData::SfxDocumentMetaData(
        css::uno::Reference< css::uno::XComponentContext > const & context)
    : m_xContext(context)
    , m_isInitialized(false)
    , m_isModified(false)
    , m_AutoloadSecs(0)
{
    assert(context.is());
    assert(context->getServiceManager().is());
    std::unique_lock g(m_aMutex);
    init(g, createDOM());
}

// com.sun.star.uno.XServiceInfo:
OUString SAL_CALL
SfxDocumentMetaData::getImplementationName()
{
    return u"SfxDocumentMetaData"_ustr;
}

sal_Bool SAL_CALL
SfxDocumentMetaData::supportsService(OUString const & serviceName)
{
    return cppu::supportsService(this, serviceName);
}

css::uno::Sequence< OUString > SAL_CALL
SfxDocumentMetaData::getSupportedServiceNames()
{
    css::uno::Sequence< OUString > s { u"com.sun.star.document.DocumentProperties"_ustr };
    return s;
}


// css::lang::XComponent:
void SfxDocumentMetaData::disposing(std::unique_lock<std::mutex>& rGuard)
{
    m_NotifyListeners.disposeAndClear(rGuard, css::lang::EventObject(
            getXWeak()));
    m_isInitialized = false;
    m_meta.clear();
    m_metaList.clear();
    m_xParent.clear();
    m_xDoc.clear();
    m_xUserDefined.clear();
}


// css::document::XDocumentProperties:
OUString SAL_CALL
SfxDocumentMetaData::getAuthor()
{
    std::unique_lock g(m_aMutex);
    return getMetaText(g, "meta:initial-creator");
}

void SAL_CALL SfxDocumentMetaData::setAuthor(const OUString & the_value)
{
    setMetaTextAndNotify(u"meta:initial-creator"_ustr, the_value);
}


OUString SAL_CALL
SfxDocumentMetaData::getGenerator()
{
    std::unique_lock g(m_aMutex);
    return getMetaText(g, "meta:generator");
}

void SAL_CALL
SfxDocumentMetaData::setGenerator(const OUString & the_value)
{
    setMetaTextAndNotify(u"meta:generator"_ustr, the_value);
}

css::util::DateTime SAL_CALL
SfxDocumentMetaData::getCreationDate()
{
    std::unique_lock g(m_aMutex);
    return textToDateTimeDefault(getMetaText(g, "meta:creation-date"));
}

void SAL_CALL
SfxDocumentMetaData::setCreationDate(const css::util::DateTime & the_value)
{
    setMetaTextAndNotify(u"meta:creation-date"_ustr, dateTimeToText(the_value));
}

OUString SAL_CALL
SfxDocumentMetaData::getTitle()
{
    std::unique_lock g(m_aMutex);
    return getMetaText(g, "dc:title");
}

void SAL_CALL SfxDocumentMetaData::setTitle(const OUString & the_value)
{
    setMetaTextAndNotify(u"dc:title"_ustr, the_value);
}

OUString SAL_CALL
SfxDocumentMetaData::getSubject()
{
    std::unique_lock g(m_aMutex);
    return getMetaText(g, "dc:subject");
}

void SAL_CALL
SfxDocumentMetaData::setSubject(const OUString & the_value)
{
    setMetaTextAndNotify(u"dc:subject"_ustr, the_value);
}

OUString SAL_CALL
SfxDocumentMetaData::getDescription()
{
    std::unique_lock g(m_aMutex);
    return getMetaText(g, "dc:description");
}

void SAL_CALL
SfxDocumentMetaData::setDescription(const OUString & the_value)
{
    setMetaTextAndNotify(u"dc:description"_ustr, the_value);
}

css::uno::Sequence< OUString >
SAL_CALL SfxDocumentMetaData::getKeywords()
{
    std::unique_lock g(m_aMutex);
    return getMetaList(g, "meta:keyword");
}

void SAL_CALL
SfxDocumentMetaData::setKeywords(
        const css::uno::Sequence< OUString > & the_value)
{
    std::unique_lock g(m_aMutex);
    if (setMetaList(g, u"meta:keyword"_ustr, the_value, nullptr)) {
        g.unlock();
        setModified(true);
    }
}

// css::document::XDocumentProperties2
css::uno::Sequence<OUString> SAL_CALL SfxDocumentMetaData::getContributor()
{
    std::unique_lock g(m_aMutex);
    return getMetaList(g, "dc:contributor");
}

void SAL_CALL SfxDocumentMetaData::setContributor(const css::uno::Sequence<OUString>& the_value)
{
    std::unique_lock g(m_aMutex);
    if (setMetaList(g, u"dc:contributor"_ustr, the_value, nullptr))
    {
        g.unlock();
        setModified(true);
    }
}

OUString SAL_CALL SfxDocumentMetaData::getCoverage()
{
    std::unique_lock g(m_aMutex);
    return getMetaText(g, "dc:coverage");
}

void SAL_CALL SfxDocumentMetaData::setCoverage(const OUString& the_value)
{
    setMetaTextAndNotify(u"dc:coverage"_ustr, the_value);
}

OUString SAL_CALL SfxDocumentMetaData::getIdentifier()
{
    std::unique_lock g(m_aMutex);
    return getMetaText(g, "dc:identifier");
}

void SAL_CALL SfxDocumentMetaData::setIdentifier(const OUString& the_value)
{
    setMetaTextAndNotify(u"dc:identifier"_ustr, the_value);
}

css::uno::Sequence<OUString> SAL_CALL SfxDocumentMetaData::getPublisher()
{
    std::unique_lock g(m_aMutex);
    return getMetaList(g, "dc:publisher");
}

void SAL_CALL SfxDocumentMetaData::setPublisher(const css::uno::Sequence<OUString>& the_value)
{
    std::unique_lock g(m_aMutex);
    if (setMetaList(g, u"dc:publisher"_ustr, the_value, nullptr))
    {
        g.unlock();
        setModified(true);
    }
}

css::uno::Sequence<OUString> SAL_CALL SfxDocumentMetaData::getRelation()
{
    std::unique_lock g(m_aMutex);
    return getMetaList(g, "dc:relation");
}

void SAL_CALL SfxDocumentMetaData::setRelation(const css::uno::Sequence<OUString>& the_value)
{
    std::unique_lock g(m_aMutex);
    if (setMetaList(g, u"dc:relation"_ustr, the_value, nullptr))
    {
        g.unlock();
        setModified(true);
    }
}

OUString SAL_CALL SfxDocumentMetaData::getRights()
{
    std::unique_lock g(m_aMutex);
    return getMetaText(g, "dc:rights");
}

void SAL_CALL SfxDocumentMetaData::setRights(const OUString& the_value)
{
    setMetaTextAndNotify(u"dc:rights"_ustr, the_value);
}

OUString SAL_CALL SfxDocumentMetaData::getSource()
{
    std::unique_lock g(m_aMutex);
    return getMetaText(g, "dc:source");
}

void SAL_CALL SfxDocumentMetaData::setSource(const OUString& the_value)
{
    setMetaTextAndNotify(u"dc:source"_ustr, the_value);
}

OUString SAL_CALL SfxDocumentMetaData::getType()
{
    std::unique_lock g(m_aMutex);
    return getMetaText(g, "dc:type");
}

void SAL_CALL SfxDocumentMetaData::setType(const OUString& the_value)
{
    setMetaTextAndNotify(u"dc:type"_ustr, the_value);
}

css::lang::Locale SAL_CALL
        SfxDocumentMetaData::getLanguage()
{
    std::unique_lock g(m_aMutex);
    css::lang::Locale loc( LanguageTag::convertToLocale( getMetaText(g, "dc:language"), false));
    return loc;
}

void SAL_CALL
SfxDocumentMetaData::setLanguage(const css::lang::Locale & the_value)
{
    OUString text( LanguageTag::convertToBcp47( the_value, false));
    setMetaTextAndNotify(u"dc:language"_ustr, text);
}

OUString SAL_CALL
SfxDocumentMetaData::getModifiedBy()
{
    std::unique_lock g(m_aMutex);
    return getMetaText(g, "dc:creator");
}

void SAL_CALL
SfxDocumentMetaData::setModifiedBy(const OUString & the_value)
{
    setMetaTextAndNotify(u"dc:creator"_ustr, the_value);
}

css::util::DateTime SAL_CALL
SfxDocumentMetaData::getModificationDate()
{
    std::unique_lock g(m_aMutex);
    return textToDateTimeDefault(getMetaText(g, "dc:date"));
}

void SAL_CALL
SfxDocumentMetaData::setModificationDate(const css::util::DateTime & the_value)
{
    setMetaTextAndNotify(u"dc:date"_ustr, dateTimeToText(the_value));
}

OUString SAL_CALL
SfxDocumentMetaData::getPrintedBy()
{
    std::unique_lock g(m_aMutex);
    return getMetaText(g, "meta:printed-by");
}

void SAL_CALL
SfxDocumentMetaData::setPrintedBy(const OUString & the_value)
{
    setMetaTextAndNotify(u"meta:printed-by"_ustr, the_value);
}

css::util::DateTime SAL_CALL
SfxDocumentMetaData::getPrintDate()
{
    std::unique_lock g(m_aMutex);
    return textToDateTimeDefault(getMetaText(g, "meta:print-date"));
}

void SAL_CALL
SfxDocumentMetaData::setPrintDate(const css::util::DateTime & the_value)
{
    setMetaTextAndNotify(u"meta:print-date"_ustr, dateTimeToText(the_value));
}

OUString SAL_CALL
SfxDocumentMetaData::getTemplateName()
{
    std::unique_lock g(m_aMutex);
    checkInit(g);
    return m_TemplateName;
}

void SAL_CALL
SfxDocumentMetaData::setTemplateName(const OUString & the_value)
{
    std::unique_lock g(m_aMutex);
    checkInit(g);
    if (m_TemplateName != the_value) {
        m_TemplateName = the_value;
        g.unlock();
        setModified(true);
    }
}

OUString SAL_CALL
SfxDocumentMetaData::getTemplateURL()
{
    std::unique_lock g(m_aMutex);
    checkInit(g);
    return m_TemplateURL;
}

void SAL_CALL
SfxDocumentMetaData::setTemplateURL(const OUString & the_value)
{
    std::unique_lock g(m_aMutex);
    checkInit(g);
    if (m_TemplateURL != the_value) {
        m_TemplateURL = the_value;
        g.unlock();
        setModified(true);
    }
}

css::util::DateTime SAL_CALL
SfxDocumentMetaData::getTemplateDate()
{
    std::unique_lock g(m_aMutex);
    checkInit(g);
    return m_TemplateDate;
}

void SAL_CALL
SfxDocumentMetaData::setTemplateDate(const css::util::DateTime & the_value)
{
    std::unique_lock g(m_aMutex);
    checkInit(g);
    if (m_TemplateDate != the_value) {
        m_TemplateDate = the_value;
        g.unlock();
        setModified(true);
    }
}

OUString SAL_CALL
SfxDocumentMetaData::getAutoloadURL()
{
    std::unique_lock g(m_aMutex);
    checkInit(g);
    return m_AutoloadURL;
}

void SAL_CALL
SfxDocumentMetaData::setAutoloadURL(const OUString & the_value)
{
    std::unique_lock g(m_aMutex);
    checkInit(g);
    if (m_AutoloadURL != the_value) {
        m_AutoloadURL = the_value;
        g.unlock();
        setModified(true);
    }
}

::sal_Int32 SAL_CALL
SfxDocumentMetaData::getAutoloadSecs()
{
    std::unique_lock g(m_aMutex);
    checkInit(g);
    return m_AutoloadSecs;
}

void SAL_CALL
SfxDocumentMetaData::setAutoloadSecs(::sal_Int32 the_value)
{
    if (the_value < 0)
        throw css::lang::IllegalArgumentException(
            u"SfxDocumentMetaData::setAutoloadSecs: argument is negative"_ustr,
            *this, 0);
    std::unique_lock g(m_aMutex);
    checkInit(g);
    if (m_AutoloadSecs != the_value) {
        m_AutoloadSecs = the_value;
        g.unlock();
        setModified(true);
    }
}

OUString SAL_CALL
SfxDocumentMetaData::getDefaultTarget()
{
    std::unique_lock g(m_aMutex);
    checkInit(g);
    return m_DefaultTarget;
}

void SAL_CALL
SfxDocumentMetaData::setDefaultTarget(const OUString & the_value)
{
    std::unique_lock g(m_aMutex);
    checkInit(g);
    if (m_DefaultTarget != the_value) {
        m_DefaultTarget = the_value;
        g.unlock();
        setModified(true);
    }
}

css::uno::Sequence< css::beans::NamedValue > SAL_CALL
SfxDocumentMetaData::getDocumentStatistics()
{
    std::unique_lock g(m_aMutex);
    checkInit(g);
    ::std::vector<css::beans::NamedValue> stats;
    for (size_t i = 0; s_stdStats[i] != nullptr; ++i) {
        OUString text = getMetaAttr(g, u"meta:document-statistic"_ustr, s_stdStatAttrs[i]);
        if (text.isEmpty()) continue;
        css::beans::NamedValue stat;
        stat.Name = OUString::createFromAscii(s_stdStats[i]);
        sal_Int32 val;
        css::uno::Any any;
        if (!::sax::Converter::convertNumber(val, text, 0) || (val < 0)) {
            val = 0;
            SAL_WARN("sfx.doc", "Invalid number: " << text);
        }
        any <<= val;
        stat.Value = any;
        stats.push_back(stat);
    }

    return ::comphelper::containerToSequence(stats);
}

void SAL_CALL
SfxDocumentMetaData::setDocumentStatistics(
        const css::uno::Sequence< css::beans::NamedValue > & the_value)
{
    {
        std::unique_lock g(m_aMutex);
        checkInit(g);
        std::vector<std::pair<OUString, OUString> > attributes;
        for (const auto& rValue : the_value) {
            const OUString name = rValue.Name;
            // inefficiently search for matching attribute
            for (size_t j = 0; s_stdStats[j] != nullptr; ++j) {
                if (name.equalsAscii(s_stdStats[j])) {
                    const css::uno::Any any = rValue.Value;
                    sal_Int32 val = 0;
                    if (any >>= val) {
                        attributes.emplace_back(s_stdStatAttrs[j],
                            OUString::number(val));
                    }
                    else {
                        SAL_WARN("sfx.doc", "Invalid statistic: " << name);
                    }
                    break;
                }
            }
        }
        updateElement(g, u"meta:document-statistic"_ustr, &attributes);
    }
    setModified(true);
}

::sal_Int16 SAL_CALL
SfxDocumentMetaData::getEditingCycles()
{
    std::unique_lock g(m_aMutex);
    OUString text = getMetaText(g, "meta:editing-cycles");
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
{
    if (the_value < 0)
        throw css::lang::IllegalArgumentException(
            u"SfxDocumentMetaData::setEditingCycles: argument is negative"_ustr,
            *this, 0);
    setMetaTextAndNotify(u"meta:editing-cycles"_ustr, OUString::number(the_value));
}

::sal_Int32 SAL_CALL
SfxDocumentMetaData::getEditingDuration()
{
    std::unique_lock g(m_aMutex);
    return textToDuration(getMetaText(g, "meta:editing-duration"));
}

void SAL_CALL
SfxDocumentMetaData::setEditingDuration(::sal_Int32 the_value)
{
    if (the_value < 0)
        throw css::lang::IllegalArgumentException(
            u"SfxDocumentMetaData::setEditingDuration: argument is negative"_ustr,
            *this, 0);
    setMetaTextAndNotify(u"meta:editing-duration"_ustr, durationToText(the_value));
}

void SAL_CALL
SfxDocumentMetaData::resetUserData(const OUString & the_value)
{
    std::unique_lock g(m_aMutex);

    bool bModified( false );
    bModified |= setMetaText(g, u"meta:initial-creator"_ustr, the_value);
    ::DateTime now( ::DateTime::SYSTEM );
    css::util::DateTime uDT(now.GetUNODateTime());
    bModified |= setMetaText(g, u"meta:creation-date"_ustr, dateTimeToText(uDT));
    bModified |= setMetaText(g, u"dc:creator"_ustr, OUString());
    bModified |= setMetaText(g, u"meta:printed-by"_ustr, OUString());
    bModified |= setMetaText(g, u"dc:date"_ustr, dateTimeToText(css::util::DateTime()));
    bModified |= setMetaText(g, u"meta:print-date"_ustr,
        dateTimeToText(css::util::DateTime()));
    bModified |= setMetaText(g, u"meta:editing-duration"_ustr, durationToText(0));
    bModified |= setMetaText(g, u"meta:editing-cycles"_ustr,
        u"1"_ustr);

    if (bModified) {
        g.unlock();
        setModified(true);
    }
}


css::uno::Reference< css::beans::XPropertyContainer > SAL_CALL
SfxDocumentMetaData::getUserDefinedProperties()
{
    std::unique_lock g(m_aMutex);
    checkInit(g);
    createUserDefined(g);
    return m_xUserDefined;
}


void SAL_CALL
SfxDocumentMetaData::loadFromStorage(
        const css::uno::Reference< css::embed::XStorage > & xStorage,
        const css::uno::Sequence< css::beans::PropertyValue > & Medium)
{
    if (!xStorage.is())
        throw css::lang::IllegalArgumentException(u"SfxDocumentMetaData::loadFromStorage: argument is null"_ustr, *this, 0);
    std::unique_lock g(m_aMutex);

    // open meta data file
    css::uno::Reference<css::io::XStream> xStream(
        xStorage->openStreamElement(
            s_meta,
            css::embed::ElementModes::READ) );
    if (!xStream.is()) throw css::uno::RuntimeException();
    css::uno::Reference<css::io::XInputStream> xInStream =
        xStream->getInputStream();
    if (!xInStream.is()) throw css::uno::RuntimeException();

    // create DOM parser service
    css::uno::Reference<css::lang::XMultiComponentFactory> xMsf (
        m_xContext->getServiceManager());
    css::xml::sax::InputSource input;
    input.aInputStream = xInStream;

    sal_uInt64 version = SotStorage::GetVersion( xStorage );
    // Oasis is also the default (0)
    bool bOasis = ( version > SOFFICE_FILEFORMAT_60 || version == 0 );
    const char *pServiceName = bOasis
        ? "com.sun.star.document.XMLOasisMetaImporter"
        : "com.sun.star.document.XMLMetaImporter";

    // set base URL
    css::uno::Reference<css::beans::XPropertySet> xPropArg =
        getURLProperties(g, Medium);
    try {
        xPropArg->getPropertyValue(u"BaseURI"_ustr)
            >>= input.sSystemId;
        input.sSystemId += OUString::Concat("/") + s_meta;
    } catch (const css::uno::Exception &) {
        input.sSystemId = s_meta;
    }
    css::uno::Sequence< css::uno::Any > args{ css::uno::Any(xPropArg) };

    // the underlying SvXMLImport implements XFastParser, XImporter, XFastDocumentHandler
    css::uno::Reference<XInterface> xFilter =
        xMsf->createInstanceWithArgumentsAndContext(
            OUString::createFromAscii(pServiceName), args, m_xContext);
    assert(xFilter);
    css::uno::Reference<css::xml::sax::XFastParser> xFastParser(xFilter, css::uno::UNO_QUERY);
    css::uno::Reference<css::document::XImporter> xImp(xFilter, css::uno::UNO_QUERY_THROW);
    xImp->setTargetDocument(css::uno::Reference<css::lang::XComponent>(this));
    g.unlock(); // NB: the implementation of XMLOasisMetaImporter calls initialize
    try {
        if (xFastParser)
            xFastParser->parseStream(input);
        else
        {
            css::uno::Reference<css::xml::sax::XDocumentHandler> xDocHandler(xFilter, css::uno::UNO_QUERY_THROW);
            css::uno::Reference<css::xml::sax::XParser> xParser = css::xml::sax::Parser::create(m_xContext);
            xParser->setDocumentHandler(xDocHandler);
            xParser->parseStream(input);
        }
    } catch (const css::xml::sax::SAXException &) {
        throw css::io::WrongFormatException(
                u"SfxDocumentMetaData::loadFromStorage:"
                " XML parsing exception"_ustr, *this);
    }
    g.lock();
    // NB: the implementation of XMLOasisMetaImporter calls initialize
    checkInit(g);
}

void SAL_CALL
SfxDocumentMetaData::storeToStorage(
        const css::uno::Reference< css::embed::XStorage > & xStorage,
        const css::uno::Sequence< css::beans::PropertyValue > & Medium)
{
    if (!xStorage.is())
        throw css::lang::IllegalArgumentException(
            u"SfxDocumentMetaData::storeToStorage: argument is null"_ustr, *this, 0);
    std::unique_lock g(m_aMutex);
    checkInit(g);

    // update user-defined meta data in DOM tree
//    updateUserDefinedAndAttributes(); // this will be done in serialize!

    // write into storage
    css::uno::Reference<css::io::XStream> xStream =
        xStorage->openStreamElement(s_meta,
            css::embed::ElementModes::WRITE
            | css::embed::ElementModes::TRUNCATE);
    if (!xStream.is()) throw css::uno::RuntimeException();
    css::uno::Reference< css::beans::XPropertySet > xStreamProps(xStream,
        css::uno::UNO_QUERY_THROW);
    xStreamProps->setPropertyValue(
        u"MediaType"_ustr,
        css::uno::Any(u"text/xml"_ustr));
    xStreamProps->setPropertyValue(
        u"Compressed"_ustr,
        css::uno::Any(false));
    xStreamProps->setPropertyValue(
        u"UseCommonStoragePasswordEncryption"_ustr,
        css::uno::Any(false));
    css::uno::Reference<css::io::XOutputStream> xOutStream =
        xStream->getOutputStream();
    if (!xOutStream.is()) throw css::uno::RuntimeException();
    css::uno::Reference<css::lang::XMultiComponentFactory> xMsf (
        m_xContext->getServiceManager());
    css::uno::Reference<css::xml::sax::XWriter> xSaxWriter(
        css::xml::sax::Writer::create(m_xContext));
    xSaxWriter->setOutputStream(xOutStream);

    const sal_uInt64 version = SotStorage::GetVersion( xStorage );
    // Oasis is also the default (0)
    const bool bOasis = ( version > SOFFICE_FILEFORMAT_60 || version == 0 );
    const char *pServiceName = bOasis
        ? "com.sun.star.document.XMLOasisMetaExporter"
        : "com.sun.star.document.XMLMetaExporter";

    // set base URL
    css::uno::Reference<css::beans::XPropertySet> xPropArg =
        getURLProperties(g, Medium);
    css::uno::Sequence< css::uno::Any > args{ css::uno::Any(xSaxWriter), css::uno::Any(xPropArg) };

    css::uno::Reference<css::document::XExporter> xExp(
        xMsf->createInstanceWithArgumentsAndContext(
            OUString::createFromAscii(pServiceName), args, m_xContext),
        css::uno::UNO_QUERY_THROW);
    xExp->setSourceDocument(css::uno::Reference<css::lang::XComponent>(this));
    css::uno::Reference<css::document::XFilter> xFilter(xExp,
        css::uno::UNO_QUERY_THROW);
    g.unlock(); // filter calls back into this
    if (!xFilter->filter(css::uno::Sequence< css::beans::PropertyValue >())) {
        throw css::io::IOException(
                u"SfxDocumentMetaData::storeToStorage: cannot filter"_ustr, *this);
    }
    css::uno::Reference<css::embed::XTransactedObject> xTransaction(
        xStorage, css::uno::UNO_QUERY);
    if (xTransaction.is()) {
        xTransaction->commit();
    }
}

void SAL_CALL
SfxDocumentMetaData::loadFromMedium(const OUString & URL,
        const css::uno::Sequence< css::beans::PropertyValue > & Medium)
{
    css::uno::Reference<css::io::XInputStream> xIn;
    utl::MediaDescriptor md(Medium);
    // if we have a URL parameter, it replaces the one in the media descriptor
    if (!URL.isEmpty()) {
        md[ utl::MediaDescriptor::PROP_URL ] <<= URL;
        md[ utl::MediaDescriptor::PROP_READONLY ] <<= true;
    }
    if (md.addInputStream()) {
        md[ utl::MediaDescriptor::PROP_INPUTSTREAM ] >>= xIn;
    }
    css::uno::Reference<css::embed::XStorage> xStorage;
    try {
        if (xIn.is()) {
            xStorage = ::comphelper::OStorageHelper::GetStorageFromInputStream(
                            xIn, m_xContext);
        } else { // fallback to url parameter
            xStorage = ::comphelper::OStorageHelper::GetStorageFromURL(
                            URL, css::embed::ElementModes::READ, m_xContext);
        }
    } catch (const css::uno::RuntimeException &) {
        throw;
    } catch (const css::io::IOException &) {
        throw;
    } catch (const css::uno::Exception &) {
        css::uno::Any anyEx = cppu::getCaughtException();
        throw css::lang::WrappedTargetException(
                u"SfxDocumentMetaData::loadFromMedium: exception"_ustr,
                css::uno::Reference<css::uno::XInterface>(*this),
                anyEx);
    }
    if (!xStorage.is()) {
        throw css::uno::RuntimeException(
                u"SfxDocumentMetaData::loadFromMedium: cannot get Storage"_ustr,
                *this);
    }
    loadFromStorage(xStorage, md.getAsConstPropertyValueList());
}

void SAL_CALL
SfxDocumentMetaData::storeToMedium(const OUString & URL,
        const css::uno::Sequence< css::beans::PropertyValue > & Medium)
{
    utl::MediaDescriptor md(Medium);
    if (!URL.isEmpty()) {
        md[ utl::MediaDescriptor::PROP_URL ] <<= URL;
    }
    SfxMedium aMedium(md.getAsConstPropertyValueList());
    css::uno::Reference<css::embed::XStorage> xStorage
        = aMedium.GetOutputStorage();


    if (!xStorage.is()) {
        throw css::uno::RuntimeException(
                u"SfxDocumentMetaData::storeToMedium: cannot get Storage"_ustr,
                *this);
    }
    // set MIME type of the storage
    utl::MediaDescriptor::const_iterator iter
        = md.find(utl::MediaDescriptor::PROP_MEDIATYPE);
    if (iter != md.end()) {
        css::uno::Reference< css::beans::XPropertySet > xProps(xStorage,
            css::uno::UNO_QUERY_THROW);
        xProps->setPropertyValue(
            utl::MediaDescriptor::PROP_MEDIATYPE,
            iter->second);
    }
    storeToStorage(xStorage, md.getAsConstPropertyValueList());


    const bool bOk = aMedium.Commit();
    aMedium.Close();
    if ( !bOk ) {
        ErrCodeMsg nError = aMedium.GetErrorIgnoreWarning();
        if ( nError == ERRCODE_NONE ) {
            nError = ERRCODE_IO_GENERAL;
        }

        throw css::task::ErrorCodeIOException(
            "SfxDocumentMetaData::storeToMedium <" + URL + "> Commit failed: " + nError.toString(),
            css::uno::Reference< css::uno::XInterface >(), sal_uInt32(nError.GetCode()));

    }
}

// css::lang::XInitialization:
void SAL_CALL SfxDocumentMetaData::initialize( const css::uno::Sequence< css::uno::Any > & aArguments)
{
    // possible arguments:
    // - no argument: default initialization (empty DOM)
    // - 1 argument, XDocument: initialize with given DOM and empty base URL
    // NB: links in document must be absolute

    std::unique_lock g(m_aMutex);
    css::uno::Reference<css::xml::dom::XDocument> xDoc;

    for (sal_Int32 i = 0; i < aArguments.getLength(); ++i) {
        const css::uno::Any any = aArguments[i];
        if (!(any >>= xDoc)) {
            throw css::lang::IllegalArgumentException(
                u"SfxDocumentMetaData::initialize: argument must be XDocument"_ustr,
                *this, static_cast<sal_Int16>(i));
        }
        if (!xDoc.is()) {
            throw css::lang::IllegalArgumentException(
                u"SfxDocumentMetaData::initialize: argument is null"_ustr,
                *this, static_cast<sal_Int16>(i));
        }
    }

    if (!xDoc.is()) {
        // For a new document, we create a new DOM tree here.
        xDoc = createDOM();
    }

    init(g, xDoc);
}

// css::util::XCloneable:
css::uno::Reference<css::util::XCloneable> SAL_CALL
SfxDocumentMetaData::createClone()
{
    std::unique_lock g(m_aMutex);
    checkInit(g);

    rtl::Reference<SfxDocumentMetaData> pNew = createMe(m_xContext);

    // NB: do not copy the modification listeners, only DOM
    css::uno::Reference<css::xml::dom::XDocument> xDoc = createDOM();
    try {
        updateUserDefinedAndAttributes(g);
        // deep copy of root node
        css::uno::Reference<css::xml::dom::XNode> xRoot(
            m_xDoc->getDocumentElement(), css::uno::UNO_QUERY_THROW);
        css::uno::Reference<css::xml::dom::XNode> xRootNew(
            xDoc->importNode(xRoot, true));
        xDoc->appendChild(xRootNew);
        g.unlock();
        std::unique_lock g2(pNew->m_aMutex);
        pNew->init(g2, xDoc);
    } catch (const css::uno::RuntimeException &) {
        throw;
    } catch (const css::uno::Exception &) {
        css::uno::Any anyEx = cppu::getCaughtException();
        throw css::lang::WrappedTargetRuntimeException(
                u"SfxDocumentMetaData::createClone: exception"_ustr,
                css::uno::Reference<css::uno::XInterface>(*this), anyEx);
    }
    return css::uno::Reference<css::util::XCloneable> (pNew);
}

// css::util::XModifiable:
sal_Bool SAL_CALL SfxDocumentMetaData::isModified(  )
{
    std::unique_lock g(m_aMutex);
    checkInit(g);
    css::uno::Reference<css::util::XModifiable> xMB(m_xUserDefined,
        css::uno::UNO_QUERY);
    return m_isModified || (xMB.is() && xMB->isModified());
}

void SAL_CALL SfxDocumentMetaData::setModified( sal_Bool bModified )
{
    css::uno::Reference<css::util::XModifiable> xMB;
    { // do not lock mutex while notifying (#i93514#) to prevent deadlock
        std::unique_lock g(m_aMutex);
        checkInit(g);
        m_isModified = bModified;
        if ( !bModified && m_xUserDefined.is() )
        {
            xMB.set(m_xUserDefined, css::uno::UNO_QUERY);
            assert(xMB.is() &&
                "SfxDocumentMetaData::setModified: PropertyBag not Modifiable?");
        }
    }
    if (bModified) {
        try {
            css::uno::Reference<css::uno::XInterface> xThis(*this);
            css::lang::EventObject event(xThis);
            std::unique_lock g(m_aMutex);
            m_NotifyListeners.notifyEach(g, &css::util::XModifyListener::modified,
                event);
        } catch (const css::uno::RuntimeException &) {
            throw;
        } catch (const css::uno::Exception &) {
            // ignore
            TOOLS_WARN_EXCEPTION("sfx.doc", "setModified");
        }
    } else {
        if (xMB.is()) {
            xMB->setModified(false);
        }
    }
}

// css::util::XModifyBroadcaster:
void SAL_CALL SfxDocumentMetaData::addModifyListener(
        const css::uno::Reference< css::util::XModifyListener > & xListener)
{
    std::unique_lock g(m_aMutex);
    checkInit(g);
    m_NotifyListeners.addInterface(g, xListener);
    css::uno::Reference<css::util::XModifyBroadcaster> xMB(m_xUserDefined,
        css::uno::UNO_QUERY);
    if (xMB.is()) {
        xMB->addModifyListener(xListener);
    }
}

void SAL_CALL SfxDocumentMetaData::removeModifyListener(
        const css::uno::Reference< css::util::XModifyListener > & xListener)
{
    std::unique_lock g(m_aMutex);
    checkInit(g);
    m_NotifyListeners.removeInterface(g, xListener);
    css::uno::Reference<css::util::XModifyBroadcaster> xMB(m_xUserDefined,
        css::uno::UNO_QUERY);
    if (xMB.is()) {
        xMB->removeModifyListener(xListener);
    }
}

// css::xml::sax::XSAXSerializable
void SAL_CALL SfxDocumentMetaData::serialize(
    const css::uno::Reference<css::xml::sax::XDocumentHandler>& i_xHandler,
    const css::uno::Sequence< css::beans::StringPair >& i_rNamespaces)
{
    std::unique_lock g(m_aMutex);
    checkInit(g);
    updateUserDefinedAndAttributes(g);
    css::uno::Reference<css::xml::sax::XSAXSerializable> xSAXable(m_xDoc,
        css::uno::UNO_QUERY_THROW);
    xSAXable->serialize(i_xHandler, i_rNamespaces);
}

void SfxDocumentMetaData::createUserDefined(std::unique_lock<std::mutex>& g)
{
    // user-defined meta data: create PropertyBag which only accepts property
    // values of allowed types
    if ( m_xUserDefined.is() )
        return;

    css::uno::Sequence<css::uno::Type> types{
        ::cppu::UnoType<bool>::get(),
        ::cppu::UnoType< OUString>::get(),
        ::cppu::UnoType<css::util::DateTime>::get(),
        ::cppu::UnoType<css::util::Date>::get(),
        ::cppu::UnoType<css::util::DateTimeWithTimezone>::get(),
        ::cppu::UnoType<css::util::DateWithTimezone>::get(),
        ::cppu::UnoType<css::util::Duration>::get(),
        ::cppu::UnoType<float>::get(),
        ::cppu::UnoType<double>::get(),
        ::cppu::UnoType<sal_Int16>::get(),
        ::cppu::UnoType<sal_Int32>::get(),
        ::cppu::UnoType<sal_Int64>::get(),
        // Time is supported for backward compatibility with OOo 3.x, x<=2
        ::cppu::UnoType<css::util::Time>::get()
    };
    // #i94175#:  ODF allows empty user-defined property names!
    m_xUserDefined.set(
        css::beans::PropertyBag::createWithTypes( m_xContext, types, true/*AllowEmptyPropertyName*/, false/*AutomaticAddition*/ ),
        css::uno::UNO_QUERY_THROW);

    const css::uno::Reference<css::util::XModifyBroadcaster> xMB(
        m_xUserDefined, css::uno::UNO_QUERY);
    if (xMB.is())
    {
        m_NotifyListeners.forEach(g,
            [xMB] (const css::uno::Reference<css::util::XModifyListener>& l)
            {
                xMB->addModifyListener(l);
            });
    }
}

} // closing anonymous implementation namespace

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
CompatWriterDocPropsImpl_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new CompatWriterDocPropsImpl(context));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
SfxDocumentMetaData_get_implementation(
    css::uno::XComponentContext *context,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new SfxDocumentMetaData(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

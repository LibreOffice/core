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

#ifndef INCLUDED_XMLOFF_SOURCE_TEXT_XMLSECTIONEXPORT_HXX
#define INCLUDED_XMLOFF_SOURCE_TEXT_XMLSECTIONEXPORT_HXX

#include <rtl/ustring.hxx>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <xmloff/xmltoken.hxx>

class SvXMLExport;
class XMLTextParagraphExport;
namespace com { namespace sun { namespace star {
    namespace text
    {
        class XTextSection;
        class XDocumentIndex;
        class XTextContent;
    }
    namespace beans
    {
        class XPropertySet;
    }
    namespace container
    {
        class XIndexReplace;
    }
} } }


enum SectionTypeEnum
{
    // indices: keep index valued together and do not change order,
    // because they are used to index into array
    TEXT_SECTION_TYPE_TOC,
    TEXT_SECTION_TYPE_TABLE,
    TEXT_SECTION_TYPE_ILLUSTRATION,
    TEXT_SECTION_TYPE_OBJECT,
    TEXT_SECTION_TYPE_USER,
    TEXT_SECTION_TYPE_ALPHABETICAL,
    TEXT_SECTION_TYPE_BIBLIOGRAPHY,

    TEXT_SECTION_TYPE_UNKNOWN
};


/**
 * This class handles the export of sections and indices (which are,
 * internally, just sections). It is intended to be used only from
 * within the XMLTextParagraphExport class.
 */
class XMLSectionExport
{
    SvXMLExport& rExport;
    XMLTextParagraphExport& rParaExport;

    bool bHeadingDummiesExported;

public:
    XMLSectionExport(SvXMLExport& rExp,
                     XMLTextParagraphExport& rParaExp);

    /**
     * export section or index start and source elements. This
     * method handles the section styles, and delegates to the
     * appropriate section or index method.
     */
    void ExportSectionStart(
        const css::uno::Reference < css::text::XTextSection > & rSection,
        bool bAutoStyles);

    /**
     * export section or index end elements
     */
    void ExportSectionEnd(
        const css::uno::Reference < css::text::XTextSection > & rSection,
        bool bAutoStyles);

    /**
     * Should the content of this section be exported?
     * (E.g. linked sections in global documents are not always exported)
     */
    bool IsMuteSection(
        const css::uno::Reference < css::text::XTextSection > & rSection) const;

    /**
     * XTextContent-version of IsMuteSection(Reference<XTextSection>&)
     * returns *true* for all non-section elements
     */
    bool IsMuteSection(
        const css::uno::Reference < css::text::XTextContent > & rSection,
        /// return value if this content doesn't support the section property
        bool bDefault) const;

    /**
     * Determine whether rContent is contained in rEnclosingSection. If the
     * current section of rContent can not be determined, return bDefault.
     */
    static bool IsInSection(
        const css::uno::Reference < css::text::XTextSection > & rEnclosingSection,
        const css::uno::Reference < css::text::XTextContent > & rContent,
        /// return value if this content doesn't support the section property
        bool bDefault);

    /**
     * Export the configuration element for bibliography indices.
     *
     * (This is part of XMLSectionExport because all section- and
     *  index-related items are handled here.)
     */
    static void ExportBibliographyConfiguration(SvXMLExport& rExport);

    /** export a heading for every level. This is used by master documents
     *  to not lose the heading information if master documents are exported
     *  without section contents
     */
    void ExportMasterDocHeadingDummies();


protected:

    inline SvXMLExport& GetExport() { return rExport; }
    inline XMLTextParagraphExport& GetParaExport() { return rParaExport; }

    // export methods for section and index start:

    /// export an index start element.
    void ExportIndexStart(
        const css::uno::Reference < css::text::XDocumentIndex > & rSection);

    /// export an index header start element.
    void ExportIndexHeaderStart(
        const css::uno::Reference < css::text::XTextSection > & rSection);

    /// export a proper section (and source elements)
    void ExportRegularSectionStart(
        const css::uno::Reference < css::text::XTextSection > & rSection);

    /// export a table of content (and source element)
    void ExportTableOfContentStart(
        const css::uno::Reference < css::beans::XPropertySet > & rSection);

    /// export a table index (and source element)
    void ExportTableIndexStart(
        const css::uno::Reference < css::beans::XPropertySet > & rSection);

    /// export an object index (and source element)
    void ExportObjectIndexStart(
        const css::uno::Reference < css::beans::XPropertySet > & rSection);

    /// export an illustration index (and source element)
    void ExportIllustrationIndexStart(
        const css::uno::Reference < css::beans::XPropertySet > & rSection);

    /// export an alphabetical/keyword index (and source element)
    void ExportAlphabeticalIndexStart(
        const css::uno::Reference < css::beans::XPropertySet > & rSection);

    /// export a user index (and source element)
    void ExportUserIndexStart(
        const css::uno::Reference < css::beans::XPropertySet > & rSection);

    /// export the bibliography (and source element)
    void ExportBibliographyStart(
        const css::uno::Reference < css::beans::XPropertySet > & rIndex);

    // helper methods:

    /**
     * If this section is an index, the index is written in the
     * rIndex parameter. The return value is sal_True for all "special"
     * sections.
     *
     * Thus we have:
     * return sal_False: regular section
     * return sal_True, xIndex is empty: index header section
     * return sal_True, xIndex is set: index section */
    static bool GetIndex(
        const css::uno::Reference < css::text::XTextSection > & rSection,
        css::uno::Reference < css::text::XDocumentIndex > & rIndex);

    /// map service name to section type
    static enum SectionTypeEnum MapSectionType(const OUString& rSectionName);

    /**
     * Export the index element start (for all index types).
     *
     * All additional attributes (usually none) for the index element
     * should have been set at GetExport() before calling this method.
     */
    void ExportBaseIndexStart(
        ::xmloff::token::XMLTokenEnum eElement,
        const css::uno::Reference < css::beans::XPropertySet > & rSection);

    /**
     * Export the index source element (common for all index types).
     *
     * All additional attributes for the source element should have
     * been set at the GetExport() before calling this method.
     */
    void ExportBaseIndexSource(
        SectionTypeEnum eType,                /// index type
        const css::uno::Reference <
            css::beans::XPropertySet > & rSection);

    /**
     * Export the index body (common for all index types).
     */
    void ExportBaseIndexBody(
        SectionTypeEnum eType,                /// index type
        const css::uno::Reference < css::beans::XPropertySet > & rSection);


    /**
     * Helper method to export common attributes for table and
     * illustration indices
     */
     void ExportTableAndIllustrationIndexSourceAttributes(
         const css::uno::Reference < css::beans::XPropertySet > & rSection);

    /// export one template for the specific index type
    bool ExportIndexTemplate(
        SectionTypeEnum eType,                /// index type
        sal_Int32 nLevel,                     /// outline level (if applicable)
        const css::uno::Reference< css::beans::XPropertySet> & rPropSet,
        css::uno::Sequence< css::uno::Sequence< css::beans::PropertyValue> > & rValues);

    /// export a single template element (e.g. span or tab-stop)
    void ExportIndexTemplateElement(
        SectionTypeEnum eType, //i90246, needed for ODF 1.0, 1.0 and 1.2 management
        css::uno::Sequence<
            css::beans::PropertyValue> & rValues);

    /// export level paragraph styles
    void ExportLevelParagraphStyles(
        css::uno::Reference<
            css::container::XIndexReplace> & xStyles);


    /// helper to export boolean properties
    void ExportBoolean(
        const css::uno::Reference<css::beans::XPropertySet> & rPropSet,
        const OUString& sPropertyName,
        enum ::xmloff::token::XMLTokenEnum eAttributeName,
        bool bDefault,
        bool bInvert = false);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

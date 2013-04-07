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

#ifndef _XMLOFF_XMLSECTIONEXPORT_HXX_
#define _XMLOFF_XMLSECTIONEXPORT_HXX_

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
    TEXT_SECTION_TYPE_SECTION,

    // indices: keep index valued together and do not change order,
    // because they are used to index into array
    TEXT_SECTION_TYPE_TOC,
    TEXT_SECTION_TYPE_TABLE,
    TEXT_SECTION_TYPE_ILLUSTRATION,
    TEXT_SECTION_TYPE_OBJECT,
    TEXT_SECTION_TYPE_USER,
    TEXT_SECTION_TYPE_ALPHABETICAL,
    TEXT_SECTION_TYPE_BIBLIOGRAPHY,

    // index header sections:
    TEXT_SECTION_TYPE_HEADER,

    TEXT_SECTION_TYPE_UNKNOWN
};


/**
 * This class handles the export of sections and indices (which are,
 * internally, just sections). It is intended to be used only from
 * within the XMLTextParagraphExport class.
 */
class XMLSectionExport
{
    const OUString sCondition;
    const OUString sCreateFromChapter;
    const OUString sCreateFromEmbeddedObjects;
    const OUString sCreateFromGraphicObjects;
    const OUString sCreateFromLabels;
    const OUString sCreateFromMarks;
    const OUString sCreateFromOtherEmbeddedObjects;
    const OUString sCreateFromOutline;
    const OUString sCreateFromStarCalc;
    const OUString sCreateFromStarChart;
    const OUString sCreateFromStarDraw;
    const OUString sCreateFromStarImage;
    const OUString sCreateFromStarMath;
    const OUString sCreateFromTables;
    const OUString sCreateFromTextFrames;
    const OUString sDdeCommandElement;
    const OUString sDdeCommandFile;
    const OUString sDdeCommandType;
    const OUString sFileLink;
    const OUString sIsCaseSensitive;
    const OUString sIsProtected;
    const OUString sIsVisible;
    const OUString sLabelCategory;
    const OUString sLabelDisplayType;
    const OUString sLevel;
    const OUString sLevelFormat;
    const OUString sLevelParagraphStyles;
    const OUString sLinkRegion;
    const OUString sMainEntryCharacterStyleName;
    const OUString sParaStyleHeading;
    const OUString sParaStyleLevel;
    const OUString sTitle;
    const OUString sName;
    const OUString sUseAlphabeticalSeparators;
    const OUString sUseCombinedEntries;
    const OUString sUseDash;
    const OUString sUseKeyAsEntry;
    const OUString sUseLevelFromSource;
    const OUString sUsePP;
    const OUString sUseUpperCase;
    const OUString sIsCommaSeparated;
    const OUString sIsAutomaticUpdate;
    const OUString sIsRelativeTabstops;
    const OUString sCreateFromLevelParagraphStyles;
    const OUString sDocumentIndex;
    const OUString sContentSection;
    const OUString sHeaderSection;

    const OUString sTextSection;
    const OUString sIsGlobalDocumentSection;
    const OUString sProtectionKey;
    const OUString sSortAlgorithm;
    const OUString sLocale;
    const OUString sUserIndexName;

    const OUString sIsCurrentlyVisible;
    const OUString sHeadingStyleName;

    const OUString sEmpty;

    SvXMLExport& rExport;
    XMLTextParagraphExport& rParaExport;

    sal_Bool bHeadingDummiesExported;

public:
    XMLSectionExport(SvXMLExport& rExp,
                     XMLTextParagraphExport& rParaExp);

    /**
     * export section or index start and source elements. This
     * method handles the section styles, and delegates to the
     * appropriate section or index method.
     */
    void ExportSectionStart(
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::text::XTextSection > & rSection,
        sal_Bool bAutoStyles);

    /**
     * export section or index end elements
     */
    void ExportSectionEnd(
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::text::XTextSection > & rSection,
        sal_Bool bAutoStyles);

    /**
     * Should the content of this section be exported?
     * (E.g. linked sections in global documents are not always exported)
     */
    sal_Bool IsMuteSection(
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::text::XTextSection > & rSection) const;

    /**
     * XTextContent-version of IsMuteSection(Reference<XTextSection>&)
     * returns *true* for all non-section elements
     */
    sal_Bool IsMuteSection(
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::text::XTextContent > & rSection,
        /// return value if this content doesn't support the section property
        sal_Bool bDefault) const;

    /**
     * Determine whether rContent is contained in rEnclosingSection. If the
     * current section of rContent can not be determined, return bDefault.
     */
    sal_Bool IsInSection(
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::text::XTextSection > & rEnclosingSection,
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::text::XTextContent > & rContent,
        /// return value if this content doesn't support the section property
        sal_Bool bDefault);

    /**
     * Export the configuration element for bibliography indices.
     *
     * (This is part of XMLSectionExport because all section- and
     *  index-related items are handled here.)
     */
    static void ExportBibliographyConfiguration(SvXMLExport& rExport);

    /** export a heading for every level. This is used by master documents
     *  to not loose the heading information if master documents are exported
     *  without section contents
     */
    void ExportMasterDocHeadingDummies();



protected:

    inline SvXMLExport& GetExport() { return rExport; }
    inline XMLTextParagraphExport& GetParaExport() { return rParaExport; }

    // export methods for section and index start:

    /// export an index start element.
    void ExportIndexStart(
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::text::XDocumentIndex > & rSection);

    /// export an index header start element.
    void ExportIndexHeaderStart(
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::text::XTextSection > & rSection);

    /// export a proper section (and source elements)
    void ExportRegularSectionStart(
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::text::XTextSection > & rSection);

    /// export a table of content (and source element)
    void ExportTableOfContentStart(
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::beans::XPropertySet > & rSection);

    /// export a table index (and source element)
    void ExportTableIndexStart(
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::beans::XPropertySet > & rSection);

    /// export an object index (and source element)
    void ExportObjectIndexStart(
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::beans::XPropertySet > & rSection);

    /// export an illustration index (and source element)
    void ExportIllustrationIndexStart(
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::beans::XPropertySet > & rSection);

    /// export an alphabetical/keyword index (and source element)
    void ExportAlphabeticalIndexStart(
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::beans::XPropertySet > & rSection);

    /// export a user index (and source element)
    void ExportUserIndexStart(
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::beans::XPropertySet > & rSection);

    /// export the bibliography (and source element)
    void ExportBibliographyStart(
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::beans::XPropertySet > & rIndex);

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
    sal_Bool GetIndex(
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::text::XTextSection > & rSection,
        ::com::sun::star::uno::Reference <
            ::com::sun::star::text::XDocumentIndex > & rIndex) const;

    /// map service name to section type
    enum SectionTypeEnum MapSectionType(const OUString& rSectionName);

    /**
     * Export the index element start (for all index types).
     *
     * All additional attributes (usually none) for the index element
     * should have been set at GetExport() before calling this method.
     */
    void ExportBaseIndexStart(
        ::xmloff::token::XMLTokenEnum eElement,
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::beans::XPropertySet > & rSection);

    /**
     * Export the index source element (common for all index types).
     *
     * All additional attributes for the source element should have
     * been set at the GetExport() before calling this method.
     */
    void ExportBaseIndexSource(
        SectionTypeEnum eType,                /// index type
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::beans::XPropertySet > & rSection);

    /**
     * Export the index body (common for all index types).
     */
    void ExportBaseIndexBody(
        SectionTypeEnum eType,                /// index type
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::beans::XPropertySet > & rSection);


    /**
     * Helper method to export common attributes for table and
     * illustration indices
     */
     void ExportTableAndIllustrationIndexSourceAttributes(
         const ::com::sun::star::uno::Reference <
            ::com::sun::star::beans::XPropertySet > & rSection);

    /// export one template for the specific index type
    sal_Bool ExportIndexTemplate(
        SectionTypeEnum eType,                /// index type
        sal_Int32 nLevel,                     /// outline level (if applicable)
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet> & rPropSet,
        ::com::sun::star::uno::Sequence<
            ::com::sun::star::uno::Sequence<
                ::com::sun::star::beans::PropertyValue> > & rValues);

    /// export a single template element (e.g. span or tab-stop)
    void ExportIndexTemplateElement(
        SectionTypeEnum eType, //i90246, needed for ODF 1.0, 1.0 and 1.2 management
        ::com::sun::star::uno::Sequence<
            ::com::sun::star::beans::PropertyValue> & rValues);

    /// export level paragraph styles
    void ExportLevelParagraphStyles(
        ::com::sun::star::uno::Reference<
            ::com::sun::star::container::XIndexReplace> & xStyles);


    /// helper to export boolean properties
    void ExportBoolean(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet> & rPropSet,
        const OUString& sPropertyName,
        enum ::xmloff::token::XMLTokenEnum eAttributeName,
        sal_Bool bDefault,
        sal_Bool bInvert = sal_False);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

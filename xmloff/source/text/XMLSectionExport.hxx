/*************************************************************************
 *
 *  $RCSfile: XMLSectionExport.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: dvo $ $Date: 2000-11-20 19:56:50 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _XMLOFF_XMLSECTIONEXPORT_HXX_
#define _XMLOFF_XMLSECTIONEXPORT_HXX_

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_H_
#include <com/sun/star/uno/Sequence.h>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif

class SvXMLExport;
class XMLTextParagraphExport;
namespace com { namespace sun { namespace star {
    namespace text
    {
        class XTextSection;
        class XDocumentIndex;
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
    // todo

    TEXT_SECTION_TYPE_UNKNOWN
};


/**
 * This class handles the export of sections and indices (which are,
 * internally, just sections). It is intended to be used only from
 * within the XMLTextParagraphExport class.
 */
class XMLSectionExport
{
    const ::rtl::OUString sCondition;
    const ::rtl::OUString sCreateFromChapter;
    const ::rtl::OUString sCreateFromEmbeddedObjects;
    const ::rtl::OUString sCreateFromGraphicObjects;
    const ::rtl::OUString sCreateFromLabels;
    const ::rtl::OUString sCreateFromMarks;
    const ::rtl::OUString sCreateFromOtherEmbeddedObjects;
    const ::rtl::OUString sCreateFromOutline;
    const ::rtl::OUString sCreateFromStarCalc;
    const ::rtl::OUString sCreateFromStarChart;
    const ::rtl::OUString sCreateFromStarDraw;
    const ::rtl::OUString sCreateFromStarImage;
    const ::rtl::OUString sCreateFromStarMath;
    const ::rtl::OUString sCreateFromTables;
    const ::rtl::OUString sCreateFromTextFrames;
    const ::rtl::OUString sDdeCommandElement;
    const ::rtl::OUString sDdeCommandFile;
    const ::rtl::OUString sDdeCommandType;
    const ::rtl::OUString sFileLink;
    const ::rtl::OUString sIsCaseSensitive;
    const ::rtl::OUString sIsProtected;
    const ::rtl::OUString sIsVisible;
    const ::rtl::OUString sLabelCategory;
    const ::rtl::OUString sLabelDisplayType;
    const ::rtl::OUString sLevel;
    const ::rtl::OUString sLevelFormat;
    const ::rtl::OUString sLevelParagraphStyles;
    const ::rtl::OUString sLinkRegion;
    const ::rtl::OUString sMainEntryCharacterStyleName;
    const ::rtl::OUString sParaStyleHeading;
    const ::rtl::OUString sParaStyleLevel;
    const ::rtl::OUString sSection;
    const ::rtl::OUString sTitle;
    const ::rtl::OUString sUseAlphabeticalSeparators;
    const ::rtl::OUString sUseCombinedEntries;
    const ::rtl::OUString sUseDash;
    const ::rtl::OUString sUseKeyAsEntry;
    const ::rtl::OUString sUseLevelFromSource;
    const ::rtl::OUString sUsePP;
    const ::rtl::OUString sUseUpperCase;
    const ::rtl::OUString sIsCommaSeparated;
    const ::rtl::OUString sIsAutomaticUpdate;
    const ::rtl::OUString sIsRelativeTabstops;

    const ::rtl::OUString sTableOfContent;
    const ::rtl::OUString sIllustrationIndex;
    const ::rtl::OUString sTableIndex;
    const ::rtl::OUString sAlphabeticalIndex;
    const ::rtl::OUString sObjectIndex;
    const ::rtl::OUString sBibliography;
    const ::rtl::OUString sUserIndex;

    const ::rtl::OUString sEmpty;

    SvXMLExport& rExport;
    XMLTextParagraphExport& rParaExport;

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
     * export an index start element. This method should eventually be replaced
     * by calls to ExportSectionStart. Also handle section styles.
     *
     * @deprecated Once the API allows to recognize an index section
     * as index, this method should be removed.
     */
    void ExportIndexStart(
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::text::XDocumentIndex > & rSection,
        sal_Bool bAutoStyles);

    /** @deprecated see ExportIndexStart */
    void ExportIndexEnd(
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::text::XDocumentIndex > & rSection,
        sal_Bool bAutoStyles);

protected:

    inline SvXMLExport& GetExport() { return rExport; }
    inline XMLTextParagraphExport& GetParaExport() { return rParaExport; }

    // export methods for section and index start:

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

    /// map service name to section type
    enum SectionTypeEnum MapSectionType(const ::rtl::OUString& rSectionName);

    /**
     * Export the index element start (for all index types).
     *
     * All additional attributes (usually none) for the index element
     * should have been set at GetExport() before calling this method.
     */
    void ExportBaseIndexStart(
        const ::rtl::OUString sElementName,
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
    void ExportIndexTemplate(
        SectionTypeEnum eType,                /// index type
        sal_Int32 nLevel,                     /// outline level (if applicable)
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet> & rPropSet,
        ::com::sun::star::uno::Sequence<
            ::com::sun::star::uno::Sequence<
                ::com::sun::star::beans::PropertyValue> > & rValues);

    /// export a single template element (e.g. span or tab-stop)
    void ExportIndexTemplateElement(
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
        const ::rtl::OUString& sPropertyName,
        const sal_Char* pAttributeName,
        sal_Bool bDefault,
        sal_Bool bInvert = sal_False);
};

#endif

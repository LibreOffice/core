/*************************************************************************
 *
 *  $RCSfile: XMLSectionExport.cxx,v $
 *
 *  $Revision: 1.4 $
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
#include "XMLSectionExport.hxx"
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#ifndef __SGI_STL_VECTOR
#include <stl/vector>
#endif


#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XINDEXREPLACE_HPP_
#include <com/sun/star/container/XIndexReplace.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUES_HPP_
#include <com/sun/star/beans/PropertyValues.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_PROPERTYSTATE_HPP_
#include <com/sun/star/beans/PropertyState.hpp>
#endif

#ifndef _COM_SUN_STAR_TEXT_XTEXT_HPP_
#include <com/sun/star/text/XText.hpp>
#endif

#ifndef _COM_SUN_STAR_TEXT_XTEXTSECTION_HPP_
#include <com/sun/star/text/XTextSection.hpp>
#endif

#ifndef _COM_SUN_STAR_TEXT_SECTIONFILELINK_HPP_
#include <com/sun/star/text/SectionFileLink.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XNAMED_HPP_
#include <com/sun/star/container/XNamed.hpp>
#endif

#ifndef _COM_SUN_STAR_TEXT_XDOCUMENTINDEX_HPP_
#include <com/sun/star/text/XDocumentIndex.hpp>
#endif

#ifndef _COM_SUN_STAR_TEXT_BIBLIOGRAPHYDATAFIELD_HPP_
#include <com/sun/star/text/BibliographyDataField.hpp>
#endif

#ifndef _XMLOFF_XMLKYWD_HXX
#include "xmlkywd.hxx"
#endif

#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif

#ifndef _XMLOFF_FAMILIES_HXX_
#include "families.hxx"
#endif

#ifndef _XMLOFF_XMLUCONV_HXX
#include "xmluconv.hxx"
#endif

#ifndef _XMLOFF_NMSPMAP_HXX
#include "nmspmap.hxx"
#endif

#ifndef _XMLOFF_XMLEXP_HXX
#include "xmlexp.hxx"
#endif

#ifndef _XMLOFF_XMLTKMAP_HXX
#include "xmltkmap.hxx"
#endif

#ifndef _XMLOFF_TXTFLDE_HXX
#include "txtflde.hxx"
#endif

#ifndef _XMLOFF_XMLTEXTNUMRULEINFO_HXX
#include "XMLTextNumRuleInfo.hxx"
#endif

using namespace ::com::sun::star;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::uno;
using namespace ::std;

using ::rtl::OUString;
using ::rtl::OUStringBuffer;
using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::beans::PropertyValue;
using ::com::sun::star::beans::PropertyValues;
using ::com::sun::star::beans::PropertyState;
using ::com::sun::star::container::XIndexReplace;
using ::com::sun::star::container::XNamed;
using ::com::sun::star::lang::XServiceInfo;




XMLSectionExport::XMLSectionExport(
    SvXMLExport& rExp,
    XMLTextParagraphExport& rParaExp) :
        rExport(rExp),
        rParaExport(rParaExp),
        sCondition(RTL_CONSTASCII_USTRINGPARAM("Condition")),
        sCreateFromChapter(RTL_CONSTASCII_USTRINGPARAM("CreateFromChapter")),
        sCreateFromLabels(RTL_CONSTASCII_USTRINGPARAM("CreateFromLabels")),
        sCreateFromMarks(RTL_CONSTASCII_USTRINGPARAM("CreateFromMarks")),
        sCreateFromOutline(RTL_CONSTASCII_USTRINGPARAM("CreateFromOutline")),
        sDdeCommandElement(RTL_CONSTASCII_USTRINGPARAM("DDECommandElement")),
        sDdeCommandFile(RTL_CONSTASCII_USTRINGPARAM("DDECommandFile")),
        sDdeCommandType(RTL_CONSTASCII_USTRINGPARAM("DDECommandType")),
        sFileLink(RTL_CONSTASCII_USTRINGPARAM("FileLink")),
        sIsCaseSensitive(RTL_CONSTASCII_USTRINGPARAM("IsCaseSensitive")),
        sIsProtected(RTL_CONSTASCII_USTRINGPARAM("IsProtected")),
        sIsVisible(RTL_CONSTASCII_USTRINGPARAM("IsVisible")),
        sLabelCategory(RTL_CONSTASCII_USTRINGPARAM("LabelCategory")),
        sLabelDisplayType(RTL_CONSTASCII_USTRINGPARAM("LabelDisplayType")),
        sLevel(RTL_CONSTASCII_USTRINGPARAM("Level")),
        sLevelFormat(RTL_CONSTASCII_USTRINGPARAM("LevelFormat")),
        sLevelParagraphStyles(
            RTL_CONSTASCII_USTRINGPARAM("LevelParagraphStyles")),
        sLinkRegion(RTL_CONSTASCII_USTRINGPARAM("LinkRegion")),
        sMainEntryCharacterStyleName(
            RTL_CONSTASCII_USTRINGPARAM("MainEntryCharacterStyleName")),
        sParaStyleHeading(RTL_CONSTASCII_USTRINGPARAM("ParaStyleHeading")),
        sParaStyleLevel(RTL_CONSTASCII_USTRINGPARAM("ParaStyleLevel")),
        sSection(RTL_CONSTASCII_USTRINGPARAM(sXML_section)),
        sTitle(RTL_CONSTASCII_USTRINGPARAM("Title")),
        sUseAlphabeticalSeparators(
            RTL_CONSTASCII_USTRINGPARAM("UseAlphabeticalSeparators")),
        sUseCombinedEntries(RTL_CONSTASCII_USTRINGPARAM("UseCombinedEntries")),
        sUseDash(RTL_CONSTASCII_USTRINGPARAM("UseDash")),
        sUseKeyAsEntry(RTL_CONSTASCII_USTRINGPARAM("UseKeyAsEntry")),
        sUsePP(RTL_CONSTASCII_USTRINGPARAM("UsePP")),
        sUseUpperCase(RTL_CONSTASCII_USTRINGPARAM("UseUpperCase")),
        sCreateFromOtherEmbeddedObjects(RTL_CONSTASCII_USTRINGPARAM("CreateFromOtherEmbeddedObjects")),
        sCreateFromStarCalc(RTL_CONSTASCII_USTRINGPARAM("CreateFromStarCalc")),
        sCreateFromStarChart(RTL_CONSTASCII_USTRINGPARAM("CreateFromStarChart")),
        sCreateFromStarDraw(RTL_CONSTASCII_USTRINGPARAM("CreateFromStarDraw")),
        sCreateFromStarImage(RTL_CONSTASCII_USTRINGPARAM("CreateFromStarImage")),
        sCreateFromStarMath(RTL_CONSTASCII_USTRINGPARAM("CreateFromStarMath")),
        sCreateFromEmbeddedObjects(RTL_CONSTASCII_USTRINGPARAM("CreateFromEmbeddedObjects")),
        sCreateFromGraphicObjects(RTL_CONSTASCII_USTRINGPARAM("CreateFromGraphicObjects")),
        sCreateFromTables(RTL_CONSTASCII_USTRINGPARAM("CreateFromTables")),
        sCreateFromTextFrames(RTL_CONSTASCII_USTRINGPARAM("CreateFromTextFrames")),
        sUseLevelFromSource(RTL_CONSTASCII_USTRINGPARAM("UseLevelFromSource")),
        sIsCommaSeparated(RTL_CONSTASCII_USTRINGPARAM("IsCommaSeparated")),
        sIsAutomaticUpdate(RTL_CONSTASCII_USTRINGPARAM("IsAutomaticUpdate")),
        sIsRelativeTabstops(RTL_CONSTASCII_USTRINGPARAM("IsRelativeTabstops")),
        sTableOfContent(RTL_CONSTASCII_USTRINGPARAM(sXML_table_of_content)),
        sIllustrationIndex(RTL_CONSTASCII_USTRINGPARAM(sXML_illustration_index)),
        sAlphabeticalIndex(RTL_CONSTASCII_USTRINGPARAM(sXML_alphabetical_index)),
        sTableIndex(RTL_CONSTASCII_USTRINGPARAM(sXML_table_index)),
        sObjectIndex(RTL_CONSTASCII_USTRINGPARAM(sXML_object_index)),
        sBibliography(RTL_CONSTASCII_USTRINGPARAM(sXML_bibliography)),
        sUserIndex(RTL_CONSTASCII_USTRINGPARAM(sXML_user_index)),
        sEmpty()
{
}

void XMLSectionExport::ExportSectionStart(
    const Reference<XTextSection> & rSection,
    sal_Bool bAutoStyles)
{
    if (bAutoStyles)
    {
        // get PropertySet and add section style
        Reference<XPropertySet> xPropertySet(rSection, UNO_QUERY);
        GetParaExport().Add( XML_STYLE_FAMILY_TEXT_SECTION, xPropertySet );
    }
    else
    {
        ExportRegularSectionStart(rSection);
    }
}

void XMLSectionExport::ExportSectionEnd(
    const Reference<XTextSection> & rSection,
    sal_Bool bAutoStyles)
{
    // any old attributes?
    GetExport().CheckAttrList();

    // TODO: end element section dependent

    // export end of element
    GetExport().GetDocHandler()->ignorableWhitespace( GetExport().sWS );
    GetExport().GetDocHandler()->endElement(
        GetExport().GetNamespaceMap().GetQNameByKey(XML_NAMESPACE_TEXT,
                                                    sSection) );
    GetExport().GetDocHandler()->ignorableWhitespace( GetExport().sWS );
}

void XMLSectionExport::ExportIndexStart(
    const Reference<XDocumentIndex> & rIndex,
    sal_Bool bAutoStyles)
{
    // HACK: disable index export until all problems have been resolved:
//  return;

    // get PropertySet
    Reference<XPropertySet> xPropertySet(rIndex, UNO_QUERY);

    if (bAutoStyles)
    {
        // treat index as section style
        GetParaExport().Add( XML_STYLE_FAMILY_TEXT_SECTION, xPropertySet );
    }
    else
    {
        switch (MapSectionType(rIndex->getServiceName()))
        {
            case TEXT_SECTION_TYPE_TOC:
                ExportTableOfContentStart(xPropertySet);
                break;

            case TEXT_SECTION_TYPE_ILLUSTRATION:
                ExportIllustrationIndexStart(xPropertySet);
                break;

            case TEXT_SECTION_TYPE_ALPHABETICAL:
                ExportAlphabeticalIndexStart(xPropertySet);
                break;

            case TEXT_SECTION_TYPE_TABLE:
                ExportTableIndexStart(xPropertySet);
                break;

            case TEXT_SECTION_TYPE_OBJECT:
                ExportObjectIndexStart(xPropertySet);
                break;

            case TEXT_SECTION_TYPE_USER:
                ExportUserIndexStart(xPropertySet);
                break;

            case TEXT_SECTION_TYPE_BIBLIOGRAPHY:
                ExportBibliographyStart(xPropertySet);
                break;

            default:
                // skip index
                DBG_ERROR("unknown index type");
                break;
        }
    }

    // TODO: remove when proper solution found
    ExportIndexEnd(rIndex, bAutoStyles);
}


SvXMLEnumMapEntry __READONLY_DATA aIndexTypeMap[] =
{
    { "com.sun.star.text.ContentIndex",     TEXT_SECTION_TYPE_TOC },
    { "com.sun.star.text.DocumentIndex",    TEXT_SECTION_TYPE_ALPHABETICAL },
    { "com.sun.star.text.TableIndex",       TEXT_SECTION_TYPE_TABLE },
    { "com.sun.star.text.ObjectIndex",      TEXT_SECTION_TYPE_OBJECT },
    { "com.sun.star.text.Bibliography",     TEXT_SECTION_TYPE_BIBLIOGRAPHY },
    { "com.sun.star.text.UserIndex",        TEXT_SECTION_TYPE_USER },
    { "com.sun.star.text.IllustrationsIndex", TEXT_SECTION_TYPE_ILLUSTRATION },
    { NULL, NULL }
};

enum SectionTypeEnum XMLSectionExport::MapSectionType(
    const OUString& rServiceName)
{
    enum SectionTypeEnum eType = TEXT_SECTION_TYPE_UNKNOWN;

    sal_uInt16 nTmp;
    if (SvXMLUnitConverter::convertEnum(nTmp, rServiceName, aIndexTypeMap))
    {
        eType = (enum SectionTypeEnum)nTmp;
    }

    // TODO: index header section types, etc.

    return eType;
}

void XMLSectionExport::ExportIndexEnd(
    const Reference<XDocumentIndex> & rIndex,
    sal_Bool bAutoStyles)
{
    // HACK: disable index export until all problems have been resolved:
//  return;

    if (! bAutoStyles)
    {
        sal_Char* pElementName = NULL;

        switch (MapSectionType(rIndex->getServiceName()))
        {
            case TEXT_SECTION_TYPE_TOC:
                pElementName = sXML_table_of_content;
                break;

            case TEXT_SECTION_TYPE_ILLUSTRATION:
                pElementName = sXML_illustration_index;
                break;

            case TEXT_SECTION_TYPE_ALPHABETICAL:
                pElementName = sXML_alphabetical_index;
                break;

            case TEXT_SECTION_TYPE_TABLE:
                pElementName = sXML_table_index;
                break;

            case TEXT_SECTION_TYPE_OBJECT:
                pElementName = sXML_object_index;
                break;

            case TEXT_SECTION_TYPE_USER:
                pElementName = sXML_user_index;
                break;

            case TEXT_SECTION_TYPE_BIBLIOGRAPHY:
                pElementName = sXML_bibliography;
                break;

            default:
                DBG_ERROR("unknown index type");
                // default: skip index!
                break;
        }

        if (NULL != pElementName)
        {
            // TODO: remove, if end handling is made proper
            GetExport().GetDocHandler()->endElement(
                GetExport().GetNamespaceMap().GetQNameByKey(
                    XML_NAMESPACE_TEXT,
                    OUString::createFromAscii(pElementName)));
            GetExport().GetDocHandler()->ignorableWhitespace(GetExport().sWS);
        }
    }
}

void XMLSectionExport::ExportRegularSectionStart(
    const Reference<XTextSection> & rSection)
{
    // any old attributes?
    GetExport().CheckAttrList();

    Reference<XNamed> xName(rSection, UNO_QUERY);
    GetExport().AddAttribute(XML_NAMESPACE_TEXT, sXML_name,
                             xName->getName());

    // get XPropertySet for other values
    Reference<XPropertySet> xPropSet(rSection, UNO_QUERY);
    Any aAny;

    // style name
    GetExport().AddAttribute(XML_NAMESPACE_TEXT, sXML_style_name,
                             GetParaExport().Find(
                                 XML_STYLE_FAMILY_TEXT_SECTION,
                                 xPropSet, sEmpty ));

    // condition and display
    aAny = xPropSet->getPropertyValue(sCondition);
    OUString sCond;
    aAny >>= sCond;
    sal_Char* pDisplay;
    if (sCond.getLength() > 0)
    {
        GetExport().AddAttribute(XML_NAMESPACE_TEXT, sXML_condition,
                                 sCond);
        pDisplay = sXML_condition;
    }
    else
    {
        pDisplay = sXML_none;
    }
    aAny = xPropSet->getPropertyValue(sIsVisible);
    if (! *(sal_Bool*)aAny.getValue())
    {
        GetExport().AddAttributeASCII(XML_NAMESPACE_TEXT, sXML_display,
                                      pDisplay);
    }

    // export element
    GetExport().GetDocHandler()->ignorableWhitespace( GetExport().sWS );
    GetExport().GetDocHandler()->startElement(
        GetExport().GetNamespaceMap().GetQNameByKey(XML_NAMESPACE_TEXT,
                                                    sSection),
        GetExport().GetXAttrList() );
    GetExport().GetDocHandler()->ignorableWhitespace( GetExport().sWS );
    GetExport().ClearAttrList();

    // data source
    // unfortunately, we have to test all relevant strings for non-zero length
    aAny = xPropSet->getPropertyValue(sFileLink);
    SectionFileLink aFileLink;
    aAny >>= aFileLink;

    aAny = xPropSet->getPropertyValue(sLinkRegion);
    OUString sRegionName;
    aAny >>= sRegionName;

    if ( (aFileLink.FileURL.getLength() > 0) ||
         (aFileLink.FilterName.getLength() > 0) ||
         (sRegionName.getLength() > 0) )
    {
        if (aFileLink.FileURL.getLength() > 0)
        {
            GetExport().AddAttribute(XML_NAMESPACE_XLINK, sXML_href,
                                     aFileLink.FileURL);
        }

        if (aFileLink.FilterName.getLength() > 0)
        {
            GetExport().AddAttribute(XML_NAMESPACE_TEXT, sXML_filter_name,
                                     aFileLink.FilterName);
        }

        if (sRegionName.getLength() > 0)
        {
            GetExport().AddAttribute(XML_NAMESPACE_TEXT, sXML_section_name,
                                     sRegionName);
        }

        SvXMLElementExport aElem(GetExport(),
                                 XML_NAMESPACE_TEXT, sXML_section_source,
                                 sal_True, sal_True);
    }
    else
    {
        // data source DDE
        // unfortunately, we have to test all relevant strings for
        // non-zero length
        aAny = xPropSet->getPropertyValue(sDdeCommandFile);
        OUString sApplication;
        aAny >>= sApplication;
        aAny = xPropSet->getPropertyValue(sDdeCommandType);
        OUString sTopic;
        aAny >>= sTopic;
        aAny = xPropSet->getPropertyValue(sDdeCommandElement);
        OUString sItem;
        aAny >>= sItem;

        if ( (sApplication.getLength() > 0) ||
             (sTopic.getLength() > 0) ||
             (sItem.getLength() > 0 )   )
        {
            GetExport().AddAttribute(XML_NAMESPACE_OFFICE,
                                     sXML_dde_application, sApplication);
            GetExport().AddAttribute(XML_NAMESPACE_OFFICE, sXML_dde_topic,
                                     sTopic);
            GetExport().AddAttribute(XML_NAMESPACE_OFFICE, sXML_dde_item,
                                     sItem);

            aAny = xPropSet->getPropertyValue(sIsAutomaticUpdate);
            if (*(sal_Bool*)aAny.getValue())
            {
                GetExport().AddAttributeASCII(XML_NAMESPACE_OFFICE,
                                             sXML_automatic_update, sXML_true);
            }

            SvXMLElementExport aElem(GetExport(),
                                     XML_NAMESPACE_OFFICE,
                                     sXML_dde_source,
                                     sal_True, sal_True);
        }
        // else: no data source
    }
}

void XMLSectionExport::ExportTableOfContentStart(
    const Reference<XPropertySet> & rPropertySet)
{
    // export TOC element start
    ExportBaseIndexStart(sTableOfContent, rPropertySet);

    // scope for table-of-content-source element
    {

        Any aAny;

        // TOC specific index source attributes:

        // outline-level (none|1..10)
        aAny = rPropertySet->getPropertyValue(sCreateFromOutline);
        if (*(sal_Bool*)aAny.getValue())
        {
            // outline-level: 1..10
            aAny = rPropertySet->getPropertyValue(sLevel);
            sal_Int16 nLevel;
            aAny >>= nLevel;

            OUStringBuffer sBuffer;
            SvXMLUnitConverter::convertNumber(sBuffer,
                                              (sal_Int32)nLevel);

            GetExport().AddAttribute(XML_NAMESPACE_TEXT,
                                     sXML_outline_level,
                                     sBuffer.makeStringAndClear());
        }
        else
        {
            // outline-level: none
            GetExport().AddAttributeASCII(XML_NAMESPACE_TEXT,
                                          sXML_outline_level,
                                          sXML_none);
        }

        // use index marks
        aAny = rPropertySet->getPropertyValue(sCreateFromMarks);
        if (! (*(sal_Bool*)aAny.getValue()))
        {
            GetExport().AddAttributeASCII(XML_NAMESPACE_TEXT,
                                          sXML_use_index_marks,
                                          sXML_true);
        }

        ExportBaseIndexSource(TEXT_SECTION_TYPE_TOC, rPropertySet);
    }

    ExportBaseIndexBody(TEXT_SECTION_TYPE_TOC, rPropertySet);
}

void XMLSectionExport::ExportObjectIndexStart(
    const Reference<XPropertySet> & rPropertySet)
{
    // export index start
    ExportBaseIndexStart(sObjectIndex, rPropertySet);

    // scope for index source element
    {
        ExportBoolean(rPropertySet, sCreateFromOtherEmbeddedObjects,
                      sXML_use_other_objects, sal_False);
        ExportBoolean(rPropertySet, sCreateFromStarCalc,
                      sXML_use_spreadsheet_objects, sal_False);
        ExportBoolean(rPropertySet, sCreateFromStarChart,
                      sXML_use_chart_objects, sal_False);
        ExportBoolean(rPropertySet, sCreateFromStarDraw,
                      sXML_use_draw_objects,sal_False);
        ExportBoolean(rPropertySet, sCreateFromStarMath,
                      sXML_use_math_objects, sal_False);

        ExportBaseIndexSource(TEXT_SECTION_TYPE_OBJECT, rPropertySet);
    }

    ExportBaseIndexBody(TEXT_SECTION_TYPE_OBJECT, rPropertySet);
}

void XMLSectionExport::ExportIllustrationIndexStart(
    const Reference<XPropertySet> & rPropertySet)
{
    // export index start
    ExportBaseIndexStart(sIllustrationIndex, rPropertySet);

    // scope for index source element
    {
        // export common attributes for illustration and table indices
        ExportTableAndIllustrationIndexSourceAttributes(rPropertySet);

        ExportBaseIndexSource(TEXT_SECTION_TYPE_ILLUSTRATION, rPropertySet);
    }

    ExportBaseIndexBody(TEXT_SECTION_TYPE_ILLUSTRATION, rPropertySet);
}

void XMLSectionExport::ExportTableIndexStart(
    const Reference<XPropertySet> & rPropertySet)
{
    // export index start
    ExportBaseIndexStart(sTableIndex, rPropertySet);

    // scope for index source element
    {
        // export common attributes for illustration and table indices
        ExportTableAndIllustrationIndexSourceAttributes(rPropertySet);

        ExportBaseIndexSource(TEXT_SECTION_TYPE_TABLE, rPropertySet);
    }

    ExportBaseIndexBody(TEXT_SECTION_TYPE_TABLE, rPropertySet);
}

void XMLSectionExport::ExportAlphabeticalIndexStart(
    const Reference<XPropertySet> & rPropertySet)
{
    // export TOC element start
    ExportBaseIndexStart(sAlphabeticalIndex, rPropertySet);

    // scope for table-of-content-source element
    {

        // style name (if present)
        Any aAny;
        aAny = rPropertySet->getPropertyValue(sMainEntryCharacterStyleName);
        OUString sStyleName;
        aAny >>= sStyleName;
        if (sStyleName.getLength())
        {
            GetExport().AddAttribute(XML_NAMESPACE_TEXT,
                                     sXML_main_entry_style_name,
                                     sStyleName);
        }

        // other (boolean) attributes
        ExportBoolean(rPropertySet, sIsCaseSensitive, sXML_ignore_case,
                      sal_False, sal_True);
        ExportBoolean(rPropertySet, sUseAlphabeticalSeparators,
                      sXML_alphabetical_separators, sal_False);
        ExportBoolean(rPropertySet, sUseCombinedEntries, sXML_combine_entries,
                      sal_True);
        ExportBoolean(rPropertySet, sUseDash, sXML_combine_entries_with_dash,
                      sal_False);
        ExportBoolean(rPropertySet, sUseKeyAsEntry, sXML_use_keys_as_entries,
                      sal_False);
        ExportBoolean(rPropertySet, sUsePP, sXML_combine_entries_with_pp,
                      sal_True);
        ExportBoolean(rPropertySet, sUseUpperCase, sXML_capitalize_entries,
                      sal_False);
        ExportBoolean(rPropertySet, sIsCommaSeparated, sXML_comma_separated,
                      sal_False);

        ExportBaseIndexSource(TEXT_SECTION_TYPE_ALPHABETICAL, rPropertySet);
    }

    ExportBaseIndexBody(TEXT_SECTION_TYPE_ALPHABETICAL, rPropertySet);
}

void XMLSectionExport::ExportUserIndexStart(
    const Reference<XPropertySet> & rPropertySet)
{
    // export TOC element start
    ExportBaseIndexStart(sUserIndex, rPropertySet);

    // scope for table-of-content-source element
    {
        // bool attributes
        ExportBoolean(rPropertySet, sCreateFromEmbeddedObjects,
                      sXML_use_objects, sal_False);
        ExportBoolean(rPropertySet, sCreateFromGraphicObjects,
                      sXML_use_graphics, sal_False);
        ExportBoolean(rPropertySet, sCreateFromMarks,
                      sXML_use_index_marks, sal_False);
        ExportBoolean(rPropertySet, sCreateFromTables,
                      sXML_use_tables, sal_False);
        ExportBoolean(rPropertySet, sCreateFromTextFrames,
                      sXML_use_floating_frames, sal_False);
        ExportBoolean(rPropertySet, sUseLevelFromSource,
                      sXML_copy_outline_levels, sal_False);

        ExportBaseIndexSource(TEXT_SECTION_TYPE_USER, rPropertySet);
    }

    ExportBaseIndexBody(TEXT_SECTION_TYPE_USER, rPropertySet);
}

void XMLSectionExport::ExportBibliographyStart(
    const Reference<XPropertySet> & rPropertySet)
{
    // export TOC element start
    ExportBaseIndexStart(sBibliography, rPropertySet);

    // scope for table-of-content-source element
    {
        // No attributes. Fine.

        ExportBaseIndexSource(TEXT_SECTION_TYPE_BIBLIOGRAPHY, rPropertySet);
    }

    ExportBaseIndexBody(TEXT_SECTION_TYPE_BIBLIOGRAPHY, rPropertySet);
}


void XMLSectionExport::ExportBaseIndexStart(
    const OUString sElementName,
    const Reference<XPropertySet> & rPropertySet)
{
    // section style name
    GetExport().AddAttribute(XML_NAMESPACE_TEXT, sXML_style_name,
                             GetParaExport().Find(
                                 XML_STYLE_FAMILY_TEXT_SECTION,
                                 rPropertySet, sEmpty ));

    // index  Element start
    GetExport().GetDocHandler()->ignorableWhitespace( GetExport().sWS );
    GetExport().GetDocHandler()->startElement(
        GetExport().GetNamespaceMap().GetQNameByKey(XML_NAMESPACE_TEXT,
                                                    sElementName),
        GetExport().GetXAttrList() );
    GetExport().ClearAttrList();
}

static const sal_Char* aTypeSourceElementNameMap[] =
{
    sXML_table_of_content_source,       // TOC
    sXML_table_index_source,            // table index
    sXML_illustration_index_source,     // illustration index
    sXML_object_index_source,           // object index
    sXML_user_index_source,             // user index
    sXML_alphabetical_index_source,     // alphabetical index
    sXML_bibliography_source            // bibliography
};

void XMLSectionExport::ExportBaseIndexSource(
    SectionTypeEnum eType,
    const Reference<XPropertySet> & rPropertySet)
{
    // check type
    DBG_ASSERT(eType >= TEXT_SECTION_TYPE_TOC, "illegal index type");
    DBG_ASSERT(eType <= TEXT_SECTION_TYPE_BIBLIOGRAPHY, "illegal index type");

    Any aAny;

    // common attributes; not supported by bibliography
    if (eType != TEXT_SECTION_TYPE_BIBLIOGRAPHY)
    {
        // document or chapter index?
        aAny = rPropertySet->getPropertyValue(sCreateFromChapter);
        if (*(sal_Bool*)aAny.getValue())
        {
            GetExport().AddAttributeASCII(XML_NAMESPACE_TEXT,
                                          sXML_index_scope,
                                          sXML_chapter);
        }

        // tab-stops relative to margin?
        aAny = rPropertySet->getPropertyValue(sIsRelativeTabstops);
        if (! *(sal_Bool*)aAny.getValue())
        {
            GetExport().AddAttributeASCII(XML_NAMESPACE_TEXT,
                                          sXML_relative_tab_stop_position,
                                          sXML_false);
        }
    }

    // the index source element (all indices)
    SvXMLElementExport aElem(GetExport(),
                             XML_NAMESPACE_TEXT,
                             aTypeSourceElementNameMap[
                                 eType - TEXT_SECTION_TYPE_TOC],
                             sal_True, sal_True);

    // scope for title template (all indices)
    {
        // header style name
        aAny = rPropertySet->getPropertyValue(sParaStyleHeading);
        OUString sStyleName;
        aAny >>= sStyleName;
        GetExport().AddAttribute(XML_NAMESPACE_TEXT,
                                 sXML_style_name,
                                 sStyleName);

        // title template
        SvXMLElementExport aHeaderTemplate(GetExport(),
                                           XML_NAMESPACE_TEXT,
                                           sXML_index_title_template,
                                           sal_True, sal_False);

        // title as element content
        aAny = rPropertySet->getPropertyValue(sTitle);
        OUString sTitleString;
        aAny >>= sTitleString;
        GetExport().GetDocHandler()->characters(sTitleString);
    }

    // export level templates (all indices)
    aAny = rPropertySet->getPropertyValue(sLevelFormat);
    Reference<XIndexReplace> xLevelTemplates;
    aAny >>= xLevelTemplates;

    // iterate over level formats;
    // skip element 0 (empty template for title)
    sal_Int32 nLevelCount = xLevelTemplates->getCount();
    for(sal_Int32 i = 1; i<nLevelCount; i++)
    {
        // get sequence
        Sequence<PropertyValues> aTemplateSequence;
        aAny = xLevelTemplates->getByIndex(i);
        aAny >>= aTemplateSequence;

        // export the sequence
        ExportIndexTemplate(eType, i, rPropertySet, aTemplateSequence);
    }

    // only TOC and user index:
    // styles from which to build the index (LevelParagraphStyles)
    if ( (TEXT_SECTION_TYPE_TOC == eType) ||
         (TEXT_SECTION_TYPE_USER == eType)   )
    {
        aAny = rPropertySet->getPropertyValue(sLevelParagraphStyles);
        Reference<XIndexReplace> xLevelParagraphStyles;
        aAny >>= xLevelParagraphStyles;
        ExportLevelParagraphStyles(xLevelParagraphStyles);
    }
}


void XMLSectionExport::ExportBaseIndexBody(
    SectionTypeEnum eType,
    const Reference<XPropertySet> & rSection)
{
    // type not used; checked anyway.
    DBG_ASSERT(eType >= TEXT_SECTION_TYPE_TOC, "illegal index type");
    DBG_ASSERT(eType <= TEXT_SECTION_TYPE_BIBLIOGRAPHY, "illegal index type");

    SvXMLElementExport aIndexBody(GetExport(),
                                  XML_NAMESPACE_TEXT,
                                  sXML_index_body,
                                  sal_True, sal_True);
}

void XMLSectionExport::ExportTableAndIllustrationIndexSourceAttributes(
    const Reference<XPropertySet> & rPropertySet)
{
    // use caption
    Any aAny = rPropertySet->getPropertyValue(sCreateFromLabels);
    if (! *(sal_Bool*)aAny.getValue())
    {
        GetExport().AddAttributeASCII(XML_NAMESPACE_TEXT,
                                      sXML_use_caption,
                                      sXML_false);
    }

    // sequence name
    aAny = rPropertySet->getPropertyValue(sLabelCategory);
    OUString sSequenceName;
    aAny >>= sSequenceName;
    GetExport().AddAttribute(XML_NAMESPACE_TEXT,
                             sXML_caption_sequence_name,
                             sSequenceName);

    // caption format
    aAny = rPropertySet->getPropertyValue(sLabelDisplayType);
    sal_Int16 nType;
    aAny >>= nType;
    GetExport().AddAttributeASCII(XML_NAMESPACE_TEXT,
                                  sXML_caption_sequence_format,
                                  XMLTextFieldExport::MapReferenceType(nType));
}


// map index of LevelFormats to attribute value;
// level 0 is always the header
static const sal_Char* aLevelNameTOCMap[] =
    { NULL, sXML_1, sXML_2, sXML_3, sXML_4, sXML_5, sXML_6, sXML_7,
          sXML_8, sXML_9, sXML_10, NULL };
static const sal_Char* aLevelNameTableMap[] =
    { NULL, "", NULL };
static const sal_Char* aLevelNameAlphaMap[] =
    { NULL, sXML_separator, sXML_1, sXML_2, sXML_3, NULL };
static const sal_Char* aLevelNameBibliographyMap[] =
    { NULL, sXML_article, sXML_book, sXML_booklet, sXML_conference,
          sXML_custom1, sXML_custom2, sXML_custom3, sXML_custom4,
          sXML_custom5, sXML_email, sXML_inbook, sXML_incollection,
          sXML_inproceedings, sXML_journal,
          sXML_manual, sXML_mastersthesis, sXML_misc, sXML_phdthesis,
          sXML_proceedings, sXML_techreport, sXML_unpublished, sXML_www,
          NULL };

static const sal_Char** aTypeLevelNameMap[] =
{
    aLevelNameTOCMap,           // TOC
    aLevelNameTableMap,         // table index
    aLevelNameTableMap,         // illustration index
    aLevelNameTableMap,         // object index
    aLevelNameTOCMap,           // user index
    aLevelNameAlphaMap,         // alphabetical index
    aLevelNameBibliographyMap   // bibliography
};

static const sal_Char* aLevelStylePropNameTOCMap[] =
    { NULL, "ParaStyleLevel1", "ParaStyleLevel2", "ParaStyleLevel3",
          "ParaStyleLevel4", "ParaStyleLevel5", "ParaStyleLevel6",
          "ParaStyleLevel7", "ParaStyleLevel8", "ParaStyleLevel9",
          "ParaStyleLevel10", NULL };
static const sal_Char* aLevelStylePropNameTableMap[] =
    { NULL, "ParaStyleLevel1", NULL };
static const sal_Char* aLevelStylePropNameAlphaMap[] =
    { NULL, "ParaStyleSeparator", "ParaStyleLevel1", "ParaStyleLevel2",
          "ParaStyleLevel3", NULL };
static const sal_Char* aLevelStylePropNameBibliographyMap[] =
          // TODO: replace with real property names, when available
    { NULL, "ParaStyleLevel1", "ParaStyleLevel1", "ParaStyleLevel1",
          "ParaStyleLevel1", "ParaStyleLevel1", "ParaStyleLevel1",
          "ParaStyleLevel1", "ParaStyleLevel1", "ParaStyleLevel1",
          "ParaStyleLevel1", "ParaStyleLevel1", "ParaStyleLevel1",
          "ParaStyleLevel1", "ParaStyleLevel1", "ParaStyleLevel1",
          "ParaStyleLevel1", "ParaStyleLevel1", "ParaStyleLevel1",
          "ParaStyleLevel1", "ParaStyleLevel1", "ParaStyleLevel1",
          "ParaStyleLevel1",
          NULL };

static const sal_Char** aTypeLevelStylePropNameMap[] =
{
    aLevelStylePropNameTOCMap,          // TOC
    aLevelStylePropNameTableMap,        // table index
    aLevelStylePropNameTableMap,        // illustration index
    aLevelStylePropNameTableMap,        // object index
    aLevelStylePropNameTOCMap,          // user index
    aLevelStylePropNameAlphaMap,        // alphabetical index
    aLevelStylePropNameBibliographyMap  // bibliography
};

static const sal_Char* aTypeLevelAttrMap[] =
{
    sXML_outline_level,     // TOC
    NULL,                   // table index
    NULL,                   // illustration index
    NULL,                   // object index
    sXML_outline_level,     // user index
    sXML_outline_level,     // alphabetical index
    sXML_bibliography_type  // bibliography
};

static const sal_Char* aTypeElementNameMap[] =
{
    sXML_table_of_content_entry_template,   // TOC
    sXML_table_index_entry_template,        // table index
    sXML_illustration_index_entry_template, // illustration index
    sXML_object_index_entry_template,       // object index
    sXML_user_index_entry_template,         // user index
    sXML_alphabetical_index_entry_template, // alphabetical index
    sXML_bibliography_entry_template        // bibliography
};


void XMLSectionExport::ExportIndexTemplate(
    SectionTypeEnum eType,
    sal_Int32 nOutlineLevel,
    const Reference<XPropertySet> & rPropertySet,
    Sequence<Sequence<PropertyValue> > & rValues)
{
    DBG_ASSERT(eType >= TEXT_SECTION_TYPE_TOC, "illegal index type");
    DBG_ASSERT(eType <= TEXT_SECTION_TYPE_BIBLIOGRAPHY, "illegal index type");
    DBG_ASSERT(nOutlineLevel >= 0, "illegal outline level");

    if ( (eType >= TEXT_SECTION_TYPE_TOC) &&
         (eType <= TEXT_SECTION_TYPE_BIBLIOGRAPHY) &&
         (nOutlineLevel >= 0) )
    {
        // get level name and level attribute name from aLevelNameMap;
        const sal_Char* pLevelAttrName =
            aTypeLevelAttrMap[eType-TEXT_SECTION_TYPE_TOC];
        const sal_Char* pLevelName =
            aTypeLevelNameMap[eType-TEXT_SECTION_TYPE_TOC][nOutlineLevel];

        // output level name
        DBG_ASSERT(NULL != pLevelName, "can't find level name");
        if ((NULL != pLevelName) && (NULL != pLevelAttrName))
        {
            GetExport().AddAttributeASCII(XML_NAMESPACE_TEXT,
                                          pLevelAttrName,
                                          pLevelName);
        }

        // paragraph level style name
        const sal_Char* pPropName =
            aTypeLevelStylePropNameMap[eType-TEXT_SECTION_TYPE_TOC][nOutlineLevel];
        DBG_ASSERT(NULL != pPropName, "can't find property name");
        if (NULL != pPropName)
        {
            Any aAny = rPropertySet->getPropertyValue(
                OUString::createFromAscii(pPropName));
            OUString sParaStyleName;
            aAny >>= sParaStyleName;
            GetExport().AddAttribute(XML_NAMESPACE_TEXT,
                                     sXML_style_name,
                                     sParaStyleName);
        }

        // template element
        const sal_Char* pElementName =
            aTypeElementNameMap[eType - TEXT_SECTION_TYPE_TOC];
        SvXMLElementExport aLevelTemplate(GetExport(),
                                          XML_NAMESPACE_TEXT,
                                          pElementName,
                                          sal_True, sal_True);

        // export sequence
        sal_Int32 nTemplateCount = rValues.getLength();
        for(sal_Int32 nTemplateNo = 0;
            nTemplateNo < nTemplateCount;
            nTemplateNo++)
        {
            ExportIndexTemplateElement(
                rValues[nTemplateNo]);
        }
    }
}


enum TemplateTypeEnum
{
    TOK_TTYPE_ENTRY_NUMBER,
    TOK_TTYPE_ENTRY_TEXT,
    TOK_TTYPE_TAB_STOP,
    TOK_TTYPE_TEXT,
    TOK_TTYPE_PAGE_NUMBER,
    TOK_TTYPE_CHAPTER_INFO,
    TOK_TTYPE_HYPERLINK_START,
    TOK_TTYPE_HYPERLINK_END,
    TOK_TTYPE_BIBLIOGRAPHY,
    TOK_TTYPE_INVALID
};

enum TemplateParamEnum
{
    TOK_TPARAM_TOKEN_TYPE,
    TOK_TPARAM_CHAR_STYLE,
    TOK_TPARAM_TAB_RIGHT_ALIGNED,
    TOK_TPARAM_TAB_POSITION,
    TOK_TPARAM_TAB_FILL_CHAR,
    TOK_TPARAM_TEXT,
    TOK_TPARAM_CHAPTER_FORMAT,
    TOK_TPARAM_BIBLIOGRAPHY_DATA
};

SvXMLEnumMapEntry __READONLY_DATA aTemplateTypeMap[] =
{
    { "TokenEntryNumber",           TOK_TTYPE_ENTRY_NUMBER },
    { "TokenEntryText",             TOK_TTYPE_ENTRY_TEXT },
    { "TokenTabStop",               TOK_TTYPE_TAB_STOP },
    { "TokenText",                  TOK_TTYPE_TEXT },
    { "TokenPageNumber",            TOK_TTYPE_PAGE_NUMBER },
    { "TokenChapterInfo",           TOK_TTYPE_CHAPTER_INFO },
    { "TokenHyperlinkStart",        TOK_TTYPE_HYPERLINK_START },
    { "TokenHyperlinkEnd",          TOK_TTYPE_HYPERLINK_END },
    { "TokenBibliographyDataField", TOK_TTYPE_BIBLIOGRAPHY },
    { NULL, NULL }
};

SvXMLEnumMapEntry __READONLY_DATA aTemplateParamMap[] =
{
    { "TokenType",              TOK_TPARAM_TOKEN_TYPE },
    { "CharacterStyleName",     TOK_TPARAM_CHAR_STYLE },
    { "TabStopRightAligned",    TOK_TPARAM_TAB_RIGHT_ALIGNED },
    { "TabStopPosition",        TOK_TPARAM_TAB_POSITION },
    { "TabStopFillCharacter",   TOK_TPARAM_TAB_FILL_CHAR },
    { "Text",                   TOK_TPARAM_TEXT },
    { "ChapterFormat",          TOK_TPARAM_CHAPTER_FORMAT },
    { "BibliographyDataField",  TOK_TPARAM_BIBLIOGRAPHY_DATA },
    { NULL, NULL }
};

SvXMLEnumMapEntry __READONLY_DATA aBibliographyDataFieldMap[] =
{
    { sXML_address,             BibliographyDataField::ADDRESS },
    { sXML_annote,              BibliographyDataField::ANNOTE },
    { sXML_author,              BibliographyDataField::AUTHOR },
    { sXML_bibiliographic_type, BibliographyDataField::BIBILIOGRAPHIC_TYPE },
    { sXML_booktitle,           BibliographyDataField::BOOKTITLE },
    { sXML_chapter,             BibliographyDataField::CHAPTER },
    { sXML_custom1,             BibliographyDataField::CUSTOM1 },
    { sXML_custom2,             BibliographyDataField::CUSTOM2 },
    { sXML_custom3,             BibliographyDataField::CUSTOM3 },
    { sXML_custom4,             BibliographyDataField::CUSTOM4 },
    { sXML_custom5,             BibliographyDataField::CUSTOM5 },
    { sXML_edition,             BibliographyDataField::EDITION },
    { sXML_editor,              BibliographyDataField::EDITOR },
    { sXML_howpublished,        BibliographyDataField::HOWPUBLISHED },
    { sXML_identifier,          BibliographyDataField::IDENTIFIER },
    { sXML_institution,         BibliographyDataField::INSTITUTION },
    { sXML_isbn,                BibliographyDataField::ISBN },
    { sXML_journal,             BibliographyDataField::JOURNAL },
    { sXML_month,               BibliographyDataField::MONTH },
    { sXML_note,                BibliographyDataField::NOTE },
    { sXML_number,              BibliographyDataField::NUMBER },
    { sXML_organizations,       BibliographyDataField::ORGANIZATIONS },
    { sXML_pages,               BibliographyDataField::PAGES },
    { sXML_publisher,           BibliographyDataField::PUBLISHER },
    { sXML_report_type,         BibliographyDataField::REPORT_TYPE },
    { sXML_school,              BibliographyDataField::SCHOOL },
    { sXML_series,              BibliographyDataField::SERIES },
    { sXML_title,               BibliographyDataField::TITLE },
    { sXML_url,                 BibliographyDataField::URL },
    { sXML_volume,              BibliographyDataField::VOLUME },
    { sXML_year,                BibliographyDataField::YEAR },
    { NULL, NULL }
};

void XMLSectionExport::ExportIndexTemplateElement(
    Sequence<PropertyValue> & rValues)
{
    // variables for template values

    // char style
    OUString sCharStyle;
    sal_Bool bCharStyleOK = sal_False;

    // text
    OUString sText;
    sal_Bool bTextOK = sal_False;

    // tab position
    sal_Bool bRightAligned = sal_False;
    sal_Bool bRightAlignedOK = sal_False;

    // tab position
    sal_Int32 nTabPosition = 0;
    sal_Bool bTabPositionOK = sal_False;

    // fill character
    OUString sFillChar;
    sal_Bool bFillCharOK = sal_False;

    // chapter format
    sal_Int16 nChapterFormat;
    sal_Bool bChapterFormatOK = sal_False;

    // Bibliography Data
    sal_Int16 nBibliographyData;
    sal_Bool bBibliographyDataOK = sal_False;


    // token type
    enum TemplateTypeEnum nTokenType = TOK_TTYPE_INVALID;

    sal_Int32 nCount = rValues.getLength();
    for(sal_Int32 i = 0; i<nCount; i++)
    {
        sal_uInt16 nToken;
        if ( SvXMLUnitConverter::convertEnum( nToken, rValues[i].Name,
                                              aTemplateParamMap ) )
        {
            // Only use direct and default values.
            // Wrong. no property states, so ignore.
            // if ( (beans::PropertyState_DIRECT_VALUE == rValues[i].State) ||
            //      (beans::PropertyState_DEFAULT_VALUE == rValues[i].State)  )

            switch (nToken)
            {
                case TOK_TPARAM_TOKEN_TYPE:
                {
                    sal_uInt16 nTmp;
                    OUString sVal;
                    rValues[i].Value >>= sVal;
                    if (SvXMLUnitConverter::convertEnum( nTmp, sVal,
                                                         aTemplateTypeMap))
                    {
                        nTokenType = (enum TemplateTypeEnum)nTmp;
                    }
                    break;
                }

                case TOK_TPARAM_CHAR_STYLE:
                    // only valid, if not empty
                    rValues[i].Value >>= sCharStyle;
                    bCharStyleOK = sCharStyle.getLength() > 0;
                    break;

                case TOK_TPARAM_TEXT:
                    rValues[i].Value >>= sText;
                    bTextOK = sal_True;
                    break;

                case TOK_TPARAM_TAB_RIGHT_ALIGNED:
                    bRightAligned =
                        *(sal_Bool *)rValues[i].Value.getValue();
                    bRightAlignedOK = sal_True;
                    break;

                case TOK_TPARAM_TAB_POSITION:
                    rValues[i].Value >>= nTabPosition;
                    bTabPositionOK = sal_True;
                    break;

                case TOK_TPARAM_TAB_FILL_CHAR:
                    rValues[i].Value >>= sFillChar;
                    bFillCharOK = sal_True;
                    break;

                case TOK_TPARAM_CHAPTER_FORMAT:
                    rValues[i].Value >>= nChapterFormat;
                    bChapterFormatOK = sal_True;
                    break;
                case TOK_TPARAM_BIBLIOGRAPHY_DATA:
                    rValues[i].Value >>= nBibliographyData;
                    bBibliographyDataOK = sal_True;
                    break;
            }
        }
    }

    // convert type to token (and check validity) ...
    sal_Char* pElement = NULL;
    switch(nTokenType)
    {
        case TOK_TTYPE_ENTRY_TEXT:
            pElement = sXML_index_entry_text;
            break;
        case TOK_TTYPE_TAB_STOP:
            // test validity
            pElement = ( bRightAligned || bTabPositionOK || bFillCharOK )
                ? sXML_index_entry_tab_stop : NULL;
            break;
        case TOK_TTYPE_TEXT:
            // test validity
            pElement = bTextOK ? sXML_index_entry_span : NULL;
            break;
        case TOK_TTYPE_PAGE_NUMBER:
            pElement = sXML_index_entry_page_number;
            break;
        case TOK_TTYPE_CHAPTER_INFO:    // keyword index
            pElement = sXML_index_entry_chapter;
            break;
        case TOK_TTYPE_ENTRY_NUMBER:    // table of content
            pElement = sXML_index_entry_chapter_number;
            break;
        case TOK_TTYPE_HYPERLINK_START:
            pElement = sXML_index_entry_link_start;
            break;
        case TOK_TTYPE_HYPERLINK_END:
            pElement = sXML_index_entry_link_end;
            break;
        case TOK_TTYPE_BIBLIOGRAPHY:
            pElement = bBibliographyDataOK
                ? sXML_index_entry_bibliography : NULL;
            break;
        default:
            ; // unknown/unimplemented template
            break;
    }

    // ... and write Element
    if (pElement != NULL)
    {
        // character style (for most templates)
        if (bCharStyleOK)
        {
            switch (nTokenType)
            {
                case TOK_TTYPE_ENTRY_TEXT:
                case TOK_TTYPE_TEXT:
                case TOK_TTYPE_PAGE_NUMBER:
                case TOK_TTYPE_ENTRY_NUMBER:
                case TOK_TTYPE_HYPERLINK_START:
                case TOK_TTYPE_HYPERLINK_END:
                case TOK_TTYPE_BIBLIOGRAPHY:
                    GetExport().AddAttribute(XML_NAMESPACE_TEXT,
                                             sXML_style_name, sCharStyle);
                    break;
                default:
                    ; // nothing: no character style
                    break;
            }
        }

        // tab properties
        if (TOK_TTYPE_TAB_STOP == nTokenType)
        {
            // tab type
            GetExport().AddAttributeASCII(XML_NAMESPACE_STYLE,
                                          sXML_type,
                                       bRightAligned ? sXML_right : sXML_left);

            if (bTabPositionOK && (! bRightAligned))
            {
                // position for left tabs (convert to measure)
                OUStringBuffer sBuf;
                GetExport().GetMM100UnitConverter().convertMeasure(sBuf,
                                                                 nTabPosition);
                GetExport().AddAttribute(XML_NAMESPACE_STYLE,
                                         sXML_position,
                                         sBuf.makeStringAndClear());
            }

            // fill char ("leader char")
            if (bFillCharOK && (sFillChar.getLength() > 0))
            {
                GetExport().AddAttribute(XML_NAMESPACE_STYLE,
                                         sXML_leader_char, sFillChar);
            }
        }

        // bibliography data
        if (TOK_TTYPE_BIBLIOGRAPHY == nTokenType)
        {
            DBG_ASSERT(bBibliographyDataOK, "need bibl data");
            OUStringBuffer sBuf;
            if (SvXMLUnitConverter::convertEnum( sBuf, nBibliographyData,
                                                 aBibliographyDataFieldMap ) )
            {
                GetExport().AddAttribute(XML_NAMESPACE_TEXT,
                                         sXML_bibliography_data_field,
                                         sBuf.makeStringAndClear());
            }
        }

        // chapter info
        if (TOK_TTYPE_CHAPTER_INFO == nTokenType)
        {
            DBG_ASSERT(bChapterFormatOK, "need chapter info");
            GetExport().AddAttributeASCII(
                XML_NAMESPACE_TEXT, sXML_display,
                XMLTextFieldExport::MapChapterDisplayFormat(nChapterFormat));
        }

        // export template
        SvXMLElementExport aTemplateElement(GetExport(), XML_NAMESPACE_TEXT,
                                            pElement, sal_True, sal_False);

        // entry text or span element: write text
        if (TOK_TTYPE_TEXT == nTokenType)
        {
            GetExport().GetDocHandler()->characters(sText);
        }
    }
}

void XMLSectionExport::ExportLevelParagraphStyles(
    Reference<XIndexReplace> & xLevelParagraphStyles)
{
    // iterate over levels
    sal_Int32 nPLevelCount = xLevelParagraphStyles->getCount();
    for(sal_Int32 nLevel = 0; nLevel < nPLevelCount; nLevel++)
    {
        Any aAny = xLevelParagraphStyles->getByIndex(nLevel);
        Sequence<OUString> aStyleNames;
        aAny >>= aStyleNames;

        // export only if at least one style is contained
        sal_Int32 nNamesCount = aStyleNames.getLength();
        if (nNamesCount > 0)
        {
            // level attribute; we count 1..10; API 0..9
            OUStringBuffer sBuf;
            sal_Int32 nLevelPlusOne = nLevel + 1;
            SvXMLUnitConverter::convertNumber(sBuf, nLevelPlusOne);
            GetExport().AddAttribute(XML_NAMESPACE_TEXT,
                                     sXML_outline_level,
                                     sBuf.makeStringAndClear());

            // source styles element
            SvXMLElementExport aParaStyles(GetExport(),
                                           XML_NAMESPACE_TEXT,
                                           sXML_index_source_styles,
                                           sal_True, sal_True);

            // iterate over styles in this level
            for(sal_Int32 nName = 0; nName < nNamesCount; nName++)
            {
                // stylename attribute
                GetExport().AddAttribute(XML_NAMESPACE_TEXT,
                                         sXML_style_name,
                                         aStyleNames[nName]);

                // element
                SvXMLElementExport aParaStyle(GetExport(),
                                              XML_NAMESPACE_TEXT,
                                              sXML_index_source_style,
                                              sal_True, sal_False);
            }
        }
    }
}

void XMLSectionExport::ExportBoolean(
    const Reference<XPropertySet> & rPropSet,
    const OUString& sPropertyName,
    const sal_Char* pAttributeName,
    sal_Bool bDefault,
    sal_Bool bInvert)
{
    DBG_ASSERT(NULL != pAttributeName, "Need attribute name");

    Any aAny = rPropSet->getPropertyValue(sPropertyName);
    sal_Bool bTmp = *(sal_Bool*)aAny.getValue();

    // value = value ^ bInvert
    // omit if value == default
    // negate forces sal_Bool to 0/1, making them comparable
    if ((!(bTmp ^ bInvert)) != (!bDefault))
    {
        // export non-default value (since default is omitted)
        GetExport().AddAttributeASCII(XML_NAMESPACE_TEXT,
                                      pAttributeName,
                                      bDefault ? sXML_false : sXML_true);
    }
}

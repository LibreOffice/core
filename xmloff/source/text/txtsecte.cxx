/*************************************************************************
 *
 *  $RCSfile: txtsecte.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: dvo $ $Date: 2000-11-02 15:51:18 $
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

#ifndef _XMLOFF_TXTPARAE_HXX
#include "txtparae.hxx"
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


void XMLTextParagraphExport::exportListAndSectionChange(
    Reference<XTextSection> & rPrevSection,
    const Reference<XTextContent> & rNextSectionContent,
    const XMLTextNumRuleInfo& rPrevRule,
    const XMLTextNumRuleInfo& rNextRule,
    sal_Bool bAutoStyles)
{
    Reference<XTextSection> xNextSection;

    // first: get current XTextSection
    Reference<XPropertySet> xPropSet(rNextSectionContent, UNO_QUERY);
    if (xPropSet.is())
    {
        if (xPropSet->getPropertySetInfo()->hasPropertyByName(sTextSection))
        {
            Any aAny = xPropSet->getPropertyValue(sTextSection);
            aAny >>= xNextSection;
        }
        // else: no current section

        if (xPropSet->getPropertySetInfo()->hasPropertyByName(sDocumentIndex))
        {
            Any aAny = xPropSet->getPropertyValue(sDocumentIndex);
            Reference<XDocumentIndex> xDocumentIndex;
            aAny >>= xDocumentIndex;

            if (xDocumentIndex.is() && !rPrevSection.is())
            {
                exportDocumentIndex(xDocumentIndex, bAutoStyles);
            }
        }
    }
    // else: no current section

    // careful: exportListChange may only be called for (!bAutoStyles)
    // I'd like a cleaner solution! Maybe export all section styles upfront.
    if ( bAutoStyles )
    {
        if ( xNextSection.is() )
        {
            Reference<XPropertySet> xPropSet( xNextSection, UNO_QUERY);
            Add( XML_STYLE_FAMILY_TEXT_SECTION, xPropSet );
        }
    }
    else
    {
        // old != new? -> start/equal?
        if (rPrevSection != xNextSection)
        {
            // a new section started, or an old one gets closed!

            // close old list
            XMLTextNumRuleInfo aEmptyNumRule;
            exportListChange(rPrevRule, aEmptyNumRule);

            // build stacks of old and new sections
            vector<Reference<XTextSection> > aOldStack;
            Reference<XTextSection> aCurrent = rPrevSection;
            while(aCurrent.is())
            {
                aOldStack.push_back(aCurrent);
                aCurrent = aCurrent->getParentSection();
            }

            vector<Reference<XTextSection> > aNewStack;
            aCurrent = xNextSection;
            while(aCurrent.is())
            {
                aNewStack.push_back(aCurrent);
                aCurrent = aCurrent->getParentSection();
            }

            // compare the two stacks
            vector<Reference<XTextSection> > ::reverse_iterator aOld =
                aOldStack.rbegin();
            vector<Reference<XTextSection> > ::reverse_iterator aNew =
                aNewStack.rbegin();
            while ( (aOld != aOldStack.rend()) &&
                    (aNew != aNewStack.rend()) &&
                    (*aOld) == (*aNew) )
            {
                aOld++;
                aNew++;
            }

            // close all elements of aOld, open all of aNew
            while (aOld != aOldStack.rend())
            {
                Reference<XNamed> xName(*aOld, UNO_QUERY);
                GetExport().GetDocHandler()->ignorableWhitespace(
                    GetExport().sWS );
                GetExport().GetDocHandler()->endElement(sText_Section);
                GetExport().GetDocHandler()->ignorableWhitespace(
                    GetExport().sWS );
                aOld++;
            }

            while (aNew != aNewStack.rend())
            {
                exportSectionStart(*aNew);
                aNew++;
            }

            // start new list
            exportListChange(aEmptyNumRule, rNextRule);
        }
        else
        {
            // list change, if sections have not changed
            exportListChange(rPrevRule, rNextRule);
        }
    }

    // save old section (old numRule gets saved in calling method
    rPrevSection = xNextSection;
}

void XMLTextParagraphExport::exportSectionStart(
    const ::com::sun::star::uno::Reference <
        ::com::sun::star::text::XTextSection > & rSection)
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
                             Find( XML_STYLE_FAMILY_TEXT_SECTION,
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
    GetExport().GetDocHandler()->startElement( sText_Section,
                                               GetExport().GetXAttrList() );
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
            GetExport().AddAttribute(XML_NAMESPACE_TEXT, sXML_dde_application,
                                     sApplication);
            GetExport().AddAttribute(XML_NAMESPACE_TEXT, sXML_dde_topic,
                                     sTopic);
            GetExport().AddAttribute(XML_NAMESPACE_TEXT, sXML_dde_item,
                                     sItem);

            SvXMLElementExport aElem(GetExport(),
                                     XML_NAMESPACE_TEXT,
                                     sXML_section_source_dde,
                                     sal_True, sal_True);
        }
        // else: no data source
    }
}

void XMLTextParagraphExport::exportDocumentIndex(
    Reference<XDocumentIndex> & rIndex,
    sal_Bool bAutoStyles)
{
    OUString sServiceName = rIndex->getServiceName();

    const sal_Char sAPI_ContentIndex[] = "com.sun.star.text.ContentIndex";

    if (sServiceName.equalsAsciiL(sAPI_ContentIndex,
                                  sizeof(sAPI_ContentIndex)-1))
    {
        Reference<XPropertySet> xPropertySet(rIndex, UNO_QUERY);

        if (bAutoStyles)
        {
            // treat index as section style
            Add( XML_STYLE_FAMILY_TEXT_SECTION, xPropertySet );
        }
        else
        {
            Any aAny;

            // style name
            GetExport().AddAttribute(XML_NAMESPACE_TEXT, sXML_style_name,
                                     Find( XML_STYLE_FAMILY_TEXT_SECTION,
                                           xPropertySet, sEmpty ));

            // table of content Element
            SvXMLElementExport aElem(GetExport(),
                                     XML_NAMESPACE_TEXT,
                                     sXML_table_of_content,
                                     sal_True, sal_True);

            {
                // scope for table-of-content-source element

                // outline-level (none|1..10)
                aAny = xPropertySet->getPropertyValue(sCreateFromOutline);
                if (*(sal_Bool*)aAny.getValue())
                {
                    // outline-level: 1..10
                    aAny = xPropertySet->getPropertyValue(sLevel);
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
                aAny = xPropertySet->getPropertyValue(sCreateFromMarks);
                if (! (*(sal_Bool*)aAny.getValue()))
                {
                    GetExport().AddAttributeASCII(XML_NAMESPACE_TEXT,
                                                  sXML_use_index_marks,
                                                  sXML_true);
                }

                // document or chapter index?
                aAny = xPropertySet->getPropertyValue(sCreateFromChapter);
                if (*(sal_Bool*)aAny.getValue())
                {
                    GetExport().AddAttributeASCII(XML_NAMESPACE_TEXT,
                                                  sXML_index_scope,
                                                  sXML_chapter);
                }

                // the TOC source element
                SvXMLElementExport aElem(GetExport(),
                                         XML_NAMESPACE_TEXT,
                                         sXML_table_of_content_source,
                                         sal_True, sal_True);

                // styles from which to build the index (LevelParagraphStyles)
                aAny = xPropertySet->getPropertyValue(sLevelParagraphStyles);
                Reference<XIndexReplace> xLevelParagraphStyles;
                aAny >>= xLevelParagraphStyles;

                // iterate over levels
                sal_Int32 nPLevelCount = xLevelParagraphStyles->getCount();
                for(sal_Int32 nLevel = 0; nLevel < nPLevelCount; nLevel++)
                {
                    aAny = xLevelParagraphStyles->getByIndex(nLevel);
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

                // scope for title template
                {
                    // header style name
                    aAny = xPropertySet->getPropertyValue(sParaStyleHeading);
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
                    aAny = xPropertySet->getPropertyValue(sTitle);
                    OUString sTitleString;
                    aAny >>= sTitleString;
                    GetExport().GetDocHandler()->characters(sTitleString);
                }

                // export level templates
                aAny = xPropertySet->getPropertyValue(sLevelFormat);
                Reference<XIndexReplace> xLevelTemplates;
                aAny >>= xLevelTemplates;

                // iterate over level formats;
                // skip element 0 (empty template for title)
                sal_Int32 nLevelCount = xLevelTemplates->getCount();
                for(sal_Int32 i = 1; i<nLevelCount; i++)
                {
                    // level number
                    OUStringBuffer sBuffer;
                    SvXMLUnitConverter::convertNumber(sBuffer, i);
                    OUString sNumber = sBuffer.makeStringAndClear();

                    GetExport().AddAttribute(XML_NAMESPACE_TEXT,
                                             sXML_outline_level,
                                             sNumber);

                    // style name
                    sBuffer.append(sParaStyleLevel);
                    sBuffer.append(i);
                    aAny = xPropertySet->getPropertyValue(
                        sBuffer.makeStringAndClear());
                    OUString sLevelStyle;
                    aAny >>= sLevelStyle;
                    GetExport().AddAttribute(XML_NAMESPACE_TEXT,
                                             sXML_style_name,
                                             sLevelStyle);

                    // template element
                    SvXMLElementExport aLevelTemplate(GetExport(),
                                                      XML_NAMESPACE_TEXT,
                                                     sXML_index_entry_template,
                                                      sal_True, sal_True);

                    // get sequence
                    Sequence<PropertyValues> aTemplateSequence;
                    aAny = xLevelTemplates->getByIndex(i);
                    aAny >>= aTemplateSequence;

                    // export sequence
                    sal_Int32 nTemplateCount = aTemplateSequence.getLength();
                    for(sal_Int32 nTemplateNo = 0;
                        nTemplateNo < nTemplateCount;
                        nTemplateNo++)
                    {
                        exportIndexTemplateElement(
                            aTemplateSequence[nTemplateNo]);
                    }
                }
            }

            {
                SvXMLElementExport aIndexBody(GetExport(),
                                              XML_NAMESPACE_TEXT,
                                              sXML_table_of_content_body,
                                              sal_True, sal_True);
            }
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
    TOK_TPARAM_CHAPTER_FORMAT
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
    { "TokenBibliogaphyDataField",  TOK_TTYPE_BIBLIOGRAPHY },
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
    { NULL, NULL }
};

void XMLTextParagraphExport::exportIndexTemplateElement(
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
        case TOK_TTYPE_ENTRY_NUMBER:    // table of content
            // sanitize format: both as chapter fields
            pElement = sXML_index_entry_chapter;
            break;
        case TOK_TTYPE_HYPERLINK_START:
            pElement = sXML_index_entry_link_start;
            break;
        case TOK_TTYPE_HYPERLINK_END:
            pElement = sXML_index_entry_link_end;
            break;
        case TOK_TTYPE_BIBLIOGRAPHY:
            pElement = sXML_index_entry_bibliography;
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
                    GetExport().AddAttribute(XML_NAMESPACE_TEXT,
                                             sXML_style_name, sCharStyle);
                    break;
                default:
                    ; // nothing: no character style
                    break;
            }
        }

        if (TOK_TTYPE_TAB_STOP == nTokenType)
        {
            // tab type
            GetExport().AddAttributeASCII(XML_NAMESPACE_STYLE,
                                          sXML_type,
                                       bRightAligned ? sXML_right : sXML_left);

            if (! bRightAligned)
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
            GetExport().AddAttribute(XML_NAMESPACE_STYLE, sXML_leader_char,
                                     sFillChar);
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

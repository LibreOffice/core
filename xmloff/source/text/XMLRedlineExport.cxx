/*************************************************************************
 *
 *  $RCSfile: XMLRedlineExport.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: dvo $ $Date: 2001-03-09 14:13:18 $
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

#ifndef _XMLOFF_XMLREDLINEEXPORT_HXX
#include "XMLRedlineExport.hxx"
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

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XENUMERATIONACCESS_HPP_
#include <com/sun/star/container/XEnumerationAccess.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XENUMERATION_HPP_
#include <com/sun/star/beans/XEnumeration.hpp>
#endif

#ifndef _COM_SUN_STAR_DOCUMENT_XREDLINESSUPPLIER_HPP_
#include <com/sun/star/document/XRedlinesSupplier.hpp>
#endif

#ifndef _COM_SUN_STAR_TEXT_XTEXT_HPP_
#include <com/sun/star/text/XText.hpp>
#endif

#ifndef _COM_SUN_STAR_TEXT_XTEXTCONTENT_HPP_
#include <com/sun/star/text/XTextContent.hpp>
#endif

#ifndef _COM_SUN_STAR_TEXT_XTEXTSECTION_HPP_
#include <com/sun/star/text/XTextSection.hpp>
#endif

#ifndef _COM_SUN_STAR_UTIL_DATETIME_HPP_
#include <com/sun/star/util/DateTime.hpp>
#endif

#ifndef _XMLOFF_XMLKYWD_HXX
#include "xmlkywd.hxx"
#endif

#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif

#ifndef _XMLOFF_XMLEXP_HXX
#include "xmlexp.hxx"
#endif

#ifndef _XMLOFF_XMLUCONV_HXX
#include "xmluconv.hxx"
#endif


using namespace ::com::sun::star;

using ::com::sun::star::beans::PropertyValue;
using ::com::sun::star::beans::XPropertySet;
using ::com::sun::star::document::XRedlinesSupplier;
using ::com::sun::star::container::XEnumerationAccess;
using ::com::sun::star::container::XEnumeration;
using ::com::sun::star::text::XText;
using ::com::sun::star::text::XTextContent;
using ::com::sun::star::text::XTextSection;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::util::DateTime;
using ::rtl::OUString;
using ::rtl::OUStringBuffer;
using ::std::list;


XMLRedlineExport::XMLRedlineExport(SvXMLExport& rExp) :
    sDelete(RTL_CONSTASCII_USTRINGPARAM("Delete")),
    sDeletion(RTL_CONSTASCII_USTRINGPARAM(sXML_deletion)),
    sFormat(RTL_CONSTASCII_USTRINGPARAM("Format")),
    sFormatChange(RTL_CONSTASCII_USTRINGPARAM(sXML_format_change)),
    sInsert(RTL_CONSTASCII_USTRINGPARAM("Insert")),
    sInsertion(RTL_CONSTASCII_USTRINGPARAM(sXML_insertion)),
    sIsCollapsed(RTL_CONSTASCII_USTRINGPARAM("IsCollapsed")),
    sIsStart(RTL_CONSTASCII_USTRINGPARAM("IsStart")),
    sRedlineAuthor(RTL_CONSTASCII_USTRINGPARAM("RedlineAuthor")),
    sRedlineComment(RTL_CONSTASCII_USTRINGPARAM("RedlineComment")),
    sRedlineDateTime(RTL_CONSTASCII_USTRINGPARAM("RedlineDateTime")),
    sRedlineSuccessorData(RTL_CONSTASCII_USTRINGPARAM("RedlineSuccessorData")),
    sRedlineType(RTL_CONSTASCII_USTRINGPARAM("RedlineType")),
    sRedlineText(RTL_CONSTASCII_USTRINGPARAM("RedlineText")),
    sStyle(RTL_CONSTASCII_USTRINGPARAM("Style")),
    sTextTable(RTL_CONSTASCII_USTRINGPARAM("TextTable")),
    sUnknownChange(RTL_CONSTASCII_USTRINGPARAM("UnknownChange")),
    sChangePrefix(RTL_CONSTASCII_USTRINGPARAM("ct")),
    sStartRedline(RTL_CONSTASCII_USTRINGPARAM("StartRedline")),
    sEndRedline(RTL_CONSTASCII_USTRINGPARAM("EndRedline")),
    sRedlineIdentifier(RTL_CONSTASCII_USTRINGPARAM("RedlineIdentifier")),
    sIsInHeaderFooter(RTL_CONSTASCII_USTRINGPARAM("IsInHeaderFooter")),
    rExport(rExp),
    aChangeMap(),
    pCurrentChangesList(NULL)
{
}


XMLRedlineExport::~XMLRedlineExport()
{
    // delete changes lists
    for( ChangesMapType::iterator aIter = aChangeMap.begin();
         aIter != aChangeMap.end();
         aIter++ )
    {
        delete aIter->second;
    }
    aChangeMap.clear();
}


void XMLRedlineExport::ExportChange(
    const Reference<XPropertySet> & rPropSet,
    sal_Bool bAutoStyle)
{
    if (bAutoStyle)
    {
        ExportChangeAutoStyle(rPropSet);
    }
    else
    {
        ExportChangeInline(rPropSet);
    }
}


void XMLRedlineExport::ExportChangesList(sal_Bool bAutoStyles)
{
    if (bAutoStyles)
    {
//      ExportChangesListAutoStyles();
    }
    else
    {
        ExportChangesListElements();
    }
}


void XMLRedlineExport::ExportChangesList(
    const Reference<XText> & rText,
    sal_Bool bAutoStyles)
{
    // do not export any auto styles
    if (bAutoStyles)
        return;

    // look for changes list for this XText
    ChangesMapType::iterator aFind = aChangeMap.find(rText);
    if (aFind != aChangeMap.end())
    {
        ChangesListType* pChangesList = aFind->second;

        // export only if changes are found
        if (pChangesList->size() > 0)
        {
            // changes container element
            SvXMLElementExport aChanges(rExport, XML_NAMESPACE_TEXT,
                                        sXML_tracked_changes,
                                        sal_True, sal_True);

            // iterate over changes list
            for( ChangesListType::iterator aIter = pChangesList->begin();
                 aIter != pChangesList->end();
                 aIter++ )
            {
                ExportChangedRegion( *aIter );
            }
        }
        // else: changes list empty -> ignore
    }
    // else: no changes list found -> empty
}

void XMLRedlineExport::SetCurrentXText(
    const Reference<XText> & rText)
{
    if (rText.is())
    {
        // look for appropriate list in map; use the found one, or create new
        ChangesMapType::iterator aIter = aChangeMap.find(rText);
        if (aIter == aChangeMap.end())
        {
            ChangesListType* pList = new ChangesListType;
            aChangeMap[rText] = pList;
            pCurrentChangesList = pList;
        }
        else
            pCurrentChangesList = aIter->second;
    }
    else
    {
        // don't record changes
        SetCurrentXText();
    }
}

void XMLRedlineExport::SetCurrentXText()
{
    pCurrentChangesList = NULL;
}


void XMLRedlineExport::ExportChangesListElements()
{
    // get redlines (aka tracked changes) from the model
    Reference<XRedlinesSupplier> xSupplier(rExport.GetModel(), uno::UNO_QUERY);
    if (xSupplier.is())
    {
        Reference<XEnumerationAccess> aEnumAccess = xSupplier->getRedlines();

        // only export if we actually have redlines
        if (aEnumAccess->hasElements())
        {
            // changes container element
            SvXMLElementExport aChanges(rExport, XML_NAMESPACE_TEXT,
                                        sXML_tracked_changes,
                                        sal_True, sal_True);

            // get enumeration and iterate over elements
            Reference<XEnumeration> aEnum = aEnumAccess->createEnumeration();
            while (aEnum->hasMoreElements())
            {
                Any aAny = aEnum->nextElement();
                Reference<XPropertySet> xPropSet;
                aAny >>= xPropSet;

                DBG_ASSERT(xPropSet.is(),
                           "can't get XPropertySet; skipping Redline");
                if (xPropSet.is())
                {
                    // export only if not in header or footer
                    // (those must be exported with their XText)
                    aAny = xPropSet->getPropertyValue(sIsInHeaderFooter);
                    if (! *(sal_Bool*)aAny.getValue())
                    {
                        // and finally, export change
                        ExportChangedRegion(xPropSet);
                    }
                }
                // else: no XPropertySet -> no export
            }
        }
        // else: no redlines -> no export
    }
    // else: no XRedlineSupplier -> no export
}

void XMLRedlineExport::ExportChangeAutoStyle(
    const Reference<XPropertySet> & rPropSet)
{
    // record change (if changes should be recorded)
    if (NULL != pCurrentChangesList)
    {
        // put redline in list if it's collapsed or the redline start
        Any aIsStart = rPropSet->getPropertyValue(sIsStart);
        Any aIsCollapsed = rPropSet->getPropertyValue(sIsCollapsed);

        if ( *(sal_Bool*)aIsStart.getValue() ||
             *(sal_Bool*)aIsCollapsed.getValue() )
            pCurrentChangesList->push_back(rPropSet);
    }

    // get XText for export of redline auto styles
    Any aAny = rPropSet->getPropertyValue(sRedlineText);
    Reference<XText> xText;
    aAny >>= xText;
    if (xText.is())
    {
        // export the auto styles
        rExport.GetTextParagraphExport()->collectTextAutoStyles(xText);
    }
}

void XMLRedlineExport::ExportChangesListAutoStyles()
{
    // get redlines (aka tracked changes) from the model
    Reference<XRedlinesSupplier> xSupplier(rExport.GetModel(), uno::UNO_QUERY);
    if (xSupplier.is())
    {
        Reference<XEnumerationAccess> aEnumAccess = xSupplier->getRedlines();

        // only export if we actually have redlines
        if (aEnumAccess->hasElements())
        {
            // get enumeration and iterate over elements
            Reference<XEnumeration> aEnum = aEnumAccess->createEnumeration();
            while (aEnum->hasMoreElements())
            {
                Any aAny = aEnum->nextElement();
                Reference<XPropertySet> xPropSet;
                aAny >>= xPropSet;

                DBG_ASSERT(xPropSet.is(),
                           "can't get XPropertySet; skipping Redline");
                if (xPropSet.is())
                {
                    ExportChangeAutoStyle(xPropSet);
                }
            }
        }
    }
}

void XMLRedlineExport::ExportChangeInline(
    const Reference<XPropertySet> & rPropSet)
{
    // determine element name (depending on collapsed, start/end)
    sal_Char* pElement = NULL;
    Any aAny = rPropSet->getPropertyValue(sIsCollapsed);
    sal_Bool bCollapsed = *(sal_Bool *)aAny.getValue();
    sal_Bool bStart = sal_True; // ignored if bCollapsed = sal_True
    if (bCollapsed)
    {
        pElement = sXML_change;
    }
    else
    {
        aAny = rPropSet->getPropertyValue(sIsStart);
        bStart = *(sal_Bool *)aAny.getValue();
        pElement = bStart ? sXML_change_start : sXML_change_end;
    }

    if (NULL != pElement)
    {
        // we always need the ID
        rExport.AddAttribute(XML_NAMESPACE_TEXT, sXML_change_id,
                             GetRedlineID(rPropSet));

        // export the element (no whitespace because we're in the text body)
        SvXMLElementExport aChangeElem(rExport, XML_NAMESPACE_TEXT,
                                       pElement, sal_False, sal_False);
    }
}


void XMLRedlineExport::ExportChangedRegion(
    const Reference<XPropertySet> & rPropSet)
{
    // export changed-region element (with change-ID)
    rExport.AddAttribute(XML_NAMESPACE_TEXT, sXML_id, GetRedlineID(rPropSet) );
    SvXMLElementExport aChangedRegion(rExport, XML_NAMESPACE_TEXT,
                                      sXML_changed_region, sal_True, sal_True);

    // scope for (first) change element
    {
        Any aAny = rPropSet->getPropertyValue(sRedlineType);
        OUString sType;
        aAny >>= sType;
        SvXMLElementExport aChange(rExport, XML_NAMESPACE_TEXT,
                                   ConvertTypeName(sType), sal_True, sal_True);

        ExportChangeInfo(rPropSet);

        // get XText from the redline and export (if the XText exists)
        aAny = rPropSet->getPropertyValue(sRedlineText);
        Reference<XText> xText;
        aAny >>= xText;
        if (xText.is())
        {
            rExport.GetTextParagraphExport()->exportText(xText);
            // default parameters: bProgress, bExportParagraph ???
        }
        // else: no text interface -> content is inline and will
        //       be exported there
    }

    // changed change? Hierarchical changes can onl be two levels
    // deep. Here we check for the second level.
    Any aAny = rPropSet->getPropertyValue(sRedlineSuccessorData);
    Sequence<PropertyValue> aSuccessorData;
    aAny >>= aSuccessorData;

    // if we actually got a hierarchical change, make element and
    // process change info
    if (aSuccessorData.getLength() > 0)
    {
        // The only change that can be "undone" is an insertion -
        // after all, you can't re-insert an deletion, but you can
        // delete an insertion. This assumption is asserted in
        // ExportChangeInfo(Sequence<PropertyValue>&).
        SvXMLElementExport aSecondChangeElem(
            rExport, XML_NAMESPACE_TEXT, sXML_insertion,
            sal_True, sal_True);

        ExportChangeInfo(aSuccessorData);
    }
    // else: no hierarchical change
}


const OUString XMLRedlineExport::ConvertTypeName(
    const OUString& sApiName)
{
    if (sApiName == sDelete)
    {
        return sDeletion;
    }
    else if (sApiName == sInsert)
    {
        return sInsertion;
    }
    else if (sApiName == sFormat)
    {
        return sFormatChange;
    }
    else
    {
        DBG_ERROR("unknown redline type");
        return sUnknownChange;
    }
}


/** Create a Redline-ID */
const OUString XMLRedlineExport::GetRedlineID(
    const Reference<XPropertySet> & rPropSet)
{
    Any aAny = rPropSet->getPropertyValue(sRedlineIdentifier);
    OUString sTmp;
    aAny >>= sTmp;

    OUStringBuffer sBuf(sChangePrefix);
    sBuf.append(sTmp);
    return sBuf.makeStringAndClear();
}


void XMLRedlineExport::ExportChangeInfo(
    const Reference<XPropertySet> & rPropSet)
{
    Any aAny = rPropSet->getPropertyValue(sRedlineAuthor);
    OUString sTmp;
    aAny >>= sTmp;
    if (sTmp.getLength() > 0)
    {
        rExport.AddAttribute(XML_NAMESPACE_OFFICE, sXML_chg_author, sTmp);
    }

    aAny = rPropSet->getPropertyValue(sRedlineDateTime);
    util::DateTime aDateTime;
    aAny >>= aDateTime;
    OUStringBuffer sBuf;
    rExport.GetMM100UnitConverter().convertDateTime(sBuf, aDateTime);
    rExport.AddAttribute(XML_NAMESPACE_OFFICE, sXML_chg_date_time,
                      sBuf.makeStringAndClear());

    SvXMLElementExport aChangeInfo(rExport, XML_NAMESPACE_OFFICE,
                                   sXML_change_info, sal_True, sal_True);

    // comment as <text:p> sequence
    aAny = rPropSet->getPropertyValue(sRedlineComment);
    aAny >>= sTmp;
    if (sTmp.getLength() > 0)
    {
        // iterate over all string-pieces separated by return (0x0a) and
        // put each inside a paragraph element.
        SvXMLTokenEnumerator aEnumerator(sTmp, sal_Char(0x0a));
        OUString aSubString;
        while (aEnumerator.getNextToken(aSubString))
        {
            SvXMLElementExport aParagraph(
                rExport, XML_NAMESPACE_TEXT, sXML_p, sal_True, sal_False);
            rExport.GetDocHandler()->characters(aSubString);
        }
    }
}

void XMLRedlineExport::ExportChangeInfo(
    const Sequence<PropertyValue> & rPropertyValues)
{
    sal_Int32 nCount = rPropertyValues.getLength();
    for(sal_Int32 i = 0; i < nCount; i++)
    {
        const PropertyValue& rVal = rPropertyValues[i];

        if (rVal.Name.equals(sRedlineAuthor))
        {
            OUString sTmp;
            rVal.Value >>= sTmp;
            if (sTmp.getLength() > 0)
            {
                rExport.AddAttribute(XML_NAMESPACE_OFFICE, sXML_chg_author,
                                     sTmp);
            }
        }
        else if (rVal.Name.equals(sRedlineComment))
        {
            OUString sTmp;
            rVal.Value >>= sTmp;
            if (sTmp.getLength() > 0)
            {
                rExport.AddAttribute(XML_NAMESPACE_OFFICE, sXML_chg_comment,
                                     sTmp);
            }
        }
        else if (rVal.Name.equals(sRedlineDateTime))
        {
            util::DateTime aDateTime;
            rVal.Value >>= aDateTime;
            OUStringBuffer sBuf;
            rExport.GetMM100UnitConverter().convertDateTime(sBuf, aDateTime);
            rExport.AddAttribute(XML_NAMESPACE_OFFICE, sXML_chg_date_time,
                                 sBuf.makeStringAndClear());
        }
        else if (rVal.Name.equals(sRedlineType))
        {
            // check if this is an insertion; cf. comment at calling location
            OUString sTmp;
            rVal.Value >>= sTmp;
            DBG_ASSERT(sTmp.equals(sInsert),
                       "hierarchical change must be insertion");
        }
        // else: unknown value -> ignore
    }

    // finally write element
    SvXMLElementExport aChangeInfo(rExport, XML_NAMESPACE_OFFICE,
                                   sXML_change_info, sal_True, sal_True);
}

void XMLRedlineExport::ExportStartOrEndRedline(
    const Reference<XPropertySet> & rPropSet,
    sal_Bool bStart)
{
    // get appropriate (start or end) property
    Any aAny =
        rPropSet->getPropertyValue(bStart ? sStartRedline : sEndRedline);
    Sequence<PropertyValue> aValues;
    aAny >>= aValues;

    // seek for redline ID
    sal_Int32 nLength = aValues.getLength();
    for(sal_Int32 i = 0; i < nLength; i++)
    {
        if (sRedlineIdentifier.equals(aValues[i].Name))
        {
            OUString sId;
            aValues[i].Value >>= sId;

            // TODO: use GetRedlineID or elimiate that function
            OUStringBuffer sBuffer(sChangePrefix);
            sBuffer.append(sId);

            rExport.AddAttribute(XML_NAMESPACE_TEXT, sXML_change_id,
                                 sBuffer.makeStringAndClear());

            // export the element
            // (whitespace because we're not inside paragraphs)
            SvXMLElementExport aChangeElem(
                rExport, XML_NAMESPACE_TEXT,
                bStart ? sXML_change_start : sXML_change_end,
                sal_True, sal_True);

            // and break out of loop, in case a second RedlineIdentifier Value
            break;
        }
        // else: ignore Value
    }
}

void XMLRedlineExport::ExportStartOrEndRedline(
    const Reference<XTextContent> & rContent,
    sal_Bool bStart)
{
    Reference<XPropertySet> xPropSet(rContent, uno::UNO_QUERY);
    if (xPropSet.is())
    {
        ExportStartOrEndRedline(xPropSet, bStart);
    }
    else
    {
        DBG_ERROR("XPropertySet expected");
    }
}

void XMLRedlineExport::ExportStartOrEndRedline(
    const Reference<XTextSection> & rSection,
    sal_Bool bStart)
{
    Reference<XPropertySet> xPropSet(rSection, uno::UNO_QUERY);
    if (xPropSet.is())
    {
        ExportStartOrEndRedline(xPropSet, bStart);
    }
    else
    {
        DBG_ERROR("XPropertySet expected");
    }
}



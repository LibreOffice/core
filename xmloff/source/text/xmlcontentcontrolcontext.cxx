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

#include "xmlcontentcontrolcontext.hxx"

#include <com/sun/star/beans/XPropertySet.hpp>

#include <sax/tools/converter.hxx>
#include <xmloff/xmlimp.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmltoken.hxx>
#include <comphelper/propertyvalue.hxx>
#include <comphelper/sequence.hxx>

#include "XMLTextMarkImportContext.hxx"
#include "txtparai.hxx"

using namespace com::sun::star;
using namespace xmloff::token;

XMLContentControlContext::XMLContentControlContext(SvXMLImport& rImport, sal_Int32 /*nElement*/,
                                                   XMLHints_Impl& rHints, bool& rIgnoreLeadingSpace)
    : SvXMLImportContext(rImport)
    , m_rHints(rHints)
    , m_rIgnoreLeadingSpace(rIgnoreLeadingSpace)
    , m_xStart(GetImport().GetTextImport()->GetCursorAsRange()->getStart())
{
}

void XMLContentControlContext::startFastElement(
    sal_Int32 /*nElement*/, const uno::Reference<xml::sax::XFastAttributeList>& xAttrList)
{
    for (auto& rIter : sax_fastparser::castToFastAttributeList(xAttrList))
    {
        bool bTmp = false;

        switch (rIter.getToken())
        {
            case XML_ELEMENT(LO_EXT, XML_SHOWING_PLACE_HOLDER):
            {
                if (sax::Converter::convertBool(bTmp, rIter.toView()))
                {
                    m_bShowingPlaceHolder = bTmp;
                }
                break;
            }
            case XML_ELEMENT(LO_EXT, XML_CHECKBOX):
            {
                if (sax::Converter::convertBool(bTmp, rIter.toView()))
                {
                    m_bCheckbox = bTmp;
                }
                break;
            }
            case XML_ELEMENT(LO_EXT, XML_CHECKED):
            {
                if (sax::Converter::convertBool(bTmp, rIter.toView()))
                {
                    m_bChecked = bTmp;
                }
                break;
            }
            case XML_ELEMENT(LO_EXT, XML_CHECKED_STATE):
            {
                m_aCheckedState = rIter.toString();
                break;
            }
            case XML_ELEMENT(LO_EXT, XML_UNCHECKED_STATE):
            {
                m_aUncheckedState = rIter.toString();
                break;
            }
            case XML_ELEMENT(LO_EXT, XML_PICTURE):
            {
                if (sax::Converter::convertBool(bTmp, rIter.toView()))
                {
                    m_bPicture = bTmp;
                }
                break;
            }
            case XML_ELEMENT(LO_EXT, XML_DATE):
            {
                if (sax::Converter::convertBool(bTmp, rIter.toView()))
                {
                    m_bDate = bTmp;
                }
                break;
            }
            case XML_ELEMENT(LO_EXT, XML_DATE_FORMAT):
            {
                m_aDateFormat = rIter.toString();
                break;
            }
            case XML_ELEMENT(LO_EXT, XML_DATE_RFC_LANGUAGE_TAG):
            {
                m_aDateLanguage = rIter.toString();
                break;
            }
            case XML_ELEMENT(LO_EXT, XML_CURRENT_DATE):
            {
                m_aCurrentDate = rIter.toString();
                break;
            }
            default:
                XMLOFF_WARN_UNKNOWN("xmloff", rIter);
        }
    }
}

void XMLContentControlContext::endFastElement(sal_Int32)
{
    if (!m_xStart.is())
    {
        SAL_WARN("xmloff.text", "XMLContentControlContext::endFastElement: no m_xStart");
        return;
    }

    uno::Reference<text::XTextRange> xEndRange
        = GetImport().GetTextImport()->GetCursorAsRange()->getStart();

    // Create range for insertion.
    uno::Reference<text::XTextCursor> xInsertionCursor
        = GetImport().GetTextImport()->GetText()->createTextCursorByRange(xEndRange);
    xInsertionCursor->gotoRange(m_xStart, /*bExpand=*/true);

    uno::Reference<text::XTextContent> xContentControl
        = XMLTextMarkImportContext::CreateAndInsertMark(
            GetImport(), "com.sun.star.text.ContentControl", OUString(), xInsertionCursor);
    if (!xContentControl.is())
    {
        SAL_WARN("xmloff.text", "cannot insert content control");
        return;
    }

    uno::Reference<beans::XPropertySet> xPropertySet(xContentControl, uno::UNO_QUERY);
    if (!xPropertySet.is())
    {
        return;
    }

    if (m_bShowingPlaceHolder)
    {
        xPropertySet->setPropertyValue("ShowingPlaceHolder", uno::Any(m_bShowingPlaceHolder));
    }

    if (m_bCheckbox)
    {
        xPropertySet->setPropertyValue("Checkbox", uno::Any(m_bCheckbox));
    }
    if (m_bChecked)
    {
        xPropertySet->setPropertyValue("Checked", uno::Any(m_bChecked));
    }
    if (!m_aCheckedState.isEmpty())
    {
        xPropertySet->setPropertyValue("CheckedState", uno::Any(m_aCheckedState));
    }
    if (!m_aUncheckedState.isEmpty())
    {
        xPropertySet->setPropertyValue("UncheckedState", uno::Any(m_aUncheckedState));
    }
    if (!m_aListItems.empty())
    {
        xPropertySet->setPropertyValue("ListItems",
                                       uno::Any(comphelper::containerToSequence(m_aListItems)));
    }

    if (m_bPicture)
    {
        xPropertySet->setPropertyValue("Picture", uno::Any(m_bPicture));
    }

    if (m_bDate)
    {
        xPropertySet->setPropertyValue("Date", uno::Any(m_bDate));
    }
    if (!m_aDateFormat.isEmpty())
    {
        xPropertySet->setPropertyValue("DateFormat", uno::Any(m_aDateFormat));
    }
    if (!m_aDateLanguage.isEmpty())
    {
        xPropertySet->setPropertyValue("DateLanguage", uno::Any(m_aDateLanguage));
    }
    if (!m_aCurrentDate.isEmpty())
    {
        xPropertySet->setPropertyValue("CurrentDate", uno::Any(m_aCurrentDate));
    }
}

css::uno::Reference<css::xml::sax::XFastContextHandler>
XMLContentControlContext::createFastChildContext(
    sal_Int32 nElement, const uno::Reference<xml::sax::XFastAttributeList>& xAttrList)
{
    switch (nElement)
    {
        case XML_ELEMENT(LO_EXT, XML_LIST_ITEM):
            return new XMLListItemContext(GetImport(), *this);
            break;
        default:
            break;
    }

    return XMLImpSpanContext_Impl::CreateSpanContext(GetImport(), nElement, xAttrList, m_rHints,
                                                     m_rIgnoreLeadingSpace);
}

void XMLContentControlContext::characters(const OUString& rChars)
{
    GetImport().GetTextImport()->InsertString(rChars, m_rIgnoreLeadingSpace);
}

void XMLContentControlContext::AppendListItem(const css::beans::PropertyValues& rListItem)
{
    m_aListItems.push_back(rListItem);
}

XMLListItemContext::XMLListItemContext(SvXMLImport& rImport,
                                       XMLContentControlContext& rContentControl)
    : SvXMLImportContext(rImport)
    , m_rContentControl(rContentControl)
{
}

void XMLListItemContext::startFastElement(
    sal_Int32 /*nElement*/, const uno::Reference<xml::sax::XFastAttributeList>& xAttrList)
{
    OUString aDisplayText;
    OUString aValue;

    for (auto& rIter : sax_fastparser::castToFastAttributeList(xAttrList))
    {
        switch (rIter.getToken())
        {
            case XML_ELEMENT(LO_EXT, XML_DISPLAY_TEXT):
            {
                aDisplayText = rIter.toString();
                break;
            }
            case XML_ELEMENT(LO_EXT, XML_VALUE):
            {
                aValue = rIter.toString();
                break;
            }
            default:
                XMLOFF_WARN_UNKNOWN("xmloff", rIter);
        }
    }

    uno::Sequence<beans::PropertyValue> aListItem = {
        comphelper::makePropertyValue("DisplayText", uno::Any(aDisplayText)),
        comphelper::makePropertyValue("Value", uno::Any(aValue)),
    };
    m_rContentControl.AppendListItem(aListItem);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

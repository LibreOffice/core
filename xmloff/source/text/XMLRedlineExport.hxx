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

#ifndef INCLUDED_XMLOFF_SOURCE_TEXT_XMLREDLINEEXPORT_HXX
#define INCLUDED_XMLOFF_SOURCE_TEXT_XMLREDLINEEXPORT_HXX

#include <rtl/ustrbuf.hxx>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/Sequence.h>

#include <list>
#include <map>

class SvXMLExport;
namespace com { namespace sun { namespace star {
    namespace beans { class XPropertySet; }
    namespace beans { struct PropertyValue; }
    namespace text { class XText; }
    namespace text { class XTextContent; }
    namespace text { class XTextSection; }
 } } }

// store a list of redline properties
typedef ::std::list<
            css::uno::Reference<css::beans::XPropertySet> > ChangesListType;

// store a list of redline properties for each XText
typedef ::std::map<
            css::uno::Reference< css::text::XText>,
            ChangesListType* > ChangesMapType;


/**
 * This class handles the export of redline portions.
 * It is to be called from XMLTextParagraphExport.
 */
class XMLRedlineExport
{
    const OUString sDelete;
    const OUString sDeletion;
    const OUString sFormat;
    const OUString sFormatChange;
    const OUString sInsert;
    const OUString sInsertion;
    const OUString sIsCollapsed;
    const OUString sIsStart;
    const OUString sRedlineAuthor;
    const OUString sRedlineComment;
    const OUString sRedlineDateTime;
    const OUString sRedlineSuccessorData;
    const OUString sRedlineText;
    const OUString sRedlineType;
    const OUString sUnknownChange;
    const OUString sStartRedline;
    const OUString sEndRedline;
    const OUString sRedlineIdentifier;
    const OUString sIsInHeaderFooter;
    const OUString sRecordChanges;
    const OUString sMergeLastPara;

    const OUString sChangePrefix;

    SvXMLExport& rExport;


    // handling of change recording:

    // To support change tracking in headers and footers we need to
    // write these changes separately. To do this, we record the
    // changes for headers and footers. For the main document body, we
    // get the complete list of changes from the document, which
    // should be much more efficient than recording all of those.

    ChangesMapType aChangeMap;              /// map of recorded changes

    /// list of current changes; is NULL or points to member of aChangeMap
    ChangesListType* pCurrentChangesList;


public:
    explicit XMLRedlineExport(SvXMLExport& rExp);

    ~XMLRedlineExport();

    /// export a change
    void ExportChange(
        /// PropertySet of RedlinePortion
        const css::uno::Reference<css::beans::XPropertySet> & rPropSet,
        bool bAutoStyle);

    /// export the list of changes (complete list minus recorded changed)
    void ExportChangesList(bool bAutoStyles);

    /// export the list of changes (recorded changes for this XText only)
    void ExportChangesList(
        const css::uno::Reference<css::text::XText> & rText,
        bool bAutoStyles);

    /// set the current XText for which changes should be recorded.
    /// An empty XText means: don't record changes
    void SetCurrentXText(
        const css::uno::Reference<css::text::XText> & rText);

    /// Do not record changes.
    /// Same as SetCurrentXText(Reference<XText>) with empty argument.
    void SetCurrentXText();

    /// export redline marks which start or end at start nodes,
    /// i.e. that include the complete paragraph/table/section
    void ExportStartOrEndRedline(
        const css::uno::Reference<
                    css::beans::XPropertySet> & rPropSet,
        bool bStart);   /// start or end of text entity (e.g. paragraph)?

    /// convenience method, calls XPropertySet-version of this method
    void ExportStartOrEndRedline(
        /// XTextContent; must also be an XPropertySet
        const css::uno::Reference<css::text::XTextContent> & rContent,
        bool bStart);

    /// convenience method, calls XPropertySet-version of this method
    void ExportStartOrEndRedline(
        /// XTextSection; must also be an XPropertySet
        const css::uno::Reference<css::text::XTextSection> & rSection,
        bool bStart);

private:

    /// export the change mark contained in the text body
    void ExportChangeInline(
        /// PropertySet of RedlinePortion
        const css::uno::Reference<css::beans::XPropertySet> & rPropSet);

    /// export the auto styles used in this change
    void ExportChangeAutoStyle(
        /// PropertySet of RedlinePortion
        const css::uno::Reference<css::beans::XPropertySet> & rPropSet);

    /// export the changes list (<text:tracked-changes>)
    void ExportChangesListElements();

    /// export the auto styles needed by the changes list
    void ExportChangesListAutoStyles();

    /// export the changed-region element
    void ExportChangedRegion(
        const css::uno::Reference<css::beans::XPropertySet> & rPropSet);

    /// export an change-info element (from a PropertySet)
    void ExportChangeInfo(
        const css::uno::Reference<css::beans::XPropertySet> & rPropSet);

    /// export an change-info element (from PropertyValues)
    void ExportChangeInfo(
        const css::uno::Sequence<css::beans::PropertyValue> & rValues);

    /// convert the change type from API to XML names
    OUString const & ConvertTypeName(const OUString& sApiName);

    /// Get ID string!
    const OUString GetRedlineID(
        const css::uno::Reference<css::beans::XPropertySet> & rPropSet);

    /// write a comment string as sequence of <text:p> elements
    void WriteComment(const OUString& rComment);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

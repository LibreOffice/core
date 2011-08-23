/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _XMLOFF_XMLREDLINEEXPORT_HXX
#define _XMLOFF_XMLREDLINEEXPORT_HXX

#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/Sequence.h>

#include <list>
#include <map>
#include <set>

namespace com { namespace sun { namespace star { 
    namespace beans { class XPropertySet; }
    namespace beans { struct PropertyValue; }
    namespace text { class XText; }
    namespace text { class XTextContent; }
    namespace text { class XTextSection; }
 } } }
namespace rtl {
    class OUString;
    class OUStringBuffer;
}
namespace binfilter {
class SvXMLExport;

// store a list of redline properties
typedef ::std::list<
            ::com::sun::star::uno::Reference<
                ::com::sun::star::beans::XPropertySet> > ChangesListType;

// store a list of redline properties for each XText
typedef ::std::map<
            ::com::sun::star::uno::Reference< ::com::sun::star::text::XText>,
            ChangesListType* > ChangesMapType;


/** 
 * This class handles the export of redline portions.
 * It is to be called from XMLTextParagraphExport.
 */
class XMLRedlineExport
{
    const ::rtl::OUString sDelete;
    const ::rtl::OUString sDeletion;
    const ::rtl::OUString sFormat;
    const ::rtl::OUString sFormatChange;
    const ::rtl::OUString sInsert;
    const ::rtl::OUString sInsertion;
    const ::rtl::OUString sIsCollapsed;
    const ::rtl::OUString sIsStart;
    const ::rtl::OUString sRedlineAuthor;
    const ::rtl::OUString sRedlineComment;
    const ::rtl::OUString sRedlineDateTime;
    const ::rtl::OUString sRedlineSuccessorData;
    const ::rtl::OUString sRedlineText;
    const ::rtl::OUString sRedlineType;
    const ::rtl::OUString sStyle;
    const ::rtl::OUString sTextTable;
    const ::rtl::OUString sUnknownChange;
    const ::rtl::OUString sStartRedline;
    const ::rtl::OUString sEndRedline;
    const ::rtl::OUString sRedlineIdentifier;
    const ::rtl::OUString sIsInHeaderFooter;
    const ::rtl::OUString sRedlineProtectionKey;
    const ::rtl::OUString sRecordChanges;
    const ::rtl::OUString sMergeLastPara;

    const ::rtl::OUString sChangePrefix;

    SvXMLExport& rExport;


    // handling of change recording: 

    // To support change tracking in headers and footers we need to
    // write these changes separately. To do this, we record the
    // changes for headers and footers. For the main document body, we
    // get the complete list of changes from the document, which
    // should be much more efficient than recording all of those.

    ChangesMapType aChangeMap;				/// map of recorded changes

    /// list of current changes; is NULL or points to member of aChangeMap
    ChangesListType* pCurrentChangesList;	


public:
    XMLRedlineExport(SvXMLExport& rExp);

    ~XMLRedlineExport();

    /// export a change 
    void ExportChange(
        /// PropertySet of RedlinePortion
        const ::com::sun::star::uno::Reference<
                    ::com::sun::star::beans::XPropertySet> & rPropSet,
        sal_Bool bAutoStyle);

    /// export the list of changes (complete list minus recorded changed)
    void ExportChangesList(sal_Bool bAutoStyles);

    /// export the list of changes (recorded changes for this XText only)
    void ExportChangesList(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::text::XText> & rText,
        sal_Bool bAutoStyles);

    /// set the current XText for which changes should be recorded.
    /// An empty XText means: don't record changes
    void SetCurrentXText(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::text::XText> & rText);

    /// Do not record changes. 
    /// Same as SetCurrentXText(Reference<XText>) with empty argument.
    void SetCurrentXText();

    /// export redline marks which start or end at start nodes,
    /// i.e. that include the complete paragraph/table/section
    void ExportStartOrEndRedline(
        const ::com::sun::star::uno::Reference<
                    ::com::sun::star::beans::XPropertySet> & rPropSet,
        sal_Bool bStart);	/// start or end of text entity (e.g. paragraph)?

    /// convenience method, calls XPropertySet-version of this method
    void ExportStartOrEndRedline(
        /// XTextContent; must also be an XPropertySet
        const ::com::sun::star::uno::Reference<
                    ::com::sun::star::text::XTextContent> & rContent,
        sal_Bool bStart);

    /// convenience method, calls XPropertySet-version of this method
    void ExportStartOrEndRedline(
        /// XTextSection; must also be an XPropertySet
        const ::com::sun::star::uno::Reference<
                    ::com::sun::star::text::XTextSection> & rSection,
        sal_Bool bStart);

private:
    
    /// export the change mark contained in the text body
    void ExportChangeInline(
        /// PropertySet of RedlinePortion
        const ::com::sun::star::uno::Reference<
                    ::com::sun::star::beans::XPropertySet> & rPropSet);

    /// export the auto styles used in this change
    void ExportChangeAutoStyle(
        /// PropertySet of RedlinePortion
        const ::com::sun::star::uno::Reference<
                    ::com::sun::star::beans::XPropertySet> & rPropSet);

    /// export the changes list (<text:tracked-changes>)
    void ExportChangesListElements();

    /// export the auto styles needed by the changes list
    void ExportChangesListAutoStyles();

    /// export the changed-region element
    void ExportChangedRegion(
        const ::com::sun::star::uno::Reference<
                    ::com::sun::star::beans::XPropertySet> & rPropSet);

    /// export an change-info element (from a PropertySet)
    void ExportChangeInfo(
        const ::com::sun::star::uno::Reference<
                    ::com::sun::star::beans::XPropertySet> & rPropSet);

    /// export an change-info element (from PropertyValues)
    void ExportChangeInfo(
        const ::com::sun::star::uno::Sequence<
                    ::com::sun::star::beans::PropertyValue> & rValues);

    /// convert the change type from API to XML names
    const ::rtl::OUString ConvertTypeName(const ::rtl::OUString& sApiName);

    /// Get ID string!
    const ::rtl::OUString GetRedlineID(
        const ::com::sun::star::uno::Reference<
                    ::com::sun::star::beans::XPropertySet> & rPropSet);

    /// write a comment string as sequence of <text:p> elements
    void WriteComment(const ::rtl::OUString& rComment);
};

}//end of namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

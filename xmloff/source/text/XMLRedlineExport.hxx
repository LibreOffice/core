/*************************************************************************
 *
 *  $RCSfile: XMLRedlineExport.hxx,v $
 *
 *  $Revision: 1.6 $
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
#define _XMLOFF_XMLREDLINEEXPORT_HXX

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_H_
#include <com/sun/star/uno/Sequence.h>
#endif

#include <list>
#include <map>
#include <set>

class SvXMLExport;
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


    const ::rtl::OUString sChangePrefix;

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
        sal_Bool bStart);   /// start or end of text entity (e.g. paragraph)?

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

};

#endif

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

#ifndef _XMLOFF_XMLINDEXMARKEXPORT_HXX_
#define _XMLOFF_XMLINDEXMARKEXPORT_HXX_

#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_ 
#include <com/sun/star/uno/Reference.h>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_H_ 
#include <com/sun/star/uno/Sequence.h>
#endif

namespace com { namespace sun { namespace star { 
    namespace beans { class XPropertySet; }
 } } }
namespace rtl {
    class OUString;
    class OUStringBuffer;
}
namespace binfilter {
class SvXMLExport;
class XMLTextParagraphExport;


/** 
 * This class handles the export of index marks for table of content,
 * alphabetical and user index. 
 *
 * Marks for bibliography indices are internally modelled as text
 * fields and thus handled in txtparae.cxx
 */
class XMLIndexMarkExport
{
    ::rtl::OUString sLevel;
    ::rtl::OUString sUserIndexName;
    ::rtl::OUString sPrimaryKey;
    ::rtl::OUString sSecondaryKey;
    ::rtl::OUString sDocumentIndexMark;
    ::rtl::OUString sIsStart;
    ::rtl::OUString sIsCollapsed;
    ::rtl::OUString sAlternativeText;
    ::rtl::OUString sTextReading;
    ::rtl::OUString sPrimaryKeyReading;
    ::rtl::OUString sSecondaryKeyReading;
    ::rtl::OUString sMainEntry;


    SvXMLExport& rExport;
    XMLTextParagraphExport& rParaExport;

public:
    XMLIndexMarkExport(SvXMLExport& rExp,
                       XMLTextParagraphExport& rParaExp);

    ~XMLIndexMarkExport();

    /**
     * export by the property set of its *text* *portion*.
     *
     * The text portion supplies us with the properties of the index
     * mark itself, as well as the information whether we are at the
     * start or end of an index mark, or whether the index mark is
     * collapsed.
     */
    void ExportIndexMark(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet> & rPropSet,
        sal_Bool bAutoStyles);

protected:

    /// export attributes of table-of-content index marks
    void ExportTOCMarkAttributes(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet> & rPropSet);

    /// export attributes of user index marks
    void ExportUserIndexMarkAttributes(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet> & rPropSet);

    /// export attributes of alphabetical  index marks
    void ExportAlphabeticalIndexMarkAttributes(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet> & rPropSet);

    /// create a numerical ID for this index mark 
    /// (represented by its properties)
    void GetID(
        ::rtl::OUStringBuffer& sBuffer,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet> & rPropSet);

};

}//end of namespace binfilter
#endif

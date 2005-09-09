/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: XMLIndexMarkExport.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 15:06:39 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _XMLOFF_XMLINDEXMARKEXPORT_HXX_
#define _XMLOFF_XMLINDEXMARKEXPORT_HXX_

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_H_
#include <com/sun/star/uno/Sequence.h>
#endif

class SvXMLExport;
class XMLTextParagraphExport;
namespace com { namespace sun { namespace star {
    namespace beans { class XPropertySet; }
 } } }
namespace rtl {
    class OUString;
    class OUStringBuffer;
}


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

#endif

/*************************************************************************
 *
 *  $RCSfile: XMLIndexMarkExport.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-27 18:20:38 $
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

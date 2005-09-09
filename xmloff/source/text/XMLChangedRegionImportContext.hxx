/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: XMLChangedRegionImportContext.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 15:00:49 $
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


#ifndef _XMLOFF_XMLCHANGEDREGIONIMPORTCONTEXT_HXX
#define _XMLOFF_XMLCHANGEDREGIONIMPORTCONTEXT_HXX

#ifndef _XMLOFF_XMLICTXT_HXX
#include "xmlictxt.hxx"
#endif

#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif


namespace com { namespace sun { namespace star {
    namespace text {
        class XTextCursor;
    }
    namespace xml { namespace sax {
        class XAttributeList;
    } }
} } }
namespace rtl {
    class OUString;
}


class XMLChangedRegionImportContext : public SvXMLImportContext
{
    const ::rtl::OUString sEmpty;

    /// if we replace the current XTextCursor/XText by the ones for
    /// the redline, we remeber the old cursor here.
    ::com::sun::star::uno::Reference<
        ::com::sun::star::text::XTextCursor> xOldCursor;

    ::rtl::OUString sID;        /// redline-ID

    sal_Bool bMergeLastPara;    /// merge-last-paragraph flag

public:

    TYPEINFO();

    XMLChangedRegionImportContext(
        SvXMLImport& rImport,
        sal_uInt16 nPrefix,
        const ::rtl::OUString& rLocalName);

    ~XMLChangedRegionImportContext();

    virtual void StartElement(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XAttributeList> & xAttrList);

    virtual SvXMLImportContext* CreateChildContext(
        sal_uInt16 nPrefix,
        const ::rtl::OUString& rLocalName,
         const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XAttributeList> & xAttrList);

    virtual void EndElement();


    /// change info: To be called from change-info context
    void SetChangeInfo(const ::rtl::OUString& rType,
                       const ::rtl::OUString& rAuthor,
                       const ::rtl::OUString& rComment,
                       const ::rtl::OUString& rDate);

    /// create redline XText/XTextCursor on demand and register with
    /// XMLTextImportHelper
    void UseRedlineText();
};


#endif

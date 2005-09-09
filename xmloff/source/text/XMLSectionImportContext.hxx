/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: XMLSectionImportContext.hxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 15:17:48 $
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

#ifndef _XMLOFF_XMLSECTIONIMPORTCONTEXT_HXX_
#define _XMLOFF_XMLSECTIONIMPORTCONTEXT_HXX_

#ifndef _XMLOFF_XMLICTXT_HXX
#include "xmlictxt.hxx"
#endif

#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_H_
#include <com/sun/star/uno/Sequence.h>
#endif

namespace com { namespace sun { namespace star {
    namespace text { class XTextRange;  }
    namespace beans { class XPropertySet; }
    namespace xml { namespace sax { class XAttributeList; } }
} } }
namespace rtl { class OUString; }
class XMLTextImportHelper;


/**
 * Import text sections.
 *
 * This context may *also* be used for index header sections. The
 * differentiates its behaviour based on GetLocalName().
 */
class XMLSectionImportContext : public SvXMLImportContext
{
    /// start position; ranges aquired via getStart(),getEnd() don't move
    ::com::sun::star::uno::Reference<
        ::com::sun::star::text::XTextRange> xStartRange;

    /// end position
    ::com::sun::star::uno::Reference<
        ::com::sun::star::text::XTextRange> xEndRange;

    /// TextSection (as XPropertySet) for passing down to data source elements
    ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet> xSectionPropertySet;

    const ::rtl::OUString sTextSection;
    const ::rtl::OUString sIndexHeaderSection;
    const ::rtl::OUString sCondition;
    const ::rtl::OUString sIsVisible;
    const ::rtl::OUString sProtectionKey;
    const ::rtl::OUString sIsProtected;
    const ::rtl::OUString sIsCurrentlyVisible;
    const ::rtl::OUString sEmpty;

    ::rtl::OUString sStyleName;
    ::rtl::OUString sName;
    ::rtl::OUString sCond;
    ::com::sun::star::uno::Sequence<sal_Int8> aSequence;
    sal_Bool bProtect;
    sal_Bool bCondOK;
    sal_Bool bIsVisible;
    sal_Bool bValid;
    sal_Bool bSequenceOK;
    sal_Bool bIsCurrentlyVisible;
    sal_Bool bIsCurrentlyVisibleOK;

    sal_Bool bHasContent;

public:

    TYPEINFO();

    XMLSectionImportContext(
        SvXMLImport& rImport,
        sal_uInt16 nPrfx,
        const ::rtl::OUString& rLocalName );

    ~XMLSectionImportContext();

protected:

    virtual void StartElement(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XAttributeList> & xAttrList);

    virtual void EndElement();

    virtual SvXMLImportContext *CreateChildContext(
        sal_uInt16 nPrefix,
        const ::rtl::OUString& rLocalName,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XAttributeList> & xAttrList );

    void ProcessAttributes(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XAttributeList> & xAttrList );
};

#endif

/*************************************************************************
 *
 *  $RCSfile: XMLSectionImportContext.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: dvo $ $Date: 2001-03-21 16:03:49 $
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
    const ::rtl::OUString sPassword;
    const ::rtl::OUString sIsProtected;
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

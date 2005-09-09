/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: XMLIndexTitleTemplateContext.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 15:12:52 $
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

#ifndef _XMLOFF_XMLINDEXTITLETEMPLATECONTEXT_HXX_
#define _XMLOFF_XMLINDEXTITLETEMPLATECONTEXT_HXX_

#ifndef _XMLOFF_XMLICTXT_HXX
#include "xmlictxt.hxx"
#endif

#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif

#ifndef _RTL_USTRING_
#include <rtl/ustring>
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif


namespace com { namespace sun { namespace star {
    namespace beans { class XPropertySet; }
    namespace xml { namespace sax { class XAttributeList; } }
} } }


/**
 * Import index title templates
 */
class XMLIndexTitleTemplateContext : public SvXMLImportContext
{

    const ::rtl::OUString sTitle;
    const ::rtl::OUString sParaStyleHeading;

    // paragraph style
    ::rtl::OUString sStyleName;
    sal_Bool bStyleNameOK;

    // content
    ::rtl::OUStringBuffer sContent;

    // TOC property set
    ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet> & rTOCPropertySet;

public:

    TYPEINFO();

     XMLIndexTitleTemplateContext(
        SvXMLImport& rImport,
        ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet> & rPropSet,
        sal_uInt16 nPrfx,
        const ::rtl::OUString& rLocalName);

    ~XMLIndexTitleTemplateContext();

protected:

    /** process parameters */
    virtual void StartElement(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XAttributeList> & xAttrList);

    /** set values */
    virtual void EndElement();

    /** pick up title characters */
    virtual void Characters(const ::rtl::OUString& sString);
};

#endif

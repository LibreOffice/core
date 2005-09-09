/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: XMLIndexTOCContext.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 15:09:15 $
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

#ifndef _XMLOFF_XMLINDEXTOCCONTEXT_HXX_
#define _XMLOFF_XMLINDEXTOCCONTEXT_HXX_

#ifndef _XMLOFF_XMLICTXT_HXX
#include "xmlictxt.hxx"
#endif

#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif


namespace com { namespace sun { namespace star {
    namespace xml { namespace sax { class XAttributeList; } }
    namespace beans { class XPropertySet; }
} } }
namespace rtl { class OUString; }


enum IndexTypeEnum
{
    TEXT_INDEX_TOC,
    TEXT_INDEX_ALPHABETICAL,
    TEXT_INDEX_TABLE,
    TEXT_INDEX_OBJECT,
    TEXT_INDEX_BIBLIOGRAPHY,
    TEXT_INDEX_USER,
    TEXT_INDEX_ILLUSTRATION,

    TEXT_INDEX_UNKNOWN
};


/**
 * Import all indices.
 *
 * Originally, this class would import only the TOC (table of
 * content), but now it's role has been expanded to handle all
 * indices, and hence is named inappropriately. Depending on the
 * element name it decides which index source element context to create.
 */
class XMLIndexTOCContext : public SvXMLImportContext
{
    const ::rtl::OUString sTitle;
    const ::rtl::OUString sIsProtected;
    const ::rtl::OUString sName;

    /** XPropertySet of the index */
    ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet> xTOCPropertySet;

    enum IndexTypeEnum eIndexType;

    /** source element name (for CreateChildContext) */
    const sal_Char* pSourceElementName;

    sal_Bool bValid;

    SvXMLImportContextRef xBodyContextRef;

public:

    TYPEINFO();

    XMLIndexTOCContext(
        SvXMLImport& rImport,
        sal_uInt16 nPrfx,
        const ::rtl::OUString& rLocalName );

    ~XMLIndexTOCContext();

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
};

#endif

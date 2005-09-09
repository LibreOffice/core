/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: XMLChangeImportContext.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 14:59:47 $
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

#ifndef _XMLOFF_XMLCHANGEIMPORTCONTEXT_HXX
#define _XMLOFF_XMLCHANGEIMPORTCONTEXT_HXX


#ifndef _XMLOFF_XMLICTXT_HXX
#include "xmlictxt.hxx"
#endif

#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif


namespace com { namespace sun { namespace star {
    namespace xml { namespace sax {
        class XAttributeList;
    } }
} } }
namespace rtl {
    class OUString;
}



/**
 * import change tracking/redlining markers
 * <text:change>, <text:change-start>, <text:change-end>
 */
class XMLChangeImportContext : public SvXMLImportContext
{
    sal_Bool bIsStart;
    sal_Bool bIsEnd;
    sal_Bool bIsOutsideOfParagraph;

public:

    TYPEINFO();

    /**
     * import a change mark
     * (<text:change>, <text:change-start>, <text:change-end>)
     * Note: a <text:change> mark denotes start and end of a change
     * simultaniously, so both bIsStart and bIsEnd parameters would
     * be set true.
     */
    XMLChangeImportContext(
        SvXMLImport& rImport,
        sal_Int16 nPrefix,
        const ::rtl::OUString& rLocalName,
        sal_Bool bIsStart,  /// mark start of a change
        sal_Bool bIsEnd,    /// mark end of a change
        /// true if change mark is encountered outside of a paragraph
        /// (usually before a section or table)
        sal_Bool bIsOutsideOfParagraph = sal_False);

    ~XMLChangeImportContext();

    virtual void StartElement(
        const ::com::sun::star::uno::Reference<
                ::com::sun::star::xml::sax::XAttributeList> & xAttrList);
};

#endif

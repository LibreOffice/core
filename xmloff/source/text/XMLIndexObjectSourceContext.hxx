/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: XMLIndexObjectSourceContext.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 15:07:06 $
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

#ifndef _XMLOFF_XMLINDEXOBJECTSOURCECONTEXT_HXX_
#define _XMLOFF_XMLINDEXOBJECTSOURCECONTEXT_HXX_

#ifndef _XMLOFF_XMLINDEXSOURCEBASECONTEXT_HXX_
#include "XMLIndexSourceBaseContext.hxx"
#endif

#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif


namespace com { namespace sun { namespace star {
    namespace xml { namespace sax { class XAttributeList; } }
    namespace beans { class XPropertySet; }
} } }
namespace rtl { class OUString; }


/**
 * Import object index source element
 */
class XMLIndexObjectSourceContext : public XMLIndexSourceBaseContext
{
    const ::rtl::OUString sCreateFromStarCalc;
    const ::rtl::OUString sCreateFromStarChart;
    const ::rtl::OUString sCreateFromStarDraw;
    const ::rtl::OUString sCreateFromStarMath;
    const ::rtl::OUString sCreateFromOtherEmbeddedObjects;

    sal_Bool bUseCalc;
    sal_Bool bUseChart;
    sal_Bool bUseDraw;
    sal_Bool bUseMath;
    sal_Bool bUseOtherObjects;

public:

    TYPEINFO();

    XMLIndexObjectSourceContext(
        SvXMLImport& rImport,
        sal_uInt16 nPrfx,
        const ::rtl::OUString& rLocalName,
        ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet> & rPropSet);

    ~XMLIndexObjectSourceContext();

protected:

    virtual void ProcessAttribute(
        enum IndexSourceParamEnum eParam,
        const ::rtl::OUString& rValue);

    virtual void EndElement();

    virtual SvXMLImportContext* CreateChildContext(
        sal_uInt16 nPrefix,
        const ::rtl::OUString& rLocalName,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XAttributeList> & xAttrList );
};

#endif

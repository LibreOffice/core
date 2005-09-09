/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ximpcustomshape.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 13:58:12 $
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

#ifndef _XMLOFF_XIMPCUSTOMSHAPE_HXX_
#define _XMLOFF_XIMPCUSTOMSHAPE_HXX_

#ifndef _XMLOFF_XMLICTXT_HXX
#include "xmlictxt.hxx"
#endif
#ifndef _XMLOFF_XMLUCONV_HXX
#include "xmluconv.hxx"
#endif
#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#include <vector>
#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef __com_sun_star_beans_PropertyValues_hpp__
#include <com/sun/star/beans/PropertyValues.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XSHAPE_HPP_
#include <com/sun/star/drawing/XShape.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_ENHANCEDCUSTOMSHAPEPARAMETER_HPP_
#include <com/sun/star/drawing/EnhancedCustomShapeParameter.hpp>
#endif

namespace com { namespace sun { namespace star {
    namespace container { class XIndexContainer; }
    namespace beans { class XPropertySet; }
    namespace xml { namespace sax { class XAttributeList; } }
} } }

class SdXMLCustomShapeContext;
class XMLEnhancedCustomShapeContext : public SvXMLImportContext
{
    SvXMLUnitConverter& mrUnitConverter;
    com::sun::star::uno::Reference< com::sun::star::drawing::XShape >& mrxShape;
    std::vector< com::sun::star::beans::PropertyValue >& mrCustomShapeGeometry;

    std::vector< com::sun::star::beans::PropertyValue > maExtrusion;
    std::vector< com::sun::star::beans::PropertyValue > maPath;
    std::vector< com::sun::star::beans::PropertyValue > maTextPath;
    std::vector< com::sun::star::beans::PropertyValues > maHandles;
    std::vector< rtl::OUString > maEquations;
    std::vector< rtl::OUString > maEquationNames;

public:

    TYPEINFO();

    XMLEnhancedCustomShapeContext( SvXMLImport& rImport, ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > &, sal_uInt16 nPrefix,
        const ::rtl::OUString& rLocalName, std::vector< com::sun::star::beans::PropertyValue >& rCustomShapeGeometry );

    virtual void StartElement( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );
    virtual void EndElement();

    SvXMLImportContext *CreateChildContext( USHORT nPrefix, const ::rtl::OUString& rLocalName,
        const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList> & xAttrList );
};

#endif

/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xformsapi.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 16:05:43 $
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
#ifndef _XMLOFF_XFORMSAPI_HXX
#define _XMLOFF_XFORMSAPI_HXX

//
// this is a collection of several functions to make dealing with the XForms
// API a little easier
//

#include <com/sun/star/uno/Any.hxx>

namespace rtl { class OUString; }
namespace com { namespace sun { namespace star {
    namespace beans { class XPropertySet; }
    namespace frame { class XModel; }
    namespace uno { template<class A> class Reference; }
    namespace xforms { class XDataTypeRepository; }
} } }
class SvXMLNamespaceMap;


#define OUSTRING(msg) rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(msg))

com::sun::star::uno::Reference<com::sun::star::beans::XPropertySet> lcl_createXFormsModel();

com::sun::star::uno::Reference<com::sun::star::beans::XPropertySet> lcl_createXFormsBinding();

void lcl_addXFormsModel(
    const com::sun::star::uno::Reference<com::sun::star::frame::XModel>& xDocument,
    const com::sun::star::uno::Reference<com::sun::star::beans::XPropertySet>& xModel );

com::sun::star::uno::Reference<com::sun::star::beans::XPropertySet> lcl_findXFormsBinding( com::sun::star::uno::Reference<com::sun::star::frame::XModel>&, const rtl::OUString& );

com::sun::star::uno::Reference<com::sun::star::beans::XPropertySet> lcl_findXFormsSubmission( com::sun::star::uno::Reference<com::sun::star::frame::XModel>&, const rtl::OUString& );

void lcl_setValue(
    com::sun::star::uno::Reference<com::sun::star::beans::XPropertySet>& xPropSet,
    const rtl::OUString& rName,
    const com::sun::star::uno::Any rAny );

template<typename T>
void lcl_setValue(
    com::sun::star::uno::Reference<com::sun::star::beans::XPropertySet>& xPropSet,
    const rtl::OUString& rName,
    T& aValue )
{
    lcl_setValue( xPropSet, rName, com::sun::star::uno::makeAny( aValue ) );
}


com::sun::star::uno::Reference<com::sun::star::beans::XPropertySet> lcl_getXFormsModel( const com::sun::star::uno::Reference<com::sun::star::frame::XModel>& );

sal_uInt16 lcl_getTypeClass(
    const com::sun::star::uno::Reference<com::sun::star::xforms::XDataTypeRepository>& xRepository,
    const SvXMLNamespaceMap& rNamespaceMap,
    const rtl::OUString& rXMLName );

rtl::OUString lcl_getTypeName(
    const com::sun::star::uno::Reference<com::sun::star::xforms::XDataTypeRepository>& xRepository,
    const SvXMLNamespaceMap& rNamespaceMap,
    const rtl::OUString& rXMLName );

rtl::OUString lcl_getBasicTypeName(
    const com::sun::star::uno::Reference<com::sun::star::xforms::XDataTypeRepository>& xRepository,
    const SvXMLNamespaceMap& rNamespaceMap,
    const rtl::OUString& rXMLName );

#endif

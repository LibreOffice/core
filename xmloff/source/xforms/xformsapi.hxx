/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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

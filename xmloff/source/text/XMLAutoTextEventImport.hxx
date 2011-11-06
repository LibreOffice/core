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



#ifndef _XMLOFF_XMLAUTOTEXTEVENTIMPORT_HXX
#define _XMLOFF_XMLAUTOTEXTEVENTIMPORT_HXX

#include <xmloff/xmlimp.hxx>
#include <com/sun/star/uno/Reference.hxx>


namespace rtl { class OUString; }
namespace com { namespace sun { namespace star {
    namespace frame { class XModel; }
    namespace lang { class XMultiServiceFactory; }
    namespace text { class XAutoTextContainer; }
    namespace text { class XAutoTextGroup; }
    namespace text { class XAutoTextEntry; }
    namespace uno { template<class X> class Reference; }
    namespace uno { template<class X> class Sequence; }
    namespace uno { class XInterface; }
    namespace uno { class Exception; }
    namespace xml { namespace sax { class XDocumentHandler; } }
} } }


class XMLAutoTextEventImport : public SvXMLImport
{
    ::com::sun::star::uno::Reference<
        ::com::sun::star::container::XNameReplace> xEvents;

public:
    // #110680#
    // XMLAutoTextEventImport() throw();
    XMLAutoTextEventImport(
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceFactory
        ) throw();

    ~XMLAutoTextEventImport() throw();

    // XInitialization
    virtual void SAL_CALL initialize(
        const ::com::sun::star::uno::Sequence<
            ::com::sun::star::uno::Any> & rArguments )
        throw(
            ::com::sun::star::uno::Exception,
            ::com::sun::star::uno::RuntimeException);

protected:

    virtual SvXMLImportContext* CreateContext(
        sal_uInt16 nPrefix,
        const ::rtl::OUString& rLocalName,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::xml::sax::XAttributeList > & xAttrList );

};


// global functions to support the component

::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL
    XMLAutoTextEventImport_getSupportedServiceNames()
    throw();

::rtl::OUString SAL_CALL XMLAutoTextEventImport_getImplementationName()
    throw();

::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL
    XMLAutoTextEventImport_createInstance(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::lang::XMultiServiceFactory > & )
    throw( ::com::sun::star::uno::Exception );

#endif


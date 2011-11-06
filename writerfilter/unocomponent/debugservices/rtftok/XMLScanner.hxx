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



#ifndef INCLUDED_RTFTOK_XMLSCANNER_HXX
#define INCLUDED_RTFTOK_XMLSCANNER_HXX

#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/lang/XMain.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

namespace writerfilter { namespace rtftok {

class XMLScanner : public cppu::WeakImplHelper1 < ::com::sun::star::lang::XMain >
{
private:
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > xContext;

public:
    XMLScanner(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > &xContext);

    virtual ::sal_Int32 SAL_CALL run( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aArguments ) throw (::com::sun::star::uno::RuntimeException);

public:
    const static sal_Char SERVICE_NAME[40];
    const static sal_Char IMPLEMENTATION_NAME[40];

};

::rtl::OUString XMLScanner_getImplementationName ();
sal_Bool SAL_CALL XMLScanner_supportsService( const ::rtl::OUString& ServiceName );
::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL XMLScanner_getSupportedServiceNames(  ) throw (::com::sun::star::uno::RuntimeException);
::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL XMLScanner_createInstance( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > &xContext) throw( ::com::sun::star::uno::Exception );

} } /* end namespace writerfilter::rtftok */

#define RTFTOK_XMLSCANNER_COMPONENT_ENTRY \
{\
   writerfilter::rtftok::XMLScanner_createInstance,\
   writerfilter::rtftok::XMLScanner_getImplementationName,\
   writerfilter::rtftok::XMLScanner_getSupportedServiceNames,\
   ::cppu::createSingleComponentFactory,\
   0, 0\
}


#endif /* INCLUDED_RTFTOK_SCANNERTESTSERVICE_HXX */

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




#ifndef EXTENSIONS_OOOIMPROVEMENT_SOAPREQUEST_HXX
#define EXTENSIONS_OOOIMPROVEMENT_SOAPREQUEST_HXX

#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <rtl/ustring.hxx>


namespace oooimprovement
{
    #ifdef css
        #error css defined globally
    #endif
    #define css ::com::sun::star
    class SoapRequest
    {
        public:
            SoapRequest(
                const css::uno::Reference< css::lang::XMultiServiceFactory>& sf,
                const ::rtl::OUString& soap_id,
                const ::rtl::OUString& logfile);
            void writeTo(const css::uno::Reference< css::io::XOutputStream>& target) const;
        private:
            const css::uno::Reference< css::lang::XMultiServiceFactory> m_ServiceFactory;
            // soap_id has to be a plain 7-Bit ASCII String for identification of
            // automated test logs etc.
            const ::rtl::OUString m_SoapId;
            // logfile is the file-url to the file to include
            const ::rtl::OUString m_Logfile;
    };
    #undef css
}
#endif

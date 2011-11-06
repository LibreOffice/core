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




#ifndef EXTENSIONS_OOOIMPROVEMENT_ERRORMAIL_HXX
#define EXTENSIONS_OOOIMPROVEMENT_ERRORMAIL_HXX

#include <rtl/string.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>


namespace oooimprovement
{
    #ifdef css
        #error css defined globally
    #endif
    #define css ::com::sun::star
    class Errormail
    {
        public:
            Errormail(const css::uno::Reference<css::lang::XMultiServiceFactory>& sf);
            // Errormail does NOT use CDATA so it can be included
            // in a CDATA of another XML document without further
            // quoting
            ::rtl::OString getXml();
        private:
            css::uno::Reference<css::lang::XMultiServiceFactory> m_ServiceFactory;
    };
    #undef css
}
#endif

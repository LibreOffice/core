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



// MARKER(update_precomp.py): autogen include statement, do not remove


#ifndef EXTENSIONS_OOOIMPROVEMENT_CONFIG_HXX
#define EXTENSIONS_OOOIMPROVEMENT_CONFIG_HXX

#include <rtl/ustring.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>


namespace oooimprovement
{
    class Config
    {
        public:
            Config(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& sf);
            bool getEnablingAllowed() const;
            bool getInvitationAccepted() const;
            bool getShowedInvitation() const;
            ::rtl::OUString getCompleteProductname() const;
            ::rtl::OUString getLogPath() const;
            ::rtl::OUString getReporterEmail() const;
            ::rtl::OUString getSetupLocale() const;
            ::rtl::OUString getSoapId() const;
            ::rtl::OUString getSoapUrl() const;
            sal_Int32 getReportCount() const;
#ifdef FUTURE
            sal_Int32 getFailedAttempts() const;
#endif
            sal_Int32 getOfficeStartCounterdown() const;
            sal_Int32 incrementEventCount(sal_Int32 by);
            sal_Int32 incrementReportCount(sal_Int32 by);
            sal_Int32 incrementFailedAttempts(sal_Int32 by);
            sal_Int32 decrementOfficeStartCounterdown(sal_Int32 by);
            void resetFailedAttempts();
            void giveupUploading();

        private:
            ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory> m_ServiceFactory;
    };
}
#endif

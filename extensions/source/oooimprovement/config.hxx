/*************************************************************************
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: config.hxx,v $
 *
 * $Revision: 1.3 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 ************************************************************************/

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
            bool getInvitationAccepted();
            bool getShowedInvitation();
            const ::rtl::OUString getCompleteProductname();
            const ::rtl::OUString getLogPath();
            const ::rtl::OUString getReporterEmail();
            const ::rtl::OUString getSetupLocale();
            const ::rtl::OUString getSoapId();
            const ::rtl::OUString getSoapUrl();
            const sal_Int32 getReportCount();
            const sal_Int32 getFailedAttempts();
            const sal_Int32 getOfficeStartCounterdown();
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

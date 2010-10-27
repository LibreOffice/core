/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
 *
 ************************************************************************/

#ifndef DBAUI_ITEMSETHELPER_HXX
#define DBAUI_ITEMSETHELPER_HXX

#include <sal/types.h>
#include <comphelper/uno3.hxx>
#include "dsntypes.hxx"

FORWARD_DECLARE_INTERFACE(beans,XPropertySet)
FORWARD_DECLARE_INTERFACE(sdbc,XConnection)
FORWARD_DECLARE_INTERFACE(sdbc,XDriver)
FORWARD_DECLARE_INTERFACE(lang,XMultiServiceFactory)

class SfxItemSet;
namespace dbaui
{
    class SAL_NO_VTABLE IItemSetHelper
    {
    public:
        virtual const SfxItemSet* getOutputSet() const = 0;
        virtual SfxItemSet* getWriteOutputSet() = 0;
    };

    class SAL_NO_VTABLE IDatabaseSettingsDialog
    {
    public:
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > getORB() const = 0;
        virtual ::std::pair< ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >,sal_Bool> createConnection() = 0;
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDriver > getDriver() = 0;
        virtual ::rtl::OUString getDatasourceType(const SfxItemSet& _rSet) const = 0;
        virtual void clearPassword() = 0;
        virtual sal_Bool saveDatasource() = 0;
        virtual void setTitle(const ::rtl::OUString& _sTitle) = 0;

        /** enables or disables the user's possibility to confirm the settings

            In a wizard, disabling this will usually disable the "Finish" button.
            In a normal tab dialog, this will usually disable the "OK" button.
        */
        virtual void enableConfirmSettings( bool _bEnable ) = 0;
    };
}
#endif // DBAUI_ITEMSETHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

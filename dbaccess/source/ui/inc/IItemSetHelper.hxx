/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef DBAUI_ITEMSETHELPER_HXX
#define DBAUI_ITEMSETHELPER_HXX

#include <sal/types.h>
#include <comphelper/uno3.hxx>
#include "dsntypes.hxx"

namespace com { namespace sun { namespace star {
    namespace sdbc {
        class XConnection;
        class XDriver;
    }
    namespace lang {
        class XMultiServiceFactory;
    }
}}}

class SfxItemSet;
namespace dbaui
{
    class SAL_NO_VTABLE IItemSetHelper
    {
    public:
        virtual const SfxItemSet* getOutputSet() const = 0;
        virtual SfxItemSet* getWriteOutputSet() = 0;

    protected:
        ~IItemSetHelper() {}
    };

    class SAL_NO_VTABLE IDatabaseSettingsDialog
    {
    public:
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > getORB() const = 0;
        virtual ::std::pair< ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >,sal_Bool> createConnection() = 0;
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDriver > getDriver() = 0;
        virtual OUString getDatasourceType(const SfxItemSet& _rSet) const = 0;
        virtual void clearPassword() = 0;
        virtual sal_Bool saveDatasource() = 0;
        virtual void setTitle(const OUString& _sTitle) = 0;

        /** enables or disables the user's possibility to confirm the settings

            In a wizard, disabling this will usually disable the "Finish" button.
            In a normal tab dialog, this will usually disable the "OK" button.
        */
        virtual void enableConfirmSettings( bool _bEnable ) = 0;

    protected:
        ~IDatabaseSettingsDialog() {}
    };
}
#endif // DBAUI_ITEMSETHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

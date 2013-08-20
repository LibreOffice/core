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

#ifndef EXTENSIONS_ABP_ADMININVOKATIONIMPL_HXX
#define EXTENSIONS_ABP_ADMININVOKATIONIMPL_HXX

#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>


class Window;
//.........................................................................
namespace abp
{
//.........................................................................

    //=====================================================================
    //= OAdminDialogInvokation
    //=====================================================================
    /** outsourced from AdminDialogInvokationPage, 'cause this class here, in opposite to
        the page, needs exception handling to be enabled.
    */
    class OAdminDialogInvokation
    {
    private:
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >
                        m_xContext;
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > m_xDataSource;
        Window*         m_pMessageParent;

    public:
        OAdminDialogInvokation(
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rxContext,
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > _xDataSource,
            Window* _pMessageParent
        );

        sal_Bool invokeAdministration( sal_Bool _bFixedType );
    };

//.........................................................................
}   // namespace abp
//.........................................................................

#endif // EXTENSIONS_ABP_ADMININVOKATIONIMPL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

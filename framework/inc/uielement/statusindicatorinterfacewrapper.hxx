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

#pragma once

#include <com/sun/star/task/XStatusIndicator.hpp>
#include <com/sun/star/lang/XComponent.hpp>

#include <rtl/ustring.hxx>
#include <rtl/ref.hxx>
#include <cppuhelper/implbase.hxx>
#include <unotools/weakref.hxx>

namespace framework
{
class ProgressBarWrapper;

class StatusIndicatorInterfaceWrapper final : public ::cppu::WeakImplHelper< css::task::XStatusIndicator>
{
    public:
        StatusIndicatorInterfaceWrapper( const rtl::Reference< ProgressBarWrapper >& rStatusIndicatorImpl );
        virtual ~StatusIndicatorInterfaceWrapper() override;

        //  XStatusIndicator

        virtual void SAL_CALL start   ( const OUString& sText  ,
                                              sal_Int32        nRange ) override;
        virtual void SAL_CALL end     (                               ) override;
        virtual void SAL_CALL reset   (                               ) override;
        virtual void SAL_CALL setText ( const OUString& sText  ) override;
        virtual void SAL_CALL setValue(       sal_Int32        nValue ) override;

    private:
        unotools::WeakReference< ProgressBarWrapper > m_xStatusIndicatorImpl;
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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
#ifndef INCLUDED_SFX2_SOURCE_INC_FLTOPTINT_HXX
#define INCLUDED_SFX2_SOURCE_INC_FLTOPTINT_HXX

#include <com/sun/star/document/XInteractionFilterOptions.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <comphelper/interaction.hxx>
#include <cppuhelper/implbase.hxx>
#include <rtl/ref.hxx>

class FilterOptionsContinuation : public comphelper::OInteraction< css::document::XInteractionFilterOptions >
{
    css::uno::Sequence< css::beans::PropertyValue > rProperties;

public:
    virtual void SAL_CALL setFilterOptions( const css::uno::Sequence< css::beans::PropertyValue >& rProp ) override;
    virtual css::uno::Sequence< css::beans::PropertyValue > SAL_CALL getFilterOptions(  ) override;
};

class RequestFilterOptions : public ::cppu::WeakImplHelper< css::task::XInteractionRequest >
{
    css::uno::Any m_aRequest;

    rtl::Reference<comphelper::OInteractionAbort>  m_xAbort;
    rtl::Reference<FilterOptionsContinuation>      m_xOptions;

public:
    RequestFilterOptions( css::uno::Reference< css::frame::XModel > const & rModel,
                              const css::uno::Sequence< css::beans::PropertyValue >& rProperties );

    bool    isAbort() const { return m_xAbort->wasSelected(); }

    css::uno::Sequence< css::beans::PropertyValue > getFilterOptions()
    {
        return m_xOptions->getFilterOptions();
    }

    virtual css::uno::Any SAL_CALL getRequest() override;

    virtual css::uno::Sequence< css::uno::Reference< css::task::XInteractionContinuation >
            > SAL_CALL getContinuations() override;
};

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

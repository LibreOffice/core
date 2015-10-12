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

class FilterOptionsContinuation : public comphelper::OInteraction< ::com::sun::star::document::XInteractionFilterOptions >
{
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > rProperties;

public:
    virtual void SAL_CALL setFilterOptions( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& rProp ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > SAL_CALL getFilterOptions(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
};

class RequestFilterOptions : public ::cppu::WeakImplHelper< ::com::sun::star::task::XInteractionRequest >
{
    ::com::sun::star::uno::Any m_aRequest;

    ::com::sun::star::uno::Sequence<
                    ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionContinuation >
                > m_lContinuations;

    comphelper::OInteractionAbort*  m_pAbort;

    FilterOptionsContinuation*  m_pOptions;

public:
    RequestFilterOptions( ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > rModel,
                              const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& rProperties );

    bool    isAbort() { return m_pAbort->wasSelected(); }

    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > getFilterOptions()
    {
        return m_pOptions->getFilterOptions();
    }

    virtual ::com::sun::star::uno::Any SAL_CALL getRequest()
        throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

    virtual ::com::sun::star::uno::Sequence<
                ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionContinuation >
            > SAL_CALL getContinuations()
        throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;
};

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

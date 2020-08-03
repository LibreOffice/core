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

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Reference.h>

namespace com::sun::star::chart2 { class XInternalDataProvider; }
namespace com::sun::star::frame { class XModel; }

namespace chart
{

    enum ModelFacet
    {
        E_MODEL,
        E_MODEL_WITH_DATA,
        E_MODEL_WITH_SELECTION
    };

    class ChartModelClone
    {
    public:
        ChartModelClone(
            const css::uno::Reference< css::frame::XModel >& i_model,
            const ModelFacet i_facet
        );

        ~ChartModelClone();

        ChartModelClone(const ChartModelClone&) = delete;
        const ChartModelClone& operator=(const ChartModelClone&) = delete;

        ModelFacet getFacet() const;

        void applyToModel( const css::uno::Reference< css::frame::XModel >& i_model ) const;

        static void applyModelContentToModel(
            const css::uno::Reference< css::frame::XModel > & i_model,
            const css::uno::Reference< css::frame::XModel > & i_modelToCopyFrom,
            const css::uno::Reference< css::chart2::XInternalDataProvider > & i_data );

        void dispose();

    private:
        bool    impl_isDisposed() const { return !m_xModelClone.is(); }

    private:
        css::uno::Reference< css::frame::XModel >                 m_xModelClone;
        css::uno::Reference< css::chart2::XInternalDataProvider > m_xDataClone;
        css::uno::Any                                             m_aSelection;
    };

} // namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

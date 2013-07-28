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

#ifndef CHART2_CHARTMODELCLONE_HXX
#define CHART2_CHARTMODELCLONE_HXX

#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/chart2/XInternalDataProvider.hpp>

#include <boost/noncopyable.hpp>

namespace chart
{

    //= ModelFacet
    enum ModelFacet
    {
        E_MODEL,
        E_MODEL_WITH_DATA,
        E_MODEL_WITH_SELECTION
    };

    //= ChartModelClone
    class ChartModelClone : public ::boost::noncopyable
    {
    public:
        ChartModelClone(
            const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& i_model,
            const ModelFacet i_facet
        );

        ~ChartModelClone();

        ModelFacet getFacet() const;

        void applyToModel( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& i_model ) const;

        static void applyModelContentToModel(
            const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > & i_model,
            const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > & i_modelToCopyFrom,
            const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XInternalDataProvider > & i_data );

        void dispose();

    private:
        bool    impl_isDisposed() const { return !m_xModelClone.is(); }

    private:
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >                 m_xModelClone;
        ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XInternalDataProvider > m_xDataClone;
        ::com::sun::star::uno::Any                                                          m_aSelection;
    };

} // namespace chart

#endif // CHART2_CHARTMODELCLONE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

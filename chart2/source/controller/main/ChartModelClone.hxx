/*************************************************************************
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

#ifndef CHART2_CHARTMODELCLONE_HXX
#define CHART2_CHARTMODELCLONE_HXX

/** === begin UNO includes === **/
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/chart2/XInternalDataProvider.hpp>
/** === end UNO includes === **/

#include <boost/noncopyable.hpp>

//......................................................................................................................
namespace chart
{
//......................................................................................................................

    //==================================================================================================================
    //= ModelFacet
    //==================================================================================================================
    enum ModelFacet
    {
        E_MODEL,
        E_MODEL_WITH_DATA,
        E_MODEL_WITH_SELECTION
    };

    //==================================================================================================================
    //= ChartModelClone
    //==================================================================================================================
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

//......................................................................................................................
} // namespace chart
//......................................................................................................................

#endif // CHART2_CHARTMODELCLONE_HXX

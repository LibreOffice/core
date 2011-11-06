/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

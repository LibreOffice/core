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



#ifndef REPORTDESIGN_FORMATNORMALIZER_HXX
#define REPORTDESIGN_FORMATNORMALIZER_HXX

/** === begin UNO includes === **/
#include <com/sun/star/report/XReportDefinition.hpp>
#include <com/sun/star/beans/PropertyChangeEvent.hpp>
#include <com/sun/star/report/XFormattedField.hpp>
/** === end UNO includes === **/

#include <boost/noncopyable.hpp>

#include <vector>

//........................................................................
namespace rptui
{
//........................................................................

    class OReportModel;
    //====================================================================
    //= FormatNormalizer
    //====================================================================
    class FormatNormalizer : public ::boost::noncopyable
    {
    public:
        struct Field
        {
            ::rtl::OUString sName;
            sal_Int32       nDataType;
            sal_Int32       nScale;
            sal_Bool        bIsCurrency;

            Field() : sName(), nDataType( 0 ), nScale( 0 ), bIsCurrency( 0 ) { }
        };
        typedef ::std::vector< Field >  FieldList;

    private:
        const OReportModel&                                                                 m_rModel;
        ::com::sun::star::uno::Reference< ::com::sun::star::report::XReportDefinition >     m_xReportDefinition;

        /// is our field list dirty?
        FieldList                       m_aFields;
        bool                            m_bFieldListDirty;

    public:
        FormatNormalizer( const OReportModel& _rModel );
        ~FormatNormalizer();

        void    notifyPropertyChange( const ::com::sun::star::beans::PropertyChangeEvent& _rEvent );
        void    notifyElementInserted( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxElement );

    private:
        bool    impl_lateInit();

        void    impl_onDefinitionPropertyChange( const ::rtl::OUString& _rChangedPropName );
        void    impl_onFormattedProperttyChange( const ::com::sun::star::uno::Reference< ::com::sun::star::report::XFormattedField >& _rxFormatted, const ::rtl::OUString& _rChangedPropName );

        bool    impl_ensureUpToDateFieldList_nothrow();

        void    impl_adjustFormatToDataFieldType_nothrow( const ::com::sun::star::uno::Reference< ::com::sun::star::report::XFormattedField >& _rxFormatted );
    };

//........................................................................
} // namespace rptui
//........................................................................

#endif // REPORTDESIGN_FORMATNORMALIZER_HXX

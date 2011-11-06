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



#ifndef RPT_REPORTCONTROLMODEL_HXX
#define RPT_REPORTCONTROLMODEL_HXX

#include "ReportComponent.hxx"
#include "com/sun/star/style/VerticalAlignment.hpp"
#include "com/sun/star/awt/FontDescriptor.hpp"
#include "com/sun/star/report/XFormatCondition.hpp"
#include "com/sun/star/container/XContainer.hpp"
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <tools/color.hxx>
#include <comphelper/uno3.hxx>
#include <cppuhelper/interfacecontainer.h>


namespace reportdesign
{
    bool operator==( const ::com::sun::star::awt::FontDescriptor& _lhs, const ::com::sun::star::awt::FontDescriptor& _rhs );

    inline bool operator!=( const ::com::sun::star::awt::FontDescriptor& _lhs, const ::com::sun::star::awt::FontDescriptor& _rhs )
    {
        return !( _lhs == _rhs );
    }

    struct OFormatProperties
    {
        ::sal_Int16                                         nAlign;
        ::com::sun::star::awt::FontDescriptor               aFontDescriptor;
        ::com::sun::star::awt::FontDescriptor               aAsianFontDescriptor;
        ::com::sun::star::awt::FontDescriptor               aComplexFontDescriptor;
        ::com::sun::star::lang::Locale                      aCharLocale;
        ::com::sun::star::lang::Locale                      aCharLocaleAsian;
        ::com::sun::star::lang::Locale                      aCharLocaleComplex;
        ::sal_Int16                                         nFontEmphasisMark;
        ::sal_Int16                                         nFontRelief;
        ::sal_Int32                                         nTextColor;
        ::sal_Int32                                         nTextLineColor;
        ::sal_Int32                                         nCharUnderlineColor;
        ::sal_Int32                                         nBackgroundColor;
        ::rtl::OUString                                     sCharCombinePrefix;
        ::rtl::OUString                                     sCharCombineSuffix;
        ::rtl::OUString                                     sHyperLinkURL;
        ::rtl::OUString                                     sHyperLinkTarget;
        ::rtl::OUString                                     sHyperLinkName;
        ::rtl::OUString                                     sVisitedCharStyleName;
        ::rtl::OUString                                     sUnvisitedCharStyleName;
        com::sun::star::style::VerticalAlignment            aVerticalAlignment;
        ::sal_Int16                                         nCharEscapement;
        ::sal_Int16                                         nCharCaseMap;
        ::sal_Int16                                         nCharKerning;
        ::sal_Int8                                          nCharEscapementHeight;
        ::sal_Bool                                          m_bBackgroundTransparent;
        ::sal_Bool                                          bCharFlash;
        ::sal_Bool                                          bCharAutoKerning;
        ::sal_Bool                                          bCharCombineIsOn;
        ::sal_Bool                                          bCharHidden;
        ::sal_Bool                                          bCharShadowed;
        ::sal_Bool                                          bCharContoured;
        OFormatProperties();
    };
    class OReportControlModel
    {
        void checkIndex(sal_Int32 _nIndex);
        OReportControlModel(OReportControlModel&);
        void operator =(OReportControlModel&);
    public:
        ::cppu::OInterfaceContainerHelper                   aContainerListeners;
        OReportComponentProperties                          aComponent;
        OFormatProperties                                   aFormatProperties;
        ::com::sun::star::container::XContainer*            m_pOwner;
        ::std::vector< ::com::sun::star::uno::Reference< ::com::sun::star::report::XFormatCondition> >
                                                            m_aFormatConditions;
        osl::Mutex&                                         m_rMutex;
        ::rtl::OUString                                     aDataField;
        ::rtl::OUString                                     aConditionalPrintExpression;
        sal_Bool                                            bPrintWhenGroupChange;

        OReportControlModel(osl::Mutex& _rMutex
                            ,::com::sun::star::container::XContainer* _pOwner
                            ,::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > const & _xContext)
            :aContainerListeners(_rMutex)
            ,aComponent(_xContext)
            ,m_pOwner(_pOwner)
            ,m_rMutex(_rMutex)
            ,bPrintWhenGroupChange(sal_False)
        {}

        // XContainer
        void addContainerListener( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainerListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
        void removeContainerListener( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainerListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);

        // XElementAccess
        ::sal_Bool hasElements(  ) throw (::com::sun::star::uno::RuntimeException);

        // XIndexReplace
        void replaceByIndex( ::sal_Int32 Index, const ::com::sun::star::uno::Any& Element ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

        // XIndexContainer
        void insertByIndex( ::sal_Int32 Index, const ::com::sun::star::uno::Any& Element ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
        void removeByIndex( ::sal_Int32 Index ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

        // XIndexAccess
        ::sal_Int32 getCount(  ) throw (::com::sun::star::uno::RuntimeException);
        ::com::sun::star::uno::Any getByIndex( ::sal_Int32 Index ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

        static bool isInterfaceForbidden(const ::com::sun::star::uno::Type& _rType);
    };
}
#endif // RPT_REPORTCONTROLMODEL_HXX


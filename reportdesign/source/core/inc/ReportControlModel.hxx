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

#ifndef INCLUDED_REPORTDESIGN_SOURCE_CORE_INC_REPORTCONTROLMODEL_HXX
#define INCLUDED_REPORTDESIGN_SOURCE_CORE_INC_REPORTCONTROLMODEL_HXX

#include "ReportComponent.hxx"
#include <com/sun/star/style/VerticalAlignment.hpp>
#include <com/sun/star/awt/FontDescriptor.hpp>
#include <com/sun/star/report/XFormatCondition.hpp>
#include <com/sun/star/container/XContainer.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <com/sun/star/lang/Locale.hpp>
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
        OUString                                     sCharCombinePrefix;
        OUString                                     sCharCombineSuffix;
        OUString                                     sHyperLinkURL;
        OUString                                     sHyperLinkTarget;
        OUString                                     sHyperLinkName;
        OUString                                     sVisitedCharStyleName;
        OUString                                     sUnvisitedCharStyleName;
        com::sun::star::style::VerticalAlignment            aVerticalAlignment;
        ::sal_Int16                                         nCharEscapement;
        ::sal_Int16                                         nCharCaseMap;
        ::sal_Int16                                         nCharKerning;
        ::sal_Int8                                          nCharEscapementHeight;
        bool                                                m_bBackgroundTransparent;
        bool                                                bCharFlash;
        bool                                                bCharAutoKerning;
        bool                                                bCharCombineIsOn;
        bool                                                bCharHidden;
        bool                                                bCharShadowed;
        bool                                                bCharContoured;
        OFormatProperties();
    };
    class OReportControlModel
    {
        void checkIndex(sal_Int32 _nIndex);
        OReportControlModel(OReportControlModel&) = delete;
        void operator =(OReportControlModel&) = delete;
    public:
        ::cppu::OInterfaceContainerHelper                   aContainerListeners;
        OReportComponentProperties                          aComponent;
        OFormatProperties                                   aFormatProperties;
        ::com::sun::star::container::XContainer*            m_pOwner;
        ::std::vector< ::com::sun::star::uno::Reference< ::com::sun::star::report::XFormatCondition> >
                                                            m_aFormatConditions;
        osl::Mutex&                                         m_rMutex;
        OUString                                            aDataField;
        OUString                                            aConditionalPrintExpression;
        bool                                                bPrintWhenGroupChange;

        OReportControlModel(osl::Mutex& _rMutex
                            ,::com::sun::star::container::XContainer* _pOwner
                            ,::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > const & _xContext)
            :aContainerListeners(_rMutex)
            ,aComponent(_xContext)
            ,m_pOwner(_pOwner)
            ,m_rMutex(_rMutex)
            ,bPrintWhenGroupChange(true)
        {}

        // XContainer
        void addContainerListener( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainerListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
        void removeContainerListener( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainerListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);

        // XElementAccess
        bool hasElements(  ) throw (::com::sun::star::uno::RuntimeException);

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
#endif // INCLUDED_REPORTDESIGN_SOURCE_CORE_INC_REPORTCONTROLMODEL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

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
    bool operator==( const css::awt::FontDescriptor& _lhs, const css::awt::FontDescriptor& _rhs );

    inline bool operator!=( const css::awt::FontDescriptor& _lhs, const css::awt::FontDescriptor& _rhs )
    {
        return !( _lhs == _rhs );
    }

    struct OFormatProperties
    {
        ::sal_Int16                                         nAlign;
        css::awt::FontDescriptor                            aFontDescriptor;
        css::awt::FontDescriptor                            aAsianFontDescriptor;
        css::awt::FontDescriptor                            aComplexFontDescriptor;
        css::lang::Locale                                   aCharLocale;
        css::lang::Locale                                   aCharLocaleAsian;
        css::lang::Locale                                   aCharLocaleComplex;
        ::sal_Int16                                         nFontEmphasisMark;
        ::sal_Int16                                         nFontRelief;
        ::sal_Int32                                         nTextColor;
        ::sal_Int32                                         nTextLineColor;
        ::sal_Int32                                         nBackgroundColor;
        OUString                                            sCharCombinePrefix;
        OUString                                            sCharCombineSuffix;
        OUString                                            sHyperLinkURL;
        OUString                                            sHyperLinkTarget;
        OUString                                            sHyperLinkName;
        OUString                                            sVisitedCharStyleName;
        OUString                                            sUnvisitedCharStyleName;
        css::style::VerticalAlignment                       aVerticalAlignment;
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
        css::container::XContainer*                         m_pOwner;
        ::std::vector< css::uno::Reference< css::report::XFormatCondition> >
                                                            m_aFormatConditions;
        osl::Mutex&                                         m_rMutex;
        OUString                                            aDataField;
        OUString                                            aConditionalPrintExpression;
        bool                                                bPrintWhenGroupChange;

        OReportControlModel(osl::Mutex& _rMutex
                            ,css::container::XContainer* _pOwner
                            ,css::uno::Reference< css::uno::XComponentContext > const & _xContext)
            :aContainerListeners(_rMutex)
            ,aComponent(_xContext)
            ,m_pOwner(_pOwner)
            ,m_rMutex(_rMutex)
            ,bPrintWhenGroupChange(true)
        {}

        // XContainer
        void addContainerListener( const css::uno::Reference< css::container::XContainerListener >& xListener ) throw (css::uno::RuntimeException);
        void removeContainerListener( const css::uno::Reference< css::container::XContainerListener >& xListener ) throw (css::uno::RuntimeException);

        // XElementAccess
        bool hasElements(  ) throw (css::uno::RuntimeException);

        // XIndexReplace
        void replaceByIndex( ::sal_Int32 Index, const css::uno::Any& Element ) throw (css::lang::IllegalArgumentException, css::lang::IndexOutOfBoundsException, css::lang::WrappedTargetException, css::uno::RuntimeException);

        // XIndexContainer
        void insertByIndex( ::sal_Int32 Index, const css::uno::Any& Element ) throw (css::lang::IllegalArgumentException, css::lang::IndexOutOfBoundsException, css::lang::WrappedTargetException, css::uno::RuntimeException);
        void removeByIndex( ::sal_Int32 Index ) throw (css::lang::IndexOutOfBoundsException, css::lang::WrappedTargetException, css::uno::RuntimeException);

        // XIndexAccess
        ::sal_Int32 getCount(  ) throw (css::uno::RuntimeException);
        css::uno::Any getByIndex( ::sal_Int32 Index ) throw (css::lang::IndexOutOfBoundsException, css::lang::WrappedTargetException, css::uno::RuntimeException);

        static bool isInterfaceForbidden(const css::uno::Type& _rType);
    };
}
#endif // INCLUDED_REPORTDESIGN_SOURCE_CORE_INC_REPORTCONTROLMODEL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

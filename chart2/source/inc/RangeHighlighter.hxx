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
#ifndef INCLUDED_CHART2_SOURCE_INC_RANGEHIGHLIGHTER_HXX
#define INCLUDED_CHART2_SOURCE_INC_RANGEHIGHLIGHTER_HXX

#include "MutexContainer.hxx"
#include <cppuhelper/compbase.hxx>
#include <com/sun/star/chart2/data/XRangeHighlighter.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

namespace com { namespace sun { namespace star {
namespace chart2 {
    class XDiagram;
    class XDataSeries;
    class XAxis;
}}}}

namespace chart
{

namespace impl
{
typedef ::cppu::WeakComponentImplHelper<
        ::com::sun::star::chart2::data::XRangeHighlighter,
        ::com::sun::star::view::XSelectionChangeListener
    >
    RangeHighlighter_Base;
}

class RangeHighlighter :
        public MutexContainer,
        public impl::RangeHighlighter_Base
{
public:
    explicit RangeHighlighter(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::view::XSelectionSupplier > & xSelectionSupplier );
    virtual ~RangeHighlighter();

protected:
    // ____ XRangeHighlighter ____
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::chart2::data::HighlightedRange > SAL_CALL getSelectedRanges()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addSelectionChangeListener(
        const ::com::sun::star::uno::Reference< ::com::sun::star::view::XSelectionChangeListener >& xListener )
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeSelectionChangeListener(
        const ::com::sun::star::uno::Reference< ::com::sun::star::view::XSelectionChangeListener >& xListener )
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // ____ XSelectionChangeListener ____
    virtual void SAL_CALL selectionChanged(
        const ::com::sun::star::lang::EventObject& aEvent )
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // ____ XEventListener (base of XSelectionChangeListener) ____
    virtual void SAL_CALL disposing(
        const ::com::sun::star::lang::EventObject& Source )
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // ____ WeakComponentImplHelperBase ____
    // is called when dispose() is called at this component
    virtual void SAL_CALL disposing() override;

private:
    void fireSelectionEvent();
    void startListening();
    void stopListening();
    void determineRanges();

    void fillRangesForDiagram( const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XDiagram > & xDiagram );
    void fillRangesForDataSeries( const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XDataSeries > & xSeries );
    void fillRangesForCategories( const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XAxis > & xAxis );
    void fillRangesForDataPoint( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > & xDataSeries, sal_Int32 nIndex );
    void fillRangesForErrorBars( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > & xErrorBar,
                                 const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XDataSeries > & xDataSeries );

    ::com::sun::star::uno::Reference< ::com::sun::star::view::XSelectionSupplier >
        m_xSelectionSupplier;
    ::com::sun::star::uno::Reference< ::com::sun::star::view::XSelectionChangeListener >
        m_xListener;
    ::com::sun::star::uno::Sequence< ::com::sun::star::chart2::data::HighlightedRange >
        m_aSelectedRanges;
    sal_Int32 m_nAddedListenerCount;
    bool m_bIncludeHiddenCells;
};

} //  namespace chart

// INCLUDED_CHART2_SOURCE_INC_RANGEHIGHLIGHTER_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

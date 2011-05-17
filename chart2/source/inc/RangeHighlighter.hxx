/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
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
#ifndef CHART2_RANGEHIGHLIGHTER_HXX
#define CHART2_RANGEHIGHLIGHTER_HXX

#include "MutexContainer.hxx"
#include <cppuhelper/compbase2.hxx>
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
typedef ::cppu::WeakComponentImplHelper2<
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
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addSelectionChangeListener(
        const ::com::sun::star::uno::Reference< ::com::sun::star::view::XSelectionChangeListener >& xListener )
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeSelectionChangeListener(
        const ::com::sun::star::uno::Reference< ::com::sun::star::view::XSelectionChangeListener >& xListener )
        throw (::com::sun::star::uno::RuntimeException);

    // ____ XSelectionChangeListener ____
    virtual void SAL_CALL selectionChanged(
        const ::com::sun::star::lang::EventObject& aEvent )
        throw (::com::sun::star::uno::RuntimeException);

    // ____ XEventListener (base of XSelectionChangeListener) ____
    virtual void SAL_CALL disposing(
        const ::com::sun::star::lang::EventObject& Source )
        throw (::com::sun::star::uno::RuntimeException);

    // ____ WeakComponentImplHelperBase ____
    // is called when dispose() is called at this component
    virtual void SAL_CALL disposing();

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

// CHART2_RANGEHIGHLIGHTER_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

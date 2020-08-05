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

#include "MutexContainer.hxx"
#include <cppuhelper/compbase.hxx>
#include <com/sun/star/chart2/data/XRangeHighlighter.hpp>
#include <com/sun/star/view/XSelectionChangeListener.hpp>

namespace com::sun::star {
    namespace chart2 {
        class XDiagram;
        class XDataSeries;
        class XAxis;
    }
}
namespace com::sun::star::beans { class XPropertySet; }
namespace com::sun::star::view { class XSelectionSupplier; }

namespace chart
{

namespace impl
{
typedef ::cppu::WeakComponentImplHelper<
        css::chart2::data::XRangeHighlighter,
        css::view::XSelectionChangeListener
    >
    RangeHighlighter_Base;
}

class RangeHighlighter :
        public MutexContainer,
        public impl::RangeHighlighter_Base
{
public:
    explicit RangeHighlighter(
        const css::uno::Reference< css::view::XSelectionSupplier > & xSelectionSupplier );
    virtual ~RangeHighlighter() override;

protected:
    // ____ XRangeHighlighter ____
    virtual css::uno::Sequence< css::chart2::data::HighlightedRange > SAL_CALL getSelectedRanges() override;
    virtual void SAL_CALL addSelectionChangeListener(
        const css::uno::Reference< css::view::XSelectionChangeListener >& xListener ) override;
    virtual void SAL_CALL removeSelectionChangeListener(
        const css::uno::Reference< css::view::XSelectionChangeListener >& xListener ) override;

    // ____ XSelectionChangeListener ____
    virtual void SAL_CALL selectionChanged(
        const css::lang::EventObject& aEvent ) override;

    // ____ XEventListener (base of XSelectionChangeListener) ____
    virtual void SAL_CALL disposing(
        const css::lang::EventObject& Source ) override;

    // ____ WeakComponentImplHelperBase ____
    // is called when dispose() is called at this component
    virtual void SAL_CALL disposing() override;

private:
    void fireSelectionEvent();
    void startListening();
    void stopListening();
    void determineRanges();

    void fillRangesForDiagram( const css::uno::Reference< css::chart2::XDiagram > & xDiagram );
    void fillRangesForDataSeries( const css::uno::Reference< css::chart2::XDataSeries > & xSeries );
    void fillRangesForCategories( const css::uno::Reference< css::chart2::XAxis > & xAxis );
    void fillRangesForDataPoint( const css::uno::Reference< css::uno::XInterface > & xDataSeries, sal_Int32 nIndex );
    void fillRangesForErrorBars( const css::uno::Reference< css::beans::XPropertySet > & xErrorBar,
                                 const css::uno::Reference< css::chart2::XDataSeries > & xDataSeries );

    css::uno::Reference< css::view::XSelectionSupplier >
        m_xSelectionSupplier;
    css::uno::Reference< css::view::XSelectionChangeListener >
        m_xListener;
    css::uno::Sequence< css::chart2::data::HighlightedRange >
        m_aSelectedRanges;
    sal_Int32 m_nAddedListenerCount;
    bool m_bIncludeHiddenCells;
};

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

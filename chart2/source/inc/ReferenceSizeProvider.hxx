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
#ifndef CHART2_REFERENCESIZEPROVIDER_HXX
#define CHART2_REFERENCESIZEPROVIDER_HXX

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/awt/Size.hpp>
#include "charttoolsdllapi.hxx"

namespace com { namespace sun { namespace star {
namespace chart2 {
    class XTitle;
    class XTitled;
}
}}}

namespace chart
{

class OOO_DLLPUBLIC_CHARTTOOLS ReferenceSizeProvider
{
public:

    enum AutoResizeState
    {
        AUTO_RESIZE_YES,
        AUTO_RESIZE_NO,
        AUTO_RESIZE_AMBIGUOUS,
        AUTO_RESIZE_UNKNOWN
    };

    ReferenceSizeProvider(
        ::com::sun::star::awt::Size aPageSize,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XChartDocument > & xChartDoc );

    ::com::sun::star::awt::Size getPageSize() const;

    /** Retrieves the state auto-resize from all objects that support this
        feature.  If all objects return the same state, AUTO_RESIZE_YES or
        AUTO_RESIZE_NO is returned.

        If no object supporting the feature is found, AUTO_RESIZE_UNKNOWN is
        returned.  If there are multiple objects, some with state YES and some
        with state NO, AUTO_RESIZE_AMBIGUOUS is returned.
    */
    static AutoResizeState getAutoResizeState(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XChartDocument > & xChartDoc );

    /** sets or resets the auto-resize at all objects that support this feature
        for text to the opposite of the current setting.  If the current state
        is ambiguous, it is turned on.  If the current state is unknown it stays
        unknown.
    */
    void toggleAutoResizeState();

    /** Sets the ReferencePageSize according to the internal settings of this
        class at the XPropertySet, and the adapted font sizes if bAdaptFontSizes
        is </sal_True>.
     */
    SAL_DLLPRIVATE void setValuesAtPropertySet(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet > & xProp,
        bool bAdaptFontSizes = true );

    /** Sets the ReferencePageSize according to the internal settings of this
        class at the XTitle, and the adapted font sizes at the contained
        XFormattedStrings
     */
    SAL_DLLPRIVATE void setValuesAtTitle(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XTitle > & xTitle );

    /** Sets the internal value at all data series in the currently set model.
        This is useful, if you have changed a chart-type and thus probably added
        some new data series via model functionality.
     */
    void setValuesAtAllDataSeries();

private:
    SAL_DLLPRIVATE bool useAutoScale() const;

    /** sets the auto-resize at all objects that support this feature for text.
        eNewState must be either AUTO_RESIZE_YES or AUTO_RESIZE_NO
    */
    SAL_DLLPRIVATE void setAutoResizeState( AutoResizeState eNewState );

    /** Retrieves the auto-resize state from the given propertyset.  The result
        will be put into eInOutState.  If you initialize eInOutState with
        AUTO_RESIZE_UNKNOWN, you will get the actual state.  If you pass any
        other state, the result will be the accumulated state,
        esp. AUTO_RESIZE_AMBIGUOUS, if the value was NO before, and is YES for
        the current property set, or the other way round.
     */
    SAL_DLLPRIVATE static void getAutoResizeFromPropSet(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet > & xProp,
        AutoResizeState & rInOutState );

    SAL_DLLPRIVATE void impl_setValuesAtTitled(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XTitled > & xTitled );
    SAL_DLLPRIVATE static void impl_getAutoResizeFromTitled(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XTitled > & xTitled,
        AutoResizeState & rInOutState );

    ::com::sun::star::awt::Size m_aPageSize;
    ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XChartDocument > m_xChartDoc;
    bool      m_bUseAutoScale;
};

} //  namespace chart

// CHART2_REFERENCESIZEPROVIDER_HXX
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */

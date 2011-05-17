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

// ----------------------------------------
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

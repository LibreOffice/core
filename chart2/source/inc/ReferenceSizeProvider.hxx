/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ReferenceSizeProvider.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 18:21:21 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef CHART2_REFERENCESIZEPROVIDER_HXX
#define CHART2_REFERENCESIZEPROVIDER_HXX

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif
#ifndef _COM_SUN_STAR_CHART2_XCHARTDOCUMENT_HPP_
#include <com/sun/star/chart2/XChartDocument.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_SIZE_HPP_
#include <com/sun/star/awt/Size.hpp>
#endif

namespace com { namespace sun { namespace star {
namespace chart2 {
    class XTitle;
    class XTitled;
}
}}}

// ----------------------------------------
namespace chart
{

class ReferenceSizeProvider
{
public:

    enum AutoResizeState
    {
        AUTO_RESIZE_YES,
        AUTO_RESIZE_NO,
        AUTO_RESIZE_AMBIGUOUS,
        AUTO_RESIZE_UNKNOWN
    };

    enum ReferenceSizeType
    {
        REF_PAGE,
        REF_DIAGRAM
    };

    ReferenceSizeProvider(
        ::com::sun::star::awt::Size aPageSize,
        ::com::sun::star::awt::Size aDiagramSize,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XChartDocument > & xChartDoc );

    ::com::sun::star::awt::Size getPageSize() const;
    ::com::sun::star::awt::Size getDiagramSize() const;
    bool useAutoScale() const;

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

    /** Retrieves the current auto-resize state from the internally set chart
        document
     */
    AutoResizeState getAutoResizeState() const;

    /** sets or resets the auto-resize at all objects that support this feature
        for text to the opposite of the current setting.  If the current state
        is ambiguous, it is turned on.  If the current state is unknown it stays
        unknown.
    */
    void toggleAutoResizeState();

    /** sets the auto-resize at all objects that support this feature for text.
        eNewState must be either AUTO_RESIZE_YES or AUTO_RESIZE_NO
    */
    void setAutoResizeState( AutoResizeState eNewState );



    /** Sets the ReferencePageSize according to the internal settings of this
        class at the XPropertySet, and the adapted font sizes if bAdaptFontSizes
        is </TRUE>.  The type determines whether the diagram or the page is used
        for reference.
     */
    void setValuesAtPropertySet(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet > & xProp,
        ReferenceSizeType eType,
        bool bAdaptFontSizes = true );

    /** Sets the ReferencePageSize according to the internal settings of this
        class at the XTitle, and the adapted font sizes at the contained
        XFormattedStrings
     */
    void setValuesAtTitle(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XTitle > & xTitle );

    /** Sets the internal value at all data series in the currently set model.
        This is useful, if you have changed a chart-type and thus probably added
        some new data series via model functionality (which does not know the
        diagram size).
     */
    void setValuesAtAllDataSeries();

    /** Retrieves the auto-resize state from the given propertyset.  The result
        will be put into eInOutState.  If you initialize eInOutState with
        AUTO_RESIZE_UNKNOWN, you will get the actual state.  If you pass any
        other state, the result will be the accumulated state,
        esp. AUTO_RESIZE_AMBIGUOUS, if the value was NO before, and is YES for
        the current property set, or the other way round.
     */
    static void getAutoResizeFromPropSet(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet > & xProp,
        ReferenceSizeType eType,
        AutoResizeState & rInOutState );

    /** calls getAutoResizeFromPropSet with the title's property set
     */
    static void getAutoResizeFromTitle(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XTitle > & xTitle,
        AutoResizeState & rInOutState );

private:
    void impl_setValuesAtTitled(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XTitled > & xTitled );
    static void impl_getAutoResizeFromTitled(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XTitled > & xTitled,
        AutoResizeState & rInOutState );

    ::com::sun::star::awt::Size m_aPageSize;
    ::com::sun::star::awt::Size m_aDiagramSize;
    ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XChartDocument > m_xChartDoc;
    bool      m_bUseAutoScale;
};

} //  namespace chart

// CHART2_REFERENCESIZEPROVIDER_HXX
#endif

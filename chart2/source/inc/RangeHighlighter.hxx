/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: RangeHighlighter.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 17:06:01 $
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
#ifndef CHART2_RANGEHIGHLIGHTER_HXX
#define CHART2_RANGEHIGHLIGHTER_HXX

#ifndef CHART_MUTEXCONTAINER_HXX
#include "MutexContainer.hxx"
#endif

#ifndef _CPPUHELPER_COMPBASE2_HXX_
#include <cppuhelper/compbase2.hxx>
#endif

#ifndef _COM_SUN_STAR_CHART2_DATA_XRANGEHIGHLIGHTER_HPP_
#include <com/sun/star/chart2/data/XRangeHighlighter.hpp>
#endif
#ifndef _COM_SUN_STAR_VIEW_XSELECTIONSUPPLIER_HPP_
#include <com/sun/star/view/XSelectionSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

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
};

} //  namespace chart

// CHART2_RANGEHIGHLIGHTER_HXX
#endif

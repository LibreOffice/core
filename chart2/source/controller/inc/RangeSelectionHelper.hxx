/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: RangeSelectionHelper.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 16:46:43 $
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
#ifndef CHART2_RANGESELECTIONHELPER_HXX
#define CHART2_RANGESELECTIONHELPER_HXX

#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>

namespace com { namespace sun { namespace star {
    namespace sheet{
        class XRangeSelection;
        class XRangeSelectionListener;
        class XCellRangesAccess;
    }
}}}

namespace chart
{

class RangeSelectionListenerParent;

class RangeSelectionHelper
{
public:
    explicit RangeSelectionHelper(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XChartDocument > & xChartDocument );
    ~RangeSelectionHelper();

    bool hasRangeSelection();
    ::com::sun::star::uno::Reference<
            ::com::sun::star::sheet::XRangeSelection > getRangeSelection();
    ::com::sun::star::uno::Reference<
            ::com::sun::star::sheet::XCellRangesAccess > getCellRangesAccess();
    void raiseRangeSelectionDocument();
    bool chooseRange(
        const ::rtl::OUString & aCurrentRange,
        const ::rtl::OUString & aUIString,
        RangeSelectionListenerParent & rListenerParent );
    void stopRangeListening( bool bRemoveListener = true );
    bool verifyCellRange( const ::rtl::OUString & rRangeStr );
    bool verifyArguments( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& rArguments );
    ::rtl::OUString getCellRangeContent( const ::rtl::OUString & rRangeStr );

private:
    ::com::sun::star::uno::Reference<
            ::com::sun::star::sheet::XRangeSelection >
        m_xRangeSelection;

    ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XChartDocument >
        m_xChartDocument;

    ::com::sun::star::uno::Reference<
            ::com::sun::star::sheet::XCellRangesAccess >
        m_xCellRangesAccess;

    ::com::sun::star::uno::Reference<
            ::com::sun::star::sheet::XRangeSelectionListener >
        m_xRangeSelectionListener;
};

} //  namespace chart

// CHART2_RANGESELECTIONHELPER_HXX
#endif

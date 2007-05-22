/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ChartDropTargetHelper.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 18:04:50 $
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
#ifndef CHART2_CHARTDROPTARGETHELPER_HXX
#define CHART2_CHARTDROPTARGETHELPER_HXX

// for DropTargetHelper
#ifndef _TRANSFER_HXX
#include <svtools/transfer.hxx>
#endif

namespace com { namespace sun { namespace star {
namespace chart2 {
    class XChartDocument;
}
}}}

namespace chart
{

class ChartDropTargetHelper : public DropTargetHelper
{
public:
    explicit ChartDropTargetHelper(
        const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::dnd::XDropTarget >& rxDropTarget,
        const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XChartDocument > & xChartDocument );
    virtual ~ChartDropTargetHelper();

protected:

    virtual sal_Int8    AcceptDrop( const AcceptDropEvent& rEvt );
    virtual sal_Int8    ExecuteDrop( const ExecuteDropEvent& rEvt );

private:
    // not available
    ChartDropTargetHelper();

    bool satisfiesPrerequisites() const;


    ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XChartDocument > m_xChartDocument;
};

} //  namespace chart

// CHART2_CHARTDROPTARGETHELPER_HXX
#endif

/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ChartItemPool.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 19:22:35 $
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
#ifndef _CHART2_ITEMPOOL_HXX
#define _CHART2_ITEMPOOL_HXX

#ifndef _SFXPOOLITEM_HXX
#include <svtools/poolitem.hxx>
#endif
#ifndef _SFXITEMPOOL_HXX
#include <svtools/itempool.hxx>
#endif

namespace chart
{
class ChartItemPool : public SfxItemPool
{
private:
    SfxPoolItem**   ppPoolDefaults;
    SfxItemInfo*    pItemInfos;

public:
    ChartItemPool();
    ChartItemPool(const ChartItemPool& rPool);
    virtual ~ChartItemPool();

    virtual SfxItemPool* Clone() const;
    SfxMapUnit GetMetric( USHORT nWhich ) const;

    /// creates a pure chart item pool
    static SfxItemPool* CreateChartItemPool();
};

/** global function to get a singleton ChartItemPool
 */
ChartItemPool * GetChartItemPool();

} //  namespace chart

#endif
// _CHART2_ITEMPOOL_HXX

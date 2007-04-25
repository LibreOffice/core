/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vbapivotcache.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-04-25 16:09:19 $
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
#ifndef SC_VBA_PIVOTCACHE_HXX
#define SC_VBA_PIVOTCACHE_HXX
#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/sheet/XDataPilotTable.hpp>

#include <org/openoffice/excel/XPivotCache.hpp>
#include "vbahelper.hxx"

typedef ::cppu::WeakImplHelper1<oo::excel::XPivotCache > PivotCacheImpl_BASE;

class ScVbaPivotCache : public PivotCacheImpl_BASE
{
    css::uno::Reference< css::uno::XComponentContext > m_xContext;
    css::uno::Reference< css::sheet::XDataPilotTable > m_xTable;

public:
    ScVbaPivotCache( const css::uno::Reference< css::uno::XComponentContext >& xContext, const css::uno::Reference< css::sheet::XDataPilotTable >& xTable );
    virtual void SAL_CALL Refresh() throw (css::uno::RuntimeException);
};

#endif //SC_VBA_PIVOTCACHE_HXX

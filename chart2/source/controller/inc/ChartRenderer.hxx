/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ChartRenderer.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ihi $ $Date: 2008-01-14 13:57:28 $
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
#ifndef CHART_RENDERER_HXX
#define CHART_RENDERER_HXX

#include <svx/chartprettypainter.hxx>
#include "ServiceMacros.hxx"

#include <cppuhelper/implbase2.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#ifndef _CPPUHELPER_WEAKREF_HXX_
#include <cppuhelper/weakref.hxx>
#endif

//.............................................................................
namespace chart
{
//.............................................................................

//#i82893#, #i75867#: charts must be painted resolution dependent!!
class ChartRenderer : public ::cppu::WeakImplHelper2<
          ::com::sun::star::lang::XServiceInfo
        , ::com::sun::star::lang::XUnoTunnel
        >, public ChartPrettyPainter
{
public:
    ChartRenderer( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& xChartModel );
    ChartRenderer( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& xContext);
    virtual ~ChartRenderer();

    // ___lang::XServiceInfo___
    APPHELPER_XSERVICEINFO_DECL()
    APPHELPER_SERVICE_FACTORY_HELPER(ChartRenderer)

    // ____ XUnoTunnel ___
    virtual ::sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< ::sal_Int8 >& aIdentifier )
            throw (::com::sun::star::uno::RuntimeException);

    // ____ ChartPrettyPainter ___
    virtual bool DoPaint(OutputDevice* pOutDev, const Rectangle& rLogicObjectRect) const;

private:
    ChartRenderer();
    ChartRenderer( const ChartRenderer& );

    ::com::sun::star::uno::WeakReference< ::com::sun::star::frame::XModel > m_xChartModel;
};

//.............................................................................
}  // namespace chart
//.............................................................................

#endif

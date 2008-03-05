/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ReportComponent.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:55:33 $
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

#ifndef RPT_REPORTCOMPONENT_HXX
#define RPT_REPORTCOMPONENT_HXX

#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCHILD_HPP_
#include <com/sun/star/container/XChild.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XSHAPE_HPP_
#include <com/sun/star/drawing/XShape.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XTYPEPROVIDER_HPP_
#include <com/sun/star/lang/XTypeProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XUNOTUNNEL_HPP_
#include <com/sun/star/lang/XUnoTunnel.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_XAGGREGATION_HPP_
#include <com/sun/star/uno/XAggregation.hpp>
#endif
#ifndef _COM_SUN_STAR_REPORT_XREPORTCOMPONENT_HPP_
#include <com/sun/star/report/XReportComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _CPPUHELPER_WEAKREF_HXX_
#include <cppuhelper/weakref.hxx>
#endif
#ifndef _COMPHELPER_UNO3_HXX_
#include <comphelper/uno3.hxx>
#endif

namespace reportdesign
{
    class OReportComponentProperties
    {
    public:
        ::com::sun::star::uno::WeakReference< ::com::sun::star::container::XChild > m_xParent;
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > m_xContext;
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >
                                                                                    m_xFactory;
        ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >       m_xShape;
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XAggregation >     m_xProxy;
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >   m_xProperty;
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XTypeProvider >   m_xTypeProvider;
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XUnoTunnel >      m_xUnoTunnel;
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XServiceInfo >    m_xServiceInfo;
        ::com::sun::star::uno::Sequence< ::rtl::OUString >                          m_aMasterFields;
        ::com::sun::star::uno::Sequence< ::rtl::OUString >                          m_aDetailFields;
        ::rtl::OUString                                                             m_sName;
        ::sal_Int32                                                                 m_nHeight;
        ::sal_Int32                                                                 m_nWidth;
        ::sal_Int32                                                                 m_nPosX;
        ::sal_Int32                                                                 m_nPosY;
        ::sal_Int32                                                                 m_nBorderColor;
        ::sal_Int16                                                                 m_nBorder;
        ::sal_Bool                                                                  m_bPrintRepeatedValues;

        OReportComponentProperties(::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > const & _xContext
            )
            :m_xContext(_xContext)
            ,m_nHeight(0)
            ,m_nWidth(0)
            ,m_nPosX(0)
            ,m_nPosY(0)
            ,m_nBorderColor(0)
            ,m_nBorder(2)
            ,m_bPrintRepeatedValues(sal_True)
        {}
        ~OReportComponentProperties();

        void setShape(::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& _xShape
                    ,const ::com::sun::star::uno::Reference< ::com::sun::star::report::XReportComponent>& _xTunnel
                    ,oslInterlockedCount& _rRefCount);

        void dispose(oslInterlockedCount& _rRefCount);
    };
}
#endif // RPT_REPORTCOMPONENT_HXX


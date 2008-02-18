/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DiagramWrapper.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2008-02-18 15:38:58 $
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
#ifndef CHART_DIAGRAMWRAPPER_HXX
#define CHART_DIAGRAMWRAPPER_HXX

#include "WrappedPropertySet.hxx"
#include "ServiceMacros.hxx"
#include "DiagramHelper.hxx"

#ifndef _CPPUHELPER_IMPLBASE10_HXX_
#include <cppuhelper/implbase10.hxx>
#endif
#ifndef _COMPHELPER_UNO3_HXX_
#include <comphelper/uno3.hxx>
#endif
#ifndef _CPPUHELPER_INTERFACECONTAINER_HXX_
#include <cppuhelper/interfacecontainer.hxx>
#endif

#ifndef _COM_SUN_STAR_CHART2_XCHARTDOCUMENT_HPP_
#include <com/sun/star/chart2/XChartDocument.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_XDIAGRAM_HPP_
#include <com/sun/star/chart2/XDiagram.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_XDIAGRAMPROVIDER_HPP_
#include <com/sun/star/chart2/XDiagramProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_XCHARTTYPETEMPLATE_HPP_
#include <com/sun/star/chart2/XChartTypeTemplate.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_XCHARTTYPEMANAGER_HPP_
#include <com/sun/star/chart2/XChartTypeManager.hpp>
#endif

#ifndef _COM_SUN_STAR_CHART_XDIAGRAM_HPP_
#include <com/sun/star/chart/XDiagram.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART_XAXISZSUPPLIER_HPP_
#include <com/sun/star/chart/XAxisZSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART_XTWOAXISXSUPPLIER_HPP_
#include <com/sun/star/chart/XTwoAxisXSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART_XTWOAXISYSUPPLIER_HPP_
#include <com/sun/star/chart/XTwoAxisYSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART_XSTATISTICDISPLAY_HPP_
#include <com/sun/star/chart/XStatisticDisplay.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART_X3DDISPLAY_HPP_
#include <com/sun/star/chart/X3DDisplay.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XEVENTLISTENER_HPP_
#include <com/sun/star/lang/XEventListener.hpp>
#endif

#ifndef _COM_SUN_STAR_CHART_XSECONDAXISTITLESUPPLIER_HPP_
#include <com/sun/star/chart/XSecondAxisTitleSupplier.hpp>
#endif

#include <boost/shared_ptr.hpp>

namespace chart
{

namespace wrapper
{

class Chart2ModelContact;

class DiagramWrapper : public ::cppu::ImplInheritanceHelper10<
                      WrappedPropertySet
                     , ::com::sun::star::chart::XDiagram
                     , ::com::sun::star::chart::XAxisZSupplier
                     , ::com::sun::star::chart::XTwoAxisXSupplier   //  : XAxisXSupplier
                     , ::com::sun::star::chart::XTwoAxisYSupplier   //  : XAxisYSupplier
                     , ::com::sun::star::chart::XStatisticDisplay
                     , ::com::sun::star::chart::X3DDisplay
                     , ::com::sun::star::lang::XServiceInfo
                        , ::com::sun::star::lang::XComponent
//                      , ::com::sun::star::lang::XEventListener
                     , ::com::sun::star::chart2::XDiagramProvider
                     , ::com::sun::star::chart::XSecondAxisTitleSupplier
                    >
{
public:
    DiagramWrapper( ::boost::shared_ptr< Chart2ModelContact > spChart2ModelContact );
    virtual ~DiagramWrapper();

    /// XServiceInfo declarations
    APPHELPER_XSERVICEINFO_DECL()

    // ____ XComponent ____
    virtual void SAL_CALL dispose()
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference<
                                            ::com::sun::star::lang::XEventListener >& xListener )
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference<
                                               ::com::sun::star::lang::XEventListener >& aListener )
        throw (::com::sun::star::uno::RuntimeException);

    // ____ XDiagram ____
    virtual ::rtl::OUString SAL_CALL getDiagramType()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet > SAL_CALL getDataRowProperties( sal_Int32 nRow )
        throw (::com::sun::star::lang::IndexOutOfBoundsException,
               ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet > SAL_CALL getDataPointProperties( sal_Int32 nCol, sal_Int32 nRow )
        throw (::com::sun::star::lang::IndexOutOfBoundsException,
               ::com::sun::star::uno::RuntimeException);

    // ____ XShape (base of XDiagram) ____
    virtual ::com::sun::star::awt::Point SAL_CALL getPosition()
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPosition( const ::com::sun::star::awt::Point& aPosition )
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::awt::Size SAL_CALL getSize()
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setSize( const ::com::sun::star::awt::Size& aSize )
        throw (::com::sun::star::beans::PropertyVetoException,
               ::com::sun::star::uno::RuntimeException);

    // ____ XShapeDescriptor (base of XShape) ____
    virtual ::rtl::OUString SAL_CALL getShapeType()
        throw (::com::sun::star::uno::RuntimeException);

    // ____ XAxisZSupplier ____
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::drawing::XShape > SAL_CALL getZAxisTitle()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet > SAL_CALL getZMainGrid()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet > SAL_CALL getZHelpGrid()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet > SAL_CALL getZAxis()
        throw (::com::sun::star::uno::RuntimeException);

    // ____ XTwoAxisXSupplier ____
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet > SAL_CALL getSecondaryXAxis()
        throw (::com::sun::star::uno::RuntimeException);

    // ____ XAxisXSupplier (base of XTwoAxisXSupplier) ____
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::drawing::XShape > SAL_CALL getXAxisTitle()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet > SAL_CALL getXAxis()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet > SAL_CALL getXMainGrid()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet > SAL_CALL getXHelpGrid()
        throw (::com::sun::star::uno::RuntimeException);

    // ____ XTwoAxisYSupplier ____
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet > SAL_CALL getSecondaryYAxis()
        throw (::com::sun::star::uno::RuntimeException);

    // ____ XAxisYSupplier (base of XTwoAxisYSupplier) ____
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::drawing::XShape > SAL_CALL getYAxisTitle()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet > SAL_CALL getYAxis()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet > SAL_CALL getYHelpGrid()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet > SAL_CALL getYMainGrid()
        throw (::com::sun::star::uno::RuntimeException);

   // ____ XSecondAxisTitleSupplier ____
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::drawing::XShape > SAL_CALL getSecondXAxisTitle()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::drawing::XShape > SAL_CALL getSecondYAxisTitle()
        throw (::com::sun::star::uno::RuntimeException);

    // ____ XStatisticDisplay ____
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet > SAL_CALL getUpBar()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet > SAL_CALL getDownBar()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet > SAL_CALL getMinMaxLine()
        throw (::com::sun::star::uno::RuntimeException);

    // ____ X3DDisplay ____
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet > SAL_CALL getWall()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet > SAL_CALL getFloor()
        throw (::com::sun::star::uno::RuntimeException);

//     // ____ XEventListener ____
//     virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source )
//         throw (::com::sun::star::uno::RuntimeException);

    // ____ XDiagramProvider ____
    virtual ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XDiagram > SAL_CALL getDiagram()
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setDiagram( const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::chart2::XDiagram >& xDiagram )
        throw (::com::sun::star::uno::RuntimeException);

protected:
    // ____ WrappedPropertySet ____
    virtual const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property >& getPropertySequence();
    virtual const std::vector< WrappedProperty* > createWrappedProperties();
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > getInnerPropertySet();

private:
    void updateFromModel();

    ::boost::shared_ptr< Chart2ModelContact >   m_spChart2ModelContact;
    ::cppu::OInterfaceContainerHelper           m_aEventListenerContainer;

    ::com::sun::star::uno::Reference<
        ::com::sun::star::drawing::XShape >
                        m_xXAxisTitle;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::drawing::XShape >
                        m_xYAxisTitle;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::drawing::XShape >
                        m_xZAxisTitle;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::drawing::XShape >
                        m_xSecondXAxisTitle;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::drawing::XShape >
                        m_xSecondYAxisTitle;

    ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet >
                        m_xXAxis;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet >
                        m_xYAxis;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet >
                        m_xZAxis;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet >
                        m_xSecondXAxis;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet >
                        m_xSecondYAxis;

    ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet >
                        m_xXMainGrid;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet >
                        m_xYMainGrid;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet >
                        m_xZMainGrid;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet >
                        m_xXHelpGrid;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet >
                        m_xYHelpGrid;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet >
                        m_xZHelpGrid;

    ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet >
                        m_xWall;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet >
                        m_xFloor;

    ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet >
                        m_xMinMaxLineWrapper;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet >
                        m_xUpBarWrapper;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet >
                        m_xDownBarWrapper;
};

} //  namespace wrapper
} //  namespace chart

// CHART_DIAGRAMWRAPPER_HXX
#endif

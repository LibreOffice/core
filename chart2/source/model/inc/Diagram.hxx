/*************************************************************************
 *
 *  $RCSfile: Diagram.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: iha $ $Date: 2003-11-10 17:49:50 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2003 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef CHART_DIAGRAM_HXX
#define CHART_DIAGRAM_HXX

#ifndef CHART_OPROPERTYSET_HXX
#include "OPropertySet.hxx"
#endif
#ifndef CHART_MUTEXCONTAINER_HXX
#include "MutexContainer.hxx"
#endif

#ifndef _CPPUHELPER_IMPLBASE8_HXX_
#include <cppuhelper/implbase8.hxx>
#endif
#ifndef _COMPHELPER_UNO3_HXX_
#include <comphelper/uno3.hxx>
#endif

#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_CHART2_XDIAGRAM_HPP_
#include <drafts/com/sun/star/chart2/XDiagram.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_CHART2_XCHARTELEMENT_HPP_
#include <drafts/com/sun/star/chart2/XChartElement.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_CHART2_XAXISCONTAINER_HPP_
#include <drafts/com/sun/star/chart2/XAxisContainer.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_CHART2_XBOUNDEDCOORDINATESYSTEMCONTAINER_HPP_
#include <drafts/com/sun/star/chart2/XBoundedCoordinateSystemContainer.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_CHART2_XGRIDCONTAINER_HPP_
#include <drafts/com/sun/star/chart2/XGridContainer.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_CHART2_XTITLED_HPP_
#include <drafts/com/sun/star/chart2/XTitled.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_LAYOUT_XLAYOUTELEMENT_HPP_
#include <drafts/com/sun/star/layout/XLayoutElement.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_CHART2_XIDENTIFIABLE_HPP_
#include <drafts/com/sun/star/chart2/XIdentifiable.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif

#include "ServiceMacros.hxx"

#include <map>
// #include <set>
#include <vector>

namespace chart
{

namespace impl
{
typedef ::cppu::WeakImplHelper8<
    ::drafts::com::sun::star::chart2::XDiagram,
    ::com::sun::star::lang::XServiceInfo,
    ::drafts::com::sun::star::chart2::XAxisContainer,
    ::drafts::com::sun::star::chart2::XBoundedCoordinateSystemContainer,
    ::drafts::com::sun::star::chart2::XGridContainer,
    ::drafts::com::sun::star::chart2::XTitled,
    ::drafts::com::sun::star::layout::XLayoutElement,
    ::drafts::com::sun::star::chart2::XIdentifiable >
    Diagram_Base;
}

class Diagram :
    public helper::MutexContainer,
    public impl::Diagram_Base,
    public ::property::OPropertySet
{
public:
    Diagram( ::com::sun::star::uno::Reference<
             ::com::sun::star::uno::XComponentContext > const & xContext );
    virtual ~Diagram();

    /// establish methods for factory instatiation
    APPHELPER_SERVICE_FACTORY_HELPER( Diagram )

    /// XServiceInfo declarations
    APPHELPER_XSERVICEINFO_DECL()

    /// merge XInterface implementations
     DECLARE_XINTERFACE()
    /// merge XTypeProvider implementations
     DECLARE_XTYPEPROVIDER()

protected:
    // ____ OPropertySet ____
    virtual ::com::sun::star::uno::Any GetDefaultValue( sal_Int32 nHandle ) const
        throw(::com::sun::star::beans::UnknownPropertyException);

    // ____ OPropertySet ____
    virtual ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper();

    // ____ XPropertySet ____
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL
        getPropertySetInfo()
        throw (::com::sun::star::uno::RuntimeException);

//  virtual sal_Bool SAL_CALL convertFastPropertyValue
//         ( ::com::sun::star::uno::Any & rConvertedValue,
//           ::com::sun::star::uno::Any & rOldValue,
//           sal_Int32 nHandle,
//           const ::com::sun::star::uno::Any& rValue )
//      throw (::com::sun::star::lang::IllegalArgumentException);

    // ____ XDiagram ____
    virtual ::com::sun::star::uno::Reference< ::drafts::com::sun::star::chart2::XDataSeriesTreeParent >
        SAL_CALL getTree()
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setTree(
        const ::com::sun::star::uno::Reference< ::drafts::com::sun::star::chart2::XDataSeriesTreeParent >& xTree )
        throw (::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::uno::RuntimeException);
//     virtual ::rtl::OUString SAL_CALL getChartTypeTemplateServiceName()
//         throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet > SAL_CALL getWall()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet > SAL_CALL getFloor()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference<
        ::drafts::com::sun::star::chart2::XLegend > SAL_CALL getLegend()
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setLegend( const ::com::sun::star::uno::Reference<
                                     ::drafts::com::sun::star::chart2::XLegend >& xLegend )
        throw (::com::sun::star::uno::RuntimeException);

    // ____ XAxisContainer ____
    virtual void SAL_CALL addAxis(
        const ::com::sun::star::uno::Reference<
        ::drafts::com::sun::star::chart2::XAxis >& aAxis )
        throw (::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeAxis(
        const ::com::sun::star::uno::Reference<
        ::drafts::com::sun::star::chart2::XAxis >& aAxis )
        throw (::com::sun::star::container::NoSuchElementException,
               ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence<
        ::com::sun::star::uno::Reference<
        ::drafts::com::sun::star::chart2::XAxis > > SAL_CALL getAxes()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference<
        ::drafts::com::sun::star::chart2::XAxis > SAL_CALL getAxisByIdentifier(
            const ::rtl::OUString& aIdentifier )
        throw (::com::sun::star::uno::RuntimeException);

    // ____ XBoundedCoordinateSystemContainer ____
    virtual void SAL_CALL addCoordinateSystem(
        const ::com::sun::star::uno::Reference<
            ::drafts::com::sun::star::chart2::XBoundedCoordinateSystem >& aCoordSys )
        throw (::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeCoordinateSystem(
        const ::com::sun::star::uno::Reference<
            ::drafts::com::sun::star::chart2::XBoundedCoordinateSystem >& aCoordSys )
        throw (::com::sun::star::container::NoSuchElementException,
               ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence<
        ::com::sun::star::uno::Reference<
        ::drafts::com::sun::star::chart2::XBoundedCoordinateSystem > > SAL_CALL getCoordinateSystems()
        throw (::com::sun::star::uno::RuntimeException);

    // ____ XGridContainer ____
    virtual void SAL_CALL addGrid(
        const ::com::sun::star::uno::Reference<
        ::drafts::com::sun::star::chart2::XGrid >& aGrid )
        throw (::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeGrid(
        const ::com::sun::star::uno::Reference<
        ::drafts::com::sun::star::chart2::XGrid >& aGrid )
        throw (::com::sun::star::container::NoSuchElementException,
               ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence<
        ::com::sun::star::uno::Reference<
        ::drafts::com::sun::star::chart2::XGrid > > SAL_CALL getGrids()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference<
        ::drafts::com::sun::star::chart2::XGrid > SAL_CALL getGridByIdentifier(
            const ::rtl::OUString& aIdentifier )
        throw (::com::sun::star::uno::RuntimeException);

    // ____ XTitled ____
    virtual ::com::sun::star::uno::Reference<
        ::drafts::com::sun::star::chart2::XTitle > SAL_CALL getTitle()
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setTitle( const ::com::sun::star::uno::Reference<
                                    ::drafts::com::sun::star::chart2::XTitle >& Title )
        throw (::com::sun::star::uno::RuntimeException);

    // ____ XLayoutElement ____
    // (empty)

    // ____ XIdentifiable ____
    virtual ::rtl::OUString SAL_CALL getIdentifier()
        throw (::com::sun::star::uno::RuntimeException);

private:
     ::com::sun::star::uno::Reference<
         ::com::sun::star::uno::XComponentContext >                m_xContext;

//         ::std::vector< ::drafts::com::sun::star::chart2::DataSeriesGroup >
//             m_aSeriesGroups;
    ::com::sun::star::uno::Reference<
        ::drafts::com::sun::star::chart2::XDataSeriesTreeParent >  m_xSeriesTree;

    typedef
        ::std::map<
            ::rtl::OUString,
            ::com::sun::star::uno::Reference< ::drafts::com::sun::star::chart2::XAxis > >
        tAxisContainerType;
    typedef
        ::std::map<
            ::rtl::OUString,
            ::com::sun::star::uno::Reference< ::drafts::com::sun::star::chart2::XGrid > >
        tGridContainerType;

    tAxisContainerType  m_aAxes;
    tGridContainerType  m_aGrids;

//     ::std::set< ::com::sun::star::uno::Reference<
//         ::drafts::com::sun::star::chart2::XBoundedCoordinateSystem > >
//                         m_aCoordSystems;
    ::std::vector< ::com::sun::star::uno::Reference<
        ::drafts::com::sun::star::chart2::XBoundedCoordinateSystem > >
                        m_aCoordSystems;

    ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet >
                        m_xWall;

    ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet >
                        m_xFloor;

    ::com::sun::star::uno::Reference<
        ::drafts::com::sun::star::chart2::XTitle >
                        m_xTitle;

    ::rtl::OUString     m_aIdentifier;

    ::com::sun::star::uno::Reference<
        ::drafts::com::sun::star::chart2::XLegend >
                        m_xLegend;

    ::rtl::OUString m_aTemplateServiceName;
};

} //  namespace chart

// CHART_DIAGRAM_HXX
#endif

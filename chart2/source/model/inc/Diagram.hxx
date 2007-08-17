/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: Diagram.hxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: ihi $ $Date: 2007-08-17 12:13:53 $
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
#ifndef CHART_DIAGRAM_HXX
#define CHART_DIAGRAM_HXX

#ifndef CHART_OPROPERTYSET_HXX
#include "OPropertySet.hxx"
#endif
#ifndef CHART_MUTEXCONTAINER_HXX
#include "MutexContainer.hxx"
#endif

#ifndef _CPPUHELPER_IMPLBASE7_HXX_
#include <cppuhelper/implbase7.hxx>
#endif
#ifndef _COMPHELPER_UNO3_HXX_
#include <comphelper/uno3.hxx>
#endif

#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_XDIAGRAM_HPP_
#include <com/sun/star/chart2/XDiagram.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_XCOORDINATESYSTEMCONTAINER_HPP_
#include <com/sun/star/chart2/XCoordinateSystemContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_XTITLED_HPP_
#include <com/sun/star/chart2/XTitled.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XCLONEABLE_HPP_
#include <com/sun/star/util/XCloneable.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XMODIFYBROADCASTER_HPP_
#include <com/sun/star/util/XModifyBroadcaster.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XMODIFYLISTENER_HPP_
#include <com/sun/star/util/XModifyListener.hpp>
#endif

#include "ServiceMacros.hxx"
#include "ModifyListenerHelper.hxx"

#include <map>
#include <vector>

namespace chart
{

namespace impl
{
typedef ::cppu::WeakImplHelper7<
    ::com::sun::star::chart2::XDiagram,
    ::com::sun::star::lang::XServiceInfo,
    ::com::sun::star::chart2::XCoordinateSystemContainer,
    ::com::sun::star::chart2::XTitled,
    ::com::sun::star::util::XModifyBroadcaster,
    ::com::sun::star::util::XModifyListener,
    ::com::sun::star::util::XCloneable >
    Diagram_Base;
}

class Diagram :
    public MutexContainer,
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
    explicit Diagram( const Diagram & rOther );

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
//     virtual ::rtl::OUString SAL_CALL getChartTypeTemplateServiceName()
//         throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet > SAL_CALL getWall()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet > SAL_CALL getFloor()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::XLegend > SAL_CALL getLegend()
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setLegend( const ::com::sun::star::uno::Reference<
                                     ::com::sun::star::chart2::XLegend >& xLegend )
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XColorScheme > SAL_CALL getDefaultColorScheme()
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setDefaultColorScheme(
        const ::com::sun::star::uno::Reference< ::com::sun::star::chart2::XColorScheme >& xColorScheme )
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setUnusedData(
        const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XLabeledDataSequence > >& aUnusedData )
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XLabeledDataSequence > > SAL_CALL getUnusedData()
        throw (::com::sun::star::uno::RuntimeException);


    // ____ XCoordinateSystemContainer ____
    virtual void SAL_CALL addCoordinateSystem(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XCoordinateSystem >& aCoordSys )
        throw (::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeCoordinateSystem(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XCoordinateSystem >& aCoordSys )
        throw (::com::sun::star::container::NoSuchElementException,
               ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence<
        ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::XCoordinateSystem > > SAL_CALL getCoordinateSystems()
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setCoordinateSystems(
        const ::com::sun::star::uno::Sequence<
            ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::XCoordinateSystem > >& aCoordinateSystems )
        throw (::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::uno::RuntimeException);

    // ____ XTitled ____
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::XTitle > SAL_CALL getTitleObject()
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setTitleObject( const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::chart2::XTitle >& Title )
        throw (::com::sun::star::uno::RuntimeException);

    // ____ XCloneable ____
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::util::XCloneable > SAL_CALL createClone()
        throw (::com::sun::star::uno::RuntimeException);

    // ____ XModifyBroadcaster ____
    virtual void SAL_CALL addModifyListener(
        const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& aListener )
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeModifyListener(
        const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& aListener )
        throw (::com::sun::star::uno::RuntimeException);

    // ____ XModifyListener ____
    virtual void SAL_CALL modified(
        const ::com::sun::star::lang::EventObject& aEvent )
        throw (::com::sun::star::uno::RuntimeException);

    // ____ XEventListener (base of XModifyListener) ____
    virtual void SAL_CALL disposing(
        const ::com::sun::star::lang::EventObject& Source )
        throw (::com::sun::star::uno::RuntimeException);

    // ____ OPropertySet ____
    virtual void firePropertyChangeEvent();

    void fireModifyEvent();

private:
     ::com::sun::star::uno::Reference<
         ::com::sun::star::uno::XComponentContext >                m_xContext;

    typedef
        ::std::vector< ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XCoordinateSystem > >
        tCoordinateSystemContainerType;

    tCoordinateSystemContainerType m_aCoordSystems;

    ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet >
                        m_xWall;

    ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet >
                        m_xFloor;

    ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::XTitle >
                        m_xTitle;

    ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::XLegend >
                        m_xLegend;

    ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::XColorScheme >
                        m_xColorScheme;

    ::com::sun::star::uno::Sequence<
            ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::data::XLabeledDataSequence > >
                        m_aUnusedData;

    ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener > m_xModifyEventForwarder;
};

} //  namespace chart

// CHART_DIAGRAM_HXX
#endif

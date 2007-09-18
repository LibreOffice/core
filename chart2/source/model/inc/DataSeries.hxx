/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DataSeries.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: vg $ $Date: 2007-09-18 14:58:27 $
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
#ifndef _CHART_DATASERIES_HXX
#define _CHART_DATASERIES_HXX

// UNO types
#ifndef _COM_SUN_STAR_CHART2_XDATASERIES_HPP_
#include <com/sun/star/chart2/XDataSeries.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_DATA_XDATASINK_HPP_
#include <com/sun/star/chart2/data/XDataSink.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_DATA_XDATASOURCE_HPP_
#include <com/sun/star/chart2/data/XDataSource.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_XREGRESSIONCURVECONTAINER_HPP_
#include <com/sun/star/chart2/XRegressionCurveContainer.hpp>
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

#ifndef _COM_SUN_STAR_CONTAINER_XINDEXCONTAINER_HPP_
#include <com/sun/star/container/XIndexContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif

// helper classes
#include "ServiceMacros.hxx"
#ifndef _CPPUHELPER_IMPLBASE8_HXX_
#include <cppuhelper/implbase8.hxx>
#endif
#ifndef _COMPHELPER_UNO3_HXX_
#include <comphelper/uno3.hxx>
#endif
#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

// STL
#include <vector>
#include <map>

#include "MutexContainer.hxx"
#include "OPropertySet.hxx"

namespace com { namespace sun { namespace star { namespace style {
    class XStyle;
}}}}

namespace chart
{

namespace impl
{
typedef ::cppu::WeakImplHelper8<
        ::com::sun::star::chart2::XDataSeries,
        ::com::sun::star::chart2::data::XDataSink,
        ::com::sun::star::chart2::data::XDataSource,
        ::com::sun::star::lang::XServiceInfo,
        ::com::sun::star::chart2::XRegressionCurveContainer,
        ::com::sun::star::util::XCloneable,
        ::com::sun::star::util::XModifyBroadcaster,
        ::com::sun::star::util::XModifyListener >
    DataSeries_Base;
}

class DataSeries :
    public MutexContainer,
    public impl::DataSeries_Base,
    public ::property::OPropertySet
{
public:
    explicit DataSeries(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::uno::XComponentContext > & xContext );
    virtual ~DataSeries();

    /// establish methods for factory instatiation
    APPHELPER_SERVICE_FACTORY_HELPER( DataSeries )
    /// XServiceInfo declarations
    APPHELPER_XSERVICEINFO_DECL()

    /// merge XInterface implementations
    DECLARE_XINTERFACE()
    /// merge XTypeProvider implementations
    DECLARE_XTYPEPROVIDER()

protected:
    explicit DataSeries( const DataSeries & rOther );

    // late initialization to call after copy-constructing
    void Init( const DataSeries & rOther );

    // ____ XDataSeries ____
    // _____________________
    /// @see ::com::sun::star::chart2::XDataSeries
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
        SAL_CALL getDataPointByIndex( sal_Int32 nIndex )
        throw (::com::sun::star::lang::IndexOutOfBoundsException,
               ::com::sun::star::uno::RuntimeException);

    // ____ XDataSink ____
    // ___________________
    /// @see ::com::sun::star::chart2::data::XDataSink
    virtual void SAL_CALL setData( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XLabeledDataSequence > >& aData )
        throw (::com::sun::star::uno::RuntimeException);

    // ____ XDataSource ____
    // _____________________
    /// @see ::com::sun::star::chart2::data::XDataSource
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::chart2::data::XLabeledDataSequence > > SAL_CALL getDataSequences()
        throw (::com::sun::star::uno::RuntimeException);

    // ____ OPropertySet ____
    // ______________________
    virtual ::com::sun::star::uno::Any GetDefaultValue( sal_Int32 nHandle ) const
        throw(::com::sun::star::beans::UnknownPropertyException);
    virtual void SAL_CALL getFastPropertyValue( ::com::sun::star::uno::Any& rValue, sal_Int32 nHandle ) const;
    virtual void SAL_CALL setFastPropertyValue_NoBroadcast
        ( sal_Int32 nHandle,
          const ::com::sun::star::uno::Any& rValue )
        throw (::com::sun::star::uno::Exception);

    virtual ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper();

    // ____ XPropertySet ____
    // ______________________
    /// @see ::com::sun::star::beans::XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL
        getPropertySetInfo()
        throw (::com::sun::star::uno::RuntimeException);

    /// make original interface function visible again
    using ::com::sun::star::beans::XFastPropertySet::getFastPropertyValue;

    // ____ XRegressionCurveContainer ____
    // ___________________________________
    /// @see ::com::sun::star::chart2::XRegressionCurveContainer
    virtual void SAL_CALL addRegressionCurve(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XRegressionCurve >& aRegressionCurve )
        throw (::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeRegressionCurve(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XRegressionCurve >& aRegressionCurve )
        throw (::com::sun::star::container::NoSuchElementException,
               ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence<
                ::com::sun::star::uno::Reference<
                    ::com::sun::star::chart2::XRegressionCurve > > SAL_CALL getRegressionCurves()
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setRegressionCurves(
        const ::com::sun::star::uno::Sequence<
            ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::XRegressionCurve > >& aRegressionCurves )
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
    using OPropertySet::disposing;

    void fireModifyEvent();

    /** const variant of getInfoHelper()
     */
    ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelperConst() const;

private:
    ::com::sun::star::uno::Reference<
        ::com::sun::star::uno::XComponentContext >
                        m_xContext;
    typedef ::std::vector< ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::data::XLabeledDataSequence > > tDataSequenceContainer;
    tDataSequenceContainer        m_aDataSequences;

    typedef ::std::map< sal_Int32,
        ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet > > tDataPointAttributeContainer;
    tDataPointAttributeContainer  m_aAttributedDataPoints;

    typedef
        ::std::vector< ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XRegressionCurve > >
        tRegressionCurveContainerType;
    tRegressionCurveContainerType m_aRegressionCurves;

    ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener > m_xModifyEventForwarder;
};

}  // namespace chart

// _CHART_DATASERIES_HXX
#endif

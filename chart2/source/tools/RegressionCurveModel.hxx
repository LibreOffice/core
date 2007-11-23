/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: RegressionCurveModel.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-23 12:08:07 $
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
#ifndef CHART2_REGRESSIONCURVEMODEL_HXX
#define CHART2_REGRESSIONCURVEMODEL_HXX

#include "MutexContainer.hxx"
#include "OPropertySet.hxx"
#include "ServiceMacros.hxx"
#include "ModifyListenerHelper.hxx"

#include <cppuhelper/implbase6.hxx>
#include <comphelper/uno3.hxx>

#include <com/sun/star/chart2/XRegressionCurve.hpp>

#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XServiceName.hpp>
#include <com/sun/star/util/XCloneable.hpp>

namespace chart
{

namespace impl
{
typedef ::cppu::WeakImplHelper6<
        ::com::sun::star::lang::XServiceInfo,
        ::com::sun::star::lang::XServiceName,
        ::com::sun::star::chart2::XRegressionCurve,
        ::com::sun::star::util::XCloneable,
        ::com::sun::star::util::XModifyBroadcaster,
        ::com::sun::star::util::XModifyListener >
    RegressionCurveModel_Base;
}

class RegressionCurveModel :
        public MutexContainer,
        public impl::RegressionCurveModel_Base,
        public ::property::OPropertySet
{
public:
    enum tCurveType
    {
        CURVE_TYPE_MEAN_VALUE,
        CURVE_TYPE_LINEAR,
        CURVE_TYPE_LOGARITHM,
        CURVE_TYPE_EXPONENTIAL,
        CURVE_TYPE_POWER
    };

    RegressionCurveModel( ::com::sun::star::uno::Reference<
                              ::com::sun::star::uno::XComponentContext > const & xContext,
                          tCurveType eCurveType );
    RegressionCurveModel( const RegressionCurveModel & rOther );
    virtual ~RegressionCurveModel();

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

    // ____ XRegressionCurve ____
    virtual ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XRegressionCurveCalculator > SAL_CALL getCalculator()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > SAL_CALL getEquationProperties()
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setEquationProperties(
        const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xEquationProperties )
        throw (::com::sun::star::uno::RuntimeException);

    // ____ XServiceName ____
    virtual ::rtl::OUString SAL_CALL getServiceName()
        throw (::com::sun::star::uno::RuntimeException);

    // ____ XCloneable ____
    // not implemented here
//     virtual ::com::sun::star::uno::Reference< ::com::sun::star::util::XCloneable > SAL_CALL createClone()
//         throw (::com::sun::star::uno::RuntimeException);

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

    using ::cppu::OPropertySetHelper::disposing;

    // ____ OPropertySet ____
    virtual void firePropertyChangeEvent();

    void fireModifyEvent();

private:
    ::com::sun::star::uno::Reference<
        ::com::sun::star::uno::XComponentContext >
                        m_xContext;

    const tCurveType    m_eRegressionCurveType;

    ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener > m_xModifyEventForwarder;
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > m_xEquationProperties;
};

// implementations for factory instantiation

class MeanValueRegressionCurve : public RegressionCurveModel
{
public:
    explicit MeanValueRegressionCurve(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::uno::XComponentContext > & xContext );
    explicit MeanValueRegressionCurve(
        const MeanValueRegressionCurve & rOther );
    virtual ~MeanValueRegressionCurve();

    // ____ XCloneable ____
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::util::XCloneable > SAL_CALL createClone()
        throw (::com::sun::star::uno::RuntimeException);

    /// XServiceInfo declarations
    APPHELPER_XSERVICEINFO_DECL()
    /// establish methods for factory instatiation
    APPHELPER_SERVICE_FACTORY_HELPER( MeanValueRegressionCurve )
};

class LinearRegressionCurve : public RegressionCurveModel
{
public:
    explicit LinearRegressionCurve(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::uno::XComponentContext > & xContext );
    explicit LinearRegressionCurve(
        const LinearRegressionCurve & rOther );
    virtual ~LinearRegressionCurve();

    // ____ XCloneable ____
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::util::XCloneable > SAL_CALL createClone()
        throw (::com::sun::star::uno::RuntimeException);

    /// XServiceInfo declarations
    APPHELPER_XSERVICEINFO_DECL()
    /// establish methods for factory instatiation
    APPHELPER_SERVICE_FACTORY_HELPER( LinearRegressionCurve )
};

class LogarithmicRegressionCurve : public RegressionCurveModel
{
public:
    explicit LogarithmicRegressionCurve(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::uno::XComponentContext > & xContext );
    explicit LogarithmicRegressionCurve(
        const LogarithmicRegressionCurve & rOther );
    virtual ~LogarithmicRegressionCurve();

    // ____ XCloneable ____
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::util::XCloneable > SAL_CALL createClone()
        throw (::com::sun::star::uno::RuntimeException);

    /// XServiceInfo declarations
    APPHELPER_XSERVICEINFO_DECL()
    /// establish methods for factory instatiation
    APPHELPER_SERVICE_FACTORY_HELPER( LogarithmicRegressionCurve )
};

class ExponentialRegressionCurve : public RegressionCurveModel
{
public:
    explicit ExponentialRegressionCurve(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::uno::XComponentContext > & xContext );
    explicit ExponentialRegressionCurve(
        const ExponentialRegressionCurve & rOther );
    virtual ~ExponentialRegressionCurve();

    // ____ XCloneable ____
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::util::XCloneable > SAL_CALL createClone()
        throw (::com::sun::star::uno::RuntimeException);

    /// XServiceInfo declarations
    APPHELPER_XSERVICEINFO_DECL()
    /// establish methods for factory instatiation
    APPHELPER_SERVICE_FACTORY_HELPER( ExponentialRegressionCurve )
};

class PotentialRegressionCurve : public RegressionCurveModel
{
public:
    explicit PotentialRegressionCurve(
        const ::com::sun::star::uno::Reference<
        ::com::sun::star::uno::XComponentContext > & xContext );
    explicit PotentialRegressionCurve(
        const PotentialRegressionCurve & rOther );
    virtual ~PotentialRegressionCurve();

    // ____ XCloneable ____
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::util::XCloneable > SAL_CALL createClone()
        throw (::com::sun::star::uno::RuntimeException);

    /// XServiceInfo declarations
    APPHELPER_XSERVICEINFO_DECL()
    /// establish methods for factory instatiation
    APPHELPER_SERVICE_FACTORY_HELPER( PotentialRegressionCurve )
};

} //  namespace chart

// CHART2_REGRESSIONCURVEMODEL_HXX
#endif

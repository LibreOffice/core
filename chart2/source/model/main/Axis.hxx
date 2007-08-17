/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: Axis.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: ihi $ $Date: 2007-08-17 12:14:21 $
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
#ifndef CHART_AXIS_HXX
#define CHART_AXIS_HXX

#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif

#ifndef CHART_MUTEXCONTAINER_HXX
#include "MutexContainer.hxx"
#endif
#ifndef CHART_OPROPERTYSET_HXX
#include "OPropertySet.hxx"
#endif

#ifndef _CPPUHELPER_IMPLBASE6_HXX_
#include <cppuhelper/implbase6.hxx>
#endif
#ifndef _COMPHELPER_UNO3_HXX_
#include <comphelper/uno3.hxx>
#endif

#include "ServiceMacros.hxx"
#include "ModifyListenerHelper.hxx"

#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_XAXIS_HPP_
#include <com/sun/star/chart2/XAxis.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_XTITLED_HPP_
#include <com/sun/star/chart2/XTitled.hpp>
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

namespace chart
{

namespace impl
{
typedef ::cppu::WeakImplHelper6<
        ::com::sun::star::chart2::XAxis,
        ::com::sun::star::chart2::XTitled,
        ::com::sun::star::lang::XServiceInfo,
        ::com::sun::star::util::XCloneable,
        ::com::sun::star::util::XModifyBroadcaster,
        ::com::sun::star::util::XModifyListener >
    Axis_Base;
}

class Axis :
    public MutexContainer,
    public impl::Axis_Base,
    public ::property::OPropertySet
{
public:
    Axis( ::com::sun::star::uno::Reference<
          ::com::sun::star::uno::XComponentContext > const & xContext );
    virtual ~Axis();

    /// establish methods for factory instatiation
    APPHELPER_SERVICE_FACTORY_HELPER( Axis )
    /// XServiceInfo declarations
    APPHELPER_XSERVICEINFO_DECL()

    /// merge XInterface implementations
     DECLARE_XINTERFACE()
    /// merge XTypeProvider implementations
     DECLARE_XTYPEPROVIDER()

protected:
    explicit Axis( const Axis & rOther );

    // late initialization to call after copy-constructing
    void Init( const Axis & rOther );

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

    // ____ XAxis ____
    virtual void SAL_CALL setScaleData( const ::com::sun::star::chart2::ScaleData& rScaleData )
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::chart2::ScaleData SAL_CALL getScaleData()
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference<
                ::com::sun::star::beans::XPropertySet > SAL_CALL getGridProperties()
                    throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence<
                ::com::sun::star::uno::Reference<
                        ::com::sun::star::beans::XPropertySet > > SAL_CALL getSubGridProperties()
                    throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence<
                ::com::sun::star::uno::Reference<
                    ::com::sun::star::beans::XPropertySet > > SAL_CALL getSubTickProperties()
                    throw (::com::sun::star::uno::RuntimeException);

    // ____ XTitled ____
    virtual ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::XTitle > SAL_CALL getTitleObject()
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setTitleObject(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XTitle >& Title )
        throw (::com::sun::star::uno::RuntimeException);

    // ____ XCloneable ____
    // Note: the coordinate systems are not cloned!
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

private: //methods
    void AllocateSubGrids();

private: //member

    ::com::sun::star::uno::Reference<
        ::com::sun::star::util::XModifyListener >   m_xModifyEventForwarder;

    ::com::sun::star::chart2::ScaleData             m_aScaleData;

    ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet >     m_xGrid;

    ::com::sun::star::uno::Sequence<
        ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet > >     m_aSubGridProperties;

    ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::XTitle >          m_xTitle;
};

} //  namespace chart

// CHART_AXIS_HXX
#endif

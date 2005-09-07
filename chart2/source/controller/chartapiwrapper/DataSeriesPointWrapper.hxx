/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DataSeriesPointWrapper.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 00:01:05 $
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
#ifndef CHART_DATASERIESPOINTWRAPPER_HXX
#define CHART_DATASERIESPOINTWRAPPER_HXX

#include "OPropertySet.hxx"
#include "ServiceMacros.hxx"

#ifndef _CPPUHELPER_IMPLBASE3_HXX_
#include <cppuhelper/implbase3.hxx>
#endif
#ifndef _COMPHELPER_UNO3_HXX_
#include <comphelper/uno3.hxx>
#endif
#ifndef _CPPUHELPER_INTERFACECONTAINER_HXX_
#include <cppuhelper/interfacecontainer.hxx>
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

#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif

#ifndef _COM_SUN_STAR_CHART2_XDATASERIES_HPP_
#include <com/sun/star/chart2/XDataSeries.hpp>
#endif

namespace chart
{
namespace wrapper
{

namespace impl
{
typedef ::cppu::WeakImplHelper3<
    com::sun::star::lang::XServiceInfo,
       com::sun::star::lang::XComponent,
       com::sun::star::lang::XEventListener >
    DataSeriesPointWrapper_Base;
}

class DataSeriesPointWrapper :
        public impl::DataSeriesPointWrapper_Base,
        public ::property::OPropertySet
{
public:
    enum eParentType
    {
        DATA_SERIES,
        DATA_POINT
    };

    DataSeriesPointWrapper( eParentType eType,
                            const ::com::sun::star::uno::Reference<
                                ::com::sun::star::beans::XPropertySet > & xParentProperties,
                            const ::com::sun::star::uno::Reference<
                                ::com::sun::star::uno::XComponentContext > & xContext,
                            ::osl::Mutex & _rMutex );
    virtual ~DataSeriesPointWrapper();

    ::osl::Mutex & GetMutex() const;

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

    virtual sal_Bool SAL_CALL convertFastPropertyValue
        ( ::com::sun::star::uno::Any & rConvertedValue,
          ::com::sun::star::uno::Any & rOldValue,
          sal_Int32 nHandle,
          const ::com::sun::star::uno::Any& rValue )
        throw (::com::sun::star::lang::IllegalArgumentException);

    virtual void SAL_CALL getFastPropertyValue
        ( ::com::sun::star::uno::Any& rValue,
          sal_Int32 nHandle ) const;

    virtual void SAL_CALL setFastPropertyValue_NoBroadcast(
        sal_Int32 nHandle,
        const ::com::sun::star::uno::Any& rValue )
        throw (::com::sun::star::uno::Exception);

    // ____ XPropertySet ____
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL
        getPropertySetInfo()
        throw (::com::sun::star::uno::RuntimeException);

    // ____ XComponent ____
    virtual void SAL_CALL dispose()
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference<
                                            ::com::sun::star::lang::XEventListener >& xListener )
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference<
                                               ::com::sun::star::lang::XEventListener >& aListener )
        throw (::com::sun::star::uno::RuntimeException);

    // ____ XEventListener ____
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source )
        throw (::com::sun::star::uno::RuntimeException);

private:

    void SAL_CALL getStatisticsPropertyValue
        ( ::com::sun::star::uno::Any& rValue,
          sal_Int32 nHandle ) const;

    void SAL_CALL setStatisticsPropertyValue_NoBroadcast(
        sal_Int32 nHandle, const ::com::sun::star::uno::Any& rValue )
        throw (::com::sun::star::uno::Exception);

    // ----------------------------------------

    mutable ::osl::Mutex &    m_rMutex;

    ::com::sun::star::uno::Reference<
        ::com::sun::star::uno::XComponentContext >
                        m_xContext;

    ::cppu::OInterfaceContainerHelper
                        m_aEventListenerContainer;

    eParentType         m_eParentType;

    ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet >
                        m_xParentProperties;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XFastPropertySet >
                        m_xParentFastProperties;
};

} //  namespace wrapper
} //  namespace chart

// CHART_DATASERIESPOINTWRAPPER_HXX
#endif

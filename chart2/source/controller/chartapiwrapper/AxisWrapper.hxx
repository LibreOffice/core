/*************************************************************************
 *
 *  $RCSfile: AxisWrapper.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: bm $ $Date: 2003-12-18 13:49:35 $
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
#ifndef CHART_AXISWRAPPER_HXX
#define CHART_AXISWRAPPER_HXX

#include "OPropertySet.hxx"
#include "ServiceMacros.hxx"

#ifndef _CPPUHELPER_IMPLBASE2_HXX_
#include <cppuhelper/implbase2.hxx>
#endif
#ifndef _COMPHELPER_UNO3_HXX_
#include <comphelper/uno3.hxx>
#endif
#ifndef _CPPUHELPER_INTERFACECONTAINER_HXX_
#include <cppuhelper/interfacecontainer.hxx>
#endif

#ifndef _COM_SUN_STAR_DRAWING_XSHAPE_HPP_
#include <com/sun/star/drawing/XShape.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif

#ifndef _DRAFTS_COM_SUN_STAR_CHART2_XDIAGRAM_HPP_
#include <drafts/com/sun/star/chart2/XDiagram.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif

namespace drafts {
namespace com { namespace sun { namespace star {
namespace chart2
{
    class XAxis;
}
}}}
}

namespace chart
{
namespace wrapper
{

namespace impl
{
typedef ::cppu::WeakImplHelper2<
    com::sun::star::lang::XComponent,
    com::sun::star::lang::XServiceInfo >
    AxisWrapper_Base;
}

class AxisWrapper :
        public impl::AxisWrapper_Base,
        public ::property::OPropertySet
{
public:
    enum eAxisType
    {
        X_AXIS,
        Y_AXIS,
        Z_AXIS,
        SECOND_X_AXIS,
        SECOND_Y_AXIS
    };

    AxisWrapper( eAxisType eType,
                  const ::com::sun::star::uno::Reference<
                      ::drafts::com::sun::star::chart2::XDiagram > & xDia,
                  const ::com::sun::star::uno::Reference<
                      ::com::sun::star::uno::XComponentContext > & xContext,
                  ::osl::Mutex & _rMutex );
    virtual ~AxisWrapper();

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

    virtual void SAL_CALL setFastPropertyValue_NoBroadcast
        ( sal_Int32 nHandle,
          const ::com::sun::star::uno::Any& rValue )
        throw (::com::sun::star::uno::Exception);

    virtual void SAL_CALL getFastPropertyValue
        ( ::com::sun::star::uno::Any& rValue,
          sal_Int32 nHandle ) const;

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

private:
    mutable ::osl::Mutex &    m_rMutex;

    ::com::sun::star::uno::Reference<
        ::com::sun::star::uno::XComponentContext >
                        m_xContext;

    eAxisType           m_eType;

    ::cppu::OInterfaceContainerHelper
                        m_aEventListenerContainer;

    ::com::sun::star::uno::Reference<
        ::drafts::com::sun::star::chart2::XDiagram >
                        m_xDiagram;

    ::com::sun::star::uno::Reference<
        ::drafts::com::sun::star::chart2::XAxis >
                        m_xAxis;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XPropertySet >
                        m_xAxisProperties;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::beans::XFastPropertySet >
                        m_xAxisFastProperties;

    /** precondition: m_xAxis.is(), mutex is locked
    */
    void getFastMeterPropertyValue( ::com::sun::star::uno::Any & rValue, sal_Int32 nHandle ) const;

    /** precondition: m_xAxis.is(), mutex is locked
     */
    void setFastMeterPropertyValue_NoBroadcast(
        sal_Int32 nHandle,
        const ::com::sun::star::uno::Any& rValue );
};

} //  namespace wrapper
} //  namespace chart

// CHART_AXISWRAPPER_HXX
#endif

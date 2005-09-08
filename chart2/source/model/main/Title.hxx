/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: Title.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 01:06:56 $
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
#ifndef CHART_TITLE_HXX
#define CHART_TITLE_HXX

#include "ServiceMacros.hxx"

#ifndef CHART_OPROPERTYSET_HXX
#include "OPropertySet.hxx"
#endif
#ifndef CHART_MUTEXCONTAINER_HXX
#include "MutexContainer.hxx"
#endif
#ifndef _CPPUHELPER_IMPLBASE3_HXX_
#include <cppuhelper/implbase3.hxx>
#endif
#ifndef _COMPHELPER_UNO3_HXX_
#include <comphelper/uno3.hxx>
#endif

#ifndef _COM_SUN_STAR_CHART2_XTITLE_HPP_
#include <com/sun/star/chart2/XTitle.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_CHART2_XIDENTIFIABLE_HPP_
#include <com/sun/star/chart2/XIdentifiable.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif

namespace chart
{

namespace impl
{
typedef ::cppu::WeakImplHelper3<
    ::com::sun::star::chart2::XTitle,
    ::com::sun::star::lang::XServiceInfo,
    ::com::sun::star::chart2::XIdentifiable >
    Title_Base;
}

class Title :
    public helper::MutexContainer,
    public impl::Title_Base,
    public ::property::OPropertySet
{
public:
    Title( ::com::sun::star::uno::Reference<
           ::com::sun::star::uno::XComponentContext > const & xContext );
    virtual ~Title();

    /// establish methods for factory instatiation
    APPHELPER_SERVICE_FACTORY_HELPER( Title )

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

    // ____ XTitle ____
    virtual ::com::sun::star::uno::Sequence<
        ::com::sun::star::uno::Reference<
        ::com::sun::star::chart2::XFormattedString > > SAL_CALL getText()
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setText( const ::com::sun::star::uno::Sequence<
                                   ::com::sun::star::uno::Reference<
                                   ::com::sun::star::chart2::XFormattedString > >& Strings )
        throw (::com::sun::star::uno::RuntimeException);

    // ____ XIdentifiable ____
    virtual ::rtl::OUString SAL_CALL getIdentifier()
        throw (::com::sun::star::uno::RuntimeException);

private:
    ::rtl::OUString m_aIdentifier;

    ::com::sun::star::uno::Sequence<
        ::com::sun::star::uno::Reference<
            ::com::sun::star::chart2::XFormattedString > > m_aStrings;
};

} //  namespace chart

// CHART_TITLE_HXX
#endif

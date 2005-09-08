/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: LayoutContainer.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 01:02:01 $
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
#ifndef CHART_LAYOUTCONTAINER_HXX
#define CHART_LAYOUTCONTAINER_HXX

#ifndef _CPPUHELPER_IMPLBASE2_HXX_
#include <cppuhelper/implbase2.hxx>
#endif

#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_LAYOUT_XLAYOUTCONTAINER_HPP_
#include <com/sun/star/layout/XLayoutContainer.hpp>
#endif

#include "ServiceMacros.hxx"

#include <vector>
#include <map>

namespace chart
{

class LayoutContainer : public
    ::cppu::WeakImplHelper2<
        ::com::sun::star::lang::XServiceInfo,
        ::com::sun::star::layout::XLayoutContainer >
{
public:
    LayoutContainer();
    virtual ~LayoutContainer();

    /// XServiceInfo declarations
    APPHELPER_XSERVICEINFO_DECL()

protected:
    // ____ XLayoutContainer ____
    virtual void SAL_CALL addConstrainedElementByIdentifier( const ::rtl::OUString& aIdentifier, const ::com::sun::star::layout::Constraint& Constraint )
        throw (::com::sun::star::layout::IllegalConstraintException,
               ::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addElementByIdentifier( const ::rtl::OUString& aIdentifier )
        throw (::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeElementByIdentifier( const ::rtl::OUString& aIdentifier )
        throw (::com::sun::star::container::NoSuchElementException,
               ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setConstraintByIdentifier( const ::rtl::OUString& aIdentifier, const ::com::sun::star::layout::Constraint& Constraint )
        throw (::com::sun::star::container::NoSuchElementException,
               ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::layout::Constraint SAL_CALL getConstraintByIdentifier( const ::rtl::OUString& aIdentifier )
        throw (::com::sun::star::container::NoSuchElementException,
               ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence<
        ::rtl::OUString > SAL_CALL getElementIdentifiers()
        throw (::com::sun::star::uno::RuntimeException);

private:
    typedef ::std::vector< ::rtl::OUString > tLayoutElements;

    typedef ::std::map<
        ::rtl::OUString,
        ::com::sun::star::layout::Constraint > tConstraintsMap;

    tLayoutElements           m_aLayoutElements;
    tConstraintsMap           m_aConstraints;
};

} //  namespace chart

// CHART_LAYOUTCONTAINER_HXX
#endif

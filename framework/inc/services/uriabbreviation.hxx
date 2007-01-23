/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: uriabbreviation.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2007-01-23 07:09:48 $
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

#ifndef __FRAMEWORK_SERVICES_URIABBREVIATION_HXX_
#define __FRAMEWORK_SERVICES_URIABBREVIATION_HXX_

#include <macros/xinterface.hxx>
#include <macros/xtypeprovider.hxx>
#include <macros/xserviceinfo.hxx>
#include <general.h>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/util/XStringAbbreviation.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <cppuhelper/weak.hxx>

namespace framework
{

class UriAbbreviation:
    public ::com::sun::star::lang::XTypeProvider,
    public ::com::sun::star::lang::XServiceInfo,
    public ::com::sun::star::util::XStringAbbreviation,
    public ::cppu::OWeakObject  // helper implements XInterface, XWeak
{
public:
    explicit UriAbbreviation(::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > const & context);

    FWK_DECLARE_XINTERFACE
    FWK_DECLARE_XTYPEPROVIDER
    DECLARE_XSERVICEINFO

    // ::com::sun::star::util::XStringAbbreviation:
    virtual ::rtl::OUString SAL_CALL abbreviateString(const ::com::sun::star::uno::Reference< ::com::sun::star::util::XStringWidth > & xStringWidth, ::sal_Int32 nWidth, const ::rtl::OUString & aString) throw (::com::sun::star::uno::RuntimeException);

private:
    UriAbbreviation(UriAbbreviation &); // not defined
    void operator =(UriAbbreviation &); // not defined

    virtual ~UriAbbreviation() {}

    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >  m_xContext;
};

} //    namespace framework

#endif  //  #ifndef __FRAMEWORK_SERVICES_URIABBREVIATION_HXX_

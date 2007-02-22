/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: WriterFilterDetection.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: os $ $Date: 2007-02-22 13:44:21 $
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

#ifndef _WRITERFILTER_DETECTION_HXX
#define _WRITERFILTER_DETECTION_HXX

#ifndef _COM_SUN_STAR_DOCUMENT_XEXTENDEDFILTERDETECTION_HPP_
#include <com/sun/star/document/XExtendedFilterDetection.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _CPPUHELPER_IMPLBASE2_HXX_
#include <cppuhelper/implbase2.hxx>
#endif
#ifndef INCLUDED_WRITERFILTERDLLAPI_H
#include <WriterFilterDllApi.hxx>
#endif

class WRITERFILTER_DLLPUBLIC WriterFilterDetection : public cppu::WeakImplHelper2
<
    com::sun::star::document::XExtendedFilterDetection,
    com::sun::star::lang::XServiceInfo
>
{
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > m_xContext;

public:
    WriterFilterDetection(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext);
    virtual ~WriterFilterDetection();

    //XExtendedFilterDetection
    virtual ::rtl::OUString SAL_CALL detect( com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue >& Descriptor )
        throw( com::sun::star::uno::RuntimeException );

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(  )
        throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
        throw (::com::sun::star::uno::RuntimeException);
};

::rtl::OUString WriterFilterDetection_getImplementationName()
    throw ( ::com::sun::star::uno::RuntimeException );

sal_Bool SAL_CALL WriterFilterDetection_supportsService( const ::rtl::OUString& ServiceName )
    throw ( ::com::sun::star::uno::RuntimeException );

::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL WriterFilterDetection_getSupportedServiceNames(  )
    throw ( ::com::sun::star::uno::RuntimeException );

::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL WriterFilterDetection_createInstance(
                                                                        const ::com::sun::star::uno::Reference<
                                                                        ::com::sun::star::uno::XComponentContext > &xContext)
    throw( ::com::sun::star::uno::Exception );

#endif


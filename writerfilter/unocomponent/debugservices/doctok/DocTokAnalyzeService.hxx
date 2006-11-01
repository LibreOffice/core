/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DocTokAnalyzeService.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hbrinkm $ $Date: 2006-11-01 09:14:37 $
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

/**
  Copyright 2005 Sun Microsystems, Inc.
*/

#ifndef INCLUDED_DOCTOK_ANALYZESERVICE_HXX
#define INCLUDED_DOCTOK_ANALYZESERVICE_HXX

#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif

#ifndef _COM_SUN_STAR_LANG_XMAIN_HPP_
#include <com/sun/star/lang/XMain.hpp>
#endif

#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif

namespace writerfilter { namespace doctoktest {

class AnalyzeService : public cppu::WeakImplHelper1 < ::com::sun::star::lang::XMain >
{
private:
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > xContext;

public:
    AnalyzeService(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > &xContext);

    virtual ::sal_Int32 SAL_CALL run( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aArguments ) throw (::com::sun::star::uno::RuntimeException);

public:
    const static sal_Char SERVICE_NAME[40];
    const static sal_Char IMPLEMENTATION_NAME[40];

};

::rtl::OUString AnalyzeService_getImplementationName ();
sal_Bool SAL_CALL AnalyzeService_supportsService( const ::rtl::OUString& ServiceName );
::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL AnalyzeService_getSupportedServiceNames(  ) throw (::com::sun::star::uno::RuntimeException);
::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL AnalyzeService_createInstance( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > &xContext) throw( ::com::sun::star::uno::Exception );

} } /* end namespace writerfilter::rtftok */

#define DOCTOK_ANALYZESERVICE_COMPONENT_ENTRY \
{\
    writerfilter::doctoktest::AnalyzeService_createInstance,\
    writerfilter::doctoktest::AnalyzeService_getImplementationName,\
    writerfilter::doctoktest::AnalyzeService_getSupportedServiceNames,\
   ::cppu::createSingleComponentFactory,\
   0, 0\
}


#endif /* INCLUDED_RTFTOK_ANALYZESERVICE_HXX */

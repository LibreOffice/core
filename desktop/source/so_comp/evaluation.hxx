/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: evaluation.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 17:47:28 $
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

/* makefile.mk changed 20030409, LO */

#ifndef _SOCOMP_EVALUATION_HXX_
#define _SOCOMP_EVALUATION_HXX_

#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_EXCEPTION_HPP_
#include <com/sun/star/uno/Exception.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif
#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XEVENTLISTENER_HPP_
#include <com/sun/star/lang/XEventListener.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XEXACTNAME_HPP_
#include <com/sun/star/beans/XExactName.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XMATERIALHOLDER_HPP_
#include <com/sun/star/beans/XMaterialHolder.hpp>
#endif
#ifndef _CPPUHELPER_IMPLBASE4_HXX_
#include <cppuhelper/implbase4.hxx>
#endif
#ifndef _CPPUHELPER_INTERFACECONTAINER_H_
#include <cppuhelper/interfacecontainer.h>
#endif

#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <osl/mutex.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;

namespace desktop {

class SOEvaluation : public ::cppu::WeakImplHelper4< XExactName, XMaterialHolder, XComponent, XServiceInfo >
{
    ::osl::Mutex                        m_aMutex;
    ::cppu::OInterfaceContainerHelper   m_aListeners;
    Reference< XMultiServiceFactory >   m_xServiceManager;

public:
                            SOEvaluation( const Reference < XMultiServiceFactory >& xFactory );
    virtual                 ~SOEvaluation();

    static Reference< XSingleServiceFactory >   GetSOEvaluationFactory( Reference< XMultiServiceFactory > & xSMgr );
    static ::rtl::OUString                      GetImplementationName();
    static Sequence< rtl::OUString >            GetSupportedServiceNames();

    // XComponent
    virtual void SAL_CALL               dispose() throw ( RuntimeException );
    virtual void SAL_CALL               addEventListener( const Reference< XEventListener > & aListener) throw ( RuntimeException );
    virtual void SAL_CALL               removeEventListener(const Reference< XEventListener > & aListener) throw ( RuntimeException );

    // XExactName
    virtual rtl::OUString SAL_CALL      getExactName( const rtl::OUString& rApproximateName ) throw ( RuntimeException );

    // XMaterialHolder
    virtual Any SAL_CALL                getMaterial() throw ( RuntimeException );

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL    getImplementationName() throw ( RuntimeException );
    virtual sal_Bool SAL_CALL           supportsService( const ::rtl::OUString& rServiceName ) throw ( RuntimeException );
    virtual Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw ( RuntimeException );

    static const char* interfaces[];
    static const char* implementationName;
    static const char* serviceName;
    static Reference<XInterface> SAL_CALL CreateInstance(
        const Reference< XMultiServiceFactory >&);



};
}
#endif // _SOCOMP_EVALUATION_HXX_

/*************************************************************************
 *
 *  $RCSfile: evaluation.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2003-04-24 13:35:57 $
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
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
};

#endif // _SOCOMP_EVALUATION_HXX_

/*************************************************************************
 *
 *  $RCSfile: iahndl.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:04:10 $
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

#ifndef UUI_IAHNDL_HXX
#define UUI_IAHNDL_HXX

#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XTYPEPROVIDER_HPP_
#include <com/sun/star/lang/XTypeProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_XINTERACTIONHANDLER_HPP_
#include <com/sun/star/task/XInteractionHandler.hpp>
#endif
#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif

namespace com { namespace sun { namespace star { namespace lang {
    class XMultiServiceFactory;
    class XSingleServiceFactory;
} } } }

//============================================================================
class UUIInteractionHandler:
    public cppu::OWeakObject,
    public com::sun::star::lang::XServiceInfo,
    public com::sun::star::lang::XTypeProvider,
    public com::sun::star::task::XInteractionHandler
{
public:
    static sal_Char const m_aImplementationName[];

    virtual com::sun::star::uno::Any SAL_CALL
    queryInterface(com::sun::star::uno::Type const & rType)
        throw (com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL acquire()
        throw (com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL release()
        throw (com::sun::star::uno::RuntimeException);

    virtual rtl::OUString SAL_CALL getImplementationName()
        throw (com::sun::star::uno::RuntimeException);

    virtual sal_Bool SAL_CALL supportsService(rtl::OUString const &
                                                  rServiceName)
        throw (com::sun::star::uno::RuntimeException);

    virtual com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL
    getSupportedServiceNames() throw (com::sun::star::uno::RuntimeException);

    virtual
    com::sun::star::uno::Sequence< com::sun::star::uno::Type > SAL_CALL
    getTypes() throw (com::sun::star::uno::RuntimeException);

    virtual com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL
    getImplementationId() throw (com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL
    handle(com::sun::star::uno::Reference<
                   com::sun::star::task::XInteractionRequest > const &
               rRequest)
        throw (com::sun::star::uno::RuntimeException);

    static com::sun::star::uno::Sequence< rtl::OUString >
    getSupportedServiceNames_static();

    static com::sun::star::uno::Reference< com::sun::star::uno::XInterface >
#if SUPD >= 590
    SAL_CALL
#endif // SUPD 590
    createInstance(com::sun::star::uno::Reference<
                       com::sun::star::lang::XMultiServiceFactory > const &);
};

#endif // UUI_IAHNDL_HXX


/*************************************************************************
 *
 *  $RCSfile: provider.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: dg $ $Date: 2000-11-10 22:42:46 $
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
#include "provider.hxx"

#ifndef CONFIGMGR_MODULE_HXX_
#include "configmodule.hxx"
#endif
#ifndef CONFIGMGR_CMTREEMODEL_HXX
#include "cmtreemodel.hxx"
#endif
#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef __SGI_STL_ALGORITHM
#include <stl/algorithm>
#endif
#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif
#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif

#define THISREF() static_cast< ::cppu::OWeakObject* >(this)

namespace configmgr
{
    namespace css  = ::com::sun::star;
    namespace uno  = css::uno;
    namespace lang = css::lang;
    namespace beans = css::beans;
    using ::rtl::OUString;
    using ::vos::ORef;
    using namespace osl;

    //=============================================================================
    //= OProvider
    //=============================================================================
    //-----------------------------------------------------------------------------
    OProvider::OProvider(Module& aModule, ServiceInfo const* pInfo)
              :ServiceComponentImpl(pInfo)
              ,m_aModule(aModule)
    {
    }

    // XTypeOProvider
    //-----------------------------------------------------------------------------
    uno::Sequence< uno::Type > SAL_CALL OProvider::getTypes(  ) throw(uno::RuntimeException)
    {
        return ::comphelper::concatSequences(ServiceComponentImpl::getTypes(), OProvider_Base::getTypes());
    }

    // XInterface
    //-----------------------------------------------------------------------------
    uno::Any SAL_CALL OProvider::queryInterface(uno::Type const& rType) throw(uno::RuntimeException)
    {
        uno::Any aRet( ServiceComponentImpl::queryInterface(rType) );
        if ( !aRet.hasValue() )
            aRet = OProvider_Base::queryInterface(rType);
        return aRet;
    }


    //-----------------------------------------------------------------------------
    void SAL_CALL OProvider::disposing()
    {
        sal_Bool bIsConnected = isConnected();
        if (bIsConnected)
        {
            ::osl::MutexGuard aGuard(m_aMutex);
            if (isConnected())
                disconnect();
        }
        ServiceComponentImpl::disposing();
    }

    // XInitialization
    //-----------------------------------------------------------------------------
    void SAL_CALL OProvider::initialize( const uno::Sequence< uno::Any >& _rArguments ) throw (uno::Exception, uno::RuntimeException)
    {
        ::osl::MutexGuard aGuard(m_aMutex);
        if (isConnected() || m_aSecurityOverride.size())
        {
            if (0 == _rArguments.getLength())
                // allow initialize without arguments ....
                return; //? Should this not ensureConnection() ?

            throw uno::Exception(::rtl::OUString::createFromAscii("The configuration OProvider has already been initialized."), THISREF());
        }

        const uno::Any* pArguments = _rArguments.getConstArray();
        beans::PropertyValue aCurrentArg;
        for (sal_Int32 i=0; i<_rArguments.getLength(); ++i, ++pArguments)
        {
            if (!((*pArguments) >>= aCurrentArg))
                throw lang::IllegalArgumentException(::rtl::OUString::createFromAscii("Arguments have to be com.sun.star.beans.PropertyValue's."), THISREF(), i);

            // no check if the argument is known and valid. This would require to much testing
            m_aSecurityOverride[aCurrentArg.Name] = aCurrentArg.Value;
        }

        // connect here and now, thus the createInstanceWithArguments fails if no connection is made
        connect();
    }
} // namespace configmgr



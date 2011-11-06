/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_cui.hxx"
#include "sdbcdriverenum.hxx"
#include <comphelper/stl_types.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

//........................................................................
namespace offapp
{
//........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::container;

    //====================================================================
    //= ODriverEnumerationImpl
    //====================================================================
    class ODriverEnumerationImpl
    {
    protected:
        ::std::vector< ::rtl::OUString >    m_aImplNames;

    public:
        ODriverEnumerationImpl();

        const ::std::vector< ::rtl::OUString >& getDriverImplNames() const { return m_aImplNames; }
    };

    //--------------------------------------------------------------------
    ODriverEnumerationImpl::ODriverEnumerationImpl()
    {
        try
        {
            Reference< XMultiServiceFactory > xORB = ::comphelper::getProcessServiceFactory();
            Reference< XInterface > xDM = xORB->createInstance(::rtl::OUString::createFromAscii("com.sun.star.sdbc.DriverManager"));
            OSL_ENSURE(xDM.is(), "ODriverEnumerationImpl::ODriverEnumerationImpl: no access to the SDBC driver manager!");

            Reference< XEnumerationAccess > xEnumAccess(xDM, UNO_QUERY);
            OSL_ENSURE(xEnumAccess.is() || !xDM.is(), "ODriverEnumerationImpl::ODriverEnumerationImpl: can't enumerate SDBC drivers (missing the interface)!");
            if (xEnumAccess.is())
            {
                Reference< XEnumeration > xEnumDrivers = xEnumAccess->createEnumeration();
                OSL_ENSURE(xEnumDrivers.is(), "ODriverEnumerationImpl::ODriverEnumerationImpl: invalid enumeration object!");

                Reference< XServiceInfo > xDriverSI;
                while (xEnumDrivers->hasMoreElements())
                {
                    xEnumDrivers->nextElement() >>= xDriverSI;
                    OSL_ENSURE(xDriverSI.is(), "ODriverEnumerationImpl::ODriverEnumerationImpl: driver without service info!");
                    if (xDriverSI.is())
                        m_aImplNames.push_back(xDriverSI->getImplementationName());
                }
            }
        }
        catch(const Exception&)
        {
            OSL_ENSURE(sal_False, "ODriverEnumerationImpl::ODriverEnumerationImpl: caught an exception while enumerating the drivers!");
        }
    }

    //====================================================================
    //= ODriverEnumeration
    //====================================================================
    //--------------------------------------------------------------------
    ODriverEnumeration::ODriverEnumeration() throw()
        :m_pImpl(new ODriverEnumerationImpl)
    {
    }

    //--------------------------------------------------------------------
    ODriverEnumeration::~ODriverEnumeration() throw()
    {
        delete m_pImpl;
    }

    //--------------------------------------------------------------------
    ODriverEnumeration::const_iterator ODriverEnumeration::begin() const throw()
    {
        return m_pImpl->getDriverImplNames().begin();
    }

    //--------------------------------------------------------------------
    ODriverEnumeration::const_iterator ODriverEnumeration::end() const throw()
    {
        return m_pImpl->getDriverImplNames().end();
    }
//........................................................................
}   // namespace offapp
//........................................................................



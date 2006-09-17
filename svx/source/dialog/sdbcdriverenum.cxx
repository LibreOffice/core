/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sdbcdriverenum.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 04:37:23 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#ifdef SVX_DLLIMPLEMENTATION
#undef SVX_DLLIMPLEMENTATION
#endif

#ifndef _OFFMGR_SDBCDRIVERENUMERATION_HXX_
#include "sdbcdriverenum.hxx"
#endif
#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XENUMERATIONACCESS_HPP_
#include <com/sun/star/container/XEnumerationAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif

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

    //--------------------------------------------------------------------
    sal_Int32 ODriverEnumeration::size() const throw()
    {
        return m_pImpl->getDriverImplNames().size();
    }

//........................................................................
}   // namespace offapp
//........................................................................


